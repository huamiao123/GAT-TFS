#!/usr/bin/env python3
"""
分析 Pass1(16-row) 和 Pass2(8-row) 之后的 leftover 行分布
关键问题：被 16-row 和 8-row 拒绝的行，它们的 anchor 列还剩几行 avail？

如果 avail 9-15 很多 → 可变 tile height 直接救回
如果 avail 2-7 很多 → 小 panel 能救但效率低
如果 avail 1 为主 → 真正的孤立行，只能走 FB
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
    col_deg = np.bincount(indices, minlength=ncol).astype(np.int64)
    col_ptr = np.zeros(ncol + 1, dtype=np.int64)
    np.cumsum(col_deg, out=col_ptr[1:])
    row_ids = np.repeat(np.arange(nrow, dtype=np.int64),
                        np.diff(indptr).astype(np.int64))
    order = np.argsort(indices, kind='mergesort')
    col_indices = row_ids[order]
    return col_deg, col_ptr, col_indices

def simulate_multipass(nrow, ncol, nnz, indptr, indices,
                       col_deg, col_ptr, col_indices, row_degrees):
    """
    模拟三种方案：
    A) 当前 V17c: Pass1(16) + Pass2(8) + FB
    B) 可变高度: Pass1(16) + Pass2(2-15, 用 avail 行数) + FB
    C) 激进版:   Pass1(16) + Pass2(2-15) + Pass3(更松) + FB
    """
    col_order = np.argsort(col_deg)[::-1]

    def run_pass(assigned, tile_r_min, tile_r_max=16):
        """跑一轮列锚点分组，tile 行数范围 [tile_r_min, tile_r_max]"""
        panels = 0
        total_nnz = 0
        total_U = 0
        leftover_avail = []  # 记录每个被跳过的列的 avail 数

        for ci in range(min(ncol, len(col_order))):
            c = col_order[ci]
            if col_deg[c] == 0:
                break
            rows_with_c = col_indices[col_ptr[c]:col_ptr[c+1]]
            avail = rows_with_c[~assigned[rows_with_c]]

            if len(avail) < tile_r_min:
                if len(avail) >= 2:
                    leftover_avail.append(len(avail))
                continue

            # 选 min(avail, tile_r_max) 行
            n_select = min(len(avail), tile_r_max)
            if n_select < len(avail):
                degs = row_degrees[avail]
                k = n_select - 1
                part_idx = np.argpartition(degs, k)[:n_select]
                selected = avail[part_idx]
            else:
                selected = avail

            # 统计
            panel_cols = set()
            panel_nnz = 0
            for r in selected:
                cols_r = indices[indptr[r]:indptr[r+1]]
                panel_cols.update(cols_r.tolist())
                panel_nnz += len(cols_r)

            total_nnz += panel_nnz
            total_U += len(panel_cols)
            panels += 1
            assigned[selected] = True

        return panels, total_nnz, total_U, leftover_avail

    # ============================================================
    # 方案 A: 当前 V17c (Pass1=16, Pass2=8)
    # ============================================================
    assigned_A = np.zeros(nrow, dtype=bool)
    p1_panels, p1_nnz, p1_U, p1_left = run_pass(assigned_A, 16, 16)
    p2_panels, p2_nnz, p2_U, p2_left = run_pass(assigned_A, 8, 8)

    fb_A_rows = int(np.sum(~assigned_A))
    fb_A_nnz = int(np.sum((indptr[1:] - indptr[:-1])[~assigned_A]))

    # ============================================================
    # 方案 B: 可变高度 (Pass1=16, Pass2=2-15)
    # ============================================================
    assigned_B = np.zeros(nrow, dtype=bool)
    b1_panels, b1_nnz, b1_U, _ = run_pass(assigned_B, 16, 16)
    # Pass2: 最低 2 行就组 panel，tile height = avail
    b2_panels, b2_nnz, b2_U, b2_left = run_pass(assigned_B, 2, 15)

    fb_B_rows = int(np.sum(~assigned_B))
    fb_B_nnz = int(np.sum((indptr[1:] - indptr[:-1])[~assigned_B]))

    # ============================================================
    # 方案 B2: 保守可变高度 (Pass1=16, Pass2=6-15)
    # ============================================================
    assigned_B2 = np.zeros(nrow, dtype=bool)
    b2_1_panels, b2_1_nnz, b2_1_U, _ = run_pass(assigned_B2, 16, 16)
    b2_2_panels, b2_2_nnz, b2_2_U, b2_2_left = run_pass(assigned_B2, 6, 15)

    fb_B2_rows = int(np.sum(~assigned_B2))
    fb_B2_nnz = int(np.sum((indptr[1:] - indptr[:-1])[~assigned_B2]))

    return {
        # 方案 A
        'A_p1': (p1_panels, p1_nnz, p1_U),
        'A_p2': (p2_panels, p2_nnz, p2_U),
        'A_fb_rows': fb_A_rows, 'A_fb_nnz': fb_A_nnz,
        'A_p2_leftover': p2_left,
        # 方案 B
        'B_p1': (b1_panels, b1_nnz, b1_U),
        'B_p2': (b2_panels, b2_nnz, b2_U),
        'B_fb_rows': fb_B_rows, 'B_fb_nnz': fb_B_nnz,
        # 方案 B2
        'B2_p1': (b2_1_panels, b2_1_nnz, b2_1_U),
        'B2_p2': (b2_2_panels, b2_2_nnz, b2_2_U),
        'B2_fb_rows': fb_B2_rows, 'B2_fb_nnz': fb_B2_nnz,
        # Pass1 leftover 分布
        'p1_leftover': np.array(p1_left),
    }

if __name__ == '__main__':
    DATA_DIR = os.path.expanduser('~/data/SpMM_project/data')
    matrices = [
        'web-Google', 'amazon0601', 'soc-Pokec',
        'as-Skitter', 'cit-Patents', 'indochina-2004', 'hollywood-2009'
    ]

    print("=" * 80)
    print("Leftover Distribution & Variable Tile Height Analysis")
    print("=" * 80)

    for mname in matrices:
        csrbin = os.path.join(DATA_DIR, mname, f'{mname}.csrbin')
        if not os.path.exists(csrbin):
            continue

        print(f"\n{'#'*60}")
        print(f"# {mname}")
        print(f"{'#'*60}")

        nrow, ncol, nnz, indptr, indices = read_csrbin(csrbin)
        avg_deg = nnz / nrow
        row_degrees = np.diff(indptr).astype(np.int64)
        print(f"M={nrow:,} N={ncol:,} NNZ={nnz:,} avg_deg={avg_deg:.1f}")

        col_deg, col_ptr, col_indices = build_csc_fast(
            nrow, ncol, nnz, indptr, indices)

        t0 = time.time()
        r = simulate_multipass(nrow, ncol, nnz, indptr, indices,
                              col_deg, col_ptr, col_indices, row_degrees)
        print(f"Simulation: {time.time()-t0:.1f}s")

        # Pass1 leftover 分布（被 16-row 跳过的列，各有几行 avail）
        left = r['p1_leftover']
        if len(left) > 0:
            print(f"\n  Pass1 leftover distribution (columns skipped, avail<16):")
            print(f"    Total columns skipped: {len(left):,}")
            bins = [(2,3), (4,5), (6,7), (8,9), (10,11), (12,13), (14,15)]
            for lo, hi in bins:
                cnt = np.sum((left >= lo) & (left <= hi))
                pct = cnt / len(left) * 100
                bar = '#' * int(pct / 2)
                print(f"    avail {lo:>2}-{hi:>2}: {cnt:>8,} ({pct:>5.1f}%) {bar}")

        # 三方案对比
        a1p, a1n, a1u = r['A_p1']
        a2p, a2n, a2u = r['A_p2']
        a_total_nnz = a1n + a2n
        a_total_U = a1u + a2u
        a_cov = a_total_nnz / nnz * 100
        a_fb_pct = r['A_fb_nnz'] / nnz * 100

        b1p, b1n, b1u = r['B_p1']
        b2p, b2n, b2u = r['B_p2']
        b_total_nnz = b1n + b2n
        b_total_U = b1u + b2u
        b_cov = b_total_nnz / nnz * 100
        b_fb_pct = r['B_fb_nnz'] / nnz * 100

        b2_1p, b2_1n, b2_1u = r['B2_p1']
        b2_2p, b2_2n, b2_2u = r['B2_p2']
        b2_total_nnz = b2_1n + b2_2n
        b2_total_U = b2_1u + b2_2u
        b2_cov = b2_total_nnz / nnz * 100
        b2_fb_pct = r['B2_fb_nnz'] / nnz * 100

        print(f"\n  {'Scheme':<28} {'Panels':>8} {'AMX_NNZ%':>9} {'FB_NNZ%':>9} "
              f"{'FB_rows':>10} {'ΣU':>10}")
        print(f"  {'-'*70}")
        print(f"  {'A: 16+8+FB (current)':<28} {a1p+a2p:>8} {a_cov:>8.1f}% "
              f"{a_fb_pct:>8.1f}% {r['A_fb_rows']:>10,} {a_total_U:>10,}")
        print(f"  {'B: 16+(2-15)+FB (variable)':<28} {b1p+b2p:>8} {b_cov:>8.1f}% "
              f"{b_fb_pct:>8.1f}% {r['B_fb_rows']:>10,} {b_total_U:>10,}")
        print(f"  {'B2: 16+(6-15)+FB (conserv)':<28} {b2_1p+b2_2p:>8} {b2_cov:>8.1f}% "
              f"{b2_fb_pct:>8.1f}% {r['B2_fb_rows']:>10,} {b2_total_U:>10,}")

        # FB 行数减少百分比
        if r['A_fb_rows'] > 0:
            b_reduce = (1 - r['B_fb_rows'] / r['A_fb_rows']) * 100
            b2_reduce = (1 - r['B2_fb_rows'] / r['A_fb_rows']) * 100
            b_nnz_reduce = (1 - r['B_fb_nnz'] / r['A_fb_nnz']) * 100 if r['A_fb_nnz'] > 0 else 0
            b2_nnz_reduce = (1 - r['B2_fb_nnz'] / r['A_fb_nnz']) * 100 if r['A_fb_nnz'] > 0 else 0
            print(f"\n  Variable (2-15) vs Current: "
                  f"FB rows -{b_reduce:.1f}%, FB NNZ -{b_nnz_reduce:.1f}%")
            print(f"  Conserv (6-15) vs Current:  "
                  f"FB rows -{b2_reduce:.1f}%, FB NNZ -{b2_nnz_reduce:.1f}%")

    print(f"\n{'='*80}")
    print("DECISION GUIDE:")
    print("  FB NNZ reduction >15% → variable tile height worth implementing")
    print("  FB NNZ reduction 5-15% → marginal, depends on implementation effort")
    print("  FB NNZ reduction <5% → not worth it")
