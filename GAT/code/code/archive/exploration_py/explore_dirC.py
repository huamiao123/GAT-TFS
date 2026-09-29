#!/usr/bin/env python3
"""Direction C: Pair-anchor (2 anchor columns, rows must be in both)"""
import numpy as np, struct, time, sys, os

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

def build_baseline(rows,cols,indptr,indices,csc_ip,csc_idx,h=16):
    rd=np.diff(indptr.astype(np.int64))
    cd=np.diff(csc_ip.astype(np.int64))
    co=np.argsort(-cd); asgn=np.zeros(rows,dtype=bool); panels=[]
    for anc in co:
        if cd[anc]==0: break
        cand=[csc_idx[i] for i in range(csc_ip[anc],csc_ip[anc+1]) if not asgn[csc_idx[i]]]
        if len(cand)<h: continue
        cand.sort(key=lambda r:rd[r]); sel=cand[:h]
        cs=set(); tn=0
        for r in sel:
            for i in range(indptr[r],indptr[r+1]): cs.add(int(indices[i]))
            tn+=int(indptr[r+1]-indptr[r])
        for r in sel: asgn[r]=True
        panels.append((list(sel),cs,tn))
    return panels

def build_pair_panels(rows,cols,indptr,indices,csc_ip,csc_idx,h=16):
    rd=np.diff(indptr.astype(np.int64))
    cd=np.diff(csc_ip.astype(np.int64))

    # Top columns
    top_k=min(3000,cols)
    top_cols=[int(c) for c in np.argsort(-cd)[:top_k] if cd[c]>=h]
    print(f"    Top columns: {len(top_cols)}")

    # Precompute row sets
    col_rows={}
    for c in top_cols:
        col_rows[c]=set(int(csc_idx[i]) for i in range(csc_ip[c],csc_ip[c+1]))

    # Find best pairs by intersection size
    print(f"    Finding pair candidates..."); sys.stdout.flush()
    asgn=np.zeros(rows,dtype=bool)
    pairs=[]
    for i,c1 in enumerate(top_cols[:2000]):
        r1=col_rows[c1]
        for c2 in top_cols[i+1:i+500]:  # check nearby columns
            r2=col_rows[c2]
            inter=r1&r2
            unassigned_inter=len([r for r in inter if not asgn[r]])
            if unassigned_inter>=h:
                pairs.append((c1,c2,unassigned_inter))
        if (i+1)%500==0:
            print(f"    ... checked {i+1} seed columns, {len(pairs)} pairs found"); sys.stdout.flush()

    pairs.sort(key=lambda x:-x[2])
    print(f"    Total pairs: {len(pairs)}"); sys.stdout.flush()

    panels=[]
    for c1,c2,_ in pairs:
        common=[r for r in col_rows[c1]&col_rows[c2] if not asgn[r]]
        if len(common)<h: continue
        common.sort(key=lambda r:rd[r])
        sel=common[:h]
        cs=set(); tn=0
        for r in sel:
            for i in range(indptr[r],indptr[r+1]): cs.add(int(indices[i]))
            tn+=int(indptr[r+1]-indptr[r])
        for r in sel: asgn[r]=True
        panels.append((list(sel),cs,tn))
        if len(panels)%5000==0:
            print(f"    ... {len(panels)} pair-panels"); sys.stdout.flush()

    n_pair=len(panels)
    print(f"    Pair-anchor formed: {n_pair} panels, {np.sum(asgn)} rows")

    # Fallback: standard anchor for remaining
    cd2=np.zeros(cols,dtype=np.int32)
    for r in range(rows):
        if asgn[r]: continue
        for i in range(indptr[r],indptr[r+1]): cd2[int(indices[i])]+=1
    co2=np.argsort(-cd2)
    for anc in co2:
        if cd2[anc]==0: break
        cand=[int(csc_idx[i]) for i in range(csc_ip[anc],csc_ip[anc+1]) if not asgn[csc_idx[i]]]
        if len(cand)<h: continue
        cand.sort(key=lambda r:rd[r]); sel=cand[:h]
        cs=set(); tn=0
        for r in sel:
            for i in range(indptr[r],indptr[r+1]): cs.add(int(indices[i]))
            tn+=int(indptr[r+1]-indptr[r])
        for r in sel: asgn[r]=True
        panels.append((list(sel),cs,tn))

    n_fb=len(panels)-n_pair
    print(f"    Fallback anchor: {n_fb} panels, total: {len(panels)}")
    return panels

def metrics(panels,total_nnz,label):
    if not panels: print(f"  [{label}] No panels"); return {}
    n=len(panels);tt=0;tg=0;tc=0;fs=[];Us=[];u32=0
    for pr,pc,pn in panels:
        nr=len(pr);U=len(pc);t=(U+31)//32
        tt+=t;tg+=U;tc+=pn;f=pn/(nr*U) if nr*U>0 else 0
        fs.append(f);Us.append(U)
        if U<=32:u32+=1
    af=np.mean(fs)*100;au=np.mean(Us);p32=u32/n*100
    cp=tc/total_nnz*100;fb=total_nnz-tc;gbw=tg*128*2/1e6
    te=tc/tt if tt else 0;ge=tc/tg if tg else 0
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
        T=wt*rt+wg*rg+wf*rf;p=(1-T)*100
        print(f"    {nm:15s} T={T:.3f} -> {p:.1f}% {'faster' if p>0 else 'slower'}")
    sys.stdout.flush()

def process(name,path):
    print(f"\n{'#'*70}\n# {name}\n{'#'*70}"); sys.stdout.flush()
    rows,cols,nnz,indptr,indices=read_csrbin(path)
    rd=np.diff(indptr.astype(np.int64))
    print(f"  Degree: mean={rd.mean():.1f}, P99={np.percentile(rd,99):.0f}")
    t0=time.time()
    csc_ip,csc_idx=build_csc(rows,cols,indptr,indices)
    print(f"  CSC: {time.time()-t0:.1f}s"); sys.stdout.flush()

    print(f"\n  === Baseline h=16 ===")
    t0=time.time()
    p16=build_baseline(rows,cols,indptr,indices,csc_ip,csc_idx)
    print(f"  {len(p16)} panels ({time.time()-t0:.1f}s)")
    m16=metrics(p16,nnz,"Baseline")

    print(f"\n  === Pair-anchor ==="); sys.stdout.flush()
    t0=time.time()
    pp=build_pair_panels(rows,cols,indptr,indices,csc_ip,csc_idx)
    print(f"  {len(pp)} panels ({time.time()-t0:.1f}s)")
    mp=metrics(pp,nnz,"Pair-anchor")
    perf(m16,mp,"Baseline","Pair-anchor")

if __name__=="__main__":
    dd=os.path.expanduser("~/data/SpMM_project/data")
    for nm in ["hollywood-2009","soc-Pokec","web-Google","amazon0601"]:
        p=f"{dd}/{nm}/{nm}.csrbin"
        if os.path.exists(p):
            try: process(nm,p)
            except Exception as e: print(f"ERROR {nm}: {e}"); import traceback; traceback.print_exc()
        else: print(f"SKIP: {nm}")
    print(f"\n# ALL DONE")
