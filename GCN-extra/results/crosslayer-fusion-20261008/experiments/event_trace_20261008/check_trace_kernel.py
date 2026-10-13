#!/usr/bin/env python3
"""Read-only static/theoretical audit. No C++ compilation, AMX or real-graph scan."""
from pathlib import Path
import sys,json,hashlib,itertools,random
assert len(sys.argv)==2, 'pass the immutable source_snapshot directory'
snap=Path(sys.argv[1]).resolve()
old=(snap/'planned_kernel.hpp').read_text()
new=(snap/'trace_kernel.hpp').read_text()
new=new.replace('#include "trace_reducer.hpp"','#include "planned_reducer.hpp"')
new=new.replace('namespace gcn_extra_event_trace','namespace gcn_extra_pair_plan')
new=new.replace('const Method& method, const Plan& plan, const TracePlan& trace, Profile* result)','const Method& method, const Plan& plan, Profile* result)')
new=new.replace('if(method.window != 64 || method.scope != 0)','if(method.window != 64 || (method.scope != 0 && method.scope != 64 && method.scope != 256))')
new=new.replace('trace experiment requires S64 and FULL','state experiment requires S64 and M64/M256/FULL')
new=new.replace('    if(trace.tile_offsets.size() != plan.tiles.size() + 1)\n        throw std::invalid_argument("trace tile offset count mismatch");\n\n','')
new=new.replace('TraceReducer<MODE>::template run<P>','PlannedReducer<MODE>::template run<P>')
new=new.replace('method, plan.tiles[size_t(i / TR)], trace, size_t(i / TR), pr, sample);','method, plan.tiles[size_t(i / TR)], plan, pr, sample);')
new=new.replace('gcn_extra_pair_plan::add_plan_logical(result->','add_plan_logical(result->')
new=new.replace('            add_trace_logical(result->trace_logical, pr.trace_logical);\n','')
assert new==old, 'kernel orchestration differs beyond declared trace plumbing/guard'
reducer=(snap/'trace_reducer.hpp').read_text()
assert 'uint8_t codes=payload[block/4]' in reducer and 'codes&3u;codes>>=2' in reducer
assert 'pair_take(' not in reducer and 'runtime_head_comparisons' not in reducer
assert reducer.count('pair_prefetch_next<P>')==2
assert '++pr.trace_logical.tile_offset_reads' in reducer
assert 'volatile' not in reducer.replace('// forced loads','// forced loads')
plan=(snap/'trace_plan.hpp').read_text()
assert 'count>128' in plan and 'nonzero trailing payload bits' in plan
assert 'event differs from original greedy' in plan and 'histogram!=p.event_histogram' in plan
assert 'std::vector<uint32_t>' not in plan, 'plan must not store source IDs'

def builder(a,b):
    i=j=0;events=[]
    while i<len(a) or j<len(b):
        if i==len(a):t=2
        elif j==len(b):t=1
        elif a[i]<b[j]:t=1
        elif b[j]<a[i]:t=2
        else:t=3
        events.append(t);i+=bool(t&1);j+=bool(t&2)
    assert len(events)<=128
    result=bytearray([len(events)])+bytearray((len(events)+3)//4)
    for k,t in enumerate(events):result[1+k//4]|=t<<(2*(k%4))
    return result,events

def validate(a,b,record):
    if not record:return False
    n=record[0]
    if n>128 or len(record)!=1+(n+3)//4:return False
    i=j=0;seen_a=[];seen_b=[]
    for k in range(n):
        t=(record[1+k//4]>>(2*(k%4)))&3
        if not t or (i==len(a) and j==len(b)):return False
        if i<len(a) and j<len(b):expected=3 if a[i]==b[j] else (1 if a[i]<b[j] else 2)
        else:expected=1 if i<len(a) else 2
        if t!=expected:return False
        if t&1:seen_a.append(a[i]);i+=1
        if t&2:seen_b.append(b[j]);j+=1
    if n%4 and record[-1]>>(2*(n%4)):return False
    return i==len(a) and j==len(b) and seen_a==a and seen_b==b

seq=[list(x) for n in range(4) for x in itertools.product(range(3),repeat=n)]
cases=mutations=0;hist=[0]*129
pairs=list(itertools.product(seq,seq))
rng=random.Random(20261008)
for n in (0,1,3,4,5,7,8,63,64,65,127,128,129,257):
    a=[rng.randrange(11) for _ in range(n)]
    for m in (0,1,7,63,64,65,128,257):
        b=[rng.randrange(11) for _ in range(m)]
        for ws in range(0,max(n,m),64):pairs.append((a[ws:ws+64],b[ws:ws+64]))
for a,b in pairs:
    encoded,events=builder(a,b);assert validate(a,b,encoded)
    shared=events.count(3);assert len(events)==len(a)+len(b)-shared
    assert sum(t in (1,3) for t in events)==len(a)
    assert sum(t in (2,3) for t in events)==len(b)
    hist[len(events)]+=1;cases+=1
    if events:
        x=encoded.copy();x[1]&=~3;assert not validate(a,b,x);mutations+=1
        x=encoded.copy();x[0]=129;assert not validate(a,b,x);mutations+=1
        x=encoded[:-1];assert not validate(a,b,x);mutations+=1
        if len(events)%4:
            x=encoded.copy();x[-1]|=3<<(2*(len(events)%4));assert not validate(a,b,x);mutations+=1
# Endpoint count128 is representable by uint8_t without wrapping to zero.
x,e=builder([0]*64,[1]*64);assert x[0]==128 and len(x)==33 and validate([0]*64,[1]*64,x)
print(json.dumps(dict(status='PASS',kernel_orchestration_identity=True,theoretical_record_cases=cases,malformed_record_model_rejections=mutations,header128_checked=True,cpp_full_validator_and_runtime_bitwise_gates_still_required=True,source_sha256={n:hashlib.sha256((snap/n).read_bytes()).hexdigest() for n in ('trace_plan.hpp','trace_reducer.hpp','trace_kernel.hpp')}),sort_keys=True))
