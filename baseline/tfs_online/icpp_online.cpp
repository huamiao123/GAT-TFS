#include "icpp_online.hpp"
#include <atomic>
#include <sys/syscall.h>
#include <unistd.h>

namespace gat::icpp_online {
namespace {
struct alignas(64) TileConfig {
    uint8_t palette=1,start=0,reserved[14]={};
    uint16_t cols[16]={};
    uint8_t rows[16]={};
    TileConfig() {for(int t=0;t<6;++t){cols[t]=64;rows[t]=16;}}
};
static_assert(sizeof(TileConfig)==64);

template<bool Measure> inline Clock::time_point stamp() {
    if constexpr(Measure)return Clock::now();
    else return {};
}
template<bool Measure> inline void charge(double& field,Clock::time_point begin) {
    if constexpr(Measure)field+=seconds(begin,Clock::now());
}

inline void store_output_tiles(float* cbuf,int obs) {
    _tile_stored(0,cbuf,64);
    if(obs>1)_tile_stored(1,cbuf+256,64);
    if(obs>2)_tile_stored(2,cbuf+512,64);
    if(obs>3)_tile_stored(3,cbuf+768,64);
}
inline void reload_output_tiles(const float* cbuf,int obs) {
    _tile_loadd(0,cbuf,64);
    if(obs>1)_tile_loadd(1,cbuf+256,64);
    if(obs>2)_tile_loadd(2,cbuf+512,64);
    if(obs>3)_tile_loadd(3,cbuf+768,64);
}

// The only change to the original resident-output tile lifecycle is the exact
// row-specific rescale on a running maximum increase. There is no U[D] buffer
// and no separate SGEMM. Every neighbor still executes BF16(pH)W in AMX.
template<bool Measure,bool Counters>
void run_tile(const Graph& g,const Param& p,const Prepared& q,
              const Schedule& sched,Workspace& w,uint64_t base,int batch,
              int h,int block,BF16* hbuf,float* cbuf,Stats& stats) {
    auto st=stamp<Measure>();
    const int dp=q.padded_in,kblocks=dp/32,obs=q.output_blocks;
    uint32_t rows[16]={};
    uint64_t starts[16]={},degrees[16]={};
    uint64_t max_degree=0;
    alignas(64) float left[16]={},maximum[16],den[16]={};
    for(int n=0;n<16;++n)maximum[n]=-std::numeric_limits<float>::infinity();
    for(int n=0;n<batch;++n) {
        rows[n]=sched.perm[base+n];
        starts[n]=g.row[rows[n]];
        degrees[n]=g.row[rows[n]+1]-starts[n];
        max_degree=std::max(max_degree,degrees[n]);
        left[n]=w.base.left[size_t(rows[n])*p.heads+h];
        if constexpr(Counters) {
            size_t idx=size_t(rows[n])*p.heads+h;
            w.row_blocks[idx]=w.row_max_updates[idx]=w.row_rescales[idx]=0;
        }
    }
    // Smart zeroing: padded features are cleared once; completed row lanes are
    // cleared exactly once when their degree is reached, as in the ICPP kernel.
    std::fill(hbuf,hbuf+size_t(16)*dp,BF16(0));
    _tile_zero(0);
    if(obs>1)_tile_zero(1);
    if(obs>2)_tile_zero(2);
    if(obs>3)_tile_zero(3);
    charge<Measure>(stats.scheduling,st);

    // Bounded 16x64 score/weight scratch. It never grows with E or row degree.
    alignas(64) float scores[64][16];
    uint32_t sources[64][16];
    for(uint64_t begin=0;begin<max_degree;begin+=uint64_t(block)) {
        const int count=int(std::min<uint64_t>(block,max_degree-begin));
        alignas(64) float block_maximum[16];
        for(int n=0;n<16;++n)block_maximum[n]=-std::numeric_limits<float>::infinity();
        // First generate all scores for the neighbor block, with independent
        // maxima and probabilities for each destination row and head.
        for(int b=0;b<count;++b) {
            st=stamp<Measure>();
            alignas(64) float right[16]={};
            unsigned valid=0;
            uint64_t step=begin+uint64_t(b);
            for(int n=0;n<batch;++n)if(step<degrees[n]) {
                valid|=1u<<n;
                sources[b][n]=g.col[starts[n]+step];
                right[n]=w.base.right[size_t(sources[b][n])*p.heads+h];
            }
            __m512 e=leaky(_mm512_add_ps(_mm512_load_ps(left),_mm512_load_ps(right)));
            e=_mm512_mask_mov_ps(_mm512_set1_ps(-std::numeric_limits<float>::infinity()),
                                 __mmask16(valid),e);
            _mm512_store_ps(scores[b],e);
            charge<Measure>(stats.score_generation,st);
            st=stamp<Measure>();
            _mm512_store_ps(block_maximum,_mm512_max_ps(_mm512_load_ps(block_maximum),e));
            charge<Measure>(stats.block_max,st);
        }

        st=stamp<Measure>();
        alignas(64) float previous_max[16],rescale[16];
        std::memcpy(previous_max,maximum,sizeof(previous_max));
        _mm512_store_ps(maximum,_mm512_max_ps(_mm512_load_ps(maximum),
                                            _mm512_load_ps(block_maximum)));
        unsigned changed=0;
        for(int n=0;n<batch;++n)if(begin<degrees[n]) {
            bool update=maximum[n]>previous_max[n];
            bool do_rescale=update && den[n]>0;
            if(do_rescale)changed|=1u<<n;
            if constexpr(Counters) {
                const size_t idx=size_t(rows[n])*p.heads+h;
                ++stats.blocks;++w.row_blocks[idx];
                if(update){++stats.max_updates;++w.row_max_updates[idx];}
                if(do_rescale) {
                    ++stats.rescales;++w.row_rescales[idx];
                    stats.rescaled_feature_elements+=uint64_t(p.dim);
                    stats.rescaled_padded_feature_elements+=uint64_t(obs)*16;
                }
            }
        }
        charge<Measure>(stats.block_max,st);

        st=stamp<Measure>();
        // Inactive and first-block lanes evaluate exp(0), never -inf-(-inf).
        __m512 shift=_mm512_maskz_sub_ps(__mmask16(changed),
                                       _mm512_load_ps(previous_max),
                                       _mm512_load_ps(maximum));
        __m512 r=_mm512_exp_ps(shift);
        _mm512_store_ps(rescale,r);
        _mm512_store_ps(den,_mm512_mul_ps(_mm512_load_ps(den),r));
        for(int b=0;b<count;++b) {
            unsigned valid=0;
            uint64_t step=begin+uint64_t(b);
            for(int n=0;n<batch;++n)if(step<degrees[n])valid|=1u<<n;
            __m512 shifted=_mm512_maskz_sub_ps(__mmask16(valid),
                                             _mm512_load_ps(scores[b]),
                                             _mm512_load_ps(maximum));
            __m512 weight=_mm512_maskz_mov_ps(__mmask16(valid),_mm512_exp_ps(shifted));
            _mm512_store_ps(scores[b],weight); // recycle scores as probabilities
            _mm512_store_ps(den,_mm512_add_ps(_mm512_load_ps(den),weight));
        }
        charge<Measure>(stats.exp_den,st);

        // Preserve output residency when the maximum is unchanged. When any
        // initialized row changes, spill all output tiles, scale only those
        // rows, and reload every tile before resuming the original AMX loop.
        if(changed) {
            st=stamp<Measure>();
            store_output_tiles(cbuf,obs);
            charge<Measure>(stats.rescale_store,st);
            st=stamp<Measure>();
            for(int ob=0;ob<obs;++ob)for(int n=0;n<batch;++n)if(changed&(1u<<n)) {
                float* row=cbuf+size_t(ob)*256+n*16;
                _mm512_storeu_ps(row,_mm512_mul_ps(_mm512_loadu_ps(row),
                                                 _mm512_set1_ps(rescale[n])));
            }
            charge<Measure>(stats.rescale_vector,st);
            st=stamp<Measure>();
            reload_output_tiles(cbuf,obs);
            charge<Measure>(stats.rescale_reload,st);
            if constexpr(Counters) {
                ++stats.rescale_events;
                stats.spill_bytes+=uint64_t(obs)*1024;
                stats.reload_bytes+=uint64_t(obs)*1024;
            }
        }

        for(int b=0;b<count;++b) {
            const uint64_t step=begin+uint64_t(b);
            st=stamp<Measure>();
            if(step+1<max_degree)for(int n=0;n<batch;++n)if(step+1<degrees[n]) {
                const char* addr=reinterpret_cast<const char*>(
                    w.base.xbf.data()+size_t(g.col[starts[n]+step+1])*p.in);
                for(int off=0;off<p.in*2;off+=64)_mm_prefetch(addr+off,_MM_HINT_T0);
            }
            for(int n=0;n<batch;++n)if(step==degrees[n])
                std::fill(hbuf+size_t(n)*dp,hbuf+size_t(n+1)*dp,BF16(0));
            charge<Measure>(stats.scheduling,st);
            for(int n=0;n<batch;++n)if(step<degrees[n]) {
                const BF16* src=w.base.xbf.data()+size_t(sources[b][n])*p.in;
                BF16* dst=hbuf+size_t(n)*dp;
                __m512 pw=_mm512_set1_ps(scores[b][n]);
                for(int k=0;k<p.in;k+=16) {
                    const int len=std::min(16,p.in-k);
                    st=stamp<Measure>();
                    __m256i raw;
                    if(len==16)raw=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(src+k));
                    else {
                        alignas(32) BF16 tail[16]={};
                        std::memcpy(tail,src+k,size_t(len)*sizeof(BF16));
                        raw=_mm256_load_si256(reinterpret_cast<const __m256i*>(tail));
                    }
                    charge<Measure>(stats.source_gather,st);
                    st=stamp<Measure>();
                    __m512 v=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(raw),16));
                    __m256bh weighted=_mm512_cvtneps_pbh(_mm512_mul_ps(v,pw));
                    _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst+k),(__m256i)weighted);
                    charge<Measure>(stats.weighted_convert,st);
                }
            }
            // Original ICPP neighbor-step BF16(pH)W. The full input feature
            // row is gathered above; all output blocks remain resident in TMM.
            for(int kb=0;kb<kblocks;++kb) {
                st=stamp<Measure>();
                _tile_loadd(4,hbuf+kb*32,dp*2);
                const BF16* wb=q.packed.data()+((size_t(h)*kblocks+kb)*obs)*512;
                _tile_loadd(5,wb,64);
                charge<Measure>(stats.tile_load,st);
                st=stamp<Measure>();_tile_dpbf16ps(0,4,5);
                charge<Measure>(stats.amx_compute,st);
                if(obs>1) {
                    st=stamp<Measure>();_tile_loadd(5,wb+512,64);
                    charge<Measure>(stats.tile_load,st);
                    st=stamp<Measure>();_tile_dpbf16ps(1,4,5);
                    charge<Measure>(stats.amx_compute,st);
                }
                if(obs>2) {
                    st=stamp<Measure>();_tile_loadd(5,wb+1024,64);
                    charge<Measure>(stats.tile_load,st);
                    st=stamp<Measure>();_tile_dpbf16ps(2,4,5);
                    charge<Measure>(stats.amx_compute,st);
                }
                if(obs>3) {
                    st=stamp<Measure>();_tile_loadd(5,wb+1536,64);
                    charge<Measure>(stats.tile_load,st);
                    st=stamp<Measure>();_tile_dpbf16ps(3,4,5);
                    charge<Measure>(stats.amx_compute,st);
                }
            }
            if constexpr(Counters) {
                ++stats.neighbor_steps;
                stats.amx_calls+=uint64_t(kblocks)*obs;
                stats.executed_fma+=uint64_t(16)*dp*obs*16;
            }
        }
    }
    st=stamp<Measure>();store_output_tiles(cbuf,obs);
    charge<Measure>(stats.final_store,st);
    st=stamp<Measure>();
    for(int n=0;n<batch;++n) {
        size_t idx=size_t(rows[n])*p.heads+h;
        w.base.max[idx]=maximum[n];w.base.den[idx]=den[n];
        float* dst=w.base.out.data()+size_t(rows[n])*p.width()+h*p.dim;
        for(int ob=0;ob<obs;++ob)
            std::memcpy(dst+ob*16,cbuf+ob*256+n*16,size_t(std::min(16,p.dim-ob*16))*4);
    }
    charge<Measure>(stats.output_write,st);
}

void combine(Stats& dst,const Stats& src) {
#define ADD_FIELD(name) dst.name+=src.name
    ADD_FIELD(score_generation);ADD_FIELD(block_max);ADD_FIELD(exp_den);
    ADD_FIELD(rescale_store);ADD_FIELD(rescale_vector);ADD_FIELD(rescale_reload);
    ADD_FIELD(source_gather);ADD_FIELD(weighted_convert);ADD_FIELD(tile_load);
    ADD_FIELD(amx_compute);ADD_FIELD(final_store);ADD_FIELD(output_write);
    ADD_FIELD(scheduling);ADD_FIELD(tile_config);ADD_FIELD(blocks);
    ADD_FIELD(max_updates);ADD_FIELD(rescales);ADD_FIELD(rescaled_feature_elements);
    ADD_FIELD(rescaled_padded_feature_elements);ADD_FIELD(spill_bytes);
    ADD_FIELD(reload_bytes);ADD_FIELD(rescale_events);ADD_FIELD(amx_calls);
    ADD_FIELD(neighbor_steps);ADD_FIELD(executed_fma);ADD_FIELD(sampled_tiles);
    ADD_FIELD(total_tiles);
#undef ADD_FIELD
}

void validate_call(const Graph& g,const Param& p,const Prepared& q,
                   const Schedule& sched,const Workspace& w,int block,int panel,
                   int profile_period,bool counters) {
    if(block<1 || block>64)throw std::runtime_error("ICPP Online block must be in [1,64]");
    if(panel<16 || panel%16)throw std::runtime_error("ICPP Online panel must be a multiple of 16");
    if(profile_period<0)throw std::runtime_error("ICPP Online profile period must be nonnegative");
    if(!g.n || p.in<=0 || p.heads<1 || p.heads>8 || p.dim<1 || p.dim>64 ||
       w.allocated_n!=g.n || w.allocated_in!=p.in ||
       w.allocated_heads!=p.heads || w.allocated_dim!=p.dim)
        throw std::runtime_error("ICPP Online workspace or parameter shape mismatch");
    if(w.thread_capacity<omp_get_max_threads())
        throw std::runtime_error("ICPP Online workspace thread capacity exceeded; reallocate");
    size_t nk=checked(g.n,p.heads),nw=checked(g.n,p.width());
    int dp=(p.in+31)/32*32,obs=(p.dim+15)/16;
    if(g.row.size()!=g.n+1 || g.col.size()!=g.e || sched.perm.size()!=g.n ||
       q.padded_in!=dp || q.output_blocks!=obs ||
       q.packed.size()!=size_t(p.heads)*(dp/32)*obs*512 ||
       w.base.xbf.size()!=checked(g.n,p.in) || w.base.left.size()!=nk ||
       w.base.right.size()!=nk || w.base.max.size()!=nk ||
       w.base.den.size()!=nk || w.base.out.size()!=nw ||
       w.base.thread_hbuf.size()<size_t(w.thread_capacity)*16*dp ||
       w.base.thread_cbuf.size()<size_t(w.thread_capacity)*16*64)
        throw std::runtime_error("ICPP Online CSR, packed weights, or buffer size mismatch");
    if(counters && (w.row_blocks.size()!=nk || w.row_max_updates.size()!=nk || w.row_rescales.size()!=nk))
        throw std::runtime_error("ICPP Online counter arrays not allocated");
}
}

void Workspace::allocate(const Graph& g,const Param& p,bool counters) {
    p.validate();
    if(!g.n || g.row.size()!=g.n+1 || g.col.size()!=g.e)
        throw std::runtime_error("ICPP Online requires a validated destination CSR");
    base=gat::Workspace{};base.allocate(g,p,"tfs_bf16");
    allocated_n=g.n;allocated_in=p.in;allocated_heads=p.heads;allocated_dim=p.dim;
    thread_capacity=omp_get_max_threads();
    const size_t nk=checked(g.n,p.heads);
    if(counters) {
        row_blocks.assign(nk,0);row_max_updates.assign(nk,0);row_rescales.assign(nk,0);
    } else {
        row_blocks.clear();row_max_updates.clear();row_rescales.clear();
    }
}
size_t Workspace::bytes() const {
    return base.bytes()+(row_blocks.size()+row_max_updates.size()+row_rescales.size())*sizeof(uint64_t);
}

void aggregate(const Graph& g,const Param& p,const Prepared& q,const Schedule& sched,
               Workspace& w,int block,int panel,Stats& stats,int profile_period,bool counters) {
    validate_call(g,p,q,sched,w,block,panel,profile_period,counters);
    stats=Stats{};
    std::atomic<bool> permission_failed{false};
    #pragma omp parallel
    {
        Stats local;
        auto ct=profile_period>0?Clock::now():Clock::time_point{};
        if(syscall(SYS_arch_prctl,0x1023,18)!=0)permission_failed.store(true);
        #pragma omp barrier
        if(!permission_failed.load()) {
            TileConfig cfg;_tile_loadconfig(&cfg);
            if(profile_period>0)local.tile_config+=seconds(ct,Clock::now());
            const int dp=q.padded_in;
            BF16* hbuf=w.base.thread_hbuf.data()+size_t(omp_get_thread_num())*16*dp;
            float* cbuf=w.base.thread_cbuf.data()+size_t(omp_get_thread_num())*16*64;
            // Persistent region and original independent-head/dynamic-panel
            // schedule. The omp-for barrier keeps each head's execution legal.
            for(int h=0;h<p.heads;++h) {
                #pragma omp for schedule(dynamic,1)
                for(uint64_t rg=0;rg<g.n;rg+=uint64_t(panel)) {
                    uint64_t rg_end=std::min(g.n,rg+uint64_t(panel));
                    for(uint64_t first=rg;first<rg_end;first+=16) {
                        int batch=int(std::min<uint64_t>(16,rg_end-first));
                        uint64_t ordinal=uint64_t(h)*((g.n+15)/16)+first/16;
                        bool measure=profile_period>0 && ordinal%uint64_t(profile_period)==0;
                        if(counters || profile_period>0)++local.total_tiles;
                        if(measure)++local.sampled_tiles;
                        if(measure) {
                            if(counters)run_tile<true,true>(g,p,q,sched,w,first,batch,h,block,hbuf,cbuf,local);
                            else run_tile<true,false>(g,p,q,sched,w,first,batch,h,block,hbuf,cbuf,local);
                        } else {
                            if(counters)run_tile<false,true>(g,p,q,sched,w,first,batch,h,block,hbuf,cbuf,local);
                            else run_tile<false,false>(g,p,q,sched,w,first,batch,h,block,hbuf,cbuf,local);
                        }
                    }
                }
            }
            _tile_release();
        }
        if(counters || profile_period>0) {
            #pragma omp critical(icpp_online_stats)
            combine(stats,local);
        }
    }
    if(permission_failed.load())throw std::runtime_error("ICPP Online AMX permission request failed on worker");
}

void layer(const Graph& g,const Param& p,const Prepared& q,const Schedule& sched,
           const std::vector<float>& input,Workspace& w,bool hidden,int block,
           int panel,Timing& t,Stats& stats,int profile_period,bool counters,
           const std::string& lr_policy) {
    validate_call(g,p,q,sched,w,block,panel,profile_period,counters);
    if(input.size()!=checked(g.n,p.in))throw std::runtime_error("ICPP Online input shape mismatch");
    const size_t blr_count=checked(p.in,2*p.heads);
    if(w.base.lr.size()!=checked(g.n,2*p.heads) ||
       (lr_policy=="fp32" && q.blr.size()!=blr_count) ||
       (lr_policy=="bf16" && q.blr_bf16.size()!=blr_count) ||
       (lr_policy=="avx" && q.blr_dot.size()!=blr_count) ||
       (lr_policy!="fp32" && lr_policy!="bf16" && lr_policy!="avx"))
        throw std::runtime_error("ICPP Online attention policy or prepared buffer mismatch");
    t=Timing{};
    auto begin=Clock::now(),a=begin;
    convert(input.data(),w.base.xbf.data(),input.size());t.convert=seconds(a,Clock::now());
    a=Clock::now();lr_reordered(g,p,q,input,w.base,lr_policy);t.lr=seconds(a,Clock::now());
    a=Clock::now();aggregate(g,p,q,sched,w,block,panel,stats,profile_period,counters);
    t.kernel=seconds(a,Clock::now());
    gat::Times finish_time;finish(g,p,w.base,hidden,finish_time);
    t.normalize=finish_time.normalize;t.activation=finish_time.activation;
    t.total=seconds(begin,Clock::now());
}
}
