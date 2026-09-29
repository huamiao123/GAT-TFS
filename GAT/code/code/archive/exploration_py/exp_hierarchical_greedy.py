#!/usr/bin/env python3
"""
exp_hierarchical_greedy.py  —  Phase 14.1 (v2, panel构建与unified对齐)
"""

import os, sys, time, struct
import numpy as np
from collections import OrderedDict, defaultdict

# ───────────────────────────────────────────────
# 0. 配置
# ───────────────────────────────────────────────
BASE = "/home/huangjianqiang_group/hdacp1/data/SpMM_project/data"
MATRICES = [
    ("web-Google",     f"{BASE}/web-Google/web-Google.csrbin"),
    ("amazon0601",     f"{BASE}/amazon0601/amazon0601.csrbin"),
    ("soc-Pokec",      f"{BASE}/soc-Pokec/soc-Pokec.csrbin"),
    ("hollywood-2009", f"{BASE}/hollywood-2009/hollywood-2009.csrbin"),
    ("cit-Patents",    f"{BASE}/cit-Patents/cit-Patents.csrbin"),
    ("as-Skitter",     f"{BASE}/as-Skitter/as-Skitter.csrbin"),
    ("indochina-2004", f"{BASE}/indochina-2004/indochina-2004.csrbin"),
]

TILE_R             = 16
L2_CACHE           = 2000
MINHASH_H          = 64
K_VALUES           = [200, 500, 1000, 2000]
GREEDY_MAX_BUCKETS = 50000
LOCAL_GREEDY_MAX   = 300
MERSENNE           = (1 << 31) - 1

# ───────────────────────────────────────────────
# 1. CSR 读取
# ───────────────────────────────────────────────
def load_csr(path):
    with open(path, "rb") as f:
        ptype, dtype, vtype = struct.unpack("III", f.read(12))
        nrow, ncol, nnz     = struct.unpack("QQQ", f.read(24))
        indptr  = np.frombuffer(f.read((nrow + 1) * 4), dtype=np.uint32)
        indices = np.frombuffer(f.read(nnz * 4),        dtype=np.uint32)
    return int(nrow), int(ncol), int(nnz), indptr, indices

# ───────────────────────────────────────────────
# 2. Panel 构建（与 unified 完全一致）
#    关键：每个锚点只取度数最低的16行，只建1个panel
# ───────────────────────────────────────────────
def build_panels_v17c(nrow, ncol, nnz, indptr, indices):
    col_to_rows = defaultdict(list)
    for r in range(nrow):
        for idx in range(indptr[r], indptr[r+1]):
            col_to_rows[indices[idx]].append(r)

    col_freq = sorted(col_to_rows.items(), key=lambda x: -len(x[1]))

    assigned = set()
    panels   = []
    bkt_panels = defaultdict(list)

    # 预计算每行的列集合
    row_cols = {}
    for r in range(nrow):
        row_cols[r] = set(indices[indptr[r]:indptr[r+1]].tolist())

    for anchor_col, rows_list in col_freq:
        if len(rows_list) < TILE_R:
            continue
        avail = [r for r in rows_list if r not in assigned]
        if len(avail) < TILE_R:
            continue
        # 按度数升序，只取最低的16行，只建1个panel（与unified一致）
        avail_deg = [(r, indptr[r+1]-indptr[r]) for r in avail]
        avail_deg.sort(key=lambda x: x[1])
        selected = [r for r, d in avail_deg[:TILE_R]]

        col_union = set()
        for r in selected:
            col_union.update(row_cols[r])

        pi = len(panels)
        panels.append({'anchor': anchor_col, 'rows': selected, 'cols': col_union})
        bkt_panels[anchor_col].append(pi)
        for r in selected:
            assigned.add(r)

    return panels, dict(bkt_panels)

# ───────────────────────────────────────────────
# 3. LRU 模拟（O(1) OrderedDict）
# ───────────────────────────────────────────────
def simulate_lru(panels, order, cache_size=L2_CACHE):
    cache  = OrderedDict()
    misses = 0
    total  = 0
    for pi in order:
        for c in panels[pi]['cols']:
            total += 1
            if c in cache:
                cache.move_to_end(c)
            else:
                misses += 1
                if len(cache) >= cache_size:
                    cache.popitem(last=False)
                cache[c] = 1
    save = (1.0 - misses / total) * 100.0 if total > 0 else 0.0
    return total, misses, save

# ───────────────────────────────────────────────
# 4. AIR
# ───────────────────────────────────────────────
def compute_air(panels, order):
    last_seen = {}
    gaps = []
    for rank, pi in enumerate(order):
        for c in panels[pi]['cols']:
            if c in last_seen:
                gaps.append(rank - last_seen[c])
            last_seen[c] = rank
    return float(np.mean(gaps)) if gaps else 0.0

# ───────────────────────────────────────────────
# 5. MinHash
# ───────────────────────────────────────────────
def make_minhash_funcs(h=MINHASH_H, seed=42):
    rng = np.random.RandomState(seed)
    a = rng.randint(1, MERSENNE, size=h).astype(np.int64)
    b = rng.randint(0, MERSENNE, size=h).astype(np.int64)
    return a, b

def minhash_signature(col_set, a, b):
    h = len(a)
    if not col_set:
        return np.zeros(h, dtype=np.int64)
    cols = np.array(list(col_set), dtype=np.int64)
    sig  = np.full(h, np.iinfo(np.int64).max, dtype=np.int64)
    for j in range(h):
        hvals  = (a[j] * cols + b[j]) % MERSENNE
        sig[j] = int(hvals.min())
    return sig

# ───────────────────────────────────────────────
# 6. 排序策略
# ───────────────────────────────────────────────

def reorder_original(panels, bkt_panels):
    return list(range(len(panels)))

def reorder_minhash(panels, bkt_panels, h=MINHASH_H):
    a, b = make_minhash_funcs(h)
    bkt_sig0 = {}
    for anchor, pidxs in bkt_panels.items():
        col_union = set()
        for pi in pidxs:
            col_union.update(panels[pi]['cols'])
        sig = minhash_signature(col_union, a, b)
        bkt_sig0[anchor] = int(sig[0])
    sorted_anchors = sorted(bkt_panels.keys(), key=lambda a: bkt_sig0[a])
    order = []
    for anchor in sorted_anchors:
        order.extend(bkt_panels[anchor])
    return order

def _greedy_on_list(col_sets):
    """最近邻贪心：每次选交集最大的下一个，O(N²)"""
    N = len(col_sets)
    if N == 0: return []
    if N == 1: return [0]
    visited = [False] * N
    start = max(range(N), key=lambda i: len(col_sets[i]))
    order = [start]; visited[start] = True
    current = set(col_sets[start])
    for _ in range(N - 1):
        best_j, best_score = -1, -1
        for j in range(N):
            if not visited[j]:
                sc = len(current & col_sets[j])
                if sc > best_score:
                    best_score, best_j = sc, j
        if best_j == -1:
            best_j = next(j for j in range(N) if not visited[j])
        order.append(best_j); visited[best_j] = True
        current.update(col_sets[best_j])
    return order

def reorder_greedy_inter_bkt(panels, bkt_panels):
    bkt_list = list(bkt_panels.items())
    col_sets = []
    for anchor, pidxs in bkt_list:
        cu = set()
        for pi in pidxs:
            cu.update(panels[pi]['cols'])
        col_sets.append(cu)
    local_ord = _greedy_on_list(col_sets)
    result = []
    for i in local_ord:
        result.extend(bkt_list[i][1])
    return result

def reorder_hierarchical_greedy(panels, bkt_panels, K=1000, intra='original', h=MINHASH_H):
    bkt_list = list(bkt_panels.items())
    B = len(bkt_list)

    if B <= K:
        print(f"    [HG K={K}] B={B}<=K，退化为精确 GreedyInterBkt", flush=True)
        return reorder_greedy_inter_bkt(panels, bkt_panels)

    # Step 1: 列并集 + MinHash 签名
    t0 = time.time()
    bkt_col_sets = []
    for anchor, pidxs in bkt_list:
        cu = set()
        for pi in pidxs:
            cu.update(panels[pi]['cols'])
        bkt_col_sets.append(cu)
    a_arr, b_arr = make_minhash_funcs(h)
    sigs = np.array([minhash_signature(cs, a_arr, b_arr) for cs in bkt_col_sets])
    t1 = time.time()

    # Step 2: 按 sig[0] 等分为 K 个超级桶
    sorted_idx  = np.argsort(sigs[:, 0])
    sb_groups   = np.array_split(sorted_idx, K)
    sb_col_sets = []
    for grp in sb_groups:
        sbc = set()
        for bi in grp:
            sbc.update(bkt_col_sets[bi])
        sb_col_sets.append(sbc)
    t2 = time.time()

    # Step 3: 超级桶间精确 Greedy（O(K²)）
    sb_order_idx = _greedy_on_list(sb_col_sets)
    t3 = time.time()

    # Step 4: 重建 panel 顺序
    result = []
    for sb_idx in sb_order_idx:
        grp = sb_groups[sb_idx]
        if intra == 'greedy' and len(grp) <= LOCAL_GREEDY_MAX:
            local_col_sets = [bkt_col_sets[bi] for bi in grp]
            local_ord      = _greedy_on_list(local_col_sets)
            for lo in local_ord:
                result.extend(bkt_list[grp[lo]][1])
        else:
            for bi in grp:
                result.extend(bkt_list[bi][1])
    t4 = time.time()

    print(f"    [HG K={K} intra={intra}] "
          f"col_union+minhash={t1-t0:.1f}s  sb_form={t2-t1:.1f}s  "
          f"sb_greedy={t3-t2:.1f}s  rebuild={t4-t3:.1f}s  total={t4-t0:.1f}s", flush=True)
    return result

# ───────────────────────────────────────────────
# 7. 矩阵统计
# ───────────────────────────────────────────────
def compute_matrix_stats(panels, bkt_panels):
    total_g  = sum(len(p['cols']) for p in panels)
    uniq_c   = len(set(c for p in panels for c in p['cols']))
    col_freq = defaultdict(int)
    for p in panels:
        for c in p['cols']:
            col_freq[c] += 1
    freqs = np.array(sorted(col_freq.values(), reverse=True), dtype=float)
    rpi  = float(freqs.max() / freqs.mean()) if len(freqs) else 0.0
    n    = len(freqs)
    gini = float(2 * np.sum(np.arange(1, n+1) * freqs) / (n * freqs.sum()) - (n+1)/n) if n > 0 else 0.0
    lq   = uniq_c / total_g if total_g > 0 else 0.0
    return {'panels': len(panels), 'buckets': len(bkt_panels),
            'total_g': total_g, 'uniq_c': uniq_c,
            'rpi': rpi, 'gini': gini, 'lq': lq}

# ───────────────────────────────────────────────
# 8. 单矩阵运行
# ───────────────────────────────────────────────
def run_matrix(name, path):
    print(f"\n{'='*70}", flush=True)
    print(f"[{name}]  {path}", flush=True)
    if not os.path.exists(path):
        print(f"  !! 文件不存在，跳过", flush=True)
        return None

    t0 = time.time()
    nrow, ncol, nnz, indptr, indices = load_csr(path)
    print(f"  CSR 加载: {nrow}行 {ncol}列 {nnz}非零  ({time.time()-t0:.1f}s)", flush=True)

    t0 = time.time()
    panels, bkt_panels = build_panels_v17c(nrow, ncol, nnz, indptr, indices)
    stats = compute_matrix_stats(panels, bkt_panels)
    print(f"  Panels={stats['panels']}  Buckets={stats['buckets']}  "
          f"TotalGather={stats['total_g']}  UniqCols={stats['uniq_c']}  "
          f"RPI={stats['rpi']:.1f}  Gini={stats['gini']:.3f}  LQ={stats['lq']:.3f}  "
          f"({time.time()-t0:.1f}s)", flush=True)

    results = {}

    # 基线：Original
    t0 = time.time()
    order = reorder_original(panels, bkt_panels)
    _, _, save = simulate_lru(panels, order)
    air = compute_air(panels, order)
    print(f"  [Original      ] Save={save:5.1f}%  AIR={air:.6f}  ({time.time()-t0:.1f}s)", flush=True)
    results['Original'] = save

    # 基线：MinHash-64
    t0 = time.time()
    order = reorder_minhash(panels, bkt_panels)
    _, _, save = simulate_lru(panels, order)
    air = compute_air(panels, order)
    print(f"  [MinHash-64    ] Save={save:5.1f}%  AIR={air:.6f}  ({time.time()-t0:.1f}s)", flush=True)
    results['MinHash-64'] = save

    # 基线：GreedyInterBkt（仅小矩阵）
    B = stats['buckets']
    if B <= GREEDY_MAX_BUCKETS:
        t0 = time.time()
        order = reorder_greedy_inter_bkt(panels, bkt_panels)
        _, _, save = simulate_lru(panels, order)
        air = compute_air(panels, order)
        print(f"  [GreedyInterBkt] Save={save:5.1f}%  AIR={air:.6f}  ({time.time()-t0:.1f}s)", flush=True)
        results['GreedyInterBkt'] = save
    else:
        print(f"  [GreedyInterBkt] SKIP (B={B} > {GREEDY_MAX_BUCKETS})", flush=True)
        results['GreedyInterBkt'] = None

    # 主实验：HierarchGreedy × K × intra
    for K in K_VALUES:
        if K >= B:
            print(f"  [HG K={K:<5}      ] SKIP (K >= B={B})", flush=True)
            continue
        for intra in ['original', 'greedy']:
            tag = f"HG_K{K}_{intra[:4]}"
            t0  = time.time()
            order = reorder_hierarchical_greedy(panels, bkt_panels, K=K, intra=intra)
            _, _, save = simulate_lru(panels, order)
            air = compute_air(panels, order)
            print(f"  [{tag:<15}] Save={save:5.1f}%  AIR={air:.6f}  ({time.time()-t0:.1f}s)", flush=True)
            results[tag] = save

    # 小结
    print(f"\n  === Summary: {name} ===", flush=True)
    baseline_g = results.get('GreedyInterBkt')
    for k, v in results.items():
        if v is None:
            print(f"    {k:<22}: SKIP", flush=True)
        else:
            cmp = ""
            if baseline_g and k.startswith('HG'):
                cmp = f"  ({v/baseline_g*100:.0f}% of GreedyInterBkt)"
            print(f"    {k:<22}: {v:5.1f}%{cmp}", flush=True)

    return stats, results

# ───────────────────────────────────────────────
# 9. 主函数
# ───────────────────────────────────────────────
def main():
    print("=" * 70, flush=True)
    print("Hierarchical Inter-Bucket Greedy — Phase 14.1 v2", flush=True)
    print(f"K 扫描: {K_VALUES}   intra: [original, greedy]", flush=True)
    print(f"LRU cache: {L2_CACHE}   MinHash h: {MINHASH_H}", flush=True)
    print("=" * 70, flush=True)

    print("\n[文件存在性检查]", flush=True)
    all_ok = True
    for name, path in MATRICES:
        exists = os.path.exists(path)
        print(f"  {'OK' if exists else 'MISSING'} {name}: {path}", flush=True)
        if not exists:
            all_ok = False
    if not all_ok:
        print("\nFATAL: 部分文件不存在，退出。", flush=True)
        sys.exit(1)
    print("所有文件就绪。\n", flush=True)

    all_results = {}
    for name, path in MATRICES:
        try:
            out = run_matrix(name, path)
            if out:
                all_results[name] = out
        except Exception as e:
            print(f"  !! {name} 出错: {e}", flush=True)
            import traceback; traceback.print_exc()

    # 跨矩阵汇总
    print(f"\n{'='*70}", flush=True)
    print("跨矩阵汇总", flush=True)
    print(f"{'矩阵':<20} {'B':>7} {'RPI':>7} {'Gini':>6} {'LQ':>5} "
          f"{'Orig':>6} {'MH64':>6} {'GIBkt':>6} {'HG500o':>7} {'HG1000o':>8} {'HG1000g':>8}", flush=True)
    for name, (stats, results) in all_results.items():
        def g(k):
            v = results.get(k)
            return f"{v:5.1f}%" if v is not None else "  SKIP"
        print(f"{name:<20} {stats['buckets']:>7} {stats['rpi']:>7.1f} "
              f"{stats['gini']:>6.3f} {stats['lq']:>5.3f} "
              f"{g('Original'):>6} {g('MinHash-64'):>6} {g('GreedyInterBkt'):>6} "
              f"{g('HG_K500_orig'):>7} {g('HG_K1000_orig'):>8} {g('HG_K1000_gree'):>8}", flush=True)

    print("\n实验完成。", flush=True)

if __name__ == "__main__":
    main()
