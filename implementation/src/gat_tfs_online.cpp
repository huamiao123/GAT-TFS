// FP32 GAT adaptation of the original TFS V3 row-panel schedule.
// Original source: data/yx/TFS/code/amx_tfs_v3.cpp (read only).
// See ../TFS_SOURCE_AUDIT.md. BF16/AMX is intentionally not claimed here.
#define GAT_EMBEDDED
#include "gat_online.cpp"
#include <omp.h>
#include <array>

struct TfsTimes {
    double projection=0, bprep=0, lr=0, wpack=0, score=0, softmax=0, rescale=0;
    double weighted=0, transition=0, gemm=0, normalization=0, activation=0, concat=0;
    double schedule=0, total=0;
};
struct TfsResult {
    std::vector<float> out;
    TfsTimes t;
    Stats s;
    uint64_t global_u_bytes=0;
    uint64_t sparse_elements=0;
};
static void add_stats(Stats& to,const Stats& from) {
    to.blocks+=from.blocks;to.max_updates+=from.max_updates;
    to.rescales+=from.rescales;to.rescaled_elements+=from.rescaled_elements;
    to.row_ratio.insert(to.row_ratio.end(),from.row_ratio.begin(),from.row_ratio.end());
    to.row_rescales.insert(to.row_rescales.end(),from.row_rescales.begin(),from.row_rescales.end());
    to.row_blocks.insert(to.row_blocks.end(),from.row_blocks.begin(),from.row_blocks.end());
    to.row_max_updates.insert(to.row_max_updates.end(),from.row_max_updates.begin(),from.row_max_updates.end());
}
static void add_times(TfsTimes& a,const TfsTimes& b) {
    a.score+=b.score;a.softmax+=b.softmax;a.rescale+=b.rescale;
    a.weighted+=b.weighted;a.transition+=b.transition;a.gemm+=b.gemm;
    a.normalization+=b.normalization;a.activation+=b.activation;a.concat+=b.concat;
}
static void row_stats(Stats& s,uint32_t blocks,uint32_t maxupdates,uint32_t rescales) {
    s.row_blocks.push_back(blocks);s.row_max_updates.push_back(maxupdates);
    s.row_rescales.push_back(rescales);
    s.row_ratio.push_back(blocks?float(rescales)/blocks:0.0f);
}
struct TfsAttention {
    std::vector<float> left,right;
    TfsTimes t;
};
static TfsAttention prepare_attention(const std::vector<float>& input,uint64_t n,const Param& p) {
    TfsAttention a;
    const int D=p.in,K=p.heads,d=p.dim,width=K*d;
    auto tick=fine_now();
    std::vector<float> B(size_t(D)*2*K,0.0f);
    for(int k=0;k<D;k++) for(int h=0;h<K;h++) {
        float bl=0,br=0;
        for(int q=0;q<d;q++) {
            float w=p.w[size_t(k)*width+h*d+q];
            bl+=w*p.al[h*d+q];br+=w*p.ar[h*d+q];
        }
        B[size_t(k)*2*K+h]=bl;B[size_t(k)*2*K+K+h]=br;
    }
    a.t.bprep=sec(tick,fine_now());
    tick=fine_now();
    std::vector<float> LR(size_t(n)*2*K);
    cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,int(n),2*K,D,
                1.0f,input.data(),D,B.data(),2*K,0.0f,LR.data(),2*K);
    a.left.resize(size_t(n)*K);a.right.resize(size_t(n)*K);
    for(uint64_t i=0;i<n;i++) for(int h=0;h<K;h++) {
        a.left[size_t(i)*K+h]=LR[size_t(i)*2*K+h];
        a.right[size_t(i)*K+h]=LR[size_t(i)*2*K+K+h];
    }
    a.t.lr=sec(tick,fine_now());
    return a;
}
static std::vector<float> pack_head_weight(const Param& p,int h,double& elapsed) {
    auto tick=fine_now();
    std::vector<float> w(size_t(p.in)*p.dim);
    for(int k=0;k<p.in;k++)
        std::memcpy(w.data()+size_t(k)*p.dim,
                    p.w.data()+size_t(k)*(p.heads*p.dim)+h*p.dim,
                    sizeof(float)*p.dim);
    elapsed+=sec(tick,fine_now());
    return w;
}
// Same stable one-pass block recurrence as gat_online.cpp, with a caller-owned
// FP32 accumulator so that the fused path keeps a contiguous 16 x D local tile.
static void update_raw(const float* scores,const uint32_t* neighbors,int count,
                       const float* H,int value_stride,int value_offset,int D,float& m,float& l,float* U,
                       Stats& stats,TfsTimes& t) {
    auto tick=fine_now();
    float blockmax=-std::numeric_limits<float>::infinity();
    for(int k=0;k<count;k++) blockmax=std::max(blockmax,scores[k]);
    float next=std::max(m,blockmax);
    bool changed=next>m,prior=l>0.0f;
    if(changed) stats.max_updates++;
    if(changed && prior) {
        auto rs=fine_now();
        float r=std::exp(m-next);
        for(int d=0;d<D;d++) U[d]*=r;
        l*=r;
        stats.rescales++;stats.rescaled_elements+=D;
        t.rescale+=sec(rs,fine_now());
    }
    float p[64];
    for(int k=0;k<count;k++){p[k]=std::exp(scores[k]-next);l+=p[k];}
    m=next;
    t.softmax+=sec(tick,fine_now());
    tick=fine_now();
    for(int k=0;k<count;k++) {
        const float* v=H+size_t(neighbors[k])*value_stride+value_offset;
        float w=p[k];
        for(int d=0;d<D;d++) U[d]+=w*v[d];
    }
    t.weighted+=sec(tick,fine_now());
}
static void block_scores(const Graph& g,const TfsAttention& a,int h,int K,
                         uint64_t row,uint64_t edge,int count,float* scores,TfsTimes& t) {
    auto tick=fine_now();
    float left=a.left[size_t(row)*K+h];
    for(int k=0;k<count;k++)
        scores[k]=leak(left+a.right[size_t(g.col[edge+k])*K+h]);
    t.score+=sec(tick,fine_now());
}
static TfsResult tfs_online_reference(const Graph& g,const std::vector<float>& input,
                                      const Param& p,int block,bool hidden) {
    TfsResult q;auto begin=Clock::now();
    TfsAttention att=prepare_attention(input,g.n,p);
    q.t.bprep=att.t.bprep;q.t.lr=att.t.lr;
    const int D=p.in,K=p.heads,d=p.dim,width=K*d;
    q.out.resize(size_t(g.n)*width);
    std::vector<float> U(size_t(g.n)*D),den(g.n),V(size_t(g.n)*d);
    q.global_u_bytes=uint64_t(g.n)*D*sizeof(float);
    q.sparse_elements=g.e*uint64_t(K)*D;
    alignas(64) float scores[64];
    for(int h=0;h<K;h++) {
        auto W=pack_head_weight(p,h,q.t.wpack);
        std::fill(U.begin(),U.end(),0.0f);
        for(uint64_t i=0;i<g.n;i++) {
            float m=-std::numeric_limits<float>::infinity(),l=0;
            uint32_t blocks=0,maxupdates=0,rescales=0;
            float* u=U.data()+size_t(i)*D;
            for(uint64_t e=g.row[i];e<g.row[i+1];e+=block) {
                int count=int(std::min<uint64_t>(block,g.row[i+1]-e));
                block_scores(g,att,h,K,i,e,count,scores,q.t);
                uint64_t oldm=q.s.max_updates,oldr=q.s.rescales;
                update_raw(scores,g.col.data()+e,count,input.data(),D,0,D,m,l,u,q.s,q.t);
                blocks++;q.s.blocks++;maxupdates+=uint32_t(q.s.max_updates-oldm);
                rescales+=uint32_t(q.s.rescales-oldr);
            }
            den[i]=l;row_stats(q.s,blocks,maxupdates,rescales);
        }
        auto tick=fine_now();
        // U is intentionally materialized for the algebra/dataflow reference.
        cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,int(g.n),d,D,
                    1.0f,U.data(),D,W.data(),d,0.0f,V.data(),d);
        q.t.gemm+=sec(tick,fine_now());
        tick=fine_now();
        for(uint64_t i=0;i<g.n;i++) {
            float inv=den[i]>0?1.0f/den[i]:0.0f;
            for(int j=0;j<d;j++) V[size_t(i)*d+j]*=inv;
        }
        q.t.normalization+=sec(tick,fine_now());
        tick=fine_now();
        if(hidden) for(auto& x:V) x=activate(x);
        q.t.activation+=sec(tick,fine_now());
        tick=fine_now();
        for(uint64_t i=0;i<g.n;i++)
            std::memcpy(q.out.data()+size_t(i)*width+h*d,V.data()+size_t(i)*d,d*sizeof(float));
        q.t.concat+=sec(tick,fine_now());
    }
    q.t.total=sec(begin,Clock::now());
    return q;
}
// Adapted from TFS V3: perm[sorted index] = original destination row.
static std::vector<uint32_t> degree_perm(const Graph& g,double& elapsed) {
    auto begin=Clock::now();
    std::vector<uint32_t> perm(g.n);
    std::iota(perm.begin(),perm.end(),0);
    std::sort(perm.begin(),perm.end(),[&](uint32_t a,uint32_t b){
        uint64_t da=g.row[a+1]-g.row[a],db=g.row[b+1]-g.row[b];
        return da==db?a<b:da<db;
    });
    elapsed=sec(begin,Clock::now());return perm;
}
// Adapted from TFS V3 adaptive-R selector.
static int choose_R(const Graph& g) {
    uint64_t maxdeg=0;
    for(uint64_t i=0;i<g.n;i++) maxdeg=std::max(maxdeg,g.row[i+1]-g.row[i]);
    double avg=double(g.e)/g.n,rho=avg?maxdeg/avg:1.0;
    if(rho<32) return 128;
    if(rho<1024) return 64;
    if(rho<20000) return 32;
    return 16;
}
static TfsResult tfs_online_fused(const Graph& g,const std::vector<float>& input,
                                  const Param& p,int block,bool hidden,
                                  const std::vector<uint32_t>& perm,int R) {
    TfsResult q;auto begin=Clock::now();
    TfsAttention att=prepare_attention(input,g.n,p);
    q.t.bprep=att.t.bprep;q.t.lr=att.t.lr;
    const int D=p.in,K=p.heads,d=p.dim,width=K*d,TR=16;
    q.out.resize(size_t(g.n)*width);
    q.sparse_elements=g.e*uint64_t(K)*D;
    // No N x D U exists. Each OpenMP worker owns only a 16 x D U tile.
    for(int h=0;h<K;h++) {
        auto W=pack_head_weight(p,h,q.t.wpack);
        #pragma omp parallel
        {
            mkl_set_num_threads_local(1);
            std::vector<float> Utile(size_t(TR)*D),Vtile(size_t(TR)*d);
            Stats local_s;TfsTimes local_t;
            alignas(64) float scores[64];
            #pragma omp for schedule(dynamic,1)
            for(int64_t rg=0;rg<int64_t(g.n);rg+=R) {
                int64_t rg_end=std::min<int64_t>(g.n,rg+R);
                for(int64_t pos=rg;pos<rg_end;pos+=TR) {
                    int batch=int(std::min<int64_t>(TR,rg_end-pos));
                    std::fill(Utile.begin(),Utile.begin()+size_t(batch)*D,0.0f);
                    float denom[TR]={};
                    for(int n=0;n<batch;n++) {
                        uint32_t row=perm[pos+n];
                        float m=-std::numeric_limits<float>::infinity(),l=0;
                        uint32_t blocks=0,maxupdates=0,rescales=0;
                        float* U=Utile.data()+size_t(n)*D;
                        for(uint64_t e=g.row[row];e<g.row[row+1];e+=block) {
                            int count=int(std::min<uint64_t>(block,g.row[row+1]-e));
                            block_scores(g,att,h,K,row,e,count,scores,local_t);
                            uint64_t oldm=local_s.max_updates,oldr=local_s.rescales;
                            update_raw(scores,g.col.data()+e,count,input.data(),D,0,D,
                                       m,l,U,local_s,local_t);
                            blocks++;local_s.blocks++;
                            maxupdates+=uint32_t(local_s.max_updates-oldm);
                            rescales+=uint32_t(local_s.rescales-oldr);
                        }
                        denom[n]=l;row_stats(local_s,blocks,maxupdates,rescales);
                    }
                    // Tile-local U is immediately consumed by GeMM, as in TFS
                    // row-panel fusion; no full U matrix is written to memory.
                    auto tick=fine_now();
                    cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,batch,d,D,
                                1.0f,Utile.data(),D,W.data(),d,0.0f,Vtile.data(),d);
                    local_t.gemm+=sec(tick,fine_now());
                    tick=fine_now();
                    for(int n=0;n<batch;n++) {
                        float inv=denom[n]>0?1.0f/denom[n]:0.0f;
                        for(int j=0;j<d;j++) Vtile[size_t(n)*d+j]*=inv;
                    }
                    local_t.normalization+=sec(tick,fine_now());
                    tick=fine_now();
                    if(hidden) for(int n=0;n<batch;n++)
                        for(int j=0;j<d;j++) Vtile[size_t(n)*d+j]=activate(Vtile[size_t(n)*d+j]);
                    local_t.activation+=sec(tick,fine_now());
                    tick=fine_now();
                    for(int n=0;n<batch;n++) {
                        uint32_t row=perm[pos+n];
                        std::memcpy(q.out.data()+size_t(row)*width+h*d,
                                    Vtile.data()+size_t(n)*d,d*sizeof(float));
                    }
                    local_t.concat+=sec(tick,fine_now());
                }
            }
            #pragma omp critical(tfs_merge)
            {add_stats(q.s,local_s);add_times(q.t,local_t);}
            mkl_set_num_threads_local(0);
        }
    }
    q.t.total=sec(begin,Clock::now());
    return q;
}
// Matched scheduling/threading control: same TFS V3 panels and same FP32
// online recurrence, but aggregate Z^h (head_dim) as transform-first GAT.
static TfsResult panel_transform_first(const Graph& g,const std::vector<float>& input,
                                       const Param& p,int block,bool hidden,
                                       const std::vector<uint32_t>& perm,int R) {
    TfsResult q;auto begin=Clock::now();
    const int K=p.heads,d=p.dim,width=K*d,TR=16;
    std::vector<float> Z,left,right;Times proj;
    projection_lr(input,g.n,p,Z,left,right,proj);
    TfsAttention att;att.left=std::move(left);att.right=std::move(right);
    q.t.projection=proj.projection;q.t.lr=proj.lr;
    q.out.resize(size_t(g.n)*width);
    q.sparse_elements=g.e*uint64_t(K)*d;
    for(int h=0;h<K;h++) {
        #pragma omp parallel
        {
            std::vector<float> Utile(size_t(TR)*d);
            Stats local_s;TfsTimes local_t;
            alignas(64) float scores[64];
            #pragma omp for schedule(dynamic,1)
            for(int64_t rg=0;rg<int64_t(g.n);rg+=R) {
                int64_t rg_end=std::min<int64_t>(g.n,rg+R);
                for(int64_t pos=rg;pos<rg_end;pos+=TR) {
                    int batch=int(std::min<int64_t>(TR,rg_end-pos));
                    std::fill(Utile.begin(),Utile.begin()+size_t(batch)*d,0.0f);
                    float denom[TR]={};
                    for(int n=0;n<batch;n++) {
                        uint32_t row=perm[pos+n];
                        float m=-std::numeric_limits<float>::infinity(),l=0;
                        uint32_t blocks=0,maxupdates=0,rescales=0;
                        float* U=Utile.data()+size_t(n)*d;
                        for(uint64_t e=g.row[row];e<g.row[row+1];e+=block) {
                            int count=int(std::min<uint64_t>(block,g.row[row+1]-e));
                            block_scores(g,att,h,K,row,e,count,scores,local_t);
                            uint64_t oldm=local_s.max_updates,oldr=local_s.rescales;
                            update_raw(scores,g.col.data()+e,count,Z.data(),width,h*d,d,
                                       m,l,U,local_s,local_t);
                            blocks++;local_s.blocks++;
                            maxupdates+=uint32_t(local_s.max_updates-oldm);
                            rescales+=uint32_t(local_s.rescales-oldr);
                        }
                        denom[n]=l;row_stats(local_s,blocks,maxupdates,rescales);
                    }
                    auto tick=fine_now();
                    for(int n=0;n<batch;n++) {
                        float inv=denom[n]>0?1.0f/denom[n]:0.0f;
                        for(int j=0;j<d;j++) Utile[size_t(n)*d+j]*=inv;
                    }
                    local_t.normalization+=sec(tick,fine_now());
                    tick=fine_now();
                    if(hidden) for(int n=0;n<batch;n++)
                        for(int j=0;j<d;j++) Utile[size_t(n)*d+j]=activate(Utile[size_t(n)*d+j]);
                    local_t.activation+=sec(tick,fine_now());
                    tick=fine_now();
                    for(int n=0;n<batch;n++) {
                        uint32_t row=perm[pos+n];
                        std::memcpy(q.out.data()+size_t(row)*width+h*d,
                                    Utile.data()+size_t(n)*d,d*sizeof(float));
                    }
                    local_t.concat+=sec(tick,fine_now());
                }
            }
            #pragma omp critical(control_merge)
            {add_stats(q.s,local_s);add_times(q.t,local_t);}
        }
    }
    q.t.total=sec(begin,Clock::now());
    return q;
}
static void print_control(int layer,const Graph& g,const Param& p,int block,int R,const TfsResult& q) {
    const auto& t=q.t;const auto& s=q.s;
    std::cout<<std::setprecision(9)
      <<"CONTROL_RESULT path=panel_transform_first_fp32 layer="<<layer
      <<" nodes="<<g.n<<" edges="<<g.e<<" heads="<<p.heads<<" D="<<p.in
      <<" head_dim="<<p.dim<<" block_size="<<block<<" panel_R="<<R
      <<" projection_s="<<t.projection<<" LR_s="<<t.lr
      <<" score_worker_s="<<t.score<<" softmax_worker_s="<<t.softmax
      <<" rescale_worker_s="<<t.rescale<<" weighted_worker_s="<<t.weighted
      <<" normalization_worker_s="<<t.normalization
      <<" activation_worker_s="<<t.activation<<" concat_worker_s="<<t.concat
      <<" sparse_feature_elements="<<q.sparse_elements
      <<" num_blocks="<<s.blocks<<" rescale_count="<<s.rescales
      <<" layer_total_s="<<t.total<<"\n";
}
static void print_tfs(const char* path,int layer,const Graph& g,const Param& p,int block,
                      int R,const TfsResult& q) {
    const auto& t=q.t;const auto& s=q.s;
    std::cout<<std::setprecision(9)
     <<"TFS_RESULT path="<<path<<" layer="<<layer<<" nodes="<<g.n<<" edges="<<g.e
     <<" heads="<<p.heads<<" D="<<p.in<<" head_dim="<<p.dim<<" block_size="<<block
     <<" panel_R="<<R<<" row_tile=16"
     <<" bL_bR_prep_s="<<t.bprep<<" LR_s="<<t.lr<<" W_pack_s="<<t.wpack
     <<" edge_score_worker_s="<<t.score<<" online_softmax_worker_s="<<t.softmax
     <<" rescale_worker_s="<<t.rescale<<" weighted_spmm_worker_s="<<t.weighted
     <<" spmm_gemm_transition_worker_s="<<t.transition<<" gemm_worker_s="<<t.gemm
     <<" normalization_worker_s="<<t.normalization<<" activation_worker_s="<<t.activation
     <<" head_concat_worker_s="<<t.concat<<" schedule_s="<<t.schedule
     <<" layer_total_s="<<t.total
     <<" sparse_feature_elements="<<q.sparse_elements
     <<" transform_first_sparse_elements="<<g.e*uint64_t(p.heads)*p.dim
     <<" amplification="<<double(p.in)/p.dim
     <<" full_U_materialized_bytes="<<q.global_u_bytes
     <<" full_U_logical_write_read_bytes="<<q.global_u_bytes*2*p.heads
     <<" num_blocks="<<s.blocks<<" running_max_updates="<<s.max_updates
     <<" rescale_count="<<s.rescales<<" rescaled_feature_elements="<<s.rescaled_elements
     <<" rescale_per_block="<<(s.blocks?double(s.rescales)/s.blocks:0)
     <<" row_rescale_ratio_p50="<<percentile(s.row_ratio,0.50)
     <<" row_rescale_ratio_p90="<<percentile(s.row_ratio,0.90)
     <<" row_rescale_ratio_p95="<<percentile(s.row_ratio,0.95)
     <<" row_rescale_ratio_p99="<<percentile(s.row_ratio,0.99)
     <<" row_rescale_count_p50="<<percentile_u32(s.row_rescales,0.50)
     <<" row_rescale_count_p90="<<percentile_u32(s.row_rescales,0.90)
     <<" row_rescale_count_p95="<<percentile_u32(s.row_rescales,0.95)
     <<" row_rescale_count_p99="<<percentile_u32(s.row_rescales,0.99)
     <<"\n";
}
static bool acceptable(const std::vector<float>& ref,const std::vector<float>& got) {
    double maxe=0,num=0,den=0;
    for(size_t i=0;i<ref.size();i++){
        double e=double(got[i])-ref[i];maxe=std::max(maxe,std::abs(e));
        num+=e*e;den+=double(ref[i])*ref[i];
    }
    return maxe<=0.003 && std::sqrt(num/std::max(den,1e-30))<=1e-4;
}
static void attention_equivalence(const Graph& g,const std::vector<float>& input,
                                  const Param& p,int layer) {
    std::vector<float> z,l,r;Times t;
    projection_lr(input,g.n,p,z,l,r,t);
    auto a=prepare_attention(input,g.n,p);
    double max_lr=0,sum_lr=0;
    for(size_t i=0;i<l.size();i++) {
        double dl=std::abs(double(l[i])-a.left[i]),dr=std::abs(double(r[i])-a.right[i]);
        max_lr=std::max(max_lr,std::max(dl,dr));sum_lr+=dl+dr;
    }
    std::cout<<"ATTENTION_EQ layer="<<layer<<" max_abs_LR="<<max_lr
             <<" mean_abs_LR="<<sum_lr/(2*l.size())<<"\n";
}
static std::vector<Param> model(const Graph& g) {
    return {make_param(g.din,8,32,11),make_param(256,8,32,22),
            make_param(256,1,g.classes,33)};
}
int main(int argc,char** argv) {
    try {
        if(argc<4 || argc>5) {
            std::cerr<<"usage: gat_tfs_online GRAPH.gatbin BLOCK(16|32|64) MODE(profile|speed) [R=auto|16|32|64|128]\n";
            return 2;
        }
        Graph g=load_graph(argv[1]);
        int block=std::stoi(argv[2]);
        if(block!=16&&block!=32&&block!=64) throw std::runtime_error("invalid block size");
        std::string mode=argv[3];
        if(mode!="profile"&&mode!="speed") throw std::runtime_error("invalid mode");
        int R=(argc==5 && std::string(argv[4])!="auto")?std::stoi(argv[4]):choose_R(g);
        if(R!=16&&R!=32&&R!=64&&R!=128) throw std::runtime_error("invalid panel R");
        double schedule=0;auto perm=degree_perm(g,schedule);
        auto ps=model(g);
        std::cout<<"RUN mode="<<mode<<" block="<<block<<" R="<<R
                 <<" omp_threads="<<omp_get_max_threads()
                 <<" schedule_prep_s="<<schedule<<"\n";
        std::vector<float> common=g.x;
        bool pass=true;
        double totals[6]={};
        for(int layer=1;layer<=3;layer++) {
            const Param& p=ps[layer-1];bool hidden=layer<3;
            auto begin=Clock::now();
            auto ref=reference(g,common,p,hidden);
            auto fp=online<false>(g,common,p,block,hidden);
            auto avx=online<true>(g,common,p,block,hidden);
            auto tfsref=tfs_online_reference(g,common,p,block,hidden);
            auto fused=tfs_online_fused(g,common,p,block,hidden,perm,R);
            auto control=panel_transform_first(g,common,p,block,hidden,perm,R);
            double full=sec(begin,Clock::now());
            if(mode=="profile") attention_equivalence(g,common,p,layer);
            print_result("reference",layer,g,p,block,ref);
            print_result("online_fp32",layer,g,p,block,fp);
            print_result("online_avx512",layer,g,p,block,avx);
            print_tfs("tfs_online_reference",layer,g,p,block,R,tfsref);
            print_tfs("tfs_online_fused",layer,g,p,block,R,fused);
            print_control(layer,g,p,block,R,control);
            compare("online_fp32",layer,ref.out,fp.out);
            compare("online_avx512",layer,ref.out,avx.out);
            compare("tfs_online_reference",layer,ref.out,tfsref.out);
            compare("tfs_online_fused",layer,ref.out,fused.out);
            compare("panel_transform_first_fp32",layer,ref.out,control.out);
            pass &= acceptable(ref.out,fp.out)&&acceptable(ref.out,avx.out)
                 &&acceptable(ref.out,tfsref.out)&&acceptable(ref.out,fused.out)
                 &&acceptable(ref.out,control.out);
            totals[0]+=ref.t.total;totals[1]+=fp.t.total;totals[2]+=avx.t.total;
            totals[3]+=tfsref.t.total;totals[4]+=fused.t.total;totals[5]+=control.t.total;
            std::cout<<"LAYER_COMPARE layer="<<layer<<" all_paths_wall_s="<<full<<"\n";
            common=std::move(ref.out);
        }
        const char* names[]={"reference","online_fp32","online_avx512","tfs_online_reference","tfs_online_fused","panel_transform_first_fp32"};
        for(int path=0;path<6;path++) {
            auto e2e_begin=Clock::now();
            std::vector<float> H=g.x;
            for(int layer=1;layer<=3;layer++) {
                const Param& p=ps[layer-1];bool hidden=layer<3;
                if(path==0) H=reference(g,H,p,hidden).out;
                else if(path==1) H=online<false>(g,H,p,block,hidden).out;
                else if(path==2) H=online<true>(g,H,p,block,hidden).out;
                else if(path==3) H=tfs_online_reference(g,H,p,block,hidden).out;
                else if(path==4) H=tfs_online_fused(g,H,p,block,hidden,perm,R).out;
                else H=panel_transform_first(g,H,p,block,hidden,perm,R).out;
            }
            if(path==0) {
                common=H;
            } else {
                compare(std::string("end_to_end_")+names[path],3,common,H);
                pass &= acceptable(common,H);
            }
            std::cout<<"E2E path="<<names[path]<<" total_s="<<sec(e2e_begin,Clock::now())
                     <<" layer_time_sum_s="<<totals[path]
                     <<" schedule_prep_s="<<schedule<<"\n";
        }
        std::cout<<"CORRECTNESS "<<(pass?"PASS":"FAIL")<<"\n";
        return pass?0:1;
    } catch(const std::exception& e) {std::cerr<<"ERROR "<<e.what()<<"\n";return 1;}
}

