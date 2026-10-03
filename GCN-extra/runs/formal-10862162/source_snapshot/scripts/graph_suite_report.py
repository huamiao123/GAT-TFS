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
blocks=['2','4','8','16','32','64','full']

def read(p):
    with p.open(encoding='utf-8',newline='') as f:return list(csv.DictReader(f))

data={};total=failed_checks=0;comparison=[]
for g in inventory:
    name=g['graph'];sub=run/name
    if statuses.get(name,{}).get('status')!='PASS':continue
    checks=read(sub/'correctness.csv');total+=len(checks);failed_checks+=sum(r['pass']!='1' for r in checks)
    work={r['method']:r for r in read(sub/'work.csv')};groups={}
    for r in read(sub/'timings.csv'):
        if r['kind'] in ['kernel','e2e']:groups.setdefault((r['kind'],r['method']),[]).append(float(r['ms']))
    med={k:statistics.median(v) for k,v in groups.items()}
    errors={(r['boundary'],r['method']):r for r in checks if r['reference']=='mkl_bf16_inputs'}
    data[name]={'med':med,'groups':groups,'work':work,'errors':errors,'inventory':g}
    for (kind,m),v in groups.items():
        quart=statistics.quantiles(v,n=4,method='inclusive')
        err=errors.get(('kernel_full' if kind=='kernel' else 'two_layer_e2e',m),{})
        comparison.append({'graph':name,'N':g['N'],'E':g['E'],'avg_degree':g['avg_degree'],'kind':kind,'method':m,
                           'median_ms':med[(kind,m)],'P25_ms':quart[0],'P75_ms':quart[2],'samples':len(v),
                           'speedup_original_nozero':med[(kind,'original_nozero')]/med[(kind,m)],
                           'speedup_mkl_fp32':med[(kind,'mkl_fp32')]/med[(kind,m)],
                           'relative_l2_bf16_reference':err.get('relative_l2',''),
                           'max_abs_bf16_reference':err.get('max_abs',''),
                           'mean_abs_bf16_reference':err.get('mean_abs',''),
                           'normalized_max_bf16_reference':err.get('normalized_max','')})

lines=['# GCN-extra 25 图覆盖实验', '', f'原始证据：`{run.name}`。尝试 {len(inventory)} 张图，完整通过 {len(data)} 张。', '',
       '## 统一条件', '',
       '- D=F=128，TR=16，R=64，保留 DegreeSort、局部 SpMM→GeMM 融合、AMX；没有全局 AH 中间矩阵。',
       '- 相同原始 CSR、自环/重复边与随机 H/W；单位边权 sum aggregation，不是归一化 GCN/checkpoint 任务准确率实验。非单位边权文件不更改，拒绝对照并保留原因。',
       '- 原始输入/权重同样截断为 BF16。Fast 每组局部和再转 BF16；Accurate 用 hi+lo 两次 AMX，减少局部和转换误差，仍不是完整 FP32。',
       '- B=2/4/8/16/32/64/Full，Fast 和 Accurate 全部测量。所有算法比较使用 nozero 版本；完整原 TFS 源码未修改且单独测量。',
       '- 每方法 1 次预热、10 次测量，轮换顺序，表格为中位数；CSV 同时给出 P25/P75，正确性在计时前验证。',
       '- 单节点独享，32 物理核心/一个 socket。通常内存 interleave 在该 socket 的4个CPU NUMA域；Friendster 若预算超过200 GiB，在全部在线CPU NUMA域分配内存。同一图所有方法策略相同。',
       '- Kernel 包括调度/归约/转换/AMX/输出；prepared 两层 E2E 包括两次 kernel、ReLU、中间 BF16 转换。初始输入转换、W packing、DegreeSort 单独记录、不包含在 prepared E2E。',
       '- 校验阶段三个不再使用的全局参考数组在两层阶段前释放，所有计时间隔外执行；不修改 kernel。',
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
    best=min(blocks,key=lambda b:m[('e2e',f'shared_b{b}_fast_nozero')]);bv=m[('e2e',f'shared_b{best}_fast_nozero')]
    speed.append(base/bv);full_speed.append(base/m[('e2e','shared_bfull_fast_nozero')])
    ab=min(m[('e2e',f'shared_b{b}_accurate_nozero')] for b in blocks);accurate_speed.append(base/ab)
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
        lines.append(f"| {b} | {m[('kernel',f)]:.3f} | {m[('kernel',a)]:.3f} | {m[('e2e',f)]:.3f} | {m[('e2e',a)]:.3f} | {base/m[('e2e',f)]:.3f}× | {float(e[('two_layer_e2e',f)]['relative_l2']):.3e} | {float(e[('two_layer_e2e',a)]['relative_l2']):.3e} |")
    lines += ['', f"两层 MKL FP32={m[('e2e','mkl_fp32')]:.3f} ms；完整原TFS={m[('e2e','original')]:.3f} ms；去清零原TFS={base:.3f} ms。", '',
              '| B | 物理 AMX 工作模型 Fast GFLOP | 行利用率 eta | 局部输出 reload GB |', '|---|---:|---:|---:|']
    for b in blocks:
        r=w[f'shared_b{b}_fast_nozero']
        lines.append(f"| {b} | {float(r['modeled_amx_flops'])/1e9:.3f} | {float(r['eta']):.6f} | {float(r['local_output_reload_bytes'])/1e9:.3f} |")
    lines.append('')

lines += ['## 未通过的图及原因', '']
issues={name:x.get('reason','') for name,x in statuses.items() if x['status']!='PASS'}
if not issues:lines.append('无。全部尝试的图均完整通过。')
for name,reason in issues.items():lines += [f'- `{name}`：{reason}；原始 stderr/status 保留。']
lines += ['', '## 汇总与边界', '', f'正确性记录 {total} 条，失败 {failed_checks} 条（覆盖成功完成的图）；失败图单独列出，不能当作通过。']
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
event={'id':f'analysis-{run.name}-25graphs','kind':'analysis','status':'PASS' if not issues and failed_checks==0 and len(data)==len(inventory) else 'PARTIAL',
       'attempted_graphs':len(inventory),'passed_graphs':len(data),'correctness_records':total,'failed_correctness_records':failed_checks,
       'issues':issues or 'no new major issue','evidence':str(run),'report':str(target),'comparison_csv':str(csvpath),'paper_eligible':False,
       'selection':'best B is posthoc oracle, not implemented adaptive dispatch','next':'Audit high-degree results and nonunit datasets; shape/task/repeated-node evidence still pending'}
target.with_suffix('.event.json').write_text(json.dumps(event,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(event,ensure_ascii=False))
