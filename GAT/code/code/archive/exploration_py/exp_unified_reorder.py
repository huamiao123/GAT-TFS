#!/usr/bin/env python3
"""
Unified Panel Reordering Experiment (v2 - correct panels)
Covers: MinHash global sort, inter-bucket ordering, NUMA partition, correlation analysis
Uses CORRECT V17c panel building logic (h=16 minimum, skip small panels)
"""
import struct, os, sys, time
import numpy as np
from collections import Counter, defaultdict

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
N_NUMA = 8
L2_CACHE = 2000

def load_csr(filepath):
    with open(filepath, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('<III', f.read(12))
        nrow, ncol, nnz = struct.unpack('<QQQ', f.read(24))
        indptr = np.frombuffer(f.read((nrow+1)*4), dtype=np.uint32).astype(np.int64)
        indices = np.frombuffer(f.read(nnz*4), dtype=np.uint32).astype(np.int64)
    return nrow, ncol, nnz, indptr, indices

def build_panels_v17c(nrow, ncol, nnz, indptr, indices):
    """CORRECT V17c panel building: only full h=16 panels."""
    col_to_rows = defaultdict(list)
    for r in range(nrow):
        for idx in range(indptr[r], indptr[r+1]):
            col_to_rows[indices[idx]].append(r)
    col_freq = sorted(col_to_rows.items(), key=lambda x: -len(x[1]))
    assigned = set()
    panels = []
    row_cols = {}
    for r in range(nrow):
        row_cols[r] = set(indices[indptr[r]:indptr[r+1]].tolist())
    for anchor_col, rows_list in col_freq:
        if len(rows_list) < TILE_R:
            continue
        avail = [r for r in rows_list if r not in assigned]
        if len(avail) < TILE_R:
            continue
        avail_deg = [(r, indptr[r+1]-indptr[r]) for r in avail]
        avail_deg.sort(key=lambda x: x[1])
        selected = [r for r, d in avail_deg[:TILE_R]]
        col_union = set()
        for r in selected:
            col_union |= row_cols[r]
        panels.append((int(anchor_col), selected, col_union))
        assigned.update(selected)
    return panels

def order_original(panels):
    return list(range(len(panels)))

def order_anchor_bucket(panels):
    af = Counter(p[0] for p in panels)
    bk = defaultdict(list)
    for i, (a, r, c) in enumerate(panels):
        bk[a].append(i)
    sa = sorted(bk.keys(), key=lambda a: -af[a])
    order = []
    for a in sa:
        bl = bk[a]
        bl.sort(key=lambda i: min(panels[i][1]))
        order.extend(bl)
    return order

def minhash_signature(col_set, n_hashes, a_arr, b_arr, p_val):
    if not col_set:
        return np.full(n_hashes, 2**62, dtype=np.uint64)
    sig = np.full(n_hashes, 2**62, dtype=np.uint64)
    for c in col_set:
        hvals = (a_arr * np.uint64(c) + b_arr) % p_val
        sig = np.minimum(sig, hvals)
    return sig

def order_minhash_sort(panels, n_hashes=64):
    t0 = time.time()
    rng = np.random.RandomState(12345)
    a_arr = rng.randint(1, 2**31, size=n_hashes).astype(np.uint64)
    b_arr = rng.randint(0, 2**31, size=n_hashes).astype(np.uint64)
    p_val = np.uint64(2147483647)
    sigs = []
    for i, (a, r, c) in enumerate(panels):
        sig = minhash_signature(c, n_hashes, a_arr, b_arr, p_val)
        sigs.append((tuple(sig[:4].tolist()), i))
        if i % 10000 == 0 and i > 0:
            print(f"    MinHash: {i}/{len(panels)} panels...", flush=True)
    sigs.sort(key=lambda x: x[0])
    order = [s[1] for s in sigs]
    return order, time.time() - t0

def order_column_centroid_sort(panels, ncol):
    centroids = [(sum(c)/len(c) if c else 0, i) for i, (a, r, c) in enumerate(panels)]
    centroids.sort()
    return [c[1] for c in centroids]

def order_greedy_inter_bucket(panels):
    bk = defaultdict(list)
    for i, (a, r, c) in enumerate(panels):
        bk[a].append(i)
    for a in bk:
        bk[a].sort(key=lambda i: min(panels[i][1]))
    bucket_list = list(bk.values())
    if len(bucket_list) <= 1:
        order = []
        for bl in bucket_list: order.extend(bl)
        return order
    bucket_sigs = []
    for bl in bucket_list:
        sig = set()
        for pidx in bl: sig.update(panels[pidx][2])
        bucket_sigs.append(sig)
    remaining = set(range(len(bucket_list)))
    cur = max(remaining, key=lambda i: len(bucket_list[i]))
    remaining.remove(cur)
    bucket_order = [cur]
    while remaining:
        cur_sig = bucket_sigs[cur]
        best_b, best_ov = None, -1
        for bi in remaining:
            ov = len(cur_sig & bucket_sigs[bi])
            if ov > best_ov: best_ov = ov; best_b = bi
        bucket_order.append(best_b)
        remaining.remove(best_b)
        cur = best_b
    order = []
    for bi in bucket_order: order.extend(bucket_list[bi])
    return order

def order_greedy_subsample(panels, max_n=5000):
    n = len(panels)
    if n <= max_n:
        indices = list(range(n))
    else:
        rng = np.random.RandomState(42)
        indices = sorted(rng.choice(n, max_n, replace=False).tolist())
    psub = [panels[i] for i in indices]
    used = [False]*len(psub)
    sub_order = [0]; used[0] = True
    for _ in range(len(psub)-1):
        cur_cols = psub[sub_order[-1]][2]
        best_j, best_ov = -1, -1
        for j in range(len(psub)):
            if used[j]: continue
            ov = len(cur_cols & psub[j][2])
            if ov > best_ov: best_ov = ov; best_j = j
        sub_order.append(best_j); used[best_j] = True
    sampled_order = [indices[i] for i in sub_order]
    sampled_set = set(sampled_order)
    remaining = [i for i in range(n) if i not in sampled_set]
    return sampled_order + remaining

def lru_sim_threaded(order, panels, cache_size, n_threads):
    n = len(order)
    chunk = (n + n_threads - 1) // n_threads
    total_misses = 0
    for t in range(n_threads):
        s = t * chunk; e = min(s + chunk, n)
        if s >= n: break
        cache = {}; tc = 0
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

def compute_air(panels, order, max_pairs=50000):
    n = len(order); step = max(1, (n-1)//max_pairs)
    airs = []
    for i in range(0, n-1, step):
        cur = panels[order[i]][2]; nxt = panels[order[i+1]][2]
        union = len(cur | nxt)
        if union > 0: airs.append(len(cur & nxt) / union)
    return np.mean(airs) if airs else 0

def compute_adj_overlap(panels, order, max_pairs=50000):
    n = len(order); step = max(1, (n-1)//max_pairs)
    ov = [len(panels[order[i]][2] & panels[order[i+1]][2]) for i in range(0, n-1, step)]
    return np.mean(ov) if ov else 0

def gini(values):
    v = np.sort(values.astype(np.float64)); n = len(v)
    if n == 0 or v.sum() == 0: return 0.0
    return (2.0 * np.sum(np.arange(1,n+1)*v) / (n*np.sum(v))) - (n+1.0)/n

def numa_eval(order, panels, n_numa, cores_per, cache_size):
    n = len(order); chunk = (n + n_numa - 1) // n_numa
    total = 0
    for ni in range(n_numa):
        s = ni*chunk; e = min(s+chunk, n)
        if s >= n: break
        grp = order[s:e]
        cc = (len(grp) + cores_per - 1) // cores_per
        for ci in range(cores_per):
            cs = ci*cc; ce = min(cs+cc, len(grp))
            if cs >= len(grp): break
            cache = {}; tc = 0
            for pidx in grp[cs:ce]:
                for col in panels[pidx][2]:
                    if col in cache: cache[col] = tc
                    else:
                        total += 1
                        if len(cache) >= cache_size:
                            lru = min(cache, key=cache.get); del cache[lru]
                        cache[col] = tc
                    tc += 1
    return total

print("="*70)
print("  Unified Panel Reordering Experiment (v2 - correct panels)")
print(f"  Threads={N_THREADS}, NUMA={N_NUMA}, L2 cache={L2_CACHE}")
print("="*70)

all_results = []
for mname, mrel in MATRICES:
    fp = os.path.join(DATA, mrel)
    if not os.path.exists(fp):
        print(f"\n  [{mname}] SKIP"); continue
    print(f"\n{'='*70}\n  {mname}\n{'='*70}"); sys.stdout.flush()
    t0 = time.time()
    nrow, ncol, nnz, indptr, indices = load_csr(fp)
    panels = build_panels_v17c(nrow, ncol, nnz, indptr, indices)
    print(f"  {nrow:,} rows, {nnz:,} NNZ, {len(panels):,} panels (V17c)")
    total_gather = sum(len(p[2]) for p in panels)
    unique_cols = len(set.union(*[p[2] for p in panels])) if panels else 0
    col_freq = Counter()
    for a,r,c in panels:
        for col in c: col_freq[col] += 1
    freqs = np.array(list(col_freq.values()), dtype=np.float64)
    rpi = float(np.sum(freqs**2)/np.sum(freqs))
    gini_val = gini(freqs)
    r_max = 1.0 - unique_cols/total_gather if total_gather > 0 else 0
    sorted_freq = np.sort(freqs)[::-1]
    x = np.log10(np.arange(1,len(sorted_freq)+1).astype(float))
    y = np.log10(sorted_freq.astype(float)+1e-10)
    mask = sorted_freq > 0
    zipf_alpha = -np.polyfit(x[mask],y[mask],1)[0] if np.sum(mask)>10 else 0
    probs = freqs/freqs.sum()
    entropy = -np.sum(probs*np.log2(probs+1e-30))
    max_ent = np.log2(len(freqs)) if len(freqs)>0 else 1
    norm_ent = entropy/max_ent
    print(f"  Gather:{total_gather:,} Unique:{unique_cols:,} RPI={rpi:.1f} Gini={gini_val:.4f} Zipf={zipf_alpha:.2f} R_max={r_max:.4f}")
    sys.stdout.flush()

    strategies = {}
    strategies['Original'] = order_original(panels)
    strategies['AnchorBucket'] = order_anchor_bucket(panels)

    mh_order, mh_time = order_minhash_sort(panels, 64)
    strategies['MinHash-64'] = mh_order
    print(f"  MinHash: {mh_time:.1f}s"); sys.stdout.flush()

    strategies['ColCentroid'] = order_column_centroid_sort(panels, ncol)

    n_bkts = len(set(p[0] for p in panels))
    if n_bkts <= 50000:
        t1 = time.time()
        strategies['GreedyInterBkt'] = order_greedy_inter_bucket(panels)
        print(f"  GreedyInterBkt: {time.time()-t1:.1f}s ({n_bkts} buckets)")
    else:
        print(f"  GreedyInterBkt: SKIP ({n_bkts} buckets)")
    sys.stdout.flush()

    t1 = time.time()
    strategies['GreedySub5K'] = order_greedy_subsample(panels, 5000)
    print(f"  GreedySub5K: {time.time()-t1:.1f}s"); sys.stdout.flush()

    print(f"\n  {'Strategy':<20s} {'AIR':>10s} {'AdjOvlp':>10s} {'L2 misses':>14s} {'Save%':>8s}")
    best_save, best_name = 0, ""
    strat_res = {}
    for sname, sorder in strategies.items():
        air = compute_air(panels, sorder)
        adj = compute_adj_overlap(panels, sorder)
        misses = lru_sim_threaded(sorder, panels, L2_CACHE, N_THREADS)
        save = 1.0 - misses/total_gather
        print(f"  {sname:<20s} {air:>10.6f} {adj:>10.1f} {misses:>14,d} {save:>7.1%}")
        strat_res[sname] = save
        if save > best_save: best_save = save; best_name = sname
        sys.stdout.flush()
    print(f"  ** Best: {best_name} ({best_save:.1%})")

    # Hot column pinning
    top_cols = sorted(col_freq.keys(), key=lambda c: col_freq[c], reverse=True)
    for budget_mb, label in [(5,"1 L3 way ~5MB"),(10,"2 L3 ways ~10MB")]:
        budget_rows = int(budget_mb*1024*1024/256)
        pinned = set(top_cols[:min(budget_rows,len(top_cols))])
        saved = sum(len(c & pinned) for _,_,c in panels)
        print(f"  CAT {label}: saves {saved/total_gather:.1%} gather")

    all_results.append({
        'name':mname, 'panels':len(panels), 'total_gather':total_gather,
        'rpi':rpi, 'gini':gini_val, 'zipf':zipf_alpha, 'r_max':r_max,
        'entropy':norm_ent, 'avg_deg':nnz/nrow,
        'best_save':best_save, 'best_name':best_name,
        'orig_save':strat_res.get('Original',0),
        'minhash_save':strat_res.get('MinHash-64',0),
    })
    print(f"  Total: {time.time()-t0:.1f}s\n"); sys.stdout.flush()

print("="*70)
print("  CORRELATION ANALYSIS")
print("="*70)
if len(all_results) >= 3:
    y = np.array([r['best_save'] for r in all_results])
    for idx in ['rpi','gini','zipf','r_max','entropy','avg_deg']:
        x = np.array([r[idx] for r in all_results])
        if np.std(x)>0 and np.std(y)>0:
            print(f"  {idx:<12s} corr={np.corrcoef(x,y)[0,1]:.4f}")
    rpi_arr = np.array([r['rpi'] for r in all_results])
    ss_tot = np.sum((y-np.mean(y))**2)
    if np.std(rpi_arr)>0 and ss_tot>0:
        s,i = np.polyfit(rpi_arr, y, 1)
        r2 = 1-np.sum((y-(s*rpi_arr+i))**2)/ss_tot
        print(f"\n  Linear RPI: R²={r2:.4f}")
    log_rpi = np.log10(rpi_arr+1)
    if np.std(log_rpi)>0 and ss_tot>0:
        s2,i2 = np.polyfit(log_rpi, y, 1)
        r2l = 1-np.sum((y-(s2*log_rpi+i2))**2)/ss_tot
        print(f"  Log(RPI):   R²={r2l:.4f}")

print(f"\n{'='*70}")
print("  SUMMARY")
print(f"{'='*70}")
print(f"  {'Matrix':<16s} {'Panels':>7s} {'RPI':>8s} {'Gini':>6s} {'R_max':>6s} {'OrigSv':>7s} {'MHSv':>7s} {'BestSv':>7s} {'Best':>16s}")
for r in all_results:
    print(f"  {r['name']:<16s} {r['panels']:>7,d} {r['rpi']:>8.1f} {r['gini']:>.4f} {r['r_max']:>.4f} "
          f"{r['orig_save']:>6.1%} {r['minhash_save']:>6.1%} {r['best_save']:>6.1%} {r['best_name']:>16s}")
print(f"\n  ALL DONE")
