#!/usr/bin/env python3
"""
全量覆盖分析 - 高速版

优化策略:
  1. CSC 构建用 numpy 向量化（避免 Python for 循环）
  2. 只处理 top-K 高频列（而非全部列）
  3. 行选择用 numpy 数组操作
  4. 每个矩阵限制最大处理时间

用法: python3 full_coverage_fast.py all
"""

import sys, os, struct
import numpy as np
import time

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


def build_csc_fast(indptr, indices, M, N):
    """
    numpy 向量化 CSR→CSC 转换
    
    原理: 不用 Python for 循环
      1. np.bincount 统计每列的非零数 → col_counts
      2. np.cumsum 构建 col_ptr
      3. 用向量化的行号扩展 + argsort 按列排序
    """
    t0 = time.time()
    nnz = int(indptr[M])
    
    # 生成每个非零对应的行号（向量化）
    # row_ids[k] = 第 k 个非零属于哪一行
    row_ids = np.empty(nnz, dtype=np.int32)
    for i in range(M):
        row_ids[indptr[i]:indptr[i+1]] = i
    # 上面这个循环虽然是 Python，但每次迭代只做一次 slice 赋值，很快
    
    col_ids = indices[:nnz].copy()
    
    # 按列排序
    sort_idx = np.argsort(col_ids, kind='mergesort')
    sorted_cols = col_ids[sort_idx]
    sorted_rows = row_ids[sort_idx]
    
    # col_ptr
    col_counts = np.bincount(col_ids.astype(np.int64), minlength=N)
    col_ptr = np.zeros(N + 1, dtype=np.int64)
    np.cumsum(col_counts, out=col_ptr[1:])
    
    elapsed = time.time() - t0
    print(f"    CSC 构建: {elapsed:.1f}s (向量化)")
    return col_ptr, sorted_rows, col_counts


def analyze_panel_fast(rows, indptr, indices):
    """向量化 panel 分析"""
    # 收集所有非零的列索引
    all_cols = []
    total_deg = 0
    for r in rows:
        s = int(indptr[r]); e = int(indptr[r+1])
        total_deg += (e - s)
        all_cols.append(indices[s:e])
    
    if total_deg == 0:
        return 0, 0, 0, 0
    
    merged = np.concatenate(all_cols)
    U = len(np.unique(merged))
    
    if U == 0:
        return 0, 0, 0, 0
    
    n_tiles = (U + TILE_COLS - 1) // TILE_COLS
    capacity = TILE_ROWS * TILE_COLS * n_tiles
    fill_rate = total_deg / capacity
    
    return total_deg, U, fill_rate, n_tiles


def full_coverage_fast(indptr, indices, M, N, col_ptr, sorted_rows, col_counts):
    """
    高速完整覆盖模拟
    
    算法:
      Phase 1: 按列频率降序，从高频列的行中取未分配的 16 行
               只处理频率 >= 16 的列（否则凑不满 panel）
               用 numpy boolean 数组跟踪分配状态
      Phase 2: 剩余行连续分组
    """
    print(f"\n  === 全量覆盖模拟 (高速版) ===")
    t0 = time.time()
    
    assigned = np.zeros(M, dtype=np.bool_)
    degrees = (indptr[1:M+1] - indptr[:M]).astype(np.int32)
    
    # Phase 1: 列锚点分组
    # 只处理频率 >= 2 的列
    valid_cols = np.where(col_counts >= 2)[0]
    col_order = valid_cols[np.argsort(col_counts[valid_cols])[::-1]]
    
    # 只处理频率 >= 16 的列（能凑满 panel 的）
    freq_threshold = 2  # 至少 2 行才组 panel
    
    amx_panels = []
    amx_stats = []  # (deg, U, fill, n_tiles)
    
    total_nnz = int(indptr[M])
    amx_nnz = 0
    amx_B_union = 0
    
    processed_cols = 0
    max_cols = min(len(col_order), M)  # 安全上限
    
    checkpoint = time.time()
    
    for idx in range(max_cols):
        col = int(col_order[idx])
        
        if col_counts[col] < freq_threshold:
            break
        
        # 取该列的所有行
        cs = int(col_ptr[col]); ce = int(col_ptr[col+1])
        col_rows = sorted_rows[cs:ce]
        
        # 过滤已分配的行（向量化）
        mask = ~assigned[col_rows]
        available = col_rows[mask]
        
        if len(available) < 2:
            continue
        
        # 取度数最小的前 16 行
        avail_degs = degrees[available]
        if len(available) > TILE_ROWS:
            top_idx = np.argpartition(avail_degs, TILE_ROWS)[:TILE_ROWS]
            panel = available[top_idx]
        else:
            panel = available
        
        # 分析 panel
        deg, U, fill, n_tiles = analyze_panel_fast(panel, indptr, indices)
        
        if deg > 0 and fill >= 0.0625:  # 过盈亏线才走 AMX
            assigned[panel] = True
            amx_panels.append(panel)
            amx_stats.append((deg, U, fill, n_tiles))
            amx_nnz += deg
            amx_B_union += U
        
        processed_cols += 1
        
        # 每 10 秒打印进度
        now = time.time()
        if now - checkpoint > 10:
            pct = np.sum(assigned) / M * 100
            print(f"    进度: {processed_cols} 列, "
                  f"{len(amx_panels)} panels, "
                  f"覆盖 {pct:.1f}%, "
                  f"耗时 {now-t0:.0f}s")
            checkpoint = now
        
        # 超时保护: 5 分钟
        if now - t0 > 300:
            print(f"    [超时] 5 分钟到，提前结束 Phase 1")
            break
        
        # 如果已覆盖 95% 的行，提前停止
        if np.sum(assigned) > 0.95 * M:
            break
    
    phase1_time = time.time() - t0
    
    # Phase 2: 剩余行（AVX-512）
    unassigned = np.where(~assigned)[0]
    avx_nnz = 0
    avx_panels_count = 0
    avx_fills = []
    
    for i in range(0, len(unassigned), TILE_ROWS):
        panel = unassigned[i:i+TILE_ROWS]
        deg, U, fill, n_tiles = analyze_panel_fast(panel, indptr, indices)
        if deg > 0:
            avx_nnz += deg
            avx_panels_count += 1
            avx_fills.append(fill)
    
    total_time = time.time() - t0
    
    # === 统计 ===
    amx_rows = int(np.sum(assigned))
    avx_rows = M - amx_rows
    
    print(f"\n  --- Phase 1: 列锚点 AMX 路径 ---")
    print(f"  处理列数: {processed_cols:,}")
    print(f"  AMX panels: {len(amx_panels):,}")
    print(f"  AMX 行覆盖: {amx_rows:,} / {M:,} ({amx_rows/M*100:.1f}%)")
    print(f"  AMX NNZ覆盖: {amx_nnz:,} / {total_nnz:,} ({amx_nnz/total_nnz*100:.1f}%)")
    
    if amx_stats:
        fills = np.array([s[2] for s in amx_stats])
        Us = np.array([s[1] for s in amx_stats])
        print(f"  填充率: 均值={np.mean(fills)*100:.2f}%, "
              f"中位={np.median(fills)*100:.2f}%, "
              f"P75={np.percentile(fills,75)*100:.2f}%, "
              f"P95={np.percentile(fills,95)*100:.2f}%")
        if amx_B_union > 0:
            print(f"  B 节省: {amx_nnz/amx_B_union:.2f}x")
        print(f"  U: 均值={np.mean(Us):.1f}, 中位={np.median(Us):.0f}")
        print(f"  U≤32: {np.mean(Us<=32)*100:.1f}%, U≤64: {np.mean(Us<=64)*100:.1f}%")
        
        # 填充率分布
        bins = [0.0625, 0.10, 0.15, 0.20, 0.30, 0.50, 0.75, 1.01]
        hist, _ = np.histogram(fills, bins=bins)
        total = len(fills)
        for k in range(len(hist)):
            pct = hist[k] / total * 100 if total > 0 else 0
            bar = '#' * int(pct / 2)
            print(f"    [{bins[k]*100:5.1f}%,{bins[k+1]*100:5.1f}%) : "
                  f"{hist[k]:>6} ({pct:5.1f}%) {bar}")
    
    print(f"\n  --- Phase 2: AVX-512 路径 ---")
    print(f"  AVX panels: {avx_panels_count:,}")
    print(f"  AVX 行: {avx_rows:,} ({avx_rows/M*100:.1f}%)")
    print(f"  AVX NNZ: {avx_nnz:,} ({avx_nnz/total_nnz*100:.1f}%)")
    
    # 全局加速估算
    print(f"\n  --- 全局加速估算 ---")
    total_B_original = total_nnz
    total_B_new = amx_B_union + avx_nnz
    mem_speedup = total_B_original / total_B_new if total_B_new > 0 else 1
    bf16_speedup = total_B_original / (amx_B_union/2 + avx_nnz) if (amx_B_union/2 + avx_nnz) > 0 else 1
    
    print(f"  原始 B 读取: {total_B_original:,}")
    print(f"  新 B 读取: {total_B_new:,} (AMX去重={amx_B_union:,} + AVX={avx_nnz:,})")
    print(f"  全局 B 节省: {mem_speedup:.2f}x")
    print(f"  + BF16 存储: {bf16_speedup:.2f}x")
    print(f"  耗时: {total_time:.1f}s (Phase1: {phase1_time:.1f}s)")
    
    return {
        'amx_row_pct': amx_rows / M * 100,
        'amx_nnz_pct': amx_nnz / total_nnz * 100 if total_nnz > 0 else 0,
        'amx_fill_avg': float(np.mean([s[2] for s in amx_stats])) * 100 if amx_stats else 0,
        'amx_B_save': amx_nnz / amx_B_union if amx_B_union > 0 else 0,
        'mem_speedup': mem_speedup,
        'bf16_speedup': bf16_speedup,
        'n_panels': len(amx_panels),
    }


def analyze_matrix(csrbin_path):
    name = os.path.basename(os.path.dirname(csrbin_path))
    if not name or name == '.':
        name = os.path.basename(csrbin_path).replace('.csrbin', '')
    
    print(f"\n{'#'*70}")
    print(f"#  {name}")
    print(f"{'#'*70}")
    
    indptr, indices, values, M, N, nnz = read_csrbin(csrbin_path)
    avg_deg = nnz / M if M > 0 else 0
    print(f"  {M:,} x {N:,}, NNZ={nnz:,}, avg_deg={avg_deg:.1f}")
    
    if nnz < 1000:
        print(f"  [跳过: NNZ 太少]")
        return None
    
    col_ptr, sorted_rows, col_counts = build_csc_fast(indptr, indices, M, N)
    
    print(f"  列度数: 均值={np.mean(col_counts):.1f}, "
          f"中位={np.median(col_counts):.0f}, "
          f"P95={np.percentile(col_counts,95):.0f}, "
          f"max={np.max(col_counts)}")
    
    # 高频列统计
    hot16 = np.sum(col_counts >= 16)
    hot100 = np.sum(col_counts >= 100)
    print(f"  高频列: ≥16行={hot16:,}, ≥100行={hot100:,}")
    
    result = full_coverage_fast(indptr, indices, M, N,
                                 col_ptr, sorted_rows, col_counts)
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
    print(f"  全量完整覆盖分析 (高速版)")
    print(f"{'='*70}")
    
    entries = sorted(os.listdir(data_dir))
    all_results = []
    
    for d in entries:
        subdir = os.path.join(data_dir, d)
        if not os.path.isdir(subdir):
            continue
        for f in sorted(os.listdir(subdir)):
            if f.endswith('.csrbin'):
                path = os.path.join(subdir, f)
                try:
                    result = analyze_matrix(path)
                    if result:
                        all_results.append(result)
                except Exception as e:
                    print(f"  [错误] {d}: {e}")
                    import traceback
                    traceback.print_exc()
    
    # 全局汇总表
    if all_results:
        print(f"\n\n{'='*100}")
        print(f"  全量汇总（按 B 节省排序）")
        print(f"{'='*100}")
        fmt = "  {:<22} {:>8} {:>10} {:>10} {:>10} {:>10} {:>8} {:>8}"
        print(fmt.format("矩阵", "avg_deg", "AMX行%", "AMX_NNZ%",
                         "AMX_fill", "AMX_panels", "B节省", "+BF16"))
        print(f"  {'_'*95}")
        for r in sorted(all_results, key=lambda x: -x['mem_speedup']):
            print(fmt.format(
                r['name'][:22],
                f"{r['avg_deg']:.1f}",
                f"{r['amx_row_pct']:.1f}%",
                f"{r['amx_nnz_pct']:.1f}%",
                f"{r['amx_fill_avg']:.1f}%",
                f"{r['n_panels']:,}",
                f"{r['mem_speedup']:.2f}x",
                f"{r['bf16_speedup']:.2f}x"))
        
        # 分类汇总
        print(f"\n  --- 分类 ---")
        good = [r for r in all_results if r['mem_speedup'] >= 1.5]
        mid  = [r for r in all_results if 1.1 <= r['mem_speedup'] < 1.5]
        bad  = [r for r in all_results if r['mem_speedup'] < 1.1]
        print(f"  显著加速 (B节省≥1.5×): {len(good)} 个矩阵")
        print(f"  有效加速 (1.1-1.5×):    {len(mid)} 个矩阵")
        print(f"  效果有限 (<1.1×):        {len(bad)} 个矩阵")


if __name__ == "__main__":
    main()
