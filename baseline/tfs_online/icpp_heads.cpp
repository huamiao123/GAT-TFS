#include "icpp_heads.hpp"
#include <array>
#include <atomic>
#include <sys/syscall.h>
#include <unistd.h>

namespace gat::icpp_heads {
namespace {
constexpr int K=8,TR=16,HD=32;
enum class Backend {SharedAVX,AMXHighLow};
struct alignas(64) TileConfig {
    uint8_t palette=1,start=0,reserved[14]={};
    uint16_t cols[16]={};uint8_t rows[16]={};
    TileConfig() {
        for(int t:{0,1,4,5}){cols[t]=64;rows[t]=16;}
        for(int t:{6,7}){cols[t]=64;rows[t]=8;}
    }
};
static_assert(sizeof(TileConfig)==64);
template<bool Measure> inline Clock::time_point stamp() {
    if constexpr(Measure)return Clock::now();else return {};
}
template<bool Measure> inline void charge(double& field,Clock::time_point begin) {
    if constexpr(Measure)field+=seconds(begin,Clock::now());
}
inline __m512 expanded(__m256i raw) {
    return _mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(raw),16));
}
inline __m256i source_vector(const BF16* src,int len) {
    if(len==16)return _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src));
    alignas(32) BF16 tail[16]={};
    std::memcpy(tail,src,size_t(len)*sizeof(BF16));
    return _mm256_load_si256(reinterpret_cast<const __m256i*>(tail));
}
Backend backend_value(const std::string& backend) {
    if(backend=="avx_shared")return Backend::SharedAVX;
    if(backend=="amx_hi_lo")return Backend::AMXHighLow;
    throw std::runtime_error("ICPP heads backend must be avx_shared or amx_hi_lo");
}
bool optimized_shape(const Param& p) {
    return p.heads==K && p.dim==HD && p.in>0 && p.in<=256;
}

// Each destination consumes one bounded neighbor block. P rows are the eight
// independent heads; every source H vector is shared between these rows.
template<bool Measure,bool Counters,Backend Method>
void run_tile(const Graph& g,const Param& p,const Prepared& q,const Schedule& sched,
              Workspace& w,uint64_t first,int batch,int block,float* ubuf,
              float* vbuf,BF16* hpacked,BF16* uwbuf,Stats& stats) {
    auto& original=w.fallback.original;auto& base=original.base;
    auto& s=stats.baseline;const int dp=q.padded_in,kblocks=dp/32;
    uint32_t rows[TR]={};uint64_t starts[TR]={},degrees[TR]={},max_degree=0;
    alignas(64) float left[K][TR]={},maximum[K][TR],den[K][TR]={};
    auto a=stamp<Measure>();
    for(int h=0;h<K;++h)for(int n=0;n<TR;++n)
        maximum[h][n]=-std::numeric_limits<float>::infinity();
    for(int n=0;n<batch;++n) {
        rows[n]=sched.perm[first+n];starts[n]=g.row[rows[n]];
        degrees[n]=g.row[rows[n]+1]-starts[n];max_degree=std::max(max_degree,degrees[n]);
        for(int h=0;h<K;++h) {
            size_t idx=size_t(rows[n])*K+h;left[h][n]=base.left[idx];
            if constexpr(Counters)
                original.row_blocks[idx]=original.row_max_updates[idx]=original.row_rescales[idx]=0;
        }
    }
    // vbuf is tile-major: [head][output16][destination16][feature16].
    std::fill(vbuf,vbuf+size_t(K)*512,0.f);
    charge<Measure>(s.scheduling,a);
    alignas(64) float probability[K][32][TR];
    uint32_t sources[32][TR];__mmask16 valid_masks[32];
    alignas(64) float rescale[K][TR];
    unsigned changed[K]={};
    alignas(64) BF16 phi[K*32],plo[K*32];
    alignas(64) float ph_result[K*16];

    for(uint64_t begin=0;begin<max_degree;begin+=uint64_t(block)) {
        int count=int(std::min<uint64_t>(block,max_degree-begin));
        a=stamp<Measure>();
        for(int b=0;b<count;++b) {
            unsigned valid=0;uint64_t step=begin+uint64_t(b);
            for(int n=0;n<batch;++n)if(step<degrees[n]) {
                valid|=1u<<n;sources[b][n]=g.col[starts[n]+step];
            }
            valid_masks[b]=__mmask16(valid);
        }
        charge<Measure>(stats.source_index,a);
        for(int h=0;h<K;++h) {
            alignas(64) float block_max[TR],previous[TR];
            for(int n=0;n<TR;++n)block_max[n]=-std::numeric_limits<float>::infinity();
            for(int b=0;b<count;++b) {
                a=stamp<Measure>();alignas(64) float right[TR]={};
                for(int n=0;n<batch;++n)if(unsigned(valid_masks[b])&(1u<<n))
                    right[n]=base.right[size_t(sources[b][n])*K+h];
                __m512 e=leaky(_mm512_add_ps(_mm512_load_ps(left[h]),_mm512_load_ps(right)));
                e=_mm512_mask_mov_ps(_mm512_set1_ps(-std::numeric_limits<float>::infinity()),valid_masks[b],e);
                _mm512_store_ps(probability[h][b],e);charge<Measure>(s.score_generation,a);
                a=stamp<Measure>();
                _mm512_store_ps(block_max,_mm512_max_ps(_mm512_load_ps(block_max),e));
                charge<Measure>(s.block_max,a);
            }
            a=stamp<Measure>();std::memcpy(previous,maximum[h],sizeof(previous));
            _mm512_store_ps(maximum[h],_mm512_max_ps(_mm512_load_ps(maximum[h]),_mm512_load_ps(block_max)));
            changed[h]=0;
            for(int n=0;n<batch;++n)if(begin<degrees[n]) {
                bool update=maximum[h][n]>previous[n],scale=update && den[h][n]>0;
                if(scale)changed[h]|=1u<<n;
                if constexpr(Counters) {
                    size_t idx=size_t(rows[n])*K+h;++s.blocks;++original.row_blocks[idx];
                    if(update){++s.max_updates;++original.row_max_updates[idx];}
                    if(scale) {
                        ++s.rescales;++original.row_rescales[idx];
                        s.rescaled_feature_elements+=HD;s.rescaled_padded_feature_elements+=HD;
                    }
                }
            }
            charge<Measure>(s.block_max,a);a=stamp<Measure>();
            __m512 shift=_mm512_maskz_sub_ps(__mmask16(changed[h]),_mm512_load_ps(previous),_mm512_load_ps(maximum[h]));
            __m512 r=_mm512_exp_ps(shift);_mm512_store_ps(rescale[h],r);
            _mm512_store_ps(den[h],_mm512_mul_ps(_mm512_load_ps(den[h]),r));
            for(int b=0;b<count;++b) {
                __m512 shifted=_mm512_maskz_sub_ps(valid_masks[b],_mm512_load_ps(probability[h][b]),_mm512_load_ps(maximum[h]));
                __m512 weight=_mm512_maskz_mov_ps(valid_masks[b],_mm512_exp_ps(shifted));
                _mm512_store_ps(probability[h][b],weight);
                _mm512_store_ps(den[h],_mm512_add_ps(_mm512_load_ps(den[h]),weight));
            }
            charge<Measure>(s.exp_den,a);
        }

        a=stamp<Measure>();
        // Valid PH rows overwrite their entire feature range. Only inactive
        // destination rows and AVX's remaining D32 pad need zero writes.
        // This avoids a full 128 KiB UB reset before every populated D256 block.
        for(int h=0;h<K;++h)for(int n=0;n<TR;++n) {
            float* rowU=ubuf+(size_t(h)*TR+n)*dp;
            if(n>=batch || begin>=degrees[n])std::fill(rowU,rowU+dp,0.f);
            else if constexpr(Method==Backend::SharedAVX) {
                int initialized=(p.in+15)/16*16;
                std::fill(rowU+initialized,rowU+dp,0.f);
            }
        }
        charge<Measure>(stats.transition,a);
        for(int n=0;n<batch;++n)if(begin<degrees[n]) {
            int row_count=int(std::min<uint64_t>(count,degrees[n]-begin));
            if constexpr(Counters) {
                stats.ph_useful_fma+=uint64_t(K)*row_count*p.in;
                stats.raw_H_bytes+=uint64_t(row_count)*p.in*sizeof(BF16);
            }
            if constexpr(Method==Backend::SharedAVX) {
                a=stamp<Measure>();
                for(int feature=0;feature<p.in;feature+=16) {
                    int len=std::min(16,p.in-feature);
                    std::array<__m512,K> accum;
                    #pragma unroll
                    for(int h=0;h<K;++h)accum[h]=_mm512_setzero_ps();
                    for(int b=0;b<row_count;++b) {
                        const BF16* src=base.xbf.data()+size_t(sources[b][n])*p.in+feature;
                        __m512 value=expanded(source_vector(src,len));
                        #pragma unroll
                        for(int h=0;h<K;++h)
                            accum[h]=_mm512_fmadd_ps(_mm512_set1_ps(probability[h][b][n]),value,accum[h]);
                    }
                    #pragma unroll
                    for(int h=0;h<K;++h)
                        _mm512_storeu_ps(ubuf+(size_t(h)*TR+n)*dp+feature,accum[h]);
                }
                charge<Measure>(stats.PH_compute,a);
                if constexpr(Counters) {
                    stats.ph_physical_fma+=uint64_t(K)*row_count*((p.in+15)/16*16);
                    stats.ub_scatter_bytes+=uint64_t(K)*((p.in+15)/16*16)*sizeof(float);
                }
            } else {
                // H RHS is [feature16][neighbor-pair16][feature BF16 pair].
                // Missing neighbors and D tail are always zero, including odd
                // row_count and the B16 half of the fixed K32 AMX operation.
                a=stamp<Measure>();
                for(int kp=0;kp<16;++kp)for(int ob=0;ob<dp/16;++ob) {
                    int feature=ob*16,len=std::max(0,std::min(16,p.in-feature));
                    __m256i raw_a=_mm256_setzero_si256(),raw_b=_mm256_setzero_si256();
                    if(len && 2*kp<row_count)
                        raw_a=source_vector(base.xbf.data()+size_t(sources[2*kp][n])*p.in+feature,len);
                    if(len && 2*kp+1<row_count)
                        raw_b=source_vector(base.xbf.data()+size_t(sources[2*kp+1][n])*p.in+feature,len);
                    __m512i lo=_mm512_cvtepu16_epi32(raw_a);
                    __m512i hi=_mm512_slli_epi32(_mm512_cvtepu16_epi32(raw_b),16);
                    _mm512_storeu_si512(hpacked+size_t(ob)*512+kp*32,_mm512_or_si512(lo,hi));
                }
                charge<Measure>(stats.H_pack,a);a=stamp<Measure>();
                for(int h=0;h<K;++h)for(int half=0;half<2;++half) {
                    alignas(64) float pvalues[16]={};
                    for(int b=0;b<16;++b)if(half*16+b<row_count)
                        pvalues[b]=probability[h][half*16+b][n];
                    __m512 values=_mm512_load_ps(pvalues);
                    __m256bh high=_mm512_cvtneps_pbh(values);
                    __m256bh low=_mm512_cvtneps_pbh(_mm512_sub_ps(values,expanded((__m256i)high)));
                    _mm256_store_si256(reinterpret_cast<__m256i*>(phi+h*32+half*16),(__m256i)high);
                    _mm256_store_si256(reinterpret_cast<__m256i*>(plo+h*32+half*16),(__m256i)low);
                }
                charge<Measure>(stats.P_pack,a);
                for(int ob=0;ob<dp/16;++ob) {
                    a=stamp<Measure>();_tile_zero(7);
                    _tile_loadd(5,hpacked+size_t(ob)*512,64);_tile_loadd(6,phi,64);
                    charge<Measure>(stats.PH_load,a);
                    a=stamp<Measure>();_tile_dpbf16ps(7,6,5);charge<Measure>(stats.PH_compute,a);
                    a=stamp<Measure>();_tile_loadd(6,plo,64);charge<Measure>(stats.PH_load,a);
                    a=stamp<Measure>();_tile_dpbf16ps(7,6,5);charge<Measure>(stats.PH_compute,a);
                    a=stamp<Measure>();_tile_stored(7,ph_result,64);
                    for(int h=0;h<K;++h)
                        std::memcpy(ubuf+(size_t(h)*TR+n)*dp+ob*16,ph_result+h*16,16*sizeof(float));
                    charge<Measure>(stats.PH_store_scatter,a);
                }
                if constexpr(Counters) {
                    stats.ph_calls+=uint64_t(dp/16)*2;
                    stats.ph_physical_fma+=uint64_t(K)*32*dp*2;
                    stats.packed_H_bytes+=uint64_t(32)*dp*sizeof(BF16);
                    stats.ub_scatter_bytes+=uint64_t(K)*dp*sizeof(float);
                }
            }
        }

        // One UW head at a time. V is explicitly saved in a bounded local
        // buffer across head switches and blocks. Rescaling is applied before
        // adding this block; no previous state exists in transient U_B.
        for(int h=0;h<K;++h) {
            float* headV=vbuf+size_t(h)*512;
            if(changed[h]) {
                a=stamp<Measure>();
                for(int ob=0;ob<2;++ob)for(int n=0;n<batch;++n)if(changed[h]&(1u<<n)) {
                    float* row=headV+ob*256+n*16;
                    _mm512_storeu_ps(row,_mm512_mul_ps(_mm512_loadu_ps(row),_mm512_set1_ps(rescale[h][n])));
                }
                if constexpr(Measure) {
                    double elapsed=seconds(a,Clock::now());stats.row_rescale+=elapsed;s.rescale_vector+=elapsed;
                }
                if constexpr(Counters)++s.rescale_events;
                // Extra rescale spill/reload is zero: head switching already
                // placed V in local memory. Its actual bytes are counted below.
            }
            a=stamp<Measure>();
            const float* headU=ubuf+size_t(h)*TR*dp;
            for(size_t at=0;at<size_t(TR)*dp;at+=16) {
                __m256bh packed=_mm512_cvtneps_pbh(_mm512_loadu_ps(headU+at));
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(uwbuf+at),(__m256i)packed);
            }
            charge<Measure>(stats.UB_convert,a);a=stamp<Measure>();
            if(begin) {_tile_loadd(0,headV,64);_tile_loadd(1,headV+256,64);}
            else {_tile_zero(0);_tile_zero(1);}
            charge<Measure>(stats.V_reload,a);
            for(int kb=0;kb<kblocks;++kb) {
                const BF16* weights=q.packed.data()+((size_t(h)*kblocks+kb)*2)*512;
                a=stamp<Measure>();_tile_loadd(4,uwbuf+kb*32,dp*sizeof(BF16));
                _tile_loadd(5,weights,64);charge<Measure>(stats.UW_load,a);
                a=stamp<Measure>();_tile_dpbf16ps(0,4,5);charge<Measure>(stats.UW_compute,a);
                a=stamp<Measure>();_tile_loadd(5,weights+512,64);charge<Measure>(stats.UW_load,a);
                a=stamp<Measure>();_tile_dpbf16ps(1,4,5);charge<Measure>(stats.UW_compute,a);
            }
            a=stamp<Measure>();_tile_stored(0,headV,64);_tile_stored(1,headV+256,64);
            charge<Measure>(stats.V_store,a);
            if constexpr(Counters) {
                ++stats.projection_calls;++s.neighbor_steps;s.amx_calls+=uint64_t(kblocks)*2;
                s.executed_fma+=uint64_t(TR)*dp*HD;
                stats.head_switch_store_bytes+=2048;
                if(begin)stats.head_switch_reload_bytes+=2048;
            }
        }
    }
    a=stamp<Measure>();
    for(int n=0;n<batch;++n)for(int h=0;h<K;++h) {
        size_t idx=size_t(rows[n])*K+h;base.max[idx]=maximum[h][n];base.den[idx]=den[h][n];
        float* output=base.out.data()+size_t(rows[n])*p.width()+h*HD;
        const float* headV=vbuf+size_t(h)*512;
        std::memcpy(output,headV+n*16,16*sizeof(float));
        std::memcpy(output+16,headV+256+n*16,16*sizeof(float));
    }
    charge<Measure>(s.output_write,a);
}

void combine(Stats& dst,const Stats& src) {
    auto& d=dst.baseline;const auto& s=src.baseline;
#define ADD_BASE(name) d.name+=s.name
    ADD_BASE(score_generation);ADD_BASE(block_max);ADD_BASE(exp_den);ADD_BASE(rescale_store);
    ADD_BASE(rescale_vector);ADD_BASE(rescale_reload);ADD_BASE(source_gather);ADD_BASE(weighted_convert);
    ADD_BASE(tile_load);ADD_BASE(amx_compute);ADD_BASE(final_store);ADD_BASE(output_write);
    ADD_BASE(scheduling);ADD_BASE(tile_config);ADD_BASE(blocks);ADD_BASE(max_updates);ADD_BASE(rescales);
    ADD_BASE(rescaled_feature_elements);ADD_BASE(rescaled_padded_feature_elements);
    ADD_BASE(spill_bytes);ADD_BASE(reload_bytes);ADD_BASE(rescale_events);ADD_BASE(amx_calls);
    ADD_BASE(neighbor_steps);ADD_BASE(executed_fma);ADD_BASE(sampled_tiles);ADD_BASE(total_tiles);
#undef ADD_BASE
#define ADD(name) dst.name+=src.name
    ADD(ph_calls);ADD(ph_physical_fma);ADD(ph_useful_fma);ADD(raw_H_bytes);ADD(packed_H_bytes);
    ADD(projection_calls);ADD(head_switch_store_bytes);ADD(head_switch_reload_bytes);ADD(ub_scatter_bytes);
    ADD(source_index);ADD(H_pack);ADD(P_pack);ADD(PH_load);ADD(PH_compute);ADD(PH_store_scatter);
    ADD(UB_convert);ADD(UW_load);ADD(UW_compute);ADD(V_store);ADD(V_reload);ADD(row_rescale);ADD(transition);
#undef ADD
}
void validate(const Graph& g,const Param& p,const Prepared& q,const Schedule& sched,
              const Workspace& w,int block,int panel,int profile_period,bool counters) {
    const auto& o=w.fallback.original;
    if(block!=16 && block!=32)throw std::runtime_error("optimized ICPP heads block must be 16 or 32");
    if(panel<16 || panel%16)throw std::runtime_error("ICPP heads panel must be a multiple of 16");
    if(profile_period<0)throw std::runtime_error("ICPP heads profile period must be nonnegative");
    if(!g.n || o.allocated_n!=g.n || o.allocated_in!=p.in || o.allocated_heads!=p.heads ||
       o.allocated_dim!=p.dim || o.thread_capacity<omp_get_max_threads())
        throw std::runtime_error("ICPP heads workspace shape/thread capacity mismatch");
    size_t nk=checked(g.n,K),nw=checked(g.n,K*HD);int dp=(p.in+31)/32*32;
    size_t workers=size_t(o.thread_capacity);
    if(g.row.size()!=g.n+1 || g.col.size()!=g.e || sched.perm.size()!=g.n ||
       q.padded_in!=dp || q.output_blocks!=2 || q.packed.size()!=size_t(K)*(dp/32)*2*512 ||
       o.base.xbf.size()!=checked(g.n,p.in) || o.base.left.size()!=nk || o.base.right.size()!=nk ||
       o.base.max.size()!=nk || o.base.den.size()!=nk || o.base.out.size()!=nw ||
       w.ubuf.size()<workers*K*TR*dp || w.vbuf.size()<workers*K*512 ||
       w.bbuf.size()<workers*32*dp || w.fallback.bbuf.size()<workers*TR*dp)
        throw std::runtime_error("ICPP heads CSR, prepared weights or local buffer shape mismatch");
    if(counters && (o.row_blocks.size()!=nk || o.row_max_updates.size()!=nk || o.row_rescales.size()!=nk))
        throw std::runtime_error("ICPP heads counter arrays not allocated");
}
template<Backend Method> void launch(const Graph& g,const Param& p,const Prepared& q,const Schedule& sched,
                                    Workspace& w,int block,int panel,Stats& stats,int profile_period,bool counters) {
    std::atomic<bool> permission_failed{false};
    #pragma omp parallel
    {
        Stats local;auto ct=profile_period>0?Clock::now():Clock::time_point{};
        if(syscall(SYS_arch_prctl,0x1023,18)!=0)permission_failed.store(true);
        #pragma omp barrier
        if(!permission_failed.load()) {
            TileConfig cfg;_tile_loadconfig(&cfg);
            if(profile_period>0)local.baseline.tile_config+=seconds(ct,Clock::now());
            int worker=omp_get_thread_num(),dp=q.padded_in;
            float* ubuf=w.ubuf.data()+size_t(worker)*K*TR*dp;
            float* vbuf=w.vbuf.data()+size_t(worker)*K*512;
            BF16* hpacked=w.bbuf.data()+size_t(worker)*32*dp;
            BF16* uwbuf=w.fallback.bbuf.data()+size_t(worker)*TR*dp;
            #pragma omp for schedule(dynamic,1)
            for(uint64_t rg=0;rg<g.n;rg+=uint64_t(panel)) {
                uint64_t end=std::min(g.n,rg+uint64_t(panel));
                for(uint64_t first=rg;first<end;first+=TR) {
                    int batch=int(std::min<uint64_t>(TR,end-first));
                    bool measure=profile_period>0 && (first/TR)%uint64_t(profile_period)==0;
                    if(counters || profile_period>0)local.baseline.total_tiles+=K;
                    if(measure)local.baseline.sampled_tiles+=K;
                    if(measure) {
                        if(counters)run_tile<true,true,Method>(g,p,q,sched,w,first,batch,block,ubuf,vbuf,hpacked,uwbuf,local);
                        else run_tile<true,false,Method>(g,p,q,sched,w,first,batch,block,ubuf,vbuf,hpacked,uwbuf,local);
                    } else {
                        if(counters)run_tile<false,true,Method>(g,p,q,sched,w,first,batch,block,ubuf,vbuf,hpacked,uwbuf,local);
                        else run_tile<false,false,Method>(g,p,q,sched,w,first,batch,block,ubuf,vbuf,hpacked,uwbuf,local);
                    }
                }
            }
            _tile_release();
        }
        if(counters || profile_period>0) {
            #pragma omp critical(icpp_heads_stats)
            combine(stats,local);
        }
    }
    if(permission_failed.load())throw std::runtime_error("ICPP heads AMX permission request failed on worker");
}
}

void Workspace::allocate(const Graph& g,const Param& p,bool counters) {
    fallback.allocate(g,p,counters);
    if(optimized_shape(p)) {
        size_t workers=size_t(fallback.original.thread_capacity),dp=size_t((p.in+31)/32*32);
        ubuf.resize(checked(workers,K*TR*dp));vbuf.resize(checked(workers,K*512));
        bbuf.resize(checked(workers,32*dp));
    } else {ubuf.clear();vbuf.clear();bbuf.clear();}
}
size_t Workspace::bytes() const {
    return fallback.bytes()+ubuf.size()*sizeof(float)+vbuf.size()*sizeof(float)+bbuf.size()*sizeof(BF16);
}
void aggregate(const Graph& g,const Param& p,const Prepared& q,const Schedule& sched,
               Workspace& w,int block,int panel,Stats& stats,int profile_period,bool counters,
               const std::string& backend) {
    Backend method=backend_value(backend);stats=Stats{};
    if(!optimized_shape(p)) {
        icpp_block::Stats fallback_stats;
        icpp_block::aggregate(g,p,q,sched,w.fallback,block,panel,fallback_stats,profile_period,counters);
        stats.baseline=fallback_stats.baseline;
        stats.projection_calls=fallback_stats.block_projection_calls;return;
    }
    validate(g,p,q,sched,w,block,panel,profile_period,counters);stats.optimized=true;
    if(method==Backend::SharedAVX)launch<Backend::SharedAVX>(g,p,q,sched,w,block,panel,stats,profile_period,counters);
    else launch<Backend::AMXHighLow>(g,p,q,sched,w,block,panel,stats,profile_period,counters);
}
void layer(const Graph& g,const Param& p,const Prepared& q,const Schedule& sched,
           const std::vector<float>& input,Workspace& w,bool hidden,int block,
           int panel,Timing& t,Stats& stats,int profile_period,bool counters,
           const std::string& lr_policy,const std::string& backend) {
    (void)backend_value(backend);
    if(!optimized_shape(p)) {
        icpp_block::Stats fallback_stats;
        icpp_block::layer(g,p,q,sched,input,w.fallback,hidden,block,panel,t,fallback_stats,profile_period,counters,lr_policy);
        stats=Stats{};stats.baseline=fallback_stats.baseline;
        stats.projection_calls=fallback_stats.block_projection_calls;return;
    }
    validate(g,p,q,sched,w,block,panel,profile_period,counters);
    if(input.size()!=checked(g.n,p.in))throw std::runtime_error("ICPP heads input shape mismatch");
    auto& base=w.fallback.original.base;size_t blr_count=checked(p.in,2*p.heads);
    if(base.lr.size()!=checked(g.n,2*p.heads) ||
       (lr_policy=="fp32" && q.blr.size()!=blr_count) ||
       (lr_policy=="bf16" && q.blr_bf16.size()!=blr_count) ||
       (lr_policy=="avx" && q.blr_dot.size()!=blr_count) ||
       (lr_policy!="fp32" && lr_policy!="bf16" && lr_policy!="avx"))
        throw std::runtime_error("ICPP heads attention policy or prepared buffer mismatch");
    t=Timing{};auto begin=Clock::now(),a=begin;
    convert(input.data(),base.xbf.data(),input.size());t.convert=seconds(a,Clock::now());
    a=Clock::now();lr_reordered(g,p,q,input,base,lr_policy);t.lr=seconds(a,Clock::now());
    a=Clock::now();aggregate(g,p,q,sched,w,block,panel,stats,profile_period,counters,backend);
    t.kernel=seconds(a,Clock::now());
    gat::Times finish_time;finish(g,p,base,hidden,finish_time);
    t.normalize=finish_time.normalize;t.activation=finish_time.activation;t.total=seconds(begin,Clock::now());
}
}
