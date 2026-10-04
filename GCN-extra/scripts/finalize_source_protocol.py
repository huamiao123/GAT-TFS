#!/usr/bin/env python3
"""Reconcile finished source comparison with raw logs and record its handoff."""
import csv
import hashlib
import json
import math
import pathlib
import statistics
import subprocess
import sys

root = pathlib.Path(__file__).resolve().parents[1]
run = pathlib.Path(sys.argv[1])
states = json.loads((run/'suite_status.json').read_text())
rows = list(csv.DictReader((run/'summary.csv').open()))
folders={s['graph']:run/s['graph'] for s in states}
evidence_runs=[run]
for extra in map(pathlib.Path,sys.argv[2:]):
    assert (extra/'binary.sha256').read_bytes()==(run/'binary.sha256').read_bytes()
    supplemental=json.loads((extra/'suite_status.json').read_text())
    assert not any(s['status']=='FAILED' for s in supplemental)
    accepted={s['graph']:s for s in supplemental if s['status']=='PASS'}
    assert accepted, 'Supplement must contain a completed graph'
    for graph,s in accepted.items():
        states=[dict(s,evidence=str(extra/graph)) if old['graph']==graph else old for old in states]
        folders[graph]=extra/graph
    rows=[r for r in rows if r['graph'] not in accepted]
    rows+=list(csv.DictReader((extra/'summary.csv').open()))
    evidence_runs.append(extra)
(run/'reconciled_suite_status.json').write_text(json.dumps(states,indent=2)+'\n')
with (run/'reconciled_summary.csv').open('w',newline='') as f:
    w=csv.DictWriter(f,list(rows[0]));w.writeheader();w.writerows(rows)
e2e = {(r['graph'], r['method']): r for r in rows if r['kind']=='e2e'}
passed = [s['graph'] for s in states if s['status']=='PASS']
checks, max_errors, configs, anchors = [], {}, {}, {}
for graph in passed:
    folder = folders[graph]
    data = json.loads((folder/'parsed.json').read_text())
    checks += data['CORRECT']
    configs[graph] = data['CONFIG']
    for r in data['CORRECT']:
        key = r['method']+' vs '+r['reference']+' '+r['boundary']
        max_errors[key] = max(max_errors.get(key, 0), float(r['relative_l2']))
    for binary in ['source_e2e_raw', 'source_mkl_raw', 'source_tfs_raw']:
        log = folder/(binary+'.log')
        if log.exists():
            anchors[graph+' '+binary] = hashlib.sha256(log.read_bytes()).hexdigest()
    # The new runner's insertions must not change original output samples.
    if (folder/'source_e2e_raw.log').exists():
        def samples(text):
            start = text.index('First 8 output values:')
            lines = text[start:].splitlines()
            return [s.strip() for s in lines[1:3]]
        assert samples((folder/'source_e2e_raw.log').read_text()) == samples((folder/'stdout.log').read_text())
assert checks and all(c['pass']=='1' for c in checks)
assert not any(s['status']=='FAILED' for s in states)
assert len(states)==25, 'Formal inventory must account for all 25 canonical graphs'
methods = sorted({method for _,method in e2e})
aggregate = []
for method in methods:
    selected = [e2e[(g,method)] for g in passed]
    aggregate.append(dict(method=method, graphs=len(selected),
        geomean_vs_mkl=math.exp(statistics.mean(math.log(float(r['speedup_vs_source_mkl'])) for r in selected)),
        geomean_vs_tfs=math.exp(statistics.mean(math.log(float(r['speedup_vs_source_tfs'])) for r in selected)),
        geomean_vs_nozero=math.exp(statistics.mean(math.log(float(e2e[(g,'original_nozero')]['min_ms'])/float(e2e[(g,method)]['min_ms'])) for g in passed)),
        faster_than_tfs=sum(float(r['speedup_vs_source_tfs'])>1 for r in selected)))
best = []
for graph in passed:
    fast = min((r for (g,m),r in e2e.items() if g==graph and m.endswith('_fast_nozero')),key=lambda r:float(r['min_ms']))
    accurate = min((r for (g,m),r in e2e.items() if g==graph and m.endswith('_accurate_nozero')),key=lambda r:float(r['min_ms']))
    base = e2e[(graph,'source_mkl_fp32')]
    orig = e2e[(graph,'source_original_tfs')]
    best.append(dict(graph=graph, mkl_ms=float(base['min_ms']),original_ms=float(orig['min_ms']),
        fast_method=fast['method'],fast_ms=float(fast['min_ms']),
        fast_vs_mkl=float(fast['speedup_vs_source_mkl']),fast_vs_tfs=float(fast['speedup_vs_source_tfs']),
        nozero_ms=float(e2e[(graph,'original_nozero')]['min_ms']),
        fast_vs_nozero=float(e2e[(graph,'original_nozero')]['min_ms'])/float(fast['min_ms']),
        accurate_method=accurate['method'],accurate_ms=float(accurate['min_ms']),
        accurate_vs_mkl=float(accurate['speedup_vs_source_mkl']),accurate_vs_tfs=float(accurate['speedup_vs_source_tfs'])))
with (run/'best_per_graph.csv').open('w',newline='') as f:
    w=csv.DictWriter(f,list(best[0]));w.writeheader();w.writerows(best)
summary = dict(run=str(run),passed=len(passed),excluded=len(states)-len(passed),
    evidence_runs=[dict(run=str(p),node=(p/'node.txt').read_text().strip(),policy=(p/'numa_policy_observed.txt').read_text()) for p in evidence_runs],
    per_graph_evidence={g:str(folders[g]) for g in passed},
    checks=len(checks),check_failures=0,policy=(run/'numa_policy_observed.txt').read_text(),
    configs=configs,aggregate_fixed_methods=aggregate,best_per_graph=best,
    error_maxima=max_errors,anchor_log_hashes=anchors,
    original_output_samples_match_untouched_anchors=True,
    caveat='best per graph is post hoc; compare fixed block methods for a deployable policy; FP32 MKL versus BF16 TFS source protocol')
summary['launcher_exit_statuses']={p.name:(p/'status.txt').read_text().strip() for p in evidence_runs}
summary['launcher_issue']='First batch shell exited 2 after SOURCE_SUITE_COMPLETE due to modifying a live launcher; all 16 graph processes had already exited zero with complete timing/correctness evidence. Preserve failed batch status; unique launcher snapshots prevent recurrence.'
(run/'audit_summary.json').write_text(json.dumps(summary,indent=2)+'\n')
lines = ['# 原 TFS / MKL 源码对齐复测（2026-10-04）','',
    '## 源码核对结论','',
    '原 TFS 和原 MKL 的源码、运行脚本均没有显式 NUMA 优化。DegreeSort 是逻辑行调度，不是 NUMA 优化。原脚本只声明 OMP_NUM_THREADS=32、OMP_PROC_BIND=close、OMP_PLACES=cores。',
    '之前的实验给所有路径添加了 numactl interleave，并改过分配方式、随机输入生成顺序、MKL hint/线程控制、统计方式和编译选项；因此那些结果不能称为原源码协议复现。新结果不能与旧表混用，也不能把差异全部归因于 NUMA。','',
    '本轮原 E2E 源码完整保留，候选测量插入原计时与精度检查之后；删除两处插入可以逐字符恢复原文件。MKL helper、分配、首次写入、hint=10、输入生成、ReLU、转换函数未改。Reddit 和 Products 另跑未修改原文件编译出的二进制，前 8 个输出与插入版一致。',
    '复现源码明确声明的运行设置与算子行为，不声称复现论文旧机器及其未知继承环境。原单层 MKL 用 H→W 随机顺序，原两层用交错 W1/W2→H0；单层独立程序只作单层对照，不能替代两层 MKL 基线。','',
    '## 实测协议','',
    '- 运行：'+ '；'.join(p.name+' / '+(p/'node.txt').read_text().strip() for p in evidence_runs)+'；每个作业一个独享 intel_expr 节点，32 线程。',
    '- 所有路径使用默认 NUMA 策略，无 interleave/bind/membind；实际策略及 CPU 布局已保存。',
    '- 原源码 MKL：FP32 SpMM→SGEMM；TFS/候选：BF16 输入与权重，FP32 累加/输出。数值误差另列，不是等精度竞速。',
    '- 1 次预热、5 次实测，主表取最小值；CSV 另列 median/P25/P75，原 E2E stdout 保留 0.01 ms 精度。',
    '- 两层准备后 E2E：第一层算子→ReLU→中间 BF16 转换→第二层算子；加载、排序、初始转换与权重 packing 在计时外。',
    '- 细分 stage/profile 另跑，采样线程时间不能当作墙钟占比。',
    '- 随机 H/W 的原源码 sum 推理计算流；不是训练后分类模型准确率，也不是包括初始化的应用 E2E。','',
    f'通过 {len(passed)} 张真实图，{len(checks)} 项数值检查，0 失败；8 张图单列排除。','',
    '首轮 supervisor 将 RGG 的 N*128==2^31 临界尺寸误判为越界，原排除记录保留。修正为判断 N*128>2^31 后单独补测；补测未更改 C++ 源码、二进制或原 MKL/TFS 协议。最终合并只替换派生汇总中的该图状态，保留两份作业原始状态和全部日志。','',
    '主批次 shell 在 Python 写完 SOURCE_SUITE_COMPLETE、REPORT 和全部 CSV 后退出 2：运行时更新启动脚本导致 Bash 读取了变化的文件尾部。16 个逐图计算进程均正常退出并通过数值门，统计区间已结束，因此保留其有效计算数据，同时明确保留主作业 FAILED 状态。后续提交使用独立冻结的 Slurm/shell/Python 启动快照；修复另以共享正确性作业验证。','',
    '## 两层 E2E（ms，五轮最小值）','',
    '每图最优 block 是事后选择，仅用于研究上限；固定 block 结果见下表与完整 CSV。','',
    '| 图 | 原 MKL FP32 | 原 TFS | 最优 Fast | Fast 耗时 | 对 MKL | 对原 TFS | 对去清零 TFS |',
    '|---|---:|---:|---|---:|---:|---:|---:|']
for r in best:
    lines.append('| {graph} | {mkl_ms:.3f} | {original_ms:.3f} | {fast_method} | {fast_ms:.3f} | {fast_vs_mkl:.3f}× | {fast_vs_tfs:.3f}× | {fast_vs_nozero:.3f}× |'.format(**r))
lines += ['','## 固定方法：17 张图加速比几何平均','',
    '| 方法 | 对原 MKL | 对原 TFS | 对去清零 TFS | 快于原 TFS 的图数 |','|---|---:|---:|---:|---:|']
for r in aggregate:
    lines.append('| {method} | {geomean_vs_mkl:.3f}× | {geomean_vs_tfs:.3f}× | {geomean_vs_nozero:.3f}× | {faster_than_tfs}/{graphs} |'.format(**r))
lines += ['', '对去清零 TFS 的比较扣除了移除原冗余全局 memset 的收益，但仍包含跨输出 panel 的 gather 复用、局部归约、AMX 次数变化等；不能把全部收益都归因于投影冗余消减。']
lines += ['','## 排除项','']
lines.extend('- '+s['graph']+'：'+s['reason'] for s in states if s['status']=='SKIPPED')
lines += ['','## 证据','',
    '- 本次完整路径：`'+str(run)+'`。',
    '- `reconciled_summary.csv` / `reconciled_suite_status.json` / `best_per_graph.csv` / `audit_summary.json`；各作业原 `summary.csv` / `suite_status.json` 不覆盖。',
    '- 每图 `stdout.log`、`time.csv`、`correct.csv`、`stage.csv`、`profile.csv`、`parsed.json` 和 exact command。',
    '- `source_snapshot/`、`build_source.sha256`、`binary.sha256`、`dataset.sha256`、环境和 NUMA 观察文件。',
    '- 详细协议：`docs/SOURCE_MKL_PROTOCOL_20261004.md`；生成可恢复性：build generation manifest。','']
doc=root/'docs/SOURCE_MKL_RESULTS_20261004.md'
doc.write_text('\n'.join(lines))
event=dict(id='source-comparison-final-'+run.name,kind='source_protocol_reconciliation',status='PASS',
    evidence=str(run),report=str(doc),paper_eligible=False,checks=len(checks),datasets=states,
    issues='Previous explicit NUMA interleave and other protocol changes are now isolated from source-faithful results; LP64 limits and nonunit values remain excluded',
    next='Use source-aligned MKL/TFS table for comparisons; independent fixed-policy validation and trained-model accuracy remain future work')
eventpath=run/'reconciliation.event.json'
eventpath.write_text(json.dumps(event,indent=2)+'\n')
subprocess.run([sys.executable,str(root/'scripts/record_event.py'),str(eventpath)],check=True)
print(json.dumps(dict(report=str(doc),graphs=len(passed),checks=len(checks),fixed_methods=aggregate)))
