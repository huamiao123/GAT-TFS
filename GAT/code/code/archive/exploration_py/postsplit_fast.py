#!/usr/bin/env python3
"""
Fast Post-split using numpy vectorization.
Key optimization: use binary matrix multiply for local co-occurrence
instead of Python pair enumeration.

For a panel with 16 rows and U columns:
  Old: O(16 * deg^2) Python loops ~ millions of iterations
  New: binary_mat.T @ binary_mat via numpy ~ instant
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
        panels.append((list(selected), col_set, total_nnz))
    return panels

# ============================================================
# FAST local column clustering using numpy
# ============================================================
def fast_local_cluster(panel_rows, indptr, indices, max_cluster=32):
    """
    Build 16 x U binary matrix, compute co-occurrence via matmul,
    then greedy cluster columns into groups of <= max_cluster.
    """
    # Step 1: collect columns and build binary matrix
    col_set = set()
    for r in panel_rows:
        for idx in range(indptr[r], indptr[r+1]):
            col_set.add(int(indices[idx]))

    col_list = sorted(col_set)
    U = len(col_list)
    if U <= max_cluster:
        return [col_set]

    col_to_idx = {c: i for i, c in enumerate(col_list)}
    n_rows = len(panel_rows)

    # Build binary matrix (n_rows x U), dtype uint8 to save memory
    bmat = np.zeros((n_rows, U), dtype=np.uint8)
    for ri, r in enumerate(panel_rows):
        for idx in range(indptr[r], indptr[r+1]):
            c = int(indices[idx])
            bmat[ri, col_to_idx[c]] = 1

    # Step 2: co-occurrence = bmat.T @ bmat (U x U matrix)
    # Each entry [i][j] = number of rows where col i and col j both appear
    cooccur = bmat.T.astype(np.int16) @ bmat.astype(np.int16)  # U x U

    # Column local degree (how many of the 16 rows contain this column)
    col_deg = bmat.sum(axis=0)  # shape (U,)

    # Step 3: greedy clustering
    unassigned = np.ones(U, dtype=bool)
    clusters = []

    # Sort by degree descending for seed selection
    seed_order = np.argsort(-col_deg)

    for seed in seed_order:
        if not unassigned[seed]:
            continue
        cluster = [seed]
        unassigned[seed] = False

        while len(cluster) < max_cluster:
            # Score all unassigned columns by total co-occurrence with cluster
            scores = np.zeros(U, dtype=np.int32)
            for c in cluster:
                scores += cooccur[c]
            scores[~unassigned] = -1  # mask assigned

            best = np.argmax(scores)
            if scores[best] <= 0:
                break
            cluster.append(best)
            unassigned[best] = False

        # Convert indices back to original column ids
        clusters.append(set(col_list[i] for i in cluster))

    # Remaining singletons
    for i in range(U):
        if unassigned[i]:
            clusters.append({col_list[i]})

    return clusters

def fast_post_split(panel_rows, panel_cols, panel_nnz, indptr, indices, max_U=32):
    """Split one panel if U > max_U using fast numpy clustering."""
    U = len(panel_cols)
    if U <= max_U:
        return [(panel_rows, panel_cols, panel_nnz)]

    col_clusters = fast_local_cluster(panel_rows, indptr, indices, max_cluster=max_U)

    sub_panels = []
    for cc in col_clusters:
        if len(cc) == 0:
            continue
        rows_nnz = []
        for r in panel_rows:
            nnz_in = 0
            for idx in range(indptr[r], indptr[r+1]):
                if int(indices[idx]) in cc:
                    nnz_in += 1
            if nnz_in > 0:
                rows_nnz.append((r, nnz_in))
        if len(rows_nnz) == 0:
            continue
        sub_rows = [r for r, _ in rows_nnz]
        sub_nnz = sum(n for _, n in rows_nnz)
        sub_panels.append((sub_rows, cc, sub_nnz))
    return sub_panels

# ============================================================
# Metrics and perf model
# ============================================================
def compute_metrics(panels, total_nnz, label=""):
    if not panels:
        print(f"  [{label}] No panels"); return {}
    n = len(panels)
    tot_tiles=0; tot_gath=0; tot_cov=0; fills=[]; Us=[]; u32=0; rows_list=[]
    for pr,pc,pn in panels:
        nr=len(pr); U=len(pc); t=(U+31)//32
        tot_tiles+=t; tot_gath+=U; tot_cov+=pn
        f=pn/(nr*U) if nr*U>0 else 0
        fills.append(f); Us.append(U); rows_list.append(nr)
        if U<=32: u32+=1
    af=np.mean(fills)*100; au=np.mean(Us); p32=u32/n*100
    cp=tot_cov/total_nnz*100; fb=total_nnz-tot_cov
    te=tot_cov/tot_tiles if tot_tiles else 0
    ge=tot_cov/tot_gath if tot_gath else 0
    gbw=tot_gath*128*2/1e6
    f16=sum(1 for r,c,nn in panels if len(r)==16)
    ar=np.mean(rows_list)
    print(f"  [{label}]")
    print(f"    Panels: {n} (full-16: {f16}), Avg rows: {ar:.1f}")
    print(f"    NNZ covered: {tot_cov:,} ({cp:.1f}%)")
    print(f"    Fallback: {fb:,} ({100-cp:.1f}%)")
    print(f"    Tiles: {tot_tiles:,}, Gathers: {tot_gath:,}")
    print(f"    Tile eff: {te:.1f}, Gather eff: {ge:.2f}")
    print(f"    Gather BW: {gbw:.1f} MB (K=128)")
    print(f"    Fill: {af:.1f}%, U: {au:.1f}, U<=32: {p32:.1f}%")
    sys.stdout.flush()
    return {'panels':n,'covered':tot_cov,'cov_pct':cp,'fallback':fb,
            'tiles':tot_tiles,'gathers':tot_gath,'tile_eff':te,'gather_eff':ge,
            'gather_bw':gbw,'avg_fill':af,'avg_U':au,'u_le32':p32,'avg_rows':ar}

def perf_model(mb, mt, lb, lt):
    if not mb or not mt: return
    rt=mt['tiles']/mb['tiles'] if mb['tiles'] else 1
    rg=mt['gathers']/mb['gathers'] if mb['gathers'] else 1
    rf=mt['fallback']/mb['fallback'] if mb['fallback'] else 1
    print(f"\n  --- Perf: {lt} vs {lb} ---")
    print(f"    Tiles: {rt:.3f}x, Gathers: {rg:.3f}x, Fallback: {rf:.3f}x")
    for nm,wt,wg,wf in [("Gather-dom",0.15,0.60,0.25),("Balanced",0.33,0.34,0.33),("FB-heavy",0.15,0.30,0.55)]:
        T=wt*rt+wg*rg+wf*rf; p=(1-T)*100
        print(f"    {nm:15s} T={T:.3f} -> {p:.1f}% {'faster' if p>0 else 'slower'}")
    sys.stdout.flush()

# ============================================================
# Main
# ============================================================
def process_matrix(name, path):
    print(f"\n{'#'*70}")
    print(f"# {name}")
    print(f"{'#'*70}"); sys.stdout.flush()

    rows, cols, nnz, indptr, indices = read_csrbin(path)
    row_deg = np.diff(indptr.astype(np.int64))
    print(f"  Degree: mean={row_deg.mean():.1f}, P99={np.percentile(row_deg,99):.0f}")

    t0=time.time()
    csc_indptr, csc_indices = build_csc(rows, cols, indptr, indices)
    print(f"  CSC: {time.time()-t0:.1f}s"); sys.stdout.flush()

    # Baseline
    print(f"\n  === Baseline ===")
    t0=time.time()
    anchor = build_anchor_panels(rows, cols, indptr, indices, csc_indptr, csc_indices)
    print(f"  {len(anchor)} panels ({time.time()-t0:.1f}s)")
    ma = compute_metrics(anchor, nnz, "Anchor")

    # Count how many need splitting
    n_big = sum(1 for r,c,n in anchor if len(c)>32)
    n_small = len(anchor) - n_big
    print(f"\n  Panels U>32: {n_big} ({n_big/len(anchor)*100:.1f}%), U<=32: {n_small}")

    # Fast Post-split
    print(f"\n  === Fast Post-split (numpy vectorized) ==="); sys.stdout.flush()
    t0 = time.time()
    post = []
    n_split=0; n_kept=0
    for i,(pr,pc,pn) in enumerate(anchor):
        if len(pc) <= 32:
            post.append((pr,pc,pn)); n_kept+=1
        else:
            sub = fast_post_split(pr,pc,pn,indptr,indices,max_U=32)
            post.extend(sub); n_split+=1
        if (i+1)%2000==0:
            elapsed=time.time()-t0
            rate=i/elapsed if elapsed>0 else 0
            eta=(len(anchor)-i)/rate if rate>0 else 0
            print(f"    {i+1}/{len(anchor)} ({elapsed:.0f}s, {rate:.0f} panels/s, ETA {eta:.0f}s)")
            sys.stdout.flush()

    elapsed=time.time()-t0
    print(f"  Done: {n_kept} kept + {n_split} split -> {len(post)} panels ({elapsed:.1f}s)")
    mp = compute_metrics(post, nnz, "Post-split")
    perf_model(ma, mp, "Anchor", "Post-split")

    # Summary
    print(f"\n  {'='*70}")
    print(f"  SUMMARY: {name}")
    print(f"  {'='*70}")
    for lbl,m in [("Anchor",ma),("Post-split",mp)]:
        if m:
            print(f"  {lbl:15s} panels={m['panels']:>7d} cov={m['cov_pct']:5.1f}% fill={m['avg_fill']:5.1f}% "
                  f"U={m['avg_U']:7.1f} U<=32={m['u_le32']:5.1f}% "
                  f"tiles={m['tiles']:>9,d} gathers={m['gathers']:>10,d} BW={m['gather_bw']:>8.1f}M")
    if ma and mp:
        print(f"\n  Ratios: tiles={mp['tiles']/ma['tiles']:.3f}x  gathers={mp['gathers']/ma['gathers']:.3f}x  "
              f"cov={mp['cov_pct']/ma['cov_pct']:.3f}x  fill={mp['avg_fill']/ma['avg_fill']:.2f}x")
    sys.stdout.flush()

if __name__ == "__main__":
    data_dir = os.path.expanduser("~/data/SpMM_project/data")
    # Hollywood first (most important), then others
    matrices = [
        ("hollywood-2009", f"{data_dir}/hollywood-2009/hollywood-2009.csrbin"),
        ("soc-Pokec",      f"{data_dir}/soc-Pokec/soc-Pokec.csrbin"),
        ("as-Skitter",     f"{data_dir}/as-Skitter/as-Skitter.csrbin"),
        ("web-Google",     f"{data_dir}/web-Google/web-Google.csrbin"),
        ("amazon0601",     f"{data_dir}/amazon0601/amazon0601.csrbin"),
        ("cit-Patents",    f"{data_dir}/cit-Patents/cit-Patents.csrbin"),
        ("indochina-2004", f"{data_dir}/indochina-2004/indochina-2004.csrbin"),
    ]
    for name, path in matrices:
        if os.path.exists(path):
            try:
                process_matrix(name, path)
            except Exception as e:
                print(f"ERROR on {name}: {e}")
                import traceback; traceback.print_exc()
        else:
            print(f"SKIP: {name}")
    print(f"\n{'#'*70}\n# ALL DONE\n{'#'*70}")
