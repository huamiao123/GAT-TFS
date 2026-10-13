#!/usr/bin/env python3
"""Read-only source and independent target-model audit; does not compile/run AMX."""
from pathlib import Path
import hashlib,json,re
ROOT=Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
HERE=Path(__file__).resolve().parent
OLD=ROOT/'runs/order-share-factorial-20261007-01/source_snapshot'
a=(OLD/'planned_kernel.hpp').read_text()
b=(HERE/'pf_kernel.hpp').read_text()
b=b.replace('#include "pf_reducer.hpp"','#include "planned_reducer.hpp"')
b=b.replace('gcn_extra_prefetch','gcn_extra_pair_plan')
b=b.replace('template<bool P, bool B16, int MODE, int PF>','template<bool P, bool B16, int MODE>')
b=b.replace('PlannedReducer<MODE, PF>','PlannedReducer<MODE>')
b=b.replace('gcn_extra_pair_plan::add_plan_logical(result->','add_plan_logical(result->')
assert a == b, 'kernel orchestration differs beyond isolated dispatch'
a=(OLD/'planned_reducer.hpp').read_text()
b=(HERE/'pf_reducer.hpp').read_text()
def body(text,marker):
    pos=text.index(marker)
    start=text.index('{',pos)
    depth=1; i=start+1
    while depth:
        if text[i]=='{':depth+=1
        elif text[i]=='}':depth-=1
        i+=1
    return text[start:i]
def normalize(text):
    text=re.sub(r'^    pf_prologue<P, PF>\(.*?\);\n','',text,flags=re.M)
    return text.replace('pf_count_row_window','count_row_window').replace('pf_steady<P, PF>','pair_prefetch_next<P>')
for old,new in [('inline void scalar_row_window','inline void pf_scalar_row_window'),('inline void exact_pair_window','inline void pf_exact_pair_window')]:
    assert body(a,old)==normalize(body(b,new)), 'nonprefetch row arithmetic differs: '+old
assert 'gcn_extra_state::pair_prefetch_next<P>' in b
assert 'if(first && ws < row_scope_end)' in b
assert 'const uint32_t count = std::min(7u, row_scope_end - ws - 1)' in b
assert 'pf_visit_prologue_targets<PF>(ws, row_scope_end, first,' in b
assert 'pf_visit_steady_targets<PF>(k, row_scope_end,' in b
# Independent position model checks complete ordered targets, not only count.
# Runtime additionally calls the C++ PF8 visitor used by the production emitter.
cases=0
for scope_start in (0,64,128,256,4294967200):
    for length in range(0,321):
        end=min(4294967295,scope_start+length)
        for row_end in (max(0,scope_start-1),scope_start,end):
            targets={0:[],1:[],8:[]}
            ws=scope_start
            while ws<end:
                we=min(ws+64,end)
                if ws==scope_start and ws<row_end:
                    targets[8].extend(range(ws+1,min(ws+8,row_end)))
                for k in range(ws,min(we,row_end)):
                    if k+1<row_end: targets[1].append(k+1)
                    if row_end-k>8: targets[8].append(k+8)
                ws=we
            expected=list(range(scope_start+1,row_end)) if scope_start<row_end else []
            assert targets[0]==[] and targets[1]==targets[8]==expected
            assert all(scope_start<t<row_end for t in targets[8])
            cases+=1
print(json.dumps({'status':'PASS','kernel_orchestration_identity':True,'scalar_nonprefetch_body_identity':True,'exact_nonprefetch_body_identity':True,'independent_target_model_cases':cases,'scope_starts':[0,64,128,256,4294967200],'scope_lengths':[0,320],'cpp_production_visitor_runtime_gate_required':True,'source_sha256':{n:hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in ['pf_reducer.hpp','pf_kernel.hpp'] }},sort_keys=True))
