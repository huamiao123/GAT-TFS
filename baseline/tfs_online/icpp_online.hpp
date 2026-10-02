#pragma once
#include "gat.hpp"

// An extension of baseline_tfs.cpp, not a replacement aggregation/GEMM backend.
// TR16 neighbor-step BF16(pH)W and resident AMX output tiles are retained.
namespace gat::icpp_online {
struct Timing {
    double convert=0,lr=0,kernel=0,normalize=0,activation=0,total=0;
};
struct Stats {
    // Sampled worker seconds: nested kernel intervals, never additive wall times.
    double score_generation=0,block_max=0,exp_den=0;
    double rescale_store=0,rescale_vector=0,rescale_reload=0;
    double source_gather=0,weighted_convert=0,tile_load=0,amx_compute=0;
    double final_store=0,output_write=0,scheduling=0,tile_config=0;
    // Exact counts are populated only when counters=true. Initial maxima are
    // updates but not rescales. Output state is V[d], not U[D].
    uint64_t blocks=0,max_updates=0,rescales=0;
    uint64_t rescaled_feature_elements=0,rescaled_padded_feature_elements=0;
    uint64_t spill_bytes=0,reload_bytes=0,rescale_events=0;
    uint64_t amx_calls=0,neighbor_steps=0,executed_fma=0;
    uint64_t sampled_tiles=0,total_tiles=0;
};
struct Workspace {
    gat::Workspace base;
    std::vector<uint64_t> row_blocks,row_max_updates,row_rescales;
    uint64_t allocated_n=0;
    int allocated_in=0,allocated_heads=0,allocated_dim=0,thread_capacity=0;
    void allocate(const Graph&,const Param&,bool counters=false);
    size_t bytes() const;
};

// Expects original validated destination CSR, original prepare(...,tfs_pack=true),
// and degree_schedule. Attention may be fixed by writing base.left/base.right.
// No score[E], alpha[E], full Z, full U, or max-prescan is allocated/executed.
// aggregate writes unnormalized V to base.out; layer normalizes and activates.
void aggregate(const Graph&,const Param&,const Prepared&,const Schedule&,
               Workspace&,int block,int panel,Stats&,int profile_period=0,
               bool counters=false);
void layer(const Graph&,const Param&,const Prepared&,const Schedule&,
           const std::vector<float>& input,Workspace&,bool hidden,int block,
           int panel,Timing&,Stats&,int profile_period=0,bool counters=false,
           const std::string& lr_policy="fp32");
}
