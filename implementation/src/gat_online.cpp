#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>
#include <immintrin.h>
#include <mkl.h>

using Clock = std::chrono::steady_clock;
static double sec(Clock::time_point a, Clock::time_point b) {
    return std::chrono::duration<double>(b-a).count();
}
static inline Clock::time_point fine_now() {
#ifdef GAT_NO_FINE_TIMING
    return Clock::time_point{};
#else
    return Clock::now();
#endif
}
#ifdef GAT_NO_FINE_TIMING
static constexpr bool kCollectStats=false;
#else
static constexpr bool kCollectStats=true;
#endif
struct Graph {
    uint64_t n=0,e=0; uint32_t din=0,classes=0;
    std::vector<uint64_t> row;
    std::vector<uint32_t> col;
    std::vector<float> x;
};
static size_t checked_size(uint64_t a,uint64_t b,const char* label) {
    if (b && a>std::numeric_limits<size_t>::max()/b)
        throw std::runtime_error(std::string("size overflow: ")+label);
    return size_t(a*b);
}
template<typename T> void read_exact(std::ifstream& f,T* p,size_t n) {
    if (n>size_t(std::numeric_limits<std::streamsize>::max())/sizeof(T))
        throw std::runtime_error("input section too large");
    f.read(reinterpret_cast<char*>(p),sizeof(T)*n);
    if (!f) throw std::runtime_error("truncated input");
}
Graph load_graph(const std::string& path) {
    std::ifstream f(path,std::ios::binary);
    if (!f) throw std::runtime_error("cannot open input");
    char magic[8]; read_exact(f,magic,8);
    if (std::memcmp(magic,"GATBIN1",7)) throw std::runtime_error("bad input magic");
    Graph g;
    read_exact(f,&g.n,1); read_exact(f,&g.e,1);
    read_exact(f,&g.din,1); read_exact(f,&g.classes,1);
    if (!g.n || !g.din || !g.classes ||
        g.din>uint32_t(std::numeric_limits<int>::max()) ||
        g.classes>uint32_t(std::numeric_limits<int>::max()) ||
        g.n>uint64_t(std::numeric_limits<int>::max()) ||
        g.n>uint64_t(std::numeric_limits<uint32_t>::max()) ||
        g.e > uint64_t(std::numeric_limits<int>::max())*4 ||
        g.n==std::numeric_limits<uint64_t>::max())
        throw std::runtime_error("invalid graph dimensions");
    g.row.resize(checked_size(g.n+1,1,"row"));
    g.col.resize(checked_size(g.e,1,"col"));
    g.x.resize(checked_size(g.n,g.din,"features"));
    read_exact(f,g.row.data(),g.row.size());
    read_exact(f,g.col.data(),g.col.size());
    read_exact(f,g.x.data(),g.x.size());
    if (g.row.front()!=0 || g.row.back()!=g.e) throw std::runtime_error("invalid CSR rowptr");
    for (uint64_t i=0;i<g.n;i++) if (g.row[i]>g.row[i+1]) throw std::runtime_error("unsorted rowptr");
    for (auto j:g.col) if (j>=g.n) throw std::runtime_error("bad neighbor id");
    for (float v:g.x) if (!std::isfinite(v)) throw std::runtime_error("nonfinite input feature");
    return g;
}
struct Param {
    int in=0,heads=0,dim=0;
    std::vector<float> w,al,ar;
};
Param make_param(int in,int heads,int dim,int seed) {
    Param p; p.in=in;p.heads=heads;p.dim=dim;
    p.w.resize(size_t(in)*heads*dim);p.al.resize(heads*dim);p.ar.resize(heads*dim);
    std::mt19937 rng(seed);
    std::normal_distribution<float> z(0.0f,1.0f);
    for(auto& v:p.w) v=z(rng)/std::sqrt(float(in));
    for(auto& v:p.al) v=z(rng)/std::sqrt(float(dim));
    for(auto& v:p.ar) v=z(rng)/std::sqrt(float(dim));
    return p;
}
struct Times {
    double projection=0,lr=0,score=0,softmax=0,rescale=0,weighted=0,normalization=0,activation=0,total=0;
};
struct Stats {
    uint64_t blocks=0,max_updates=0,rescales=0,rescaled_elements=0;
    std::vector<float> row_ratio;
    std::vector<uint32_t> row_rescales;
    std::vector<uint32_t> row_blocks,row_max_updates;
};
struct Result {
    std::vector<float> out;
    Times t;
    Stats s;
};
static inline float activate(float x) {return x>=0?x:std::exp(x)-1.0f;}
static inline float leak(float x) {return x>=0?x:0.2f*x;}
static inline double percentile(std::vector<float> v,double q) {
    if(v.empty()) return 0;
    size_t k=std::min(v.size()-1,size_t(std::ceil(q*v.size())-1));
    std::nth_element(v.begin(),v.begin()+k,v.end());return v[k];
}
static inline double percentile_u32(const std::vector<uint32_t>& a,double q) {
    std::vector<float> v(a.begin(),a.end());return percentile(std::move(v),q);
}
void print_result(const std::string& name,int layer,const Graph& g,const Param& p,int block,const Result& r) {
    const auto& t=r.t;const auto& s=r.s;
    std::cout<<std::setprecision(9)
      <<"RESULT path="<<name<<" layer="<<layer<<" nodes="<<g.n<<" edges="<<g.e
      <<" heads="<<p.heads<<" head_dim="<<p.dim<<" block_size="<<block
      <<" projection_s="<<t.projection<<" attention_lr_s="<<t.lr
      <<" edge_score_s="<<t.score<<" online_softmax_s="<<t.softmax
      <<" rescale_s="<<t.rescale<<" softmax_includes_rescale=true"
      <<" stats_enabled="<<kCollectStats<<" weighted_aggregation_s="<<t.weighted
      <<" normalization_s="<<t.normalization<<" activation_s="<<t.activation<<" layer_total_s="<<t.total
      <<" num_blocks="<<s.blocks<<" running_max_updates="<<s.max_updates
      <<" rescale_count="<<s.rescales<<" rescaled_feature_elements="<<s.rescaled_elements
      <<" rescale_per_block="<<(s.blocks?double(s.rescales)/s.blocks:0)
      <<" row_rescale_ratio_p50="<<percentile(s.row_ratio,0.50)
      <<" row_rescale_ratio_p90="<<percentile(s.row_ratio,0.90)
      <<" row_rescale_ratio_p95="<<percentile(s.row_ratio,0.95)
      <<" row_rescale_ratio_p99="<<percentile(s.row_ratio,0.99)
      <<" row_blocks_p50="<<percentile_u32(s.row_blocks,0.50)
      <<" row_blocks_p90="<<percentile_u32(s.row_blocks,0.90)
      <<" row_blocks_p95="<<percentile_u32(s.row_blocks,0.95)
      <<" row_blocks_p99="<<percentile_u32(s.row_blocks,0.99)
      <<" row_max_updates_p50="<<percentile_u32(s.row_max_updates,0.50)
      <<" row_max_updates_p90="<<percentile_u32(s.row_max_updates,0.90)
      <<" row_max_updates_p95="<<percentile_u32(s.row_max_updates,0.95)
      <<" row_max_updates_p99="<<percentile_u32(s.row_max_updates,0.99)
      <<" row_rescale_count_p50="<<percentile_u32(s.row_rescales,0.50)
      <<" row_rescale_count_p90="<<percentile_u32(s.row_rescales,0.90)
      <<" row_rescale_count_p95="<<percentile_u32(s.row_rescales,0.95)
      <<" row_rescale_count_p99="<<percentile_u32(s.row_rescales,0.99)
      <<"\n";
}
void projection_lr(const std::vector<float>& input,uint64_t n,const Param& p,
                   std::vector<float>& z,std::vector<float>& left,std::vector<float>& right,Times& t) {
    auto a=fine_now();
    int width=p.heads*p.dim;
    z.resize(n*width);
    cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,
                int(n),width,p.in,1.0f,input.data(),p.in,p.w.data(),width,0.0f,z.data(),width);
    t.projection=sec(a,fine_now());
    a=fine_now();
    left.assign(n*p.heads,0.0f);right.assign(n*p.heads,0.0f);
    for(uint64_t i=0;i<n;i++) for(int h=0;h<p.heads;h++) {
        const float* v=&z[i*width+h*p.dim];
        const float* al=&p.al[h*p.dim];const float* ar=&p.ar[h*p.dim];
        float l=0,r=0;
        for(int d=0;d<p.dim;d++){l+=v[d]*al[d];r+=v[d]*ar[d];}
        left[i*p.heads+h]=l;right[i*p.heads+h]=r;
    }
    t.lr=sec(a,fine_now());
}
Result reference(const Graph& g,const std::vector<float>& input,const Param& p,bool hidden) {
    Result q;auto start=Clock::now();
    std::vector<float> z,l,r;projection_lr(input,g.n,p,z,l,r,q.t);
    int width=p.heads*p.dim;
    std::vector<float> scores(g.e*p.heads),alpha(g.e*p.heads);
    auto a=fine_now();
    for(uint64_t i=0;i<g.n;i++) for(uint64_t e=g.row[i];e<g.row[i+1];e++)
        for(int h=0;h<p.heads;h++)
            scores[e*p.heads+h]=leak(l[i*p.heads+h]+r[uint64_t(g.col[e])*p.heads+h]);
    q.t.score=sec(a,fine_now());
    a=fine_now();
    for(uint64_t i=0;i<g.n;i++) for(int h=0;h<p.heads;h++) {
        uint64_t b=g.row[i],end=g.row[i+1];
        if(b==end) continue;
        float m=-std::numeric_limits<float>::infinity();
        for(uint64_t e=b;e<end;e++) m=std::max(m,scores[e*p.heads+h]);
        float sum=0;
        for(uint64_t e=b;e<end;e++){float v=std::exp(scores[e*p.heads+h]-m);alpha[e*p.heads+h]=v;sum+=v;}
        for(uint64_t e=b;e<end;e++) alpha[e*p.heads+h]/=sum;
    }
    q.t.softmax=sec(a,fine_now());
    q.out.assign(g.n*width,0.0f);
    a=fine_now();
    for(uint64_t i=0;i<g.n;i++) for(uint64_t e=g.row[i];e<g.row[i+1];e++) {
        const float* val=&z[uint64_t(g.col[e])*width];
        for(int h=0;h<p.heads;h++) {
            float weight=alpha[e*p.heads+h];float* out=&q.out[i*width+h*p.dim];
            for(int d=0;d<p.dim;d++) out[d]+=weight*val[h*p.dim+d];
        }
    }
    q.t.weighted=sec(a,fine_now());
    a=fine_now();
    if(hidden) for(auto& v:q.out) v=activate(v);
    q.t.activation=sec(a,fine_now());
    q.t.total=sec(start,Clock::now());
    return q;
}
struct OnlineState {
    float m=-std::numeric_limits<float>::infinity();
    float l=0;
    float* u;
    OnlineState(int dim,float* workspace):u(workspace) {std::fill(u,u+dim,0.0f);}
};
// The state interface accepts a score/value block. Future tiled backends can use
// several destination rows while retaining independent m, l and U per row/head.
template<bool AVX> __attribute__((noinline)) void online_softmax_update(const float* scores,const uint32_t* neighbors,int count,
                  const float* z,int width,int head,int dim,OnlineState& state,
                  Stats& stats,Times& times) {
    auto a=fine_now();
    float blockmax=-std::numeric_limits<float>::infinity();
    for(int k=0;k<count;k++) blockmax=std::max(blockmax,scores[k]);
    float newmax=std::max(state.m,blockmax);
    bool update=newmax>state.m;
    bool prior=state.l>0.0f;
    if constexpr(kCollectStats) if(update) stats.max_updates++;
    float scale=prior?std::exp(state.m-newmax):0.0f;
    if constexpr(kCollectStats) if(update && prior) {stats.rescales++;stats.rescaled_elements+=dim;}
    if(update && prior) {
        auto rs=fine_now();
        if constexpr (AVX) {
            __m512 f=_mm512_set1_ps(scale);
            int d=0;
            for(;d+16<=dim;d+=16) _mm512_storeu_ps(state.u+d,_mm512_mul_ps(_mm512_loadu_ps(state.u+d),f));
            if(d<dim){__mmask16 mask=(1u<<(dim-d))-1;_mm512_mask_storeu_ps(state.u+d,mask,_mm512_mul_ps(_mm512_maskz_loadu_ps(mask,state.u+d),f));}
        } else for(int d=0;d<dim;d++) state.u[d]*=scale;
        state.l*=scale;
        times.rescale+=sec(rs,fine_now());
    }
    alignas(64) float weights[64];
    if constexpr (AVX) {
        int k=0;__m512 mm=_mm512_set1_ps(newmax);
        for(;k+16<=count;k+=16) _mm512_store_ps(weights+k,_mm512_exp_ps(_mm512_sub_ps(_mm512_loadu_ps(scores+k),mm)));
        if(k<count){__mmask16 mask=(1u<<(count-k))-1;_mm512_mask_storeu_ps(weights+k,mask,_mm512_exp_ps(_mm512_sub_ps(_mm512_maskz_loadu_ps(mask,scores+k),mm)));}
    } else for(int k=0;k<count;k++) weights[k]=std::exp(scores[k]-newmax);
    for(int k=0;k<count;k++) state.l+=weights[k];
    state.m=newmax;
    times.softmax+=sec(a,fine_now());
    a=fine_now();
    for(int k=0;k<count;k++) {
        const float* val=z+uint64_t(neighbors[k])*width+head*dim;
        float weight=weights[k];
        if constexpr (AVX) {
            __m512 w=_mm512_set1_ps(weight);int d=0;
            for(;d+16<=dim;d+=16) _mm512_storeu_ps(state.u+d,_mm512_fmadd_ps(w,_mm512_loadu_ps(val+d),_mm512_loadu_ps(state.u+d)));
            if(d<dim){__mmask16 mask=(1u<<(dim-d))-1;_mm512_mask_storeu_ps(state.u+d,mask,_mm512_fmadd_ps(w,_mm512_maskz_loadu_ps(mask,val+d),_mm512_maskz_loadu_ps(mask,state.u+d)));}
        } else for(int d=0;d<dim;d++) state.u[d]+=weight*val[d];
    }
    times.weighted+=sec(a,fine_now());
}
template<bool AVX> Result online(const Graph& g,const std::vector<float>& input,const Param& p,int block,bool hidden) {
    Result q;auto start=Clock::now();
    std::vector<float> z,l,r;projection_lr(input,g.n,p,z,l,r,q.t);
    int width=p.heads*p.dim;
    q.out.resize(g.n*width);
    if constexpr(kCollectStats) {
        q.s.row_ratio.reserve(g.n*p.heads);
        q.s.row_rescales.reserve(g.n*p.heads);
        q.s.row_blocks.reserve(g.n*p.heads);
        q.s.row_max_updates.reserve(g.n*p.heads);
    }
    std::vector<float> workspace(p.dim);
    alignas(64) float scores[64];
    for(uint64_t i=0;i<g.n;i++) for(int h=0;h<p.heads;h++) {
        OnlineState state(p.dim,workspace.data());
        uint32_t nblocks=0,nrescales=0,nmaxupdates=0;
        for(uint64_t e=g.row[i];e<g.row[i+1];e+=block) {
            int count=int(std::min<uint64_t>(block,g.row[i+1]-e));
            auto a=fine_now();
            if constexpr (AVX) {
                alignas(64) float rv[64];
                for(int k=0;k<count;k++) rv[k]=r[uint64_t(g.col[e+k])*p.heads+h];
                __m512 lv=_mm512_set1_ps(l[i*p.heads+h]);
                __m512 zero=_mm512_setzero_ps();
                __m512 slope=_mm512_set1_ps(0.2f);
                int k=0;
                for(;k+16<=count;k+=16) {
                    __m512 sum=_mm512_add_ps(lv,_mm512_loadu_ps(rv+k));
                    _mm512_store_ps(scores+k,_mm512_mask_mul_ps(sum,_mm512_cmp_ps_mask(sum,zero,_CMP_LT_OQ),sum,slope));
                }
                if(k<count){
                    __mmask16 mask=(1u<<(count-k))-1;
                    __m512 sum=_mm512_add_ps(lv,_mm512_maskz_loadu_ps(mask,rv+k));
                    __m512 evec=_mm512_mask_mul_ps(sum,_mm512_cmp_ps_mask(sum,zero,_CMP_LT_OQ),sum,slope);
                    _mm512_mask_storeu_ps(scores+k,mask,evec);
                }
            } else for(int k=0;k<count;k++) scores[k]=leak(l[i*p.heads+h]+r[uint64_t(g.col[e+k])*p.heads+h]);
            q.t.score+=sec(a,fine_now());
            uint64_t before=0,before_max=0;
            if constexpr(kCollectStats) {before=q.s.rescales;before_max=q.s.max_updates;}
            online_softmax_update<AVX>(scores,g.col.data()+e,count,z.data(),width,h,p.dim,state,q.s,q.t);
            if constexpr(kCollectStats) {
                nrescales+=uint32_t(q.s.rescales-before);
                nmaxupdates+=uint32_t(q.s.max_updates-before_max);
                nblocks++;q.s.blocks++;
            }
        }
        auto a=fine_now();
        float* out=&q.out[i*width+h*p.dim];
        if(state.l>0) for(int d=0;d<p.dim;d++) out[d]=state.u[d]/state.l;
        else for(int d=0;d<p.dim;d++) out[d]=0;
        q.t.normalization+=sec(a,fine_now());
        a=fine_now();
        if(hidden) for(int d=0;d<p.dim;d++) out[d]=activate(out[d]);
        q.t.activation+=sec(a,fine_now());
        if constexpr(kCollectStats) {
            q.s.row_ratio.push_back(nblocks?float(nrescales)/nblocks:0);
            q.s.row_rescales.push_back(nrescales);
            q.s.row_blocks.push_back(nblocks);
            q.s.row_max_updates.push_back(nmaxupdates);
        }
    }
    q.t.total=sec(start,Clock::now());
    return q;
}
struct ErrorMetrics {double max_abs=0,mean_abs=0,relative_l2=0;bool pass=false;};
static ErrorMetrics error_metrics(const std::vector<float>& ref,const std::vector<float>& got) {
    if(ref.empty() || ref.size()!=got.size()) throw std::runtime_error("empty output or output size mismatch");
    double maxe=0,sume=0,num=0,den=0;
    for(size_t i=0;i<ref.size();i++) {
        if(!std::isfinite(ref[i]) || !std::isfinite(got[i]))
            throw std::runtime_error("nonfinite output at index "+std::to_string(i));
        double diff=double(got[i])-ref[i];maxe=std::max(maxe,std::abs(diff));
        sume+=std::abs(diff);num+=diff*diff;den+=double(ref[i])*ref[i];
    }
    ErrorMetrics result{maxe,sume/ref.size(),std::sqrt(num/std::max(den,1e-30)),false};
    result.pass=std::isfinite(result.relative_l2) && result.max_abs<=0.003 && result.relative_l2<=1e-4;
    return result;
}
bool compare(const std::string& name,int layer,const std::vector<float>& ref,const std::vector<float>& got) {
    ErrorMetrics error=error_metrics(ref,got);
    std::cout<<std::setprecision(9)<<"ERROR path="<<name<<" layer="<<layer
             <<" max_abs_error="<<error.max_abs<<" mean_abs_error="<<error.mean_abs
             <<" relative_L2_error="<<error.relative_l2<<"\n";
    return error.pass;
}
#ifndef GAT_EMBEDDED
int main(int argc,char** argv) {
    try {
        if(argc!=4) {std::cerr<<"usage: gat_online GRAPH.gatbin BLOCK(16|32|64) OUTPUT_PREFIX\n";return 2;}
        Graph g=load_graph(argv[1]);
        int block=std::stoi(argv[2]);if(block!=16&&block!=32&&block!=64) throw std::runtime_error("block must be 16, 32 or 64");
        std::string prefix=argv[3];
        std::ofstream sink(prefix+".metadata.txt");
        sink<<"graph="<<argv[1]<<"\nblock="<<block<<"\nnodes="<<g.n<<"\nedges="<<g.e
            <<"\nin_features="<<g.din<<"\nclasses="<<g.classes<<"\n";
        std::vector<Param> ps;
        ps.push_back(make_param(g.din,8,32,11));
        ps.push_back(make_param(256,8,32,22));
        ps.push_back(make_param(256,1,g.classes,33));
        std::vector<float> input=g.x;
        bool pass=true;
        for(int layer=1;layer<=3;layer++) {
            bool hidden=layer<3;const Param& p=ps[layer-1];
            auto ref=reference(g,input,p,hidden);
            auto fp=online<false>(g,input,p,block,hidden);
            auto avx=online<true>(g,input,p,block,hidden);
            print_result("reference",layer,g,p,block,ref);
            print_result("online_fp32",layer,g,p,block,fp);
            print_result("online_avx512",layer,g,p,block,avx);
            pass &= compare("online_fp32",layer,ref.out,fp.out);
            pass &= compare("online_avx512",layer,ref.out,avx.out);
            input=std::move(fp.out);
        }
        std::vector<float> ref_chain=g.x,avx_chain=g.x;
        for(int layer=1;layer<=3;layer++) {
            bool hidden=layer<3;const Param& p=ps[layer-1];
            ref_chain=reference(g,ref_chain,p,hidden).out;
            avx_chain=online<true>(g,avx_chain,p,block,hidden).out;
        }
        pass &= compare("end_to_end_online_fp32",3,ref_chain,input);
        pass &= compare("end_to_end_online_avx512",3,ref_chain,avx_chain);
        std::cout<<"CORRECTNESS "<<(pass?"PASS":"FAIL")<<"\n";
        return pass?0:1;
    } catch(const std::exception& e) {std::cerr<<"ERROR "<<e.what()<<"\n";return 1;}
}


#endif // GAT_EMBEDDED
