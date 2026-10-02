#include "icpp_online.hpp"
#include "checks.hpp"
#include <array>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
using namespace gat;
namespace io = gat::icpp_online;

static Error difference(const std::vector<float>& a,const std::vector<float>& b) {
    if(a.empty()||a.size()!=b.size())throw std::runtime_error("comparison shape");
    double mx=0,ab=0,sq=0,norm=0;int bad=0;
    #pragma omp parallel for reduction(max:mx) reduction(+:ab,sq,norm) reduction(|:bad)
    for(size_t i=0;i<a.size();++i){bad|=!std::isfinite(a[i])||!std::isfinite(b[i]);double d=double(b[i])-a[i];mx=std::max(mx,std::abs(d));ab+=std::abs(d);sq+=d*d;norm+=double(a[i])*a[i];}
    Error e;e.finite=!bad;e.max_abs=mx;e.mean_abs=ab/a.size();e.relative_l2=norm?std::sqrt(sq/norm):(sq?INFINITY:0);
    if(bad)e.max_abs=e.mean_abs=e.relative_l2=INFINITY;return e;
}
static void report(const std::string& path,int layer,const std::string& ref,const Error& e) {
    std::cout<<std::setprecision(12)<<"CHECK path="<<path<<" layer="<<layer<<" reference="<<ref<<" max_abs_error="<<e.max_abs<<" mean_abs_error="<<e.mean_abs<<" relative_L2_error="<<e.relative_l2<<" finite="<<e.finite<<" original_abs_003_gate="<<(e.finite&&e.max_abs<=.003?"PASS":"FAIL")<<" task_accuracy=UNVERIFIED\n";
    if(!e.finite)throw std::runtime_error("nonfinite comparison");
}
static uint64_t fingerprint(const std::vector<float>& x){uint64_t out=0;
    #pragma omp parallel for reduction(^:out)
    for(size_t i=0;i<x.size();++i){uint32_t bits;std::memcpy(&bits,x.data()+i,4);uint64_t z=uint64_t(bits)^((i+1)*0x9e3779b97f4a7c15ULL);z=(z^(z>>30))*0xbf58476d1ce4e5b9ULL;out^=z^(z>>27);}return out;}

// Diagnostics only: complete neighborhoods, including the global maximum-degree
// row. FP64 AH then W is an oracle, never a measured implementation path.
static void selected_oracle(const Graph& g,const Param& p,const Prepared& q,
                            const std::vector<float>& input,const io::Workspace& w,int layer){
    std::set<uint32_t> selected={0,uint32_t(g.n/2),uint32_t(g.n-1)};
    uint32_t largest=0;uint64_t degree=0;
    for(uint64_t i=0;i<g.n;++i)if(g.row[i+1]-g.row[i]>degree){degree=g.row[i+1]-g.row[i];largest=uint32_t(i);}
    selected.insert(largest);
    for(uint64_t target:{16ULL,32ULL,64ULL,128ULL})for(uint64_t i=0;i<g.n;++i)if(g.row[i+1]-g.row[i]==target){selected.insert(uint32_t(i));break;}
    std::vector<float> master,quantized_data,actual;uint64_t edges=0;
    for(uint32_t row:selected){edges+=g.row[row+1]-g.row[row];
        for(int h=0;h<p.heads;++h){double m=-INFINITY;std::vector<double> u(p.in,0),uq(p.in,0);double den=0;
            for(uint64_t e=g.row[row];e<g.row[row+1];++e){float s=leak(w.base.left[size_t(row)*p.heads+h]+w.base.right[size_t(g.col[e])*p.heads+h]);m=std::max(m,double(s));}
            for(uint64_t e=g.row[row];e<g.row[row+1];++e){uint32_t src=g.col[e];float s=leak(w.base.left[size_t(row)*p.heads+h]+w.base.right[size_t(src)*p.heads+h]);double weight=std::exp(double(s)-m);den+=weight;
                for(int k=0;k<p.in;++k){u[k]+=weight*double(input[size_t(src)*p.in+k]);uq[k]+=weight*double(expand(w.base.xbf[size_t(src)*p.in+k]));}}
            for(int d=0;d<p.dim;++d){double a=0,b=0;for(int k=0;k<p.in;++k){size_t idx=size_t(k)*p.width()+h*p.dim+d;a+=u[k]*double(p.w[idx]);b+=uq[k]*double(expand(q.w[idx]));}
                master.push_back(float(den>0?a/den:0));quantized_data.push_back(float(den>0?b/den:0));actual.push_back(w.base.out[size_t(row)*p.width()+h*p.dim+d]);}
        }
    }
    std::cout<<"ORACLE_COVERAGE layer="<<layer<<" rows="<<selected.size()<<" edges="<<edges<<" global_max_degree_node="<<largest<<" global_max_degree="<<degree<<" global_max_degree_included=true complete_neighborhoods=true activation=false outputs_rounded_to_fp32=true\n";
    report("ICPP_ONLINE_FIXED_INPUT",layer,"FP64_master_same_LR_selected_rows",difference(master,actual));
    report("ICPP_ONLINE_FIXED_INPUT",layer,"FP64_quantized_H_W_same_LR_without_pH_quantization",difference(quantized_data,actual));
}

static void print_profile(int layer,int block,const io::Timing& t,const io::Stats& s,const io::Workspace& w,const Param& p){
    std::cout<<std::setprecision(12)<<"PROFILE path=ICPP_TFS_ONLINE_B"<<block<<" layer="<<layer<<" layer_wall_ms="<<t.total*1000<<" kernel_wall_ms="<<t.kernel*1000<<" sampled_tiles="<<s.sampled_tiles<<" worker_times_are_sampled_sums=true"
      <<" score_generation_worker_ms="<<s.score_generation*1000<<" block_max_worker_ms="<<s.block_max*1000<<" exp_den_worker_ms="<<s.exp_den*1000
      <<" rescale_store_worker_ms="<<s.rescale_store*1000<<" rescale_vector_worker_ms="<<s.rescale_vector*1000<<" rescale_reload_worker_ms="<<s.rescale_reload*1000
      <<" source_gather_worker_ms="<<s.source_gather*1000<<" weighted_convert_worker_ms="<<s.weighted_convert*1000
      <<" tile_load_worker_ms="<<s.tile_load*1000<<" amx_compute_worker_ms="<<s.amx_compute*1000<<" final_store_worker_ms="<<s.final_store*1000<<" output_write_worker_ms="<<s.output_write*1000
      <<" scheduling_worker_ms="<<s.scheduling*1000<<" tile_config_worker_ms="<<s.tile_config*1000<<"\n";
    std::vector<double> ratios;ratios.reserve(w.row_blocks.size());uint64_t nb=0,nu=0,nr=0;
    for(size_t i=0;i<w.row_blocks.size();++i){nb+=w.row_blocks[i];nu+=w.row_max_updates[i];nr+=w.row_rescales[i];ratios.push_back(w.row_blocks[i]?double(w.row_rescales[i])/w.row_blocks[i]:0);}
    std::sort(ratios.begin(),ratios.end());auto pct=[&](double f){return ratios.empty()?0:ratios[std::min(ratios.size()-1,size_t(std::ceil(f*ratios.size())-1))];};
    if(nb!=s.blocks||nu!=s.max_updates||nr!=s.rescales)throw std::runtime_error("row/worker counter mismatch");
    std::cout<<"ONLINE_STATS layer="<<layer<<" block="<<block<<" heads="<<p.heads<<" D="<<p.in<<" d="<<p.dim<<" blocks="<<nb<<" max_updates_including_initial="<<nu<<" rescales="<<nr<<" rescale_per_block="<<(nb?double(nr)/nb:0)
      <<" P50="<<pct(.5)<<" P90="<<pct(.9)<<" P95="<<pct(.95)<<" P99="<<pct(.99)<<" rescaled_feature_elements="<<s.rescaled_feature_elements<<" expected_elements="<<nr*uint64_t(p.dim)
      <<" rescaled_padded_feature_elements="<<s.rescaled_padded_feature_elements<<" rescale_events="<<s.rescale_events
      <<" spill_bytes="<<s.spill_bytes<<" reload_bytes="<<s.reload_bytes<<" row_scale_load_bytes="<<s.rescaled_padded_feature_elements*4<<" row_scale_store_bytes="<<s.rescaled_padded_feature_elements*4
      <<" amx_calls="<<s.amx_calls<<" neighbor_steps="<<s.neighbor_steps<<" executed_fma="<<s.executed_fma<<" total_tiles="<<s.total_tiles<<"\n";
    if(s.rescaled_feature_elements!=nr*uint64_t(p.dim))throw std::runtime_error("rescale dim is not output dim");
    if(s.spill_bytes!=s.reload_bytes||s.spill_bytes!=s.rescale_events*uint64_t((p.dim+15)/16)*1024)throw std::runtime_error("physical spill byte mismatch");
}
struct Path {std::string name;int block=0;};
int main(int argc,char** argv){try{
    if(argc==2&&std::string(argv[1])=="--smoke"){omp_set_dynamic(0);omp_set_num_threads(2);mkl_set_num_threads(2);mkl_set_dynamic(0);io::run_smoke();return 0;}
    if(argc!=7)throw std::runtime_error("usage: icpp_online GRAPH THREADS WARMUPS REPEATS BLOCKS_CSV PROFILE_PERIOD | --smoke");
    const int threads=std::stoi(argv[2]),warm=std::stoi(argv[3]),reps=std::stoi(argv[4]),period=std::stoi(argv[6]);
    if(threads<1||warm<0||reps<1||period<1)throw std::runtime_error("invalid config");
    std::vector<Path> paths={{"B0_FP32",0},{"B0_BF16",0},{"B1_TFS_PREMAX",0}};std::stringstream csv(argv[5]);std::string word;
    while(std::getline(csv,word,',')){int block=std::stoi(word);if(block<1||block>64)throw std::runtime_error("invalid block");paths.push_back({"ICPP_TFS_ONLINE_B"+std::to_string(block),block});}
    if(paths.size()==3)throw std::runtime_error("no Online path");
    omp_set_dynamic(0);omp_set_num_threads(threads);mkl_set_num_threads(threads);mkl_set_dynamic(0);
    auto g=load_graph(argv[1]);auto model=random_model(g.din,g.classes,11);auto sched=degree_schedule(g);
    std::vector<Prepared> prep;std::vector<Workspace> standard(3),b1(3);std::vector<io::Workspace> online(3);std::vector<std::vector<float>> ref(3);
    for(int l=0;l<3;++l){prep.push_back(prepare(model[l]));standard[l].allocate(g,model[l],"standard_bf16");b1[l].allocate(g,model[l],"tfs_bf16");online[l].allocate(g,model[l],true);}
    std::cout<<std::setprecision(12)<<"CONFIG N="<<g.n<<" E="<<g.e<<" Din="<<g.din<<" C="<<g.classes<<" threads="<<threads<<" seed=11 untrained=true warmups="<<warm<<" repeats="<<reps<<" degree_sort_ms="<<sched.degree_sort_s*1000<<" panel=64 tile_rows=16 LR_policy=fp32 online_max_prescan=false output_accumulator=TMM output_rescale_dim=d task_accuracy=UNVERIFIED performance=EXPLORATORY\n";
    const std::vector<float>* x=&g.x;
    for(int l=0;l<3;++l){Times t;standard_layer(g,model[l],prep[l],*x,standard[l],l<2,t,true);ref[l]=standard[l].out;x=&standard[l].out;
        std::cout<<"STATIC layer="<<l+1<<" prepare_ms="<<prep[l].weight_prepare_s*1000<<" online_workspace_bytes="<<online[l].bytes()<<" global_Z_bytes="<<online[l].base.z.size()*4<<" global_edge_score_bytes="<<online[l].base.score.size()*4<<" global_alpha_bytes="<<online[l].base.alpha.size()*4<<"\n";}
    // Separate common-input controls, never inside the three-layer timer.
    for(int l=0;l<3;++l){const auto& input=l?ref[l-1]:g.x;Times bt;tfs_layer(g,model[l],prep[l],sched,input,b1[l],false,64,bt,"fp32");
        for(const auto& path:paths)if(path.block){auto& w=online[l];convert(input.data(),w.base.xbf.data(),input.size());w.base.left=b1[l].left;w.base.right=b1[l].right;io::Stats st;
            io::aggregate(g,model[l],prep[l],sched,w,path.block,64,st);Times ft;finish(g,model[l],w.base,false,ft);
            report(path.name,l+1,"B1_PREMAX_same_input_same_LR",difference(b1[l].out,w.base.out));
            if(path.block==paths.back().block)selected_oracle(g,model[l],prep[l],input,w,l+1);
        }
    }
    auto run=[&](const Path& path,std::array<Times,3>& bt,std::array<io::Timing,3>& ot,int sample,bool counters){
        const std::vector<float>* input=&g.x;
        for(int l=0;l<3;++l){if(path.block){io::Stats st;io::layer(g,model[l],prep[l],sched,*input,online[l],l<2,path.block,64,ot[l],st,sample,counters,"fp32");input=&online[l].base.out;}
            else if(path.name=="B1_TFS_PREMAX"){tfs_layer(g,model[l],prep[l],sched,*input,b1[l],l<2,64,bt[l],"fp32");input=&b1[l].out;}
            else{standard_layer(g,model[l],prep[l],*input,standard[l],l<2,bt[l],path.name=="B0_FP32");input=&standard[l].out;}}
    };
    for(int rep=-warm-1;rep<reps;++rep){auto order=paths;if(rep>=0&&rep%2)std::reverse(order.begin(),order.end());
        for(const auto& path:order){std::array<Times,3> bt{};std::array<io::Timing,3> ot{};auto begin=Clock::now();run(path,bt,ot,0,false);double e2e=seconds(begin,Clock::now());
            if(rep==-warm-1)for(int l=0;l<3;++l)report(path.name,l+1,"B0_FP32_own_output_3layer",difference(ref[l],path.block?online[l].base.out:path.name=="B1_TFS_PREMAX"?b1[l].out:standard[l].out));
            if(rep>=0){const auto& final=path.block?online[2].base.out:path.name=="B1_TFS_PREMAX"?b1[2].out:standard[2].out;require_finite(final,path.name);
                for(int l=0;l<3;++l)std::cout<<"{\"path\":\""<<path.name<<"\",\"rep\":"<<rep<<",\"layer\":"<<l+1<<",\"e2e_ms\":"<<e2e*1000<<",\"layer_ms\":"<<(path.block?ot[l].total:bt[l].total)*1000<<",\"projection_ms\":"<<bt[l].projection*1000<<",\"conversion_ms\":"<<(path.block?ot[l].convert:bt[l].convert)*1000<<",\"lr_ms\":"<<(path.block?ot[l].lr:bt[l].lr)*1000<<",\"max_prescan_ms\":"<<bt[l].max_prescan*1000<<",\"kernel_ms\":"<<(path.block?ot[l].kernel:bt[l].aggregate)*1000<<",\"normalization_ms\":"<<(path.block?ot[l].normalize:bt[l].normalize)*1000<<",\"activation_ms\":"<<(path.block?ot[l].activation:bt[l].activation)*1000<<",\"block\":"<<path.block<<"}\n";
            }
        }
    }
    for(const auto& path:paths)if(path.block){std::array<Times,3> bt{};std::array<io::Timing,3> ot{};run(path,bt,ot,0,false);std::array<uint64_t,3> hashes{};for(int l=0;l<3;++l)hashes[l]=fingerprint(online[l].base.out);
        x=&g.x;for(int l=0;l<3;++l){io::Timing t;io::Stats st;io::layer(g,model[l],prep[l],sched,*x,online[l],l<2,path.block,64,t,st,period,true,"fp32");x=&online[l].base.out;
            bool equal=hashes[l]==fingerprint(*x);std::cout<<"PROFILE_IDENTITY layer="<<l+1<<" block="<<path.block<<" bit_fingerprint_equal="<<equal<<"\n";if(!equal)throw std::runtime_error("profile output changed");print_profile(l+1,path.block,t,st,online[l],model[l]);}}
    std::cout<<"ICPP_ONLINE_COMPLETE architecture=original_TFS_neighbor_AMX_fusion_preserved master_gate=SEE_PER_LAYER_ERRORS task_accuracy=UNVERIFIED performance=EXPLORATORY\n";return 0;
}catch(const std::exception& e){std::cerr<<"ICPP_ONLINE_FAIL "<<e.what()<<"\n";return 1;}}
