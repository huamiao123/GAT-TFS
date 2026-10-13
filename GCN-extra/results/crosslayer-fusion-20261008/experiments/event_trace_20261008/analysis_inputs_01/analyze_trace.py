#!/usr/bin/env python3
"""Independent event-trace raw-log audit. No kernels or jobs are executed."""
import sys,pathlib,importlib.util,collections,shlex,json,math,statistics,re,hashlib,array,struct
sys.dont_write_bytecode=True
BASE=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
HELPER=BASE/'experiments/pair_plan_20261007/analyze_plan.py'
spec=importlib.util.spec_from_file_location('primary_audit',HELPER);a=importlib.util.module_from_spec(spec);spec.loader.exec_module(a)
NAMES=('pack_epi_degree','pack_epi_qguard','dyn_i','dyn_s','exact_s','trace_i','trace_s_cached','trace_s_rep')
SPEC={n:dict(kind=k,mode=m,schedule=s) for n,k,m,s in zip(NAMES,(3,3,4,4,4,5,5,5),(0,0,1,2,4,0,1,2),(0,4,0,0,0,0,0,0))}
FROZEN=('state_kernels.hpp','pair_reducer.hpp','state_runtime.hpp','grouping_schedule.hpp','pair_plan.hpp','planned_reducer.hpp','planned_kernel.hpp','plan_runtime.hpp','paper_methods_kernels.hpp','paper_methods_runtime.hpp','projection_window_kernels.hpp','projection_window_pmu.hpp','gcn_e2e_v3.cpp')
KINDS={'consecutive':5,'alternating':10}

def parse(path):
 out=collections.defaultdict(list);errors=[]
 for n,line in enumerate(a.read_text(path).splitlines(),1):
  label,_,body=line.partition(' ')
  if not label.startswith(('TRACE_','METHOD_','PAIRPLAN_','PLAN_GATE','GROUP_')):continue
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
 need(a.hashed(HELPER)=='bcac6ddc97042cbf4d3643ae7fa846a2f716ee1fc4c05d6b238269874cf90fd9','validated_helper_identity')
 need(bool(st) and m.get('source_hashes')==st,'source_manifest')
 need(set(ss)=={str(p.resolve()) for p in (run/'source_snapshot').rglob('*') if p.is_file()},'source_coverage')
 need(bool(bt) and m.get('binary_hashes')==bt,'binary_manifest')
 need(set(bb)=={str((run/'build'/n).resolve()) for n in ('trace_methods','paper_original')},'binary_coverage')
 need(m.get('dimensions')==[128,128,128] and m.get('numa_interleave') is False and m.get('node_cap')==10,'build_contract')
 ev=a.read_json(run/'build/event.json');need(ev.get('status')=='PASS' and ev.get('exit_status')==0,'build_exit')
 ka=a.read_json(run/'build/kernel_static_audit.json')
 need(ka.get('status')=='PASS' and ka.get('theoretical_record_cases')==1848 and ka.get('malformed_record_model_rejections')==6734 and all(ka.get(k) is True for k in ('cpp_full_validator_and_runtime_bitwise_gates_still_required','header128_checked','kernel_orchestration_identity')),'trace_static_audit')
 for n in ('trace_kernel.hpp','trace_plan.hpp','trace_reducer.hpp'):need(ka.get('source_sha256',{}).get(n)==a.hashed(run/'source_snapshot'/n),'static_source_hash:'+n)
 asm_path=BASE/'experiments/event_trace_20261008/codegen_audit_01/audit.json';asm=a.read_json(asm_path)
 need(a.hashed(asm_path)=='be569f3d11363d6a0fec81f0394144307cd38baf648ff3229a188fec7a46672e' and asm.get('schema')=='event_trace_codegen_audit_v1' and asm.get('status')=='PASS','codegen_artifact_identity')
 need(pathlib.Path(asm.get('run','')).resolve()==run.resolve() and len(asm.get('checks',[]))==47 and all(x.get('pass') is True for x in asm.get('checks',[])),'codegen_47_checks')
 need(a.hashed(asm_path.parent/'audit_codegen.py')==asm.get('script_sha256')=='3b40a57c30fc913e17ee2845385eabd2aa87f5547b186aa93c620c7055f7ade7','codegen_audit_script_identity')
 for filename,h in asm.get('inputs_sha256',{}).items():need(a.hashed(pathlib.Path(filename))==h,'codegen_input_hash:'+filename)
 for filename,h in asm.get('extracts_sha256',{}).items():need(a.hashed(asm_path.parent/filename)==h,'codegen_extract_hash:'+filename)
 for name,count in [('dyn_i',522),('dyn_s',563)]:
  x=asm.get('old_dynamic_reducer_identity',{}).get(name,{})
  need(x.get('equal') is True and x.get('current_instruction_count')==x.get('previous_instruction_count')==count and x.get('current_normalized_sha256')==x.get('previous_normalized_sha256') and x.get('difference')==[],'frozen_dynamic_instruction_identity:'+name)
 eq=[]
 for n in FROZEN:
  old=BASE/'runs/access-prefetch-20261008-01/source_snapshot'/n;new=run/'source_snapshot'/n
  oh=a.hashed(old);nh=a.hashed(new);eq.append(dict(file=n,primary_sha256=oh,control_sha256=nh,equal=oh==nh));need(oh==nh,'frozen_source:'+n)
 return dict(valid=not errs,errors=errs,manifest=m,sources=ss,binaries=bb,source_text=st,build_fingerprint=hashlib.sha256(st.encode()).hexdigest(),frozen_kernel_equality=eq,kernel_static_audit=ka,codegen_audit=asm)

METADATA_CACHE={}
def metadata_work(path,iw):
 key=str(path.resolve())
 if key in METADATA_CACHE:return METADATA_CACHE[key]
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
 orders=16*windows-(16-tail)*lastwindows if tail else 16*windows
 flags=8*windows-(8-tail//2)*lastwindows if tail else 8*windows
 headers=8*windows-(8-(tail+1)//2)*lastwindows if tail else 8*windows
 result=dict(order_reads=orders,pair_flags=flags,trace_headers=headers)
 METADATA_CACHE[key]=result;return result

def unsigned_csv(text):
 if text in ('none',''):return []
 if not re.fullmatch(r'[0-9]+(?:,[0-9]+)*',text):raise ValueError('invalid unsigned CSV')
 return [int(x) for x in text.split(',')]

def audit_smoke_trace(path,dump,need):
 """Independent Python CSR replay, used ONLY for bounded smoke fixtures.
 Every code is checked against scalar head comparison. Neither builder stats
 nor its visitor/helper participates in this reconstruction.
 """
 with path.open('rb') as f:
  header=f.read(36);_,_,_,N,M,E=struct.unpack('<IIIQQQ',header)
  if N>4096 or E>1000000:raise ValueError('smoke-only independent replay size bound')
  row=array.array('I');row.frombytes(f.read(4*(N+1)))
  col=array.array('I');col.frombytes(f.read(4*E))
  if sys.byteorder!='little':row.byteswap();col.byteswap()
 need(N==M and len(row)==N+1 and len(col)==E and row[-1]==E,'smoke_CSR_shape')
 perm=unsigned_csv(dump.get('degree_perm',''));offsets=unsigned_csv(dump.get('tile_offsets',''))
 raw=dump.get('data_hex','');raw='' if raw=='none' else raw
 if not re.fullmatch(r'(?:[0-9a-f]{2})*',raw):raise ValueError('noncanonical trace hex')
 data=bytes.fromhex(raw)
 need(len(perm)==N and set(perm)==set(range(N)),'smoke_degree_permutation')
 degrees=[row[i+1]-row[i] for i in range(N)]
 need([degrees[i] for i in perm]==sorted(degrees),'smoke_degree_order')
 need(len(offsets)==(N+15)//16+1 and offsets[0]==0 and offsets[-1]==len(data) and offsets==sorted(offsets),'smoke_trace_offsets')
 stats=collections.Counter();hist=collections.Counter();opp=collections.Counter();offset=0;hash_perm=1469598103934665603
 for v in perm:hash_perm=((hash_perm^v)*1099511628211)&((1<<64)-1)
 for ti,first in enumerate(range(0,N,16)):
  need(offset==offsets[ti],'smoke_tile_start:'+str(ti))
  rows=perm[first:first+16];md=max((degrees[i] for i in rows),default=0)
  stats['nonempty_tiles']+=md>0;pair_shared=collections.Counter()
  for ws in range(0,md,64):
   for p in range(0,len(rows),2):
    i0=rows[p];i1=rows[p+1] if p+1<len(rows) else None
    seq0=col[row[i0]+min(ws,degrees[i0]):row[i0]+min(ws+64,degrees[i0])]
    seq1=col[row[i1]+min(ws,degrees[i1]):row[i1]+min(ws+64,degrees[i1])] if i1 is not None else []
    expected=[];k0=k1=0
    while k0<len(seq0) or k1<len(seq1):
     if k0==len(seq0):code=2
     elif k1==len(seq1):code=1
     else:
      stats['comparisons']+=1
      code=3 if seq0[k0]==seq1[k1] else (1 if seq0[k0]<seq1[k1] else 2)
     expected.append(code);k0+=bool(code&1);k1+=bool(code&2)
    if offset>=len(data):raise ValueError('missing record header')
    count=data[offset];offset+=1;hist[count]+=1;stats['headers']+=1
    need(count<=128 and count==len(expected),'smoke_record_event_count')
    nbytes=(count+3)//4
    if offset+nbytes>len(data):raise ValueError('truncated record payload')
    blob=data[offset:offset+nbytes];offset+=nbytes
    actual=[(blob[k//4]>>(2*(k%4)))&3 for k in range(count)]
    need(all(x in (1,2,3) for x in actual) and actual==expected,'smoke_event_greedy_exact')
    if count%4:need(blob[-1]>>(2*(count%4))==0,'smoke_payload_zero_padding')
    stats['events']+=count;stats['payload_bytes']+=nbytes
    for code in expected:stats['take'+str(code)]+=1
    pair_shared[p]+=expected.count(3)
  need(offset==offsets[ti+1],'smoke_tile_end:'+str(ti))
  for p in range(0,len(rows),2):
   seqs=[list(col[row[r]:row[r+1]]) for r in rows[p:p+2]]
   sorted_flags=[all(q[j]<=q[j+1] for j in range(len(q)-1)) for q in seqs]
   opp['rows']+=len(seqs);opp['sorted_rows']+=sum(sorted_flags);opp['unsorted_rows']+=len(seqs)-sum(sorted_flags)
   if len(seqs)==1:seqs.append([]);opp['odd_rows']+=1
   else:opp['pairs']+=1
   x,y=seqs;i=j=shared=0
   while i<len(x) or j<len(y):
    opp['events']+=1
    if i==len(x):j+=1
    elif j==len(y):i+=1
    else:
     opp['comparisons']+=1
     if x[i]==y[j]:i+=1;j+=1;shared+=1
     elif x[i]<y[j]:i+=1
     else:j+=1
   opp['source_reads']+=len(x)+len(y);s64=pair_shared[p];opp['shared_s64']+=s64;opp['shared_full']+=shared
   if p+1<len(rows):
    relation='gain' if shared>s64 else ('loss' if shared<s64 else 'tie');opp[relation+'_pairs']+=1
    group='sorted' if all(sorted_flags) else 'unsorted';opp[group+'_pairs']+=1;opp[group+'_shared_s64']+=s64;opp[group+'_shared_full']+=shared
    if group=='sorted':opp['sorted_'+relation+'_pairs']+=1;need(shared>=s64,'sorted_full_greedy_dominates_S64')
 need(offset==len(data),'smoke_exact_data_exhaustion')
 need(stats['take1']+stats['take2']+2*stats['take3']==E,'smoke_source_edge_conservation')
 trace_hash=1469598103934665603
 for v in b''.join(struct.pack('<Q',x) for x in offsets)+data:trace_hash=((trace_hash^v)*1099511628211)&((1<<64)-1)
 return dict(stats={k:stats[k] for k in ('headers','events','payload_bytes','take1','take2','take3','comparisons','nonempty_tiles')},histogram=[hist[i] for i in range(129)],data_bytes=len(data),offset_bytes=8*len(offsets),permutation_fnv64=hash_perm,payload_hash=trace_hash,opportunity=dict(opp),independent_python_replay=True)

def pmu_audit(t,need):
 rr=t.get('METHOD_PMU',[]);threads=t.get('METHOD_PMU_THREAD',[]);out=[]
 for x in rr:
  tr=[z for z in threads if (z.get('method'),z.get('repeat'))==(x.get('method'),x.get('repeat'))]
  usable=x.get('available')==x.get('coverage_complete')==x.get('scaled_complete')=='1' and all(x.get(k)=='32' for k in ('initial_threads','final_threads','counted_threads','scaled_threads')) and all(x.get(k)=='0' for k in ('failed_threads','zero_running_threads','new_threads','disappeared_threads')) and len(tr)==32 and len({z.get('tid') for z in tr})==32 and all(z.get('available')==z.get('present_at_stop')==z.get('scaled_valid')=='1' and a.as_int(z.get('time_running_ns')) is not None and int(z['time_running_ns'])>0 for z in tr)
  sums=True
  for z in tr:
   enabled=a.as_int(z.get('time_enabled_ns'));running=a.as_int(z.get('time_running_ns'))
   if enabled is None or running is None or running<=0 or enabled<running:sums=False;continue
   for ev in ('cycles','instructions','cache_misses','cache_references','amx_busy'):
    raw=a.as_int(z.get(ev+'_raw'));scaled=a.as_float(z.get(ev+'_scaled'))
    if raw is None or scaled is None or not math.isclose(scaled,raw*enabled/running,rel_tol=1e-9,abs_tol=1e-3):sums=False
  for ev in ('cycles','instructions','cache_misses','cache_references','amx_busy'):
   raw=[a.as_int(z.get(ev+'_raw')) for z in tr];scaled=[a.as_float(z.get(ev+'_scaled')) for z in tr]
   ok=bool(tr) and None not in raw and None not in scaled and all(v>=0 for v in raw+scaled) and a.as_int(x.get(ev+'_raw'))==sum(raw) and a.as_float(x.get(ev+'_scaled')) is not None and math.isclose(float(x[ev+'_scaled']),sum(scaled),rel_tol=1e-9,abs_tol=1e-3)
   sums=sums and ok
  usable=usable and sums and x.get('event_count')=='5' and x.get('amx_busy_requested')=='1' and x.get('cpu_model')=='143' and all(a.as_float(x.get(ev+'_scaled')) is not None and float(x[ev+'_scaled'])>0 for ev in ('cycles','instructions','amx_busy'))
  need(usable,'PMU_coverage:'+x.get('method','?')+':'+x.get('repeat','?'))
  out.append(dict(method=x.get('method'),repeat=x.get('repeat'),usable=usable,thread_records=len(tr),counter_sums_valid=sums))
 return out


def opportunity_audit(x,N,E,shared,need):
 keys=('rows sorted_rows unsorted_rows pairs odd_rows sorted_pairs unsorted_pairs gain_pairs loss_pairs tie_pairs sorted_gain_pairs sorted_loss_pairs sorted_tie_pairs shared_s64 shared_full sorted_shared_s64 sorted_shared_full unsorted_shared_s64 unsorted_shared_full source_reads events comparisons invalid').split()
 v={k:a.as_int(x.get(k)) for k in keys};need(None not in v.values() and all(z>=0 for z in v.values() if z is not None),'opportunity_nonnegative_fields')
 if None in v.values():return
 need(x.get('diagnostic_only')==x.get('source_level')==x.get('pass')=='1' and x.get('semantics')=='full_row_greedy' and v['invalid']==0,'opportunity_contract')
 need(a.as_float(x.get('diagnostic_ms')) is not None and float(x['diagnostic_ms'])>=0,'opportunity_time')
 need(v['rows']==N==v['pairs']*2+v['odd_rows'] and v['odd_rows']==N%2,'opportunity_row_partition')
 need(v['sorted_rows']+v['unsorted_rows']==N and v['sorted_pairs']+v['unsorted_pairs']==v['pairs'],'opportunity_sorted_partition')
 need(v['gain_pairs']+v['loss_pairs']+v['tie_pairs']==v['pairs'] and v['sorted_gain_pairs']+v['sorted_tie_pairs']==v['sorted_pairs'] and v['sorted_loss_pairs']==0,'opportunity_relation_partition')
 need(v['shared_s64']==shared and v['shared_full']<=E//2 and v['events']==E-v['shared_full'] and v['source_reads']==E and v['comparisons']<=v['events'],'opportunity_conservation')
 for k in ('shared_s64','shared_full'):need(v[k]==v['sorted_'+k]+v['unsorted_'+k],'opportunity_shared_partition:'+k)
 need(v['sorted_shared_full']>=v['sorted_shared_s64'],'opportunity_sorted_bound')
 for k in ('gain_pairs','loss_pairs','tie_pairs'):need(v['sorted_'+k]<=v[k],'opportunity_subset:'+k)

def trace_plan_audit(t,one,need,input_path,smoke,iw,meta,st):
 N,E=iw['N'],iw['E'];w=iw['work']['0'];p=one('TRACE_PLAN');setup=one('TRACE_PLAN_SETUP');gate=one('TRACE_PLAN_GATE');opp=one('TRACE_OPPORTUNITY')
 def eq(k,v):need(a.as_int(p.get(k))==v,'trace_plan:'+k)
 dyn=int(st['dynamic_shared_heads']);comparisons=int(st['comparisons'])
 need(all(p.get(k)=='1' for k in ('order_preserved','exact_exhaustion','padding_zero','header_hist_checked','pass')),'trace_plan_independent_gate')
 need(all(gate.get(k)=='1' for k in ('pass','independent_scalar_validator','original_greedy')) and gate.get('source_ids_stored')=='0','trace_builder_gate')
 need(setup.get('format')=='count_u8_plus_lsb2' and setup.get('source_ids_stored')=='0' and setup.get('inspector_includes_full_validator')=='1' and setup.get('bytes_semantics')=='persistent_vector_payload','trace_layout_contract')
 for k in ('build_ms','validate_ms','total_ms'):need(a.as_float(setup.get(k)) is not None and float(setup[k])>=0,'trace_setup_time:'+k)
 need(float(setup['total_ms'])+1e-6>=float(setup['build_ms'])+float(setup['validate_ms']),'trace_setup_phases')
 for k,v in dict(tiles=(N+15)//16,nonempty_tiles=w['scopes'],windows=w['windows'],source_edges=E,events=E-dyn,shared_heads=dyn,comparisons=comparisons,take3=dyn,headers=meta['trace_headers']).items():eq(k,v)
 need(int(p['take1'])+int(p['take2'])+2*dyn==E,'trace_edge_conservation')
 hist=t.get('TRACE_PLAN_HIST',[]);need(len(hist)==129 and {a.as_int(x.get('event_count')) for x in hist}==set(range(129)),'trace_hist_matrix')
 hh=[0]*129
 for x in hist:
  c=a.as_int(x.get('event_count'));v=a.as_int(x.get('builder_records'))
  need(c is not None and 0<=c<=128 and v is not None and v>=0 and a.as_int(x.get('validator_records'))==v and x.get('pass')=='1','trace_hist_gate')
  if c is not None and 0<=c<=128 and v is not None:hh[c]=v
 eq('headers',sum(hh));eq('events',sum(c*n for c,n in enumerate(hh)));eq('payload_bytes',sum(((c+3)//4)*n for c,n in enumerate(hh)))
 eq('data_bytes',int(p['headers'])+int(p['payload_bytes']));eq('data_size',int(p['data_bytes']));eq('offsets_size',(N+15)//16+1);eq('offset_element_bytes',8);eq('offset_bytes',8*int(p['offsets_size']));eq('plan_bytes',int(p['data_bytes'])+int(p['offset_bytes']));eq('allocated_bytes',int(p['data_capacity'])+8*int(p['offsets_capacity']))
 need(int(p['data_capacity'])>=int(p['data_size']) and int(p['offsets_capacity'])>=int(p['offsets_size']) and int(p['object_bytes'])>0,'trace_capacity_bounds')
 for k in ('source_edges','shared_heads','comparisons','nonempty_tiles','windows','headers','payload_bytes','data_bytes','offset_bytes','plan_bytes','allocated_bytes','take1','take2','take3','object_bytes'):need(setup.get(k)==p.get(k),'builder_runtime:'+k)
 need(setup.get('merge_steps')==p.get('events') and setup.get('fnv64')==p.get('plan_fnv64')==p.get('payload_hash'),'trace_hash_identity')
 opportunity_audit(opp,N,E,dyn,need);replay=None
 if smoke:
  replay=audit_smoke_trace(input_path,one('TRACE_SMOKE_DUMP'),need)
  for k,v in replay['stats'].items():need(a.as_int(p.get(k))==v,'independent_smoke_plan:'+k)
  need(hh==replay['histogram'],'independent_smoke_histogram')
  for k in ('data_bytes','offset_bytes','payload_hash'):need(a.as_int(p.get(k))==replay[k],'independent_smoke:'+k)
  need(a.as_int(one('GROUP_NATIVE_PERM').get('fnv64'))==replay['permutation_fnv64'],'independent_smoke_permutation_hash')
  for k in ('rows sorted_rows unsorted_rows pairs odd_rows sorted_pairs unsorted_pairs gain_pairs loss_pairs tie_pairs sorted_gain_pairs sorted_loss_pairs sorted_tie_pairs shared_s64 shared_full sorted_shared_s64 sorted_shared_full unsorted_shared_s64 unsorted_shared_full source_reads events comparisons invalid').split():need(a.as_int(opp.get(k))==replay['opportunity'].get(k,0),'independent_smoke_opportunity:'+k)
  rr=t.get('TRACE_REJECT_CASE',[]);need(len(rr)==2 and {x.get('case') for x in rr}=={'used_code_zero','tile_offset_oob'} and all(all(x.get(k)=='1' for k in ('valid_fixture','rejected','reason_present','pass')) for x in rr),'malformed_rejections')
  rej=one('TRACE_REJECT_AUDIT');need(rej.get('cases')==rej.get('rejected')=='2' and rej.get('valid_fixture')==rej.get('pass')=='1','malformed_rejection_audit')
 else:need(not any(t.get(k) for k in ('TRACE_SMOKE_DUMP','TRACE_REJECT_CASE','TRACE_REJECT_AUDIT')),'performance_no_fixture_dump')
 return p,replay

def audit_job_counts(jobs,audits):
 smoke_names={'window_tail','window_high_tail','window_scope_tail'}|{'state_tail_'+str(n) for n in (1,15,16,17,31,33,65)}|{'plan_exact_tail','plan_exact_unsorteddup','plan_exact_mismatch'}
 fields={'checks':'METHOD_CHECK','equiv':'PAIRPLAN_EQUIV','exec_equiv':'TRACE_EXEC_EQUIV','dyns_equiv':'TRACE_DYNS_EQUIV','profile':'PAIRPLAN_PROFILE_GATE','work':'PAIRPLAN_WORK_GATE','trace_logical':'TRACE_LOGICAL','trace_hist':'TRACE_PLAN_HIST','trace_reject_cases':'TRACE_REJECT_CASE','slot_checks':'TRACE_SLOT','pmu_regions':'METHOD_PMU'}
 for job in jobs:
  packets=[x for x in audits if x['job']==job['job']];errors=job['errors']
  if len(packets)!=job['completion'].get('graphs'):errors.append('packet_completion_count')
  if job['mode']=='smoke' and {x['graph'] for x in packets}!=smoke_names:errors.append('exact_smoke_fixture_coverage')
  if job['mode']=='synthetic' and {x['graph'] for x in packets}!=set(a.SYN3):errors.append('exact_synthetic_fixture_coverage')
  for field,label in fields.items():
   actual=sum(len(x.get('raw_records',{}).get(label,[])) for x in packets)
   if job['completion'].get(field)!=actual:errors.append('completion_'+field+'_vs_raw')
  job['valid']=not errors and not job['pending']
  if not job['valid']:
   for x in packets:x['valid']=False;x['errors'].append('job_completion_audit_failed')

def packet_audit(p):
 t=p['parsed'];errs=list(p['parse_errors']);warnings=[];folder=pathlib.Path(p['folder']);smoke=p['mode']=='smoke'
 def need(v,k):
  if not v:errs.append(k)
 def one(label):
  rr=t.get(label,[]);need(len(rr)==1,'cardinality:'+label);return rr[0] if len(rr)==1 else {}
 need(p['job_status']=='PASS','job_context')
 proc=a.read_json(folder/'methods.status.json');cmd=a.read_json(folder/'methods.command.json');stamp=a.read_json(folder/'binary_input_hashes.json');argv=cmd.get('argv',[])
 need(proc.get('returncode')==0,'process_exit');need(len(argv)==3 and pathlib.Path(argv[0]).resolve()==pathlib.Path(p['root'])/'build/trace_methods' and argv[-1]==p['graph'],'process_command');need(cmd.get('numa_policy') is None,'process_NUMA_override')
 input_path=pathlib.Path(p['graph_item'].get('path',''));actual=a.hashed(input_path)
 need(actual==p['graph_item'].get('input_sha256')==stamp.get('input_sha256')==p['status'].get('actual_input_sha256'),'input_hash')
 if p.get('expected_input_sha256'):need(actual==p['expected_input_sha256'],'authoritative_inventory_hash')
 for key in ('immutable_source_manifest','immutable_source_status'):
  source=pathlib.Path(p['graph_item'].get(key,''));need(source.is_file() and a.hashed(source)==p['graph_item'].get(key+'_sha256'),'prior_evidence_hash:'+key)
 prior=a.read_json(pathlib.Path(p['graph_item']['immutable_source_status']));need(prior.get('status')=='PASS' and prior.get('actual_input_sha256')==actual,'frozen_prior_input_acceptance')
 need(stamp.get('binary_sha256')==a.hashed(pathlib.Path(p['root'])/'build/trace_methods'),'process_binary_hash');need(p['status'].get('status')=='PASS','suite_status')
 env=a.read_json(folder/'experiment_env.json');need(env.get('TRACE_SMOKE')==str(int(smoke)) and env.get('TRACE_PMU')==str(int(not smoke)),'experiment_env')
 for rr in t.values():need(all(x.get('graph',p['graph'])==p['graph'] and x.get('pass','1')!='0' for x in rr),'graph_labels_and_no_failed_gate')
 cfg=one('TRACE_CONFIG');complete=one('TRACE_COMPLETE');gate=one('TRACE_GATE_COMPLETE');sem=one('PAIRPLAN_SEMANTIC')
 for x in (complete,gate,sem):need(x.get('pass')=='1','terminal_gate')
 need(sem.get('values')=='19' and sem.get('bitwise')=='1','semantic_values')
 need(all(cfg.get(k)=='128' for k in ('D','H','F')) and cfg.get('S')==cfg.get('R')=='64' and cfg.get('TR')=='16' and cfg.get('M')=='FULL','shape')
 need(cfg.get('methods')==complete.get('methods')=='8' and cfg.get('pf')=='1' and cfg.get('numa')=='default' and cfg.get('final_output')=='BF16' and cfg.get('copy_in_compute')=='0' and cfg.get('trace_resident_for_all_methods')=='1' and cfg.get('fixed_expansion_candidate')=='trace_s_rep','execution_contract')
 need(cfg.get('warmup_consecutive')==cfg.get('warmup_each_alternating_switch')=='1' and cfg.get('consecutive')=='5' and cfg.get('alternating')=='10' and cfg.get('smoke')==complete.get('smoke')==str(int(smoke)),'timing_contract')
 iw=a.degree_work(input_path);N=iw['N'];E=iw['E'];w=iw['work']['0'];meta=metadata_work(input_path,iw);need(int(cfg.get('N',-1))==N and int(cfg.get('E',-1))==E,'CSR_shape')
 mm=t.get('TRACE_METHOD',[]);need(len(mm)==8 and [x.get('method') for x in mm]==list(NAMES),'method_matrix')
 for x in mm:
  m=SPEC.get(x.get('method'),{});need(all(a.as_int(x.get(k))==m.get(k) for k in ('kind','mode')) and a.as_int(x.get('schedule_index'))==m.get('schedule') and x.get('scope')==x.get('accurate')=='0' and x.get('epilogue')==x.get('pf')=='1' and a.as_int(x.get('plan_index'))==(-1 if m.get('kind')==3 else 0),'method_contract')
  need(x.get('schedule')==('qguard' if m.get('schedule')==4 else 'degree'),'method_schedule')
 group=t.get('GROUP_AUDIT',[]);need(len(group)==5 and {x.get('schedule') for x in group}=={'degree','qsource','qpage','qsamplepage','qguard'},'grouping_matrix')
 for x in group:
  need(x.get('pass')=='1' and int(x['rows'])==N and all(a.as_int(x.get(k))==v for k,v in dict(full_projections=w['scopes'],windows=w['windows'],state_load_bytes=w['loads'],state_store_bytes=w['stores']+512*w['direct']).items()),'independent_q64_preserved_grouping_budget')
 need(one('GROUP_GUARD_SELECT').get('uses_timing')=='0','guard_no_timing_selection')
 st=one('PAIRPLAN_STRUCTURE');need(st.get('plan')=='adj_degree' and st.get('kind')=='0','adjacent_plan')
 c=int(st['exact_shared_edges']);dyn=int(st['dynamic_shared_heads']);cw=int(st['exact_windows']);cmp=int(st['comparisons'])
 need(0<=c<=dyn<=E//2 and int(st['source_edges'])==E and int(st['merge_steps'])+dyn==E and int(st['neighbor_windows'])==w['windows'] and int(st['full_projections'])==w['scopes'],'base_structural_bounds')
 pg=one('PLAN_GATE');need(pg.get('plan')=='adj_degree' and pg.get('pass')=='1' and pg.get('invalid')=='0' and pg.get('feature_first_touch_changed')=='0','base_gate')
 plan,replay=trace_plan_audit(t,one,need,input_path,smoke,iw,meta,st)
 numeric=t.get('METHOD_CHECK',[]);eq=t.get('PAIRPLAN_EQUIV',[]);exec_eq=t.get('TRACE_EXEC_EQUIV',[]);dyneq=t.get('TRACE_DYNS_EQUIV',[])
 need(len(numeric)==len(eq)==len(exec_eq)==32 and len(dyneq)==8,'gate_cardinalities')
 for name in NAMES:
  need(a.numerical_ok([x for x in numeric if x.get('method')==name],False,name),'numeric:'+name)
  for lab,rr in [('canonical',eq),('execution',exec_eq)]:
   zz=[x for x in rr if x.get('method')==name];need(a.freeze_check(zz,a.EQ_BOUNDARIES,name),'bitwise:'+lab+name)
   for x in zz:
    need(x.get('scope')==x.get('accurate')=='0' and all(a.as_int(x.get(k))==SPEC[name][k] and a.as_int(x.get('execution_'+k))==SPEC[name][k] for k in ('kind','mode')),'checked_execution_contract')
    need(x.get('reference')==('canonical_full' if lab=='canonical' else name) and x.get('execution_name')==('trace_exec' if lab=='execution' and SPEC[name]['kind']>=4 else name),'checked_execution_identity')
 for name in ('trace_s_cached','trace_s_rep'):
  zz=[x for x in dyneq if x.get('method')==name];need(a.freeze_check(zz,a.EQ_BOUNDARIES,name) and all(x.get('reference')=='dyn_s_reference' for x in zz),'dynamic_shared_bitwise:'+name)
 for lab in ('PAIRPLAN_COUNTERS','PAIRPLAN_WORK_GATE','PAIRPLAN_PROFILE_GATE','TRACE_LOGICAL'):
  rr=t.get(lab,[]);need(len(rr)==16 and {(x.get('method'),x.get('layer')) for x in rr}=={(n,l) for n in NAMES for l in ('1','2')},'layer_matrix:'+lab)
  if lab!='PAIRPLAN_COUNTERS':need(all(x.get('pass')=='1' and (lab!='PAIRPLAN_PROFILE_GATE' or x.get('bitwise')=='1') for x in rr),'layer_gate:'+lab)
 for x in t.get('PAIRPLAN_COUNTERS',[]):
  name=x['method'];m=SPEC[name];exact=name=='exact_s';hits=0 if m['kind']==3 else (c if exact else dyn);share=name in ('dyn_s','exact_s','trace_s_cached','trace_s_rep');representative=name in ('exact_s','trace_s_rep');comparisons=cmp if name in ('dyn_i','dyn_s') else 0
  desired=dict(source_visits=E,feature_adds=128*E,decode_rows=E-hits if share else E,csr_reads=E-hits if representative else E,shared_heads=hits,comparisons=comparisons,runtime_head_comparisons=comparisons,exact_mask_bytes_read=w['windows'] if exact else 0,exact_pair_flags_tested=meta['pair_flags'] if exact else 0,exact_pair_windows=cw if exact else 0,exact_pair_edges=c if exact else 0,exact_shared_edges=c if exact else 0,order_index_reads=meta['order_reads'] if m['kind']>=4 else 0,projection_scopes=w['scopes'],windows=w['windows'],amx_tdp=32*w['scopes'],partial_zero_bytes=0,packing_load_bytes=0,partial_load_bytes=w['loads'],partial_store_bytes=w['stores'],packed_bytes=w['packed'],direct_rows=w['direct'],prefetch_requests=w['prefetch'],prefetch_index_reads=w['prefetch']//4,relu_elements=128*iw['epilogue_rows'] if x.get('layer')=='1' else 0)
  bad={k:{'expected':v,'actual':a.as_int(x.get(k))} for k,v in desired.items() if a.as_int(x.get(k))!=v};need(not bad,'independent_CSR_work:'+name+':'+x['layer']+':'+str(bad))
  need(x.get('scope')==x.get('accurate')=='0' and x.get('profile_path')=='production_BF16_epilogue' and x.get('unit')=='source_level_not_DRAM','counter_semantics')
 for x in t.get('TRACE_LOGICAL',[]):
  for k,pk in [('headers','headers'),('payload_bytes','payload_bytes'),('events','events'),('take1','take1'),('take2','take2'),('take3','take3'),('tile_offset_reads','nonempty_tiles')]:
   v=int(plan[pk]) if SPEC[x['method']]['kind']==5 else 0;need(a.as_int(x.get(k))==a.as_int(x.get('expected_'+k))==v,'trace_logical:'+x['method']+':'+k)
  need(x.get('unit')=='source_level_not_retired','trace_logical_semantics')
 immutable=t.get('TRACE_IMMUTABLE',[]);need(len(immutable)==2 and {x.get('phase') for x in immutable}=={'before','after'},'immutable_matrix')
 for x in immutable:need(x.get('hash_checked')==x.get('pass')=='1' and x.get('base_hash')==x.get('expected_base_hash')==st.get('fnv64') and x.get('trace_hash')==x.get('expected_trace_hash')==plan.get('payload_hash'),'immutable_content_hash')
 slots=t.get('TRACE_SLOT',[]);need(len(slots)==(8 if smoke else 120)==int(complete.get('slot_checks',-1)),'slot_count');need([a.as_int(x.get('serial')) for x in slots]==list(range(len(slots))),'slot_serial')
 fixed=[k for k in slots[0] if (k.endswith('_ptr') and k not in ('execution_method_ptr','schedule_ptr')) or k.endswith(('_size','_capacity'))] if slots else []
 for x in slots:
  name=x['method'];m=SPEC[name];need(all(a.as_int(x.get(k))==a.as_int(x.get('execution_'+k))==m[k] for k in ('kind','mode')) and x.get('execution_name')==('trace_exec' if m['kind']>=4 else name),'slot_execution')
  need(x.get('stable')==x.get('pass')=='1' and x.get('hash_checked')==x.get('copy_in_compute')=='0' and all(x.get(k)==slots[0].get(k) for k in fixed),'slot_storage')
  need(x.get('schedule')==('qguard' if m['schedule']==4 else 'degree'),'slot_schedule')
 for name in NAMES:
  rr=[x for x in slots if x['method']==name];need(len({x.get('execution_method_ptr') for x in rr})==len({x.get('schedule_ptr') for x in rr})==1,'fixed_method_addresses:'+name)
  need({x.get('execution_method_ptr') for x in exec_eq if x.get('method')==name}=={x.get('execution_method_ptr') for x in rr},'checked_timed_object:'+name)
 need(len({x.get('execution_method_ptr') for x in slots if SPEC[x['method']]['kind']>=4})==1 and len({x.get('schedule_ptr') for x in slots if x['method']!='pack_epi_qguard'})==1,'shared_execution_object_and_degree_schedule')
 keys={(m,'correctness',0) for m in NAMES}
 if not smoke:keys|={(m,'consecutive',0) for m in NAMES}|{(m,'alternating',i) for m in NAMES for i in range(10)}|{(m,'pmu',i) for m in NAMES for i in range(3)}
 need({(x.get('method'),x.get('phase'),a.as_int(x.get('repeat'))) for x in slots}==keys,'switch_matrix')
 times=t.get('TRACE_TIME',[]);pmus=t.get('METHOD_PMU',[]);threads=t.get('METHOD_PMU_THREAD',[]);walls=t.get('TRACE_PMU_WALL',[]);pmua=[]
 if smoke:need(not(times or pmus or threads or walls),'smoke_no_performance')
 else:
  need(len(times)==120,'time_count')
  for kind,nr in KINDS.items():
   for name in NAMES:
    rr=[x for x in times if x.get('kind')==kind and x.get('method')==name];need(len(rr)==nr and {a.as_int(x.get('repeat')) for x in rr}==set(range(nr)),'time_repeat_matrix:'+kind+name)
    for x in rr:
     order=NAMES.index(name) if kind=='consecutive' or int(x['repeat'])%2==0 else 7-NAMES.index(name)
     need(a.as_float(x.get('ms')) is not None and float(x['ms'])>0 and a.as_int(x.get('order'))==order and x.get('warmups_before_series' if kind=='consecutive' else 'warmups_before_measure')=='1','time_contract')
  pk={(m,str(i)) for m in NAMES for i in range(3)}
  for lab,rr in [('PMU',pmus),('PMU_WALL',walls)]:need(len(rr)==24 and {(x.get('method'),x.get('repeat')) for x in rr}==pk,lab+'_matrix')
  need(len(threads)==768,'PMU_thread_count');pmua=pmu_audit(t,need)
 counts={k:len(t.get(label,[])) for k,label in {'numerical':'METHOD_CHECK','canonical_bitwise':'PAIRPLAN_EQUIV','execution_bitwise':'TRACE_EXEC_EQUIV','dyns_bitwise':'TRACE_DYNS_EQUIV','work':'PAIRPLAN_WORK_GATE','profile':'PAIRPLAN_PROFILE_GATE','trace_logical':'TRACE_LOGICAL','trace_hist':'TRACE_PLAN_HIST','smoke_dumps':'TRACE_SMOKE_DUMP','reject_cases':'TRACE_REJECT_CASE','immutable':'TRACE_IMMUTABLE','opportunity':'TRACE_OPPORTUNITY','slots':'TRACE_SLOT','timings':'TRACE_TIME','pmu_regions':'METHOD_PMU','pmu_threads':'METHOD_PMU_THREAD'}.items()}
 return dict(**a.context(p),valid=not errs,errors=errs,warnings=warnings,raw_records=t,independent_CSR_work=iw,independent_smoke_replay=replay,process_status=proc,pmu_audit=pmua,counts=counts)

def summaries(audits):
 rows=[];ratios=[];pmu=[];setup=[];structures=[]
 for x in audits:
  if not x.get('valid') or x['data_class']=='smoke':continue
  ctx={k:x[k] for k in ('job','node','partition','exclusive','graph','data_class','input_sha256','build_fingerprint')};t=x['raw_records'];tt=t['TRACE_TIME']
  for kind,nr in KINDS.items():
   lookup={}
   for name in NAMES:
    rr=sorted([z for z in tt if z['kind']==kind and z['method']==name],key=lambda z:int(z['repeat']));v=[float(z['ms']) for z in rr]
    row=dict(**ctx,method=name,protocol=kind,kind=SPEC[name]['kind'],mode=SPEC[name]['mode'],median_ms=statistics.median(v),min_ms=min(v),max_ms=max(v),spread_ratio=max(v)/min(v),repeats=nr,raw_ms=json.dumps(v));rows.append(row);lookup[name]=row
   def ratio(ref,cand,effect):
    r,c=lookup[ref],lookup[cand]
    ratios.append(dict(**ctx,protocol=kind,effect=effect,reference=ref,method=cand,speedup=r['median_ms']/c['median_ms'],reference_median_ms=r['median_ms'],method_median_ms=c['median_ms'],observed_range_lower=r['min_ms']/c['max_ms'],observed_range_upper=r['max_ms']/c['min_ms'],range_is_confidence_interval=False))
   for ref,cand,effect in [('dyn_i','trace_i','compiled_control_independent_decode'),('dyn_s','trace_s_cached','compiled_control_cached_shared_decode'),('trace_s_cached','trace_s_rep','representative_current_CSR_read'),('dyn_i','dyn_s','dynamic_decode_sharing'),('trace_i','trace_s_cached','compiled_decode_sharing'),('trace_i','trace_s_rep','compiled_sharing_and_representative_read'),('dyn_s','exact_s','exact_mask_vs_dynamic_different_coverage'),('pack_epi_degree','pack_epi_qguard','qguard_schedule')]:ratio(ref,cand,effect)
   for baseline in NAMES[:2]:
    for cand in NAMES[2:]:ratio(baseline,cand,'vs_'+baseline)
  pp=t.get('METHOD_PMU',[])
  for name in NAMES:
   rr=[z for z in pp if z['method']==name];r=dict(**ctx,method=name,repeats=len(rr),all_usable=all(z['usable'] for z in x['pmu_audit'] if z['method']==name))
   for k in ('cycles_scaled','instructions_scaled','cache_misses_scaled','cache_references_scaled','amx_busy_scaled','region_wall_ms'):
    vv=[a.as_float(z.get(k)) for z in rr];r['median_'+k]=statistics.median(vv) if vv and None not in vv else None;r['raw_'+k]=json.dumps(vv)
   tr=[z for z in t.get('METHOD_PMU_THREAD',[]) if z['method']==name];scale=[float(z['time_enabled_ns'])/float(z['time_running_ns']) for z in tr if float(z['time_running_ns'])>0]
   r.update(thread_records=len(tr),max_scale=max(scale) if scale else None,min_scale=min(scale) if scale else None,multiplexed_thread_records=sum(float(z['time_enabled_ns'])!=float(z['time_running_ns']) for z in tr))
   pmu.append(r)
  for z in t.get('PAIRPLAN_SETUP',[]):setup.append(dict(**ctx,phase='base_plan',timing_semantics='actual_pipeline_excluded_primary',total_ms=float(z['total_ms']),plan_bytes=z.get('plan_bytes'),raw=json.dumps(z)))
  for z in t.get('TRACE_PLAN_SETUP',[]):setup.append(dict(**ctx,phase='trace_plan_including_validator',timing_semantics='actual_pipeline_excluded_primary',total_ms=float(z['total_ms']),build_ms=float(z['build_ms']),validate_ms=float(z['validate_ms']),plan_bytes=z.get('plan_bytes'),raw=json.dumps(z)))
  for z in t.get('TRACE_SCHEDULE_SETUP',[]):setup.append(dict(**ctx,phase='all_five_schedules',timing_semantics='actual_full_pipeline_not_isolated_qguard',total_ms=float(z['ms']),raw=json.dumps(z)))
  for z in t.get('TRACE_PLAN',[]):setup.append(dict(**ctx,phase='independent_runtime_audit',timing_semantics='correctness_validation_excluded_primary',total_ms=float(z['independent_audit_ms']),raw=json.dumps(z)))
  for z in t.get('TRACE_OPPORTUNITY',[]):
   setup.append(dict(**ctx,phase='opportunity_diagnostic',timing_semantics='optional_diagnostic_not_necessary_setup_or_breakeven',total_ms=float(z['diagnostic_ms']),raw=json.dumps(z)))
   structure=dict(**ctx,**{k:int(v) for k,v in z.items() if k not in ('graph','semantics','diagnostic_ms')});plan=t['TRACE_PLAN'][0];structure.update({k:int(plan[k]) for k in ('headers','payload_bytes','data_bytes','offset_bytes','plan_bytes','allocated_bytes','object_bytes')});E=x['independent_CSR_work']['E'];structure.update(shared_s64_fraction_of_E=int(z['shared_s64'])/E if E else 0,shared_full_fraction_of_E=int(z['shared_full'])/E if E else 0,payload_rounding_overhead_bytes=int(plan['payload_bytes'])-(int(plan['events'])+3)//4,additional_shared_fraction_of_E=(int(z['shared_full'])-int(z['shared_s64']))/E if E else 0,potential_decode_reduction_fraction=(int(z['shared_full'])-int(z['shared_s64']))/(E-int(z['shared_s64'])) if E-int(z['shared_s64']) else 0,sorted_shared_delta=int(z['sorted_shared_full'])-int(z['sorted_shared_s64']),unsorted_shared_delta=int(z['unsorted_shared_full'])-int(z['unsorted_shared_s64']),full_row_pair_optimality_scope='sorted_pairs_only',event_model='at_most_one_edge_per_row_per_event',unsorted_full_greedy_may_lose=True,opportunity_predicts_time=False);structures.append(structure)
 return rows,ratios,pmu,setup,structures

def expansion_gate(rows,all_gates):
 cohorts=[];candidate='trace_s_rep'
 for baseline in NAMES[:2]:
  for partition,exclusive in [('intel','false'),('intel_expr','true')]:
   for protocol in KINDS:
    zz=[x for x in rows if x['partition']==partition and x['exclusive']==exclusive and x['protocol']==protocol and x['graph'] in a.DIAG4];rr=[]
    for graph in a.DIAG4:
     cr=[x for x in zz if x['graph']==graph and x['method']==candidate];br=[x for x in zz if x['graph']==graph and x['method']==baseline]
     if len(cr)==len(br)==1 and cr[0]['job']==br[0]['job']:rr.append(dict(graph=graph,job=cr[0]['job'],speedup=br[0]['median_ms']/cr[0]['median_ms']))
    gm=a.gm(x['speedup'] for x in rr) if len(rr)==4 else None;wins=sum(x['speedup']>1 for x in rr)
    cohorts.append(dict(baseline=baseline,partition=partition,exclusive=exclusive,protocol=protocol,graphs=len(rr),geometric_mean=gm,wins=wins,required_geometric_mean=1.03,required_wins=3,pass_filter=gm is not None and gm>=1.03 and wins>=3,ratios=rr))
 return dict(candidate=candidate,all_other_gates_pass=all_gates,cohorts=cohorts,eligible_to_expand_full17=all_gates and len(cohorts)==8 and all(x['pass_filter'] for x in cohorts),rule='Fixed trace_s_rep must beat BOTH PACKdegree and PACKqguard in EACH protocol x partition cohort: GM>=1.03 and wins>=3/4. No candidate substitution.',scope='Predeclared practical expansion filter, not a statistical significance guarantee.')

def main():
 if len(sys.argv)!=3:raise SystemExit('usage analyze_trace.py RUN NEW_OUTPUT_DIR')
 run=pathlib.Path(sys.argv[1]).resolve();out=pathlib.Path(sys.argv[2]).resolve()
 if not str(out).startswith(str(BASE.resolve())+'/results/') or out.exists():raise ValueError('New results directory required')
 build=build_audit(run);packets,jobs,slurm=a.load_packets(run,build);audits=[]
 for p in packets:
  try:audits.append(packet_audit(p))
  except Exception as e:audits.append(dict(**a.context(p),valid=False,errors=['audit_exception:'+repr(e)],raw_records=p['parsed']))
 audit_job_counts(jobs,audits);epilogue=a.epilogue_counter_regressions();errs=[]
 if len(jobs)!=7 or not all(j['valid'] for j in jobs):errs.append('expected_seven_completed_valid_jobs')
 if len(audits)!=27 or sum(x['data_class']=='smoke' for x in audits)!=13:errs.append('expected_27_packets_13_smoke')
 perf=[x for x in audits if x['data_class']!='smoke'];expected={(p,e,g) for p,e in [('intel','false'),('intel_expr','true')] for g in a.SYN3+a.DIAG4}
 if len(perf)!=14 or {(x['partition'],x['exclusive'],x['graph']) for x in perf}!=expected:errs.append('expected_3synthetic_4diagnostic_each_allocation_class')
 rows,ratios,pmu,setup,structures=summaries(audits);totals=collections.Counter()
 for x in audits:totals.update(x.get('counts',{}))
 target=dict(numerical=864,canonical_bitwise=864,execution_bitwise=864,dyns_bitwise=216,work=432,profile=432,trace_logical=432,trace_hist=3483,smoke_dumps=13,reject_cases=26,immutable=54,opportunity=27,slots=1784,timings=1680,pmu_regions=336,pmu_threads=10752)
 for k,v in target.items():
  if totals[k]!=v:errs.append('count_'+k+'_expected_'+str(v)+'_got_'+str(totals[k]))
 if len(rows)!=224 or len(ratios)!=560 or len(structures)!=14:errs.append('expected_224_summary_560_ratios_14_structures')
 if not all(x['pass_case'] for x in epilogue):errs.append('epilogue_formula_regression')
 valid=build['valid'] and not errs and all(x['valid'] for x in audits);gate=expansion_gate(rows,valid)
 report=dict(analysis_pass=valid,errors=errs,build=build,jobs=jobs,packets=audits,counts=dict(totals),slurm=slurm,helper=dict(path=str(HELPER),sha256=a.hashed(HELPER)),source_script_sha256=a.digest(pathlib.Path(__file__)),input_hashes=a.INPUT_HASHES,expansion_gate=gate,epilogue_counter_regressions=epilogue,interpretation_limits=['Ratios use same graph, job, protocol and build; every raw timing is retained. Sample extrema are descriptive, not confidence intervals.','Trace records reproduce the old greedy schedule, including unsorted/duplicate CSR; this is not an SCS algorithm.','Source-level counters do not establish retired load instructions or DRAM traffic. Current CSR reads exclude unchanged next-index prefetch reads.','Trace payload rounding is summed per record, including zero-count headers and odd rows. sizeof(TracePlan) is separate from vector payload and capacity.','All methods retain both immutable plans; warmed incremental comparison does not represent baseline standalone memory footprint.','Opportunity statistics are optional diagnostics and excluded from necessary preprocessing/breakeven. Full-row greedy may lose sharing for unsorted pairs; sorted optimality assumes at most one edge per row per event and does not include same-row run reuse.','Generic cache events and AMX busy cannot establish a unique memory bottleneck or AVX-AMX overlap; PMU wall time is not primary end-to-end timing.','Only four original diagnostic benchmark graphs; Mycielskian19 is constructed and legacy real label is a cohort.','No NUMA interleave or feature first-touch changes; dimensions128 throughout.'])
 out.mkdir(parents=True);(out/'audit.json').write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n');(out/'expansion_gate.json').write_text(json.dumps(gate,ensure_ascii=False,indent=2)+'\n')
 for name,rr,fields in [('summary',rows,['job','node','partition','exclusive','graph','method','protocol','median_ms']),('ratios',ratios,['job','graph','protocol','effect','reference','method','speedup']),('pmu_summary',pmu,['job','graph','method','all_usable']),('setup',setup,['job','graph','phase','total_ms']),('structure',structures,['job','graph','shared_s64','shared_full'])]:a.write_csv(out/(name+'.csv'),rr,fields)
 print(json.dumps(dict(analysis_pass=valid,errors=errs,invalid_packets=[dict(graph=x['graph'],job=x['job'],errors=x['errors']) for x in audits if not x['valid']],counts=dict(totals),summary_rows=len(rows),pmu_usable=sum(y['usable'] for x in audits for y in x.get('pmu_audit',[])),expand_full17=gate['eligible_to_expand_full17'],out=str(out)),ensure_ascii=False))
 return 0 if valid else 2
if __name__=='__main__':sys.exit(main())


