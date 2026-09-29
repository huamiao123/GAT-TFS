#!/usr/bin/env python3
"""
Anchor-Bucket deep analysis with per-thread LRU cache simulation.
Generates panel ordering files for C code integration.
"""
import numpy as np
import struct, os, time, sys
from collections import defaultdict

DATA = os.path.expanduser("~/data/SpMM_project/data")
OUT  = os.path.expanduser("~/data/SpMM_project/results/ab_deep")
os.makedirs(OUT, exist_ok=True)

MATRICES = [
    "hollywood-2009", "web-Google", "amazon0601", "soc-Pokec",
    "kron_g500-logn21", "rajat31", "wiki-Talk"
]
H = 16          # panel height
N_THREADS = 64  # thread count for simulation
CACHE_SIZES = [500, 1000, 2000, 4000, 8000]  # LRU cache sizes (B rows)

def read_csrbin(path):
    with open(path, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('III', f.read(12))
        nrow, ncol, nnz = struct.unpack('QQQ', f.read(24))
        indptr = np.frombuffer(f.read((nrow+1)*4), dtype=np.uint32).copy()
        indices = np.frombuffer(f.read(nnz*4), dtype=np.uint32).copy()
    return nrow, ncol, nnz, indptr, indices

def build_panels(nrow, ncol, indptr, indices, h=16):
    """Column-anchor panel building (same as V17c)."""
    # Build CSC
    col_deg = np.zeros(ncol, dtype=np.int64)
    for i in range(len(indices)):
        col_deg[indices[i]] += 1
    # Sort columns by degree descending
    col_order = np.argsort(-col_deg)
    
    assigned = np.zeros(nrow, dtype=bool)
    panels = []  # list of (anchor_col, row_list, col_set)
    
    # Build inverted index: col -> rows
    col_to_rows = defaultdict(list)
    for r in range(nrow):
        for j in range(indptr[r], indptr[r+1]):
            col_to_rows[indices[j]].append(r)
    
    for anchor in col_order:
        if col_deg[anchor] == 0:
            continue
        # Get unassigned rows containing this anchor column
        candidates = [r for r in col_to_rows[anchor] if not assigned[r]]
        if len(candidates) == 0:
            continue
        # Take h rows at a time
        for start in range(0, len(candidates), h):
            batch = candidates[start:start+h]
            if len(batch) == 0:
                break
            # Compute column union
            col_set = set()
            for r in batch:
                for j in range(indptr[r], indptr[r+1]):
                    col_set.add(int(indices[j]))
            panels.append((int(anchor), batch, col_set))
            for r in batch:
                assigned[r] = True
    
    return panels

def anchor_bucket_sort(panels):
    """Group panels by anchor column, sort within each bucket by column set similarity."""
    buckets = defaultdict(list)
    for i, (anchor, rows, cols) in enumerate(panels):
        buckets[anchor].append(i)
    
    # Sort buckets by size descending (process big buckets first)
    sorted_buckets = sorted(buckets.values(), key=len, reverse=True)
    
    # Within each bucket, greedy sort by max overlap with previous
    result_order = []
    for bucket in sorted_buckets:
        if len(bucket) <= 1:
            result_order.extend(bucket)
            continue
        # Greedy: start with first, then pick max overlap
        remaining = set(bucket)
        cur = bucket[0]
        remaining.remove(cur)
        result_order.append(cur)
        while remaining:
            best_idx = None
            best_overlap = -1
            cur_cols = panels[cur][2]
            for idx in remaining:
                overlap = len(cur_cols & panels[idx][2])
                if overlap > best_overlap:
                    best_overlap = overlap
                    best_idx = idx
            result_order.append(best_idx)
            remaining.remove(best_idx)
            cur = best_idx
    
    return result_order

def global_greedy_sort(panels):
    """Global greedy: always pick panel with max overlap to current."""
    n = len(panels)
    if n == 0:
        return []
    remaining = set(range(n))
    # Start with the panel that has the largest column set
    first = max(remaining, key=lambda i: len(panels[i][2]))
    remaining.remove(first)
    order = [first]
    cur = first
    while remaining:
        best_idx = None
        best_overlap = -1
        cur_cols = panels[cur][2]
        for idx in remaining:
            overlap = len(cur_cols & panels[idx][2])
            if overlap > best_overlap:
                best_overlap = overlap
                best_idx = idx
        order.append(best_idx)
        remaining.remove(best_idx)
        cur = best_idx
    return order

def lru_cache_sim(panel_order, panels, cache_size):
    """Simulate LRU cache for B-matrix rows. Returns total cache misses (= actual gathers)."""
    cache = {}  # col -> access_time
    time_counter = 0
    total_misses = 0
    total_accesses = 0
    
    for pidx in panel_order:
        _, _, col_set = panels[pidx]
        for col in col_set:
            total_accesses += 1
            if col in cache:
                # Hit: update access time
                cache[col] = time_counter
            else:
                # Miss
                total_misses += 1
                if len(cache) >= cache_size:
                    # Evict LRU
                    lru_col = min(cache, key=cache.get)
                    del cache[lru_col]
                cache[col] = time_counter
            time_counter += 1
    
    return total_misses, total_accesses

def thread_simulation(panel_order, panels, n_threads, cache_size):
    """Assign panels to threads in contiguous chunks, simulate per-thread LRU cache."""
    n = len(panel_order)
    chunk = (n + n_threads - 1) // n_threads
    
    total_misses = 0
    total_accesses = 0
    thread_gathers = []
    
    for t in range(n_threads):
        start = t * chunk
        end = min(start + chunk, n)
        if start >= n:
            break
        thread_panels = panel_order[start:end]
        misses, accesses = lru_cache_sim(thread_panels, panels, cache_size)
        total_misses += misses
        total_accesses += accesses
        thread_gathers.append(misses)
    
    return total_misses, total_accesses, thread_gathers

def compute_adjacent_overlap(panel_order, panels):
    """Compute mean adjacent panel column overlap."""
    if len(panel_order) <= 1:
        return 0.0
    overlaps = []
    for i in range(len(panel_order)-1):
        c1 = panels[panel_order[i]][2]
        c2 = panels[panel_order[i+1]][2]
        overlaps.append(len(c1 & c2))
    return np.mean(overlaps)

# ============================================================
# Main
# ============================================================
print("=" * 70)
print("  Anchor-Bucket Deep Analysis + Panel Order Generation")
print("=" * 70)
print(f"  Threads: {N_THREADS}, Cache sizes: {CACHE_SIZES}")
print(f"  Output dir: {OUT}")
print()

for mname in MATRICES:
    csrbin = os.path.join(DATA, mname, mname + ".csrbin")
    if not os.path.exists(csrbin):
        print(f"  [{mname}] SKIP (file not found: {csrbin})")
        continue
    
    print("=" * 70)
    print(f"  {mname}")
    print("=" * 70)
    sys.stdout.flush()
    
    t0 = time.time()
    nrow, ncol, nnz, indptr, indices = read_csrbin(csrbin)
    print(f"  {nrow:,} x {ncol:,}, NNZ={nnz:,}")
    
    # Build panels
    t1 = time.time()
    panels = build_panels(nrow, ncol, indptr, indices, H)
    print(f"  Panels: {len(panels):,} ({time.time()-t1:.1f}s)")
    
    total_gather_baseline = sum(len(p[2]) for p in panels)
    print(f"  Total gather (no cache): {total_gather_baseline:,}")
    sys.stdout.flush()
    
    # Original order
    orig_order = list(range(len(panels)))
    
    # Anchor-Bucket sort
    t1 = time.time()
    ab_order = anchor_bucket_sort(panels)
    ab_time = time.time() - t1
    print(f"  Anchor-Bucket sort: {ab_time:.2f}s")
    sys.stdout.flush()
    
    # Global greedy (only for small panel counts, otherwise too slow)
    if len(panels) <= 30000:
        t1 = time.time()
        greedy_order = global_greedy_sort(panels)
        greedy_time = time.time() - t1
        print(f"  Global greedy sort: {greedy_time:.2f}s")
        has_greedy = True
    else:
        print(f"  Global greedy: SKIP (panels={len(panels):,} > 30000)")
        greedy_order = None
        has_greedy = False
    sys.stdout.flush()
    
    # Adjacent overlap comparison
    print(f"\n  --- Adjacent Overlap ---")
    orig_overlap = compute_adjacent_overlap(orig_order, panels)
    ab_overlap = compute_adjacent_overlap(ab_order, panels)
    print(f"  Original:       mean={orig_overlap:.1f}")
    print(f"  Anchor-Bucket:  mean={ab_overlap:.1f}")
    if has_greedy:
        greedy_overlap = compute_adjacent_overlap(greedy_order, panels)
        print(f"  Global Greedy:  mean={greedy_overlap:.1f}")
    sys.stdout.flush()
    
    # Per-thread LRU cache simulation
    print(f"\n  --- Thread-aware LRU Cache Simulation (static chunks, {N_THREADS} threads) ---")
    print(f"  {'Cache':>8s} | {'Orig misses':>14s} {'AB misses':>14s} {'AB save':>8s}", end="")
    if has_greedy:
        print(f" {'Greedy misses':>14s} {'Greedy save':>10s}", end="")
    print()
    
    for cs in CACHE_SIZES:
        om, oa, _ = thread_simulation(orig_order, panels, N_THREADS, cs)
        am, aa, ab_tg = thread_simulation(ab_order, panels, N_THREADS, cs)
        ab_save = 1.0 - am / om if om > 0 else 0
        line = f"  {cs:>8d} | {om:>14,d} {am:>14,d} {ab_save:>7.1%}"
        if has_greedy:
            gm, ga, _ = thread_simulation(greedy_order, panels, N_THREADS, cs)
            gs = 1.0 - gm / om if om > 0 else 0
            line += f" {gm:>14,d} {gs:>9.1%}"
        print(line)
        sys.stdout.flush()
    
    # Load balance analysis for AB
    print(f"\n  --- Load Balance (AB, static, cache={CACHE_SIZES[2]}) ---")
    _, _, ab_thread_gathers = thread_simulation(ab_order, panels, N_THREADS, CACHE_SIZES[2])
    if ab_thread_gathers:
        avg_g = np.mean(ab_thread_gathers)
        max_g = np.max(ab_thread_gathers)
        min_g = np.min(ab_thread_gathers)
        imbalance = max_g / avg_g if avg_g > 0 else 0
        print(f"  Per-thread gathers: avg={avg_g:.0f}, max={max_g:.0f}, min={min_g:.0f}")
        print(f"  Imbalance ratio: {imbalance:.3f} (1.0=perfect)")
    
    # Save panel order files
    order_file = os.path.join(OUT, f"{mname}_ab_order.txt")
    with open(order_file, 'w') as f:
        f.write(f"# Anchor-Bucket panel order for {mname}\n")
        f.write(f"# Panels: {len(panels)}, Threads: {N_THREADS}\n")
        f.write(f"# Format: panel_index anchor_col num_rows U\n")
        for pidx in ab_order:
            anchor, rows, cols = panels[pidx]
            f.write(f"{pidx} {anchor} {len(rows)} {len(cols)}\n")
    print(f"\n  Saved order file: {order_file}")
    
    # Also save panel details (for C code to reconstruct)
    detail_file = os.path.join(OUT, f"{mname}_panels.bin")
    with open(detail_file, 'wb') as f:
        f.write(struct.pack('I', len(panels)))
        for anchor, rows, cols in panels:
            f.write(struct.pack('I', anchor))
            f.write(struct.pack('I', len(rows)))
            for r in rows:
                f.write(struct.pack('I', r))
    print(f"  Saved panel detail: {detail_file}")
    
    elapsed = time.time() - t0
    print(f"\n  Total time for {mname}: {elapsed:.1f}s")
    print()
    sys.stdout.flush()

print("=" * 70)
print("  ALL DONE")
print("=" * 70)
