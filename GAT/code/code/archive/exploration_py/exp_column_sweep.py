#!/usr/bin/env python3
"""Column-Sweep I/O Duality Simulation"""
import sys, os, struct, time
import numpy as np
from collections import Counter

K = 128
BF16_SIZE = 2
FP32_SIZE = 4
TILE_R = 16
TILE_C = 32
ROW_B = K * BF16_SIZE
BLOCK_C = TILE_R * K * FP32_SIZE
ELEM_A = 4
L2_SIZE = 2 * 1024 * 1024
L3_NUMA = 9.4 * 1024 * 1024
HOT_PCTS = [0.01, 0.05, 0.1, 0.5, 1.0, 2.0, 5.0, 10.0, 20.0, 50.0]

def load_csrbin(path):
    with open(path, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('III', f.read(12))
        nrow, ncol, nnz = struct.unpack('QQQ', f.read(24))
        indptr = np.fromfile(f, dtype=np.uint32, count=nrow+1)
        indices = np.fromfile(f, dtype=np.uint32, count=nnz)
    return int(nrow), int(ncol), int(nnz), indptr, indices

def build_panels_v17c(nrow, ncol, nnz, indptr, indices):
    row_cols = {}
    for r in range(nrow):
        s, e = int(indptr[r]), int(indptr[r+1])
        if e > s:
            row_cols[r] = set(indices[s:e].tolist())
    col_to_rows = {}
    for r, cols in row_cols.items():
        for c in cols:
            if c not in col_to_rows:
                col_to_rows[c] = []
            col_to_rows[c].append(r)
    col_freq = sorted(col_to_rows.items(), key=lambda x: -len(x[1]))
    assigned = set()
    panels = []
    for anchor_col, rows_with_col in col_freq:
        avail = [r for r in rows_with_col if r not in assigned]
        if len(avail) < TILE_R:
            continue
        avail_deg = [(r, indptr[r+1]-indptr[r]) for r in avail]
        avail_deg.sort(key=lambda x: x[1])
        selected = [r for r, d in avail_deg[:TILE_R]]
        selected.sort()
        col_union = set()
        for r in selected:
            col_union |= row_cols[r]
        panels.append((anchor_col, selected, col_union))
        assigned.update(selected)
    return panels

def col_freq_analysis(panels):
    cf = Counter()
    for _, _, cs in panels:
        for c in cs:
            cf[c] += 1
    return cf

def panel_centric_traffic(panels):
    N = len(panels)
    tU = sum(len(p[2]) for p in panels)
    B = tU * ROW_B
    A = tU * TILE_R * ELEM_A
    C = N * BLOCK_C
    return {'B': B, 'A': A, 'C': C, 'tot': B+A+C, 'tU': tU, 'N': N}

def column_sweep_traffic(panels, cf):
    uc = len(cf)
    sc = sorted(cf.keys(), key=lambda c: -cf[c])
    grps = []
    for i in range(0, len(sc), TILE_C):
        grps.append(set(sc[i:i+TILE_C]))
    c2g = {}
    for gi, g in enumerate(grps):
        for c in g:
            c2g[c] = gi
    tpv = 0
    vcs = []
    for _, _, cs in panels:
        vg = set()
        for c in cs:
            if c in c2g:
                vg.add(c2g[c])
        vcs.append(len(vg))
        tpv += len(vg)
    tU = sum(len(p[2]) for p in panels)
    B = uc * ROW_B
    A = tU * TILE_R * ELEM_A
    C = tpv * BLOCK_C * 2
    return {'B': B, 'A': A, 'C': C, 'tot': B+A+C,
            'uc': uc, 'ng': len(grps), 'tpv': tpv,
            'avg_v': np.mean(vcs), 'max_v': max(vcs), 'min_v': min(vcs)}

def hybrid_traffic(panels, cf, hot_pct):
    N = len(panels)
    uc = len(cf)
    sbf = sorted(cf.items(), key=lambda x: -x[1])
    nh = max(1, int(uc * hot_pct / 100))
    hcols = set(c for c, f in sbf[:nh])
    hfsum = sum(f for c, f in sbf[:nh])
    hl = [c for c, f in sbf[:nh]]
    nhg = (len(hl) + TILE_C - 1) // TILE_C
    hgrps = []
    for i in range(0, len(hl), TILE_C):
        hgrps.append(set(hl[i:i+TILE_C]))
    c2hg = {}
    for gi, g in enumerate(hgrps):
        for c in g:
            c2hg[c] = gi
    Bh = len(hcols) * ROW_B
    Bc = 0
    for _, _, cs in panels:
        Bc += len(cs - hcols) * ROW_B
    Bhpc = hfsum * ROW_B
    hpv = 0
    for _, _, cs in panels:
        vhg = set()
        for c in cs:
            if c in c2hg:
                vhg.add(c2hg[c])
        hpv += len(vhg)
    Chn = hpv * BLOCK_C * 2
    Cc = N * BLOCK_C
    tU = sum(len(p[2]) for p in panels)
    At = tU * TILE_R * ELEM_A
    Bpct = tU * ROW_B
    Bsv = Bhpc - Bh
    Bsvp = Bsv / Bpct * 100 if Bpct > 0 else 0
    naive_t = Bh + Bc + At + Chn + Cc
    tiled_t = Bh + Bc + At + Cc
    return {'hp': hot_pct, 'nh': len(hcols), 'nhg': nhg,
            'hfs': hfsum, 'hpv': hpv,
            'Bh': Bh, 'Bc': Bc, 'Bt': Bh+Bc,
            'Bsv': Bsv, 'Bsvp': Bsvp,
            'Chn': Chn, 'Cc': Cc, 'A': At,
            'nt': naive_t, 'tt': tiled_t,
            'nn': Bsv - Chn, 'nti': Bsv}

def process(name, path):
    print(f"\n{'='*75}", flush=True)
    print(f"  Column-Sweep I/O Duality: {name}", flush=True)
    print(f"{'='*75}", flush=True)
    t0 = time.time()
    nrow, ncol, nnz, indptr, indices = load_csrbin(path)
    print(f"  {nrow:,} rows, {ncol:,} cols, {nnz:,} NNZ", flush=True)
    panels = build_panels_v17c(nrow, ncol, nnz, indptr, indices)
    N = len(panels)
    print(f"  Panels: {N}  Build: {time.time()-t0:.1f}s", flush=True)
    cf = col_freq_analysis(panels)
    uc = len(cf)
    freqs = sorted(cf.values(), reverse=True)
    tref = sum(cf.values())
    print(f"  UniqueCols: {uc:,}  TotalRefs: {tref:,}", flush=True)
    print(f"  ColFreq: max={freqs[0]} p99={freqs[int(len(freqs)*0.01)]} "
          f"p50={freqs[len(freqs)//2]} min={freqs[-1]}", flush=True)
    print(f"\n  --- Column Frequency CDF ---", flush=True)
    for pct in [0.01, 0.05, 0.1, 0.5, 1.0, 2.0, 5.0, 10.0, 20.0, 50.0]:
        n = max(1, int(uc * pct / 100))
        ts = sum(freqs[:n])
        print(f"    Top-{pct:5.2f}% ({n:>7,} cols): "
              f"{ts:>12,} refs = {ts/tref*100:5.1f}%", flush=True)
    pc = panel_centric_traffic(panels)
    print(f"\n  --- Panel-Centric ---", flush=True)
    print(f"    B:{pc['B']/1e9:7.2f}G  A:{pc['A']/1e9:7.2f}G  "
          f"C:{pc['C']/1e9:7.2f}G  TOT:{pc['tot']/1e9:7.2f}G", flush=True)
    cs = column_sweep_traffic(panels, cf)
    print(f"\n  --- Pure Column-Sweep ---", flush=True)
    print(f"    B:{cs['B']/1e9:7.2f}G  A:{cs['A']/1e9:7.2f}G  "
          f"C:{cs['C']/1e9:7.2f}G  TOT:{cs['tot']/1e9:7.2f}G", flush=True)
    print(f"    visits: avg={cs['avg_v']:.1f}x  "
          f"min={cs['min_v']}  max={cs['max_v']}", flush=True)
    print(f"\n  --- I/O DUALITY ---", flush=True)
    print(f"    PC: B repeat {pc['tU']/uc:.1f}x, C write 1x", flush=True)
    print(f"    CS: B write 1x, C repeat {cs['avg_v']:.1f}x", flush=True)
    br = cs['B']/pc['B'] if pc['B']>0 else 0
    cr = cs['C']/pc['C'] if pc['C']>0 else 0
    w = "ColSweep" if cs['tot']<pc['tot'] else "PanelCentric"
    g = abs(cs['tot']-pc['tot'])
    print(f"    B: {pc['B']/1e9:.2f}->{cs['B']/1e9:.2f}G ({br:.4f}x)", flush=True)
    print(f"    C: {pc['C']/1e9:.2f}->{cs['C']/1e9:.2f}G ({cr:.1f}x)", flush=True)
    print(f"    Winner: {w} by {g/1e9:.1f}G ({g/pc['tot']*100:.0f}%)", flush=True)
    pL3 = int(L3_NUMA / BLOCK_C)
    print(f"\n  --- Cache Tiling: L3/NUMA fits {pL3} panels' C ---", flush=True)
    print(f"\n  --- Hybrid: Hot Sweep + Cold Panel ---", flush=True)
    print(f"  {'Hot%':>6s}|{'nHot':>7s}|{'nGrp':>5s}|"
          f"{'B_sv':>7s}|{'B%':>5s}|"
          f"{'C_ovN':>7s}|{'NetN':>8s}|{'NetT':>8s}|"
          f"{'TotT':>7s}|{'vs%':>6s}", flush=True)
    print(f"  {'-'*80}", flush=True)
    bt = None
    bp = 0
    for pct in HOT_PCTS:
        h = hybrid_traffic(panels, cf, pct)
        vp = (1 - h['tt']/pc['tot'])*100
        print(f"  {pct:5.2f}%|{h['nh']:>7,}|{h['nhg']:>5}|"
              f"{h['Bsv']/1e9:6.2f}G|{h['Bsvp']:4.1f}%|"
              f"{h['Chn']/1e9:6.2f}G|{h['nn']/1e9:+7.2f}G|"
              f"{h['nti']/1e9:+7.2f}G|"
              f"{h['tt']/1e9:6.2f}G|{vp:+5.1f}%", flush=True)
        if bt is None or h['tt'] < bt['tt']:
            bt = h
            bp = pct
    print(f"\n  --- Break-Even ---", flush=True)
    nbe = "Never"
    for pct in HOT_PCTS:
        h = hybrid_traffic(panels, cf, pct)
        if h['nn'] > 0:
            nbe = f"hot>={pct}%"
            break
    print(f"    Naive: {nbe}", flush=True)
    print(f"    Tiled: Always positive", flush=True)
    print(f"    Best: hot={bp}%, saves "
          f"{(1-bt['tt']/pc['tot'])*100:.1f}% vs PC", flush=True)
    aU = pc['tU']/N if N>0 else 0
    print(f"\n  === SUMMARY: {name} ===", flush=True)
    print(f"    avgU={aU:.0f} uc={uc:,} "
          f"B_rpt={pc['tU']/uc:.1f}x C_amp={cs['avg_v']:.1f}x", flush=True)
    print(f"    PC: {pc['tot']/1e9:.2f}G  CS: {cs['tot']/1e9:.2f}G "
          f"({'WORSE' if cs['tot']>pc['tot'] else 'BETTER'})", flush=True)
    print(f"    HybridTiled: {bt['tt']/1e9:.2f}G "
          f"(hot={bp}%, {(1-bt['tt']/pc['tot'])*100:+.1f}%)", flush=True)
    thB = uc * ROW_B
    print(f"    B_min={thB/1e9:.2f}G  B_cur={pc['B']/1e9:.2f}G  "
          f"gap={pc['B']/thB:.1f}x", flush=True)

def main():
    if len(sys.argv) < 2:
        print("Usage: python exp_column_sweep.py <csrbin1> ...")
        sys.exit(1)
    print(f"{'='*75}", flush=True)
    print(f"  Column-Sweep I/O Duality | K={K} BF16 TILE={TILE_R}x{TILE_C}", flush=True)
    print(f"{'='*75}", flush=True)
    ta = time.time()
    for p in sys.argv[1:]:
        if not os.path.exists(p):
            print(f"  MISSING: {p}", flush=True)
            continue
        nm = os.path.basename(p).replace('.csrbin','')
        try:
            t0 = time.time()
            process(nm, p)
            print(f"\n  Time: {time.time()-t0:.1f}s\n", flush=True)
        except Exception as ex:
            print(f"  ERROR {nm}: {ex}", flush=True)
            import traceback; traceback.print_exc()
    print(f"\n  Done. Total: {time.time()-ta:.1f}s", flush=True)

if __name__ == '__main__':
    main()
