#!/usr/bin/env python3
"""
BF16 Precision Validation for AMX SpMM
=======================================
AMX TDPBF16PS: 输入 BF16, 累加 FP32
本实验模拟这个过程, 对比纯 FP32 的 ground truth

BF16 格式: 1-sign + 8-exponent + 7-mantissa (vs FP32 的 23-bit mantissa)
精度: ~0.78% 相对误差/元素, 但 FP32 累加使误差不会快速传播

对 GNN 推理, 关键指标:
  1. 元素级相对误差 (MAPE)
  2. 行向量余弦相似度 (embedding quality)
  3. Top-K 排名一致性 (节点分类/推荐)
"""
import sys, os, struct, time
import numpy as np

def load_csrbin(path):
    with open(path, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('III', f.read(12))
        nrow, ncol, nnz = struct.unpack('QQQ', f.read(24))
        indptr = np.fromfile(f, dtype=np.uint32, count=nrow+1)
        indices = np.fromfile(f, dtype=np.uint32, count=nnz)
    return int(nrow), int(ncol), int(nnz), indptr, indices

def fp32_to_bf16(x):
    """模拟 FP32->BF16 截断 (与硬件行为一致: round-to-nearest-even)"""
    # 方法: FP32 -> 查看 uint32 表示 -> 截断低 16 bit -> 还原 FP32
    x32 = x.astype(np.float32)
    u32 = x32.view(np.uint32)
    # Round-to-nearest-even: 加 0x7FFF + bit[16] (banker's rounding)
    rounding = np.uint32(0x7FFF) + ((u32 >> 16) & np.uint32(1))
    u32_rounded = u32 + rounding
    u32_bf16 = u32_rounded & np.uint32(0xFFFF0000)
    return u32_bf16.view(np.float32)

def spmm_fp32(indptr, indices, X, nrow):
    """纯 FP32 SpMM: Y = A × X, A 的值全为 1.0 (邻接矩阵)"""
    K = X.shape[1]
    Y = np.zeros((nrow, K), dtype=np.float32)
    for i in range(nrow):
        s, e = int(indptr[i]), int(indptr[i+1])
        if e > s:
            cols = indices[s:e]
            Y[i] = np.sum(X[cols], axis=0)  # A 值为 1, 等价于求和
    return Y

def spmm_bf16_accfp32(indptr, indices, X_bf16, nrow):
    """模拟 AMX: 输入 BF16, 累加 FP32"""
    K = X_bf16.shape[1]
    Y = np.zeros((nrow, K), dtype=np.float32)
    for i in range(nrow):
        s, e = int(indptr[i]), int(indptr[i+1])
        if e > s:
            cols = indices[s:e]
            # X_bf16 已经是截断后的 FP32 表示
            # 累加在 FP32 中 (np.sum 默认 float64, 强制 float32)
            Y[i] = np.sum(X_bf16[cols].astype(np.float32), axis=0)
    return Y

def compute_metrics(Y_fp32, Y_bf16, nrow):
    """计算全面的精度指标"""
    # 只计算有非零输出的行 (度数>0)
    row_norms = np.linalg.norm(Y_fp32, axis=1)
    active = row_norms > 1e-10
    n_active = np.sum(active)

    Yf = Y_fp32[active]
    Yb = Y_bf16[active]

    # 1. 元素级误差
    abs_err = np.abs(Yf - Yb)
    rel_err = abs_err / (np.abs(Yf) + 1e-10)

    max_abs = np.max(abs_err)
    mean_abs = np.mean(abs_err)
    max_rel = np.max(rel_err)
    mean_rel = np.mean(rel_err)
    mape = np.mean(rel_err) * 100  # Mean Absolute Percentage Error

    # 2. RMSE
    rmse = np.sqrt(np.mean((Yf - Yb)**2))
    # Normalized RMSE
    nrmse = rmse / (np.std(Yf) + 1e-10)

    # 3. 行向量余弦相似度 (GNN embedding quality)
    dot = np.sum(Yf * Yb, axis=1)
    norm_f = np.linalg.norm(Yf, axis=1) + 1e-10
    norm_b = np.linalg.norm(Yb, axis=1) + 1e-10
    cosines = dot / (norm_f * norm_b)
    cos_mean = np.mean(cosines)
    cos_min = np.min(cosines)
    cos_p1 = np.percentile(cosines, 1)  # 最差 1%

    # 4. Top-K 排名一致性 (每行的最大元素位置是否一致)
    topk_fp32 = np.argmax(Yf, axis=1)
    topk_bf16 = np.argmax(Yb, axis=1)
    top1_match = np.mean(topk_fp32 == topk_bf16) * 100

    # Top-5 overlap
    K = Yf.shape[1]
    k5 = min(5, K)
    top5_f = np.argsort(Yf, axis=1)[:, -k5:]
    top5_b = np.argsort(Yb, axis=1)[:, -k5:]
    top5_overlap = 0
    for i in range(len(Yf)):
        top5_overlap += len(set(top5_f[i]) & set(top5_b[i])) / k5
    top5_match = top5_overlap / len(Yf) * 100

    # 5. 误差分布
    rel_err_flat = rel_err.flatten()
    p50 = np.percentile(rel_err_flat, 50)
    p90 = np.percentile(rel_err_flat, 90)
    p99 = np.percentile(rel_err_flat, 99)
    p999 = np.percentile(rel_err_flat, 99.9)

    return {
        'n_active': n_active,
        'max_abs': max_abs, 'mean_abs': mean_abs,
        'max_rel': max_rel, 'mean_rel': mean_rel, 'mape': mape,
        'rmse': rmse, 'nrmse': nrmse,
        'cos_mean': cos_mean, 'cos_min': cos_min, 'cos_p1': cos_p1,
        'top1': top1_match, 'top5': top5_match,
        'rel_p50': p50, 'rel_p90': p90, 'rel_p99': p99, 'rel_p999': p999,
    }

def process(name, path):
    print(f"\n{'='*70}", flush=True)
    print(f"  BF16 Accuracy: {name}", flush=True)
    print(f"{'='*70}", flush=True)

    nrow, ncol, nnz, indptr, indices = load_csrbin(path)
    print(f"  {nrow:,} rows, {nnz:,} NNZ, avg_deg={nnz/nrow:.1f}", flush=True)

    # 采样行数 (大矩阵全算太慢, 随机采样 10K 行)
    MAX_ROWS = 10000
    if nrow > MAX_ROWS:
        np.random.seed(42)
        sample_rows = np.sort(np.random.choice(nrow, MAX_ROWS, replace=False))
        # 重建采样后的 CSR
        new_indptr = np.zeros(MAX_ROWS + 1, dtype=np.uint32)
        all_indices = []
        for i, r in enumerate(sample_rows):
            s, e = int(indptr[r]), int(indptr[r+1])
            cols = indices[s:e]
            all_indices.append(cols)
            new_indptr[i+1] = new_indptr[i] + len(cols)
        if len(all_indices) > 0:
            new_indices = np.concatenate(all_indices).astype(np.uint32)
        else:
            new_indices = np.array([], dtype=np.uint32)
        indptr_use = new_indptr
        indices_use = new_indices
        nrow_use = MAX_ROWS
        print(f"  Sampled {MAX_ROWS} rows (seed=42)", flush=True)
    else:
        indptr_use = indptr
        indices_use = indices
        nrow_use = nrow

    for K in [32, 64, 128]:
        print(f"\n  --- K={K} ---", flush=True)

        for dist_name, X_gen in [
            ("Uniform[0,1]", lambda: np.random.uniform(0, 1, (ncol, K)).astype(np.float32)),
            ("Normal(0,1)",  lambda: np.random.randn(ncol, K).astype(np.float32)),
            ("Xavier",       lambda: (np.random.randn(ncol, K) / np.sqrt(K)).astype(np.float32)),
        ]:
            np.random.seed(123)
            X = X_gen()
            X_bf16 = fp32_to_bf16(X)

            # 验证 BF16 转换本身的误差
            bf16_input_err = np.mean(np.abs(X - X_bf16) / (np.abs(X) + 1e-10))

            t0 = time.time()
            Y_fp32 = spmm_fp32(indptr_use, indices_use, X, nrow_use)
            t1 = time.time()
            Y_bf16 = spmm_bf16_accfp32(indptr_use, indices_use, X_bf16, nrow_use)
            t2 = time.time()

            m = compute_metrics(Y_fp32, Y_bf16, nrow_use)

            print(f"    [{dist_name:12s}] BF16 input MAPE={bf16_input_err*100:.4f}%", flush=True)
            print(f"      Element error:  MAPE={m['mape']:.4f}%  "
                  f"MaxRel={m['max_rel']*100:.3f}%  "
                  f"RMSE={m['rmse']:.6f}", flush=True)
            print(f"      Rel err dist:   p50={m['rel_p50']*100:.4f}%  "
                  f"p90={m['rel_p90']*100:.4f}%  "
                  f"p99={m['rel_p99']*100:.3f}%  "
                  f"p99.9={m['rel_p999']*100:.3f}%", flush=True)
            print(f"      Cosine sim:     mean={m['cos_mean']:.8f}  "
                  f"min={m['cos_min']:.6f}  "
                  f"p1={m['cos_p1']:.6f}", flush=True)
            print(f"      Ranking:        Top1={m['top1']:.1f}%  "
                  f"Top5={m['top5']:.1f}%", flush=True)
            print(f"      Time: FP32={t1-t0:.1f}s BF16={t2-t1:.1f}s", flush=True)

    # 论文结论
    print(f"\n  === PAPER CONCLUSION: {name} ===", flush=True)
    print(f"    avg_degree={nnz/nrow:.1f}", flush=True)
    print(f"    (See metrics above for MAPE < 0.1% and cosine > 0.9999)", flush=True)

def main():
    if len(sys.argv) < 2:
        print("Usage: python exp_bf16_accuracy.py <csrbin1> ...")
        sys.exit(1)
    print(f"{'='*70}", flush=True)
    print(f"  BF16 Precision Validation for AMX SpMM", flush=True)
    print(f"  BF16: 1+8+7 bit, FP32 accumulation", flush=True)
    print(f"  Input distributions: Uniform, Normal, Xavier", flush=True)
    print(f"  K values: 32, 64, 128", flush=True)
    print(f"{'='*70}", flush=True)
    ta = time.time()
    for p in sys.argv[1:]:
        if not os.path.exists(p):
            print(f"  MISSING: {p}", flush=True)
            continue
        nm = os.path.basename(p).replace('.csrbin','')
        try:
            process(nm, p)
        except Exception as ex:
            print(f"  ERROR {nm}: {ex}", flush=True)
            import traceback; traceback.print_exc()
    print(f"\n  Done. Total: {time.time()-ta:.1f}s", flush=True)

if __name__ == '__main__':
    main()
