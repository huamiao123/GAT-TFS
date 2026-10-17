#!/usr/bin/env python3
"""Create a separate diagnostic-only fork; never overwrite main frozen build."""
import hashlib
import json
import pathlib
import shutil
import sys

parent,out=map(pathlib.Path,sys.argv[1:3])
assert not out.exists()
shutil.copytree(parent,out)
changed={}


def change(name,s):
    p=out/name
    old=p.read_bytes()
    p.write_text(s)
    changed[name]=dict(before=hashlib.sha256(old).hexdigest(),after=hashlib.sha256(p.read_bytes()).hexdigest())


p=out/'pair_reducer.hpp';s=p.read_text()
old='''            for(int f = 0; f < 8; ++f) val[f] = pair_decode16(src + f * 16);
            // This coarse interval includes the loads and conversion together;
            // do not claim separately measured load/decode sub-intervals.
'''
new='''            __m256i raw[8];
            double d = tick<P>(sample);
            for(int f=0;f<8;f++)raw[f]=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(src+f*16));
            gcn_extra_projection::detail_tock<P>(pr,gcn_extra_projection::SOURCE_BF16_LOAD,d,sample);
            d=tick<P>(sample);
            for(int f=0;f<8;f++)val[f]=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(raw[f]),16));
            gcn_extra_projection::detail_tock<P>(pr,gcn_extra_projection::SOURCE_BF16_DECODE,d,sample);
            // Diagnostic sampled branch only: split load/decode, original add below.
'''
assert s.count(old)==1
change('pair_reducer.hpp',s.replace(old,new))
p=out/'state_kernels.hpp';s=p.read_text()
old='''                    for(int f = 0; f < 8; f++)
                        val[f] = _mm512_castsi512_ps(_mm512_slli_epi32(
                            _mm512_cvtepu16_epi32(_mm256_loadu_si256(
                                reinterpret_cast<const __m256i*>(src + f * 16))), 16));
'''
assert s.count(old)==1
new='''                    if constexpr(P) {
                        if(sample) {
                            __m256i raw[8];
                            double d=tick<P>(sample);
                            for(int f=0;f<8;f++)raw[f]=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(src+f*16));
                            detail_tock<P>(pr,SOURCE_BF16_LOAD,d,sample);
                            d=tick<P>(sample);
                            for(int f=0;f<8;f++)val[f]=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(raw[f]),16));
                            detail_tock<P>(pr,SOURCE_BF16_DECODE,d,sample);
                        } else {
'''+old+'''                        }
                    } else {
'''+old+'''                    }
'''
change('state_kernels.hpp',s.replace(old,new))
p=out/'ordered_runtime.hpp';s=p.read_text()
needle='    bool full=smoke'
assert s.count(needle)==1
s=s.replace(needle,'    bool diagnostic=getenv("EVENT_DIAGNOSTIC_ONLY")!=nullptr;\n'+needle)
# Only the performance sweep, independent stage sweeps and PMU are bypassed.
# Original numerical, equivalence, work and production-profile gates remain.
assert s.count('    if(!smoke){')==1
s=s.replace('    if(!smoke){','    if(!smoke&&!diagnostic){')
s=s.replace('    if(!smoke)for(auto& m:methods)for(int rep=0;rep<3;rep++) {',
            '    if(!smoke&&!diagnostic)for(auto& m:methods)for(int rep=0;rep<3;rep++) {')
s=s.replace('if(!smoke&&getenv("STATE_PMU")','if(!smoke&&!diagnostic&&getenv("STATE_PMU")')
change('ordered_runtime.hpp',s)
p=out/'ordered_suite.py';s=p.read_text()
assert s.count("if mode=='smoke_structure':")==1
s=s.replace("if mode=='smoke_structure':","if mode in ['smoke_structure','smoke_detail']:")
s=s.replace("else:graphs=[(inv[g],'performance') for g in mode.split(',')]",
            "elif mode=='detail':graphs=[(g,'diagnostic') for g in diagnosis]\nelse:graphs=[(inv[g],'performance') for g in mode.split(',')]")
needle=" os.environ['STATE_SMOKE']='1' if phase=='smoke' else '0'"
assert s.count(needle)==1
s=s.replace(needle," if phase=='diagnostic':os.environ['EVENT_DIAGNOSTIC_ONLY']='1'\n else:os.environ.pop('EVENT_DIAGNOSTIC_ONLY',None)\n"+needle)
change('ordered_suite.py',s)
(out/'detail_generation.json').write_text(json.dumps(dict(parent=str(parent),changed=changed,
    scope='separate diagnostic-only sampled load/decode/add split; no main performance samples from this build',
    retained='all numerical/equivalence/work/profile gates; original AMX/W/schedule/default NUMA'),indent=2)+'\n')
print('DETAIL_GENERATED',out)
