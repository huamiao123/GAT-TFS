#!/usr/bin/env python3
"""
Comprehensive matrix structural analysis for paper.
Generates: degree distribution, column frequency CDF, power-law metrics,
           RPI, Gini, graph density, clustering stats.
"""
import numpy as np
import struct, os, time, sys
from collections import Counter

DATA = os.path.expanduser("~/data/SpMM_project/data")
OUT  = os.path.expanduser("~/data/SpMM_project/results/matrix_profile")
os.makedirs(OUT, exist_ok=True)

MATRICES = [
    "hollywood-2009", "web-Google", "amazon0601", "soc-Pokec",
    "kron_g500-logn21", "rajat31", "rgg_n_2_24_s0", "wiki-Talk"
]

def read_csrbin(path):
    with open(path, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('III', f.read(12))
        nrow, ncol, nnz = struct.unpack('QQQ', f.read(24))
        indptr = np.frombuffer(f.read((nrow+1)*4), dtype=np.uint32).copy()
        indices = np.frombuffer(f.read(nnz*4), dtype=np.uint32).copy()
    return nrow, ncol, nnz, indptr, indices

def gini_coefficient(values):
    """Compute Gini coefficient of a distribution."""
    v = np.sort(values.astype(np.float64))
    n = len(v)
    if n == 0 or v.sum() == 0:
        return 0.0
    idx = np.arange(1, n+1)
    return (2.0 * np.sum(idx * v) / (n * np.sum(v))) - (n + 1.0) / n

def compute_rpi(col_freq):
    """RPI = sum(freq^2) / sum(freq)"""
    s1 = np.sum(col_freq.astype(np.float64))
    s2 = np.sum(col_freq.astype(np.float64)**2)
    return s2 / s1 if s1 > 0 else 0

print("=" * 70)
print("  Matrix Structural Profiling for Paper")
print("=" * 70)
print()

# Summary table header
summary_rows = []

for mname in MATRICES:
    csrbin = os.path.join(DATA, mname, mname + ".csrbin")
    if not os.path.exists(csrbin):
        print(f"  [{mname}] SKIP")
        continue
    
    print("=" * 70)
    print(f"  {mname}")
    print("=" * 70)
    sys.stdout.flush()
    
    t0 = time.time()
    nrow, ncol, nnz, indptr, indices = read_csrbin(csrbin)
    
    # Row degree distribution
    row_deg = np.diff(indptr).astype(np.int64)
    
    # Column degree distribution
    col_deg = np.zeros(ncol, dtype=np.int64)
    for i in range(nnz):
        col_deg[indices[i]] += 1
    
    # Basic stats
    density = nnz / (float(nrow) * ncol) * 100
    avg_row_deg = np.mean(row_deg)
    avg_col_deg = np.mean(col_deg[col_deg > 0])
    max_row_deg = np.max(row_deg)
    max_col_deg = np.max(col_deg)
    
    print(f"  Size: {nrow:,} x {ncol:,}")
    print(f"  NNZ: {nnz:,}")
    print(f"  Density: {density:.6f}%")
    print(f"  Row degree: mean={avg_row_deg:.1f}, max={max_row_deg}, "
          f"P50={np.median(row_deg):.0f}, P99={np.percentile(row_deg,99):.0f}")
    print(f"  Col degree: mean={avg_col_deg:.1f}, max={max_col_deg}, "
          f"P50={np.median(col_deg[col_deg>0]):.0f}, P99={np.percentile(col_deg[col_deg>0],99):.0f}")
    
    # Column frequency CDF (for AMX panels)
    # "Column frequency" = how many panels reference this column = col_deg (in CSR, col index appears col_deg times)
    # For panel context, it's the column degree
    nonzero_col_deg = col_deg[col_deg > 0]
    sorted_deg = np.sort(nonzero_col_deg)[::-1]
    cumsum = np.cumsum(sorted_deg.astype(np.float64))
    total = cumsum[-1]
    
    # CDF percentiles
    n_cols = len(sorted_deg)
    pcts = [0.01, 0.05, 0.10, 0.20, 0.50]
    print(f"\n  Column Frequency CDF (top X% cols contribute Y% of total references):")
    for p in pcts:
        k = max(1, int(n_cols * p))
        contrib = cumsum[k-1] / total * 100
        print(f"    Top {p*100:5.1f}% cols ({k:>8,d} cols) -> {contrib:5.1f}% of references")
    
    # RPI
    rpi = compute_rpi(nonzero_col_deg)
    print(f"\n  RPI (Reuse Potential Index): {rpi:.2f}")
    
    # Gini
    gini_row = gini_coefficient(row_deg)
    gini_col = gini_coefficient(nonzero_col_deg)
    print(f"  Gini (row degree): {gini_row:.4f}")
    print(f"  Gini (col degree): {gini_col:.4f}")
    
    # Locality Quotient
    # LQ = unique_cols / total_gather_references
    # total_gather = sum of all column degrees = nnz (each nonzero references its column once)
    # But in panel context, total_gather = sum of U across all panels
    # Without building panels, approximate: each row's columns are gathered once per panel it belongs to
    # Simpler: unique_cols = number of columns with deg > 0
    unique_cols = np.sum(col_deg > 0)
    # Approximate total gather (without panels) = nnz
    lq_approx = unique_cols / nnz
    print(f"  Unique columns: {unique_cols:,}")
    print(f"  LQ (approx, no panels): {lq_approx:.4f} (lower = more reuse potential)")
    
    # Degree distribution shape
    # Check power-law: log-log slope
    deg_counts = Counter(row_deg.tolist())
    degs = np.array(sorted(deg_counts.keys()))
    counts = np.array([deg_counts[d] for d in degs])
    mask = (degs > 0) & (counts > 0)
    if np.sum(mask) > 2:
        log_d = np.log10(degs[mask].astype(float))
        log_c = np.log10(counts[mask].astype(float))
        # Linear fit in log-log space
        slope, intercept = np.polyfit(log_d, log_c, 1)
        print(f"\n  Power-law fit (row degree): slope={slope:.2f} (steeper = more skewed)")
    
    col_deg_counts = Counter(nonzero_col_deg.tolist())
    col_degs = np.array(sorted(col_deg_counts.keys()))
    col_counts_arr = np.array([col_deg_counts[d] for d in col_degs])
    mask2 = (col_degs > 0) & (col_counts_arr > 0)
    if np.sum(mask2) > 2:
        log_d2 = np.log10(col_degs[mask2].astype(float))
        log_c2 = np.log10(col_counts_arr[mask2].astype(float))
        slope2, _ = np.polyfit(log_d2, log_c2, 1)
        print(f"  Power-law fit (col degree): slope={slope2:.2f}")
    
    # Hub analysis: how many rows does top-1% columns cover?
    top1_k = max(1, int(n_cols * 0.01))
    top1_cols = set(np.argsort(-col_deg)[:top1_k].tolist())
    rows_covered = set()
    for r in range(nrow):
        for j in range(indptr[r], indptr[r+1]):
            if int(indices[j]) in top1_cols:
                rows_covered.add(r)
                break
    hub_coverage = len(rows_covered) / nrow * 100
    print(f"\n  Hub analysis: top 1% cols ({top1_k:,}) cover {len(rows_covered):,} rows ({hub_coverage:.1f}%)")
    
    # NNZ covered by top-1% columns
    nnz_in_hub_rows = sum(indptr[r+1] - indptr[r] for r in rows_covered)
    nnz_coverage = nnz_in_hub_rows / nnz * 100
    print(f"  NNZ in hub rows: {nnz_in_hub_rows:,} ({nnz_coverage:.1f}%)")
    
    elapsed = time.time() - t0
    print(f"\n  Time: {elapsed:.1f}s")
    print()
    sys.stdout.flush()
    
    summary_rows.append({
        'name': mname, 'nrow': nrow, 'ncol': ncol, 'nnz': nnz,
        'avg_deg': avg_row_deg, 'max_deg': max_row_deg,
        'rpi': rpi, 'gini_col': gini_col, 'hub_cov': hub_coverage,
        'lq': lq_approx
    })

# Summary table
print("\n" + "=" * 70)
print("  SUMMARY TABLE (for paper)")
print("=" * 70)
print(f"  {'Matrix':<20s} {'N':>10s} {'NNZ':>12s} {'AvgDeg':>7s} {'MaxDeg':>7s} {'RPI':>8s} {'Gini':>6s} {'Hub%':>6s} {'LQ':>6s}")
for r in summary_rows:
    print(f"  {r['name']:<20s} {r['nrow']:>10,d} {r['nnz']:>12,d} {r['avg_deg']:>7.1f} {r['max_deg']:>7d} {r['rpi']:>8.1f} {r['gini_col']:>.4f} {r['hub_cov']:>5.1f}% {r['lq']:>.4f}")

print("\n  ALL DONE")
