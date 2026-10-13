#!/usr/bin/env python3
"""Independent raw-log same-storage order audit. Usage RUN NEW_OUTPUT_DIR.
Reuses previously audited read-only hashing, CSR degree formulas, and scheduler
inventory helpers, but independently parses and validates this control matrix.
No executable kernels or benchmarks are run by this program.
"""
import sys,pathlib,importlib.util,collections,shlex,json,math,statistics,re,hashlib
sys.dont_write_bytecode=True
BASE=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
HELPER=BASE/'experiments/pair_plan_20261007/analyze_plan.py'
spec=importlib.util.spec_from_file_location('primary_audit',HELPER);a=importlib.util.module_from_spec(spec);spec.loader.exec_module(a)
NAMES=('pack_epi_degree','slot_adj_scalar','slot_opt_scalar','slot_rand_scalar','slot_pairs_scalar')
PLAN={'slot_adj_scalar':'adj_degree','slot_opt_scalar':'opt_degree','slot_rand_scalar':'rand_degree','slot_pairs_scalar':'pairs_degree'}
KINDS={'same_slot_consecutive':5,'same_slot_alternating':10}
FROZEN=('state_kernels.hpp','pair_reducer.hpp','state_runtime.hpp','grouping_schedule.hpp','pair_plan.hpp','planned_reducer.hpp','planned_kernel.hpp','plan_runtime.hpp','paper_methods_kernels.hpp','paper_methods_runtime.hpp','projection_window_kernels.hpp','projection_window_pmu.hpp','gcn_e2e_v3.cpp')
def parse(path):
 out=collections.defaultdict(list);errors=[]
 for n,line in enumerate(a.read_text(path).splitlines(),1):
  label,_,body=line.partition(' ')
  if not label.startswith(('ORDERCTRL_','METHOD_','PAIRPLAN_','PLAN_GATE')):continue
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
 need(set(bb)=={str((run/'build'/n).resolve()) for n in ('order_control_methods','paper_original')},'binary_coverage')
 need(m.get('dimensions')==[128,128,128] and m.get('numa_interleave') is False and m.get('node_cap')==10,'build_contract')
 ev=a.read_json(run/'build/event.json');need(ev.get('status')=='PASS' and ev.get('exit_status')==0,'build_exit')
 eq=[]
 for n in FROZEN:
  old=BASE/'runs/pair-plan-20261007-01/source_snapshot'/n;new=run/'source_snapshot'/n
  oh=a.hashed(old);nh=a.hashed(new);eq.append(dict(file=n,primary_sha256=oh,control_sha256=nh,equal=oh==nh));need(oh==nh,'frozen_source:'+n)
 return dict(valid=not errs,errors=errs,manifest=m,sources=ss,binaries=bb,source_text=st,build_fingerprint=hashlib.sha256(st.encode()).hexdigest(),frozen_kernel_equality=eq)

def packet_audit(p):
 t=p['parsed'];errs=list(p['parse_errors']);warnings=[];folder=pathlib.Path(p['folder']);smoke=p['mode']=='smoke'
 def need(v,k):
  if not v:errs.append(k)
 def one(label):
  rr=t.get(label,[]);need(len(rr)==1,'cardinality:'+label);return rr[0] if len(rr)==1 else {}
 need(p['job_status']=='PASS','job_context')
 proc=a.read_json(folder/'methods.status.json');cmd=a.read_json(folder/'methods.command.json');stamp=a.read_json(folder/'binary_input_hashes.json')
 need(proc.get('returncode')==0,'process_exit')
 argv=cmd.get('argv',[]);need(len(argv)==3 and pathlib.Path(argv[0]).resolve()==pathlib.Path(p['root'])/'build/order_control_methods' and argv[-1]==p['graph'],'process_command')
 need(cmd.get('numa_policy') is None,'process_NUMA_override')
 input_path=pathlib.Path(p['graph_item'].get('path',''));actual=a.hashed(input_path)
 need(actual==p['graph_item'].get('input_sha256')==stamp.get('input_sha256')==p['status'].get('actual_input_sha256'),'input_hash')
 need(stamp.get('binary_sha256')==a.hashed(pathlib.Path(p['root'])/'build/order_control_methods'),'process_binary_hash')
 need(p['status'].get('status')=='PASS','suite_status')
 env=a.read_json(folder/'experiment_env.json');need(env.get('ORDERCTRL_SMOKE')==str(int(smoke)) and env.get('ORDERCTRL_PMU')==str(int(not smoke)),'experiment_env')
 for rr in t.values():need(all(x.get('graph',p['graph'])==p['graph'] for x in rr),'graph_label')
 cfg=one('ORDERCTRL_CONFIG');complete=one('ORDERCTRL_COMPLETE');gate=one('ORDERCTRL_GATE_COMPLETE');sem=one('PAIRPLAN_SEMANTIC');proof=one('ORDERCTRL_PAIR_PROOF')
 for x in (complete,gate,sem,proof):need(x.get('pass')=='1','terminal_gate')
 need(sem.get('values')=='19','semantic_values')
 need(all(cfg.get(k)=='128' for k in ('D','H','F')) and cfg.get('S')==cfg.get('R')=='64' and cfg.get('TR')=='16' and cfg.get('M')=='FULL','shape')
 need(cfg.get('methods')=='5' and cfg.get('planned_mode')=='0' and cfg.get('numa')=='default' and cfg.get('final_output')=='BF16' and cfg.get('copy_in_compute')=='0','execution_contract')
 need(cfg.get('warmup_consecutive')==cfg.get('warmup_each_alternating_switch')=='1' and cfg.get('consecutive')=='5' and cfg.get('alternating')=='10' and cfg.get('smoke')==str(int(smoke)),'timing_contract')
 iw=a.degree_work(input_path);N=iw['N'];E=iw['E'];w=iw['work']['0'];need(int(cfg.get('N',-1))==N and int(cfg.get('E',-1))==E,'CSR_shape')
 methods=t.get('ORDERCTRL_METHOD',[]);need(len(methods)==5 and {x.get('method') for x in methods}==set(NAMES),'method_matrix')
 for x in methods:need(x.get('kind')==('3' if x.get('method')==NAMES[0] else '4') and x.get('scope')==x.get('mode')==x.get('accurate')=='0' and x.get('epilogue')=='1','method_contract')
 plans=t.get('ORDERCTRL_PLAN',[]);pl={x.get('plan'):x for x in plans};need(len(plans)==len(pl)==4 and set(pl)==set(PLAN.values()),'plan_matrix')
 need(len({(x.get('tiles'),x.get('mask_bytes')) for x in plans})==1,'plan_same_sizes')
 pg=t.get('PLAN_GATE',[]);need(len(pg)==3 and {x.get('plan') for x in pg}=={'adj_degree','opt_degree','rand_degree'} and all(x.get('pass')=='1' and x.get('invalid')=='0' for x in pg),'builder_gates')
 need(proof.get('seed')=='20261007' and proof.get('invalid')=='0' and all(proof.get(k)=='1' for k in ('pair_set_preserved','pair_direction_preserved','odd_row_last','exact_mask_remapped')),'pair_proof')
 need(proof.get('fnv64')==pl.get('pairs_degree',{}).get('slot_hash'),'pair_hash')
 for k in ('shared_heads','exact_shared_edges'):need(pl.get('pairs_degree',{}).get(k)==pl.get('adj_degree',{}).get(k),'preserved_pair_'+k)
 numeric=t.get('METHOD_CHECK',[]);eq=t.get('PAIRPLAN_EQUIV',[])
 need(len(numeric)==len(eq)==20,'gate_cardinalities')
 for name in NAMES:
  need(a.numerical_ok([x for x in numeric if x.get('method')==name],False,name),'numeric:'+name)
  rr=[x for x in eq if x.get('method')==name];need(a.freeze_check(rr,a.EQ_BOUNDARIES,name) and all(x.get('scope')==x.get('accurate')=='0' for x in rr),'bitwise:'+name)
 for lab in ('PAIRPLAN_COUNTERS','PAIRPLAN_WORK_GATE','PAIRPLAN_PROFILE_GATE'):
  rr=t.get(lab,[]);need(len(rr)==10 and {x.get('method') for x in rr}==set(NAMES),'counter_matrix:'+lab)
  for name in NAMES:
   rows=[x for x in rr if x.get('method')==name];need(len(rows)==2 and {x.get('layer') for x in rows}=={'1','2'},'two_layers:'+lab+name)
   if lab!='PAIRPLAN_COUNTERS':need(all(x.get('pass')=='1' and (lab!='PAIRPLAN_PROFILE_GATE' or x.get('bitwise')=='1') for x in rows),'profile_work_gate:'+name)
 counters=t.get('PAIRPLAN_COUNTERS',[])
 for x in counters:
  desired=dict(source_visits=E,feature_adds=128*E,decode_rows=E,csr_reads=E,shared_heads=0,comparisons=0,runtime_head_comparisons=0,exact_mask_bytes_read=0,exact_pair_flags_tested=0,exact_pair_windows=0,exact_pair_edges=0,exact_shared_edges=0,projection_scopes=w['scopes'],windows=w['windows'],amx_tdp=32*w['scopes'],partial_zero_bytes=0,packing_load_bytes=0,partial_load_bytes=w['loads'],partial_store_bytes=w['stores'],packed_bytes=w['packed'],direct_rows=w['direct'],prefetch_requests=w['prefetch'],prefetch_index_reads=w['prefetch']//4,relu_elements=128*iw['epilogue_rows'] if x.get('layer')=='1' else 0)
  need(all(a.as_int(x.get(k))==v for k,v in desired.items()),'independent_CSR_work:'+x.get('method','?')+':'+x.get('layer','?'))
  need(x.get('profile_path')=='production_BF16_epilogue' and x.get('scope')==x.get('accurate')=='0','profile_context')
 for li in ('1','2'):
  rows=[x for x in counters if x.get('layer')==li and x.get('method')!=NAMES[0]]
  need(len(rows)==4 and all({k:v for k,v in x.items() if k!='method'}=={k:v for k,v in rows[0].items() if k!='method'} for x in rows),'identical_slot_work_L'+li)
 slots=t.get('ORDERCTRL_SLOT',[]);need(len(slots)==(5 if smoke else 75) and int(complete.get('slot_checks',-1))==len(slots),'slot_count')
 need([a.as_int(x.get('serial')) for x in slots]==list(range(len(slots))),'slot_serial')
 ptrkeys=[k for k in slots[0] if k.endswith('_ptr') and k!='execution_method_ptr']+['tiles_size','mask_size','tiles_capacity','mask_capacity'] if slots else []
 for x in slots:
  need(x.get('method') in NAMES and x.get('stable')==x.get('pass')=='1' and x.get('mode')=='0' and x.get('copy_in_compute')=='0','slot_gate')
  need(all(x.get(k)==slots[0].get(k) for k in ptrkeys),'same_addresses_sizes_capacities')
  need(x.get('slot_hash')==x.get('expected_hash') and a.as_float(x.get('copy_ms')) is not None and float(x['copy_ms'])>=0,'slot_content_copy')
  if x.get('method')!=NAMES[0]:need(x.get('uses_slot')=='1' and x.get('slot_hash')==pl.get(PLAN.get(x.get('method')),{}).get('slot_hash') and x.get('execution_name')=='slot_mode0','selected_slot_payload')
 plannedslots=[x for x in slots if x.get('method')!=NAMES[0]];need(len({x.get('execution_method_ptr') for x in plannedslots})==1 and all(x.get('execution_method_ptr') for x in plannedslots),'same_execution_object')
 keys={(m,'correctness',0) for m in NAMES}
 if not smoke:keys|={(m,'same_slot_consecutive',0) for m in NAMES}|{(m,'same_slot_alternating',i) for m in NAMES for i in range(10)}|{(m,'pmu',i) for m in NAMES for i in range(3)}
 need({(x.get('method'),x.get('phase'),a.as_int(x.get('repeat'))) for x in slots}==keys,'switch_matrix')
 times=t.get('ORDERCTRL_TIME',[]);pmus=t.get('METHOD_PMU',[]);threads=t.get('METHOD_PMU_THREAD',[]);walls=t.get('ORDERCTRL_PMU_WALL',[]);pmua=[]
 if smoke:need(not(times or pmus or threads or walls),'smoke_no_performance')
 else:
  need(len(times)==75,'time_count')
  for kind,nr in KINDS.items():
   for name in NAMES:
    rr=[x for x in times if x.get('kind')==kind and x.get('method')==name];need(len(rr)==nr and {a.as_int(x.get('repeat')) for x in rr}==set(range(nr)),'repeats:'+kind+name)
    for x in rr:
     rep=int(x['repeat']);order=NAMES.index(name) if kind=='same_slot_consecutive' or rep%2==0 else 4-NAMES.index(name)
     need(a.as_float(x.get('ms')) is not None and float(x['ms'])>0 and a.as_int(x.get('order'))==order and x.get('warmups_before_series' if kind=='same_slot_consecutive' else 'warmups_before_measure')=='1','time_context')
  pk={(m,str(i)) for m in NAMES for i in range(3)}
  for lab,rr in [('PMU',pmus),('PMU_WALL',walls)]:need(len(rr)==15 and {(x.get('method'),x.get('repeat')) for x in rr}==pk,lab+'_matrix')
  for x in pmus:
   rr=[z for z in threads if (z.get('method'),z.get('repeat'))==(x.get('method'),x.get('repeat'))]
   usable=x.get('available')==x.get('coverage_complete')==x.get('scaled_complete')=='1' and all(x.get(k)=='32' for k in ('initial_threads','final_threads','counted_threads','scaled_threads')) and all(x.get(k)=='0' for k in ('failed_threads','zero_running_threads','new_threads','disappeared_threads')) and len(rr)==32 and len({z.get('tid') for z in rr})==32 and all(z.get('available')==z.get('present_at_stop')==z.get('scaled_valid')=='1' and a.as_int(z.get('time_running_ns')) is not None and int(z['time_running_ns'])>0 for z in rr)
   sums_valid=True
   for ev in ('cycles','instructions','cache_misses','cache_references','amx_busy'):
    raw=[a.as_int(z.get(ev+'_raw')) for z in rr];scaled=[a.as_float(z.get(ev+'_scaled')) for z in rr]
    valid=bool(rr) and None not in raw and None not in scaled and all(v>=0 for v in raw+scaled) and a.as_int(x.get(ev+'_raw'))==sum(raw) and a.as_float(x.get(ev+'_scaled')) is not None and math.isclose(float(x[ev+'_scaled']),sum(scaled),rel_tol=1e-9,abs_tol=1e-3)
    sums_valid=sums_valid and valid
   usable=usable and sums_valid and x.get('event_count')=='5' and x.get('amx_busy_requested')=='1' and x.get('cpu_model')=='143' and all(a.as_float(x.get(ev+'_scaled')) is not None and float(x[ev+'_scaled'])>0 for ev in ('cycles','instructions','amx_busy'))
   pmua.append(dict(method=x.get('method'),repeat=x.get('repeat'),usable=usable,thread_records=len(rr),counter_sums_valid=sums_valid))
   if not usable:warnings.append('PMU_unusable:'+x.get('method','?')+':'+x.get('repeat','?'))
 return dict(**a.context(p),valid=not errs,errors=errs,warnings=warnings,raw_records=t,independent_CSR_work=iw,process_status=proc,pmu_audit=pmua,counts=dict(numerical=len(numeric),bitwise=len(eq),work=len(t.get('PAIRPLAN_WORK_GATE',[])),profile=len(t.get('PAIRPLAN_PROFILE_GATE',[])),slots=len(slots),timings=len(times),pmu_regions=len(pmus),pmu_threads=len(threads)))

def main():
 if len(sys.argv)!=3:raise SystemExit('usage analyze_order_control.py RUN NEW_OUTPUT_DIR')
 run=pathlib.Path(sys.argv[1]).resolve();out=pathlib.Path(sys.argv[2]).resolve()
 if not str(out).startswith(str(BASE.resolve())+'/results/') or out.exists():raise ValueError('New results directory required')
 build=build_audit(run);packets,jobs,slurm=a.load_packets(run,build);audits=[]
 for p in packets:
  try:audits.append(packet_audit(p))
  except Exception as e:audits.append(dict(**a.context(p),valid=False,errors=['audit_exception:'+repr(e)],raw_records=p['parsed']))
 errs=[]
 if len(jobs)!=3 or not all(j['valid'] for j in jobs):errs.append('expected_three_completed_valid_jobs')
 if len(audits)!=19 or sum(x['data_class']=='smoke' for x in audits)!=13:errs.append('expected_19_packets_13_smoke')
 perf=[x for x in audits if x['data_class']=='synthetic']
 if len(perf)!=6 or {(x['partition'],x['exclusive'],x['graph']) for x in perf}!={(p,e,g) for p,e in [('intel','false'),('intel_expr','true')] for g in a.SYN3}:errs.append('expected_3_synthetic_graphs_each_allocation_class')
 rows=[];ratios=[];pmurows=[];copies=[]
 for x in audits:
  if not x.get('valid') or x['data_class']=='smoke':continue
  ctx={k:x[k] for k in ('job','node','partition','exclusive','graph','input_sha256','build_fingerprint')};tt=x['raw_records']['ORDERCTRL_TIME'];lookup={}
  for kind,nr in KINDS.items():
   for name in NAMES:
    rr=sorted([z for z in tt if z['kind']==kind and z['method']==name],key=lambda z:int(z['repeat']));vals=[float(z['ms']) for z in rr];row=dict(**ctx,method=name,protocol=kind,median_ms=statistics.median(vals),min_ms=min(vals),max_ms=max(vals),repeats=nr,raw_ms=json.dumps(vals));rows.append(row);lookup[(kind,name)]=row
   for ref in NAMES:
    for method in NAMES:
     if method==ref:continue
     ratios.append(dict(**ctx,protocol=kind,reference=ref,method=method,reference_median_ms=lookup[(kind,ref)]['median_ms'],method_median_ms=lookup[(kind,method)]['median_ms'],speedup=lookup[(kind,ref)]['median_ms']/lookup[(kind,method)]['median_ms']))
  for name in NAMES:
   rr=[z for z in x['raw_records'].get('METHOD_PMU',[]) if z['method']==name];r=dict(**ctx,method=name,repeats=len(rr),all_usable=all(z['usable'] for z in x['pmu_audit'] if z['method']==name))
   for k in ('cycles_scaled','instructions_scaled','cache_misses_scaled','cache_references_scaled','amx_busy_scaled','region_wall_ms'):
    vv=[a.as_float(z.get(k)) for z in rr];r['median_'+k]=statistics.median(vv) if vv and None not in vv else None
   pmurows.append(r)
   cc=[float(z['copy_ms']) for z in x['raw_records']['ORDERCTRL_SLOT'] if z['method']==name];copies.append(dict(**ctx,method=name,copies=len(cc),median_copy_ms=statistics.median(cc),min_copy_ms=min(cc),max_copy_ms=max(cc)))
 totals=collections.Counter()
 for x in audits:totals.update(x.get('counts',{}))
 if len(rows)!=60 or totals['timings']!=450 or totals['pmu_regions']!=90:errs.append('expected_60_summary_450_timings_90_PMU_regions')
 report=dict(analysis_pass=build['valid'] and not errs and all(x['valid'] for x in audits),errors=errs,build=build,jobs=jobs,packets=audits,counts=dict(totals),slurm=slurm,helper=dict(path=str(HELPER),sha256=a.hashed(HELPER)),source_script_sha256=a.digest(pathlib.Path(__file__)),input_hashes=a.INPUT_HASHES,interpretation_limits=['All speedup denominators are within same graph/job/protocol/build.','Exploratory order control; never merge into primary17 graph geometric mean.','Same-slot tests order variation under warmed execution and fixed data/metadata addresses, not deployment plan-copy overhead.','Generic cache misses/references are not DRAM traffic; AMX busy is not overlap.','PMU unusable regions are retained with coverage flags and excluded from counter claims.'])
 out.mkdir(parents=True)
 (out/'audit.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n')
 a.write_csv(out/'summary.csv',rows,['job','node','partition','exclusive','graph','method','protocol','median_ms'])
 a.write_csv(out/'ratios.csv',ratios,['job','node','partition','exclusive','graph','protocol','reference','method','speedup'])
 a.write_csv(out/'pmu_summary.csv',pmurows,['job','node','partition','exclusive','graph','method','all_usable'])
 a.write_csv(out/'copy_summary.csv',copies,['job','node','partition','exclusive','graph','method','median_copy_ms'])
 print(json.dumps(dict(analysis_pass=report['analysis_pass'],errors=errs,invalid_packets=[dict(graph=x['graph'],job=x['job'],errors=x['errors']) for x in audits if not x['valid']],counts=dict(totals),summary_rows=len(rows),pmu_usable=sum(y['usable'] for x in audits for y in x.get('pmu_audit',[])),out=str(out)),ensure_ascii=False))
 return 0 if report['analysis_pass'] else 2
if __name__=='__main__':sys.exit(main())
