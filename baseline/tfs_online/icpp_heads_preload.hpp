#pragma once
#include "icpp_heads.hpp"

// Same PH arithmetic, bounded buffers and UW contraction as icpp_heads,
// retaining both BF16 probability components in TMM6/TMM2 across feature tiles.
// Existing icpp_heads remains the baseline; only amx_hi_lo is accepted here.
namespace gat::icpp_heads_preload {
using Timing=icpp_heads::Timing;
using Workspace=icpp_heads::Workspace;
using Stats=icpp_heads::Stats;
void aggregate(const Graph&,const Param&,const Prepared&,const Schedule&,
               Workspace&,int block,int panel,Stats&,int profile_period=0,
               bool counters=false,const std::string& backend="amx_hi_lo");
void layer(const Graph&,const Param&,const Prepared&,const Schedule&,
           const std::vector<float>& input,Workspace&,bool hidden,int block,
           int panel,Timing&,Stats&,int profile_period=0,bool counters=false,
           const std::string& lr_policy="fp32",const std::string& backend="amx_hi_lo");
}
