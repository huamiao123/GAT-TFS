#include "icpp_online.hpp"
#include "icpp_pair.hpp"
#include "icpp_block.hpp"
#include "checks.hpp"
#include "pair_checks.hpp"
#include "block_checks.hpp"
#include "reuse_probe.hpp"
#include "head_ph_probe.hpp"
#include <array>
#include <iomanip>
#include <iostream>
using namespace gat;
namespace io=gat::icpp_online;
namespace ip=gat::icpp_pair;
namespace ib=gat::icpp_block;

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
static void profile(int layer,const Graph& g,const Param& p,const ib::Timing& t,const ib::Stats& st,const ib::Workspace& w,int block) {
    const auto& s=st.baseline;uint64_t blocks=0,updates=0,rescales=0;std::vector<double> ratios;
    ratios.reserve(w.original.row_blocks.size());
    for(size_t at=0;at<w.original.row_blocks.size();++at){auto b=w.original.row_blocks[at];blocks+=b;updates+=w.original.row_max_updates[at];rescales+=w.original.row_rescales[at];ratios.push_back(b?double(w.original.row_rescales[at])/b:0);}
    const uint64_t dp=uint64_t((p.in+31)/32*32),obs=uint64_t((p.dim+15)/16);
    if(blocks!=s.blocks||updates!=s.max_updates||rescales!=s.rescales||s.rescaled_feature_elements!=rescales*uint64_t(p.dim)||s.spill_bytes!=s.reload_bytes||s.spill_bytes!=s.rescale_events*obs*1024||st.actual_input_feature_FMA!=g.e*uint64_t(p.heads)*p.in||st.logical_H_bytes!=g.e*uint64_t(p.heads)*p.in*2||st.conversion_elements!=st.block_projection_calls*16*dp||s.amx_calls!=st.block_projection_calls*(dp/32)*obs||s.executed_fma!=st.block_projection_calls*16*dp*obs*16||st.useful_projection_FMA!=blocks*uint64_t(p.in)*p.dim||s.neighbor_steps!=st.block_projection_calls)throw std::runtime_error("block counter identity failure");
    std::sort(ratios.begin(),ratios.end());auto pct=[&](double f){return ratios.empty()?0:ratios[std::min(ratios.size()-1,size_t(std::ceil(f*ratios.size())-1))];};
    std::cout<<"PROFILE path=ICPP_TFS_BLOCK_B"<<block<<" layer="<<layer<<" layer_wall_ms="<<t.total*1000<<" kernel_wall_ms="<<t.kernel*1000<<" sampled_tiles="<<s.sampled_tiles<<" worker_times_are_sampled_sums=true"
             <<" score_generation_worker_ms="<<s.score_generation*1000<<" block_max_worker_ms="<<s.block_max*1000<<" exp_den_worker_ms="<<s.exp_den*1000
             <<" feature_gather_FP32_SpMM_worker_ms="<<s.source_gather*1000<<" block_BF16_pack_worker_ms="<<s.weighted_convert*1000<<" amx_load_compute_worker_ms="<<s.amx_compute*1000
             <<" rescale_store_worker_ms="<<s.rescale_store*1000<<" rescale_vector_worker_ms="<<s.rescale_vector*1000<<" rescale_reload_worker_ms="<<s.rescale_reload*1000
             <<" final_store_worker_ms="<<s.final_store*1000<<" output_write_worker_ms="<<s.output_write*1000<<" scheduling_worker_ms="<<s.scheduling*1000<<" tile_config_worker_ms="<<s.tile_config*1000<<" combined_staging_span=true combined_amx_load_compute_span=true\n";
    std::cout<<"ONLINE_STATS path=ICPP_TFS_BLOCK_B"<<block<<" layer="<<layer<<" block="<<block<<" heads="<<p.heads<<" D="<<p.in<<" d="<<p.dim
             <<" blocks="<<blocks<<" max_updates_including_initial="<<updates<<" rescales="<<rescales<<" rescale_per_block="<<(blocks?double(rescales)/blocks:0)<<" P50="<<pct(.5)<<" P90="<<pct(.9)<<" P95="<<pct(.95)<<" P99="<<pct(.99)
             <<" rescaled_feature_elements="<<s.rescaled_feature_elements<<" rescaled_padded_feature_elements="<<s.rescaled_padded_feature_elements<<" rescale_events="<<s.rescale_events<<" spill_bytes="<<s.spill_bytes<<" reload_bytes="<<s.reload_bytes<<" amx_calls="<<s.amx_calls<<" executed_fma="<<s.executed_fma<<" total_tiles="<<s.total_tiles
             <<" neighbor_steps="<<s.neighbor_steps<<" neighbor_steps_unit=block_projection_calls\n";
    std::cout<<"BLOCK_WORK layer="<<layer<<" block="<<block<<" actual_input_feature_FMA="<<st.actual_input_feature_FMA<<" padded_input_feature_FMA="<<st.padded_input_feature_FMA<<" useful_projection_FMA="<<st.useful_projection_FMA
             <<" block_projection_calls="<<st.block_projection_calls<<" conversion_elements="<<st.conversion_elements<<" source_neighbor_steps="<<st.source_neighbor_steps<<" logical_H_bytes="<<st.logical_H_bytes
             <<" edgewise_original_AMX_calls="<<st.source_neighbor_steps*(dp/32)*obs<<" TDP_reduction="<<(st.block_projection_calls?double(st.source_neighbor_steps)/st.block_projection_calls:0)<<" original_input_width_amplification="<<double(p.in)/p.dim<<" traffic_is_logical_not_DRAM=true\n";
}
struct Path {std::string name;int kind,block;};
int main(int argc,char** argv) {try {
    if(argc==2&&std::string(argv[1])=="--smoke"){omp_set_dynamic(0);omp_set_num_threads(2);mkl_set_num_threads(2);mkl_set_dynamic(0);io::run_smoke();ip::run_pair_smoke();ib::run_block_smoke();return 0;}
    if(argc!=6)throw std::runtime_error("usage: icpp_block GRAPH THREADS WARMUPS REPEATS PROFILE_PERIOD | --smoke");
    int threads=std::stoi(argv[2]),warm=std::stoi(argv[3]),reps=std::stoi(argv[4]),period=std::stoi(argv[5]);
    if(threads<1||warm<0||reps<1||period<1)throw std::runtime_error("invalid configuration");
    omp_set_dynamic(0);omp_set_num_threads(threads);mkl_set_num_threads(threads);mkl_set_dynamic(0);
    Graph g=load_graph(argv[1]);auto model=random_model(g.din,g.classes,11);auto schedule=degree_schedule(g);
    std::vector<Prepared> prep;std::vector<Workspace> standard(3),b1(3);std::vector<io::Workspace> online(3);std::vector<ip::Workspace> paired(3);std::vector<ib::Workspace> blocked(3);std::vector<std::vector<float>> master(3);
    for(int l=0;l<3;++l){prep.push_back(prepare(model[l]));standard[l].allocate(g,model[l],"standard_bf16");b1[l].allocate(g,model[l],"tfs_bf16");online[l].allocate(g,model[l],false);paired[l].allocate(g,model[l],false);blocked[l].allocate(g,model[l],true);}
    // The post-timing matched-attention control calls lr_reordered, unlike
    // ordinary B0's lr_from_z. Allocate its extra scratch before all timing.
    standard[1].lr.resize(checked(g.n,2*model[1].heads));
    std::vector<Path> paths={{"B0_FP32",0,0},{"B0_BF16",1,0},{"B1_TFS_PREMAX",2,0},{"ICPP_TFS_ONLINE_B32",3,32},{"ICPP_TFS_PAIR_B32",4,32},{"ICPP_TFS_BLOCK_B16",5,16},{"ICPP_TFS_BLOCK_B32",5,32},{"ICPP_TFS_BLOCK_B64",5,64}};
    std::cout<<std::setprecision(12)<<"CONFIG N="<<g.n<<" E="<<g.e<<" Din="<<g.din<<" C="<<g.classes<<" threads="<<threads<<" warmups="<<warm<<" repeats="<<reps<<" degree_sort_ms="<<schedule.degree_sort_s*1000<<" seed=11 untrained=true LR_policy=fp32 panel=64 tile_rows=16 attention_independent=true block_AMX_fusion=true performance=EXPLORATORY task_accuracy=UNVERIFIED\n";
    const std::vector<float>* input=&g.x;
    for(int l=0;l<3;++l){Times t;standard_layer(g,model[l],prep[l],*input,standard[l],l<2,t,true);master[l]=standard[l].out;input=&standard[l].out;std::cout<<"STATIC layer="<<l+1<<" prepare_ms="<<prep[l].weight_prepare_s*1000<<" block_workspace_bytes="<<blocked[l].bytes()<<" full_Z_bytes="<<blocked[l].original.base.z.size()*4<<" edge_score_bytes="<<blocked[l].original.base.score.size()*4<<" alpha_bytes="<<blocked[l].original.base.alpha.size()*4<<"\n";}
    auto output=[&](const Path& path,int l)->const std::vector<float>& {if(path.kind==5)return blocked[l].original.base.out;if(path.kind==4)return paired[l].original.base.out;if(path.kind==3)return online[l].base.out;if(path.kind==2)return b1[l].out;return standard[l].out;};
    auto run=[&](const Path& path,std::array<Times,3>& base_times,std::array<io::Timing,3>& online_times) {
        const std::vector<float>* x=&g.x;
        for(int l=0;l<3;++l){if(path.kind==5){ib::Stats stats;ib::layer(g,model[l],prep[l],schedule,*x,blocked[l],l<2,path.block,64,online_times[l],stats,0,false,"fp32");}
            else if(path.kind==4){ip::Stats stats;ip::layer(g,model[l],prep[l],schedule,*x,paired[l],l<2,path.block,64,online_times[l],stats,0,false,"fp32");}
            else if(path.kind==3){io::Stats stats;io::layer(g,model[l],prep[l],schedule,*x,online[l],l<2,path.block,64,online_times[l],stats,0,false,"fp32");}
            else if(path.kind==2)tfs_layer(g,model[l],prep[l],schedule,*x,b1[l],l<2,64,base_times[l],"fp32");
            else standard_layer(g,model[l],prep[l],*x,standard[l],l<2,base_times[l],path.kind==0);
            x=&output(path,l);
        }
    };
    for(int rep=-warm-1;rep<reps;++rep){auto order=paths;if(rep>=0&&rep%2)std::reverse(order.begin(),order.end());
        for(const auto& path:order){std::array<Times,3> bt{};std::array<io::Timing,3> ot{};auto begin=Clock::now();run(path,bt,ot);double e2e=seconds(begin,Clock::now());
            if(rep==-warm-1)for(int l=0;l<3;++l)report(path.name,l+1,"B0_FP32_own_output_3layer",difference(master[l],output(path,l)));
            if(rep>=0){require_finite(output(path,2),path.name);for(int l=0;l<3;++l){bool on=path.kind>=3;
                std::cout<<"{\"path\":\""<<path.name<<"\",\"rep\":"<<rep<<",\"layer\":"<<l+1<<",\"e2e_ms\":"<<e2e*1000<<",\"layer_ms\":"<<(on?ot[l].total:bt[l].total)*1000<<",\"projection_ms\":"<<bt[l].projection*1000<<",\"conversion_ms\":"<<(on?ot[l].convert:bt[l].convert)*1000<<",\"lr_ms\":"<<(on?ot[l].lr:bt[l].lr)*1000<<",\"max_prescan_ms\":"<<bt[l].max_prescan*1000<<",\"kernel_ms\":"<<(on?ot[l].kernel:bt[l].aggregate)*1000<<",\"normalization_ms\":"<<(on?ot[l].normalize:bt[l].normalize)*1000<<",\"activation_ms\":"<<(on?ot[l].activation:bt[l].activation)*1000<<",\"block\":"<<path.block<<"}\n";
            }}
        }
    }
    // Identical FP32 input and contracted attention isolate new block rounding.
    if(standard[1].lr.size()!=checked(g.n,2*model[1].heads))throw std::runtime_error("matched attention LR scratch shape");
    Times matched_times;standard_layer(g,model[1],prep[1],master[0],standard[1],false,matched_times,true,"fp32");
    for(int b:{16,32,64}){ib::Timing t;ib::Stats st;ib::layer(g,model[1],prep[1],schedule,master[0],blocked[1],false,b,64,t,st,0,false,"fp32");report("ICPP_TFS_BLOCK_B"+std::to_string(b),2,"FP32_same_input_same_contracted_LR_no_ELU",difference(standard[1].out,blocked[1].original.base.out));}
    for(const auto& path:paths)if(path.kind==5) {
        std::array<Times,3> bt{};std::array<io::Timing,3> ot{};run(path,bt,ot);std::array<uint64_t,3> hashes{};
        for(int l=0;l<3;++l)hashes[l]=fingerprint(blocked[l].original.base.out);
        input=&g.x;
        for(int l=0;l<3;++l){ib::Timing t;ib::Stats st;ib::layer(g,model[l],prep[l],schedule,*input,blocked[l],l<2,path.block,64,t,st,period,true,"fp32");input=&blocked[l].original.base.out;
            bool same=hashes[l]==fingerprint(*input);std::cout<<"PROFILE_IDENTITY path="<<path.name<<" layer="<<l+1<<" bit_fingerprint_equal="<<same<<'\n';if(!same)throw std::runtime_error("block profile changed output");profile(l+1,g,model[l],t,st,blocked[l],path.block);}
    }
    probe_graph_reuse(g,schedule,32);
    std::cout<<"HEAD_PH_INPUT source=ICPP_TFS_BLOCK_B64_own_L1_output contracted_LR=fp32 value_storage=native_BF16_H full_model_input=NOT_FP32_master probe_wall_includes_instrumentation_and_OpenMP_startup=1\n";
    ib::probe_head_PH(g,model[1],blocked[1].original.base,schedule);
    std::cout<<"ICPP_BLOCK_COMPLETE architecture=degree_sorted_TR16_block_SpMM_AMX_GeMM_fusion attention_independent=true source_width_amplification_retained=true task_accuracy=UNVERIFIED master_gate=SEE_PER_LAYER_ERRORS performance=EXPLORATORY\n";
    return 0;
}catch(const std::exception& e){std::cerr<<"ICPP_BLOCK_FAIL "<<e.what()<<'\n';return 1;}}
