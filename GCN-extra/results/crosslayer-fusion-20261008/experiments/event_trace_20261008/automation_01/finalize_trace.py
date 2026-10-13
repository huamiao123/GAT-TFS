#!/usr/bin/env python3
"""One-shot evidence finalization. No polling, jobs, kernel execution or retries.
CLI RUN ANALYSIS_DIR FINAL_DIR. Exit 0 SUCCESS, 2 FAILED, 3 NOT_READY.
"""
import csv,datetime,fcntl,hashlib,json,math,os,pathlib,shlex,subprocess,sys,traceback
sys.dont_write_bytecode=True
P=pathlib.Path
W=P('/home/huangjianqiang_group/hdacp1/data/wzh');ROOT=W/'GCN-extra'
EXP=ROOT/'experiments/event_trace_20261008';AUTO=EXP/'automation_01'
RUN_EXPECTED=ROOT/'runs/event-trace-20261008-01';RESULT=ROOT/'results/event_trace_20261008'
ANALYZER=EXP/'analysis_inputs_01/analyze_trace.py'
HELPER=ROOT/'experiments/pair_plan_20261007/analyze_plan.py';RECORDER=ROOT/'scripts/record_event.py'
CODEGEN=EXP/'codegen_audit_01/audit.json'
PINS={str(ANALYZER):'107fb1fa18034fb687d4e18011e511817498245cebb44b7a085793c2716be9c2',str(HELPER):'bcac6ddc97042cbf4d3643ae7fa846a2f716ee1fc4c05d6b238269874cf90fd9',str(RECORDER):'21a4ff57263f50d0d90a649a7b8fc92a04be544817d68d37acb0e067434bf70e',str(CODEGEN):'be569f3d11363d6a0fec81f0394144307cd38baf648ff3229a188fec7a46672e',str(EXP/'PROTOCOL.md'):'c4b432e2953a2388d40b9133f06db282c2fe7924b9719ded62ab23fd48c8a993'}
RULE_PINS={
str(W/'AGENTS.md'):'55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7',
str(W/'skills/tfs-research-engineering/SKILL.md'):'641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca',
str(ROOT/'IMPLEMENTATION_CONTRACT.md'):'b6419e5106278106da6727c90f4cfa185cc70c587d70b4087c0bc286fd0dc244',
str(W/'TFS-Train/docs/spec/MATH_SPEC_V2.md'):'1938d301e3fc913766744af166c0a41b1cb767d7d7419af9c098d3de955cf266',
str(W/'TFS-Train/docs/spec/KERNEL_LAYOUT_V2.md'):'732e859128baa3cf87813df82cf7562d14358fc6e1cf17c32ec0dde5a743595f',
str(W/'TFS-Train/docs/spec/PYTORCH_EXTENSION_SPEC.md'):'a3784dd66d4986bde33a40d523f761ee8956da9bbbe711f1707d13d8d2d82df2',
str(W/'TFS-Train/docs/spec/EXPERIMENT_SPEC_V2.md'):'ecb8fe1c394d9ec47aae97d8f5b87c5b948963c7e8c8b25d70128483c7c0d10d'}
NAMES=('pack_epi_degree','pack_epi_qguard','dyn_i','dyn_s','exact_s','trace_i','trace_s_cached','trace_s_rep')
ALIASES=('PD','PQ','DI','DS','ES','TI','TC','TR')
DIAG=('mycielskian19','reddit','roadNet-CA','amazon0601')
SYN=('regular128_reuse1','regular128_reuse4','regular128_reuse16')
SMOKE={'window_tail','window_high_tail','window_scope_tail','plan_exact_tail','plan_exact_unsorteddup','plan_exact_mismatch'}|{'state_tail_'+str(x) for x in (1,15,16,17,31,33,65)}
JOBS={'10896223':('intel',False,'smoke',SMOKE),'10896227':('intel_expr',True,'synthetic',set(SYN)),'10896229':('intel_expr',True,'mycielskian19',{'mycielskian19'}),'10896230':('intel_expr',True,'reddit,roadNet-CA,amazon0601',set(DIAG[1:])),'10896231':('intel',False,'synthetic',set(SYN)),'10896233':('intel',False,'mycielskian19',{'mycielskian19'}),'10896234':('intel',False,'reddit,roadNet-CA,amazon0601',set(DIAG[1:]))}
COHORTS=(('intel','false'),('intel_expr','true'));PROTOCOLS=('consecutive','alternating')
def sha(path):
 h=hashlib.sha256()
 with P(path).open('rb') as f:
  for b in iter(lambda:f.read(1048576),b''):h.update(b)
 return h.hexdigest()
def readj(p):return json.loads(P(p).read_text(encoding='utf-8-sig'))
def dump(p,v):P(p).write_text(json.dumps(v,ensure_ascii=False,sort_keys=True,indent=2)+'\n',encoding='utf-8')
def now():return datetime.datetime.now(datetime.timezone.utc).isoformat()
def finite(v):
 x=float(v)
 if not math.isfinite(x):raise ValueError('nonfinite number')
 return x
def num(v,n=4):return format(finite(v),'.'+str(n)+'f')
def pct(n,d):return num(100*finite(n)/finite(d),3)+'%' if finite(d) else 'NA(分母0)'
def one(rows,**kw):
 rr=[r for r in rows if all(str(r.get(k))==str(v) for k,v in kw.items())]
 if len(rr)!=1:raise ValueError('expected unique record '+repr(kw)+' got '+str(len(rr)))
 return rr[0]
def tag(p,n):return one(p['raw_records'][n])
def capture(cmd,stem,timeout=60):
 dump(str(stem)+'.command.json',list(map(str,cmd)))
 with P(str(stem)+'.stdout.txt').open('w',encoding='utf-8') as out,P(str(stem)+'.stderr.txt').open('w',encoding='utf-8') as err:
  p=subprocess.run(list(map(str,cmd)),stdout=out,stderr=err,text=True,timeout=timeout,check=False,cwd=str(AUTO),env=dict(os.environ,PYTHONDONTWRITEBYTECODE='1'))
 dump(str(stem)+'.exit.json',{'returncode':p.returncode,'ended_utc':now()});return p.returncode
def accounting_state(rows):
 missing=sorted(set(JOBS)-set(rows));active=[];failed=[]
 for jid,r in rows.items():
  if jid not in JOBS:continue
  state=r['state'].split()[0].split('+')[0]
  if state in ('PENDING','RUNNING','CONFIGURING','COMPLETING','SUSPENDED','RESIZING'):active.append(jid)
  elif state!='COMPLETED' or r['exit_code']!='0:0':failed.append(jid)
 return dict(ready=not(missing or active or failed),missing=missing,active=active,failed=failed)
def read_rules(attempt):
 rows=[]
 for i,(path,expected) in enumerate(RULE_PINS.items()):
  b=P(path).read_bytes();s=b.decode('utf-8-sig');actual=hashlib.sha256(b).hexdigest()
  target=attempt/('rule_%02d_'%i+P(path).name+'.read.txt');target.write_text(s,encoding='utf-8')
  rows.append(dict(path=path,sha256=actual,expected_sha256=expected,read_bytes=len(b),snapshot=str(target),match=actual==expected))
 dump(attempt/'current_rules.json',rows)
 if not all(r['match'] for r in rows):raise ValueError('current authority changed; manual review required')
 return rows
def readiness(run,attempt):
 ids=[f.read_text().strip().split(';')[0] for f in (run/'launcher').glob('*/job_id.txt')]
 if len(ids)!=7 or set(ids)!=set(JOBS):raise ValueError('designated launcher JobIDs changed')
 cmd=['sacct','--parsable2','--noheader','-j',','.join(sorted(JOBS)),'--format=JobIDRaw,State,ExitCode,NodeList,Partition']
 if capture(cmd,attempt/'sacct',30):return dict(ready=False,reason='sacct unavailable')
 rows={}
 for line in (attempt/'sacct.stdout.txt').read_text().splitlines():
  z=line.rstrip('|').split('|')
  if len(z)!=5 or z[0] not in JOBS:continue
  jid,state,code,node,partition=z
  if jid in rows:raise ValueError('duplicate primary accounting row')
  rows[jid]=dict(state=state,exit_code=code,node=node,partition=partition)
 r=accounting_state(rows);r['accounting']=rows
 if r['failed']:raise ValueError('terminal job failure '+repr(r['failed']))
 if not r['ready']:return r
 if {p.name for p in (run/'logs').iterdir() if p.is_dir()}!={'trace-'+j for j in JOBS}:raise ValueError('unexpected job directory set')
 comps=[];packets=[]
 for jid,(part,ex,mode,graphs) in JOBS.items():
  d=run/'logs'/('trace-'+jid);c=readj(d/'completion.json');ev=readj(d/'event.json')
  if rows[jid]['partition']!=part or ev.get('status')!='PASS' or ev.get('exit_status')!=0 or (d/'exit_status.txt').read_text().strip()!='0':raise ValueError('job wrapper/partition '+jid)
  if c.get('study')!='event_trace' or c.get('mode')!=mode or c.get('graphs')!=len(graphs) or c.get('passed')!=len(graphs) or c.get('failed')!=0:raise ValueError('completion contract '+jid)
  listing=readj(d/'graph_list.json')
  if len(listing)!=len(graphs) or {g['graph'] for g in listing}!=graphs:raise ValueError('graph matrix '+jid)
  comps.append(dict(job=jid,partition=part,exclusive=ex,path=str(d/'completion.json'),sha256=sha(d/'completion.json'),completion=c))
  for g in sorted(graphs):
   f=d/g;s=readj(f/'status.json');ps=readj(f/'methods.status.json')
   if s.get('status')!='PASS' or ps.get('returncode')!=0 or ps.get('timed_out',False):raise ValueError('packet failed '+jid+'/'+g)
   done=[]
   with (f/'methods.log').open(encoding='utf-8') as stream:
    for line in stream:
     if line.startswith('TRACE_COMPLETE '):done.append(dict(x.split('=',1) for x in shlex.split(line)[1:] if '=' in x))
   if len(done)!=1 or done[0].get('pass')!='1' or done[0].get('graph')!=g:raise ValueError('raw completion '+jid+'/'+g)
   packets.append(dict(job=jid,graph=g,smoke=mode=='smoke',folder=str(f),trace_complete=done[0]))
 if len(packets)!=27 or sum(p['smoke'] for p in packets)!=13:raise ValueError('13+14 packet contract')
 r.update(completions=comps,packets=packets);dump(attempt/'readiness.json',r);return r
def tables(analysis):
 out={}
 for n in ('summary','ratios','pmu_summary','setup','structure'):
  with (analysis/(n+'.csv')).open(encoding='utf-8-sig',newline='') as f:out[n]=list(csv.DictReader(f))
 return out

def render_report(audit,ts,run,analysis,stamp):
 """Pure in-memory formatting; no external calls or writes."""
 if audit.get('analysis_pass') is not True:raise ValueError('failed analysis cannot generate success report')
 summary=ts['summary'];lookup={}
 for r in summary:
  k=tuple(r[x] for x in ('partition','exclusive','protocol','graph','method'))
  if k in lookup or finite(r['median_ms'])<=0:raise ValueError('duplicate/nonpositive summary')
  lookup[k]=r
 expected={(p,e,q,g,m) for p,e in COHORTS for q in PROTOCOLS for g in DIAG+SYN for m in NAMES}
 if len(summary)!=224 or set(lookup)!=expected:raise ValueError('224-row summary matrix')
 gate=audit['expansion_gate'];cs=gate['cohorts'];perf=[p for p in audit['packets'] if p['data_class']!='smoke']
 if gate['candidate']!='trace_s_rep' or len(cs)!=8 or len(perf)!=14:raise ValueError('candidate/cohort/packet matrix')
 def row(p,e,q,g,m):return lookup[p,e,q,g,m]
 def ratio(p,e,q,g,a,b):
  x,y=row(p,e,q,g,a),row(p,e,q,g,b)
  if x['job']!=y['job'] or x['build_fingerprint']!=y['build_fingerprint']:raise ValueError('cross-job/build ratio')
  return finite(x['median_ms'])/finite(y['median_ms'])
 lines=['TFS / GCN-extra 事件轨迹实验技术说明（2026-10-08）','',
 '生成方式：自动证据汇总，不是实验完成后新增的一次独立人工解读。结论由预注册规则及审计数据生成，数值门限未改变。',
 '生成时间（UTC）：'+stamp,'原始运行：'+str(run),'独立分析：'+str(analysis),'',
 '一、范围与固定结论',
 '推理128→128→128，单位边权，原随机种子H/W，BF16截断，S64、FULL投影、PF1，32个单socket物理核。默认NUMA策略；未使用NUMA interleave，未改变特征first-touch。共享intel与独享intel_expr分开报告。',
 '本轮离线编码原dynamic greedy的合法消费序列，保留每行CSR加法顺序。它测试移除在线决策后能否兑现共享解码收益。Inspector–executor和编码是已有思想，本轮不据此宣布新科学贡献。',
 '固定扩展候选仅trace_s_rep：两个分区×两个协议分别相对两种PACK基线，均须四图GM≥1.03、胜图≥3/4，且全部正确性门禁通过。不能事后换候选。']
 eligible=gate.get('eligible_to_expand_full17') is True
 lines.append('筛选结果：'+('全部八项通过，仅获得后续扩展资格；本脚本仍不会启动新作业。' if eligible else '未满足全部八项条件，因此不扩展到17图。在线方案变快或个别图获益，不能替代相对两种PACK的净收益要求。'))
 lines+=['','二、门禁、机器码与计时边界',
 '7个指定作业均COMPLETED、ExitCode=0:0；13个smoke包+14个性能包通过独立审计。Smoke由Python逐事件重放；性能图完整trace验证在作业内由独立C++路径完成。',
 '门禁记录数（不是独立随机实验次数）：'+json.dumps(audit['counts'],ensure_ascii=False,sort_keys=True),
 '机器码47项审计通过：共享事件中TI/DI保留16次向量解码+16次shift+16次add；TC/TR/DS为8次解码+8次shift+16次add，每向量16维。TI独立解码未被CSE抹掉，代表head路径确实不同。DI/DS reducer与前轮规范化指令一致；未见直接rsp/rbp向量spill，但有标量控制栈流量。',
 '主计时包括两层及激活/转换，排除图读取、输入/权重准备、schedule/plan构建。consecutive=1次预热+5次样本；alternating=10次正反交替样本，每次切换预热。均报告median；速度比=同图/同job/同build/同协议的参考median÷候选median，>1为候选更快。',
 '原样本和min/max保留在summary.csv；范围不是置信区间。每方法3次PMU是单独测量，其wall time不替代主计时。',
 '缩写：'+'；'.join(a+'='+n for a,n in zip(ALIASES,NAMES)),
 'PD/PQ是PACK基线；DI/DS动态独立/共享解码；ES共享完整一致窗口；TI轨迹独立解码；TC轨迹共享且缓存双head；TR轨迹共享且读取代表head。','',
 '三、八项扩展筛选','分区 | 协议 | 基线 | 四图GM | 胜图 | 通过']
 for p,e in COHORTS:
  for q in PROTOCOLS:
   for baseline in NAMES[:2]:
    c=one(cs,partition=p,exclusive=e,protocol=q,baseline=baseline)
    v=[ratio(p,e,q,g,baseline,'trace_s_rep') for g in DIAG];gm=math.exp(sum(math.log(x) for x in v)/4);wins=sum(x>1 for x in v)
    if not math.isclose(gm,finite(c['geometric_mean']),rel_tol=1e-12) or wins!=int(c['wins']):raise ValueError('gate arithmetic changed')
    lines.append(' | '.join((p,q,baseline,num(gm)+'×',str(wins)+'/4','是' if c['pass_filter'] else '否')))
 lines+=['','逐图TR相对两种PACK：','分区 | 协议 | 图 | PD/TR | PQ/TR']
 for p,e in COHORTS:
  for q in PROTOCOLS:
   for g in DIAG:lines.append(' | '.join((p,q,g,num(ratio(p,e,q,g,NAMES[0],NAMES[7]))+'×',num(ratio(p,e,q,g,NAMES[1],NAMES[7]))+'×')))
 lines+=['','四、原基准四图：八种方法两层median（毫秒）',
 'Mycielskian19是构造图；四图统称原基准诊断集合，不将全部图称为真实业务图。',
 '分区 | 协议 | 图 | '+' | '.join(ALIASES)]
 for p,e in COHORTS:
  for q in PROTOCOLS:
   for g in DIAG:lines.append(' | '.join((p,q,g)+tuple(num(row(p,e,q,g,m)['median_ms']) for m in NAMES)))
 lines+=['','五、合成图机制对照',
 '分区 | 协议 | 图 | DI/TI(离线控制) | DS/TC(共享离线控制) | TC/TR(代表索引) | TI/TC(共享解码) | PD/TR | PQ/TR']
 effects=(('dyn_i','trace_i'),('dyn_s','trace_s_cached'),('trace_s_cached','trace_s_rep'),('trace_i','trace_s_cached'),('pack_epi_degree','trace_s_rep'),('pack_epi_qguard','trace_s_rep'))
 for p,e in COHORTS:
  for q in PROTOCOLS:
   for g in SYN:lines.append(' | '.join((p,q,g)+tuple(num(ratio(p,e,q,g,a,b))+'×' for a,b in effects)))
 lines+=['共享率见下一表。相同共享率不保证相同性能；控制分支、元数据、工作集和访问局部性仍不同，不能将所有收益归因于比较指令消除。',
 '合成图两层median（毫秒）：','分区 | 协议 | 图 | '+' | '.join(ALIASES)]
 for p,e in COHORTS:
  for q in PROTOCOLS:
   for g in SYN:lines.append(' | '.join((p,q,g)+tuple(num(row(p,e,q,g,m)['median_ms']) for m in NAMES)))
 lines+=['','六、分组边界结构机会（只读诊断，没有执行新FULL-row性能内核）',
 '有序=CSR原顺序非递减；sorted pair要求两行均有序。S64/Sfull为共享事件数；额外decode空间=(Sfull-S64)/(E-S64)，负值保留。',
 '分区 | 图 | E | 有序行/全部行 | 有序pair/全部pair | S64(占E) | Sfull(占E) | ΔS | 额外decode空间']
 for p,e in COHORTS:
  for g in DIAG+SYN:
   packet=one(perf,partition=p,exclusive=e,graph=g);o=tag(packet,'TRACE_OPPORTUNITY');E=int(packet['independent_CSR_work']['E']);a=int(o['shared_s64']);b=int(o['shared_full'])
   lines.append(' | '.join((p,g,str(E),o['sorted_rows']+'/'+o['rows']+' ('+pct(o['sorted_rows'],o['rows'])+')',o['sorted_pairs']+'/'+o['pairs']+' ('+pct(o['sorted_pairs'],o['pairs'])+')',str(a)+' ('+pct(a,E)+')',str(b)+' ('+pct(b,E)+')',str(b-a),pct(b-a,E-a))))
 lines+=['','有序pair子集机会：','分区 | 图 | gain有序pair/全部有序pair | sorted S64 | sorted Sfull | sorted ΔS | 无序pair ΔS | 全体loss pair']
 for p,e in COHORTS:
  for g in DIAG+SYN:
   o=tag(one(perf,partition=p,exclusive=e,graph=g),'TRACE_OPPORTUNITY')
   lines.append(' | '.join((p,g,o['sorted_gain_pairs']+'/'+o['sorted_pairs']+' ('+pct(o['sorted_gain_pairs'],o['sorted_pairs'])+')',o['sorted_shared_s64'],o['sorted_shared_full'],str(int(o['sorted_shared_full'])-int(o['sorted_shared_s64'])),str(int(o['unsorted_shared_full'])-int(o['unsorted_shared_s64'])),o['loss_pairs'])))
 lines+=['有序两行full greedy最优性只限固定pair、每事件每行至多消费一条边的模型；不覆盖同一行重复邻居run的多次复用，也不是全局最优分组定理。无序CSR的full greedy可能共享更少。结构事件差不能直接预测时间。',
 '本轮未改pair选择、未排序CSR或去重。Sfull只是诊断；主kernel的FULL指投影scope，两者不同。','',
 '七、setup与内存','分区 | 图 | 全五schedule ms | base plan ms | trace build ms | trace validator ms | trace total ms | 独立复核 ms | 可选诊断 ms | trace有效MiB | trace容量MiB | object B | 峰值RSS MiB']
 for p,e in COHORTS:
  for g in DIAG+SYN:
   x=one(perf,partition=p,exclusive=e,graph=g);s=tag(x,'TRACE_SCHEDULE_SETUP');b=tag(x,'PAIRPLAN_SETUP');t=tag(x,'TRACE_PLAN_SETUP');v=tag(x,'TRACE_PLAN');o=tag(x,'TRACE_OPPORTUNITY');c=tag(x,'TRACE_COMPLETE')
   rss=num(finite(c['max_rss_kib'])/1024,3) if c.get('max_rss_kib') is not None else '未记录'
   z=(s['ms'],b['total_ms'],t['build_ms'],t['validate_ms'],t['total_ms'],v['independent_audit_ms'],o['diagnostic_ms'])
   lines.append(' | '.join((p,g)+tuple(num(a,3) for a in z)+(num(int(t['plan_bytes'])/1048576,3),num(int(t['allocated_bytes'])/1048576,3),str(t['object_bytes']),rss)))
 lines+=['setup是实际本次pipeline测值，排除在主median之外。schedule含全部五种旧schedule，不能当作独立qguard开销；base plan、trace builder、trace validator分别列出。独立复核是正确性检查；可选结构诊断不是候选必需预处理，不计入必要setup或摊销回本。当前不提供部署break-even结论。',
 'trace有效字节=headers+Σrecord ceil(events_record/4)+8*(tiles+1)。不能用ceil(总events/4)替代逐record求和。vector有效/容量字节不含全部allocator开销，object另列；空/耗尽pair-window也可能有count header。',
 '所有方法计时都保留相同不可变base/trace plan。RSS是含全部方法、图、特征、验证与临时状态的整进程峰值，不能称作某方法独占内存或相对独立PACK的增加量。','',
 '八、技术边界与后续判断',
 '每层边归约仍做128E个浮点加法；组变大不自动增加总AVX加法，变化是依赖组织、partial状态与投影重复次数。共享只减少重复加载/解码请求，每个destination的加法仍须保留。',
 '本轮S64与投影FULL固定，各方法AMX投影计数相同，不能将差异解释为省AMX投影。独立decode请求E，共享decode请求E-S64；cached当前CSR请求E，代表模式E-S64；PF1下一索引请求没有同步消失。',
 '源码计数与机器码路径不是retired计数、cache-line次数或DRAM流量；也未证明AVX–AMX有效重叠或两套完整吞吐能相加。PMU原始值、有效性和multiplex比例见pmu_summary.csv/audit.json；generic cache事件与AMX busy不能唯一定位瓶颈。',
 '若筛选失败，应保留负结果：合法轨迹和可见工作减少仍可能无法抵销强PACK基线的额外开销。是否研究跨窗口消费或新分组，需要新协议和受控实验；本脚本不实现、不启动它们。',
 '本轮没有训练精度、归一化GCN任务精度或期刊创新充分性的结论。后续创新需落在新的工作量/状态/调度机制及可解释适用区间；不能把已有协作或inspector–executor思想重新命名为贡献。','',
 '九、证据定位',
 '原始run：'+str(run),'冻结源码：'+str(run/'source_snapshot'),'协议：'+str(run/'source_snapshot/PROTOCOL.md'),'机器码审计：'+str(CODEGEN)]
 for n in ('audit.json','summary.csv','ratios.csv','setup.csv','structure.csv','pmu_summary.csv','expansion_gate.json'):lines.append(str(analysis/n))
 lines+=['完整SHA、当前规则读档、7作业状态和三份handoff同步记录见delivery_manifest.json及compact_audit.json。']
 return '\n'.join(lines)+'\n'

def sync_event(event,attempt,prefix):
 ep=attempt/(prefix+'.event.json');dump(ep,event)
 if capture([sys.executable,RECORDER,ep],attempt/(prefix+'.record_event')):raise ValueError('record_event failed')
 if capture([sys.executable,RECORDER,'check'],attempt/(prefix+'.handoff_check')):raise ValueError('handoff check failed')
 docs=[ROOT/'docs/handoff'/n for n in ('ALL_EXPERIMENTS.md','EXPERIMENT_ISSUES.md','PROJECT_PROGRESS.md')]
 for d in docs:
  if event['id'] not in d.read_text(encoding='utf-8'):raise ValueError('event missing from '+str(d))
 return dict(event_path=str(ep),event_sha256=sha(ep),event_id=event['id'],three_documents=[str(d) for d in docs])
def main(argv):
 if len(argv)!=3:print(json.dumps(dict(status='FAILED',reason='usage finalize_trace.py RUN ANALYSIS_DIR FINAL_DIR')));return 2
 run,analysis,final=[P(x).resolve() for x in argv]
 if (run,analysis,final)!=(RUN_EXPECTED.resolve(),(RESULT/'analysis-01').resolve(),(RESULT/'final').resolve()):
  print(json.dumps(dict(status='FAILED',reason='paths outside registered targets')));return 2
 lock=(AUTO/'finalizer.lock').open('a+b')
 try:fcntl.flock(lock.fileno(),fcntl.LOCK_EX|fcntl.LOCK_NB)
 except BlockingIOError:print(json.dumps(dict(status='NOT_READY',reason='another finalizer holds lock')));return 3
 attempt=AUTO/('attempt_'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ')+'_'+str(os.getpid()));attempt.mkdir()
 stage='preflight';created=False;rules=[];ready={};handoff={}
 try:
  dump(attempt/'invocation.json',dict(argv=argv,started_utc=now(),script_sha256=sha(__file__),no_polling=True,no_jobs_submitted=True,automatic_summary_not_new_human_review=True))
  rules=read_rules(attempt)
  for path,h in PINS.items():
   if sha(path)!=h:raise ValueError('reviewed input hash changed '+path)
  dump(attempt/'reviewed_inputs.json',PINS)
  ready=readiness(run,attempt)
  if not ready['ready']:
   result=dict(status='NOT_READY',reason='all seven jobs and completion packets required',attempt=str(attempt),readiness=ready,final_output_created=False)
   dump(attempt/'NOT_READY.json',result);print(json.dumps(result,ensure_ascii=False));return 3
  if analysis.exists() or final.exists():raise ValueError('output already exists: refusing overwrite/retry')
  if capture([sys.executable,RECORDER,'check'],attempt/'pre_analysis_handoff_check'):raise ValueError('previous handoff not synchronized')
  stage='independent_analysis'
  rc=capture([sys.executable,ANALYZER,run,analysis],attempt/'analyzer',1800)
  if rc:raise ValueError('independent analyzer exit '+str(rc))
  a=readj(analysis/'audit.json')
  if a.get('analysis_pass') is not True or a.get('source_script_sha256')!=PINS[str(ANALYZER)]:raise ValueError('analysis gate/self hash')
  if P(a['build']['manifest']['root']).resolve()!=run:raise ValueError('analysis bound to wrong build root')
  if len(a.get('jobs',[]))!=7 or {str(j['job']) for j in a['jobs']}!=set(JOBS) or not all(j['valid'] and not j['pending'] for j in a['jobs']):raise ValueError('analysis jobs invalid')
  for j in a['jobs']:
   if P(j['jobdir']).resolve()!=(run/'logs'/('trace-'+str(j['job']))).resolve():raise ValueError('analysis jobdir outside run')
  if len(a.get('packets',[]))!=27 or not all(x['valid'] for x in a['packets']):raise ValueError('analysis packets invalid')
  stage='format_report';stamp=now();report=render_report(a,tables(analysis),run,analysis,stamp)
  analysis_hashes={str(p):sha(p) for p in sorted(analysis.iterdir()) if p.is_file()}
  refs=dict(run=str(run),source_snapshot=str(run/'source_snapshot'),source_hash_list=str(run/'source.sha256'),binary_hash_list=str(run/'build/binary.sha256'),protocol=str(run/'source_snapshot/PROTOCOL.md'),codegen_audit=str(CODEGEN),analyzer=str(ANALYZER),helper=str(HELPER),raw_logs=str(run/'logs'))
  compact=dict(schema='event_trace_compact_audit_v1',status='ANALYSIS_PASS',analysis_pass=True,automatic_summary_not_new_human_review=True,generated_utc=stamp,run=str(run),analysis_dir=str(analysis),counts=a['counts'],expansion_gate=a['expansion_gate'],no_new_jobs=True,scope='inference128_FULL_S64_PF1_default_NUMA',
   jobs=[{k:j[k] for k in ('job','node','partition','exclusive','valid','completion','slurm')} for j in a['jobs']],packet_counts=dict(total=27,smoke=13,performance=14),
   pmu_regions=sum(len(p.get('pmu_audit',[])) for p in a['packets']),pmu_usable_regions=sum(bool(y['usable']) for p in a['packets'] for y in p.get('pmu_audit',[])),codegen_status='PASS',codegen_checks=47,
   reviewed_input_hashes=PINS,current_rules=rules,analysis_files_sha256=analysis_hashes,source_paths=refs,limits=a['interpretation_limits'])
  final.mkdir(parents=True);created=True
  reportpath=final/'TECHNICAL_REPORT_20261008.txt';reportpath.write_text(report,encoding='utf-8-sig');dump(final/'compact_audit.json',compact)
  stage='handoff'
  event=dict(id='event-trace-20261008-01-auto-final-'+attempt.name,kind='automatic_final_analysis',status='PASS',date=stamp,evidence=str(final),analysis=str(analysis),run=str(run),jobs=sorted(JOBS),analysis_pass=True,counts=a['counts'],fixed_candidate='trace_s_rep',expand_full17=a['expansion_gate']['eligible_to_expand_full17'],new_jobs_submitted=False,paper_eligible=False,automatic_summary_not_new_human_review=True,technical_report_sha256=sha(reportpath),compact_audit_sha256=sha(final/'compact_audit.json'),issues='No new experimental issue when all gates pass. Preserve negative results and limits; no DRAM/overlap claim.',next='Deliver automatic TXT and compact manifests; no additional research/jobs.')
  handoff=sync_event(event,attempt,'success')
  stage='manifest'
  manifest=dict(schema='event_trace_delivery_v1',status='SUCCESS',analysis_pass=True,generated_utc=stamp,automatic_summary_not_new_human_review=True,run=str(run),analysis_dir=str(analysis),final_dir=str(final),attempt=str(attempt),
   report_path=str(reportpath),report_sha256=sha(reportpath),report_bytes=reportpath.stat().st_size,script_sha256=sha(__file__),reviewed_input_hashes=PINS,current_rules=rules,readiness=ready,source_paths=refs,analysis_files_sha256=analysis_hashes,
   final_files_sha256={n:sha(final/n) for n in ('TECHNICAL_REPORT_20261008.txt','compact_audit.json')},handoff=handoff,expand_full17=a['expansion_gate']['eligible_to_expand_full17'],new_jobs_submitted=False,local_desktop_delivery_performed_by_this_script=False)
  dump(final/'delivery_manifest.json',manifest)
  result=dict(status='SUCCESS',analysis_pass=True,expand_full17=a['expansion_gate']['eligible_to_expand_full17'],final_dir=str(final),report_path=str(reportpath),report_sha256=sha(reportpath),report_bytes=reportpath.stat().st_size,manifest_sha256=sha(final/'delivery_manifest.json'),compact_audit_sha256=sha(final/'compact_audit.json'),attempt=str(attempt))
  dump(attempt/'SUCCESS.json',result);print(json.dumps(result,ensure_ascii=False));return 0
 except Exception as exc:
  fail=dict(status='FAILED',stage=stage,reason=repr(exc),traceback=traceback.format_exc(),attempt=str(attempt),analysis_dir=str(analysis),final_dir=str(final),analysis_outputs_retained=analysis.exists(),final_output_created=created,current_rules=rules,readiness=ready,handoff=handoff)
  dump(attempt/'FAILED.json',fail)
  if created:dump(final/'FAILED.json',fail)
  try:
   if sha(RECORDER)==PINS[str(RECORDER)]:
    ev=dict(id='event-trace-20261008-01-auto-final-failed-'+attempt.name,kind='automatic_final_analysis',status='FAILED',evidence=str(attempt),reason=repr(exc),stage=stage,paper_eligible=False,issues='Automatic finalization stopped; preserve all outputs; no success/performance claim.',next='Manual diagnosis; no automatic retry or new jobs.')
    fail['failure_handoff']=sync_event(ev,attempt,'failed');dump(attempt/'FAILED.json',fail)
  except Exception as err:fail['failure_handoff_error']=repr(err);dump(attempt/'FAILED.json',fail)
  print(json.dumps(dict(status='FAILED',stage=stage,reason=repr(exc),attempt=str(attempt)),ensure_ascii=False));return 2
if __name__=='__main__':sys.exit(main(sys.argv[1:]))
