#!/usr/bin/env python3
"""
K-tiling 理论分析与模拟

分析三个维度：
1. Working set 分析：不同 K_tile 下每个 panel 的 gather 数据量 vs cache 容量
2. Cache 命中率模型：估算 L1/L2 命中率变化
3. 时间模型：基于命中率估算加速比

硬件参数（Xeon Max 9462）：
  L1D: 48 KB/core, ~4 cycle latency（约 1.5ns @ 2.6GHz）
  L2:  2 MB/core, ~12 cycle latency（约 4.6ns）
  LLC: 共享 112.5 MB, ~40 cycle latency（约 15ns）
  HBM: ~150 cycle latency（约 58ns），带宽 ~1 TB/s 共享 64 核
  cache line: 64 bytes
"""

import struct
import numpy as np
import os

def read_csrbin(path):
    """读取 .csrbin 二进制格式"""
    with open(path, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('III', f.read(12))
        nrow, ncol, nnz = struct.unpack('QQQ', f.read(24))
        indptr = np.frombuffer(f.read((nrow+1)*4), dtype=np.uint32)
        indices = np.frombuffer(f.read(nnz*4), dtype=np.uint32)
    return nrow, ncol, nnz, indptr, indices

# ============================================================
# 硬件参数
# ============================================================
L1_SIZE = 48 * 1024      # 48 KB per core
L2_SIZE = 2 * 1024 * 1024  # 2 MB per core
CACHELINE = 64            # 64 bytes per cache line

# 访问延迟（cycles @ ~2.6 GHz）
LAT_L1 = 4       # L1 命中
LAT_L2 = 12      # L2 命中（L1 miss）
LAT_LLC = 40     # LLC 命中（L2 miss）
LAT_HBM = 150    # HBM 访问（LLC miss）

BF16 = 2  # bytes per BF16 element

def simulate_panels(nrow, ncol, nnz, indptr, indices, max_panels=3000):
    """
    模拟列锚点分组，返回每个 panel 的 U（unique columns）列表
    和行度数分布
    """
    TILE_R = 16
    col_deg = np.bincount(indices, minlength=ncol)
    col_order = np.argsort(col_deg)[::-1]
    row_degrees = np.diff(indptr).astype(np.int64)
    assigned = np.zeros(nrow, dtype=bool)
    
    # 构建 CSC
    col_ptr = np.zeros(ncol + 1, dtype=np.int64)
    col_ptr[1:] = np.cumsum(col_deg)
    col_indices = np.empty(nnz, dtype=np.int64)
    col_pos = col_ptr[:-1].copy()
    for i in range(nrow):
        for idx in range(indptr[i], indptr[i+1]):
            j = indices[idx]
            col_indices[col_pos[j]] = i
            col_pos[j] += 1
    
    panel_U_list = []
    panel_nnz_list = []
    total_panels = 0
    
    for ci in range(min(ncol, len(col_order))):
        c = col_order[ci]
        if col_deg[c] == 0:
            break
        rows_with_c = col_indices[col_ptr[c]:col_ptr[c+1]]
        avail = rows_with_c[~assigned[rows_with_c]]
        if len(avail) < 2:
            continue
        
        if len(avail) > TILE_R:
            k = min(TILE_R, len(avail)) - 1
            part_idx = np.argpartition(row_degrees[avail], k)[:TILE_R]
            selected = avail[part_idx]
        else:
            selected = avail
        
        panel_cols = set()
        panel_nnz = 0
        for r in selected:
            cols_r = indices[indptr[r]:indptr[r+1]]
            panel_cols.update(cols_r)
            panel_nnz += len(cols_r)
        
        U = len(panel_cols)
        panel_U_list.append(U)
        panel_nnz_list.append(panel_nnz)
        assigned[selected] = True
        total_panels += 1
        
        if total_panels >= max_panels:
            break
    
    # Fallback 行的度数分布
    fb_degrees = row_degrees[~assigned]
    
    return {
        'panel_U': np.array(panel_U_list),
        'panel_nnz': np.array(panel_nnz_list),
        'n_panels': total_panels,
        'fb_degrees': fb_degrees,
        'assigned': int(np.sum(assigned)),
        'fb_rows': int(np.sum(~assigned)),
    }


def analyze_ktiling(panel_U, K=128, K_tiles=[32, 64, 128]):
    """
    对比不同 K_tile 下的 working set 和 cache 行为
    
    模型假设：
    - Gather B 时，每个 panel 的 U 行数据是随机分散在 B 矩阵中的
    - 每次 gather 的 working set = U × K_tile × BF16 bytes
    - 如果 working set < L1_SIZE → 假设 90% L1 hit
    - 如果 working set < L2_SIZE → 假设 70% L2 hit, 20% L1 hit
    - 否则 → 假设 50% LLC hit, 30% L2 hit
    
    注意：这是简化模型。实际 cache 行为取决于：
    - B 行的物理地址分布（是否跨 cache set 冲突）
    - 其他线程的 cache 争抢
    - prefetch 效果
    但这个模型足够指导方向选择
    """
    
    results = {}
    
    for kt in K_tiles:
        n_passes = K // kt  # 每个 panel 需要几个 K-tile pass
        
        # 每个 panel 每个 K-tile pass 的 gather 数据量
        # gather_bytes[i] = panel_U[i] × K_tile × BF16
        gather_bytes = panel_U * kt * BF16
        
        # 分类统计：fits in L1 / L2 / LLC / HBM
        fits_L1 = gather_bytes < L1_SIZE          # < 48 KB
        fits_L2 = (gather_bytes >= L1_SIZE) & (gather_bytes < L2_SIZE)  # 48KB ~ 2MB
        fits_LLC = (gather_bytes >= L2_SIZE)       # > 2MB，理论上不会出现 for K_tile<=128
        
        pct_L1 = np.mean(fits_L1) * 100
        pct_L2 = np.mean(fits_L2) * 100
        pct_LLC = np.mean(fits_LLC) * 100
        
        # 估算每个 panel 的 gather 时间（cycles）
        # 核心假设：
        #   gather 一行 B[j, K_tile_start:K_tile_start+K_tile]:
        #     需要读 K_tile × BF16 = K_tile × 2 bytes
        #     = ceil(K_tile × 2 / 64) 个 cache line
        #   如果在 L1 → 每 cache line 花 LAT_L1 cycles
        #   如果在 L2 → 每 cache line 花 LAT_L2 cycles
        #   etc.
        
        cachelines_per_row = int(np.ceil(kt * BF16 / CACHELINE))
        
        # 简化模型：panel 的所有 U 行在同一 cache 层级
        # （因为它们属于同一次 gather burst）
        gather_cycles = np.zeros(len(panel_U))
        
        # L1 fit: 大部分 gather 命中 L1（gather buffer 在 L1 内循环使用）
        # 但首次读 B 行仍然是 L2/LLC miss（B 矩阵太大）
        # 所以实际模型：
        #   - 读 B 行本身：总是从 L2/LLC/HBM 来（B 矩阵远大于 cache）
        #   - gather buffer（组装好的 tile 数据）：如果 < L1 则留在 L1
        #   
        # 真正的收益在于：
        #   1. gather buffer 小 → 更多 L1 空间给 A tile 和 C tile
        #   2. 每次 tileload 从 gather buffer 读数据时命中 L1
        #   3. 多个 KC pass 之间不互相驱逐

        # 更准确的模型：
        # 对于每个 B 行的读取，延迟取决于 B 行是否在 cache 中
        # B 矩阵大小 = N × K × BF16，通常 >> LLC
        # 所以每次 gather B 行 ≈ LLC miss → HBM access
        # 
        # 但如果 K_tile 较小，每次只读 B[j, kt_start:kt_start+kt]
        # = kt × 2 bytes = 1-4 个 cache line
        # 而如果 K=128，一次读 256 bytes = 4 个 cache line
        #
        # 关键洞察：B 行的不同 K_tile 段落在不同的 cache line 中
        # 所以 K-tiling 不会让 B 行的同一段被重复读取
        # K-tiling 的真正收益是让 gather buffer 更小！
        
        # gather buffer 大小 = U × K_tile × BF16
        # 如果 buffer < L1:
        #   tileload 从 buffer 读 → L1 hit (4 cycles/CL)
        #   后续 KC pass 的 tileload 也命中 L1
        # 如果 buffer > L1:
        #   tileload 可能 L1 miss → L2 hit (12 cycles/CL)
        
        # 总 gather 时间 = 
        #   读 B 行时间（从 HBM，不变） + 
        #   tileload 时间（从 gather buffer，受 K-tiling 影响）
        
        # 读 B 行时间（对所有 K_tile 方案相同的部分）：
        # 每个 panel 读 U 行 × K_tile 列，每行 ceil(K_tile×2/64) 个 cacheline
        # 首次读：假设从 L2 来（如果 B 列的 K_tile 段刚好在 L2 热区）
        # 否则从 HBM 来
        # 简化：假设 70% 从 L2，30% 从 HBM（基于实测 LLC miss ~30%）
        avg_read_lat = 0.7 * LAT_L2 + 0.3 * LAT_HBM  # ~53 cycles/CL
        
        read_B_cycles = panel_U * cachelines_per_row * avg_read_lat
        
        # tileload 时间（受 K-tiling 影响）：
        # 每个 KC pass 需要 tileload gather buffer 中的 B tile
        # n_tiles = ceil(U/32) per KC pass
        # 每个 tile = 16×32×2 = 1024 bytes = 16 cache lines
        # 如果 gather buffer 在 L1 → 4 cycles/CL
        # 如果溢出 L1 → 12 cycles/CL (L2)
        
        tiles_per_pass = np.ceil(panel_U / 32).astype(int)
        CL_per_tile = 16  # 1024 bytes / 64 bytes
        
        tileload_lat = np.where(
            gather_bytes < L1_SIZE,
            LAT_L1,   # buffer 在 L1
            LAT_L2    # buffer 溢出到 L2
        )
        tileload_cycles = tiles_per_pass * CL_per_tile * tileload_lat * n_passes
        
        # AMX compute 时间（不受 K-tiling 影响，TDPBF16PS 吞吐固定）
        # 每个 tile 做 1 次 TDPBF16PS ≈ 16 cycles
        compute_cycles = tiles_per_pass * 16 * n_passes
        
        # panel 开销（每个 K-tile pass 重复一次）
        # 索引查找、地址计算等 ≈ 50 cycles/panel/pass
        overhead_cycles = np.full(len(panel_U), 50 * n_passes)
        
        # A tile 重读（K-tiling 的代价）
        # 每个 K-tile pass 需要重新 tileload A
        # A tile 已经预打包成连续内存，应该在 L1/L2
        # 每个 panel 有 ceil(U/32) 个 A tile
        # 每个 A tile = 16×32×2 = 1024 bytes
        a_reread_cycles = tiles_per_pass * CL_per_tile * LAT_L1 * (n_passes - 1)
        
        # C 写回的额外开销（K-tiling 时 C 可能被驱逐）
        # 每个 K-tile pass 写 16 × K_tile × 4 bytes 到 C
        # 如果 K_tile=32: 16×32×4 = 2048 bytes（2KB，L1 内）
        # 后续 pass 需要 read-modify-write（但 C 段很小，应该在 L1）
        c_rw_cycles = np.full(len(panel_U), 
                              16 * kt // 16 * LAT_L1 * max(0, n_passes - 1))
        
        total_cycles = (read_B_cycles + tileload_cycles + compute_cycles + 
                       overhead_cycles + a_reread_cycles + c_rw_cycles)
        
        results[kt] = {
            'K_tile': kt,
            'n_passes': n_passes,
            'gather_bytes_mean': np.mean(gather_bytes),
            'gather_bytes_p50': np.median(gather_bytes),
            'gather_bytes_p95': np.percentile(gather_bytes, 95),
            'pct_fits_L1': pct_L1,
            'pct_fits_L2': pct_L2,
            'total_cycles_mean': np.mean(total_cycles),
            'read_B_cycles_mean': np.mean(read_B_cycles),
            'tileload_cycles_mean': np.mean(tileload_cycles),
            'compute_cycles_mean': np.mean(compute_cycles),
            'a_reread_cycles_mean': np.mean(a_reread_cycles),
            'overhead_cycles_mean': np.mean(overhead_cycles),
        }
    
    return results


def analyze_fb_ktiling(fb_degrees, K=128, K_tiles=[32, 64, 128]):
    """
    分析 K-tiling 对 Fallback 路径的影响
    
    FB 路径：逐行处理，每行 gather deg 个 B 行
    当前：每个 B 行读 K×BF16 = 256 bytes
    K-tiling：每个 B 行读 K_tile×BF16 bytes
    
    FB 路径的 working set = deg × K_tile × BF16
    如果能装进 L1，后续行可能复用之前行 cache 的 B 行
    """
    results = {}
    
    for kt in K_tiles:
        n_passes = K // kt
        ws = fb_degrees * kt * BF16
        
        pct_L1 = np.mean(ws < L1_SIZE) * 100
        pct_L2 = np.mean((ws >= L1_SIZE) & (ws < L2_SIZE)) * 100
        
        results[kt] = {
            'K_tile': kt,
            'n_passes': n_passes,
            'ws_mean_KB': np.mean(ws) / 1024,
            'ws_p95_KB': np.percentile(ws, 95) / 1024,
            'pct_fits_L1': pct_L1,
        }
    
    return results


# ============================================================
# 主程序
# ============================================================
if __name__ == '__main__':
    DATA_DIR = os.path.expanduser('~/data/SpMM_project/data')
    K = 128
    K_TILES = [16, 32, 64, 128]  # 128 = 当前方案（无 K-tiling）
    
    matrices = [
        'web-Google', 'amazon0601', 'soc-Pokec',
        'as-Skitter', 'cit-Patents', 'indochina-2004', 'hollywood-2009'
    ]
    
    print("=" * 90)
    print(f"K-Tiling Analysis for AMX SpMM (K={K})")
    print("=" * 90)
    print(f"Hardware: L1D={L1_SIZE//1024}KB, L2={L2_SIZE//1024//1024}MB, "
          f"CL={CACHELINE}B")
    print(f"Latency: L1={LAT_L1}cy, L2={LAT_L2}cy, LLC={LAT_LLC}cy, "
          f"HBM={LAT_HBM}cy")
    print()
    
    all_summaries = []
    
    for mname in matrices:
        csrbin = os.path.join(DATA_DIR, mname, f'{mname}.csrbin')
        if not os.path.exists(csrbin):
            print(f"SKIP: {mname}")
            continue
        
        print(f"{'#'*70}")
        print(f"# {mname}")
        print(f"{'#'*70}")
        
        nrow, ncol, nnz, indptr, indices = read_csrbin(csrbin)
        avg_deg = nnz / nrow
        print(f"M={nrow:,}  N={ncol:,}  NNZ={nnz:,}  avg_deg={avg_deg:.1f}")
        
        # 模拟 panel 分组
        print("Simulating panels...")
        pinfo = simulate_panels(nrow, ncol, nnz, indptr, indices)
        panel_U = pinfo['panel_U']
        
        print(f"Panels: {pinfo['n_panels']}, "
              f"AMX rows: {pinfo['assigned']:,}, "
              f"FB rows: {pinfo['fb_rows']:,}")
        print(f"Panel U: mean={panel_U.mean():.0f}  "
              f"median={np.median(panel_U):.0f}  "
              f"P95={np.percentile(panel_U, 95):.0f}  "
              f"max={panel_U.max()}")
        
        # ================================================
        # AMX 路径 K-tiling 分析
        # ================================================
        print(f"\n--- AMX Path: K-tiling analysis ---")
        print(f"{'K_tile':>7} {'passes':>7} {'gatherMean':>11} {'gatherP95':>11} "
              f"{'FitsL1%':>8} {'tileload':>10} {'A_reread':>10} "
              f"{'total_cy':>10} {'vs_K128':>8}")
        print("-" * 88)
        
        amx_results = analyze_ktiling(panel_U, K, K_TILES)
        baseline_cycles = amx_results[128]['total_cycles_mean']
        
        best_kt = 128
        best_ratio = 1.0
        
        for kt in K_TILES:
            r = amx_results[kt]
            ratio = baseline_cycles / r['total_cycles_mean'] if r['total_cycles_mean'] > 0 else 0
            if ratio > best_ratio:
                best_ratio = ratio
                best_kt = kt
            
            print(f"{kt:>7} {r['n_passes']:>7} "
                  f"{r['gather_bytes_mean']/1024:>10.1f}KB "
                  f"{r['gather_bytes_p95']/1024:>10.1f}KB "
                  f"{r['pct_fits_L1']:>7.1f}% "
                  f"{r['tileload_cycles_mean']:>10.0f} "
                  f"{r['a_reread_cycles_mean']:>10.0f} "
                  f"{r['total_cycles_mean']:>10.0f} "
                  f"{ratio:>7.2f}x")
        
        print(f"  → Best K_tile = {best_kt} ({best_ratio:.2f}x vs K=128)")
        
        # ================================================
        # Fallback 路径 K-tiling 分析
        # ================================================
        if len(pinfo['fb_degrees']) > 0:
            print(f"\n--- FB Path: K-tiling working set ---")
            fb_results = analyze_fb_ktiling(pinfo['fb_degrees'], K, K_TILES)
            
            print(f"{'K_tile':>7} {'passes':>7} {'ws_mean':>10} "
                  f"{'ws_P95':>10} {'FitsL1%':>8}")
            print("-" * 50)
            for kt in K_TILES:
                r = fb_results[kt]
                print(f"{kt:>7} {r['n_passes']:>7} "
                      f"{r['ws_mean_KB']:>9.1f}KB "
                      f"{r['ws_p95_KB']:>9.1f}KB "
                      f"{r['pct_fits_L1']:>7.1f}%")
        
        # ================================================
        # 大 U panel 分析（K-tiling 收益最大的 panel）
        # ================================================
        print(f"\n--- Large-U panels (U>128, K-tiling impact) ---")
        large_U = panel_U[panel_U > 128]
        if len(large_U) > 0:
            pct_large = len(large_U) / len(panel_U) * 100
            print(f"Count: {len(large_U)} ({pct_large:.1f}% of panels)")
            print(f"U mean={large_U.mean():.0f}  P95={np.percentile(large_U, 95):.0f}")
            
            # 这些 panel 在不同 K_tile 下的 working set
            for kt in K_TILES:
                ws = large_U * kt * BF16
                fits = np.mean(ws < L1_SIZE) * 100
                print(f"  K_tile={kt:>3}: gather mean={ws.mean()/1024:.1f}KB  "
                      f"FitsL1={fits:.0f}%")
        else:
            print(f"  No panels with U>128")
        
        all_summaries.append({
            'matrix': mname,
            'avg_deg': avg_deg,
            'avg_U': panel_U.mean(),
            'best_kt': best_kt,
            'best_ratio': best_ratio,
            'L1_fit_128': amx_results[128]['pct_fits_L1'],
            'L1_fit_32': amx_results[32]['pct_fits_L1'],
        })
        
        print()
    
    # ============================================================
    # 汇总表
    # ============================================================
    print("=" * 90)
    print("SUMMARY: K-tiling potential")
    print("=" * 90)
    print(f"{'Matrix':<18} {'avg_deg':>8} {'avg_U':>7} "
          f"{'L1fit@128':>10} {'L1fit@32':>10} {'BestKt':>7} {'Speedup':>8}")
    print("-" * 70)
    for s in all_summaries:
        print(f"{s['matrix']:<18} {s['avg_deg']:>8.1f} {s['avg_U']:>7.0f} "
              f"{s['L1_fit_128']:>9.1f}% {s['L1_fit_32']:>9.1f}% "
              f"{s['best_kt']:>7} {s['best_ratio']:>7.2f}x")
    
    print(f"\n关键结论:")
    print(f"  - L1fit@32 vs L1fit@128 的差值大 → K-tiling 收益大")
    print(f"  - BestKt < 128 → 该矩阵适合 K-tiling")
    print(f"  - Speedup 仅为 AMX 路径的估算，E2E 提升取决于 AMX 占比")
    print(f"  - 注意：模型是简化的，实际收益需要 C 代码验证")
