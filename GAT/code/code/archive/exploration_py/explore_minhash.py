#!/usr/bin/env python3
"""
Min-Hash row clustering for panel formation.

Core idea:
  Column-anchor uses a single column as similarity proxy.
  Min-Hash computes row-row Jaccard similarity via signatures.
  Rows with similar column sets → small U → less gather.

  If Min-Hash >> Column-Anchor: room for improvement
  If Min-Hash ≈ Column-Anchor: anchor is near-optimal (paper value!)

Method:
  1. Compute k min-hash signatures per row
  2. LSH banding: group rows with identical band signatures
  3. Within each bucket, greedily form panels of 16 rows
  4. Compare U, fill, gather vs column-anchor baseline
"""

import numpy as np
import struct
import time, sys, os
from collections import defaultdict

def read_csrbin(fn):
    with open(fn,'rb') as f:
        struct.unpack('III',f.read(12))
        nr,nc,nz=struct.unpack('QQQ',f.read(24))
        ip=np.frombuffer(f.read(4*(nr+1)),dtype=np.uint32)
        idx=np.frombuffer(f.read(4*nz),dtype=np.uint32)
    print(f"  Read: {nr:,} rows, {nc:,} cols, {nz:,} NNZ")
    return nr,nc,nz,ip,idx

def build_csc(rows,cols,indptr,indices):
    cc=np.zeros(cols,dtype=np.int32)
    for j in indices: cc[j]+=1
    cp=np.zeros(cols+1,dtype=np.int32); cp[1:]=np.cumsum(cc)
    ci=np.empty(len(indices),dtype=np.int32)
    pos=cp[:-1].copy()
    for r in range(rows):
        for i in range(indptr[r],indptr[r+1]):
            c=indices[i]; ci[pos[c]]=r; pos[c]+=1
    return cp,ci

def build_anchor_panels(rows,cols,indptr,indices,csc_ip,csc_idx,h=16):
    rd=np.diff(indptr.astype(np.int64))
    cd=np.diff(csc_ip.astype(np.int64))
    co=np.argsort(-cd); asgn=np.zeros(rows,dtype=bool); panels=[]
    for anc in co:
        if cd[anc]==0: break
        cand=[int(csc_idx[i]) for i in range(csc_ip[anc],csc_ip[anc+1]) if not asgn[csc_idx[i]]]
        if len(cand)<h: continue
        cand.sort(key=lambda r:rd[r]); sel=cand[:h]
        cs=set()
        for r in sel:
            for i in range(indptr[r],indptr[r+1]): cs.add(int(indices[i]))
        for r in sel: asgn[r]=True
        panels.append((list(sel),cs))
    return panels

# ============================================================
# Min-Hash signatures
# ============================================================
def compute_minhash(rows, cols, indptr, indices, num_hashes=64):
    """Compute min-hash signatures for each row."""
    print(f"    Computing {num_hashes} min-hash signatures...")
    sys.stdout.flush()
    t0 = time.time()

    # Generate random hash functions: h(x) = (a*x + b) % p
    p = 2147483647  # large prime
    rng = np.random.RandomState(42)
    a = rng.randint(1, p, size=num_hashes).astype(np.int64)
    b = rng.randint(0, p, size=num_hashes).astype(np.int64)

    sigs = np.full((rows, num_hashes), np.iinfo(np.int64).max, dtype=np.int64)

    for r in range(rows):
        start, end = int(indptr[r]), int(indptr[r+1])
        if start == end:
            continue
        cols_r = indices[start:end].astype(np.int64)
        # For each hash function, compute min hash value over this row's columns
        for hi in range(num_hashes):
            hvals = (a[hi] * cols_r + b[hi]) % p
            sigs[r, hi] = hvals.min()

        if (r+1) % 200000 == 0:
            print(f"      {r+1}/{rows} ({time.time()-t0:.0f}s)")
            sys.stdout.flush()

    print(f"    Signatures done ({time.time()-t0:.1f}s)")
    sys.stdout.flush()
    return sigs

# ============================================================
# LSH banding
# ============================================================
def lsh_banding(sigs, num_bands, rows_per_band):
    """Group rows into buckets using LSH banding."""
    print(f"    LSH banding: {num_bands} bands x {rows_per_band} rows/band")
    sys.stdout.flush()
    t0 = time.time()

    n_rows = sigs.shape[0]
    buckets = defaultdict(list)

    for r in range(n_rows):
        if sigs[r, 0] == np.iinfo(np.int64).max:
            continue  # empty row
        for band_id in range(num_bands):
            start = band_id * rows_per_band
            end = start + rows_per_band
            band_sig = tuple(sigs[r, start:end].tolist())
            key = (band_id, band_sig)
            buckets[key].append(r)

    # Filter buckets with >= 16 rows
    big_buckets = {k: v for k, v in buckets.items() if len(v) >= 16}
    sizes = [len(v) for v in big_buckets.values()]
    print(f"    Total buckets: {len(buckets):,}, big (>=16): {len(big_buckets):,}")
    if sizes:
        print(f"    Big bucket sizes: mean={np.mean(sizes):.0f}, "
              f"max={max(sizes):,}, total rows={sum(sizes):,}")
    print(f"    Banding done ({time.time()-t0:.1f}s)")
    sys.stdout.flush()
    return big_buckets

# ============================================================
# Form panels from LSH buckets
# ============================================================
def form_panels_from_buckets(big_buckets, rows, indptr, indices, h=16):
    """Greedily form panels from LSH buckets."""
    print(f"    Forming panels from {len(big_buckets)} buckets...")
    sys.stdout.flush()
    t0 = time.time()

    rd = np.diff(indptr.astype(np.int64))
    assigned = np.zeros(rows, dtype=bool)
    panels = []

    # Sort buckets by size descending (bigger = more choices)
    sorted_buckets = sorted(big_buckets.values(), key=len, reverse=True)

    for bucket in sorted_buckets:
        unassigned = [r for r in bucket if not assigned[r]]
        if len(unassigned) < h:
            continue

        # Sort by degree ascending (prefer low-degree rows for smaller U)
        unassigned.sort(key=lambda r: rd[r])

        # Greedy: form panels of h rows
        # Simple approach: just take consecutive groups
        for start in range(0, len(unassigned) - h + 1, h):
            sel = unassigned[start:start+h]
            if any(assigned[r] for r in sel):
                continue
            cs = set()
            for r in sel:
                for i in range(indptr[r], indptr[r+1]):
                    cs.add(int(indices[i]))
            for r in sel:
                assigned[r] = True
            panels.append((list(sel), cs))

    # Greedy-overlap: within each bucket, pick 16 most similar rows
    # (more sophisticated but slower - skip for now)

    n_assigned = np.sum(assigned)
    print(f"    MinHash panels: {len(panels)}, rows assigned: {n_assigned:,} "
          f"({n_assigned/rows*100:.1f}%)")
    print(f"    Panel formation done ({time.time()-t0:.1f}s)")
    sys.stdout.flush()
    return panels, assigned

def form_fallback_panels(assigned, rows, cols, indptr, indices, csc_ip, csc_idx, h=16):
    """Standard column-anchor for remaining rows."""
    rd = np.diff(indptr.astype(np.int64))
    cd2 = np.zeros(cols, dtype=np.int32)
    for r in range(rows):
        if assigned[r]: continue
        for i in range(indptr[r], indptr[r+1]):
            cd2[int(indices[i])] += 1
    co2 = np.argsort(-cd2)
    panels = []
    for anc in co2:
        if cd2[anc] == 0: break
        cand = [int(csc_idx[i]) for i in range(csc_ip[anc], csc_ip[anc+1])
                if not assigned[csc_idx[i]]]
        if len(cand) < h: continue
        cand.sort(key=lambda r: rd[r])
        sel = cand[:h]
        cs = set()
        for r in sel:
            for i in range(indptr[r], indptr[r+1]):
                cs.add(int(indices[i]))
            assigned[r] = True
        panels.append((list(sel), cs))
    return panels

# ============================================================
# Metrics
# ============================================================
def metrics(panels, total_nnz, indptr, label):
    if not panels:
        print(f"  [{label}] No panels"); return {}
    n=len(panels); tt=0; tg=0; tc=0; fs=[]; Us=[]; u32=0
    for pr,pc in panels:
        nr=len(pr); U=len(pc); t=(U+31)//32
        pn = sum(int(indptr[r+1]-indptr[r]) for r in pr)
        # But NNZ covered = NNZ that fall within the column set
        # For simplicity, count row NNZ (overcount if cols outside panel, but
        # for column-anchor this is exact since all cols are in the union)
        tt+=t; tg+=U; tc+=pn
        f=pn/(nr*U) if nr*U>0 else 0
        fs.append(f); Us.append(U)
        if U<=32: u32+=1
    af=np.mean(fs)*100; au=np.mean(Us); p32=u32/n*100
    cp=tc/total_nnz*100; fb=total_nnz-tc; gbw=tg*128*2/1e6
    te=tc/tt if tt else 0; ge=tc/tg if tg else 0
    print(f"  [{label}]")
    print(f"    Panels: {n}")
    print(f"    NNZ covered: {tc:,} ({cp:.1f}%), Fallback: {fb:,} ({100-cp:.1f}%)")
    print(f"    Tiles: {tt:,}, Gathers: {tg:,}, GathBW: {gbw:.1f} MB")
    print(f"    Tile eff: {te:.1f}, Gather eff: {ge:.2f}")
    print(f"    Fill: {af:.1f}%, U: {au:.1f}, U<=32: {p32:.1f}%")
    sys.stdout.flush()
    return {'panels':n,'covered':tc,'cov_pct':cp,'fallback':fb,'tiles':tt,
            'gathers':tg,'gather_bw':gbw,'avg_fill':af,'avg_U':au,'u_le32':p32}

def perf(mb,mt,lb,lt):
    if not mb or not mt: return
    rt=mt['tiles']/mb['tiles'] if mb['tiles'] else 1
    rg=mt['gathers']/mb['gathers'] if mb['gathers'] else 1
    rf=mt['fallback']/mb['fallback'] if mb['fallback'] else 1
    print(f"\n  --- Perf: {lt} vs {lb} ---")
    print(f"    Tiles: {rt:.3f}x, Gathers: {rg:.3f}x, Fallback: {rf:.3f}x")
    for nm,wt,wg,wf in [("Gather-dom",0.15,0.60,0.25),("Balanced",0.33,0.34,0.33),("FB-heavy",0.15,0.30,0.55)]:
        T=wt*rt+wg*rg+wf*rf; p=(1-T)*100
        print(f"    {nm:15s} T={T:.3f} -> {p:.1f}% {'faster' if p>0 else 'slower'}")
    sys.stdout.flush()

# ============================================================
# Main
# ============================================================
def process(name, path):
    print(f"\n{'#'*70}")
    print(f"# {name}")
    print(f"{'#'*70}"); sys.stdout.flush()

    rows,cols,nnz,indptr,indices = read_csrbin(path)
    rd = np.diff(indptr.astype(np.int64))
    print(f"  Degree: mean={rd.mean():.1f}, P99={np.percentile(rd,99):.0f}")

    t0=time.time()
    csc_ip,csc_idx = build_csc(rows,cols,indptr,indices)
    print(f"  CSC: {time.time()-t0:.1f}s"); sys.stdout.flush()

    # Baseline
    print(f"\n  === Baseline: Column-Anchor ===")
    t0=time.time()
    anchor_panels = build_anchor_panels(rows,cols,indptr,indices,csc_ip,csc_idx)
    print(f"  {len(anchor_panels)} panels ({time.time()-t0:.1f}s)")
    ma = metrics(anchor_panels, nnz, indptr, "Column-Anchor")

    # Min-Hash with different configurations
    for num_hashes, num_bands in [(64, 16), (128, 16), (64, 32)]:
        rows_per_band = num_hashes // num_bands
        label = f"MinHash(h={num_hashes},b={num_bands},r={rows_per_band})"
        print(f"\n  === {label} ==="); sys.stdout.flush()

        sigs = compute_minhash(rows, cols, indptr, indices, num_hashes=num_hashes)
        big_buckets = lsh_banding(sigs, num_bands, rows_per_band)
        mh_panels, assigned = form_panels_from_buckets(big_buckets, rows, indptr, indices)

        # Fallback: column-anchor for unassigned rows
        fb_panels = form_fallback_panels(assigned, rows, cols, indptr, indices, csc_ip, csc_idx)
        all_panels = mh_panels + fb_panels
        print(f"    Total: {len(mh_panels)} minhash + {len(fb_panels)} fallback = {len(all_panels)}")

        mm = metrics(all_panels, nnz, indptr, label)
        perf(ma, mm, "Anchor", label)

        # Also report minhash-only panels stats
        if mh_panels:
            mh_Us = [len(pc) for _,pc in mh_panels]
            print(f"\n    MinHash-only panel stats:")
            print(f"      Avg U: {np.mean(mh_Us):.1f}, Med U: {np.median(mh_Us):.0f}, "
                  f"U<=32: {np.mean(np.array(mh_Us)<=32)*100:.1f}%")

    print(f"\n  {'='*70}")
    print(f"  DONE: {name}")
    print(f"  {'='*70}"); sys.stdout.flush()

if __name__ == "__main__":
    dd = os.path.expanduser("~/data/SpMM_project/data")
    for nm in ["web-Google","amazon0601","hollywood-2009","soc-Pokec"]:
        p = f"{dd}/{nm}/{nm}.csrbin"
        if os.path.exists(p):
            try: process(nm,p)
            except Exception as e:
                print(f"ERROR {nm}: {e}")
                import traceback; traceback.print_exc()
        else: print(f"SKIP: {nm}")
    print(f"\n{'#'*70}\n# ALL DONE\n{'#'*70}")
