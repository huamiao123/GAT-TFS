#!/usr/bin/env python3
"""
边际 NNZ/U 行选择策略 vs 当前策略对比

核心思路：
  当前策略：anchor column c → 选包含 c 的低度行（控制 U）
  新策略：  anchor column c → 贪心选"每新增 1 个 U 带来最多 NNZ"的行

对比指标：
  1. AMX 覆盖的 NNZ 占比（越高越好）
  2. 总 gather = ΣU（越低越好）
  3. NNZ / ΣU = 全局 B-reuse（越高越好）
  4. FB 剩余 NNZ（越低越好）
"""

import struct
import numpy as np
import os
import time

def read_csrbin(path):
    with open(path, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('III', f.read(12))
        nrow, ncol, nnz = struct.unpack('QQQ', f.read(24))
        indptr = np.frombuffer(f.read((nrow+1)*4), dtype=np.uint32)
        indices = np.frombuffer(f.read(nnz*4), dtype=np.uint32)
    return nrow, ncol, nnz, indptr, indices

def build_csc(nrow, ncol, nnz, indptr, indices):
    """构建 CSC（列到行的映射）"""
    col_deg = np.bincount(indices, minlength=ncol)
    col_ptr = np.zeros(ncol + 1, dtype=np.int64)
    col_ptr[1:] = np.cumsum(col_deg)
    col_indices = np.empty(nnz, dtype=np.int64)
    col_pos = col_ptr[:-1].copy()
    for i in range(nrow):
        for idx in range(indptr[i], indptr[i+1]):
            j = indices[idx]
            col_indices[col_pos[j]] = i
            col_pos[j] += 1
    return col_deg, col_ptr, col_indices

def get_row_cols(r, indptr, indices):
    """获取行 r 的列集合"""
    return set(indices[indptr[r]:indptr[r+1]])

def strategy_low_degree(avail_rows, row_degrees, indptr, indices, 
                        panel_union, TILE_R=16):
    """当前策略：选度数最低的行"""
    if len(avail_rows) <= TILE_R:
        return list(avail_rows)
    
    # 按度数升序，取前 TILE_R 个
    degs = row_degrees[avail_rows]
    k = min(TILE_R, len(avail_rows)) - 1
    part_idx = np.argpartition(degs, k)[:TILE_R]
    return [avail_rows[i] for i in part_idx]

def strategy_high_degree(avail_rows, row_degrees, indptr, indices,
                         panel_union, TILE_R=16):
    """Config B：选度数最高的行"""
    if len(avail_rows) <= TILE_R:
        return list(avail_rows)
    
    degs = row_degrees[avail_rows]
    k = min(TILE_R, len(avail_rows)) - 1
    part_idx = np.argpartition(-degs, k)[:TILE_R]
    return [avail_rows[i] for i in part_idx]

def strategy_marginal(avail_rows, row_degrees, indptr, indices,
                      panel_union, TILE_R=16):
    """
    新策略：贪心选边际 NNZ/U 最高的行
    
    每一步：
      对所有候选行，计算 new_cols = cols(row) - current_union
      value = degree(row) / max(len(new_cols), 1)
      选 value 最高的行加入 panel
      更新 current_union
    
    复杂度：O(TILE_R × |avail| × avg_deg)
    对 |avail| 很大时需要剪枝
    """
    if len(avail_rows) <= TILE_R:
        return list(avail_rows)
    
    selected = []
    current_union = set(panel_union)  # 初始包含 anchor column
    remaining = set(range(len(avail_rows)))
    
    # 预计算每行的列集合
    row_col_sets = {}
    for idx in range(len(avail_rows)):
        r = avail_rows[idx]
        row_col_sets[idx] = set(indices[indptr[r]:indptr[r+1]])
    
    for _ in range(min(TILE_R, len(avail_rows))):
        best_idx = -1
        best_value = -1
        best_new_cols = None
        
        # 如果候选太多，随机采样加速
        if len(remaining) > 200:
            # 采样 200 个候选 + 所有度数 top-50 的行
            sampled = set(np.random.choice(list(remaining), 
                         min(200, len(remaining)), replace=False))
            # 确保高度行被考虑
            rem_list = list(remaining)
            rem_degs = row_degrees[avail_rows[rem_list]]
            if len(rem_list) > 50:
                top50 = np.argpartition(-rem_degs, 50)[:50]
                for t in top50:
                    sampled.add(rem_list[t])
            candidates = sampled
        else:
            candidates = remaining
        
        for idx in candidates:
            cols = row_col_sets[idx]
            new_cols = cols - current_union
            # 边际价值 = 该行的总 NNZ / 新增的列数
            # 如果新增列数=0，说明完全被 union 覆盖 → 价值极高（免费NNZ）
            new_col_count = len(new_cols)
            deg = len(cols)
            if new_col_count == 0:
                value = deg * 1000  # 免费行，极高优先级
            else:
                value = deg / new_col_count
            
            if value > best_value:
                best_value = value
                best_idx = idx
                best_new_cols = new_cols
        
        if best_idx < 0:
            break
        
        selected.append(avail_rows[best_idx])
        current_union.update(best_new_cols)
        remaining.discard(best_idx)
    
    return selected

def run_strategy(name, strategy_fn, nrow, ncol, nnz, indptr, indices,
                 col_deg, col_ptr, col_indices, row_degrees,
                 max_panels=3000):
    """运行一种策略，返回统计"""
    TILE_R = 16
    col_order = np.argsort(col_deg)[::-1]
    assigned = np.zeros(nrow, dtype=bool)
    
    total_U = 0
    total_nnz_amx = 0
    total_panels = 0
    panel_U_list = []
    panel_nnz_list = []
    panel_fill_list = []
    
    t0 = time.time()
    
    for ci in range(min(ncol, len(col_order))):
        c = col_order[ci]
        if col_deg[c] == 0:
            break
        
        rows_with_c = col_indices[col_ptr[c]:col_ptr[c+1]]
        avail = rows_with_c[~assigned[rows_with_c]]
        if len(avail) < 2:
            continue
        
        # 初始 union 包含 anchor column
        initial_union = {int(c)}
        
        # 用指定策略选行
        selected = strategy_fn(avail, row_degrees, indptr, indices,
                               initial_union, TILE_R)
        
        # 计算 panel 统计
        panel_cols = set()
        panel_nnz = 0
        for r in selected:
            cols_r = indices[indptr[r]:indptr[r+1]]
            panel_cols.update(cols_r)
            panel_nnz += len(cols_r)
        
        U = len(panel_cols)
        n_tiles = (U + 31) // 32
        fill = panel_nnz / (len(selected) * 32 * n_tiles) if n_tiles > 0 else 0
        
        total_U += U
        total_nnz_amx += panel_nnz
        total_panels += 1
        panel_U_list.append(U)
        panel_nnz_list.append(panel_nnz)
        panel_fill_list.append(fill)
        
        for r in selected:
            assigned[r] = True
        
        if total_panels >= max_panels:
            break
    
    elapsed = time.time() - t0
    
    fb_nnz = 0
    for r in range(nrow):
        if not assigned[r]:
            fb_nnz += (indptr[r+1] - indptr[r])
    
    amx_coverage = total_nnz_amx / nnz * 100
    fb_pct = fb_nnz / nnz * 100
    global_reuse = total_nnz_amx / total_U if total_U > 0 else 0
    avg_fill = np.mean(panel_fill_list) if panel_fill_list else 0
    avg_U = np.mean(panel_U_list) if panel_U_list else 0
    
    return {
        'name': name,
        'panels': total_panels,
        'amx_nnz': total_nnz_amx,
        'amx_coverage': amx_coverage,
        'fb_nnz': fb_nnz,
        'fb_pct': fb_pct,
        'total_U': total_U,
        'avg_U': avg_U,
        'global_reuse': global_reuse,
        'avg_fill': avg_fill,
        'elapsed': elapsed,
        'panel_U': np.array(panel_U_list),
        'panel_nnz': np.array(panel_nnz_list),
    }

# ============================================================
# 主程序
# ============================================================
if __name__ == '__main__':
    DATA_DIR = os.path.expanduser('~/data/SpMM_project/data')
    
    matrices = [
        'web-Google', 'amazon0601', 'soc-Pokec',
        'as-Skitter', 'cit-Patents', 'indochina-2004', 'hollywood-2009'
    ]
    
    print("=" * 85)
    print("Row Selection Strategy Comparison: Low-Degree vs High-Degree vs Marginal NNZ/U")
    print("=" * 85)
    
    all_results = []
    
    for mname in matrices:
        csrbin = os.path.join(DATA_DIR, mname, f'{mname}.csrbin')
        if not os.path.exists(csrbin):
            continue
        
        print(f"\n{'#'*65}")
        print(f"# {mname}")
        print(f"{'#'*65}")
        
        nrow, ncol, nnz, indptr, indices = read_csrbin(csrbin)
        avg_deg = nnz / nrow
        row_degrees = np.diff(indptr).astype(np.int64)
        print(f"M={nrow:,}  N={ncol:,}  NNZ={nnz:,}  avg_deg={avg_deg:.1f}")
        
        print("Building CSC...")
        col_deg, col_ptr, col_indices = build_csc(nrow, ncol, nnz, indptr, indices)
        
        strategies = [
            ("Config-A (low deg)", strategy_low_degree),
            ("Config-B (high deg)", strategy_high_degree),
            ("Marginal NNZ/U", strategy_marginal),
        ]
        
        results = []
        for sname, sfn in strategies:
            print(f"  Running {sname}...")
            r = run_strategy(sname, sfn, nrow, ncol, nnz, indptr, indices,
                           col_deg, col_ptr, col_indices, row_degrees,
                           max_panels=3000)
            results.append(r)
        
        # 打印对比表
        print(f"\n  {'Strategy':<22} {'Panels':>7} {'AMX_NNZ%':>9} {'FB_NNZ%':>8} "
              f"{'ΣU':>10} {'avg_U':>7} {'Reuse':>7} {'Fill%':>7} {'Time':>6}")
        print(f"  {'-'*87}")
        for r in results:
            print(f"  {r['name']:<22} {r['panels']:>7} {r['amx_coverage']:>8.1f}% "
                  f"{r['fb_pct']:>7.1f}% {r['total_U']:>10,} {r['avg_U']:>7.0f} "
                  f"{r['global_reuse']:>6.2f}x {r['avg_fill']*100:>6.1f}% "
                  f"{r['elapsed']:>5.1f}s")
        
        # 关键对比：Marginal vs 两个 baseline 的改善
        m = results[2]  # Marginal
        a = results[0]  # Config A
        b = results[1]  # Config B
        
        print(f"\n  Marginal vs Config-A:")
        print(f"    AMX coverage: {a['amx_coverage']:.1f}% → {m['amx_coverage']:.1f}% "
              f"({m['amx_coverage']-a['amx_coverage']:+.1f}%)")
        print(f"    ΣU (gather):  {a['total_U']:,} → {m['total_U']:,} "
              f"({(m['total_U']/a['total_U']-1)*100:+.1f}%)")
        print(f"    Global reuse: {a['global_reuse']:.2f}x → {m['global_reuse']:.2f}x")
        
        print(f"\n  Marginal vs Config-B:")
        print(f"    AMX coverage: {b['amx_coverage']:.1f}% → {m['amx_coverage']:.1f}% "
              f"({m['amx_coverage']-b['amx_coverage']:+.1f}%)")
        print(f"    ΣU (gather):  {b['total_U']:,} → {m['total_U']:,} "
              f"({(m['total_U']/b['total_U']-1)*100:+.1f}%)")
        print(f"    Global reuse: {b['global_reuse']:.2f}x → {m['global_reuse']:.2f}x")
        
        # U 分布对比
        print(f"\n  U distribution:")
        for r in results:
            U = r['panel_U']
            print(f"    {r['name']:<22}: mean={U.mean():.0f} median={np.median(U):.0f} "
                  f"P95={np.percentile(U,95):.0f} max={U.max()}")
        
        all_results.append((mname, results))
    
    # 全局汇总
    print(f"\n{'='*85}")
    print("GLOBAL SUMMARY")
    print(f"{'='*85}")
    print(f"{'Matrix':<16} {'Config-A':>10} {'Config-B':>10} {'Marginal':>10} "
          f"{'M vs A':>10} {'M vs B':>10}")
    print(f"{'':16} {'cov%':>10} {'cov%':>10} {'cov%':>10} "
          f"{'Δcov':>10} {'Δcov':>10}")
    print("-" * 68)
    for mname, results in all_results:
        a, b, m = results[0], results[1], results[2]
        print(f"{mname:<16} {a['amx_coverage']:>9.1f}% {b['amx_coverage']:>9.1f}% "
              f"{m['amx_coverage']:>9.1f}% "
              f"{m['amx_coverage']-a['amx_coverage']:>+9.1f}% "
              f"{m['amx_coverage']-b['amx_coverage']:>+9.1f}%")
    
    print(f"\n{'Matrix':<16} {'Config-A':>10} {'Config-B':>10} {'Marginal':>10} "
          f"{'M vs A':>10} {'M vs B':>10}")
    print(f"{'':16} {'reuse':>10} {'reuse':>10} {'reuse':>10} "
          f"{'Δreuse':>10} {'Δreuse':>10}")
    print("-" * 68)
    for mname, results in all_results:
        a, b, m = results[0], results[1], results[2]
        print(f"{mname:<16} {a['global_reuse']:>9.2f}x {b['global_reuse']:>9.2f}x "
              f"{m['global_reuse']:>9.2f}x "
              f"{m['global_reuse']-a['global_reuse']:>+9.2f}x "
              f"{m['global_reuse']-b['global_reuse']:>+9.2f}x")
