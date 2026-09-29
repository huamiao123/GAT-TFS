#!/usr/bin/env python3
"""
完整覆盖分析：模拟实际分区过程

关键问题:
  1. 列锚点能覆盖多少比例的行？
  2. 剩余行（孤儿行）的特征是什么？
  3. AMX路径 vs AVX-512路径的行数分配？
  4. 全局加权填充率是多少？

同时测试改进的列锚点策略:
  策略 A: 列锚点-简单 (前16行)
  策略 B: 列锚点-相似 (从高频列的行中选最相似的16行)
  策略 C: 完整覆盖模拟 (贪心分配所有行)

在所有可用矩阵上测试。

用法: python3 full_coverage_analysis.py <file.csrbin>
"""

import sys, os, struct
import numpy as np
import time
from collections import defaultdict

TILE_ROWS = 16
TILE_COLS = 32

def read_csrbin(filepath):
    with open(filepath, 'rb') as f:
        ptype = struct.unpack('<I', f.read(4))[0]
        dtype = struct.unpack('<I', f.read(4))[0]
        vtype = struct.unpack('<I', f.read(4))[0]
        nrow  = struct.unpack('<Q', f.read(8))[0]
        ncol  = struct.unpack('<Q', f.read(8))[0]
        nnz   = struct.unpack('<Q', f.read(8))[0]
        indptr  = np.frombuffer(f.read((nrow+1)*4), dtype=np.uint32).astype(np.int64)
        indices = np.frombuffer(f.read(nnz*4),      dtype=np.uint32).astype(np.int32)
        values  = np.frombuffer(f.read(nnz*4),      dtype=np.float32)
    return indptr, indices, values, int(nrow), int(ncol), int(nnz)


def analyze_panel(rows, indptr, indices, M):
    col_set = set()
    deg = 0
    for r in rows:
        if r >= M: continue
        s = int(indptr[r]); e = int(indptr[r+1])
        deg += (e - s)
        for idx in range(s, e):
            col_set.add(int(indices[idx]))
    U = len(col_set)
    if U == 0 or deg == 0:
        return 0, 0, 0, 0, 0
    overlap = deg / U
    n_tiles = (U + TILE_COLS - 1) // TILE_COLS
    capacity = TILE_ROWS * TILE_COLS * n_tiles
    fill_rate = deg / capacity
    return deg, U, overlap, n_tiles, fill_rate


def build_csc(indptr, indices, M, N):
    """CSR → CSC"""
    t0 = time.time()
    nnz = int(indptr[M])
    col_counts = np.zeros(N, dtype=np.int64)
    for idx in range(nnz):
        c = int(indices[idx])
        if c < N: col_counts[c] += 1
    col_ptr = np.zeros(N + 1, dtype=np.int64)
    np.cumsum(col_counts, out=col_ptr[1:])
    row_indices = np.empty(nnz, dtype=np.int32)
    write_pos = col_ptr[:-1].copy()
    for i in range(M):
        s = int(indptr[i]); e = int(indptr[i+1])
        for idx in range(s, e):
            c = int(indices[idx])
            if c < N:
                row_indices[int(write_pos[c])] = i
                write_pos[c] += 1
    return col_ptr, row_indices, col_counts, time.time() - t0


def get_row_cols(i, indptr, indices):
    s = int(indptr[i]); e = int(indptr[i+1])
    return set(int(indices[idx]) for idx in range(s, e))


def full_coverage_simulation(indptr, indices, M, N, col_ptr, row_indices, col_counts):
    """
    完整覆盖模拟:
    
    Phase 1: 列锚点分组
      按列频率降序，每次取一个高频列
      从该列的行中选未分配的行，组成 panel
      在行选择时，优先选与已选行列重叠最大的行（贪心）
    
    Phase 2: 剩余行
      用连续分组处理（AVX-512 路径）
    
    统计: AMX 覆盖率、全局填充率、B 节省
    """
    print(f"\n  === 完整覆盖模拟 ===")
    t0 = time.time()
    
    assigned = np.zeros(M, dtype=np.bool_)
    degrees = np.array([int(indptr[i+1] - indptr[i]) for i in range(M)])
    
    # Phase 1: 列锚点分组
    # 按列频率降序
    col_order = np.argsort(col_counts)[::-1]
    
    amx_panels = []
    amx_fills = []
    amx_overlaps = []
    amx_U_values = []
    amx_B_total = 0
    amx_B_union = 0
    amx_nnz = 0
    
    max_panels = M // 8  # 安全上限
    
    for col in col_order:
        col = int(col)
        if col_counts[col] < 2:
            break  # 不再有高频列
        if len(amx_panels) >= max_panels:
            break
            
        # 取该列未分配的行
        cs = int(col_ptr[col]); ce = int(col_ptr[col+1])
        available = []
        for idx in range(cs, ce):
            r = int(row_indices[idx])
            if r < M and not assigned[r]:
                available.append(r)
        
        if len(available) < 2:
            continue
        
        # 简单版: 取度数最小的前 16 行（小度数行列集合小 → U 小）
        available.sort(key=lambda r: degrees[r])
        panel = available[:16]
        
        # 标记已分配
        for r in panel:
            assigned[r] = True
        
        deg, U, overlap, n_tiles, fill = analyze_panel(panel, indptr, indices, M)
        if deg > 0 and fill >= 0.0625:  # 只有过盈亏线的才走 AMX
            amx_panels.append(panel)
            amx_fills.append(fill)
            amx_overlaps.append(overlap)
            amx_U_values.append(U)
            amx_B_total += deg
            amx_B_union += U
            amx_nnz += deg
        else:
            # 不过盈亏线的，退回未分配
            for r in panel:
                assigned[r] = False
    
    phase1_time = time.time() - t0
    
    # Phase 2: 剩余行 (AVX-512 路径)
    unassigned = np.where(~assigned)[0]
    avx_panels = []
    avx_fills = []
    avx_nnz = 0
    
    for i in range(0, len(unassigned), TILE_ROWS):
        panel = unassigned[i:i+TILE_ROWS].tolist()
        deg, U, overlap, n_tiles, fill = analyze_panel(panel, indptr, indices, M)
        if deg > 0:
            avx_panels.append(panel)
            avx_fills.append(fill)
            avx_nnz += deg
    
    total_time = time.time() - t0
    total_nnz = int(indptr[M])
    
    # 统计
    amx_rows = int(np.sum(assigned))
    avx_rows = M - amx_rows
    amx_fills = np.array(amx_fills) if amx_fills else np.array([0.0])
    avx_fills = np.array(avx_fills) if avx_fills else np.array([0.0])
    
    print(f"\n  --- Phase 1: 列锚点 AMX 路径 ---")
    print(f"  AMX panels: {len(amx_panels):,}")
    print(f"  AMX 行覆盖: {amx_rows:,} / {M:,} ({amx_rows/M*100:.1f}%)")
    print(f"  AMX NNZ覆盖: {amx_nnz:,} / {total_nnz:,} ({amx_nnz/total_nnz*100:.1f}%)")
    print(f"  填充率: 均值={np.mean(amx_fills)*100:.2f}%, "
          f"中位={np.median(amx_fills)*100:.2f}%")
    if amx_overlaps:
        print(f"  overlap: 均值={np.mean(amx_overlaps):.2f}x")
    if amx_B_union > 0:
        print(f"  B 节省: {amx_B_total/amx_B_union:.2f}x")
    if amx_U_values:
        U_arr = np.array(amx_U_values)
        print(f"  U: 均值={np.mean(U_arr):.1f}, 中位={np.median(U_arr):.0f}")
        print(f"  U≤32: {np.mean(U_arr<=32)*100:.1f}%, U≤64: {np.mean(U_arr<=64)*100:.1f}%")
    
    # AMX 填充率分布
    bins = [0.0625, 0.10, 0.15, 0.20, 0.30, 0.50, 0.75, 1.01]
    hist, _ = np.histogram(amx_fills, bins=bins)
    total = len(amx_fills)
    print(f"  AMX 填充率分布:")
    for k in range(len(hist)):
        pct = hist[k] / total * 100 if total > 0 else 0
        bar = '#' * int(pct / 2)
        print(f"    [{bins[k]*100:5.1f}%,{bins[k+1]*100:5.1f}%) : "
              f"{hist[k]:>6} ({pct:5.1f}%) {bar}")
    
    print(f"\n  --- Phase 2: AVX-512 路径 (剩余行) ---")
    print(f"  AVX panels: {len(avx_panels):,}")
    print(f"  AVX 行: {avx_rows:,} ({avx_rows/M*100:.1f}%)")
    print(f"  AVX NNZ: {avx_nnz:,} ({avx_nnz/total_nnz*100:.1f}%)")
    if len(avx_fills) > 0:
        print(f"  AVX 填充率: 均值={np.mean(avx_fills)*100:.2f}%")
    
    # 全局加速估算
    print(f"\n  --- 全局加速估算 ---")
    amx_fill_avg = float(np.mean(amx_fills)) if len(amx_fills) > 0 else 0
    amx_speedup = amx_fill_avg * 16  # AMX 有效吞吐 vs AVX-512
    amx_B_save = amx_B_total / amx_B_union if amx_B_union > 0 else 1
    amx_frac = amx_nnz / total_nnz if total_nnz > 0 else 0
    avx_frac = 1 - amx_frac
    
    # Memory-bound 场景: 加速 ≈ B 读取减少
    # AMX 路径 B 读取 = amx_B_union (去重后)
    # AVX 路径 B 读取 = avx_nnz (每个非零读一次)
    total_B_original = total_nnz
    total_B_new = amx_B_union + avx_nnz
    mem_speedup = total_B_original / total_B_new if total_B_new > 0 else 1
    
    print(f"  AMX 有效吞吐: {amx_speedup:.2f}x AVX-512")
    print(f"  AMX B节省: {amx_B_save:.2f}x")
    print(f"  NNZ 分配: AMX {amx_frac*100:.1f}% + AVX {avx_frac*100:.1f}%")
    print(f"  全局 B 读取节省: {mem_speedup:.2f}x")
    print(f"  (原始: {total_B_original:,} → 新: {total_B_new:,})")
    
    # BF16 额外节省
    bf16_speedup = total_B_original / (total_B_new / 2) if total_B_new > 0 else 1
    print(f"  + BF16 B存储的全局节省: {bf16_speedup:.2f}x")
    
    print(f"\n  总耗时: {total_time:.2f}s (Phase1: {phase1_time:.2f}s)")
    
    return {
        'amx_row_pct': amx_rows / M * 100,
        'amx_nnz_pct': amx_nnz / total_nnz * 100,
        'amx_fill_avg': amx_fill_avg * 100,
        'amx_B_save': amx_B_save,
        'mem_speedup': mem_speedup,
        'bf16_speedup': bf16_speedup,
    }


def analyze_matrix(csrbin_path):
    name = os.path.basename(os.path.dirname(csrbin_path))
    if not name or name == '.':
        name = os.path.basename(csrbin_path).replace('.csrbin', '')
    
    print(f"\n{'#'*70}")
    print(f"#  {name}")
    print(f"{'#'*70}")
    
    indptr, indices, values, M, N, nnz = read_csrbin(csrbin_path)
    avg_deg = nnz / M
    print(f"  {M:,}x{N:,}, NNZ={nnz:,}, avg_deg={avg_deg:.1f}")
    
    # 小矩阵或太稀疏的跳过详细分析
    if nnz < 1000:
        print(f"  [跳过: NNZ 太少]")
        return None
    
    col_ptr, row_indices, col_counts, csc_time = build_csc(indptr, indices, M, N)
    print(f"  CSC 构建: {csc_time:.1f}s")
    print(f"  列度数: 均值={np.mean(col_counts):.1f}, "
          f"中位={np.median(col_counts):.0f}, "
          f"P95={np.percentile(col_counts,95):.0f}, "
          f"max={np.max(col_counts)}")
    
    result = full_coverage_simulation(indptr, indices, M, N,
                                       col_ptr, row_indices, col_counts)
    result['name'] = name
    result['M'] = M
    result['nnz'] = nnz
    result['avg_deg'] = avg_deg
    return result


def main():
    data_dir = os.path.expanduser("~/data/SpMM_project/data")
    
    if len(sys.argv) > 1 and sys.argv[1] != "all":
        path = sys.argv[1]
        if not os.path.exists(path):
            candidate = os.path.join(data_dir, path, path + '.csrbin')
            if os.path.exists(candidate):
                path = candidate
        analyze_matrix(path)
        return
    
    # 全量测试
    print(f"{'='*70}")
    print(f"  全量完整覆盖分析")
    print(f"{'='*70}")
    
    entries = sorted(os.listdir(data_dir))
    all_results = []
    
    for d in entries:
        subdir = os.path.join(data_dir, d)
        if not os.path.isdir(subdir):
            continue
        for f in os.listdir(subdir):
            if f.endswith('.csrbin'):
                path = os.path.join(subdir, f)
                try:
                    result = analyze_matrix(path)
                    if result:
                        all_results.append(result)
                except Exception as e:
                    print(f"  [错误] {path}: {e}")
                    import traceback
                    traceback.print_exc()
    
    # 全局汇总表
    if all_results:
        print(f"\n\n{'='*90}")
        print(f"  全量汇总")
        print(f"{'='*90}")
        fmt = "  {:<20} {:>8} {:>10} {:>10} {:>10} {:>8} {:>8} {:>8}"
        print(fmt.format("矩阵", "avg_deg", "AMX行%", "AMX_NNZ%",
                         "AMX_fill", "B节省", "mem加速", "+BF16"))
        print(f"  {'_'*85}")
        for r in sorted(all_results, key=lambda x: -x['mem_speedup']):
            print(fmt.format(
                r['name'][:20],
                f"{r['avg_deg']:.1f}",
                f"{r['amx_row_pct']:.1f}%",
                f"{r['amx_nnz_pct']:.1f}%",
                f"{r['amx_fill_avg']:.1f}%",
                f"{r['amx_B_save']:.1f}x",
                f"{r['mem_speedup']:.2f}x",
                f"{r['bf16_speedup']:.2f}x"))


if __name__ == "__main__":
    main()
