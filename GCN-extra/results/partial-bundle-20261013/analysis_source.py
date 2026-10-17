#!/usr/bin/env python3
"""Read-only reconciliation of frozen raw runs; no new benchmark execution."""
import csv
import hashlib
import json
import math
import pathlib
import statistics
import sys

build = pathlib.Path(sys.argv[1])
out = pathlib.Path(sys.argv[2])
out.mkdir(parents=True, exist_ok=True)
perf = build / 'logs/bundle-10944719'
smoke = build / 'logs/bundle-10944718'


def rows(p):
    return list(csv.DictReader(p.open()))


def write(p, rr):
    if not rr:
        return
    with p.open('w', newline='') as f:
        w = csv.DictWriter(f, list(dict.fromkeys(k for r in rr for k in r)))
        w.writeheader()
        w.writerows(rr)


complete = json.loads((perf / 'completion.json').read_text())
small = json.loads((smoke / 'completion.json').read_text())
assert complete['status'] == small['status'] == 'PASS'
assert len(complete['graphs']) == len(small['graphs']) == 5
for run in [perf, smoke]:
    for name in ['method_check', 'state_equiv', 'state_semantic', 'state_work_gate', 'state_profile_gate']:
        rr = rows(run / (name + '.csv'))
        assert rr and all(r['pass'] == '1' for r in rr), (run, name)

raw = rows(perf / 'state_time.csv')
groups = {}
for r in raw:
    groups.setdefault((r['graph'], r['method'], r['kind']), []).append(float(r['ms']))
assert len(raw) == 1725
stats = {}
for key, v in groups.items():
    assert len(v) == (10 if key[2] == 'alternating' else 5)
    stats[key] = {'median_ms': statistics.median(v), 'min_ms': min(v), 'max_ms': max(v),
                  'CV': statistics.stdev(v) / statistics.mean(v), 'repeats': len(v)}
summary = [dict(graph=k[0], method=k[1], protocol=k[2], **s) for k, s in stats.items()]
write(out / 'timing_summary.csv', summary)
graphs = ['ogbn-products', 'reddit', 'mycielskian19', 'roadNet-CA', 'wiki-Talk']


def get(g, m, protocol='alternating'):
    return stats[g, m, protocol]['median_ms']


paired = []
for graph in graphs:
    for scope in ['m64', 'm256', 'full']:
        for precision in ['fast', 'accurate']:
            tag = scope + '_' + precision
            control = 'pack_epi_' + tag
            for b in ([1, 2, 4] if scope != 'full' else [4]):
                method = 'bundle_g' + str(b) + '_' + tag
                for protocol in ['alternating', 'consecutive']:
                    ref, cand = stats[graph, control, protocol], stats[graph, method, protocol]
                    paired.append(dict(graph=graph, scope=scope, precision=precision, G=b, protocol=protocol,
                                       control_ms=ref['median_ms'], candidate_ms=cand['median_ms'],
                                       ratio_control_over_candidate=ref['median_ms']/cand['median_ms'],
                                       control_CV=ref['CV'], candidate_CV=cand['CV'],
                                       noisy=(ref['CV']>.05 or cand['CV']>.05)))
write(out / 'paired_comparisons.csv', paired)
noisy = [r for r in summary if r['CV'] > .05]
write(out / 'timing_anomalies.csv', noisy)

model = {(r['graph'], r['method']): r for r in rows(perf / 'bundle_model.csv')}
work = []
for graph in graphs:
    for scope in ['m64', 'm256', 'full']:
        a, b = model[graph, 'pack_epi_'+scope+'_fast'], model[graph, 'bundle_g4_'+scope+'_fast']
        old = int(a['C_store_bytes']) + int(a['C_reload_bytes'])
        new = int(b['C_store_bytes']) + int(b['C_reload_bytes'])
        work.append(dict(graph=graph, scope=scope, N=int(a['N']), E=int(a['E']),
                         projection_scopes=int(a['scopes']), source_bytes_per_layer=int(a['source_bytes']),
                         C_bytes_control_per_layer=old, C_bytes_G4_per_layer=new,
                         C_requested_byte_reduction_fraction=1-new/old,
                         saved_C_bytes_over_source_requested_bytes=(old-new)/int(a['source_bytes']),
                         tdp_per_layer=int(a['tdp']), source_or_TDP_reduction=0,
                         units='logical_requested_not_DRAM'))
write(out / 'work_comparison.csv', work)

pmu = rows(perf / 'method_pmu.csv')
usable = [r for r in pmu if all(r[k]=='1' for k in ['available','coverage_complete','scaled_complete'])
          and all(r[k]=='32' for k in ['initial_threads','final_threads','counted_threads','scaled_threads'])
          and all(r[k]=='0' for k in ['failed_threads','zero_running_threads','new_threads','disappeared_threads'])]
pr = []
for graph in graphs:
    for method in ['pack_epi_m64_fast','bundle_g4_m64_fast','pack_epi_full_fast']:
        rr=[r for r in usable if r['graph']==graph and r['method']==method]
        if not rr:
            continue
        rec=dict(graph=graph,method=method,repeats=len(rr))
        for k in ['cycles_scaled','instructions_scaled','cache_misses_scaled','cache_references_scaled','amx_busy_scaled']:
            rec[k]=statistics.median(float(r[k]) for r in rr)
        rec['IPC_ratio_of_counter_medians']=rec['instructions_scaled']/rec['cycles_scaled']
        pr.append(rec)
write(out / 'pmu_summary.csv', pr)

stages = rows(perf / 'bundle_stage.csv')
stage_summary = []
for graph in graphs:
    for method in sorted({r['method'] for r in stages if r['graph']==graph}):
        rr=[r for r in stages if r['graph']==graph and r['method']==method]
        rec=dict(graph=graph,method=method,epilogue=rr[0]['epilogue'])
        for k in ['layer1_ms','relu_ms','convert_ms','layer2_ms','total_ms']:
            rec[k]=statistics.median(float(r[k]) for r in rr)
        stage_summary.append(rec)
write(out / 'stage_summary.csv', stage_summary)

audit={'status':'PASS','scope':'existing_raw_result_analysis_not_new_execution',
       'build':str(build),'smoke_job':'10944718','performance_job':'10944719',
       'performance_node':'qhcn198','partition':'intel_shared','threads':32,
       'source_manifest':json.loads((build/'manifest.json').read_text()),
       'smoke_counts':small['counts'],'diagnostic_counts':complete['counts'],
       'raw_timing_records':len(raw),'pmu_records':len(pmu),'usable_pmu_records':len(usable),
       'noisy_method_protocol_records':len(noisy),'paper_eligible':False,
       'analysis_source_sha256':hashlib.sha256(pathlib.Path(__file__).read_bytes()).hexdigest()}
(out/'audit.json').write_text(json.dumps(audit,indent=2)+'\n')

L=[]
def add(s=''):
    L.append(s)
def table(header, rr):
    add('| '+' | '.join(header)+' |');add('| '+' | '.join(['---']*len(header))+' |')
    for r in rr:add('| '+' | '.join(map(str,r))+' |')
    add()

add('# 保持量化边界的 partial bundle：五图实现与实验结果')
add()
add('分析日期：2026-10-14。GCN-extra 静态单位边权、随机 H/W seed12345、128→128→128，两层前向；不是 checkpoint 分类实验。')
add()
add('## 结论')
add()
add('新 G=1/2/4 路径已实现、编译并完成小图与五张诊断图验证。保持 DegreeSort/TR16/R64、S64、M64/M256/FULL、FAST/ACCURATE、BF16 截断、原权重打包和 AMX 每个输出元素的投影顺序。只调整已量化 partial 的缓冲容量、输出 panel 循环和 C 暂存频率；原 TFS/MKL 与 frozen 依赖不修改。')
add()
add('五图增量总体较小，没有形成值得立即扩展17图的强通用收益。减少 C 暂存请求已经实现，但未减少逐边 source 读取/解码/加法或 AMX 投影。FULL 的单 partial 控制也存在时间变化，所以不能把全部差异归因于 C traffic。')
add()
add('## 环境与完整性')
add()
add('build：`partial-bundle-build-20261013-225110`。smoke Job10944718/qhcn015，一物理核，9秒；performance Job10944719/qhcn198，32物理核、单socket、close/cores、默认NUMA、无interleave，共享intel，25分22秒。两作业 COMPLETED 0:0。共享结果为探索性证据，`paper_eligible=false`。')
add()
add('PASS：五图440数值门槛、420逐位边界检查、200源级工作量检查、200production插桩一致性检查；另有五张小图、原19值EPI语义、抵消及bundle尾部检查。`audit.json`保存全部计数。')
add()
add('同精度同M的kernel FP32、最终FP32、H1 BF16、最终BF16均逐位一致。相对原TFS的FAST FP32门槛.01、ACCURATE .001，BF16分别.02/.01；相对原FP32 MKL BF16门槛.03未放宽。')
add()
add('## 改动与真实数据流')
add()
add('```text\n原控制：归约一个 M partial→独立 hi/lo 截断→两个panel投影→保存C\n新方法：分别归约/截断 G 个 M partial→一个panel依次消费这些partial→保存C→另一个panel\n```')
add()
add('G不会将多个partial合并后再量化。各输出元素仍按原scope/KB/hi-lo顺序累加；生产Layer1使用已验证PACK_EPI直接写H1b。FULL每tile只有一个有效partial，G4不减少其C存取，是必要负控制。')
add()
add('## 主性能：完整两层，10次正反顺序中位数，ms')
add()
add('ratio=同M同精度PACK_EPI时间/候选时间，>1表示候选更快。未使用逐图最快G形成方法，G2/G4固定策略分别列出。完整两协议结果在paired_comparisons.csv。')
add()
for precision in ['fast','accurate']:
    for scope in ['m64','m256']:
        add('### '+scope.upper()+' '+precision.upper());add()
        rr=[]
        for g in graphs:
            tag=scope+'_'+precision;c=get(g,'pack_epi_'+tag)
            vals=[get(g,'bundle_g'+str(b)+'_'+tag) for b in [1,2,4]]
            rr.append([g,f'{c:.3f}']+[f'{v:.3f}' for v in vals]+[f'{c/vals[1]:.4f}',f'{c/vals[2]:.4f}'])
        table(['图','强控制','G1','G2','G4','控制/G2','控制/G4'],rr)
        for b in [2,4]:
            ratios=[get(g,'pack_epi_'+scope+'_'+precision)/get(g,'bundle_g'+str(b)+'_'+scope+'_'+precision) for g in graphs]
            gm=math.exp(sum(map(math.log,ratios))/len(ratios))
            add(f'固定G{b}五图描述性GM：{gm:.4f}×。')
        add()
add('### FULL：没有C暂存减少的控制');add()
table(['图','FAST控制','FAST G4','ratio','ACCURATE控制','ACCURATE G4','ratio'],[
 [g,f'{get(g,"pack_epi_full_fast"):.3f}',f'{get(g,"bundle_g4_full_fast"):.3f}',f'{get(g,"pack_epi_full_fast")/get(g,"bundle_g4_full_fast"):.4f}',
    f'{get(g,"pack_epi_full_accurate"):.3f}',f'{get(g,"bundle_g4_full_accurate"):.3f}',f'{get(g,"pack_epi_full_accurate")/get(g,"bundle_g4_full_accurate"):.4f}'] for g in graphs])
add('FULL的变化不能由减少C store/reload解释。代码生成、控制流程、地址/缓存状态、共享节点噪声等尚未被逐项隔离；不提前指定唯一原因。')
add()
add('## 原TFS/MKL锚点与累积收益');add()
table(['图','源TFS ms','源MKL FP32 ms','PACK_EPI FULL FAST ms'],[
 [g,f'{get(g,"paper_tfs"):.3f}',f'{get(g,"source_mkl_fp32"):.3f}',f'{get(g,"pack_epi_full_fast"):.3f}'] for g in graphs])
add('源TFS/MKL保持原实现；它们精度不同。已有邻居归约/EPI收益不得归给本轮bundle。增量归因只能使用上面的同M同精度控制。')
add()
add('## 工作量与逻辑访存');add()
add('以下单层请求，不是实测DRAM。每层source=256E bytes，decode=8E组，FP32 scalar add=128E；TDP保持32Q/64Q。C原请求=8192(2Q-1)/tile，新请求=8192(2ceil(Q/G)-1)。')
add()
table(['图/M','source GB','C控制 GB','C G4 GB','C减少','所省C/source'],[
 [r['graph']+'/'+r['scope'],f"{r['source_bytes_per_layer']/1e9:.3f}",f"{r['C_bytes_control_per_layer']/1e9:.3f}",f"{r['C_bytes_G4_per_layer']/1e9:.3f}",
  f"{100*r['C_requested_byte_reduction_fraction']:.2f}%",f"{100*r['saved_C_bytes_over_source_requested_bytes']:.2f}%"] for r in work if r['scope']=='m64'])
add('即使C相对自身大幅下降，对source请求的节省也有限，且C主要是局部状态。不能将两种逻辑bytes按相同DRAM成本相加。bundle扩大hi/lo存活工作集，也可能抵消收益。model中的packed_bundle_capacity是活跃表示容量；代码保留hi/lo数组，不能当作实际stack/RSS测量。')
add()
add('## Stage与细分计时');add()
ss={(r['graph'],r['method']):r for r in stage_summary}
table(['图','方法','Layer1 ms','ReLU/Convert','Layer2 ms','Stage Total ms'],[
 [g,m,f"{ss[g,m]['layer1_ms']:.3f}",'fused inside Layer1',f"{ss[g,m]['layer2_ms']:.3f}",f"{ss[g,m]['total_ms']:.3f}"]
 for g in graphs for m in ['pack_epi_m64_fast','bundle_g2_m64_fast','bundle_g4_m64_fast']])
add('独立3-repeat stage，不与主计时混分母；每列分别取中位数，段之和可能不同于total中位数。EPI中的ReLU/转换不是没有工作，而是被计入Layer1。')
add()
add('保存12粗阶段、24采样子阶段，以及每线程active/start/finish/edges/tiles。子项覆盖source load+decode、FP32 add、partial状态、hi/lo packing、AMX A/B load、TDP、C zero/load/store、EPI/store及线程setup。原始4800细分记录与6400线程记录都保留。')
add()
add('这些是独立插桩版本的sampled/thread-local diagnostic time；有父子重叠、时钟开销和代码调度变化，不能求和当E2E wall百分比，更不能仅由AMX compute timer宣称硬件重叠。')
add()
add('## PMU');add()
add(f'独立完整forward PMU：{len(usable)}/{len(pmu)}条通过32线程覆盖与scaling检查，每方法3次。记录cycles/instructions/generic cache misses/references与AMX speculative busy cycles；没有L1/L2/LLC细分、DRAM BW或TLB。初末线程快照不能证明覆盖所有瞬时线程。')
add()
table(['图/方法','cycles B','instructions B','IPC','generic misses M','AMX busy M'],[
 [r['graph']+'/'+r['method'],f"{r['cycles_scaled']/1e9:.3f}",f"{r['instructions_scaled']/1e9:.3f}",f"{r['IPC_ratio_of_counter_medians']:.3f}",
 f"{r['cache_misses_scaled']/1e6:.3f}",f"{r['amx_busy_scaled']/1e6:.3f}"] for r in pr if r['method']!='pack_epi_full_fast'])
add('以上为线程求和counter中位数，IPC为counter中位数之比。AMX busy是speculative arithmetic busy cycles，不是退役TDP数或AMX wall time。PMU region包含runtime；启停有时间跨度，不能直接当严格内核占比。')
add()
add('## 稳定性、逐图解释与下一步');add()
add(f'两个主协议共{len(summary)}条方法汇总，其中{len(noisy)}条CV>5%，原样保留在timing_anomalies.csv。不称小幅差异具有统计显著性。')
add()
add('- Products：固定G的改善小；FULL负控制也变化，不能证明C优化是主要因素。')
add('- Reddit：M64下G2 FAST和G4 ACCURATE有较明显正增量，但应面对FULL同精度强控制与独立复测，不能推广为通用策略。')
add('- Myciel：C逻辑请求明显减少，主时间仍约秒级，说明本轮没有解决逐边重负载；具体硬件原因不唯一。')
add('- RoadNet：大多数tile仅一个scope，bundle基本没有C存取可省，是自然负对照。')
add('- wiki-Talk：部分改善，需结合偏斜/空行与两协议波动；不能用非空destination数量替代真实source工作。')
add()
add('当前不将bundle作为默认取代FULL的路径，也不立即铺17图。保留独立实现，作为量化边界与C驻留分离的已验证机制对照。下一阶段更应研究逐边source load/decode复用与partial寄存器状态的交易；已有PAIR/exact-window负结果必须作为控制。此结论不是证明任何更强bundle调度都无效。')
add()
add('## 保存与复现');add()
add('源：build/source_snapshot；精确命令与原依赖hash：build/commands.txt、source.sha256、manifest.json。raw日志、全部CSV、affinity/NUMA/environment/graph hashes：logs/bundle-10944718与10944719。分析只读原记录，analysis脚本单独冻结；没有重写raw或源baseline。')
add()
add('派生文件：timing_summary.csv、paired_comparisons.csv、timing_anomalies.csv、work_comparison.csv、stage_summary.csv、pmu_summary.csv、audit.json。本报告不代表GitHub已提交。')
(out/'GCN_EXTRA_PARTIAL_BUNDLE_RESULTS.md').write_text('\n'.join(L)+'\n',encoding='utf-8')
print(json.dumps({'status':'PASS','report':str(out/'GCN_EXTRA_PARTIAL_BUNDLE_RESULTS.md'),
                  'timing_records':len(raw),'usable_pmu':len(usable),'noisy_records':len(noisy)}))
