#!/usr/bin/env python3
"""
Inter-bucket ordering optimization.
Compare: (1) current: buckets sorted by size
         (2) greedy: connect bucket tails to most-similar bucket heads
         (3) anchor-column adjacency: if two anchors co-occur in rows, place buckets adjacent
Tests whether inter-bucket ordering matters for B-reuse.
"""
import numpy as np
import struct, os, time, sys
from collections import defaultdict, Counter

DATA = os.path.expanduser("~/data/SpMM_project/data")
MATRICES = ["hollywood-2009", "web-Google", "amazon0601", "soc-Pokec"]
H = 16

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

def intra_bucket_greedy(bucket, panels):
    """Greedy chain within a bucket by max overlap."""
    if len(bucket) <= 1:
        return list(bucket)
    remaining = set(bucket)
    cur = bucket[0]
    remaining.remove(cur)
    order = [cur]
    while remaining:
        best, best_ov = None, -1
        cc = panels[cur][2]
        for idx in remaining:
            ov = len(cc & panels[idx][2])
            if ov > best_ov:
                best_ov = ov
                best = idx
        order.append(best)
        remaining.remove(best)
        cur = best
    return order

def compute_bucket_signature(bucket_panels, panels):
    """Union of all column sets in this bucket."""
    sig = set()
    for pidx in bucket_panels:
        sig.update(panels[pidx][2])
    return sig

def lru_sim_thread(order, panels, cache_size, n_threads):
    """Per-thread LRU simulation with static chunking."""
    n = len(order)
    chunk = (n + n_threads - 1) // n_threads
    total_misses = 0
    for t in range(n_threads):
        s = t * chunk
        e = min(s + chunk, n)
        if s >= n:
            break
        cache = {}
        tc = 0
        for pidx in order[s:e]:
            for col in panels[pidx][2]:
                if col in cache:
                    cache[col] = tc
                else:
                    total_misses += 1
                    if len(cache) >= cache_size:
                        lru = min(cache, key=cache.get)
                        del cache[lru]
                    cache[col] = tc
                tc += 1
    return total_misses

def compute_adjacent_overlap(order, panels):
    if len(order) <= 1:
        return 0.0
    total = 0
    for i in range(len(order)-1):
        total += len(panels[order[i]][2] & panels[order[i+1]][2])
    return total / (len(order)-1)

print("=" * 70)
print("  Inter-Bucket Ordering Experiment")
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
    print(f"  {len(panels):,} panels")
    total_gather = sum(len(p[2]) for p in panels)
    
    # Build buckets
    buckets = defaultdict(list)
    for i, (anchor, rows, cols) in enumerate(panels):
        buckets[anchor].append(i)
    bucket_list = sorted(buckets.values(), key=len, reverse=True)
    print(f"  {len(bucket_list):,} buckets, largest={len(bucket_list[0])}")
    
    # Intra-bucket greedy sort (shared across all strategies)
    sorted_buckets = {}
    for bi, bucket in enumerate(bucket_list):
        sorted_buckets[bi] = intra_bucket_greedy(bucket, panels)
    print(f"  Intra-bucket sort done ({time.time()-t0:.1f}s)")
    sys.stdout.flush()
    
    # Compute bucket signatures (union of all cols in bucket)
    bucket_sigs = {}
    bucket_anchors = {}
    for bi, bucket in enumerate(bucket_list):
        bucket_sigs[bi] = compute_bucket_signature(bucket, panels)
        bucket_anchors[bi] = panels[bucket[0]][0]  # anchor col
    
    # Strategy 1: Buckets sorted by size descending (current AB)
    strat1_order = []
    for bi in range(len(bucket_list)):
        strat1_order.extend(sorted_buckets[bi])
    
    # Strategy 2: Greedy inter-bucket ordering (max overlap between tail/head)
    t1 = time.time()
    remaining_b = set(range(len(bucket_list)))
    cur_b = 0  # start with largest bucket
    remaining_b.remove(cur_b)
    bucket_order = [cur_b]
    while remaining_b:
        # Get tail panel's col_set of current bucket
        tail_cols = panels[sorted_buckets[cur_b][-1]][2]
        best_b, best_ov = None, -1
        for bi in remaining_b:
            # Head panel's col_set of candidate bucket
            head_cols = panels[sorted_buckets[bi][0]][2]
            ov = len(tail_cols & head_cols)
            if ov > best_ov:
                best_ov = ov
                best_b = bi
        bucket_order.append(best_b)
        remaining_b.remove(best_b)
        cur_b = best_b
    
    strat2_order = []
    for bi in bucket_order:
        strat2_order.extend(sorted_buckets[bi])
    greedy_time = time.time() - t1
    
    # Strategy 3: Anchor-column co-occurrence ordering
    # Two anchors are "close" if they co-occur in many rows
    # Build anchor co-occurrence from CSR
    t1 = time.time()
    anchor_set = set(bucket_anchors.values())
    # For each row, find which anchor columns it contains
    anchor_cooccur = Counter()
    for r in range(nrow):
        row_anchors = []
        for j in range(indptr[r], indptr[r+1]):
            c = int(indices[j])
            if c in anchor_set:
                row_anchors.append(c)
        # Count pairs
        for ii in range(len(row_anchors)):
            for jj in range(ii+1, len(row_anchors)):
                pair = (min(row_anchors[ii], row_anchors[jj]),
                        max(row_anchors[ii], row_anchors[jj]))
                anchor_cooccur[pair] += 1
    
    # Map bucket index to anchor
    anchor_to_bucket = {}
    for bi in range(len(bucket_list)):
        a = bucket_anchors[bi]
        if a not in anchor_to_bucket:
            anchor_to_bucket[a] = bi
    
    # Greedy ordering using anchor co-occurrence
    remaining_b = set(range(len(bucket_list)))
    cur_b = 0
    remaining_b.remove(cur_b)
    bucket_order3 = [cur_b]
    while remaining_b:
        cur_anchor = bucket_anchors[cur_b]
        best_b, best_cooc = None, -1
        for bi in remaining_b:
            bi_anchor = bucket_anchors[bi]
            pair = (min(cur_anchor, bi_anchor), max(cur_anchor, bi_anchor))
            cooc = anchor_cooccur.get(pair, 0)
            if cooc > best_cooc:
                best_cooc = cooc
                best_b = bi
        if best_b is None:
            best_b = next(iter(remaining_b))
        bucket_order3.append(best_b)
        remaining_b.remove(best_b)
        cur_b = best_b
    
    strat3_order = []
    for bi in bucket_order3:
        strat3_order.extend(sorted_buckets[bi])
    cooccur_time = time.time() - t1
    
    # Strategy 4: Random bucket order (lower bound)
    rng = np.random.RandomState(42)
    random_bucket_order = list(range(len(bucket_list)))
    rng.shuffle(random_bucket_order)
    strat4_order = []
    for bi in random_bucket_order:
        strat4_order.extend(sorted_buckets[bi])
    
    print(f"  Inter-bucket ordering done ({time.time()-t0:.1f}s)")
    sys.stdout.flush()
    
    # Evaluate all strategies
    CACHE = 2000  # L2 ~ 2000 B-rows
    strategies = [
        ("Size-desc (current AB)", strat1_order),
        ("Greedy tail-head overlap", strat2_order),
        ("Anchor co-occurrence", strat3_order),
        ("Random bucket order", strat4_order),
    ]
    
    print(f"\n  --- Results (cache={CACHE}, 64 threads) ---")
    print(f"  {'Strategy':<30s} {'Adj overlap':>12s} {'L2 misses':>14s} {'Save vs naive':>14s}")
    
    for name, order in strategies:
        adj_ov = compute_adjacent_overlap(order, panels)
        misses = lru_sim_thread(order, panels, CACHE, 64)
        save = 1 - misses / total_gather
        print(f"  {name:<30s} {adj_ov:>12.1f} {misses:>14,d} {save:>13.1%}")
        sys.stdout.flush()
    
    # Quantify: how much does inter-bucket ordering matter vs intra-bucket?
    # Test: intra-bucket greedy + random inter-bucket vs intra-bucket random + size-desc inter
    # Intra-random: within each bucket, panels in original order (no greedy)
    strat5_order = []
    for bi in range(len(bucket_list)):
        strat5_order.extend(bucket_list[bi])  # original order within bucket
    
    adj_ov5 = compute_adjacent_overlap(strat5_order, panels)
    misses5 = lru_sim_thread(strat5_order, panels, CACHE, 64)
    save5 = 1 - misses5 / total_gather
    print(f"  {'No intra-sort (baseline)':<30s} {adj_ov5:>12.1f} {misses5:>14,d} {save5:>13.1%}")
    
    # Contribution analysis
    print(f"\n  --- Contribution Analysis ---")
    misses_random = lru_sim_thread(strat4_order, panels, CACHE, 64)
    misses_current = lru_sim_thread(strat1_order, panels, CACHE, 64)
    misses_best = min(lru_sim_thread(s[1], panels, CACHE, 64) for s in strategies[:3])
    misses_no_intra = misses5
    
    intra_contrib = misses_no_intra - misses_current
    inter_contrib = misses_current - misses_best if misses_current > misses_best else 0
    total_contrib = misses_no_intra - misses_best
    
    if total_contrib > 0:
        print(f"  Intra-bucket sorting contribution: {intra_contrib:,d} ({intra_contrib/total_contrib*100:.0f}%)")
        print(f"  Inter-bucket ordering contribution: {inter_contrib:,d} ({inter_contrib/total_contrib*100:.0f}%)")
    
    print(f"\n  Time: {time.time()-t0:.1f}s")
    sys.stdout.flush()

print(f"\n{'='*70}")
print("  ALL DONE")
print(f"{'='*70}")
