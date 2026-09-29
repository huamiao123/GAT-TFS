#!/usr/bin/env python3
"""
MinHash 分组 + U 分布分析

两个关键实验:
1. U 分布: 看 U 的阶梯效应 (U<=32 → fill 31%, U<=64 → fill 15%)
2. MinHash: 直接按列集合相似度分组，不依赖图距离

用法: python3 minhash_group_analysis.py <file.csrbin> [sample_size]
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
    """返回 (deg, U, overlap, n_tiles, fill_rate)"""
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


def print_U_distribution(U_values, label):
    """打印 U 分布，重点关注阶梯边界"""
    U_arr = np.array(U_values)
    print(f"\n  --- {label}: U 分布 ---")
    print(f"  U 均值={np.mean(U_arr):.1f}, 中位={np.median(U_arr):.1f}, "
          f"P25={np.percentile(U_arr,25):.0f}, P75={np.percentile(U_arr,75):.0f}")

    # 阶梯边界统计
    thresholds = [32, 64, 96, 128, 192, 256, 512]
    total = len(U_arr)
    print(f"")
    print(f"  {'U范围':<20} {'数量':>8} {'占比':>8} {'该范围fill理论值':>20}")
    print(f"  {'_'*60}")
    prev = 0
    for th in thresholds:
        count = np.sum((U_arr > prev) & (U_arr <= th))
        pct = count / total * 100
        # 理论 fill: 假设 16行 × avg 10 nnz = 160 total_deg
        # fill = 160 / (16 × 32 × ceil(mid/32)) 其中 mid = (prev+th)/2
        mid = (prev + th) / 2
        n_tiles = int(np.ceil(mid / 32))
        if n_tiles > 0:
            theo_fill = 160 / (16 * 32 * n_tiles) * 100
        else:
            theo_fill = 0
        bar = '#' * int(pct / 2)
        print(f"  U in ({prev:>3},{th:>3}]  {count:>8} ({pct:>5.1f}%) "
              f"  ~{theo_fill:.1f}%  {bar}")
        prev = th
    count = np.sum(U_arr > thresholds[-1])
    pct = count / total * 100
    print(f"  U > {thresholds[-1]:<5}       {count:>8} ({pct:>5.1f}%)")


def run_strategy_with_U(name, panel_list, indptr, indices, M):
    """运行策略，同时收集 U 分布"""
    fills = []
    overlaps = []
    U_values = []
    B_total = 0
    B_union = 0

    for panel in panel_list:
        deg, U, overlap, n_tiles, fill = analyze_panel(panel, indptr, indices, M)
        if deg > 0:
            fills.append(fill)
            overlaps.append(overlap)
            U_values.append(U)
            B_total += deg
            B_union += U

    fills = np.array(fills) if fills else np.array([0.0])
    overlaps = np.array(overlaps) if overlaps else np.array([0.0])

    print(f"\n  === 策略: {name} ===")
    print(f"  panels: {len(fills)}")
    print(f"  填充率: 均值={np.mean(fills)*100:.2f}%, "
          f"中位={np.median(fills)*100:.2f}%, "
          f"P75={np.percentile(fills,75)*100:.2f}%, "
          f"P95={np.percentile(fills,95)*100:.2f}%")
    print(f"  overlap: 均值={np.mean(overlaps):.2f}x")
    if B_union > 0:
        print(f"  B 节省: {B_total/B_union:.2f}x")

    bins = [0, 0.0625, 0.10, 0.15, 0.20, 0.30, 0.50, 0.75, 1.01]
    hist, _ = np.histogram(fills, bins=bins)
    total = len(fills)
    for k in range(len(hist)):
        bar_len = int(hist[k] / total * 50) if total > 0 else 0
        bar = '#' * bar_len
        pct = hist[k] / total * 100 if total > 0 else 0
        print(f"  [{bins[k]*100:5.1f}%,{bins[k+1]*100:5.1f}%) : "
              f"{hist[k]:>6} ({pct:5.1f}%) {bar}")

    if U_values:
        print_U_distribution(U_values, name)

    return {'name': name, 'avg_fill': float(np.mean(fills)),
            'median_fill': float(np.median(fills)),
            'p95_fill': float(np.percentile(fills, 95)),
            'avg_U': float(np.mean(U_values)) if U_values else 0,
            'B_savings': B_total / B_union if B_union > 0 else 0}


# ============================================================
# MinHash 实现
# ============================================================

def compute_minhash_signatures(indptr, indices, M, num_hashes=64):
    """
    MinHash 签名: 对每行的列集合，用 num_hashes 个哈希函数
    生成签名向量。签名相似 ≈ Jaccard 相似度高 ≈ 列并集小。

    哈希函数: h_k(x) = (a_k * x + b_k) mod P
    P 取大素数，a_k, b_k 随机选。
    """
    P = 2147483647  # 2^31 - 1, Mersenne 素数
    rng = np.random.RandomState(12345)
    a = rng.randint(1, P, size=num_hashes, dtype=np.int64)
    b = rng.randint(0, P, size=num_hashes, dtype=np.int64)

    # 初始化签名为最大值
    signatures = np.full((M, num_hashes), np.iinfo(np.int64).max, dtype=np.int64)

    print(f"    计算 MinHash 签名 ({num_hashes} 哈希)...", end=' ', flush=True)
    t0 = time.time()

    for i in range(M):
        start = int(indptr[i])
        end = int(indptr[i+1])
        if start == end:
            continue
        cols = indices[start:end].astype(np.int64)
        # 向量化: 对所有列和所有哈希函数同时计算
        # hash_vals[col_idx, hash_idx] = (a * col + b) % P
        # cols: shape (nnz_row,), a: shape (num_hashes,)
        for idx in range(start, end):
            c = int(indices[idx])
            hvals = (a * c + b) % P  # shape (num_hashes,)
            np.minimum(signatures[i], hvals, out=signatures[i])

    print(f"{time.time()-t0:.1f}s")
    return signatures


def lsh_banding(signatures, num_bands=16):
    """
    LSH 分桶: 把签名切成 num_bands 个 band，
    每个 band 内签名完全相同的行放进同一个桶。

    band 越宽 → 要求更高相似度才会碰撞 → 分组更紧凑
    """
    M, num_hashes = signatures.shape
    rows_per_band = num_hashes // num_bands

    print(f"    LSH 分桶 ({num_bands} bands × {rows_per_band} rows)...",
          end=' ', flush=True)
    t0 = time.time()

    # 收集所有桶
    buckets = defaultdict(list)
    for i in range(M):
        if np.all(signatures[i] == np.iinfo(np.int64).max):
            continue  # 跳过空行
        for band_id in range(num_bands):
            start = band_id * rows_per_band
            end = start + rows_per_band
            # 用 band 签名的 tuple 作为桶 key
            band_sig = tuple(signatures[i, start:end].tolist())
            key = (band_id, band_sig)
            buckets[key].append(i)

    # 过滤: 只保留 >= 2 行的桶
    groups = [rows for rows in buckets.values() if len(rows) >= 2]
    print(f"{time.time()-t0:.1f}s, {len(groups)} 个非空桶")

    return groups


def minhash_grouping(indptr, indices, M, target_panels=5000):
    """
    用 MinHash + LSH 找列集合相似的行，组成 panel

    流程:
    1. MinHash 签名 (64 哈希)
    2. LSH 分桶 (16 bands)
    3. 每个桶内取前 16 行作为 panel
    """
    sigs = compute_minhash_signatures(indptr, indices, M, num_hashes=64)
    groups = lsh_banding(sigs, num_bands=16)

    # 按桶大小排序，大桶优先（大桶说明更多行相似）
    groups.sort(key=len, reverse=True)

    panels = []
    used = set()
    for group in groups:
        # 从每个桶中取未使用的行，最多 16 个
        available = [r for r in group if r not in used]
        if len(available) < 2:
            continue
        panel = available[:16]
        for r in panel:
            used.add(r)
        panels.append(panel)
        if len(panels) >= target_panels:
            break

    print(f"    生成 {len(panels)} 个 MinHash panel")
    return panels


def main():
    if len(sys.argv) < 2:
        print("用法: python3 minhash_group_analysis.py <file.csrbin> [sample_size]")
        sys.exit(1)

    path = sys.argv[1]
    sample_size = int(sys.argv[2]) if len(sys.argv) > 2 else 3000

    data_dir = os.path.expanduser("~/data/SpMM_project/data")
    if not os.path.exists(path):
        candidate = os.path.join(data_dir, path, path + '.csrbin')
        if os.path.exists(candidate):
            path = candidate

    name = os.path.basename(os.path.dirname(path))

    print(f"\n{'#'*70}")
    print(f"#  MinHash 分组 + U 分布分析: {name}")
    print(f"{'#'*70}")

    indptr, indices, values, M, N, nnz = read_csrbin(path)
    avg_deg = nnz / M
    print(f"  {M}x{N}, NNZ={nnz:,}, avg_deg={avg_deg:.1f}")

    # 选种子
    degrees = np.array([int(indptr[i+1] - indptr[i]) for i in range(M)])
    candidates = np.where((degrees >= 5) & (degrees <= 300))[0]
    if len(candidates) == 0:
        candidates = np.where(degrees >= 2)[0]
    rng = np.random.RandomState(42)
    sampled = rng.choice(candidates, min(sample_size, len(candidates)), replace=False)
    print(f"  采样: {len(sampled)} 个种子")

    results = []

    # ============================================================
    # 策略 1: 连续 16 行 (baseline)
    # ============================================================
    print(f"\n{'='*65}")
    print(f"  策略 1: 连续 16 行")
    panels_consec = []
    n_panels = (M + TILE_ROWS - 1) // TILE_ROWS
    # 采样一些 panel
    panel_indices = rng.choice(n_panels, min(sample_size, n_panels), replace=False)
    for pi in panel_indices:
        r_start = pi * TILE_ROWS
        r_end = min(r_start + TILE_ROWS, M)
        panels_consec.append(list(range(r_start, r_end)))
    results.append(run_strategy_with_U("连续16行", panels_consec, indptr, indices, M))

    # ============================================================
    # 策略 2: 邻居分组 (baseline)
    # ============================================================
    print(f"\n{'='*65}")
    print(f"  策略 2: 邻居分组")
    panels_nb = []
    for seed in sampled:
        seed = int(seed)
        start = int(indptr[seed])
        end = int(indptr[seed+1])
        nbs = [int(indices[idx]) for idx in range(start, end)][:15]
        panels_nb.append([seed] + nbs)
    results.append(run_strategy_with_U("邻居分组", panels_nb, indptr, indices, M))

    # ============================================================
    # 策略 3: MinHash 分组
    # ============================================================
    print(f"\n{'='*65}")
    print(f"  策略 3: MinHash + LSH 分组")
    t0 = time.time()
    panels_mh = minhash_grouping(indptr, indices, M, target_panels=sample_size)
    mh_time = time.time() - t0
    print(f"  MinHash 总耗时: {mh_time:.1f}s")
    if panels_mh:
        results.append(run_strategy_with_U("MinHash", panels_mh, indptr, indices, M))

    # ============================================================
    # 策略 4: 邻居分组 + U 限制
    # ============================================================
    # 在邻居分组基础上，贪心地只加入不会让 U 超过 64 的邻居
    print(f"\n{'='*65}")
    print(f"  策略 4: 邻居分组 + U≤64 约束 (贪心)")
    print(f"  (目标: 控制 U 在 2 个 tile 内)")
    panels_ulimit = []
    for seed in sampled:
        seed = int(seed)
        seed_cols = get_row_cols_set(seed, indptr, indices, M)
        if not seed_cols:
            continue
        group = [seed]
        group_cols = set(seed_cols)
        # 取 seed 的邻居
        start = int(indptr[seed])
        end = int(indptr[seed+1])
        nbs = [int(indices[idx]) for idx in range(start, end) if indices[idx] < M]
        # 按列集合大小排序（小集合优先，因为不容易增大 U）
        nbs_sorted = sorted(nbs, key=lambda n: int(indptr[n+1]) - int(indptr[n]))
        for nb in nbs_sorted:
            if len(group) >= 16:
                break
            nb_cols = get_row_cols_set(nb, indptr, indices, M)
            new_U = len(group_cols | nb_cols)
            if new_U <= 64:  # 最多 2 个 tile
                group.append(nb)
                group_cols |= nb_cols
        panels_ulimit.append(group)
    results.append(run_strategy_with_U("邻居+U≤64", panels_ulimit, indptr, indices, M))

    # ============================================================
    # 策略 5: 邻居分组 + U≤32 约束 (最激进)
    # ============================================================
    print(f"\n{'='*65}")
    print(f"  策略 5: 邻居分组 + U≤32 约束 (最激进)")
    print(f"  (目标: 1 个 tile，fill 理论上限 ~31%)")
    panels_u32 = []
    u32_group_sizes = []
    for seed in sampled:
        seed = int(seed)
        seed_cols = get_row_cols_set(seed, indptr, indices, M)
        if not seed_cols:
            continue
        group = [seed]
        group_cols = set(seed_cols)
        start = int(indptr[seed])
        end = int(indptr[seed+1])
        nbs = [int(indices[idx]) for idx in range(start, end) if indices[idx] < M]
        # 按与当前组的列交集大小降序（重叠最多的优先，增加 U 最少）
        nbs_with_overlap = []
        for nb in nbs:
            nb_cols = get_row_cols_set(nb, indptr, indices, M)
            ov = len(nb_cols & group_cols)
            new_cols = len(nb_cols - group_cols)
            nbs_with_overlap.append((new_cols, -ov, nb, nb_cols))
        nbs_with_overlap.sort()  # 按新增列数升序

        for new_cols, neg_ov, nb, nb_cols in nbs_with_overlap:
            if len(group) >= 16:
                break
            new_U = len(group_cols | nb_cols)
            if new_U <= 32:
                group.append(nb)
                group_cols |= nb_cols
        panels_u32.append(group)
        u32_group_sizes.append(len(group))

    u32_sizes = np.array(u32_group_sizes)
    print(f"  组大小: 均值={np.mean(u32_sizes):.1f}, "
          f"中位={np.median(u32_sizes):.0f}, "
          f">=16的比例={np.mean(u32_sizes>=16)*100:.1f}%")
    results.append(run_strategy_with_U("邻居+U≤32", panels_u32, indptr, indices, M))

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
    if best['avg_fill'] > 0.30:
        print(f"  >>> 突破 30%!!!")


if __name__ == "__main__":
    main()
