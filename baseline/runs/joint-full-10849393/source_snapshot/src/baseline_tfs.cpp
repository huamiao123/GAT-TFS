// Adapted from GAT/code/code/amx_tfs_v3.cpp: ascending degree permutation,
// dynamic R panels, TR=16, smart zeroing, next-neighbor prefetch, original-row
// scatter and neighbor-step projection with output partial sums resident in TMM.
// Extensions: independent heads, dynamic p, BF16 RNE, D/d tails, FP32 denominator.
#include "gat.hpp"
#include <atomic>
#include <sys/syscall.h>
#include <unistd.h>
namespace gat {
struct alignas(64) TileConfig {
    uint8_t palette=1,start=0,reserved[14]={};
    uint16_t cols[16]={};uint8_t rows[16]={};
    TileConfig() {for(int t=0;t<6;++t){cols[t]=64;rows[t]=16;}}
};
static_assert(sizeof(TileConfig)==64);
void tfs_aggregate(const Graph& g,const Param& p,const Prepared& q,const Schedule& sched,Workspace& w,int panel,Times& t,const float* fixed_p,const float* fixed_den) {
    if(panel<16 || panel%16)throw std::runtime_error("panel must be a multiple of 16");
    const int dp=q.padded_in,kblocks=dp/32,obs=q.output_blocks;
    std::atomic<bool> permission_failed{false};
    #pragma omp parallel
    {
        Times local;auto ct=tick();
        if(syscall(SYS_arch_prctl,0x1023,18)!=0)permission_failed.store(true);
        #pragma omp barrier
        if(!permission_failed.load()) {
            TileConfig cfg;_tile_loadconfig(&cfg);local.tile_config_worker+=seconds(ct,tick());
            BF16* hbuf=w.thread_hbuf.data()+size_t(omp_get_thread_num())*16*dp;
            float* cbuf=w.thread_cbuf.data()+size_t(omp_get_thread_num())*16*64;
            // One persistent region; head barriers are required for omp-for legality.
            for(int h=0;h<p.heads;++h) {
                #pragma omp for schedule(dynamic,1)
                for(uint64_t rg=0;rg<g.n;rg+=panel) {
                    uint64_t rg_end=std::min(g.n,rg+panel);
                    for(uint64_t base=rg;base<rg_end;base+=16){
                        auto st=tick();int batch=int(std::min<uint64_t>(16,rg_end-base));
                        uint32_t rows[16]={};uint64_t starts[16]={},degrees[16]={};uint64_t max_degree=0;
                        alignas(64) float den[16]={},left[16]={},maximum[16]={};
                        for(int n=0;n<batch;++n){rows[n]=sched.perm[base+n];starts[n]=g.row[rows[n]];degrees[n]=g.row[rows[n]+1]-starts[n];max_degree=std::max(max_degree,degrees[n]);
                            left[n]=w.left[size_t(rows[n])*p.heads+h];maximum[n]=w.max[size_t(rows[n])*p.heads+h];}
                        std::fill(hbuf,hbuf+16*dp,BF16(0));int active_from=0;
                        _tile_zero(0);if(obs>1)_tile_zero(1);if(obs>2)_tile_zero(2);if(obs>3)_tile_zero(3);
                        local.scheduling_worker+=seconds(st,tick());
                        for(uint64_t s=0;s<max_degree;++s) {
                            st=tick();
                            if(s+1<max_degree)for(int n=active_from;n<batch;++n)if(s+1<degrees[n]){
                                const char* addr=reinterpret_cast<const char*>(w.xbf.data()+size_t(g.col[starts[n]+s+1])*p.in);
                                for(int off=0;off<p.in*2;off+=64)_mm_prefetch(addr+off,_MM_HINT_T0);}
                            while(active_from<batch && s>=degrees[active_from]){std::fill(hbuf+active_from*dp,hbuf+(active_from+1)*dp,BF16(0));++active_from;}
                            local.scheduling_worker+=seconds(st,tick());if(active_from>=batch)break;
                            alignas(64) float right[16]={},weight[16]={};uint32_t source[16]={};
                            st=tick();
                            for(int n=active_from;n<batch;++n){source[n]=g.col[starts[n]+s];
                                if(fixed_p)weight[n]=fixed_p[(starts[n]+s)*p.heads+h];
                                else right[n]=w.right[size_t(source[n])*p.heads+h];}
                            if(!fixed_p){__m512 score=leaky(_mm512_add_ps(_mm512_load_ps(left),_mm512_load_ps(right)));
                                __m512 v=_mm512_exp_ps(_mm512_sub_ps(score,_mm512_load_ps(maximum)));
                                __mmask16 mask=__mmask16(((1u<<batch)-1)&~((1u<<active_from)-1));_mm512_store_ps(weight,_mm512_maskz_mov_ps(mask,v));}
                            for(int n=active_from;n<batch;++n)den[n]+=weight[n];
                            local.score_exp_worker+=seconds(st,tick());st=tick();
                            for(int n=active_from;n<batch;++n){const BF16* src=w.xbf.data()+size_t(source[n])*p.in;BF16* dst=hbuf+n*dp;
                                __m512 pw=_mm512_set1_ps(weight[n]);
                                for(int k=0;k<p.in;k+=16){int len=std::min(16,p.in-k);__m256i b;
                                    auto gt=tick();
                                    if(len==16)b=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(src+k));
                                    else {alignas(32) BF16 tmp[16]={};std::memcpy(tmp,src+k,len*2);b=_mm256_load_si256(reinterpret_cast<const __m256i*>(tmp));}
                                    local.gather_load_worker+=seconds(gt,tick());gt=tick();
                                    __m512 v=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(b),16));
                                    __m256bh weighted=_mm512_cvtneps_pbh(_mm512_mul_ps(v,pw));
                                    _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst+k),(__m256i)weighted);
                                    local.p_times_x_convert_worker+=seconds(gt,tick());
                                }
                            }
                            local.gather_weight_convert_worker+=seconds(st,tick());
                            // No accumulator stores inside neighbor scan. All head output
                            // blocks (up to d=64) stay in tiles 0..3 across every neighbor.
                            for(int kb=0;kb<kblocks;++kb){
                                st=tick();_tile_loadd(4,hbuf+kb*32,dp*2);local.tile_load_worker+=seconds(st,tick());
                                const BF16* wb=q.packed.data()+((size_t(h)*kblocks+kb)*obs)*512;
                                st=tick();_tile_loadd(5,wb,64);local.tile_load_worker+=seconds(st,tick());
                                st=tick();_tile_dpbf16ps(0,4,5);local.amx_compute_worker+=seconds(st,tick());
                                if(obs>1){st=tick();_tile_loadd(5,wb+512,64);local.tile_load_worker+=seconds(st,tick());st=tick();_tile_dpbf16ps(1,4,5);local.amx_compute_worker+=seconds(st,tick());}
                                if(obs>2){st=tick();_tile_loadd(5,wb+1024,64);local.tile_load_worker+=seconds(st,tick());st=tick();_tile_dpbf16ps(2,4,5);local.amx_compute_worker+=seconds(st,tick());}
                                if(obs>3){st=tick();_tile_loadd(5,wb+1536,64);local.tile_load_worker+=seconds(st,tick());st=tick();_tile_dpbf16ps(3,4,5);local.amx_compute_worker+=seconds(st,tick());}
                            }
                            if constexpr(profiling){++local.neighbor_steps;local.executed_fma+=uint64_t(16)*dp*obs*16;}
                        }
                        st=tick();_tile_stored(0,cbuf,64);if(obs>1)_tile_stored(1,cbuf+256,64);if(obs>2)_tile_stored(2,cbuf+512,64);if(obs>3)_tile_stored(3,cbuf+768,64);
                        local.tile_store_worker+=seconds(st,tick());st=tick();
                        for(int n=0;n<batch;++n){w.den[size_t(rows[n])*p.heads+h]=fixed_den?fixed_den[size_t(rows[n])*p.heads+h]:den[n];
                            float* dst=w.out.data()+size_t(rows[n])*p.width()+h*p.dim;
                            for(int ob=0;ob<obs;++ob)std::memcpy(dst+ob*16,cbuf+ob*256+n*16,std::min(16,p.dim-ob*16)*4);}
                        local.output_write_worker+=seconds(st,tick());
                    }
                }
            }
            _tile_release();
        }
        if constexpr(profiling){
            #pragma omp critical(gat_profile)
            {
                t.score_exp_worker+=local.score_exp_worker;t.gather_weight_convert_worker+=local.gather_weight_convert_worker;
                t.gather_load_worker+=local.gather_load_worker;t.p_times_x_convert_worker+=local.p_times_x_convert_worker;
                t.tile_load_worker+=local.tile_load_worker;t.amx_compute_worker+=local.amx_compute_worker;t.tile_store_worker+=local.tile_store_worker;
                t.tile_config_worker+=local.tile_config_worker;t.scheduling_worker+=local.scheduling_worker;t.output_write_worker+=local.output_write_worker;
                t.neighbor_steps+=local.neighbor_steps;t.executed_fma+=local.executed_fma;
            }
        }
    }
    if(permission_failed.load())throw std::runtime_error("AMX permission request failed on worker");
}
void tfs_layer(const Graph& g,const Param& p,const Prepared& q,const Schedule& sched,const std::vector<float>& input,Workspace& w,bool hidden,int panel,Times& t,const std::string& lr_policy) {
    auto begin=Clock::now(),a=begin;convert(input.data(),w.xbf.data(),input.size());t.convert=seconds(a,Clock::now());
    a=Clock::now();lr_reordered(g,p,q,input,w,lr_policy);t.lr=seconds(a,Clock::now());
    a=Clock::now();max_prescan(g,p,w);t.max_prescan=seconds(a,Clock::now());
    a=Clock::now();tfs_aggregate(g,p,q,sched,w,panel,t);t.aggregate=seconds(a,Clock::now());
    finish(g,p,w,hidden,t);t.total=seconds(begin,Clock::now());
}
}
