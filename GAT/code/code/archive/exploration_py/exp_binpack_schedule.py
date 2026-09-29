#!/usr/bin/env python3
"""
Experiment B: Scheduling Strategy Comparison
7 strategies x 4 matrices, measures: imbalance, overlap, cache hit rate
"""
import struct, os, time
import numpy as np
from collections import Counter, defaultdict

def load_csr_binary(filepath):
    with open(filepath, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('<III', f.read(12))
        nrow, ncol, nnz = struct.unpack('<QQQ', f.read(24))
        indptr = np.frombuffer(f.read((nrow+1)*4), dtype=np.uint32).astype(np.int64)
        indices = np.frombuffer(f.read(nnz*4), dtype=np.uint32).astype(np.int64)
    return nrow, ncol, nnz, indptr, indices

def build_panels(nrow, ncol, nnz, indptr, indices, TILE_R=16):
    col_to_rows = defaultdict(list)
    for r in range(nrow):
        for idx in range(indptr[r], indptr[r+1]):
            col_to_rows[indices[idx]].append(r)
    col_freq = sorted(col_to_rows.items(), key=lambda x: -len(x[1]))
    assigned = set()
    panels = []
    row_cols = {}
    for r in range(nrow):
        row_cols[r] = set(indices[indptr[r]:indptr[r+1]])
    for anchor_col, rows_list in col_freq:
        if len(rows_list) < TILE_R:
            continue
        avail = [r for r in rows_list if r not in assigned]
        if len(avail) < TILE_R:
            continue
        avail_deg = [(r, indptr[r+1]-indptr[r]) for r in avail]
        avail_deg.sort(key=lambda x: x[1])
        selected = [r for r, d in avail_deg[:TILE_R]]
        col_union = set()
        for r in selected:
            col_union |= row_cols[r]
        panels.append((anchor_col, set(selected), col_union))
        assigned.update(selected)
    return panels

def anchor_bucket_reorder(panels):
    af = Counter(p[0] for p in panels)
    bk = defaultdict(list)
    for i, (a, r, c) in enumerate(panels):
        bk[a].append(i)
    sa = sorted(bk.keys(), key=lambda a: -af[a])
    order = []
    for a in sa:
        bl = bk[a]
        bl.sort(key=lambda i: min(panels[i][1]))
        order.extend(bl)
    return order

# ============================================================
# 7 种调度策略
# ============================================================
def sched_static(n, nt):
    """schedule(static): 连续均分, 线程t拿第t块"""
    a = [[] for _ in range(nt)]
    ch = (n+nt-1)//nt
    for i in range(n):
        a[min(i//ch, nt-1)].append(i)
    return a

def sched_dynamic(n, nt, cs=8):
    """schedule(dynamic,cs): 轮询每次取cs个panel"""
    a = [[] for _ in range(nt)]
    pos, t = 0, 0
    while pos < n:
        end = min(pos+cs, n)
        a[t].extend(range(pos, end))
        pos = end
        t = (t+1) % nt
    return a

def sched_guided(n, nt, mc=8):
    """
    schedule(guided,mc): 初始chunk大,后期chunk小
    每次分配 max(remaining/nt, mc) 个panel
    前期大块保cache局部性, 后期小块保负载均衡
    """
    a = [[] for _ in range(nt)]
    pos, t, rem = 0, 0, n
    while pos < n:
        ch = max(rem//nt, mc)
        ch = min(ch, rem)
        a[t].extend(range(pos, pos+ch))
        pos += ch
        rem = n - pos
        t = (t+1) % nt
    return a

def sched_binpack_lpt(panels, order, nt):
    """
    Bin Packing (LPT = Longest Processing Time):
    
    经典 makespan 最小化贪心:
      1. 按 panel 成本(=列并集大小U)降序排
      2. 每次把最大的panel分给当前总成本最小的线程
      3. 分配后保持每个线程内panel的重排顺序(保cache)
    
    近似比: 4/3 - 1/(3m), m=线程数
    比 static 均衡得多, 因为大U panel 被均匀分散
    """
    pc = [(order[i], len(panels[order[i]][2])) for i in range(len(order))]
    pc.sort(key=lambda x: -x[1])
    tc = [0]*nt
    tp = [[] for _ in range(nt)]
    for pidx, cost in pc:
        mt = min(range(nt), key=lambda t: tc[t])
        tp[mt].append((pidx, cost))
        tc[mt] += cost
    # 每个线程内按原始重排顺序排(保cache局部性)
    opos = {order[i]: i for i in range(len(order))}
    return [[p for p,c in sorted(tp[t], key=lambda x: opos.get(x[0],0))]
            for t in range(nt)]

def sched_hot_cold(panels, order, nt, hr=0.8):
    """
    Hot/Cold 混合调度:
      计算每个panel与下一个panel的列交集
      交集大的top80% = "热panel" -> static分配(保cache)
      交集小的bottom20% = "冷panel" -> round-robin(保均衡)
    
    思路: 冷panel本来就没什么cache可复用,
          用dynamic不损失什么, 但能均衡负载
    """
    n = len(order)
    overlaps = []
    for i in range(n-1):
        overlaps.append(len(panels[order[i]][2] & panels[order[i+1]][2]))
    overlaps.append(0)
    thr = int(n * hr)
    si = sorted(range(n), key=lambda i: -overlaps[i])
    hot = set(si[:thr])
    hp = [order[i] for i in range(n) if i in hot]
    cp = [order[i] for i in range(n) if i not in hot]
    a = [[] for _ in range(nt)]
    ch = max(1, (len(hp)+nt-1)//nt)
    for i, p in enumerate(hp):
        a[min(i//ch, nt-1)].append(p)
    for i, p in enumerate(cp):
        a[i % nt].append(p)
    return a

def sched_static_cost_balanced(panels, order, nt):
    """
    成本均衡的静态分配:
      不是按panel数量均分, 而是按总成本(sum of U)均分
      扫描重排后的panel序列, 累积成本达到 total/nt 时切换线程
      保持连续性(cache友好), 同时按成本而非数量切分(更均衡)
    """
    costs = [len(panels[order[i]][2]) for i in range(len(order))]
    total_cost = sum(costs)
    target_per_thread = total_cost / nt
    a = [[] for _ in range(nt)]
    t, running = 0, 0
    for i in range(len(order)):
        a[t].append(order[i])
        running += costs[i]
        if running >= target_per_thread and t < nt-1:
            t += 1
            running = 0
    return a

# ============================================================
# 评估函数
# ============================================================
def evaluate(name, panels, assign, nt):
    """
    三维度评估:
      1. 负载不均衡度 = max_cost / avg_cost  (越接近1越好)
      2. 线程内相邻panel平均列交集 (越大=cache越友好)
      3. 模拟per-thread LRU cache命中率 (cache=8000列, 约L2大小)
    """
    costs = [sum(len(panels[p][2]) for p in assign[t]) for t in range(nt)]
    avg_c = np.mean(costs)
    imb = max(costs)/avg_c if avg_c > 0 else 0

    tol, tpr = 0, 0
    for t in range(nt):
        ps = assign[t]
        for i in range(1, len(ps)):
            tol += len(panels[ps[i]][2] & panels[ps[i-1]][2])
            tpr += 1
    avg_ol = tol/tpr if tpr > 0 else 0

    hits, accs = 0, 0
    for t in range(nt):
        cs, cl = set(), []
        for p in assign[t]:
            for c in panels[p][2]:
                accs += 1
                if c in cs:
                    hits += 1
                    cl.remove(c); cl.append(c)
                else:
                    if len(cl) >= 8000:
                        ev = cl.pop(0); cs.remove(ev)
                    cl.append(c); cs.add(c)
    hr = hits/accs if accs > 0 else 0

    print(f"    {name:30s}: imb={imb:.3f} overlap={avg_ol:7.1f} "
          f"hit={hr*100:5.1f}% range=[{min(costs):,}-{max(costs):,}]")
    return imb, avg_ol, hr

def analyze_matrix(name, filepath, nt=32):
    print(f"\n{'='*70}")
    print(f"  {name} (threads={nt})")
    print(f"{'='*70}")
    nrow, ncol, nnz, indptr, indices = load_csr_binary(filepath)
    print(f"  {nrow:,} x {ncol:,}, NNZ={nnz:,}")
    t0 = time.time()
    panels = build_panels(nrow, ncol, nnz, indptr, indices)
    print(f"  {len(panels):,} panels (build={time.time()-t0:.1f}s)")
    if len(panels) < nt*2:
        print("  Too few panels"); return

    ab = anchor_bucket_reorder(panels)
    costs = [len(panels[i][2]) for i in ab]
    print(f"  U: mean={np.mean(costs):.0f} std={np.std(costs):.0f} "
          f"min={min(costs)} max={max(costs)} CV={np.std(costs)/np.mean(costs):.3f}")

    print(f"\n  --- 8 Scheduling Strategies ---")
    print(f"  {'Strategy':30s}  {'imb':>5s} {'overlap':>8s} "
          f"{'hit%':>6s} {'cost_range':>20s}")
    print(f"  {'-'*75}")

    # 1. Current V17c baseline
    orig = list(range(len(panels)))
    d8 = sched_dynamic(len(orig), nt, 8)
    evaluate("1.Current(orig+dyn8)", panels, d8, nt)

    # 2-8: All use anchor-bucket reorder
    strategies = [
        ("2.AB+static",          sched_static(len(ab), nt)),
        ("3.AB+static_costbal",  None),  # special
        ("4.AB+guided(16)",      sched_guided(len(ab), nt, 16)),
        ("5.AB+guided(64)",      sched_guided(len(ab), nt, 64)),
        ("6.AB+guided(128)",     sched_guided(len(ab), nt, 128)),
    ]
    for label, ai in strategies:
        if ai is None:
            # cost-balanced static
            mapped = sched_static_cost_balanced(panels, ab, nt)
        else:
            mapped = [[ab[i] for i in ai[t]] for t in range(nt)]
        evaluate(label, panels, mapped, nt)

    # 7. BinPack LPT
    bp = sched_binpack_lpt(panels, ab, nt)
    evaluate("7.AB+BinPack(LPT)", panels, bp, nt)

    # 8. Hot/Cold hybrid
    hc = sched_hot_cold(panels, ab, nt)
    evaluate("8.AB+HotCold(80/20)", panels, hc, nt)

def main():
    base = os.path.expanduser("~/data/SpMM_project/data")
    matrices = [
        ("web-Google",     "web-Google/web-Google.csrbin"),
        ("amazon0601",     "amazon0601/amazon0601.csrbin"),
        ("soc-Pokec",      "soc-Pokec/soc-Pokec.csrbin"),
        ("hollywood-2009", "hollywood-2009/hollywood-2009.csrbin"),
    ]
    print("="*70)
    print("  Scheduling Strategy Comparison (8 strategies x 4 matrices)")
    print("  Metrics: load imbalance | avg adjacent overlap | cache hit rate")
    print("="*70)
    for name, rel in matrices:
        fp = os.path.join(base, rel)
        if os.path.exists(fp):
            try: analyze_matrix(name, fp)
            except Exception as e:
                print(f"  ERROR: {e}")
                import traceback; traceback.print_exc()
        else:
            print(f"\n  SKIP: {fp}")
    print("\n=== Done ===")

if __name__ == "__main__":
    main()
