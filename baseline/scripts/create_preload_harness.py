#!/usr/bin/env python3
"""Derive an isolated full-model preload comparison from the checked heads harness."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
source=root/'tfs_online'/'heads_main.cpp'
text=source.read_text(encoding='utf-8')
def replace_once(a,b):
    global text
    assert text.count(a)==1,(a,text.count(a))
    text=text.replace(a,b)
replace_once('#include "icpp_heads.hpp"','#include "icpp_heads.hpp"\n#include "icpp_heads_preload.hpp"\n#include "preload_checks.hpp"')
replace_once('namespace ih=gat::icpp_heads;','namespace ih=gat::icpp_heads;\nnamespace ipp=gat::icpp_heads_preload;')
replace_once('io::run_smoke();ip::run_pair_smoke();ib::run_block_smoke();ih::run_heads_smoke();return 0;',
             'io::run_smoke();ip::run_pair_smoke();ib::run_block_smoke();ih::run_heads_smoke();ipp::run_preload_smoke();return 0;')
replace_once('{"HEADS_AMX_HILO_B32",5,32,"amx_hi_lo"}};',
             '{"HEADS_AMX_HILO_B32",5,32,"amx_hi_lo"},{"HEADS_AMX_PRELOAD_B16",6,16,"amx_hi_lo"},{"HEADS_AMX_PRELOAD_B32",6,32,"amx_hi_lo"}};')
replace_once('if(path.kind==5)return heads[l].fallback.original.base.out;',
             'if(path.kind==5||path.kind==6)return heads[l].fallback.original.base.out;')
replace_once('if(path.kind==5){ih::Stats s;ih::layer',
             'if(path.kind==6){ipp::Stats s;ipp::layer(g,model[l],prep[l],schedule,*x,heads[l],l<2,path.block,64,ot[l],s,0,false,"fp32",path.backend);}\n        else if(path.kind==5){ih::Stats s;ih::layer')
replace_once('if(path.kind==5){ih::Timing t;ih::Stats st;ih::layer(g,model[1],prep[1],schedule,master[0],heads[1],false,path.block,64,t,st,0,false,"fp32",path.backend);report',
             'if(path.kind==5||path.kind==6){ih::Timing t;ih::Stats st;if(path.kind==6)ipp::layer(g,model[1],prep[1],schedule,master[0],heads[1],false,path.block,64,t,st,0,false,"fp32",path.backend);else ih::layer(g,model[1],prep[1],schedule,master[0],heads[1],false,path.block,64,t,st,0,false,"fp32",path.backend);report')
replace_once('for(const auto& path:paths)if(path.kind==5){std::array<Times,3>',
             'for(const auto& path:paths)if(path.kind==5||path.kind==6){std::array<Times,3>')
replace_once('ih::layer(g,model[l],prep[l],schedule,*input,heads[l],l<2,path.block,64,t,st,period,true,"fp32",path.backend);input=&output(path,l);',
             'if(path.kind==6)ipp::layer(g,model[l],prep[l],schedule,*input,heads[l],l<2,path.block,64,t,st,period,true,"fp32",path.backend);else ih::layer(g,model[l],prep[l],schedule,*input,heads[l],l<2,path.block,64,t,st,period,true,"fp32",path.backend);input=&output(path,l);')
replace_once('std::cout<<"ICPP_HEADS_COMPLETE architecture=',
             'std::cout<<"ICPP_PRELOAD_COMPLETE old_heads_smoke_plus_bitwise_preload_regression=true \n";std::cout<<"ICPP_HEADS_COMPLETE architecture=')
text=text.replace('old_heads_smoke_plus_bitwise_preload_regression=true \n"','old_heads_smoke_plus_bitwise_preload_regression=true\\n"')
(root/'tfs_online'/'preload_main.cpp').write_text(text,encoding='utf-8',newline='\n')

scripts=root/'scripts'
for old,new in [('build_icpp_heads.sh','build_icpp_preload.sh'),('run_icpp_heads.slurm','run_icpp_preload.slurm'),('icpp_heads_payload.sh','icpp_preload_payload.sh')]:
    script=(scripts/old).read_text(encoding='utf-8')
    script=script.replace('icpp-heads','icpp-preload').replace('icpp_heads_payload','icpp_preload_payload').replace('icpp_heads"','icpp_preload"').replace('icpp_heads.s','icpp_preload.s')
    script=script.replace('build_icpp_heads.sh','build_icpp_preload.sh')
    if new.startswith('build_'):
        script=script.replace('"$ROOT/tfs_online/heads_main.cpp")',
           '"$ROOT/tfs_online/icpp_heads_preload.cpp" "$ROOT/tfs_online/preload_checks.cpp" "$ROOT/tfs_online/preload_main.cpp")')
        script=script.replace('"$ROOT/tfs_online/icpp_heads.cpp" -o "$RUN/icpp_preload.s"',
           '"$ROOT/tfs_online/icpp_heads_preload.cpp" -o "$RUN/icpp_preload.s"')
    elif new.startswith('run_'):
        script=script.replace('--time=00:40:00','--time=00:40:00')
    else:
        script=script.replace('purpose=cross_head_sharedH_PH_AMX_UW_fusion_retaining_DegreeSort_TR16',
          'purpose=preloaded_Phi_Plo_into_TMM6_TMM2_once_per_destination_keeping_DegreeSort_TR16_AMX_UW')
        script=script.replace('correctness=original_pair_block_and_independent_heads_instruction_oracles_before_speed;master_abs_gate=.003_unchanged',
          'correctness=old_heads_independent_oracle_plus_18operator_6layer_bitwise_preload_regression_before_speed;master_abs_gate=.003_unchanged')
        script=script.replace('candidate=HEADS_AVX_B16_B32,HEADS_AMX_HILO_B16_B32',
          'candidate=HEADS_AVX_B16_B32,HEADS_AMX_HILO_B16_B32,HEADS_AMX_PRELOAD_B16_B32')
    (scripts/new).write_text(script,encoding='utf-8',newline='\n')
print('created=preload_main.cpp,build_icpp_preload.sh,run_icpp_preload.slurm,icpp_preload_payload.sh')
