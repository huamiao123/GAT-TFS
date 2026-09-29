#!/usr/bin/env python3
"""
Cache Size Sensitivity + NUMA-local GreedyInterBkt
验证三个猜想:
  1. L3 级别重排收益远高于 L2=2000 的模拟
  2. NUMA-local GreedyInterBkt 对大矩阵可行且有效
  3. cache/unique_cols 比率存在 phase transition
"""
import sys, os, struct, time
import numpy as np
from collections import OrderedDict

TILE_R = 16
N_HASH = 64
N_NUMA = 8
TOTAL_THREADS = 64
CACHE_SIZES = [2000, 4000, 8000, 16000, 32000, 64000, 128000, 256000]
GREEDY_GLOBAL_LIMIT = 50000
GREEDY_LOCAL_LIMIT = 30000

def load_csrbin(path):
    with open(path, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('III', f.read(12))
        nrow, ncol, nnz = struct.unpack('QQQ', f.read(24))
        indptr = np.fromfile(f, dtype=np.uint32, count=nrow+1)
        indices = np.fromfile(f, dtype=np.uint32, count=nnz)
    return int(nrow), int(ncol), int(nnz), indptr, indices

def build_panels_v17c(nrow, ncol, nnz, indptr, indices):
    t0 = time.time()
    row_cols = {}
    for r in range(nrow):
        s, e = int(indptr[r]), int(indptr[r+1])
        if e > s:
            row_cols[r] = set(indices[s:e].tolist())
    col_to_rows = {}
    for r, cols in row_cols.items():
        for c in cols:
            if c not in col_to_rows:
                col_to_rows[c] = []
            col_to_rows[c].append(r)
    col_freq = sorted(col_to_rows.items(), key=lambda x: -len(x[1]))
    assigned = set()
    panels = []
    for anchor_col, rows_with_col in col_freq:
        avail = [r for r in rows_with_col if r not in assigned]
        if len(avail) < TILE_R:
            continue
        avail_deg = [(r, indptr[r+1]-indptr[r]) for r in avail]
        avail_deg.sort(key=lambda x: x[1])
        selected = [r for r, d in avail_deg[:TILE_R]]
        selected.sort()
        col_union = set()
        for r in selected:
            col_union |= row_cols[r]
        panels.append((anchor_col, selected, col_union))
        assigned.update(selected)
    return panels, time.time()-t0

def compute_minhash(panels, n_hash=N_HASH, seed=42):
    rng = np.random.RandomState(seed)
    p = (1 << 31) - 1
    a = rng.randint(1, p, size=n_hash).astype(np.int64)
    b = rng.randint(0, p, size=n_hash).astype(np.int64)
    sigs = np.full((len(panels), n_hash), np.iinfo(np.int64).max, dtype=np.int64)
    for i, (_, _, col_set) in enumerate(panels):
        if i % 10000 == 0 and i > 0:
            print(f"    MinHash: {i}/{len(panels)} panels...", flush=True)
        for c in col_set:
            hv = (a * int(c) + b) % p
            sigs[i] = np.minimum(sigs[i], hv)
    return sigs

def minhash_reorder(panels, sigs):
    keys = [tuple(sigs[i, :4]) for i in range(len(panels))]
    return sorted(range(len(panels)), key=lambda i: keys[i])

def greedy_interbucket(panels, bucket_limit=GREEDY_GLOBAL_LIMIT):
    buckets = {}
    for i, (anchor, rows, cols) in enumerate(panels):
        if anchor not in buckets:
            buckets[anchor] = []
        buckets[anchor].append(i)
    n_bkts = len(buckets)
    if n_bkts > bucket_limit:
        return None, n_bkts
    for anchor in buckets:
        buckets[anchor].sort(key=lambda i: min(panels[i][1]))
    bkt_keys = list(buckets.keys())
    bkt_cols = {}
    for anchor in bkt_keys:
        union = set()
        for pi in buckets[anchor]:
            union |= panels[pi][2]
        bkt_cols[anchor] = union
    visited = set()
    start = max(bkt_keys, key=lambda a: len(bkt_cols[a]))
    order_bkts = [start]
    visited.add(start)
    for _ in range(n_bkts - 1):
        cur = order_bkts[-1]
        cur_cols = bkt_cols[cur]
        best, best_ovlp = None, -1
        for cand in bkt_keys:
            if cand in visited:
                continue
            ovlp = len(cur_cols & bkt_cols[cand])
            if ovlp > best_ovlp:
                best_ovlp = ovlp
                best = cand
        if best is None:
            break
        order_bkts.append(best)
        visited.add(best)
    result = []
    for anchor in order_bkts:
        result.extend(buckets[anchor])
    for anchor in bkt_keys:
        if anchor not in visited:
            result.extend(buckets[anchor])
    return result, n_bkts

def numa_local_greedy(panels, sigs, n_numa=N_NUMA):
    """NUMA-local: MinHash分8组, 组内精确GreedyInterBkt"""
    t0 = time.time()
    N = len(panels)
    mh_order = minhash_reorder(panels, sigs)
    chunk = (N + n_numa - 1) // n_numa
    groups = []
    for g in range(n_numa):
        s = g * chunk
        e = min(s + chunk, N)
        groups.append(mh_order[s:e])
    result = []
    for g, group_indices in enumerate(groups):
        if len(group_indices) == 0:
            continue
        sub_panels = [(panels[i][0], panels[i][1], panels[i][2]) for i in group_indices]
        sub_buckets = {}
        for local_i, (anchor, rows, cols) in enumerate(sub_panels):
            if anchor not in sub_buckets:
                sub_buckets[anchor] = []
            sub_buckets[anchor].append(local_i)
        n_sub_bkts = len(sub_buckets)
        if n_sub_bkts <= 1 or n_sub_bkts > GREEDY_LOCAL_LIMIT:
            result.extend(group_indices)
            continue
        for anchor in sub_buckets:
            sub_buckets[anchor].sort(key=lambda li: min(sub_panels[li][1]))
        bkt_keys = list(sub_buckets.keys())
        bkt_cols = {}
        for anchor in bkt_keys:
            union = set()
            for li in sub_buckets[anchor]:
                union |= sub_panels[li][2]
            bkt_cols[anchor] = union
        visited = set()
        start_bkt = max(bkt_keys, key=lambda a: len(bkt_cols[a]))
        order_bkts = [start_bkt]
        visited.add(start_bkt)
        for _ in range(n_sub_bkts - 1):
            cur = order_bkts[-1]
            cur_cols = bkt_cols[cur]
            best, best_ovlp = None, -1
            for cand in bkt_keys:
                if cand in visited:
                    continue
                ovlp = len(cur_cols & bkt_cols[cand])
                if ovlp > best_ovlp:
                    best_ovlp = ovlp
                    best = cand
            if best is None:
                break
            order_bkts.append(best)
            visited.add(best)
        for anchor in order_bkts:
            for local_i in sub_buckets[anchor]:
                result.append(group_indices[local_i])
        for anchor in bkt_keys:
            if anchor not in visited:
                for local_i in sub_buckets[anchor]:
                    result.append(group_indices[local_i])
    return result, time.time()-t0

def lru_simulate(panels, order, cache_size, n_threads=TOTAL_THREADS):
    N = len(order)
    if N == 0:
        return 0, 0, 0.0
    costs = [len(panels[order[i]][2]) for i in range(N)]
    total_cost = sum(costs)
    cum = 0
    splits = [0]
    target = total_cost / n_threads
    t_idx = 1
    for i in range(N):
        cum += costs[i]
        while t_idx < n_threads and cum >= target * t_idx:
            splits.append(i + 1)
            t_idx += 1
    splits.append(N)
    total_misses = 0
    total_gather = 0
    for t in range(n_threads):
        s = splits[t]
        e = splits[min(t+1, len(splits)-1)]
        if s >= e:
            continue
        cache = OrderedDict()
        misses = 0
        gather = 0
        for idx in range(s, e):
            pi = order[idx]
            col_set = panels[pi][2]
            gather += len(col_set)
            for c in col_set:
                if c in cache:
                    cache.move_to_end(c)
                else:
                    misses += 1
                    if len(cache) >= cache_size:
                        cache.popitem(last=False)
                    cache[c] = True
        total_misses += misses
        total_gather += gather
    save_pct = (1.0 - total_misses / total_gather) * 100 if total_gather > 0 else 0.0
    return total_misses, total_gather, save_pct

def process_matrix(name, path):
    print(f"\n{'='*70}", flush=True)
    print(f"  {name}", flush=True)
    print(f"{'='*70}", flush=True)
    t0 = time.time()
    nrow, ncol, nnz, indptr, indices = load_csrbin(path)
    print(f"  {nrow:,} rows, {ncol:,} cols, {nnz:,} NNZ  ({time.time()-t0:.1f}s)", flush=True)
    panels, t_build = build_panels_v17c(nrow, ncol, nnz, indptr, indices)
    N = len(panels)
    total_gather = sum(len(p[2]) for p in panels)
    unique_cols = len(set().union(*(p[2] for p in panels))) if panels else 0
    anchor_to_bkt = {}
    for i, (a, _, _) in enumerate(panels):
        if a not in anchor_to_bkt:
            anchor_to_bkt[a] = []
        anchor_to_bkt[a].append(i)
    n_buckets = len(anchor_to_bkt)
    print(f"  Panels={N}  Buckets={n_buckets}  TotalGather={total_gather:,}  "
          f"UniqueCols={unique_cols:,}  ({t_build:.1f}s)", flush=True)

    order_orig = list(range(N))

    print(f"  Computing MinHash-64...", flush=True)
    t0 = time.time()
    sigs = compute_minhash(panels, N_HASH)
    print(f"  MinHash: {time.time()-t0:.1f}s", flush=True)
    order_minhash = minhash_reorder(panels, sigs)

    order_greedy = None
    if n_buckets <= GREEDY_GLOBAL_LIMIT:
        print(f"  Computing GreedyInterBkt ({n_buckets} buckets)...", flush=True)
        t0 = time.time()
        order_greedy, _ = greedy_interbucket(panels, GREEDY_GLOBAL_LIMIT)
        print(f"  GreedyInterBkt: {time.time()-t0:.1f}s", flush=True)
    else:
        print(f"  GreedyInterBkt: SKIP ({n_buckets} > {GREEDY_GLOBAL_LIMIT})", flush=True)

    print(f"  Computing NUMA-local Greedy (8 groups)...", flush=True)
    order_numa, t_numa = numa_local_greedy(panels, sigs, N_NUMA)
    print(f"  NUMA-local Greedy: {t_numa:.1f}s", flush=True)

    # --- Exp A: Cache Size Sweep ---
    print(f"\n  --- Experiment A: Cache Size Sensitivity ---", flush=True)
    print(f"  {'Cache':>8s} | {'Original':>10s} | {'MinHash':>10s} | {'Greedy':>10s} | {'NUMA-Grdy':>10s}", flush=True)
    print(f"  {'-'*8}-+-{'-'*10}-+-{'-'*10}-+-{'-'*10}-+-{'-'*10}", flush=True)
    for cs in CACHE_SIZES:
        _, _, so = lru_simulate(panels, order_orig, cs)
        _, _, sm = lru_simulate(panels, order_minhash, cs)
        if order_greedy is not None:
            _, _, sg = lru_simulate(panels, order_greedy, cs)
            gs = f"{sg:9.1f}%"
        else:
            gs = "     SKIP"
        _, _, sn = lru_simulate(panels, order_numa, cs)
        print(f"  {cs:>8d} | {so:9.1f}% | {sm:9.1f}% | {gs:>10s} | {sn:9.1f}%", flush=True)

    # --- Exp B: NUMA group detail ---
    print(f"\n  --- Experiment B: NUMA-local Group Detail ---", flush=True)
    mh_order = minhash_reorder(panels, sigs)
    chunk = (N + N_NUMA - 1) // N_NUMA
    for g in range(N_NUMA):
        s = g * chunk
        e = min(s + chunk, N)
        group = mh_order[s:e]
        ga = set(panels[i][0] for i in group)
        gc = set()
        for i in group:
            gc |= panels[i][2]
        print(f"    NUMA-{g}: {len(group):>6d} panels, {len(ga):>6d} buckets, "
              f"{len(gc):>8,} unique cols", flush=True)

    # --- Key ratios ---
    print(f"\n  --- Key Ratios ---", flush=True)
    for cs in [2000, 8000, 36000, 128000, 256000]:
        ratio = cs / unique_cols * 100 if unique_cols > 0 else 0
        print(f"    cache={cs:>6d} -> cache/cols = {ratio:.3f}%", flush=True)

    # --- Summary ---
    print(f"\n  === SUMMARY: {name} ===", flush=True)
    for cs, label in [(2000,"L2-25%"), (8000,"Full-L2"), (36000,"L3/NUMA"), (128000,"L3-half"), (256000,"L3-full")]:
        _, _, so = lru_simulate(panels, order_orig, cs)
        _, _, sm = lru_simulate(panels, order_minhash, cs)
        _, _, sn = lru_simulate(panels, order_numa, cs)
        if order_greedy is not None:
            _, _, sg = lru_simulate(panels, order_greedy, cs)
            print(f"    [{label:>8s} c={cs:>6d}] Orig={so:5.1f}% MH={sm:5.1f}% Greedy={sg:5.1f}% NUMA-G={sn:5.1f}%", flush=True)
        else:
            print(f"    [{label:>8s} c={cs:>6d}] Orig={so:5.1f}% MH={sm:5.1f}% Greedy=SKIP NUMA-G={sn:5.1f}%", flush=True)

def main():
    if len(sys.argv) < 2:
        print("Usage: python exp_cache_sweep_numa.py <csrbin1> [csrbin2] ...")
        sys.exit(1)
    print(f"{'='*70}", flush=True)
    print(f"  Cache Sweep + NUMA-local Greedy Experiment", flush=True)
    print(f"  Threads={TOTAL_THREADS}, NUMA={N_NUMA}, MinHash={N_HASH}", flush=True)
    print(f"  Cache sizes: {CACHE_SIZES}", flush=True)
    print(f"{'='*70}", flush=True)
    t_all = time.time()
    for path in sys.argv[1:]:
        if not os.path.exists(path):
            print(f"  MISSING: {path}", flush=True)
            continue
        name = os.path.basename(path).replace('.csrbin','')
        try:
            t0 = time.time()
            process_matrix(name, path)
            print(f"\n  Total time for {name}: {time.time()-t0:.1f}s", flush=True)
        except Exception as ex:
            print(f"  ERROR on {name}: {ex}", flush=True)
            import traceback; traceback.print_exc()
    print(f"\n{'='*70}", flush=True)
    print(f"  All done. Total: {time.time()-t_all:.1f}s", flush=True)
    print(f"{'='*70}", flush=True)

if __name__ == '__main__':
    main()
