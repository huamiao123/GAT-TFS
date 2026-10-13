#!/usr/bin/env python3
"""Frozen eight-method event-trace mechanism study; all gates precede timing.
No new graph generation, oracle selection, prefetch treatment or node policy.
"""
import pathlib,sys,json,shlex,statistics,math,time,os,resource,traceback,struct
from evidence_utils import ROOT,save,sha,invoke,csvfile
resource.setrlimit(resource.RLIMIT_CORE,(0,0))
run,build=map(pathlib.Path,sys.argv[1:3]);mode=sys.argv[3]
PRIOR=ROOT/'runs/pair-plan-20261007-01/logs'
FOCUS={'mycielskian19','reddit','roadNet-CA','amazon0601'}
ORDER=['pack_epi_degree','pack_epi_qguard','dyn_i','dyn_s','exact_s','trace_i','trace_s_cached','trace_s_rep']
NAMES=set(ORDER);KINDS=dict(zip(ORDER,[3,3,4,4,4,5,5,5]));MODES=dict(zip(ORDER,[0,0,1,2,4,0,1,2]))
TRACE_NAMES={'trace_i','trace_s_cached','trace_s_rep'}
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
os.environ['TRACE_SMOKE']='1' if mode=='smoke' else '0';os.environ['TRACE_PMU']='0' if mode=='smoke' else '1'
save(run/'graph_list.json',graphs);states=[];combined={};summary=[]

def finite_number(value,positive=False):
 x=float(value);assert math.isfinite(x) and (x>0 if positive else x>=0),value;return x


def records_for(tables,label,name):return [r for r in tables.get(label,[]) if r.get('method')==name]


def audit_pmu(tables):
 expected={(name,k) for name in NAMES for k in range(3)}
 walls=tables.get('TRACE_PMU_WALL',[]);totals=tables.get('METHOD_PMU',[]);threads=tables.get('METHOD_PMU_THREAD',[])
 assert len(walls)==len(totals)==24 and len(threads)==24*32,'PMU matrix count must be 8 x 3 x 32'
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


def one(tables,label):
 rr=tables.get(label,[]);assert len(rr)==1,(label,len(rr));return rr[0]

def audit_smoke_trace(path,tables,plan):
 dump=one(tables,'TRACE_SMOKE_DUMP');data=bytes.fromhex(dump['data_hex']) if dump['data_hex']!='none' else b''
 perm=list(map(int,dump['degree_perm'].split(','))) if dump['degree_perm']!='none' else []
 offsets=list(map(int,dump['tile_offsets'].split(',')))
 raw=path.read_bytes();n,nc,e=struct.unpack_from('<QQQ',raw,12);assert n==nc and len(perm)==n and sorted(perm)==list(range(n))
 rows=struct.unpack_from('<'+str(n+1)+'I',raw,36);cols=struct.unpack_from('<'+str(e)+'I',raw,36+4*(n+1))
 assert rows[0]==0 and rows[-1]==e and all(a<=b for a,b in zip(rows,rows[1:]))
 assert len(offsets)==(n+15)//16+1 and offsets[0]==0 and offsets[-1]==len(data)
 hist=[0]*129;events=shared=headers=payload=0
 for ti,first in enumerate(range(0,n,16)):
  cursor,end=offsets[ti:ti+2];assert 0<=cursor<=end<=len(data)
  rr=perm[first:first+16];mx=max((rows[r+1]-rows[r] for r in rr),default=0)
  for ws in range(0,mx,64):
   for pos in range(0,len(rr),2):
    r0=rr[pos];r1=rr[pos+1] if pos+1<len(rr) else None
    left=cols[rows[r0]+min(ws,rows[r0+1]-rows[r0]):rows[r0]+min(ws+64,rows[r0+1]-rows[r0])]
    right=() if r1 is None else cols[rows[r1]+min(ws,rows[r1+1]-rows[r1]):rows[r1]+min(ws+64,rows[r1+1]-rows[r1])]
    i=j=0;wanted=[]
    while i<len(left) or j<len(right):
     if i==len(left):take=2
     elif j==len(right):take=1
     else:take=3 if left[i]==right[j] else (1 if left[i]<right[j] else 2)
     wanted.append(take);i+=bool(take&1);j+=bool(take&2)
    assert cursor<end;count=data[cursor];cursor+=1;assert count==len(wanted)<=128
    hist[count]+=1;headers+=1;nb=(count+3)//4;assert cursor+nb<=end
    actual=[(data[cursor+k//4]>>(2*(k%4)))&3 for k in range(count)];assert actual==wanted
    if count%4:assert data[cursor+nb-1]>>(2*(count%4))==0
    cursor+=nb;payload+=nb;events+=count;shared+=wanted.count(3)
  assert cursor==end
 assert len(data)==int(plan['data_bytes']) and events==int(plan['events']) and shared==int(plan['shared_heads']) and headers==int(plan['headers']) and payload==int(plan['payload_bytes'])
 for x in tables['TRACE_PLAN_HIST']:assert hist[int(x['event_count'])]==int(x['builder_records'])==int(x['validator_records'])

for graph in graphs:
 folder=run/graph['graph'];folder.mkdir();start=time.time();state=dict(graph);tables={};error=None;graph_summary=[]
 try:
  path=pathlib.Path(graph['path']);state['actual_input_sha256']=sha(path)
  assert state['actual_input_sha256']==graph['input_sha256'],'Frozen CSR changed'
  state['binary_sha256']=sha(build/'trace_methods');state['experiment_env']={k:os.environ[k] for k in ('TRACE_SMOKE','TRACE_PMU')}
  save(folder/'binary_input_hashes.json',dict(binary=str(build/'trace_methods'),binary_sha256=state['binary_sha256'],input=str(path),input_sha256=state['actual_input_sha256']))
  save(folder/'experiment_env.json',state['experiment_env']);print('TRACE_START',graph['graph'],flush=True)
  invoke(folder,'methods',[build/'trace_methods',path.parent.parent,graph['graph']])
 except Exception as exc:error=exc
 text=(folder/'methods.log').read_text(errors='replace') if (folder/'methods.log').exists() else '';parse_errors=[]
 for line_no,line in enumerate(text.splitlines(),1):
  label,_,content=line.partition(' ')
  if label.startswith(('METHOD_','PAIRPLAN_','TRACE_','GROUP_')) or label=='PLAN_GATE':
   try:
    row={}
    for token in shlex.split(content):
     if '=' not in token:continue
     k,v=token.split('=',1);assert k not in row,'Duplicate field '+k;row[k]=v
    tables.setdefault(label,[]).append(row)
   except Exception as exc:parse_errors.append(dict(line=line_no,error=repr(exc)))
 save(folder/'parsed.json',tables)
 if parse_errors:save(folder/'parse_errors.json',parse_errors)
 for key,rr in tables.items():csvfile(folder/(key.lower()+'.csv'),rr)
 try:
  if error:raise error
  assert not parse_errors,parse_errors
  assert all(x.get('graph',graph['graph'])==graph['graph'] for rr in tables.values() for x in rr),'Cross-graph record'
  assert all(x.get('pass','1')!='0' for rr in tables.values() for x in rr),'A runtime gate failed'
  for label in ('TRACE_COMPLETE','TRACE_GATE_COMPLETE','PAIRPLAN_SEMANTIC','PLAN_GATE','TRACE_PLAN_GATE'):assert one(tables,label)['pass']=='1'
  sem=one(tables,'PAIRPLAN_SEMANTIC');assert sem['values']=='19' and sem['bitwise']=='1'
  opportunity=one(tables,'TRACE_OPPORTUNITY');assert opportunity['pass']==opportunity['diagnostic_only']=='1';finite_number(opportunity['diagnostic_ms'])
  cfg=one(tables,'TRACE_CONFIG');done=one(tables,'TRACE_COMPLETE')
  assert all(cfg[k]=='128' for k in ('D','H','F')) and cfg['S']==cfg['R']=='64' and cfg['TR']=='16' and cfg['M']=='FULL'
  assert cfg['methods']==done['methods']=='8' and cfg['pf']=='1' and cfg['numa']=='default' and cfg['final_output']=='BF16' and cfg['copy_in_compute']=='0'
  assert cfg['warmup_consecutive']==cfg['warmup_each_alternating_switch']=='1' and cfg['consecutive']=='5' and cfg['alternating']=='10'
  assert cfg['smoke']==done['smoke']==str(int(mode=='smoke')) and cfg['fixed_expansion_candidate']=='trace_s_rep' and cfg['trace_resident_for_all_methods']=='1'
  assert int(cfg['N'])==int(graph['N']) and int(cfg['E'])==int(graph['E']) and int(done['max_rss_kib'])>0
  methods=tables['TRACE_METHOD'];assert len(methods)==8 and [x['method'] for x in methods]==ORDER
  for x in methods:
   name=x['method'];assert int(x['kind'])==KINDS[name] and int(x['mode'])==MODES[name]
   assert x['scope']==x['accurate']=='0' and x['epilogue']==x['pf']=='1'
   assert x['schedule']==('qguard' if name=='pack_epi_qguard' else 'degree') and int(x['schedule_index'])==(4 if name=='pack_epi_qguard' else 0)
   assert int(x['plan_index'])==(-1 if KINDS[name]==3 else 0)
  grouping=tables['GROUP_AUDIT'];assert len(grouping)==5 and {x['schedule'] for x in grouping}=={'degree','qsource','qpage','qsamplepage','qguard'}
  for x in grouping:
   assert x['pass']=='1' and int(x['rows'])==int(cfg['N'])
   assert all(x[k]==grouping[0][k] for k in ('full_projections','b64_projections','b256_projections','windows','state_load_bytes','state_store_bytes'))
  adj=one(tables,'PAIRPLAN_STRUCTURE');assert adj['plan']=='adj_degree' and adj['kind']=='0' and one(tables,'PLAN_GATE')['plan']=='adj_degree'
  e=int(cfg['E']);dynamic=int(adj['dynamic_shared_heads']);exact=int(adj['exact_shared_edges']);comparisons=int(adj['comparisons']);exact_windows=int(adj['exact_windows'])
  assert int(adj['source_edges'])==e and 0<=exact<=dynamic<=e//2 and int(adj['merge_steps'])+dynamic==e
  if graph['graph'] in ('plan_exact_tail','plan_exact_unsorteddup'):assert exact>0
  if graph['graph']=='plan_exact_mismatch':assert exact==0
  plan=one(tables,'TRACE_PLAN');setup=one(tables,'TRACE_PLAN_SETUP');gate=one(tables,'TRACE_PLAN_GATE')
  assert all(gate[k]=='1' for k in ('independent_scalar_validator','original_greedy')) and gate['source_ids_stored']=='0'
  assert all(plan[k]=='1' for k in ('order_preserved','exact_exhaustion','padding_zero','header_hist_checked','pass'))
  assert setup['format']=='count_u8_plus_lsb2' and setup['source_ids_stored']=='0' and setup['inspector_includes_full_validator']=='1'
  assert setup['bytes_semantics']=='persistent_vector_payload'
  for key in ('build_ms','validate_ms','total_ms'):finite_number(setup[key])
  assert float(setup['total_ms'])+1e-6>=float(setup['build_ms'])+float(setup['validate_ms'])
  finite_number(plan['independent_audit_ms']);finite_number(one(tables,'TRACE_SCHEDULE_SETUP')['ms'])
  assert int(plan['source_edges'])==e and int(plan['events'])==e-dynamic and int(plan['shared_heads'])==dynamic and int(plan['comparisons'])==comparisons
  assert int(plan['take3'])==dynamic and int(plan['take1'])+int(plan['take2'])+2*dynamic==e
  assert int(plan['tiles'])==int(adj['tiles']) and int(plan['nonempty_tiles'])==int(adj['full_projections']) and int(plan['windows'])==int(adj['neighbor_windows'])
  hist=tables['TRACE_PLAN_HIST'];assert len(hist)==129 and {int(x['event_count']) for x in hist}==set(range(129))
  assert all(x['builder_records']==x['validator_records'] and x['pass']=='1' for x in hist)
  assert sum(int(x['builder_records']) for x in hist)==int(plan['headers'])
  assert sum(int(x['builder_records'])*int(x['event_count']) for x in hist)==int(plan['events'])
  assert sum(int(x['builder_records'])*((int(x['event_count'])+3)//4) for x in hist)==int(plan['payload_bytes'])
  assert int(plan['data_bytes'])==int(plan['data_size'])==int(plan['headers'])+int(plan['payload_bytes'])
  assert int(plan['offsets_size'])==int(plan['tiles'])+1 and plan['offset_element_bytes']=='8'
  assert int(plan['offset_bytes'])==8*int(plan['offsets_size']) and int(plan['plan_bytes'])==int(plan['data_bytes'])+int(plan['offset_bytes'])
  assert int(plan['allocated_bytes'])==int(plan['data_capacity'])+8*int(plan['offsets_capacity'])
  assert int(plan['data_capacity'])>=int(plan['data_size']) and int(plan['offsets_capacity'])>=int(plan['offsets_size'])
  for key in ('source_edges','shared_heads','comparisons','nonempty_tiles','windows','headers','payload_bytes','data_bytes','offset_bytes','plan_bytes','allocated_bytes','take1','take2','take3','object_bytes'):assert setup[key]==plan[key],key
  assert setup['merge_steps']==plan['events'] and setup['fnv64']==plan['plan_fnv64']
  if mode=='smoke':
   audit_smoke_trace(path,tables,plan)
   rr=tables['TRACE_REJECT_CASE'];assert len(rr)==2 and {x['case'] for x in rr}=={'used_code_zero','tile_offset_oob'}
   assert all(x['valid_fixture']==x['rejected']==x['reason_present']==x['pass']=='1' for x in rr)
   rej=one(tables,'TRACE_REJECT_AUDIT');assert rej['cases']==rej['rejected']=='2' and rej['valid_fixture']==rej['pass']=='1'
  else:assert not any(tables.get(k) for k in ('TRACE_SMOKE_DUMP','TRACE_REJECT_CASE','TRACE_REJECT_AUDIT'))
  for label in ('METHOD_CHECK','PAIRPLAN_EQUIV','TRACE_EXEC_EQUIV'):
   rr=tables[label];assert len(rr)==32 and {x['method'] for x in rr}==NAMES
   for name in ORDER:
    xx=records_for(tables,label,name);assert len(xx)==4
    if label=='METHOD_CHECK':
     assert {(x['boundary'],x['reference']) for x in xx}==CHECKS
     for x in xx:
      limit=.03 if x['reference']=='source_MKL_FP32' else (.02 if x['boundary']=='e2e_BF16_final' else .01)
      assert float(x['gate'])==limit and x['pass']=='1' and finite_number(x['relative_L2'])<limit and finite_number(x['normalized_max'])<limit
      finite_number(x['max_abs']);finite_number(x['mean_abs'])
    else:
     assert {x['boundary'] for x in xx}==BOUNDARIES
     assert all(x['bitwise']==x['pass']=='1' and x['scope']==x['accurate']=='0' for x in xx)
     for x in xx:
      assert int(x['kind'])==int(x['execution_kind'])==KINDS[name] and int(x['mode'])==int(x['execution_mode'])==MODES[name]
      assert x['execution_name']==('trace_exec' if label=='TRACE_EXEC_EQUIV' and KINDS[name]>=4 else name)
      assert x['reference']==(name if label=='TRACE_EXEC_EQUIV' else 'canonical_full')
  rr=tables['TRACE_DYNS_EQUIV'];assert len(rr)==8 and {x['method'] for x in rr}=={'trace_s_cached','trace_s_rep'}
  for name in ('trace_s_cached','trace_s_rep'):
   xx=records_for(tables,'TRACE_DYNS_EQUIV',name);assert len(xx)==4 and {x['boundary'] for x in xx}==BOUNDARIES
   assert all(x['bitwise']==x['pass']=='1' and x['reference']=='dyn_s_reference' for x in xx)
  for label in ('PAIRPLAN_COUNTERS','PAIRPLAN_WORK_GATE','PAIRPLAN_PROFILE_GATE','TRACE_LOGICAL'):
   rr=tables[label];assert len(rr)==16 and {x['method'] for x in rr}==NAMES
   for name in ORDER:
    xx=records_for(tables,label,name);assert len(xx)==2 and {x['layer'] for x in xx}=={'1','2'}
    if label!='PAIRPLAN_COUNTERS':assert all(x['pass']=='1' for x in xx)
    if label=='PAIRPLAN_PROFILE_GATE':assert all(x['bitwise']=='1' for x in xx)
  counters=tables['PAIRPLAN_COUNTERS']
  common=('source_visits','feature_adds','projection_scopes','windows','amx_tdp','partial_zero_bytes','partial_load_bytes','partial_store_bytes','packing_load_bytes','direct_rows','packed_bytes','prefetch_requests','prefetch_index_reads','relu_elements')
  for layer in ('1','2'):
   rr=[x for x in counters if x['layer']==layer];baseline=next(x for x in rr if x['method']=='pack_epi_degree')
   assert all(all(x[k]==baseline[k] for k in common) for x in rr),'Frozen q64-controlled work differs'
  for x in counters:
   name=x['method'];kind=KINDS[name];md=MODES[name];hits=0 if kind==3 else (exact if name=='exact_s' else dynamic)
   saved=hits if name in ('dyn_s','exact_s','trace_s_cached','trace_s_rep') else 0
   saved_csr=hits if name in ('exact_s','trace_s_rep') else 0
   expected_comparisons=comparisons if name in ('dyn_i','dyn_s') else 0
   assert int(x['source_visits'])==e and int(x['feature_adds'])==128*e and int(x['shared_heads'])==hits
   assert int(x['decode_rows'])==e-saved and int(x['csr_reads'])==e-saved_csr
   assert int(x['comparisons'])==int(x['runtime_head_comparisons'])==expected_comparisons
   assert x['scope']==x['accurate']=='0' and x['profile_path']=='production_BF16_epilogue' and x['unit']=='source_level_not_DRAM'
   assert int(x['amx_tdp'])==32*int(x['projection_scopes']) and x['partial_zero_bytes']==x['packing_load_bytes']=='0'
   assert int(x['prefetch_requests'])==4*int(x['prefetch_index_reads'])
   if name!='exact_s':assert all(x[k]=='0' for k in ('exact_mask_bytes_read','exact_pair_flags_tested','exact_pair_windows','exact_pair_edges','exact_shared_edges'))
   else:assert int(x['exact_pair_windows'])==exact_windows and int(x['exact_pair_edges'])==int(x['exact_shared_edges'])==exact
   if kind==3:assert x['order_index_reads']=='0'
   for left,right in (('partial_load_bytes','expected_loads'),('partial_store_bytes','expected_stores'),('packed_bytes','expected_packed'),('direct_rows','expected_direct'),('prefetch_requests','expected_prefetch'),('decode_rows','expected_decode'),('csr_reads','expected_csr'),('projection_scopes','expected_scopes'),('windows','expected_windows'),('shared_heads','expected_hits')):assert x[left]==x[right],(name,left,right)
  for x in tables['TRACE_LOGICAL']:
   is_trace=x['method'] in TRACE_NAMES
   for key,pkey in [('headers','headers'),('payload_bytes','payload_bytes'),('events','events'),('take1','take1'),('take2','take2'),('take3','take3'),('tile_offset_reads','nonempty_tiles')]:assert int(x[key])==int(x['expected_'+key])==(int(plan[pkey]) if is_trace else 0)
   assert x['unit']=='source_level_not_retired'
  immutable=tables['TRACE_IMMUTABLE'];assert len(immutable)==2 and {x['phase'] for x in immutable}=={'before','after'}
  for x in immutable:assert x['hash_checked']==x['pass']=='1' and x['base_hash']==x['expected_base_hash']==adj['fnv64'] and x['trace_hash']==x['expected_trace_hash']==plan['payload_hash']
  slots=tables['TRACE_SLOT'];count=8 if mode=='smoke' else 120;assert len(slots)==int(done['slot_checks'])==count
  assert [int(x['serial']) for x in slots]==list(range(count))
  fixed=[k for k in slots[0] if k.endswith('_ptr') and k not in ('execution_method_ptr','schedule_ptr')]+[k for k in slots[0] if k.endswith(('_size','_capacity'))]
  assert all(k in slots[0] for k in ('base_plan_ptr','tiles_ptr','masks_ptr','trace_plan_ptr','offsets_ptr','data_ptr','execution_method_ptr','schedule_ptr','h0b_ptr','wv1_ptr','h1b_ptr','h2b_ptr','row_ptr','col_ptr'))
  for x in slots:
   name=x['method'];assert name in NAMES and x['stable']==x['pass']=='1' and x['hash_checked']==x['copy_in_compute']=='0'
   assert int(x['kind'])==int(x['execution_kind'])==KINDS[name] and int(x['mode'])==int(x['execution_mode'])==MODES[name]
   assert x['execution_name']==('trace_exec' if KINDS[name]>=4 else name)
   assert x['schedule']==('qguard' if name=='pack_epi_qguard' else 'degree')
   assert all(x[k]==slots[0][k] for k in fixed),'Runtime allocation changed'
  assert len({x['execution_method_ptr'] for x in slots if KINDS[x['method']]>=4})==1
  for name in ORDER:
   xx=[x for x in slots if x['method']==name];assert len({x['execution_method_ptr'] for x in xx})==1 and len({x['schedule_ptr'] for x in xx})==1
   assert {x['execution_method_ptr'] for x in xx}=={x['execution_method_ptr'] for x in records_for(tables,'TRACE_EXEC_EQUIV',name)},'Timed object differs from checked object'
  assert len({x['schedule_ptr'] for x in slots if x['method']!='pack_epi_qguard'})==1
  expected={(name,'correctness',0) for name in ORDER}
  if mode!='smoke':expected|={(name,'consecutive',0) for name in ORDER}|{(name,'alternating',r) for name in ORDER for r in range(10)}|{(name,'pmu',r) for name in ORDER for r in range(3)}
  assert {(x['method'],x['phase'],int(x['repeat'])) for x in slots}==expected
  if mode=='smoke':assert not any(tables.get(k) for k in ('TRACE_TIME','TRACE_PMU_WALL','METHOD_PMU','METHOD_PMU_THREAD'))
  else:
   times={}
   for x in tables['TRACE_TIME']:
    name=x['method'];assert name in NAMES;finite_number(x['ms'],True);kind=x['kind'];assert kind in ('consecutive','alternating');rep=int(x['repeat'])
    order=ORDER.index(name) if kind=='consecutive' or rep%2==0 else 7-ORDER.index(name)
    assert int(x['order'])==order and x['warmups_before_series' if kind=='consecutive' else 'warmups_before_measure']=='1'
    times.setdefault((name,kind),[]).append(x)
   assert set(times)=={(name,kind) for name in ORDER for kind in ('consecutive','alternating')}
   for (name,kind),rr in times.items():
    nr=5 if kind=='consecutive' else 10;assert len(rr)==nr and {int(x['repeat']) for x in rr}==set(range(nr));vals=[float(x['ms']) for x in rr]
    graph_summary.append(dict(graph=graph['graph'],method=name,kind=kind,median_ms=statistics.median(vals),min_ms=min(vals),max_ms=max(vals),repeats=nr))
   audit_pmu(tables)
  state.update(status='PASS',checks=32,equiv=32,exec_equiv=32,dyns_equiv=8,profile=16,work=16,trace_logical=16,trace_hist=129,trace_reject_cases=2 if mode=='smoke' else 0,slot_checks=len(slots),pmu_regions=0 if mode=='smoke' else 24)
  summary.extend(graph_summary);print('TRACE_PASS',graph['graph'],flush=True)
 except Exception as exc:
  state.update(status='FAILED',reason=repr(exc),traceback=traceback.format_exc());print('TRACE_FAILED',graph['graph'],repr(exc),flush=True)
 state['wall_seconds']=time.time()-start;states.append(state);save(folder/'status.json',state);save(run/'suite_status.json',states)
 for label,rr in tables.items():combined.setdefault(label,[]).extend(rr)
 for label,rr in combined.items():csvfile(run/(label.lower()+'.csv'),rr)
 csvfile(run/'summary.csv',summary)
completion=dict(study='event_trace',mode=mode,graphs=len(states),passed=sum(x['status']=='PASS' for x in states),failed=sum(x['status']!='PASS' for x in states))
for key in ('checks','equiv','exec_equiv','dyns_equiv','profile','work','trace_logical','trace_hist','trace_reject_cases','slot_checks','pmu_regions'):completion[key]=sum(x.get(key,0) for x in states)
save(run/'completion.json',completion);print('TRACE_SUITE_COMPLETE',json.dumps(completion),flush=True);sys.exit(2 if completion['failed'] else 0)
