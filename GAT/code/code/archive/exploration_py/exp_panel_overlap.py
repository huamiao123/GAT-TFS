#!/usr/bin/env python3
"""
Inter-Panel B-Reuse 验证
目的：按 anchor_col 排序后，相邻 panel 的 column union 交集有多大？

如果交集 >30% → inter-panel B-reuse 调度值得做
如果交集 <10% → 不值得，放弃这个方向
"""

import struct
import numpy as np
import os

def read_csrbin(path):
    with open(path, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('III', f.read(12))
        nrow, ncol, nnz = struct.unpack('QQQ', f.read(24))
        indptr = np.frombuffer(f.read((nrow+1)*4), dtype=np.uint32)
        indices = np.frombuffer(f.read(nnz*4), dtype=np.uint32)
    return nrow, ncol, nnz, indptr, indices

def build_panels(nrow, ncol, nnz, indptr, indices, max_panels=5000):
    """模拟列锚点分组，返回 panel 列表（含 anchor_col 和 column set）"""
    TILE_R = 16
    col_deg = np.bincount(indices, minlength=ncol)
    col_order = np.argsort(col_deg)[::-1]
    row_degrees = np.diff(indptr).astype(np.int64)
    assigned = np.zeros(nrow, dtype=bool)
    
    # 构建 CSC
    col_ptr = np.zeros(ncol + 1, dtype=np.int64)
    col_ptr[1:] = np.cumsum(col_deg)
    col_indices = np.empty(nnz, dtype=np.int64)
    col_pos = col_ptr[:-1].copy()
    for i in range(nrow):
        for idx in range(indptr[i], indptr[i+1]):
            j = indices[idx]
            col_indices[col_pos[j]] = i
            col_pos[j] += 1
    
    panels = []  # list of (anchor_col, column_set, n_nnz)
    
    for ci in range(min(ncol, len(col_order))):
        c = col_order[ci]
        if col_deg[c] == 0:
            break
        rows_with_c = col_indices[col_ptr[c]:col_ptr[c+1]]
        avail = rows_with_c[~assigned[rows_with_c]]
        if len(avail) < 2:
            continue
        
        if len(avail) > TILE_R:
            k = min(TILE_R, len(avail)) - 1
            part_idx = np.argpartition(row_degrees[avail], k)[:TILE_R]
            selected = avail[part_idx]
        else:
            selected = avail
        
        # 收集 column union
        col_set = set()
        panel_nnz = 0
        for r in selected:
            cols_r = indices[indptr[r]:indptr[r+1]]
            col_set.update(cols_r)
            panel_nnz += len(cols_r)
        
        panels.append({
            'anchor': int(c),
            'cols': col_set,
            'U': len(col_set),
            'nnz': panel_nnz,
        })
        
        assigned[selected] = True
        if len(panels) >= max_panels:
            break
    
    return panels

def analyze_overlap(panels, label="original"):
    """分析相邻 panel 之间的 column union 交集"""
    n = len(panels)
    if n < 2:
        return
    
    overlaps = []       # 交集占比 = |intersection| / min(|U1|, |U2|)
    overlaps_union = [] # 交集占联合 = |intersection| / |union|
    abs_overlaps = []   # 绝对交集大小
    
    for i in range(n - 1):
        c1 = panels[i]['cols']
        c2 = panels[i+1]['cols']
        inter = len(c1 & c2)
        union = len(c1 | c2)
        min_u = min(len(c1), len(c2))
        
        if min_u > 0:
            overlaps.append(inter / min_u)
        if union > 0:
            overlaps_union.append(inter / union)
        abs_overlaps.append(inter)
    
    ov = np.array(overlaps)
    ov_u = np.array(overlaps_union)
    ab = np.array(abs_overlaps)
    
    print(f"\n  [{label}] Adjacent panel overlap ({n-1} pairs):")
    print(f"    |intersection| / min(|U1|,|U2|):")
    print(f"      mean={ov.mean():.1%}  median={np.median(ov):.1%}  "
          f"P25={np.percentile(ov,25):.1%}  P75={np.percentile(ov,75):.1%}")
    print(f"    |intersection| / |union| (Jaccard):")
    print(f"      mean={ov_u.mean():.1%}  median={np.median(ov_u):.1%}")
    print(f"    |intersection| absolute:")
    print(f"      mean={ab.mean():.1f}  median={np.median(ab):.0f}  "
          f"max={ab.max()}")
    print(f"    Pairs with overlap >30%: {np.mean(ov>0.3):.1%}")
    print(f"    Pairs with overlap >10%: {np.mean(ov>0.1):.1%}")
    print(f"    Pairs with overlap =0%:  {np.mean(ov==0):.1%}")
    
    # 估算 B-reuse 收益
    # 如果相邻 panel 共享 B 行还在 L2 中
    # 节省的 gather 量 = Σ |intersection| × K × BF16
    K = 128
    BF16 = 2
    saved_bytes = ab.sum() * K * BF16
    total_bytes = sum(p['U'] for p in panels) * K * BF16
    print(f"    B gather saved if L2 hit: {saved_bytes/1e6:.1f} MB / "
          f"{total_bytes/1e6:.1f} MB = {saved_bytes/total_bytes:.1%}")
    
    return ov

def analyze_window_overlap(panels, window_sizes=[2, 4, 8, 16]):
    """分析窗口内的 B-reuse（模拟同一线程连续执行多个 panel）"""
    n = len(panels)
    
    print(f"\n  Window overlap (simulating thread-local execution):")
    print(f"  {'Window':>8} {'Unique B rows':>14} {'vs no-reuse':>12} {'Reuse':>8}")
    print(f"  {'-'*46}")
    
    for ws in window_sizes:
        total_unique = 0
        total_no_reuse = 0
        n_windows = 0
        
        for start in range(0, n - ws + 1, ws):
            window_panels = panels[start:start+ws]
            # 窗口内所有 panel 的 column union
            window_union = set()
            window_sum_U = 0
            for p in window_panels:
                window_union.update(p['cols'])
                window_sum_U += p['U']
            
            total_unique += len(window_union)
            total_no_reuse += window_sum_U
            n_windows += 1
        
        if total_no_reuse > 0:
            reuse = total_no_reuse / total_unique
            saving = 1 - total_unique / total_no_reuse
            print(f"  {ws:>8} {total_unique:>14,} {total_no_reuse:>12,} "
                  f"{reuse:>7.2f}x ({saving:.1%} saved)")

# ============================================================
# 主程序
# ============================================================
if __name__ == '__main__':
    DATA_DIR = os.path.expanduser('~/data/SpMM_project/data')
    
    matrices = [
        'web-Google', 'amazon0601', 'soc-Pokec',
        'as-Skitter', 'cit-Patents', 'indochina-2004', 'hollywood-2009'
    ]
    
    print("=" * 70)
    print("Inter-Panel B-Reuse Verification")
    print("=" * 70)
    
    summary = []
    
    for mname in matrices:
        csrbin = os.path.join(DATA_DIR, mname, f'{mname}.csrbin')
        if not os.path.exists(csrbin):
            print(f"SKIP: {mname}")
            continue
        
        print(f"\n{'#'*60}")
        print(f"# {mname}")
        print(f"{'#'*60}")
        
        nrow, ncol, nnz, indptr, indices = read_csrbin(csrbin)
        avg_deg = nnz / nrow
        print(f"M={nrow:,}  N={ncol:,}  NNZ={nnz:,}  avg_deg={avg_deg:.1f}")
        
        print("Building panels...")
        panels = build_panels(nrow, ncol, nnz, indptr, indices)
        print(f"Panels: {len(panels)}")
        
        # A. 原始顺序（列锚点构建顺序 = 按列度数降序）
        ov_orig = analyze_overlap(panels, "Original order (by col degree)")
        analyze_window_overlap(panels, [2, 4, 8, 16, 32])
        
        # B. 按 anchor_col 排序
        sorted_panels = sorted(panels, key=lambda p: p['anchor'])
        ov_sorted = analyze_overlap(sorted_panels, "Sorted by anchor_col")
        analyze_window_overlap(sorted_panels, [2, 4, 8, 16, 32])
        
        # C. 随机顺序（对照组）
        rng = np.random.RandomState(42)
        rand_idx = rng.permutation(len(panels))
        rand_panels = [panels[i] for i in rand_idx]
        ov_rand = analyze_overlap(rand_panels, "Random order")
        
        summary.append({
            'matrix': mname,
            'orig_mean': np.mean(ov_orig) if ov_orig is not None else 0,
            'sorted_mean': np.mean(ov_sorted) if ov_sorted is not None else 0,
            'rand_mean': np.mean(ov_rand) if ov_rand is not None else 0,
        })
    
    # 汇总
    print(f"\n{'='*70}")
    print("SUMMARY: Adjacent panel overlap (mean)")
    print(f"{'='*70}")
    print(f"{'Matrix':<18} {'Original':>10} {'Sorted':>10} {'Random':>10} {'Sorted/Rand':>12}")
    print("-" * 62)
    for s in summary:
        ratio = s['sorted_mean']/s['rand_mean'] if s['rand_mean'] > 0 else 0
        print(f"{s['matrix']:<18} {s['orig_mean']:>9.1%} {s['sorted_mean']:>9.1%} "
              f"{s['rand_mean']:>9.1%} {ratio:>11.1f}x")
    
    print(f"\n决策标准:")
    print(f"  Sorted mean > 30% → 值得做 inter-panel B-reuse 调度")
    print(f"  Sorted mean 10-30% → 可能值得，但收益有限")
    print(f"  Sorted mean < 10% → 不值得")
