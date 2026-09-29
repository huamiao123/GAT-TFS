#!/usr/bin/env python3
"""
Phase 12.2: Column Clustering Improvements
方向 A: Hybrid（好簇用聚类 + 剩余退回列锚点）
方向 B: 低阈值 + 孤岛挂靠（min_cooccur=1 + orphan adoption）
"""
import struct, sys, time, os
import numpy as np
from collections import defaultdict

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
                         min_cooccur=1, top_k_neighbors=96):
    t0 = time.time()
    np.random.seed(42)
    nrow_actual = int(indptr.shape[0]) - 1
    row_degrees = np.diff(indptr).astype(np.int64)
    total_rows = 0; skipped_hd = 0
    cooccur = defaultdict(int)
    for r in range(nrow_actual):
        deg = int(row_degrees[r])
        if deg < 2: continue
        if deg > max_row_deg: skipped_hd += 1; continue
        if sample_rate < 1.0 and np.random.random() > sample_rate: continue
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
    filtered = {k:v for k,v in cooccur.items() if v >= min_cooccur}
    print(f"    After filter (>={min_cooccur}): {len(filtered):,}")
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
    return dict(col_neighbors), filtered

def greedy_column_clustering(ncol, col_count, col_neighbors, max_cluster_size=32):
    t0 = time.time()
    valid_cols = sorted([(c, int(col_count[c])) for c in range(ncol) if col_count[c] >= 1],
                        key=lambda x: -x[1])
    col_to_cluster = {}; clusters = {}; cid = 0; assigned = set()
    for seed_col, seed_deg in valid_cols:
        if seed_col in assigned: continue
        cur = [seed_col]; assigned.add(seed_col)
        while len(cur) < max_cluster_size:
            scores = defaultdict(int)
            for c in cur:
                for nb, cnt in col_neighbors.get(c, []):
                    if nb not in assigned: scores[nb] += cnt
            if not scores: break
            best = max(scores, key=scores.get)
            if scores[best] < 1: break
            cur.append(best); assigned.add(best)
        clusters[cid] = cur
        for c in cur: col_to_cluster[c] = cid
        cid += 1
    for c in range(ncol):
        if c not in col_to_cluster and col_count[c] > 0:
            clusters[cid] = [c]; col_to_cluster[c] = cid; cid += 1
    sizes = np.array([len(v) for v in clusters.values()])
    print(f"  Clustering: {cid:,} clusters ({time.time()-t0:.2f}s)")
    print(f"    Size: min={sizes.min()}, med={int(np.median(sizes))}, mean={sizes.mean():.1f}, max={sizes.max()}")
    print(f"    Size=1: {(sizes==1).sum():,} ({100*(sizes==1).sum()/len(sizes):.1f}%)")
    print(f"    Size>=8: {(sizes>=8).sum():,} ({100*(sizes>=8).sum()/len(sizes):.1f}%)")
    print(f"    Size=32: {(sizes==max_cluster_size).sum():,} ({100*(sizes==max_cluster_size).sum()/len(sizes):.1f}%)")
    return col_to_cluster, clusters

def column_anchor_baseline(nrow, ncol, indptr, indices, col_count, panel_size=16):
    t0 = time.time()
    nrow_actual = int(indptr.shape[0]) - 1
    col_to_rows = defaultdict(list)
    for r in range(nrow_actual):
        for idx in range(int(indptr[r]), int(indptr[r+1])):
            col_to_rows[int(indices[idx])].append(r)
    sorted_cols = sorted(col_to_rows.keys(), key=lambda c: -len(col_to_rows[c]))
    row_deg = np.diff(indptr).astype(np.int64)
    used = set(); panels = []
    for ac in sorted_cols:
        cands = [r for r in col_to_rows[ac] if r not in used]
        if len(cands) < panel_size: continue
        cands.sort(key=lambda r: row_deg[r])
        sel = cands[:panel_size]
        ucols = set(); pnnz = 0
        for r in sel:
            for idx in range(int(indptr[r]), int(indptr[r+1])):
                ucols.add(int(indices[idx])); pnnz += 1
        U = len(ucols)
        fill = pnnz / (panel_size * U) if U > 0 else 0
        panels.append({'fill': fill, 'U': U, 'nnz': pnnz,
                        'n_tiles': max(1,(U+31)//32), 'n_rows': panel_size})
        for r in sel: used.add(r)
    print(f"  Column-Anchor: {len(panels):,} panels ({time.time()-t0:.2f}s)")
    return panels

def hybrid_strategy(nrow, ncol, indptr, indices, col_count,
                    col_to_cluster, clusters, min_cluster_size=8, panel_size=16):
    t0 = time.time()
    nrow_actual = int(indptr.shape[0]) - 1
    row_deg = np.diff(indptr).astype(np.int64)
    good_clusters = {cid: cols for cid, cols in clusters.items() if len(cols) >= min_cluster_size}
    good_cids = set(good_clusters.keys())
    print(f"  Good clusters (size>={min_cluster_size}): {len(good_clusters):,}")
    if good_clusters:
        gs = np.array([len(v) for v in good_clusters.values()])
        print(f"    Size: mean={gs.mean():.1f}, med={int(np.median(gs))}, max={gs.max()}")

    cluster_rows = defaultdict(list); leftover_rows = []
    for r in range(nrow_actual):
        s, e = int(indptr[r]), int(indptr[r+1])
        if s == e: continue
        votes = defaultdict(int)
        for idx in range(s, e):
            c = int(indices[idx])
            if c in col_to_cluster:
                cid = col_to_cluster[c]
                if cid in good_cids: votes[cid] += 1
        if votes:
            best = max(votes, key=votes.get)
            if votes[best] >= max(2, (e-s)*0.3):
                cluster_rows[best].append((r, votes[best], e-s))
            else: leftover_rows.append(r)
        else: leftover_rows.append(r)

    n_clust = sum(len(v) for v in cluster_rows.values())
    print(f"  Row assignment ({time.time()-t0:.2f}s): {n_clust:,} clustered, {len(leftover_rows):,} leftover")

    # Cluster panels
    t1 = time.time(); panels_c = []; used = set()
    for cid, rlist in cluster_rows.items():
        cols_set = set(clusters[cid]); rlist.sort(key=lambda x: -x[1])
        for si in range(0, len(rlist), panel_size):
            batch = rlist[si:si+panel_size]; nr = len(batch)
            if nr < panel_size: continue
            pcols = set(); pnnz = 0
            for (r, _, _) in batch:
                for idx in range(int(indptr[r]), int(indptr[r+1])):
                    c = int(indices[idx])
                    if c in cols_set: pcols.add(c); pnnz += 1
            U = len(pcols)
            if U == 0: continue
            panels_c.append({'fill': pnnz/(nr*U), 'U': U, 'n_rows': nr,
                             'nnz': pnnz, 'n_tiles': max(1,(U+31)//32), 'src': 'C'})
            for (r,_,_) in batch: used.add(r)
    print(f"  Cluster panels: {len(panels_c):,} ({time.time()-t1:.2f}s)")

    # Anchor fallback for leftover
    t2 = time.time()
    all_left = set(leftover_rows) | (set(range(nrow_actual)) - used)
    col_to_rows_left = defaultdict(list)
    for r in all_left:
        for idx in range(int(indptr[r]), int(indptr[r+1])):
            col_to_rows_left[int(indices[idx])].append(r)
    sorted_cols = sorted(col_to_rows_left.keys(), key=lambda c: -len(col_to_rows_left[c]))
    used_l = set(); panels_a = []
    for ac in sorted_cols:
        cands = [r for r in col_to_rows_left[ac] if r not in used_l]
        if len(cands) < panel_size: continue
        cands.sort(key=lambda r: row_deg[r]); sel = cands[:panel_size]
        ucols = set(); pnnz = 0
        for r in sel:
            for idx in range(int(indptr[r]), int(indptr[r+1])):
                ucols.add(int(indices[idx])); pnnz += 1
        U = len(ucols)
        if U == 0: continue
        panels_a.append({'fill': pnnz/(panel_size*U), 'U': U, 'n_rows': panel_size,
                         'nnz': pnnz, 'n_tiles': max(1,(U+31)//32), 'src': 'A'})
        for r in sel: used_l.add(r)
    print(f"  Anchor panels: {len(panels_a):,} ({time.time()-t2:.2f}s)")
    all_p = panels_c + panels_a
    print(f"  HYBRID TOTAL: {len(all_p):,} = {len(panels_c)} cluster + {len(panels_a)} anchor")
    return all_p

def orphan_adoption(ncol, col_count, col_to_cluster, clusters, col_neighbors, max_cluster_size=32):
    t0 = time.time()
    orphans = [cid for cid, cols in clusters.items() if len(cols) == 1]
    adopted = 0; rej_full = 0; rej_no = 0
    for ocid in orphans:
        ocol = clusters[ocid][0]
        nbs = col_neighbors.get(ocol, [])
        if not nbs: rej_no += 1; continue
        aff = defaultdict(int)
        for nb, cnt in nbs:
            if nb in col_to_cluster:
                nbcid = col_to_cluster[nb]
                if nbcid != ocid and len(clusters.get(nbcid,[])) >= 2:
                    aff[nbcid] += cnt
        if not aff: rej_no += 1; continue
        best = max(aff, key=aff.get)
        if len(clusters[best]) >= max_cluster_size: rej_full += 1; continue
        clusters[best].append(ocol); col_to_cluster[ocol] = best
        del clusters[ocid]; adopted += 1
    print(f"  Orphan adoption ({time.time()-t0:.2f}s): {adopted:,} adopted, "
          f"{rej_full:,} full, {rej_no:,} no-neighbor")
    sizes = np.array([len(v) for v in clusters.values()])
    print(f"    Clusters: {len(clusters):,}, Size=1: {(sizes==1).sum():,} ({100*(sizes==1).sum()/len(sizes):.1f}%), "
          f"Size>=8: {(sizes>=8).sum():,}, Size=32: {(sizes==max_cluster_size).sum():,}")
    return col_to_cluster, clusters

def build_panels(nrow, ncol, indptr, indices, col_to_cluster, clusters, panel_size=16):
    t0 = time.time()
    nrow_actual = int(indptr.shape[0]) - 1
    cluster_rows = defaultdict(list)
    total_in = 0; total_spill = 0
    for r in range(nrow_actual):
        s, e = int(indptr[r]), int(indptr[r+1])
        if s == e: continue
        votes = defaultdict(int)
        for idx in range(s, e):
            c = int(indices[idx])
            if c in col_to_cluster: votes[col_to_cluster[c]] += 1
        if not votes: continue
        best = max(votes, key=votes.get); deg = e-s
        cluster_rows[best].append((r, votes[best], deg))
        total_in += votes[best]; total_spill += (deg - votes[best])
    total = total_in + total_spill
    print(f"  Row assignment ({time.time()-t0:.2f}s): "
          f"in-cluster {total_in:,} ({100*total_in/max(total,1):.1f}%), "
          f"spill {total_spill:,} ({100*total_spill/max(total,1):.1f}%)")
    t1 = time.time(); panels = []
    for cid, rlist in cluster_rows.items():
        cols_set = set(clusters[cid]); rlist.sort(key=lambda x: -x[1])
        for si in range(0, len(rlist), panel_size):
            batch = rlist[si:si+panel_size]; nr = len(batch)
            pcols = set(); pnnz = 0
            for (r,_,_) in batch:
                for idx in range(int(indptr[r]), int(indptr[r+1])):
                    c = int(indices[idx])
                    if c in cols_set: pcols.add(c); pnnz += 1
            U = len(pcols)
            if U == 0: continue
            panels.append({'fill': pnnz/(nr*U), 'U': U, 'n_rows': nr,
                           'nnz': pnnz, 'n_tiles': max(1,(U+31)//32)})
    print(f"  Panels: {len(panels):,} ({time.time()-t1:.2f}s)")
    return panels

def report(panels, nnz, label):
    if not panels: print(f"  [{label}] No panels!"); return None, None
    fills = np.array([p['fill'] for p in panels])
    Us = np.array([p['U'] for p in panels])
    tiles = np.array([p['n_tiles'] for p in panels])
    nnzs = np.array([p['nnz'] for p in panels])
    n_rows = np.array([p['n_rows'] for p in panels])
    full_mask = n_rows == 16; n_full = full_mask.sum()
    print(f"\n  [{label}]")
    print(f"    Panels:      {len(panels):,} (full-16: {n_full:,})")
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
    if n_full > 100:
        print(f"    [Full-16 only] fill={fills[full_mask].mean()*100:.1f}%, U={Us[full_mask].mean():.1f}, U<=32={100*(Us[full_mask]<=32).sum()/n_full:.1f}%")
    return fills, Us

def analyze(path, name, max_row_deg, sample_rate):
    print(f"\n{'#'*70}")
    print(f"# {name}")
    print(f"{'#'*70}")
    nrow, ncol, nnz, indptr, indices = read_csrbin(path)
    col_count = np.zeros(ncol, dtype=np.int64)
    for j in range(nnz): col_count[indices[j]] += 1

    # A^T×A (低阈值, 给 B 用; 高阈值从中筛选, 给 A 用)
    print(f"\n===== A^T×A Co-occurrence (min_cooccur=1) =====")
    col_nb_low, cooccur_all = compute_cooccurrence(
        nrow, ncol, indptr, indices, col_count,
        max_row_deg=max_row_deg, sample_rate=sample_rate,
        min_cooccur=1, top_k_neighbors=96)

    # 构建高阈值邻居
    col_nb_high = defaultdict(list)
    for (ci,cj), cnt in cooccur_all.items():
        if cnt >= 3:
            col_nb_high[ci].append((cj, cnt))
            col_nb_high[cj].append((ci, cnt))
    for c in col_nb_high:
        col_nb_high[c].sort(key=lambda x: -x[1])
        if len(col_nb_high[c]) > 64: col_nb_high[c] = col_nb_high[c][:64]
    col_nb_high = dict(col_nb_high)
    print(f"  High-threshold (>=3): {len(col_nb_high):,} columns")

    # Baseline
    print(f"\n===== Baseline: Column Anchor =====")
    ca_panels = column_anchor_baseline(nrow, ncol, indptr, indices, col_count)

    # 方向 A: Hybrid
    print(f"\n===== Strategy A: Hybrid =====")
    c2c_a, cl_a = greedy_column_clustering(ncol, col_count, col_nb_high, 32)
    hyb_panels = hybrid_strategy(nrow, ncol, indptr, indices, col_count,
                                  c2c_a, cl_a, min_cluster_size=8)

    # 方向 B: Low-threshold + Orphan
    print(f"\n===== Strategy B: Low-threshold + Orphan =====")
    c2c_b, cl_b = greedy_column_clustering(ncol, col_count, col_nb_low, 32)
    c2c_b, cl_b = orphan_adoption(ncol, col_count, c2c_b, cl_b, col_nb_low, 32)
    b_panels = build_panels(nrow, ncol, indptr, indices, c2c_b, cl_b, 16)

    # 对比
    print(f"\n{'='*70}")
    print(f" COMPARISON: {name}")
    print(f"{'='*70}")
    ca_f, ca_u = report(ca_panels, nnz, "Column Anchor (baseline)")
    ha_f, ha_u = report(hyb_panels, nnz, "Hybrid (A)")
    lb_f, lb_u = report(b_panels, nnz, "Low-thresh+Adopt (B)")

    print(f"\n  --- SUMMARY TABLE ---")
    print(f"  {'Method':<30s} {'Panels':>8s} {'NNZ_cov':>10s} {'Cov%':>6s} {'Fill':>7s} {'U':>6s} {'U<=32':>6s}")
    print(f"  {'-'*30} {'-'*8} {'-'*10} {'-'*6} {'-'*7} {'-'*6} {'-'*6}")
    for lbl, pnl, f, u in [("Col-Anchor", ca_panels, ca_f, ca_u),
                             ("Hybrid(A)", hyb_panels, ha_f, ha_u),
                             ("LowThr+Adopt(B)", b_panels, lb_f, lb_u)]:
        if f is None: continue
        cov = sum(p['nnz'] for p in pnl)
        print(f"  {lbl:<30s} {len(pnl):>8,} {cov:>10,} {100*cov/nnz:>5.1f}% "
              f"{f.mean()*100:>6.1f}% {u.mean():>5.1f} {100*(u<=32).sum()/len(u):>5.1f}%")

    if ca_f is not None and ha_f is not None:
        print(f"\n  Hybrid vs Anchor: fill {ha_f.mean()/ca_f.mean():.2f}x, "
              f"coverage {sum(p['nnz'] for p in hyb_panels)/sum(p['nnz'] for p in ca_panels):.2f}x")
    if ca_f is not None and lb_f is not None:
        print(f"  LowThr  vs Anchor: fill {lb_f.mean()/ca_f.mean():.2f}x, "
              f"coverage {sum(p['nnz'] for p in b_panels)/sum(p['nnz'] for p in ca_panels):.2f}x")
    print(f"\nDone: {name}\n")

if __name__ == "__main__":
    D = os.path.expanduser("~/data/SpMM_project/data")
    cfgs = {
        'web-Google':     (f"{D}/web-Google/web-Google.csrbin",         500, 1.0),
        'amazon0601':     (f"{D}/amazon0601/amazon0601.csrbin",         500, 1.0),
        'as-Skitter':     (f"{D}/as-Skitter/as-Skitter.csrbin",        200, 0.5),
        'soc-Pokec':      (f"{D}/soc-Pokec/soc-Pokec.csrbin",          200, 0.3),
        'hollywood-2009': (f"{D}/hollywood-2009/hollywood-2009.csrbin", 150, 0.1),
    }
    names = sys.argv[1:] if len(sys.argv) > 1 else ['web-Google', 'amazon0601']
    for n in names:
        if n in cfgs:
            p, mrd, sr = cfgs[n]
            analyze(p, n, mrd, sr)
        else: print(f"Unknown: {n}")
    print(f"\n{'#'*70}\n# ALL DONE\n{'#'*70}")
