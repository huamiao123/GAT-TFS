#!/usr/bin/env python3
"""
动态 Row-Panel 分析：测量 AMX SpMM 的真实可行性

与固定 BSR 的区别：
  BSR: 按固定 16x32 网格切分 → 大量空块
  动态: 取 16 行的列并集，紧密打包 → 填充率有下界保证

核心指标：
  U = 列并集大小（决定 B 读取次数和 tile 数量）
  overlap = 16*avg_deg / U（列复用率，>1 越好）
  fill_rate = total_nnz / (16 * 32 * ceil(U/32))（AMX 计算效率）
"""

import sys, os, struct
import numpy as np
from collections import deque
import time

TILE_ROWS = 16
TILE_COLS = 32

# ============================================================
# 读取 .csrbin（与上一版相同）
# ============================================================
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


# ============================================================
# RCM 重排序（与上一版相同）
# ============================================================
def build_adjacency(indptr, indices, M):
    adj = [[] for _ in range(M)]
    seen = [set() for _ in range(M)]
    for i in range(M):
        for idx in range(indptr[i], indptr[i+1]):
            j = int(indices[idx])
            if j < M and i != j:
                if j not in seen[i]:
                    seen[i].add(j)
                    adj[i].append(j)
                if i not in seen[j]:
                    seen[j].add(i)
                    adj[j].append(i)
    del seen
    return adj

def find_pseudo_peripheral(adj, M, degree):
    start = 0
    for i in range(M):
        if degree[i] > 0:
            start = i
            break
    for _ in range(2):
        visited = np.zeros(M, dtype=np.bool_)
        queue = deque([start])
        visited[start] = True
        last = start
        while queue:
            node = queue.popleft()
            last = node
            for nb in adj[node]:
                if not visited[nb]:
                    visited[nb] = True
                    queue.append(nb)
        start = last
    return start

def rcm_reorder(indptr, indices, M):
    print(f"    构建邻接表...", end=' ', flush=True)
    t0 = time.time()
    adj = build_adjacency(indptr, indices, M)
    degree = np.array([len(adj[i]) for i in range(M)], dtype=np.int32)
    print(f"{time.time()-t0:.1f}s")

    print(f"    BFS 排序...", end=' ', flush=True)
    t0 = time.time()
    start = find_pseudo_peripheral(adj, M, degree)
    perm = []
    visited = np.zeros(M, dtype=np.bool_)
    for seed in [start] + list(range(M)):
        if visited[seed]:
            continue
        queue = deque([seed])
        visited[seed] = True
        while queue:
            node = queue.popleft()
            perm.append(node)
            neighbors = [(degree[nb], nb) for nb in adj[node] if not visited[nb]]
            neighbors.sort()
            for _, nb in neighbors:
                if not visited[nb]:
                    visited[nb] = True
                    queue.append(nb)
    perm = np.array(perm[::-1], dtype=np.int32)
    print(f"{time.time()-t0:.1f}s")
    return perm

def apply_permutation(indptr, indices, values, M, perm):
    inv_perm = np.empty(M, dtype=np.int32)
    inv_perm[perm] = np.arange(M, dtype=np.int32)
    new_row_nnz = np.zeros(M, dtype=np.int64)
    for old_i in range(M):
        new_i = inv_perm[old_i]
        new_row_nnz[new_i] = indptr[old_i+1] - indptr[old_i]
    new_indptr = np.zeros(M + 1, dtype=np.int64)
    np.cumsum(new_row_nnz, out=new_indptr[1:])
    nnz = int(new_indptr[M])
    new_indices = np.empty(nnz, dtype=np.int32)
    new_values = np.empty(nnz, dtype=np.float32)
    write_pos = new_indptr[:-1].copy()
    for old_i in range(M):
        new_i = inv_perm[old_i]
        start = int(indptr[old_i])
        end = int(indptr[old_i+1])
        if start == end:
            continue
        old_cols = indices[start:end]
        new_cols = inv_perm[old_cols[old_cols < M]]
        old_vals = values[start:start+len(new_cols)]
        wp = int(write_pos[new_i])
        n = len(new_cols)
        new_indices[wp:wp+n] = new_cols
        new_values[wp:wp+n] = old_vals
        write_pos[new_i] += n
    for i in range(M):
        s = int(new_indptr[i])
        e = int(new_indptr[i+1])
        if e - s > 1:
            order = np.argsort(new_indices[s:e])
            new_indices[s:e] = new_indices[s:e][order]
            new_values[s:e] = new_values[s:e][order]
    return new_indptr, new_indices, new_values


# ============================================================
# 核心：动态 Row-Panel 指标分析
# ============================================================
def analyze_dynamic_panels(indptr, indices, M, N, nnz, label=""):
    """
    对每个 16 行 panel 计算：
    - total_deg: 16行的总非零数（= sum of row degrees）
    - U: 列并集大小（= 这16行涉及的不同列数）
    - overlap: total_deg / U（列复用率）
    - n_tiles: ceil(U / 32)（需要的 AMX tile 数）
    - fill_rate: total_deg / (16 * 32 * n_tiles)（AMX 计算效率）

    同时统计：
    - B_reads_avx: AVX-512 方式下读 B 的总次数 = total_nnz
    - B_reads_amx: AMX 方式下读 B 的总次数 = sum(U per panel)
    """
    n_panels = (M + TILE_ROWS - 1) // TILE_ROWS

    panel_U = []          # 每个 panel 的列并集大小
    panel_deg = []        # 每个 panel 的总非零数
    panel_overlap = []    # 每个 panel 的重叠率
    panel_fill = []       # 每个 panel 的动态填充率
    panel_ntiles = []     # 每个 panel 需要的 tile 数

    total_B_reads_avx = 0
    total_B_reads_amx = 0
    total_amx_useful = 0
    total_amx_capacity = 0

    for pi in range(n_panels):
        r_start = pi * TILE_ROWS
        r_end = min(r_start + TILE_ROWS, M)

        # 收集这 16 行的所有列索引
        col_set = set()
        deg = 0
        for r in range(r_start, r_end):
            start = int(indptr[r])
            end = int(indptr[r+1])
            row_deg = end - start
            deg += row_deg
            for idx in range(start, end):
                col_set.add(int(indices[idx]))

        U = len(col_set)

        if deg == 0:
            continue

        # 计算指标
        overlap = deg / U if U > 0 else 0
        n_tiles = (U + TILE_COLS - 1) // TILE_COLS  # ceil(U/32)
        capacity = TILE_ROWS * TILE_COLS * n_tiles
        fill_rate = deg / capacity if capacity > 0 else 0

        panel_U.append(U)
        panel_deg.append(deg)
        panel_overlap.append(overlap)
        panel_fill.append(fill_rate)
        panel_ntiles.append(n_tiles)

        total_B_reads_avx += deg      # AVX-512: 每个非零读一次 B
        total_B_reads_amx += U        # AMX: 每个唯一列只读一次

        total_amx_useful += deg
        total_amx_capacity += capacity

    # 转 numpy 做统计
    panel_U = np.array(panel_U)
    panel_deg = np.array(panel_deg)
    panel_overlap = np.array(panel_overlap)
    panel_fill = np.array(panel_fill)
    panel_ntiles = np.array(panel_ntiles)

    global_fill = total_amx_useful / total_amx_capacity if total_amx_capacity > 0 else 0
    B_savings = total_B_reads_avx / total_B_reads_amx if total_B_reads_amx > 0 else 0

    print(f"\n{'='*65}")
    print(f"  {label} -- 动态 Row-Panel 分析")
    print(f"{'='*65}")
    print(f"  矩阵: {M} x {N}, NNZ = {nnz:,}, avg_deg = {nnz/M:.1f}")
    print(f"  有效 panel 数: {len(panel_U):,} (每 {TILE_ROWS} 行一组)")
    print(f"")
    print(f"  --- 列并集 U ---")
    print(f"  U 均值:   {np.mean(panel_U):.1f}")
    print(f"  U 中位:   {np.median(panel_U):.1f}")
    print(f"  U P25:    {np.percentile(panel_U, 25):.1f}")
    print(f"  U P75:    {np.percentile(panel_U, 75):.1f}")
    print(f"  U P95:    {np.percentile(panel_U, 95):.1f}")
    print(f"  U max:    {np.max(panel_U)}")
    print(f"")
    print(f"  --- 列重叠率 (overlap = total_deg / U, >1 = 有复用) ---")
    print(f"  overlap 均值:  {np.mean(panel_overlap):.2f}x")
    print(f"  overlap 中位:  {np.median(panel_overlap):.2f}x")
    print(f"")
    print(f"  --- AMX 动态填充率 ---")
    print(f"  * 全局效率:      {global_fill*100:.2f}%")
    print(f"  * 填充率均值:    {np.mean(panel_fill)*100:.2f}%")
    print(f"  * 填充率中位:    {np.median(panel_fill)*100:.2f}%")
    print(f"  * 填充率 P25:    {np.percentile(panel_fill, 25)*100:.2f}%")
    print(f"  * 填充率 P75:    {np.percentile(panel_fill, 75)*100:.2f}%")
    print(f"")

    # 填充率分布
    bins = [0, 0.0625, 0.10, 0.15, 0.20, 0.30, 0.50, 0.75, 1.01]
    hist, _ = np.histogram(panel_fill, bins=bins)
    total = len(panel_fill)
    print(f"  --- 填充率分布 ---")
    for i in range(len(hist)):
        bar_len = int(hist[i] / total * 50) if total > 0 else 0
        bar = '#' * bar_len
        pct = hist[i] / total * 100 if total > 0 else 0
        lo = f"{bins[i]*100:.1f}%"
        hi = f"{bins[i+1]*100:.1f}%"
        marker = " <-- 盈亏线" if i == 0 else ""
        print(f"  [{lo:>6},{hi:>6}) : {hist[i]:>7} ({pct:5.1f}%) {bar}{marker}")

    print(f"")
    print(f"  --- B 矩阵读取对比 (K=32) ---")
    print(f"  AVX-512 读 B 次数:  {total_B_reads_avx:>12,} (每个非零读一次)")
    print(f"  AMX 读 B 次数:      {total_B_reads_amx:>12,} (每个唯一列读一次)")
    print(f"  * B 读取节省:       {B_savings:.2f}x")
    print(f"")
    print(f"  --- tiles/panel 分布 ---")
    print(f"  tiles/panel 均值:  {np.mean(panel_ntiles):.1f}")
    print(f"  tiles/panel 中位:  {np.median(panel_ntiles):.1f}")
    print(f"  tiles/panel P95:   {np.percentile(panel_ntiles, 95):.1f}")

    return {
        'global_fill': global_fill,
        'avg_fill': float(np.mean(panel_fill)),
        'median_fill': float(np.median(panel_fill)),
        'avg_overlap': float(np.mean(panel_overlap)),
        'B_savings': B_savings,
        'avg_U': float(np.mean(panel_U)),
    }


# ============================================================
# 主流程
# ============================================================
def analyze_matrix(csrbin_path):
    name = os.path.basename(os.path.dirname(csrbin_path))
    if not name or name == '.':
        name = os.path.basename(csrbin_path).replace('.csrbin', '')

    print(f"\n{'#'*70}")
    print(f"#  矩阵: {name}")
    print(f"{'#'*70}")

    t0 = time.time()
    indptr, indices, values, M, N, nnz = read_csrbin(csrbin_path)
    print(f"  读取完成: {M}x{N}, NNZ={nnz:,}, avg_deg={nnz/M:.1f}, 耗时 {time.time()-t0:.2f}s")

    # 原始
    t0 = time.time()
    stats_orig = analyze_dynamic_panels(indptr, indices, M, N, nnz, label=f"{name} 原始")
    print(f"  原始分析耗时 {time.time()-t0:.2f}s")

    # RCM 重排
    if M != N:
        print(f"  [跳过 RCM] 非方阵")
        return stats_orig, None, name

    if M > 5_000_000:
        print(f"  [警告] 大矩阵 ({M:,}), RCM 可能需要几分钟...")

    t0 = time.time()
    perm = rcm_reorder(indptr, indices, M)
    t_rcm = time.time() - t0
    print(f"  RCM 总耗时: {t_rcm:.2f}s")

    t0 = time.time()
    new_indptr, new_indices, new_values = apply_permutation(
        indptr, indices, values, M, perm)
    print(f"  重排完成, 耗时 {time.time()-t0:.2f}s")

    t0 = time.time()
    stats_rcm = analyze_dynamic_panels(new_indptr, new_indices, M, N, nnz, label=f"{name} RCM")
    print(f"  RCM 分析耗时 {time.time()-t0:.2f}s")

    # 对比
    print(f"\n  {'_'*60}")
    print(f"  *** 动态 vs 固定 BSR 对比: {name}")
    print(f"  {'_'*60}")
    fmt = "  {:<25} {:>12} {:>12}"
    print(fmt.format("指标", "原始", "RCM"))
    print(f"  {'_'*60}")
    print(fmt.format("动态填充率(全局)",
        f"{stats_orig['global_fill']*100:.2f}%",
        f"{stats_rcm['global_fill']*100:.2f}%"))
    print(fmt.format("列重叠率(overlap)",
        f"{stats_orig['avg_overlap']:.2f}x",
        f"{stats_rcm['avg_overlap']:.2f}x"))
    print(fmt.format("B读取节省",
        f"{stats_orig['B_savings']:.2f}x",
        f"{stats_rcm['B_savings']:.2f}x"))
    print(fmt.format("平均U(列并集)",
        f"{stats_orig['avg_U']:.1f}",
        f"{stats_rcm['avg_U']:.1f}"))
    print(fmt.format("RCM耗时", "--", f"{t_rcm:.2f}s"))

    # 判定
    eff = stats_rcm['global_fill']
    print(f"\n  * 判决 (动态 row-panel):")
    if eff > 0.30:
        print(f"    EXCELLENT: 全局效率 {eff*100:.1f}% > 30%!")
    elif eff > 0.15:
        print(f"    GOOD: 全局效率 {eff*100:.1f}% > 15%")
    elif eff > 0.0625:
        print(f"    VIABLE: 全局效率 {eff*100:.1f}% > 6.25% (超过盈亏线)")
    else:
        print(f"    NO-GO: 全局效率 {eff*100:.1f}% < 6.25%")

    return stats_orig, stats_rcm, name


def main():
    if len(sys.argv) < 2:
        print("用法: python3 dynamic_panel_analysis.py <file.csrbin>")
        print("      python3 dynamic_panel_analysis.py all")
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
        print(f"找到 {len(csrbin_files)} 个 .csrbin 文件")

        summary = []
        for path in csrbin_files:
            try:
                s_orig, s_rcm, name = analyze_matrix(path)
                if s_rcm is not None:
                    summary.append((name, s_orig, s_rcm))
            except Exception as e:
                print(f"  [错误] {path}: {e}")
                import traceback
                traceback.print_exc()

        if summary:
            print(f"\n\n{'='*95}")
            print(f"  全量汇总: 动态 Row-Panel 分析")
            print(f"{'='*95}")
            hdr = "  {:<18} {:>10} {:>10} {:>10} {:>10} {:>10} {:>8}"
            print(hdr.format("矩阵", "avg_deg", "原始fill", "RCM_fill",
                             "overlap", "B节省", "判定"))
            print(f"  {'_'*82}")
            for name, so, sr in summary:
                avg_deg = so.get('avg_U', 0) / 1  # approximate
                eo = so['global_fill'] * 100
                er = sr['global_fill'] * 100
                ov = sr['avg_overlap']
                bs = sr['B_savings']
                if er > 30: verdict = "EXCEL"
                elif er > 15: verdict = "GOOD"
                elif er > 6.25: verdict = "VIABLE"
                else: verdict = "NO-GO"
                row = "  {:<18} {:>10.1f} {:>9.2f}% {:>9.2f}% {:>9.2f}x {:>9.2f}x {:>8}"
                print(row.format(name, ov, eo, er, ov, bs, verdict))
    else:
        path = sys.argv[1]
        if not os.path.exists(path):
            candidate = os.path.join(data_dir, path, path + '.csrbin')
            if os.path.exists(candidate):
                path = candidate
        analyze_matrix(path)


if __name__ == "__main__":
    main()
