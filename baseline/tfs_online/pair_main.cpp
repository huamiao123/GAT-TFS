#include "icpp_online.hpp"
#include "icpp_pair.hpp"
#include "checks.hpp"
#include "pair_checks.hpp"
#include <array>
#include <iomanip>
#include <iostream>
using namespace gat;
namespace io=gat::icpp_online;
namespace ip=gat::icpp_pair;

static Error difference(const std::vector<float>& ref,const std::vector<float>& actual){
    if(ref.empty()||ref.size()!=actual.size())throw std::runtime_error("comparison shape");
    double mx=0,ab=0,sq=0,norm=0;int bad=0;
    #pragma omp parallel for reduction(max:mx) reduction(+:ab,sq,norm) reduction(|:bad)
    for(size_t i=0;i<ref.size();++i){bad|=!std::isfinite(ref[i])||!std::isfinite(actual[i]);double d=double(actual[i])-ref[i];mx=std::max(mx,std::abs(d));ab+=std::abs(d);sq+=d*d;norm+=double(ref[i])*ref[i];}
    Error e;e.finite=!bad;e.max_abs=mx;e.mean_abs=ab/ref.size();e.relative_l2=norm?std::sqrt(sq/norm):(sq?INFINITY:0);if(bad)e.max_abs=e.mean_abs=e.relative_l2=INFINITY;return e;
}
static void report(const std::string& path,int layer,const std::string& ref,const Error& e){
    std::cout<<std::setprecision(12)<<"CHECK path="<<path<<" layer="<<layer<<" reference="<<ref<<" max_abs_error="<<e.max_abs<<" mean_abs_error="<<e.mean_abs<<" relative_L2_error="<<e.relative_l2<<" finite="<<e.finite<<" original_abs_003_gate="<<(e.finite&&e.max_abs<=.003?"PASS":"FAIL")<<" task_accuracy=UNVERIFIED\n";
    if(!e.finite)throw std::runtime_error("nonfinite output");
}
template<class T> static bool bit_equal(const std::vector<T>& a,const std::vector<T>& b){return a.size()==b.size()&&(a.empty()||std::memcmp(a.data(),b.data(),a.size()*sizeof(T))==0);}
static void require_identity(const io::Workspace& a,const ip::Workspace& b,int layer,const char* scope){
    const bool out=bit_equal(a.base.out,b.original.base.out),den=bit_equal(a.base.den,b.original.base.den),max=bit_equal(a.base.max,b.original.base.max);
    std::cout<<"PAIR_IDENTITY layer="<<layer<<" scope="<<scope<<" output_bitwise_equal="<<out<<" denominator_bitwise_equal="<<den<<" max_bitwise_equal="<<max<<"\n";
    if(!out||!den||!max)throw std::runtime_error("pair changed original Online arithmetic");
}
static uint64_t fingerprint(const std::vector<float>& x){uint64_t out=0;
    #pragma omp parallel for reduction(^:out)
    for(size_t i=0;i<x.size();++i){uint32_t bits;std::memcpy(&bits,x.data()+i,4);uint64_t z=uint64_t(bits)^((i+1)*0x9e3779b97f4a7c15ULL);z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;out^=z^(z>>27);}return out;
}
static void profile(int layer,const Graph& g,const Param& p,const ip::Timing& t,const ip::Stats& st,const ip::Workspace& w,int block){
    const auto& s=st.baseline;bool pair=p.heads%2==0&&p.dim<=32;uint64_t groups=pair?p.heads/2:p.heads;
    if(st.raw_H_vectors!=g.e*groups*uint64_t((p.in+15)/16)||st.logical_raw_H_bytes!=g.e*groups*uint64_t(p.in)*2||st.source_index_loads!=g.e*groups)throw std::runtime_error("pair logical read counter mismatch");
    uint64_t blocks=0,updates=0,rescales=0;std::vector<double> ratios;ratios.reserve(w.original.row_blocks.size());
    for(size_t i=0;i<w.original.row_blocks.size();++i){auto b=w.original.row_blocks[i];blocks+=b;updates+=w.original.row_max_updates[i];rescales+=w.original.row_rescales[i];ratios.push_back(b?double(w.original.row_rescales[i])/b:0);}
    if(blocks!=s.blocks||updates!=s.max_updates||rescales!=s.rescales||s.rescaled_feature_elements!=rescales*uint64_t(p.dim)||s.spill_bytes!=s.reload_bytes||s.spill_bytes!=s.rescale_events*uint64_t((p.dim+15)/16)*1024)throw std::runtime_error("pair state/staging counter mismatch");
    std::sort(ratios.begin(),ratios.end());auto pct=[&](double f){return ratios.empty()?0:ratios[std::min(ratios.size()-1,size_t(std::ceil(f*ratios.size())-1))];};
    std::cout<<std::setprecision(12)<<"PROFILE path=ICPP_TFS_PAIR_B"<<block<<" layer="<<layer<<" layer_wall_ms="<<t.total*1000<<" kernel_wall_ms="<<t.kernel*1000<<" sampled_tiles="<<s.sampled_tiles<<" sampled_tile_unit=row_tile_head measured_work_unit="<<(pair?"head_pair":"single_head")<<" worker_times_are_sampled_sums=true"
      <<" source_gather_worker_ms="<<s.source_gather*1000<<" weighted_convert_worker_ms="<<s.weighted_convert*1000<<" tile_load_worker_ms="<<s.tile_load*1000<<" amx_compute_worker_ms="<<s.amx_compute*1000
      <<" feature_staging_worker_ms="<<(s.source_gather+s.weighted_convert)*1000<<" amx_load_compute_worker_ms="<<(s.tile_load+s.amx_compute)*1000
      <<" score_generation_worker_ms="<<s.score_generation*1000<<" block_max_worker_ms="<<s.block_max*1000<<" exp_den_worker_ms="<<s.exp_den*1000
      <<" rescale_store_worker_ms="<<s.rescale_store*1000<<" rescale_vector_worker_ms="<<s.rescale_vector*1000<<" rescale_reload_worker_ms="<<s.rescale_reload*1000
      <<" final_store_worker_ms="<<s.final_store*1000<<" output_write_worker_ms="<<s.output_write*1000<<" scheduling_worker_ms="<<s.scheduling*1000<<" tile_config_worker_ms="<<s.tile_config*1000
      <<" combined_staging_span="<<pair<<" combined_amx_load_compute_span="<<pair<<"\n";
    std::cout<<"ONLINE_STATS path=ICPP_TFS_PAIR_B"<<block<<" layer="<<layer<<" block="<<block<<" heads="<<p.heads<<" D="<<p.in<<" d="<<p.dim<<" blocks="<<blocks<<" max_updates_including_initial="<<updates<<" rescales="<<rescales<<" rescale_per_block="<<(blocks?double(rescales)/blocks:0)<<" P50="<<pct(.5)<<" P90="<<pct(.9)<<" P95="<<pct(.95)<<" P99="<<pct(.99)
      <<" rescaled_feature_elements="<<s.rescaled_feature_elements<<" rescaled_padded_feature_elements="<<s.rescaled_padded_feature_elements<<" rescale_events="<<s.rescale_events<<" spill_bytes="<<s.spill_bytes<<" reload_bytes="<<s.reload_bytes<<" amx_calls="<<s.amx_calls<<" neighbor_steps="<<s.neighbor_steps<<" executed_fma="<<s.executed_fma<<" total_tiles="<<s.total_tiles<<"\n";
    std::cout<<"PAIR_ACCESS layer="<<layer<<" optimized="<<pair<<" groups="<<groups<<" raw_H_vectors="<<st.raw_H_vectors<<" logical_raw_H_bytes="<<st.logical_raw_H_bytes<<" source_index_loads="<<st.source_index_loads<<" source_index_scope=score_gather_excludes_prefetch single_head_logical_raw_H_bytes="<<g.e*uint64_t(p.heads)*uint64_t(p.in)*2<<" traffic_is_logical_not_DRAM=true\n";
}
struct Path{std::string name;int kind;}; // 0 FP32,1 BF16,2 premax B1,3 Online,4 pair
int main(int argc,char** argv){try{
    if(argc==2&&std::string(argv[1])=="--smoke"){omp_set_dynamic(0);omp_set_num_threads(2);mkl_set_num_threads(2);mkl_set_dynamic(0);io::run_smoke();ip::run_pair_smoke();return 0;}
    if(argc!=7)throw std::runtime_error("usage: icpp_pair GRAPH THREADS WARMUPS REPEATS BLOCK PROFILE_PERIOD | --smoke");
    int threads=std::stoi(argv[2]),warm=std::stoi(argv[3]),reps=std::stoi(argv[4]),block=std::stoi(argv[5]),period=std::stoi(argv[6]);
    if(threads<1||warm<0||reps<1||block<1||block>64||period<1)throw std::runtime_error("invalid configuration");
    omp_set_dynamic(0);omp_set_num_threads(threads);mkl_set_num_threads(threads);mkl_set_dynamic(0);
    Graph g=load_graph(argv[1]);auto model=random_model(g.din,g.classes,11);auto sched=degree_schedule(g);std::vector<Prepared> prep;
    std::vector<Workspace> standard(3),b1(3);std::vector<io::Workspace> online(3);std::vector<ip::Workspace> pair(3);std::vector<std::vector<float>> ref(3);
    for(int l=0;l<3;++l){prep.push_back(prepare(model[l]));standard[l].allocate(g,model[l],"standard_bf16");b1[l].allocate(g,model[l],"tfs_bf16");online[l].allocate(g,model[l],false);pair[l].allocate(g,model[l],true);}
    const std::vector<Path> paths={{"B0_FP32",0},{"B0_BF16",1},{"B1_TFS_PREMAX",2},{"ICPP_TFS_ONLINE_B"+std::to_string(block),3},{"ICPP_TFS_PAIR_B"+std::to_string(block),4}};
    std::cout<<std::setprecision(12)<<"CONFIG N="<<g.n<<" E="<<g.e<<" Din="<<g.din<<" C="<<g.classes<<" threads="<<threads<<" warmups="<<warm<<" repeats="<<reps<<" block="<<block<<" degree_sort_ms="<<sched.degree_sort_s*1000<<" seed=11 untrained=true LR_policy=fp32 panel=64 tile_rows=16 attention_independent=true neighbor_AMX_fusion=true performance=EXPLORATORY task_accuracy=UNVERIFIED\n";
    const std::vector<float>* input=&g.x;
    for(int l=0;l<3;++l){Times t;standard_layer(g,model[l],prep[l],*input,standard[l],l<2,t,true);ref[l]=standard[l].out;input=&standard[l].out;
        std::cout<<"STATIC layer="<<l+1<<" prepare_ms="<<prep[l].weight_prepare_s*1000<<" pair_workspace_bytes="<<pair[l].bytes()<<" full_Z_bytes="<<pair[l].original.base.z.size()*4<<" edge_score_bytes="<<pair[l].original.base.score.size()*4<<" alpha_bytes="<<pair[l].original.base.alpha.size()*4<<"\n";}
    // Strong exact control before timing, using independent own-output chains.
    const std::vector<float>* a=&g.x;const std::vector<float>* b=&g.x;
    for(int l=0;l<3;++l){io::Timing at,bt;io::Stats as;ip::Stats bs;
        io::layer(g,model[l],prep[l],sched,*a,online[l],l<2,block,64,at,as,0,false,"fp32");
        ip::layer(g,model[l],prep[l],sched,*b,pair[l],l<2,block,64,bt,bs,0,false,"fp32");
        require_identity(online[l],pair[l],l+1,"three_layer_own_outputs");a=&online[l].base.out;b=&pair[l].original.base.out;}
    auto output=[&](const Path& path,int l)->const std::vector<float>&{return path.kind==4?pair[l].original.base.out:path.kind==3?online[l].base.out:path.kind==2?b1[l].out:standard[l].out;};
    auto run=[&](const Path& path,std::array<Times,3>& bt,std::array<io::Timing,3>& ot){const std::vector<float>* x=&g.x;
        for(int l=0;l<3;++l){if(path.kind==4){ip::Stats st;ip::layer(g,model[l],prep[l],sched,*x,pair[l],l<2,block,64,ot[l],st,0,false,"fp32");}
            else if(path.kind==3){io::Stats st;io::layer(g,model[l],prep[l],sched,*x,online[l],l<2,block,64,ot[l],st,0,false,"fp32");}
            else if(path.kind==2)tfs_layer(g,model[l],prep[l],sched,*x,b1[l],l<2,64,bt[l],"fp32");
            else standard_layer(g,model[l],prep[l],*x,standard[l],l<2,bt[l],path.kind==0);x=&output(path,l);}};
    for(int rep=-warm-1;rep<reps;++rep){auto order=paths;if(rep>=0&&rep%2)std::reverse(order.begin(),order.end());
        for(const auto& path:order){std::array<Times,3> bt{};std::array<io::Timing,3> ot{};auto start=Clock::now();run(path,bt,ot);double e2e=seconds(start,Clock::now());
            if(rep==-warm-1)for(int l=0;l<3;++l)report(path.name,l+1,"B0_FP32_own_output_3layer",difference(ref[l],output(path,l)));
            if(rep>=0){require_finite(output(path,2),path.name);for(int l=0;l<3;++l){bool is_online=path.kind>=3;
                std::cout<<"{\"path\":\""<<path.name<<"\",\"rep\":"<<rep<<",\"layer\":"<<l+1<<",\"e2e_ms\":"<<e2e*1000<<",\"layer_ms\":"<<(is_online?ot[l].total:bt[l].total)*1000<<",\"projection_ms\":"<<bt[l].projection*1000<<",\"conversion_ms\":"<<(is_online?ot[l].convert:bt[l].convert)*1000<<",\"lr_ms\":"<<(is_online?ot[l].lr:bt[l].lr)*1000<<",\"max_prescan_ms\":"<<bt[l].max_prescan*1000<<",\"kernel_ms\":"<<(is_online?ot[l].kernel:bt[l].aggregate)*1000<<",\"normalization_ms\":"<<(is_online?ot[l].normalize:bt[l].normalize)*1000<<",\"activation_ms\":"<<(is_online?ot[l].activation:bt[l].activation)*1000<<",\"block\":"<<(is_online?block:0)<<"}\n";}}}}
    std::array<Times,3> bt{};std::array<io::Timing,3> ot{};run(paths.back(),bt,ot);std::array<uint64_t,3> hashes{};for(int l=0;l<3;++l)hashes[l]=fingerprint(pair[l].original.base.out);
    input=&g.x;for(int l=0;l<3;++l){ip::Timing t;ip::Stats st;ip::layer(g,model[l],prep[l],sched,*input,pair[l],l<2,block,64,t,st,period,true,"fp32");input=&pair[l].original.base.out;
        bool equal=hashes[l]==fingerprint(*input);std::cout<<"PROFILE_IDENTITY path=ICPP_TFS_PAIR layer="<<l+1<<" bit_fingerprint_equal="<<equal<<"\n";if(!equal)throw std::runtime_error("pair profile changed output");profile(l+1,g,model[l],t,st,pair[l],block);}
    std::cout<<"ICPP_PAIR_COMPLETE architecture=original_TFS_neighbor_AMX_fusion_preserved source_gather=head_pair_reuse task_accuracy=UNVERIFIED master_gate=SEE_PER_LAYER_ERRORS performance=EXPLORATORY\n";return 0;
}catch(const std::exception& e){std::cerr<<"ICPP_PAIR_FAIL "<<e.what()<<"\n";return 1;}}
