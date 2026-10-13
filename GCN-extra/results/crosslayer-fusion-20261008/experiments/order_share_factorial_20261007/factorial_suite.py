#!/usr/bin/env python3
"""Fixed-pair order x decode factorial; immutable inputs and strict full gates.
Only frozen prior CSR manifests are reused. No graph generation, candidate
selection or performance-dependent seed choice occurs in this suite.
"""
import pathlib,sys,json,shlex,statistics,math,time,os,resource,traceback
from evidence_utils import ROOT,save,sha,invoke,csvfile
resource.setrlimit(resource.RLIMIT_CORE,(0,0))
run,build=map(pathlib.Path,sys.argv[1:3]);mode=sys.argv[3]
PRIOR=ROOT/'runs/pair-plan-20261007-01/logs'
FOCUS={'mycielskian19','reddit','roadNet-CA','amazon0601'}
ORDERS=('adj','pairs_s1','pairs_s2','pairs_s3')
MODE_SUFFIX=((0,'scalar'),(3,'exact_i'),(4,'exact_s'))
ORDER=['pack_epi_degree']+['slot_'+order+'_'+suffix for order in ORDERS for _,suffix in MODE_SUFFIX]
NAMES=set(ORDER);assert len(NAMES)==13
MODES={'pack_epi_degree':0};PLANNAME={};PLANINDEX={};SEEDS={'pairs_s1':20261007,'pairs_s2':20261017,'pairs_s3':20261027}
for pi,order in enumerate(ORDERS):
 for md,suffix in MODE_SUFFIX:
  name='slot_'+order+'_'+suffix;MODES[name]=md;PLANNAME[name]='adj_degree' if pi==0 else order;PLANINDEX[name]=pi
CHECKS={('kernel_FP32','paper_tfs'),('e2e_FP32_final','paper_tfs'),('e2e_BF16_final','paper_tfs'),('e2e_BF16_final','source_MKL_FP32')}
BOUNDARIES={'kernel_FP32','e2e_FP32','H1_BF16','e2e_BF16'}
SMOKE_NAMES={'window_tail','window_high_tail','window_scope_tail'}|{'state_tail_'+str(n) for n in (1,15,16,17,31,33,65)}|{'plan_exact_tail','plan_exact_unsorteddup','plan_exact_mismatch'}

def frozen_graphs(job):
 prior=PRIOR/('plan-'+str(job));manifest=prior/'graph_list.json';items=json.loads(manifest.read_text())
 out=[]
 for item in items:
  g=dict(item);prev=prior/g['graph']/'status.json';status=json.loads(prev.read_text())
  assert status['status']=='PASS',(job,g['graph'],'frozen source graph not accepted')
  g['input_sha256']=status['actual_input_sha256'];g['immutable_source_manifest']=str(manifest)
  g['immutable_source_manifest_sha256']=sha(manifest);g['immutable_source_status']=str(prev);g['immutable_source_status_sha256']=sha(prev)
  out.append(g)
 return out

if mode=='smoke':
 graphs=frozen_graphs(10889177);assert len(graphs)==13 and {g['graph'] for g in graphs}==SMOKE_NAMES
elif mode=='synthetic':
 graphs=frozen_graphs(10889185);assert len(graphs)==3 and {g['graph'] for g in graphs}=={'regular128_reuse1','regular128_reuse4','regular128_reuse16'}
else:
 wanted=mode.split(',');assert wanted and len(set(wanted))==len(wanted) and set(wanted)<=FOCUS,'Only frozen diagnostic4 real graphs authorized in this protocol'
 available={g['graph']:g for job in (10889181,10889182) for g in frozen_graphs(job)}
 assert set(available)==FOCUS
 graphs=[available[name] for name in wanted]
os.environ['FACT_SMOKE']='1' if mode=='smoke' else '0';os.environ['FACT_PMU']='0' if mode=='smoke' else '1'
save(run/'graph_list.json',graphs);states=[];combined={};summary=[]

def finite_number(value,positive=False):
 x=float(value);assert math.isfinite(x) and (x>0 if positive else x>=0),value;return x


def records_for(tables,label,name):return [r for r in tables.get(label,[]) if r.get('method')==name]


def audit_pmu(tables):
 expected={(name,k) for name in NAMES for k in range(3)}
 walls=tables.get('FACT_PMU_WALL',[]);totals=tables.get('METHOD_PMU',[]);threads=tables.get('METHOD_PMU_THREAD',[])
 assert len(walls)==len(totals)==39 and len(threads)==39*32,'PMU matrix count must be 13 x 3 x 32'
 assert {(x['method'],int(x['repeat'])) for x in walls}==expected
 assert {(x['method'],int(x['repeat'])) for x in totals}==expected
 for x in walls:finite_number(x['ms'],True)
 events=('cycles','instructions','cache_misses','cache_references','amx_busy')
 for x in totals:
  key=x['method'],int(x['repeat'])
  assert all(x[k]=='1' for k in ('available','coverage_complete','scaled_complete','amx_busy_requested'))
  assert all(x[k]=='32' for k in ('initial_threads','final_threads','counted_threads','scaled_threads'))
  assert all(x[k]=='0' for k in ('failed_threads','zero_running_threads','new_threads','disappeared_threads','errno'))
  assert x['error']=='' and x['event_count']=='5' and x['amx_busy_type']=='raw' and x['amx_busy_config']=='0x02b7'
  rr=[z for z in threads if (z['method'],int(z['repeat']))==key]
  assert len(rr)==32 and len({z['tid'] for z in rr})==32
  for z in rr:
   assert all(z[k]=='1' for k in ('available','present_at_stop','scaled_valid'))
   assert z['zero_running_time']==z['errno']=='0' and z['error']==''
   assert int(z['time_enabled_ns'])>=int(z['time_running_ns'])>0
   assert finite_number(z['scale'],True)>=1
   assert 0<finite_number(z['running_fraction'],True)<=1.000000001
   for ev in events:
    assert int(z[ev+'_raw'])>=0;finite_number(z[ev+'_scaled'])
  for ev in events:
   assert int(x[ev+'_raw'])==sum(int(z[ev+'_raw']) for z in rr)
   finite_number(x[ev+'_scaled'])
   assert math.isclose(float(x[ev+'_scaled']),sum(float(z[ev+'_scaled']) for z in rr),rel_tol=1e-9,abs_tol=1e-3)
  for ev in ('cycles','instructions','amx_busy'):finite_number(x[ev+'_scaled'],True)

for graph in graphs:
 folder=run/graph['graph'];folder.mkdir();start=time.time();state=dict(graph);tables={};error=None;graph_summary=[]
 try:
  path=pathlib.Path(graph['path']);state['actual_input_sha256']=sha(path)
  assert state['actual_input_sha256']==graph['input_sha256'],'Frozen CSR file changed'
  state['binary_sha256']=sha(build/'factorial_methods')
  state['experiment_env']={k:os.environ[k] for k in ('FACT_SMOKE','FACT_PMU')}
  save(folder/'binary_input_hashes.json',dict(binary=str(build/'factorial_methods'),binary_sha256=state['binary_sha256'],input=str(path),input_sha256=state['actual_input_sha256']))
  save(folder/'experiment_env.json',state['experiment_env'])
  print('FACT_START',graph['graph'],flush=True)
  invoke(folder,'methods',[build/'factorial_methods',path.parent.parent,graph['graph']])
 except Exception as exc:error=exc
 text=(folder/'methods.log').read_text(errors='replace') if (folder/'methods.log').exists() else ''
 parse_errors=[]
 for line_no,line in enumerate(text.splitlines(),1):
  label,_,body=line.partition(' ')
  if label.startswith(('METHOD_','PAIRPLAN_','FACT_','GROUP_')) or label=='PLAN_GATE':
   try:
    row={}
    for token in shlex.split(body):
     if '=' not in token:continue
     k,v=token.split('=',1);assert k not in row,'duplicate field '+k;row[k]=v
    tables.setdefault(label,[]).append(row)
   except Exception as exc:parse_errors.append(dict(line=line_no,error=repr(exc)))
 save(folder/'parsed.json',tables)
 if parse_errors:save(folder/'parse_errors.json',parse_errors)
 for key,rows in tables.items():csvfile(folder/(key.lower()+'.csv'),rows)
 try:
  if error:raise error
  assert not parse_errors,parse_errors
  assert all(x.get('graph',graph['graph'])==graph['graph'] for rr in tables.values() for x in rr),'Cross-graph records'
  assert all(x.get('pass','1')!='0' for rr in tables.values() for x in rr),'A raw gate failed'
  for label in ('FACT_COMPLETE','FACT_GATE_COMPLETE','PAIRPLAN_SEMANTIC'):
   assert len(tables.get(label,[]))==1 and tables[label][0]['pass']=='1',label
  semantic=tables['PAIRPLAN_SEMANTIC'][0];assert semantic['values']=='19' and semantic['bitwise']=='1'
  cfg=tables['FACT_CONFIG'];assert len(cfg)==1;cfg=cfg[0]
  assert all(cfg[k]=='128' for k in ('D','H','F')) and cfg['S']==cfg['R']=='64' and cfg['TR']=='16' and cfg['M']=='FULL'
  assert cfg['methods']=='13' and cfg['modes']=='0,3,4' and cfg['numa']=='default' and cfg['final_output']=='BF16' and cfg['copy_in_compute']=='0'
  assert cfg['warmup_consecutive']==cfg['warmup_each_alternating_switch']=='1' and cfg['consecutive']=='5' and cfg['alternating']=='10' and cfg['smoke']==str(int(mode=='smoke'))
  assert int(cfg['N'])==int(graph['N']) and int(cfg['E'])==int(graph['E'])
  done=tables['FACT_COMPLETE'][0];assert done['methods']=='13' and done['smoke']==cfg['smoke']
  methods=tables['FACT_METHOD'];assert len(methods)==13 and [x['method'] for x in methods]==ORDER,'Candidate order changed'
  for x in methods:
   name=x['method'];assert x['kind']==('3' if name=='pack_epi_degree' else '4') and x['scope']==x['accurate']=='0' and x['epilogue']=='1'
   assert int(x['mode'])==MODES[name] and int(x['plan_index'])==int(x['order_index'])==PLANINDEX.get(name,-1)
   assert int(x['order_seed'])==SEEDS.get(PLANNAME.get(name,''),0)
  plans=tables['FACT_PLAN'];assert len(plans)==4 and {x['plan'] for x in plans}=={'adj_degree','pairs_s1','pairs_s2','pairs_s3'}
  pl={x['plan']:x for x in plans};assert len({(x['tiles'],x['mask_bytes']) for x in plans})==1
  assert len({(x['shared_heads'],x['exact_shared_edges'],x['exact_windows']) for x in plans})==1,'Pair-preserving order changed reusable work'
  for pi,name in enumerate(('adj_degree','pairs_s1','pairs_s2','pairs_s3')):
   assert int(pl[name]['order_index'])==pi and int(pl[name]['order_seed'])==SEEDS.get(name,0)
  pg=tables['PLAN_GATE'];assert len(pg)==1 and pg[0]['plan']=='adj_degree' and pg[0]['pass']=='1'
  structure=tables['PAIRPLAN_STRUCTURE'];assert len(structure)==1 and structure[0]['plan']=='adj_degree'
  adj=structure[0];e=int(cfg['E']);exact=int(adj['exact_shared_edges']);exact_windows=int(adj['exact_windows'])
  assert int(adj['source_edges'])==e and int(adj['dynamic_shared_heads'])==int(pl['adj_degree']['shared_heads']) and exact==int(pl['adj_degree']['exact_shared_edges']) and exact_windows==int(pl['adj_degree']['exact_windows'])
  assert 0<=exact<=int(adj['dynamic_shared_heads'])<=e//2
  proofs=tables['FACT_PAIR_PROOF'];assert len(proofs)==3 and {x['plan'] for x in proofs}==set(SEEDS)
  for proof in proofs:
   name=proof['plan'];assert int(proof['seed'])==SEEDS[name] and proof['invalid']=='0'
   assert all(proof[k]=='1' for k in ('pass','pair_set_preserved','pair_direction_preserved','odd_row_last','exact_mask_remapped','shared_heads_preserved_by_pair_set'))
   assert proof['fnv64']==pl[name]['slot_hash']
   assert int(proof['actual_exact_shared_edges'])==int(proof['expected_exact_shared_edges'])==exact
   assert int(proof['actual_exact_windows'])==int(proof['expected_exact_windows'])==exact_windows
  if graph['graph'] in ('plan_exact_tail','plan_exact_unsorteddup'):assert exact>0,'Positive exact-path fixture did not cover sharing'
  if graph['graph']=='plan_exact_mismatch':assert exact==0,'Mismatching window admitted exact sharing'
  checks=tables['METHOD_CHECK'];eq=tables['PAIRPLAN_EQUIV'];executed=tables['FACT_EXEC_EQUIV']
  assert len(checks)==len(eq)==len(executed)==52 and all({x['method'] for x in rr}==NAMES for rr in (checks,eq,executed))
  for name in ORDER:
   cc=records_for(tables,'METHOD_CHECK',name);ee=records_for(tables,'PAIRPLAN_EQUIV',name);xx=records_for(tables,'FACT_EXEC_EQUIV',name)
   assert len(cc)==len(ee)==len(xx)==4
   assert {(x['boundary'],x['reference']) for x in cc}==CHECKS and {x['boundary'] for x in ee}=={x['boundary'] for x in xx}==BOUNDARIES
   for x in cc:
    gate=.03 if x['reference']=='source_MKL_FP32' else (.02 if x['boundary']=='e2e_BF16_final' else .01)
    assert x['pass']=='1' and float(x['gate'])==gate
    assert finite_number(x['relative_L2'])<gate and finite_number(x['normalized_max'])<gate
    finite_number(x['max_abs']);finite_number(x['mean_abs'])
   assert all(x['pass']==x['bitwise']=='1' and x['scope']==x['accurate']=='0' for x in ee)
   for x in xx:
    assert x['pass']==x['bitwise']=='1' and int(x['mode'])==int(x['execution_mode'])==MODES[name]
    assert x['execution_name']==('pack_epi_degree' if name=='pack_epi_degree' else 'slot_exec')
  for label in ('PAIRPLAN_COUNTERS','PAIRPLAN_WORK_GATE','PAIRPLAN_PROFILE_GATE'):
   rr=tables[label];assert len(rr)==26 and {x['method'] for x in rr}==NAMES
   for name in NAMES:
    ss=[x for x in rr if x['method']==name];assert len(ss)==2 and {x['layer'] for x in ss}=={'1','2'}
    if label!='PAIRPLAN_COUNTERS':assert all(x['pass']=='1' for x in ss)
    if label=='PAIRPLAN_PROFILE_GATE':assert all(x['bitwise']=='1' for x in ss)
  counters=tables['PAIRPLAN_COUNTERS']
  common=('source_visits','feature_adds','projection_scopes','windows','amx_tdp','partial_zero_bytes','partial_load_bytes','partial_store_bytes','packing_load_bytes','direct_rows','packed_bytes','prefetch_requests','prefetch_index_reads','relu_elements')
  for layer in ('1','2'):
   rr=[x for x in counters if x['layer']==layer];baseline=next(x for x in rr if x['method']=='pack_epi_degree')
   assert all(all(x[k]==baseline[k] for k in common) for x in rr),'Projection/addition/state/prefetch/activation budget differs across factorial'
   for md,_ in MODE_SUFFIX:
    group=[x for x in rr if x['method']!='pack_epi_degree' and MODES[x['method']]==md]
    assert len(group)==4
    ref={k:v for k,v in group[0].items() if k!='method'}
    assert all({k:v for k,v in x.items() if k!='method'}==ref for x in group),'Same-mode counters changed with pair order'
   for order in ORDERS:
    ci=next(x for x in rr if x['method']=='slot_'+order+'_exact_i')
    cs=next(x for x in rr if x['method']=='slot_'+order+'_exact_s')
    changed={'method','decode_rows','csr_reads','exact_shared_edges','expected_decode','expected_csr'}
    assert {k:v for k,v in ci.items() if k not in changed}=={k:v for k,v in cs.items() if k not in changed},'ExactI/ExactS differ beyond theoretically allowed counters'
  for x in counters:
   name=x['method'];md=MODES[name];saved=exact if md==4 else 0;hits=exact if md in (3,4) else 0
   assert int(x['source_visits'])==e and int(x['feature_adds'])==128*e
   assert int(x['decode_rows'])==int(x['csr_reads'])==e-saved and int(x['shared_heads'])==hits
   assert x['comparisons']==x['runtime_head_comparisons']=='0'
   assert x['scope']==x['accurate']=='0' and x['profile_path']=='production_BF16_epilogue'
   assert int(x['amx_tdp'])==32*int(x['projection_scopes']) and x['partial_zero_bytes']==x['packing_load_bytes']=='0'
   if md==0:
    assert all(x[k]=='0' for k in ('exact_mask_bytes_read','exact_pair_flags_tested','exact_pair_windows','exact_pair_edges','exact_shared_edges'))
   else:
    assert int(x['exact_pair_windows'])==exact_windows and int(x['exact_pair_edges'])==exact and int(x['exact_shared_edges'])==saved
   for left,right in (('partial_load_bytes','expected_loads'),('partial_store_bytes','expected_stores'),('packed_bytes','expected_packed'),('direct_rows','expected_direct'),('prefetch_requests','expected_prefetch'),('decode_rows','expected_decode'),('csr_reads','expected_csr'),('projection_scopes','expected_scopes'),('windows','expected_windows'),('shared_heads','expected_hits')):
    assert x[left]==x[right],(name,left,x[left],right,x[right])
  slots=tables['FACT_SLOT'];expected_slot_count=13 if mode=='smoke' else 195
  assert len(slots)==expected_slot_count and int(done['slot_checks'])==expected_slot_count
  assert [int(x['serial']) for x in slots]==list(range(len(slots)))
  address_keys=[k for k in slots[0] if k.endswith('_ptr') and k!='execution_method_ptr']+['tiles_size','mask_size','tiles_capacity','mask_capacity']
  assert all(k in slots[0] for k in ('plan_ptr','tiles_ptr','mask_ptr','h0b_ptr','wv1_ptr','wv2_ptr','h1b_ptr','h2b_ptr','row_ptr','col_ptr','execution_method_ptr'))
  for x in slots:
   name=x['method'];assert name in NAMES and x['stable']==x['pass']=='1' and int(x['mode'])==int(x['execution_mode'])==MODES[name] and x['copy_in_compute']=='0'
   assert all(x[k]==slots[0][k] for k in address_keys),'Pointer or allocation changed'
   assert x['slot_hash']==x['expected_hash'];finite_number(x['copy_ms'])
   assert x['uses_slot']==str(int(name!='pack_epi_degree'))
   assert int(x['order_index'])==PLANINDEX.get(name,-1) and int(x['order_seed'])==SEEDS.get(PLANNAME.get(name,''),0)
   assert x['execution_name']==('pack_epi_degree' if name=='pack_epi_degree' else 'slot_exec')
   if x['uses_slot']=='1':assert x['slot_hash']==pl[PLANNAME[name]]['slot_hash']
  planned_slots=[x for x in slots if x['uses_slot']=='1'];assert len({x['execution_method_ptr'] for x in planned_slots})==1
  for name in NAMES:
   pointers={x['execution_method_ptr'] for x in slots if x['method']==name}
   assert len(pointers)==1 and {x['execution_method_ptr'] for x in executed if x['method']==name}==pointers,'Checked execution object does not match timed execution object'
  expected_slots={(m,'correctness',0) for m in NAMES}
  if mode!='smoke':expected_slots|={(m,'same_slot_consecutive',0) for m in NAMES}|{(m,'same_slot_alternating',k) for m in NAMES for k in range(10)}|{(m,'pmu',k) for m in NAMES for k in range(3)}
  assert {(x['method'],x['phase'],int(x['repeat'])) for x in slots}==expected_slots
  if mode=='smoke':
   assert not tables.get('FACT_TIME') and not tables.get('FACT_PMU_WALL') and not tables.get('METHOD_PMU') and not tables.get('METHOD_PMU_THREAD')
  else:
   times={}
   for x in tables['FACT_TIME']:
    assert x['method'] in NAMES;finite_number(x['ms'],True)
    kind=x['kind'];assert kind in ('same_slot_consecutive','same_slot_alternating')
    rep=int(x['repeat']);expected_order=ORDER.index(x['method']) if kind=='same_slot_consecutive' or rep%2==0 else 12-ORDER.index(x['method'])
    assert int(x['order'])==expected_order and x['warmups_before_series' if kind=='same_slot_consecutive' else 'warmups_before_measure']=='1'
    times.setdefault((x['method'],kind),[]).append(x)
   assert set(times)=={(m,k) for m in NAMES for k in ('same_slot_consecutive','same_slot_alternating')}
   for (name,kind),rr in times.items():
    nr=5 if kind=='same_slot_consecutive' else 10;assert len(rr)==nr and {int(x['repeat']) for x in rr}==set(range(nr))
    vals=[float(x['ms']) for x in rr]
    graph_summary.append(dict(graph=graph['graph'],method=name,kind=kind,median_ms=statistics.median(vals),min_ms=min(vals),max_ms=max(vals),repeats=nr))
   audit_pmu(tables)
  state.update(status='PASS',checks=52,equiv=52,exec_equiv=52,profile=26,work=26,slot_checks=len(slots),pmu_regions=0 if mode=='smoke' else 39)
  summary.extend(graph_summary);print('FACT_PASS',graph['graph'],flush=True)
 except Exception as exc:
  state.update(status='FAILED',reason=repr(exc),traceback=traceback.format_exc())
  print('FACT_FAILED',graph['graph'],repr(exc),flush=True)
 state['wall_seconds']=time.time()-start;states.append(state);save(folder/'status.json',state);save(run/'suite_status.json',states)
 for label,rows in tables.items():combined.setdefault(label,[]).extend(rows)
 for label,rows in combined.items():csvfile(run/(label.lower()+'.csv'),rows)
 csvfile(run/'summary.csv',summary)
completion=dict(study='order_share_factorial',mode=mode,graphs=len(states),passed=sum(x['status']=='PASS' for x in states),failed=sum(x['status']!='PASS' for x in states))
for field in ('checks','equiv','exec_equiv','profile','work','slot_checks','pmu_regions'):completion[field]=sum(x.get(field,0) for x in states)
save(run/'completion.json',completion);print('FACT_SUITE_COMPLETE',json.dumps(completion),flush=True);sys.exit(2 if completion['failed'] else 0)
