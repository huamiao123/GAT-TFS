#!/usr/bin/env python3
"""
Phase 12.2 Job2: Performance Impact Analysis
对 web-Google & amazon0601, 精确计算 Hybrid vs Column-Anchor 的:
  - total tiles, gather rows, AMX coverage
  - 性能模型估算 (基于 Phase 10 profiling 权重)
  - K 敏感性分析
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
                         max_row_deg, sample_rate, min_cooccur=3, top_k=64):
    t0 = time.time()
    np.random.seed(42)
    nra = int(indptr.shape[0]) - 1
    rd = np.diff(indptr).astype(np.int64)
    total = 0; skipped = 0
    cooccur = defaultdict(int)
    for r in range(nra):
        d = int(rd[r])
        if d < 2: continue
        if d > max_row_deg: skipped += 1; continue
        if sample_rate < 1.0 and np.random.random() > sample_rate: continue
        total += 1
        cols = indices[indptr[r]:indptr[r+1]]
        n = len(cols)
        for i in range(n):
            for j in range(i+1, n):
                ci, cj = int(cols[i]), int(cols[j])
                if ci > cj: ci, cj = cj, ci
                cooccur[(ci,cj)] += 1
        if total % 200000 == 0:
            print(f"    ... {total:,} rows, {time.time()-t0:.1f}s")
    filtered = {k:v for k,v in cooccur.items() if v >= min_cooccur}
    col_nb = defaultdict(list)
    for (ci,cj), cnt in filtered.items():
        col_nb[ci].append((cj,cnt)); col_nb[cj].append((ci,cnt))
    for c in col_nb:
        col_nb[c].sort(key=lambda x: -x[1])
        if len(col_nb[c]) > top_k: col_nb[c] = col_nb[c][:top_k]
    print(f"  Co-occurrence: {total:,} rows, {len(filtered):,} pairs ({time.time()-t0:.1f}s)")
    return dict(col_nb)

def greedy_clustering(ncol, col_count, col_nb, max_size=32):
    valid = sorted([(c, int(col_count[c])) for c in range(ncol) if col_count[c]>=1], key=lambda x:-x[1])
    c2c = {}; clusters = {}; cid = 0; used = set()
    for sc, _ in valid:
        if sc in used: continue
        cur = [sc]; used.add(sc)
        while len(cur) < max_size:
            scores = defaultdict(int)
            for c in cur:
                for nb, cnt in col_nb.get(c, []):
                    if nb not in used: scores[nb] += cnt
            if not scores: break
            best = max(scores, key=scores.get)
            if scores[best] < 1: break
            cur.append(best); used.add(best)
        clusters[cid] = cur
        for c in cur: c2c[c] = cid
        cid += 1
    for c in range(ncol):
        if c not in c2c and col_count[c] > 0:
            clusters[cid] = [c]; c2c[c] = cid; cid += 1
    sizes = np.array([len(v) for v in clusters.values()])
    print(f"  Clustering: {cid:,} clusters, size=1:{(sizes==1).sum():,}, "
          f"size>=8:{(sizes>=8).sum():,}, size=32:{(sizes==32).sum():,}")
    return c2c, clusters

def gen_anchor_panels(nrow, ncol, indptr, indices, col_count, PS=16):
    t0 = time.time()
    nra = int(indptr.shape[0]) - 1
    c2r = defaultdict(list)
    for r in range(nra):
        for idx in range(int(indptr[r]), int(indptr[r+1])):
            c2r[int(indices[idx])].append(r)
    sc = sorted(c2r.keys(), key=lambda c: -len(c2r[c]))
    rd = np.diff(indptr).astype(np.int64)
    used = set(); panels = []
    for ac in sc:
        cands = [r for r in c2r[ac] if r not in used]
        if len(cands) < PS: continue
        cands.sort(key=lambda r: rd[r]); sel = cands[:PS]
        ucols = set(); pnnz = 0
        for r in sel:
            for idx in range(int(indptr[r]), int(indptr[r+1])):
                ucols.add(int(indices[idx])); pnnz += 1
        U = len(ucols)
        panels.append({'U': U, 'fill': pnnz/(PS*U) if U else 0,
                        'nnz': pnnz, 'n_tiles': max(1,(U+31)//32)})
        for r in sel: used.add(r)
    print(f"  Anchor: {len(panels):,} panels ({time.time()-t0:.2f}s)")
    return panels

def gen_hybrid_panels(nrow, ncol, indptr, indices, col_count,
                      c2c, clusters, min_cs=8, PS=16):
    t0 = time.time()
    nra = int(indptr.shape[0]) - 1
    rd = np.diff(indptr).astype(np.int64)
    good = {cid for cid, cols in clusters.items() if len(cols) >= min_cs}
    cr = defaultdict(list); leftover = []
    for r in range(nra):
        s, e = int(indptr[r]), int(indptr[r+1])
        if s == e: continue
        votes = defaultdict(int)
        for idx in range(s, e):
            c = int(indices[idx])
            if c in c2c and c2c[c] in good: votes[c2c[c]] += 1
        if votes:
            best = max(votes, key=votes.get)
            if votes[best] >= max(2, (e-s)*0.3):
                cr[best].append((r, votes[best], e-s))
            else: leftover.append(r)
        else: leftover.append(r)

    panels = []; used = set()
    for cid, rlist in cr.items():
        cs = set(clusters[cid]); rlist.sort(key=lambda x:-x[1])
        for si in range(0, len(rlist), PS):
            batch = rlist[si:si+PS]; nr = len(batch)
            if nr < PS: continue
            pcols = set(); pnnz = 0
            for (r,_,_) in batch:
                for idx in range(int(indptr[r]), int(indptr[r+1])):
                    c = int(indices[idx])
                    if c in cs: pcols.add(c); pnnz += 1
            U = len(pcols)
            if U == 0: continue
            panels.append({'U': U, 'fill': pnnz/(nr*U), 'nnz': pnnz,
                           'n_tiles': max(1,(U+31)//32), 'src': 'C'})
            for (r,_,_) in batch: used.add(r)

    aleft = set(leftover) | (set(range(nra)) - used)
    c2r = defaultdict(list)
    for r in aleft:
        for idx in range(int(indptr[r]), int(indptr[r+1])):
            c2r[int(indices[idx])].append(r)
    sc = sorted(c2r.keys(), key=lambda c: -len(c2r[c]))
    ul = set()
    for ac in sc:
        cands = [r for r in c2r[ac] if r not in ul]
        if len(cands) < PS: continue
        cands.sort(key=lambda r: rd[r]); sel = cands[:PS]
        ucols = set(); pnnz = 0
        for r in sel:
            for idx in range(int(indptr[r]), int(indptr[r+1])):
                ucols.add(int(indices[idx])); pnnz += 1
        U = len(ucols)
        if U == 0: continue
        panels.append({'U': U, 'fill': pnnz/(PS*U), 'nnz': pnnz,
                       'n_tiles': max(1,(U+31)//32), 'src': 'A'})
        for r in sel: ul.add(r)

    nc = sum(1 for p in panels if p.get('src')=='C')
    na = sum(1 for p in panels if p.get('src')=='A')
    print(f"  Hybrid: {len(panels):,} = {nc} cluster + {na} anchor ({time.time()-t0:.2f}s)")
    return panels

def perf_metrics(panels, nnz, K, label):
    fills = np.array([p['fill'] for p in panels])
    Us    = np.array([p['U'] for p in panels])
    tiles = np.array([p['n_tiles'] for p in panels])
    nnzs  = np.array([p['nnz'] for p in panels])
    cov   = int(nnzs.sum())
    fb    = nnz - cov
    tt    = int(tiles.sum())
    tg    = int(Us.sum())
    gb    = tg * K * 2

    print(f"\n  [{label}]")
    print(f"    NNZ covered:   {cov:>12,} ({100*cov/nnz:.1f}%)")
    print(f"    Fallback NNZ:  {fb:>12,} ({100*fb/nnz:.1f}%)")
    print(f"    Total tiles:   {tt:>12,}")
    print(f"    Total gathers: {tg:>12,}")
    print(f"    Tile eff:      {cov/tt:>12.1f} NNZ/tile")
    print(f"    Gather eff:    {cov/tg:>12.2f} NNZ/gather")
    print(f"    Avg fill:      {fills.mean()*100:>11.1f}%")
    print(f"    Gather BW:     {gb/1e6:>11.1f} MB (K={K})")
    return {'cov': cov, 'fb': fb, 'tt': tt, 'tg': tg, 'gb': gb,
            'fill': fills.mean(), 'tile_eff': cov/tt, 'gather_eff': cov/tg}

def compare(ca, hyb, nnz, name, K):
    print(f"\n  {'='*62}")
    print(f"  PERFORMANCE COMPARISON: {name} (K={K})")
    print(f"  {'='*62}")

    print(f"\n  {'Metric':<25s} {'Anchor':>12s} {'Hybrid':>12s} {'H/A':>8s} {'Note':>12s}")
    print(f"  {'-'*25} {'-'*12} {'-'*12} {'-'*8} {'-'*12}")
    rows = [
        ("NNZ covered",    ca['cov'],  hyb['cov'],  False, ""),
        ("Fallback NNZ",   ca['fb'],   hyb['fb'],   True,  "fewer=good"),
        ("Total tiles",    ca['tt'],   hyb['tt'],   True,  "fewer=good"),
        ("Total gathers",  ca['tg'],   hyb['tg'],   True,  "fewer=good"),
        ("Tile eff (NNZ/t)",ca['tile_eff'], hyb['tile_eff'], False, "higher=good"),
        ("Gather eff",     ca['gather_eff'], hyb['gather_eff'], False, "higher=good"),
        ("Avg fill",       ca['fill'], hyb['fill'], False, "higher=good"),
        ("Gather BW (MB)", ca['gb']/1e6, hyb['gb']/1e6, True, "fewer=good"),
    ]
    for lbl, cv, hv, lower_better, note in rows:
        r = hv/cv if cv else 0
        w = "✓Hybrid" if (hv<cv)==lower_better and hv!=cv else ("==" if hv==cv else "Anchor")
        if isinstance(cv, float) and abs(cv) < 1000:
            print(f"  {lbl:<25s} {cv:>12.2f} {hv:>12.2f} {r:>7.3f}x {w:>12s}")
        else:
            print(f"  {lbl:<25s} {cv:>12,.0f} {hv:>12,.0f} {r:>7.3f}x {w:>12s}")

    # 性能模型
    tr = hyb['tt']/ca['tt'] if ca['tt'] else 1
    gr = hyb['tg']/ca['tg'] if ca['tg'] else 1
    fr = hyb['fb']/ca['fb']  if ca['fb'] else 1

    print(f"\n  --- Performance Model ---")
    print(f"  Component ratios (Hybrid/Anchor):")
    print(f"    Tiles:    {tr:.3f}x")
    print(f"    Gathers:  {gr:.3f}x")
    print(f"    Fallback: {fr:.3f}x")

    scenarios = [
        ("Gather-dominant (likely)",  0.15, 0.60, 0.25),
        ("Balanced",                  0.33, 0.34, 0.33),
        ("Fallback-heavy",            0.15, 0.30, 0.55),
    ]
    print(f"\n  {'Scenario':<28s} {'w_tile':>7s} {'w_gath':>7s} {'w_fb':>7s} {'T_ratio':>8s} {'Verdict':>12s}")
    for sn, w1, w2, w3 in scenarios:
        t = w1*tr + w2*gr + w3*fr
        v = f"{(1-t)*100:.1f}% faster" if t<1 else f"{(t-1)*100:.1f}% slower"
        print(f"  {sn:<28s} {w1:>7.2f} {w2:>7.2f} {w3:>7.2f} {t:>8.3f} {v:>12s}")

    # K 敏感性
    print(f"\n  --- K Sensitivity (gather BW savings) ---")
    for k in [32, 64, 128]:
        ca_gb  = ca['tg'] * k * 2
        hyb_gb = hyb['tg'] * k * 2
        save = 100*(1-hyb_gb/ca_gb) if ca_gb else 0
        print(f"    K={k:>3d}: anchor={ca_gb/1e6:.1f}MB, hybrid={hyb_gb/1e6:.1f}MB, save={save:.1f}%")

def analyze(path, name, max_row_deg, sample_rate, K=128):
    print(f"\n{'#'*70}")
    print(f"# Performance Analysis: {name} (K={K})")
    print(f"{'#'*70}")
    nrow, ncol, nnz, indptr, indices = read_csrbin(path)
    col_count = np.zeros(ncol, dtype=np.int64)
    for j in range(nnz): col_count[indices[j]] += 1

    print(f"\n--- A^T×A ---")
    col_nb = compute_cooccurrence(nrow, ncol, indptr, indices, col_count,
                                  max_row_deg, sample_rate, 3, 64)

    print(f"\n--- Generate Panels ---")
    ca_p = gen_anchor_panels(nrow, ncol, indptr, indices, col_count)
    c2c, cl = greedy_clustering(ncol, col_count, col_nb, 32)
    hyb_p = gen_hybrid_panels(nrow, ncol, indptr, indices, col_count, c2c, cl, 8)

    print(f"\n--- Metrics ---")
    ca_m = perf_metrics(ca_p, nnz, K, "Column Anchor")
    hyb_m = perf_metrics(hyb_p, nnz, K, "Hybrid")

    compare(ca_m, hyb_m, nnz, name, K)
    print(f"\nDone: {name}\n")

if __name__ == "__main__":
    D = os.path.expanduser("~/data/SpMM_project/data")
    cfgs = {
        'web-Google': (f"{D}/web-Google/web-Google.csrbin", 500, 1.0),
        'amazon0601': (f"{D}/amazon0601/amazon0601.csrbin", 500, 1.0),
    }
    names = sys.argv[1:] if len(sys.argv) > 1 else ['web-Google', 'amazon0601']
    for n in names:
        if n in cfgs:
            p, mrd, sr = cfgs[n]
            analyze(p, n, mrd, sr, K=128)
        else: print(f"Unknown: {n}")
    print(f"\n{'#'*70}\n# ALL DONE\n{'#'*70}")
