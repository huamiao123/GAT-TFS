#pragma once
#include "icpp_heads.hpp"

// Per destination/block PH dispatch using the actual valid neighbor count.
// Short blocks use the original FP32 shared-H AVX reduction; long blocks use
// the checked preload BF16 high/low PH. Both immediately consume RNE(U_B) in
// the identical AMX UW contraction and preserve independent FP32 m/l/p.
namespace gat::icpp_heads_hybrid {
using Timing=icpp_heads::Timing;
using Workspace=icpp_heads::Workspace;
using Stats=icpp_heads::Stats;
// threshold is in [1,33]. 1 gives all-AMX and 33 all-AVX controls; the
// experimental thresholds are 16/24/32. B remains 16 or 32 for K8,d32,D<=256.
// With counters, AMX rowblocks = ph_calls / (2*(Dpad/16)); all rowblocks =
// baseline.blocks/heads. The difference gives AVX rowblocks without new fields.
// PH physical FMA includes AMX high/low and K32/D32 padding only for AMX rows,
// and AVX D16 lanes only for AVX rows. raw_H_bytes retains E*D*sizeof(BF16).
void aggregate(const Graph&,const Param&,const Prepared&,const Schedule&,
               Workspace&,int block,int panel,Stats&,int profile_period=0,
               bool counters=false,int threshold=16);
void layer(const Graph&,const Param&,const Prepared&,const Schedule&,
           const std::vector<float>& input,Workspace&,bool hidden,int block,
           int panel,Timing&,Stats&,int profile_period=0,bool counters=false,
           const std::string& lr_policy="fp32",int threshold=16);
}
