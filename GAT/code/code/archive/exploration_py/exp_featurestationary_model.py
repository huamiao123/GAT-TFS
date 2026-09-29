#!/usr/bin/env python3
"""
Feature-Stationary Fusion 理论模型
====================================
对比三种方案的内存流量：
  A) 当前 Fusion v3/v7（行中心，C反复spill）
  B) Feature-Stationary（特征块外循环，C只store 1次）
  C) 两步法最优（V17c SpMM + Dense GeMM）

对每个矩阵量化：
  1. C spill 流量差异
  2. KC-Fusion 损失（H读取次数变化）
  3. W 驻留 L1 的收益（W cache miss 减少）
  4. 净收益预测 & 盈亏平衡度数阈值

用法（登录节点直接跑，无需SLURM）：
  python3 exp_featurestationary_model.py \
    --data_dir /home/huangjianqiang_group/hdacp1/data/SpMM_project/data \
    --matrices web-Google amazon0601 cit-Patents as-Skitter soc-Pokec hollywood-2009 indochina

作者：自动生成
日期：2026-03-09
"""

import struct
import numpy as np
import os
import argparse

# ──────────────────────────────────────────────────────────
# CSRbin 读取
# ──────────────────────────────────────────────────────────
def read_csrbin(path):
    with open(path, 'rb') as f:
        f.read(12)                                          # header
        nrow, ncol, nnz = struct.unpack('<QQQ', f.read(24))
        indptr  = np.frombuffer(f.read(4*(nrow+1)), dtype=np.uint32).copy()
        indices = np.frombuffer(f.read(4*nnz),      dtype=np.uint32).copy()
    return int(nrow), int(ncol), int(nnz), indptr, indices


# ──────────────────────────────────────────────────────────
# 从CSR提取度数分布（用于精确建模）
# ──────────────────────────────────────────────────────────
def get_deg_stats(indptr, nrow):
    deg = np.diff(indptr.astype(np.int64))
    return deg


# ──────────────────────────────────────────────────────────
# 核心理论模型
# ──────────────────────────────────────────────────────────
def model_feature_stationary(name, nrow, ncol, nnz, indptr, indices,
                              K=128, TILE_H=16, N_THREADS=32,
                              L1_BYTES=48*1024, L2_BYTES=2*1024*1024):
    """
    参数说明
    --------
    K        : GNN 特征维度（128）
    TILE_H   : AMX tile 行数（16，硬件固定）
    N_THREADS: OpenMP 线程数
    L1_BYTES : 每核 L1 cache（48KB）
    L2_BYTES : 每核 L2 cache（2MB）
    """
    deg = get_deg_stats(indptr, nrow)
    avg_deg   = nnz / nrow
    max_deg   = deg.max()
    p50_deg   = np.percentile(deg, 50)
    p95_deg   = np.percentile(deg, 95)
    p99_deg   = np.percentile(deg, 99)

    # ── 基础内存参数 ───────────────────────────────────────
    BF16   = 2   # bytes per BF16
    FP32   = 4   # bytes per FP32

    H_row_bytes  = K * BF16          # 256B  每行H特征
    C_row_bytes  = K * FP32          # 512B  每行C输出（FP32累加）
    W_bytes      = K * K * BF16      # 32KB  权重矩阵W

    # KC-Fusion 的 K_block 数（V17c: K=128分2个pass，每pass读64B×2=128B）
    # 当前Fusion v3: 2 passes, gather4 reads 64B each = 128B per neighbor
    KC_PASSES   = K // 64            # = 2（每pass处理64个BF16 = 128B）
    # Feature-Stationary: K_BLOCKS个外层块，每块读32B per neighbor
    K_BLOCKS    = K // 32            # = 4

    print(f"\n{'='*66}")
    print(f"矩阵: {name}")
    print(f"  nrow={nrow:,}  nnz={nnz:,}  avg_deg={avg_deg:.1f}")
    print(f"  deg: p50={p50_deg:.0f}  p95={p95_deg:.0f}  p99={p99_deg:.0f}  max={max_deg:,}")
    print(f"{'='*66}")

    # ════════════════════════════════════════════════════════
    # 方案A：当前 Fusion v3（行中心，C反复spill）
    # ════════════════════════════════════════════════════════
    #
    # 内层循环每步：
    #   load  C_tile(8KB) + gather H_row(128B) + TDPBF16PS + store C_tile(8KB)
    # 循环 max_deg_in_batch 次（每批16节点，用最大度数作上界）
    #
    # C spill 建模：
    #   每批16节点，C tile被load/store avg_deg次
    #   total C spill = (nrow/16) batches × avg_deg × 16KB（load+store）
    #
    # H 读取建模（KC-Fusion）：
    #   gather4: 每个neighbor读 2×64B = 128B，4个TDPBF16PS共享
    #   等效每NNZ读 128B / 4 share = 实际还是128B，但从寄存器复用角度：
    #   gather4 → 1次内存读（128B），寄存器内拆给4次TDPBF16PS
    #   → H reads = nnz × 128B
    #
    # W 读取建模：
    #   每批16节点 × avg_deg步 → W_tile需要被load avg_deg次
    #   但W在L1时 cache hit，在高度数时被H踢出 → cache miss
    #   用一个简化模型：当 H_working_set > L1-W_bytes 时，W被踢出
    H_working_set_A = TILE_H * avg_deg * H_row_bytes   # 每批H的工作集

    # W cache命中率（简化线性模型）
    # 当 H_ws << L1-W 时，W完全驻留；当 H_ws >> L1 时，W每步都miss
    L1_for_H = L1_BYTES - W_bytes   # 16KB可用于H
    if H_working_set_A <= L1_for_H:
        W_hit_rate_A = 1.0
    elif H_working_set_A >= L1_BYTES:
        W_hit_rate_A = 0.05   # L1基本被H占满，W几乎全miss
    else:
        # 线性插值
        W_hit_rate_A = 1.0 - (H_working_set_A - L1_for_H) / (L1_BYTES - L1_for_H) * 0.95

    n_batches = nrow / TILE_H

    # C spill流量（load + store，每步8KB × avg_deg次/批）
    C_spill_A   = n_batches * avg_deg * 8192 * 2           # bytes
    # H读取流量（每NNZ读128B）
    H_read_A    = nnz * H_row_bytes                        # bytes（128B/NNZ）
    # W读取流量（cache miss部分，avg_deg次/批 × W大小 × miss率）
    W_read_A    = n_batches * avg_deg * W_bytes * (1 - W_hit_rate_A)  # miss流量
    # C最终写出（nrow × 512B，只算1次）
    C_final_A   = nrow * C_row_bytes

    total_A = C_spill_A + H_read_A + W_read_A + C_final_A

    print(f"\n  [方案A] 当前 Fusion v3（行中心，C反复spill）")
    print(f"  ┌──────────────────────────────────────────────────┐")
    print(f"  │ H working set/批: {H_working_set_A/1024:.1f}KB   "
          f"L1可用: {L1_for_H/1024:.0f}KB")
    print(f"  │ W cache命中率:    {W_hit_rate_A*100:.0f}%")
    print(f"  │ C spill 流量:     {C_spill_A/1e9:.2f} GB  "
          f"({C_spill_A/total_A*100:.0f}% of total)")
    print(f"  │ H 读取流量:       {H_read_A/1e9:.2f} GB  "
          f"({H_read_A/total_A*100:.0f}%)")
    print(f"  │ W miss 流量:      {W_read_A/1e9:.2f} GB  "
          f"({W_read_A/total_A*100:.0f}%)")
    print(f"  │ C 最终写出:       {C_final_A/1e9:.2f} GB  "
          f"({C_final_A/total_A*100:.0f}%)")
    print(f"  │ ─────────────────────────────────────────────── │")
    print(f"  │ 总流量（估计）:   {total_A/1e9:.2f} GB             │")
    print(f"  └──────────────────────────────────────────────────┘")

    # ════════════════════════════════════════════════════════
    # 方案B：Feature-Stationary（特征块外循环）
    # ════════════════════════════════════════════════════════
    #
    # 循环结构：
    #   for kb in 0..K_BLOCKS(=4):          ← 外层：特征块
    #     load W_slice(8KB) → L1 锁定
    #     for batch in node_batches:         ← 中层：节点批次
    #       C_partial(2KB) → tile寄存器      ← 永不spill（2KB < tile寄存器）
    #       for j in neighbors:             ← 内层：邻居
    #         gather H[j][kb*32:(kb+1)*32]   ← 读32B（原来128B）
    #         TDPBF16PS
    #       store C_partial（1次）
    #
    # 关键变化：
    #   C tile从8KB→2KB，装进tile寄存器（AMX有8×1KB=8KB，2KB完全够）
    #   → C 不再spill！C_spill_B = 0
    #   W_slice = 8KB → 完全在L1（L1=48KB，绰绰有余）
    #   → W miss率 ≈ 0
    #
    # KC-Fusion损失：
    #   原来: gather4读128B → 寄存器内拆4个32B → 4× TDPBF16PS
    #   现在: gather读32B → 1× TDPBF16PS
    #   H读取流量不变（还是nnz × 128B，只是分K_BLOCKS次读）
    #   但每次读的粒度从128B→32B，cache line利用率下降（64B cache line）
    #
    # Cache line 效率：
    #   128B读取：2个完整cache line，100%利用
    #    32B读取：仅用1个cache line的一半，但预取可以部分弥补
    #   → 保守估计：H流量等效增加 1.5×（cache line浪费 + prefetch开销）
    H_efficiency_loss = 1.5   # 保守估计：32B读取 vs 128B读取的cache line效率差

    H_working_set_B = TILE_H * avg_deg * (H_row_bytes / K_BLOCKS)  # 每批H工作集（只用1/4）

    C_spill_B  = 0.0                                        # 核心优势：C不再spill
    H_read_B   = nnz * H_row_bytes * H_efficiency_loss      # 效率损失后的等效流量
    W_read_B   = 0.0                                        # W_slice=8KB常驻L1
    C_final_B  = nrow * C_row_bytes * K_BLOCKS              # 每个K_block写1次，共4次
    # 注意：C_final_B要×K_BLOCKS因为每次只写1/4的C，需要partial accumulation
    # 更精确：C_partial每批写1次×4个K_block = nrow/16 × 4 × 2KB
    # 但partial result需要在K_blocks之间累加（写到L2/L3而非HBM）
    # 估算：前3个K_block写L2（假设L2命中），第4个K_block写HBM
    C_accumulate_B = nrow * C_row_bytes * (K_BLOCKS - 1)    # K_blocks间的累加读写
    # 假设这些累加在L2中完成（每核2MB L2，nrow×512B可能溢出）
    # 对大矩阵（nrow>4K），会溢出到L3（假设L3带宽是L1的1/4）
    nrow_fits_L2 = L2_BYTES // C_row_bytes                  # L2能容纳的行数
    L2_hit_C = min(1.0, nrow_fits_L2 / (nrow / N_THREADS))  # 每线程分到的行
    C_accumulate_effective = C_accumulate_B * (L2_hit_C * 0.25 + (1-L2_hit_C) * 1.0)

    total_B = C_spill_B + H_read_B + W_read_B + C_final_B + C_accumulate_effective

    print(f"\n  [方案B] Feature-Stationary（特征块外循环）")
    print(f"  ┌──────────────────────────────────────────────────┐")
    print(f"  │ H working set/批/块: {H_working_set_B/1024:.1f}KB  "
          f"W_slice=8KB 常驻L1")
    print(f"  │ C spill 流量:     0.00 GB  ← 核心优势（C不再spill）")
    print(f"  │ H 读取流量:       {H_read_B/1e9:.2f} GB  "
          f"(×{H_efficiency_loss:.1f} cache line效率损失)")
    print(f"  │ W miss 流量:      0.00 GB  ← W_slice常驻L1")
    print(f"  │ C 累加读写(L2/L3):{C_accumulate_effective/1e9:.2f} GB")
    print(f"  │ C 最终写出:       {C_final_B/1e9:.2f} GB")
    print(f"  │ ─────────────────────────────────────────────── │")
    print(f"  │ 总流量（估计）:   {total_B/1e9:.2f} GB             │")
    print(f"  └──────────────────────────────────────────────────┘")

    # ════════════════════════════════════════════════════════
    # 方案C：两步法最优（V17c SpMM + 优化Dense GeMM）
    # ════════════════════════════════════════════════════════
    #
    # SpMM（V17c）：
    #   H读取：nnz × 128B（FB路径主导）
    #   Z写出：nrow × 512B（FP32）
    #   A读取：nnz × 4B（列索引）← 已在indptr中
    #
    # GeMM（Z×W → H'）：
    #   Z读取：nrow × 512B
    #   W读取：≈ nrow/L1_rows × W_bytes（W可以驻留L1）
    #   H'写出：nrow × 256B（BF16输出）
    #
    Z_bytes = nrow * K * FP32                               # 中间矩阵Z

    H_read_C    = nnz * H_row_bytes                        # SpMM H读取
    Z_write_C   = Z_bytes                                  # SpMM Z写出
    Z_read_C    = Z_bytes                                  # GeMM Z读入
    W_read_C    = W_bytes * (nrow // (L1_BYTES // C_row_bytes) + 1)  # W复用
    W_read_C    = min(W_read_C, W_bytes * avg_deg / 16 * 0.1)       # 实际W复用好
    Hout_write_C = nrow * H_row_bytes                      # GeMM H'写出

    total_C = H_read_C + Z_write_C + Z_read_C + Hout_write_C
    # W复用较好（L1 GeMM），这里简化忽略W流量

    print(f"\n  [方案C] 两步法最优（V17c + Dense GeMM）")
    print(f"  ┌──────────────────────────────────────────────────┐")
    print(f"  │ H 读取（SpMM）:   {H_read_C/1e9:.2f} GB")
    print(f"  │ Z 写出→读入:      {Z_write_C/1e9:.2f} + {Z_read_C/1e9:.2f} = {(Z_write_C+Z_read_C)/1e9:.2f} GB ← Fusion节省的")
    print(f"  │ H' 写出（GeMM）:  {Hout_write_C/1e9:.2f} GB")
    print(f"  │ ─────────────────────────────────────────────── │")
    print(f"  │ 总流量（估计）:   {total_C/1e9:.2f} GB             │")
    print(f"  └──────────────────────────────────────────────────┘")

    # ════════════════════════════════════════════════════════
    # 对比分析 & 盈亏预测
    # ════════════════════════════════════════════════════════
    # 带宽换算（假设HBM节点）
    BW_HBM  = 1000e9   # 1 TB/s HBM2e
    BW_DDR5 =  300e9   # 300 GB/s DDR5

    print(f"\n  ── 对比分析 ─────────────────────────────────────────")

    ratio_B_vs_A = total_A / total_B
    ratio_B_vs_C = total_C / total_B

    print(f"  流量对比:")
    print(f"    方案A（当前Fusion） : {total_A/1e9:.2f} GB  (基准 1.00×)")
    print(f"    方案B（FS-Fusion）  : {total_B/1e9:.2f} GB  ({ratio_B_vs_A:.2f}× 少于A)")
    print(f"    方案C（两步法）     : {total_C/1e9:.2f} GB  ({total_C/total_A:.2f}× vs A)")

    print(f"\n  理论执行时间（HBM 1TB/s）:")
    t_A_hbm = total_A / BW_HBM * 1000
    t_B_hbm = total_B / BW_HBM * 1000
    t_C_hbm = total_C / BW_HBM * 1000
    print(f"    方案A: {t_A_hbm:.1f} ms")
    print(f"    方案B: {t_B_hbm:.1f} ms  → 理论加速 {t_A_hbm/t_B_hbm:.2f}× vs A，"
          f"{t_C_hbm/t_B_hbm:.2f}× vs 两步法")
    print(f"    方案C: {t_C_hbm:.1f} ms")

    print(f"\n  理论执行时间（DDR5 300GB/s）:")
    t_A_ddr = total_A / BW_DDR5 * 1000
    t_B_ddr = total_B / BW_DDR5 * 1000
    t_C_ddr = total_C / BW_DDR5 * 1000
    print(f"    方案A: {t_A_ddr:.1f} ms")
    print(f"    方案B: {t_B_ddr:.1f} ms  → 理论加速 {t_A_ddr/t_B_ddr:.2f}× vs A，"
          f"{t_C_ddr/t_B_ddr:.2f}× vs 两步法")
    print(f"    方案C: {t_C_ddr:.1f} ms")

    # ── 盈亏平衡分析 ──────────────────────────────────────
    # 方案B vs 方案C 的盈亏：total_B < total_C 时 Fusion有意义
    print(f"\n  ── 盈亏分析 ──────────────────────────────────────────")
    if total_B < total_C:
        margin = (total_C - total_B) / total_C * 100
        print(f"  ✅ FS-Fusion < 两步法  →  Fusion有意义！节省 {margin:.1f}%")
    else:
        margin = (total_B - total_C) / total_C * 100
        print(f"  ❌ FS-Fusion > 两步法  →  Fusion仍然更慢 {margin:.1f}%")

    # ── C spill vs Z节省 的核心对决 ───────────────────────
    Z_saving    = (Z_write_C + Z_read_C)          # 两步法需要的Z读写
    C_spill_cost = C_spill_A                       # 当前Fusion的C spill代价
    print(f"\n  ── 核心对决：C spill 开销 vs Z 节省 ─────────────────")
    print(f"  Z 读写（两步法）:    {Z_saving/1e9:.3f} GB  ← Fusion想节省的")
    print(f"  C spill（方案A）:    {C_spill_cost/1e9:.3f} GB  ← 当前Fusion引入的")
    ratio_spill_saving = C_spill_cost / Z_saving if Z_saving > 0 else 999
    if ratio_spill_saving > 1:
        print(f"  → C spill是Z节省的 {ratio_spill_saving:.1f}×！这就是Fusion更慢的根源")
    else:
        print(f"  → C spill < Z节省，Fusion理论上有正收益")

    print(f"  FS方案消除C spill后的净收益: "
          f"{(C_spill_cost - 0) / 1e9:.3f} GB 流量节省")

    return {
        'avg_deg': avg_deg,
        'total_A': total_A,
        'total_B': total_B,
        'total_C': total_C,
        'C_spill_A': C_spill_A,
        'Z_saving': Z_saving,
        'ratio_B_vs_A': ratio_B_vs_A,
        'B_beats_C': total_B < total_C,
        'W_hit_rate_A': W_hit_rate_A,
    }


# ──────────────────────────────────────────────────────────
# 盈亏平衡度数分析（解析公式）
# ──────────────────────────────────────────────────────────
def breakeven_analysis():
    """
    解析推导盈亏平衡度数

    方案B总流量 < 方案C总流量 时，Fusion有意义

    简化为：
      方案C总流量 ≈ nnz×H_row + 2×N×K×FP32 + N×H_row
               ≈ N×d×256B + 2×N×512B + N×256B
               = N × (256d + 1024 + 256)
               = N × (256d + 1280)

      方案B总流量 ≈ nnz×H_row×1.5 + N×C_row×K_BLOCKS（C累加）
               ≈ N×d×256B×1.5 + N×512B×4
               = N × (384d + 2048)

    盈亏平衡：
      384d + 2048 = 256d + 1280
      128d = -768
      d = -6  → 任何正度数下B都比C流量大？

    等等，这说明了什么？重新推导...
    C累加流量的假设需要修正：
    如果C累加能在L2内完成（每线程分到的C行能装进L2），
    则C_accumulate实际代价很小。

    更精确的模型需要考虑：
      - L2容量 vs 每线程C矩阵大小
      - HBM vs DDR5节点
    """
    print(f"\n\n{'='*66}")
    print("盈亏平衡度数分析（解析公式）")
    print(f"{'='*66}")

    K = 128
    BF16, FP32 = 2, 4
    H_row = K * BF16    # 256B
    C_row = K * FP32    # 512B

    print(f"\n关键流量项（以 N=节点数 为单位，d=avg_deg）：")
    print(f"")
    print(f"  方案C（两步法）：")
    print(f"    H读取:   N×d×{H_row}B = {H_row}×N×d")
    print(f"    Z读写:   2×N×{C_row}B = {2*C_row}×N    ← Fusion要消灭这个")
    print(f"    H'写出:  N×{H_row}B   = {H_row}×N")
    print(f"    合计:    N×({H_row}d + {2*C_row+H_row}) = N×({H_row}d + {2*C_row+H_row})")
    print(f"")
    print(f"  方案A（当前Fusion）：")
    print(f"    H读取:   N×d×{H_row}B = {H_row}×N×d")
    print(f"    C spill: (N/16)×d×{2*8192}B = {int(d_c_spill := 2*8192/16)}×N×d  ← 每批每步16KB×2")
    print(f"    合计:    N×({H_row+int(d_c_spill)}d)  → spill主导！")
    print(f"")
    print(f"  方案B（Feature-Stationary）：")
    print(f"    H读取:   N×d×{H_row}B×1.5 = {int(H_row*1.5)}×N×d  (cache line效率损失)")
    print(f"    C累加:   N×{C_row}B×3 = {3*C_row}×N  (K_BLOCKS-1次累加，假设L2)")
    print(f"    C写出:   N×{C_row}B = {C_row}×N")
    print(f"    合计:    N×({int(H_row*1.5)}d + {3*C_row+C_row})")

    print(f"\n盈亏平衡（方案B vs 方案C）：")
    print(f"  {int(H_row*1.5)}d + {3*C_row+C_row} = {H_row}d + {2*C_row+H_row}")
    lhs_coeff = int(H_row * 1.5)
    rhs_coeff = H_row
    lhs_const = 3 * C_row + C_row
    rhs_const = 2 * C_row + H_row
    # (lhs_coeff - rhs_coeff)×d = rhs_const - lhs_const
    # 128d = (2×512+256) - (4×512) = 1280 - 2048 = -768
    d_breakeven = (rhs_const - lhs_const) / (lhs_coeff - rhs_coeff)
    print(f"  ({lhs_coeff}-{rhs_coeff})×d = {rhs_const} - {lhs_const}")
    print(f"  {lhs_coeff-rhs_coeff}d = {rhs_const - lhs_const}")
    if d_breakeven < 0:
        print(f"  → d* = {d_breakeven:.1f}（负数！）")
        print(f"")
        print(f"  ⚠️  这说明：在上述保守假设下（C累加全在HBM），")
        print(f"     方案B在任意度数下都比方案C流量大。")
        print(f"")
        print(f"  但如果 C累加在L2内完成（乘以0.25延迟系数），则：")
        lhs_const_opt = int(C_row * 3 * 0.25) + C_row
        d_be_opt = (rhs_const - lhs_const_opt) / (lhs_coeff - rhs_coeff)
        print(f"  C累加有效流量 = {3*C_row}B×0.25 = {int(3*C_row*0.25)}B")
        print(f"  新盈亏点: d* = ({rhs_const} - {lhs_const_opt}) / {lhs_coeff-rhs_coeff} = {d_be_opt:.1f}")
        if d_be_opt > 0:
            print(f"  → avg_deg > {d_be_opt:.1f} 时，FS-Fusion比两步法更快！")
        else:
            print(f"  → d* 仍为负，FS-Fusion流量优势还不够")

    print(f"\n  关键结论：")
    print(f"  C累加能否在L2内完成，是决定FS-Fusion是否有意义的关键变量！")
    print(f"  需要检查：每线程负责的C行数 × 512B 是否 < L2(2MB)")
    print(f"")
    print(f"  以hollywood为例（nrow=1.14M，32线程）：")
    nrow_hw = 1_139_905
    c_per_thread = nrow_hw / 32 * 512
    print(f"    每线程C大小 = {nrow_hw//32} 行 × 512B = {c_per_thread/1024:.0f} KB")
    print(f"    L2 = 2048 KB")
    if c_per_thread < 2 * 1024 * 1024:
        print(f"    ✅ C能装入L2！C累加代价低，FS-Fusion有望有正收益")
    else:
        print(f"    ❌ C溢出L2，C累加走L3/HBM")

    print(f"")
    print(f"  以cit-Patents为例（nrow=3.77M，32线程）：")
    nrow_cp = 3_774_768
    c_per_thread_cp = nrow_cp / 32 * 512
    print(f"    每线程C大小 = {nrow_cp//32} 行 × 512B = {c_per_thread_cp/1024:.0f} KB")
    if c_per_thread_cp < 2 * 1024 * 1024:
        print(f"    ✅ C能装入L2！")
    else:
        print(f"    ❌ C溢出L2（{c_per_thread_cp/1024:.0f}KB > 2048KB）")


# ──────────────────────────────────────────────────────────
# 主程序
# ──────────────────────────────────────────────────────────
def main():
    parser = argparse.ArgumentParser(description='Feature-Stationary Fusion 理论模型')
    parser.add_argument('--data_dir', default='.',
                        help='数据根目录')
    parser.add_argument('--matrices', nargs='+',
                        default=['web-Google', 'amazon0601', 'cit-Patents',
                                 'as-Skitter', 'soc-Pokec', 'hollywood-2009',
                                 'indochina'])
    parser.add_argument('--K', type=int, default=128)
    args = parser.parse_args()

    print("Feature-Stationary Fusion 理论模型")
    print(f"K={args.K}  TILE_H=16  L1=48KB  L2=2MB  THR=32")
    print(f"建模方案: A=当前Fusion  B=FS-Fusion  C=两步法最优")

    all_results = {}

    for name in args.matrices:
        path = os.path.join(args.data_dir, name, f'{name}.csrbin')
        if not os.path.exists(path):
            print(f"\n[SKIP] {name}: 找不到 {path}")
            continue
        nrow, ncol, nnz, indptr, indices = read_csrbin(path)
        r = model_feature_stationary(name, nrow, ncol, nnz, indptr, indices, K=args.K)
        all_results[name] = r

    # ── 跨矩阵汇总 ────────────────────────────────────────
    print(f"\n\n{'='*78}")
    print("跨矩阵汇总")
    print(f"{'='*78}")
    print(f"{'矩阵':<22} {'avg_deg':>8} "
          f"{'A(GB)':>8} {'B(GB)':>8} {'C(GB)':>8} "
          f"{'B/A节省':>8} {'B<C?':>6} {'C_spill/Z':>10}")
    print('-'*78)
    for name, r in all_results.items():
        print(f"{name:<22} {r['avg_deg']:>8.1f} "
              f"{r['total_A']/1e9:>8.2f} "
              f"{r['total_B']/1e9:>8.2f} "
              f"{r['total_C']/1e9:>8.2f} "
              f"{r['ratio_B_vs_A']:>8.2f}× "
              f"{'✅' if r['B_beats_C'] else '❌':>6} "
              f"{r['C_spill_A']/r['Z_saving']:>10.1f}×")

    print(f"\n说明:")
    print(f"  A = 当前Fusion（C反复spill）")
    print(f"  B = FS-Fusion（C不spill，H×1.5效率损失）")
    print(f"  C = 两步法最优（V17c + Dense GeMM）")
    print(f"  B/A节省 = A流量/B流量（越大越好）")
    print(f"  B<C? = FS-Fusion是否比两步法流量少（✅=Fusion有意义）")
    print(f"  C_spill/Z = C spill开销是Z节省的几倍（>1=当前Fusion更慢的根因）")

    # ── 解析公式推导 ──────────────────────────────────────
    breakeven_analysis()


if __name__ == '__main__':
    main()
