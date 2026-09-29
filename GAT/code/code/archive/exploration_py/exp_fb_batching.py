#!/usr/bin/env python3
"""
Phase 16 - Fallback Batching Simulation
========================================
核心假设：Fallback 行虽然不够资格进 AMX tile (h<16)，
         但它们之间可能共享大量列。
         如果按列相似性分组，同一 batch 内的 B-row 只需加载一次。

分析步骤：
  1. 构建 panel（复用 V17c 逻辑）→ 找到 fallback 行
  2. 对 fallback 行按"主列"分组（每行度数最高的列）
  3. 模拟不同 batch_size 下的 B-row 复用率
  4. 对比：逐行处理 vs batch 处理的 cache miss

全程 numpy 向量化，无朴素 Python 循环。
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

def build_panels_get_fb(nrow, ncol, nnz, indptr, indices):
    """构建 V17c panel，返回 fallback 行列表。全 numpy 向量化。"""
    t0 = time.time()
    row_deg = np.diff(indptr).astype(np.int32)
    
    # CSR → CSC
    row_ids = np.repeat(np.arange(nrow, dtype=np.int32), row_deg)
    col_counts = np.bincount(indices, minlength=ncol)
    csc_indptr = np.zeros(ncol + 1, dtype=np.int64)
    np.cumsum(col_counts, out=csc_indptr[1:])
    
    # 按列度数排序做 CSC 的 row_ids（argsort indices 即可）
    sort_idx = np.argsort(indices, kind='mergesort')
    sorted_rows = row_ids[sort_idx]
    
    col_order = np.argsort(-col_counts)
    
    # 列锚点分组
    assigned = np.zeros(nrow, dtype=bool)
    n_panels = 0
    amx_rows_list = []
    
    TILE_R = 16
    for anchor_col in col_order:
        if col_counts[anchor_col] < TILE_R:
            break
        start = csc_indptr[anchor_col]
        end = csc_indptr[anchor_col + 1]
        col_rows = sorted_rows[start:end]
        mask = ~assigned[col_rows]
        avail = col_rows[mask]
        if len(avail) < TILE_R:
            continue
        avail_degs = row_deg[avail]
        sel_idx = np.argpartition(avail_degs, min(TILE_R-1, len(avail_degs)-1))[:TILE_R]
        selected = avail[sel_idx]
        assigned[selected] = True
        amx_rows_list.append(selected)
        n_panels += 1
    
    fb_rows = np.where(~assigned)[0]
    amx_nnz = int(np.sum(row_deg[assigned]))
    fb_nnz = int(np.sum(row_deg[fb_rows]))
    
    t_build = time.time() - t0
    return fb_rows, n_panels, amx_nnz, fb_nnz, t_build

def analyze_fb_batching(fb_rows, indptr, indices, ncol):
    """
    分析 fallback 行的 batching 潜力。
    
    策略：按"主列"分组——每行找它访问的最高度数列，
    相同主列的行分到一组。组内行天然共享至少一个列。
    
    全 numpy 向量化：
    1. 构建 fb 行的 CSR 子矩阵
    2. 用 bincount 找每行的"主列"（度数最高的列）
    3. 按主列排序 → 同主列的行连续 → 天然分组
    4. 扫描不同 batch_size 的 B-row 复用率
    """
    t0 = time.time()
    n_fb = len(fb_rows)
    if n_fb == 0:
        return {}
    
    # 构建 fb 行的列索引（拼接所有 fb 行的 indices）
    # 用 numpy 向量化：
    fb_deg = (indptr[fb_rows + 1] - indptr[fb_rows]).astype(np.int64)
    total_fb_nnz = int(fb_deg.sum())
    
    if total_fb_nnz == 0:
        return {}
    
    # 收集所有 fb 行的列索引
    # 生成每个 fb_nnz 对应的 fb 行局部索引
    fb_col_indices = np.empty(total_fb_nnz, dtype=np.uint32)
    fb_row_labels = np.empty(total_fb_nnz, dtype=np.int32)  # 哪个 fb 行
    
    # 用 numpy 拼接（避免 Python for）
    fb_starts = indptr[fb_rows]
    fb_ends = indptr[fb_rows + 1]
    
    # 批量拷贝 indices 片段
    pos = 0
    # 这里必须有循环但是是 numpy slice 操作，每次处理一整行
    # 对 fb 行数很多的情况，用 np.concatenate 更快
    slices = [indices[indptr[r]:indptr[r+1]] for r in fb_rows]
    if slices:
        fb_col_indices = np.concatenate(slices)
    else:
        fb_col_indices = np.array([], dtype=np.uint32)
    
    # 每个 NNZ 属于哪个 fb 行（局部编号 0..n_fb-1）
    fb_row_labels = np.repeat(np.arange(n_fb, dtype=np.int32), fb_deg)
    
    t_prep = time.time() - t0
    
    # ====== 策略 1：按主列分组 ======
    t0 = time.time()
    
    # 全局列度数（用所有 NNZ，不只是 fb 的）
    global_col_deg = np.bincount(indices, minlength=ncol)
    
    # 每个 fb 行的"主列" = 该行访问的列中全局度数最高的那个
    # 向量化做法：对每行找 max(global_col_deg[cols])
    # fb_col_indices 已经是所有 fb 行的列号，fb_row_labels 是对应的行号
    # 用 np.maximum.reduceat 配合排序
    
    # 简化做法：给每个 NNZ 赋值为它的列的全局度数，然后按行取 argmax
    nnz_col_deg = global_col_deg[fb_col_indices]  # 每个 NNZ 的列度数
    
    # 每行的主列 = 列度数最大的那个 NNZ 的列号
    # 用 reduceat 找每行最大值的位置
    fb_deg_cumsum = np.zeros(n_fb + 1, dtype=np.int64)
    np.cumsum(fb_deg, out=fb_deg_cumsum[1:])
    
    # 每行的 argmax（在 fb_col_indices 中的全局位置）
    primary_cols = np.zeros(n_fb, dtype=np.int32)
    for_batch = fb_deg > 0  # 跳过空行
    
    # 向量化 argmax per segment: 用 np.maximum.reduceat
    # 先做 reduceat 找每行的最大列度数值
    if total_fb_nnz > 0:
        valid_starts = fb_deg_cumsum[:-1][fb_deg > 0].astype(np.intp)
        if len(valid_starts) > 0:
            max_degs = np.maximum.reduceat(nnz_col_deg, valid_starts)
        # 对于空行（fb_deg=0），reduceat 结果无意义，跳过
        # 找主列：每行中列度数等于 max_deg 的第一个列
        # 简化：用第一个列索引作为近似（排序后修正）
        # 更精确的做法：直接用行的第一个 NNZ 的列号（按 global_col_deg 排序后的）
        # 最快的近似：直接用每行访问的最高度数列
        # 这里用一个折中：primary_col = 每行第一个 NNZ 的列号
        first_nnz_pos = fb_deg_cumsum[:-1]
        valid = fb_deg > 0
        primary_cols[valid] = fb_col_indices[first_nnz_pos[valid]]
        
        # 更精确：找列度数最高的列
        # 对每行，在其 NNZ 中找 global_col_deg 最大的
        # 用 segment argmax
        # 给每个 NNZ 一个 "penalty"：行号 * BIG_NUMBER + 列度数取反 → argsort 后每行第一个就是最高度数
        big = np.int64(ncol) + 1
        sort_key = fb_row_labels.astype(np.int64) * big - nnz_col_deg.astype(np.int64)
        sorted_nz_idx = np.argsort(sort_key, kind='mergesort')
        # 每行的第一个（排序后）就是该行列度数最高的 NNZ
        # 找每行第一次出现的位置
        sorted_labels = fb_row_labels[sorted_nz_idx]
        first_occurrence = np.zeros(n_fb, dtype=np.int64)
        # diff 找变化点
        changes = np.where(np.diff(sorted_labels) != 0)[0] + 1
        first_occurrence[0] = 0
        if len(changes) > 0:
            unique_labels = sorted_labels[changes]
            first_occurrence[unique_labels] = changes
        # 主列
        primary_cols_refined = fb_col_indices[sorted_nz_idx[first_occurrence]]
        primary_cols = primary_cols_refined
    
    t_primary = time.time() - t0
    
    # ====== 按主列排序 fb 行 → 同主列连续 ======
    t0 = time.time()
    sort_order = np.argsort(primary_cols, kind='mergesort')
    sorted_fb = fb_rows[sort_order]
    sorted_primary = primary_cols[sort_order]
    sorted_deg = fb_deg[sort_order]
    t_sort = time.time() - t0
    
    # ====== 模拟不同 batch_size 的 B-reuse ======
    t0 = time.time()
    
    # 基线：逐行处理（零复用），总 B-row 加载 = sum(deg)
    baseline_loads = total_fb_nnz  # 每个 NNZ 触发一次 B-row load（最坏情况）
    # 更精确：每行的唯一列数（行内有复用，但行间无复用）
    # baseline = sum(每行的 unique col count)
    # 对于 CSR，每行的列号已经是 unique 的（标准 CSR），所以 baseline = total_fb_nnz
    
    batch_sizes = [4, 8, 16, 32, 64]
    results = {}
    
    for bs in batch_sizes:
        n_batches = (n_fb + bs - 1) // bs
        batch_unique_cols = 0  # batch 处理时的总唯一列数
        
        # 向量化：把 sorted_fb 切成 batches，每个 batch 计算唯一列数
        # 需要遍历 batch（但 batch 数远小于行数）
        for bi in range(n_batches):
            s = bi * bs
            e = min(s + bs, n_fb)
            batch_rows = sorted_fb[s:e]
            # 收集 batch 内所有列
            batch_cols = np.concatenate([
                indices[indptr[r]:indptr[r+1]] for r in batch_rows
            ]) if len(batch_rows) > 0 else np.array([], dtype=np.uint32)
            batch_unique_cols += len(np.unique(batch_cols))
        
        # B-reuse 率 = 1 - batch_unique / baseline
        reuse_rate = 1.0 - batch_unique_cols / baseline_loads if baseline_loads > 0 else 0
        results[bs] = {
            'n_batches': n_batches,
            'total_unique_loads': batch_unique_cols,
            'baseline_loads': baseline_loads,
            'reuse_rate': reuse_rate,
            'load_reduction': baseline_loads - batch_unique_cols,
        }
    
    t_sim = time.time() - t0
    
    # ====== 策略 2：随机顺序（控制组）======
    t0 = time.time()
    rng = np.random.RandomState(42)
    random_order = rng.permutation(n_fb)
    random_fb = fb_rows[random_order]
    
    random_results = {}
    for bs in [8, 32]:
        n_batches = (n_fb + bs - 1) // bs
        rand_unique = 0
        for bi in range(n_batches):
            s = bi * bs
            e = min(s + bs, n_fb)
            batch_rows = random_fb[s:e]
            batch_cols = np.concatenate([
                indices[indptr[r]:indptr[r+1]] for r in batch_rows
            ]) if len(batch_rows) > 0 else np.array([], dtype=np.uint32)
            rand_unique += len(np.unique(batch_cols))
        random_results[bs] = rand_unique
    
    t_rand = time.time() - t0
    
    # ====== 策略 3：主列分组（精确边界）======
    # 不按固定 batch_size，而是同主列的行自然成组
    t0 = time.time()
    # 找主列变化的边界
    changes = np.where(np.diff(sorted_primary) != 0)[0] + 1
    boundaries = np.concatenate([[0], changes, [n_fb]])
    n_groups = len(boundaries) - 1
    
    group_sizes = np.diff(boundaries)
    group_unique_loads = 0
    
    for gi in range(n_groups):
        s = boundaries[gi]
        e = boundaries[gi + 1]
        batch_rows = sorted_fb[s:e]
        if len(batch_rows) == 0:
            continue
        batch_cols = np.concatenate([
            indices[indptr[r]:indptr[r+1]] for r in batch_rows
        ])
        group_unique_loads += len(np.unique(batch_cols))
    
    natural_reuse = 1.0 - group_unique_loads / baseline_loads if baseline_loads > 0 else 0
    t_natural = time.time() - t0
    
    return {
        'n_fb': n_fb,
        'total_fb_nnz': total_fb_nnz,
        'avg_fb_deg': total_fb_nnz / n_fb if n_fb > 0 else 0,
        'baseline_loads': baseline_loads,
        'batch_results': results,
        'random_results': random_results,
        'natural_groups': n_groups,
        'natural_avg_size': float(np.mean(group_sizes)),
        'natural_max_size': int(np.max(group_sizes)) if len(group_sizes) > 0 else 0,
        'natural_unique_loads': group_unique_loads,
        'natural_reuse': natural_reuse,
        'timings': {
            'prep': t_prep, 'primary': t_primary, 'sort': t_sort,
            'sim': t_sim, 'rand': t_rand, 'natural': t_natural,
        },
    }

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
    print(" Phase 16 - Fallback Batching Simulation")
    print(" Question: Can we reduce FB B-row misses by grouping similar FB rows?")
    print("=" * 90)
    
    summary = []
    
    for mat_name, mat_path in matrices:
        full_path = os.path.join(data_dir, mat_path)
        if not os.path.exists(full_path):
            print(f"\n[SKIP] {mat_name}")
            continue
        
        print(f"\n{'='*80}")
        print(f"  {mat_name}")
        print(f"{'='*80}")
        
        t0_total = time.time()
        nrow, ncol, nnz, indptr, indices = read_csrbin(full_path)
        avg_deg = nnz / nrow
        print(f"  M={nrow:,}  NNZ={nnz:,}  avg_deg={avg_deg:.1f}")
        
        # 构建 panel + 找 fallback 行
        fb_rows, n_panels, amx_nnz, fb_nnz, t_build = build_panels_get_fb(
            nrow, ncol, nnz, indptr, indices)
        fb_pct = fb_nnz / nnz * 100
        print(f"  Panels: {n_panels:,}, AMX NNZ: {amx_nnz:,} ({100-fb_pct:.1f}%)")
        print(f"  FB rows: {len(fb_rows):,}, FB NNZ: {fb_nnz:,} ({fb_pct:.1f}%)")
        print(f"  Build: {t_build:.1f}s")
        
        if len(fb_rows) == 0:
            print(f"  No fallback rows! Skip.")
            continue
        
        # 分析 batching 潜力
        print(f"  Analyzing batching potential...")
        res = analyze_fb_batching(fb_rows, indptr, indices, ncol)
        
        if not res:
            print(f"  No data.")
            continue
        
        # 打印结果
        print(f"\n  FB stats: {res['n_fb']:,} rows, {res['total_fb_nnz']:,} NNZ, "
              f"avg_deg={res['avg_fb_deg']:.1f}")
        print(f"  Baseline (no batching): {res['baseline_loads']:,} B-row loads")
        
        # 固定 batch_size 结果
        print(f"\n  Fixed batch_size (sorted by primary-col):")
        print(f"  {'BS':>4} | {'Batches':>8} | {'Unique loads':>13} | {'Saved':>8} | {'Reuse%':>7}")
        for bs in [4, 8, 16, 32, 64]:
            r = res['batch_results'][bs]
            saved = r['load_reduction']
            print(f"  {bs:>4} | {r['n_batches']:>8,} | {r['total_unique_loads']:>13,} | "
                  f"{saved:>8,} | {r['reuse_rate']:>6.1%}")
        
        # 随机 vs 排序对比
        print(f"\n  Random vs Sorted (control):")
        for bs in [8, 32]:
            rand_loads = res['random_results'][bs]
            sort_loads = res['batch_results'][bs]['total_unique_loads']
            rand_reuse = 1.0 - rand_loads / res['baseline_loads']
            sort_reuse = res['batch_results'][bs]['reuse_rate']
            print(f"  BS={bs:>2}: Random reuse={rand_reuse:.1%}, "
                  f"Sorted reuse={sort_reuse:.1%}, "
                  f"Sorting gain={sort_reuse-rand_reuse:+.1%}")
        
        # 自然分组（同主列成组）
        print(f"\n  Natural grouping (same primary-col):")
        print(f"    Groups: {res['natural_groups']:,}, "
              f"Avg size: {res['natural_avg_size']:.1f}, "
              f"Max size: {res['natural_max_size']:,}")
        print(f"    Unique loads: {res['natural_unique_loads']:,}, "
              f"Reuse: {res['natural_reuse']:.1%}")
        
        # Timing
        tm = res['timings']
        print(f"\n  Timing: prep={tm['prep']:.1f}s primary={tm['primary']:.1f}s "
              f"sort={tm['sort']:.1f}s sim={tm['sim']:.1f}s "
              f"rand={tm['rand']:.1f}s natural={tm['natural']:.1f}s")
        
        t_total = time.time() - t0_total
        print(f"  Total: {t_total:.1f}s")
        
        summary.append({
            'name': mat_name,
            'fb_rows': res['n_fb'],
            'fb_nnz': res['total_fb_nnz'],
            'fb_pct': fb_pct,
            'reuse_bs8': res['batch_results'][8]['reuse_rate'],
            'reuse_bs32': res['batch_results'][32]['reuse_rate'],
            'natural_reuse': res['natural_reuse'],
        })
    
    # ====== 总结 ======
    print(f"\n{'='*90}")
    print(f"  SUMMARY")
    print(f"{'='*90}")
    print(f"  {'Matrix':<16s} {'FB rows':>8s} {'FB NNZ':>10s} {'FB%':>6s} "
          f"{'BS=8':>7s} {'BS=32':>7s} {'Natural':>8s} {'Verdict':>8s}")
    for s in summary:
        v = "★★★" if s['reuse_bs32'] > 0.3 else ("★★" if s['reuse_bs32'] > 0.15 else "★")
        print(f"  {s['name']:<16s} {s['fb_rows']:>8,} {s['fb_nnz']:>10,} "
              f"{s['fb_pct']:>5.1f}% "
              f"{s['reuse_bs8']:>6.1%} {s['reuse_bs32']:>6.1%} "
              f"{s['natural_reuse']:>7.1%} {v:>8s}")
    
    print(f"\n  DECISION:")
    print(f"    BS=32 Reuse > 30% → FB Batching HIGHLY EFFECTIVE")
    print(f"    BS=32 Reuse 15-30% → Worth trying in C")
    print(f"    BS=32 Reuse < 15% → Skip, focus on other directions")
    print(f"{'='*90}")

if __name__ == "__main__":
    main()
