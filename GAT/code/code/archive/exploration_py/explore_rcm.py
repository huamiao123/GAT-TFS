#!/usr/bin/env python3
"""
RCM (Reverse Cuthill-McKee) row reordering + Column-Anchor.

Hypothesis:
  RCM makes non-zeros cluster near diagonal
  → adjacent rows share more columns
  → column-anchor panels have smaller U
  → cross-panel overlap also increases (better cache reuse)

Experiment:
  1. Load matrix A
  2. Apply RCM reordering to A
  3. Run column-anchor on both original and RCM-reordered A
  4. Compare: U, fill, gather, cross-panel overlap
"""

import numpy as np
import struct
import time, sys, os
from scipy.sparse import csr_matrix, csc_matrix
from scipy.sparse.csgraph import reverse_cuthill_mckee
from collections import defaultdict

def read_csrbin(fn):
    with open(fn,'rb') as f:
        struct.unpack('III',f.read(12))
        nr,nc,nz=struct.unpack('QQQ',f.read(24))
        ip=np.frombuffer(f.read(4*(nr+1)),dtype=np.uint32).copy()
        idx=np.frombuffer(f.read(4*nz),dtype=np.uint32).copy()
    print(f"  Read: {nr:,} rows, {nc:,} cols, {nz:,} NNZ")
    return nr,nc,nz,ip,idx

def build_csc(rows,cols,indptr,indices):
    cc=np.zeros(cols,dtype=np.int32)
    for j in indices: cc[j]+=1
    cp=np.zeros(cols+1,dtype=np.int32); cp[1:]=np.cumsum(cc)
    ci=np.empty(len(indices),dtype=np.int32)
    pos=cp[:-1].copy()
    for r in range(rows):
        for i in range(indptr[r],indptr[r+1]):
            c=indices[i]; ci[pos[c]]=r; pos[c]+=1
    return cp,ci

def build_anchor_panels(rows,cols,indptr,indices,csc_ip,csc_idx,h=16):
    rd=np.diff(indptr.astype(np.int64))
    cd=np.diff(csc_ip.astype(np.int64))
    co=np.argsort(-cd); asgn=np.zeros(rows,dtype=bool); panels=[]
    for anc in co:
        if cd[anc]==0: break
        cand=[int(csc_idx[i]) for i in range(csc_ip[anc],csc_ip[anc+1]) if not asgn[csc_idx[i]]]
        if len(cand)<h: continue
        cand.sort(key=lambda r:rd[r]); sel=cand[:h]
        cs=set()
        for r in sel:
            for i in range(indptr[r],indptr[r+1]): cs.add(int(indices[i]))
        for r in sel: asgn[r]=True
        panels.append(cs)  # store column sets only
    return panels

def metrics(panels, total_nnz, indptr, label):
    if not panels:
        print(f"  [{label}] No panels"); return {}
    n=len(panels); tt=0; tg=0; fs=[]; Us=[]; u32=0
    for pc in panels:
        U=len(pc); t=(U+31)//32
        tt+=t; tg+=U; Us.append(U)
        if U<=32: u32+=1
    au=np.mean(Us); p32=u32/n*100; gbw=tg*128*2/1e6
    print(f"  [{label}]")
    print(f"    Panels: {n}")
    print(f"    Tiles: {tt:,}, Gathers: {tg:,}, GathBW: {gbw:.1f} MB")
    print(f"    Avg U: {au:.1f}, Med U: {np.median(Us):.0f}, U<=32: {p32:.1f}%")
    print(f"    P90 U: {np.percentile(Us,90):.0f}, Max U: {max(Us)}")
    sys.stdout.flush()
    return {'panels':n, 'tiles':tt, 'gathers':tg, 'gather_bw':gbw,
            'avg_U':au, 'u_le32':p32}

def cross_panel_overlap(panels, label):
    """Measure adjacent panel column overlap."""
    n = len(panels)
    if n < 2: return
    overlaps = []
    total_naive = sum(len(cs) for cs in panels)
    total_cached = len(panels[0])
    for i in range(1, n):
        ov = len(panels[i] & panels[i-1])
        overlaps.append(ov)
        total_cached += len(panels[i]) - ov
    savings = (1 - total_cached / total_naive) * 100
    print(f"  [{label}] Cross-panel overlap:")
    print(f"    Adjacent overlap: mean={np.mean(overlaps):.1f}, "
          f"med={np.median(overlaps):.0f}, P90={np.percentile(overlaps,90):.0f}")
    print(f"    1-cache savings: {savings:.1f}%")

    # 5-panel cache
    total_5 = len(panels[0])
    for i in range(1, n):
        cache = set()
        for j in range(max(0, i-5), i):
            cache |= panels[j]
        total_5 += len(panels[i] - cache)
    sav5 = (1 - total_5 / total_naive) * 100
    print(f"    5-cache savings: {sav5:.1f}%")
    sys.stdout.flush()

def greedy_reorder_sample(panels, max_sample=8000):
    """Quick greedy reorder on a sample."""
    n = len(panels)
    if n > max_sample:
        idx = np.random.choice(n, max_sample, replace=False)
        idx.sort()
        sampled = [panels[i] for i in idx]
    else:
        sampled = panels

    ns = len(sampled)
    # Build inverted index
    col_to_panels = defaultdict(list)
    for pi, cs in enumerate(sampled):
        for c in cs:
            col_to_panels[c].append(pi)

    visited = np.zeros(ns, dtype=bool)
    order = [0]; visited[0] = True
    for step in range(1, ns):
        cur_cols = sampled[order[-1]]
        candidates = set()
        for c in cur_cols:
            for pi in col_to_panels[c]:
                if not visited[pi]: candidates.add(pi)
        best_pi = -1; best_ov = -1
        if candidates:
            for pi in candidates:
                ov = len(cur_cols & sampled[pi])
                if ov > best_ov: best_ov = ov; best_pi = pi
        if best_pi == -1:
            for pi in range(ns):
                if not visited[pi]: best_pi = pi; break
        order.append(best_pi); visited[best_pi] = True

    reordered = [sampled[i] for i in order]
    return reordered

# ============================================================
# Main
# ============================================================
def process(name, path):
    print(f"\n{'#'*70}")
    print(f"# {name}")
    print(f"{'#'*70}"); sys.stdout.flush()

    rows,cols,nnz,indptr,indices = read_csrbin(path)
    rd = np.diff(indptr.astype(np.int64))
    print(f"  Degree: mean={rd.mean():.1f}, P99={np.percentile(rd,99):.0f}")

    # ---- Original matrix ----
    print(f"\n  === Original Matrix ===")
    t0=time.time()
    csc_ip, csc_idx = build_csc(rows, cols, indptr, indices)
    print(f"  CSC: {time.time()-t0:.1f}s")

    t0=time.time()
    orig_panels = build_anchor_panels(rows, cols, indptr, indices, csc_ip, csc_idx)
    print(f"  Anchor: {len(orig_panels)} panels ({time.time()-t0:.1f}s)")
    mo = metrics(orig_panels, nnz, indptr, "Original")
    cross_panel_overlap(orig_panels, "Original-sequential")

    # Greedy reorder
    print(f"\n  Greedy reorder (original)...")
    t0=time.time()
    reord_orig = greedy_reorder_sample(orig_panels)
    print(f"  Reorder: {time.time()-t0:.1f}s")
    cross_panel_overlap(reord_orig, "Original-greedy-reordered")

    # ---- RCM reordering ----
    print(f"\n  === RCM Reordering ==="); sys.stdout.flush()
    t0=time.time()

    # Build scipy sparse matrix (symmetric for RCM)
    vals = np.ones(nnz, dtype=np.float32)
    A_scipy = csr_matrix((vals, indices.astype(np.int32), indptr.astype(np.int32)),
                         shape=(rows, cols))
    # Make symmetric for RCM (A + A^T)
    A_sym = A_scipy + A_scipy.T
    A_sym.data[:] = 1.0  # binary

    print(f"  Symmetric matrix built ({time.time()-t0:.1f}s)")
    sys.stdout.flush()

    t0=time.time()
    perm = reverse_cuthill_mckee(A_sym, symmetric_mode=True)
    print(f"  RCM permutation computed ({time.time()-t0:.1f}s)")
    sys.stdout.flush()

    # Apply permutation: reorder both rows and columns
    # New row i = old row perm[i]
    # Also reorder columns to keep symmetry
    t0=time.time()
    inv_perm = np.empty(rows, dtype=np.int64)
    inv_perm[perm] = np.arange(rows)

    # Build reordered CSR
    new_indptr = np.zeros(rows+1, dtype=np.int64)
    for new_r in range(rows):
        old_r = perm[new_r]
        new_indptr[new_r+1] = int(indptr[old_r+1]) - int(indptr[old_r])
    new_indptr = np.cumsum(new_indptr).astype(np.uint32)

    new_nnz = int(new_indptr[-1])
    new_indices = np.empty(new_nnz, dtype=np.uint32)
    for new_r in range(rows):
        old_r = perm[new_r]
        old_start = int(indptr[old_r])
        old_end = int(indptr[old_r+1])
        new_start = int(new_indptr[new_r])
        length = old_end - old_start
        # Remap column indices
        old_cols = indices[old_start:old_end].astype(np.int64)
        new_cols = inv_perm[old_cols]
        new_indices[new_start:new_start+length] = new_cols.astype(np.uint32)

    print(f"  Reordered matrix built ({time.time()-t0:.1f}s)")
    sys.stdout.flush()

    # Verify
    rcm_rd = np.diff(new_indptr.astype(np.int64))
    print(f"  RCM degree: mean={rcm_rd.mean():.1f} (should match original)")

    # Measure bandwidth (how spread out are non-zeros)
    bandwidths = []
    for r in range(min(rows, 100000)):
        start, end = int(new_indptr[r]), int(new_indptr[r+1])
        if start < end:
            cols_r = new_indices[start:end].astype(np.int64)
            bw = cols_r.max() - cols_r.min()
            bandwidths.append(bw)
    orig_bws = []
    for r in range(min(rows, 100000)):
        start, end = int(indptr[r]), int(indptr[r+1])
        if start < end:
            cols_r = indices[start:end].astype(np.int64)
            bw = cols_r.max() - cols_r.min()
            orig_bws.append(bw)
    print(f"  Bandwidth (first 100K rows):")
    print(f"    Original: mean={np.mean(orig_bws):.0f}, med={np.median(orig_bws):.0f}")
    print(f"    RCM:      mean={np.mean(bandwidths):.0f}, med={np.median(bandwidths):.0f}")

    # Build CSC for RCM matrix
    t0=time.time()
    rcm_cols = cols  # same number of columns
    rcm_csc_ip, rcm_csc_idx = build_csc(rows, rcm_cols, new_indptr, new_indices)
    print(f"  RCM CSC: {time.time()-t0:.1f}s")

    # Run column-anchor on RCM matrix
    t0=time.time()
    rcm_panels = build_anchor_panels(rows, rcm_cols, new_indptr, new_indices,
                                      rcm_csc_ip, rcm_csc_idx)
    print(f"  RCM Anchor: {len(rcm_panels)} panels ({time.time()-t0:.1f}s)")
    mr = metrics(rcm_panels, nnz, new_indptr, "RCM")
    cross_panel_overlap(rcm_panels, "RCM-sequential")

    # Greedy reorder on RCM panels
    print(f"\n  Greedy reorder (RCM)...")
    t0=time.time()
    reord_rcm = greedy_reorder_sample(rcm_panels)
    print(f"  Reorder: {time.time()-t0:.1f}s")
    cross_panel_overlap(reord_rcm, "RCM-greedy-reordered")

    # ---- Comparison ----
    print(f"\n  {'='*60}")
    print(f"  COMPARISON: {name}")
    print(f"  {'='*60}")
    if mo and mr:
        rg = mr['gathers']/mo['gathers'] if mo['gathers'] else 1
        rt = mr['tiles']/mo['tiles'] if mo['tiles'] else 1
        print(f"    Gathers: {mo['gathers']:,} → {mr['gathers']:,} ({rg:.3f}x)")
        print(f"    Tiles:   {mo['tiles']:,} → {mr['tiles']:,} ({rt:.3f}x)")
        print(f"    Avg U:   {mo['avg_U']:.1f} → {mr['avg_U']:.1f}")
        print(f"    GathBW:  {mo['gather_bw']:.1f} → {mr['gather_bw']:.1f} MB")
    print(f"  {'='*60}")
    sys.stdout.flush()

if __name__ == "__main__":
    dd = os.path.expanduser("~/data/SpMM_project/data")
    for nm in ["web-Google","amazon0601","hollywood-2009","soc-Pokec"]:
        p = f"{dd}/{nm}/{nm}.csrbin"
        if os.path.exists(p):
            try: process(nm,p)
            except Exception as e:
                print(f"ERROR {nm}: {e}")
                import traceback; traceback.print_exc()
        else: print(f"SKIP: {nm}")
    print(f"\n{'#'*70}\n# ALL DONE\n{'#'*70}")
