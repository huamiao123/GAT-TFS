#!/usr/bin/env python3
"""
Scheduling Strategy Comparison (v2 - O(1) LRU, correct panels)
8 strategies x 7 matrices, fast OrderedDict cache simulation
"""
import struct, os, time, sys
import numpy as np
from collections import Counter, defaultdict, OrderedDict

DATA = os.path.expanduser("~/data/SpMM_project/data")
MATRICES = [
    ("web-Google",      "web-Google/web-Google.csrbin"),
    ("amazon0601",      "amazon0601/amazon0601.csrbin"),
    ("soc-Pokec",       "soc-Pokec/soc-Pokec.csrbin"),
    ("hollywood-2009",  "hollywood-2009/hollywood-2009.csrbin"),
    ("cit-Patents",     "cit-Patents/cit-Patents.csrbin"),
    ("as-Skitter",      "as-Skitter/as-Skitter.csrbin"),
    ("indochina-2004",  "indochina-2004/indochina-2004.csrbin"),
]
TILE_R = 16
N_THREADS = 64
L2_CACHE = 2000

def load_csr(fp):
    with open(fp,'rb') as f:
        f.read(12)  # ptype,dtype,vtype
        nrow,ncol,nnz = struct.unpack('<QQQ',f.read(24))
        indptr = np.frombuffer(f.read((nrow+1)*4),dtype=np.uint32).astype(np.int64)
        indices = np.frombuffer(f.read(nnz*4),dtype=np.uint32).astype(np.int64)
    return nrow,ncol,nnz,indptr,indices

def build_panels_v17c(nrow,ncol,nnz,indptr,indices):
    col_to_rows = defaultdict(list)
    for r in range(nrow):
        for idx in range(indptr[r],indptr[r+1]):
            col_to_rows[indices[idx]].append(r)
    col_freq = sorted(col_to_rows.items(), key=lambda x:-len(x[1]))
    assigned = set(); panels = []; row_cols = {}
    for r in range(nrow):
        row_cols[r] = set(indices[indptr[r]:indptr[r+1]].tolist())
    for anchor_col, rows_list in col_freq:
        if len(rows_list) < TILE_R: continue
        avail = [r for r in rows_list if r not in assigned]
        if len(avail) < TILE_R: continue
        avail_deg = [(r,indptr[r+1]-indptr[r]) for r in avail]
        avail_deg.sort(key=lambda x:x[1])
        selected = [r for r,d in avail_deg[:TILE_R]]
        col_union = set()
        for r in selected: col_union |= row_cols[r]
        panels.append((int(anchor_col), selected, col_union))
        assigned.update(selected)
    return panels

# ============ Fast LRU using OrderedDict ============
def lru_sim_fast(panel_indices, panels, cache_size):
    """O(1) per-operation LRU cache simulation using OrderedDict."""
    cache = OrderedDict()  # key -> None, order = access order
    misses = 0
    for pidx in panel_indices:
        for col in panels[pidx][2]:
            if col in cache:
                cache.move_to_end(col)  # O(1) move to most recent
            else:
                misses += 1
                if len(cache) >= cache_size:
                    cache.popitem(last=False)  # O(1) evict LRU
                cache[col] = None
    return misses

def eval_schedule(assignment, panels, cache_size):
    """Evaluate a schedule: per-thread LRU, return total misses + imbalance."""
    thread_misses = []
    thread_gathers = []
    for thread_panels in assignment:
        if not thread_panels:
            thread_misses.append(0)
            thread_gathers.append(0)
            continue
        m = lru_sim_fast(thread_panels, panels, cache_size)
        g = sum(len(panels[p][2]) for p in thread_panels)
        thread_misses.append(m)
        thread_gathers.append(g)
    total_misses = sum(thread_misses)
    total_gather = sum(thread_gathers)
    # Imbalance = max_thread_time / avg_thread_time
    if thread_gathers:
        avg_g = np.mean([g for g in thread_gathers if g > 0])
        max_g = max(thread_gathers)
        imbalance = max_g / avg_g if avg_g > 0 else 0
    else:
        imbalance = 0
    return total_misses, total_gather, imbalance

# ============ 8 Scheduling Strategies ============

def sched_static(order, nt):
    """Static: contiguous chunks."""
    a = [[] for _ in range(nt)]
    ch = (len(order)+nt-1)//nt
    for i,pidx in enumerate(order):
        a[min(i//ch, nt-1)].append(pidx)
    return a

def sched_dynamic(order, nt, cs=8):
    """Dynamic(cs): round-robin chunks of cs."""
    a = [[] for _ in range(nt)]
    pos, t = 0, 0
    while pos < len(order):
        end = min(pos+cs, len(order))
        a[t].extend(order[pos:end])
        pos = end; t = (t+1)%nt
    return a

def sched_guided(order, nt, min_chunk=8):
    """Guided: decreasing chunk sizes."""
    a = [[] for _ in range(nt)]
    remaining = len(order); pos = 0; t = 0
    while pos < len(order):
        ch = max(min_chunk, remaining // (nt*2))
        end = min(pos+ch, len(order))
        a[t].extend(order[pos:end])
        remaining -= (end-pos); pos = end; t = (t+1)%nt
    return a

def sched_cost_balanced(order, panels, nt):
    """Cost-balanced static: assign panels to minimize max-cost thread."""
    costs = [len(panels[p][2]) for p in order]
    # LPT (Longest Processing Time first)
    sorted_idx = sorted(range(len(order)), key=lambda i:-costs[i])
    a = [[] for _ in range(nt)]
    thread_cost = [0]*nt
    for i in sorted_idx:
        t = min(range(nt), key=lambda x:thread_cost[x])
        a[t].append(order[i])
        thread_cost[t] += costs[i]
    return a

def sched_cost_balanced_contiguous(order, panels, nt):
    """Cost-balanced but contiguous: find cut points that equalize cost."""
    costs = [len(panels[p][2]) for p in order]
    total = sum(costs)
    target = total / nt
    a = [[] for _ in range(nt)]
    cur_t = 0; cur_cost = 0
    for i, pidx in enumerate(order):
        a[cur_t].append(pidx)
        cur_cost += costs[i]
        if cur_cost >= target and cur_t < nt-1:
            cur_t += 1; cur_cost = 0
    return a

def sched_static_large_chunk(order, nt):
    """Static with larger chunks (NUMA-friendly): 8 NUMA groups × 8 cores."""
    n_numa = 8; cores_per = nt // n_numa
    numa_chunk = (len(order)+n_numa-1)//n_numa
    a = [[] for _ in range(nt)]
    for ni in range(n_numa):
        s = ni*numa_chunk; e = min(s+numa_chunk, len(order))
        numa_panels = order[s:e]
        core_chunk = (len(numa_panels)+cores_per-1)//cores_per
        for ci in range(cores_per):
            cs = ci*core_chunk; ce = min(cs+core_chunk, len(numa_panels))
            tid = ni*cores_per + ci
            if tid < nt:
                a[tid].extend(numa_panels[cs:ce])
    return a

def sched_interleaved(order, nt):
    """Interleaved: thread t gets panels t, t+nt, t+2nt, ..."""
    a = [[] for _ in range(nt)]
    for i, pidx in enumerate(order):
        a[i%nt].append(pidx)
    return a

def sched_hot_cold(order, panels, nt):
    """Hot-Cold: heavy panels (large U) to first half of threads, light to second half."""
    costs = [(len(panels[p][2]), p) for p in order]
    median_cost = np.median([c for c,_ in costs])
    hot = [p for c,p in costs if c >= median_cost]
    cold = [p for c,p in costs if c < median_cost]
    half = nt // 2
    a = [[] for _ in range(nt)]
    # Distribute hot to first half (static)
    ch = (len(hot)+half-1)//half
    for i,p in enumerate(hot):
        a[min(i//ch, half-1)].append(p)
    # Distribute cold to second half (static)
    ch2 = (len(cold)+half-1)//half if half > 0 else 1
    for i,p in enumerate(cold):
        a[half + min(i//ch2, half-1)].append(p)
    return a

# ============ Main ============
print("="*70)
print("  Scheduling Strategy Comparison (v2 - fast LRU)")
print(f"  Threads={N_THREADS}, L2 cache={L2_CACHE}")
print("="*70)

summary = []

for mname, mrel in MATRICES:
    fp = os.path.join(DATA, mrel)
    if not os.path.exists(fp):
        print(f"\n  [{mname}] SKIP"); continue
    print(f"\n{'='*70}\n  {mname}\n{'='*70}"); sys.stdout.flush()
    
    t0 = time.time()
    nrow,ncol,nnz,indptr,indices = load_csr(fp)
    panels = build_panels_v17c(nrow,ncol,nnz,indptr,indices)
    print(f"  {nrow:,} rows, {nnz:,} NNZ, {len(panels):,} panels")
    
    total_gather = sum(len(p[2]) for p in panels)
    
    # Use original order (= AB order since they're the same)
    base_order = list(range(len(panels)))
    
    strategies = [
        ("static",              lambda o: sched_static(o, N_THREADS)),
        ("dynamic(8)",          lambda o: sched_dynamic(o, N_THREADS, 8)),
        ("dynamic(64)",         lambda o: sched_dynamic(o, N_THREADS, 64)),
        ("guided(8)",           lambda o: sched_guided(o, N_THREADS, 8)),
        ("cost-balanced-LPT",   lambda o: sched_cost_balanced(o, panels, N_THREADS)),
        ("cost-balanced-contig", lambda o: sched_cost_balanced_contiguous(o, panels, N_THREADS)),
        ("NUMA-static(8x8)",   lambda o: sched_static_large_chunk(o, N_THREADS)),
        ("interleaved",         lambda o: sched_interleaved(o, N_THREADS)),
        ("hot-cold",            lambda o: sched_hot_cold(o, panels, N_THREADS)),
    ]
    
    print(f"\n  {'Strategy':<24s} {'Misses':>14s} {'Save%':>8s} {'Imbal':>7s} {'Time':>6s}")
    
    mat_results = {}
    for sname, sfunc in strategies:
        t1 = time.time()
        assignment = sfunc(base_order)
        misses, tg, imbal = eval_schedule(assignment, panels, L2_CACHE)
        save = 1.0 - misses/total_gather if total_gather > 0 else 0
        elapsed = time.time()-t1
        print(f"  {sname:<24s} {misses:>14,d} {save:>7.1%} {imbal:>7.2f} {elapsed:>5.1f}s")
        mat_results[sname] = {'save': save, 'imbal': imbal, 'misses': misses}
        sys.stdout.flush()
    
    # Best: maximize save while keeping imbalance < 1.5
    balanced = [(s, r['save'], r['imbal']) for s,r in mat_results.items() if r['imbal'] < 1.5]
    if balanced:
        best = max(balanced, key=lambda x: x[1])
        print(f"  ** Best (imbal<1.5): {best[0]} ({best[1]:.1%} save, imbal={best[2]:.2f})")
    
    summary.append({'name': mname, 'results': mat_results})
    print(f"  Total: {time.time()-t0:.1f}s"); sys.stdout.flush()

# Cross-matrix summary
print(f"\n{'='*70}")
print("  CROSS-MATRIX SUMMARY")
print(f"{'='*70}")
all_strats = [s for s,_ in strategies]
print(f"  {'Matrix':<16s}", end="")
for s in all_strats:
    print(f" {s[:10]:>10s}", end="")
print()
for sr in summary:
    print(f"  {sr['name']:<16s}", end="")
    for s in all_strats:
        r = sr['results'].get(s, {})
        sv = r.get('save', 0)
        print(f" {sv:>9.1%}", end="")
    print()

print(f"\n  ALL DONE")
