#include "icpp_block.hpp"
#include <atomic>
#include <sys/syscall.h>
#include <unistd.h>

namespace gat::icpp_block {
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
inline void store_outputs(float* cbuf,int obs) {
    _tile_stored(0,cbuf,64);
    if(obs>1)_tile_stored(1,cbuf+256,64);
    if(obs>2)_tile_stored(2,cbuf+512,64);
    if(obs>3)_tile_stored(3,cbuf+768,64);
}
inline void reload_outputs(const float* cbuf,int obs) {
    _tile_loadd(0,cbuf,64);
    if(obs>1)_tile_loadd(1,cbuf+256,64);
    if(obs>2)_tile_loadd(2,cbuf+512,64);
    if(obs>3)_tile_loadd(3,cbuf+768,64);
}

template<bool Measure,bool Counters>
void run_tile(const Graph& g,const Param& p,const Prepared& q,
              const Schedule& sched,Workspace& w,uint64_t first,int batch,
              int h,int block,float* ubuf,BF16* bbuf,float* cbuf,Stats& stats) {
    auto& base=w.original.base;
    auto& s=stats.baseline;
    const int dp=q.padded_in,kblocks=dp/32,obs=q.output_blocks;
    auto st=stamp<Measure>();
    uint32_t rows[16]={};
    uint64_t starts[16]={},degrees[16]={};
    uint64_t max_degree=0;
    alignas(64) float left[16]={},maximum[16],den[16]={};
    for(int n=0;n<16;++n)maximum[n]=-std::numeric_limits<float>::infinity();
    for(int n=0;n<batch;++n) {
        rows[n]=sched.perm[first+n];starts[n]=g.row[rows[n]];
        degrees[n]=g.row[rows[n]+1]-starts[n];
        max_degree=std::max(max_degree,degrees[n]);
        size_t idx=size_t(rows[n])*p.heads+h;
        left[n]=base.left[idx];
        if constexpr(Counters)
            w.original.row_blocks[idx]=w.original.row_max_updates[idx]=w.original.row_rescales[idx]=0;
    }
    // V is initialized only once. No block is allowed to reset these tiles.
    _tile_zero(0);
    if(obs>1)_tile_zero(1);
    if(obs>2)_tile_zero(2);
    if(obs>3)_tile_zero(3);
    charge<Measure>(s.scheduling,st);

    alignas(64) float scores[64][16];
    uint32_t sources[64][16];
    __mmask16 valid_masks[64];
    for(uint64_t begin=0;begin<max_degree;begin+=uint64_t(block)) {
        const int count=int(std::min<uint64_t>(block,max_degree-begin));
        alignas(64) float block_maximum[16];
        for(int n=0;n<16;++n)block_maximum[n]=-std::numeric_limits<float>::infinity();
        for(int b=0;b<count;++b) {
            st=stamp<Measure>();
            alignas(64) float right[16]={};
            unsigned valid=0;
            uint64_t step=begin+uint64_t(b);
            for(int n=0;n<batch;++n)if(step<degrees[n]) {
                valid|=1u<<n;
                sources[b][n]=g.col[starts[n]+step];
                right[n]=base.right[size_t(sources[b][n])*p.heads+h];
            }
            valid_masks[b]=__mmask16(valid);
            __m512 e=leaky(_mm512_add_ps(_mm512_load_ps(left),_mm512_load_ps(right)));
            e=_mm512_mask_mov_ps(_mm512_set1_ps(-std::numeric_limits<float>::infinity()),valid_masks[b],e);
            _mm512_store_ps(scores[b],e);
            charge<Measure>(s.score_generation,st);
            st=stamp<Measure>();
            _mm512_store_ps(block_maximum,_mm512_max_ps(_mm512_load_ps(block_maximum),e));
            charge<Measure>(s.block_max,st);
        }
        st=stamp<Measure>();
        alignas(64) float previous_max[16],rescale[16];
        std::memcpy(previous_max,maximum,sizeof(previous_max));
        _mm512_store_ps(maximum,_mm512_max_ps(_mm512_load_ps(maximum),_mm512_load_ps(block_maximum)));
        unsigned changed=0;
        for(int n=0;n<batch;++n)if(begin<degrees[n]) {
            bool update=maximum[n]>previous_max[n];
            bool do_rescale=update && den[n]>0;
            if(do_rescale)changed|=1u<<n;
            if constexpr(Counters) {
                size_t idx=size_t(rows[n])*p.heads+h;
                ++s.blocks;++w.original.row_blocks[idx];
                if(update){++s.max_updates;++w.original.row_max_updates[idx];}
                if(do_rescale) {
                    ++s.rescales;++w.original.row_rescales[idx];
                    s.rescaled_feature_elements+=uint64_t(p.dim);
                    s.rescaled_padded_feature_elements+=uint64_t(obs)*16;
                }
                stats.useful_projection_FMA+=uint64_t(p.in)*p.dim;
            }
        }
        charge<Measure>(s.block_max,st);

        // Identical m/l/exp update shape to icpp_online; no new softmax scheme.
        st=stamp<Measure>();
        __m512 shift=_mm512_maskz_sub_ps(__mmask16(changed),
                                       _mm512_load_ps(previous_max),_mm512_load_ps(maximum));
        __m512 r=_mm512_exp_ps(shift);
        _mm512_store_ps(rescale,r);
        _mm512_store_ps(den,_mm512_mul_ps(_mm512_load_ps(den),r));
        for(int b=0;b<count;++b) {
            __m512 shifted=_mm512_maskz_sub_ps(valid_masks[b],
                                             _mm512_load_ps(scores[b]),_mm512_load_ps(maximum));
            __m512 weight=_mm512_maskz_mov_ps(valid_masks[b],_mm512_exp_ps(shifted));
            _mm512_store_ps(scores[b],weight);
            _mm512_store_ps(den,_mm512_add_ps(_mm512_load_ps(den),weight));
        }
        charge<Measure>(s.exp_den,st);

        // Only old V changes scale. Temporary U_B is about to start from zero,
        // contains no past neighbors, and must never receive this rescale.
        if(changed) {
            st=stamp<Measure>();store_outputs(cbuf,obs);
            charge<Measure>(s.rescale_store,st);
            st=stamp<Measure>();
            for(int ob=0;ob<obs;++ob)for(int n=0;n<batch;++n)if(changed&(1u<<n)) {
                float* row=cbuf+size_t(ob)*256+n*16;
                _mm512_storeu_ps(row,_mm512_mul_ps(_mm512_loadu_ps(row),_mm512_set1_ps(rescale[n])));
            }
            charge<Measure>(s.rescale_vector,st);
            st=stamp<Measure>();reload_outputs(cbuf,obs);
            charge<Measure>(s.rescale_reload,st);
            if constexpr(Counters) {
                ++s.rescale_events;s.spill_bytes+=uint64_t(obs)*1024;
                s.reload_bytes+=uint64_t(obs)*1024;
            }
        }
        st=stamp<Measure>();
        std::fill(ubuf,ubuf+size_t(16)*dp,0.f);
        charge<Measure>(s.scheduling,st);

        // Transient U_B[r,D] = sum_{j in this block} p[r,j] * BF16(H_j).
        // For each feature vector, the entire neighbor-block reduction stays
        // in a ZMM and is written once. U_B is consumed immediately below.
        st=stamp<Measure>();
        for(int n=0;n<batch;++n)if(begin<degrees[n]) {
            int row_count=int(std::min<uint64_t>(count,degrees[n]-begin));
            float* dst=ubuf+size_t(n)*dp;
            for(int k=0;k<p.in;k+=16) {
                int len=std::min(16,p.in-k);
                __m512 accumulator=_mm512_setzero_ps();
                for(int b=0;b<row_count;++b) {
                    const BF16* src=base.xbf.data()+size_t(sources[b][n])*p.in+k;
                    __m256i raw;
                    if(len==16)raw=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(src));
                    else {
                        alignas(32) BF16 tail[16]={};
                        std::memcpy(tail,src,size_t(len)*sizeof(BF16));
                        raw=_mm256_load_si256(reinterpret_cast<const __m256i*>(tail));
                    }
                    __m512 value=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(raw),16));
                    accumulator=_mm512_fmadd_ps(_mm512_set1_ps(scores[b][n]),value,accumulator);
                    if constexpr(Counters) {
                        stats.actual_input_feature_FMA+=uint64_t(len);
                        stats.padded_input_feature_FMA+=16;
                        stats.logical_H_bytes+=uint64_t(len)*sizeof(BF16);
                    }
                }
                _mm512_storeu_ps(dst+k,accumulator);
            }
        }
        charge<Measure>(s.source_gather,st);

        // This moves quantization from each pH edge to its block aggregate.
        // Consequently output is not claimed bitwise-identical to old BF16 TFS.
        st=stamp<Measure>();
        for(size_t at=0;at<size_t(16)*dp;at+=16) {
            __m256bh packed=_mm512_cvtneps_pbh(_mm512_loadu_ps(ubuf+at));
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(bbuf+at),(__m256i)packed);
        }
        charge<Measure>(s.weighted_convert,st);

        // Real TFS tile fusion: immediately project this local block into V.
        // Output tiles were NOT zeroed here, and survive for the next block.
        st=stamp<Measure>();
        for(int kb=0;kb<kblocks;++kb) {
            const BF16* wb=q.packed.data()+((size_t(h)*kblocks+kb)*obs)*512;
            _tile_loadd(4,bbuf+kb*32,dp*2);
            _tile_loadd(5,wb,64);_tile_dpbf16ps(0,4,5);
            if(obs>1){_tile_loadd(5,wb+512,64);_tile_dpbf16ps(1,4,5);}
            if(obs>2){_tile_loadd(5,wb+1024,64);_tile_dpbf16ps(2,4,5);}
            if(obs>3){_tile_loadd(5,wb+1536,64);_tile_dpbf16ps(3,4,5);}
        }
        charge<Measure>(s.amx_compute,st);
        if constexpr(Counters) {
            ++stats.block_projection_calls;
            stats.conversion_elements+=uint64_t(16)*dp;
            stats.source_neighbor_steps+=uint64_t(count);
            ++s.neighbor_steps; // actual AMX accumulation step: one block
            s.amx_calls+=uint64_t(kblocks)*obs;
            s.executed_fma+=uint64_t(16)*dp*obs*16;
        }
    }
    st=stamp<Measure>();store_outputs(cbuf,obs);
    charge<Measure>(s.final_store,st);
    st=stamp<Measure>();
    for(int n=0;n<batch;++n) {
        size_t idx=size_t(rows[n])*p.heads+h;
        base.max[idx]=maximum[n];base.den[idx]=den[n];
        float* dst=base.out.data()+size_t(rows[n])*p.width()+h*p.dim;
        for(int ob=0;ob<obs;++ob)
            std::memcpy(dst+ob*16,cbuf+ob*256+n*16,size_t(std::min(16,p.dim-ob*16))*4);
    }
    charge<Measure>(s.output_write,st);
}

void combine(Stats& dst,const Stats& src) {
    auto& d=dst.baseline;const auto& s=src.baseline;
#define ADD(name) d.name+=s.name
    ADD(score_generation);ADD(block_max);ADD(exp_den);ADD(rescale_store);
    ADD(rescale_vector);ADD(rescale_reload);ADD(source_gather);ADD(weighted_convert);
    ADD(tile_load);ADD(amx_compute);ADD(final_store);ADD(output_write);
    ADD(scheduling);ADD(tile_config);ADD(blocks);ADD(max_updates);ADD(rescales);
    ADD(rescaled_feature_elements);ADD(rescaled_padded_feature_elements);
    ADD(spill_bytes);ADD(reload_bytes);ADD(rescale_events);ADD(amx_calls);
    ADD(neighbor_steps);ADD(executed_fma);ADD(sampled_tiles);ADD(total_tiles);
#undef ADD
#define ADD_EXTRA(name) dst.name+=src.name
    ADD_EXTRA(actual_input_feature_FMA);ADD_EXTRA(padded_input_feature_FMA);
    ADD_EXTRA(useful_projection_FMA);ADD_EXTRA(logical_H_bytes);
    ADD_EXTRA(block_projection_calls);ADD_EXTRA(conversion_elements);
    ADD_EXTRA(source_neighbor_steps);
#undef ADD_EXTRA
}
void validate_call(const Graph& g,const Param& p,const Prepared& q,
                   const Schedule& sched,const Workspace& w,int block,int panel,
                   int profile_period,bool counters) {
    const auto& o=w.original;
    if(block<1 || block>64)throw std::runtime_error("ICPP block size must be in [1,64]");
    if(panel<16 || panel%16)throw std::runtime_error("ICPP block panel must be a multiple of 16");
    if(profile_period<0)throw std::runtime_error("ICPP block profile period must be nonnegative");
    if(!g.n || p.in<=0 || p.heads<1 || p.heads>8 || p.dim<1 || p.dim>64 ||
       o.allocated_n!=g.n || o.allocated_in!=p.in ||
       o.allocated_heads!=p.heads || o.allocated_dim!=p.dim)
        throw std::runtime_error("ICPP block workspace or parameter shape mismatch");
    if(o.thread_capacity<omp_get_max_threads())
        throw std::runtime_error("ICPP block workspace thread capacity exceeded; reallocate");
    size_t nk=checked(g.n,p.heads),nw=checked(g.n,p.width());
    int dp=(p.in+31)/32*32,obs=(p.dim+15)/16;
    size_t scratch=size_t(o.thread_capacity)*16*dp;
    if(g.row.size()!=g.n+1 || g.col.size()!=g.e || sched.perm.size()!=g.n ||
       q.padded_in!=dp || q.output_blocks!=obs ||
       q.packed.size()!=size_t(p.heads)*(dp/32)*obs*512 ||
       o.base.xbf.size()!=checked(g.n,p.in) || o.base.left.size()!=nk ||
       o.base.right.size()!=nk || o.base.max.size()!=nk || o.base.den.size()!=nk ||
       o.base.out.size()!=nw || o.base.thread_cbuf.size()<size_t(o.thread_capacity)*16*64 ||
       w.ubuf.size()<scratch || w.bbuf.size()<scratch)
        throw std::runtime_error("ICPP block CSR, packed weights, or buffer size mismatch");
    if(counters && (o.row_blocks.size()!=nk || o.row_max_updates.size()!=nk || o.row_rescales.size()!=nk))
        throw std::runtime_error("ICPP block counter arrays not allocated");
}
}

void Workspace::allocate(const Graph& g,const Param& p,bool counters) {
    original.allocate(g,p,counters);
    size_t per_thread=checked(16,(p.in+31)/32*32);
    size_t count=checked(original.thread_capacity,per_thread);
    ubuf.resize(count);bbuf.resize(count);
}
size_t Workspace::bytes() const {
    return original.bytes()+ubuf.size()*sizeof(float)+bbuf.size()*sizeof(BF16);
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
            if(profile_period>0)local.baseline.tile_config+=seconds(ct,Clock::now());
            int worker=omp_get_thread_num(),dp=q.padded_in;
            float* ubuf=w.ubuf.data()+size_t(worker)*16*dp;
            BF16* bbuf=w.bbuf.data()+size_t(worker)*16*dp;
            float* cbuf=w.original.base.thread_cbuf.data()+size_t(worker)*16*64;
            for(int h=0;h<p.heads;++h) {
                #pragma omp for schedule(dynamic,1)
                for(uint64_t rg=0;rg<g.n;rg+=uint64_t(panel)) {
                    uint64_t end=std::min(g.n,rg+uint64_t(panel));
                    for(uint64_t first=rg;first<end;first+=16) {
                        int batch=int(std::min<uint64_t>(16,end-first));
                        uint64_t ordinal=uint64_t(h)*((g.n+15)/16)+first/16;
                        bool measure=profile_period>0 && ordinal%uint64_t(profile_period)==0;
                        if(counters || profile_period>0)++local.baseline.total_tiles;
                        if(measure)++local.baseline.sampled_tiles;
                        if(measure) {
                            if(counters)run_tile<true,true>(g,p,q,sched,w,first,batch,h,block,ubuf,bbuf,cbuf,local);
                            else run_tile<true,false>(g,p,q,sched,w,first,batch,h,block,ubuf,bbuf,cbuf,local);
                        } else {
                            if(counters)run_tile<false,true>(g,p,q,sched,w,first,batch,h,block,ubuf,bbuf,cbuf,local);
                            else run_tile<false,false>(g,p,q,sched,w,first,batch,h,block,ubuf,bbuf,cbuf,local);
                        }
                    }
                }
            }
            _tile_release();
        }
        if(counters || profile_period>0) {
            #pragma omp critical(icpp_block_stats)
            combine(stats,local);
        }
    }
    if(permission_failed.load())throw std::runtime_error("ICPP block AMX permission request failed on worker");
}
void layer(const Graph& g,const Param& p,const Prepared& q,const Schedule& sched,
           const std::vector<float>& input,Workspace& w,bool hidden,int block,
           int panel,Timing& t,Stats& stats,int profile_period,bool counters,
           const std::string& lr_policy) {
    validate_call(g,p,q,sched,w,block,panel,profile_period,counters);
    if(input.size()!=checked(g.n,p.in))throw std::runtime_error("ICPP block input shape mismatch");
    size_t blr_count=checked(p.in,2*p.heads);
    if(w.original.base.lr.size()!=checked(g.n,2*p.heads) ||
       (lr_policy=="fp32" && q.blr.size()!=blr_count) ||
       (lr_policy=="bf16" && q.blr_bf16.size()!=blr_count) ||
       (lr_policy=="avx" && q.blr_dot.size()!=blr_count) ||
       (lr_policy!="fp32" && lr_policy!="bf16" && lr_policy!="avx"))
        throw std::runtime_error("ICPP block attention policy or prepared buffer mismatch");
    t=Timing{};
    auto begin=Clock::now(),a=begin;
    convert(input.data(),w.original.base.xbf.data(),input.size());t.convert=seconds(a,Clock::now());
    a=Clock::now();lr_reordered(g,p,q,input,w.original.base,lr_policy);t.lr=seconds(a,Clock::now());
    a=Clock::now();aggregate(g,p,q,sched,w,block,panel,stats,profile_period,counters);
    t.kernel=seconds(a,Clock::now());
    gat::Times finish_time;finish(g,p,w.original.base,hidden,finish_time);
    t.normalize=finish_time.normalize;t.activation=finish_time.activation;
    t.total=seconds(begin,Clock::now());
}
}
