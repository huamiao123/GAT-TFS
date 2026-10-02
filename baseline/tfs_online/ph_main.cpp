#include "icpp_block.hpp"
#include "head_ph_probe.hpp"
#include "head_ph_compact_probe.hpp"
#include <iostream>
using namespace gat;
int main(int argc,char** argv){try{
    if(argc!=2)throw std::runtime_error("usage: icpp_ph GRAPH");
    omp_set_dynamic(0);omp_set_num_threads(16);mkl_set_num_threads(16);mkl_set_dynamic(0);
    Graph g=load_graph(argv[1]);auto model=random_model(g.din,g.classes,11);auto sched=degree_schedule(g);
    auto first=prepare(model[0]),second=prepare(model[1]);
    icpp_block::Workspace layer1;layer1.allocate(g,model[0],false);
    icpp_block::Timing timing;icpp_block::Stats stats;
    icpp_block::layer(g,model[0],first,sched,g.x,layer1,true,64,64,timing,stats,0,false,"fp32");
    Workspace base;base.allocate(g,model[1],"tfs_bf16");
    convert(layer1.original.base.out.data(),base.xbf.data(),layer1.original.base.out.size());
    lr_reordered(g,model[1],second,layer1.original.base.out,base,"fp32");
    std::cout<<"HEAD_PH_INPUT source=ICPP_TFS_BLOCK_B64_own_L1_output contracted_LR=fp32 value_storage=native_BF16_H independent_head_weights=1 seed=11 upstream_layer_prepare_ms="<<timing.total*1000<<" upstream_outside_microtimer=1\n";
    std::cout<<"PH_LAYOUT_CONTROL variant=ORIGINAL16_scalar_pack\n";
    icpp_block::probe_head_PH(g,model[1],base,sched);
    std::cout<<"PH_LAYOUT_CONTROL variant=COMPACT8_direct_vector_pack\n";
    icpp_ph_compact::probe_head_PH_compact(g,model[1],base,sched);
    std::cout<<"ICPP_PH_COMPLETE scope=sampled_firstblock_micro_only full_model_speedup=NOT_MEASURED full_model_master_gate=NOT_EVALUATED\n";
    return 0;
}catch(const std::exception& e){std::cerr<<"ICPP_PH_FAIL "<<e.what()<<'\n';return 1;}}
