#!/usr/bin/env python3
"""
Phase 16 - Direction 1: Column-Frequency Stratification (FAST VERSION)
======================================================================
优化：numpy 向量化 + scipy CSR→CSC，避免纯 Python 循环
预计加速 10-50×
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

def build_panels_fast(nrow, ncol, nnz, indptr, indices, TILE_R=16):
    """
    用 numpy 向量化的列锚点分组。
    关键优化：
      1. CSR→CSC 用 numpy 排序实现（不用 scipy，超算可能没装）
      2. 行度数用 np.diff(indptr) 一行搞定
      3. assigned 用 numpy bool 数组
    """
    t0 = time.time()
    
    # 行度数：numpy 向量化，一行搞定
    # np.diff 计算相邻元素的差值：indptr[1]-indptr[0], indptr[2]-indptr[1], ...
    # 结果就是每行的非零元素个数
    row_deg = np.diff(indptr).astype(np.int32)
    
    # 构建 CSC（列 → 行映射）：用 numpy argsort
    # 思路：把所有 NNZ 按列号排序，就得到了 CSC 的 indices
    # 先给每个 NNZ 元素生成它所在的行号
    row_ids = np.repeat(np.arange(nrow, dtype=np.int32), row_deg)
    # row_ids[i] = 第 i 个 NNZ 元素所在的行号
    # indices[i] = 第 i 个 NNZ 元素的列号
    
    # 按列号排序
    col_sort_idx = np.argsort(indices, kind='mergesort')  # 稳定排序保持行内顺序
    sorted_cols = indices[col_sort_idx]
    sorted_rows = row_ids[col_sort_idx]
    
    # 构建 CSC indptr：每列起始位置
    # np.searchsorted 找每个列号的第一个位置
    col_counts = np.bincount(indices, minlength=ncol)
    csc_indptr = np.zeros(ncol + 1, dtype=np.int64)
    np.cumsum(col_counts, out=csc_indptr[1:])
    
    # 列度数降序排列
    col_order = np.argsort(-col_counts)  # 度数最高的列排前面
    
    t_csc = time.time() - t0
    
    # Panel 构建
    t0 = time.time()
    assigned = np.zeros(nrow, dtype=bool)
    panels = []
    
    for anchor_col in col_order:
        if col_counts[anchor_col] == 0:
            break  # 后面都是空列
        
        # 该列的所有行
        start = csc_indptr[anchor_col]
        end = csc_indptr[anchor_col + 1]
        col_rows = sorted_rows[start:end]
        
        # 未分配的行
        mask = ~assigned[col_rows]
        avail = col_rows[mask]
        
        if len(avail) < TILE_R:
            continue
        
        # 选度数最低的 TILE_R 行
        avail_degs = row_deg[avail]
        low_deg_idx = np.argpartition(avail_degs, min(TILE_R-1, len(avail_degs)-1))[:TILE_R]
        selected = avail[low_deg_idx]
        
        # 计算列并集（用 numpy unique）
        all_cols = []
        for r in selected:
            all_cols.append(indices[indptr[r]:indptr[r+1]])
        col_union = np.unique(np.concatenate(all_cols))
        
        panels.append({
            'anchor': int(anchor_col),
            'rows': selected,  # numpy array
            'U': len(col_union),
        })
        
        assigned[selected] = True
    
    t_build = time.time() - t0
    
    return panels, assigned, t_csc, t_build

def analyze_frequency_fast(panels, indptr, indices, TILE_R=16):
    """
    分析每个 panel 的列频率分布。
    优化：用 numpy bincount 替代 Python dict。
    """
    thresholds = [4, 6, 8, 10, 12, 14]
    
    # 全局累计
    results = {t: {
        'n_core_cols': 0, 'n_mid_cols': 0, 'n_sparse_cols': 0,
        'nnz_core': 0, 'nnz_mid': 0, 'nnz_sparse': 0,
        'fill_sum_core': 0.0, 'n_core_tiles': 0, 'panels_analyzed': 0,
    } for t in thresholds}
    
    # 采样详情
    sample_details = []
    
    ncol_max = int(indices.max()) + 1 if len(indices) > 0 else 0
    
    for pi, panel in enumerate(panels):
        rows = panel['rows']
        
        # 收集该 panel 所有 NNZ 的列号
        all_col_indices = []
        for r in rows:
            all_col_indices.append(indices[indptr[r]:indptr[r+1]])
        all_cols = np.concatenate(all_col_indices)
        panel_nnz = len(all_cols)
        
        # 用 bincount 统计每列出现次数（即频率）
        # bincount: 输入一个整数数组，输出每个整数出现的次数
        col_freq = np.bincount(all_cols, minlength=ncol_max)
        
        # 只看出现过的列
        active_cols = np.where(col_freq > 0)[0]
        active_freq = col_freq[active_cols]
        U = len(active_cols)
        
        for t in thresholds:
            mid_t = max(t // 2, 2)
            
            core_mask = active_freq >= t
            mid_mask = (active_freq >= mid_t) & (active_freq < t)
            sparse_mask = active_freq < mid_t
            
            n_core = core_mask.sum()
            n_mid = mid_mask.sum()
            n_sparse = sparse_mask.sum()
            
            # 每层覆盖的 NNZ = 该层所有列的 freq 之和
            nnz_core = active_freq[core_mask].sum()
            nnz_mid = active_freq[mid_mask].sum()
            nnz_sparse = active_freq[sparse_mask].sum()
            
            results[t]['n_core_cols'] += int(n_core)
            results[t]['n_mid_cols'] += int(n_mid)
            results[t]['n_sparse_cols'] += int(n_sparse)
            results[t]['nnz_core'] += int(nnz_core)
            results[t]['nnz_mid'] += int(nnz_mid)
            results[t]['nnz_sparse'] += int(nnz_sparse)
            results[t]['panels_analyzed'] += 1
            
            if n_core > 0:
                n_tile_cols = ((int(n_core) + 31) // 32) * 32
                total_slots = TILE_R * n_tile_cols
                fill_rate = int(nnz_core) / total_slots if total_slots > 0 else 0
                results[t]['fill_sum_core'] += fill_rate
                results[t]['n_core_tiles'] += (int(n_core) + 31) // 32
        
        # 采样
        if pi < 10 or pi % 2000 == 0:
            # 频率直方图
            sample_details.append({
                'idx': pi, 'U': U, 'nnz': panel_nnz,
                'f14_16': int((active_freq >= 14).sum()),
                'f10_13': int(((active_freq >= 10) & (active_freq < 14)).sum()),
                'f6_9':   int(((active_freq >= 6) & (active_freq < 10)).sum()),
                'f2_5':   int(((active_freq >= 2) & (active_freq < 6)).sum()),
                'f1':     int((active_freq == 1).sum()),
            })
        
        # 进度报告
        if (pi + 1) % 5000 == 0:
            print(f"    ... analyzed {pi+1}/{len(panels)} panels", flush=True)
    
    return results, sample_details

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
    
    print("=" * 90)
    print(" Phase 16 - Column-Frequency Stratification (FAST VERSION)")
    print(" Optimized: numpy vectorization, ~10-50x faster than pure Python")
    print("=" * 90)
    
    for mat_name, mat_path in matrices:
        full_path = os.path.join(data_dir, mat_path)
        if not os.path.exists(full_path):
            print(f"\n[SKIP] {mat_name}: not found")
            continue
        
        print(f"\n{'='*80}")
        print(f"  {mat_name}")
        print(f"{'='*80}")
        
        t0 = time.time()
        nrow, ncol, nnz, indptr, indices = read_csrbin(full_path)
        avg_deg = nnz / nrow
        print(f"  M={nrow:,}  NNZ={nnz:,}  avg_deg={avg_deg:.1f}  (read {time.time()-t0:.1f}s)")
        
        # ====== H=16 ======
        panels, assigned, t_csc, t_build = build_panels_fast(nrow, ncol, nnz, indptr, indices, 16)
        amx_nnz = sum(int(indptr[r+1]-indptr[r]) for r in np.where(assigned)[0])
        amx_cov = amx_nnz / nnz * 100
        fb_nnz = nnz - amx_nnz
        print(f"  H=16: {len(panels):,} panels, AMX cov={amx_cov:.1f}%, "
              f"FB NNZ={fb_nnz:,} ({100-amx_cov:.1f}%)")
        print(f"  Timing: CSC={t_csc:.1f}s, Build={t_build:.1f}s")
        
        t0 = time.time()
        results, samples = analyze_frequency_fast(panels, indptr, indices, 16)
        print(f"  Freq analysis: {time.time()-t0:.1f}s")
        
        # 采样频率直方图
        print(f"\n  Sample panels (col freq distribution):")
        print(f"  {'#':>5} {'U':>6} {'NNZ':>8} {'f≥14':>5} {'10-13':>5} "
              f"{'6-9':>5} {'2-5':>5} {'f=1':>5}")
        for s in samples[:5]:
            print(f"  {s['idx']:>5} {s['U']:>6} {s['nnz']:>8} "
                  f"{s['f14_16']:>5} {s['f10_13']:>5} {s['f6_9']:>5} "
                  f"{s['f2_5']:>5} {s['f1']:>5}")
        
        # 分层结果表
        print(f"\n  Stratification results (H=16):")
        print(f"  {'t':>3} | {'CoreC':>6} {'MidC':>6} {'SparC':>6} | "
              f"{'Core%':>6} {'Mid%':>6} {'Spar%':>6} | {'C+M%':>6} {'Fill':>6}")
        for t in [4, 6, 8, 10, 12, 14]:
            r = results[t]
            tot = r['nnz_core'] + r['nnz_mid'] + r['nnz_sparse']
            if tot == 0: continue
            np_ = max(r['panels_analyzed'], 1)
            pc = r['nnz_core']/tot*100
            pm = r['nnz_mid']/tot*100
            ps = r['nnz_sparse']/tot*100
            pcm = pc + pm
            fl = r['fill_sum_core']/np_
            print(f"  {t:>3} | {r['n_core_cols']/np_:>6.1f} {r['n_mid_cols']/np_:>6.1f} "
                  f"{r['n_sparse_cols']/np_:>6.1f} | "
                  f"{pc:>5.1f}% {pm:>5.1f}% {ps:>5.1f}% | {pcm:>5.1f}% {fl:>6.3f}")
        
        # ★ 第四位专家核心问题 ★
        print(f"\n  ★ Expert 4 Key Question: AMX NNZ coverage improvement potential ★")
        print(f"  Current V17c AMX NNZ coverage: {amx_cov:.1f}%")
        print(f"  {'t':>3} | {'Core%':>7} {'C+M%':>7} | {'Verdict':>8}")
        for t in [4, 6, 8, 10, 12]:
            r = results[t]
            tot = r['nnz_core'] + r['nnz_mid'] + r['nnz_sparse']
            if tot == 0: continue
            pc = r['nnz_core']/tot*100
            pcm = (r['nnz_core']+r['nnz_mid'])/tot*100
            v = "GREAT" if pcm > 80 else ("GOOD" if pcm > 60 else ("OK" if pcm > 40 else "WEAK"))
            print(f"  {t:>3} | {pc:>6.1f}% {pcm:>6.1f}% | {v:>8}")
        
        # ====== H=32（第四位专家方向 A）======
        if nrow < 5000000:
            panels32, assigned32, t_csc32, t_build32 = build_panels_fast(
                nrow, ncol, nnz, indptr, indices, 32)
            amx32 = sum(int(indptr[r+1]-indptr[r]) for r in np.where(assigned32)[0])
            cov32 = amx32 / nnz * 100
            print(f"\n  H=32: {len(panels32):,} panels, AMX cov={cov32:.1f}% "
                  f"(vs H=16: {'+' if cov32>=amx_cov else ''}{cov32-amx_cov:.1f}%)")
            
            res32, _ = analyze_frequency_fast(panels32, indptr, indices, 32)
            r = res32[8]
            tot = r['nnz_core']+r['nnz_mid']+r['nnz_sparse']
            if tot > 0:
                np_ = max(r['panels_analyzed'], 1)
                pcm = (r['nnz_core']+r['nnz_mid'])/tot*100
                print(f"  H=32 t=8: Core+Mid={pcm:.1f}%, "
                      f"CoreCols/panel={r['n_core_cols']/np_:.1f}, "
                      f"Fill={r['fill_sum_core']/np_:.3f}")
        else:
            print(f"\n  [SKIP H=32: matrix too large]")
    
    print(f"\n{'='*80}")
    print("DECISION (Expert 4 criterion):")
    print("  Core+Mid% >> current → Direction 1 WORTH IT (pursue C implementation)")
    print("  Core+Mid% ≈  current → SKIP Direction 1 (focus on GreedyInterBkt)")
    print(f"{'='*80}")

if __name__ == "__main__":
    main()
