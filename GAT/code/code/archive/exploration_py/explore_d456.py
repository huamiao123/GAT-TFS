#!/usr/bin/env python3
"""
Phase 12.3b: Parallel exploration - Directions 4, 5, 6
  Direction 4: Greedy column pruning (keep top-32 cols per panel)
  Direction 5: Multi-pass anchor (only keep U<=32 panels each pass)
  Direction 6: Row overlap (allow rows in multiple panels)
"""

import numpy as np
import struct
import time, sys, os
from collections import defaultdict

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

def build_anchor_panels(rows, cols, indptr, indices, csc_indptr, csc_indices, panel_h=16):
    row_deg = np.diff(indptr.astype(np.int64))
    col_deg = np.diff(csc_indptr.astype(np.int64))
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
# Metrics
# ============================================================
def compute_metrics(panels, total_nnz, label=""):
    if len(panels) == 0:
        print(f"  [{label}] No panels")
        return {}
    n_panels = len(panels)
    total_tiles = 0; total_gathers = 0; total_covered_nnz = 0
    fills = []; Us = []; u_le32 = 0
    rows_per_panel = []

    for (p_rows, p_cols, p_nnz) in panels:
        n_rows = len(p_rows); U = len(p_cols)
        tiles = (U + 31) // 32
        total_tiles += tiles; total_gathers += U; total_covered_nnz += p_nnz
        fill = p_nnz / (n_rows * U) if (n_rows * U) > 0 else 0
        fills.append(fill); Us.append(U); rows_per_panel.append(n_rows)
        if U <= 32: u_le32 += 1

    avg_fill = np.mean(fills)*100; avg_U = np.mean(Us)
    pct_u32 = u_le32/n_panels*100
    cov_pct = total_covered_nnz/total_nnz*100
    fallback = total_nnz - total_covered_nnz
    tile_eff = total_covered_nnz/total_tiles if total_tiles>0 else 0
    gather_eff = total_covered_nnz/total_gathers if total_gathers>0 else 0
    gather_bw = total_gathers*128*2/1e6
    n_full16 = sum(1 for r,c,n in panels if len(r)==16)
    avg_rows = np.mean(rows_per_panel)

    print(f"  [{label}]")
    print(f"    Panels: {n_panels} (full-16: {n_full16}), Avg rows: {avg_rows:.1f}")
    print(f"    NNZ covered: {total_covered_nnz:,} ({cov_pct:.1f}%)")
    print(f"    Fallback: {fallback:,} ({100-cov_pct:.1f}%)")
    print(f"    Tiles: {total_tiles:,}, Gathers: {total_gathers:,}")
    print(f"    Tile eff: {tile_eff:.1f}, Gather eff: {gather_eff:.2f}")
    print(f"    Gather BW: {gather_bw:.1f} MB (K=128)")
    print(f"    Fill: {avg_fill:.1f}%, U: {avg_U:.1f}, U<=32: {pct_u32:.1f}%")
    sys.stdout.flush()
    return {'panels':n_panels, 'covered':total_covered_nnz, 'cov_pct':cov_pct,
            'fallback':fallback, 'tiles':total_tiles, 'gathers':total_gathers,
            'tile_eff':tile_eff, 'gather_eff':gather_eff, 'gather_bw':gather_bw,
            'avg_fill':avg_fill, 'avg_U':avg_U, 'u_le32':pct_u32, 'avg_rows':avg_rows}

def perf_model(m_base, m_test, label_base, label_test):
    if not m_base or not m_test: return
    r_tiles = m_test['tiles']/m_base['tiles'] if m_base['tiles']>0 else 1
    r_gath = m_test['gathers']/m_base['gathers'] if m_base['gathers']>0 else 1
    r_fb = m_test['fallback']/m_base['fallback'] if m_base['fallback']>0 else 1
    print(f"\n  --- Perf Model: {label_test} vs {label_base} ---")
    print(f"    Tiles: {r_tiles:.3f}x, Gathers: {r_gath:.3f}x, Fallback: {r_fb:.3f}x")
    for name, wt, wg, wf in [("Gather-dominant",0.15,0.60,0.25),("Balanced",0.33,0.34,0.33),("Fallback-heavy",0.15,0.30,0.55)]:
        T = wt*r_tiles + wg*r_gath + wf*r_fb
        pct = (1-T)*100
        print(f"    {name:25s} T={T:.3f} -> {pct:.1f}% {'faster' if pct>0 else 'slower'}")
    sys.stdout.flush()

# ============================================================
# Direction 4: Greedy Column Pruning
# For each panel with U>32, keep top-32 columns by NNZ count
# ============================================================
def col_pruning(anchor_panels, indptr, indices, total_nnz, max_U=32):
    print(f"\n  === Direction 4: Column Pruning (top-{max_U} cols) ===")
    sys.stdout.flush()
    t0 = time.time()
    pruned_panels = []
    n_pruned = 0; n_kept = 0

    for p_rows, p_cols, p_nnz in anchor_panels:
        U = len(p_cols)
        if U <= max_U:
            pruned_panels.append((p_rows, p_cols, p_nnz))
            n_kept += 1
            continue
        n_pruned += 1
        # Count NNZ per column within this panel
        col_nnz = defaultdict(int)
        for r in p_rows:
            for idx in range(indptr[r], indptr[r+1]):
                c = indices[idx]
                if c in p_cols:
                    col_nnz[c] += 1
        # Keep top max_U columns
        sorted_cols = sorted(col_nnz.items(), key=lambda x: -x[1])
        keep_cols = set(c for c, _ in sorted_cols[:max_U])
        # Recompute NNZ with only kept columns
        new_nnz = 0
        for r in p_rows:
            for idx in range(indptr[r], indptr[r+1]):
                if indices[idx] in keep_cols:
                    new_nnz += 1
        pruned_panels.append((p_rows, keep_cols, new_nnz))

    elapsed = time.time() - t0
    print(f"  Pruned {n_pruned}, kept {n_kept} ({elapsed:.1f}s)")
    m = compute_metrics(pruned_panels, total_nnz, "Col-Pruning")
    return pruned_panels, m

# ============================================================
# Direction 5: Multi-pass Anchor
# Pass 1: normal anchor, keep only U<=threshold panels
# Pass 2+: remaining rows re-run anchor
# ============================================================
def multipass_anchor(rows, cols, indptr, indices, csc_indptr, csc_indices, total_nnz,
                     max_passes=5, U_threshold=64):
    print(f"\n  === Direction 5: Multi-pass Anchor (max {max_passes} passes, U<={U_threshold}) ===")
    sys.stdout.flush()
    t0 = time.time()

    row_deg = np.diff(indptr.astype(np.int64))
    col_deg_orig = np.diff(csc_indptr.astype(np.int64))
    all_assigned = np.zeros(rows, dtype=bool)
    all_good_panels = []

    for pass_num in range(1, max_passes+1):
        # Build panels from unassigned rows
        assigned_this = np.zeros(rows, dtype=bool)
        # Use original CSC but filter for unassigned rows
        panels_this = []

        col_order = np.argsort(-col_deg_orig)
        for anchor in col_order:
            if col_deg_orig[anchor] == 0:
                break
            cand_rows = []
            for idx in range(csc_indptr[anchor], csc_indptr[anchor+1]):
                r = csc_indices[idx]
                if not all_assigned[r] and not assigned_this[r]:
                    cand_rows.append(r)
            if len(cand_rows) < 16:
                continue
            cand_rows.sort(key=lambda r: row_deg[r])
            selected = cand_rows[:16]
            col_set = set()
            total_nnz_p = 0
            for r in selected:
                for idx in range(indptr[r], indptr[r+1]):
                    col_set.add(indices[idx])
                total_nnz_p += indptr[r+1] - indptr[r]
            for r in selected:
                assigned_this[r] = True
            panels_this.append((selected, col_set, total_nnz_p))

        # Split into good (U<=threshold) and bad
        good = [(r,c,n) for r,c,n in panels_this if len(c) <= U_threshold]
        bad  = [(r,c,n) for r,c,n in panels_this if len(c) > U_threshold]

        all_good_panels.extend(good)
        for r, c, n in good:
            for row in r:
                all_assigned[row] = True

        n_unassigned = np.sum(~all_assigned)
        print(f"    Pass {pass_num}: {len(panels_this)} panels -> "
              f"{len(good)} good + {len(bad)} bad, "
              f"{n_unassigned} rows remaining")
        sys.stdout.flush()

        if len(bad) == 0 or n_unassigned < 16:
            break

    elapsed = time.time() - t0
    print(f"  Multi-pass done: {len(all_good_panels)} total panels ({elapsed:.1f}s)")
    m = compute_metrics(all_good_panels, total_nnz, f"Multi-pass(U<={U_threshold})")
    return all_good_panels, m

# ============================================================
# Direction 6: Row Overlap
# Build panels normally, then for each panel with U>32,
# re-select rows that BEST fit top-32 columns (allowing re-use)
# ============================================================
def row_overlap_panels(rows, cols, indptr, indices, csc_indptr, csc_indices, total_nnz):
    print(f"\n  === Direction 6: Row Overlap (allow rows in multiple panels) ===")
    sys.stdout.flush()
    t0 = time.time()

    row_deg = np.diff(indptr.astype(np.int64))
    col_deg = np.diff(csc_indptr.astype(np.int64))
    col_order = np.argsort(-col_deg)

    # Track how many NNZ each row has been "covered" for
    row_covered_nnz = np.zeros(rows, dtype=np.int32)
    panels = []
    used_in_panel = np.zeros(rows, dtype=np.int32)  # count how many panels

    # Build column groups: top-32 columns by degree, greedily
    # For each high-degree column, find rows, pick 16 best, form panel
    max_panels = 200000  # safety limit
    processed_anchors = set()

    for anchor in col_order:
        if len(panels) >= max_panels:
            break
        if col_deg[anchor] < 16:
            break
        if anchor in processed_anchors:
            continue
        processed_anchors.add(anchor)

        # Get all rows in this column
        cand_rows = []
        for idx in range(csc_indptr[anchor], csc_indptr[anchor+1]):
            r = csc_indices[idx]
            cand_rows.append(r)

        if len(cand_rows) < 16:
            continue

        # Score rows: prefer rows with low degree (small U contribution)
        # AND rows that still have uncovered NNZ
        def row_score(r):
            total = int(indptr[r+1] - indptr[r])
            covered = int(row_covered_nnz[r])
            uncovered = total - covered
            if uncovered <= 0:
                return (0, -total)  # fully covered, low priority
            return (uncovered, -total)  # prefer more uncovered, then lower degree

        cand_rows.sort(key=row_score, reverse=True)

        # Take top 16
        selected = cand_rows[:16]

        # Compute column union and NNZ
        col_set = set()
        total_nnz_p = 0
        for r in selected:
            for idx in range(indptr[r], indptr[r+1]):
                col_set.add(indices[idx])
            total_nnz_p += indptr[r+1] - indptr[r]

        # Only keep if panel is "worth it" (fill above breakeven)
        U = len(col_set)
        fill = total_nnz_p / (16 * U) if U > 0 else 0
        if fill < 0.0625:  # below breakeven
            continue

        panels.append((selected, col_set, total_nnz_p))

        # Mark rows as covered
        for r in selected:
            used_in_panel[r] += 1
            for idx in range(indptr[r], indptr[r+1]):
                if indices[idx] in col_set:
                    row_covered_nnz[r] += 1

        # Stop when most NNZ are covered
        total_covered = int(np.sum(row_covered_nnz))
        if total_covered > total_nnz * 0.95:
            break

    elapsed = time.time() - t0
    multi_use = np.sum(used_in_panel > 1)
    max_use = np.max(used_in_panel) if len(used_in_panel) > 0 else 0
    print(f"  Built {len(panels)} panels ({elapsed:.1f}s)")
    print(f"  Rows in >1 panel: {multi_use} ({multi_use/rows*100:.1f}%)")
    print(f"  Max times a row appears: {max_use}")

    # Note: NNZ coverage calculation is different for overlap -
    # a row's NNZ can be counted multiple times across panels
    # For fair comparison, compute unique NNZ coverage
    covered_pairs = set()
    for p_rows, p_cols, p_nnz in panels:
        for r in p_rows:
            for idx in range(indptr[r], indptr[r+1]):
                c = indices[idx]
                if c in p_cols:
                    covered_pairs.add((r, c))
    unique_covered = len(covered_pairs)
    print(f"  Unique NNZ covered: {unique_covered:,} ({unique_covered/total_nnz*100:.1f}%)")
    del covered_pairs

    m = compute_metrics(panels, total_nnz, "Row-Overlap")
    m['unique_covered'] = unique_covered
    m['unique_cov_pct'] = unique_covered / total_nnz * 100
    return panels, m

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

    # Baseline
    print(f"\n  === Baseline: Column Anchor ===")
    t0 = time.time()
    anchor_panels = build_anchor_panels(rows, cols, indptr, indices, csc_indptr, csc_indices)
    print(f"  Built {len(anchor_panels)} panels ({time.time()-t0:.1f}s)")
    m_anchor = compute_metrics(anchor_panels, nnz, "Column Anchor")

    # Direction 4: Column Pruning
    _, m_prune = col_pruning(anchor_panels, indptr, indices, nnz, max_U=32)
    perf_model(m_anchor, m_prune, "Anchor", "Col-Pruning")

    # Direction 5: Multi-pass Anchor (try two thresholds)
    _, m_mp32 = multipass_anchor(rows, cols, indptr, indices, csc_indptr, csc_indices, nnz,
                                  max_passes=5, U_threshold=32)
    perf_model(m_anchor, m_mp32, "Anchor", "Multi-pass(U<=32)")

    _, m_mp64 = multipass_anchor(rows, cols, indptr, indices, csc_indptr, csc_indices, nnz,
                                  max_passes=5, U_threshold=64)
    perf_model(m_anchor, m_mp64, "Anchor", "Multi-pass(U<=64)")

    # Direction 6: Row Overlap
    _, m_overlap = row_overlap_panels(rows, cols, indptr, indices, csc_indptr, csc_indices, nnz)
    perf_model(m_anchor, m_overlap, "Anchor", "Row-Overlap")

    # Summary
    print(f"\n  {'='*80}")
    print(f"  FINAL SUMMARY: {name}")
    print(f"  {'='*80}")
    methods = [
        ("Anchor (baseline)", m_anchor),
        ("Col-Pruning (D4)", m_prune),
        ("MultiPass-32 (D5a)", m_mp32),
        ("MultiPass-64 (D5b)", m_mp64),
        ("Row-Overlap (D6)", m_overlap),
    ]
    header = f"  {'Method':25s} {'Panels':>8s} {'Cov%':>7s} {'Fill':>7s} {'AvgU':>7s} {'U<=32':>7s} {'Tiles':>9s} {'Gathers':>10s} {'GathBW':>8s}"
    print(header)
    print(f"  {'-'*25} {'-'*8} {'-'*7} {'-'*7} {'-'*7} {'-'*7} {'-'*9} {'-'*10} {'-'*8}")
    for lbl, m in methods:
        if m:
            print(f"  {lbl:25s} {m['panels']:8d} {m['cov_pct']:6.1f}% {m['avg_fill']:6.1f}% "
                  f"{m['avg_U']:7.1f} {m['u_le32']:6.1f}% {m['tiles']:9,d} {m['gathers']:10,d} "
                  f"{m['gather_bw']:7.1f}M")
    print(f"\n  Ratios vs Anchor:")
    for lbl, m in methods[1:]:
        if m and m_anchor:
            print(f"    {lbl}: tiles {m['tiles']/m_anchor['tiles']:.3f}x, "
                  f"gathers {m['gathers']/m_anchor['gathers']:.3f}x, "
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
