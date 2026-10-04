#!/usr/bin/env python3
"""Summarize full graph coverage, including failures and oracle-selection caveat."""
import csv
import json
import math
import pathlib
import statistics
import sys

run=pathlib.Path(sys.argv[1]);target=pathlib.Path(sys.argv[2])
inventory=json.loads((run/'graph_list.json').read_text(encoding='utf-8'))
statuses={x['graph']:x for x in json.loads((run/'suite_status.json').read_text(encoding='utf-8'))}
history=dict(statuses)
paths={g['graph']:run/g['graph'] for g in inventory}
for extra in sys.argv[3:]:
    other=pathlib.Path(extra)
    for st in json.loads((other/'suite_status.json').read_text(encoding='utf-8')):
        if st['status']=='PASS':
            statuses[st['graph']]=dict(st,source_run=other.name)
            paths[st['graph']]=other/st['graph']
blocks=['2','4','8','16','32','64','full']

def read(p):
    with p.open(encoding='utf-8',newline='') as f:return list(csv.DictReader(f))

data={};total=failed_checks=0;comparison=[]
for g in inventory:
    name=g['graph'];sub=paths[name]
    if statuses.get(name,{}).get('status')!='PASS':continue
    checks=read(sub/'correctness.csv');total+=len(checks);failed_checks+=sum(r['pass']!='1' for r in checks)
    work={r['method']:r for r in read(sub/'work.csv')};groups={};components={}
    for r in read(sub/'timings.csv'):
        if r['kind'] in ['kernel','e2e']:groups.setdefault((r['kind'],r['method']),[]).append(float(r['ms']))
        if r['kind']=='e2e':
            for field in ['layer1_ms','activation_conversion_ms','layer2_ms']:
                components.setdefault((r['method'],field),[]).append(float(r[field]))
    med={k:statistics.median(v) for k,v in groups.items()}
    errors={(r['boundary'],r['method']):r for r in checks if r['reference']=='mkl_bf16_inputs'}
    data[name]={'med':med,'groups':groups,'components':components,'work':work,'errors':errors,'inventory':g}
    for (kind,m),v in groups.items():
        quart=statistics.quantiles(v,n=4,method='inclusive')
        err=errors.get(('kernel_full' if kind=='kernel' else 'two_layer_e2e',m),{})
        comparison.append({'graph':name,'source_run':sub.parent.name,'N':g['N'],'E':g['E'],'avg_degree':g['avg_degree'],'kind':kind,'method':m,
                           'median_ms':med[(kind,m)],'P25_ms':quart[0],'P75_ms':quart[2],'samples':len(v),
                           'speedup_original_nozero':med[(kind,'original_nozero')]/med[(kind,m)],
                           'speedup_mkl_fp32':med[(kind,'mkl_fp32')]/med[(kind,m)],
                           'layer1_median_ms':statistics.median(components[(m,'layer1_ms')]) if kind=='e2e' else '',
                           'activation_conversion_median_ms':statistics.median(components[(m,'activation_conversion_ms')]) if kind=='e2e' else '',
                           'layer2_median_ms':statistics.median(components[(m,'layer2_ms')]) if kind=='e2e' else '',
                           'relative_l2_bf16_reference':err.get('relative_l2',''),
                           'max_abs_bf16_reference':err.get('max_abs',''),
                           'mean_abs_bf16_reference':err.get('mean_abs',''),
                           'normalized_max_bf16_reference':err.get('normalized_max','')})

evidence_runs=', '.join(f'`{pathlib.Path(p).name}`' for p in [str(run),*sys.argv[3:]])
lines=['# GCN-extra 25 图覆盖实验', '', f'原始证据：{evidence_runs}。尝试 {len(inventory)} 张图，完整通过 {len(data)} 张。', '',
       '## 统一条件', '',
       '- D=F=128，TR=16，R=64，保留 DegreeSort、局部 SpMM→GeMM 融合、AMX；没有全局 AH 中间矩阵。',
       '- 相同原始 CSR、自环/重复边与随机 H/W；单位边权 sum aggregation，不是归一化 GCN/checkpoint 任务准确率实验。非单位边权文件不更改，拒绝对照并保留原因。',
       '- 原始输入/权重同样截断为 BF16。Fast 每组局部和再转 BF16；Accurate 用 hi+lo 两次 AMX，减少局部和转换误差，仍不是完整 FP32。',
       '- B=2/4/8/16/32/64/Full，Fast 和 Accurate 全部测量。所有算法比较使用 nozero 版本；完整原 TFS 源码未修改且单独测量。',
       '- 每方法 1 次预热、10 次测量；超大图 Friendster 的隔离补测为5次，覆盖B8/16/32/64/Full，其余图覆盖全部7种B。同一图所有基线与方法次数相同。轮换顺序，表格为中位数；CSV 同时给出 P25/P75，正确性在计时前验证。',
       '- 单节点独享，32 物理核心/一个 socket。通常内存 interleave 在该 socket 的4个CPU NUMA域；Friendster 若预算超过200 GiB，在全部在线CPU NUMA域分配内存。同一图所有方法策略相同。',
       '- Kernel 包括调度/归约/转换/AMX/输出；prepared 两层 E2E 包括两次 kernel、ReLU、中间 BF16 转换。初始输入转换、W packing、DegreeSort 单独记录、不包含在 prepared E2E。',
       '- 校验阶段三个不再使用的全局参考数组在两层阶段前释放，所有计时间隔外执行；不修改 kernel。',
       '- Friendster 的边数与 Road-USA 的128维 dense 索引分别超过32位有符号范围；两图切换 MKL ILP64 后补测通过（仅索引64位、浮点精度不变）。原始LP64失败仍保留；缺少内部崩溃栈，具体故障行未证明。成功补测源目录见CSV的source_run。',
       '- 细分 profiles.csv 是采样累计线程时间，不能直接作为 wall time 百分比。FLOPs/bytes 是源码模型，尚无 PMU 计数器。', '',
       '## 全图覆盖与两层结果', '',
       '最佳 B 是对本次所有候选事后取最小中位数，属于 oracle 上界，不是实现了自适应选择器。', '',
       '| 图 | 平均度 E/N | 原TFS去清零 ms | B8 Fast ms | B16 Fast ms | Full Fast ms | 本轮最快 Fast B | 最快 Fast ms | 加速比 vs 原TFS去清零 |',
       '|---|---:|---:|---:|---:|---:|---|---:|---:|']
speed=[];full_speed=[];accurate_speed=[]
for g in inventory:
    name=g['graph']
    if name not in data:
        st=statuses.get(name,{});lines.append(f"| {name} | {g['avg_degree']:.2f} | {st.get('status','PENDING')} | — | — | — | — | — | — |")
        continue
    d=data[name];m=d['med'];base=m[('e2e','original_nozero')]
    available=[b for b in blocks if ('e2e',f'shared_b{b}_fast_nozero') in m]
    best=min(available,key=lambda b:m[('e2e',f'shared_b{b}_fast_nozero')]);bv=m[('e2e',f'shared_b{best}_fast_nozero')]
    speed.append(base/bv);full_speed.append(base/m[('e2e','shared_bfull_fast_nozero')])
    ab=min(m[('e2e',f'shared_b{b}_accurate_nozero')] for b in available);accurate_speed.append(base/ab)
    lines.append(f"| {name} | {g['avg_degree']:.2f} | {base:.3f} | {m[('e2e','shared_b8_fast_nozero')]:.3f} | {m[('e2e','shared_b16_fast_nozero')]:.3f} | {m[('e2e','shared_bfull_fast_nozero')]:.3f} | {best} | {bv:.3f} | {base/bv:.3f}× |")
lines+=['','## 高平均度图：完整分组与精度','']
for name,d in data.items():
    if d['inventory']['avg_degree']<50:continue
    m=d['med'];w=d['work'];base=m[('e2e','original_nozero')]
    lines += [f'### {name}', '',
              f"N={d['inventory']['N']:,}，E={d['inventory']['E']:,}，平均度={d['inventory']['avg_degree']:.2f}。实际 memory nodes：{statuses[name]['memory_nodes']}。", '',
              '| B | Fast kernel ms | Accurate kernel ms | Fast 两层 ms | Accurate 两层 ms | Fast 两层加速比 | Fast 两层 rel L2 | Accurate 两层 rel L2 |',
              '|---|---:|---:|---:|---:|---:|---:|---:|']
    for b in blocks:
        f,a=f'shared_b{b}_fast_nozero',f'shared_b{b}_accurate_nozero';e=d['errors']
        if ('kernel',f) not in m:
            lines.append(f'| {b} | — | — | — | — | — | — | — |');continue
        lines.append(f"| {b} | {m[('kernel',f)]:.3f} | {m[('kernel',a)]:.3f} | {m[('e2e',f)]:.3f} | {m[('e2e',a)]:.3f} | {base/m[('e2e',f)]:.3f}× | {float(e[('two_layer_e2e',f)]['relative_l2']):.3e} | {float(e[('two_layer_e2e',a)]['relative_l2']):.3e} |")
    lines += ['', f"两层 MKL FP32={m[('e2e','mkl_fp32')]:.3f} ms；完整原TFS={m[('e2e','original')]:.3f} ms；去清零原TFS={base:.3f} ms。", '',
              '| 路径 | Layer 1 ms | ReLU + BF16转换 ms | Layer 2 ms | 两层 E2E ms |',
              '|---|---:|---:|---:|---:|']
    for method in ['mkl_fp32','original_nozero',*[f'shared_b{b}_fast_nozero' for b in ['8','16','64','full']]]:
        c=d['components'];vals=[statistics.median(c[(method,f)]) for f in ['layer1_ms','activation_conversion_ms','layer2_ms']]
        lines.append(f"| {method} | {vals[0]:.3f} | {vals[1]:.3f} | {vals[2]:.3f} | {m[('e2e',method)]:.3f} |")
    lines += ['', '各区间独立取中位数，所以其和不必等于 E2E 中位数。MKL FP32 的激活区间不需要 BF16 转换。', '',
              '| B | 物理 AMX 工作模型 Fast GFLOP | 行利用率 eta | 局部输出 reload GB |', '|---|---:|---:|---:|']
    for b in blocks:
        if f'shared_b{b}_fast_nozero' not in w:continue
        r=w[f'shared_b{b}_fast_nozero']
        lines.append(f"| {b} | {float(r['modeled_amx_flops'])/1e9:.3f} | {float(r['eta']):.6f} | {float(r['local_output_reload_bytes'])/1e9:.3f} |")
    lines.append('')

lines += ['## 不包含在 prepared E2E 的一次性准备', '',
          '这些区间只执行一次，未经过10轮中位数统计；不能直接拼接成重复推理延迟。随机输入生成仅是实验准备。完整准备记录见各图 preprocess.csv。', '',
          '| 图 | CSR载入校验 ms | 边权完整扫描 ms | DegreeSort ms | 输入BF16转换及参考解码 ms | 两层权重packing ms | MKL图设置 ms |',
          '|---|---:|---:|---:|---:|---:|---:|']
for name in data:
    prep={r['phase']:float(r['ms']) for r in read(paths[name]/'preprocess.csv')}
    lines.append(f"| {name} | {prep['csr_load_validate']:.3f} | {prep['full_values_scan']:.3f} | {prep['degree_sort']:.3f} | {prep['input_bf16_convert_and_reference_decode']:.3f} | {prep['weight1_pack']+prep['weight2_pack']:.3f} | {prep['mkl_graph_setup']:.3f} |")
lines += ['', '## 空行与非空行平均度', '',
          '非空行数来自 Full 的 useful_block_rows（每个非空行一个逻辑block），不需要重新扫描图。', '',
          '| 图 | 全图 E/N | 空行比例 | 非空行 E/N_active | 最大度 |', '|---|---:|---:|---:|---:|']
for name,d in data.items():
    r=d['work']['shared_bfull_fast_nozero'];n=int(r['N']);active=int(r['useful_block_rows'])
    lines.append(f"| {name} | {float(r['avg_degree']):.2f} | {100*(n-active)/n:.2f}% | {int(r['E'])/max(1,active):.2f} | {r['max_degree']} |")
lines += ['', '例如 Wiki-Talk 的全图平均度很低，但绝大多数行为零度；非空行平均度与全图平均度差别很大。',
          '源码中局部路径对整块空行直接逐行清零，原TFS仍执行 tile zero/store/scatter。因此总收益同时包含分组归约、跨输出panel输入复用与空行输出路径变化，不能把所有收益归因于投影FLOPs减少。原因份额需专门消融验证。', '']

lines += ['## 未通过的图及原因', '']
issues={name:x.get('reason','') for name,x in statuses.items() if x['status']!='PASS'}
if not issues:lines.append('无。全部尝试的图均完整通过。')
for name,reason in issues.items():lines += [f"- `{name}`：{reason or 'exit_code='+str(statuses[name].get('exit_code'))}；原始 stderr/status 保留。"]
for name,st in history.items():
    if st['status']!='PASS' and statuses.get(name,{}).get('status')=='PASS':
        lines += [f"- `{name}` 原始尝试失败：{st.get('reason') or 'exit_code='+str(st.get('exit_code'))}；已由 `{statuses[name]['source_run']}` 隔离补测通过。失败证据未覆盖。"]
lines += ['', '## 汇总与边界', '', f'正确性记录 {total} 条，失败 {failed_checks} 条（覆盖成功完成的图）；失败图单独列出，不能当作通过。','']
if speed:
    gm=lambda v:math.exp(statistics.mean(math.log(x) for x in v))
    lines += [f'- 成功图的事后最佳 Fast 几何平均加速比 {gm(speed):.3f}×，超过1×的图 {sum(x>1 for x in speed)}/{len(speed)}。这不包含 B 选择开销。',
              f'- 固定 Full Fast 几何平均加速比 {gm(full_speed):.3f}×；事后最佳 Accurate 几何平均加速比 {gm(accurate_speed):.3f}×。',
              '- 全图统计与高平均度图分开解释。平均度、度数分布、源特征局部性、输出成本都会影响收益；不能仅凭 E/N 给出通用最佳 B。',
              '- 固定128维、单独享节点、随机H/W；任务准确率、维度敏感性与重复节点验证仍未完成，论文级泛化结论 UNVERIFIED。']
target.parent.mkdir(exist_ok=True,parents=True);target.write_text('\n'.join(lines)+'\n',encoding='utf-8')
csvpath=target.with_suffix('.csv')
if comparison:
    with csvpath.open('w',encoding='utf-8',newline='') as f:
        w=csv.DictWriter(f,fieldnames=list(comparison[0]));w.writeheader();w.writerows(comparison)
event={'id':f'analysis-{run.name}-{target.stem.lower()}','kind':'analysis','status':'PASS' if not issues and failed_checks==0 and len(data)==len(inventory) else 'PARTIAL',
       'attempted_graphs':len(inventory),'passed_graphs':len(data),'correctness_records':total,'failed_correctness_records':failed_checks,
       'issues':issues or 'no new major issue','evidence':str(run),'evidence_runs':[str(run),*sys.argv[3:]],'report':str(target),'comparison_csv':str(csvpath),'paper_eligible':False,
       'selection':'best B is posthoc oracle, not implemented adaptive dispatch','next':'Audit high-degree results and nonunit datasets; shape/task/repeated-node evidence still pending'}
target.with_suffix('.event.json').write_text(json.dumps(event,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(event,ensure_ascii=False))
