#!/usr/bin/env python3
"""
Hot column pinning simulation: if top-K hottest columns are always in cache
(simulating perfect L3 CAT), how much gather is saved?
Also: RPI vs actual B-reuse correlation analysis.
"""
import numpy as np
import struct, os, time, sys
from collections import defaultdict, Counter

DATA = os.path.expanduser("~/data/SpMM_project/data")
MATRICES = ["hollywood-2009", "web-Google", "amazon0601", "soc-Pokec",
            "kron_g500-logn21", "rajat31", "wiki-Talk"]
H = 16
# Pinning budgets: number of hot B-rows to keep always in cache
# 5MB L3 way = ~19500 rows (BF16 K=128)
PIN_BUDGETS = [1000, 5000, 10000, 20000, 50000]

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

print("=" * 70)
print("  Hot Column Pinning Simulation")
print(f"  Pin budgets: {PIN_BUDGETS}")
print("=" * 70)

summary = []

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
    
    # Count column frequency across panels
    col_freq = Counter()
    for anchor, rows, cols in panels:
        for c in cols:
            col_freq[c] += 1
    
    total_gather = sum(len(p[2]) for p in panels)
    unique_cols = len(col_freq)
    
    # RPI
    freqs = np.array(list(col_freq.values()), dtype=np.float64)
    rpi = np.sum(freqs**2) / np.sum(freqs)
    
    # Gini of panel-level column frequency
    sorted_f = np.sort(freqs)
    n_f = len(sorted_f)
    gini = (2.0 * np.sum(np.arange(1, n_f+1) * sorted_f) / (n_f * np.sum(sorted_f))) - (n_f+1.0)/n_f
    
    print(f"  Total gather (naive): {total_gather:,}")
    print(f"  Unique columns: {unique_cols:,}")
    print(f"  Theoretical min (each col once): {unique_cols:,} ({unique_cols/total_gather*100:.1f}%)")
    print(f"  RPI: {rpi:.2f}")
    print(f"  Gini (col freq): {gini:.4f}")
    print(f"  Max col freq: {max(col_freq.values()):,}")
    print(f"  Avg col freq: {np.mean(freqs):.2f}")
    
    # Column frequency CDF
    top_cols = sorted(col_freq.keys(), key=lambda c: col_freq[c], reverse=True)
    cum_gather = 0
    print(f"\n  Column Frequency CDF (panel-level):")
    reported = set()
    for i, c in enumerate(top_cols):
        cum_gather += col_freq[c]
        pct = (i+1) / unique_cols
        gather_pct = cum_gather / total_gather
        for threshold in [0.01, 0.05, 0.10, 0.20, 0.50]:
            if pct >= threshold and threshold not in reported:
                print(f"    Top {threshold*100:5.1f}% cols ({i+1:>8,d}) -> {gather_pct*100:5.1f}% of gather references")
                reported.add(threshold)
    
    # Pinning simulation
    print(f"\n  --- Hot Column Pinning ---")
    print(f"  {'Budget':>8s} | {'Pinned cols':>12s} | {'Gather saved':>14s} | {'Save %':>8s} | {'MB pinned':>10s}")
    
    row_data = {'name': mname, 'rpi': rpi, 'gini': gini, 'total_gather': total_gather}
    
    for budget in PIN_BUDGETS:
        # Pin the top-budget hottest columns
        pinned = set(top_cols[:min(budget, len(top_cols))])
        
        # For each panel, gather only non-pinned columns
        saved = 0
        for anchor, rows, cols in panels:
            hits = len(cols & pinned)
            saved += hits
        
        save_pct = saved / total_gather
        mb_pinned = budget * 256 / (1024*1024)  # 256 bytes per row (K=128, BF16)
        print(f"  {budget:>8d} | {min(budget, len(top_cols)):>12,d} | {saved:>14,d} | {save_pct:>7.1%} | {mb_pinned:>8.1f} MB")
        
        row_data[f'pin_{budget}'] = save_pct
    
    # Optimal pinning: what if we pin exactly 1 L3 way (~19500 rows)?
    l3_way_budget = 19500
    pinned_l3 = set(top_cols[:min(l3_way_budget, len(top_cols))])
    saved_l3 = sum(len(p[2] & pinned_l3) for _, _, p2 in [(0,0,p[2]) for p in panels])
    # Fix the comprehension
    saved_l3 = 0
    for anchor, rows, cols in panels:
        saved_l3 += len(cols & pinned_l3)
    save_l3_pct = saved_l3 / total_gather
    print(f"\n  1 L3 way (~5MB, {l3_way_budget} rows): saves {save_l3_pct:.1%} gather")
    print(f"  2 L3 ways (~10MB, {l3_way_budget*2} rows): ", end="")
    pinned_2way = set(top_cols[:min(l3_way_budget*2, len(top_cols))])
    saved_2way = sum(len(p[2] & pinned_2way) for _, _, p2 in [(0,0,p[2]) for p in panels])
    saved_2way = 0
    for anchor, rows, cols in panels:
        saved_2way += len(cols & pinned_2way)
    print(f"saves {saved_2way/total_gather:.1%} gather")
    
    row_data['pin_l3_1way'] = save_l3_pct
    summary.append(row_data)
    
    print(f"\n  Time: {time.time()-t0:.1f}s")
    sys.stdout.flush()

# Summary table
print(f"\n{'='*70}")
print("  SUMMARY: RPI vs Pinning Effectiveness")
print(f"{'='*70}")
print(f"  {'Matrix':<20s} {'RPI':>8s} {'Gini':>6s} {'Pin 1K':>8s} {'Pin 5K':>8s} {'Pin 20K':>8s} {'L3 1way':>8s}")
for r in summary:
    print(f"  {r['name']:<20s} {r['rpi']:>8.1f} {r['gini']:>.4f} "
          f"{r.get('pin_1000',0):>7.1%} {r.get('pin_5000',0):>7.1%} "
          f"{r.get('pin_20000',0):>7.1%} {r.get('pin_l3_1way',0):>7.1%}")

print(f"\n  ALL DONE")
