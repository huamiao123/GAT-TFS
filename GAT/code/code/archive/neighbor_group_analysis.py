#!/usr/bin/env python3
"""
邻居分组实验：验证"智能行分组"的潜力上界

核心思路：
  连续16行分组 → 列几乎不重叠 → fill ≈ 6~9%
  邻居分组      → 邻居天然共享列 → fill = ???

方法：
  对每个度数 >= 15 的节点 i：
    取 i 本身 + 15 个邻居 组成一个 16 行 panel
    这 16 个节点互相认识（至少都认识 i），列重叠天然高
  测量这些 panel 的填充率分布

用法: python3 neighbor_group_analysis.py <file.csrbin>
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
    assert indptr[0] == 0 and indptr[nrow] == nnz
    return indptr, indices, values, int(nrow), int(ncol), int(nnz)


def get_neighbors_symmetric(indptr, indices, M):
    """构建对称邻接表（只存邻居列表，不用 set，节省内存）"""
    # 先用 CSR 本身，再补反向边
    adj = [set() for _ in range(M)]
    for i in range(M):
        for idx in range(indptr[i], indptr[i+1]):
            j = int(indices[idx])
            if j < M and i != j:
                adj[i].add(j)
                adj[j].add(i)
    return [sorted(list(s)) for s in adj]


def analyze_panel(rows, indptr, indices, M):
    """
    给定一组行号，计算动态 row-panel 指标

    返回: (total_deg, U, overlap, n_tiles, fill_rate)
    """
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


def analyze_matrix(csrbin_path):
    name = os.path.basename(os.path.dirname(csrbin_path))
    if not name or name == '.':
        name = os.path.basename(csrbin_path).replace('.csrbin', '')

    print(f"\n{'#'*70}")
    print(f"#  矩阵: {name}")
    print(f"{'#'*70}")

    t0 = time.time()
    indptr, indices, values, M, N, nnz = read_csrbin(csrbin_path)
    avg_deg = nnz / M
    print(f"  读取完成: {M}x{N}, NNZ={nnz:,}, avg_deg={avg_deg:.1f}")

    # ============================================================
    # 方法 1：连续 16 行（baseline）
    # ============================================================
    print(f"\n  --- 方法 1: 连续 16 行 (baseline) ---")
    t0 = time.time()
    consec_fills = []
    consec_overlaps = []
    consec_B_total = 0
    consec_B_union = 0

    n_panels = (M + TILE_ROWS - 1) // TILE_ROWS
    for pi in range(n_panels):
        r_start = pi * TILE_ROWS
        r_end = min(r_start + TILE_ROWS, M)
        rows = list(range(r_start, r_end))
        deg, U, overlap, n_tiles, fill = analyze_panel(rows, indptr, indices, M)
        if deg > 0:
            consec_fills.append(fill)
            consec_overlaps.append(overlap)
            consec_B_total += deg
            consec_B_union += U

    consec_fills = np.array(consec_fills)
    consec_overlaps = np.array(consec_overlaps)
    print(f"  panels: {len(consec_fills):,}")
    print(f"  填充率: 均值={np.mean(consec_fills)*100:.2f}%, "
          f"中位={np.median(consec_fills)*100:.2f}%")
    print(f"  overlap: 均值={np.mean(consec_overlaps):.2f}x, "
          f"中位={np.median(consec_overlaps):.2f}x")
    print(f"  B 节省: {consec_B_total/consec_B_union:.2f}x")
    print(f"  耗时: {time.time()-t0:.2f}s")

    # ============================================================
    # 方法 2：邻居分组
    # ============================================================
    # 对每个度数 >= 15 的节点 i:
    #   panel = {i} + 前 15 个邻居
    #   这 16 个节点至少都共享列 i（因为都与 i 相连）
    # 采样：如果高度节点太多，随机采样最多 10000 个

    print(f"\n  --- 方法 2: 邻居分组 (seed + 15 neighbors) ---")
    t0 = time.time()

    # 找度数 >= 15 的节点
    degrees = np.array([int(indptr[i+1] - indptr[i]) for i in range(M)])
    candidates = np.where(degrees >= 1)[0]
    print(f"  度数 >= 15 的节点: {len(candidates):,} / {M:,} ({len(candidates)/M*100:.1f}%)")

    # 采样
    max_samples = min(10000, len(candidates))
    if len(candidates) > max_samples:
        rng = np.random.RandomState(42)
        sampled = rng.choice(candidates, max_samples, replace=False)
    else:
        sampled = candidates

    # 需要对称邻接表来取邻居
    # 但构建完整邻接表对大图太慢，改用 CSR 直接取出边邻居
    # 注意：CSR 的 indices 是出边邻居，不一定对称
    # 对于我们的实验，用出边邻居就够了

    nb_fills = []
    nb_overlaps = []
    nb_B_total = 0
    nb_B_union = 0

    for i in sampled:
        start = int(indptr[i])
        end = int(indptr[i+1])
        neighbors = indices[start:end].tolist()
        # 取前 15 个邻居（如果 > 15 个，取前 15 个）
        panel_rows = [int(i)] + neighbors[:15]
        # 确保恰好 16 行（不够补到最近的行，但一般 deg>=15 够了）

        deg, U, overlap, n_tiles, fill = analyze_panel(
            panel_rows, indptr, indices, M)
        if deg > 0:
            nb_fills.append(fill)
            nb_overlaps.append(overlap)
            nb_B_total += deg
            nb_B_union += U

    nb_fills = np.array(nb_fills)
    nb_overlaps = np.array(nb_overlaps)
    print(f"  采样 panels: {len(nb_fills):,}")
    print(f"  填充率: 均值={np.mean(nb_fills)*100:.2f}%, "
          f"中位={np.median(nb_fills)*100:.2f}%")
    print(f"  overlap: 均值={np.mean(nb_overlaps):.2f}x, "
          f"中位={np.median(nb_overlaps):.2f}x")
    print(f"  B 节省: {nb_B_total/nb_B_union:.2f}x") if nb_B_union > 0 else print("  B 节省: N/A")
    print(f"  耗时: {time.time()-t0:.2f}s")

    # 分布
    bins = [0, 0.0625, 0.10, 0.15, 0.20, 0.30, 0.50, 0.75, 1.01]
    print(f"\n  --- 邻居分组填充率分布 ---")
    hist, _ = np.histogram(nb_fills, bins=bins)
    total = len(nb_fills)
    for k in range(len(hist)):
        bar_len = int(hist[k] / total * 50) if total > 0 else 0
        bar = '#' * bar_len
        pct = hist[k] / total * 100 if total > 0 else 0
        lo = f"{bins[k]*100:.1f}%"
        hi = f"{bins[k+1]*100:.1f}%"
        print(f"  [{lo:>6},{hi:>6}) : {hist[k]:>7} ({pct:5.1f}%) {bar}")

    # ============================================================
    # 方法 3：贪心分组（最激进的验证）
    # ============================================================
    # 对每个种子节点 i，取与 i 共享邻居最多的 15 个节点
    # 太贵了对大图，只在小图上做
    # 改用更简单的方法：取 i 的邻居中度数最相似的 15 个
    # （度数相似 ≈ 列集合大小相似 ≈ 重叠可能更高）

    print(f"\n  --- 方法 3: 邻居排序分组 (seed + 15个度数最接近的邻居) ---")
    t0 = time.time()

    sorted_fills = []
    sorted_overlaps = []
    sorted_B_total = 0
    sorted_B_union = 0

    for i in sampled:
        start = int(indptr[i])
        end = int(indptr[i+1])
        neighbors = indices[start:end].tolist()

        # 按度数差排序，取度数最接近 i 的 15 个邻居
        deg_i = degrees[i]
        nb_with_deg = [(abs(int(degrees[n]) - deg_i), n) for n in neighbors if n < M]
        nb_with_deg.sort()
        panel_rows = [int(i)] + [int(n) for _, n in nb_with_deg[:15]]

        deg, U, overlap, n_tiles, fill = analyze_panel(
            panel_rows, indptr, indices, M)
        if deg > 0:
            sorted_fills.append(fill)
            sorted_overlaps.append(overlap)
            sorted_B_total += deg
            sorted_B_union += U

    sorted_fills = np.array(sorted_fills)
    sorted_overlaps = np.array(sorted_overlaps)
    print(f"  采样 panels: {len(sorted_fills):,}")
    print(f"  填充率: 均值={np.mean(sorted_fills)*100:.2f}%, "
          f"中位={np.median(sorted_fills)*100:.2f}%")
    print(f"  overlap: 均值={np.mean(sorted_overlaps):.2f}x, "
          f"中位={np.median(sorted_overlaps):.2f}x")
    print(f"  B 节省: {sorted_B_total/sorted_B_union:.2f}x") if sorted_B_union > 0 else print("  B 节省: N/A")
    print(f"  耗时: {time.time()-t0:.2f}s")

    # 分布
    print(f"\n  --- 度数排序分组填充率分布 ---")
    hist, _ = np.histogram(sorted_fills, bins=bins)
    total = len(sorted_fills)
    for k in range(len(hist)):
        bar_len = int(hist[k] / total * 50) if total > 0 else 0
        bar = '#' * bar_len
        pct = hist[k] / total * 100 if total > 0 else 0
        lo = f"{bins[k]*100:.1f}%"
        hi = f"{bins[k+1]*100:.1f}%"
        print(f"  [{lo:>6},{hi:>6}) : {hist[k]:>7} ({pct:5.1f}%) {bar}")

    # ============================================================
    # 总对比
    # ============================================================
    print(f"\n  {'='*65}")
    print(f"  *** 三种分组策略对比: {name}")
    print(f"  {'='*65}")
    fmt = "  {:<30} {:>10} {:>10} {:>10}"
    print(fmt.format("", "连续16行", "邻居分组", "度数排序"))
    print(f"  {'_'*65}")
    print(fmt.format("填充率(均值)",
        f"{np.mean(consec_fills)*100:.2f}%",
        f"{np.mean(nb_fills)*100:.2f}%",
        f"{np.mean(sorted_fills)*100:.2f}%"))
    print(fmt.format("填充率(中位)",
        f"{np.median(consec_fills)*100:.2f}%",
        f"{np.median(nb_fills)*100:.2f}%",
        f"{np.median(sorted_fills)*100:.2f}%"))
    print(fmt.format("overlap(均值)",
        f"{np.mean(consec_overlaps):.2f}x",
        f"{np.mean(nb_overlaps):.2f}x",
        f"{np.mean(sorted_overlaps):.2f}x"))
    print(fmt.format("B读取节省",
        f"{consec_B_total/consec_B_union:.2f}x",
        f"{nb_B_total/nb_B_union:.2f}x" if nb_B_union > 0 else "N/A",
        f"{sorted_B_total/sorted_B_union:.2f}x" if sorted_B_union > 0 else "N/A"))

    # 提升倍数
    base_fill = np.mean(consec_fills)
    nb_fill = np.mean(nb_fills)
    sort_fill = np.mean(sorted_fills)
    print(f"\n  相比连续16行的提升:")
    print(f"    邻居分组:   {nb_fill/base_fill:.2f}x 填充率提升")
    print(f"    度数排序:   {sort_fill/base_fill:.2f}x 填充率提升")

    if nb_fill > 0.15:
        print(f"\n  *** 邻居分组填充率 {nb_fill*100:.1f}% > 15%: 智能分组大有可为!")
    elif nb_fill > 0.10:
        print(f"\n  ** 邻居分组填充率 {nb_fill*100:.1f}% > 10%: 值得继续深挖")


def main():
    if len(sys.argv) < 2:
        print("用法: python3 neighbor_group_analysis.py <file.csrbin>")
        print("      python3 neighbor_group_analysis.py all")
        sys.exit(1)

    data_dir = os.path.expanduser("~/data/SpMM_project/data")

    if sys.argv[1] == "all":
        entries = sorted(os.listdir(data_dir))
        csrbin_files = []
        for d in entries:
            subdir = os.path.join(data_dir, d)
            if os.path.isdir(subdir):
                for f in os.listdir(subdir):
                    if f.endswith('.csrbin'):
                        csrbin_files.append(os.path.join(subdir, f))
        print(f"数据目录: {data_dir}")
        print(f"找到 {len(csrbin_files)} 个 .csrbin 文件\n")

        for path in csrbin_files:
            try:
                analyze_matrix(path)
            except Exception as e:
                print(f"  [错误] {path}: {e}")
                import traceback
                traceback.print_exc()
    else:
        path = sys.argv[1]
        if not os.path.exists(path):
            candidate = os.path.join(data_dir, path, path + '.csrbin')
            if os.path.exists(candidate):
                path = candidate
        analyze_matrix(path)

if __name__ == "__main__":
    main()
