#!/usr/bin/env python3
"""
Phase 12.3: Multi-direction exploration
  Direction 1: Post-split (build panels first, then split large ones)
  Direction 2: Improved clustering params (relax high-deg cutoff)
  Direction 3: Hybrid + Post-split combination
"""

import numpy as np
import struct
import time, sys, os
from collections import defaultdict

# ============================================================
# Utility: read CSR binary (correct format)
# ============================================================
def read_csrbin(filename):
    t0 = time.time()
    with open(filename, 'rb') as f:
        ptype = struct.unpack('I', f.read(4))[0]
        dtype = struct.unpack('I', f.read(4))[0]
        vtype = struct.unpack('I', f.read(4))[0]
        nrow  = struct.unpack('Q', f.read(8))[0]
        ncol  = struct.unpack('Q', f.read(8))[0]
        nnz   = struct.unpack('Q', f.read(8))[0]
        indptr  = np.frombuffer(f.read(4*(nrow+1)), dtype=np.uint32)
        indices = np.frombuffer(f.read(4*nnz),      dtype=np.uint32)
    print(f"  Read: {nrow:,} rows, {ncol:,} cols, {nnz:,} NNZ ({time.time()-t0:.2f}s)")
    return nrow, ncol, nnz, indptr, indices

# ============================================================
# Build CSC from CSR
# ============================================================
def build_csc(rows, cols, indptr, indices):
    col_counts = np.zeros(cols, dtype=np.int32)
    for j in indices:
        col_counts[j] += 1
    csc_indptr = np.zeros(cols+1, dtype=np.int32)
    csc_indptr[1:] = np.cumsum(col_counts)
    csc_indices = np.empty(len(indices), dtype=np.int32)
    pos = csc_indptr[:-1].copy()
    for r in range(rows):
        for idx in range(indptr[r], indptr[r+1]):
            c = indices[idx]
            csc_indices[pos[c]] = r
            pos[c] += 1
    return csc_indptr, csc_indices

# ============================================================
# Column-Anchor panel building
# ============================================================
def build_anchor_panels(rows, cols, indptr, indices, csc_indptr, csc_indices, panel_h=16):
    row_deg = np.diff(indptr)
    col_deg = np.diff(csc_indptr)
    col_order = np.argsort(-col_deg)
    assigned = np.zeros(rows, dtype=bool)
    panels = []

    for anchor in col_order:
        if col_deg[anchor] == 0:
            break
        cand_rows = []
        for idx in range(csc_indptr[anchor], csc_indptr[anchor+1]):
            r = csc_indices[idx]
            if not assigned[r]:
                cand_rows.append(r)
        if len(cand_rows) < panel_h:
            continue
        cand_rows.sort(key=lambda r: row_deg[r])
        selected = cand_rows[:panel_h]
        col_set = set()
        total_nnz = 0
        for r in selected:
            for idx in range(indptr[r], indptr[r+1]):
                col_set.add(indices[idx])
            total_nnz += indptr[r+1] - indptr[r]
        for r in selected:
            assigned[r] = True
        panels.append((selected, col_set, total_nnz))

    return panels

# ============================================================
# Direction 1: Post-split
# ============================================================
def local_col_clustering(selected_rows, indptr, indices, max_cluster=32):
    col_to_rows = defaultdict(set)
    for r in selected_rows:
        for idx in range(indptr[r], indptr[r+1]):
            c = indices[idx]
            col_to_rows[c].add(r)

    all_cols = list(col_to_rows.keys())
    if len(all_cols) <= max_cluster:
        return [set(all_cols)]

    cooccur = defaultdict(int)
    for r in selected_rows:
        cols_in_row = []
        for idx in range(indptr[r], indptr[r+1]):
            c = indices[idx]
            if c in col_to_rows:
                cols_in_row.append(c)
        if len(cols_in_row) > 500:
            np.random.seed(r % 10000)
            cols_in_row = list(np.random.choice(cols_in_row, 500, replace=False))
        for i in range(len(cols_in_row)):
            for j in range(i+1, len(cols_in_row)):
                a, b = cols_in_row[i], cols_in_row[j]
                if a > b:
                    a, b = b, a
                cooccur[(a, b)] += 1

    col_neighbors = defaultdict(list)
    for (a, b), w in cooccur.items():
        col_neighbors[a].append((b, w))
        col_neighbors[b].append((a, w))

    col_local_deg = {c: len(col_to_rows[c]) for c in all_cols}
    unassigned = set(all_cols)
    clusters = []
    sorted_cols = sorted(all_cols, key=lambda c: col_local_deg[c], reverse=True)

    for seed in sorted_cols:
        if seed not in unassigned:
            continue
        cluster = {seed}
        unassigned.remove(seed)
        while len(cluster) < max_cluster:
            best_col = None
            best_score = 0
            for c in cluster:
                for nb, w in col_neighbors.get(c, []):
                    if nb in unassigned and w > best_score:
                        best_score = w
                        best_col = nb
            if best_col is None or best_score == 0:
                break
            cluster.add(best_col)
            unassigned.remove(best_col)
        clusters.append(cluster)

    for c in unassigned:
        clusters.append({c})
    return clusters

def post_split_panel(panel_rows, panel_cols, panel_nnz, indptr, indices, max_U=32):
    U = len(panel_cols)
    if U <= max_U:
        return [(panel_rows, panel_cols, panel_nnz)]

    col_clusters = local_col_clustering(panel_rows, indptr, indices, max_cluster=max_U)
    sub_panels = []
    for cc in col_clusters:
        if len(cc) == 0:
            continue
        cc_set = cc if isinstance(cc, set) else set(cc)
        rows_nnz = []
        for r in panel_rows:
            nnz_in_cluster = 0
            for idx in range(indptr[r], indptr[r+1]):
                if indices[idx] in cc_set:
                    nnz_in_cluster += 1
            if nnz_in_cluster > 0:
                rows_nnz.append((r, nnz_in_cluster))
        if len(rows_nnz) == 0:
            continue
        sub_rows = [r for r, _ in rows_nnz]
        sub_nnz = sum(n for _, n in rows_nnz)
        sub_panels.append((sub_rows, cc_set, sub_nnz))
    return sub_panels

# ============================================================
# Direction 2: Improved A^T*A clustering
# ============================================================
def ata_clustering_improved(rows, cols, indptr, indices,
                            max_row_deg=1000, sample_rate=0.5,
                            min_cooccur=2, max_cluster=32):
    row_deg = np.diff(indptr)
    np.random.seed(42)
    cooccur = defaultdict(int)
    n_processed = 0
    for r in range(rows):
        deg = int(row_deg[r])
        if deg < 2 or deg > max_row_deg:
            continue
        if np.random.random() > sample_rate:
            continue
        n_processed += 1
        row_cols = indices[indptr[r]:indptr[r+1]]
        if deg > 300:
            row_cols = np.random.choice(row_cols, 300, replace=False)
        for i in range(len(row_cols)):
            for j in range(i+1, len(row_cols)):
                a, b = int(row_cols[i]), int(row_cols[j])
                if a > b:
                    a, b = b, a
                cooccur[(a, b)] += 1
        if n_processed % 50000 == 0:
            print(f"    ... {n_processed} rows, {len(cooccur)} pairs", flush=True)

    print(f"    Processed {n_processed} rows, {len(cooccur)} pairs", flush=True)
    filtered = {k: v for k, v in cooccur.items() if v >= min_cooccur}
    print(f"    After filter (>={min_cooccur}): {len(filtered)} pairs", flush=True)
    del cooccur

    col_neighbors = defaultdict(list)
    for (a, b), w in filtered.items():
        col_neighbors[a].append((b, w))
        col_neighbors[b].append((a, w))
    del filtered

    cols_with_neighbors = set(col_neighbors.keys())
    print(f"    Columns with neighbors: {len(cols_with_neighbors)}", flush=True)

    col_wdeg = {c: sum(w for _, w in nbs) for c, nbs in col_neighbors.items()}
    sorted_cols = sorted(cols_with_neighbors, key=lambda c: col_wdeg[c], reverse=True)
    assigned = set()
    clusters = []

    for seed in sorted_cols:
        if seed in assigned:
            continue
        cluster = [seed]
        assigned.add(seed)
        while len(cluster) < max_cluster:
            best_col = None
            best_score = 0
            for c in cluster:
                for nb, w in col_neighbors[c]:
                    if nb not in assigned and w > best_score:
                        best_score = w
                        best_col = nb
            if best_col is None or best_score < min_cooccur:
                break
            cluster.append(best_col)
            assigned.add(best_col)
        clusters.append(cluster)

    sizes = [len(c) for c in clusters]
    n_good = sum(1 for s in sizes if s >= 8)
    n_full = sum(1 for s in sizes if s == max_cluster)
    n_orphan = cols - len(assigned)
    print(f"    Clusters: {len(clusters)}, good(>=8): {n_good}, full(=32): {n_full}", flush=True)
    print(f"    Orphan columns: {n_orphan}", flush=True)
    return clusters, col_neighbors

# ============================================================
# Metrics computation
# ============================================================
def compute_metrics(panels, total_nnz, label=""):
    if len(panels) == 0:
        print(f"  [{label}] No panels")
        return {}

    n_panels = len(panels)
    total_tiles = 0
    total_gathers = 0
    total_covered_nnz = 0
    fills = []
    Us = []
    u_le32 = 0

    for (p_rows, p_cols, p_nnz) in panels:
        n_rows = len(p_rows)
        U = len(p_cols)
        tiles = (U + 31) // 32
        total_tiles += tiles
        total_gathers += U
        total_covered_nnz += p_nnz
        fill = p_nnz / (n_rows * U) if (n_rows * U) > 0 else 0
        fills.append(fill)
        Us.append(U)
        if U <= 32:
            u_le32 += 1

    avg_fill = np.mean(fills) * 100
    med_fill = np.median(fills) * 100
    avg_U = np.mean(Us)
    med_U = np.median(Us)
    pct_u32 = u_le32 / n_panels * 100
    cov_pct = total_covered_nnz / total_nnz * 100
    fallback = total_nnz - total_covered_nnz
    tile_eff = total_covered_nnz / total_tiles if total_tiles > 0 else 0
    gather_eff = total_covered_nnz / total_gathers if total_gathers > 0 else 0
    gather_bw = total_gathers * 128 * 2 / 1e6

    n_full16 = sum(1 for (r, c, n) in panels if len(r) == 16)
    n_big = sum(1 for (r, c, n) in panels if len(r) >= 8)
    avg_rows = np.mean([len(r) for r, c, n in panels])

    print(f"  [{label}]")
    print(f"    Panels:          {n_panels} (full-16: {n_full16}, rows>=8: {n_big})")
    print(f"    Avg rows/panel:  {avg_rows:.1f}")
    print(f"    NNZ covered:     {total_covered_nnz:,} ({cov_pct:.1f}%)")
    print(f"    Fallback NNZ:    {fallback:,} ({100-cov_pct:.1f}%)")
    print(f"    Total tiles:     {total_tiles:,}")
    print(f"    Total gathers:   {total_gathers:,}")
    print(f"    Tile eff:        {tile_eff:.1f} NNZ/tile")
    print(f"    Gather eff:      {gather_eff:.2f} NNZ/gather")
    print(f"    Gather BW:       {gather_bw:.1f} MB (K=128)")
    print(f"    Avg fill:        {avg_fill:.1f}%")
    print(f"    Med fill:        {med_fill:.1f}%")
    print(f"    Avg U:           {avg_U:.1f}")
    print(f"    Med U:           {med_U:.1f}")
    print(f"    U<=32:           {pct_u32:.1f}%")
    sys.stdout.flush()

    return {
        'panels': n_panels, 'full16': n_full16, 'big8': n_big,
        'avg_rows': avg_rows,
        'covered': total_covered_nnz, 'cov_pct': cov_pct,
        'fallback': fallback,
        'tiles': total_tiles, 'gathers': total_gathers,
        'tile_eff': tile_eff, 'gather_eff': gather_eff,
        'gather_bw': gather_bw,
        'avg_fill': avg_fill, 'med_fill': med_fill,
        'avg_U': avg_U, 'med_U': med_U,
        'u_le32': pct_u32
    }

# ============================================================
# Performance model
# ============================================================
def perf_model(m_base, m_test, label_base, label_test):
    if not m_base or not m_test:
        return
    r_tiles = m_test['tiles'] / m_base['tiles'] if m_base['tiles'] > 0 else 1
    r_gath = m_test['gathers'] / m_base['gathers'] if m_base['gathers'] > 0 else 1
    r_fb = m_test['fallback'] / m_base['fallback'] if m_base['fallback'] > 0 else 1

    print(f"\n  --- Performance Model: {label_test} vs {label_base} ---")
    print(f"    Tiles ratio:    {r_tiles:.3f}x")
    print(f"    Gathers ratio:  {r_gath:.3f}x")
    print(f"    Fallback ratio: {r_fb:.3f}x")

    scenarios = [
        ("Gather-dominant", 0.15, 0.60, 0.25),
        ("Balanced",        0.33, 0.34, 0.33),
        ("Fallback-heavy",  0.15, 0.30, 0.55),
    ]
    for name, wt, wg, wf in scenarios:
        T = wt * r_tiles + wg * r_gath + wf * r_fb
        pct = (1 - T) * 100
        verdict = f"{pct:.1f}% faster" if pct > 0 else f"{-pct:.1f}% slower"
        print(f"    {name:25s}  T_ratio={T:.3f}  -> {verdict}")
    sys.stdout.flush()

# ============================================================
# Main
# ============================================================
def process_matrix(name, path):
    print(f"\n{'#'*70}")
    print(f"# {name}")
    print(f"{'#'*70}")
    sys.stdout.flush()

    rows, cols, nnz, indptr, indices = read_csrbin(path)

    row_deg = np.diff(indptr.astype(np.int64))
    print(f"  Degree: mean={row_deg.mean():.1f}, med={np.median(row_deg):.0f}, "
          f"max={row_deg.max()}, P99={np.percentile(row_deg,99):.0f}")
    sys.stdout.flush()

    t0 = time.time()
    csc_indptr, csc_indices = build_csc(rows, cols, indptr, indices)
    print(f"  CSC built ({time.time()-t0:.1f}s)")
    sys.stdout.flush()

    # === Baseline ===
    print(f"\n  === Baseline: Column Anchor ===")
    t0 = time.time()
    anchor_panels = build_anchor_panels(rows, cols, indptr, indices, csc_indptr, csc_indices)
    print(f"  Built {len(anchor_panels)} panels ({time.time()-t0:.1f}s)")
    m_anchor = compute_metrics(anchor_panels, nnz, "Column Anchor")

    # === Direction 1: Post-split ===
    print(f"\n  === Direction 1: Post-split (split U>32 panels) ===")
    sys.stdout.flush()
    t0 = time.time()
    n_split = 0
    n_kept = 0
    post_panels = []

    for i, (p_rows, p_cols, p_nnz) in enumerate(anchor_panels):
        U = len(p_cols)
        if U <= 32:
            post_panels.append((p_rows, p_cols, p_nnz))
            n_kept += 1
        else:
            sub = post_split_panel(p_rows, p_cols, p_nnz, indptr, indices, max_U=32)
            post_panels.extend(sub)
            n_split += 1
        if (i+1) % 5000 == 0:
            print(f"    ... {i+1}/{len(anchor_panels)} panels processed ({time.time()-t0:.0f}s)")
            sys.stdout.flush()

    elapsed = time.time() - t0
    print(f"  Post-split: {n_kept} kept + {n_split} split -> {len(post_panels)} total ({elapsed:.1f}s)")
    m_post = compute_metrics(post_panels, nnz, "Post-split")
    perf_model(m_anchor, m_post, "Anchor", "Post-split")

    # === Direction 2: Improved clustering ===
    print(f"\n  === Direction 2: Improved A^T*A (max_deg=1000, sample=0.5, min_co=2) ===")
    sys.stdout.flush()
    t0 = time.time()
    imp_clusters, _ = ata_clustering_improved(
        rows, cols, indptr, indices,
        max_row_deg=1000, sample_rate=0.5, min_cooccur=2, max_cluster=32
    )
    elapsed_cluster = time.time() - t0
    print(f"  Clustering done ({elapsed_cluster:.1f}s)")
    sys.stdout.flush()

    col_to_cluster = {}
    good_clusters = {}
    for ci, cl in enumerate(imp_clusters):
        if len(cl) >= 8:
            good_clusters[ci] = set(cl)
            for c in cl:
                col_to_cluster[c] = ci

    print(f"  Good clusters (size>=8): {len(good_clusters)}")

    assigned_rows = np.zeros(rows, dtype=bool)
    cluster_rows = defaultdict(list)
    leftover = []

    for r in range(rows):
        deg = int(indptr[r+1] - indptr[r])
        if deg == 0:
            continue
        votes = defaultdict(int)
        for idx in range(indptr[r], indptr[r+1]):
            c = indices[idx]
            if c in col_to_cluster:
                votes[col_to_cluster[c]] += 1
        if votes:
            best = max(votes, key=votes.get)
            if votes[best] >= max(2, deg * 0.3):
                cluster_rows[best].append(r)
                continue
        leftover.append(r)

    n_clustered = sum(len(v) for v in cluster_rows.values())
    print(f"  Rows: {n_clustered} clustered, {len(leftover)} leftover")
    sys.stdout.flush()

    imp_panels = []
    for ci, cl_set in good_clusters.items():
        cr = cluster_rows.get(ci, [])
        if len(cr) < 16:
            leftover.extend(cr)
            continue
        cr_scored = []
        for r in cr:
            in_cl = sum(1 for idx in range(indptr[r], indptr[r+1]) if indices[idx] in cl_set)
            cr_scored.append((r, in_cl))
        cr_scored.sort(key=lambda x: -x[1])

        for batch_start in range(0, len(cr_scored) - 15, 16):
            batch = [r for r, _ in cr_scored[batch_start:batch_start+16]]
            col_set = set()
            total_nnz_p = 0
            for r in batch:
                for idx in range(indptr[r], indptr[r+1]):
                    col_set.add(indices[idx])
                total_nnz_p += indptr[r+1] - indptr[r]
            imp_panels.append((batch, col_set, total_nnz_p))
            for r in batch:
                assigned_rows[r] = True

        remaining_start = (len(cr_scored) // 16) * 16
        for r, _ in cr_scored[remaining_start:]:
            if not assigned_rows[r]:
                leftover.append(r)

    print(f"  Improved-Hybrid cluster panels: {len(imp_panels)}")

    leftover_set = set(r for r in leftover if not assigned_rows[r])
    col_deg_local = np.zeros(cols, dtype=np.int32)
    for r in leftover_set:
        for idx in range(indptr[r], indptr[r+1]):
            col_deg_local[indices[idx]] += 1

    col_order = np.argsort(-col_deg_local)
    lo_assigned = set()

    for anchor in col_order:
        if col_deg_local[anchor] == 0:
            break
        cand = []
        for idx in range(csc_indptr[anchor], csc_indptr[anchor+1]):
            r = csc_indices[idx]
            if r in leftover_set and r not in lo_assigned:
                cand.append(r)
        if len(cand) < 16:
            continue
        cand.sort(key=lambda r: int(row_deg[r]))
        selected = cand[:16]
        col_set = set()
        total_p = 0
        for r in selected:
            for idx in range(indptr[r], indptr[r+1]):
                col_set.add(indices[idx])
            total_p += indptr[r+1] - indptr[r]
        imp_panels.append((selected, col_set, total_p))
        lo_assigned.update(selected)

    print(f"  Improved-Hybrid total panels: {len(imp_panels)}")
    m_imp = compute_metrics(imp_panels, nnz, "Improved-Hybrid")
    perf_model(m_anchor, m_imp, "Anchor", "Improved-Hybrid")

    # === Direction 3: Hybrid + Post-split ===
    print(f"\n  === Direction 3: Improved-Hybrid + Post-split ===")
    sys.stdout.flush()
    t0 = time.time()
    combo_panels = []
    n_split3 = 0
    n_kept3 = 0

    for i, (p_rows, p_cols, p_nnz) in enumerate(imp_panels):
        U = len(p_cols)
        if U <= 32:
            combo_panels.append((p_rows, p_cols, p_nnz))
            n_kept3 += 1
        else:
            sub = post_split_panel(p_rows, p_cols, p_nnz, indptr, indices, max_U=32)
            combo_panels.extend(sub)
            n_split3 += 1
        if (i+1) % 5000 == 0:
            print(f"    ... {i+1}/{len(imp_panels)} panels processed ({time.time()-t0:.0f}s)")
            sys.stdout.flush()

    elapsed3 = time.time() - t0
    print(f"  Combo: {n_kept3} kept + {n_split3} split -> {len(combo_panels)} total ({elapsed3:.1f}s)")
    m_combo = compute_metrics(combo_panels, nnz, "Hybrid+PostSplit")
    perf_model(m_anchor, m_combo, "Anchor", "Hybrid+PostSplit")

    # === Final summary ===
    print(f"\n  {'='*80}")
    print(f"  FINAL SUMMARY: {name}")
    print(f"  {'='*80}")

    methods = [
        ("Anchor (baseline)", m_anchor),
        ("Post-split (D1)", m_post),
        ("Imp-Hybrid (D2)", m_imp),
        ("Hybrid+PS (D3)", m_combo),
    ]

    header = f"  {'Method':25s} {'Panels':>8s} {'Cov%':>7s} {'Fill':>7s} {'AvgU':>7s} " \
             f"{'U<=32':>7s} {'Tiles':>9s} {'Gathers':>10s} {'GathBW':>8s} {'TileEff':>8s} {'GathEff':>8s}"
    print(header)
    print(f"  {'-'*25} {'-'*8} {'-'*7} {'-'*7} {'-'*7} {'-'*7} {'-'*9} {'-'*10} {'-'*8} {'-'*8} {'-'*8}")

    for lbl, m in methods:
        if m:
            print(f"  {lbl:25s} {m['panels']:8d} {m['cov_pct']:6.1f}% {m['avg_fill']:6.1f}% "
                  f"{m['avg_U']:7.1f} {m['u_le32']:6.1f}% {m['tiles']:9,d} {m['gathers']:10,d} "
                  f"{m['gather_bw']:7.1f}M {m['tile_eff']:7.1f} {m['gather_eff']:8.2f}")

    print(f"\n  Ratios vs Anchor:")
    for lbl, m in methods[1:]:
        if m and m_anchor:
            print(f"    {lbl}: tiles {m['tiles']/m_anchor['tiles']:.3f}x, "
                  f"gathers {m['gathers']/m_anchor['gathers']:.3f}x, "
                  f"cov {m['cov_pct']/m_anchor['cov_pct']:.3f}x, "
                  f"fill {m['avg_fill']/m_anchor['avg_fill']:.2f}x")
    sys.stdout.flush()


if __name__ == "__main__":
    data_dir = os.path.expanduser("~/data/SpMM_project/data")
    matrices = [
        ("hollywood-2009", f"{data_dir}/hollywood-2009/hollywood-2009.csrbin"),
        ("as-Skitter",     f"{data_dir}/as-Skitter/as-Skitter.csrbin"),
        ("soc-Pokec",      f"{data_dir}/soc-Pokec/soc-Pokec.csrbin"),
        ("web-Google",     f"{data_dir}/web-Google/web-Google.csrbin"),
        ("amazon0601",     f"{data_dir}/amazon0601/amazon0601.csrbin"),
    ]

    for name, path in matrices:
        if os.path.exists(path):
            try:
                process_matrix(name, path)
            except Exception as e:
                print(f"ERROR on {name}: {e}")
                import traceback
                traceback.print_exc()
        else:
            print(f"SKIP: {name} not found at {path}")

    print(f"\n{'#'*70}")
    print(f"# ALL DONE")
    print(f"{'#'*70}")
