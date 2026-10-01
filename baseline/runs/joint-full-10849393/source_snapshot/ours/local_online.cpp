// Independent candidate. Original B0/B1 remain unchanged.
// Degree permutation, TR16 and dynamic row panels reuse original TFS scheduling.
// New execution order: block online FP32 U in worker-local memory, then one UW.
#include "local_online.hpp"
#include <atomic>
#include <iostream>
#include <iomanip>
#include <sys/syscall.h>
#include <unistd.h>
namespace gat::local {
void WorkspaceLocal::allocate(const Graph& g,const Param& p,bool counters){
    lr.resize(checked(g.n,2*p.heads));out.resize(checked(g.n,p.width()));
    size_t threads=omp_get_max_threads(),dp=(p.in+31)/32*32;
    u.resize(threads*16*dp);c.resize(threads*16*64);uh.resize(u.size());ul.resize(u.size());
    if(counters){size_t n=checked(g.n,p.heads);blocks.resize(n);updates.resize(n);rescales.resize(n);}
}
PreparedLocal prepare_local(const Param& p){
    auto a=Clock::now();PreparedLocal q;q.base=prepare(p);q.wh.resize(p.w.size());q.low.assign(q.base.packed.size(),0);
    for(int h=0;h<p.heads;++h)for(int k=0;k<p.in;++k)for(int d=0;d<p.dim;++d)
        q.wh[(size_t(h)*p.in+k)*p.dim+d]=p.w[size_t(k)*p.width()+h*p.dim+d];
    for(int h=0;h<p.heads;++h)for(int kb=0;kb<q.base.padded_in/32;++kb)for(int ob=0;ob<q.base.output_blocks;++ob)
        for(int kp=0;kp<16;++kp)for(int n=0;n<16;++n)for(int pair=0;pair<2;++pair){
            int k=kb*32+kp*2+pair,d=ob*16+n;
            size_t ix=(((size_t(h)*(q.base.padded_in/32)+kb)*q.base.output_blocks+ob)*512)+kp*32+n*2+pair;
            if(k<p.in&&d<p.dim){size_t wi=size_t(k)*p.width()+h*p.dim+d;q.low[ix]=rne(p.w[wi]-expand(q.base.w[wi]));}
        }
    q.prepare_s=seconds(a,Clock::now());return q;
}
struct alignas(64) Config {uint8_t palette=1,start=0,reserved[14]={};uint16_t cols[16]={};uint8_t rows[16]={};Config(){for(int i=0;i<6;++i){cols[i]=64;rows[i]=16;}}};
static_assert(sizeof(Config)==64);
static uint64_t mix(uint64_t x){x+=0x9e3779b97f4a7c15ULL;x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;x=(x^(x>>27))*0x94d049bb133111ebULL;return x^(x>>31);}
static __m512 bf_expand(__m256bh b){return _mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32((__m256i)b),16));}
// Four products retain both BF16 high and low components of U and W.
// This is an approximation to FP32 UW, not a claim of exact FP32 hardware arithmetic.
static void amx_gemm(const Param& p,const PreparedLocal& q,int h,const BF16* uh,const BF16* ul,float* c){
    int dp=q.base.padded_in,obs=q.base.output_blocks;
    _tile_zero(0);if(obs>1)_tile_zero(1);if(obs>2)_tile_zero(2);if(obs>3)_tile_zero(3);
    for(int kb=0;kb<dp/32;++kb)for(int pass=0;pass<4;++pass){
        const BF16* ub=(pass>=2?ul:uh)+kb*32;
        const BF16* wb=(pass&1?q.low.data():q.base.packed.data())+((size_t(h)*(dp/32)+kb)*obs)*512;
        _tile_loadd(4,ub,dp*2);_tile_loadd(5,wb,64);_tile_dpbf16ps(0,4,5);
        if(obs>1){_tile_loadd(5,wb+512,64);_tile_dpbf16ps(1,4,5);}
        if(obs>2){_tile_loadd(5,wb+1024,64);_tile_dpbf16ps(2,4,5);}
        if(obs>3){_tile_loadd(5,wb+1536,64);_tile_dpbf16ps(3,4,5);}
    }
    _tile_stored(0,c,64);if(obs>1)_tile_stored(1,c+256,64);if(obs>2)_tile_stored(2,c+512,64);if(obs>3)_tile_stored(3,c+768,64);
}
template<bool Measure,bool Counters> static void tile(const Graph& g,const Param& p,const PreparedLocal& q,const Schedule& sched,
 const std::vector<float>& x,WorkspaceLocal& w,uint64_t base,int h,int block,bool amx,float* u,float* c,BF16* uh,BF16* ul,Stats& stats){
    auto now=[](){if constexpr(Measure)return Clock::now();else return Clock::time_point{};};
    auto a=now();int rows=int(std::min<uint64_t>(16,g.n-base)),dp=q.base.padded_in;
    std::fill(u,u+16*dp,0.f);alignas(64) float den[16]={};uint32_t rowids[16];
    if constexpr(Measure){stats.init+=seconds(a,now());++stats.sampled_tiles;}
    for(int ri=0;ri<rows;++ri){uint32_t row=sched.perm[base+ri];rowids[ri]=row;
        float* acc=u+ri*dp;float m=-INFINITY,l=0;uint32_t nb=0,nu=0,nr=0;
        const float left=w.lr[size_t(row)*2*p.heads+h];
        for(uint64_t begin=g.row[row];begin<g.row[row+1];begin+=block){
            a=now();int count=int(std::min<uint64_t>(block,g.row[row+1]-begin));
            alignas(64) float scores[64],weight[64];uint32_t sources[64];float bm=-INFINITY;
            for(int e=0;e<count;++e){sources[e]=g.col[begin+e];scores[e]=w.lr[size_t(sources[e])*2*p.heads+p.heads+h];}
            for(int e=0;e<count;e+=16){auto mask=tail_mask(std::min(16,count-e));
                __m512 v=leaky(_mm512_add_ps(_mm512_set1_ps(left),_mm512_maskz_loadu_ps(mask,scores+e)));
                _mm512_mask_storeu_ps(scores+e,mask,v);bm=std::max(bm,_mm512_mask_reduce_max_ps(mask,v));}
            float next=std::max(m,bm);bool changed=next>m;float r=l>0&&changed?std::exp(m-next):(l>0?1.f:0.f);
            float sum=0;for(int e=0;e<count;e+=16){auto mask=tail_mask(std::min(16,count-e));
                __m512 z=_mm512_mask_loadu_ps(_mm512_set1_ps(next),mask,scores+e);__m512 v=_mm512_exp_ps(_mm512_sub_ps(z,_mm512_set1_ps(next)));
                _mm512_mask_storeu_ps(weight+e,mask,v);sum+=_mm512_mask_reduce_add_ps(mask,v);}
            if constexpr(Measure)stats.score+=seconds(a,now());a=now();
            if(l>0&&changed){__m512 scale=_mm512_set1_ps(r);for(int d=0;d<p.in;d+=16){auto mask=tail_mask(std::min(16,p.in-d));_mm512_mask_storeu_ps(acc+d,mask,_mm512_mul_ps(_mm512_maskz_loadu_ps(mask,acc+d),scale));}}
            if constexpr(Measure)stats.rescale+=seconds(a,now());a=now();
            // One vector accumulator per feature block: store U only once per neighbor block.
            for(int d=0;d<p.in;d+=16){auto mask=tail_mask(std::min(16,p.in-d));__m512 v=_mm512_maskz_loadu_ps(mask,acc+d);
                for(int e=0;e<count;++e)v=_mm512_fmadd_ps(_mm512_set1_ps(weight[e]),_mm512_maskz_loadu_ps(mask,x.data()+size_t(sources[e])*p.in+d),v);
                _mm512_mask_storeu_ps(acc+d,mask,v);}
            if constexpr(Measure)stats.spmm+=seconds(a,now());
            if constexpr(Counters){++nb;nu+=changed;nr+=(l>0&&changed);}
            l=r*l+sum;m=next;
        }
        den[ri]=l;
        if constexpr(Counters){size_t ix=size_t(row)*p.heads+h;w.blocks[ix]=nb;w.updates[ix]=nu;w.rescales[ix]=nr;}
    }
    a=now();
    if(amx){for(int j=0;j<16*dp;j+=16){__m512 v=_mm512_loadu_ps(u+j);__m256bh hi=_mm512_cvtneps_pbh(v),lo=_mm512_cvtneps_pbh(_mm512_sub_ps(v,bf_expand(hi)));
        _mm256_storeu_si256(reinterpret_cast<__m256i*>(uh+j),(__m256i)hi);_mm256_storeu_si256(reinterpret_cast<__m256i*>(ul+j),(__m256i)lo);}}
    if constexpr(Measure)stats.packing+=seconds(a,now());a=now();
    if(amx)amx_gemm(p,q,h,uh,ul,c);
    else cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,rows,p.dim,p.in,1,u,dp,q.wh.data()+size_t(h)*p.in*p.dim,p.dim,0,c,p.dim);
    if constexpr(Measure)stats.gemm+=seconds(a,now());a=now();
    for(int ri=0;ri<rows;++ri){float inv=den[ri]>0?1.f/den[ri]:0;
        float* dest=w.out.data()+size_t(rowids[ri])*p.width()+h*p.dim;
        for(int d=0;d<p.dim;d+=16){auto mask=tail_mask(std::min(16,p.dim-d));
            const float* src=amx?c+(d/16)*256+ri*16:c+ri*p.dim+d;
            _mm512_mask_storeu_ps(dest+d,mask,_mm512_mul_ps(_mm512_maskz_loadu_ps(mask,src),_mm512_set1_ps(inv)));}}
    if constexpr(Measure)stats.output+=seconds(a,now());
}
template<bool Counters> static void run(const Graph& g,const Param& p,const PreparedLocal& q,const Schedule& sched,const std::vector<float>& x,
 WorkspaceLocal& w,bool amx,int block,int panel,Stats& result,uint64_t period){
    std::atomic<bool> failed{false};
    #pragma omp parallel
    {
        Stats local;int id=omp_get_thread_num(),dp=q.base.padded_in;int old_threads=mkl_set_num_threads_local(1);
        if(amx&&syscall(SYS_arch_prctl,0x1023,18)!=0)failed.store(true);
        #pragma omp barrier
        if(!failed.load()){
            Config cfg;if(amx)_tile_loadconfig(&cfg);
            float* u=w.u.data()+size_t(id)*16*dp;float* c=w.c.data()+size_t(id)*16*64;
            BF16* uh=w.uh.data()+size_t(id)*16*dp;BF16* ul=w.ul.data()+size_t(id)*16*dp;
            // All heads of one row panel share an owner; attention/state remain independent.
            #pragma omp for schedule(dynamic,1)
            for(uint64_t rg=0;rg<g.n;rg+=panel)for(uint64_t base=rg;base<std::min<uint64_t>(g.n,rg+panel);base+=16)for(int h=0;h<p.heads;++h){
                bool measure=period&&mix((base/16)^(uint64_t(h)<<48))%period==0;
                if(measure)tile<true,Counters>(g,p,q,sched,x,w,base,h,block,amx,u,c,uh,ul,local);
                else tile<false,Counters>(g,p,q,sched,x,w,base,h,block,amx,u,c,uh,ul,local);
            }
            if(amx)_tile_release();
        }
        mkl_set_num_threads_local(old_threads);
        #pragma omp critical(gat_local_stats)
        {result.init+=local.init;result.score+=local.score;result.rescale+=local.rescale;result.spmm+=local.spmm;result.packing+=local.packing;result.gemm+=local.gemm;result.output+=local.output;result.sampled_tiles+=local.sampled_tiles;}
    }
    if(failed.load())throw std::runtime_error("local AMX permission failed");
}
void aggregate(const Graph& g,const Param& p,const PreparedLocal& q,const Schedule& sched,const std::vector<float>& x,WorkspaceLocal& w,
 bool amx,int block,int panel,Stats& stats,uint64_t period,bool counters){
    if(block<1||block>64||panel<16||panel%16||x.size()!=checked(g.n,p.in))throw std::runtime_error("invalid local configuration");
    if(counters&&w.blocks.size()!=checked(g.n,p.heads))throw std::runtime_error("counter workspace not allocated");
    if(counters)run<true>(g,p,q,sched,x,w,amx,block,panel,stats,period);
    else run<false>(g,p,q,sched,x,w,amx,block,panel,stats,period);
}
void layer(const Graph& g,const Param& p,const PreparedLocal& q,const Schedule& sched,const std::vector<float>& x,WorkspaceLocal& w,
 bool hidden,bool amx,int block,int panel,Timing& t,uint64_t period,bool counters){
    auto begin=Clock::now(),a=begin;
    cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,int(g.n),2*p.heads,p.in,1,x.data(),p.in,q.base.blr.data(),2*p.heads,0,w.lr.data(),2*p.heads);
    t.lr=seconds(a,Clock::now());a=Clock::now();aggregate(g,p,q,sched,x,w,amx,block,panel,t.sample,period,counters);
    t.kernel=seconds(a,Clock::now());a=Clock::now();
    if(hidden){
        #pragma omp parallel for schedule(static)
        for(size_t i=0;i<w.out.size();i+=16){auto mask=tail_mask(int(std::min<size_t>(16,w.out.size()-i)));_mm512_mask_storeu_ps(w.out.data()+i,mask,activation(_mm512_maskz_loadu_ps(mask,w.out.data()+i)));}}
    t.activation=seconds(a,Clock::now());t.total=seconds(begin,Clock::now());
}
void print_stats(const WorkspaceLocal& w,const Param& p,int layer){
    uint64_t nb=0,nu=0,nr=0;std::vector<float> ratios;ratios.reserve(w.blocks.size());
    for(size_t i=0;i<w.blocks.size();++i){nb+=w.blocks[i];nu+=w.updates[i];nr+=w.rescales[i];ratios.push_back(w.blocks[i]?float(w.rescales[i])/w.blocks[i]:0);}
    std::sort(ratios.begin(),ratios.end());auto pct=[&](double f){return ratios.empty()?0:ratios[std::min(ratios.size()-1,size_t(std::ceil(ratios.size()*f)-1))];};
    std::cout<<std::setprecision(10)<<"ONLINE_STATS layer="<<layer<<" blocks="<<nb<<" running_max_updates_including_initial="<<nu<<" rescales="<<nr<<" rescaled_feature_elements="<<nr*uint64_t(p.in)<<" rescale_per_block="<<(nb?double(nr)/nb:0)<<" row_head_ratio_P50="<<pct(.5)<<" P90="<<pct(.9)<<" P95="<<pct(.95)<<" P99="<<pct(.99)<<" empty_rows_ratio=0\n";
}
}
