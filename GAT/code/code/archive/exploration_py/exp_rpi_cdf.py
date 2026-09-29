#!/usr/bin/env python3
"""
Experiment A: RPI + CDF + LQ + Stack Distance + Anchor-Bucket Analysis
"""
import struct, os, sys, time
import numpy as np
from collections import Counter, defaultdict

def load_csr_binary(filepath):
    with open(filepath, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('<III', f.read(12))
        nrow, ncol, nnz = struct.unpack('<QQQ', f.read(24))
        indptr = np.frombuffer(f.read((nrow+1)*4), dtype=np.uint32).astype(np.int64)
        indices = np.frombuffer(f.read(nnz*4), dtype=np.uint32).astype(np.int64)
    return nrow, ncol, nnz, indptr, indices

def build_panels(nrow, ncol, nnz, indptr, indices, TILE_R=16):
    col_to_rows = defaultdict(list)
    for r in range(nrow):
        for idx in range(indptr[r], indptr[r+1]):
            col_to_rows[indices[idx]].append(r)
    col_freq = sorted(col_to_rows.items(), key=lambda x: -len(x[1]))
    assigned = set()
    panels = []
    row_cols = {}
    for r in range(nrow):
        row_cols[r] = set(indices[indptr[r]:indptr[r+1]])
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
        panels.append((anchor_col, set(selected), col_union))
        assigned.update(selected)
    return panels

def compute_rpi(panels):
    col_counter = Counter()
    for _, _, col_union in panels:
        for c in col_union:
            col_counter[c] += 1
    freqs = np.array(list(col_counter.values()), dtype=np.float64)
    rpi = np.sum(freqs**2) / np.sum(freqs)
    total_gathers = int(np.sum(freqs))
    unique_cols = len(freqs)
    return rpi, total_gathers, unique_cols, np.mean(freqs), int(np.max(freqs)), freqs

def compute_gini(freqs):
    sf = np.sort(freqs)
    n = len(sf)
    return (2.0 * np.sum((np.arange(1, n+1) * sf))) / (n * np.sum(sf)) - (n+1)/n

def compute_cdf(freqs, pcts=[1,5,10,20,50]):
    sf = np.sort(freqs)[::-1]
    total = np.sum(sf)
    res = {}
    for p in pcts:
        k = max(1, int(len(sf)*p/100))
        res[p] = (k, np.sum(sf[:k])/total*100, k*256/1024/1024)
    return res

def compute_lq(panels, total_gathers):
    all_cols = set()
    for _, _, cu in panels:
        all_cols |= cu
    theo = len(all_cols)
    return theo/total_gathers if total_gathers > 0 else 0, theo

def anchor_bucket_reorder(panels):
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

def compute_air(panels, order):
    airs = []
    for i in range(1, len(order)):
        cur = panels[order[i]][2]
        prev = panels[order[i-1]][2]
        inter = len(cur & prev)
        union = len(cur | prev)
        if union > 0:
            airs.append(inter/union)
    return np.mean(airs) if airs else 0

def simulate_cache(panels, order, cache_size):
    cache_list, cache_set = [], set()
    hits, total = 0, 0
    for pi in order:
        for c in panels[pi][2]:
            total += 1
            if c in cache_set:
                hits += 1
                cache_list.remove(c)
                cache_list.append(c)
            else:
                if len(cache_list) >= cache_size:
                    ev = cache_list.pop(0)
                    cache_set.remove(ev)
                cache_list.append(c)
                cache_set.add(c)
    return hits/total if total > 0 else 0

def greedy_reorder(panels, max_n=3000):
    if len(panels) > max_n:
        rng = np.random.RandomState(42)
        idx = rng.choice(len(panels), max_n, replace=False)
        psub = [panels[i] for i in idx]
    else:
        psub = panels
    n = len(psub)
    used = [False]*n
    order = [0]; used[0] = True
    for _ in range(n-1):
        cur_cols = psub[order[-1]][2]
        best_j, best_ov = -1, -1
        for j in range(n):
            if used[j]: continue
            ov = len(cur_cols & psub[j][2])
            if ov > best_ov:
                best_ov = ov; best_j = j
        order.append(best_j); used[best_j] = True
    return order, psub

def compute_stack_distance(panels, sample_rate=1.0):
    last_access = {}
    distances = []
    step_val = max(1, int(1/sample_rate)) if sample_rate < 1.0 else 1
    gs = 0
    for pi in range(0, len(panels), step_val):
        for c in panels[pi][2]:
            if c in last_access:
                distances.append(gs - last_access[c])
            last_access[c] = gs
            gs += 1
    return np.array(distances) if distances else None

def analyze_matrix(name, filepath):
    print(f"\n{'='*70}")
    print(f"  Matrix: {name}")
    print(f"{'='*70}")
    t0 = time.time()
    nrow, ncol, nnz, indptr, indices = load_csr_binary(filepath)
    print(f"  Size: {nrow:,} x {ncol:,}, NNZ: {nnz:,}, AvgDeg: {nnz/nrow:.1f}")
    print(f"  Load: {time.time()-t0:.1f}s")

    t0 = time.time()
    panels = build_panels(nrow, ncol, nnz, indptr, indices)
    print(f"  Panels: {len(panels):,} (build: {time.time()-t0:.1f}s)")
    if len(panels) < 10:
        print("  Too few panels, skip"); return

    rpi, tg, uc, af, mf, freqs = compute_rpi(panels)
    print(f"\n  [RPI] = {rpi:.1f}")
    print(f"    Total gathers: {tg:,}, Unique cols: {uc:,}")
    print(f"    Avg freq: {af:.1f}x, Max freq: {mf}")

    gini = compute_gini(freqs)
    print(f"\n  [Gini] = {gini:.4f}  (0=uniform, 1=skewed)")

    cdf = compute_cdf(freqs)
    print(f"\n  [CDF] Top-k% cols -> % of total gathers (MB in B@K=128):")
    for p in sorted(cdf.keys()):
        k, pct, mb = cdf[p]
        print(f"    Top {p:2d}%: {k:6,} cols -> {pct:5.1f}% gathers ({mb:.1f} MB)")

    lq, theo = compute_lq(panels, tg)
    print(f"\n  [LQ] = {lq:.4f} ({lq*100:.2f}%)")
    print(f"    Theoretical min: {theo:,}, Actual: {tg:,}")
    print(f"    Headroom: {(1-lq)*100:.1f}%")

    t0 = time.time()
    ab = anchor_bucket_reorder(panels)
    abt = time.time()-t0
    orig = list(range(len(panels)))
    air_orig = compute_air(panels, orig)
    air_ab = compute_air(panels, ab)
    print(f"\n  [Anchor-Bucket] time={abt:.3f}s")
    print(f"    AIR orig:  {air_orig:.6f}")
    print(f"    AIR AB:    {air_ab:.6f}")
    if air_orig > 0:
        print(f"    Improve:   {air_ab/air_orig:.1f}x")

    for label, cs in [("1-panel(1500)", 1500), ("5-panel(7500)", 7500), ("L2(8000)", 8000)]:
        hr_o = simulate_cache(panels, orig, cs)
        hr_a = simulate_cache(panels, ab, cs)
        print(f"    Cache {label:18s}: orig={hr_o*100:.1f}%, AB={hr_a*100:.1f}%")

    max_g = min(len(panels), 3000)
    print(f"\n  [Greedy] subsample={max_g}")
    t0 = time.time()
    go, ps = greedy_reorder(panels, max_n=max_g)
    gt = time.time()-t0
    air_g = compute_air(ps, go)
    ab_sub = anchor_bucket_reorder(ps)
    air_ab_s = compute_air(ps, ab_sub)
    air_o_s = compute_air(ps, list(range(len(ps))))
    print(f"    AIR orig:  {air_o_s:.6f}")
    print(f"    AIR AB:    {air_ab_s:.6f}")
    print(f"    AIR greedy:{air_g:.6f}")
    print(f"    Time: greedy={gt:.1f}s, AB={abt:.3f}s")
    if air_ab_s > 0:
        print(f"    Greedy/AB ratio: {air_g/air_ab_s:.2f}x")

    sr = min(1.0, 5000/len(panels))
    print(f"\n  [Stack Distance] sample_rate={sr:.2f}")
    sd_o = compute_stack_distance(panels, sr)
    panels_ab = [panels[i] for i in ab]
    sd_a = compute_stack_distance(panels_ab, sr)
    if sd_o is not None and sd_a is not None:
        for lb, sd in [("Original", sd_o), ("AnchorBucket", sd_a)]:
            print(f"    {lb:14s}: mean={np.mean(sd):.0f} p50={np.median(sd):.0f} "
                  f"p90={np.percentile(sd,90):.0f} p99={np.percentile(sd,99):.0f}")

    print(f"\n  === SUMMARY: {name} ===")
    print(f"  RPI={rpi:.1f}  Gini={gini:.3f}  LQ={lq:.4f}")
    print(f"  Top5%cols->{cdf[5][1]:.1f}%gathers({cdf[5][2]:.1f}MB)")
    print(f"  AIR: orig={air_orig:.6f} AB={air_ab:.6f}")

def main():
    base = os.path.expanduser("~/data/SpMM_project/data")
    matrices = [
        ("web-Google",      "web-Google/web-Google.csrbin"),
        ("amazon0601",      "amazon0601/amazon0601.csrbin"),
        ("soc-Pokec",       "soc-Pokec/soc-Pokec.csrbin"),
        ("hollywood-2009",  "hollywood-2009/hollywood-2009.csrbin"),
        ("cit-Patents",     "cit-Patents/cit-Patents.csrbin"),
        ("as-Skitter",      "as-Skitter/as-Skitter.csrbin"),
        ("indochina-2004",  "indochina-2004/indochina-2004.csrbin"),
    ]
    print("="*70)
    print("  RPI + CDF + LQ + StackDist + Anchor-Bucket Analysis")
    print("="*70)
    for name, rel in matrices:
        fp = os.path.join(base, rel)
        if os.path.exists(fp):
            try: analyze_matrix(name, fp)
            except Exception as e:
                print(f"  ERROR on {name}: {e}")
                import traceback; traceback.print_exc()
        else:
            print(f"\n  SKIP: {fp} not found")
    print("\n" + "="*70)
    print("  All done!")
    print("="*70)

if __name__ == "__main__":
    main()
