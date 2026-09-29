"""
INT8 Precision Simulation for SpMM (numpy-vectorized version)
Compare: FP32 ground truth vs INT8 quantized → INT32 accumulate → FP32
"""
import numpy as np
import struct
import time
import os
import sys

def read_csrbin(path):
    with open(path, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('III', f.read(12))
        nrow, ncol, nnz = struct.unpack('QQQ', f.read(24))
        indptr = np.frombuffer(f.read((nrow+1)*4), dtype=np.uint32).copy()
        indices = np.frombuffer(f.read(nnz*4), dtype=np.uint32).copy()
    values = np.ones(nnz, dtype=np.float32)
    return nrow, ncol, nnz, indptr, indices, values

def spmm_dense_subset(indptr, indices, values, B, row_ids):
    """SpMM for a subset of rows using dense accumulation (vectorized)"""
    K = B.shape[1]
    nrows = len(row_ids)
    C = np.zeros((nrows, K), dtype=np.float64)
    for ri, i in enumerate(row_ids):
        cols = indices[indptr[i]:indptr[i+1]]
        vals = values[indptr[i]:indptr[i+1]]
        if len(cols) > 0:
            # Vectorized: C[ri] = sum(vals[:, None] * B[cols])
            C[ri] = np.dot(vals.astype(np.float64), B[cols].astype(np.float64))
    return C

def quantize_per_tensor_sym(B_fp32):
    amax = np.abs(B_fp32).max()
    if amax == 0: return np.zeros_like(B_fp32, dtype=np.int8), 1.0
    scale = amax / 127.0
    B_int8 = np.clip(np.round(B_fp32 / scale), -128, 127).astype(np.int8)
    return B_int8, scale

def quantize_per_column_sym(B_fp32):
    amax = np.abs(B_fp32).max(axis=0)
    amax = np.where(amax == 0, 1.0, amax)
    scales = amax / 127.0
    B_int8 = np.clip(np.round(B_fp32 / scales[np.newaxis, :]), -128, 127).astype(np.int8)
    return B_int8, scales

def quantize_per_column_asym(B_fp32):
    col_min = B_fp32.min(axis=0)
    col_max = B_fp32.max(axis=0)
    ranges = col_max - col_min
    ranges = np.where(ranges == 0, 1.0, ranges)
    scales = ranges / 255.0
    zero_points = np.clip(np.round(-col_min / scales), 0, 255).astype(np.int32)
    B_uint8 = np.clip(np.round(B_fp32 / scales[np.newaxis, :] + zero_points[np.newaxis, :]),
                       0, 255).astype(np.uint8)
    return B_uint8, scales, zero_points

def spmm_int8_pertensor(indptr, indices, values, B_int8, scale_b, row_ids):
    """Per-tensor INT8: values stay FP32, B is int8, accumulate in int32"""
    K = B_int8.shape[1]
    nrows = len(row_ids)
    C = np.zeros((nrows, K), dtype=np.float64)
    for ri, i in enumerate(row_ids):
        cols = indices[indptr[i]:indptr[i+1]]
        vals = values[indptr[i]:indptr[i+1]]
        if len(cols) > 0:
            # INT8 gather + INT32 accumulate
            B_rows = B_int8[cols].astype(np.int32)  # (deg, K)
            # Weighted sum: vals are float, B_rows are int
            # In real AMX: both would be int8, but for unweighted graph vals=1
            acc = np.dot(vals.astype(np.float64), B_rows.astype(np.float64))
            C[ri] = acc * scale_b
    return C

def spmm_int8_percol(indptr, indices, values, B_int8, scales, row_ids):
    """Per-column INT8: each column has its own scale"""
    K = B_int8.shape[1]
    nrows = len(row_ids)
    C = np.zeros((nrows, K), dtype=np.float64)
    for ri, i in enumerate(row_ids):
        cols = indices[indptr[i]:indptr[i+1]]
        vals = values[indptr[i]:indptr[i+1]]
        if len(cols) > 0:
            B_rows = B_int8[cols].astype(np.int32)
            acc = np.dot(vals.astype(np.float64), B_rows.astype(np.float64))
            C[ri] = acc * scales
    return C

def spmm_int8_percol_asym(indptr, indices, values, B_uint8, scales, zps, row_ids):
    """Per-column asymmetric INT8"""
    K = B_uint8.shape[1]
    nrows = len(row_ids)
    C = np.zeros((nrows, K), dtype=np.float64)
    for ri, i in enumerate(row_ids):
        cols = indices[indptr[i]:indptr[i+1]]
        vals = values[indptr[i]:indptr[i+1]]
        deg = len(cols)
        if deg > 0:
            B_rows = B_uint8[cols].astype(np.int32)
            acc = np.dot(vals.astype(np.float64), B_rows.astype(np.float64))
            # Dequantize with zero point correction
            C[ri] = (acc - np.sum(vals).astype(np.float64) * zps.astype(np.float64)) * scales
    return C

def spmm_bf16(indptr, indices, values, B_fp32, row_ids):
    """BF16 simulation"""
    def to_bf16(x):
        x32 = x.view(np.uint32)
        x32 = x32 & 0xFFFF0000
        return x32.view(np.float32)
    
    B_bf16 = to_bf16(B_fp32.copy())
    K = B_fp32.shape[1]
    nrows = len(row_ids)
    C = np.zeros((nrows, K), dtype=np.float32)
    for ri, i in enumerate(row_ids):
        cols = indices[indptr[i]:indptr[i+1]]
        vals = values[indptr[i]:indptr[i+1]]
        if len(cols) > 0:
            vals_bf16 = to_bf16(vals.copy())
            C[ri] = np.dot(vals_bf16.astype(np.float32), B_bf16[cols].astype(np.float32))
    return C.astype(np.float64)

def compute_metrics(C_truth, C_test, label):
    nrow, K = C_truth.shape
    abs_truth = np.abs(C_truth)
    mask = abs_truth > 1e-6
    if mask.sum() > 0:
        rel_err = np.abs(C_truth[mask] - C_test[mask]) / abs_truth[mask]
        mape = rel_err.mean() * 100
        p99_rel = np.percentile(rel_err, 99) * 100
        max_rel = rel_err.max() * 100
    else:
        mape = p99_rel = max_rel = 0.0
    
    rmse = np.sqrt(np.mean((C_truth - C_test)**2))
    
    cos_sims = []
    for i in range(nrow):
        a, b = C_truth[i], C_test[i]
        na, nb = np.linalg.norm(a), np.linalg.norm(b)
        if na > 1e-10 and nb > 1e-10:
            cos_sims.append(np.dot(a, b) / (na * nb))
    cos_mean = np.mean(cos_sims) if cos_sims else 0.0
    
    top1_match = top5_match = valid_rows = 0
    for i in range(nrow):
        if np.abs(C_truth[i]).max() < 1e-10: continue
        valid_rows += 1
        rank_t = np.argsort(-np.abs(C_truth[i]))
        rank_q = np.argsort(-np.abs(C_test[i]))
        if rank_t[0] == rank_q[0]: top1_match += 1
        if rank_t[0] in rank_q[:5]: top5_match += 1
    top1 = 100.0 * top1_match / max(valid_rows, 1)
    top5 = 100.0 * top5_match / max(valid_rows, 1)
    
    print(f"  [{label:22s}] MAPE={mape:8.4f}% p99={p99_rel:8.3f}% "
          f"Cos={cos_mean:.8f} Top1={top1:.1f}% Top5={top5:.1f}%")
    sys.stdout.flush()
    return mape, cos_mean, top1

# ===== MAIN =====
DATA = "/home/huangjianqiang_group/hdacp1/data/SpMM_project/data"
MATRICES = [
    "web-Google", "amazon0601", "cit-Patents", "as-Skitter",
    "soc-Pokec", "hollywood-2009", "indochina-2004"
]
K_VALUES = [32, 64, 128]
SAMPLE = 5000
SEED = 42

print("=" * 100)
print(f"INT8 Precision Simulation (numpy-vectorized, sample={SAMPLE})")
print("=" * 100)

for mat_name in MATRICES:
    path = f"{DATA}/{mat_name}/{mat_name}.csrbin"
    if not os.path.exists(path):
        print(f"\nSKIP: {mat_name}"); continue
    
    nrow, ncol, nnz, indptr, indices, values = read_csrbin(path)
    avg_deg = nnz / nrow
    print(f"\n{'='*80}")
    print(f"{mat_name}  M={nrow} N={ncol} NNZ={nnz} avg_deg={avg_deg:.1f}")
    print(f"{'='*80}")
    
    np.random.seed(SEED)
    if nrow > SAMPLE:
        row_ids = np.sort(np.random.choice(nrow, SAMPLE, replace=False))
    else:
        row_ids = np.arange(nrow)
    sampled_nnz = sum(indptr[i+1]-indptr[i] for i in row_ids)
    print(f"  Sampled {len(row_ids)} rows, NNZ={sampled_nnz}, avg_deg={sampled_nnz/len(row_ids):.1f}")
    
    for K in K_VALUES:
        print(f"\n  --- K={K} Uniform[0,1] ---")
        np.random.seed(SEED + K)
        B_fp32 = np.random.uniform(0, 1, (ncol, K)).astype(np.float32)
        
        t0 = time.time()
        C_truth = spmm_dense_subset(indptr, indices, values, B_fp32, row_ids)
        print(f"  FP64 ground truth: {time.time()-t0:.1f}s")
        
        # BF16
        t0 = time.time()
        C_bf16 = spmm_bf16(indptr, indices, values, B_fp32, row_ids)
        print(f"  BF16: {time.time()-t0:.1f}s")
        compute_metrics(C_truth, C_bf16, "BF16")
        
        # INT8 Per-Tensor
        B_i8_pt, sc_pt = quantize_per_tensor_sym(B_fp32)
        t0 = time.time()
        C_i8_pt = spmm_int8_pertensor(indptr, indices, values, B_i8_pt, sc_pt, row_ids)
        print(f"  INT8-PerTensor: {time.time()-t0:.1f}s")
        compute_metrics(C_truth, C_i8_pt, "INT8-PerTensor-Sym")
        
        # INT8 Per-Column Symmetric
        B_i8_pc, sc_pc = quantize_per_column_sym(B_fp32)
        t0 = time.time()
        C_i8_pc = spmm_int8_percol(indptr, indices, values, B_i8_pc, sc_pc, row_ids)
        print(f"  INT8-PerCol-Sym: {time.time()-t0:.1f}s")
        compute_metrics(C_truth, C_i8_pc, "INT8-PerCol-Sym")
        
        # INT8 Per-Column Asymmetric
        B_u8_pa, sc_pa, zp_pa = quantize_per_column_asym(B_fp32)
        t0 = time.time()
        C_i8_pa = spmm_int8_percol_asym(indptr, indices, values, B_u8_pa, sc_pa, zp_pa, row_ids)
        print(f"  INT8-PerCol-Asym: {time.time()-t0:.1f}s")
        compute_metrics(C_truth, C_i8_pa, "INT8-PerCol-Asym")
    
    # K=128 Normal distribution
    print(f"\n  --- K=128 Normal(0,1) ---")
    np.random.seed(SEED + 1000)
    B_normal = np.random.randn(ncol, 128).astype(np.float32)
    
    C_truth_n = spmm_dense_subset(indptr, indices, values, B_normal, row_ids)
    
    C_bf16_n = spmm_bf16(indptr, indices, values, B_normal, row_ids)
    compute_metrics(C_truth_n, C_bf16_n, "BF16-Normal")
    
    B_i8_n, sc_n = quantize_per_column_sym(B_normal)
    C_i8_n = spmm_int8_percol(indptr, indices, values, B_i8_n, sc_n, row_ids)
    compute_metrics(C_truth_n, C_i8_n, "INT8-PerCol-Normal")
    
    B_u8_an, sc_an, zp_an = quantize_per_column_asym(B_normal)
    C_i8_an = spmm_int8_percol_asym(indptr, indices, values, B_u8_an, sc_an, zp_an, row_ids)
    compute_metrics(C_truth_n, C_i8_an, "INT8-AsymCol-Normal")

print(f"\n{'='*100}")
print("=== ALL DONE ===")
