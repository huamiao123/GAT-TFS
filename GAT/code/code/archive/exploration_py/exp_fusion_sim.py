"""
算子融合验证脚本：SpMM + Dense GeMM 融合
验证目标：
  1. 计算/访存比：融合后是否达到 compute-bound
  2. 理论加速比：vs 当前纯 SpMM
  3. numpy 实际运行时间对比（作为 proxy）
矩阵：web-Google, amazon0601
"""
import numpy as np
import struct
import os
import time

# ============================================================
# 工具函数
# ============================================================

def load_csrbin(path):
    with open(path, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('3I', f.read(12))
        nrow, ncol, nnz    = struct.unpack('3Q', f.read(24))
        indptr  = np.frombuffer(f.read((nrow+1)*4), dtype=np.uint32).astype(np.int64)
        indices = np.frombuffer(f.read(nnz*4),      dtype=np.uint32).astype(np.int64)
        values  = np.ones(nnz, dtype=np.float32)
    return indptr, indices, values, int(nrow), int(ncol), int(nnz)

def timer(func, *args, repeat=3):
    """多次计时取最小值，单位 ms"""
    best = 1e9
    for _ in range(repeat):
        t0 = time.perf_counter()
        result = func(*args)
        t1 = time.perf_counter()
        best = min(best, (t1 - t0) * 1000)
    return best, result

# ============================================================
# 三种计算方式
# ============================================================

def method_spmm_then_gemm(indptr, indices, values, H, W):
    """
    标准两步：
      Z = A × H   (SpMM)
      H' = Z × W  (Dense GeMM)
    """
    M = len(indptr) - 1
    K = H.shape[1]
    # Step1: SpMM
    Z = np.zeros((M, K), dtype=np.float32)
    row_ids = np.repeat(np.arange(M, dtype=np.int64), np.diff(indptr))
    np.add.at(Z, row_ids, values[:, None] * H[indices])
    # Step2: Dense GeMM
    H_out = Z @ W
    return H_out

def method_fused(indptr, indices, values, H, W):
    """
    融合：对每个邻居 j，先算 H[j,:] × W，再累加到 H'[i,:]
    H'[i,:] = Σ_j A[i,j] × (H[j,:] × W)
    等价于标准两步（线性等价），但避免写出中间结果 Z
    """
    M = len(indptr) - 1
    K_out = W.shape[1]
    # 先把所有 B 行变换：H_transformed[j,:] = H[j,:] × W
    # 在真实 AMX 实现中，这一步和 gather 融合；
    # 这里为了 numpy 模拟，先批量变换再聚合
    H_transformed = H @ W  # (N, K_out)，模拟 W 驻留 L1
    H_out = np.zeros((M, K_out), dtype=np.float32)
    row_ids = np.repeat(np.arange(M, dtype=np.int64), np.diff(indptr))
    np.add.at(H_out, row_ids, values[:, None] * H_transformed[indices])
    return H_out

def method_fused_true(indptr, indices, values, H, W):
    """
    真融合：逐行处理，每个邻居 gather 后立即 × W
    模拟真实 AMX kernel 的执行顺序
    注意：这里用 numpy 模拟，速度较慢，但体现真实数据流
    """
    M = len(indptr) - 1
    K_out = W.shape[1]
    H_out = np.zeros((M, K_out), dtype=np.float32)
    for i in range(M):
        s, e = indptr[i], indptr[i+1]
        if s == e:
            continue
        # gather 邻居特征
        H_neighbors = H[indices[s:e]]          # (deg, K)
        # 融合：先聚合再变换（等价于先变换再聚合）
        H_agg = values[s:e] @ H_neighbors      # (K,)
        H_out[i] = H_agg @ W                   # (K_out,)
    return H_out

# ============================================================
# 理论分析
# ============================================================

def theoretical_analysis(name, M, N, nnz, K, K_out, avg_deg):
    print(f"\n  {'─'*56}")
    print(f"  理论分析（K={K}, K_out={K_out}）")
    print(f"  {'─'*56}")

    bytes_per_elem = 2  # BF16

    # ── 方法1：SpMM then GeMM ──────────────────────────────
    # SpMM: 每个 NNZ 读一次 H[j,:] → nnz × K × 2 bytes
    spmm_read   = nnz * K * bytes_per_elem
    # SpMM: 写 Z → M × K × 2 bytes
    spmm_write  = M * K * bytes_per_elem
    # GeMM: 读 Z → M × K × 2 bytes
    gemm_read_Z = M * K * bytes_per_elem
    # GeMM: 读 W → K × K_out × 2 bytes（一次，可缓存）
    gemm_read_W = K * K_out * bytes_per_elem
    # GeMM: 写 H_out → M × K_out × 2 bytes
    gemm_write  = M * K_out * bytes_per_elem

    total_bytes_2step = spmm_read + spmm_write + gemm_read_Z + gemm_read_W + gemm_write
    total_flops_2step = (2 * nnz * K) + (2 * M * K * K_out)
    ratio_2step = total_flops_2step / total_bytes_2step

    print(f"\n  [方法1] SpMM → GeMM 两步")
    print(f"    SpMM  读 H：       {spmm_read/1e9:.3f} GB")
    print(f"    SpMM  写 Z：       {spmm_write/1e9:.3f} GB")
    print(f"    GeMM  读 Z：       {gemm_read_Z/1e9:.3f} GB")
    print(f"    GeMM  读 W：       {gemm_read_W/1e6:.1f} MB（可 L2 缓存）")
    print(f"    GeMM  写 H_out：   {gemm_write/1e9:.3f} GB")
    print(f"    总数据搬运：       {total_bytes_2step/1e9:.3f} GB")
    print(f"    总 FLOP：          {total_flops_2step/1e9:.1f} GFLOP")
    print(f"    计算/访存比：      {ratio_2step:.2f} FMAs/byte")

    # ── 方法2：融合 ────────────────────────────────────────
    # gather H[j,:]: nnz × K × 2 bytes（同 SpMM）
    fused_read_H  = nnz * K * bytes_per_elem
    # W 驻留 L1（32KB for K=128,K_out=128），不计入主存访问
    # 写 H_out: M × K_out × 2 bytes
    fused_write   = M * K_out * bytes_per_elem
    # 消除了：写 Z（M×K×2）+ 读 Z（M×K×2）

    total_bytes_fused = fused_read_H + fused_write
    total_flops_fused = 2 * nnz * K * K_out  # 每个NNZ做 K×K_out FMAs
    ratio_fused = total_flops_fused / total_bytes_fused

    eliminated = spmm_write + gemm_read_Z
    print(f"\n  [方法2] 融合 kernel")
    print(f"    gather 读 H：      {fused_read_H/1e9:.3f} GB")
    print(f"    W 驻留 L1：        {gemm_read_W/1e6:.1f} MB ✅ 不计入主存")
    print(f"    写 H_out：         {fused_write/1e9:.3f} GB")
    print(f"    总数据搬运：       {total_bytes_fused/1e9:.3f} GB")
    print(f"    消除的中间 IO：    {eliminated/1e9:.3f} GB ← Z的写+读")
    print(f"    总 FLOP：          {total_flops_fused/1e9:.1f} GFLOP")
    print(f"    计算/访存比：      {ratio_fused:.2f} FMAs/byte")

    # ── 对比 ───────────────────────────────────────────────
    print(f"\n  [对比]")
    print(f"    计算/访存比提升：  {ratio_fused/ratio_2step:.1f}×")
    print(f"    数据搬运减少：     {total_bytes_2step/total_bytes_fused:.2f}×")

    # AMX 峰值计算
    hbm_bw   = 1e12          # 1 TB/s HBM
    amx_peak = 167e12        # 167 TFLOPS BF16

    time_bound_mem    = total_bytes_fused  / hbm_bw * 1000   # ms
    time_bound_compute= total_flops_fused  / amx_peak * 1000 # ms

    print(f"\n  [融合 kernel 性能估算]")
    print(f"    内存下界（HBM 1TB/s）：  {time_bound_mem:.2f} ms")
    print(f"    计算下界（AMX 167T）：   {time_bound_compute:.2f} ms")
    if time_bound_compute > time_bound_mem:
        bottleneck = "compute-bound ✅ AMX满载！"
        roofline_ms = time_bound_compute
    else:
        bottleneck = "memory-bound ⚠️"
        roofline_ms = time_bound_mem
    print(f"    瓶颈：{bottleneck}")
    print(f"    Roofline 预测时间：      {roofline_ms:.2f} ms")

    return ratio_fused, roofline_ms

# ============================================================
# 主流程
# ============================================================

def analyze_matrix(name, path, K=128, K_out=128):
    print(f"\n{'='*60}")
    print(f"  矩阵：{name}")
    print(f"{'='*60}")

    indptr, indices, values, M, N, nnz = load_csrbin(path)
    avg_deg = nnz / M
    print(f"  M={M:,}  NNZ={nnz:,}  avg_deg={avg_deg:.1f}")

    # 随机初始化
    rng = np.random.default_rng(42)
    H = rng.standard_normal((N, K)).astype(np.float32) * 0.1
    W = rng.standard_normal((K, K_out)).astype(np.float32) * 0.1

    # ── 理论分析 ──────────────────────────────────────────
    ratio_fused, roofline_ms = theoretical_analysis(
        name, M, N, nnz, K, K_out, avg_deg)

    # ── 正确性验证 ────────────────────────────────────────
    print(f"\n  [正确性验证]")
    H_ref   = method_spmm_then_gemm(indptr, indices, values, H, W)
    H_fused = method_fused(indptr, indices, values, H, W)
    diff = np.abs(H_ref - H_fused).max()
    rel  = diff / (np.abs(H_ref).max() + 1e-8)
    print(f"    两步 vs 融合 最大误差：{diff:.2e}  相对误差：{rel:.2e}")
    print(f"    {'✅ 数学等价' if rel < 1e-4 else '❌ 误差过大'}")

    # ── numpy 运行时间（proxy）──────────────────────────────
    print(f"\n  [numpy 运行时间对比（repeat=3，取最小）]")

    t_2step, _ = timer(method_spmm_then_gemm,
                       indptr, indices, values, H, W, repeat=3)
    t_fused, _ = timer(method_fused,
                       indptr, indices, values, H, W, repeat=3)

    print(f"    两步（SpMM+GeMM）：  {t_2step:.1f} ms")
    print(f"    融合 kernel：        {t_fused:.1f} ms")
    print(f"    融合 vs 两步：       {t_2step/t_fused:.2f}×")
    print(f"    注：numpy 不代表 AMX 性能，仅验证等价性和数据流")

    # ── 关键结论 ──────────────────────────────────────────
    print(f"\n  [核心结论]")
    print(f"    计算/访存比：{ratio_fused:.1f} FMAs/byte（>1 即 compute-bound）")
    if ratio_fused > 10:
        print(f"    → 强 compute-bound，AMX 满载，gather 延迟被完全隐藏 ✅")
    elif ratio_fused > 1:
        print(f"    → 轻度 compute-bound，AMX 有收益 ⚠️")
    else:
        print(f"    → memory-bound，AMX 仍然受限 ❌")
    print(f"    Roofline 预测：{roofline_ms:.2f} ms")

if __name__ == '__main__':
    DATA = '/home/huangjianqiang_group/hdacp1/data/SpMM_project/data'
    matrices = [
        ('web-Google',  f'{DATA}/web-Google/web-Google.csrbin'),
        ('amazon0601',  f'{DATA}/amazon0601/amazon0601.csrbin'),
    ]
    for name, path in matrices:
        if os.path.exists(path):
            analyze_matrix(name, path, K=128, K_out=128)
        else:
            print(f"文件不存在：{path}")
    print("\n\n=== 完成 ===")
