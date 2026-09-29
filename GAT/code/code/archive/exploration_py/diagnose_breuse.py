#!/usr/bin/env python3
"""
Diagnose cross-panel B-reuse potential.

Questions to answer:
  1. Current panel order: how much column overlap between adjacent panels?
  2. Greedy reorder: how much overlap if we sort panels to maximize adjacency overlap?
  3. What's the theoretical savings in gather bandwidth?
  4. Which columns are "hot" (appear in many panels)?
"""

import numpy as np
import struct
import time, sys, os
from collections import defaultdict

def read_csrbin(fn):
    with open(fn,'rb') as f:
        struct.unpack('III',f.read(12))
        nr,nc,nz = struct.unpack('QQQ',f.read(24))
        ip = np.frombuffer(f.read(4*(nr+1)),dtype=np.uint32)
        idx = np.frombuffer(f.read(4*nz),dtype=np.uint32)
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
        panels.append(cs)  # only store column set for this analysis
    return panels

# ============================================================
# Analysis 1: Column hotness (how many panels use each column)
# ============================================================
def analyze_column_hotness(panels, cols):
    print(f"\n  === Analysis 1: Column Hotness ===")
    col_count = np.zeros(cols, dtype=np.int32)
    for cs in panels:
        for c in cs:
            col_count[c] += 1

    active = col_count > 0
    n_active = np.sum(active)
    print(f"    Active columns: {n_active:,} / {cols:,} ({n_active/cols*100:.1f}%)")
    print(f"    Col appearance: mean={col_count[active].mean():.1f}, "
          f"med={np.median(col_count[active]):.0f}, "
          f"max={col_count.max()}, "
          f"P99={np.percentile(col_count[active],99):.0f}")

    # Distribution
    for thresh in [1, 5, 10, 50, 100, 500, 1000]:
        n = np.sum(col_count >= thresh)
        if n > 0:
            print(f"    Cols in >={thresh} panels: {n:,} ({n/n_active*100:.1f}%)")

    # Top-10 hottest columns
    top10 = np.argsort(-col_count)[:10]
    print(f"    Top-10 hottest columns:")
    for c in top10:
        print(f"      col {c}: appears in {col_count[c]} panels ({col_count[c]/len(panels)*100:.1f}%)")

    sys.stdout.flush()
    return col_count

# ============================================================
# Analysis 2: Adjacent panel overlap in current order
# ============================================================
def analyze_current_order(panels):
    print(f"\n  === Analysis 2: Current Order Overlap ===")
    n = len(panels)
    overlaps = []
    overlap_ratios = []
    new_cols_list = []

    total_gather_naive = sum(len(cs) for cs in panels)

    # Simulate cache: assume previous panel's columns are cached
    total_gather_cached = len(panels[0])  # first panel: full gather
    for i in range(1, n):
        overlap = len(panels[i] & panels[i-1])
        new_cols = len(panels[i]) - overlap
        overlaps.append(overlap)
        overlap_ratios.append(overlap / len(panels[i]) if len(panels[i]) > 0 else 0)
        new_cols_list.append(new_cols)
        total_gather_cached += new_cols

    print(f"    Total panels: {n}")
    print(f"    Adjacent overlap: mean={np.mean(overlaps):.1f}, "
          f"med={np.median(overlaps):.0f}, "
          f"P90={np.percentile(overlaps,90):.0f}, "
          f"max={max(overlaps)}")
    print(f"    Overlap ratio: mean={np.mean(overlap_ratios)*100:.1f}%, "
          f"med={np.median(overlap_ratios)*100:.1f}%")
    print(f"    Naive gather (no cache):    {total_gather_naive:,}")
    print(f"    Cached gather (1-panel):    {total_gather_cached:,} "
          f"(saved {(1-total_gather_cached/total_gather_naive)*100:.1f}%)")

    # Simulate bigger cache: keep last K panels' columns
    for cache_k in [2, 5, 10, 20]:
        total_g = len(panels[0])
        for i in range(1, n):
            cache = set()
            for j in range(max(0, i-cache_k), i):
                cache |= panels[j]
            new = len(panels[i] - cache)
            total_g += new
        savings = (1 - total_g / total_gather_naive) * 100
        print(f"    Cached gather ({cache_k}-panel):   {total_g:,} (saved {savings:.1f}%)")

    sys.stdout.flush()
    return total_gather_naive, total_gather_cached

# ============================================================
# Analysis 3: Greedy nearest-neighbor reordering
# ============================================================
def greedy_reorder(panels, sample_limit=10000):
    """
    Reorder panels so adjacent panels share maximum columns.
    Greedy: always pick the unvisited panel with largest overlap to current.
    
    For large panel counts, we use approximate method:
    - Build inverted index (column -> panels)
    - For each panel, only check panels sharing at least 1 column
    """
    print(f"\n  === Analysis 3: Greedy Reorder ===")
    sys.stdout.flush()
    t0 = time.time()
    n = len(panels)

    if n > sample_limit:
        print(f"    Too many panels ({n}), sampling {sample_limit}")
        # Sample panels for reordering analysis
        idx_sample = np.random.choice(n, sample_limit, replace=False)
        idx_sample.sort()
        sampled_panels = [panels[i] for i in idx_sample]
    else:
        sampled_panels = panels
        sample_limit = n

    ns = len(sampled_panels)

    # Build inverted index: column -> list of panel indices
    col_to_panels = defaultdict(list)
    for pi, cs in enumerate(sampled_panels):
        for c in cs:
            col_to_panels[c].append(pi)

    # Greedy nearest neighbor
    visited = np.zeros(ns, dtype=bool)
    order = [0]
    visited[0] = True

    for step in range(1, ns):
        cur = order[-1]
        cur_cols = sampled_panels[cur]

        # Find candidates: panels sharing columns with current
        candidates = set()
        for c in cur_cols:
            for pi in col_to_panels[c]:
                if not visited[pi]:
                    candidates.add(pi)

        best_pi = -1
        best_overlap = -1

        if candidates:
            # Check overlap with candidates
            for pi in candidates:
                ov = len(cur_cols & sampled_panels[pi])
                if ov > best_overlap:
                    best_overlap = ov
                    best_pi = pi
        
        if best_pi == -1:
            # No overlapping panel found, pick any unvisited
            for pi in range(ns):
                if not visited[pi]:
                    best_pi = pi
                    best_overlap = 0
                    break

        order.append(best_pi)
        visited[best_pi] = True

        if (step+1) % 2000 == 0:
            print(f"    ... reordered {step+1}/{ns} ({time.time()-t0:.0f}s)")
            sys.stdout.flush()

    elapsed = time.time() - t0
    print(f"    Reorder done ({elapsed:.1f}s)")

    # Measure overlap in new order
    reordered = [sampled_panels[i] for i in order]
    overlaps = []
    total_naive = sum(len(cs) for cs in reordered)
    total_cached = len(reordered[0])
    for i in range(1, len(reordered)):
        ov = len(reordered[i] & reordered[i-1])
        overlaps.append(ov)
        total_cached += len(reordered[i]) - ov

    print(f"    Reordered overlap: mean={np.mean(overlaps):.1f}, "
          f"med={np.median(overlaps):.0f}, "
          f"P90={np.percentile(overlaps,90):.0f}")
    print(f"    Naive gather:     {total_naive:,}")
    print(f"    Reordered cached: {total_cached:,} "
          f"(saved {(1-total_cached/total_naive)*100:.1f}%)")

    # Bigger cache on reordered
    for cache_k in [2, 5, 10]:
        total_g = len(reordered[0])
        for i in range(1, len(reordered)):
            cache = set()
            for j in range(max(0, i-cache_k), i):
                cache |= reordered[j]
            new = len(reordered[i] - cache)
            total_g += new
        savings = (1 - total_g / total_naive) * 100
        print(f"    Reordered {cache_k}-cache: {total_g:,} (saved {savings:.1f}%)")

    sys.stdout.flush()
    return order, overlaps

# ============================================================
# Analysis 4: Theoretical potential
# ============================================================
def analyze_theoretical(panels, col_count, cols):
    print(f"\n  === Analysis 4: Theoretical Potential ===")
    n = len(panels)
    total_gather = sum(len(cs) for cs in panels)
    unique_cols = np.sum(col_count > 0)

    print(f"    Total gather (naive):    {total_gather:,}")
    print(f"    Unique columns used:     {unique_cols:,}")
    print(f"    Avg times each col gathered: {total_gather/unique_cols:.1f}")
    print(f"    Theoretical minimum (perfect cache): {unique_cols:,} "
          f"(saved {(1-unique_cols/total_gather)*100:.1f}%)")
    print(f"    → This is the absolute lower bound:")
    print(f"      each column gathered exactly once, never evicted from cache")

    # More realistic: L2 cache can hold ~X columns of B
    # B column = K * 2 bytes = 128 * 2 = 256 bytes
    # L2 = 2 MB per core, usable ~1 MB
    # → can cache ~4000 columns of B
    l2_cols = 4000
    print(f"\n    L2 cache model (~{l2_cols} B-columns cacheable):")

    # Simulate LRU cache
    from collections import OrderedDict
    
    for cache_size in [1000, 2000, 4000, 8000, 16000]:
        cache = OrderedDict()
        cache_hits = 0
        cache_misses = 0
        
        for pi, cs in enumerate(panels):
            for c in sorted(cs):  # sorted for determinism
                if c in cache:
                    cache.move_to_end(c)
                    cache_hits += 1
                else:
                    cache_misses += 1
                    cache[c] = True
                    if len(cache) > cache_size:
                        cache.popitem(last=False)
        
        hit_rate = cache_hits / (cache_hits + cache_misses) * 100
        effective_gather = cache_misses
        savings = (1 - effective_gather / total_gather) * 100
        print(f"      Cache={cache_size:>6d} cols: hits={hit_rate:.1f}%, "
              f"effective_gather={effective_gather:,} (saved {savings:.1f}%)")

    sys.stdout.flush()

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

    t0=time.time()
    csc_ip,csc_idx = build_csc(rows,cols,indptr,indices)
    print(f"  CSC: {time.time()-t0:.1f}s"); sys.stdout.flush()

    print(f"\n  Building panels...")
    t0=time.time()
    panels = build_anchor_panels(rows,cols,indptr,indices,csc_ip,csc_idx)
    print(f"  {len(panels)} panels ({time.time()-t0:.1f}s)")
    total_g = sum(len(cs) for cs in panels)
    avg_u = np.mean([len(cs) for cs in panels])
    print(f"  Total gathers: {total_g:,}, Avg U: {avg_u:.1f}")
    sys.stdout.flush()

    # Run all analyses
    col_count = analyze_column_hotness(panels, cols)
    analyze_current_order(panels)
    greedy_reorder(panels)
    analyze_theoretical(panels, col_count, cols)

    print(f"\n  {'='*70}")
    print(f"  DONE: {name}")
    print(f"  {'='*70}")
    sys.stdout.flush()

if __name__ == "__main__":
    dd = os.path.expanduser("~/data/SpMM_project/data")
    for nm in ["hollywood-2009","soc-Pokec","web-Google","amazon0601"]:
        p = f"{dd}/{nm}/{nm}.csrbin"
        if os.path.exists(p):
            try: process(nm,p)
            except Exception as e:
                print(f"ERROR {nm}: {e}")
                import traceback; traceback.print_exc()
        else: print(f"SKIP: {nm}")
    print(f"\n{'#'*70}\n# ALL DONE\n{'#'*70}")
