#!/usr/bin/env python3
"""
NUMA-aware panel partitioning simulation.
Compare: (1) global AB sort + static split
         (2) NUMA-aware: cluster panels into 8 groups by column similarity, per-group AB sort
Simulates per-NUMA-node L3 cache (9.4MB = ~36K B-rows for K=128 BF16).
"""
import numpy as np
import struct, os, time, sys
from collections import defaultdict

DATA = os.path.expanduser("~/data/SpMM_project/data")
MATRICES = ["hollywood-2009", "web-Google", "amazon0601", "soc-Pokec"]
H = 16
N_NUMA = 8        # NUMA nodes
CORES_PER_NUMA = 8
N_THREADS = N_NUMA * CORES_PER_NUMA  # 64
L3_PER_NUMA_ROWS = 36000   # ~9.4MB / 256B per row
L2_PER_CORE_ROWS = 8000    # ~2MB / 256B per row

def read_csrbin(path):
    with open(path, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('III', f.read(12))
        nrow, ncol, nnz = struct.unpack('QQQ', f.read(24))
        indptr = np.frombuffer(f.read((nrow+1)*4), dtype=np.uint32).copy()
        indices = np.frombuffer(f.read(nnz*4), dtype=np.uint32).copy()
    return nrow, ncol, nnz, indptr, indices

def build_panels(nrow, ncol, indptr, indices, h=16):
    col_deg = np.zeros(ncol, dtype=np.int64)
    for i in range(len(indices)):
        col_deg[indices[i]] += 1
    col_order = np.argsort(-col_deg)
    assigned = np.zeros(nrow, dtype=bool)
    panels = []
    col_to_rows = defaultdict(list)
    for r in range(nrow):
        for j in range(indptr[r], indptr[r+1]):
            col_to_rows[indices[j]].append(r)
    for anchor in col_order:
        if col_deg[anchor] == 0:
            continue
        candidates = [r for r in col_to_rows[anchor] if not assigned[r]]
        if not candidates:
            continue
        for start in range(0, len(candidates), h):
            batch = candidates[start:start+h]
            if not batch:
                break
            col_set = set()
            for r in batch:
                for j in range(indptr[r], indptr[r+1]):
                    col_set.add(int(indices[j]))
            panels.append((int(anchor), batch, col_set))
            for r in batch:
                assigned[r] = True
    return panels

def anchor_bucket_sort(panels):
    buckets = defaultdict(list)
    for i, (anchor, rows, cols) in enumerate(panels):
        buckets[anchor].append(i)
    sorted_buckets = sorted(buckets.values(), key=len, reverse=True)
    result = []
    for bucket in sorted_buckets:
        if len(bucket) <= 1:
            result.extend(bucket)
            continue
        remaining = set(bucket)
        cur = bucket[0]
        remaining.remove(cur)
        result.append(cur)
        while remaining:
            best, best_ov = None, -1
            cur_cols = panels[cur][2]
            for idx in remaining:
                ov = len(cur_cols & panels[idx][2])
                if ov > best_ov:
                    best_ov = ov
                    best = idx
            result.append(best)
            remaining.remove(best)
            cur = best
    return result

def lru_sim(order, panels, cache_size):
    cache = {}
    t = 0
    misses = 0
    for pidx in order:
        for col in panels[pidx][2]:
            if col in cache:
                cache[col] = t
            else:
                misses += 1
                if len(cache) >= cache_size:
                    lru = min(cache, key=cache.get)
                    del cache[lru]
                cache[col] = t
            t += 1
    return misses

def partition_panels_greedy(panels, order, n_groups):
    """Partition sorted panels into n_groups by greedy column-similarity clustering."""
    n = len(order)
    # Simple: k-way partition minimizing inter-group column overlap loss
    # Use: assign each panel to the group whose existing column set has max overlap
    groups = [[] for _ in range(n_groups)]
    group_cols = [set() for _ in range(n_groups)]
    
    for pidx in order:
        cols = panels[pidx][2]
        best_g, best_ov = 0, -1
        for g in range(n_groups):
            ov = len(cols & group_cols[g])
            if ov > best_ov:
                best_ov = ov
                best_g = g
        # Check load balance: don't overfill
        min_size = min(len(g) for g in groups)
        max_size = max(len(g) for g in groups)
        if max_size > min_size + n // n_groups:
            # Force assign to smallest group
            best_g = min(range(n_groups), key=lambda g: len(groups[g]))
        groups[best_g].append(pidx)
        group_cols[best_g].update(cols)
    
    return groups

# ============================================================
print("=" * 70)
print("  NUMA-aware Panel Partitioning Simulation")
print(f"  {N_NUMA} NUMA nodes × {CORES_PER_NUMA} cores")
print(f"  L3/NUMA: {L3_PER_NUMA_ROWS} rows, L2/core: {L2_PER_CORE_ROWS} rows")
print("=" * 70)

for mname in MATRICES:
    csrbin = os.path.join(DATA, mname, mname + ".csrbin")
    if not os.path.exists(csrbin):
        print(f"  [{mname}] SKIP")
        continue
    
    print(f"\n{'='*70}")
    print(f"  {mname}")
    print(f"{'='*70}")
    sys.stdout.flush()
    
    t0 = time.time()
    nrow, ncol, nnz, indptr, indices = read_csrbin(csrbin)
    panels = build_panels(nrow, ncol, indptr, indices, H)
    print(f"  {nrow:,} rows, {nnz:,} NNZ, {len(panels):,} panels")
    
    total_gather_naive = sum(len(p[2]) for p in panels)
    print(f"  Total naive gather: {total_gather_naive:,}")
    sys.stdout.flush()
    
    # Strategy 1: Global AB sort + static chunk split
    ab_order = anchor_bucket_sort(panels)
    print(f"  AB sort: {time.time()-t0:.1f}s")
    sys.stdout.flush()
    
    chunk = (len(ab_order) + N_THREADS - 1) // N_THREADS
    strat1_misses = 0
    strat1_numa_misses = [0] * N_NUMA
    for t_id in range(N_THREADS):
        s = t_id * chunk
        e = min(s + chunk, len(ab_order))
        if s >= len(ab_order):
            break
        thread_order = ab_order[s:e]
        m = lru_sim(thread_order, panels, L2_PER_CORE_ROWS)
        strat1_misses += m
        strat1_numa_misses[t_id // CORES_PER_NUMA] += m
    
    # Also simulate L3 level (per NUMA node)
    numa_chunk = (len(ab_order) + N_NUMA - 1) // N_NUMA
    strat1_l3_misses = 0
    for n_id in range(N_NUMA):
        s = n_id * numa_chunk
        e = min(s + numa_chunk, len(ab_order))
        if s >= len(ab_order):
            break
        numa_order = ab_order[s:e]
        m = lru_sim(numa_order, panels, L3_PER_NUMA_ROWS)
        strat1_l3_misses += m
    
    print(f"\n  --- Strategy 1: Global AB + static split ---")
    print(f"  L2 misses (per-core): {strat1_misses:,} ({strat1_misses/total_gather_naive*100:.1f}%)")
    print(f"  L3 misses (per-NUMA): {strat1_l3_misses:,} ({strat1_l3_misses/total_gather_naive*100:.1f}%)")
    l2_save1 = 1 - strat1_misses / total_gather_naive
    l3_save1 = 1 - strat1_l3_misses / total_gather_naive
    print(f"  L2 gather saved: {l2_save1:.1%}")
    print(f"  L3 gather saved: {l3_save1:.1%}")
    # Load balance
    imb = max(strat1_numa_misses) / (sum(strat1_numa_misses)/N_NUMA) if sum(strat1_numa_misses) > 0 else 0
    print(f"  NUMA load imbalance: {imb:.3f}")
    sys.stdout.flush()
    
    # Strategy 2: NUMA-aware partition + per-group AB sort
    t1 = time.time()
    numa_groups = partition_panels_greedy(panels, ab_order, N_NUMA)
    
    # Within each NUMA group, do AB sort
    strat2_misses = 0
    strat2_l3_misses = 0
    strat2_numa_misses = [0] * N_NUMA
    strat2_numa_cols = []
    
    for n_id in range(N_NUMA):
        group = numa_groups[n_id]
        if not group:
            strat2_numa_cols.append(0)
            continue
        # Intra-group AB sort
        sub_panels_map = {i: panels[i] for i in group}
        # Simple: just sort by anchor then by column overlap
        group_sorted = sorted(group, key=lambda i: panels[i][0])
        # Greedy within group
        if len(group_sorted) > 1:
            remaining = set(group_sorted)
            cur = group_sorted[0]
            remaining.remove(cur)
            ordered = [cur]
            while remaining:
                best, best_ov = None, -1
                cc = panels[cur][2]
                for idx in remaining:
                    ov = len(cc & panels[idx][2])
                    if ov > best_ov:
                        best_ov = ov
                        best = idx
                if best is None:
                    break
                ordered.append(best)
                remaining.remove(best)
                cur = best
            ordered.extend(remaining)
        else:
            ordered = group_sorted
        
        # Unique columns in this NUMA group
        all_cols = set()
        for pidx in ordered:
            all_cols.update(panels[pidx][2])
        strat2_numa_cols.append(len(all_cols))
        
        # L3 simulation for the whole NUMA group
        l3m = lru_sim(ordered, panels, L3_PER_NUMA_ROWS)
        strat2_l3_misses += l3m
        
        # L2 simulation per core within this NUMA group
        core_chunk = (len(ordered) + CORES_PER_NUMA - 1) // CORES_PER_NUMA
        numa_l2 = 0
        for c in range(CORES_PER_NUMA):
            s = c * core_chunk
            e = min(s + core_chunk, len(ordered))
            if s >= len(ordered):
                break
            m = lru_sim(ordered[s:e], panels, L2_PER_CORE_ROWS)
            numa_l2 += m
        strat2_misses += numa_l2
        strat2_numa_misses[n_id] = numa_l2
    
    print(f"\n  --- Strategy 2: NUMA-aware partition + per-group sort ---")
    print(f"  Partition time: {time.time()-t1:.1f}s")
    print(f"  Group sizes: {[len(g) for g in numa_groups]}")
    print(f"  Group unique cols: {strat2_numa_cols}")
    print(f"  L2 misses (per-core): {strat2_misses:,} ({strat2_misses/total_gather_naive*100:.1f}%)")
    print(f"  L3 misses (per-NUMA): {strat2_l3_misses:,} ({strat2_l3_misses/total_gather_naive*100:.1f}%)")
    l2_save2 = 1 - strat2_misses / total_gather_naive
    l3_save2 = 1 - strat2_l3_misses / total_gather_naive
    print(f"  L2 gather saved: {l2_save2:.1%}")
    print(f"  L3 gather saved: {l3_save2:.1%}")
    imb2 = max(strat2_numa_misses) / (sum(strat2_numa_misses)/N_NUMA) if sum(strat2_numa_misses) > 0 else 0
    print(f"  NUMA load imbalance: {imb2:.3f}")
    
    # Comparison
    print(f"\n  --- Comparison ---")
    print(f"  L2: Global AB {l2_save1:.1%} vs NUMA-aware {l2_save2:.1%} (delta {(l2_save2-l2_save1)*100:+.1f}%)")
    print(f"  L3: Global AB {l3_save1:.1%} vs NUMA-aware {l3_save2:.1%} (delta {(l3_save2-l3_save1)*100:+.1f}%)")
    
    print(f"\n  Total time: {time.time()-t0:.1f}s")
    sys.stdout.flush()

print(f"\n{'='*70}")
print("  ALL DONE")
print(f"{'='*70}")
