#pragma once
#include "icpp_online.hpp"

namespace gat::icpp_pair {
using Timing=icpp_online::Timing;
struct Workspace {
    icpp_online::Workspace original;
    std::vector<BF16> paired_hbuf;
    void allocate(const Graph&,const Param&,bool counters=false);
    size_t bytes() const;
};
struct Stats {
    icpp_online::Stats baseline;
    // Logical raw-source reads, populated only with counters=true. A tail is
    // one feature-vector chunk; bytes count actual D BF16 elements, no padding.
    // Source-index loads refer to CSR indices used by score/gather, not cache
    // prefetch instructions. The optimized path reuses those indices to prefetch.
    uint64_t raw_H_vectors=0,logical_raw_H_bytes=0,source_index_loads=0;
};

// Even head counts and head_dim<=32 use independent head pairs; every other
// shape falls back to icpp_online unchanged. No full Z/U/edge weights are made.
// Optimized profile fields are coarse sampled worker seconds:
//   source_gather = raw gather + both independent weighted conversions;
//   amx_compute = AMX tile loads + AMX arithmetic;
//   weighted_convert and tile_load = 0 (included above).
// Exact baseline counters keep original per-head units, including tile counts.
void aggregate(const Graph&,const Param&,const Prepared&,const Schedule&,
               Workspace&,int block,int panel,Stats&,int profile_period=0,
               bool counters=false);
void layer(const Graph&,const Param&,const Prepared&,const Schedule&,
           const std::vector<float>& input,Workspace&,bool hidden,int block,
           int panel,Timing&,Stats&,int profile_period=0,bool counters=false,
           const std::string& lr_policy="fp32");
}
