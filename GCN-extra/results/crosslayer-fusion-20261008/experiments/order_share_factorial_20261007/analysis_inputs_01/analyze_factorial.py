#!/usr/bin/env python3
"""Independent raw-log factorial audit: analyze_factorial.py RUN NEW_OUTPUT_DIR.
No kernels/jobs are executed. Read-only source/hash/CSR/sacct helpers are reused.
"""
import sys,pathlib,importlib.util,collections,shlex,json,math,statistics,re,hashlib
sys.dont_write_bytecode=True
BASE=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
HELPER=BASE/'experiments/pair_plan_20261007/analyze_plan.py'
spec=importlib.util.spec_from_file_location('primary_audit',HELPER);a=importlib.util.module_from_spec(spec);spec.loader.exec_module(a)
ORDERS=('adj','pairs_s1','pairs_s2','pairs_s3');SEEDS=(0,20261007,20261017,20261027)
MODES={'scalar':0,'exact_i':3,'exact_s':4}
NAMES=('pack_epi_degree',)+tuple('slot_'+o+'_'+m for o in ORDERS for m in MODES)
SPEC={'pack_epi_degree':dict(kind=3,mode=0,order_index=-1,seed=0,plan=None)}
for oi,o in enumerate(ORDERS):
 for m,md in MODES.items():SPEC['slot_'+o+'_'+m]=dict(kind=4,mode=md,order_index=oi,seed=SEEDS[oi],plan='adj_degree' if oi==0 else o)
PLANS={'adj_degree','pairs_s1','pairs_s2','pairs_s3'}
KINDS={'same_slot_consecutive':5,'same_slot_alternating':10}
FROZEN=('state_kernels.hpp','pair_reducer.hpp','state_runtime.hpp','grouping_schedule.hpp','pair_plan.hpp','planned_reducer.hpp','planned_kernel.hpp','plan_runtime.hpp','paper_methods_kernels.hpp','paper_methods_runtime.hpp','projection_window_kernels.hpp','projection_window_pmu.hpp','gcn_e2e_v3.cpp')
def parse(path):
 out=collections.defaultdict(list);errors=[]
 for n,line in enumerate(a.read_text(path).splitlines(),1):
  label,_,body=line.partition(' ')
  if not label.startswith(('FACT_','METHOD_','PAIRPLAN_','PLAN_GATE')):continue
  try:
   row={}
   for token in shlex.split(body):
    if '=' in token:
     k,v=token.split('=',1)
     if k in row:raise ValueError('duplicate '+k)
     row[k]=v
   out[label].append(row)
  except Exception as e:errors.append('parse:%d:%r'%(n,e))
 return dict(out),errors
a.parse_records=parse
def build_audit(run):
 m=a.read_json(run/'manifest.json');errs=[];st=a.read_text(run/'source.sha256');bt=a.read_text(run/'build/binary.sha256')
 ss,e=a.hash_list(st,run);errs+=e;bb,e=a.hash_list(bt,run);errs+=e
 def need(v,k):
  if not v:errs.append(k)
 need(bool(st) and m.get('source_hashes')==st,'source_manifest')
 need(set(ss)=={str(p.resolve()) for p in (run/'source_snapshot').rglob('*') if p.is_file()},'source_coverage')
 need(bool(bt) and m.get('binary_hashes')==bt,'binary_manifest')
 need(set(bb)=={str((run/'build'/n).resolve()) for n in ('factorial_methods','paper_original')},'binary_coverage')
 need(m.get('dimensions')==[128,128,128] and m.get('numa_interleave') is False and m.get('node_cap')==10,'build_contract')
 ev=a.read_json(run/'build/event.json');need(ev.get('status')=='PASS' and ev.get('exit_status')==0,'build_exit')
 eq=[]
 for n in FROZEN:
  old=BASE/'runs/pair-plan-20261007-01/source_snapshot'/n;new=run/'source_snapshot'/n
  oh=a.hashed(old);nh=a.hashed(new);eq.append(dict(file=n,primary_sha256=oh,control_sha256=nh,equal=oh==nh));need(oh==nh,'frozen_source:'+n)
 return dict(valid=not errs,errors=errs,manifest=m,sources=ss,binaries=bb,source_text=st,build_fingerprint=hashlib.sha256(st.encode()).hexdigest(),frozen_kernel_equality=eq)

METADATA_CACHE={}
def metadata_work(path,iw):
 key=str(path.resolve())
 if key in METADATA_CACHE:return METADATA_CACHE[key]
 import array
 N=iw['N'];maxdeg=0;previous=None;left=N+1
 with path.open('rb') as f:
  f.seek(36)
  while left:
   raw=f.read(min(left,262144)*4);rr=array.array('I');rr.frombytes(raw)
   if sys.byteorder!='little':rr.byteswap()
   for v in rr:
    if previous is not None:maxdeg=max(maxdeg,v-previous)
    previous=v
   left-=len(rr)
 windows=iw['work']['0']['windows'];tail=N%16;lastwindows=(maxdeg+63)//64
 result=(16*windows-(16-tail)*lastwindows if tail else 16*windows,8*windows-(8-tail//2)*lastwindows if tail else 8*windows)
 METADATA_CACHE[key]=result;return result

def packet_audit(p):
 t=p['parsed'];errs=list(p['parse_errors']);warnings=[];folder=pathlib.Path(p['folder']);smoke=p['mode']=='smoke'
 def need(v,k):
  if not v:errs.append(k)
 def one(label):
  rr=t.get(label,[]);need(len(rr)==1,'cardinality:'+label);return rr[0] if len(rr)==1 else {}
 need(p['job_status']=='PASS','job_context')
 proc=a.read_json(folder/'methods.status.json');cmd=a.read_json(folder/'methods.command.json');stamp=a.read_json(folder/'binary_input_hashes.json')
 need(proc.get('returncode')==0,'process_exit');argv=cmd.get('argv',[])
 need(len(argv)==3 and pathlib.Path(argv[0]).resolve()==pathlib.Path(p['root'])/'build/factorial_methods' and argv[-1]==p['graph'],'process_command')
 need(cmd.get('numa_policy') is None,'process_NUMA_override')
 input_path=pathlib.Path(p['graph_item'].get('path',''));actual=a.hashed(input_path)
 need(actual==p['graph_item'].get('input_sha256')==stamp.get('input_sha256')==p['status'].get('actual_input_sha256'),'input_hash')
 if p.get('expected_input_sha256'):need(actual==p['expected_input_sha256'],'authoritative_inventory_hash')
 for key in ('immutable_source_manifest','immutable_source_status'):
  source=pathlib.Path(p['graph_item'].get(key,''));need(source.is_file() and a.hashed(source)==p['graph_item'].get(key+'_sha256'),'prior_evidence_hash:'+key)
 prior_status=a.read_json(pathlib.Path(p['graph_item']['immutable_source_status']));need(prior_status.get('status')=='PASS' and prior_status.get('actual_input_sha256')==actual,'frozen_prior_input_acceptance')
 need(stamp.get('binary_sha256')==a.hashed(pathlib.Path(p['root'])/'build/factorial_methods'),'process_binary_hash')
 need(p['status'].get('status')=='PASS','suite_status')
 env=a.read_json(folder/'experiment_env.json');need(env.get('FACT_SMOKE')==str(int(smoke)) and env.get('FACT_PMU')==str(int(not smoke)),'experiment_env')
 for rr in t.values():need(all(x.get('graph',p['graph'])==p['graph'] for x in rr),'graph_label')
 cfg=one('FACT_CONFIG');complete=one('FACT_COMPLETE');gate=one('FACT_GATE_COMPLETE');sem=one('PAIRPLAN_SEMANTIC')
 for x in (complete,gate,sem):need(x.get('pass')=='1','terminal_gate')
 need(sem.get('values')=='19','semantic_values')
 need(all(cfg.get(k)=='128' for k in ('D','H','F')) and cfg.get('S')==cfg.get('R')=='64' and cfg.get('TR')=='16' and cfg.get('M')=='FULL','shape')
 need(cfg.get('methods')=='13' and cfg.get('numa')=='default' and cfg.get('final_output')=='BF16' and cfg.get('copy_in_compute')=='0','execution_contract')
 need(cfg.get('warmup_consecutive')==cfg.get('warmup_each_alternating_switch')=='1' and cfg.get('consecutive')=='5' and cfg.get('alternating')=='10' and cfg.get('smoke')==str(int(smoke)),'timing_contract')
 iw=a.degree_work(input_path);N=iw['N'];E=iw['E'];w=iw['work']['0'];need(int(cfg.get('N',-1))==N and int(cfg.get('E',-1))==E,'CSR_shape')
 mm=t.get('FACT_METHOD',[]);need(len(mm)==13 and {x.get('method') for x in mm}==set(NAMES),'method_matrix')
 for x in mm:
  m=SPEC.get(x.get('method'),{});need(a.as_int(x.get('kind'))==m.get('kind') and a.as_int(x.get('mode'))==m.get('mode') and x.get('scope')==x.get('accurate')=='0' and x.get('epilogue')=='1','method_contract');need(a.as_int(x.get('order_index'))==m.get('order_index') and a.as_int(x.get('order_seed'))==m.get('seed'),'method_order_seed')
 plans=t.get('FACT_PLAN',[]);pl={x.get('plan'):x for x in plans};need(len(plans)==len(pl)==4 and set(pl)==PLANS,'plan_matrix')
 need(len({(x.get('tiles'),x.get('mask_bytes')) for x in plans})==1,'plan_same_sizes')
 structs=t.get('PAIRPLAN_STRUCTURE',[]);need(len(structs)==1 and structs[0].get('plan')=='adj_degree','adjacent_structure')
 st=structs[0] if structs else {};c=a.as_int(st.get('exact_shared_edges'));cw=a.as_int(st.get('exact_windows'))
 need(c is not None and 0<=c<=E//2 and cw is not None and cw>=0,'exact_count_bounds')
 pg=t.get('PLAN_GATE',[]);need(len(pg)==1 and pg[0].get('plan')=='adj_degree' and pg[0].get('pass')=='1' and pg[0].get('invalid')=='0','adjacent_builder_gate')
 proofs=t.get('FACT_PAIR_PROOF',[]);need(len(proofs)==3 and {a.as_int(x.get('seed')) for x in proofs}==set(SEEDS[1:]),'three_fixed_seed_proofs')
 for x in proofs:
  need(x.get('invalid')=='0' and x.get('pass')=='1' and all(x.get(k)=='1' for k in ('pair_set_preserved','pair_direction_preserved','odd_row_last','exact_mask_remapped')),'pair_proof')
 for x in proofs:
  need(a.as_int(x.get('actual_exact_shared_edges'))==a.as_int(x.get('expected_exact_shared_edges'))==c and a.as_int(x.get('actual_exact_windows'))==a.as_int(x.get('expected_exact_windows'))==cw and x.get('shared_heads_preserved_by_pair_set')=='1','independent_exact_proof')
  name=x.get('plan');need(name in PLANS and x.get('fnv64')==pl.get(name,{}).get('slot_hash'),'derived_plan_hash')
 for name in PLANS:
  for k in ('shared_heads','exact_shared_edges'):need(pl.get(name,{}).get(k)==pl.get('adj_degree',{}).get(k),'preserved_pair_'+name+'_'+k)
 numeric=t.get('METHOD_CHECK',[]);eq=t.get('PAIRPLAN_EQUIV',[]);exec_eq=t.get('FACT_EXEC_EQUIV',[])
 need(len(numeric)==len(eq)==len(exec_eq)==52,'gate_cardinalities')
 for name in NAMES:
  need(a.numerical_ok([x for x in numeric if x.get('method')==name],False,name),'numeric:'+name)
  rr=[x for x in eq if x.get('method')==name];need(a.freeze_check(rr,a.EQ_BOUNDARIES,name) and all(x.get('scope')==x.get('accurate')=='0' for x in rr),'canonical_bitwise:'+name)
  rr=[x for x in exec_eq if x.get('method')==name];need(a.freeze_check(rr,a.EQ_BOUNDARIES,name),'actual_execution_bitwise:'+name)
  need(all(a.as_int(x.get('mode'))==a.as_int(x.get('execution_mode'))==SPEC[name]['mode'] and x.get('execution_name')==('pack_epi_degree' if name==NAMES[0] else 'slot_exec') for x in rr),'checked_execution_mode:'+name)
 for lab in ('PAIRPLAN_COUNTERS','PAIRPLAN_WORK_GATE','PAIRPLAN_PROFILE_GATE'):
  rr=t.get(lab,[]);need(len(rr)==26 and {x.get('method') for x in rr}==set(NAMES),'counter_matrix:'+lab)
  for name in NAMES:
   zz=[x for x in rr if x.get('method')==name];need(len(zz)==2 and {x.get('layer') for x in zz}=={'1','2'},'two_layers:'+lab+name)
   if lab!='PAIRPLAN_COUNTERS':need(all(x.get('pass')=='1' and (lab!='PAIRPLAN_PROFILE_GATE' or x.get('bitwise')=='1') for x in zz),'profile_work_gate:'+name)
 counters=t.get('PAIRPLAN_COUNTERS',[])
 for x in counters:
  m=SPEC.get(x.get('method'),{});md=m.get('mode');planned=m.get('kind')==4;exact=md in (3,4);share=md==4
  desired=dict(source_visits=E,feature_adds=128*E,decode_rows=E-c if share else E,csr_reads=E-c if share else E,shared_heads=c if exact else 0,comparisons=0,runtime_head_comparisons=0,exact_mask_bytes_read=w['windows'] if exact else 0,exact_pair_windows=cw if exact else 0,exact_pair_edges=c if exact else 0,exact_shared_edges=c if share else 0,projection_scopes=w['scopes'],windows=w['windows'],amx_tdp=32*w['scopes'],partial_zero_bytes=0,packing_load_bytes=0,partial_load_bytes=w['loads'],partial_store_bytes=w['stores'],packed_bytes=w['packed'],direct_rows=w['direct'],prefetch_requests=w['prefetch'],prefetch_index_reads=w['prefetch']//4,relu_elements=128*iw['epilogue_rows'] if x.get('layer')=='1' else 0)
  bad={k:{'expected':v,'actual':a.as_int(x.get(k))} for k,v in desired.items() if a.as_int(x.get(k))!=v};need(not bad,'independent_CSR_work:'+x.get('method','?')+':'+x.get('layer','?')+':'+str(bad))
  need(x.get('profile_path')=='production_BF16_epilogue' and x.get('scope')==x.get('accurate')=='0','profile_context')
  if not planned:need(x.get('order_index_reads')==x.get('exact_pair_flags_tested')=='0','baseline_metadata_work')
  else:
   orders,flags=metadata_work(input_path,iw)
   need(a.as_int(x.get('order_index_reads'))==orders and a.as_int(x.get('exact_pair_flags_tested'))==(flags if exact else 0),'independent_metadata_work')
 for li in ('1','2'):
  for mode in MODES:
   rr=[x for x in counters if x.get('layer')==li and x.get('method') in {'slot_'+o+'_'+mode for o in ORDERS}]
   need(len(rr)==4 and all({k:v for k,v in x.items() if k!='method'}=={k:v for k,v in rr[0].items() if k!='method'} for x in rr),'identical_work_across_orders:'+li+mode)
 slots=t.get('FACT_SLOT',[]);need(len(slots)==(13 if smoke else 195) and int(complete.get('slot_checks',-1))==len(slots),'slot_count')
 need([a.as_int(x.get('serial')) for x in slots]==list(range(len(slots))),'slot_serial')
 ptrkeys=[k for k in slots[0] if k.endswith('_ptr') and k!='execution_method_ptr']+['tiles_size','mask_size','tiles_capacity','mask_capacity'] if slots else []
 for x in slots:
  name=x.get('method');m=SPEC.get(name,{});need(name in NAMES and x.get('stable')==x.get('pass')=='1' and a.as_int(x.get('mode'))==m.get('mode') and x.get('copy_in_compute')=='0','slot_gate')
  need(all(x.get(k)==slots[0].get(k) for k in ptrkeys),'same_addresses_sizes_capacities')
  need(x.get('slot_hash')==x.get('expected_hash') and a.as_float(x.get('copy_ms')) is not None and float(x['copy_ms'])>=0,'slot_content_copy')
  if name!=NAMES[0]:need(x.get('uses_slot')=='1' and x.get('slot_hash')==pl.get(m.get('plan'),{}).get('slot_hash') and x.get('execution_name')=='slot_exec' and a.as_int(x.get('execution_mode'))==m.get('mode') and a.as_int(x.get('order_index'))==m.get('order_index') and a.as_int(x.get('order_seed'))==m.get('seed'),'selected_slot_payload')
 for name in NAMES:
  need({x.get('execution_method_ptr') for x in exec_eq if x.get('method')==name}=={x.get('execution_method_ptr') for x in slots if x.get('method')==name},'checked_timed_object_address:'+name)
 plannedslots=[x for x in slots if x.get('method')!=NAMES[0]];need(len({x.get('execution_method_ptr') for x in plannedslots})==1 and all(x.get('execution_method_ptr') for x in plannedslots),'same_execution_object')
 keys={(m,'correctness',0) for m in NAMES}
 if not smoke:keys|={(m,'same_slot_consecutive',0) for m in NAMES}|{(m,'same_slot_alternating',i) for m in NAMES for i in range(10)}|{(m,'pmu',i) for m in NAMES for i in range(3)}
 need({(x.get('method'),x.get('phase'),a.as_int(x.get('repeat'))) for x in slots}==keys,'switch_matrix')
 times=t.get('FACT_TIME',[]);pmus=t.get('METHOD_PMU',[]);threads=t.get('METHOD_PMU_THREAD',[]);walls=t.get('FACT_PMU_WALL',[]);pmua=[]
 if smoke:need(not(times or pmus or threads or walls),'smoke_no_performance')
 else:
  need(len(times)==195,'time_count')
  for kind,nr in KINDS.items():
   for name in NAMES:
    rr=[x for x in times if x.get('kind')==kind and x.get('method')==name];need(len(rr)==nr and {a.as_int(x.get('repeat')) for x in rr}==set(range(nr)),'repeats:'+kind+name)
    for x in rr:
     rep=int(x['repeat']);order=NAMES.index(name) if kind=='same_slot_consecutive' or rep%2==0 else 12-NAMES.index(name)
     need(a.as_float(x.get('ms')) is not None and float(x['ms'])>0 and a.as_int(x.get('order'))==order and x.get('warmups_before_series' if kind=='same_slot_consecutive' else 'warmups_before_measure')=='1','time_context')
  pk={(m,str(i)) for m in NAMES for i in range(3)}
  for lab,rr in [('PMU',pmus),('PMU_WALL',walls)]:need(len(rr)==39 and {(x.get('method'),x.get('repeat')) for x in rr}==pk,lab+'_matrix')
  for x in pmus:
   rr=[z for z in threads if (z.get('method'),z.get('repeat'))==(x.get('method'),x.get('repeat'))]
   usable=x.get('available')==x.get('coverage_complete')==x.get('scaled_complete')=='1' and all(x.get(k)=='32' for k in ('initial_threads','final_threads','counted_threads','scaled_threads')) and all(x.get(k)=='0' for k in ('failed_threads','zero_running_threads','new_threads','disappeared_threads')) and len(rr)==32 and len({z.get('tid') for z in rr})==32 and all(z.get('available')==z.get('present_at_stop')==z.get('scaled_valid')=='1' and a.as_int(z.get('time_running_ns')) is not None and int(z['time_running_ns'])>0 for z in rr)
   sums_valid=True
   for ev in ('cycles','instructions','cache_misses','cache_references','amx_busy'):
    raw=[a.as_int(z.get(ev+'_raw')) for z in rr];scaled=[a.as_float(z.get(ev+'_scaled')) for z in rr]
    valid=bool(rr) and None not in raw and None not in scaled and all(v>=0 for v in raw+scaled) and a.as_int(x.get(ev+'_raw'))==sum(raw) and a.as_float(x.get(ev+'_scaled')) is not None and math.isclose(float(x[ev+'_scaled']),sum(scaled),rel_tol=1e-9,abs_tol=1e-3);sums_valid=sums_valid and valid
   usable=usable and sums_valid and x.get('event_count')=='5' and x.get('amx_busy_requested')=='1' and x.get('cpu_model')=='143' and all(a.as_float(x.get(ev+'_scaled')) is not None and float(x[ev+'_scaled'])>0 for ev in ('cycles','instructions','amx_busy'))
   pmua.append(dict(method=x.get('method'),repeat=x.get('repeat'),usable=usable,thread_records=len(rr),counter_sums_valid=sums_valid))
   need(usable,'PMU_coverage:'+x.get('method','?')+':'+x.get('repeat','?'))
   if not usable:warnings.append('PMU_unusable:'+x.get('method','?')+':'+x.get('repeat','?'))
 return dict(**a.context(p),valid=not errs,errors=errs,warnings=warnings,raw_records=t,independent_CSR_work=iw,process_status=proc,pmu_audit=pmua,counts=dict(numerical=len(numeric),canonical_bitwise=len(eq),execution_bitwise=len(exec_eq),work=len(t.get('PAIRPLAN_WORK_GATE',[])),profile=len(t.get('PAIRPLAN_PROFILE_GATE',[])),slots=len(slots),timings=len(times),pmu_regions=len(pmus),pmu_threads=len(threads)))

def summaries(audits):
 rows=[];ratios=[];interactions=[];ranges=[];pmu=[];setup=[]
 for x in audits:
  if not x.get('valid') or x['data_class']=='smoke':continue
  ctx={k:x[k] for k in ('job','node','partition','exclusive','graph','data_class','input_sha256','build_fingerprint')}
  tt=x['raw_records']['FACT_TIME'];lookup={}
  for kind,nr in KINDS.items():
   for name in NAMES:
    rr=sorted([z for z in tt if z['kind']==kind and z['method']==name],key=lambda z:int(z['repeat']));v=[float(z['ms']) for z in rr]
    row=dict(**ctx,method=name,protocol=kind,median_ms=statistics.median(v),min_ms=min(v),max_ms=max(v),repeats=nr,raw_ms=json.dumps(v));rows.append(row);lookup[name]=row
   def ratio(ref,cand,effect,order='',mode=''):
    r,c=lookup[ref],lookup[cand];value=r['median_ms']/c['median_ms']
    ratios.append(dict(**ctx,protocol=kind,effect=effect,order=order,seed=SEEDS[ORDERS.index(order)] if order in ORDERS else 0,mode=mode,reference=ref,method=cand,speedup=value,reference_median_ms=r['median_ms'],method_median_ms=c['median_ms'],observed_range_lower=r['min_ms']/c['max_ms'],observed_range_upper=r['max_ms']/c['min_ms'],range_is_confidence_interval=False));return value
   seedeffects=collections.defaultdict(list)
   for order in ORDERS:
    sc='slot_'+order+'_scalar';ei='slot_'+order+'_exact_i';es='slot_'+order+'_exact_s'
    v=ratio(sc,es,'share_vs_scalar',order,'exact_s');seedeffects['share_vs_scalar'].append((order,v))
    v=ratio(ei,es,'share_vs_exact_i',order,'exact_s');seedeffects['share_vs_exact_i'].append((order,v))
    ratio(sc,ei,'exact_i_overhead',order,'exact_i')
    for mode in MODES:
     name='slot_'+order+'_'+mode;ratio(NAMES[0],name,'vs_PACK',order,mode)
     if order!='adj':
      v=ratio('slot_adj_'+mode,name,'order_effect',order,mode);seedeffects['order_effect_'+mode].append((order,v))
    if order!='adj':
     A=lookup['slot_adj_exact_s'];B=lookup[es];C=lookup['slot_adj_scalar'];D=lookup[sc]
     interaction=(A['median_ms']/B['median_ms'])/(C['median_ms']/D['median_ms'])
     ir=dict(**ctx,protocol=kind,order=order,seed=SEEDS[ORDERS.index(order)],interaction_ratio=interaction,order_speedup_exact_s=A['median_ms']/B['median_ms'],order_speedup_scalar=C['median_ms']/D['median_ms'],share_gain_adj=C['median_ms']/A['median_ms'],share_gain_shuffle=D['median_ms']/B['median_ms'],combined_speedup_over_adj_scalar=C['median_ms']/B['median_ms'],observed_range_lower=A['min_ms']*D['min_ms']/(B['max_ms']*C['max_ms']),observed_range_upper=A['max_ms']*D['max_ms']/(B['min_ms']*C['min_ms']),range_is_confidence_interval=False)
     interactions.append(ir);seedeffects['interaction_ratio'].append((order,interaction))
   for effect,vals in seedeffects.items():
    vv=[(o,v) for o,v in vals if o!='adj'];ranges.append(dict(**ctx,protocol=kind,effect=effect,seed_count=len(vv),min_across_prespecified_seeds=min(v for o,v in vv),max_across_prespecified_seeds=max(v for o,v in vv),seed_values=json.dumps({str(SEEDS[ORDERS.index(o)]):v for o,v in vv}),selection_by_timing=False))
  pp=x['raw_records'].get('METHOD_PMU',[])
  for name in NAMES:
   rr=[z for z in pp if z['method']==name];r=dict(**ctx,method=name,repeats=len(rr),all_usable=all(z['usable'] for z in x['pmu_audit'] if z['method']==name))
   for k in ('cycles_scaled','instructions_scaled','cache_misses_scaled','cache_references_scaled','amx_busy_scaled','region_wall_ms'):
    vv=[a.as_float(z.get(k)) for z in rr];r['median_'+k]=statistics.median(vv) if vv and None not in vv else None
   pmu.append(r)
  for z in x['raw_records'].get('PAIRPLAN_SETUP',[]):setup.append(dict(**ctx,plan=z['plan'],kind='adjacent_full_build',total_ms=float(z['total_ms']),plan_bytes=z.get('plan_bytes'),raw=json.dumps(z)))
  for z in x['raw_records'].get('FACT_PAIR_PROOF',[]):setup.append(dict(**ctx,plan=z['plan'],kind='derive_and_independent_audit',total_ms=float(z['derive_ms'])+float(z['audit_ms']),derive_ms=float(z['derive_ms']),audit_ms=float(z['audit_ms']),raw=json.dumps(z)))
 return rows,ratios,interactions,ranges,pmu,setup

def expansion_gate(rows,all_gates):
 cohorts=[];candidate='slot_pairs_s1_exact_s'
 for partition,exclusive in [('intel','false'),('intel_expr','true')]:
  for protocol in KINDS:
   zz=[x for x in rows if x['partition']==partition and x['exclusive']==exclusive and x['protocol']==protocol and x['graph'] in a.DIAG4]
   ratios=[]
   for graph in a.DIAG4:
    cr=[x for x in zz if x['graph']==graph and x['method']==candidate];br=[x for x in zz if x['graph']==graph and x['method']==NAMES[0]]
    if len(cr)==len(br)==1 and cr[0]['job']==br[0]['job']:ratios.append(dict(graph=graph,job=cr[0]['job'],speedup=br[0]['median_ms']/cr[0]['median_ms']))
   gm=a.gm(x['speedup'] for x in ratios) if len(ratios)==4 else None;wins=sum(x['speedup']>1 for x in ratios)
   cohorts.append(dict(partition=partition,exclusive=exclusive,protocol=protocol,graphs=len(ratios),geometric_mean=gm,wins=wins,required_geometric_mean=1.03,required_wins=3,pass_filter=gm is not None and gm>=1.03 and wins>=3,ratios=ratios))
 return dict(candidate=candidate,seed=20261007,all_other_gates_pass=all_gates,cohorts=cohorts,eligible_to_expand_full17=all_gates and all(x['pass_filter'] for x in cohorts),rule='All four protocol x partition cohorts must each achieve GM>=1.03 and wins>=3/4; no replacement by better seed or candidate.',scope='Predeclared practical expansion filter, not a statistical significance guarantee.')

def main():
 if len(sys.argv)!=3:raise SystemExit('usage analyze_factorial.py RUN NEW_OUTPUT_DIR')
 run=pathlib.Path(sys.argv[1]).resolve();out=pathlib.Path(sys.argv[2]).resolve()
 if not str(out).startswith(str(BASE.resolve())+'/results/') or out.exists():raise ValueError('New results directory required')
 build=build_audit(run);packets,jobs,slurm=a.load_packets(run,build);audits=[]
 for p in packets:
  try:audits.append(packet_audit(p))
  except Exception as e:audits.append(dict(**a.context(p),valid=False,errors=['audit_exception:'+repr(e)],raw_records=p['parsed']))
 errs=[]
 if len(jobs)!=7 or not all(j['valid'] for j in jobs):errs.append('expected_seven_completed_valid_jobs')
 if len(audits)!=27 or sum(x['data_class']=='smoke' for x in audits)!=13:errs.append('expected_27_packets_13_smoke')
 perf=[x for x in audits if x['data_class']!='smoke']
 expected={(p,e,g) for p,e in [('intel','false'),('intel_expr','true')] for g in a.SYN3+a.DIAG4}
 if len(perf)!=14 or {(x['partition'],x['exclusive'],x['graph']) for x in perf}!=expected:errs.append('expected_3synthetic_4real_each_allocation_class')
 rows,ratios,interactions,ranges,pmu,setup=summaries(audits)
 totals=collections.Counter()
 for x in audits:totals.update(x.get('counts',{}))
 target={'numerical':1404,'canonical_bitwise':1404,'execution_bitwise':1404,'work':702,'profile':702,'slots':2899,'timings':2730,'pmu_regions':546,'pmu_threads':17472}
 for k,v in target.items():
  if totals[k]!=v:errs.append('count_'+k+'_expected_'+str(v)+'_got_'+str(totals[k]))
 if len(rows)!=364 or len(interactions)!=84:errs.append('expected_364_summary_84_interactions')
 valid=build['valid'] and not errs and all(x['valid'] for x in audits);gate=expansion_gate(rows,valid)
 report=dict(analysis_pass=valid,errors=errs,build=build,jobs=jobs,packets=audits,counts=dict(totals),slurm=slurm,helper=dict(path=str(HELPER),sha256=a.hashed(HELPER)),source_script_sha256=a.digest(pathlib.Path(__file__)),input_hashes=a.INPUT_HASHES,expansion_gate=gate,interpretation_limits=['All ratio denominators are same graph/job/protocol/build.','Interaction ratio equals order benefit under ExactS divided by order benefit under Scalar; raw sample extrema are descriptive ranges, not confidence bounds.','Seeds s1/s2/s3 are prespecified and all retained; extrema across seeds are a range, not a chosen method.','All scalar orders have fixed pair membership; ExactI/S use remapped identical exact-mask coverage.','Generic cache events do not establish DRAM traffic or a unique cause; AMX busy does not establish AVX-AMX overlap.','Same-storage factorial uses warmed calls and excludes separately reported copy/setup costs.','Only diagnostic4 real graphs in this stage; no primary17 claim or best-seed substitute.'])
 out.mkdir(parents=True)
 (out/'audit.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
 (out/'expansion_gate.json').write_text(json.dumps(gate,ensure_ascii=False,indent=2)+'\n')
 for n,rr,fields in [('summary',rows,['job','node','partition','exclusive','graph','method','protocol','median_ms']),('ratios',ratios,['job','graph','protocol','effect','order','mode','reference','method','speedup']),('interactions',interactions,['job','graph','protocol','order','seed','interaction_ratio']),('seed_ranges',ranges,['job','graph','protocol','effect']),('pmu_summary',pmu,['job','graph','method','all_usable']),('setup',setup,['job','graph','plan','kind','total_ms'])]:a.write_csv(out/(n+'.csv'),rr,fields)
 print(json.dumps(dict(analysis_pass=valid,errors=errs,invalid_packets=[dict(graph=x['graph'],job=x['job'],errors=x['errors']) for x in audits if not x['valid']],counts=dict(totals),summary_rows=len(rows),pmu_usable=sum(y['usable'] for x in audits for y in x.get('pmu_audit',[])),expand_full17=gate['eligible_to_expand_full17'],out=str(out)),ensure_ascii=False))
 return 0 if valid else 2
if __name__=='__main__':sys.exit(main())
