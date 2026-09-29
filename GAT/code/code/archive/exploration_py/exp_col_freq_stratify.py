#!/usr/bin/env python3
"""
Phase 16 - Direction 1: Column-Frequency Stratification Analysis
核心问题（第四位专家修正）：
  分层后 AMX 覆盖的 NNZ 占比能提升多少？
  不只看 fill rate！
"""
import numpy as np
import struct
import time
import os

def read_csrbin(path):
    with open(path, 'rb') as f:
        ptype = struct.unpack('I', f.read(4))[0]
        dtype = struct.unpack('I', f.read(4))[0]
        vtype = struct.unpack('I', f.read(4))[0]
        nrow  = struct.unpack('Q', f.read(8))[0]
        ncol  = struct.unpack('Q', f.read(8))[0]
        nnz   = struct.unpack('Q', f.read(8))[0]
        indptr  = np.fromfile(f, dtype=np.uint32, count=nrow+1)
        indices = np.fromfile(f, dtype=np.uint32, count=nnz)
    return nrow, ncol, nnz, indptr, indices

def build_panels_v17c(nrow, ncol, nnz, indptr, indices, TILE_R=16):
    col_to_rows = {}
    for r in range(nrow):
        for p in range(indptr[r], indptr[r+1]):
            c = indices[p]
            if c not in col_to_rows:
                col_to_rows[c] = []
            col_to_rows[c].append(r)
    col_freq = sorted(col_to_rows.items(), key=lambda x: -len(x[1]))
    assigned = set()
    panels = []
    for anchor_col, rows in col_freq:
        avail = [r for r in rows if r not in assigned]
        if len(avail) < TILE_R:
            continue
        avail_deg = [(r, indptr[r+1]-indptr[r]) for r in avail]
        avail_deg.sort(key=lambda x: x[1])
        selected = [r for r, d in avail_deg[:TILE_R]]
        col_union = set()
        for r in selected:
            for p in range(indptr[r], indptr[r+1]):
                col_union.add(indices[p])
        panels.append({'anchor': anchor_col, 'rows': selected, 'col_union': col_union, 'U': len(col_union)})
        for r in selected:
            assigned.add(r)
    return panels, assigned

def analyze_column_frequency(panels, indptr, indices, TILE_R=16):
    thresholds = [4, 6, 8, 10, 12, 14]
    results = {t: {'n_core_cols': 0, 'n_mid_cols': 0, 'n_sparse_cols': 0,
                    'nnz_core': 0, 'nnz_mid': 0, 'nnz_sparse': 0,
                    'fill_sum_core': 0, 'n_core_tiles': 0, 'panels_analyzed': 0}
               for t in thresholds}
    panel_details = []
    
    for pi, panel in enumerate(panels):
        rows = panel['rows']
        col_freq_map = {}
        col_nnz_map = {}
        panel_total_nnz = 0
        for r in rows:
            for p in range(indptr[r], indptr[r+1]):
                c = indices[p]
                col_freq_map[c] = col_freq_map.get(c, 0) + 1
                col_nnz_map[c] = col_nnz_map.get(c, 0) + 1
                panel_total_nnz += 1
        cols_by_freq = sorted(col_freq_map.items(), key=lambda x: -x[1])
        
        for t in thresholds:
            mid_t = max(t // 2, 2)
            core_cols = [(c, f) for c, f in cols_by_freq if f >= t]
            mid_cols = [(c, f) for c, f in cols_by_freq if mid_t <= f < t]
            sparse_cols = [(c, f) for c, f in cols_by_freq if f < mid_t]
            nnz_core = sum(col_nnz_map[c] for c, f in core_cols)
            nnz_mid = sum(col_nnz_map[c] for c, f in mid_cols)
            nnz_sparse = sum(col_nnz_map[c] for c, f in sparse_cols)
            results[t]['n_core_cols'] += len(core_cols)
            results[t]['n_mid_cols'] += len(mid_cols)
            results[t]['n_sparse_cols'] += len(sparse_cols)
            results[t]['nnz_core'] += nnz_core
            results[t]['nnz_mid'] += nnz_mid
            results[t]['nnz_sparse'] += nnz_sparse
            results[t]['panels_analyzed'] += 1
            n_core = len(core_cols)
            if n_core > 0:
                n_tile_cols = ((n_core + 31) // 32) * 32
                total_slots = TILE_R * n_tile_cols
                fill_rate = nnz_core / total_slots if total_slots > 0 else 0
                results[t]['fill_sum_core'] += fill_rate
                results[t]['n_core_tiles'] += (n_core + 31) // 32
        
        if pi < 10 or pi % 1000 == 0:
            panel_details.append({
                'idx': pi, 'U': len(col_freq_map), 'total_nnz': panel_total_nnz,
                'freq_histogram': {
                    '14-16': len([1 for c, f in cols_by_freq if f >= 14]),
                    '10-13': len([1 for c, f in cols_by_freq if 10 <= f < 14]),
                    '6-9':   len([1 for c, f in cols_by_freq if 6 <= f < 10]),
                    '2-5':   len([1 for c, f in cols_by_freq if 2 <= f < 6]),
                    '1':     len([1 for c, f in cols_by_freq if f == 1]),
                },
            })
    return results, panel_details

def main():
    data_dir = os.path.expanduser("~/data/SpMM_project/data")
    matrices = [
        ("web-Google",      "web-Google/web-Google.csrbin"),
        ("amazon0601",      "amazon0601/amazon0601.csrbin"),
        ("soc-Pokec",       "soc-Pokec/soc-Pokec.csrbin"),
        ("cit-Patents",     "cit-Patents/cit-Patents.csrbin"),
        ("as-Skitter",      "as-Skitter/as-Skitter.csrbin"),
        ("hollywood-2009",  "hollywood-2009/hollywood-2009.csrbin"),
        ("indochina-2004",  "indochina-2004/indochina-2004.csrbin"),
    ]
    
    print("=" * 100)
    print("Phase 16 - Direction 1: Column-Frequency Stratification Analysis")
    print("Expert 4 corrected metric: AMX NNZ coverage, NOT just fill rate")
    print("=" * 100)
    
    for mat_name, mat_path in matrices:
        full_path = os.path.join(data_dir, mat_path)
        if not os.path.exists(full_path):
            print(f"[SKIP] {mat_name}: not found")
            continue
        
        print(f"\n{'='*80}")
        print(f"  Matrix: {mat_name}")
        print(f"{'='*80}")
        
        t0 = time.time()
        nrow, ncol, nnz, indptr, indices = read_csrbin(full_path)
        avg_deg = nnz / nrow
        print(f"  M={nrow:,} N={ncol:,} NNZ={nnz:,} avg_deg={avg_deg:.1f} (read: {time.time()-t0:.1f}s)")
        
        # === h=16 ===
        print(f"\n  --- Panel Height = 16 ---")
        t0 = time.time()
        panels_16, assigned_16 = build_panels_v17c(nrow, ncol, nnz, indptr, indices, 16)
        amx_nnz_16 = sum(indptr[r+1]-indptr[r] for r in assigned_16)
        amx_cov_16 = amx_nnz_16 / nnz * 100
        print(f"  Panels: {len(panels_16):,}, AMX coverage: {amx_cov_16:.1f}% ({time.time()-t0:.1f}s)")
        
        t0 = time.time()
        results_16, details_16 = analyze_column_frequency(panels_16, indptr, indices, 16)
        print(f"  Analysis: {time.time()-t0:.1f}s")
        
        # 频率直方图采样
        print(f"\n  Col Freq Distribution (sample panels):")
        print(f"  {'Panel':>6} {'U':>6} {'NNZ':>8} {'f>=14':>6} {'10-13':>6} {'6-9':>6} {'2-5':>6} {'f=1':>6}")
        for d in details_16[:5]:
            h = d['freq_histogram']
            print(f"  {d['idx']:>6} {d['U']:>6} {d['total_nnz']:>8} "
                  f"{h['14-16']:>6} {h['10-13']:>6} {h['6-9']:>6} {h['2-5']:>6} {h['1']:>6}")
        
        # 分层结果
        print(f"\n  Stratification Results:")
        print(f"  {'Thr':>4} | {'AvgCoreCols':>11} {'AvgMidCols':>11} {'AvgSparse':>10} | "
              f"{'Core%':>7} {'Mid%':>7} {'Spar%':>7} | {'C+M%':>7} {'AvgFill':>8}")
        for t in [4, 6, 8, 10, 12, 14]:
            r = results_16[t]
            tot = r['nnz_core']+r['nnz_mid']+r['nnz_sparse']
            if tot == 0: continue
            np_ = r['panels_analyzed']
            print(f"  {t:>4} | {r['n_core_cols']/np_:>11.1f} {r['n_mid_cols']/np_:>11.1f} "
                  f"{r['n_sparse_cols']/np_:>10.1f} | "
                  f"{r['nnz_core']/tot*100:>6.1f}% {r['nnz_mid']/tot*100:>6.1f}% "
                  f"{r['nnz_sparse']/tot*100:>6.1f}% | "
                  f"{(r['nnz_core']+r['nnz_mid'])/tot*100:>6.1f}% "
                  f"{r['fill_sum_core']/np_:>8.3f}")
        
        # 第四位专家核心问题
        print(f"\n  === Expert 4 Key Q: Can stratification boost AMX NNZ coverage? ===")
        print(f"  Current V17c AMX NNZ coverage: {amx_cov_16:.1f}%")
        print(f"  {'Thr':>4} | {'CoreNNZ% (of panel)':>20} | {'Core+Mid% (of panel)':>22} | {'Verdict':>10}")
        for t in [4, 6, 8, 10, 12, 14]:
            r = results_16[t]
            tot = r['nnz_core']+r['nnz_mid']+r['nnz_sparse']
            if tot == 0: continue
            pc = r['nnz_core']/tot*100
            pcm = (r['nnz_core']+r['nnz_mid'])/tot*100
            verdict = "GOOD" if pcm > 70 else ("OK" if pcm > 50 else "WEAK")
            print(f"  {t:>4} | {pc:>19.1f}% | {pcm:>21.1f}% | {verdict:>10}")
        
        # === h=32（第四位专家方向A）===
        if nrow < 5000000:
            print(f"\n  --- Panel Height = 32 (Expert 4 Direction A) ---")
            t0 = time.time()
            panels_32, assigned_32 = build_panels_v17c(nrow, ncol, nnz, indptr, indices, 32)
            amx_nnz_32 = sum(indptr[r+1]-indptr[r] for r in assigned_32)
            amx_cov_32 = amx_nnz_32 / nnz * 100
            print(f"  Panels: {len(panels_32):,}, AMX coverage: {amx_cov_32:.1f}% "
                  f"(vs H=16: {'+' if amx_cov_32>=amx_cov_16 else ''}{amx_cov_32-amx_cov_16:.1f}%)")
            print(f"  Build: {time.time()-t0:.1f}s")
            
            results_32, _ = analyze_column_frequency(panels_32, indptr, indices, 32)
            r = results_32[8]
            tot = r['nnz_core']+r['nnz_mid']+r['nnz_sparse']
            if tot > 0:
                np_ = r['panels_analyzed']
                print(f"  H=32 t=8: CoreCols/panel={r['n_core_cols']/np_:.1f}, "
                      f"Core%={r['nnz_core']/tot*100:.1f}%, "
                      f"C+M%={(r['nnz_core']+r['nnz_mid'])/tot*100:.1f}%, "
                      f"Fill={r['fill_sum_core']/np_:.3f}")
        else:
            print(f"\n  [SKIP] H=32 for {mat_name} (too large)")
    
    print("\n" + "=" * 80)
    print("DECISION CRITERIA (Expert 4):")
    print("  Core+Mid NNZ% >> current coverage → Direction 1 WORTH IT")
    print("  Core+Mid NNZ% ≈ current coverage → SKIP, focus on Direction 2")
    print("=" * 80)

if __name__ == "__main__":
    main()
