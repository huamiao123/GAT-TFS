#include "icpp_heads.hpp"
#include "icpp_heads_preload.hpp"
#include "preload_checks.hpp"
#include "icpp_pair.hpp"
#include "heads_checks.hpp"
#include "checks.hpp"
#include "pair_checks.hpp"
#include "block_checks.hpp"
#include <array>
#include <iomanip>
#include <iostream>
using namespace gat;
namespace io=gat::icpp_online;
namespace ip=gat::icpp_pair;
namespace ib=gat::icpp_block;
namespace ih=gat::icpp_heads;
namespace ipp=gat::icpp_heads_preload;

static Error difference(const std::vector<float>& ref,const std::vector<float>& out) {
    if(ref.empty()||ref.size()!=out.size())throw std::runtime_error("comparison shape");
    double maximum=0,absolute=0,squared=0,norm=0;int bad=0;
    #pragma omp parallel for reduction(max:maximum) reduction(+:absolute,squared,norm) reduction(|:bad)
    for(size_t i=0;i<ref.size();++i){bad|=!std::isfinite(ref[i])||!std::isfinite(out[i]);double d=double(out[i])-ref[i];maximum=std::max(maximum,std::abs(d));absolute+=std::abs(d);squared+=d*d;norm+=double(ref[i])*ref[i];}
    Error e;e.finite=!bad;e.max_abs=maximum;e.mean_abs=absolute/ref.size();e.relative_l2=norm?std::sqrt(squared/norm):(squared?INFINITY:0);
    if(bad)e.max_abs=e.mean_abs=e.relative_l2=INFINITY;return e;
}
static void report(const std::string& path,int layer,const std::string& reference,const Error& e) {
    std::cout<<std::setprecision(12)<<"CHECK path="<<path<<" layer="<<layer<<" reference="<<reference
      <<" max_abs_error="<<e.max_abs<<" mean_abs_error="<<e.mean_abs<<" relative_L2_error="<<e.relative_l2
      <<" finite="<<e.finite<<" original_abs_003_gate="<<(e.finite&&e.max_abs<=.003?"PASS":"FAIL")<<" task_accuracy=UNVERIFIED\n";
    if(!e.finite)throw std::runtime_error("nonfinite output");
}
static uint64_t fingerprint(const std::vector<float>& x) {
    uint64_t out=0;
    #pragma omp parallel for reduction(^:out)
    for(size_t i=0;i<x.size();++i){uint32_t bits;std::memcpy(&bits,x.data()+i,4);uint64_t z=uint64_t(bits)^((i+1)*0x9e3779b97f4a7c15ULL);z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;out^=z^(z>>27);}return out;
}
struct Path {std::string name;int kind,block;std::string backend;};
static void profile(const Path& path,int layer,const Graph& g,const Param& p,const Schedule& sched,const ih::Timing& t,const ih::Stats& st,const ih::Workspace& w) {
    const auto& s=st.baseline;const auto& original=w.fallback.original;
    uint64_t blocks=0,updates=0,rescales=0;std::vector<double> ratios;
    for(size_t at=0;at<original.row_blocks.size();++at){auto b=original.row_blocks[at];blocks+=b;updates+=original.row_max_updates[at];rescales+=original.row_rescales[at];ratios.push_back(b?double(original.row_rescales[at])/b:0);}
    if(blocks!=s.blocks||updates!=s.max_updates||rescales!=s.rescales||s.rescaled_feature_elements!=rescales*uint64_t(p.dim))throw std::runtime_error("heads per-row identity");
    uint64_t dp=(p.in+31)/32*32;
    if(st.optimized){
        uint64_t rowblocks=0,groups=0,nonempty=0;
        for(uint64_t i=0;i<g.n;++i)rowblocks+=(g.row[i+1]-g.row[i]+path.block-1)/path.block;
        for(uint64_t first=0;first<g.n;first+=16){uint64_t md=0;for(uint64_t n=first;n<std::min(g.n,first+16);++n){auto r=sched.perm[n];md=std::max(md,g.row[r+1]-g.row[r]);}groups+=(md+path.block-1)/path.block;nonempty+=md>0;}
        if(blocks!=rowblocks*p.heads||st.ph_useful_fma!=g.e*uint64_t(p.heads)*p.in||st.raw_H_bytes!=g.e*uint64_t(p.in)*2||st.projection_calls!=groups*p.heads||s.amx_calls!=st.projection_calls*(dp/32)*2||s.executed_fma!=st.projection_calls*16*dp*32||st.head_switch_store_bytes!=groups*p.heads*2048||st.head_switch_reload_bytes!=(groups-nonempty)*p.heads*2048||s.spill_bytes||s.reload_bytes)throw std::runtime_error("heads work identity");
        if(path.backend=="amx_hi_lo" && (st.ph_calls!=rowblocks*(dp/16)*2||st.ph_physical_fma!=rowblocks*p.heads*32*dp*2||st.packed_H_bytes!=rowblocks*32*dp*2||st.ub_scatter_bytes!=rowblocks*p.heads*dp*4))throw std::runtime_error("heads PH AMX identity");
        if(path.backend=="avx_shared" && (st.ph_calls||st.packed_H_bytes||st.ph_physical_fma!=g.e*uint64_t(p.heads)*((p.in+15)/16*16)||st.ub_scatter_bytes!=rowblocks*p.heads*((p.in+15)/16*16)*4))throw std::runtime_error("heads PH AVX identity");
    }
    std::sort(ratios.begin(),ratios.end());auto pct=[&](double f){return ratios.empty()?0:ratios[std::min(ratios.size()-1,size_t(std::ceil(f*ratios.size())-1))];};
    std::cout<<"PROFILE path="<<path.name<<" layer="<<layer<<" optimized="<<st.optimized<<" layer_wall_ms="<<t.total*1000<<" kernel_wall_ms="<<t.kernel*1000<<" sampled_head_tiles="<<s.sampled_tiles<<" worker_times_are_sampled_sums=true"
      <<" source_index_worker_ms="<<st.source_index*1000<<" score_worker_ms="<<s.score_generation*1000<<" block_max_worker_ms="<<s.block_max*1000<<" exp_den_worker_ms="<<s.exp_den*1000
      <<" H_pack_worker_ms="<<st.H_pack*1000<<" P_pack_worker_ms="<<st.P_pack*1000<<" PH_load_worker_ms="<<st.PH_load*1000<<" PH_compute_worker_ms="<<st.PH_compute*1000<<" PH_store_scatter_worker_ms="<<st.PH_store_scatter*1000
      <<" UB_convert_worker_ms="<<st.UB_convert*1000<<" UW_load_worker_ms="<<st.UW_load*1000<<" UW_compute_worker_ms="<<st.UW_compute*1000<<" V_store_worker_ms="<<st.V_store*1000<<" V_reload_worker_ms="<<st.V_reload*1000<<" row_rescale_worker_ms="<<st.row_rescale*1000<<" transition_worker_ms="<<st.transition*1000
      <<" output_write_worker_ms="<<s.output_write*1000<<" scheduling_worker_ms="<<s.scheduling*1000<<" tile_config_worker_ms="<<s.tile_config*1000
      <<" fallback_gather_worker_ms="<<s.source_gather*1000<<" fallback_UB_convert_worker_ms="<<s.weighted_convert*1000<<" fallback_AMX_load_compute_worker_ms="<<s.amx_compute*1000
      <<" baseline_rescale_store_worker_ms="<<s.rescale_store*1000<<" baseline_rescale_vector_worker_ms="<<s.rescale_vector*1000<<" baseline_rescale_reload_worker_ms="<<s.rescale_reload*1000<<" fallback_final_store_worker_ms="<<s.final_store*1000<<" instruction_span_timers_have_overhead=true\n";
    std::cout<<"ONLINE_STATS path="<<path.name<<" layer="<<layer<<" block="<<path.block<<" heads="<<p.heads<<" D="<<p.in<<" d="<<p.dim<<" edges="<<g.e<<" blocks="<<blocks<<" max_updates_including_initial="<<updates<<" rescales="<<rescales<<" rescale_per_block="<<(blocks?double(rescales)/blocks:0)<<" P50="<<pct(.5)<<" P90="<<pct(.9)<<" P95="<<pct(.95)<<" P99="<<pct(.99)<<" rescaled_feature_elements="<<s.rescaled_feature_elements<<" rescale_events="<<s.rescale_events<<" spill_bytes="<<s.spill_bytes<<" reload_bytes="<<s.reload_bytes<<" UW_calls="<<s.amx_calls<<" UW_physical_FMA="<<s.executed_fma<<" head_tiles="<<s.total_tiles<<" projection_groups="<<st.projection_calls<<'\n';
    std::cout<<"HEAD_WORK path="<<path.name<<" layer="<<layer<<" optimized="<<st.optimized<<" PH_calls="<<st.ph_calls<<" PH_physical_FMA="<<st.ph_physical_fma<<" PH_useful_FMA="<<st.ph_useful_fma<<" raw_H_bytes="<<st.raw_H_bytes<<" packed_H_bytes="<<st.packed_H_bytes<<" UB_scatter_bytes="<<st.ub_scatter_bytes<<" head_switch_store_bytes="<<st.head_switch_store_bytes<<" head_switch_reload_bytes="<<st.head_switch_reload_bytes<<" sparse_width_amplification="<<double(p.in)/p.dim<<" traffic_is_logical_not_DRAM=true no_full_Z_U_e_alpha=true bounded_V_in_local_memory=true\n";
}
int main(int argc,char** argv){try {
    if(argc==2&&std::string(argv[1])=="--smoke"){omp_set_dynamic(0);omp_set_num_threads(2);mkl_set_num_threads(2);mkl_set_dynamic(0);io::run_smoke();ip::run_pair_smoke();ib::run_block_smoke();ih::run_heads_smoke();ipp::run_preload_smoke();return 0;}
    if(argc!=6)throw std::runtime_error("usage: icpp_heads GRAPH THREADS WARMUPS REPEATS PROFILE_PERIOD | --smoke");
    int threads=std::stoi(argv[2]),warm=std::stoi(argv[3]),reps=std::stoi(argv[4]),period=std::stoi(argv[5]);
    if(threads<1||warm<0||reps<1||period<1)throw std::runtime_error("invalid configuration");
    omp_set_dynamic(0);omp_set_num_threads(threads);mkl_set_num_threads(threads);mkl_set_dynamic(0);
    Graph g=load_graph(argv[1]);auto model=random_model(g.din,g.classes,11);auto schedule=degree_schedule(g);
    std::vector<Prepared> prep;std::vector<Workspace> standard(3),b1(3);std::vector<ip::Workspace> paired(3);std::vector<ib::Workspace> blocked(3);std::vector<ih::Workspace> heads(3);std::vector<std::vector<float>> master(3);
    for(int l=0;l<3;++l){prep.push_back(prepare(model[l]));standard[l].allocate(g,model[l],"standard_bf16");b1[l].allocate(g,model[l],"tfs_bf16");paired[l].allocate(g,model[l],false);blocked[l].allocate(g,model[l],false);heads[l].allocate(g,model[l],true);}
    standard[1].lr.resize(checked(g.n,2*model[1].heads));
    std::vector<Path> paths={{"B0_FP32",0,0,""},{"B0_BF16",1,0,""},{"B1_TFS_PREMAX",2,0,""},{"ICPP_TFS_PAIR_B32",3,32,""},{"ICPP_TFS_BLOCK_B32",4,32,""},{"HEADS_AVX_B16",5,16,"avx_shared"},{"HEADS_AVX_B32",5,32,"avx_shared"},{"HEADS_AMX_HILO_B16",5,16,"amx_hi_lo"},{"HEADS_AMX_HILO_B32",5,32,"amx_hi_lo"},{"HEADS_AMX_PRELOAD_B16",6,16,"amx_hi_lo"},{"HEADS_AMX_PRELOAD_B32",6,32,"amx_hi_lo"}};
    std::cout<<std::setprecision(12)<<"CONFIG N="<<g.n<<" E="<<g.e<<" Din="<<g.din<<" C="<<g.classes<<" threads="<<threads<<" check_runs=1 warmups="<<warm<<" repeats="<<reps<<" degree_sort_ms="<<schedule.degree_sort_s*1000<<" seed=11 untrained=true LR_policy=fp32 panel=64 tile_rows=16 attention_independent=true master_abs_gate=.003 performance=EXPLORATORY task_accuracy=UNVERIFIED\n";
    const std::vector<float>* input=&g.x;
    for(int l=0;l<3;++l){Times t;standard_layer(g,model[l],prep[l],*input,standard[l],l<2,t,true);master[l]=standard[l].out;input=&standard[l].out;auto& b=heads[l].fallback.original.base;std::cout<<"STATIC layer="<<l+1<<" weight_and_bLR_prepare_ms="<<prep[l].weight_prepare_s*1000<<" heads_workspace_bytes="<<heads[l].bytes()<<" full_Z_bytes="<<b.z.size()*4<<" edge_score_bytes="<<b.score.size()*4<<" alpha_bytes="<<b.alpha.size()*4<<" optimized_U_bytes="<<heads[l].ubuf.size()*4<<" local_V_bytes="<<heads[l].vbuf.size()*4<<'\n';}
    auto output=[&](const Path& path,int l)->const std::vector<float>& {if(path.kind==5||path.kind==6)return heads[l].fallback.original.base.out;if(path.kind==4)return blocked[l].original.base.out;if(path.kind==3)return paired[l].original.base.out;if(path.kind==2)return b1[l].out;return standard[l].out;};
    auto run=[&](const Path& path,std::array<Times,3>& bt,std::array<io::Timing,3>& ot){const std::vector<float>* x=&g.x;for(int l=0;l<3;++l){
        if(path.kind==6){ipp::Stats s;ipp::layer(g,model[l],prep[l],schedule,*x,heads[l],l<2,path.block,64,ot[l],s,0,false,"fp32",path.backend);}
        else if(path.kind==5){ih::Stats s;ih::layer(g,model[l],prep[l],schedule,*x,heads[l],l<2,path.block,64,ot[l],s,0,false,"fp32",path.backend);}
        else if(path.kind==4){ib::Stats s;ib::layer(g,model[l],prep[l],schedule,*x,blocked[l],l<2,path.block,64,ot[l],s,0,false,"fp32");}
        else if(path.kind==3){ip::Stats s;ip::layer(g,model[l],prep[l],schedule,*x,paired[l],l<2,path.block,64,ot[l],s,0,false,"fp32");}
        else if(path.kind==2)tfs_layer(g,model[l],prep[l],schedule,*x,b1[l],l<2,64,bt[l],"fp32");
        else standard_layer(g,model[l],prep[l],*x,standard[l],l<2,bt[l],path.kind==0);
        x=&output(path,l);}};
    for(int rep=-warm-1;rep<reps;++rep){auto order=paths;if(rep>=0&&rep%2)std::reverse(order.begin(),order.end());for(const auto& path:order){std::array<Times,3> bt{};std::array<io::Timing,3> ot{};auto begin=Clock::now();run(path,bt,ot);double e2e=seconds(begin,Clock::now());
        if(rep==-warm-1)for(int l=0;l<3;++l)report(path.name,l+1,"B0_FP32_own_output_3layer",difference(master[l],output(path,l)));
        if(rep>=0){require_finite(output(path,2),path.name);for(int l=0;l<3;++l){bool on=path.kind>=3;std::cout<<"{\"path\":\""<<path.name<<"\",\"rep\":"<<rep<<",\"layer\":"<<l+1<<",\"e2e_ms\":"<<e2e*1000<<",\"layer_ms\":"<<(on?ot[l].total:bt[l].total)*1000<<",\"projection_ms\":"<<bt[l].projection*1000<<",\"conversion_ms\":"<<(on?ot[l].convert:bt[l].convert)*1000<<",\"lr_ms\":"<<(on?ot[l].lr:bt[l].lr)*1000<<",\"max_prescan_ms\":"<<bt[l].max_prescan*1000<<",\"kernel_ms\":"<<(on?ot[l].kernel:bt[l].aggregate)*1000<<",\"normalization_ms\":"<<(on?ot[l].normalize:bt[l].normalize)*1000<<",\"activation_ms\":"<<(on?ot[l].activation:bt[l].activation)*1000<<",\"block\":"<<path.block<<"}\n";}}
    }}
    Times matched;standard_layer(g,model[1],prep[1],master[0],standard[1],false,matched,true,"fp32");
    for(const auto& path:paths)if(path.kind==5||path.kind==6){ih::Timing t;ih::Stats st;if(path.kind==6)ipp::layer(g,model[1],prep[1],schedule,master[0],heads[1],false,path.block,64,t,st,0,false,"fp32",path.backend);else ih::layer(g,model[1],prep[1],schedule,master[0],heads[1],false,path.block,64,t,st,0,false,"fp32",path.backend);report(path.name,2,"FP32_same_input_same_contracted_LR_no_ELU",difference(standard[1].out,output(path,1)));}
    for(const auto& path:paths)if(path.kind==5||path.kind==6){std::array<Times,3> bt{};std::array<io::Timing,3> ot{};run(path,bt,ot);std::array<uint64_t,3> hashes{};for(int l=0;l<3;++l)hashes[l]=fingerprint(output(path,l));input=&g.x;for(int l=0;l<3;++l){ih::Timing t;ih::Stats st;if(path.kind==6)ipp::layer(g,model[l],prep[l],schedule,*input,heads[l],l<2,path.block,64,t,st,period,true,"fp32",path.backend);else ih::layer(g,model[l],prep[l],schedule,*input,heads[l],l<2,path.block,64,t,st,period,true,"fp32",path.backend);input=&output(path,l);bool same=hashes[l]==fingerprint(*input);std::cout<<"PROFILE_IDENTITY path="<<path.name<<" layer="<<l+1<<" bit_fingerprint_equal="<<same<<'\n';if(!same)throw std::runtime_error("heads profile changed output");profile(path,l+1,g,model[l],schedule,t,st,heads[l]);}}
    std::cout<<"ICPP_PRELOAD_COMPLETE old_heads_smoke_plus_bitwise_preload_regression=true\n";std::cout<<"ICPP_HEADS_COMPLETE architecture=DegreeSort_TR16_sharedH_PH_AMX_UW_fusion no_global_U_Z_e_alpha=true PH_coefficients=FP32_or_BF16_high_plus_low normalization_FP32=true independent_heads=true bounded_V_in_local_memory=true source_width_amplification_retained=true master_gate=SEE_PER_LAYER_ERRORS performance=EXPLORATORY task_accuracy=UNVERIFIED\n";return 0;
}catch(const std::exception& e){std::cerr<<"ICPP_HEADS_FAIL "<<e.what()<<'\n';return 1;}}
