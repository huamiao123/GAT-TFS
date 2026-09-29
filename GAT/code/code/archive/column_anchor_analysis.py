#!/usr/bin/env python3
"""
列锚点分组 vs MinHash 对比

核心思路:
  MinHash: 全局搜索列集合相似的行 → 好但慢且有废品
  列锚点: 找被很多行共享的"热门列"，以它为锚点聚集行 → 直接且快

方法:
  1. 构建 CSC（列反向索引）
  2. 找高频列（被最多行共享的列）
  3. 对每个高频列，取共享它的行组成 panel
  4. 进一步优化：在共享同一列的行中，优先选共享更多其他列的行

用法: python3 column_anchor_analysis.py <file.csrbin>
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
    assert indptr[0] == 0 and indptr[nrow] == nnz
    return indptr, indices, values, int(nrow), int(ncol), int(nnz)


def get_row_cols_set(i, indptr, indices, M):
    start = int(indptr[i])
    end = int(indptr[i+1])
    return set(int(indices[idx]) for idx in range(start, end) if indices[idx] < M)


def analyze_panel(rows, indptr, indices, M):
    col_set = set()
    deg = 0
    for r in rows:
        if r >= M:
            continue
        start = int(indptr[r])
        end = int(indptr[r+1])
        deg += (end - start)
        for idx in range(start, end):
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
    """CSR → CSC: 构建列反向索引"""
    print(f"    构建 CSC (列反向索引)...", end=' ', flush=True)
    t0 = time.time()
    col_counts = np.zeros(N, dtype=np.int64)
    nnz = int(indptr[M])
    for idx in range(nnz):
        c = int(indices[idx])
        if c < N:
            col_counts[c] += 1

    col_ptr = np.zeros(N + 1, dtype=np.int64)
    np.cumsum(col_counts, out=col_ptr[1:])

    row_indices = np.empty(nnz, dtype=np.int32)
    write_pos = col_ptr[:-1].copy()
    for i in range(M):
        start = int(indptr[i])
        end = int(indptr[i+1])
        for idx in range(start, end):
            c = int(indices[idx])
            if c < N:
                wp = int(write_pos[c])
                row_indices[wp] = i
                write_pos[c] += 1

    print(f"{time.time()-t0:.1f}s")
    return col_ptr, row_indices, col_counts


def run_strategy(name, panels, indptr, indices, M):
    fills = []
    overlaps = []
    U_values = []
    B_total = 0
    B_union = 0
    for panel in panels:
        deg, U, overlap, n_tiles, fill = analyze_panel(panel, indptr, indices, M)
        if deg > 0:
            fills.append(fill)
            overlaps.append(overlap)
            U_values.append(U)
            B_total += deg
            B_union += U
    fills = np.array(fills) if fills else np.array([0.0])
    overlaps = np.array(overlaps) if overlaps else np.array([0.0])
    U_arr = np.array(U_values) if U_values else np.array([0])

    print(f"\n  === {name} ===")
    print(f"  panels: {len(fills)}")
    print(f"  填充率: 均值={np.mean(fills)*100:.2f}%, "
          f"中位={np.median(fills)*100:.2f}%, "
          f"P75={np.percentile(fills,75)*100:.2f}%, "
          f"P95={np.percentile(fills,95)*100:.2f}%")
    print(f"  overlap: 均值={np.mean(overlaps):.2f}x, "
          f"中位={np.median(overlaps):.2f}x")
    if B_union > 0:
        print(f"  B 节省: {B_total/B_union:.2f}x")
    print(f"  U: 均值={np.mean(U_arr):.1f}, 中位={np.median(U_arr):.0f}")

    # 填充率分布
    bins = [0, 0.0625, 0.10, 0.15, 0.20, 0.30, 0.50, 0.75, 1.01]
    hist, _ = np.histogram(fills, bins=bins)
    total = len(fills)
    for k in range(len(hist)):
        bar_len = int(hist[k] / total * 50) if total > 0 else 0
        bar = '#' * bar_len
        pct = hist[k] / total * 100 if total > 0 else 0
        print(f"  [{bins[k]*100:5.1f}%,{bins[k+1]*100:5.1f}%) : "
              f"{hist[k]:>6} ({pct:5.1f}%) {bar}")

    # U ≤ 32 的比例
    u32_pct = np.mean(U_arr <= 32) * 100
    u64_pct = np.mean(U_arr <= 64) * 100
    print(f"  U≤32: {u32_pct:.1f}%, U≤64: {u64_pct:.1f}%")

    return {'name': name, 'avg_fill': float(np.mean(fills)),
            'median_fill': float(np.median(fills)),
            'p95_fill': float(np.percentile(fills, 95)),
            'avg_U': float(np.mean(U_arr)),
            'B_savings': B_total / B_union if B_union > 0 else 0}


def main():
    if len(sys.argv) < 2:
        print("用法: python3 column_anchor_analysis.py <file.csrbin>")
        sys.exit(1)

    path = sys.argv[1]
    data_dir = os.path.expanduser("~/data/SpMM_project/data")
    if not os.path.exists(path):
        candidate = os.path.join(data_dir, path, path + '.csrbin')
        if os.path.exists(candidate):
            path = candidate

    name = os.path.basename(os.path.dirname(path))

    print(f"\n{'#'*70}")
    print(f"#  列锚点分组分析: {name}")
    print(f"{'#'*70}")

    indptr, indices, values, M, N, nnz = read_csrbin(path)
    avg_deg = nnz / M
    print(f"  {M}x{N}, NNZ={nnz:,}, avg_deg={avg_deg:.1f}")

    # 构建 CSC
    col_ptr, row_indices, col_counts = build_csc(indptr, indices, M, N)

    # 列的度数分布
    print(f"\n  --- 列度数分布 ---")
    print(f"  均值={np.mean(col_counts):.1f}, 中位={np.median(col_counts):.0f}, "
          f"P95={np.percentile(col_counts,95):.0f}, max={np.max(col_counts)}")

    rng = np.random.RandomState(42)
    sample_size = 3000

    results = []

    # ============================================================
    # 策略 1: 连续 16 行 (baseline)
    # ============================================================
    print(f"\n{'='*65}")
    print(f"  策略 1: 连续 16 行 (baseline)")
    n_panels = (M + TILE_ROWS - 1) // TILE_ROWS
    panel_ids = rng.choice(n_panels, min(sample_size, n_panels), replace=False)
    panels_consec = []
    for pi in panel_ids:
        r_start = pi * TILE_ROWS
        r_end = min(r_start + TILE_ROWS, M)
        panels_consec.append(list(range(r_start, r_end)))
    results.append(run_strategy("连续16行", panels_consec, indptr, indices, M))

    # ============================================================
    # 策略 2: 简单邻居分组
    # ============================================================
    print(f"\n{'='*65}")
    print(f"  策略 2: 邻居分组")
    degrees = np.array([int(indptr[i+1] - indptr[i]) for i in range(M)])
    candidates = np.where((degrees >= 5) & (degrees <= 300))[0]
    if len(candidates) == 0:
        candidates = np.where(degrees >= 2)[0]
    sampled = rng.choice(candidates, min(sample_size, len(candidates)), replace=False)

    panels_nb = []
    for seed in sampled:
        seed = int(seed)
        start = int(indptr[seed])
        end = int(indptr[seed+1])
        nbs = [int(indices[idx]) for idx in range(start, end)][:15]
        panels_nb.append([seed] + nbs)
    results.append(run_strategy("邻居分组", panels_nb, indptr, indices, M))

    # ============================================================
    # 策略 3: 列锚点 - 简单版
    # ============================================================
    # 找高频列，取共享该列的前 16 行
    print(f"\n{'='*65}")
    print(f"  策略 3: 列锚点 (高频列 + 前16行)")
    t0 = time.time()

    # 选高频列 (度数 >= 16，即至少有 16 行共享)
    hot_cols = np.where(col_counts >= 16)[0]
    print(f"  高频列 (度数>=16): {len(hot_cols):,}")

    if len(hot_cols) > sample_size:
        hot_sampled = rng.choice(hot_cols, sample_size, replace=False)
    else:
        hot_sampled = hot_cols

    panels_anchor_simple = []
    for col in hot_sampled:
        col = int(col)
        cs = int(col_ptr[col])
        ce = int(col_ptr[col+1])
        rows = [int(row_indices[idx]) for idx in range(cs, min(ce, cs+16))]
        if len(rows) >= 2:
            panels_anchor_simple.append(rows)
    print(f"  耗时: {time.time()-t0:.2f}s")
    results.append(run_strategy("列锚点-简单", panels_anchor_simple, indptr, indices, M))

    # ============================================================
    # 策略 4: 列锚点 - 贪心 U 控制版
    # ============================================================
    # 高频列的行中，贪心选择使 U 最小的 16 行
    print(f"\n{'='*65}")
    print(f"  策略 4: 列锚点 + 贪心 U 控制")
    t0 = time.time()

    panels_anchor_greedy = []
    for col in hot_sampled[:sample_size]:
        col = int(col)
        cs = int(col_ptr[col])
        ce = int(col_ptr[col+1])
        all_rows = [int(row_indices[idx]) for idx in range(cs, ce)]
        if len(all_rows) < 2:
            continue

        # 从度数最小的行开始（小集合不容易扩大 U）
        all_rows_sorted = sorted(all_rows, key=lambda r: int(indptr[r+1] - indptr[r]))

        group = [all_rows_sorted[0]]
        group_cols = get_row_cols_set(all_rows_sorted[0], indptr, indices, M)

        for r in all_rows_sorted[1:]:
            if len(group) >= 16:
                break
            r_cols = get_row_cols_set(r, indptr, indices, M)
            new_U = len(group_cols | r_cols)
            # 只加入不会让 U 暴增的行
            if new_U <= len(group_cols) + 16:  # 允许每行最多新增 16 列
                group.append(r)
                group_cols |= r_cols

        if len(group) >= 2:
            panels_anchor_greedy.append(group)

    print(f"  耗时: {time.time()-t0:.2f}s")
    results.append(run_strategy("列锚点-贪心", panels_anchor_greedy, indptr, indices, M))

    # ============================================================
    # 策略 5: 多列锚点 (取共享 ≥2 个高频列的行)
    # ============================================================
    print(f"\n{'='*65}")
    print(f"  策略 5: 多列锚点 (共享 ≥2 个高频列)")
    t0 = time.time()

    # 取 top-100 高频列
    top_k = 200
    top_cols = np.argsort(col_counts)[-top_k:][::-1]

    # 对每个行，计算它出现在多少个 top 列中
    row_hot_count = defaultdict(list)  # row -> list of hot cols it has
    for col in top_cols:
        col = int(col)
        cs = int(col_ptr[col])
        ce = int(col_ptr[col+1])
        for idx in range(cs, ce):
            r = int(row_indices[idx])
            row_hot_count[r].append(col)

    # 找共享 >= 2 个热门列的行对
    # 用热门列组合作为 key 来分桶
    pair_buckets = defaultdict(list)
    for row, hot_list in row_hot_count.items():
        if len(hot_list) >= 2:
            # 用前 2 个热门列的组合作为 key
            for i in range(min(len(hot_list), 5)):
                for j in range(i+1, min(len(hot_list), 5)):
                    key = (hot_list[i], hot_list[j])
                    pair_buckets[key].append(row)

    # 从大桶中取 panel
    panels_multi = []
    used = set()
    sorted_buckets = sorted(pair_buckets.values(), key=len, reverse=True)
    for bucket in sorted_buckets:
        available = [r for r in bucket if r not in used]
        if len(available) < 2:
            continue
        panel = available[:16]
        for r in panel:
            used.add(r)
        panels_multi.append(panel)
        if len(panels_multi) >= sample_size:
            break

    print(f"  生成 {len(panels_multi)} 个多锚点 panel")
    print(f"  耗时: {time.time()-t0:.2f}s")
    if panels_multi:
        results.append(run_strategy("多列锚点", panels_multi, indptr, indices, M))

    # ============================================================
    # 汇总
    # ============================================================
    print(f"\n\n{'='*80}")
    print(f"  最终汇总: {name}")
    print(f"{'='*80}")
    fmt = "  {:<18} {:>10} {:>10} {:>10} {:>8} {:>8}"
    print(fmt.format("策略", "均值fill", "中位fill", "P95_fill", "平均U", "B节省"))
    print(f"  {'_'*70}")
    for r in results:
        print(fmt.format(
            r['name'],
            f"{r['avg_fill']*100:.2f}%",
            f"{r['median_fill']*100:.2f}%",
            f"{r['p95_fill']*100:.2f}%",
            f"{r['avg_U']:.0f}",
            f"{r['B_savings']:.2f}x"))

    best = max(results, key=lambda x: x['avg_fill'])
    print(f"\n  最佳: {best['name']}, fill={best['avg_fill']*100:.2f}%")


if __name__ == "__main__":
    main()
