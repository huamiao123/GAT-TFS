#!/usr/bin/env python3
"""
Column co-occurrence analysis + RPI/Gini/ERI vs actual B-reuse correlation.
Goal: prove that graph structure indices predict B-reuse effectiveness.
"""
import numpy as np
import struct, os, time, sys
from collections import defaultdict, Counter

DATA = os.path.expanduser("~/data/SpMM_project/data")
MATRICES = ["hollywood-2009", "web-Google", "amazon0601", "soc-Pokec",
            "kron_g500-logn21", "rajat31", "wiki-Talk"]
H = 16
CACHE = 2000

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
    for i, (a, r, c) in enumerate(panels):
        buckets[a].append(i)
    sorted_bkts = sorted(buckets.values(), key=len, reverse=True)
    result = []
    for bkt in sorted_bkts:
        if len(bkt) <= 1:
            result.extend(bkt)
            continue
        rem = set(bkt)
        cur = bkt[0]; rem.remove(cur); order = [cur]
        while rem:
            best, bov = None, -1
            cc = panels[cur][2]
            for idx in rem:
                ov = len(cc & panels[idx][2])
                if ov > bov: bov = ov; best = idx
            order.append(best); rem.remove(best); cur = best
        result.extend(order)
    return result

def lru_sim_thread(order, panels, cache_size, n_threads):
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

def gini(values):
    v = np.sort(values.astype(np.float64))
    n = len(v)
    if n == 0 or v.sum() == 0: return 0.0
    idx = np.arange(1, n+1)
    return (2.0 * np.sum(idx * v) / (n * np.sum(v))) - (n + 1.0) / n

print("=" * 70)
print("  Column Co-occurrence + Prediction Model")
print("=" * 70)

results = []

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
    
    # Column frequency across panels
    col_freq = Counter()
    for a, r, cols in panels:
        for c in cols:
            col_freq[c] += 1
    
    total_gather = sum(len(p[2]) for p in panels)
    unique_cols = len(col_freq)
    freqs = np.array(list(col_freq.values()), dtype=np.float64)
    
    # RPI
    rpi = np.sum(freqs**2) / np.sum(freqs)
    
    # Gini
    gini_val = gini(freqs)
    
    # Column Entropy
    probs = freqs / freqs.sum()
    entropy = -np.sum(probs * np.log2(probs + 1e-30))
    max_entropy = np.log2(len(freqs))
    norm_entropy = entropy / max_entropy if max_entropy > 0 else 0
    
    # R_max (theoretical maximum reuse)
    r_max = 1.0 - unique_cols / total_gather
    
    # Power-law exponent (for col frequency distribution)
    sorted_freq = np.sort(freqs)[::-1]
    # Fit log-log
    x = np.log10(np.arange(1, len(sorted_freq)+1).astype(float))
    y = np.log10(sorted_freq.astype(float) + 1e-10)
    mask = sorted_freq > 0
    if np.sum(mask) > 10:
        slope, _ = np.polyfit(x[mask], y[mask], 1)
        zipf_alpha = -slope
    else:
        zipf_alpha = 0
    
    # Column co-occurrence (sampled for large matrices)
    # For each panel, track which columns appear together
    # Co-occurrence strength = sum of (# panels where c1 and c2 both appear)
    # Sample top-1000 frequent columns for tractability
    print(f"  Computing column co-occurrence (sampled)...")
    sys.stdout.flush()
    top_k = min(1000, len(col_freq))
    top_cols = set(sorted(col_freq.keys(), key=lambda c: col_freq[c], reverse=True)[:top_k])
    
    cooccur_sum = 0
    cooccur_count = 0
    for a, r, cols in panels:
        hot = cols & top_cols
        n_hot = len(hot)
        cooccur_count += n_hot * (n_hot - 1) // 2
    
    # Average co-occurrence density
    max_pairs = top_k * (top_k - 1) // 2
    cooccur_density = cooccur_count / (len(panels) * max_pairs) if max_pairs > 0 else 0
    
    print(f"  Indices: RPI={rpi:.2f}, Gini={gini_val:.4f}, Entropy(norm)={norm_entropy:.4f}")
    print(f"  Zipf alpha={zipf_alpha:.2f}, R_max={r_max:.4f}")
    print(f"  Co-occur density (top-{top_k})={cooccur_density:.6f}")
    sys.stdout.flush()
    
    # Actual B-reuse with AB sort
    ab_order = anchor_bucket_sort(panels)
    ab_misses = lru_sim_thread(ab_order, panels, CACHE, 64)
    actual_reuse = 1.0 - ab_misses / total_gather
    
    # No-reuse baseline (original order)
    orig_misses = lru_sim_thread(list(range(len(panels))), panels, CACHE, 64)
    orig_reuse = 1.0 - orig_misses / total_gather
    
    # AB improvement over original
    ab_improvement = (orig_misses - ab_misses) / orig_misses if orig_misses > 0 else 0
    
    print(f"\n  Actual B-reuse:")
    print(f"    Original order:  {orig_reuse:.4f} ({orig_misses:,d} misses)")
    print(f"    AB order:        {actual_reuse:.4f} ({ab_misses:,d} misses)")
    print(f"    AB improvement:  {ab_improvement:.1%}")
    print(f"    R_max:           {r_max:.4f}")
    print(f"    Reuse achieved / R_max: {actual_reuse/r_max:.4f}" if r_max > 0 else "")
    
    results.append({
        'name': mname, 'nrow': nrow, 'nnz': nnz,
        'rpi': rpi, 'gini': gini_val, 'entropy': norm_entropy,
        'zipf_alpha': zipf_alpha, 'r_max': r_max,
        'cooccur_density': cooccur_density,
        'actual_reuse': actual_reuse, 'ab_improvement': ab_improvement,
        'avg_deg': nnz / nrow
    })
    
    print(f"  Time: {time.time()-t0:.1f}s")
    sys.stdout.flush()

# Correlation analysis
print(f"\n{'='*70}")
print("  CORRELATION: Graph Indices vs Actual B-reuse")
print(f"{'='*70}")

if len(results) >= 3:
    indices_names = ['rpi', 'gini', 'zipf_alpha', 'r_max', 'cooccur_density', 'avg_deg']
    y = np.array([r['actual_reuse'] for r in results])
    y_imp = np.array([r['ab_improvement'] for r in results])
    
    print(f"\n  {'Index':<20s} {'Corr(reuse)':>12s} {'Corr(improve)':>14s}")
    for idx_name in indices_names:
        x = np.array([r[idx_name] for r in results])
        if np.std(x) > 0 and np.std(y) > 0:
            corr_r = np.corrcoef(x, y)[0, 1]
            corr_i = np.corrcoef(x, y_imp)[0, 1]
            print(f"  {idx_name:<20s} {corr_r:>12.4f} {corr_i:>14.4f}")
    
    # Simple linear regression: RPI -> actual_reuse
    rpi_arr = np.array([r['rpi'] for r in results])
    if np.std(rpi_arr) > 0:
        slope, intercept = np.polyfit(rpi_arr, y, 1)
        y_pred = slope * rpi_arr + intercept
        ss_res = np.sum((y - y_pred)**2)
        ss_tot = np.sum((y - np.mean(y))**2)
        r_squared = 1 - ss_res / ss_tot if ss_tot > 0 else 0
        print(f"\n  Linear model: reuse = {slope:.6f} * RPI + {intercept:.4f}")
        print(f"  R² = {r_squared:.4f}")
    
    # Multi-variate: RPI + Gini -> actual_reuse
    X = np.column_stack([
        np.array([r['rpi'] for r in results]),
        np.array([r['gini'] for r in results]),
        np.ones(len(results))
    ])
    try:
        beta = np.linalg.lstsq(X, y, rcond=None)[0]
        y_pred_mv = X @ beta
        ss_res_mv = np.sum((y - y_pred_mv)**2)
        r_sq_mv = 1 - ss_res_mv / ss_tot if ss_tot > 0 else 0
        print(f"\n  Multi-var model: reuse = {beta[0]:.6f}*RPI + {beta[1]:.4f}*Gini + {beta[2]:.4f}")
        print(f"  R² = {r_sq_mv:.4f}")
    except:
        print("  Multi-var regression failed")

# Summary table
print(f"\n{'='*70}")
print("  SUMMARY TABLE")
print(f"{'='*70}")
print(f"  {'Matrix':<18s} {'AvgDeg':>7s} {'RPI':>8s} {'Gini':>6s} {'Zipf':>6s} {'R_max':>6s} {'Actual':>7s} {'AB_imp':>7s}")
for r in results:
    print(f"  {r['name']:<18s} {r['avg_deg']:>7.1f} {r['rpi']:>8.1f} {r['gini']:>.4f} "
          f"{r['zipf_alpha']:>6.2f} {r['r_max']:>.4f} {r['actual_reuse']:>.4f} {r['ab_improvement']:>6.1%}")

print(f"\n  ALL DONE")
