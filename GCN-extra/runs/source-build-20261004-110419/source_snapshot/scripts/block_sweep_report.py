#!/usr/bin/env python3
"""Report complete nozero block sweep from frozen raw measurements."""
import csv
import json
import pathlib
import statistics
import sys

run = pathlib.Path(sys.argv[1])
target = pathlib.Path(sys.argv[2])
blocks = ['2', '4', '8', '16', '32', '64', 'full']
lines = ['# GCN-extra 邻居分组补测', '',
         f'证据：`{run.name}`。每种方法 1 次预热、10 次测量，轮换运行顺序。', '',
         '固定 D=F=128、16 个 destination rows、R=64，保留 DegreeSort、AMX 和局部 SpMM→GeMM 融合。',
         '分组 B 是沿每行 CSR 顺序每次先聚合 B 个邻居，再做 AMX 投影；例如度数 20：B8 为 8+8+4，B16 为 16+4。',
         '尾组按实际邻居数累加，不丢边。Full 一次聚合整行。每组局部和仅存在于当前 destination tile，完整 AH 不落地主存。', '',
         '所有带 nozero 的方法统一去除冗余全局输出清零；原始源码完整版本单独保留，算法加速比以 Original_nozero 为基准。',
         'Kernel 包含调度、局部归约/转换、AMX、输出；两层 E2E 还包含 ReLU 和中间 BF16 转换。初始输入转换、W packing、DegreeSort 单独记录，不在 prepared E2E 内。',
         'H/W 是固定随机输入；算子是源码的单位边权 sum aggregation，不是 checkpoint 任务准确率实验。', '']
total = failures = 0
results = {}

def read(p):
    with p.open(encoding='utf-8', newline='') as f:
        return list(csv.DictReader(f))

for sub in sorted(run.iterdir()):
    if not sub.is_dir() or not (sub/'timings.csv').exists():
        continue
    timing = read(sub/'timings.csv')
    checks = read(sub/'correctness.csv')
    work = {r['method']:r for r in read(sub/'work.csv')}
    total += len(checks)
    failures += sum(r['pass'] != '1' for r in checks)
    groups = {}
    for r in timing:
        if r['kind'] in ['kernel', 'e2e']:
            groups.setdefault((r['kind'], r['method']), []).append(float(r['ms']))
    meds = {k:statistics.median(v) for k,v in groups.items()}
    info = json.loads((sub/'info.json').read_text(encoding='utf-8'))
    lines += [f'## {sub.name}', '', f"N={info['N']:,}，E={info['E']:,}。", '']
    dataset = {}
    for kind in ['kernel', 'e2e']:
        base = meds[(kind, 'original_nozero')]
        lines += [f'### {kind}，ms', '',
                  '| B | Fast | Accurate | Fast 加速比 vs 原TFS去清零 | Accurate 加速比 |',
                  '|---|---:|---:|---:|---:|']
        for b in blocks:
            fm = f'shared_b{b}_fast_nozero'
            am = f'shared_b{b}_accurate_nozero'
            f, a = meds[(kind,fm)], meds[(kind,am)]
            lines.append(f'| {b} | {f:.3f} | {a:.3f} | {base/f:.3f}× | {base/a:.3f}× |')
        lines += ['', f"完整原TFS：{meds[(kind,'original')]:.3f} ms；去清零原TFS：{base:.3f} ms；MKL FP32：{meds[(kind,'mkl_fp32')]:.3f} ms。", '',
                  '| B | Fast P25—P75 ms | Accurate P25—P75 ms |', '|---|---:|---:|']
        for b in blocks:
            quartiles = [statistics.quantiles(groups[(kind,f'shared_b{b}_{mode}_nozero')], n=4, method='inclusive') for mode in ['fast','accurate']]
            lines.append(f'| {b} | {quartiles[0][0]:.3f}—{quartiles[0][2]:.3f} | {quartiles[1][0]:.3f}—{quartiles[1][2]:.3f} |')
        lines.append('')
        dataset[kind] = {m:ms for (k,m),ms in meds.items() if k==kind}
    lines += ['### 精度与实际工作模型', '',
              '| B | Fast kernel rel L2 | Accurate kernel rel L2 | Fast E2E rel L2 | Accurate E2E rel L2 | Fast 模型AMX GFLOP | 局部输出reload GB |',
              '|---|---:|---:|---:|---:|---:|---:|']
    errors = {(r['boundary'],r['method']):float(r['relative_l2']) for r in checks if r['reference']=='mkl_bf16_inputs'}
    for b in blocks:
        fm, am = f'shared_b{b}_fast_nozero', f'shared_b{b}_accurate_nozero'
        vals = [errors[(bd,m)] for bd in ['kernel_full','two_layer_e2e'] for m in [fm,am]]
        lines.append(f'| {b} | '+ ' | '.join(f'{v:.3e}' for v in vals) + f" | {float(work[fm]['modeled_amx_flops'])/1e9:.3f} | {float(work[fm]['local_output_reload_bytes'])/1e9:.3f} |")
    lines += ['', 'AMX FLOPs/reload bytes 来自源码模型，尚无 PMU 计数器证据。细分 profile 为采样累计线程时间，不能当作总 wall time 百分比。', '']
    results[sub.name] = dataset

lines += ['## 结论边界', '',
          f'本轮正确性记录 {total} 条，失败 {failures} 条。空行、尾组、尾 destination tile 和全空图另见本轮 smoke。',
          '分组越大，投影次数与局部输出 spill/reload 越少，但特征 gather/FP32 归约工作并不消失；性能不一定随 B 单调改善。',
          '四张图、固定 128 维仍不足以判断所有图的最佳 B。完整图集、特征维度、不同节点重复测量及任务精度仍待验证。', '']
target.parent.mkdir(parents=True,exist_ok=True)
target.write_text('\n'.join(lines),encoding='utf-8')
event = {'id':f'analysis-{run.name}-block-sweep', 'kind':'analysis', 'status':'PASS' if failures==0 else 'FAILED',
         'purpose':'Complete B2/4/8/16/32/64/Full Fast and Accurate nozero sweep including two-layer E2E',
         'evidence':str(run), 'report':str(target), 'correctness_records':total, 'failed_records':failures,
         'repeats':10, 'warmups':1, 'metrics_ms':results, 'paper_eligible':False,
         'issues':'no new major issue' if failures==0 else 'See correctness.csv',
         'next':'Expand graph/shape sensitivity and repeat-node evidence; retain strong Original_nozero control'}
target.with_suffix('.event.json').write_text(json.dumps(event,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps({'records':total,'failures':failures,'report':str(target),'metrics':results},ensure_ascii=False))
