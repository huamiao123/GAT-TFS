#!/usr/bin/env python3
"""
Column-Cluster Splitting Simulation
=====================================
对列并集 U 过大的 panel，按列共现关系切分成多个子 panel
验证切分后 fill rate 和 tile 数的改善
"""

import numpy as np
import struct
import sys
import os
import time
from collections import defaultdict

def read_csrbin(filepath):
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

def generate_panels(nrow, ncol, indptr, indices, col_deg, row_deg,
                    panel_size=16, min_col_freq=16):
    col_ptr, row_indices = build_csc(nrow, ncol, int(indptr[-1]), indptr, indices)
    col_order = np.argsort(-col_deg)
    assigned = np.zeros(nrow, dtype=bool)
    panels = []
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
        candidates.sort(key=lambda r: row_deg[r])
        selected = candidates[:panel_size]
        row_colsets = {}
        col_union = set()
        total_deg = 0
        for r in selected:
            cols = set(indices[indptr[r]:indptr[r + 1]].tolist())
            row_colsets[r] = cols
            col_union |= cols
            total_deg += len(cols)
        U = len(col_union)
        ntiles = (U + 31) // 32
        fill = total_deg / (panel_size * 32 * ntiles) if ntiles > 0 else 0
        panels.append({
            'rows': selected,
            'row_colsets': row_colsets,
            'col_union': col_union,
            'U': U,
            'ntiles': ntiles,
            'total_deg': total_deg,
            'fill': fill,
        })
        for r in selected:
            assigned[r] = True
    return panels, assigned

def cluster_columns_greedy(panel, max_cluster_size=32):
    rows = panel['rows']
    row_colsets = panel['row_colsets']
    col_union = panel['col_union']
    col_to_rows = defaultdict(set)
    for r in rows:
        for c in row_colsets[r]:
            col_to_rows[c].add(r)
    remaining = set(col_union)
    clusters = []
    while remaining:
        seed = max(remaining, key=lambda c: len(col_to_rows[c]))
        cluster = {seed}
        remaining.remove(seed)
        cluster_rows = set(col_to_rows[seed])
        while len(cluster) < max_cluster_size and remaining:
            best_col = None
            best_score = -1
            if len(remaining) > 500:
                candidates = sorted(remaining,
                                   key=lambda c: len(col_to_rows[c]),
                                   reverse=True)[:500]
            else:
                candidates = list(remaining)
            for c in candidates:
                score = len(col_to_rows[c] & cluster_rows)
                if score > best_score:
                    best_score = score
                    best_col = c
            if best_col is None or best_score == 0:
                break
            cluster.add(best_col)
            remaining.remove(best_col)
            cluster_rows |= col_to_rows[best_col]
        clusters.append(cluster)
    return clusters, col_to_rows

def analyze_split_panel(panel, clusters, col_to_rows, tile_width=32):
    rows = panel['rows']
    row_colsets = panel['row_colsets']
    sub_panels = []
    total_nnz_covered = 0
    total_tiles = 0
    total_useful_cells = 0
    total_tile_cells = 0
    for ci, cluster in enumerate(clusters):
        active_rows = []
        sub_nnz = 0
        for r in rows:
            nnz_in_cluster = len(row_colsets[r] & cluster)
            if nnz_in_cluster > 0:
                active_rows.append(r)
                sub_nnz += nnz_in_cluster
        n_active = len(active_rows)
        sub_U = len(cluster)
        sub_tiles = (sub_U + tile_width - 1) // tile_width
        if n_active > 0 and sub_tiles > 0:
            sub_fill = sub_nnz / (n_active * tile_width * sub_tiles)
        else:
            sub_fill = 0
        total_nnz_covered += sub_nnz
        total_tiles += sub_tiles
        total_useful_cells += sub_nnz
        total_tile_cells += n_active * tile_width * sub_tiles
        sub_panels.append({
            'cluster_id': ci,
            'cluster_size': sub_U,
            'active_rows': n_active,
            'sub_nnz': sub_nnz,
            'sub_tiles': sub_tiles,
            'sub_fill': sub_fill,
        })
    global_fill = total_useful_cells / total_tile_cells if total_tile_cells > 0 else 0
    return sub_panels, {
        'num_clusters': len(clusters),
        'total_tiles': total_tiles,
        'total_nnz_covered': total_nnz_covered,
        'global_fill': global_fill,
    }

def estimate_efficiency(panel, split_summary, sub_panels, K=128):
    orig_tiles = panel['ntiles']
    orig_rows = len(panel['rows'])
    orig_gather_units = orig_rows * orig_tiles
    orig_fill = panel['fill']
    split_gather_units = sum(sp['active_rows'] * sp['sub_tiles'] for sp in sub_panels)
    split_tiles = split_summary['total_tiles']
    split_fill = split_summary['global_fill']
    return {
        'orig_tiles': orig_tiles,
        'orig_gather': orig_gather_units,
        'orig_fill': orig_fill,
        'split_tiles': split_tiles,
        'split_gather': split_gather_units,
        'split_fill': split_fill,
        'tile_reduction': orig_tiles / split_tiles if split_tiles > 0 else 0,
        'fill_improvement': split_fill / orig_fill if orig_fill > 0 else 0,
        'gather_ratio': split_gather_units / orig_gather_units if orig_gather_units > 0 else 0,
    }

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 cluster_split_sim.py matrix.csrbin [U_threshold] [cluster_size]")
        print("  U_threshold: only split panels with U > this (default: 128)")
        print("  cluster_size: max columns per cluster (default: 32)")
        sys.exit(1)
    filepath = sys.argv[1]
    U_threshold = int(sys.argv[2]) if len(sys.argv) > 2 else 128
    max_cluster_size = int(sys.argv[3]) if len(sys.argv) > 3 else 32
    name = os.path.basename(filepath).replace('.csrbin', '')
    print(f"{'='*70}")
    print(f" Column-Cluster Splitting Simulation")
    print(f" Matrix: {name}")
    print(f" U threshold: {U_threshold}")
    print(f" Max cluster size: {max_cluster_size}")
    print(f"{'='*70}")
    t0 = time.time()
    nrow, ncol, nnz, indptr, indices = read_csrbin(filepath)
    print(f"\nRead: {nrow:,} rows, {nnz:,} NNZ ({time.time()-t0:.2f}s)")
    row_deg = np.diff(indptr).astype(np.int64)
    col_deg = np.zeros(ncol, dtype=np.int64)
    np.add.at(col_deg, indices.astype(np.int64), 1)
    t0 = time.time()
    panels, assigned = generate_panels(nrow, ncol, indptr, indices, col_deg, row_deg)
    print(f"Generated {len(panels)} panels ({time.time()-t0:.2f}s)")
    small_panels = [p for p in panels if p['U'] <= U_threshold]
    large_panels = [p for p in panels if p['U'] > U_threshold]
    print(f"\n--- Panel Distribution ---")
    print(f"  U <= {U_threshold} (keep): {len(small_panels)} panels")
    print(f"  U >  {U_threshold} (split): {len(large_panels)} panels")
    if not large_panels:
        print(f"\nNo panels with U > {U_threshold}, nothing to split!")
        return
    large_Us = [p['U'] for p in large_panels]
    large_fills = [p['fill'] for p in large_panels]
    large_tiles = [p['ntiles'] for p in large_panels]
    large_nnz = sum(p['total_deg'] for p in large_panels)
    print(f"\n--- Large Panel Stats (U > {U_threshold}) ---")
    print(f"  Count:      {len(large_panels)}")
    print(f"  Total NNZ:  {large_nnz:,}")
    print(f"  Mean U:     {np.mean(large_Us):.1f}")
    print(f"  Median U:   {np.median(large_Us):.0f}")
    print(f"  Max U:      {max(large_Us)}")
    print(f"  Mean fill:  {np.mean(large_fills)*100:.1f}%")
    print(f"  Mean tiles: {np.mean(large_tiles):.1f}")
    sample_size = min(200, len(large_panels))
    large_panels_sorted = sorted(large_panels, key=lambda p: p['U'], reverse=True)
    sample_panels = large_panels_sorted[:sample_size]
    print(f"\n{'='*70}")
    print(f" Cluster Splitting Simulation (top {sample_size} largest panels)")
    print(f"{'='*70}")
    all_orig_fills = []
    all_split_fills = []
    all_orig_tiles = []
    all_split_tiles = []
    all_fill_improvements = []
    all_tile_reductions = []
    all_gather_ratios = []
    all_num_clusters = []
    detail_count = 5
    for pi, panel in enumerate(sample_panels):
        t0 = time.time()
        clusters, col_to_rows = cluster_columns_greedy(panel, max_cluster_size)
        sub_panels, split_summary = analyze_split_panel(panel, clusters, col_to_rows)
        eff = estimate_efficiency(panel, split_summary, sub_panels)
        elapsed = time.time() - t0
        all_orig_fills.append(panel['fill'])
        all_split_fills.append(split_summary['global_fill'])
        all_orig_tiles.append(panel['ntiles'])
        all_split_tiles.append(split_summary['total_tiles'])
        all_fill_improvements.append(eff['fill_improvement'])
        all_tile_reductions.append(eff['tile_reduction'])
        all_gather_ratios.append(eff['gather_ratio'])
        all_num_clusters.append(split_summary['num_clusters'])
        if pi < detail_count:
            print(f"\n--- Panel #{pi} (U={panel['U']}, {elapsed:.2f}s) ---")
            print(f"  Original: {panel['ntiles']} tiles, fill={panel['fill']*100:.1f}%")
            print(f"  Split:    {split_summary['num_clusters']} clusters, "
                  f"{split_summary['total_tiles']} tiles, "
                  f"fill={split_summary['global_fill']*100:.1f}%")
            print(f"  Improve:  fill {eff['fill_improvement']:.2f}x, "
                  f"tiles {eff['orig_tiles']}->{eff['split_tiles']} "
                  f"({eff['tile_reduction']:.2f}x reduction)")
            print(f"  Gather:   {eff['gather_ratio']:.2f}x "
                  f"({'MORE' if eff['gather_ratio'] > 1 else 'LESS'})")
            print(f"  Cluster details:")
            sorted_subs = sorted(sub_panels, key=lambda s: s['sub_nnz'], reverse=True)
            for si, sp in enumerate(sorted_subs[:8]):
                bar = '#' * min(50, int(sp['sub_fill'] * 50))
                print(f"    C{si}: {sp['cluster_size']:>3}col x "
                      f"{sp['active_rows']:>2}row, "
                      f"{sp['sub_tiles']}tile, "
                      f"fill={sp['sub_fill']*100:>5.1f}% "
                      f"{bar}")
            if len(sorted_subs) > 8:
                print(f"    ... +{len(sorted_subs)-8} more clusters")
    print(f"\n\n{'='*70}")
    print(f" Summary ({sample_size} large panels)")
    print(f"{'='*70}")
    print(f"\n--- Fill Rate ---")
    print(f"  Original avg:  {np.mean(all_orig_fills)*100:.1f}%")
    print(f"  Split avg:     {np.mean(all_split_fills)*100:.1f}%")
    print(f"  Mean improve:  {np.mean(all_fill_improvements):.2f}x")
    print(f"  Median improve:{np.median(all_fill_improvements):.2f}x")
    print(f"\n--- Tile Count ---")
    print(f"  Original avg:  {np.mean(all_orig_tiles):.1f}")
    print(f"  Split avg:     {np.mean(all_split_tiles):.1f}")
    print(f"  Reduction:     {np.mean(all_tile_reductions):.2f}x")
    print(f"\n--- Gather Cost ---")
    print(f"  Gather ratio (split/orig): {np.mean(all_gather_ratios):.2f}x")
    if np.mean(all_gather_ratios) > 1.2:
        print(f"  WARNING: gather increases! Need real benchmark to verify net gain")
    else:
        print(f"  OK: gather decreases or stays flat")
    print(f"\n--- Cluster Count ---")
    print(f"  Mean clusters: {np.mean(all_num_clusters):.1f}")
    print(f"  Median:        {np.median(all_num_clusters):.0f}")
    print(f"\n\n{'='*70}")
    print(f" By U Range")
    print(f"{'='*70}")
    u_ranges = [(128, 256), (256, 512), (512, 1024), (1024, 2048), (2048, 99999)]
    for u_lo, u_hi in u_ranges:
        mask = [(p['U'] > u_lo and p['U'] <= u_hi) for p in sample_panels]
        count = sum(mask)
        if count == 0:
            continue
        fills_orig = [all_orig_fills[i] for i in range(len(mask)) if mask[i]]
        fills_split = [all_split_fills[i] for i in range(len(mask)) if mask[i]]
        tiles_orig = [all_orig_tiles[i] for i in range(len(mask)) if mask[i]]
        tiles_split = [all_split_tiles[i] for i in range(len(mask)) if mask[i]]
        fill_imps = [all_fill_improvements[i] for i in range(len(mask)) if mask[i]]
        u_label = f"U={u_lo+1}~{u_hi}" if u_hi < 99999 else f"U>{u_lo}"
        print(f"\n  {u_label} ({count} panels):")
        print(f"    fill: {np.mean(fills_orig)*100:.1f}% -> {np.mean(fills_split)*100:.1f}%  "
              f"({np.mean(fill_imps):.2f}x)")
        print(f"    tiles: {np.mean(tiles_orig):.1f} -> {np.mean(tiles_split):.1f}")
    avg_fill_imp = np.mean(all_fill_improvements)
    avg_gather_ratio = np.mean(all_gather_ratios)
    avg_tile_red = np.mean(all_tile_reductions)
    print(f"\n\n{'='*70}")
    print(f" Conclusion")
    print(f"{'='*70}")
    if avg_fill_imp > 1.5 and avg_gather_ratio < 1.5:
        print(f"\n  POSITIVE: Column-cluster splitting is effective!")
        print(f"    fill +{avg_fill_imp:.2f}x, tiles -{avg_tile_red:.2f}x")
        if avg_gather_ratio > 1.0:
            print(f"    Note: gather +{avg_gather_ratio:.2f}x, needs real benchmark")
        else:
            print(f"    Gather also reduced ({avg_gather_ratio:.2f}x), all-around win!")
    elif avg_fill_imp > 1.2:
        print(f"\n  MODERATE: Some improvement, consider larger cluster_size")
        print(f"    fill +{avg_fill_imp:.2f}x, gather {avg_gather_ratio:.2f}x")
    else:
        print(f"\n  WEAK: Limited improvement, need different strategy")
    print(f"\nDone!")

if __name__ == '__main__':
    main()
