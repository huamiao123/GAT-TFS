#pragma once
#include "gat.hpp"
namespace gat::local {
struct PreparedLocal {
    Prepared base;
    std::vector<float> wh; // contiguous per-head FP32 W[D][d]
    std::vector<BF16> low; // VNNI-packed residual W
    double prepare_s=0;
};
struct WorkspaceLocal {
    std::vector<float> lr,out,u,c;
    std::vector<BF16> uh,ul;
    std::vector<uint32_t> blocks,updates,rescales;
    void allocate(const Graph&,const Param&,bool counters=false);
};
struct Stats {
    double init=0,score=0,rescale=0,spmm=0,packing=0,gemm=0,output=0;
    uint64_t sampled_tiles=0;
};
struct Timing {
    double lr=0,kernel=0,activation=0,total=0;
    Stats sample;
};
PreparedLocal prepare_local(const Param&);
void layer(const Graph&,const Param&,const PreparedLocal&,const Schedule&,
           const std::vector<float>&,WorkspaceLocal&,bool hidden,bool amx,
           int block,int panel,Timing&,uint64_t sample_period=0,bool counters=false);
void aggregate(const Graph&,const Param&,const PreparedLocal&,const Schedule&,
               const std::vector<float>&,WorkspaceLocal&,bool amx,int block,int panel,
               Stats&,uint64_t sample_period=0,bool counters=false);
void print_stats(const WorkspaceLocal&,const Param&,int layer);
}
