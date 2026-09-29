#!/usr/bin/env python3
"""
Column-Strip Flash 理论模拟脚本
==============================
核心假设：把 H 矩阵按"列范围"切成 strip，每次处理一个 strip 时
  - H_strip 整体装进 L1（8KB per 32行）
  - W 驻留 L1（32KB）
  - 扫描 A 的列索引找到落在该 strip 的所有 (i,j)
  - 理论上每行 H[j] 只读 1 次 → B_amp = 1×

验证三个问题：
  Q1: strip NNZ 分布是否均匀（负载均衡）
  Q2: C 输出写入稀疏度（每 strip 写多少个 C[i]？散写代价）
  Q3: 最优 strip 大小（32/64/128 行）及 L1 fitting 分析

用法：
  python3 exp_colstrip_sim.py \
    --data_dir /home/huangjianqiang_group/hdacp1/data/SpMM_project/data \
    --matrices web-Google amazon0601 cit-Patents as-Skitter soc-Pokec hollywood-2009 indochina

作者：自动生成
日期：2026-03-09
"""

import struct
import numpy as np
import os
import sys
import argparse

# ──────────────────────────────────────────────
# CSRbin 读取（与项目其他脚本保持一致）
# ──────────────────────────────────────────────
def read_csrbin(path):
    with open(path, 'rb') as f:
        # 3×uint32 header (ptype, dtype, vtype)
        f.read(12)
        # 3×uint64: nrow, ncol, nnz
        nrow, ncol, nnz = struct.unpack('<QQQ', f.read(24))
        # indptr: (nrow+1) × uint32
        indptr  = np.frombuffer(f.read(4*(nrow+1)), dtype=np.uint32).copy()
        # indices: nnz × uint32
        indices = np.frombuffer(f.read(4*nnz),      dtype=np.uint32).copy()
    return int(nrow), int(ncol), int(nnz), indptr, indices


# ──────────────────────────────────────────────
# 核心模拟函数
# ──────────────────────────────────────────────
def simulate_colstrip(name, nrow, ncol, nnz, indptr, indices,
                      strip_sizes=(32, 64, 128),
                      K=128,
                      L1_bytes=48*1024,
                      sample_strips=None):
    """
    对给定矩阵模拟 Column-Strip 访问模式。

    参数
    ----
    strip_sizes : 候选 strip 大小（H 矩阵的行数）
    K           : 特征维度（GNN K=128）
    L1_bytes    : L1 cache 容量（48KB）
    sample_strips: None=全量；否则只分析前 N 个 strip（大矩阵加速）
    """
    print(f"\n{'='*62}")
    print(f"矩阵: {name}")
    print(f"  nrow={nrow:,}  ncol={ncol:,}  nnz={nnz:,}")
    avg_deg = nnz / nrow
    print(f"  avg_deg={avg_deg:.1f}")
    print(f"{'='*62}")

    # W 矩阵大小：K×K BF16 = K*K*2 bytes
    W_bytes = K * K * 2          # 32KB for K=128
    # 每行 H/C 大小：K BF16 = K*2 bytes
    H_row_bytes = K * 2          # 256B for K=128

    # ── CSR → CSC 转换（numpy, 无 Python 循环）
    # 利用 argsort(indices) 得到按列排序的 NNZ 编号
    col_order   = np.argsort(indices, kind='stable')
    sorted_cols = indices[col_order]
    # 对应的行号：从 indptr 反推
    row_ids_all = np.repeat(np.arange(nrow, dtype=np.uint32),
                            np.diff(indptr.astype(np.int64)))
    sorted_rows = row_ids_all[col_order]   # 列排序后的行号
    # CSC indptr（每列开始位置）
    csc_indptr = np.zeros(ncol + 1, dtype=np.int64)
    np.add.at(csc_indptr[1:], sorted_cols, 1)
    np.cumsum(csc_indptr, out=csc_indptr)

    results = {}

    for S in strip_sizes:
        H_strip_bytes = S * H_row_bytes          # strip 的字节数
        W_and_strip   = W_bytes + H_strip_bytes  # L1 需容纳两者
        fits_L1       = (W_and_strip <= L1_bytes)
        n_strips      = int(np.ceil(ncol / S))   # strip 总数

        # ── Q1: 每个 strip 的 NNZ 分布 ──────────────────────
        # strip_id 对每个 NNZ 元素是其列号 // S
        strip_ids   = sorted_cols // S           # shape: (nnz,)
        strip_nnz   = np.bincount(strip_ids, minlength=n_strips).astype(np.int64)

        nnz_mean    = strip_nnz.mean()
        nnz_std     = strip_nnz.std()
        nnz_cv      = nnz_std / nnz_mean if nnz_mean > 0 else 0   # 变异系数
        nnz_max     = strip_nnz.max()
        nnz_p95     = np.percentile(strip_nnz, 95)
        # 空 strip 比例
        empty_frac  = (strip_nnz == 0).mean()

        # ── Q2: 每个 strip 的 C 写入稀疏度 ──────────────────
        # 每个 strip 中有多少个 不同的输出行 i 被写到
        # 利用 CSC：对 strip s，读 sorted_rows[csc_indptr[s*S] : csc_indptr[min((s+1)*S, ncol)]]
        # 全量统计用 numpy groupby 技巧（避免 Python 循环）
        #
        # 等价于：strip_unique_rows[s] = len(unique(sorted_rows[strip_s_range]))
        # 用向量化近似：对每个 NNZ，标记 (strip_id, row_id)，去重计数
        # 完全向量化方法：把 (strip_id, row_id) 编码为一个 uint64，整体 unique
        if nnz <= 50_000_000:  # 内存允许时全量计算
            keys = strip_ids.astype(np.int64) * nrow + sorted_rows.astype(np.int64)
            unique_keys = np.unique(keys)
            unique_strips_for_keys = (unique_keys // nrow).astype(np.int32)
            c_writes_per_strip = np.bincount(unique_strips_for_keys,
                                             minlength=n_strips).astype(np.int64)
            c_density_mean = c_writes_per_strip.mean() / nrow   # 平均每 strip 写多少比例的 C
            c_writes_mean  = c_writes_per_strip.mean()
            c_writes_p95   = np.percentile(c_writes_per_strip, 95)
        else:
            # 大矩阵：采样前 min(sample_strips,n_strips) 个 strip
            ns = min(500, n_strips)
            mask = sorted_cols < ns * S
            sk = strip_ids[mask].astype(np.int64) * nrow + sorted_rows[mask].astype(np.int64)
            usk = np.unique(sk)
            ustrip = (usk // nrow).astype(np.int32)
            cw = np.bincount(ustrip, minlength=ns).astype(np.int64)
            c_density_mean = cw.mean() / nrow
            c_writes_mean  = cw.mean()
            c_writes_p95   = np.percentile(cw, 95)

        # ── Q3: 理论 B 读取节省 ─────────────────────────────
        # 当前 V17c：B_amp ≈ avg_deg（每行 H 平均被读 avg_deg 次）
        # Column-Strip：每行只读 1 次
        # 节省倍数 = avg_deg（上界，实际 panel 有复用，以流量分析为准）
        b_reads_v17c   = nnz * H_row_bytes          # 每条边读一次 H 行
        b_reads_colstrip = ncol * H_row_bytes        # 每行 H 只读 1 次
        amp_reduction  = b_reads_v17c / b_reads_colstrip   # ≈ avg_deg

        # ── 汇报 ────────────────────────────────────────────
        print(f"\n  [Strip Size = {S:3d} 行 = {H_strip_bytes//1024:.0f}KB]"
              f"  W+strip = {W_and_strip//1024:.0f}KB  "
              f"{'✅ 装入L1' if fits_L1 else '❌ 超出L1'}")
        print(f"  ── Q1: strip NNZ 分布 ──────────────────────────────")
        print(f"     strip 总数:    {n_strips:,}")
        print(f"     NNZ mean/std:  {nnz_mean:.0f} / {nnz_std:.0f}  (CV={nnz_cv:.2f})")
        print(f"     NNZ max:       {nnz_max:,}   P95={nnz_p95:.0f}")
        print(f"     空strip比例:   {empty_frac*100:.1f}%")
        imbalance = nnz_max / nnz_mean if nnz_mean > 0 else 0
        print(f"     负载不均衡度:  {imbalance:.2f}×  "
              f"{'⚠️ 需关注' if imbalance > 3 else '✅ 均匀'}")

        print(f"  ── Q2: C 输出写入稀疏度 ────────────────────────────")
        print(f"     每strip写C行数 mean: {c_writes_mean:.0f}  P95={c_writes_p95:.0f}")
        print(f"     写入密度(占nrow%):   {c_density_mean*100:.1f}%  "
              f"{'✅ 连续' if c_density_mean > 0.5 else '⚠️ 稀疏散写'}")

        print(f"  ── Q3: B 读取流量分析 ──────────────────────────────")
        print(f"     V17c B读取量:   {b_reads_v17c/1e9:.2f} GB")
        print(f"     ColStrip B读取: {b_reads_colstrip/1e9:.2f} GB")
        print(f"     理论节省倍数:   {amp_reduction:.1f}×  (≈ avg_deg)")

        results[S] = {
            'fits_L1': fits_L1,
            'n_strips': n_strips,
            'nnz_cv': nnz_cv,
            'imbalance': imbalance,
            'empty_frac': empty_frac,
            'c_density_mean': c_density_mean,
            'b_amp_reduction': amp_reduction,
        }

    # ── 推荐 strip 大小 ────────────────────────────────────
    print(f"\n  ── 综合推荐 ────────────────────────────────────────")
    for S, r in results.items():
        score = 0
        note  = []
        if r['fits_L1']:
            score += 3
            note.append("L1✅")
        if r['imbalance'] < 3:
            score += 2
            note.append("均衡✅")
        if r['c_density_mean'] > 0.3:
            score += 1
            note.append("C密✅")
        print(f"     strip={S:3d}: score={score}  {' '.join(note)}")

    return results


# ──────────────────────────────────────────────
# 主程序
# ──────────────────────────────────────────────
def main():
    parser = argparse.ArgumentParser(description='Column-Strip Flash 模拟')
    parser.add_argument('--data_dir', default='.',
                        help='数据根目录（每个矩阵有子目录 name/name.csrbin）')
    parser.add_argument('--matrices', nargs='+',
                        default=['web-Google', 'amazon0601', 'cit-Patents',
                                 'as-Skitter', 'soc-Pokec', 'hollywood-2009',
                                 'indochina'],
                        help='矩阵名列表')
    parser.add_argument('--strip_sizes', nargs='+', type=int, default=[32, 64, 128])
    parser.add_argument('--K', type=int, default=128)
    args = parser.parse_args()

    print("Column-Strip Flash 模拟")
    print(f"K={args.K}  L1=48KB  W={args.K*args.K*2//1024}KB")
    print(f"strip候选: {args.strip_sizes} 行")
    print(f"每行H = {args.K*2}B  每strip(32行) = {32*args.K*2}B = {32*args.K*2//1024}KB\n")

    all_results = {}

    for name in args.matrices:
        # 两种可能路径
        path1 = os.path.join(args.data_dir, name, f'{name}.csrbin')
        path2 = os.path.join(args.data_dir, name, f'{name}.csrbin')
        path  = path1 if os.path.exists(path1) else path2

        if not os.path.exists(path):
            print(f"[SKIP] {name}: 文件不存在 ({path})")
            continue

        nrow, ncol, nnz, indptr, indices = read_csrbin(path)
        r = simulate_colstrip(name, nrow, ncol, nnz, indptr, indices,
                              strip_sizes=args.strip_sizes,
                              K=args.K)
        all_results[name] = r

    # ── 跨矩阵汇总表 ───────────────────────────────────────
    print(f"\n\n{'='*72}")
    print("跨矩阵汇总（strip=32行）")
    print(f"{'='*72}")
    print(f"{'矩阵':<22} {'avg_deg':>8} {'CV':>6} {'不均':>6} {'空%':>6} {'C密%':>7} {'B节省':>7} {'L1':>5}")
    print('-'*72)

    # 重新读 avg_deg
    for name in args.matrices:
        path = os.path.join(args.data_dir, name, f'{name}.csrbin')
        if not os.path.exists(path) or name not in all_results:
            continue
        nrow, ncol, nnz, _, _ = read_csrbin(path)
        avg_deg = nnz / nrow
        r = all_results[name].get(32, {})
        if not r:
            continue
        print(f"{name:<22} {avg_deg:>8.1f} "
              f"{r['nnz_cv']:>6.2f} "
              f"{r['imbalance']:>6.1f}× "
              f"{r['empty_frac']*100:>6.1f}% "
              f"{r['c_density_mean']*100:>7.1f}% "
              f"{r['b_amp_reduction']:>6.1f}× "
              f"{'✅' if r['fits_L1'] else '❌':>5}")

    print(f"\n说明:")
    print(f"  CV       = NNZ变异系数（越小越均衡）")
    print(f"  不均     = max/mean（越接近1越好）")
    print(f"  空%      = 空strip占比（理想<10%）")
    print(f"  C密%     = 每strip平均写入C行数/nrow（越高越连续）")
    print(f"  B节省    = 理论B读取流量减少倍数（≈avg_deg）")
    print(f"  L1       = W(32KB)+strip 是否装入L1(48KB)")


if __name__ == '__main__':
    main()
