#pragma once
#include "icpp_block.hpp"

// DegreeSort/TR16 TFS extension. A bounded block U[head][row][D] is
// immediately consumed by AMX U W. V[head][row][32] is worker-local across
// blocks: switching heads stores/reloads V explicitly, not eight resident TMMs.
namespace gat::icpp_heads {
using Timing=icpp_online::Timing;
struct Workspace {
    icpp_block::Workspace fallback;
    std::vector<float> ubuf,vbuf;
    std::vector<BF16> bbuf;
    void allocate(const Graph&,const Param&,bool counters=false);
    size_t bytes() const;
};
struct Stats {
    icpp_online::Stats baseline;
    bool optimized=false;
    uint64_t ph_calls=0,ph_physical_fma=0,ph_useful_fma=0;
    uint64_t raw_H_bytes=0,packed_H_bytes=0,projection_calls=0;
    uint64_t head_switch_store_bytes=0,head_switch_reload_bytes=0,ub_scatter_bytes=0;
    double source_index=0,H_pack=0,P_pack=0,PH_load=0,PH_compute=0;
    double PH_store_scatter=0,UB_convert=0,UW_load=0,UW_compute=0;
    double V_store=0,V_reload=0,row_rescale=0,transition=0;
};
// Optimized only for K8,d32,D<=256,B16/B32. Both backends use exactly the
// same FP32 running m/l/p and the same RNE(U_B)->AMX U_B W contraction.
// amx_hi_lo represents each FP32 p as two RNE BF16 components for PH.
// AVX shared-H PH keeps FP32 p. Other shapes use icpp_block unchanged.
// baseline.amx_calls/executed_fma count UW only; PH has separate counters.
// ph_useful_fma=E*K*D for either backend; AMX physical includes two passes,
// 32-neighbor padding and padded D, AVX physical includes 16-feature tails.
// Logical byte counts are software accesses, not measured DRAM traffic.
void aggregate(const Graph&,const Param&,const Prepared&,const Schedule&,
               Workspace&,int block,int panel,Stats&,int profile_period=0,
               bool counters=false,const std::string& backend="amx_hi_lo");
void layer(const Graph&,const Param&,const Prepared&,const Schedule&,
           const std::vector<float>& input,Workspace&,bool hidden,int block,
           int panel,Timing&,Stats&,int profile_period=0,bool counters=false,
           const std::string& lr_policy="fp32",const std::string& backend="amx_hi_lo");
}
