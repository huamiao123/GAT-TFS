#!/usr/bin/env python3
"""
Hybrid Post-split: 
  U <= 32:   keep as-is
  32 < U <= 500: local clustering split (fast enough)
  U > 500:  column pruning (keep top-32 cols, instant)
  
Plus: skip sub-panel generation Python loop, use bmat directly
"""

import numpy as np
import struct
import time, sys, os

def read_csrbin(filename):
    with open(filename, 'rb') as f:
        struct.unpack('III', f.read(12))
        nrow, ncol, nnz = struct.unpack('QQQ', f.read(24))
        indptr  = np.frombuffer(f.read(4*(nrow+1)), dtype=np.uint32)
        indices = np.frombuffer(f.read(4*nnz),      dtype=np.uint32)
    print(f"  Read: {nrow:,} rows, {ncol:,} cols, {nnz:,} NNZ")
    return nrow, ncol, nnz, indptr, indices

def build_csc(rows, cols, indptr, indices):
    col_counts = np.zeros(cols, dtype=np.int32)
    for j in indices: col_counts[j] += 1
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
        if col_deg[anchor] == 0: break
        cand = []
        for idx in range(csc_indptr[anchor], csc_indptr[anchor+1]):
            r = csc_indices[idx]
            if not assigned[r]: cand.append(r)
        if len(cand) < panel_h: continue
        cand.sort(key=lambda r: row_deg[r])
        sel = cand[:panel_h]
        cs = set()
        tn = 0
        for r in sel:
            for idx in range(indptr[r], indptr[r+1]):
                cs.add(int(indices[idx]))
            tn += int(indptr[r+1] - indptr[r])
        for r in sel: assigned[r] = True
        panels.append((list(sel), cs, tn))
    return panels

def build_bmat(panel_rows, indptr, indices):
    """Build binary matrix and column mapping for a panel."""
    col_set = set()
    for r in panel_rows:
        for idx in range(indptr[r], indptr[r+1]):
            col_set.add(int(indices[idx]))
    col_list = sorted(col_set)
    U = len(col_list)
    col_to_idx = {c: i for i, c in enumerate(col_list)}
    bmat = np.zeros((len(panel_rows), U), dtype=np.uint8)
    for ri, r in enumerate(panel_rows):
        for idx in range(indptr[r], indptr[r+1]):
            bmat[ri, col_to_idx[int(indices[idx])]] = 1
    return bmat, col_list, col_to_idx

def greedy_cluster_from_cooccur(cooccur, col_deg, U, max_cluster=32):
    """Greedy clustering given co-occurrence matrix."""
    unassigned = np.ones(U, dtype=bool)
    clusters = []
    seed_order = np.argsort(-col_deg)
    for seed in seed_order:
        if not unassigned[seed]: continue
        cluster = [int(seed)]
        unassigned[seed] = False
        while len(cluster) < max_cluster:
            scores = cooccur[cluster].sum(axis=0)
            scores[~unassigned] = -1
            best = int(np.argmax(scores))
            if scores[best] <= 0: break
            cluster.append(best)
            unassigned[best] = False
        clusters.append(cluster)
    for i in range(U):
        if unassigned[i]: clusters.append([int(i)])
    return clusters

def split_by_clustering(bmat, col_list, panel_rows, max_cluster=32):
    """Split using local co-occurrence clustering."""
    U = len(col_list)
    cooccur = bmat.T.astype(np.int16) @ bmat.astype(np.int16)
    col_deg = bmat.sum(axis=0)
    clusters = greedy_cluster_from_cooccur(cooccur, col_deg, U, max_cluster)
    
    sub_panels = []
    for cl in clusters:
        if not cl: continue
        # Use bmat to compute sub-panel info directly
        sub_bmat = bmat[:, cl]  # n_rows x len(cl)
        row_nnz = sub_bmat.sum(axis=1)  # NNZ per row in this cluster
        active = row_nnz > 0
        if not np.any(active): continue
        sub_rows = [panel_rows[i] for i in range(len(panel_rows)) if active[i]]
        sub_nnz = int(row_nnz.sum())
        sub_cols = set(col_list[c] for c in cl)
        sub_panels.append((sub_rows, sub_cols, sub_nnz))
    return sub_panels

def split_by_pruning(bmat, col_list, panel_rows, max_cols=32):
    """Split using column pruning: keep top-max_cols columns by NNZ count."""
    col_nnz = bmat.sum(axis=0)  # NNZ per column
    top_idx = np.argsort(-col_nnz)[:max_cols]
    top_idx_sorted = np.sort(top_idx)
    
    # Sub-panel with top columns
    sub_bmat = bmat[:, top_idx_sorted]
    row_nnz = sub_bmat.sum(axis=1)
    active = row_nnz > 0
    if not np.any(active):
        return [(panel_rows, set(col_list[i] for i in top_idx_sorted), int(bmat.sum()))]
    
    sub_rows = [panel_rows[i] for i in range(len(panel_rows)) if active[i]]
    sub_nnz = int(row_nnz[active].sum())
    sub_cols = set(col_list[i] for i in top_idx_sorted)
    
    # Remaining columns - also form sub-panels via simple chunking
    remaining = np.setdiff1d(np.arange(len(col_list)), top_idx_sorted)
    result = [(sub_rows, sub_cols, sub_nnz)]
    
    # Chunk remaining columns into groups of max_cols
    for chunk_start in range(0, len(remaining), max_cols):
        chunk = remaining[chunk_start:chunk_start+max_cols]
        ch_bmat = bmat[:, chunk]
        ch_nnz = ch_bmat.sum(axis=1)
        ch_active = ch_nnz > 0
        if not np.any(ch_active): continue
        ch_rows = [panel_rows[i] for i in range(len(panel_rows)) if ch_active[i]]
        ch_total = int(ch_nnz[ch_active].sum())
        ch_cols = set(col_list[i] for i in chunk)
        result.append((ch_rows, ch_cols, ch_total))
    
    return result

def hybrid_post_split(panel_rows, panel_cols, panel_nnz, indptr, indices,
                      max_U=32, cluster_threshold=500):
    """
    Hybrid strategy:
      U <= max_U:              keep
      max_U < U <= threshold:  local clustering split
      U > threshold:           column pruning + chunking
    """
    U = len(panel_cols)
    if U <= max_U:
        return [(panel_rows, panel_cols, panel_nnz)]
    
    bmat, col_list, _ = build_bmat(panel_rows, indptr, indices)
    
    if U <= cluster_threshold:
        return split_by_clustering(bmat, col_list, panel_rows, max_cluster=max_U)
    else:
        return split_by_pruning(bmat, col_list, panel_rows, max_cols=max_U)

# ============================================================
# Metrics
# ============================================================
def compute_metrics(panels, total_nnz, label=""):
    if not panels:
        print(f"  [{label}] No panels"); return {}
    n=len(panels); tt=0; tg=0; tc=0; fills=[]; Us=[]; u32=0
    for pr,pc,pn in panels:
        nr=len(pr); U=len(pc); t=(U+31)//32
        tt+=t; tg+=U; tc+=pn
        f=pn/(nr*U) if nr*U>0 else 0
        fills.append(f); Us.append(U)
        if U<=32: u32+=1
    af=np.mean(fills)*100; au=np.mean(Us); p32=u32/n*100
    cp=tc/total_nnz*100; fb=total_nnz-tc
    te=tc/tt if tt else 0; ge=tc/tg if tg else 0
    gbw=tg*128*2/1e6
    f16=sum(1 for r,c,nn in panels if len(r)==16)
    ar=np.mean([len(r) for r,c,n in panels])
    print(f"  [{label}]")
    print(f"    Panels: {n} (full-16: {f16}), Avg rows: {ar:.1f}")
    print(f"    NNZ covered: {tc:,} ({cp:.1f}%), Fallback: {fb:,} ({100-cp:.1f}%)")
    print(f"    Tiles: {tt:,}, Gathers: {tg:,}, GathBW: {gbw:.1f} MB")
    print(f"    Tile eff: {te:.1f}, Gather eff: {ge:.2f}")
    print(f"    Fill: {af:.1f}%, U: {au:.1f}, U<=32: {p32:.1f}%")
    sys.stdout.flush()
    return {'panels':n,'covered':tc,'cov_pct':cp,'fallback':fb,
            'tiles':tt,'gathers':tg,'tile_eff':te,'gather_eff':ge,
            'gather_bw':gbw,'avg_fill':af,'avg_U':au,'u_le32':p32}

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

    # Distribution of U
    u_vals = [len(c) for r,c,n in anchor]
    u_arr = np.array(u_vals)
    print(f"\n  U distribution:")
    print(f"    U<=32: {np.sum(u_arr<=32)} ({np.mean(u_arr<=32)*100:.1f}%)")
    print(f"    33-100: {np.sum((u_arr>32)&(u_arr<=100))} ({np.mean((u_arr>32)&(u_arr<=100))*100:.1f}%)")
    print(f"    101-500: {np.sum((u_arr>100)&(u_arr<=500))} ({np.mean((u_arr>100)&(u_arr<=500))*100:.1f}%)")
    print(f"    501-2000: {np.sum((u_arr>500)&(u_arr<=2000))} ({np.mean((u_arr>500)&(u_arr<=2000))*100:.1f}%)")
    print(f"    >2000: {np.sum(u_arr>2000)} ({np.mean(u_arr>2000)*100:.1f}%)")
    sys.stdout.flush()

    # Hybrid post-split with multiple thresholds
    for thresh in [200, 500, 99999]:
        label = f"Hybrid-PS(thr={thresh})" if thresh < 99999 else "Full-Clustering"
        print(f"\n  === {label} ==="); sys.stdout.flush()
        t0 = time.time()
        post = []
        n_kept=0; n_cluster=0; n_prune=0
        for i,(pr,pc,pn) in enumerate(anchor):
            U = len(pc)
            if U <= 32:
                post.append((pr,pc,pn)); n_kept+=1
            elif U <= thresh:
                sub = hybrid_post_split(pr,pc,pn,indptr,indices,max_U=32,cluster_threshold=99999)
                post.extend(sub); n_cluster+=1
            else:
                sub = hybrid_post_split(pr,pc,pn,indptr,indices,max_U=32,cluster_threshold=0)
                post.extend(sub); n_prune+=1
            if (i+1)%5000==0:
                el=time.time()-t0
                rt=i/el if el>0 else 1
                eta=(len(anchor)-i)/rt if rt>0 else 0
                print(f"    {i+1}/{len(anchor)} ({el:.0f}s, ETA {eta:.0f}s) kept={n_kept} cluster={n_cluster} prune={n_prune}")
                sys.stdout.flush()

        elapsed=time.time()-t0
        print(f"  Done: kept={n_kept} cluster={n_cluster} prune={n_prune} -> {len(post)} panels ({elapsed:.1f}s)")
        mp = compute_metrics(post, nnz, label)
        perf_model(ma, mp, "Anchor", label)

    # Summary
    print(f"\n  {'='*70}")
    print(f"  DONE: {name}")
    print(f"  {'='*70}")
    sys.stdout.flush()

if __name__ == "__main__":
    data_dir = os.path.expanduser("~/data/SpMM_project/data")
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
            try: process_matrix(name, path)
            except Exception as e:
                print(f"ERROR on {name}: {e}")
                import traceback; traceback.print_exc()
        else: print(f"SKIP: {name}")
    print(f"\n{'#'*70}\n# ALL DONE\n{'#'*70}")
