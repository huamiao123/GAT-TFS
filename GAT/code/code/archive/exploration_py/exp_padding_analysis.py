"""
Short Row Padding/Packing Analysis

For each matrix, analyze:
1. Degree distribution of Fallback rows (rows not assigned to AMX)
2. If we pad short rows to 16 NNZ, how many can move from Fallback -> AMX?
3. Extra Gather cost of padding vs saved Fallback time
4. Multi-row packing: groups of short rows that share columns
"""
import numpy as np
import struct
import os

def read_csrbin(path):
    with open(path, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('III', f.read(12))
        nrow, ncol, nnz = struct.unpack('QQQ', f.read(24))
        indptr = np.frombuffer(f.read((nrow+1)*4), dtype=np.uint32).copy()
        indices = np.frombuffer(f.read(nnz*4), dtype=np.uint32).copy()
    return nrow, ncol, nnz, indptr, indices

DATA = "/home/huangjianqiang_group/hdacp1/data/SpMM_project/data"
MATRICES = [
    "web-Google", "amazon0601", "cit-Patents", "as-Skitter",
    "soc-Pokec", "hollywood-2009", "indochina-2004"
]

# V17c uses TILE_C=32, so a panel has 32 rows
# AMX tile is 16x16 (for BF16: 16 rows x 32 cols = 16x16 BF16 pairs)
# A row is assigned to AMX if its degree >= some threshold (depends on panel structure)
# Simplified model: row goes to AMX if it contributes enough NNZ to fill tiles

TILE_R = 16  # AMX tile rows
TILE_C = 32  # Panel size (rows per panel)
K = 128
BF16_ROW_BYTES = K * 2  # 256 bytes per B row in BF16
INT8_ROW_BYTES = K * 1  # 128 bytes per B row in INT8

print("=" * 100)
print("Short Row Padding / Packing Analysis")
print("=" * 100)

for mat_name in MATRICES:
    path = f"{DATA}/{mat_name}/{mat_name}.csrbin"
    if not os.path.exists(path):
        continue
    
    nrow, ncol, nnz, indptr, indices = read_csrbin(path)
    degrees = np.diff(indptr).astype(np.int64)
    avg_deg = nnz / nrow
    
    print(f"\n{'='*80}")
    print(f"Matrix: {mat_name}  M={nrow} N={ncol} NNZ={nnz} avg_deg={avg_deg:.1f}")
    print(f"{'='*80}")
    
    # Degree distribution
    print(f"\n  Degree Distribution:")
    thresholds = [0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]
    for i in range(len(thresholds)-1):
        lo, hi = thresholds[i], thresholds[i+1]
        count = np.sum((degrees >= lo) & (degrees < hi))
        nnz_in = np.sum(degrees[(degrees >= lo) & (degrees < hi)])
        pct = 100.0 * count / nrow
        nnz_pct = 100.0 * nnz_in / nnz
        print(f"    deg [{lo:5d}, {hi:5d}): {count:10d} rows ({pct:5.1f}%)  NNZ={nnz_in:12d} ({nnz_pct:5.1f}%)")
    count = np.sum(degrees >= thresholds[-1])
    nnz_in = np.sum(degrees[degrees >= thresholds[-1]])
    pct = 100.0 * count / nrow
    nnz_pct = 100.0 * nnz_in / nnz
    print(f"    deg [{thresholds[-1]:5d},   inf): {count:10d} rows ({pct:5.1f}%)  NNZ={nnz_in:12d} ({nnz_pct:5.1f}%)")
    
    # Panel-based analysis (simplified column-anchor model)
    # Sort rows by degree descending
    sorted_idx = np.argsort(-degrees)
    
    # Form panels of TILE_C=32 rows
    n_panels = (nrow + TILE_C - 1) // TILE_C
    
    # For each panel, compute column union |U| and AMX coverage
    print(f"\n  Panel AMX Coverage Analysis (TILE_C={TILE_C}):")
    
    amx_rows = 0
    fb_rows = 0
    amx_nnz = 0
    fb_nnz = 0
    
    # Simple model: a row contributes to AMX if it shares columns with "anchor" rows
    # More precisely: within a panel, rows whose columns overlap with the tile's column set
    # For now, use V17c's actual 1-Pass coverage as reference
    # and analyze what happens if we pad deg<16 rows to 16
    
    # Padding analysis
    print(f"\n  === Padding Analysis ===")
    for pad_threshold in [4, 8, 16, 32]:
        short_rows = np.sum(degrees < pad_threshold)
        short_nnz = np.sum(degrees[degrees < pad_threshold])
        short_pct = 100.0 * short_rows / nrow
        
        # If we pad all rows with deg < threshold to threshold:
        # Extra NNZ = sum(threshold - deg) for all short rows
        extra_nnz = np.sum(pad_threshold - degrees[degrees < pad_threshold])
        extra_gather_bytes = extra_nnz * BF16_ROW_BYTES  # each extra NNZ loads one B row
        extra_gather_gb = extra_gather_bytes / 1e9
        
        # Benefit: these rows move from Fallback to AMX
        # Saved time estimate: short_nnz * (FB_cost_per_nnz - AMX_cost_per_nnz)
        # From profiling: FB is ~3-5x slower than AMX per NNZ
        
        # Also compute for INT8
        extra_gather_int8 = extra_nnz * INT8_ROW_BYTES / 1e9
        
        print(f"    Pad to {pad_threshold}: {short_rows:10d} rows ({short_pct:5.1f}%)  "
              f"orig_nnz={short_nnz:12d}  extra_nnz={extra_nnz:12d}  "
              f"extra_gather(BF16)={extra_gather_gb:.2f}GB  "
              f"extra_gather(INT8)={extra_gather_int8:.2f}GB")
    
    # Multi-row packing analysis (sampling-based)
    print(f"\n  === Multi-Row Packing Analysis (sample 10000 short rows) ===")
    short_mask = degrees < 16
    short_indices_list = np.where(short_mask)[0]
    
    if len(short_indices_list) > 10000:
        np.random.seed(42)
        sample = np.random.choice(short_indices_list, 10000, replace=False)
    else:
        sample = short_indices_list
    
    if len(sample) >= 4:
        # For sampled short rows, compute pairwise column overlap
        # Group into packs of 4 rows, compute union size
        np.random.shuffle(sample)
        n_packs = len(sample) // 4
        union_sizes = []
        overlap_fracs = []
        
        for p in range(min(n_packs, 2500)):
            rows = sample[p*4:(p+1)*4]
            cols_set = set()
            total_nnz = 0
            for r in rows:
                r_cols = indices[indptr[r]:indptr[r+1]]
                cols_set.update(r_cols)
                total_nnz += len(r_cols)
            union_size = len(cols_set)
            union_sizes.append(union_size)
            if total_nnz > 0:
                overlap_fracs.append(1.0 - union_size / total_nnz)
        
        if union_sizes:
            print(f"    Packs of 4 short rows (n={len(union_sizes)}):")
            print(f"      Union size: mean={np.mean(union_sizes):.1f}  "
                  f"median={np.median(union_sizes):.0f}  "
                  f"p90={np.percentile(union_sizes, 90):.0f}  "
                  f"max={np.max(union_sizes)}")
            print(f"      Overlap frac: mean={np.mean(overlap_fracs):.3f}  "
                  f"(0=no overlap, 1=perfect)")
            print(f"      If union<16, row pack fits one AMX tile:")
            fits_tile = sum(1 for u in union_sizes if u <= 16)
            print(f"        {fits_tile}/{len(union_sizes)} packs ({100*fits_tile/len(union_sizes):.1f}%) fit in one tile")

print(f"\n{'='*100}")
print("=== ALL DONE ===")
