#!/usr/bin/env python3
"""Derive an isolated exact mixed-PH benchmark from the preload harness."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
text=(root/'tfs_online'/'preload_main.cpp').read_text(encoding='utf-8')
def one(a,b):
    global text
    assert text.count(a)==1,(a,text.count(a))
    text=text.replace(a,b)
one('#include "icpp_heads_preload.hpp"','#include "icpp_heads_preload.hpp"\n#include "icpp_heads_hybrid.hpp"\n#include "hybrid_checks.hpp"')
one('namespace ipp=gat::icpp_heads_preload;','namespace ipp=gat::icpp_heads_preload;\nnamespace ihy=gat::icpp_heads_hybrid;')
one('struct Path {std::string name;int kind,block;std::string backend;};',
    'struct Path {std::string name;int kind,block;std::string backend;int threshold=0;};')
one('ipp::run_preload_smoke();return 0;','ipp::run_preload_smoke();ihy::run_hybrid_smoke();return 0;')
one('{"HEADS_AMX_PRELOAD_B32",6,32,"amx_hi_lo"}};',
    '{"HEADS_AMX_PRELOAD_B32",6,32,"amx_hi_lo"},{"HYBRID_T16_B32",7,32,"",16},{"HYBRID_T24_B32",7,32,"",24},{"HYBRID_T32_B32",7,32,"",32}};')
one('if(path.kind==5||path.kind==6)return heads[l].fallback.original.base.out;',
    'if(path.kind>=5)return heads[l].fallback.original.base.out;')
one('if(path.kind==6){ipp::Stats s;ipp::layer',
    'if(path.kind==7){ihy::Stats s;ihy::layer(g,model[l],prep[l],schedule,*x,heads[l],l<2,path.block,64,ot[l],s,0,false,"fp32",path.threshold);}\n        else if(path.kind==6){ipp::Stats s;ipp::layer')
one('for(const auto& path:paths)if(path.kind==5||path.kind==6){ih::Timing t;ih::Stats st;if(path.kind==6)ipp::layer(g,model[1],prep[1],schedule,master[0],heads[1],false,path.block,64,t,st,0,false,"fp32",path.backend);else ih::layer',
    'for(const auto& path:paths)if(path.kind>=5){ih::Timing t;ih::Stats st;if(path.kind==7)ihy::layer(g,model[1],prep[1],schedule,master[0],heads[1],false,path.block,64,t,st,0,false,"fp32",path.threshold);else if(path.kind==6)ipp::layer(g,model[1],prep[1],schedule,master[0],heads[1],false,path.block,64,t,st,0,false,"fp32",path.backend);else ih::layer')
one('for(const auto& path:paths)if(path.kind==5||path.kind==6){std::array<Times,3>',
    'for(const auto& path:paths)if(path.kind>=5){std::array<Times,3>')
one('if(path.kind==6)ipp::layer(g,model[l],prep[l],schedule,*input,heads[l],l<2,path.block,64,t,st,period,true,"fp32",path.backend);else ih::layer',
    'if(path.kind==7)ihy::layer(g,model[l],prep[l],schedule,*input,heads[l],l<2,path.block,64,t,st,period,true,"fp32",path.threshold);else if(path.kind==6)ipp::layer(g,model[l],prep[l],schedule,*input,heads[l],l<2,path.block,64,t,st,period,true,"fp32",path.backend);else ih::layer')
one('std::cout<<"ICPP_PRELOAD_COMPLETE old_heads_smoke_plus_bitwise_preload_regression=true\\n";',
    'std::cout<<"ICPP_HYBRID_COMPLETE old_and_preload_smoke_plus_mixed_oracle=true\\n";')
one('if(path.backend=="amx_hi_lo" && (st.ph_calls',
    'if(path.kind!=7 && path.backend=="amx_hi_lo" && (st.ph_calls')
one('if(path.backend=="avx_shared" && (st.ph_calls',
    'if(path.kind!=7 && path.backend=="avx_shared" && (st.ph_calls')
one('    }\n    std::sort(ratios.begin(),ratios.end());',
    '''        if(path.kind==7){
            uint64_t avx=0,amx=0,avx_edges=0;
            for(uint64_t i=0;i<g.n;++i){uint64_t deg=g.row[i+1]-g.row[i];for(uint64_t b=0;b<deg;b+=path.block){auto len=std::min<uint64_t>(path.block,deg-b);if(len<uint64_t(path.threshold)){++avx;avx_edges+=len;}else ++amx;}}
            uint64_t expect_ph=amx*(dp/16)*2,expect_fma=amx*uint64_t(p.heads)*32*dp*2+avx_edges*uint64_t(p.heads)*((p.in+15)/16*16);
            uint64_t expect_packed=amx*32*dp*2,expect_scatter=amx*uint64_t(p.heads)*dp*4+avx*uint64_t(p.heads)*((p.in+15)/16*16)*4;
            if(avx+amx!=rowblocks||st.ph_calls!=expect_ph||st.ph_physical_fma!=expect_fma||st.packed_H_bytes!=expect_packed||st.ub_scatter_bytes!=expect_scatter)throw std::runtime_error("hybrid PH work identity");
            std::cout<<"HYBRID_MIX path="<<path.name<<" layer="<<layer<<" threshold="<<path.threshold<<" avx_rowblocks="<<avx<<" amx_rowblocks="<<amx<<" avx_edges="<<avx_edges<<" AMX_P_tile_load_bytes="<<amx*1024<<" physical_to_useful_PH_FMA="<<(st.ph_useful_fma?double(st.ph_physical_fma)/st.ph_useful_fma:0)<<"\\n";
        }
    }
    std::sort(ratios.begin(),ratios.end());''')
(root/'tfs_online'/'hybrid_main.cpp').write_text(text,encoding='utf-8',newline='\n')
scripts=root/'scripts'
for old,new in [('build_icpp_preload.sh','build_icpp_hybrid.sh'),('run_icpp_preload.slurm','run_icpp_hybrid.slurm'),('icpp_preload_payload.sh','icpp_hybrid_payload.sh')]:
    code=(scripts/old).read_text(encoding='utf-8')
    code=code.replace('icpp-preload','icpp-hybrid').replace('icpp_preload_payload','icpp_hybrid_payload').replace('icpp_preload"','icpp_hybrid"').replace('icpp_preload.s','icpp_hybrid.s').replace('build_icpp_preload.sh','build_icpp_hybrid.sh')
    if new.startswith('build_'):
        code=code.replace('"$ROOT/tfs_online/preload_main.cpp")',
          '"$ROOT/tfs_online/icpp_heads_hybrid.cpp" "$ROOT/tfs_online/hybrid_checks.cpp" "$ROOT/tfs_online/hybrid_main.cpp")')
        code=code.replace('"$ROOT/tfs_online/icpp_heads_preload.cpp" -o "$RUN/icpp_hybrid.s"',
          '"$ROOT/tfs_online/icpp_heads_hybrid.cpp" -o "$RUN/icpp_hybrid.s"')
    elif new.endswith('_payload.sh'):
        code=code.replace('purpose=preloaded_Phi_Plo_into_TMM6_TMM2_once_per_destination_keeping_DegreeSort_TR16_AMX_UW',
          'purpose=per_destination_exact_short_AVX_dense_AMX_PH_dispatch_keeping_DegreeSort_TR16_AMX_UW')
        code=code.replace('candidate=HEADS_AVX_B16_B32,HEADS_AMX_HILO_B16_B32,HEADS_AMX_PRELOAD_B16_B32',
          'candidate=HEADS_AVX_B16_B32,HEADS_AMX_HILO_B16_B32,HEADS_AMX_PRELOAD_B16_B32,HYBRID_T16_T24_T32_B32')
        code=code.replace('correctness=old_heads_independent_oracle_plus_18operator_6layer_bitwise_preload_regression_before_speed;master_abs_gate=.003_unchanged',
          'correctness=old_heads_independent_oracle_plus_preload_bitwise_plus_mixed_quantized_oracle_before_speed;master_abs_gate=.003_unchanged')
    (scripts/new).write_text(code,encoding='utf-8',newline='\n')
print('created=hybrid_main.cpp,build_icpp_hybrid.sh,run_icpp_hybrid.slurm,icpp_hybrid_payload.sh')
