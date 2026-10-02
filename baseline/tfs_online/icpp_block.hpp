#pragma once
#include "icpp_online.hpp"

// Block-granularity TFS extension: transient U_B is consumed immediately by
// AMX, while V[d] remains resident across blocks. No trailing SGEMM or full U/Z.
namespace gat::icpp_block {
using Timing=icpp_online::Timing;
struct Workspace {
    icpp_online::Workspace original;
    std::vector<float> ubuf;   // [worker][16][paddedD], reset for each block
    std::vector<BF16> bbuf;    // RNE(U_B), same bounded worker-local shape
    void allocate(const Graph&,const Param&,bool counters=false);
    size_t bytes() const;
};
struct Stats {
    icpp_online::Stats baseline;
    uint64_t actual_input_feature_FMA=0; // valid D feature updates, one per edge/head
    uint64_t padded_input_feature_FMA=0; // issued AVX vector lanes, including D tail
    uint64_t useful_projection_FMA=0;   // active row/head blocks * D * head_dim
    uint64_t logical_H_bytes=0;        // actual D BF16 source bytes, no cache claim
    uint64_t block_projection_calls=0; // one TR16/head/block AMX U_B W invocation
    uint64_t conversion_elements=0;    // all 16*paddedD BF16 conversions per call
    uint64_t source_neighbor_steps=0;   // original synchronous neighbor positions
};

// baseline.blocks/max_updates/rescales retain row/head Online units.
// baseline.neighbor_steps NOW counts block AMX accumulation steps, equal to
// block_projection_calls. baseline.amx_calls/executed_fma are actual block TDP
// instruction/physical arithmetic counts; they are not the edgewise counts.
// Coarse sampled-worker profile: source_gather includes raw H + FP32 SpMM;
// weighted_convert is block RNE packing; amx_compute includes tile load+compute;
// tile_load remains zero. Exact counts populate only when counters=true.
void aggregate(const Graph&,const Param&,const Prepared&,const Schedule&,
               Workspace&,int block,int panel,Stats&,int profile_period=0,
               bool counters=false);
void layer(const Graph&,const Param&,const Prepared&,const Schedule&,
           const std::vector<float>& input,Workspace&,bool hidden,int block,
           int panel,Timing&,Stats&,int profile_period=0,bool counters=false,
           const std::string& lr_policy="fp32");
}
