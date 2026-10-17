#!/usr/bin/env python3
"""Reconcile completed raw evidence; never execute kernels or choose thresholds."""
import csv
import hashlib
import json
import math
import pathlib
import statistics
import sys

build, out = map(pathlib.Path, sys.argv[1:3])
gate_job, perf_job = sys.argv[3:5]
gate = build / ('logs/ordered-' + gate_job)
perf = build / ('logs/ordered-' + perf_job)
out.mkdir(parents=True, exist_ok=True)
graphs = ['ogbn-products', 'reddit', 'mycielskian19', 'roadNet-CA', 'wiki-Talk']
methods = ['paper_tfs', 'source_mkl_fp32', 'p0_degree', 'pack_epi_degree',
           'dynamic_all_share', 'event_independent', 'event_share',
           'dynamic_selected_share', 'exact_only_share']


def rows(p):
    with p.open(newline='') as f:
        return list(csv.DictReader(f))


def write(p, rr):
    if rr:
        with p.open('w', newline='') as f:
            w = csv.DictWriter(f, list(dict.fromkeys(k for r in rr for k in r)))
            w.writeheader()
            w.writerows(rr)


completions = []
for run, count in [(gate, 11), (perf, 5)]:
    c = json.loads((run / 'completion.json').read_text())
    assert c['status'] == 'PASS' and len(c['graphs']) == count
    assert (run / 'exit_status.txt').read_text().strip() == '0'
    completions.append(c)
    for name in ['method_check', 'state_equiv', 'state_semantic', 'state_work_gate', 'state_profile_gate']:
        rr = rows(run / (name + '.csv'))
        assert rr and all(r['pass'] == '1' for r in rr), (run, name)

raw = rows(perf / 'state_time.csv')
assert len(raw) == 675
times = {}
for r in raw:
    times.setdefault((r['graph'], r['method'], r['kind']), []).append(float(r['ms']))
stats = {}
for key, v in times.items():
    assert len(v) == (10 if key[2] == 'alternating' else 5)
    stats[key] = dict(median_ms=statistics.median(v), min_ms=min(v), max_ms=max(v),
                      CV=statistics.stdev(v)/statistics.mean(v), repeats=len(v))
assert len(stats) == 5 * 9 * 2
write(out / 'timing_summary.csv', [dict(graph=k[0], method=k[1], protocol=k[2], **v) for k, v in stats.items()])
noisy = [dict(graph=k[0], method=k[1], protocol=k[2], **v) for k, v in stats.items() if v['CV'] > .05]
write(out / 'timing_anomalies.csv', noisy)


def ms(g, m, protocol='alternating'):
    return stats[g, m, protocol]['median_ms']


paired = []
for g in graphs:
    for m in methods:
        for protocol in ['alternating', 'consecutive']:
            for ref in ['pack_epi_degree', 'event_independent', 'dynamic_selected_share', 'exact_only_share']:
                paired.append(dict(graph=g, candidate=m, reference=ref, protocol=protocol,
                                   candidate_ms=ms(g,m,protocol), reference_ms=ms(g,ref,protocol),
                                   reference_over_candidate=ms(g,ref,protocol)/ms(g,m,protocol),
                                   candidate_CV=stats[g,m,protocol]['CV'], reference_CV=stats[g,ref,protocol]['CV']))
write(out / 'paired_comparisons.csv', paired)
plans = {r['graph']: r for r in rows(perf / 'event_plan.csv')}
structure_plans = {r['graph']: r for r in rows(gate / 'event_plan.csv') if r['phase_kind']=='structure'}
coverage = []
for g in graphs:
    p, first = plans[g], structure_plans[g]
    for k in ['E','total_pair_windows','all_matches','eligible_matches','partial_matches','exact_matches','run_count','plan_bytes']:
        assert p[k] == first[k], (g,k)
    e = int(p['E'])
    gain = ms(g,'pack_epi_degree') - ms(g,'event_share')
    setup = float(p['construct_ms']) + float(p['audit_ms'])
    coverage.append(dict(graph=g, E=e, all_matches=int(p['all_matches']),
                         eligible_matches=int(p['eligible_matches']), partial_matches=int(p['partial_matches']),
                         exact_matches=int(p['exact_matches']), eligible_fraction=int(p['eligible_matches'])/e,
                         partial_fraction=int(p['partial_matches'])/e, all_match_fraction=int(p['all_matches'])/e,
                         plan_bytes=int(p['plan_bytes']), allocated_bytes=int(p['allocated_bytes']),
                         construct_ms=float(p['construct_ms']), audit_ms=float(p['audit_ms']),
                         prepared_gain_ms=gain, optimistic_setup_amortization_calls=math.ceil(setup/gain) if gain>0 else 'NO_POSITIVE_MEDIAN_GAIN',
                         interpretation='observed shared-node median; excludes common DegreeSort; not guaranteed break-even'))
write(out / 'coverage_and_setup.csv', coverage)

counters = rows(perf / 'event_counters.csv')
work = []
for r in counters:
    g,m = r['graph'],r['method']; p=plans[g]
    ref=next(x for x in counters if x['graph']==g and x['method']=='pack_epi_degree' and x['layer']==r['layer'])
    for k in ['edges','adds','tdp','prefetch','partial_load_bytes','partial_store_bytes','packed_bytes']:
        assert r[k]==ref[k],(g,m,k)
    expected = {'pack_epi_degree':0,'dynamic_all_share':int(p['all_matches']),
                'event_independent':0,'event_share':int(p['eligible_matches']),
                'dynamic_selected_share':int(p['eligible_matches']),'exact_only_share':int(p['exact_matches'])}[m]
    assert int(r['saved_decode_rows'])==expected
    assert int(r['decoded_rows'])==int(r['edges'])-expected
    if m=='event_share':assert int(r['head_comparisons'])==0
    work.append(dict(r, requested_source_bytes=256*int(r['decoded_rows']),
                     decoded_16BF16_vector_ops=8*int(r['decoded_rows']), saved_requested_source_bytes=256*expected,
                     units='per-layer logical source-level requests; not DRAM bytes or retired decode instructions'))
write(out / 'work_comparison.csv', work)

stage = rows(perf / 'event_stage.csv')
stage_summary=[]
for g in graphs:
    for m in methods:
        rr=[r for r in stage if r['graph']==g and r['method']==m]
        if m=='source_mkl_fp32':assert not rr;continue
        assert len(rr)==3
        stage_summary.append(dict(graph=g,method=m,epilogue=rr[0]['epilogue'],
                                  **{k:statistics.median(float(r[k]) for r in rr) for k in ['layer1_ms','relu_ms','convert_ms','layer2_ms','total_ms']}))
write(out / 'stage_summary.csv', stage_summary)

checks=rows(perf/'method_check.csv');correctness=[]
for g in graphs:
    for m in methods:
        if m=='source_mkl_fp32':continue
        rr=[r for r in checks if r['graph']==g and r['method']==m]
        assert len(rr)==4
        correctness.append(dict(graph=g,method=m,checks=len(rr),max_rel_L2=max(float(r['relative_L2']) for r in rr),
                                max_abs=max(float(r['max_abs']) for r in rr),PASS=all(r['pass']=='1' for r in rr)))
write(out/'correctness_summary.csv',correctness)
thread_rows=rows(perf/'event_thread.csv');imbalance=[]
for g in graphs:
    for m in methods:
        if m not in {r['method'] for r in thread_rows}:continue
        for layer in ['1','2']:
            rr=[r for r in thread_rows if r['graph']==g and r['method']==m and r['layer']==layer]
            assert len(rr)==32
            active=[float(r['active_ms']) for r in rr]
            imbalance.append(dict(graph=g,method=m,layer=layer,threads=len(rr),active_max_ms=max(active),
                                  active_mean_ms=statistics.mean(active),active_max_over_mean=max(active)/statistics.mean(active),
                                  active_CV=statistics.stdev(active)/statistics.mean(active),edges_sum=sum(int(r['edges']) for r in rr)))
write(out/'thread_imbalance.csv',imbalance)

pmu=rows(perf/'method_pmu.csv')
usable=[r for r in pmu if all(r[k]=='1' for k in ['available','coverage_complete','scaled_complete'])
        and all(r[k]=='32' for k in ['initial_threads','final_threads','counted_threads','scaled_threads'])
        and all(r[k]=='0' for k in ['failed_threads','zero_running_threads','new_threads','disappeared_threads'])]
pmu_summary=[]
for g in graphs:
    for m in ['pack_epi_degree','event_independent','event_share','dynamic_selected_share']:
        rr=[r for r in usable if r['graph']==g and r['method']==m]
        if not rr:continue
        a=dict(graph=g,method=m,repeats=len(rr))
        for k in ['cycles_scaled','instructions_scaled','cache_misses_scaled','cache_references_scaled','amx_busy_scaled']:
            a[k]=statistics.median(float(r[k]) for r in rr)
        a['IPC_ratio_of_counter_medians']=a['instructions_scaled']/a['cycles_scaled']
        pmu_summary.append(a)
write(out/'pmu_summary.csv',pmu_summary)

audit=dict(status='PASS',scope='ordered_partial_event_plan_static_unit_inference_128x2',
           build=str(build),gate_job=gate_job,performance_job=perf_job,
           node=(perf/'node.txt').read_text().strip(),affinity=json.loads((perf/'affinity.json').read_text()),
           environment=json.loads((perf/'environment.json').read_text()),
           source_manifest=json.loads((build/'manifest.json').read_text()),
           completion_counts=[c['counts'] for c in completions],raw_timing_records=len(raw),
           PMU_records=len(pmu),usable_PMU_records=len(usable),noisy_method_protocol_records=len(noisy),
           fixed_selection='>=8 shared source heads, <=8 RLE runs, adjacent existing DegreeSort pairs, S64',
           precision='FAST main; FAST/ACCURATE x M64/M256/FULL correctness only',
           paper_eligible=False,analysis_sha256=hashlib.sha256(pathlib.Path(__file__).read_bytes()).hexdigest())
(out/'audit.json').write_text(json.dumps(audit,indent=2)+'\n')

L=[]
def add(s=''):L.append(s)
def table(head,rr):
    add('| '+' | '.join(head)+' |');add('| '+' | '.join(['---']*len(head))+' |')
    for r in rr:add('| '+' | '.join(map(str,r))+' |')
    add()

add('# 保留 TFS 的部分邻居静态执行计划：实现与五图实验')
add()
add('日期：2026-10-14。静态单位边权、随机 H/W seed12345、两层 128→128→128。不是训练或真实 checkpoint 分类准确率实验。')
add()
add('## 研究问题与改动')
add()
add('上一轮 partial bundle 减少 C 暂存，但逐边 source 解码未减少。本轮检验：把已有相邻两行 current-head 匹配提前编码，能否在不改变各行 CSR 加法顺序的条件下复用部分 source 读取/解码，并避免在线匹配开销。')
add()
add('保留原 DegreeSort/TR16 行成员和 AMX 行槽、R64 dynamic(1)、S64、W packing、BF16 高16位截断、PACK_EPI、AMX 配置/投影和默认 NUMA。主性能固定 FULL FAST；M64/M256 和 ACCURATE 仅用于正确性检查。原 TFS/MKL 与冻结父源码不改动。')
add()
add('静态 inspector 复用已有 greedy current-head 合并，编码 mask1/2/3 为两字节 run。每个 S64 窗口固定选择“至少8次共享、至多8个run”；结果出来后未调阈值。不是邻居集合去重，不合并公共和，不缓存跨 forward 的特征，也不改变逐行浮点加法顺序。共享路径也可能省当前 CSR 索引读取，收益不能全部归于 BF16 decode。')
add()
table(['名称','真实作用'],[
 ['PACK_EPI','规范控制；原 ScalarReducer + 已验证融合层间 epilogue'],
 ['DynamicAll','旧动态匹配，所有相邻窗口均执行 current-head 比较'],
 ['StaticIndependent','新计划、相同选区，分别加载/解码两个 source'],
 ['StaticShared','新计划、相同选区，相同 source 只加载/解码一次'],
 ['DynamicSelected','只在新计划选择的相同窗口做动态匹配与共享'],
 ['ExactOnly','只共享完全相同正长度窗口，含不足8条的短窗口'],
 ['P0/Original/MKL','既有控制及源码锚点']])
add('## 环境与完整性')
add()
add(f'build：`{build.name}`；小图/结构 Job {gate_job}，性能 Job {perf_job}/{audit["node"]}。共享 intel、单 socket32物理核、close/cores、无 interleave，`paper_eligible=false`。共享结果为探索证据。每方法1次 warmup、5次连续和10次正反顺序测量；以10次中位数为主，两种协议/raw/CV均保存。')
add()
add('初始 H/W、图读取、DegreeSort、计划构造在 prepared E2E 外。计划构造/审核/容量及摊销另列。PACK_EPI 路径 ReLU/截断在 Layer1 内，stage 中独立 ReLU/convert 为0；不是删除模型激活。子段分别取中位数，不能保证相加恰好等于 total 的中位数。')
add()
table(['阶段','数值检查','逐位边界','源级工作量','production profile一致性'],[
 ['小图+结构',completions[0]['counts']['METHOD_CHECK'],completions[0]['counts']['STATE_EQUIV'],completions[0]['counts']['STATE_WORK_GATE'],completions[0]['counts']['STATE_PROFILE_GATE']],
 ['五图性能',completions[1]['counts']['METHOD_CHECK'],completions[1]['counts']['STATE_EQUIV'],completions[1]['counts']['STATE_WORK_GATE'],completions[1]['counts']['STATE_PROFILE_GATE']]])
add('上述检查全部PASS。六张小图覆盖空行、尾tile、S/M边界、无序、重复、部分公共run、交叉序列及抵消输入；保留原19值 ReLU/截断语义检查。FAST/ACCURATE、kernel_FP32/final_FP32/H1_BF16/final_BF16 四边界逐位一致；原数值阈值未放宽。完整逐边加法和 AMX 次数不变。')
add()
add('## 主性能：完整两层 prepared E2E，ms')
add()
add('ratio=PACK_EPI / StaticShared，>1 表示新路径更快。没有逐图择优或混合部署策略。')
add()
table(['图','PACK_EPI','StaticIndependent','StaticShared','DynamicSelected','ExactOnly','ratio'],[
 [g]+[f'{ms(g,m):.3f}' for m in ['pack_epi_degree','event_independent','event_share','dynamic_selected_share','exact_only_share']]+[f'{ms(g,"pack_epi_degree")/ms(g,"event_share"):.4f}'] for g in graphs])
for protocol in ['alternating','consecutive']:
    gm=math.exp(statistics.mean(math.log(ms(g,'pack_epi_degree',protocol)/ms(g,'event_share',protocol)) for g in graphs))
    add(f'固定 StaticShared 五图描述性 GM ({protocol})：{gm:.4f}×。')
add()
table(['图','Original TFS','源码 MKL FP32','P0','DynamicAll'],[
 [g]+[f'{ms(g,m):.3f}' for m in ['paper_tfs','source_mkl_fp32','p0_degree','dynamic_all_share']] for g in graphs])
add(f'共有{len(noisy)}个方法/协议CV>5%；见 timing_anomalies.csv。不能把共享节点小差异自动视为稳定论文加速。')
add()
add('## 覆盖率、准备成本与容量')
add()
table(['图','所有匹配/E','选中匹配/E','新增部分匹配/E','plan MiB','allocated MiB','构造 ms','审核 ms','乐观摊销次数'],[
 [r['graph'],f'{100*r["all_match_fraction"]:.6f}%',f'{100*r["eligible_fraction"]:.6f}%',f'{100*r["partial_fraction"]:.6f}%',
  f'{r["plan_bytes"]/2**20:.3f}',f'{r["allocated_bytes"]/2**20:.3f}',f'{r["construct_ms"]:.3f}',f'{r["audit_ms"]:.3f}',r['optimistic_setup_amortization_calls']] for r in coverage])
add('“所有匹配”仍仅是既有 DegreeSort 相邻配对、窗口和 greedy head 顺序约束下的匹配，并非图的复用上界。partial列不含完全相同窗口。摊销只用观测中位数差估算，未考虑噪声、不含双方共同的 DegreeSort，不能承诺真实 break-even。先在一核结构作业核对覆盖率，再运行同阈值32核五图；两次结构计数一致。')
add()
add('## 精细计时与工作量')
add()
table(['图','方法','Layer1','ReLU','独立Convert','Layer2','Total'],[
 [r['graph'],r['method']]+[f'{r[k]:.3f}' for k in ['layer1_ms','relu_ms','convert_ms','layer2_ms','total_ms']]
 for r in stage_summary if r['method'] in ['pack_epi_degree','event_independent','event_share','dynamic_selected_share']])
add('12个外层phase、24个细分字段与每线程active/finish/edges/tiles均保留。注意父快照中source load/decode只在外层 gather_decode 合测，24字段中的独立 load/decode 为未测占位零值；add已经另测。不能把这两个零值说成零成本。partial state、packing、AMX A/B/TDP/C、output、permission/config/release的已有独立计时保留。另建只用于诊断的快照补全load/decode拆分，附加结果单列，不替换本主性能快照。')
add()
add('细分结果来自 sampled/thread-local diagnostic instrumentation，仅标明采样范围中的线程时间，不是 wall 分解，不能加成 E2E 百分比，采样计时开销也可能改变流水。每个production profile与无插桩输出逐位一致。详见 raw/event_profile.csv、event_detail.csv、event_thread.csv；thread_imbalance.csv提供运行时间不均衡指标。')
add()
table(['图','每层真实边','静态共享省decode行','省decode比例','独立16BF16 ops','共享16BF16 ops','AMX TDP'],[
 [g,int(plans[g]['E']),int(plans[g]['eligible_matches']),f'{100*int(plans[g]["eligible_matches"])/int(plans[g]["E"]):.6f}%',
  int(plans[g]['E'])*8,(int(plans[g]['E'])-int(plans[g]['eligible_matches']))*8,
  next(r['tdp'] for r in counters if r['graph']==g and r['method']=='event_share' and r['layer']=='1')] for g in graphs])
add('这些是源级向量操作与逻辑读取请求，不是精确 retired decode 指令或实际 DRAM bytes。编译器可自然优化，未引入 volatile/barrier 强迫独立控制。每个真实 incidence 的128维加法仍执行；共享只省 source load/decode/部分当前CSR读取。prefetch请求、partial状态与packing、AMX工作量保持相同。')
add()
add('## PMU')
add()
add(f'成功有效采集 {len(usable)}/{len(pmu)} 次。只纳入32线程完整覆盖、无新增/失踪/零running线程、成功缩放的记录。cycles/instructions/cache_misses/cache_references为线程组总量；EXE.AMX_BUSY为原helper在SPR模型识别后采集的busy cycles，不是TDP数量或wall百分比。')
add()
table(['图','方法','instructions','cycles','IPC','generic cache misses','AMX busy'],[
 [r['graph'],r['method'],f'{r["instructions_scaled"]:.0f}',f'{r["cycles_scaled"]:.0f}',f'{r["IPC_ratio_of_counter_medians"]:.4f}',f'{r["cache_misses_scaled"]:.0f}',f'{r["amx_busy_scaled"]:.0f}'] for r in pmu_summary])
add('没有独立 L1/L2/LLC 分级、DRAM带宽或TLB归因证据，不能把缺失计数填成已证实的memory bound。PMU和sampled profile为另外独立诊断调用，不混入主表。')
add()
add('## 可证明与不能证明的结论')
add()
add('可证明：部分公共邻居的静态执行计划可以保持TFS和逐行浮点加法顺序，并达到四边界逐位一致；新增部分共享不改变projection数量及量化边界。真实图中固定DegreeSort相邻配对、原CSR顺序与压缩元数据约束显著限制其覆盖率，不能把集合相似度等同于可执行复用。')
add()
add('需按表逐图解释：StaticShared vs StaticIndependent检验相同计划的共享收益；StaticShared vs DynamicSelected检验相同选区的静态匹配收益；StaticShared vs ExactOnly观察新增部分窗口价值；vs PACK_EPI才是新增整体收益。RoadNet和wiki基本无新增覆盖，是元数据/分支控制；Products覆盖接近零，任何明显加速不能仅归因于省decode。Myciel拥有最高新增覆盖，但仍很小，FULL退化是否改变必须按实际总时间判断。')
add()
add('不能证明：这是强通用优化、是已满足期刊创新的独立方法、classification accuracy改善、所有匹配可独立省DRAM读取、PMU cache_misses等于带宽或FULL退化由decode单独主导。共享节点单轮结果不能替代独占重复实验。')
add()
add('后续设计应先量化“合法且可压缩的复用覆盖率”，再决定是否增加通用配对/重排开销。若需改变DegreeSort配对或CSR序列，必须作为另一实验同时控制padding、逐行加法顺序、预处理成本、矩阵投影不均衡及NUMA，不能把这些变化藏在本轮共享解码实现里。本轮不据五图结果回调8/8阈值，也不构造逐图最优oracle。')
add()
add('## 文件与复现')
add()
add('`source_snapshot/`：实际编译快照；`raw/ordered-*`：所有原始输出/命令/哈希/gates；`audit.json`：完整环境、manifest、counts；`timing_summary.csv`、`paired_comparisons.csv`、`coverage_and_setup.csv`、`stage_summary.csv`、`work_comparison.csv`、`pmu_summary.csv`、`correctness_summary.csv`、`thread_imbalance.csv`。原 frozen snapshot 未覆盖。')
(out/'GCN_EXTRA_ORDERED_SOURCE_PLAN_RESULTS.md').write_text('\n'.join(L)+'\n',encoding='utf-8')
print(json.dumps(dict(status='PASS',timing_records=len(raw),usable_PMU=len(usable),report=str(out/'GCN_EXTRA_ORDERED_SOURCE_PLAN_RESULTS.md')),indent=2))
