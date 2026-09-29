#!/usr/bin/env python3
"""
Phase 12: Column Clustering via A^T×A Co-occurrence Matrix
===========================================================
核心思想: 先聚列再分行 (columns cluster → rows assign)
方法:
  1. 计算 A^T×A 共现矩阵（稀疏，带采样/截断）
  2. 贪心列聚类（每簇 ≤ 32 列 = 1 AMX tile 宽度）
  3. 行投票到最匹配的列簇
  4. 列簇内建 panel（16行），评估 fill rate
"""
import struct, sys, time
import numpy as np
from collections import defaultdict, Counter

def read_csrbin(filename):
    t0 = time.time()
    with open(filename, 'rb') as f:
        ptype = struct.unpack('I', f.read(4))[0]
        dtype = struct.unpack('I', f.read(4))[0]
        vtype = struct.unpack('I', f.read(4))[0]
        nrow  = struct.unpack('Q', f.read(8))[0]
        ncol  = struct.unpack('Q', f.read(8))[0]
        nnz   = struct.unpack('Q', f.read(8))[0]
        indptr  = np.frombuffer(f.read(4*(nrow+1)), dtype=np.uint32)
        indices = np.frombuffer(f.read(4*nnz),      dtype=np.uint32)
    print(f"  Read: {nrow:,} rows, {ncol:,} cols, {nnz:,} NNZ ({time.time()-t0:.2f}s)")
    return nrow, ncol, nnz, indptr, indices

def compute_cooccurrence(nrow, ncol, indptr, indices, col_count,
                         max_row_deg=300, sample_rate=1.0,
                         min_cooccur=3, top_k_neighbors=64):
    """计算列共现: 对每行枚举列对, 带度数截断和行采样"""
    t0 = time.time()
    np.random.seed(42)
    nrow_actual = int(indptr.shape[0]) - 1
    row_degrees = np.diff(indptr).astype(np.int64)

    total_rows = 0
    skipped_hd = 0
    cooccur = defaultdict(int)

    for r in range(nrow_actual):
        deg = int(row_degrees[r])
        if deg < 2:
            continue
        if deg > max_row_deg:
            skipped_hd += 1
            continue
        if sample_rate < 1.0 and np.random.random() > sample_rate:
            continue
        total_rows += 1
        cols = indices[indptr[r]:indptr[r+1]]
        n = len(cols)
        for i in range(n):
            for j in range(i+1, n):
                ci, cj = int(cols[i]), int(cols[j])
                if ci > cj: ci, cj = cj, ci
                cooccur[(ci, cj)] += 1
        if total_rows % 200000 == 0:
            print(f"    ... {total_rows:,} rows, {len(cooccur):,} pairs, {time.time()-t0:.1f}s")

    print(f"  Co-occurrence: {total_rows:,} rows, skipped {skipped_hd:,} high-deg")
    print(f"    Unique pairs: {len(cooccur):,} ({time.time()-t0:.2f}s)")

    # 过滤
    filtered = {k:v for k,v in cooccur.items() if v >= min_cooccur}
    print(f"    After filter (>={min_cooccur}): {len(filtered):,}")

    # 构建邻居列表
    col_neighbors = defaultdict(list)
    for (ci,cj), cnt in filtered.items():
        col_neighbors[ci].append((cj, cnt))
        col_neighbors[cj].append((ci, cnt))
    for c in col_neighbors:
        col_neighbors[c].sort(key=lambda x: -x[1])
        if len(col_neighbors[c]) > top_k_neighbors:
            col_neighbors[c] = col_neighbors[c][:top_k_neighbors]

    print(f"    Columns with neighbors: {len(col_neighbors):,}")
    if filtered:
        vals = np.array(list(filtered.values()))
        print(f"    Co-occur dist: min={vals.min()}, med={int(np.median(vals))}, "
              f"mean={vals.mean():.1f}, max={vals.max()}, P99={int(np.percentile(vals,99))}")

    return dict(col_neighbors)

def greedy_column_clustering(ncol, col_count, col_neighbors, max_cluster_size=32):
    """贪心列聚类: 种子驱动生长"""
    t0 = time.time()
    valid_cols = sorted([(c, int(col_count[c])) for c in range(ncol) if col_count[c] >= 1],
                        key=lambda x: -x[1])
    print(f"  Valid columns: {len(valid_cols):,}")

    col_to_cluster = {}
    clusters = {}
    cid = 0
    assigned = set()

    for seed_col, seed_deg in valid_cols:
        if seed_col in assigned:
            continue
        cur = [seed_col]
        assigned.add(seed_col)

        while len(cur) < max_cluster_size:
            scores = defaultdict(int)
            for c in cur:
                for nb, cnt in col_neighbors.get(c, []):
                    if nb not in assigned:
                        scores[nb] += cnt
            if not scores:
                break
            best = max(scores, key=scores.get)
            if scores[best] < 1:
                break
            cur.append(best)
            assigned.add(best)

        clusters[cid] = cur
        for c in cur:
            col_to_cluster[c] = cid
        cid += 1
        if cid % 10000 == 0:
            print(f"    ... {cid:,} clusters, {len(assigned):,} cols, {time.time()-t0:.1f}s")

    # 未分配列各自成簇
    for c in range(ncol):
        if c not in col_to_cluster and col_count[c] > 0:
            clusters[cid] = [c]
            col_to_cluster[c] = cid
            cid += 1

    print(f"  Clustering: {cid:,} clusters ({time.time()-t0:.2f}s)")
    sizes = np.array([len(v) for v in clusters.values()])
    print(f"    Size: min={sizes.min()}, med={int(np.median(sizes))}, "
          f"mean={sizes.mean():.1f}, max={sizes.max()}")
    print(f"    Size=1: {(sizes==1).sum():,} ({100*(sizes==1).sum()/len(sizes):.1f}%)")
    print(f"    Size=32: {(sizes==max_cluster_size).sum():,} ({100*(sizes==max_cluster_size).sum()/len(sizes):.1f}%)")
    return col_to_cluster, clusters

def build_panels_from_clusters(nrow, ncol, indptr, indices,
                                col_to_cluster, clusters, panel_size=16):
    """行投票到列簇，簇内每16行建 panel"""
    t0 = time.time()
    nrow_actual = int(indptr.shape[0]) - 1
    cluster_rows = defaultdict(list)
    total_in = 0
    total_spill = 0

    for r in range(nrow_actual):
        s, e = int(indptr[r]), int(indptr[r+1])
        if s == e: continue
        votes = defaultdict(int)
        for idx in range(s, e):
            c = int(indices[idx])
            if c in col_to_cluster:
                votes[col_to_cluster[c]] += 1
        if not votes: continue
        best = max(votes, key=votes.get)
        deg = e - s
        cluster_rows[best].append((r, votes[best], deg))
        total_in += votes[best]
        total_spill += (deg - votes[best])

    total = total_in + total_spill
    print(f"  Row assignment ({time.time()-t0:.2f}s)")
    print(f"    In-cluster NNZ:  {total_in:,} ({100*total_in/max(total,1):.1f}%)")
    print(f"    Spillover NNZ:   {total_spill:,} ({100*total_spill/max(total,1):.1f}%)")

    # 建 panel
    t1 = time.time()
    panels = []
    total_paneled = 0
    for cid, rlist in cluster_rows.items():
        cols_set = set(clusters[cid])
        rlist.sort(key=lambda x: -x[1])
        for si in range(0, len(rlist), panel_size):
            batch = rlist[si:si+panel_size]
            nr = len(batch)
            pcols = set()
            pnnz = 0
            for (r, _, _) in batch:
                for idx in range(int(indptr[r]), int(indptr[r+1])):
                    c = int(indices[idx])
                    if c in cols_set:
                        pcols.add(c)
                        pnnz += 1
            U = len(pcols)
            fill = pnnz / (nr * U) if U > 0 else 0
            panels.append({'fill': fill, 'U': U, 'n_rows': nr,
                           'nnz': pnnz, 'n_tiles': max(1,(U+31)//32),
                           'csize': len(cols_set)})
            total_paneled += pnnz

    print(f"  Panels built: {len(panels):,} ({time.time()-t1:.2f}s)")
    return panels, total_paneled

def current_column_anchor(nrow, ncol, indptr, indices, col_count, panel_size=16):
    """当前列锚点 Config A 复现"""
    t0 = time.time()
    nrow_actual = int(indptr.shape[0]) - 1
    col_to_rows = defaultdict(list)
    for r in range(nrow_actual):
        for idx in range(int(indptr[r]), int(indptr[r+1])):
            col_to_rows[int(indices[idx])].append(r)

    sorted_cols = sorted(col_to_rows.keys(), key=lambda c: -len(col_to_rows[c]))
    row_deg = np.diff(indptr).astype(np.int64)
    used = set()
    panels = []

    for ac in sorted_cols:
        cands = [r for r in col_to_rows[ac] if r not in used]
        if len(cands) < panel_size: continue
        cands.sort(key=lambda r: row_deg[r])
        sel = cands[:panel_size]
        ucols = set()
        pnnz = 0
        for r in sel:
            for idx in range(int(indptr[r]), int(indptr[r+1])):
                ucols.add(int(indices[idx]))
                pnnz += 1
        U = len(ucols)
        fill = pnnz / (panel_size * U) if U > 0 else 0
        panels.append({'fill': fill, 'U': U, 'nnz': pnnz, 'n_tiles': max(1,(U+31)//32)})
        for r in sel: used.add(r)

    print(f"  Column-Anchor baseline: {len(panels):,} panels ({time.time()-t0:.2f}s)")
    return panels

def report(panels, nnz, label):
    """输出统计报告"""
    if not panels:
        print(f"  [{label}] No panels!"); return
    fills = np.array([p['fill'] for p in panels])
    Us    = np.array([p['U']    for p in panels])
    tiles = np.array([p['n_tiles'] for p in panels])
    nnzs  = np.array([p['nnz'] for p in panels])

    print(f"\n  [{label}]")
    print(f"    Panels:      {len(panels):,}")
    print(f"    NNZ covered: {int(nnzs.sum()):,} ({100*nnzs.sum()/nnz:.1f}%)")
    print(f"    Avg fill:    {fills.mean()*100:.1f}%")
    print(f"    Med fill:    {np.median(fills)*100:.1f}%")
    print(f"    P25/P75:     {np.percentile(fills,25)*100:.1f}% / {np.percentile(fills,75)*100:.1f}%")
    print(f"    Avg U:       {Us.mean():.1f}")
    print(f"    Med U:       {np.median(Us):.0f}")
    print(f"    Avg tiles:   {tiles.mean():.1f}")
    print(f"    U<=32:       {100*(Us<=32).sum()/len(Us):.1f}%")
    print(f"    U<=64:       {100*(Us<=64).sum()/len(Us):.1f}%")
    print(f"    >=盈亏线:    {100*(fills>=0.0625).sum()/len(fills):.1f}%")
    return fills, Us

def analyze(path, name, max_row_deg, sample_rate, min_cooccur, max_cluster=32):
    print(f"\n{'#'*70}")
    print(f"# {name}  (deg_cap={max_row_deg}, sample={sample_rate}, "
          f"min_co={min_cooccur}, cluster={max_cluster})")
    print(f"{'#'*70}")

    nrow, ncol, nnz, indptr, indices = read_csrbin(path)
    col_count = np.zeros(ncol, dtype=np.int64)
    for j in range(nnz):
        col_count[indices[j]] += 1

    # A^T×A co-occurrence
    print(f"\n--- A^T×A Co-occurrence ---")
    col_nb = compute_cooccurrence(nrow, ncol, indptr, indices, col_count,
                                  max_row_deg, sample_rate, min_cooccur)

    # Clustering
    print(f"\n--- Greedy Column Clustering ---")
    c2c, clusters = greedy_column_clustering(ncol, col_count, col_nb, max_cluster)

    # Build panels from clusters
    print(f"\n--- Panel Building (Column Clustering) ---")
    cc_panels, cc_nnz = build_panels_from_clusters(
        nrow, ncol, indptr, indices, c2c, clusters, 16)

    # Baseline
    print(f"\n--- Column-Anchor Baseline ---")
    ca_panels = current_column_anchor(nrow, ncol, indptr, indices, col_count, 16)

    # Report
    print(f"\n{'='*70}")
    print(f" RESULTS: {name}")
    print(f"{'='*70}")
    cc_f, cc_u = report(cc_panels, nnz, "Column Clustering (A^T×A)")
    ca_f, ca_u = report(ca_panels, nnz, "Column Anchor (current)")

    if cc_f is not None and ca_f is not None:
        print(f"\n  --- HEAD-TO-HEAD ---")
        print(f"    Fill ratio (CC/CA): {cc_f.mean()/ca_f.mean():.2f}x")
        print(f"    U ratio (CC/CA):    {cc_u.mean()/ca_u.mean():.2f}x {'(BETTER!)' if cc_u.mean()<ca_u.mean() else ''}")
        cc_cov = sum(p['nnz'] for p in cc_panels)
        ca_cov = sum(p['nnz'] for p in ca_panels)
        print(f"    Coverage (CC/CA):   {cc_cov:,} vs {ca_cov:,}")

    print(f"\nDone: {name}\n")

if __name__ == "__main__":
    import os
    D = os.path.expanduser("~/data/SpMM_project/data")
    cfgs = {
        'web-Google':     (f"{D}/web-Google/web-Google.csrbin",         500, 1.0, 3),
        'amazon0601':     (f"{D}/amazon0601/amazon0601.csrbin",         500, 1.0, 3),
        'as-Skitter':     (f"{D}/as-Skitter/as-Skitter.csrbin",        200, 0.5, 4),
        'soc-Pokec':      (f"{D}/soc-Pokec/soc-Pokec.csrbin",          200, 0.3, 4),
        'hollywood-2009': (f"{D}/hollywood-2009/hollywood-2009.csrbin", 150, 0.1, 5),
    }
    names = sys.argv[1:] if len(sys.argv) > 1 else ['web-Google', 'amazon0601']
    for n in names:
        if n in cfgs:
            p, mrd, sr, mc = cfgs[n]
            analyze(p, n, mrd, sr, mc)
        else:
            print(f"Unknown: {n}. Available: {list(cfgs.keys())}")
    print(f"\n{'#'*70}\n# ALL DONE\n{'#'*70}")
