#!/usr/bin/env python3
"""Same-allocation traversal controls; immutable old fixtures and explicit warmup protocol."""
import pathlib,sys,json,shlex,statistics,math,time,os,resource
from evidence_utils import ROOT,save,sha,invoke,csvfile
resource.setrlimit(resource.RLIMIT_CORE,(0,0))
run,build=map(pathlib.Path,sys.argv[1:3]);mode=sys.argv[3];assert mode in ('smoke','synthetic')
prior=ROOT/'runs/pair-plan-20261007-01/logs'/('plan-10889177' if mode=='smoke' else 'plan-10889185')
graphs=json.loads((prior/'graph_list.json').read_text());assert len(graphs)==(13 if mode=='smoke' else 3)
for g in graphs:
 prev=json.loads((prior/g['graph']/'status.json').read_text());assert prev['status']=='PASS';g['input_sha256']=prev['actual_input_sha256'];g['immutable_source_manifest']=str(prior/'graph_list.json')
os.environ['ORDERCTRL_SMOKE']='1' if mode=='smoke' else '0';os.environ['ORDERCTRL_PMU']='0' if mode=='smoke' else '1'
NAMES={'pack_epi_degree','slot_adj_scalar','slot_opt_scalar','slot_rand_scalar','slot_pairs_scalar'}
ORDER=['pack_epi_degree','slot_adj_scalar','slot_opt_scalar','slot_rand_scalar','slot_pairs_scalar']
PLANNAME={'slot_adj_scalar':'adj_degree','slot_opt_scalar':'opt_degree','slot_rand_scalar':'rand_degree','slot_pairs_scalar':'pairs_degree'}
CHECKS={('kernel_FP32','paper_tfs'),('e2e_FP32_final','paper_tfs'),('e2e_BF16_final','paper_tfs'),('e2e_BF16_final','source_MKL_FP32')}
BOUNDARIES={'kernel_FP32','e2e_FP32','H1_BF16','e2e_BF16'}
save(run/'graph_list.json',graphs);states=[];combined={};summary=[]
for graph in graphs:
 folder=run/graph['graph'];folder.mkdir();start=time.time();state=dict(graph);tables={};error=None
 try:
  path=pathlib.Path(graph['path']);state['actual_input_sha256']=sha(path);assert state['actual_input_sha256']==graph['input_sha256']
  state['binary_sha256']=sha(build/'order_control_methods');state['experiment_env']={k:os.environ[k] for k in ['ORDERCTRL_SMOKE','ORDERCTRL_PMU']}
  save(folder/'binary_input_hashes.json',dict(binary=str(build/'order_control_methods'),binary_sha256=state['binary_sha256'],input=str(path),input_sha256=state['actual_input_sha256']));save(folder/'experiment_env.json',state['experiment_env'])
  print('ORDERCTRL_START',graph['graph'],flush=True);invoke(folder,'methods',[build/'order_control_methods',path.parent.parent,graph['graph']])
 except Exception as exc:error=exc
 text=(folder/'methods.log').read_text(errors='replace') if (folder/'methods.log').exists() else ''
 for line in text.splitlines():
  label,_,body=line.partition(' ')
  if label.startswith(('METHOD_','PAIRPLAN_','ORDERCTRL_','GROUP_')) or label=='PLAN_GATE':tables.setdefault(label,[]).append(dict(x.split('=',1) for x in shlex.split(body) if '=' in x))
 save(folder/'parsed.json',tables)
 for key,rows in tables.items():csvfile(folder/(key.lower()+'.csv'),rows)
 try:
  if error:raise error
  assert all(x.get('graph',graph['graph'])==graph['graph'] for rows in tables.values() for x in rows)
  for label in ['ORDERCTRL_COMPLETE','ORDERCTRL_GATE_COMPLETE','PAIRPLAN_SEMANTIC','ORDERCTRL_PAIR_PROOF']:
   assert len(tables.get(label,[]))==1 and tables[label][0]['pass']=='1',label
  assert tables['PAIRPLAN_SEMANTIC'][0]['values']=='19'
  cfg=tables['ORDERCTRL_CONFIG'];assert len(cfg)==1;cfg=cfg[0]
  assert all(cfg[k]=='128' for k in ['D','H','F']) and cfg['S']==cfg['R']=='64' and cfg['TR']=='16' and cfg['M']=='FULL'
  assert cfg['methods']=='5' and cfg['planned_mode']=='0' and cfg['numa']=='default' and cfg['final_output']=='BF16' and cfg['copy_in_compute']=='0'
  assert cfg['warmup_consecutive']==cfg['warmup_each_alternating_switch']=='1' and cfg['consecutive']=='5' and cfg['alternating']=='10' and cfg['smoke']==str(int(mode=='smoke'))
  methods=tables['ORDERCTRL_METHOD'];assert len(methods)==5 and {x['method'] for x in methods}==NAMES
  for x in methods:
   assert x['kind']==('3' if x['method']=='pack_epi_degree' else '4') and x['scope']==x['mode']==x['accurate']=='0' and x['epilogue']=='1'
  plans=tables['ORDERCTRL_PLAN'];assert len(plans)==4 and {x['plan'] for x in plans}==set(PLANNAME.values());pl={x['plan']:x for x in plans}
  assert len({(x['tiles'],x['mask_bytes']) for x in plans})==1
  pg=tables['PLAN_GATE'];assert len(pg)==3 and {x['plan'] for x in pg}=={'adj_degree','opt_degree','rand_degree'} and all(x['pass']=='1' for x in pg)
  pf=tables['ORDERCTRL_PAIR_PROOF'][0]
  assert pf['seed']=='20261007' and pf['invalid']=='0' and all(pf[x]=='1' for x in ['pair_set_preserved','pair_direction_preserved','odd_row_last','exact_mask_remapped'])
  assert pf['fnv64']==pl['pairs_degree']['slot_hash'] and pl['pairs_degree']['shared_heads']==pl['adj_degree']['shared_heads'] and pl['pairs_degree']['exact_shared_edges']==pl['adj_degree']['exact_shared_edges']
  checks=tables['METHOD_CHECK'];eq=tables['PAIRPLAN_EQUIV'];assert len(checks)==len(eq)==20 and {x['method'] for x in checks}=={x['method'] for x in eq}==NAMES
  for name in NAMES:
   cc=[x for x in checks if x['method']==name];ee=[x for x in eq if x['method']==name];assert len(cc)==len(ee)==4 and {(x['boundary'],x['reference']) for x in cc}==CHECKS and {x['boundary'] for x in ee}==BOUNDARIES
   for x in cc:assert x['pass']=='1' and float(x['gate'])==(.03 if x['reference']=='source_MKL_FP32' else (.02 if x['boundary']=='e2e_BF16_final' else .01))
   assert all(x['pass']==x['bitwise']=='1' for x in ee)
  for label in ['PAIRPLAN_COUNTERS','PAIRPLAN_WORK_GATE','PAIRPLAN_PROFILE_GATE']:
   rr=tables[label];assert len(rr)==10 and {x['method'] for x in rr}==NAMES
   for name in NAMES:
    ss=[x for x in rr if x['method']==name];assert len(ss)==2 and {x['layer'] for x in ss}=={'1','2'}
    if label!='PAIRPLAN_COUNTERS':assert all(x['pass']=='1' for x in ss)
  counters=tables['PAIRPLAN_COUNTERS']
  for x in counters:
   e=int(cfg['E']);assert int(x['source_visits'])==int(x['decode_rows'])==int(x['csr_reads'])==e and int(x['feature_adds'])==128*e
   assert x['shared_heads']==x['comparisons']==x['exact_mask_bytes_read']==x['exact_pair_flags_tested']==x['exact_pair_edges']==x['exact_shared_edges']==x['runtime_head_comparisons']=='0'
   assert x['scope']==x['accurate']=='0' and x['profile_path']=='production_BF16_epilogue'
   assert int(x['amx_tdp'])==32*int(x['projection_scopes']) and x['partial_zero_bytes']==x['packing_load_bytes']=='0'
   for left,right in [('partial_load_bytes','expected_loads'),('partial_store_bytes','expected_stores'),('packed_bytes','expected_packed'),('direct_rows','expected_direct'),('prefetch_requests','expected_prefetch'),('decode_rows','expected_decode'),('csr_reads','expected_csr')]:assert x[left]==x[right]
  for layer in ['1','2']:
   rr=[x for x in counters if x['layer']==layer and x['method']!='pack_epi_degree'];ref={k:v for k,v in rr[0].items() if k!='method'}
   assert all({k:v for k,v in x.items() if k!='method'}==ref for x in rr),'planned same-mode source work differs'
  slots=tables['ORDERCTRL_SLOT'];assert len(slots)==(5 if mode=='smoke' else 75) and int(tables['ORDERCTRL_COMPLETE'][0]['slot_checks'])==len(slots)
  assert [int(x['serial']) for x in slots]==list(range(len(slots)))
  address_keys=[k for k in slots[0] if k.endswith('_ptr') and k!='execution_method_ptr']+['tiles_size','mask_size','tiles_capacity','mask_capacity']
  for x in slots:
   assert x['method'] in NAMES and x['stable']==x['pass']=='1' and x['mode']=='0' and x['copy_in_compute']=='0'
   assert all(x[k]==slots[0][k] for k in address_keys),'pointer or allocation changed'
   assert x['slot_hash']==x['expected_hash'] and math.isfinite(float(x['copy_ms'])) and float(x['copy_ms'])>=0
   assert x['uses_slot']==str(int(x['method']!='pack_epi_degree'))
   if x['uses_slot']=='1':assert x['slot_hash']==pl[PLANNAME[x['method']]]['slot_hash'] and x['execution_name']=='slot_mode0'
   else:assert x['execution_name']=='pack_epi_degree'
  planned_slots=[x for x in slots if x['uses_slot']=='1'];assert len({x['execution_method_ptr'] for x in planned_slots})==1
  expected_slots={(m,'correctness',0) for m in NAMES}
  if mode!='smoke':expected_slots|={(m,'same_slot_consecutive',0) for m in NAMES}|{(m,'same_slot_alternating',k) for m in NAMES for k in range(10)}|{(m,'pmu',k) for m in NAMES for k in range(3)}
  assert {(x['method'],x['phase'],int(x['repeat'])) for x in slots}==expected_slots
  if mode=='smoke':assert not tables.get('ORDERCTRL_TIME') and not tables.get('ORDERCTRL_PMU_WALL') and not tables.get('METHOD_PMU') and not tables.get('METHOD_PMU_THREAD')
  else:
   tt={}
   for x in tables['ORDERCTRL_TIME']:
    assert x['method'] in NAMES and math.isfinite(float(x['ms'])) and float(x['ms'])>0
    kind=x['kind'];assert kind in ['same_slot_consecutive','same_slot_alternating']
    rep=int(x['repeat']);expected_order=ORDER.index(x['method']) if kind=='same_slot_consecutive' or rep%2==0 else 4-ORDER.index(x['method']);assert int(x['order'])==expected_order
    assert x['warmups_before_series' if kind=='same_slot_consecutive' else 'warmups_before_measure']=='1'
    tt.setdefault((x['method'],kind),[]).append(x)
   assert set(tt)=={(m,k) for m in NAMES for k in ['same_slot_consecutive','same_slot_alternating']}
   for (name,kind),rr in tt.items():
    nr=5 if kind=='same_slot_consecutive' else 10;assert len(rr)==nr and {int(x['repeat']) for x in rr}==set(range(nr));v=[float(x['ms']) for x in rr];summary.append(dict(graph=graph['graph'],method=name,kind=kind,median_ms=statistics.median(v),min_ms=min(v),max_ms=max(v),repeats=nr))
   pmu=tables['ORDERCTRL_PMU_WALL'];assert len(pmu)==15 and {(x['method'],int(x['repeat'])) for x in pmu}=={(m,k) for m in NAMES for k in range(3)}
   totals=tables.get('METHOD_PMU',[]);threads=tables.get('METHOD_PMU_THREAD',[]);pmu_keys={(m,k) for m in NAMES for k in range(3)}
   assert len(totals)==15 and {(x['method'],int(x['repeat'])) for x in totals}==pmu_keys and len(threads)==15*32
   events=['cycles','instructions','cache_misses','cache_references','amx_busy']
   for x in totals:
    key=(x['method'],int(x['repeat']));assert all(x[k]=='1' for k in ['available','coverage_complete','scaled_complete','amx_busy_requested'])
    assert all(x[k]=='32' for k in ['initial_threads','final_threads','counted_threads','scaled_threads']) and all(x[k]=='0' for k in ['failed_threads','zero_running_threads','new_threads','disappeared_threads','errno'])
    assert x['error']=='' and x['event_count']=='5' and x['amx_busy_type']=='raw' and x['amx_busy_config']=='0x02b7'
    tt=[z for z in threads if (z['method'],int(z['repeat']))==key];assert len(tt)==32 and len({z['tid'] for z in tt})==32
    for z in tt:
     assert all(z[k]=='1' for k in ['available','present_at_stop','scaled_valid']) and z['zero_running_time']==z['errno']=='0' and z['error']==''
     assert int(z['time_enabled_ns'])>=int(z['time_running_ns'])>0 and math.isfinite(float(z['scale'])) and float(z['scale'])>=1
     assert math.isfinite(float(z['running_fraction'])) and 0<float(z['running_fraction'])<=1.000000001
     for ev in events:assert int(z[ev+'_raw'])>=0 and math.isfinite(float(z[ev+'_scaled'])) and float(z[ev+'_scaled'])>=0
    for ev in events:
     assert int(x[ev+'_raw'])==sum(int(z[ev+'_raw']) for z in tt) and math.isfinite(float(x[ev+'_scaled']))
     assert math.isclose(float(x[ev+'_scaled']),sum(float(z[ev+'_scaled']) for z in tt),rel_tol=1e-9,abs_tol=1e-3)
    assert all(float(x[ev+'_scaled'])>0 for ev in ['cycles','instructions','amx_busy'])

  state.update(status='PASS',checks=20,equiv=20,profile=10,work=10,slot_checks=len(slots));print('ORDERCTRL_PASS',graph['graph'],flush=True)
 except Exception as exc:state.update(status='FAILED',reason=repr(exc));print('ORDERCTRL_FAILED',graph['graph'],repr(exc),flush=True)
 state['wall_seconds']=time.time()-start;states.append(state);save(folder/'status.json',state);save(run/'suite_status.json',states)
 for label,rows in tables.items():combined.setdefault(label,[]).extend(rows)
 for label,rows in combined.items():csvfile(run/(label.lower()+'.csv'),rows)
 csvfile(run/'summary.csv',summary)
completion=dict(study='pair_order_control',mode=mode,graphs=len(states),passed=sum(x['status']=='PASS' for x in states),failed=sum(x['status']!='PASS' for x in states),checks=sum(x.get('checks',0) for x in states),equiv=sum(x.get('equiv',0) for x in states),profile=sum(x.get('profile',0) for x in states),work=sum(x.get('work',0) for x in states),slot_checks=sum(x.get('slot_checks',0) for x in states))
save(run/'completion.json',completion);print('ORDERCTRL_SUITE_COMPLETE',json.dumps(completion),flush=True);sys.exit(2 if completion['failed'] else 0)
