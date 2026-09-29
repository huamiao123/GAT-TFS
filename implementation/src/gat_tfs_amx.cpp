// Experimental full three-layer aggregate-first path with real AMX-BF16
// weighted aggregation and FP32 local GeMM. Not a validated speedup path.
#define GAT_TFS_EMBEDDED
#include "gat_tfs_online.cpp"
#include "amx_head_row.hpp"
#include <exception>

struct AmxTimes {
    double bprep=0,lr=0,wpack=0,score_worker=0,gather_worker=0;
    double amx_worker=0,merge_worker=0,gemm_worker=0,norm_worker=0,total=0,parallel_wall=0;
    double panel_zero_worker=0,merge_factor_worker=0,merge_accum_worker=0,merge_denom_worker=0;
    double merge_accum_rescale_worker=0,merge_accum_norescale_worker=0;
    double normalization_worker=0,activation_worker=0,scatter_worker=0;
    double block_setup_worker=0,block_score_max_worker=0,block_weight_exp_bf16_worker=0;
    double feature_zero_worker=0,feature_bf16_pack_worker=0,output_extract_worker=0,tile_release_worker=0;
    double pack_worker=0,config_worker=0,load_worker=0,compute_worker=0,store_worker=0;
    uint64_t blocks=0,logical_rescales=0,pack_bytes=0,tile_load_bytes=0,tile_store_bytes=0;
    uint64_t base_accumulator_bytes=0;
};
struct AmxResult {std::vector<float> out;AmxTimes t;};

static AmxResult amx_head_row_layer(const Graph& g,const std::vector<float>& input,
                                    const Param& p,int block,bool hidden,
                                    const std::vector<uint32_t>& perm,int R) {
    if(block!=16 && block!=32) throw std::invalid_argument("AMX block must be 16 or 32");
    if(p.heads<1 || p.heads>8 || p.in<1 || p.dim<1)
        throw std::invalid_argument("AMX shape outside current contract");
    AmxResult q;auto begin=Clock::now();
    auto att=prepare_attention(input,g.n,p);
    q.t.bprep=att.t.bprep;q.t.lr=att.t.lr;
    const int K=p.heads,D=p.in,d=p.dim,width=K*d;
    q.out.resize(checked_size(g.n,width,"AMX output"));
    std::vector<std::vector<float>> W(K);
    for(int h=0;h<K;h++) W[h]=pack_head_weight(p,h,q.t.wpack);
    std::exception_ptr failure;

    auto parallel_begin=Clock::now();
    #pragma omp parallel
    {
        mkl_set_num_threads_local(1);
        AmxTimes local;
        std::vector<float> U(size_t(K)*R*D);
        std::vector<float> X(size_t(block)*D);
        std::vector<float> scores(size_t(K)*block);
        std::vector<float> V(size_t(R)*d);
        std::vector<double> denom(size_t(R)*K,0);
        #pragma omp for schedule(dynamic,1)
        for(int64_t pos=0;pos<int64_t(g.n);pos+=R) {
          try {
            int batch=int(std::min<int64_t>(R,int64_t(g.n)-pos));
            auto panel_tick=fine_now();
            for(int h=0;h<K;h++)
                std::fill(U.begin()+size_t(h)*R*D,
                          U.begin()+size_t(h)*R*D+size_t(batch)*D,0.0f);
            local.panel_zero_worker+=sec(panel_tick,fine_now());
            for(int n=0;n<batch;n++) {
                uint32_t row=perm[pos+n];
                double running_max[8],running_den[8]{};
                for(int h=0;h<K;h++) running_max[h]=-std::numeric_limits<double>::infinity();
                for(uint64_t e=g.row[row];e<g.row[row+1];e+=block) {
                    int count=int(std::min<uint64_t>(block,g.row[row+1]-e));
                    auto tick=fine_now();
                    for(int h=0;h<K;h++) for(int k=0;k<count;k++)
                        scores[size_t(h)*count+k]=leak(att.left[size_t(row)*K+h]
                            +att.right[size_t(g.col[e+k])*K+h]);
                    local.score_worker+=sec(tick,fine_now());
                    tick=fine_now();
                    for(int k=0;k<count;k++)
                        std::memcpy(X.data()+size_t(k)*D,
                                    input.data()+size_t(g.col[e+k])*D,size_t(D)*sizeof(float));
                    local.gather_worker+=sec(tick,fine_now());
                    tick=fine_now();
                    auto b=gat_amx::head_as_row_block(scores.data(),X.data(),K,count,D,D,true);
                    local.amx_worker+=sec(tick,fine_now());
                    local.blocks++;
                    local.pack_bytes+=b.pack_bytes;
                    local.tile_load_bytes+=b.tile_load_bytes;
                    local.tile_store_bytes+=b.tile_store_bytes;
                    local.pack_worker+=b.packing_s;local.config_worker+=b.tile_config_s;
                    local.load_worker+=b.tile_load_s;local.compute_worker+=b.tile_compute_s;
                    local.store_worker+=b.tile_store_s;
                    local.block_setup_worker+=b.setup_s;
                    local.block_score_max_worker+=b.score_max_s;
                    local.block_weight_exp_bf16_worker+=b.weight_exp_bf16_s;
                    local.feature_zero_worker+=b.feature_zero_s;
                    local.feature_bf16_pack_worker+=b.feature_bf16_pack_s;
                    local.output_extract_worker+=b.output_extract_s;
                    local.tile_release_worker+=b.tile_release_s;
                    tick=fine_now();
                    for(int h=0;h<K;h++) {
                        float* u=U.data()+size_t(h)*R*D+size_t(n)*D;
                        const float* part=b.numerator.data()+size_t(h)*D;
                        double bm=b.reference[h];
                        if(!std::isfinite(running_max[h])) {
                            running_max[h]=bm;
                            running_den[h]=b.denominator[h];
                            auto subtick=fine_now();
                            std::memcpy(u,part,size_t(D)*sizeof(float));
                            local.merge_accum_worker+=sec(subtick,fine_now());
                            local.base_accumulator_bytes+=uint64_t(D)*sizeof(float);
                        } else {
                            auto subtick=fine_now();
                            double next=std::max(running_max[h],bm);
                            double old_factor=std::exp(running_max[h]-next);
                            double new_factor=std::exp(bm-next);
                            local.merge_factor_worker+=sec(subtick,fine_now());
                            bool rescaled=next>running_max[h];
                            if(rescaled) local.logical_rescales++;
                            // Multiply in FP64 before rounding to FP32; this avoids
                            // losing a representable product when the factor is tiny.
                            subtick=fine_now();
                            for(int f=0;f<D;f++)
                                u[f]=float(double(u[f])*old_factor+double(part[f])*new_factor);
                            double merge_elapsed=sec(subtick,fine_now());
                            local.merge_accum_worker+=merge_elapsed;
                            if(rescaled) local.merge_accum_rescale_worker+=merge_elapsed;
                            else local.merge_accum_norescale_worker+=merge_elapsed;
                            local.base_accumulator_bytes+=uint64_t(D)*sizeof(float)*2;
                            subtick=fine_now();
                            running_den[h]=running_den[h]*old_factor
                                           +double(b.denominator[h])*new_factor;
                            running_max[h]=next;
                            local.merge_denom_worker+=sec(subtick,fine_now());
                        }
                    }
                    local.merge_worker+=sec(tick,fine_now());
                }
                for(int h=0;h<K;h++) denom[size_t(n)*K+h]=running_den[h];
            }
            for(int h=0;h<K;h++) {
                auto tick=fine_now();
                cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,batch,d,D,
                    1.0f,U.data()+size_t(h)*R*D,D,W[h].data(),d,0.0f,V.data(),d);
                local.gemm_worker+=sec(tick,fine_now());
                tick=fine_now();
                for(int n=0;n<batch;n++) {
                    double den=denom[size_t(n)*K+h];
                    if(den<0 || !std::isfinite(den)) throw std::runtime_error("invalid AMX denominator");
                    float inv=den>0?float(1.0/den):0.0f;
                    for(int f=0;f<d;f++) V[size_t(n)*d+f]*=inv;
                }
                local.normalization_worker+=sec(tick,fine_now());
                tick=fine_now();
                if(hidden) for(int n=0;n<batch;n++) for(int f=0;f<d;f++)
                    V[size_t(n)*d+f]=activate(V[size_t(n)*d+f]);
                local.activation_worker+=sec(tick,fine_now());
                tick=fine_now();
                for(int n=0;n<batch;n++) {
                    uint32_t row=perm[pos+n];
                    std::memcpy(q.out.data()+size_t(row)*width+h*d,
                                V.data()+size_t(n)*d,size_t(d)*sizeof(float));
                }
                local.scatter_worker+=sec(tick,fine_now());
                local.norm_worker=local.normalization_worker+local.activation_worker+local.scatter_worker;
            }
          } catch(...) {
              #pragma omp critical(amx_error)
              {if(!failure) failure=std::current_exception();}
          }
        }
        #pragma omp critical(amx_times)
        {
            q.t.score_worker+=local.score_worker;q.t.gather_worker+=local.gather_worker;
            q.t.amx_worker+=local.amx_worker;q.t.merge_worker+=local.merge_worker;
            q.t.gemm_worker+=local.gemm_worker;q.t.norm_worker+=local.norm_worker;
            q.t.panel_zero_worker+=local.panel_zero_worker;
            q.t.merge_factor_worker+=local.merge_factor_worker;
            q.t.merge_accum_worker+=local.merge_accum_worker;
            q.t.merge_accum_rescale_worker+=local.merge_accum_rescale_worker;
            q.t.merge_accum_norescale_worker+=local.merge_accum_norescale_worker;
            q.t.merge_denom_worker+=local.merge_denom_worker;
            q.t.normalization_worker+=local.normalization_worker;
            q.t.activation_worker+=local.activation_worker;
            q.t.scatter_worker+=local.scatter_worker;
            q.t.pack_worker+=local.pack_worker;q.t.config_worker+=local.config_worker;
            q.t.load_worker+=local.load_worker;q.t.compute_worker+=local.compute_worker;
            q.t.store_worker+=local.store_worker;
            q.t.block_setup_worker+=local.block_setup_worker;
            q.t.block_score_max_worker+=local.block_score_max_worker;
            q.t.block_weight_exp_bf16_worker+=local.block_weight_exp_bf16_worker;
            q.t.feature_zero_worker+=local.feature_zero_worker;
            q.t.feature_bf16_pack_worker+=local.feature_bf16_pack_worker;
            q.t.output_extract_worker+=local.output_extract_worker;
            q.t.tile_release_worker+=local.tile_release_worker;
            q.t.blocks+=local.blocks;q.t.logical_rescales+=local.logical_rescales;
            q.t.pack_bytes+=local.pack_bytes;q.t.tile_load_bytes+=local.tile_load_bytes;
            q.t.tile_store_bytes+=local.tile_store_bytes;
            q.t.base_accumulator_bytes+=local.base_accumulator_bytes;
        }
        mkl_set_num_threads_local(0);
    }
    q.t.parallel_wall=sec(parallel_begin,Clock::now());
    if(failure) std::rethrow_exception(failure);
    q.t.total=sec(begin,Clock::now());
    return q;
}

int main(int argc,char** argv) {
    try {
        if(argc<3 || argc>5) {
            std::cerr<<"usage: gat_tfs_amx GRAPH.gatbin BLOCK(16|32) [PANEL_R=16|32|64|128] [verify|benchmark]\n";
            return 2;
        }
        int block=std::stoi(argv[2]);
        if(block!=16 && block!=32) throw std::invalid_argument("invalid AMX block");
        Graph g=load_graph(argv[1]);
        int R=argc>=4?std::stoi(argv[3]):choose_R(g);
        if(R!=16 && R!=32 && R!=64 && R!=128) throw std::invalid_argument("invalid panel");
        const std::string mode=argc==5?argv[4]:"profile";
        if(mode!="profile" && mode!="verify" && mode!="benchmark")
            throw std::invalid_argument("invalid mode");
        bool verify=mode=="verify",benchmark=mode=="benchmark";
        if(!gat_amx::available_and_permitted()) {
            std::cerr<<"AMX_UNAVAILABLE: CPU support or OS permission missing\n";
            return 3;
        }
        double schedule=0;auto perm=degree_perm(g,schedule);
        auto ps=model(g);
        std::vector<float> H=g.x;
        bool all_layers_pass=true;
        auto start=Clock::now();
        for(int layer=1;layer<=3;layer++) {
            auto result=amx_head_row_layer(g,H,ps[layer-1],block,layer<3,perm,R);
            if(verify) {
                auto layer_ref=reference(g,H,ps[layer-1],layer<3);
                auto layer_error=error_metrics(layer_ref.out,result.out);
                all_layers_pass &= layer_error.pass;
                std::cout<<"AMX_LAYER_VERIFY layer="<<layer
                         <<" max_abs="<<layer_error.max_abs
                         <<" mean_abs="<<layer_error.mean_abs
                         <<" relative_l2="<<layer_error.relative_l2
                         <<" fp32_tolerance_pass="<<layer_error.pass<<"\n";
            }
            H=std::move(result.out);
            const auto& t=result.t;
            if(!benchmark) std::cout<<std::setprecision(9)<<"AMX_LAYER layer="<<layer
                     <<" backend=amx_bf16_head_as_row precision=bf16_hi_lo"
                     <<" layer_wall_s="<<t.total
                     <<" parallel_wall_s="<<t.parallel_wall
                     <<" bprep_s="<<t.bprep<<" LR_s="<<t.lr<<" W_pack_s="<<t.wpack
                     <<" panel_zero_worker_s="<<t.panel_zero_worker
                     <<" score_worker_s="<<t.score_worker
                     <<" gather_worker_s="<<t.gather_worker
                     <<" amx_block_inclusive_worker_s="<<t.amx_worker
                     <<" packing_worker_s="<<t.pack_worker
                     <<" block_setup_worker_s="<<t.block_setup_worker
                     <<" block_score_max_worker_s="<<t.block_score_max_worker
                     <<" block_weight_exp_bf16_worker_s="<<t.block_weight_exp_bf16_worker
                     <<" feature_zero_worker_s="<<t.feature_zero_worker
                     <<" feature_bf16_pack_worker_s="<<t.feature_bf16_pack_worker
                     <<" tile_config_worker_s="<<t.config_worker
                     <<" tile_load_worker_s="<<t.load_worker
                     <<" tile_compute_worker_s="<<t.compute_worker
                     <<" tile_store_worker_s="<<t.store_worker
                     <<" output_extract_worker_s="<<t.output_extract_worker
                     <<" tile_release_worker_s="<<t.tile_release_worker
                     <<" online_block_merge_worker_s="<<t.merge_worker
                     <<" merge_factor_worker_s="<<t.merge_factor_worker
                     <<" merge_accum_worker_s="<<t.merge_accum_worker
                     <<" merge_accum_rescale_worker_s="<<t.merge_accum_rescale_worker
                     <<" merge_accum_norescale_worker_s="<<t.merge_accum_norescale_worker
                     <<" merge_denom_worker_s="<<t.merge_denom_worker
                     <<" local_gemm_worker_s="<<t.gemm_worker
                     <<" normalization_activation_worker_s="<<t.norm_worker
                     <<" normalization_worker_s="<<t.normalization_worker
                     <<" activation_worker_s="<<t.activation_worker
                     <<" output_scatter_worker_s="<<t.scatter_worker
                     <<" blocks="<<t.blocks<<" logical_rescales="<<t.logical_rescales
                     <<" base_accumulator_logical_bytes="<<t.base_accumulator_bytes
                     <<" extra_rescale_pass_bytes=0"
                     <<" pack_bytes="<<t.pack_bytes<<" tile_load_bytes="<<t.tile_load_bytes
                     <<" tile_store_bytes="<<t.tile_store_bytes<<"\n";
        }
        // In benchmark mode the timer ends before checksum and any output.
        // Profile/verify mode prints per-layer diagnostics inside this interval.
        double forward_s=sec(start,Clock::now());
        double checksum=std::accumulate(H.begin(),H.end(),0.0);
        std::cout<<(benchmark?"AMX_BENCHMARK forward_only_s=":"AMX_E2E diagnostic_s=")<<forward_s
                 <<" schedule_prep_s="<<schedule<<" checksum="<<checksum
                 <<" numeric_contract="<<(verify?"fp32_smoke_checked":"requires_separate_verification")<<"\n";
        if(verify) {
            std::vector<float> ref=g.x;
            for(int layer=1;layer<=3;layer++)
                ref=reference(g,ref,ps[layer-1],layer<3).out;
            auto error=error_metrics(ref,H);
            std::cout<<"AMX_VERIFY fp32_reference_max_abs="<<error.max_abs
                     <<" fp32_reference_mean_abs="<<error.mean_abs
                     <<" fp32_reference_relative_l2="<<error.relative_l2
                     <<" fp32_tolerance_pass="<<error.pass<<"\n";
            return error.pass && all_layers_pass?0:4;
        }
        return 0;
    } catch(const std::exception& e) {std::cerr<<"ERROR "<<e.what()<<"\n";return 1;}
}
