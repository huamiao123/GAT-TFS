#!/usr/bin/env python3
"""Pair-plan experiment harness: retain all raw/parsed records before strict checks."""
import pathlib,sys,json,shlex,statistics,math,time,os,struct,re,resource
from evidence_utils import ROOT,save,sha,invoke,fixtures,csvfile,baseline
resource.setrlimit(resource.RLIMIT_CORE,(0,0))
run,build=map(pathlib.Path,sys.argv[1:3]);mode=sys.argv[3]
MODES=('scalar','dyn_i','dyn_s','exact_i','exact_s')
FOCUS={'mycielskian19','reddit','roadNet-CA','amazon0601'}
def method_names(full):
 a=['paper_tfs','pack_epi_degree','legacy_s_degree']
 if full:a.insert(2,'legacy_i_degree')
 for prefix,mm in ([('adj',range(5)),('opt',range(5)),('rand',(0,2,4))] if full else [('adj',(4,)),('opt',(1,2,3,4))]):
  a.extend(prefix+'_'+MODES[m]+'_degree' for m in mm)
 a.append('pack_epi_qguard');a.extend('opt_'+MODES[m]+'_qguard' for m in (range(5) if full else (1,2,3,4)))
 return set(a)
def write_fixture(name,rows,kind):
 folder=run/'fixtures'/name;folder.mkdir(parents=True);n=len(rows);row=[0];col=[]
 for v in rows:col.extend(v);row.append(len(col))
 path=folder/(name+'.csrbin')
 with path.open('wb') as f:
  f.write(struct.pack('<IIIQQQ',0,0,2,n,n,len(col)))
  for fmt,values in [('I',row),('I',col),('f',[1.]*len(col))]:f.write(struct.pack('<'+fmt*len(values),*values))
 return dict(graph=name,path=str(path),N=n,E=len(col),fixture=kind)
if mode=='smoke':
 graphs=fixtures(run)
 for n in [1,15,16,17,31,33,65]:
  degs=[0,1,16,32,63,64,65,127,128,255,256,257,512];rows=[]
  for i in range(n):
   d=degs[(i+1)%len(degs)] if i%4!=0 or n==1 else 0;rows.append([(i*3+j*7)%n for j in range(d)])
  graphs.append(write_fixture('state_tail_'+str(n),rows,'empty_tail_unsorted_duplicate'))
 graphs.append(write_fixture('plan_exact_tail',[[(j*7)%17 for j in range(65)] for i in range(17)],'exact_identical_65_tail17'))
 graphs.append(write_fixture('plan_exact_unsorteddup',[[(j*7+j//3)%33 for j in range(257)] for i in range(33)],'exact_identical_unsorted_duplicate_257_tail33'))
 rows=[]
 for i in range(16):
  row=[7]*65;row[0]=i;row[64]=(i+1)%16;rows.append(row)
 graphs.append(write_fixture('plan_exact_mismatch',rows,'every_pair_differs_in_each_window'))
elif mode=='synthetic':
 candidates=[]
 for f in (ROOT/'runs/grouping-balance-20261007-01/logs').glob('*/graph_list.json'):
  items=json.loads(f.read_text())
  if len(items)==3 and all(x['graph'].startswith('regular128_reuse') for x in items):candidates.append(items)
 assert len(candidates)==1,'Expected one frozen equal-marginal synthetic cohort'
 graphs=candidates[0]
else:
 inv={r['graph']:r for r in json.loads((ROOT/'docs/CURRENT_GRAPH_INVENTORY_20261004.json').read_text())};graphs=[inv[n] for n in mode.split(',')]
os.environ['PAIRPLAN_SMOKE']='1' if mode=='smoke' else '0'
save(run/'graph_list.json',graphs);states=[];alltables={};summary=[]
CHECK_PAIRS={('kernel_FP32','paper_tfs'),('e2e_FP32_final','paper_tfs'),('e2e_BF16_final','paper_tfs'),('e2e_BF16_final','source_MKL_FP32')}
BOUNDARIES={'kernel_FP32','e2e_FP32','H1_BF16','e2e_BF16'}
extras={f'smoke_m{sc}_a{ac}_p{pi}_v{md}' for sc in [64,256,0] for ac in [0,1] for pi in range(3) for md in range(5)}
stresses={x.replace('smoke_','stress_',1) for x in extras}
for graph in graphs:
 folder=run/graph['graph'];folder.mkdir();state=dict(graph);start=time.time();tables={};full=mode in ('smoke','synthetic') or graph['graph'] in FOCUS
 os.environ['PAIRPLAN_ABLATIONS']='1' if full else '0';os.environ['PAIRPLAN_PMU']='1' if full and mode!='smoke' else '0'
 state['experiment_env']={k:os.environ[k] for k in ('PAIRPLAN_SMOKE','PAIRPLAN_ABLATIONS','PAIRPLAN_PMU')}
 save(folder/'experiment_env.json',state['experiment_env']);error=None
 try:
  path=pathlib.Path(graph['path']);state['actual_input_sha256']=sha(path);state['binary_sha256']=sha(build/'plan_methods');save(folder/'binary_input_hashes.json',dict(binary=str(build/'plan_methods'),binary_sha256=state['binary_sha256'],input=str(path),input_sha256=state['actual_input_sha256']))
  if 'input_sha256' in graph:assert state['actual_input_sha256']==graph['input_sha256']
  print('PAIRPLAN_START',graph['graph'],flush=True)
  if mode!='smoke':state['anchor']=baseline(invoke(folder,'original_raw',[build/'paper_original',path.parent.parent,graph['graph']]))
  invoke(folder,'methods',[build/'plan_methods',path.parent.parent,graph['graph']])
 except Exception as e:error=e
 log=(folder/'methods.log').read_text(errors='replace') if (folder/'methods.log').exists() else ''
 for line in log.splitlines():
  label,_,body=line.partition(' ')
  if label.startswith(('METHOD_','GROUP_','PAIRPLAN_')) or label=='PLAN_GATE':tables.setdefault(label,[]).append(dict(x.split('=',1) for x in shlex.split(body) if '=' in x))
 for key,rows in tables.items():csvfile(folder/(key.lower()+'.csv'),rows)
 save(folder/'parsed.json',tables)
 try:
  if error:raise error
  assert all(x.get('graph',graph['graph'])==graph['graph'] for rows in tables.values() for x in rows),'cross graph record'
  for label in ['PAIRPLAN_COMPLETE','PAIRPLAN_SEMANTIC','PAIRPLAN_GATE_COMPLETE']:
   assert len(tables.get(label,[]))==1 and tables[label][0]['pass']=='1',label
  assert tables['PAIRPLAN_SEMANTIC'][0]['values']=='19'
  assert len(tables.get('PAIRPLAN_CONFIG',[]))==1;cfg=tables['PAIRPLAN_CONFIG'][0]
  assert all(cfg[k]=='128' for k in ['D','H','F']) and cfg['numa']=='default'
  assert cfg['S']==cfg['R']=='64' and cfg['TR']=='16' and cfg['final_output']=='BF16'
  assert cfg['warmups']=='1' and cfg['consecutive']=='5' and cfg['alternating']=='10'
  assert cfg['smoke']==str(int(mode=='smoke')) and cfg['ablations']==str(int(full))
  methods=tables['PAIRPLAN_PLAN'];names={x['method'] for x in methods};lookup={x['method']:x for x in methods}
  assert len(methods)==len(names)==int(cfg['methods']) and names==method_names(full)
  assert all(x['scope']==('1' if x['method']=='paper_tfs' else '0') and x['accurate']=='0' for x in methods)
  for x in methods:
   if x['method']=='paper_tfs':assert x['kind']=='0' and x['epilogue']=='0'
   elif x['method'].startswith(('pack_','legacy_')):assert x['kind']=='3' and x['epilogue']=='1'
   else:assert x['kind']=='4' and x['epilogue']=='1'
  expected_plans={'adj_degree','opt_degree','opt_qguard'}|({'rand_degree'} if full else set())
  structrows=tables['PAIRPLAN_STRUCTURE'];structures={x['plan']:x for x in structrows}
  assert len(structrows)==len(expected_plans) and set(structures)==expected_plans
  plan_gates=tables['PLAN_GATE'];assert len(plan_gates)==len(expected_plans) and {x['plan'] for x in plan_gates}==expected_plans and all(x['pass']=='1' for x in plan_gates)
  assert len(tables['PAIRPLAN_SETUP'])==len(expected_plans)
  for st in structrows:
   e=int(st['source_edges']);dh=int(st['dynamic_shared_heads']);eh=int(st['exact_shared_edges'])
   assert e==int(cfg['E']) and 0<=eh<=dh<=e and int(st['merge_steps'])+dh==e
  numerical=names|(extras if mode=='smoke' else set());equiv=(names-{'paper_tfs'})|((extras|stresses) if mode=='smoke' else set());profiled=(names-{'paper_tfs'})|(extras if mode=='smoke' else set())
  checks=tables['METHOD_CHECK'];eq=tables['PAIRPLAN_EQUIV']
  assert {x['method'] for x in checks}==numerical and len(checks)==4*len(numerical)
  assert {x['method'] for x in eq}==equiv and len(eq)==4*len(equiv)
  for name in numerical:
   rows=[x for x in checks if x['method']==name];assert len(rows)==4 and {(x['boundary'],x['reference']) for x in rows}==CHECK_PAIRS
   accurate='_a1_' in name
   for x in rows:
    gate=.03 if x['reference']=='source_MKL_FP32' else ((.01 if accurate else .02) if x['boundary']=='e2e_BF16_final' else (.001 if accurate else .01))
    assert x['pass']=='1' and float(x['gate'])==gate,(name,'numeric',x)
  for name in equiv:
   rows=[x for x in eq if x['method']==name];assert len(rows)==4 and {x['boundary'] for x in rows}==BOUNDARIES and all(x['pass']==x['bitwise']=='1' for x in rows),(name,'bitwise')
  for label in ['PAIRPLAN_COUNTERS','PAIRPLAN_WORK_GATE','PAIRPLAN_PROFILE_GATE']:
   rows=tables[label];assert {x['method'] for x in rows}==profiled and len(rows)==2*len(profiled),label
   for name in profiled:
    rr=[x for x in rows if x['method']==name];assert len(rr)==2 and {x['layer'] for x in rr}=={'1','2'}
    if label!='PAIRPLAN_COUNTERS':assert all(x['pass']=='1' for x in rr)
  for c in tables['PAIRPLAN_COUNTERS']:
   name=c['method'];e=int(cfg['E']);assert c['profile_path']=='production_BF16_epilogue';assert int(c['source_visits'])==e and int(c['feature_adds'])==e*128
   assert c['decode_rows']==c['expected_decode'] and c['csr_reads']==c['expected_csr'] and c['shared_heads']==c['expected_hits']
   assert c['projection_scopes']==c['expected_scopes'] and c['windows']==c['expected_windows']
   assert int(c['amx_tdp'])==int(c['expected_scopes'])*32*(2 if c['accurate']=='1' else 1)
   for left,right in [('partial_load_bytes','expected_loads'),('partial_store_bytes','expected_stores'),('packed_bytes','expected_packed'),('direct_rows','expected_direct'),('prefetch_requests','expected_prefetch')]:assert c[left]==c[right]
   assert c['partial_zero_bytes']==c['packing_load_bytes']=='0'
   if name in lookup:method=lookup[name];pi=int(method['plan_index']);md=int(method['mode']);pair=int(method['pair']);plan=method['plan']
   else:
    mm=re.fullmatch(r'smoke_m(0|64|256)_a([01])_p([012])_v([0-4])',name);assert mm
    pi,md=int(mm[3]),int(mm[4]);pair=0;plan=['adj_degree','opt_degree','rand_degree'][pi]
   if pi>=0:
    st=structures[plan];hits=st['dynamic_shared_heads'] if md in [1,2] else (st['exact_shared_edges'] if md in [3,4] else '0')
    assert c['shared_heads']==hits
    assert int(c['decode_rows'])==e-(int(hits) if md in [2,4] else 0)
    assert int(c['csr_reads'])==e-(int(hits) if md==4 else 0)
    if md in [1,2]:assert c['runtime_head_comparisons']==c['comparisons']==st['comparisons']
    if md in [3,4]:assert c['exact_pair_edges']==st['exact_shared_edges'] and c['exact_pair_windows']==st['exact_windows'] and c['runtime_head_comparisons']=='0'
   elif pair:assert c['shared_heads']==structures['adj_degree']['dynamic_shared_heads']
  if mode=='smoke':
   assert len(tables.get('PAIRPLAN_COVERAGE',[]))==1;cv=tables['PAIRPLAN_COVERAGE'][0];assert cv['pass']=='1'
   if graph['graph'] in ['plan_exact_tail','plan_exact_unsorteddup']:assert cv['required']=='positive' and int(cv['adj_exact_shared_edges'])>0
   if graph['graph']=='plan_exact_mismatch':assert cv['required']=='zero' and cv['adj_exact_shared_edges']=='0'
   assert not tables.get('PAIRPLAN_TIME')
  else:
   times={}
   for x in tables['PAIRPLAN_TIME']:
    assert x['method'] in names|{'source_mkl_fp32'} and math.isfinite(float(x['ms'])) and float(x['ms'])>0
    times.setdefault((x['method'],x['kind']),[]).append(x)
   assert set(times)=={(m,k) for m in names|{'source_mkl_fp32'} for k in ['consecutive','alternating']}
   for (m,k),rows in times.items():
    nr=5 if k=='consecutive' else 10;assert len(rows)==nr and {int(x['repeat']) for x in rows}==set(range(nr))
    vals=[float(x['ms']) for x in rows];summary.append(dict(graph=graph['graph'],method=m,kind=k,median_ms=statistics.median(vals),min_ms=min(vals),max_ms=max(vals),repeats=nr))
   if full:
    walls=tables.get('PAIRPLAN_PMU_WALL',[]);expected_pmu={'pack_epi_degree','legacy_s_degree','opt_dyn_s_degree','opt_exact_s_degree'}
    assert len(walls)==12 and {(x['method'],int(x['repeat'])) for x in walls}=={(m,k) for m in expected_pmu for k in range(3)}
  state.update(status='PASS',checks=len(checks),equiv=len(eq),profile=len(tables['PAIRPLAN_PROFILE_GATE']),work=len(tables['PAIRPLAN_WORK_GATE']))
  print('PAIRPLAN_PASS',graph['graph'],flush=True)
 except Exception as exc:state.update(status='FAILED',reason=repr(exc));print('PAIRPLAN_FAILED',graph['graph'],repr(exc),flush=True)
 state['wall_seconds']=time.time()-start;states.append(state);save(folder/'status.json',state);save(run/'suite_status.json',states)
 for key,rows in tables.items():alltables.setdefault(key,[]).extend(rows)
 for key,rows in alltables.items():csvfile(run/(key.lower()+'.csv'),rows)
 csvfile(run/'summary.csv',summary)
completion=dict(study='pair_plan',mode=mode,graphs=len(states),passed=sum(x['status']=='PASS' for x in states),failed=sum(x['status']!='PASS' for x in states),checks=sum(x.get('checks',0) for x in states),equiv=sum(x.get('equiv',0) for x in states),profile=sum(x.get('profile',0) for x in states),work=sum(x.get('work',0) for x in states))
save(run/'completion.json',completion);print('PAIRPLAN_SUITE_COMPLETE',json.dumps(completion),flush=True);sys.exit(2 if completion['failed'] else 0)
