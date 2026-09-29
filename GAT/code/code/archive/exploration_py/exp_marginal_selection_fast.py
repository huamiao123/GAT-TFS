#!/usr/bin/env python3
"""
Marginal NNZ/U 行选择策略 — numpy bitmap 加速版

核心优化：
  1. CSC 构建用 numpy 向量化（不用 Python for 循环）
  2. 列集合用 uint8 bitmap 表示（ncol 位）
  3. "新增列数" = popcount(row_bitmap AND NOT union_bitmap)
  4. 度数查找用 numpy array 直接索引
"""

import struct
import numpy as np
import os
import time

def read_csrbin(path):
    with open(path, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('III', f.read(12))
        nrow, ncol, nnz = struct.unpack('QQQ', f.read(24))
        indptr = np.frombuffer(f.read((nrow+1)*4), dtype=np.uint32)
        indices = np.frombuffer(f.read(nnz*4), dtype=np.uint32)
    return nrow, ncol, nnz, indptr, indices

def build_csc_fast(nrow, ncol, nnz, indptr, indices):
    """numpy 向量化 CSC 构建"""
    col_deg = np.bincount(indices, minlength=ncol).astype(np.int64)
    col_ptr = np.zeros(ncol + 1, dtype=np.int64)
    np.cumsum(col_deg, out=col_ptr[1:])

    # 向量化填充 CSC
    col_pos = col_ptr[:-1].copy()
    col_indices = np.empty(nnz, dtype=np.int64)

    # 按行展开所有 (row, col) 对
    row_ids = np.repeat(np.arange(nrow, dtype=np.int64),
                        np.diff(indptr).astype(np.int64))
    # argsort by column → 按列排序
    order = np.argsort(indices, kind='mergesort')
    col_indices = row_ids[order]

    return col_deg, col_ptr, col_indices

def make_row_bitmap(r, indptr, indices, ncol_bytes):
    """为行 r 创建 bitmap（uint8 数组，每 bit 代表一列）"""
    bm = np.zeros(ncol_bytes, dtype=np.uint8)
    cols = indices[indptr[r]:indptr[r+1]]
    # 设置对应 bit
    byte_idx = cols >> 3        # 列号 / 8 = 第几个 byte
    bit_idx = cols & 7          # 列号 % 8 = byte 内第几位
    # np.add.at: 原地在指定索引处加值（处理重复索引）
    np.add.at(bm, byte_idx, (1 << bit_idx).astype(np.uint8))
    return bm

def popcount_bitmap(bm):
    """统计 bitmap 中 1 的个数"""
    # 用查找表加速 popcount
    lut = np.array([bin(i).count('1') for i in range(256)], dtype=np.int64)
    return np.sum(lut[bm])

def new_cols_count(row_bm, union_bm):
    """计算行 bitmap 中有多少列不在 union 中"""
    # row AND (NOT union) = 新增的列
    new_bits = row_bm & ~union_bm
    lut = np.array([bin(i).count('1') for i in range(256)], dtype=np.int64)
    return np.sum(lut[new_bits])

def run_strategy(name, nrow, ncol, nnz, indptr, indices,
                 col_deg, col_ptr, col_indices, row_degrees,
                 mode='low_degree', max_panels=3000):
    """
    运行一种策略
    mode: 'low_degree' / 'high_degree' / 'marginal'
    """
    TILE_R = 16
    col_order = np.argsort(col_deg)[::-1]
    assigned = np.zeros(nrow, dtype=bool)
    ncol_bytes = (ncol + 7) // 8  # bitmap 的字节数

    # popcount 查找表（0-255 每个数有几个 1）
    pop_lut = np.array([bin(i).count('1') for i in range(256)], dtype=np.int64)

    total_U = 0
    total_nnz_amx = 0
    total_panels = 0
    panel_U_list = []
    panel_nnz_list = []
    panel_fill_list = []

    t0 = time.time()

    for ci in range(min(ncol, len(col_order))):
        c = col_order[ci]
        if col_deg[c] == 0:
            break
        rows_with_c = col_indices[col_ptr[c]:col_ptr[c+1]]
        avail = rows_with_c[~assigned[rows_with_c]]
        if len(avail) < 2:
            continue

        if mode == 'low_degree':
            if len(avail) > TILE_R:
                degs = row_degrees[avail]
                k = min(TILE_R, len(avail)) - 1
                part_idx = np.argpartition(degs, k)[:TILE_R]
                selected = avail[part_idx]
            else:
                selected = avail

        elif mode == 'high_degree':
            if len(avail) > TILE_R:
                degs = row_degrees[avail]
                k = min(TILE_R, len(avail)) - 1
                part_idx = np.argpartition(-degs, k)[:TILE_R]
                selected = avail[part_idx]
            else:
                selected = avail

        elif mode == 'marginal':
            if len(avail) <= TILE_R:
                selected = avail
            else:
                # bitmap 贪心
                union_bm = np.zeros(ncol_bytes, dtype=np.uint8)
                # 初始 union 包含 anchor column
                union_bm[c >> 3] |= np.uint8(1 << (c & 7))

                # 预计算候选行的 bitmap 和度数
                # 如果候选太多，采样加速
                if len(avail) > 300:
                    # 采样 200 + top-50 高度行
                    rng = np.random.RandomState(ci)
                    sample_idx = rng.choice(len(avail),
                                            min(200, len(avail)),
                                            replace=False)
                    degs = row_degrees[avail]
                    if len(avail) > 50:
                        top50 = np.argpartition(-degs, 50)[:50]
                        sample_idx = np.unique(
                            np.concatenate([sample_idx, top50]))
                    cand_rows = avail[sample_idx]
                else:
                    cand_rows = avail.copy()

                # 预计算所有候选行的 bitmap
                cand_bitmaps = []
                cand_degs = []
                for r in cand_rows:
                    bm = np.zeros(ncol_bytes, dtype=np.uint8)
                    cols = indices[indptr[r]:indptr[r+1]]
                    byte_idx = cols >> 3
                    bit_idx = (cols & 7).astype(np.uint8)
                    np.bitwise_or.at(bm, byte_idx, np.uint8(1) << bit_idx)
                    cand_bitmaps.append(bm)
                    cand_degs.append(len(cols))
                cand_degs = np.array(cand_degs)

                selected_list = []
                active = np.ones(len(cand_rows), dtype=bool)

                for _ in range(min(TILE_R, len(cand_rows))):
                    best_idx = -1
                    best_value = -1.0

                    # 向量化计算所有活跃候选的 new_cols
                    active_indices = np.where(active)[0]
                    if len(active_indices) == 0:
                        break

                    for ai in active_indices:
                        new_bits = cand_bitmaps[ai] & ~union_bm
                        nc = int(np.sum(pop_lut[new_bits]))
                        deg = cand_degs[ai]
                        if nc == 0:
                            value = deg * 1000.0
                        else:
                            value = deg / nc
                        if value > best_value:
                            best_value = value
                            best_idx = ai

                    if best_idx < 0:
                        break

                    selected_list.append(cand_rows[best_idx])
                    # 更新 union
                    union_bm |= cand_bitmaps[best_idx]
                    active[best_idx] = False

                selected = np.array(selected_list, dtype=np.int64)

        # 计算 panel 统计
        panel_cols_bm = np.zeros(ncol_bytes, dtype=np.uint8)
        panel_nnz = 0
        for r in selected:
            cols = indices[indptr[r]:indptr[r+1]]
            byte_idx = cols >> 3
            bit_idx = (cols & 7).astype(np.uint8)
            np.bitwise_or.at(panel_cols_bm, byte_idx,
                             np.uint8(1) << bit_idx)
            panel_nnz += len(cols)

        U = int(np.sum(pop_lut[panel_cols_bm]))
        n_tiles = (U + 31) // 32
        fill = panel_nnz / (len(selected) * 32 * n_tiles) if n_tiles > 0 else 0

        total_U += U
        total_nnz_amx += panel_nnz
        total_panels += 1
        panel_U_list.append(U)
        panel_nnz_list.append(panel_nnz)
        panel_fill_list.append(fill)

        for r in selected:
            assigned[r] = True

        if total_panels >= max_panels:
            break

        # 进度提示（每 500 个 panel）
        if total_panels % 500 == 0:
            elapsed = time.time() - t0
            print(f"    [{name}] {total_panels} panels, {elapsed:.1f}s",
                  flush=True)

    elapsed = time.time() - t0

    fb_nnz = 0
    fb_rows = 0
    for r in range(nrow):
        if not assigned[r]:
            fb_nnz += int(indptr[r+1] - indptr[r])
            fb_rows += 1

    amx_coverage = total_nnz_amx / nnz * 100
    fb_pct = fb_nnz / nnz * 100
    global_reuse = total_nnz_amx / total_U if total_U > 0 else 0
    avg_fill = np.mean(panel_fill_list) if panel_fill_list else 0
    avg_U = np.mean(panel_U_list) if panel_U_list else 0

    return {
        'name': name,
        'panels': total_panels,
        'amx_nnz': total_nnz_amx,
        'amx_coverage': amx_coverage,
        'fb_nnz': fb_nnz,
        'fb_pct': fb_pct,
        'total_U': total_U,
        'avg_U': avg_U,
        'global_reuse': global_reuse,
        'avg_fill': avg_fill,
        'elapsed': elapsed,
        'panel_U': np.array(panel_U_list),
    }

# ============================================================
# 主程序
# ============================================================
if __name__ == '__main__':
    DATA_DIR = os.path.expanduser('~/data/SpMM_project/data')

    matrices = [
        'web-Google', 'amazon0601', 'soc-Pokec',
        'as-Skitter', 'cit-Patents', 'indochina-2004', 'hollywood-2009'
    ]

    print("=" * 85)
    print("Row Selection: Low-Degree vs High-Degree vs Marginal NNZ/U (bitmap accelerated)")
    print("=" * 85)

    all_results = []

    for mname in matrices:
        csrbin = os.path.join(DATA_DIR, mname, f'{mname}.csrbin')
        if not os.path.exists(csrbin):
            continue

        print(f"\n{'#'*65}")
        print(f"# {mname}")
        print(f"{'#'*65}")

        nrow, ncol, nnz, indptr, indices = read_csrbin(csrbin)
        avg_deg = nnz / nrow
        row_degrees = np.diff(indptr).astype(np.int64)
        print(f"M={nrow:,}  N={ncol:,}  NNZ={nnz:,}  avg_deg={avg_deg:.1f}")

        print("Building CSC (vectorized)...")
        t_csc = time.time()
        col_deg, col_ptr, col_indices = build_csc_fast(
            nrow, ncol, nnz, indptr, indices)
        print(f"CSC built in {time.time()-t_csc:.1f}s")

        strategies = [
            ("Config-A (low deg)", 'low_degree'),
            ("Config-B (high deg)", 'high_degree'),
            ("Marginal NNZ/U", 'marginal'),
        ]

        results = []
        for sname, smode in strategies:
            print(f"  Running {sname}...")
            r = run_strategy(sname, nrow, ncol, nnz, indptr, indices,
                           col_deg, col_ptr, col_indices, row_degrees,
                           mode=smode, max_panels=3000)
            print(f"  → {r['panels']} panels in {r['elapsed']:.1f}s, "
                  f"coverage={r['amx_coverage']:.1f}%")
            results.append(r)

        # 打印对比表
        print(f"\n  {'Strategy':<22} {'Panels':>7} {'AMX%':>7} {'FB%':>7} "
              f"{'ΣU':>10} {'avgU':>6} {'Reuse':>6} {'Fill%':>6} {'Time':>6}")
        print(f"  {'-'*80}")
        for r in results:
            print(f"  {r['name']:<22} {r['panels']:>7} "
                  f"{r['amx_coverage']:>6.1f}% {r['fb_pct']:>6.1f}% "
                  f"{r['total_U']:>10,} {r['avg_U']:>6.0f} "
                  f"{r['global_reuse']:>5.2f}x {r['avg_fill']*100:>5.1f}% "
                  f"{r['elapsed']:>5.1f}s")

        # Marginal vs baselines
        a, b, m = results[0], results[1], results[2]
        print(f"\n  Marginal vs Config-A: "
              f"coverage {a['amx_coverage']:.1f}%→{m['amx_coverage']:.1f}% "
              f"({m['amx_coverage']-a['amx_coverage']:+.1f}%), "
              f"ΣU {a['total_U']:,}→{m['total_U']:,} "
              f"({(m['total_U']/a['total_U']-1)*100:+.1f}%), "
              f"reuse {a['global_reuse']:.2f}x→{m['global_reuse']:.2f}x")

        print(f"  Marginal vs Config-B: "
              f"coverage {b['amx_coverage']:.1f}%→{m['amx_coverage']:.1f}% "
              f"({m['amx_coverage']-b['amx_coverage']:+.1f}%), "
              f"ΣU {b['total_U']:,}→{m['total_U']:,} "
              f"({(m['total_U']/b['total_U']-1)*100:+.1f}%), "
              f"reuse {b['global_reuse']:.2f}x→{m['global_reuse']:.2f}x")

        all_results.append((mname, results))

    # 全局汇总
    print(f"\n{'='*85}")
    print("GLOBAL SUMMARY: AMX Coverage")
    print(f"{'='*85}")
    print(f"{'Matrix':<16} {'Config-A':>10} {'Config-B':>10} {'Marginal':>10} "
          f"{'M vs best':>10}")
    print("-" * 58)
    for mname, results in all_results:
        a, b, m = results[0], results[1], results[2]
        best_ab = max(a['amx_coverage'], b['amx_coverage'])
        delta = m['amx_coverage'] - best_ab
        print(f"{mname:<16} {a['amx_coverage']:>9.1f}% {b['amx_coverage']:>9.1f}% "
              f"{m['amx_coverage']:>9.1f}% {delta:>+9.1f}%")

    print(f"\n{'Matrix':<16} {'Config-A':>10} {'Config-B':>10} {'Marginal':>10} "
          f"{'M vs best':>10}")
    print(f"{'':16} {'reuse':>10} {'reuse':>10} {'reuse':>10}")
    print("-" * 58)
    for mname, results in all_results:
        a, b, m = results[0], results[1], results[2]
        print(f"{mname:<16} {a['global_reuse']:>9.2f}x {b['global_reuse']:>9.2f}x "
              f"{m['global_reuse']:>9.2f}x")
