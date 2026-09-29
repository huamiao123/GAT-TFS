#!/usr/bin/env python3
"""
BSR Fill Rate Analysis (numpy-only, 读取 .csrbin 格式)

核心问题：重排序后，16x32 BSR 块的平均非零填充率是多少？
- > 6.25%: AMX 比 AVX-512 快
- 30~50%: AMX 5~8x 加速，论文成立
"""

import sys, os, struct
import numpy as np
from collections import deque
import time

TILE_ROWS = 16
TILE_COLS = 32

# ============================================================
# 1. 读取 .csrbin（JitSpMM 二进制格式）
# ============================================================
# 格式:
#   bytes 0-3:   ptype  (uint32) - indptr/indices 类型, 0=uint32
#   bytes 4-7:   dtype  (uint32) - 同上
#   bytes 8-11:  vtype  (uint32) - values 类型, 2=float32
#   bytes 12-19: nrow   (uint64) - 行数
#   bytes 20-27: ncol   (uint64) - 列数
#   bytes 28-35: nnz    (uint64) - 非零元素数
#   后续: indptr[nrow+1] (uint32), indices[nnz] (uint32), values[nnz] (float32)

def read_csrbin(filepath):
    with open(filepath, 'rb') as f:
        # 读头部: 3个uint32 + 3个uint64 = 12 + 24 = 36 bytes
        ptype = struct.unpack('<I', f.read(4))[0]   # '<I' = little-endian uint32
        dtype = struct.unpack('<I', f.read(4))[0]
        vtype = struct.unpack('<I', f.read(4))[0]
        nrow  = struct.unpack('<Q', f.read(8))[0]   # '<Q' = little-endian uint64
        ncol  = struct.unpack('<Q', f.read(8))[0]
        nnz   = struct.unpack('<Q', f.read(8))[0]

        # 读 CSR 三数组
        indptr  = np.frombuffer(f.read((nrow+1)*4), dtype=np.uint32).astype(np.int64)
        indices = np.frombuffer(f.read(nnz*4),      dtype=np.uint32).astype(np.int32)
        values  = np.frombuffer(f.read(nnz*4),      dtype=np.float32)

    # 校验
    assert indptr[0] == 0, f"indptr[0]={indptr[0]}, expected 0"
    assert indptr[nrow] == nnz, f"indptr[nrow]={indptr[nrow]}, expected {nnz}"

    return indptr, indices, values, int(nrow), int(ncol), int(nnz)


# ============================================================
# 2. RCM 重排序（纯 Python + numpy, 无 scipy）
# ============================================================
# Reverse Cuthill-McKee 算法:
#   1. 对称化得到无向图邻接表
#   2. 找伪外围节点（BFS最远点）作为起点
#   3. BFS 每层按度数升序展开
#   4. 翻转序列 = RCM 序
# 效果: 最小化带宽，非零元素聚集到对角线附近

def build_adjacency(indptr, indices, M):
    """从 CSR 构建对称邻接表"""
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

    del seen  # 释放内存
    return adj


def find_pseudo_peripheral(adj, M, degree):
    """找伪外围节点: 两轮 BFS 取最远点"""
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
    """RCM 重排序，返回 perm 数组 (perm[new] = old)"""
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
            # 邻居按度数升序
            neighbors = [(degree[nb], nb) for nb in adj[node] if not visited[nb]]
            neighbors.sort()
            for _, nb in neighbors:
                if not visited[nb]:
                    visited[nb] = True
                    queue.append(nb)

    perm = np.array(perm[::-1], dtype=np.int32)  # reverse = RCM
    print(f"{time.time()-t0:.1f}s")
    return perm


# ============================================================
# 3. 应用重排序 (P * A * P^T)
# ============================================================

def apply_permutation(indptr, indices, values, M, perm):
    """行列同时重排"""
    inv_perm = np.empty(M, dtype=np.int32)
    inv_perm[perm] = np.arange(M, dtype=np.int32)

    # 计算新的每行非零数（预分配）
    new_row_nnz = np.zeros(M, dtype=np.int64)
    for old_i in range(M):
        new_i = inv_perm[old_i]
        cnt = indptr[old_i+1] - indptr[old_i]
        new_row_nnz[new_i] = cnt

    new_indptr = np.zeros(M + 1, dtype=np.int64)
    np.cumsum(new_row_nnz, out=new_indptr[1:])

    nnz = int(new_indptr[M])
    new_indices = np.empty(nnz, dtype=np.int32)
    new_values = np.empty(nnz, dtype=np.float32)

    # 填充
    write_pos = new_indptr[:-1].copy()
    for old_i in range(M):
        new_i = inv_perm[old_i]
        start = int(indptr[old_i])
        end = int(indptr[old_i+1])
        if start == end:
            continue
        old_cols = indices[start:end]
        new_cols = inv_perm[old_cols[old_cols < M]]  # 安全过滤
        old_vals = values[start:start+len(new_cols)]

        wp = int(write_pos[new_i])
        n = len(new_cols)
        new_indices[wp:wp+n] = new_cols
        new_values[wp:wp+n] = old_vals
        write_pos[new_i] += n

    # 每行内按列排序（BSR统计不要求，但更规范）
    for i in range(M):
        s = int(new_indptr[i])
        e = int(new_indptr[i+1])
        if e - s > 1:
            order = np.argsort(new_indices[s:e])
            new_indices[s:e] = new_indices[s:e][order]
            new_values[s:e] = new_values[s:e][order]

    return new_indptr, new_indices, new_values


# ============================================================
# 4. BSR 填充率统计
# ============================================================

def compute_bsr_stats(indptr, indices, M, N, nnz, label=""):
    """统计 TILE_ROWS x TILE_COLS 分块下的填充率"""
    capacity = TILE_ROWS * TILE_COLS
    fill_rates = []
    total_blocks = 0
    total_padded = 0
    total_useful = 0

    n_block_rows = (M + TILE_ROWS - 1) // TILE_ROWS

    for bi in range(n_block_rows):
        r_start = bi * TILE_ROWS
        r_end = min(r_start + TILE_ROWS, M)

        # 按 block col 统计非零数
        block_nnz = {}
        for r in range(r_start, r_end):
            for idx in range(int(indptr[r]), int(indptr[r+1])):
                bj = int(indices[idx]) // TILE_COLS
                block_nnz[bj] = block_nnz.get(bj, 0) + 1

        for bj, bnnz in block_nnz.items():
            total_blocks += 1
            fill_rates.append(bnnz / capacity)
            total_padded += capacity
            total_useful += bnnz

    fill_rates = np.array(fill_rates) if fill_rates else np.array([])
    avg_fill = float(np.mean(fill_rates)) if len(fill_rates) > 0 else 0
    median_fill = float(np.median(fill_rates)) if len(fill_rates) > 0 else 0
    overall_eff = total_useful / total_padded if total_padded > 0 else 0

    pct = lambda th: float(np.mean(fill_rates > th) * 100) if len(fill_rates) > 0 else 0

    bins = [0, 0.01, 0.05, 0.10, 0.20, 0.30, 0.50, 0.75, 1.01]
    hist, _ = np.histogram(fill_rates, bins=bins)

    return {
        'label': label, 'M': M, 'N': N, 'nnz': nnz,
        'total_blocks': total_blocks,
        'avg_fill': avg_fill, 'median_fill': median_fill,
        'overall_efficiency': overall_eff,
        'pct_above_6': pct(0.0625), 'pct_above_12': pct(0.125),
        'pct_above_25': pct(0.25), 'pct_above_50': pct(0.50),
        'hist': hist, 'hist_bins': bins,
    }


def print_stats(stats):
    label = stats['label']
    print(f"\n{'='*60}")
    print(f"  {label}")
    print(f"{'='*60}")
    print(f"  矩阵: {stats['M']} x {stats['N']}, NNZ = {stats['nnz']:,}")
    print(f"  非空 {TILE_ROWS}x{TILE_COLS} 块数: {stats['total_blocks']:,}")
    print(f"")
    print(f"  * 平均填充率:   {stats['avg_fill']*100:6.2f}%")
    print(f"  * 中位填充率:   {stats['median_fill']*100:6.2f}%")
    print(f"  * 全局效率:     {stats['overall_efficiency']*100:6.2f}%  (有效ops / 总tile ops)")
    print(f"")
    print(f"  --- 关键阈值 ---")
    print(f"  填充率 > 6.25% (AMX盈亏线):  {stats['pct_above_6']:5.1f}% 的块")
    print(f"  填充率 > 12.5%:               {stats['pct_above_12']:5.1f}% 的块")
    print(f"  填充率 > 25%:                 {stats['pct_above_25']:5.1f}% 的块")
    print(f"  填充率 > 50%:                 {stats['pct_above_50']:5.1f}% 的块")
    print(f"")
    print(f"  --- 填充率分布 ---")
    bins = stats['hist_bins']
    hist = stats['hist']
    total = stats['total_blocks']
    for i in range(len(hist)):
        bar_len = int(hist[i] / total * 50) if total > 0 else 0
        bar = '#' * bar_len
        pct = hist[i] / total * 100 if total > 0 else 0
        print(f"  [{bins[i]*100:5.1f}%,{bins[i+1]*100:5.1f}%) : {hist[i]:7d} ({pct:5.1f}%) {bar}")


# ============================================================
# 5. 主流程
# ============================================================

def analyze_matrix(csrbin_path):
    name = os.path.basename(os.path.dirname(csrbin_path))
    if not name or name == '.':
        name = os.path.basename(csrbin_path).replace('.csrbin', '')
    print(f"\n{'#'*70}")
    print(f"#  矩阵: {name}")
    print(f"{'#'*70}")

    # 读取
    t0 = time.time()
    indptr, indices, values, M, N, nnz = read_csrbin(csrbin_path)
    print(f"  读取完成: {M}x{N}, NNZ={nnz:,}, avg_deg={nnz/M:.1f}, 耗时 {time.time()-t0:.2f}s")

    # 原始 BSR 统计
    t0 = time.time()
    stats_orig = compute_bsr_stats(indptr, indices, M, N, nnz,
                                    label=f"{name} -- 原始")
    print(f"  原始统计完成, 耗时 {time.time()-t0:.2f}s")
    print_stats(stats_orig)

    # RCM 重排
    if M != N:
        print(f"  [跳过RCM] 非方阵")
        return stats_orig, None, name

    # 大矩阵的 RCM 可能很慢（纯Python），给个预估
    if M > 5_000_000:
        print(f"  [警告] 矩阵很大 ({M:,} 行), RCM 可能需要几分钟...")

    t0 = time.time()
    perm = rcm_reorder(indptr, indices, M)
    t_rcm = time.time() - t0
    print(f"  RCM 总耗时: {t_rcm:.2f}s")

    # 应用重排
    t0 = time.time()
    new_indptr, new_indices, new_values = apply_permutation(
        indptr, indices, values, M, perm)
    print(f"  重排矩阵构建完成, 耗时 {time.time()-t0:.2f}s")

    # 重排后统计
    t0 = time.time()
    stats_reord = compute_bsr_stats(new_indptr, new_indices, M, N, nnz,
                                     label=f"{name} -- RCM重排后")
    print(f"  重排统计完成, 耗时 {time.time()-t0:.2f}s")
    print_stats(stats_reord)

    # 对比
    print(f"\n  {'_'*55}")
    print(f"  *** 对比: {name}")
    print(f"  {'_'*55}")
    fmt = "  {:<20} {:>10} {:>10} {:>10}"
    print(fmt.format("指标", "原始", "RCM", "变化"))
    print(f"  {'_'*55}")
    print(fmt.format("非空块数",
        f"{stats_orig['total_blocks']:,}", f"{stats_reord['total_blocks']:,}",
        f"{stats_reord['total_blocks']-stats_orig['total_blocks']:+,}"))
    print(fmt.format("平均填充率",
        f"{stats_orig['avg_fill']*100:.2f}%", f"{stats_reord['avg_fill']*100:.2f}%",
        f"{(stats_reord['avg_fill']-stats_orig['avg_fill'])*100:+.2f}%"))
    print(fmt.format("全局效率",
        f"{stats_orig['overall_efficiency']*100:.2f}%",
        f"{stats_reord['overall_efficiency']*100:.2f}%",
        f"{(stats_reord['overall_efficiency']-stats_orig['overall_efficiency'])*100:+.2f}%"))
    print(fmt.format(">6.25%块占比",
        f"{stats_orig['pct_above_6']:.1f}%", f"{stats_reord['pct_above_6']:.1f}%",
        f"{stats_reord['pct_above_6']-stats_orig['pct_above_6']:+.1f}%"))
    print(f"  {'RCM耗时':<20} {'--':>10} {t_rcm:>9.2f}s")

    eff = stats_reord['overall_efficiency']
    print(f"\n  * 判决:")
    if eff > 0.30:
        print(f"    EXCELLENT: 全局效率 {eff*100:.1f}% > 30%!")
    elif eff > 0.10:
        print(f"    GOOD: 全局效率 {eff*100:.1f}% > 10%")
    elif eff > 0.0625:
        print(f"    MARGINAL: 全局效率 {eff*100:.1f}% > 6.25%")
    else:
        print(f"    NO-GO: 全局效率 {eff*100:.1f}% < 6.25%")

    return stats_orig, stats_reord, name


def main():
    if len(sys.argv) < 2:
        print("用法: python3 bsr_analysis.py <file.csrbin>")
        print("      python3 bsr_analysis.py all")
        sys.exit(1)

    data_dir = os.path.expanduser("~/data/SpMM_project/data")

    if sys.argv[1] == "all":
        # 扫描所有子目录下的 .csrbin
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
                s_orig, s_reord, name = analyze_matrix(path)
                if s_reord is not None:
                    summary.append((name, s_orig, s_reord))
            except Exception as e:
                print(f"  [错误] {path}: {e}")
                import traceback
                traceback.print_exc()

        if summary:
            print(f"\n\n{'='*85}")
            print(f"  全量汇总: RCM 重排序对 {TILE_ROWS}x{TILE_COLS} BSR 填充率的影响")
            print(f"{'='*85}")
            hdr = "  {:<22} {:>12} {:>10} {:>10} {:>8} {:>8}"
            print(hdr.format("矩阵", "NNZ", "原始效率", "RCM效率", "提升", "判定"))
            print(f"  {'_'*74}")
            for name, so, sr in summary:
                eo = so['overall_efficiency'] * 100
                er = sr['overall_efficiency'] * 100
                delta = er - eo
                if er > 30: verdict = "EXCEL"
                elif er > 10: verdict = "GOOD"
                elif er > 6.25: verdict = "MARGIN"
                else: verdict = "NO-GO"
                row = "  {:<22} {:>12,} {:>9.2f}% {:>9.2f}% {:>+7.2f}% {:>8}"
                print(row.format(name, so['nnz'], eo, er, delta, verdict))
    else:
        path = sys.argv[1]
        if not os.path.exists(path):
            # 尝试在 data_dir 下找
            candidate = os.path.join(data_dir, path, path + '.csrbin')
            if os.path.exists(candidate):
                path = candidate
        analyze_matrix(path)


if __name__ == "__main__":
    main()
