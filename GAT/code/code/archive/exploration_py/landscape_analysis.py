#!/usr/bin/env python3
"""
Landscape Analysis for Column-Anchor Tiling
============================================
全景分析脚本：对稀疏矩阵计算图拓扑指标 + 列锚点分组效果 + 改进策略对比

用法：python3 landscape_analysis.py /path/to/matrix1.csrbin [matrix2.csrbin ...]

输出：
  1. 每个矩阵的详细报告
  2. 所有矩阵的汇总对比表
  3. 关键指标之间的相关性分析

零外部依赖，纯 Python + numpy（超算兼容）
"""

import numpy as np
import struct
import sys
import os
import time
from collections import defaultdict

# ============================================================
# 第一部分：读取 csrbin 格式
# ============================================================

def read_csrbin(filepath):
    """
    读取 .csrbin 二进制格式的稀疏矩阵
    
    格式说明：
      字节 0-3:   ptype (uint32) — 0=uint32 indptr, 否则 uint64
      字节 4-7:   dtype (uint32) — 0=uint32 indices, 否则 uint64
      字节 8-11:  vtype (uint32) — 2=float values, 否则 double
      字节 12-19: nrow  (uint64) — 行数
      字节 20-27: ncol  (uint64) — 列数
      字节 28-35: nnz   (uint64) — 非零元素个数
      之后依次是 indptr, indices, values 数组
    """
    with open(filepath, 'rb') as f:
        ptype = struct.unpack('I', f.read(4))[0]
        dtype = struct.unpack('I', f.read(4))[0]
        vtype = struct.unpack('I', f.read(4))[0]
        nrow  = struct.unpack('Q', f.read(8))[0]
        ncol  = struct.unpack('Q', f.read(8))[0]
        nnz   = struct.unpack('Q', f.read(8))[0]
        
        if ptype == 0:
            indptr = np.frombuffer(f.read(4 * (nrow + 1)), dtype=np.uint32)
        else:
            indptr = np.frombuffer(f.read(8 * (nrow + 1)), dtype=np.uint64)
        
        if dtype == 0:
            indices = np.frombuffer(f.read(4 * nnz), dtype=np.uint32)
        else:
            indices = np.frombuffer(f.read(8 * nnz), dtype=np.uint64)
    
    return nrow, ncol, nnz, indptr, indices


# ============================================================
# 第二部分：图拓扑指标计算
# ============================================================

def compute_topology_metrics(nrow, ncol, nnz, indptr, indices):
    metrics = {}
    
    metrics['nrow'] = int(nrow)
    metrics['ncol'] = int(ncol)
    metrics['nnz'] = int(nnz)
    metrics['density'] = nnz / (nrow * ncol) if nrow * ncol > 0 else 0
    metrics['sparsity'] = 1.0 - metrics['density']
    
    # 行度数
    row_deg = np.diff(indptr).astype(np.int64)
    metrics['avg_row_deg'] = float(np.mean(row_deg))
    metrics['median_row_deg'] = float(np.median(row_deg))
    metrics['std_row_deg'] = float(np.std(row_deg))
    metrics['max_row_deg'] = int(np.max(row_deg))
    metrics['min_row_deg'] = int(np.min(row_deg))
    metrics['p90_row_deg'] = float(np.percentile(row_deg, 90))
    metrics['p99_row_deg'] = float(np.percentile(row_deg, 99))
    metrics['zero_rows'] = int(np.sum(row_deg == 0))
    
    # 行度 Gini 系数
    sorted_deg = np.sort(row_deg).astype(np.float64)
    n = len(sorted_deg)
    if np.sum(sorted_deg) > 0:
        metrics['gini_row_deg'] = float((2.0 * np.sum((np.arange(1, n+1) * sorted_deg))) / (n * np.sum(sorted_deg)) - (n + 1) / n)
    else:
        metrics['gini_row_deg'] = 0.0
    
    # 列度数
    col_deg = np.zeros(ncol, dtype=np.int64)
    np.add.at(col_deg, indices.astype(np.int64), 1)
    
    nonzero_cols = col_deg[col_deg > 0]
    metrics['num_nonzero_cols'] = int(len(nonzero_cols))
    metrics['avg_col_deg'] = float(np.mean(nonzero_cols)) if len(nonzero_cols) > 0 else 0
    metrics['median_col_deg'] = float(np.median(nonzero_cols)) if len(nonzero_cols) > 0 else 0
    metrics['max_col_deg'] = int(np.max(col_deg))
    metrics['p99_col_deg'] = float(np.percentile(nonzero_cols, 99)) if len(nonzero_cols) > 0 else 0
    
    # 列度 Gini
    sorted_cdeg = np.sort(nonzero_cols).astype(np.float64)
    nc = len(sorted_cdeg)
    if nc > 0 and np.sum(sorted_cdeg) > 0:
        metrics['gini_col_deg'] = float((2.0 * np.sum(np.arange(1, nc+1) * sorted_cdeg)) / (nc * np.sum(sorted_cdeg)) - (nc + 1) / nc)
    else:
        metrics['gini_col_deg'] = 0.0
    
    # Top-1% 列覆盖率
    top_k = max(1, int(0.01 * len(nonzero_cols)))
    top_cols_nnz = int(np.sum(np.sort(nonzero_cols)[-top_k:]))
    metrics['top1pct_col_coverage'] = top_cols_nnz / nnz if nnz > 0 else 0
    
    top_k10 = max(1, int(0.10 * len(nonzero_cols)))
    top10_cols_nnz = int(np.sum(np.sort(nonzero_cols)[-top_k10:]))
    metrics['top10pct_col_coverage'] = top10_cols_nnz / nnz if nnz > 0 else 0
    
    # 采样聚集系数
    metrics['clustering_coeff'] = estimate_clustering_coefficient(
        nrow, indptr, indices, sample_size=5000
    )
    
    return metrics, row_deg, col_deg


def estimate_clustering_coefficient(nrow, indptr, indices, sample_size=5000):
    row_deg = np.diff(indptr).astype(np.int64)
    eligible = np.where(row_deg >= 2)[0]
    
    if len(eligible) == 0:
        return 0.0
    
    sample_size = min(sample_size, len(eligible))
    rng = np.random.RandomState(42)
    sampled = rng.choice(eligible, size=sample_size, replace=False)
    
    cc_values = []
    for node in sampled:
        deg = int(indptr[node + 1] - indptr[node])
        if deg < 2 or deg > 1000:
            continue
        
        neighbors = set(indices[indptr[node]:indptr[node + 1]].tolist())
        neighbors.discard(int(node))
        
        if len(neighbors) < 2:
            continue
        
        triangles = 0
        neighbor_list = list(neighbors)
        for ni in neighbor_list:
            ni = int(ni)
            if ni >= nrow:
                continue
            ni_neighbors = set(indices[indptr[ni]:indptr[ni + 1]].tolist())
            triangles += len(neighbors & ni_neighbors)
        
        triangles //= 2
        max_edges = len(neighbors) * (len(neighbors) - 1) // 2
        if max_edges > 0:
            cc_values.append(triangles / max_edges)
    
    return float(np.mean(cc_values)) if cc_values else 0.0


# ============================================================
# 第三部分：列锚点分组模拟
# ============================================================

def build_csc(nrow, ncol, nnz, indptr, indices):
    col_ptr = np.zeros(ncol + 1, dtype=np.int64)
    for idx in range(nnz):
        col_ptr[int(indices[idx]) + 1] += 1
    for j in range(ncol):
        col_ptr[j + 1] += col_ptr[j]
    
    row_indices = np.zeros(nnz, dtype=np.int64)
    temp_ptr = col_ptr[:ncol].copy()
    for i in range(nrow):
        for pos in range(int(indptr[i]), int(indptr[i + 1])):
            col = int(indices[pos])
            row_indices[temp_ptr[col]] = i
            temp_ptr[col] += 1
    
    return col_ptr, row_indices


def simulate_column_anchor_current(nrow, ncol, indptr, indices, col_deg,
                                    row_deg, panel_size=16, tile_width=32,
                                    min_col_freq=16):
    col_ptr, row_indices = build_csc(nrow, ncol, int(indptr[-1]), indptr, indices)
    col_order = np.argsort(-col_deg)
    
    assigned = np.zeros(nrow, dtype=bool)
    panels = []
    
    for c in col_order:
        c = int(c)
        if col_deg[c] < min_col_freq:
            break
        
        candidate_rows = []
        for pos in range(int(col_ptr[c]), int(col_ptr[c + 1])):
            r = int(row_indices[pos])
            if not assigned[r]:
                candidate_rows.append(r)
        
        if len(candidate_rows) < panel_size:
            continue
        
        candidate_rows.sort(key=lambda r: row_deg[r])
        selected = candidate_rows[:panel_size]
        
        col_union = set()
        total_deg = 0
        for r in selected:
            cols_of_row = set(indices[indptr[r]:indptr[r + 1]].tolist())
            col_union |= cols_of_row
            total_deg += len(cols_of_row)
        
        U = len(col_union)
        ntiles = (U + tile_width - 1) // tile_width
        fill_rate = total_deg / (panel_size * tile_width * ntiles) if ntiles > 0 else 0
        b_reuse = total_deg / U if U > 0 else 0
        
        panels.append({
            'anchor_col': c,
            'anchor_deg': int(col_deg[c]),
            'U': U,
            'ntiles': ntiles,
            'total_deg': total_deg,
            'fill_rate': fill_rate,
            'b_reuse': b_reuse,
        })
        
        for r in selected:
            assigned[r] = True
    
    return panels, assigned


def simulate_incremental_greedy(nrow, ncol, indptr, indices, col_deg,
                                 row_deg, panel_size=16, tile_width=32,
                                 min_col_freq=16):
    col_ptr, row_indices = build_csc(nrow, ncol, int(indptr[-1]), indptr, indices)
    col_order = np.argsort(-col_deg)
    
    assigned = np.zeros(nrow, dtype=bool)
    panels = []
    row_colsets = {}
    
    for c in col_order:
        c = int(c)
        if col_deg[c] < min_col_freq:
            break
        
        candidates = []
        for pos in range(int(col_ptr[c]), int(col_ptr[c + 1])):
            r = int(row_indices[pos])
            if not assigned[r]:
                candidates.append(r)
        
        if len(candidates) < panel_size:
            continue
        
        for r in candidates:
            if r not in row_colsets:
                row_colsets[r] = set(indices[indptr[r]:indptr[r + 1]].tolist())
        
        candidates.sort(key=lambda r: row_deg[r])
        
        selected = [candidates[0]]
        current_U = set(row_colsets[candidates[0]])
        remaining = set(candidates[1:])
        
        for k in range(1, panel_size):
            if not remaining:
                break
            
            current_tiles = (len(current_U) + tile_width - 1) // tile_width
            
            best_r = None
            best_cost = float('inf')
            best_new_cols = float('inf')
            best_deg = float('inf')
            
            if len(remaining) > 200:
                remaining_sorted = sorted(remaining, key=lambda r: row_deg[r])[:200]
            else:
                remaining_sorted = list(remaining)
            
            for r in remaining_sorted:
                new_U = current_U | row_colsets[r]
                new_tiles = (len(new_U) + tile_width - 1) // tile_width
                cost = new_tiles - current_tiles
                new_cols = len(new_U) - len(current_U)
                
                if (cost < best_cost or
                    (cost == best_cost and new_cols < best_new_cols) or
                    (cost == best_cost and new_cols == best_new_cols and row_deg[r] < best_deg)):
                    best_r = r
                    best_cost = cost
                    best_new_cols = new_cols
                    best_deg = row_deg[r]
            
            if best_r is not None:
                selected.append(best_r)
                current_U = current_U | row_colsets[best_r]
                remaining.discard(best_r)
        
        if len(selected) < panel_size:
            continue
        
        U = len(current_U)
        total_deg = sum(row_deg[r] for r in selected)
        ntiles = (U + tile_width - 1) // tile_width
        fill_rate = total_deg / (panel_size * tile_width * ntiles) if ntiles > 0 else 0
        b_reuse = total_deg / U if U > 0 else 0
        
        panels.append({
            'anchor_col': c,
            'U': U,
            'ntiles': ntiles,
            'total_deg': total_deg,
            'fill_rate': fill_rate,
            'b_reuse': b_reuse,
        })
        
        for r in selected:
            assigned[r] = True
    
    row_colsets.clear()
    return panels, assigned


# ============================================================
# 第四部分：统计汇总
# ============================================================

def summarize_panels(panels, nrow, nnz, row_deg, assigned, label=""):
    if not panels:
        return {
            'label': label, 'num_panels': 0,
            'coverage_rows': 0, 'coverage_rows_pct': 0,
            'coverage_nnz': 0, 'coverage_nnz_pct': 0,
            'mean_fill': 0, 'median_fill': 0, 'p25_fill': 0, 'p75_fill': 0,
            'mean_U': 0, 'median_U': 0, 'mean_ntiles': 0,
            'mean_b_reuse': 0, 'global_b_reuse': 0,
            'u_le_32_pct': 0, 'u_le_64_pct': 0, 'u_le_128_pct': 0,
            'above_breakeven_pct': 0,
        }
    
    fills = [p['fill_rate'] for p in panels]
    Us = [p['U'] for p in panels]
    ntiles_list = [p['ntiles'] for p in panels]
    b_reuses = [p['b_reuse'] for p in panels]
    total_degs = [p['total_deg'] for p in panels]
    
    covered_rows = int(np.sum(assigned))
    covered_nnz = sum(row_deg[i] for i in range(len(row_deg)) if assigned[i])
    
    Us_arr = np.array(Us)
    fills_arr = np.array(fills)
    
    total_B_actual = sum(Us)
    total_B_baseline = sum(total_degs)
    global_b_reuse = total_B_baseline / total_B_actual if total_B_actual > 0 else 0
    
    return {
        'label': label,
        'num_panels': len(panels),
        'coverage_rows': covered_rows,
        'coverage_rows_pct': covered_rows / nrow * 100,
        'coverage_nnz': covered_nnz,
        'coverage_nnz_pct': covered_nnz / nnz * 100 if nnz > 0 else 0,
        'mean_fill': float(np.mean(fills)) * 100,
        'median_fill': float(np.median(fills)) * 100,
        'p25_fill': float(np.percentile(fills, 25)) * 100,
        'p75_fill': float(np.percentile(fills, 75)) * 100,
        'mean_U': float(np.mean(Us)),
        'median_U': float(np.median(Us)),
        'mean_ntiles': float(np.mean(ntiles_list)),
        'mean_b_reuse': float(np.mean(b_reuses)),
        'global_b_reuse': global_b_reuse,
        'u_le_32_pct': float(np.sum(Us_arr <= 32) / len(Us_arr) * 100),
        'u_le_64_pct': float(np.sum(Us_arr <= 64) / len(Us_arr) * 100),
        'u_le_128_pct': float(np.sum(Us_arr <= 128) / len(Us_arr) * 100),
        'above_breakeven_pct': float(np.sum(fills_arr >= 0.0625) / len(fills_arr) * 100),
    }


# ============================================================
# 第五部分：输出报告
# ============================================================

def print_matrix_report(name, metrics, summary_current, summary_greedy):
    print(f"\n{'='*70}")
    print(f" 矩阵: {name}")
    print(f"{'='*70}")
    
    print(f"\n--- 基本信息 ---")
    print(f"  行数:       {metrics['nrow']:>12,}")
    print(f"  列数:       {metrics['ncol']:>12,}")
    print(f"  NNZ:        {metrics['nnz']:>12,}")
    print(f"  稀疏度:     {metrics['sparsity']*100:>11.4f}%")
    
    print(f"\n--- 行度数分布 ---")
    print(f"  平均:    {metrics['avg_row_deg']:>8.1f}")
    print(f"  中位数:  {metrics['median_row_deg']:>8.1f}")
    print(f"  标准差:  {metrics['std_row_deg']:>8.1f}")
    print(f"  最小/最大: {metrics['min_row_deg']} / {metrics['max_row_deg']}")
    print(f"  P90/P99: {metrics['p90_row_deg']:.0f} / {metrics['p99_row_deg']:.0f}")
    print(f"  Gini:    {metrics['gini_row_deg']:.4f}  (0=均匀, 1=集中)")
    print(f"  零度行:  {metrics['zero_rows']}")
    
    print(f"\n--- 列度数分布 ---")
    print(f"  非零列数:  {metrics['num_nonzero_cols']:>10,}")
    print(f"  平均:      {metrics['avg_col_deg']:>8.1f}")
    print(f"  中位数:    {metrics['median_col_deg']:>8.1f}")
    print(f"  最大:      {metrics['max_col_deg']:>8}")
    print(f"  Gini:      {metrics['gini_col_deg']:.4f}")
    print(f"  Top-1%列覆盖NNZ:  {metrics['top1pct_col_coverage']*100:.1f}%  <- 越高越适合列锚点")
    print(f"  Top-10%列覆盖NNZ: {metrics['top10pct_col_coverage']*100:.1f}%")
    
    print(f"\n--- 聚集系数 ---")
    print(f"  采样聚集系数: {metrics['clustering_coeff']:.4f}")
    
    for s in [summary_current, summary_greedy]:
        if s['num_panels'] == 0:
            print(f"\n--- {s['label']} --- 无 panel 生成")
            continue
        
        print(f"\n--- {s['label']} ---")
        print(f"  Panel 数:    {s['num_panels']:>8}")
        print(f"  覆盖行:      {s['coverage_rows']:>8,} ({s['coverage_rows_pct']:.1f}%)")
        print(f"  覆盖 NNZ:    {s['coverage_nnz']:>8,} ({s['coverage_nnz_pct']:.1f}%)")
        print(f"  平均 fill:   {s['mean_fill']:>7.1f}%")
        print(f"  中位 fill:   {s['median_fill']:>7.1f}%")
        print(f"  P25/P75:     {s['p25_fill']:.1f}% / {s['p75_fill']:.1f}%")
        print(f"  平均 U:      {s['mean_U']:>7.1f}")
        print(f"  中位 U:      {s['median_U']:>7.1f}")
        print(f"  平均 tile数: {s['mean_ntiles']:>6.1f}")
        print(f"  平均 B-reuse: {s['mean_b_reuse']:>6.2f}x")
        print(f"  全局 B-reuse: {s['global_b_reuse']:>6.2f}x")
        print(f"  U<=32 占比:  {s['u_le_32_pct']:>6.1f}%  (单tile最高效)")
        print(f"  U<=64 占比:  {s['u_le_64_pct']:>6.1f}%")
        print(f"  U<=128 占比: {s['u_le_128_pct']:>6.1f}%")
        print(f"  >=盈亏线:    {s['above_breakeven_pct']:>6.1f}%  (fill >= 6.25%)")


def print_comparison_table(all_results):
    print(f"\n\n{'='*120}")
    print(f" 全景对比表 (Landscape Summary)")
    print(f"{'='*120}")
    
    header = (f"{'Matrix':<20} {'avg_deg':>8} {'Gini_r':>7} {'Gini_c':>7} {'Top1%':>6} {'CC':>6} "
              f"| {'fill_C':>7} {'cov_C':>6} {'U_C':>6} "
              f"| {'fill_G':>7} {'cov_G':>6} {'U_G':>6} "
              f"| {'dfill':>6} {'dcov':>6}")
    print(header)
    print("-" * 120)
    
    for r in all_results:
        name = r['name'][:20]
        m = r['metrics']
        sc = r['summary_current']
        sg = r['summary_greedy']
        
        if sc['num_panels'] == 0 or sg['num_panels'] == 0:
            print(f"{name:<20} {m['avg_row_deg']:>8.1f} {m['gini_row_deg']:>7.3f} "
                  f"{m['gini_col_deg']:>7.3f} {m['top1pct_col_coverage']*100:>5.1f}% "
                  f"{m['clustering_coeff']:>6.3f} | (insufficient data)")
            continue
        
        delta_fill = sg['mean_fill'] - sc['mean_fill']
        delta_cov = sg['coverage_nnz_pct'] - sc['coverage_nnz_pct']
        
        print(f"{name:<20} {m['avg_row_deg']:>8.1f} {m['gini_row_deg']:>7.3f} "
              f"{m['gini_col_deg']:>7.3f} {m['top1pct_col_coverage']*100:>5.1f}% "
              f"{m['clustering_coeff']:>6.3f} "
              f"| {sc['mean_fill']:>6.1f}% {sc['coverage_nnz_pct']:>5.1f}% {sc['mean_U']:>6.1f} "
              f"| {sg['mean_fill']:>6.1f}% {sg['coverage_nnz_pct']:>5.1f}% {sg['mean_U']:>6.1f} "
              f"| {delta_fill:>+5.1f}% {delta_cov:>+5.1f}%")
    
    print("-" * 120)
    print(f"fill_C/G = Current/Greedy mean fill rate")
    print(f"cov_C/G  = Current/Greedy NNZ coverage")
    print(f"U_C/G    = Current/Greedy mean column union size")
    print(f"dfill/dcov = Greedy improvement over Current")


# ============================================================
# 主函数
# ============================================================

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 landscape_analysis.py matrix1.csrbin [matrix2.csrbin ...]")
        sys.exit(1)
    
    matrix_files = sys.argv[1:]
    all_results = []
    
    for filepath in matrix_files:
        if not os.path.exists(filepath):
            print(f"[WARN] File not found: {filepath}, skipping")
            continue
        
        name = os.path.basename(filepath).replace('.csrbin', '')
        print(f"\n{'#'*70}")
        print(f"# Analyzing: {name}")
        print(f"# File: {filepath}")
        print(f"{'#'*70}")
        
        t0 = time.time()
        nrow, ncol, nnz, indptr, indices = read_csrbin(filepath)
        print(f"  Read: {nrow:,} rows, {ncol:,} cols, {nnz:,} NNZ ({time.time()-t0:.2f}s)")
        
        t0 = time.time()
        metrics, row_deg, col_deg = compute_topology_metrics(nrow, ncol, nnz, indptr, indices)
        print(f"  Topology analysis done ({time.time()-t0:.2f}s)")
        
        t0 = time.time()
        panels_current, assigned_current = simulate_column_anchor_current(
            nrow, ncol, indptr, indices, col_deg, row_deg)
        summary_current = summarize_panels(
            panels_current, nrow, nnz, row_deg, assigned_current,
            label="Current Column-Anchor (Config A)")
        print(f"  Current anchor: {len(panels_current)} panels ({time.time()-t0:.2f}s)")
        
        t0 = time.time()
        panels_greedy, assigned_greedy = simulate_incremental_greedy(
            nrow, ncol, indptr, indices, col_deg, row_deg)
        summary_greedy = summarize_panels(
            panels_greedy, nrow, nnz, row_deg, assigned_greedy,
            label="Incremental Greedy (ceil(U/32) aware)")
        print(f"  Incremental greedy: {len(panels_greedy)} panels ({time.time()-t0:.2f}s)")
        
        print_matrix_report(name, metrics, summary_current, summary_greedy)
        
        all_results.append({
            'name': name,
            'metrics': metrics,
            'summary_current': summary_current,
            'summary_greedy': summary_greedy,
        })
    
    if len(all_results) > 1:
        print_comparison_table(all_results)
    
    print(f"\nDone! Processed {len(all_results)} matrices.")


if __name__ == '__main__':
    main()
