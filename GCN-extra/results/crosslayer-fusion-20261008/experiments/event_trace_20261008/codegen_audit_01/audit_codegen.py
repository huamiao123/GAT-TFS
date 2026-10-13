#!/usr/bin/env python3
"""Read frozen objdump text only; write audit artifacts only beside this script.
No compilation, binary execution, scheduler submission, or source modification.
Address proofs are deliberately bound to the build01 binary hash.
"""
import re, json, hashlib, collections, difflib
from pathlib import Path
ROOT=Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
RUN=ROOT/'runs/event-trace-20261008-01'
OLD=ROOT/'runs/access-prefetch-20261008-01'
OUT=Path(__file__).resolve().parent
assert OUT == (ROOT/'experiments/event_trace_20261008/codegen_audit_01').resolve()
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
assert sha(RUN/'build/trace_methods')=='a59e61fea119331082150f926c7c337236b9f25160311b7caa4e7d2888d483e2', 'audit addresses tied to build01'
def parse(p):
 t=p.read_text(); ms=list(re.finditer(r'^([0-9a-f]+) <(.*)>:$',t,re.M)); out={}
 for i,m in enumerate(ms):
  body=t[m.start():ms[i+1].start() if i+1<len(ms) else len(t)]
  ins=[]
  for line in body.splitlines()[1:]:
   z=line.split('\t')
   if len(z)>=3 and z[2].strip():
    ins.append((int(z[0].strip().rstrip(':'),16),z[2].strip()))
  out[m.group(2)]={'start':int(m.group(1),16),'end':int(ms[i+1].group(1),16) if i+1<len(ms) else 1<<64,'ins':ins,'body':body}
 return out
new=parse(RUN/'build/disassembly.txt');old=parse(OLD/'build/disassembly.txt')
def select(fs,fragment):
 got=[(k,v) for k,v in fs.items() if fragment in k]
 assert len(got)==1,(fragment,len(got))
 return got[0]
def normalize(f):
 result=[]
 for a,s in f['ins']:
  s=s.split('#')[0].strip()
  m=re.match(r'((?:j\w+|call\w*|loop\w*)\s+)([0-9a-f]+)\s+<(.*)>$',s)
  if m:
   target=int(m[2],16)
   s=m[1]+('LOCAL+'+hex(target-f['start']) if f['start']<=target<f['end'] else 'SYMBOL:'+m[3])
  s=re.sub(r'-?0x[0-9a-f]+\(%rip\)','RIP_DISP(%rip)',s)
  result.append(s)
 return result
fragments={
 'trace_i':'gcn_extra_event_trace::trace_pair_window<false, 0>',
 'trace_s_cached':'gcn_extra_event_trace::trace_pair_window<false, 1>',
 'trace_s_rep':'gcn_extra_event_trace::trace_pair_window<false, 2>',
 'dyn_i':'gcn_extra_pair_plan::dynamic_pair_window<false, false>',
 'dyn_s':'gcn_extra_pair_plan::dynamic_pair_window<false, true>',
}
functions={};fdata={};extracts={};checks=[]
for label,fragment in fragments.items():
 name,f=select(new,fragment);fdata[label]=f
 extract=OUT/(label+'.asm');extract.write_text(f['body']);extracts[extract.name]=sha(extract)
 stack=[{'address':hex(a),'instruction':s} for a,s in f['ins'] if re.search(r'\([^)]*%(?:rsp|rbp)[^)]*\)',s)]
 vector_stack=[x for x in stack if re.search(r'%(?:xmm|ymm|zmm)\d+',x['instruction'])]
 direct_calls=[]
 for cname,cf in new.items():
  for a,s in cf['ins']:
   if s.startswith('call') and '<'+name+'>' in s:
    direct_calls.append({'caller':cname,'address':hex(a),'bf16_output':('kernel<false, true,' in cname)})
 functions[label]={'symbol':name,'entry':hex(f['start']),'instruction_count_static':len(f['ins']),
  'mnemonic_counts_static':dict(collections.Counter(s.split()[0] for _,s in f['ins'])),
  'direct_stack_memory_instruction_count':len(stack),'direct_vector_stack_memory_instructions':vector_stack,
  'direct_callers':direct_calls,'extract':str(extract),'extract_sha256':sha(extract)}
 checks.append(('production_bf16_caller_'+label,any(x['bf16_output'] for x in direct_calls)))
 checks.append(('no_direct_vector_stack_memory_'+label,not vector_stack))
identities={}
for label in ('dyn_i','dyn_s'):
 name,of=select(old,fragments[label]); nf=fdata[label]
 a,b=normalize(of),normalize(nf)
 extract=OUT/(label+'_previous.asm');extract.write_text(of['body']);extracts[extract.name]=sha(extract)
 digest=lambda lines: hashlib.sha256(('\n'.join(lines)+'\n').encode()).hexdigest()
 identities[label]={'previous_entry':hex(of['start']),'current_entry':hex(nf['start']),
  'previous_instruction_count':len(a),'current_instruction_count':len(b),'equal':a==b,
  'previous_normalized_sha256':digest(a),'current_normalized_sha256':digest(b),
  'difference':list(difflib.unified_diff(a,b))[:80]}
 checks.append(('old_reducer_identity_'+label,a==b))
# Address ranges manually followed through the take=3 control-flow paths.
proofs={
 'trace_i':{'ranges':[(0x463657,0x4636f8),(0x46370f,0x4637b0)],'expected':(16,16,16),
  'control':'take bit0 at 0x46361c selects row0; take bit1 test at 0x4636fe falls through to second decode at 0x463708 for mask11; no equality/CSE bypass.'},
 'trace_s_cached':{'ranges':[(0x464184,0x464225),(0x4642ae,0x4642d8)],'expected':(8,8,16),
  'control':'cmp3 at 0x464174 selects shared block; jump 0x46422b -> 0x4642ae reuses row1-add suffix, skipping row1 decode.'},
 'trace_s_rep':{'ranges':[(0x464c20,0x464cc1),(0x464afe,0x464b28)],'expected':(8,8,16),
  'control':'cmp3 at 0x464c10 selects shared block; jump 0x464cc7 -> 0x464afe reuses row1-add suffix, skipping row1 decode.'},
 'dyn_i':{'ranges':[(0x468027,0x4680c8),(0x4680df,0x468180)],'expected':(16,16,16),
  'control':'take3 falls from row0 through bit1 test 0x4680ce into second independent row decode at 0x4680d8.'},
 'dyn_s':{'ranges':[(0x468aa0,0x468b71)],'expected':(8,8,16),
  'control':'cmp3 at 0x468a90 falls through shared decode block; each decoded zmm value then feeds both row accumulators.'},
}
for label,p in proofs.items():
 ins=[(a,s) for lo,hi in p['ranges'] for a,s in fdata[label]['ins'] if lo<=a<=hi]
 counts=collections.Counter(s.split()[0] for _,s in ins)
 actual=tuple(counts[x] for x in ('vpmovzxwd','vpslld','vaddps'))
 checks.append(('take3_decode_shift_add_'+label,actual==p['expected']))
 p['ranges']=[[hex(a),hex(b)] for a,b in p['ranges']]
 p['observed_vpmovzxwd_vpslld_vaddps']=list(actual);p['expected']=list(p['expected'])
 p['static_instruction_evidence']=[{'address':hex(a),'instruction':s} for a,s in ins]
# Exact sites that separate current-index demand loads from PF1 index loads.
sites={
 'trace_i':{'current_head_update':[0x4637d4,0x4637fe],'prefetch_next_index':[0x4635ba,0x4635f6],'packed_payload_byte':[0x46357a],'mask_shift':[0x463593]},
 'trace_s_cached':{'current_head_update':[0x4642f8,0x464326],'prefetch_next_index':[0x464038,0x464153],'packed_payload_byte':[0x464007],'mask_shift':[0x464010]},
 'trace_s_rep':{'mutually_exclusive_current_head':[0x464b5d,0x464b83],'prefetch_next_index':[0x464b90,0x464be1],'packed_payload_byte':[0x464a76],'mask_shift':[0x464b2e]},
 'dyn_i':{'head_comparison':[0x467f16,0x467f31],'current_head_update':[0x4681a9,0x4681ce],'prefetch_next_index':[0x467f80,0x467fd1]},
 'dyn_s':{'head_comparison':[0x4689c4,0x4689dd],'current_head_update':[0x468d19,0x468d3d],'prefetch_next_index':[0x468a1c,0x468a66]},
}
for label,groups in sites.items():
 by_addr=dict(fdata[label]['ins'])
 for kind,addresses in list(groups.items()):
  entries=[]
  for addr in addresses:
   assert addr in by_addr,(label,kind,hex(addr))
   inst=by_addr[addr]
   prefix='cmp' if kind=='head_comparison' else 'shr' if kind=='mask_shift' else 'mov'
   checks.append(('site_'+label+'_'+kind+'_'+hex(addr),inst.startswith(prefix)))
   entries.append({'address':hex(addr),'instruction':inst})
  groups[kind]=entries
inputs=[RUN/'build/trace_methods',RUN/'build/disassembly.txt',RUN/'build/kernel_static_audit.json',RUN/'build/commands.txt',RUN/'source.sha256',RUN/'manifest.json',OLD/'build/disassembly.txt']
inputs += [RUN/'source_snapshot'/x for x in ('trace_plan.hpp','trace_reducer.hpp','trace_kernel.hpp','planned_reducer.hpp','planned_kernel.hpp')]
report={
 'schema':'event_trace_codegen_audit_v1','status':'PASS' if all(v for _,v in checks) else 'FAIL',
 'scope':'Static control-flow and code-generation evidence from frozen build01 P=false pair-window functions; production BF16-output callers verified. No binaries/jobs executed.',
 'run':str(RUN),'previous_run':str(OLD),'inputs_sha256':{str(p):sha(p) for p in inputs},
 'script_sha256':sha(Path(__file__)),'extracts_sha256':extracts,
 'checks':[{'name':k,'pass':v} for k,v in checks],
 'normalization':'Ignore instruction addresses/bytes and objdump comments; internal direct control-flow target -> relative offset; external target -> full symbol; RIP displacement -> placeholder. Preserve instruction order, registers, immediate values and stack offsets. This is reducer instruction identity, not whole-binary timing identity.',
 'old_dynamic_reducer_identity':identities,'functions':functions,'take3_paths':proofs,'load_and_control_sites':sites,
 'conclusions':[
  'trace_i and dyn_i retain two independently decoded BF16 feature vectors on a shared (mask11) event: 16 vpmovzxwd + 16 vpslld + 16 vaddps. No CSE collapsed the intended independent-decode control.',
  'trace_s_cached, trace_s_rep and dyn_s shared events decode once: 8 vpmovzxwd + 8 vpslld + 16 vaddps. Both destination additions remain; single-destination events use 8/8/8.',
  'Cached trace modes retain separate per-row next-current-head loads when consumed rows remain nonempty. Representative mode chooses one of two current-head load sites at event entry; take11 chooses row0. PF1 next-index loads and four prefetch hints per eligible consumed row remain separate in all treatments.',
  'Trace hot loops read packed event bytes and shift masks. Dynamic head-value comparison instructions are replaced by trace decoding and mask control, not by a free oracle.',
  'Each examined pair reducer has scalar stack traffic, but no direct rsp/rbp-based memory instruction involving an xmm/ymm/zmm register. This narrow screen supports no obvious direct vector-accumulator spill, not absence of all spills.',
  'dyn_i/dyn_s pair-window instruction streams match the prior access-prefetch build after address normalization.'
 ],
 'source_vs_machine_limits':[
  'Let E be row-edge consumptions, S be mask11 events, T=E-S. Source feature-decode requests are E for independent and T for shared modes; additions are 128E. The inspected branch paths support the corresponding vector instructions, but software Profile counters are not retired-instruction PMU measurements.',
  'Source current-index requests are E in cached modes and T in representative mode. PF1 index loads, row bases, degree/plan headers, loop control, compiler stack traffic and speculative execution are additional. Neither 4S fewer source index bytes nor 256S fewer source feature bytes means equal DRAM-byte savings.',
  'Histogram-counted payload bytes sum ceil(record_events/4), with one count header per pair-window and one tile-offset read per nonempty tile. The assembly confirms packed-byte outer loops; it does not measure memory hierarchy traffic.',
  'Static mnemonic totals include mutually exclusive paths and epilogues. In particular 24 static decodes in a shared reducer do not mean 24 decodes per event.',
  'No speedup, bottleneck, AMX/AVX overlap, bandwidth saturation, branch-miss rate, cache hit rate, or absence of indirect stack aliasing is established by this audit. Benchmark and hardware evidence are separate.',
  'Baseline equality here covers the two dynamic pair reducers only; no claim of whole-application byte identity or identical placement is made.'
 ]
}
(OUT/'audit.json').write_text(json.dumps(report,indent=2,sort_keys=True)+'\n')
text='Event trace build01 assembly audit\n\nStatus: '+report['status']+'\n\n'+'\n\n'.join(report['conclusions'])+'\n\nLimits\n\n'+'\n\n'.join(report['source_vs_machine_limits'])+'\n'
(OUT/'README.txt').write_text(text)
print(json.dumps({'status':report['status'],'audit_json':str(OUT/'audit.json'),'audit_sha256':sha(OUT/'audit.json'),'script_sha256':sha(Path(__file__)),'checks':len(checks),'failed':[k for k,v in checks if not v]},sort_keys=True))
raise SystemExit(0 if report['status']=='PASS' else 1)
