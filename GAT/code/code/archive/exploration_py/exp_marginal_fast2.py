#!/usr/bin/env python3
"""
Marginal NNZ/U — 快速近似版

核心思路：不做逐步贪心，而是一次性算每个候选行的"列共享度"

对每个 anchor column c:
  1. 收集所有候选行 R_c
  2. 统计候选行中每列的出现频率 freq[j]
  3. 对每个候选行 r，算：
     - n_common = r 的列中 freq >= 2 的列数（和其他行共享的列）
     - n_rare = degree(r) - n_common（只有 r 自己用的列）
     - score = degree(r) / max(n_rare, 1)
       score 高 = 加入后新增 U 少、贡献 NNZ 多
  4. 按 score 降序取 top-16

复杂度: O(|R_c| × avg_deg) per anchor，无迭代
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

def run_strategy(name, mode, nrow, ncol, nnz, indptr, indices,
                 col_deg, col_ptr, col_indices, row_degrees,
                 max_panels=3000):
    TILE_R = 16
    col_order = np.argsort(col_deg)[::-1]
    assigned = np.zeros(nrow, dtype=bool)

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
                k = min(TILE_R-1, len(avail)-1)
                part_idx = np.argpartition(degs, k)[:TILE_R]
                selected = avail[part_idx]
            else:
                selected = avail

        elif mode == 'high_degree':
            if len(avail) > TILE_R:
                degs = row_degrees[avail]
                k = min(TILE_R-1, len(avail)-1)
                part_idx = np.argpartition(-degs, k)[:TILE_R]
                selected = avail[part_idx]
            else:
                selected = avail

        elif mode == 'marginal':
            if len(avail) <= TILE_R:
                selected = avail
            else:
                # 如果候选太多，先采样
                if len(avail) > 500:
                    rng = np.random.RandomState(ci & 0xFFFF)
                    sample_idx = rng.choice(len(avail), 500, replace=False)
                    # 确保 top-50 高度行包含
                    degs = row_degrees[avail]
                    if len(avail) > 50:
                        top50 = np.argpartition(-degs, 50)[:50]
                        sample_idx = np.unique(
                            np.concatenate([sample_idx, top50]))
                    cand = avail[sample_idx]
                else:
                    cand = avail

                # Step 1: 收集所有候选行的列索引
                # np.concatenate: 把多个数组拼成一个
                all_cols = np.concatenate([
                    indices[indptr[r]:indptr[r+1]] for r in cand
                ])

                # Step 2: 统计列频率（在候选行中每列出现几次）
                # np.bincount: 统计每个值的出现次数
                if len(all_cols) > 0:
                    local_freq = np.bincount(all_cols,
                                             minlength=ncol)
                else:
                    local_freq = np.zeros(ncol, dtype=np.int64)

                # Step 3: 对每个候选行算 score
                scores = np.empty(len(cand), dtype=np.float64)
                for i, r in enumerate(cand):
                    cols = indices[indptr[r]:indptr[r+1]]
                    deg = len(cols)
                    if deg == 0:
                        scores[i] = 0
                        continue
                    # n_rare = 该行中只被自己用到的列数
                    #   freq == 1 意味着只有这一行有这列
                    #   这些列加入 panel 后必定增加 U
                    freqs = local_freq[cols]
                    n_rare = int(np.sum(freqs <= 1))
                    # score = NNZ / 新增U估计
                    scores[i] = deg / max(n_rare, 1)

                # Step 4: 取 score 最高的 TILE_R 行
                k = min(TILE_R-1, len(cand)-1)
                top_idx = np.argpartition(-scores, k)[:TILE_R]
                selected = cand[top_idx]

        # 计算 panel 统计
        panel_cols = set()
        panel_nnz = 0
        for r in selected:
            cols_r = indices[indptr[r]:indptr[r+1]]
            panel_cols.update(cols_r.tolist())
            panel_nnz += len(cols_r)

        U = len(panel_cols)
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

        if total_panels % 1000 == 0:
            print(f"    [{name}] {total_panels} panels, "
                  f"{time.time()-t0:.1f}s", flush=True)

    elapsed = time.time() - t0

    fb_nnz = int(np.sum(
        (indptr[1:] - indptr[:-1])[~assigned]))

    amx_cov = total_nnz_amx / nnz * 100
    fb_pct = fb_nnz / nnz * 100
    reuse = total_nnz_amx / total_U if total_U > 0 else 0
    avg_fill = np.mean(panel_fill_list) if panel_fill_list else 0
    avg_U = np.mean(panel_U_list) if panel_U_list else 0

    return {
        'name': name, 'panels': total_panels,
        'amx_nnz': total_nnz_amx, 'amx_cov': amx_cov,
        'fb_nnz': fb_nnz, 'fb_pct': fb_pct,
        'total_U': total_U, 'avg_U': avg_U,
        'reuse': reuse, 'avg_fill': avg_fill,
        'elapsed': elapsed,
        'panel_U': np.array(panel_U_list),
    }

if __name__ == '__main__':
    DATA_DIR = os.path.expanduser('~/data/SpMM_project/data')
    matrices = [
        'web-Google', 'amazon0601', 'soc-Pokec',
        'as-Skitter', 'cit-Patents', 'indochina-2004', 'hollywood-2009'
    ]

    print("=" * 85)
    print("Row Selection: Config-A vs Config-B vs Marginal (fast approximate)")
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
        print(f"M={nrow:,} N={ncol:,} NNZ={nnz:,} avg_deg={avg_deg:.1f}")

        print("Building CSC...")
        t_csc = time.time()
        col_deg, col_ptr, col_indices = build_csc_fast(
            nrow, ncol, nnz, indptr, indices)
        print(f"CSC: {time.time()-t_csc:.1f}s")

        results = []
        for sname, smode in [("Config-A", 'low_degree'),
                              ("Config-B", 'high_degree'),
                              ("Marginal", 'marginal')]:
            print(f"  Running {sname}...")
            r = run_strategy(sname, smode, nrow, ncol, nnz,
                           indptr, indices, col_deg, col_ptr,
                           col_indices, row_degrees, max_panels=3000)
            print(f"  → {r['panels']} panels, {r['elapsed']:.1f}s, "
                  f"cov={r['amx_cov']:.1f}%, reuse={r['reuse']:.2f}x, "
                  f"fill={r['avg_fill']*100:.1f}%")
            results.append(r)

        # 详细对比
        a, b, m = results
        print(f"\n  {'Strategy':<14} {'Panels':>7} {'AMX%':>7} {'FB%':>7} "
              f"{'ΣU':>9} {'avgU':>6} {'Reuse':>6} {'Fill%':>6}")
        print(f"  {'-'*65}")
        for r in results:
            print(f"  {r['name']:<14} {r['panels']:>7} "
                  f"{r['amx_cov']:>6.1f}% {r['fb_pct']:>6.1f}% "
                  f"{r['total_U']:>9,} {r['avg_U']:>6.0f} "
                  f"{r['reuse']:>5.2f}x {r['avg_fill']*100:>5.1f}%")

        print(f"\n  Marginal vs A: cov {a['amx_cov']:.1f}→{m['amx_cov']:.1f}% "
              f"({m['amx_cov']-a['amx_cov']:+.1f}), "
              f"ΣU {a['total_U']:,}→{m['total_U']:,}, "
              f"reuse {a['reuse']:.2f}→{m['reuse']:.2f}x")
        print(f"  Marginal vs B: cov {b['amx_cov']:.1f}→{m['amx_cov']:.1f}% "
              f"({m['amx_cov']-b['amx_cov']:+.1f}), "
              f"ΣU {b['total_U']:,}→{m['total_U']:,}, "
              f"reuse {b['reuse']:.2f}→{m['reuse']:.2f}x")

        all_results.append((mname, results))

    # 全局汇总
    print(f"\n{'='*85}")
    print("SUMMARY")
    print(f"{'='*85}")
    print(f"{'Matrix':<16} {'A-cov%':>8} {'B-cov%':>8} {'M-cov%':>8} "
          f"{'A-reuse':>8} {'B-reuse':>8} {'M-reuse':>8}")
    print("-" * 60)
    for mname, res in all_results:
        a, b, m = res
        print(f"{mname:<16} {a['amx_cov']:>7.1f}% {b['amx_cov']:>7.1f}% "
              f"{m['amx_cov']:>7.1f}% "
              f"{a['reuse']:>7.2f}x {b['reuse']:>7.2f}x {m['reuse']:>7.2f}x")
