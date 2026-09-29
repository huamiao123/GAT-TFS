#!/usr/bin/env python3
"""
高级行分组策略实验

策略 A (baseline): seed + 前 15 个邻居
策略 B (共同邻居): seed + 与 seed 共同邻居最多的 15 个邻居
策略 C (三角形贪心): 从 seed 出发，每次加入与当前组共享列最多的邻居
策略 D (2-hop 聚类): seed 的邻居的邻居中，出现频率最高的节点

在 web-Google 上测试（当前最佳 20%，目标 30%+）
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


def get_row_cols(i, indptr, indices, M):
    """取第 i 行的列索引集合"""
    start = int(indptr[i])
    end = int(indptr[i+1])
    return set(int(indices[idx]) for idx in range(start, end) if indices[idx] < M)


def analyze_panel(rows, indptr, indices, M):
    """给定一组行号，计算填充率指标"""
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


def strategy_A(seed, indptr, indices, M):
    """Baseline: seed + 前 15 个邻居"""
    nbs = get_row_cols(seed, indptr, indices, M)
    nbs_list = sorted(nbs)[:15]
    return [seed] + nbs_list


def strategy_B(seed, indptr, indices, M):
    """共同邻居: seed + 与 seed 共同邻居数最多的 15 个邻居
    
    直觉: 共同邻居多 = 列集合重叠多 = tile 填充率高
    """
    seed_cols = get_row_cols(seed, indptr, indices, M)
    if len(seed_cols) == 0:
        return [seed]
    
    # 对每个邻居，计算它与 seed 的共同邻居数
    scored = []
    for nb in seed_cols:
        nb_cols = get_row_cols(nb, indptr, indices, M)
        common = len(seed_cols & nb_cols)
        scored.append((common, nb))
    
    # 按共同邻居数降序排列，取前 15 个
    scored.sort(reverse=True)
    return [seed] + [nb for _, nb in scored[:15]]


def strategy_C(seed, indptr, indices, M):
    """三角形贪心: 从 seed 出发，每次加入与当前组列重叠最大的邻居
    
    直觉: 贪心地最大化每一步的列重叠增量
    """
    seed_cols = get_row_cols(seed, indptr, indices, M)
    if len(seed_cols) == 0:
        return [seed]
    
    group = [seed]
    group_cols = set(seed_cols)  # 当前组的列并集
    candidates = set(seed_cols) - {seed}  # 候选: seed 的邻居
    
    while len(group) < 16 and candidates:
        best_nb = -1
        best_overlap = -1
        
        for nb in candidates:
            nb_cols = get_row_cols(nb, indptr, indices, M)
            # 重叠 = 这个邻居的列与当前组列并集的交集大小
            ov = len(nb_cols & group_cols)
            if ov > best_overlap:
                best_overlap = ov
                best_nb = nb
        
        if best_nb < 0:
            break
            
        group.append(best_nb)
        nb_cols = get_row_cols(best_nb, indptr, indices, M)
        group_cols |= nb_cols
        candidates.discard(best_nb)
    
    return group


def strategy_D(seed, indptr, indices, M):
    """2-hop 聚类: 找 seed 邻居的邻居中出现频率最高的节点
    
    直觉: 如果某个节点 x 被 seed 的多个邻居共同指向，
    那么 x 在很多行的列集合中都出现 = 天然的列重叠
    
    做法:
      1. 收集 seed 所有邻居的列集合
      2. 统计每个列索引出现在多少个邻居的列集合中
      3. 取出现频率最高的 15 个节点作为 panel
    """
    seed_cols = get_row_cols(seed, indptr, indices, M)
    if len(seed_cols) == 0:
        return [seed]
    
    # 统计 seed 每个邻居的列集合中，各列出现的频率
    col_freq = {}
    for nb in seed_cols:
        nb_cols = get_row_cols(nb, indptr, indices, M)
        for c in nb_cols:
            if c != seed and c not in seed_cols:
                col_freq[c] = col_freq.get(c, 0) + 1
    
    # 也把 seed 的直接邻居加入候选（它们至少共享 seed 这一列）
    for nb in seed_cols:
        col_freq[nb] = col_freq.get(nb, 0) + len(seed_cols)  # 高优先
    
    # 按频率降序，取前 15 个作为行
    ranked = sorted(col_freq.items(), key=lambda x: -x[1])
    panel = [seed] + [int(node) for node, _ in ranked[:15]]
    
    return panel


def run_strategy(name, strategy_fn, sampled, indptr, indices, M):
    """运行一个策略并统计结果"""
    fills = []
    overlaps = []
    B_total = 0
    B_union = 0
    
    t0 = time.time()
    for seed in sampled:
        panel = strategy_fn(int(seed), indptr, indices, M)
        deg, U, overlap, n_tiles, fill = analyze_panel(panel, indptr, indices, M)
        if deg > 0:
            fills.append(fill)
            overlaps.append(overlap)
            B_total += deg
            B_union += U
    elapsed = time.time() - t0
    
    fills = np.array(fills) if fills else np.array([0.0])
    overlaps = np.array(overlaps) if overlaps else np.array([0.0])
    
    print(f"\n  --- 策略 {name} ---")
    print(f"  填充率: 均值={np.mean(fills)*100:.2f}%, "
          f"中位={np.median(fills)*100:.2f}%, "
          f"P75={np.percentile(fills, 75)*100:.2f}%, "
          f"P95={np.percentile(fills, 95)*100:.2f}%")
    print(f"  overlap: 均值={np.mean(overlaps):.2f}x, "
          f"中位={np.median(overlaps):.2f}x")
    if B_union > 0:
        print(f"  B 节省: {B_total/B_union:.2f}x")
    print(f"  耗时: {elapsed:.2f}s")
    
    # 分布
    bins = [0, 0.0625, 0.10, 0.15, 0.20, 0.30, 0.50, 0.75, 1.01]
    hist, _ = np.histogram(fills, bins=bins)
    total = len(fills)
    for k in range(len(hist)):
        bar_len = int(hist[k] / total * 50) if total > 0 else 0
        bar = '#' * bar_len
        pct = hist[k] / total * 100 if total > 0 else 0
        print(f"  [{bins[k]*100:5.1f}%,{bins[k+1]*100:5.1f}%) : "
              f"{hist[k]:>6} ({pct:5.1f}%) {bar}")
    
    return {
        'name': name,
        'avg_fill': float(np.mean(fills)),
        'median_fill': float(np.median(fills)),
        'p75_fill': float(np.percentile(fills, 75)),
        'avg_overlap': float(np.mean(overlaps)),
        'B_savings': B_total / B_union if B_union > 0 else 0,
        'time': elapsed
    }


def main():
    if len(sys.argv) < 2:
        print("用法: python3 advanced_group_analysis.py <file.csrbin> [sample_size]")
        sys.exit(1)
    
    path = sys.argv[1]
    sample_size = int(sys.argv[2]) if len(sys.argv) > 2 else 3000
    
    data_dir = os.path.expanduser("~/data/SpMM_project/data")
    if not os.path.exists(path):
        name = path
        candidate = os.path.join(data_dir, name, name + '.csrbin')
        if os.path.exists(candidate):
            path = candidate
    
    name = os.path.basename(os.path.dirname(path))
    
    print(f"\n{'#'*70}")
    print(f"#  高级分组策略实验: {name}")
    print(f"#  采样: {sample_size} 个种子节点")
    print(f"{'#'*70}")
    
    indptr, indices, values, M, N, nnz = read_csrbin(path)
    avg_deg = nnz / M
    print(f"  {M}x{N}, NNZ={nnz:,}, avg_deg={avg_deg:.1f}")
    
    # 选种子: 度数在 [10, 200] 的节点（太低没邻居，太高计算太慢）
    degrees = np.array([int(indptr[i+1] - indptr[i]) for i in range(M)])
    candidates = np.where((degrees >= 10) & (degrees <= 200))[0]
    print(f"  候选节点 (10<=deg<=200): {len(candidates):,}")
    
    if len(candidates) == 0:
        # 放宽条件
        candidates = np.where(degrees >= 3)[0]
        print(f"  放宽条件 (deg>=3): {len(candidates):,}")
    
    rng = np.random.RandomState(42)
    sampled = rng.choice(candidates, min(sample_size, len(candidates)), replace=False)
    print(f"  实际采样: {len(sampled)}")
    
    # 跑四个策略
    results = []
    
    print(f"\n{'='*65}")
    print(f"  策略 A: seed + 前15个邻居 (baseline)")
    results.append(run_strategy("A_baseline", strategy_A, sampled, indptr, indices, M))
    
    print(f"\n{'='*65}")
    print(f"  策略 B: seed + 共同邻居最多的15个")
    print(f"  (共同邻居多 = 列集合重叠大)")
    results.append(run_strategy("B_common_nb", strategy_B, sampled, indptr, indices, M))
    
    print(f"\n{'='*65}")
    print(f"  策略 C: 三角形贪心 (每步加入重叠最大的邻居)")
    print(f"  (贪心最大化列重叠)")
    results.append(run_strategy("C_greedy", strategy_C, sampled, indptr, indices, M))
    
    print(f"\n{'='*65}")
    print(f"  策略 D: 2-hop 高频节点")
    print(f"  (被多个邻居共同指向的节点)")
    results.append(run_strategy("D_2hop", strategy_D, sampled, indptr, indices, M))
    
    # 汇总对比
    print(f"\n\n{'='*75}")
    print(f"  汇总对比: {name}")
    print(f"{'='*75}")
    fmt = "  {:<20} {:>10} {:>10} {:>10} {:>10} {:>8}"
    print(fmt.format("策略", "均值fill", "中位fill", "P75_fill", "B节省", "耗时"))
    print(f"  {'_'*70}")
    for r in results:
        print(fmt.format(
            r['name'],
            f"{r['avg_fill']*100:.2f}%",
            f"{r['median_fill']*100:.2f}%",
            f"{r['p75_fill']*100:.2f}%",
            f"{r['B_savings']:.2f}x",
            f"{r['time']:.1f}s"))
    
    best = max(results, key=lambda x: x['avg_fill'])
    print(f"\n  最佳策略: {best['name']}, 填充率 {best['avg_fill']*100:.2f}%")
    
    if best['avg_fill'] > 0.30:
        print(f"  >>> 突破 30%! AMX 有效吞吐 = {best['avg_fill']*16:.1f}x AVX-512!")
    elif best['avg_fill'] > 0.20:
        print(f"  >>> 超过 20%, AMX 有效吞吐 = {best['avg_fill']*16:.1f}x AVX-512")
    elif best['avg_fill'] > 0.15:
        print(f"  >> 超过 15%, 有实用价值")


if __name__ == "__main__":
    main()
