"""
验证一：采样 SpMM 精度实验（纯 numpy，无需 scipy）
"""
import numpy as np
import struct
import os

def load_csrbin(path):
    with open(path, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('3I', f.read(12))
        nrow, ncol, nnz = struct.unpack('3Q', f.read(24))
        indptr  = np.frombuffer(f.read((nrow+1)*4), dtype=np.uint32).astype(np.int64)
        indices = np.frombuffer(f.read(nnz*4),      dtype=np.uint32).astype(np.int64)
        values  = np.ones(nnz, dtype=np.float32)
    return indptr, indices, values, int(nrow), int(ncol), int(nnz)

def full_spmm_numpy(indptr, indices, values, B):
    """纯 numpy 向量化 SpMM，作为 ground truth"""
    M = len(indptr) - 1
    K = B.shape[1]
    C = np.zeros((M, K), dtype=np.float32)
    # 利用 np.add.at 做稀疏累加
    # C[i] += val * B[j]  等价于：对每个NNZ，把 val*B[j] 加到 C[i]
    row_ids = np.repeat(np.arange(M), np.diff(indptr))  # 每个NNZ对应的行号
    np.add.at(C, row_ids, values[:, None] * B[indices])
    return C

def sampled_spmm(indptr, indices, values, B, S, seed=42):
    """
    采样 SpMM：每个节点固定采样 S 个邻居
    - 度数 >= S：随机采样 S 个（不放回），均匀权重 1/S
    - 度数 <  S：全部使用，均匀权重 1/deg
    """
    rng = np.random.default_rng(seed)
    M = len(indptr) - 1
    K = B.shape[1]
    C = np.zeros((M, K), dtype=np.float32)
    deg = np.diff(indptr)

    for i in range(M):
        d = deg[i]
        if d == 0:
            continue
        s, e = indptr[i], indptr[i+1]
        cols = indices[s:e]
        if d <= S:
            w = np.float32(1.0 / d)
            C[i] = w * B[cols].sum(axis=0)
        else:
            idx = rng.choice(d, size=S, replace=False)
            w = np.float32(1.0 / S)
            C[i] = w * B[cols[idx]].sum(axis=0)
    return C

def compute_error(C_ref, C_sampled):
    row_norms = np.linalg.norm(C_ref, axis=1)
    mask = row_norms > 1e-6
    diff = np.abs(C_ref[mask] - C_sampled[mask])
    rel = diff / row_norms[mask, None]
    return {
        'mean':   float(np.mean(rel)),
        'median': float(np.median(rel)),
        'p95':    float(np.percentile(rel, 95)),
        'max':    float(np.max(rel)),
    }

def analyze_matrix(name, path, K=32, S_list=[4, 8, 16, 32, 64]):
    print(f"\n{'='*60}")
    print(f"  矩阵：{name}")
    print(f"{'='*60}")

    indptr, indices, values, M, N, nnz = load_csrbin(path)
    deg = np.diff(indptr)
    avg_deg = nnz / M
    print(f"  M={M:,}  NNZ={nnz:,}  avg_deg={avg_deg:.1f}")
    print(f"  度数分布：min={deg.min()}  "
          f"p25={int(np.percentile(deg,25))}  "
          f"median={int(np.median(deg))}  "
          f"p75={int(np.percentile(deg,75))}  "
          f"p95={int(np.percentile(deg,95))}  "
          f"max={deg.max()}")

    rng = np.random.default_rng(0)
    B = rng.standard_normal((N, K)).astype(np.float32)

    print(f"\n  计算全图 SpMM ground truth (K={K})...")
    C_ref = full_spmm_numpy(indptr, indices, values, B)
    print(f"  完成，C shape={C_ref.shape}")

    print(f"\n  {'S':>4} | {'覆盖率(deg>=S)':>14} | "
          f"{'mean_rel_err':>12} | {'median':>8} | "
          f"{'p95':>8} | {'max':>8}")
    print(f"  {'-'*70}")

    for S in S_list:
        coverage = float(np.mean(deg >= S)) * 100
        C_s = sampled_spmm(indptr, indices, values, B, S)
        err = compute_error(C_ref, C_s)
        print(f"  {S:>4} | {coverage:>13.1f}% | "
              f"{err['mean']:>12.4f} | {err['median']:>8.4f} | "
              f"{err['p95']:>8.4f} | {err['max']:>8.4f}")

    print(f"\n  === AMX 性能理论估算（S=32, K=128）===")
    print(f"  全图 gather：{nnz * 128 * 2 / 1e9:.2f} GB")
    print(f"  采样 gather：{M * 32 * 128 * 2 / 1e9:.2f} GB")
    print(f"  AMX tile 利用率：{min(32,32)/32*100:.0f}%（S=32 完美对齐）")
    print(f"  理论有效 TFLOPS = 167 × 100% = 167 TFLOPS")

if __name__ == '__main__':
    DATA = '/home/huangjianqiang_group/hdacp1/data/SpMM_project/data'
    matrices = [
        ('web-Google',  f'{DATA}/web-Google/web-Google.csrbin'),
        ('amazon0601',  f'{DATA}/amazon0601/amazon0601.csrbin'),
    ]
    for name, path in matrices:
        if os.path.exists(path):
            analyze_matrix(name, path)
        else:
            print(f"文件不存在：{path}")
    print("\n=== 完成 ===")
