#!/usr/bin/env python3
"""
V17c 内存流量分析
目的：计算实际 gather 流量 vs 理论最小值，找到优化空间

理论最小流量 = 读A(NNZ×4B) + 读B(N×K×2B) + 写C(M×K×4B)
  但 B 太大通常无法全放 cache，所以实际要多次读

V17c 实际流量 = Σ(每个panel的U) × K × 2B (gather B)
              + AMX tile 加载 A  
              + 写回 C
              + Fallback 的逐行 B 访问

关键比值：traffic_amplification = 实际B流量 / 理论最小B流量
  如果 >>1，说明同一 B 行被多个 panel 重复读取，有 B-reuse 空间
  如果 ≈1，说明已经接近最优，优化空间在别处
"""

import struct
import numpy as np
import sys
import os

def read_csrbin(path):
    """读取 .csrbin 二进制格式
    格式: 3×uint32 header + 3×uint64 sizes + uint32 indptr + uint32 indices
    """
    with open(path, 'rb') as f:
        ptype, dtype, vtype = struct.unpack('III', f.read(12))
        nrow, ncol, nnz = struct.unpack('QQQ', f.read(24))
        indptr = np.frombuffer(f.read((nrow+1)*4), dtype=np.uint32)
        indices = np.frombuffer(f.read(nnz*4), dtype=np.uint32)
    return nrow, ncol, nnz, indptr, indices

def simulate_v17c_panels(nrow, ncol, nnz, indptr, indices, K=128):
    """
    模拟 V17c 的列锚点分组，计算每个 panel 的 U（unique columns）
    和总 gather 流量
    
    简化版本：用 CSC 列度数降序做列锚点，贪心选 16 行
    """
    TILE_R = 16
    
    # CSR -> CSC: 统计每列的度数
    # np.bincount: 统计每个值出现的次数
    #   indices 是列索引数组，bincount 后 col_deg[j] = 列 j 的非零个数
    col_deg = np.bincount(indices, minlength=ncol)
    
    # 按列度数降序排列
    # np.argsort: 返回排序后的索引
    #   [::-1] 反转为降序
    col_order = np.argsort(col_deg)[::-1]
    
    # 建立列 -> 行的反向索引（CSC）
    # 这告诉我们每一列被哪些行引用
    row_degrees = np.diff(indptr).astype(np.int64)  # 每行的度数
    
    assigned = np.zeros(nrow, dtype=bool)  # 标记哪些行已分配
    
    total_U = 0           # 所有 AMX panel 的 U 之和
    total_nnz_amx = 0     # AMX 路径覆盖的 NNZ
    total_panels = 0
    total_tiles = 0       # TDPBF16PS 调用次数估算
    panel_U_list = []     # 每个 panel 的 U 值
    
    # 构建 CSC（列 -> 行的映射）
    # 手动构建，避免 scipy 依赖
    col_ptr = np.zeros(ncol + 1, dtype=np.int64)
    col_ptr[1:] = np.cumsum(col_deg)
    col_indices = np.empty(nnz, dtype=np.int64)
    col_pos = col_ptr[:-1].copy()
    
    for i in range(nrow):
        for idx in range(indptr[i], indptr[i+1]):
            j = indices[idx]
            col_indices[col_pos[j]] = i
            col_pos[j] += 1
    
    # 列锚点分组
    for ci in range(min(ncol, len(col_order))):
        c = col_order[ci]
        if col_deg[c] == 0:
            break
        
        # 找所有包含列 c 且未分配的行
        rows_with_c = col_indices[col_ptr[c]:col_ptr[c+1]]
        avail = rows_with_c[~assigned[rows_with_c]]
        
        if len(avail) < 2:
            continue
        
        # 贪心选最多 TILE_R 行（优先选度数低的，控制 U）
        if len(avail) > TILE_R:
            # np.argpartition: 找最小的 k 个元素的索引
            #   比完全排序快，O(n) vs O(n log n)
            k = min(TILE_R, len(avail)) - 1
            part_idx = np.argpartition(row_degrees[avail], k)[:TILE_R]
            selected = avail[part_idx]
        else:
            selected = avail
        
        # 计算这个 panel 的 column union
        panel_cols = set()
        panel_nnz = 0
        for r in selected:
            cols_r = indices[indptr[r]:indptr[r+1]]
            panel_cols.update(cols_r)
            panel_nnz += len(cols_r)
        
        U = len(panel_cols)
        n_tiles = (U + 31) // 32  # 每 32 列一个 tile
        
        total_U += U
        total_nnz_amx += panel_nnz
        total_panels += 1
        total_tiles += n_tiles
        panel_U_list.append(U)
        
        assigned[selected] = True
        
        # 限制处理时间（大矩阵只分析前 N 个 panel）
        if total_panels >= 5000:
            break
    
    fb_rows = np.sum(~assigned)
    fb_nnz = 0
    for r in range(nrow):
        if not assigned[r]:
            fb_nnz += (indptr[r+1] - indptr[r])
    
    return {
        'total_panels': total_panels,
        'total_U': total_U,
        'total_nnz_amx': total_nnz_amx,
        'total_tiles': total_tiles,
        'panel_U_list': panel_U_list,
        'fb_rows': int(fb_rows),
        'fb_nnz': int(fb_nnz),
        'assigned_rows': int(np.sum(assigned)),
    }

def analyze_traffic(nrow, ncol, nnz, panel_info, K=128):
    """
    计算内存流量和关键比值
    
    单位：bytes
    BF16 = 2 bytes, FP32 = 4 bytes
    """
    
    BF16 = 2
    FP32 = 4
    
    # ============================================================
    # 1. 理论最小流量（整个 SpMM 的不可避免的数据量）
    # ============================================================
    # 读 A: 每个非零 = 列索引(4B) + 值(4B，但 BF16 是 2B)
    traffic_read_A = nnz * (4 + BF16)  # col_idx + bf16_val
    
    # 读 B: 如果 B 能全部放 cache，只需读一次 = N × K × BF16
    traffic_read_B_once = ncol * K * BF16
    
    # 写 C: M × K × FP32（结果矩阵）
    traffic_write_C = nrow * K * FP32
    
    # 理论最小 = 读A + 读B一次 + 写C
    traffic_min = traffic_read_A + traffic_read_B_once + traffic_write_C
    
    # ============================================================
    # 2. V17c 实际流量估算
    # ============================================================
    # AMX 路径: 每个 panel gather U 行 B，每行 K×BF16
    traffic_amx_gather_B = panel_info['total_U'] * K * BF16
    
    # AMX 路径: 加载 A tiles (已预打包，连续访问)
    traffic_amx_load_A = panel_info['total_tiles'] * 16 * 32 * BF16
    
    # AMX 路径: 写回 C = assigned_rows × K × FP32
    traffic_amx_write_C = panel_info['assigned_rows'] * K * FP32
    
    # Fallback 路径: 每个 FB 行的每个非零都 gather B 的一行
    # 实际上是 FB_NNZ × K × BF16（因为 BF16 fallback 也读 B_bf16）
    traffic_fb_gather_B = panel_info['fb_nnz'] * K * BF16
    
    # Fallback 路径: 写 C
    traffic_fb_write_C = panel_info['fb_rows'] * K * FP32
    
    # 总实际流量
    traffic_actual = (traffic_amx_gather_B + traffic_amx_load_A + 
                      traffic_amx_write_C + traffic_fb_gather_B + 
                      traffic_fb_write_C)
    
    # ============================================================
    # 3. 关键比值
    # ============================================================
    
    # B 的 traffic amplification:
    #   实际读 B 的总量 / B 的大小
    #   如果 =1，说明 B 每个元素平均只读 1 次（完美 reuse）
    #   如果 >1，说明 B 的行被多个 panel 重复读取
    traffic_B_total = traffic_amx_gather_B + traffic_fb_gather_B
    B_size = ncol * K * BF16
    B_amplification = traffic_B_total / B_size if B_size > 0 else 0
    
    # AMX 内部: gather B 占 AMX 总流量的比例
    traffic_amx_total = traffic_amx_gather_B + traffic_amx_load_A + traffic_amx_write_C
    gather_ratio = traffic_amx_gather_B / traffic_amx_total if traffic_amx_total > 0 else 0
    
    # 每个 panel 的平均 gather 流量
    avg_U = np.mean(panel_info['panel_U_list']) if panel_info['panel_U_list'] else 0
    avg_gather_per_panel = avg_U * K * BF16
    
    # B reuse factor (AMX 路径):
    #   AMX 路径 NNZ / AMX 路径 gather 的 B 行数
    #   >1 说明 B 行被复用（同一 B 行服务于 panel 内多个 A 的非零）
    B_reuse = panel_info['total_nnz_amx'] / panel_info['total_U'] if panel_info['total_U'] > 0 else 0
    
    # 算术强度 (Arithmetic Intensity)
    #   FLOP = 2 × NNZ × K（每个非零做 K 次 multiply-add = 2K FLOP）
    flop = 2 * nnz * K
    AI_actual = flop / traffic_actual
    AI_min = flop / traffic_min
    
    return {
        'traffic_min': traffic_min,
        'traffic_actual': traffic_actual,
        'traffic_amx_gather_B': traffic_amx_gather_B,
        'traffic_fb_gather_B': traffic_fb_gather_B,
        'traffic_B_total': traffic_B_total,
        'B_size': B_size,
        'B_amplification': B_amplification,
        'gather_ratio': gather_ratio,
        'avg_U': avg_U,
        'avg_gather_per_panel_KB': avg_gather_per_panel / 1024,
        'B_reuse': B_reuse,
        'AI_actual': AI_actual,
        'AI_min': AI_min,
        'flop': flop,
    }

# ============================================================
# 主程序
# ============================================================
if __name__ == '__main__':
    DATA_DIR = os.path.expanduser(
        '~/data/SpMM_project/data')
    K = 128
    
    matrices = [
        'web-Google', 'amazon0601', 'soc-Pokec', 
        'as-Skitter', 'cit-Patents', 'indochina-2004', 'hollywood-2009'
    ]
    
    print("=" * 80)
    print(f"V17c Memory Traffic Analysis (K={K})")
    print("=" * 80)
    
    # 用于汇总的表格
    summary = []
    
    for mname in matrices:
        csrbin = os.path.join(DATA_DIR, mname, f'{mname}.csrbin')
        if not os.path.exists(csrbin):
            print(f"SKIP: {csrbin} not found")
            continue
        
        print(f"\n{'#'*60}")
        print(f"# {mname}")
        print(f"{'#'*60}")
        
        nrow, ncol, nnz, indptr, indices = read_csrbin(csrbin)
        avg_deg = nnz / nrow
        print(f"M={nrow:,}  N={ncol:,}  NNZ={nnz:,}  avg_deg={avg_deg:.1f}")
        
        # 模拟 V17c 分组
        print("Simulating column-anchor panels...")
        pinfo = simulate_v17c_panels(nrow, ncol, nnz, indptr, indices, K)
        
        amx_cov = pinfo['assigned_rows'] / nrow * 100
        fb_pct = pinfo['fb_nnz'] / nnz * 100
        print(f"AMX panels: {pinfo['total_panels']}  "
              f"AMX rows: {pinfo['assigned_rows']:,} ({amx_cov:.1f}%)  "
              f"FB rows: {pinfo['fb_rows']:,}  FB NNZ%: {fb_pct:.1f}%")
        
        # 流量分析
        t = analyze_traffic(nrow, ncol, nnz, pinfo, K)
        
        print(f"\n--- Memory Traffic ---")
        print(f"Theoretical minimum:     {t['traffic_min']/1e9:.2f} GB")
        print(f"V17c actual estimate:    {t['traffic_actual']/1e9:.2f} GB")
        print(f"Amplification:           {t['traffic_actual']/t['traffic_min']:.2f}x")
        
        print(f"\n--- B Matrix Traffic ---")
        print(f"B matrix size:           {t['B_size']/1e9:.2f} GB")
        print(f"AMX gather B:            {t['traffic_amx_gather_B']/1e9:.2f} GB")
        print(f"FB gather B:             {t['traffic_fb_gather_B']/1e9:.2f} GB")
        print(f"Total B reads:           {t['traffic_B_total']/1e9:.2f} GB")
        print(f"B amplification:         {t['B_amplification']:.2f}x "
              f"(每个B元素平均被读{t['B_amplification']:.1f}次)")
        
        print(f"\n--- AMX Path ---")
        print(f"Gather B / AMX total:    {t['gather_ratio']*100:.1f}% "
              f"(gather B 在 AMX 流量中的占比)")
        print(f"Avg U per panel:         {t['avg_U']:.0f} columns")
        print(f"Avg gather per panel:    {t['avg_gather_per_panel_KB']:.1f} KB "
              f"(L1D=48KB, L2=2MB)")
        print(f"B reuse (intra-panel):   {t['B_reuse']:.2f}x "
              f"(panel内每个B行服务{t['B_reuse']:.1f}个A非零)")
        print(f"AMX tiles (TDPBF16PS):   {pinfo['total_tiles']:,}")
        
        print(f"\n--- Arithmetic Intensity ---")
        print(f"AI (actual):             {t['AI_actual']:.2f} FLOP/Byte")
        print(f"AI (theoretical min):    {t['AI_min']:.2f} FLOP/Byte")
        print(f"AMX Ridge Point:         ~358 FLOP/Byte")
        print(f"Gap to AMX Ridge:        {358/t['AI_actual']:.0f}x")
        
        # U 分布统计
        U_arr = np.array(pinfo['panel_U_list'])
        if len(U_arr) > 0:
            print(f"\n--- Panel U Distribution ---")
            print(f"U mean={U_arr.mean():.0f}  "
                  f"median={np.median(U_arr):.0f}  "
                  f"P95={np.percentile(U_arr,95):.0f}  "
                  f"max={U_arr.max()}")
            print(f"U<=32: {np.mean(U_arr<=32)*100:.1f}%  "
                  f"U<=64: {np.mean(U_arr<=64)*100:.1f}%  "
                  f"U<=128: {np.mean(U_arr<=128)*100:.1f}%")
            # 每个 panel 的 gather 数据量分布
            gather_KB = U_arr * K * 2 / 1024
            print(f"Gather/panel: mean={gather_KB.mean():.1f}KB  "
                  f"P50={np.median(gather_KB):.1f}KB  "
                  f"P95={np.percentile(gather_KB,95):.1f}KB")
            fits_L1 = np.mean(gather_KB < 48) * 100
            fits_L2 = np.mean(gather_KB < 2048) * 100
            print(f"Fits in L1(<48KB): {fits_L1:.1f}%  "
                  f"Fits in L2(<2MB): {fits_L2:.1f}%")
        
        summary.append({
            'matrix': mname,
            'avg_deg': avg_deg,
            'B_amp': t['B_amplification'],
            'gather_ratio': t['gather_ratio'],
            'avg_U': t['avg_U'],
            'AI': t['AI_actual'],
            'fits_L1': fits_L1 if len(U_arr) > 0 else 0,
        })
    
    # 汇总表
    print(f"\n{'='*80}")
    print("SUMMARY TABLE")
    print(f"{'='*80}")
    print(f"{'Matrix':<18} {'avg_deg':>8} {'B_amp':>8} {'GatherB%':>9} "
          f"{'avg_U':>7} {'AI':>8} {'FitsL1%':>8}")
    print("-" * 68)
    for s in summary:
        print(f"{s['matrix']:<18} {s['avg_deg']:>8.1f} {s['B_amp']:>8.2f}x "
              f"{s['gather_ratio']*100:>8.1f}% {s['avg_U']:>7.0f} "
              f"{s['AI']:>7.2f} {s['fits_L1']:>7.1f}%")
    
    print(f"\n关键问题:")
    print(f"  1. B_amp >> 1 的矩阵 → 同一 B 行被反复读取，inter-panel B-reuse 有空间")
    print(f"  2. GatherB% > 80% → AMX 路径几乎全在搬数据")
    print(f"  3. FitsL1% < 50% → 大部分 panel 的 gather 数据溢出 L1，需要 K-tiling")
    print(f"  4. avg_U > 128 → 大 U panel 消耗大量 gather 带宽，需要 panel splitting")
