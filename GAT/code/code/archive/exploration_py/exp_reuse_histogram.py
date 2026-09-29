#!/usr/bin/env python3
"""
Panel Reuse 分布分析
关键问题：多少 panel 的 reuse > 阈值（值得走 AMX）？
"""
import struct, numpy as np, os, time

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

def analyze_reuse(nrow, ncol, nnz, indptr, indices, max_panels=5000):
    TILE_R = 16
    col_deg = np.bincount(indices, minlength=ncol).astype(np.int64)
    col_order = np.argsort(col_deg)[::-1]
    row_degrees = np.diff(indptr).astype(np.int64)
    
    col_deg2, col_ptr, col_indices = build_csc_fast(nrow, ncol, nnz, indptr, indices)
    assigned = np.zeros(nrow, dtype=bool)
    
    reuse_list = []     # NNZ / U per panel
    nnz_list = []       # NNZ per panel
    U_list = []         # U per panel
    nrows_list = []     # actual rows per panel
    
    for ci in range(min(ncol, len(col_order))):
        c = col_order[ci]
        if col_deg[c] == 0: break
        rows_with_c = col_indices[col_ptr[c]:col_ptr[c+1]]
        avail = rows_with_c[~assigned[rows_with_c]]
        if len(avail) < 2: continue
        
        n_select = min(TILE_R, len(avail))
        if len(avail) > TILE_R:
            degs = row_degrees[avail]
            k = TILE_R - 1
            part_idx = np.argpartition(degs, k)[:TILE_R]
            selected = avail[part_idx]
        else:
            selected = avail
        
        panel_cols = set()
        panel_nnz = 0
        for r in selected:
            cols_r = indices[indptr[r]:indptr[r+1]]
            panel_cols.update(cols_r.tolist())
            panel_nnz += len(cols_r)
        
        U = len(panel_cols)
        reuse = panel_nnz / U if U > 0 else 0
        
        reuse_list.append(reuse)
        nnz_list.append(panel_nnz)
        U_list.append(U)
        nrows_list.append(len(selected))
        
        for r in selected: assigned[r] = True
        if len(reuse_list) >= max_panels: break
    
    # FB 行统计
    fb_nnz = int(np.sum(row_degrees[~assigned]))
    fb_rows = int(np.sum(~assigned))
    
    return (np.array(reuse_list), np.array(nnz_list), 
            np.array(U_list), np.array(nrows_list),
            fb_nnz, fb_rows)

if __name__ == '__main__':
    DATA = os.path.expanduser('~/data/SpMM_project/data')
    matrices = ['web-Google', 'amazon0601', 'soc-Pokec', 'as-Skitter',
                'cit-Patents', 'indochina-2004', 'hollywood-2009']
    
    print("=" * 80)
    print("Panel Reuse Distribution Analysis")
    print("=" * 80)
    
    thresholds = [2, 3, 4, 6, 8, 10]
    
    for mname in matrices:
        csrbin = os.path.join(DATA, mname, f'{mname}.csrbin')
        if not os.path.exists(csrbin): continue
        
        nrow, ncol, nnz, indptr, indices = read_csrbin(csrbin)
        avg_deg = nnz / nrow
        print(f"\n{'#'*60}")
        print(f"# {mname} (avg_deg={avg_deg:.1f})")
        print(f"{'#'*60}")
        
        t0 = time.time()
        reuse, pnnz, U, nrows, fb_nnz, fb_rows = analyze_reuse(
            nrow, ncol, nnz, indptr, indices)
        print(f"  Panels: {len(reuse)}, Time: {time.time()-t0:.1f}s")
        print(f"  FB rows: {fb_rows:,}, FB NNZ: {fb_nnz:,} ({fb_nnz/nnz*100:.1f}%)")
        
        # Reuse 分布
        print(f"\n  Reuse distribution:")
        print(f"    mean={reuse.mean():.2f}  median={np.median(reuse):.2f}  "
              f"P25={np.percentile(reuse,25):.2f}  P75={np.percentile(reuse,75):.2f}  "
              f"max={reuse.max():.1f}")
        
        # Histogram
        bins = [(0,1), (1,2), (2,3), (3,4), (4,6), (6,8), (8,12), (12,100)]
        print(f"\n  {'Reuse range':>14} {'Panels':>8} {'%':>7} {'NNZ covered':>12} {'NNZ%':>7}")
        print(f"  {'-'*52}")
        for lo, hi in bins:
            mask = (reuse >= lo) & (reuse < hi)
            cnt = np.sum(mask)
            pct = cnt / len(reuse) * 100
            nnz_cov = np.sum(pnnz[mask])
            nnz_pct = nnz_cov / nnz * 100
            bar = '#' * int(pct / 2)
            print(f"  [{lo:>4},{hi:>4})  {cnt:>8} {pct:>6.1f}% {nnz_cov:>12,} {nnz_pct:>6.1f}% {bar}")
        
        # Threshold analysis: 如果只对 reuse > T 的 panel 用 AMX
        print(f"\n  Threshold analysis (AMX only if reuse > T):")
        print(f"  {'T':>4} {'AMX panels':>11} {'AMX NNZ':>12} {'AMX NNZ%':>9} {'FB NNZ':>12} {'FB NNZ%':>9}")
        print(f"  {'-'*60}")
        for T in thresholds:
            amx_mask = reuse >= T
            amx_panels = np.sum(amx_mask)
            amx_nnz = int(np.sum(pnnz[amx_mask]))
            skip_nnz = int(np.sum(pnnz[~amx_mask]))
            total_fb = fb_nnz + skip_nnz
            print(f"  {T:>4} {amx_panels:>11,} {amx_nnz:>12,} {amx_nnz/nnz*100:>8.1f}% "
                  f"{total_fb:>12,} {total_fb/nnz*100:>8.1f}%")
        
        # 当前 V17c (全部 panel 走 AMX) vs 理想 threshold
        print(f"\n  Current V17c: ALL {len(reuse)} panels → AMX")
        print(f"    AMX NNZ = {int(np.sum(pnnz)):,} ({np.sum(pnnz)/nnz*100:.1f}%)")
        print(f"    Mean reuse = {reuse.mean():.2f}")
        low_reuse = reuse < 3
        print(f"    Panels with reuse < 3: {np.sum(low_reuse)} ({np.mean(low_reuse)*100:.1f}%)")
        print(f"    NNZ in low-reuse panels: {int(np.sum(pnnz[low_reuse])):,} "
              f"({np.sum(pnnz[low_reuse])/nnz*100:.1f}%)")
        print(f"    → These panels HURT performance (AMX overhead > B-reuse saving)")

