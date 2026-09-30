// Independent diagnostic copy of B1. Tile-granularity selection dispatches to
// separate compile-time specializations: no clock reads in unsampled tiles.
// Phases remain coarse to avoid timing individual vector/tile instructions.
#include "tfs_sampled.hpp"
#include <atomic>
#include <sys/syscall.h>
#include <unistd.h>
namespace gat {
struct alignas(64) SampleTileConfig {
    uint8_t palette=1,start=0,reserved[14]={};uint16_t cols[16]={};uint8_t rows[16]={};
    SampleTileConfig(){for(int t=0;t<6;++t){cols[t]=64;rows[t]=16;}}
};
static_assert(sizeof(SampleTileConfig)==64);
static uint64_t mix(uint64_t x){x+=0x9e3779b97f4a7c15ULL;x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;x=(x^(x>>27))*0x94d049bb133111ebULL;return x^(x>>31);}
template<bool Measure> static void tile(const Graph& g,const Param& p,const Prepared& q,const Schedule& sched,Workspace& w,
 uint64_t base,int batch,int h,BF16* hbuf,float* cbuf,SampleStats& stats){
    auto now=[](){if constexpr(Measure)return Clock::now();else return Clock::time_point{};};
    auto st=now();const int dp=q.padded_in,kblocks=dp/32,obs=q.output_blocks;
    uint32_t rows[16]={};uint64_t starts[16]={},degrees[16]={};uint64_t max_degree=0;
    alignas(64) float den[16]={},left[16]={},maximum[16]={};
    for(int n=0;n<batch;++n){rows[n]=sched.perm[base+n];starts[n]=g.row[rows[n]];degrees[n]=g.row[rows[n]+1]-starts[n];max_degree=std::max(max_degree,degrees[n]);
        left[n]=w.left[size_t(rows[n])*p.heads+h];maximum[n]=w.max[size_t(rows[n])*p.heads+h];}
    std::fill(hbuf,hbuf+16*dp,BF16(0));int active_from=0;
    _tile_zero(0);if(obs>1)_tile_zero(1);if(obs>2)_tile_zero(2);if(obs>3)_tile_zero(3);
    if constexpr(Measure){stats.schedule+=seconds(st,now());++stats.tiles;}
    for(uint64_t s=0;s<max_degree;++s){
        st=now();
        if(s+1<max_degree)for(int n=active_from;n<batch;++n)if(s+1<degrees[n]){
            const char* addr=reinterpret_cast<const char*>(w.xbf.data()+size_t(g.col[starts[n]+s+1])*p.in);
            for(int off=0;off<p.in*2;off+=64)_mm_prefetch(addr+off,_MM_HINT_T0);}
        while(active_from<batch&&s>=degrees[active_from]){std::fill(hbuf+active_from*dp,hbuf+(active_from+1)*dp,BF16(0));++active_from;}
        if constexpr(Measure)stats.schedule+=seconds(st,now());if(active_from>=batch)break;
        alignas(64) float right[16]={},weight[16]={};uint32_t source[16]={};st=now();
        for(int n=active_from;n<batch;++n){source[n]=g.col[starts[n]+s];right[n]=w.right[size_t(source[n])*p.heads+h];}
        __m512 score=leaky(_mm512_add_ps(_mm512_load_ps(left),_mm512_load_ps(right)));
        __m512 v=_mm512_exp_ps(_mm512_sub_ps(score,_mm512_load_ps(maximum)));
        __mmask16 mask=__mmask16(((1u<<batch)-1)&~((1u<<active_from)-1));_mm512_store_ps(weight,_mm512_maskz_mov_ps(mask,v));
        for(int n=active_from;n<batch;++n)den[n]+=weight[n];
        if constexpr(Measure)stats.score+=seconds(st,now());st=now();
        for(int n=active_from;n<batch;++n){const BF16* src=w.xbf.data()+size_t(source[n])*p.in;BF16* dst=hbuf+n*dp;__m512 pw=_mm512_set1_ps(weight[n]);
            for(int k=0;k<p.in;k+=16){int len=std::min(16,p.in-k);__m256i b;
                if(len==16)b=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(src+k));
                else{alignas(32) BF16 tmp[16]={};std::memcpy(tmp,src+k,len*2);b=_mm256_load_si256(reinterpret_cast<const __m256i*>(tmp));}
                __m512 x=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(b),16));
                __m256bh weighted=_mm512_cvtneps_pbh(_mm512_mul_ps(x,pw));_mm256_storeu_si256(reinterpret_cast<__m256i*>(dst+k),(__m256i)weighted);
            }
        }
        if constexpr(Measure)stats.stage+=seconds(st,now());st=now();
        for(int kb=0;kb<kblocks;++kb){_tile_loadd(4,hbuf+kb*32,dp*2);
            const BF16* wb=q.packed.data()+((size_t(h)*kblocks+kb)*obs)*512;
            _tile_loadd(5,wb,64);_tile_dpbf16ps(0,4,5);
            if(obs>1){_tile_loadd(5,wb+512,64);_tile_dpbf16ps(1,4,5);}
            if(obs>2){_tile_loadd(5,wb+1024,64);_tile_dpbf16ps(2,4,5);}
            if(obs>3){_tile_loadd(5,wb+1536,64);_tile_dpbf16ps(3,4,5);}
        }
        if constexpr(Measure){stats.matrix+=seconds(st,now());++stats.steps;stats.active_edges+=batch-active_from;}
    }
    st=now();_tile_stored(0,cbuf,64);if(obs>1)_tile_stored(1,cbuf+256,64);if(obs>2)_tile_stored(2,cbuf+512,64);if(obs>3)_tile_stored(3,cbuf+768,64);
    for(int n=0;n<batch;++n){w.den[size_t(rows[n])*p.heads+h]=den[n];float* dst=w.out.data()+size_t(rows[n])*p.width()+h*p.dim;
        for(int ob=0;ob<obs;++ob)std::memcpy(dst+ob*16,cbuf+ob*256+n*16,std::min(16,p.dim-ob*16)*4);}
    if constexpr(Measure)stats.output+=seconds(st,now());
}
void sampled_aggregate(const Graph& g,const Param& p,const Prepared& q,const Schedule& sched,Workspace& w,int panel,uint64_t period,uint64_t seed,SampleStats& result){
    if(panel<16||panel%16)throw std::runtime_error("bad panel");std::atomic<bool> failure{false};
    #pragma omp parallel
    {
        SampleStats local;
        if(syscall(SYS_arch_prctl,0x1023,18)!=0)failure.store(true);
        #pragma omp barrier
        if(!failure.load()){
            SampleTileConfig cfg;_tile_loadconfig(&cfg);
            BF16* hbuf=w.thread_hbuf.data()+size_t(omp_get_thread_num())*16*q.padded_in;
            float* cbuf=w.thread_cbuf.data()+size_t(omp_get_thread_num())*16*64;
            for(int h=0;h<p.heads;++h){
                #pragma omp for schedule(dynamic,1)
                for(uint64_t rg=0;rg<g.n;rg+=panel){uint64_t end=std::min(g.n,rg+panel);
                    for(uint64_t base=rg;base<end;base+=16){int batch=int(std::min<uint64_t>(16,end-base));
                        bool sample=period&&mix(seed^(base/16)^(uint64_t(h)<<48))%period==0;
                        if(sample)tile<true>(g,p,q,sched,w,base,batch,h,hbuf,cbuf,local);
                        else tile<false>(g,p,q,sched,w,base,batch,h,hbuf,cbuf,local);
                    }
                }
            }
            _tile_release();
        }
        #pragma omp critical(gat_sample_merge)
        {result.schedule+=local.schedule;result.score+=local.score;result.stage+=local.stage;result.matrix+=local.matrix;result.output+=local.output;result.tiles+=local.tiles;result.steps+=local.steps;result.active_edges+=local.active_edges;}
    }
    if(failure.load())throw std::runtime_error("AMX worker permission failed");
}
}
