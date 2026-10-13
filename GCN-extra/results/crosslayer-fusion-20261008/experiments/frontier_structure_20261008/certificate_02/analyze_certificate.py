import csv
import hashlib
import json
from pathlib import Path
import subprocess

ROOT=Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
STUDY=ROOT/'runs/frontier-certificate-20261008-02'
RUN=STUDY/'compute-10899028'
PARENT=ROOT/'runs/frontier-structure-20261008-01/compute-10898893'
RESULT=ROOT/'results/frontier_structure_20261008/certificate-02'

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def save(path,obj):
    with path.open('x',encoding='utf-8') as f:json.dump(obj,f,ensure_ascii=False,indent=2);f.write('\n')

accounting=subprocess.check_output(['sacct','-n','-P','-j','10898893,10899017,10899028',
   '--format=JobID,State,ExitCode,Elapsed,NodeList'],text=True)
rows=[r.split('|') for r in accounting.splitlines() if r.strip()]
for job in ['10898893','10899028']:
    selected=[r for r in rows if r[0]==job or r[0].startswith(job+'.')]
    assert selected and all(r[1:3]==['COMPLETED','0:0'] for r in selected),selected
assert next(r for r in rows if r[0]=='10899017')[1:3]==['FAILED','1:0']
manifest=json.loads((STUDY/'snapshot_manifest.json').read_text())
completion=json.loads((RUN/'completion.json').read_text())
assert completion['status']=='PASS' and (RUN/'exit_status.txt').read_text().strip()=='0'
assert completion['source_manifest_sha256']==sha(STUDY/'snapshot_manifest.json')
assert completion['self_test']['status']=='PASS'
assert len(completion['self_test']['valid'])==5 and len(completion['self_test']['mutations'])==5
for n,d in manifest['source_sha256'].items():assert sha(STUDY/'source_snapshot'/n)==d,n
for p,d in manifest['parent_evidence_sha256'].items():assert sha(Path(p))==d,p
assert json.loads((PARENT/'completion.json').read_text())['status']=='PASS'

frontiers=[];bounds=[];states=[];total_verified=0
for g in completion['graphs']:
    name=g['graph'];folder=RUN/name
    assert sha(folder/'verified.json')==g['verified_sha256']
    assert sha(folder/'native/summary.json')==g['summary_sha256']
    verified=json.loads((folder/'verified.json').read_text());assert verified['status']=='PASS'
    native=json.loads((folder/'native/summary.json').read_text())
    assert verified['verifier_sha256']==manifest['source_sha256']['certificate_02/verify_certificate.py']
    selected=json.loads((folder/'selection.json').read_text())['frontiers']
    assert [f['p'] for f in verified['frontiers']]==selected==g['frontiers']
    for f,n in zip(verified['frontiers'],native['frontiers']):
        path=folder/'native'/n['certificate_file']
        assert sha(path)==f['certificate_sha256']
        assert path.stat().st_size==16*f['certificate_records_verified']
        total_verified+=f['certificate_records_verified']
        frontiers.append(dict(graph=name,p=f['p'],verified_matching_edges=f['certificate_records_verified'],
            lower_bound_bytes=f['certified_lower_bound_bytes'],
            rowcomplete_postdrain_upper_bytes=f['parent_rowlate_peak_after_drain_bytes'],
            lower_over_upper=f['lower_over_parent_rowlate_postdrain_peak'],
            certificate_sha256=f['certificate_sha256']))
    best=max(verified['frontiers'],key=lambda f:f['certified_lower_bound_bytes'])
    lower=best['certified_lower_bound_bytes'];upper=best['parent_rowlate_peak_after_drain_bytes']
    bounds.append(dict(graph=name,frontier=best['p'],certified_peak_lower_bound_bytes=lower,
        constructive_postdrain_peak_upper_bytes=upper,lower_over_upper=lower/upper,
        max_possible_postdrain_peak_saving_fraction=1-lower/upper,
        saving_is_upper_bound_not_achieved=True,globally_optimal_order_claim=False,
        restricted_representation=True,certified_frontiers=len(verified['frontiers'])))

for name in ['mycielskian19','reddit','roadNet-CA','amazon0601']:
    parent=json.loads((PARENT/name/'structure.json').read_text())
    node=next(o for o in parent['orders'] if o['name']=='nodeid')
    degree=next(o for o in parent['orders'] if o['name']=='degree')
    late=next(m for m in node['methods'] if m['name']=='row_complete')
    eager=next(m for m in node['methods'] if m['name']=='checkpoint_1')
    states.append(dict(graph=name,full_hidden_bytes=parent['full_hidden_reference_bytes'],
        rowcomplete_modeled_boundary_peak_bytes=late['peak_all_boundaries_bytes'],
        peak_ratio=late['peak_all_boundaries_bytes']/parent['full_hidden_reference_bytes'],
        eager_resumes=eager['resumes'],eager_logical_partial_io_bytes=eager['logical_partial_store_bytes']+eager['logical_partial_load_bytes'],
        nodeid_over_degree_s64_work_proxy=node['first_layer_work_proxy']['sum_tile_s64_windows']/degree['first_layer_work_proxy']['sum_tile_s64_windows']))

with (RESULT/'slurm_accounting.txt').open('x') as f:f.write(accounting)
with (RESULT/'frontier_certificates.csv').open('x',newline='') as f:
    writer=csv.DictWriter(f,fieldnames=list(frontiers[0]));writer.writeheader();writer.writerows(frontiers)
audit=dict(status='PASS',scope='GRAPH_STRUCTURE_AND_RESTRICTED_STATE_CERTIFICATE',
    parent_job='10898893',failed_build_job_retained='10899017',certificate_job='10899028',
    parent_legal_fixtures=81,parent_expected_rejections=6,parent_real_plans=60,
    certificate_legal_fixtures=5,certificate_expected_rejections=5,real_frontiers=len(frontiers),
    certificate_occurrences_verified=total_verified,bounds=bounds,parent_state_comparison=states,
    completion_sha256=sha(RUN/'completion.json'),snapshot_manifest_sha256=sha(STUDY/'snapshot_manifest.json'),
    paper_performance_eligible=False,desktop_write=False,numa_interleave=False,
    interpretation_limits=['fixed node-ID producer order','only whole BF16 sources or independent FP32 destination partials',
      'no recomputation or cross-destination factorization','source/partial boundary-state model, not total process memory',
      'greedy matching need not be maximum','remaining gap is not attainable savings',
      'no measured AVX-AMX overlap, speedup, DRAM traffic or task accuracy'])
save(RESULT/'audit.json',audit)
mi=lambda x:f'{x/(1024*1024):.3f}'
lines=['# 2026-10-08：跨层状态实验与下一步取舍','',
'主配置仍为 128→128→128；本轮只审计图结构与存储状态，不执行特征内核。未使用 NUMA interleave，未改变 first-touch，未向桌面写文件。', '',
'## 已完成的实验','',
'- 结构作业 10898893：COMPLETED 0:0，81 个合法小图、6 个预期拒绝、4 图 × 3 排列 × 5 消费方案通过。',
'- 证书初次作业 10899017：链接失败，GCC 8 的 filesystem 需要 -lstdc++fs；所有日志保留。',
'- 修复后的证书作业 10899028：COMPLETED 0:0，5 个合法小图、5 类损坏证书拒绝、2 图共 12 个前沿通过独立验证。',
f'- 真实图证书共核验 {total_verified:,} 条 CSR occurrence。作业顺序执行，最多占用 1 个共享节点、1 个 CPU。','',
'## 结构结果','',
'下表的峰值是每个生产批次前、生产后、消费后的三个边界状态最大值，排除了内核 scratch、元数据、批次内部瞬时峰值；不能解释成实测进程内存或速度。第一层工作数只是代理。','',
'| 图 | 完整 H1 / MiB | nodeID＋整行就绪消费峰值 / MiB | 相对完整 H1 | nodeID/degree S64 工作代理 |',
'|---|---:|---:|---:|---:|']
for s in states:
    lines.append(f"| {s['graph']} | {mi(s['full_hidden_bytes'])} | {mi(s['rowcomplete_modeled_boundary_peak_bytes'])} | {s['peak_ratio']:.4%} | {s['nodeid_over_degree_s64_work_proxy']:.5f} |")
lines+=['',
'Road 与 Amazon 的简单整行就绪消费已明显减少状态；需要实际内核才能判断收益。Myc 与 Reddit 仍接近保存完整 H1。把源一到就吸收到 FP32 partial 中并不免费：对应 eager 的逻辑 partial 保存／恢复分别约 280 GiB 与 103 GiB。这些是逻辑载荷，不是实测 DRAM 流量。', '',
'同一排列下，整行就绪方案在峰值、恢复次数、状态面积三个指标上严格支配 48 个 checkpoint 对照中的 43 个。不能说全部支配：Myc/nodeID 的 ALL 因 ready 尾部少 3072 B 而支配整行就绪；Myc 的 degree/qguard 中 checkpoint16/ALL 与整行就绪存在小幅峰值／面积权衡。这些例外保留在首轮 comparison.json。','',
'## 可验证的下界','',
'固定 nodeID 生产顺序。在前沿 p 选择实际边，每个已生产源最多出现一次，每个尚未完成的目的最多出现两次。源向量成本 256 B，目的 FP32 partial 成本 512 B，因此 M 条合法选择边要求至少 256M B 状态。验证器直接检查 CSR occurrence、生产与完成状态、唯一性和容量；无需相信原生程序的最优匹配声明。','',
'| 图 | 已证峰值下界 / MiB | 简单方案消费后峰值 / MiB | 下界/简单方案 | 尚不能排除的最多节省 |',
'|---|---:|---:|---:|---:|']
for b in bounds:
    lines.append(f"| {b['graph']} | {mi(b['certified_peak_lower_bound_bytes'])} | {mi(b['constructive_postdrain_peak_upper_bytes'])} | {b['lower_over_upper']:.4%} | {b['max_possible_postdrain_peak_saving_fraction']:.4%} |")
lines+=['',
'Reddit：这个受限模型中，单纯源／partial 混合至多还能降低约 4.45% 的消费后峰值，且未计调度成本；据此不优先开发复杂运行时。',
'Mycielskian19：68.26% 是一个合法但不够紧的下界，不能推出剩余 31.74% 可实现，也不能宣布简单方案接近最优。不要把构造图单例直接提升为普遍方法。','',
'这个下界不限制不同生产排列、重算、跨目的代数共享、其他精度或其他状态表示；它也不保证所有前沿的独立最优选择能拼成合法执行计划。', '',
'## 对创新主线的影响','',
'目前没有证据支持将“更频繁分组、尽早消费”包装成新加速方法。更具体的问题是：什么时候 AVX 的归约真正消除了未来仍需保存的信息，什么时候只是把便宜的 BF16 源换成更昂贵且反复恢复的 FP32 状态？',
'一次合法状态转换若释放 F 个源、新建 A 个 partial、完成 C 个已有 partial、ready 数净变化为 ΔR，则边界字节变化为 -256F+512(A-C)+256ΔR。这是方法约束，不是独立创新声明。', '',
'下一步只保留两个可被证伪的问题：',
'1. 在下界仍有差距的图上，是否存在能沿时间执行、又保持 CSR 顺序及 AMX 16 行批次的状态转换计划？先证明优于 nodeID＋整行就绪，而非仅优于 eager；单个前沿的最小覆盖不等于可执行调度。',
'2. 若需打破大前沿，有限按需重新生成 H1 是否划算？必须同时计入额外第一层边访问、AVX 归约、AMX 投影和不足 16 行的 padding。128→128 时，把 checkpoint 前移到 BF16 聚合结果仍是每行 256 B，不会自动减少容量。', '',
'不继续扫描固定 checkpoint 参数；不把匹配算法、ready 队列或源缓存本身当成论文贡献。若后续只得到通用缓存／重算启发式而没有新的计算组织与跨图证据，应停止这条候选。','',
'## 文献边界','',
'- [TileFusion，ICS 2025](https://www.cheshmi.cc/KazemCheshmi_files/tile_fusion_ics25.pdf) 已研究稀疏依赖、可变 tile 与部分安全融合。仅按依赖提前消费不足以形成区别。',
'- [SuperGCN，ICS 2025](https://hpcrl.github.io/ICS2025-webpage/program/Proceedings_ICS25/ics25-62.pdf) 是最小顶点覆盖相关选择的直接近邻；本轮容量二匹配只用于研究证伪。',
'- [NeutronStar，SIGMOD 2022](https://xiaodongzhang1911.github.io/Zhang-papers/TR-22-1.pdf) 已研究依赖的重算／通信选择。把该思想移到 CPU 并非不能做，但需要证明当前数值路径、源／partial 转换和 AMX 批次约束促成了实质的新机制。','',
'## 证据路径','',f'- 首轮：`{PARENT}`',f'- 证书：`{RUN}`',f'- 逐前沿与审核：`{RESULT}`',
'- 首轮完整 60 方案：`results/frontier_structure_20261008/analysis-01/plan_summary.csv`',
'- 失败记录：`runs/frontier-certificate-20261008-01/compute-10899017`','',
'结论状态：结构与证书成立；期刊新方法、端到端加速和任务精度仍未验证。','']
with (RESULT/'RESEARCH_REVIEW.md').open('x',encoding='utf-8') as f:f.write('\n'.join(lines))
event=dict(id='frontier-certificate-analysis-20261008-02',status='ANALYZED_PASS',evidence=str(RESULT),
    audit_sha256=sha(RESULT/'audit.json'),report_sha256=sha(RESULT/'RESEARCH_REVIEW.md'),
    scope=audit['scope'],paper_eligible=False,bounds=bounds,
    issues='Retained link failure10899017 resolved by explicit -lstdc++fs in immutable retry10899028. New negative research result: Reddit fixed-order source/partial peak has at most 4.4517% saving relative to simple postdrain upper bound; Myc lower bound is not tight. No hardware speedup proven.',
    next='Do not develop periodic prefix-drain runtime or scan checkpoint sizes. Only investigate reachable state transformations where a certified gap remains, or explicit recomputation that counts additional AVX/AMX work; compare against row-complete baseline and direct priors.',
    desktop_write=False,numa_interleave=False)
save(RESULT/'event.json',event)
subprocess.run(['python3',str(ROOT/'scripts/record_event.py'),str(RESULT/'event.json')],check=True)
subprocess.run(['python3',str(ROOT/'scripts/record_event.py'),'check'],check=True)
print(json.dumps(dict(status='PASS',bounds=bounds,real_frontiers=len(frontiers),records=total_verified,result=str(RESULT)),ensure_ascii=False))
