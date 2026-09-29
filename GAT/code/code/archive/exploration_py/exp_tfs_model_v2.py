#!/usr/bin/env python3
"""
TFS (Tiled Feature-Stationary) 精确模拟器 v2
=============================================
在 exp_featurestationary_model.py 基础上，修正两个关键遗漏：

遗漏1（专家2指出）：H cache line 跨 K_block 复用
  原模型：H 读取 = nnz × 128B × 1.5（假设 32B 读取浪费 50%）
  正确：若 R_group 的 unique 邻居集合 × 64B < L2_per_thread
        → K_block 0 把 H[j] 的 64B cache line 带入 L2
        → K_block 1/2/3 直接 L2 命中，0 额外 DRAM 流量
        → 有效 H 读取 = unique_neighbors × 64B（比原来还省！）

遗漏2（专家1指出）：W miss 才是高度数矩阵的主犯
  需精确建模：W_slice(8KB) + C_group(R×512B) + H_active 的 L1 竞争

核心输出：
  对每个 R 值（64/128/256），精确计算：
  1. 每 R_group 的 unique 邻居数（H working set 大小）
  2. H cache lines 是否驻留 L2（跨 K_block 复用是否成立）
  3. 有效总流量 vs 两步法
  4. 盈亏平衡 R 阈值

用法（登录节点直接跑）：
  python3 exp_tfs_model_v2.py \
    --data_dir /home/huangjianqiang_group/hdacp1/data/SpMM_project/data \
    --matrices web-Google amazon0601 cit-Patents as-Skitter soc-Pokec hollywood-2009

作者：自动生成
日期：2026-03-09
"""

import struct
import numpy as np
import os
import argparse

# ─────────────────────────────────────────────────────────────────────
# CSRbin 读取
# ─────────────────────────────────────────────────────────────────────
def read_csrbin(path):
    with open(path, 'rb') as f:
        f.read(12)
        nrow, ncol, nnz = struct.unpack('<QQQ', f.read(24))
        indptr  = np.frombuffer(f.read(4*(nrow+1)), dtype=np.uint32).copy()
        indices = np.frombuffer(f.read(4*nnz),      dtype=np.uint32).copy()
    return int(nrow), int(ncol), int(nnz), indptr, indices


# ─────────────────────────────────────────────────────────────────────
# 核心：采样 R_group 的 unique 邻居数分布
# ─────────────────────────────────────────────────────────────────────
def sample_unique_neighbors(indptr, indices, nrow, R,
                             n_sample=2000, seed=42):
    """
    对随机采样的 n_sample 个 R_group，统计每组的：
      - unique 邻居数（H working set 行数）
      - total 邻居数（NNZ 数，等于内层循环次数）

    返回：unique 数的统计数组
    """
    rng = np.random.default_rng(seed)
    n_groups = nrow // R
    if n_groups == 0:
        return np.array([nrow]), np.array([nnz])

    # 采样 group 起始行
    sampled = rng.choice(n_groups, size=min(n_sample, n_groups), replace=False)

    unique_counts = np.empty(len(sampled), dtype=np.int64)
    total_counts  = np.empty(len(sampled), dtype=np.int64)

    for k, g in enumerate(sampled):
        row_start = g * R
        row_end   = min(row_start + R, nrow)
        nnz_start = int(indptr[row_start])
        nnz_end   = int(indptr[row_end])
        nbrs = indices[nnz_start:nnz_end]
        unique_counts[k] = len(np.unique(nbrs))
        total_counts[k]  = nnz_end - nnz_start

    return unique_counts, total_counts


# ─────────────────────────────────────────────────────────────────────
# TFS 流量模型（带 cache line 复用）
# ─────────────────────────────────────────────────────────────────────
def model_tfs(name, nrow, ncol, nnz, indptr, indices,
              R_values=(64, 128, 256, 512),
              K=128, N_THREADS=32,
              L1=48*1024, L2=2*1024*1024, CACHELINE=64):
    """
    对每个 R 值建模 TFS 的有效内存流量。

    TFS 循环结构：
      for R_group (nrow/R 个组):
        for K_block (K/32=4 个块):
          load W_slice(8KB) → L1（整个 K_block 期间不变）
          for row i in R_group:
            C_tile(2KB) ← AMX 寄存器（不 spill）
            for neighbor j of i:
              gather H[j][kb*32:(kb+1)*32]（32B，但首次读整行 64B cache line）
            store C_tile → C_group(在 L2 中累加)
        write C_group → HBM（R_group 处理完后只写 1 次）

    关键：H[j] 的 64B cache line 在 K_block 0 被加载后，
          如果 unique_neighbors × 64B < L2_per_thread，
          则 K_block 1/2/3 全部 L2 命中 → 有效 H 读取 = unique × 64B
    """
    avg_deg = nnz / nrow
    BF16, FP32 = 2, 4
    H_row = K * BF16     # 256B
    C_row = K * FP32     # 512B
    W_tot = K * K * BF16 # 32KB
    K_BLOCKS = K // 32   # 4

    L2_per_thread = L2   # 每核独享 2MB（Sapphire Rapids 无超线程时）
    # 每线程处理的 R_group 数（近似均匀分配）
    groups_per_thread = max(1, (nrow // R_values[0]) // N_THREADS)

    print(f"\n{'='*70}")
    print(f"矩阵: {name}  nrow={nrow:,}  nnz={nnz:,}  avg_deg={avg_deg:.1f}")
    print(f"{'='*70}")

    results = {}

    for R in R_values:
        n_groups = max(1, nrow // R)

        # ── 采样 unique 邻居数 ──────────────────────────────────────
        unique_arr, total_arr = sample_unique_neighbors(
            indptr, indices, nrow, R, n_sample=2000)

        unique_mean = unique_arr.mean()
        unique_p50  = np.percentile(unique_arr, 50)
        unique_p95  = np.percentile(unique_arr, 95)
        total_mean  = total_arr.mean()   # ≈ R × avg_deg

        # ── Cache 层级分析 ──────────────────────────────────────────
        W_slice     = W_tot // K_BLOCKS            # 8KB，每 K_block 的 W 切片
        C_group     = R * C_row                    # R×512B
        H_active_CL = unique_mean * CACHELINE      # unique 邻居占用的 cache line

        # L1 装载情况（只需 W_slice + 当前 tile 的 H 行，不是全部）
        # 内层每次只处理 1 个邻居的 32B，但硬件预取 64B cache line
        # 关键：W_slice(8KB) 需要驻留 L1
        L1_for_H    = L1 - W_slice                 # W_slice 占后，L1 剩余
        # 每批 16 节点 × 当前步 H 需求（1 个邻居 × 64B cache line）
        H_batch_CL  = 16 * CACHELINE               # 最小单步 H 需求

        W_L1_safe   = (W_slice <= L1)              # W_slice 必须在 L1

        # L2 装载情况（C_group 必须驻留，H cache lines 尽量驻留）
        # 每线程同时有 groups_per_thread 个 R_group 在飞（保守取 1 个）
        L2_budget   = L2_per_thread
        C_in_L2     = C_group <= L2_budget * 0.5   # C_group 用不超过 L2 的一半
        L2_for_H    = L2_budget - C_group if C_in_L2 else 0
        H_CL_fits_L2 = H_active_CL <= L2_for_H    # H cache lines 能否全装进 L2

        # ── 流量计算 ────────────────────────────────────────────────
        # H 读取流量：
        if H_CL_fits_L2:
            # ✅ H cache lines 在 K_block 0 装入 L2 后，K_block 1/2/3 命中
            # 有效流量 = unique_neighbors × 64B（只读 1 次）
            H_traffic = n_groups * unique_mean * CACHELINE
            H_reuse_mode = f"L2复用✅ ({K_BLOCKS}×K_block共享, 有效读1次)"
        else:
            # ❌ H cache lines 在 K_block 间被 C_group 或其他数据挤出 L2
            # 有效流量 = nnz × 64B（每次 K_block 重新读）
            H_traffic = nnz * CACHELINE * K_BLOCKS
            H_reuse_mode = f"L2溢出❌ (H被驱逐, 每K_block重读)"

        # W 读取流量（W_slice 驻留 L1，每 R_group 换 1 次 K_block 时重新加载）
        # 每个 R_group 处理 K_BLOCKS 次，每次 load W_slice
        # 如果 W_slice 在 L1，则只有 K_block 切换时的 1 次 load
        W_traffic = n_groups * K_BLOCKS * W_slice  # 每 R_group 加载 4 次 W_slice
        # 但 W_slice 在 L1 期间被反复复用（R 个节点 × avg_deg 步）
        # 实际 W DRAM 流量只是 group 切换时的 load，≈ n_groups×K_BLOCKS×W_slice
        # 简化：如果 W 在 L1，则 DRAM W 流量 ≈ total W / (nrow/R) = W × (1/n_groups)
        # 更精确：只有 n_groups 次 W_slice 加载（每次8KB，4个K_block）
        W_dram = n_groups * K_BLOCKS * W_slice     # 实际是从 L2/L3 加载
        # 进一步优化：W_slice 可以预先放 L1，跨 R_group 复用
        # → W DRAM 流量 ≈ W_tot（总共只读 1 遍）
        W_dram_optimized = W_tot * K_BLOCKS        # 4 次 pass 各读一遍 W_tot
        # 取较小值
        W_dram = min(W_dram, W_dram_optimized)

        # C 流量：
        # - C_group 在 L2 中累加（K_BLOCKS 次写入 L2）
        # - 最终 1 次写回 HBM
        C_L2_accumulate = n_groups * K_BLOCKS * C_group * (1 if not C_in_L2 else 0.1)
        C_hbm_write     = nrow * C_row             # 最终写出，1 次

        # Z 节省（消灭中间矩阵）
        Z_saved = nrow * C_row * 2                 # Z 读 + Z 写

        total_TFS = H_traffic + W_dram + C_L2_accumulate + C_hbm_write

        # ── 对比两步法 ──────────────────────────────────────────────
        # 两步法流量：H读(nnz×256B) + Z写读(2×nrow×512B) + H'写(nrow×256B)
        total_2step = nnz * H_row + 2 * nrow * C_row + nrow * H_row

        ratio = total_2step / total_TFS
        beats_2step = total_TFS < total_2step

        # ── 打印 ────────────────────────────────────────────────────
        print(f"\n  ── R = {R} 行 ─────────────────────────────────────────────────")
        print(f"  Working Set 分析:")
        print(f"    W_slice   = {W_slice//1024}KB  {'✅ L1' if W_L1_safe else '❌ 超L1'}")
        print(f"    C_group   = {C_group//1024}KB  {'✅ L2' if C_in_L2 else '❌ 超L2一半'}")
        print(f"    H active CL = {H_active_CL/1024:.1f}KB (unique邻居{unique_mean:.0f}个 × 64B)")
        print(f"    H fits L2?  {H_active_CL/1024:.1f}KB vs L2剩余{L2_for_H/1024:.0f}KB → "
              f"{'✅' if H_CL_fits_L2 else '❌'}")
        print(f"")
        print(f"  H 访问模式: {H_reuse_mode}")
        print(f"    unique邻居/组: mean={unique_mean:.0f}  p50={unique_p50:.0f}  p95={unique_p95:.0f}")
        print(f"    total邻居/组:  mean={total_mean:.0f}  (= R×avg_deg≈{R*avg_deg:.0f})")
        print(f"")
        print(f"  流量估算:")
        print(f"    H 读取:   {H_traffic/1e9:.2f} GB")
        print(f"    W 读取:   {W_dram/1e9:.2f} GB  (W_slice 在 L1 复用)")
        print(f"    C 累加:   {C_L2_accumulate/1e9:.2f} GB  ({'L2驻留,低代价' if C_in_L2 else 'L2溢出,HBM代价'})")
        print(f"    C HBM写:  {C_hbm_write/1e9:.2f} GB")
        print(f"    ─────────────────────────────────────────────────────")
        print(f"    TFS总计:  {total_TFS/1e9:.2f} GB")
        print(f"    两步法:   {total_2step/1e9:.2f} GB")
        print(f"    TFS/两步: {ratio:.2f}×  {'✅ TFS更优!' if beats_2step else '❌ 两步法仍更优'}")
        if beats_2step:
            print(f"    ★ 理论加速: {ratio:.2f}× vs 两步法  ← Fusion真正有意义！")

        results[R] = {
            'unique_mean': unique_mean,
            'H_CL_fits_L2': H_CL_fits_L2,
            'C_in_L2': C_in_L2,
            'W_L1_safe': W_L1_safe,
            'total_TFS': total_TFS,
            'total_2step': total_2step,
            'ratio': ratio,
            'beats_2step': beats_2step,
        }

    return avg_deg, results


# ─────────────────────────────────────────────────────────────────────
# 全局盈亏平衡曲线
# ─────────────────────────────────────────────────────────────────────
def breakeven_curve():
    """
    解析推导：在 H cache line 复用成立的条件下，
    TFS vs 两步法的盈亏平衡度数 d*

    TFS总流量（H复用成立）:
      unique_neighbors × 64B + W_tot×4 + nrow×C_row
      unique ≈ nrow × (1 - (1-1/ncol)^(R×d)) ≈ 小图中 ≈ R×d × alpha
      （alpha = 列去重率，幂律图约 0.7-0.9）

    简化：unique = min(ncol, R × d)
    → H_traffic = n_groups × min(ncol, R×d) × 64B
                = (nrow/R) × min(ncol, R×d) × 64B

    两步法：nrow×d×256B + 2×nrow×512B + nrow×256B
          = nrow × (256d + 1280)

    令 TFS = 两步法：
      (nrow/R) × R×d × 64 + nrow×C_row = nrow×(256d + 1280)
      nrow×d×64 + nrow×512 = nrow×(256d + 1280)
      64d + 512 = 256d + 1280
      -192d = 768
      d* = -4 → TFS在任意d下都有正收益！（H复用成立时）

    实际要考虑 H 复用不成立的边界（unique×64B > L2/2）
    """
    print(f"\n\n{'='*70}")
    print("解析盈亏平衡（H cache line 复用成立时）")
    print(f"{'='*70}")
    K = 128
    BF16, FP32 = 2, 4
    H_row = K * BF16    # 256B
    C_row = K * FP32    # 512B
    CACHELINE = 64
    K_BLOCKS = K // 32  # 4

    print(f"""
设 N=节点数, d=avg_deg, R=行块大小, unique≈R×d（邻居去重后）

[两步法总流量]
  H读取:   N×d×256B
  Z读写:   2×N×512B = 1024N
  H'写出:  N×256B
  合计:    N×(256d + 1280)

[TFS总流量（H复用✅，unique邻居驻L2）]
  H读取:   (N/R)×unique×64B ≈ (N/R)×(R×d)×64B = N×d×64B
  W读取:   W_tot×4 = 32KB×4 = 128KB ≈ 0（相对N很小）
  C累加:   ≈ 0（驻L2）
  C写出:   N×512B
  合计:    N×(64d + 512)

盈亏平衡：64d + 512 = 256d + 1280
  → -192d = 768
  → d* = -4（负数！）

结论：当 H cache line 复用成立时：
  ✅ TFS 在任意正度数下都比两步法流量少！
  理论收益：
    d=5:   两步法={256*5+1280}N，TFS={64*5+512}N，TFS节省{(256*5+1280-(64*5+512))/(256*5+1280)*100:.0f}%
    d=10:  两步法={256*10+1280}N，TFS={64*10+512}N，TFS节省{(256*10+1280-(64*10+512))/(256*10+1280)*100:.0f}%
    d=100: 两步法={256*100+1280}N，TFS={64*100+512}N，TFS节省{(256*100+1280-(64*100+512))/(256*100+1280)*100:.0f}%
""")

    print("H复用成立的条件（R_group的 unique邻居×64B < L2/2）：")
    L2_half = 2*1024*1024 // 2
    for d in [5, 10, 20, 50, 100]:
        for R in [64, 128, 256]:
            unique_approx = R * d * 0.8  # 假设 80% 去重率
            H_CL = unique_approx * 64
            fits = "✅" if H_CL <= L2_half else "❌"
            print(f"  d={d:3d}, R={R}: unique≈{unique_approx:.0f}, H_CL={H_CL/1024:.0f}KB "
                  f"vs L2/2={L2_half//1024}KB → {fits}")
        print()


# ─────────────────────────────────────────────────────────────────────
# 汇总表
# ─────────────────────────────────────────────────────────────────────
def print_summary(all_results):
    print(f"\n\n{'='*78}")
    print("跨矩阵汇总（最优 R 选择）")
    print(f"{'='*78}")
    print(f"{'矩阵':<22} {'avg_deg':>8} {'最优R':>7} {'H_L2复用':>9} "
          f"{'TFS(GB)':>9} {'2step(GB)':>10} {'TFS/2step':>10} {'结论':>6}")
    print('-'*78)

    for name, (avg_deg, r_results) in all_results.items():
        # 找最优 R（TFS/两步法 最大且 beats_2step）
        best_R = None
        best_ratio = 0
        for R, r in r_results.items():
            if r['beats_2step'] and r['ratio'] > best_ratio:
                best_ratio = r['ratio']
                best_R = R
        if best_R is None:
            # 没有 beats_2step 的，选流量比最小的
            best_R = min(r_results, key=lambda R: r_results[R]['total_TFS'])

        r = r_results[best_R]
        print(f"{name:<22} {avg_deg:>8.1f} {best_R:>7} "
              f"{'✅' if r['H_CL_fits_L2'] else '❌':>9} "
              f"{r['total_TFS']/1e9:>9.2f} "
              f"{r['total_2step']/1e9:>10.2f} "
              f"{r['ratio']:>10.2f}× "
              f"{'✅快' if r['beats_2step'] else '❌慢':>6}")

    print(f"\n说明: TFS/2step > 1.0× 表示 TFS 比两步法流量少（Fusion有意义）")


# ─────────────────────────────────────────────────────────────────────
# 主程序
# ─────────────────────────────────────────────────────────────────────
def main():
    parser = argparse.ArgumentParser(description='TFS 精确流量模型 v2')
    parser.add_argument('--data_dir', default='.',
                        help='数据根目录')
    parser.add_argument('--matrices', nargs='+',
                        default=['web-Google', 'amazon0601', 'cit-Patents',
                                 'as-Skitter', 'soc-Pokec', 'hollywood-2009'])
    parser.add_argument('--R_values', nargs='+', type=int,
                        default=[64, 128, 256, 512])
    parser.add_argument('--K', type=int, default=128)
    args = parser.parse_args()

    print("TFS (Tiled Feature-Stationary) 精确流量模型 v2")
    print(f"K={args.K}  L1=48KB  L2=2MB/core  CACHELINE=64B  THR=32")
    print(f"R候选: {args.R_values} 行")
    print(f"关键新假设: H cache line 在 L2 中跨 K_block 复用（专家2洞见）")

    all_results = {}

    for name in args.matrices:
        path = os.path.join(args.data_dir, name, f'{name}.csrbin')
        if not os.path.exists(path):
            print(f"\n[SKIP] {name}: {path}")
            continue
        nrow, ncol, nnz, indptr, indices = read_csrbin(path)
        avg_deg, r_results = model_tfs(
            name, nrow, ncol, nnz, indptr, indices,
            R_values=args.R_values, K=args.K)
        all_results[name] = (avg_deg, r_results)

    print_summary(all_results)
    breakeven_curve()


if __name__ == '__main__':
    main()
