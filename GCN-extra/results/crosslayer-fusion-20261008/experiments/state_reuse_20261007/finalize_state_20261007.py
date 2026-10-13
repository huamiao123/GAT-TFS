#!/usr/bin/env python3
"""Read frozen experiment evidence; create a new final audit and Chinese report."""
import csv, datetime, hashlib, json, math, pathlib, shutil, statistics, subprocess

ROOT = pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
EXP = ROOT / 'experiments/state_reuse_20261007'
RUN = ROOT / 'runs/state-reuse-20261007-05'
OUT = ROOT / 'results/state_reuse_20261007'
FINAL = OUT / 'final'
JOBS = ['10888825','10888828','10888829','10888830','10888831','10888832',
        '10888847','10888848','10888849','10888850']
GRAPHS = ['ogbn-products','reddit','mycielskian19','hollywood-2009','indochina-2004',
          'soc-Pokec','cit-Patents','soc-LiveJournal1','com-LiveJournal','as-Skitter',
          'rgg_n_2_24_s0','web-Google','roadNet-CA','wiki-Talk','email-Enron','amazon0601','com-Youtube']
DIAG = ['mycielskian19','reddit','roadNet-CA','amazon0601']

def sha(p):
    h = hashlib.sha256()
    with pathlib.Path(p).open('rb') as f:
        for b in iter(lambda: f.read(1048576), b''): h.update(b)
    return h.hexdigest()

def rows(p):
    with pathlib.Path(p).open(newline='') as f: return list(csv.DictReader(f))

def write_json(p, obj):
    p.write_text(json.dumps(obj, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')

def verify_hash_lines(s):
    ans = {}
    for line in s.splitlines():
        if not line.strip(): continue
        expected, filename = line.split(None, 1)
        actual = sha(filename)
        assert actual == expected, ('HASH_MISMATCH', filename, actual, expected)
        ans[filename] = actual
    return ans

assert not FINAL.exists(), 'Final directory must be new; preserve prior results'
assert set(JOBS) == {p.name.split('-')[-1] for p in (RUN/'logs').iterdir() if p.is_dir()}
for job in JOBS:
    p = RUN/'logs'/('state-'+job)
    c = json.loads((p/'completion.json').read_text())
    assert c['failed'] == 0 and c['passed'] == c['graphs'], (job,c)
    assert (p/'exit_status.txt').read_text().strip() == '0', job

subprocess.run(['python3',str(ROOT/'scripts/record_event.py'),'check'],check=True)
FINAL.mkdir()
preflight = {'time':datetime.datetime.now().isoformat(),'queue':subprocess.check_output(
    ['squeue','-u','hdacp1','-o','%i %P %j %T %M %D %R'],text=True),'hashes':{}}
for src, name in [(ROOT.parent/'AGENTS.md','AGENTS.read.txt'),
                  (ROOT.parent/'skills/tfs-research-engineering/SKILL.md','SKILL.read.txt'),
                  (ROOT/'IMPLEMENTATION_CONTRACT.md','contract.read.txt'),
                  (EXP/'analyze_state.py','analyze_state.py'),
                  (EXP/'assembly_audit.json','assembly_audit.json'),
                  (pathlib.Path(__file__),'finalize_state_20261007.py'),
                  (RUN/'source_snapshot/PROTOCOL.md','PROTOCOL.md')]:
    shutil.copyfile(str(src),str(FINAL/name)); preflight['hashes'][name] = sha(FINAL/name)
assert preflight['hashes']['analyze_state.py'] == '07ff8813f9d5cf27b976b64a5bae43f49e3abe5a625e90fb062dafe20da7d882'
preflight['analysis_snapshot_note'] = ('The build captured a 10000-byte intermediate analyzer. '
    'It was not used for analysis-01 or final. The completed 35654-byte analyzer is independently '
    'frozen here. All run05 build/source snapshots remain unchanged.')
write_json(FINAL/'preflight.json',preflight)
manifest = json.loads((RUN/'manifest.json').read_text())
sources = verify_hash_lines(manifest['source_hashes'])
binaries = verify_hash_lines(manifest['binary_hashes'])
assert manifest['dimensions'] == [128,128,128] and not manifest['numa_interleave']
sacct = subprocess.check_output(['sacct','-j',','.join(JOBS),'-X','-P','-n',
    '--format=JobID,Partition,State,ExitCode,Start,End,Elapsed,AllocNodes,NodeList'],text=True)
(FINAL/'sacct.txt').write_text(sacct)
accounting = {l.split('|')[0]:l.split('|') for l in sacct.splitlines() if l.strip()}
assert set(accounting) == set(JOBS), accounting
job_audits=[]; gate_counts={}; timing_rows=0
for job in JOBS:
    a=accounting[job]; assert a[2]=='COMPLETED' and a[3]=='0:0' and int(a[7])==1, a
    p=RUN/'logs'/('state-'+job)
    affinity=json.loads((p/'affinity.json').read_text()); t=affinity['topology']
    assert len(t)==32 and len({(x['socket'],x['core']) for x in t})==32
    assert len({x['socket'] for x in t})==1
    env=json.loads((p/'environment.json').read_text())
    assert env['OMP_NUM_THREADS']=='32' and env['OMP_PROC_BIND']=='close' and env['OMP_PLACES']=='cores'
    assert all(env.get(k) is None for k in ['MKL_NUM_THREADS','MKL_DYNAMIC','MKL_DOMAIN_NUM_THREADS'])
    policy=(p/'numa_policy.txt').read_text(); assert 'policy: default' in policy
    assert (p/'source.sha256').read_bytes()==(RUN/'source.sha256').read_bytes()
    verify_hash_lines((p/'build_files.sha256').read_text())
    counts={}
    for name in ['method_check','state_equiv','state_work_gate','state_profile_gate','state_semantic']:
        rs=rows(p/(name+'.csv')); assert all(x.get('pass')=='1' for x in rs), (job,name)
        counts[name]=len(rs); gate_counts[name]=gate_counts.get(name,0)+len(rs)
    if (p/'state_time.csv').exists(): timing_rows += len(rows(p/'state_time.csv'))
    job_audits.append({'job':job,'partition':a[1],'state':a[2],'exit':a[3],
        'node':a[8],'start':a[4],'end':a[5],'elapsed':a[6],
        'physical_cores':32,'sockets':1,'numa_policy':policy,'gates':counts,
        'completion':json.loads((p/'completion.json').read_text())})
pmu=[]
for job in JOBS:
    p=RUN/'logs'/('state-'+job)/'method_pmu.csv'
    if p.exists():
        for r in rows(p):
            assert all(r[x]=='1' for x in ['available','coverage_complete','scaled_complete'])
            assert all(r[x]=='32' for x in ['initial_threads','final_threads','counted_threads','scaled_threads'])
            assert all(r[x]=='0' for x in ['failed_threads','zero_running_threads','new_threads','disappeared_threads'])
            r['job']=job; r['partition']=accounting[job][1]; pmu.append(r)
launcher_events=[json.loads(p.read_text()) for p in (RUN/'launcher').glob('*/event.json')]
peak_requested=max(e['requested_nodes_before']+1 for e in launcher_events)
assert peak_requested<=10
subprocess.run(['python3',str(FINAL/'analyze_state.py'),str(RUN),str(FINAL/'analysis')],check=True)
audit=json.loads((FINAL/'analysis/audit.json').read_text())
assert audit['analysis_pass'] and audit['failed_graph_jobs']==0 and audit['graph_job_packets']==34, audit.keys()
cohort=json.loads((FINAL/'analysis/cohort_summary.json').read_text())
assert any(c['partition']=='intel' and c['graphset']=='all17' and c['complete'] for c in cohort['coverage'])
assert any(c['partition']=='intel_expr' and c['graphset']=='diag4' and c['complete'] for c in cohort['coverage'])
summary=rows(FINAL/'analysis/summary.csv')
eligible=[r for r in summary if r['eligible_for_performance']=='1']
def entry(g,m,part='intel',kind='alternating'):
    rr=[r for r in eligible if r['graph']==g and r['method']==m and r['partition']==part and r['timing_kind']==kind]
    assert len(rr)==1,(g,m,part,kind,len(rr))
    assert rr[0]['attempt_role']=='primary' and rr[0]['exclusive']==str(part=='intel_expr').lower()
    return rr[0]
def get(g,m,part='intel',kind='alternating'):return float(entry(g,m,part,kind)['median_ms'])
def ratio(g,m,ref,part='intel',kind='alternating'):
    a,b=entry(g,m,part,kind),entry(g,ref,part,kind)
    assert a['job']==b['job'] and a['build_fingerprint']==b['build_fingerprint']
    return float(b['median_ms'])/float(a['median_ms'])
def gm(gs,m,ref,part='intel',kind='alternating'):
    rs=[ratio(g,m,ref,part,kind) for g in gs]
    return math.exp(sum(map(math.log,rs))/len(rs)),sum(x>1 for x in rs),sum(x<1 for x in rs)

root_audit={'status':'PASS','run':str(RUN),'dimensions':[128,128,128],
    'source_hashes':sources,'binary_hashes':binaries,'gate_totals':gate_counts,
    'raw_timing_rows':timing_rows,'graph_job_packets':audit['graph_job_packets'],
    'performance_records':audit['performance_records'],'max_requested_nodes_at_submission':peak_requested,
    'pmu_regions_coverage_passed':len(pmu),
    'jobs':job_audits,'analysis_source_sha256':sha(FINAL/'analyze_state.py'),
    'assembly_audit_sha256':sha(FINAL/'assembly_audit.json'),
    'snapshot_analyzer_caveat':preflight['analysis_snapshot_note']}
write_json(FINAL/'FINAL_AUDIT.json',root_audit)

L=[]
def add(s=''): L.append(s)
add('TFS 128维状态寿命、层间融合与共享解码：实验计划及技术报告')
add('日期：2026-10-07；状态：本轮全部实验完成并通过审计。')
add('主构建：state-reuse-20261007-05。以下数值为本轮实际运行结果，不是旧报告转述。')
v,w,_=gm(GRAPHS,'pack_epi_degree','p0_degree')
t,tw,_=gm(GRAPHS,'pack_epi_degree','paper_tfs')
s,sw,_=gm(GRAPHS,'pair_s_degree','pack_epi_degree')
add('主要结论：PACK_EPI相对当前P0强基线的17图几何平均加速为%.4fx（%d/17胜出），相对原TFS为%.4fx（%d/17）。后一个数字包含已有归约/投影重组的贡献，不能全归本轮改动。'%(v,w,t,tw))
add('固定degree配对的SHARE相对PACK_EPI为%.4fx；相对同遍历INTERLEAVE为%.4fx。本轮尚未证明共享解码在真实图上有整体净收益；人工图提供的是适用条件证据。'%(s,gm(GRAPHS,'pair_s_degree','pair_i_degree')[0]))
add()
add('一、问题与本轮计划')
add('目标是在现有 S64/FULL AVX 聚合 + AMX 投影强基线上，区分三个因素：局部 partial 状态读写、层间全局中间结果、跨目的行的特征解码复用。实验不把“AVX 和 AMX 都忙”当作目标，而看总时间是否减少。')
add('先冻结协议，隔离实现和构建；通过小图/尾部/重复与无序边/数值边界门槛；再运行固定4张诊断图的独占及共享对照；正确性通过后补齐13图，并运行3张同规模、同度数、同源访问边际分布的人工复用图。没有根据实测成绩替换固定候选。')
add('约束：128→128→128，单位边权，原始随机 H/W 种子，FAST 主性能，原始 BF16 截断；32物理核、单socket、固定亲和；默认 NUMA，无 interleave，无新的 first-touch。原始 TFS/MKL、历史结果保持不变。')
add('范围：17张真实图在共享 intel 分区；4张重点图另在 intel_expr 独占复核；3张人工图独立统计；10张 smoke 只作正确性。最大同时请求节点数为 %d，低于用户上限10。'%peak_requested)
add()
add('二、计算过程与数据流')
add('同一张图的 AVX 聚合主体为 E×128 个 FP32 加法/层。增大邻居归约范围 M 并不会凭空增加这部分总工作；它主要改变 AMX 投影次数、转换次数、partial 保存与重读次数、任务长度与访存顺序。不要把“每组 AVX 工作更多”等同于“全图 AVX 总工作更多”。')
add('S 表示访问窗口，本轮固定64；M 表示投影范围，主方法FULL，诊断另测256。TR=16。FULL 每个非空 destination tile 每层32条 TDPBF16PS；每个窗口仍需访问边。保持相同M/分组时，本轮状态优化和配对方法不减少加法或TDP数量。')
add('P0：CSR索引 → BF16源特征 → AVX展开/FP32累加 → 线程局部partial → BF16打包 → AMX投影 → 局部FP32输出 → 全局H1 FP32 → 全图ReLU → 全图BF16截断 → H1b → 第二层。')
add('PACK：删除无消费者的局部初始化，并在行的最后访问窗口直接从累加器打包；中间窗口仍保存partial。尾行和空行明确写零。')
add('EPI：第一层AMX输出在写回时执行与原程序相同的比较ReLU和整数截断，直接生成H1b；保留层间依赖和同步。生产计时不再写全局FP32 H1；诊断路径仍单独产生它供检查。')
add('全局层间逻辑访问量：基线1792N+4K字节，融合后256N字节，净减少1536N+4K字节；K为第一层被原ReLU判定为负并清零的元素个数，0≤K≤128N。还省去两个整数组遍历/并行区。这是按源码访问计数，不是实测DRAM流量；局部AMX暂存仍存在。')
add('PAIR_I：相邻两目的行按各自原CSR顺序交错扫描；共同当前源仍分别加载解码。PAIR_S：在同样遍历上共同源只解码一次，再加入两份累加器。S64边界处重新开始匹配，不重排单行加法，无序和重复CSR均支持。')
add('若实际共同前沿匹配数为c，PAIR_S解码行数为E-c，加法仍为128E。c是实际访问窗口与顺序下可兑现的匹配，不能拿邻居集合交集或Jaccard直接代替。')
add()
add('三、对照、数值与计时口径')
add('固定方法：paper_tfs；耦合B256与FULL；规范S64/FULL P0；PACK_EPI；PAIR_I；PAIR_S；以上P0/生产候选分别用degree和原有qguard分组。4张诊断图另有state_identity、NOZERO、DIRECTPACK、PACK、EPI-only、M256消融。qguard沿用先前冻结实现，本轮没有训练或新增逐图择优器。')
add('FAST单层和最终FP32对原TFS relL2/normalized-max门槛0.01，最终BF16门槛0.02；对原FP32 MKL最终BF16门槛0.03。所有新同M同精度路径还须在单层FP32、最终FP32、H1b、最终BF16四边界逐位一致。ACCURATE补充正确性检验不混入FAST性能。')
add('先检查再计时；每方法1次预热、5次连续计时，另有10次正反交替计时。正文主表用交替样本中位数；加速比是同图、同job、同协议的中位数之比，再逐图等权几何平均。连续计时另列，不混合分母。')
add('计时包含两层、激活及层间/最终转换；不含图读入、调度构造、内存分配与初始权重准备。因此是准备完成后的两层执行时间，不是含预处理的单次应用延迟。FP32 MKL是精度不匹配的历史锚点，不用于同精度创新归因。')
add('审计：%d个图/job数据包，%d条性能汇总，%d条原始计时；检查数量 %s。全部通过。'%(audit['graph_job_packets'],audit['performance_records'],timing_rows,json.dumps(gate_counts,ensure_ascii=False)))
add()
add('四、17图固定方法结果（共享节点；加速比大于1为更快）')
comparisons=[('b256_fast','paper_tfs'),('p0_degree','paper_tfs'),('pack_epi_degree','paper_tfs'),
             ('pack_epi_degree','p0_degree'),('pack_epi_qguard','p0_qguard'),
             ('pack_epi_qguard','paper_tfs'),('pack_epi_qguard','pack_epi_degree'),
             ('pair_i_degree','pack_epi_degree'),('pair_s_degree','pack_epi_degree'),
             ('pair_s_degree','pair_i_degree'),('pair_s_qguard','pack_epi_qguard')]
add('候选 / 分母 | 交替GM | 胜/负 | 连续GM')
for m,b in comparisons:
    a,w,l=gm(GRAPHS,m,b); c,_,_=gm(GRAPHS,m,b,kind='consecutive')
    add('%s / %s | %.4fx | %d/%d | %.4fx'%(m,b,a,w,l,c))
add('胜负按中位数严格比较，不表示统计显著；微小差异需要结合完整repeat范围看。')
add('逐图两层中位数，单位ms；同一行均来自同一共享job。')
cols=['paper_tfs','b256_fast','p0_degree','pack_epi_degree','pair_i_degree','pair_s_degree','p0_qguard','pack_epi_qguard','pair_s_qguard']
add('graph | '+' | '.join(cols))
for g in GRAPHS:add(g+' | '+' | '.join('%.3f'%get(g,m) for m in cols))
add()
add('五、4图独占复核与消融')
cols=['paper_tfs','p0_degree','state_identity','nozero_degree','direct_degree','pack_degree','epi_degree','pack_epi_degree','pair_i_degree','pair_s_degree','p0_qguard','pack_epi_qguard']
add('单位ms，交替中位数。graph | '+' | '.join(cols))
for g in DIAG:add(g+' | '+' | '.join('%.3f'%get(g,m,'intel_expr') for m in cols))
for m,b in [('pack_epi_degree','p0_degree'),('epi_degree','state_identity'),('pack_degree','state_identity'),('pair_s_degree','pack_epi_degree'),('pack_epi_qguard','paper_tfs')]:
    a,w,l=gm(DIAG,m,b,'intel_expr');add('独占4图 %s / %s：GM %.4fx，胜%d/4。'%(m,b,a,w))
add('EPI-only对state_identity用于隔离融合输出收益；NOZERO/DIRECTPACK单项与组合不要求收益相加。必须同时看identity控制，避免把不同模板代码生成造成的变化全算成算法收益。')
add()
add('六、人工复用对照（不计入真实17图）')
add('三图N=524288、degree128、E=67108864，规模和全局源访问边际相同；局部共同消费和缓存工作集不同。')
add('graph | PACK_EPI ms | PAIR_I ms | PAIR_S ms | S/I | S/PACK')
for g in ['regular128_reuse1','regular128_reuse4','regular128_reuse16']:
    add('%s | %.3f | %.3f | %.3f | %.4fx | %.4fx'%(g,get(g,'pack_epi_degree'),get(g,'pair_i_degree'),get(g,'pair_s_degree'),ratio(g,'pair_s_degree','pair_i_degree'),ratio(g,'pair_s_degree','pack_epi_degree')))
add('reuse1实际shared-head比例0，reuse4和reuse16均0.5；相同共享率仍有不同收益。因此需要把可兑现共享与访存局部性分开建模。PAIR_I的收益不能归入共享解码，PAIR_S还必须超过PACK_EPI才体现净收益。')
add()
add('七、机器码、计数和数值修复证据')
add('反汇编确认共同邻居分支：自然优化后的INTERLEAVE保留16次vpmovzxwd，SHARE为8次；两者均16次向量加法。未用volatile汇编阻止基线CSE。Scalar/Pair热CSR循环没有向量寄存器栈溢出；显式partial读写另计。配对helper的调用、标量控制与比较开销真实包含在运行时间中。')
add('STATE_COUNTERS的第一层是诊断FP32输出路径，不能拿其写回字节代替生产EPI流量。诊断PMU固定测P0/PACK_EPI/PAIR_I/PAIR_S，调用完整生产两层路径，每方法3次；这些带计数时间不进入主加速比。具体PMU计数、覆盖率、每线程启停时间和未缩放原始值留在原日志。')
add('PMU共%d个region，通过32线程覆盖与计数有效性检查。下表为独占每列3次独立中位数之比。'%len(pmu))
add('graph | PACK/P0 instructions | PACK/P0 cache-misses | PACK/P0 AMX_BUSY | I/PACK instructions | S/I instructions')
for g in DIAG:
    def pm(m,k):
        rs=[r for r in pmu if r['partition']=='intel_expr' and r['graph']==g and r['method']==m]
        assert len(rs)==3,(g,m,len(rs))
        return statistics.median(float(r[k+'_raw']) for r in rs)
    add(g+' | '+' | '.join('%.4f'%v for v in [
        pm('pack_epi_degree','instructions')/pm('p0_degree','instructions'),
        pm('pack_epi_degree','cache_misses')/pm('p0_degree','cache_misses'),
        pm('pack_epi_degree','amx_busy')/pm('p0_degree','amx_busy'),
        pm('pair_i_degree','instructions')/pm('pack_epi_degree','instructions'),
        pm('pair_s_degree','instructions')/pm('pair_i_degree','instructions')]))
add('PMU支持“输出融合减少部分额外工作，配对控制有明显指令成本”的解释。generic cache事件不是DRAM字节，线程总和包含等待与启停偏差；人工复用图没有PMU，不能宣称已测得它的具体缓存层级机制。AMX_BUSY是speculative busy cycles，不能证明AVX/AMX重叠；事件定义：https://perfmon-events.intel.com/platforms/sapphirerapids/core-events/core/ 。')
add('早期构建失败记录保留：run01命名空间与宏冲突编译失败；run02/03语义门槛暴露fast-math把比较ReLU化简成vmaxps后改变subnormal位模式；run04独立helper优化又暴露NaN语义差异。未放宽门槛、未改全局MXCSR。')
add('最终仅在scatter助手局部启用precise，并用整数位清零/截断保持原语义；生产和独立helper均通过19种±0/NaN/Inf/subnormal边界，10张smoke另覆盖FAST/ACCURATE、M64/M256/FULL、尾部、无序重复边和正负抵消。run04独立helper失败不等于其所有生产内核也失败。')
add('run05构建快照曾捕获10000字节的未完成分析脚本；它未用于analysis-01或最终分析。这里单独冻结并使用完整分析器，SHA见FINAL_AUDIT.json。该问题不涉及被计时的程序；原构建快照未被改写。')
add()
add('八、预处理及复用次数')
add('qguard需要签名、候选排序和guard模拟；不能因为准备后执行变快，就忽略构造成本。下表保留本轮调度构造总时间和PACK_EPI下每次两层节省；收益非正时没有有限摊销点。摊销是该批构造全部候选的保守对照，不代表精简生产实现的最低成本。')
add('graph | 本轮schedule构造ms | degree→qguard每次节省ms | 构造/节省')
for g in GRAPHS:
    r=next(x for x in eligible if x['graph']==g and x['partition']=='intel')
    logs=pathlib.Path(r['folder']).parent
    gr=[x for x in rows(logs/'group_setup.csv') if x['graph']==g]
    allrows=[x for x in gr if x.get('includes_all_candidates_and_guard')=='1']
    if allrows:
        v=float(allrows[-1]['ms']);saving=get(g,'pack_epi_degree')-get(g,'pack_epi_qguard')
        add('%s | %.3f | %.3f | %s'%(g,v,saving,'%.1f次'%(v/saving) if saving>0 else '无'))
    else:add(g+' | 详见group_setup.csv各阶段 | 未自动合计 | 未自动估算')
add()
add('九、创新边界与下一步取舍')
add('1. 本轮优先把EPI纳入强基线；局部清零/打包和算子输出融合属于实现完善，不能各自包装成首次提出的期刊创新。任何后续分组方法都应比较包含这些优化的基线。')
add('2. 共享解码机制确实存在，人工高复用图证明它有条件成立；真实图的匹配/控制成本必须一起算。当前固定相邻配对不能被称为普适最优分组，也没有完成AVX与AMX的流水重叠证明。')
add('3. 更有研究价值的问题是：在固定投影工作预算下，如何形成能兑现复用、又不增加过多控制和状态成本的目的行分组。候选应在受限度区间内选择，保留AMX padding/投影预算，使用S64实际前沿匹配数而非纯集合重叠，并显式计入局部工作集、比较数、尾部失衡和预处理摊销。')
add('4. 下一轮先固定TR16成员和AMX行槽，仅优化8对行如何配对，用小配对表驱动执行器。这样投影、padding和S64状态预算天然保持。用带窗口编号的少量签名生成固定数量伙伴候选，确定性贪心选对，最后用实际前沿计数复核；不要全图暴力求120对/tile再把昂贵搜索当免费。原相邻配对、同度随机配对作为控制。分别用Scalar、PAIR_I、PAIR_S执行，保持逐行CSR加法顺序；若固定tile内结构上界很小，再考虑同q64桶内跨tile重组。结构oracle仅作上界，不能充当部署策略。')
add('5. 若运行时merge开销主导，可先验证一个更容易证伪的快路径：静态CSR中完全相同的S64邻居序列只确认一次，运行时免逐边比较地共享解码，其余窗口回退Scalar。保持加法顺序，计入计划空间、加载和构建摊销。它检验“动态匹配成本吞掉收益”，不预先保证实际图覆盖率足够。进一步的预计算消费计划再单独开展。')
add('6. 真正AVX/AMX交错流水、延迟残差修正、选择性物化属于后续独立问题。本轮没有把这些未经实现验证的点写成已完成贡献。创新边界应落在新的约束、执行机制和实证解释，而不是“别的平台做过所以不能做”或“换到CPU就自动算新”。')
add('外推限制：当前仍是128维、随机H/W、单位权推理；尚不能代表归一化GCN训练模型任务精度、全部形状、全部CPU或通用GAT。共享节点的波动不能靠多次repeat彻底消除，4图独占复核是补充证据。')
add()
add('十、复现位置')
add('项目根目录：'+str(ROOT))
add('冻结构建及全部原始日志：'+str(RUN))
add('实验源码与协议：'+str(EXP))
add('最终审计、完整分析器、CSV与汇编审计：'+str(FINAL))
add('运行最终分析器：python3 '+str(FINAL/'analyze_state.py')+' '+str(RUN)+' <新的wzh内输出目录>')
add('原TFS/MKL均保持；未向GitHub推送。历史失败与初步analysis-01未覆盖。')
add('作业：')
for j in job_audits:add('%s | %s | %s | %s | %s'%(j['job'],j['partition'],j['node'],j['elapsed'],j['completion']['mode']))
report='\n'.join(L)+'\n'
(OUT/'TECHNICAL_REPORT_20261007.txt').write_text(report,encoding='utf-8')
event={'id':'state-reuse-final-20261007','kind':'state_final','status':'PASS','evidence':str(FINAL),
       'report':str(OUT/'TECHNICAL_REPORT_20261007.txt'),'paper_eligible':False,
       'issues':'All planned jobs complete; 128-only, default NUMA; synthetic and shared/exclusive separate; frozen intermediate analyzer preserved and not used',
       'next':'Use EPI strong baseline and measured decode/cost boundary to design grouping followup'}
write_json(FINAL/'event.json',event)
subprocess.run(['python3',str(ROOT/'scripts/record_event.py'),str(FINAL/'event.json')],check=True)
artifacts={str(p.relative_to(OUT)):sha(p) for p in FINAL.rglob('*') if p.is_file()}
artifacts['TECHNICAL_REPORT_20261007.txt']=sha(OUT/'TECHNICAL_REPORT_20261007.txt')
write_json(FINAL/'artifact_hashes.json',artifacts)
print(json.dumps({'status':'PASS','report':str(OUT/'TECHNICAL_REPORT_20261007.txt'),
    'report_bytes':len(report.encode()),'gate_totals':gate_counts,'performance_records':audit['performance_records']}))
