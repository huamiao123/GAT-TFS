#include "icpp_pair.hpp"
#include <atomic>
#include <sys/syscall.h>
#include <unistd.h>

namespace gat::icpp_pair {
namespace {
struct alignas(64) TileConfig {
    uint8_t palette=1,start=0,reserved[14]={};
    uint16_t cols[16]={};
    uint8_t rows[16]={};
    TileConfig() {for(int t=0;t<6;++t){cols[t]=64;rows[t]=16;}}
};
static_assert(sizeof(TileConfig)==64);

inline bool paired_shape(const Param& p) {return p.heads%2==0 && p.dim<=32;}
template<bool Measure> inline Clock::time_point stamp() {
    if constexpr(Measure)return Clock::now();
    else return {};
}
template<bool Measure> inline void charge(double& field,Clock::time_point begin) {
    if constexpr(Measure)field+=seconds(begin,Clock::now());
}

// Fixed tile indices are required by the AMX intrinsics. Each head owns its
// output registers throughout the neighbor scan, including independent rescale.
template<int Head> inline void store_head(float* cbuf,int obs) {
    if constexpr(Head==0) {
        _tile_stored(0,cbuf,64);
        if(obs>1)_tile_stored(1,cbuf+256,64);
    } else {
        _tile_stored(2,cbuf,64);
        if(obs>1)_tile_stored(3,cbuf+256,64);
    }
}
template<int Head> inline void reload_head(const float* cbuf,int obs) {
    if constexpr(Head==0) {
        _tile_loadd(0,cbuf,64);
        if(obs>1)_tile_loadd(1,cbuf+256,64);
    } else {
        _tile_loadd(2,cbuf,64);
        if(obs>1)_tile_loadd(3,cbuf+256,64);
    }
}
template<int Head,bool Measure,bool Counters>
void rescale_head(float* cbuf,int obs,int batch,unsigned changed,
                  const float* rescale,icpp_online::Stats& stats) {
    if(!changed)return;
    auto st=stamp<Measure>();store_head<Head>(cbuf,obs);
    charge<Measure>(stats.rescale_store,st);
    st=stamp<Measure>();
    for(int ob=0;ob<obs;++ob)for(int n=0;n<batch;++n)if(changed&(1u<<n)) {
        float* row=cbuf+size_t(ob)*256+n*16;
        _mm512_storeu_ps(row,_mm512_mul_ps(_mm512_loadu_ps(row),
                                         _mm512_set1_ps(rescale[n])));
    }
    charge<Measure>(stats.rescale_vector,st);
    st=stamp<Measure>();reload_head<Head>(cbuf,obs);
    charge<Measure>(stats.rescale_reload,st);
    if constexpr(Counters) {
        ++stats.rescale_events;
        stats.spill_bytes+=uint64_t(obs)*1024;
        stats.reload_bytes+=uint64_t(obs)*1024;
    }
}

template<bool Measure,bool Counters>
void run_pair(const Graph& g,const Param& p,const Prepared& q,
              const Schedule& sched,Workspace& w,uint64_t first,int batch,
              int first_head,int block,BF16* hbuf,float* cbuf,Stats& stats) {
    auto& base=w.original.base;
    auto& count_stats=stats.baseline;
    const int dp=q.padded_in,kblocks=dp/32,obs=q.output_blocks;
    BF16* hbuf0=hbuf;
    BF16* hbuf1=hbuf+size_t(16)*dp;
    float* cbuf0=cbuf;
    float* cbuf1=cbuf+512;
    auto st=stamp<Measure>();
    uint32_t rows[16]={};
    uint64_t starts[16]={},degrees[16]={};
    uint64_t max_degree=0;
    alignas(64) float left[2][16]={},maximum[2][16],den[2][16]={};
    for(int a=0;a<2;++a)for(int n=0;n<16;++n)
        maximum[a][n]=-std::numeric_limits<float>::infinity();
    for(int n=0;n<batch;++n) {
        rows[n]=sched.perm[first+n];
        starts[n]=g.row[rows[n]];
        degrees[n]=g.row[rows[n]+1]-starts[n];
        max_degree=std::max(max_degree,degrees[n]);
        for(int a=0;a<2;++a) {
            size_t idx=size_t(rows[n])*p.heads+first_head+a;
            left[a][n]=base.left[idx];
            if constexpr(Counters)
                w.original.row_blocks[idx]=w.original.row_max_updates[idx]=w.original.row_rescales[idx]=0;
        }
    }
    std::fill(hbuf,hbuf+size_t(2)*16*dp,BF16(0));
    _tile_zero(0);_tile_zero(2);
    if(obs>1){_tile_zero(1);_tile_zero(3);}
    charge<Measure>(count_stats.scheduling,st);

    // Both heads have their own probabilities and maximum. CSR indices and
    // validity masks belong to the graph and can be shared without shared GAT
    // attention. Scratch stays bounded by 2 heads x 16 rows x 64 neighbors.
    alignas(64) float scores[2][64][16];
    uint32_t sources[64][16];
    __mmask16 valid_masks[64];
    for(uint64_t begin=0;begin<max_degree;begin+=uint64_t(block)) {
        int block_count=int(std::min<uint64_t>(block,max_degree-begin));
        alignas(64) float block_maximum[2][16];
        for(int a=0;a<2;++a)for(int n=0;n<16;++n)
            block_maximum[a][n]=-std::numeric_limits<float>::infinity();
        for(int b=0;b<block_count;++b) {
            st=stamp<Measure>();
            alignas(64) float right[2][16]={};
            unsigned valid=0;
            const uint64_t step=begin+uint64_t(b);
            for(int n=0;n<batch;++n)if(step<degrees[n]) {
                valid|=1u<<n;
                uint32_t source=g.col[starts[n]+step];
                sources[b][n]=source;
                right[0][n]=base.right[size_t(source)*p.heads+first_head];
                right[1][n]=base.right[size_t(source)*p.heads+first_head+1];
                if constexpr(Counters)++stats.source_index_loads;
            }
            valid_masks[b]=__mmask16(valid);
            for(int a=0;a<2;++a) {
                __m512 e=leaky(_mm512_add_ps(_mm512_load_ps(left[a]),_mm512_load_ps(right[a])));
                _mm512_store_ps(scores[a][b],_mm512_mask_mov_ps(
                    _mm512_set1_ps(-std::numeric_limits<float>::infinity()),valid_masks[b],e));
            }
            charge<Measure>(count_stats.score_generation,st);
            st=stamp<Measure>();
            for(int a=0;a<2;++a)
                _mm512_store_ps(block_maximum[a],_mm512_max_ps(
                    _mm512_load_ps(block_maximum[a]),_mm512_load_ps(scores[a][b])));
            charge<Measure>(count_stats.block_max,st);
        }

        alignas(64) float rescale[2][16];
        unsigned changed[2]={};
        for(int a=0;a<2;++a) {
            st=stamp<Measure>();
            alignas(64) float previous_max[16];
            std::memcpy(previous_max,maximum[a],sizeof(previous_max));
            _mm512_store_ps(maximum[a],_mm512_max_ps(_mm512_load_ps(maximum[a]),
                                                  _mm512_load_ps(block_maximum[a])));
            for(int n=0;n<batch;++n)if(begin<degrees[n]) {
                bool update=maximum[a][n]>previous_max[n];
                bool do_rescale=update && den[a][n]>0;
                if(do_rescale)changed[a]|=1u<<n;
                if constexpr(Counters) {
                    size_t idx=size_t(rows[n])*p.heads+first_head+a;
                    ++count_stats.blocks;++w.original.row_blocks[idx];
                    if(update){++count_stats.max_updates;++w.original.row_max_updates[idx];}
                    if(do_rescale) {
                        ++count_stats.rescales;++w.original.row_rescales[idx];
                        count_stats.rescaled_feature_elements+=uint64_t(p.dim);
                        count_stats.rescaled_padded_feature_elements+=uint64_t(obs)*16;
                    }
                }
            }
            charge<Measure>(count_stats.block_max,st);
            st=stamp<Measure>();
            // Keep precisely the original vector-exp form and reduction order:
            // changed lanes rescale, initial/finished/inactive lanes exp(0)=1.
            __m512 shift=_mm512_maskz_sub_ps(__mmask16(changed[a]),
                                            _mm512_load_ps(previous_max),
                                            _mm512_load_ps(maximum[a]));
            __m512 r=_mm512_exp_ps(shift);
            _mm512_store_ps(rescale[a],r);
            _mm512_store_ps(den[a],_mm512_mul_ps(_mm512_load_ps(den[a]),r));
            for(int b=0;b<block_count;++b) {
                __m512 shifted=_mm512_maskz_sub_ps(valid_masks[b],
                                                 _mm512_load_ps(scores[a][b]),
                                                 _mm512_load_ps(maximum[a]));
                __m512 weight=_mm512_maskz_mov_ps(valid_masks[b],_mm512_exp_ps(shifted));
                _mm512_store_ps(scores[a][b],weight);
                _mm512_store_ps(den[a],_mm512_add_ps(_mm512_load_ps(den[a]),weight));
            }
            charge<Measure>(count_stats.exp_den,st);
        }
        // One head's running-max update never forces stores of the other head.
        rescale_head<0,Measure,Counters>(cbuf0,obs,batch,changed[0],rescale[0],count_stats);
        rescale_head<1,Measure,Counters>(cbuf1,obs,batch,changed[1],rescale[1],count_stats);

        for(int b=0;b<block_count;++b) {
            const uint64_t step=begin+uint64_t(b);
            st=stamp<Measure>();
            // Reuse already collected CSR indices. At a block boundary the next
            // block has not been generated, so no extra CSR reads are prefetched.
            if(b+1<block_count)for(int n=0;n<batch;++n)
                if(unsigned(valid_masks[b+1])&(1u<<n)) {
                    const char* addr=reinterpret_cast<const char*>(
                        base.xbf.data()+size_t(sources[b+1][n])*p.in);
                    for(int off=0;off<p.in*2;off+=64)_mm_prefetch(addr+off,_MM_HINT_T0);
                }
            for(int n=0;n<batch;++n)if(step==degrees[n]) {
                std::fill(hbuf0+size_t(n)*dp,hbuf0+size_t(n+1)*dp,BF16(0));
                std::fill(hbuf1+size_t(n)*dp,hbuf1+size_t(n+1)*dp,BF16(0));
            }
            charge<Measure>(count_stats.scheduling,st);

            // Coarse profile bracket: one raw H vector load expands once and
            // feeds two independently rounded pH vectors. No attention sharing.
            st=stamp<Measure>();
            for(int n=0;n<batch;++n)if(unsigned(valid_masks[b])&(1u<<n)) {
                const BF16* src=base.xbf.data()+size_t(sources[b][n])*p.in;
                BF16* dst0=hbuf0+size_t(n)*dp;
                BF16* dst1=hbuf1+size_t(n)*dp;
                __m512 pw0=_mm512_set1_ps(scores[0][b][n]);
                __m512 pw1=_mm512_set1_ps(scores[1][b][n]);
                for(int k=0;k<p.in;k+=16) {
                    int len=std::min(16,p.in-k);
                    __m256i raw;
                    if(len==16)raw=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(src+k));
                    else {
                        alignas(32) BF16 tail[16]={};
                        std::memcpy(tail,src+k,size_t(len)*sizeof(BF16));
                        raw=_mm256_load_si256(reinterpret_cast<const __m256i*>(tail));
                    }
                    __m512 v=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(raw),16));
                    __m256bh weighted0=_mm512_cvtneps_pbh(_mm512_mul_ps(v,pw0));
                    __m256bh weighted1=_mm512_cvtneps_pbh(_mm512_mul_ps(v,pw1));
                    _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst0+k),(__m256i)weighted0);
                    _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst1+k),(__m256i)weighted1);
                    if constexpr(Counters) {
                        ++stats.raw_H_vectors;
                        stats.logical_raw_H_bytes+=uint64_t(len)*sizeof(BF16);
                    }
                }
            }
            charge<Measure>(count_stats.source_gather,st);

            // Coarse profile bracket: tile loads and AMX arithmetic together.
            // Each head retains precisely the original neighbor/kb/ob order;
            // interleaving heads cannot change its accumulator rounding order.
            st=stamp<Measure>();
            for(int kb=0;kb<kblocks;++kb) {
                const BF16* wb0=q.packed.data()+((size_t(first_head)*kblocks+kb)*obs)*512;
                const BF16* wb1=q.packed.data()+((size_t(first_head+1)*kblocks+kb)*obs)*512;
                _tile_loadd(4,hbuf0+kb*32,dp*2);
                _tile_loadd(5,wb0,64);
                _tile_dpbf16ps(0,4,5);
                if(obs>1){_tile_loadd(5,wb0+512,64);_tile_dpbf16ps(1,4,5);}
                _tile_loadd(4,hbuf1+kb*32,dp*2);
                _tile_loadd(5,wb1,64);
                _tile_dpbf16ps(2,4,5);
                if(obs>1){_tile_loadd(5,wb1+512,64);_tile_dpbf16ps(3,4,5);}
            }
            charge<Measure>(count_stats.amx_compute,st);
            if constexpr(Counters) {
                count_stats.neighbor_steps+=2;
                count_stats.amx_calls+=uint64_t(2)*kblocks*obs;
                count_stats.executed_fma+=uint64_t(2)*16*dp*obs*16;
            }
        }
    }
    st=stamp<Measure>();store_head<0>(cbuf0,obs);store_head<1>(cbuf1,obs);
    charge<Measure>(count_stats.final_store,st);
    st=stamp<Measure>();
    for(int n=0;n<batch;++n)for(int a=0;a<2;++a) {
        size_t idx=size_t(rows[n])*p.heads+first_head+a;
        base.max[idx]=maximum[a][n];base.den[idx]=den[a][n];
        float* dst=base.out.data()+size_t(rows[n])*p.width()+(first_head+a)*p.dim;
        const float* src=cbuf+size_t(a)*512+n*16;
        for(int ob=0;ob<obs;++ob)
            std::memcpy(dst+ob*16,src+ob*256,size_t(std::min(16,p.dim-ob*16))*4);
    }
    charge<Measure>(count_stats.output_write,st);
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
    dst.raw_H_vectors+=src.raw_H_vectors;
    dst.logical_raw_H_bytes+=src.logical_raw_H_bytes;
    dst.source_index_loads+=src.source_index_loads;
}

void validate_call(const Graph& g,const Param& p,const Prepared& q,
                   const Schedule& sched,const Workspace& w,int block,int panel,
                   int profile_period,bool counters) {
    const auto& o=w.original;
    if(block<1 || block>64)throw std::runtime_error("ICPP pair block must be in [1,64]");
    if(panel<16 || panel%16)throw std::runtime_error("ICPP pair panel must be a multiple of 16");
    if(profile_period<0)throw std::runtime_error("ICPP pair profile period must be nonnegative");
    if(!g.n || p.in<=0 || p.heads<1 || p.heads>8 || p.dim<1 || p.dim>64 ||
       o.allocated_n!=g.n || o.allocated_in!=p.in ||
       o.allocated_heads!=p.heads || o.allocated_dim!=p.dim)
        throw std::runtime_error("ICPP pair workspace or parameter shape mismatch");
    if(o.thread_capacity<omp_get_max_threads())
        throw std::runtime_error("ICPP pair workspace thread capacity exceeded; reallocate");
    size_t nk=checked(g.n,p.heads),nw=checked(g.n,p.width());
    int dp=(p.in+31)/32*32,obs=(p.dim+15)/16;
    if(g.row.size()!=g.n+1 || g.col.size()!=g.e || sched.perm.size()!=g.n ||
       q.padded_in!=dp || q.output_blocks!=obs ||
       q.packed.size()!=size_t(p.heads)*(dp/32)*obs*512 ||
       o.base.xbf.size()!=checked(g.n,p.in) || o.base.left.size()!=nk ||
       o.base.right.size()!=nk || o.base.max.size()!=nk || o.base.den.size()!=nk ||
       o.base.out.size()!=nw || o.base.thread_cbuf.size()<size_t(o.thread_capacity)*16*64 ||
       (paired_shape(p) && w.paired_hbuf.size()<size_t(o.thread_capacity)*2*16*dp))
        throw std::runtime_error("ICPP pair CSR, packed weights, or buffer size mismatch");
    if(counters && (o.row_blocks.size()!=nk || o.row_max_updates.size()!=nk || o.row_rescales.size()!=nk))
        throw std::runtime_error("ICPP pair counter arrays not allocated");
}

void fallback_read_counts(const Graph& g,const Param& p,Stats& stats) {
    // Original single-head gathering visits every valid edge once per head.
    // Logical counts deliberately exclude T0 prefetch and padded feature slots.
    uint64_t edge_heads=uint64_t(checked(g.e,p.heads));
    stats.source_index_loads=edge_heads;
    stats.raw_H_vectors=uint64_t(checked(edge_heads,(p.in+15)/16));
    stats.logical_raw_H_bytes=uint64_t(checked(edge_heads,uint64_t(p.in)*sizeof(BF16)));
}
}

void Workspace::allocate(const Graph& g,const Param& p,bool counters) {
    original.allocate(g,p,counters);
    if(paired_shape(p)) {
        size_t per_thread=checked(32,(p.in+31)/32*32);
        paired_hbuf.resize(checked(original.thread_capacity,per_thread));
    } else paired_hbuf.clear();
}
size_t Workspace::bytes() const {return original.bytes()+paired_hbuf.size()*sizeof(BF16);}

void aggregate(const Graph& g,const Param& p,const Prepared& q,const Schedule& sched,
               Workspace& w,int block,int panel,Stats& stats,int profile_period,bool counters) {
    validate_call(g,p,q,sched,w,block,panel,profile_period,counters);
    stats=Stats{};
    if(!paired_shape(p)) {
        icpp_online::aggregate(g,p,q,sched,w.original,block,panel,stats.baseline,profile_period,counters);
        if(counters)fallback_read_counts(g,p,stats);
        return;
    }
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
            const int dp=q.padded_in;
            BF16* hbuf=w.paired_hbuf.data()+size_t(omp_get_thread_num())*2*16*dp;
            float* cbuf=w.original.base.thread_cbuf.data()+size_t(omp_get_thread_num())*16*64;
            for(int h=0;h<p.heads;h+=2) {
                #pragma omp for schedule(dynamic,1)
                for(uint64_t rg=0;rg<g.n;rg+=uint64_t(panel)) {
                    uint64_t end=std::min(g.n,rg+uint64_t(panel));
                    for(uint64_t first=rg;first<end;first+=16) {
                        int batch=int(std::min<uint64_t>(16,end-first));
                        uint64_t ordinal=uint64_t(h/2)*((g.n+15)/16)+first/16;
                        bool measure=profile_period>0 && ordinal%uint64_t(profile_period)==0;
                        // Counters retain the original head-tile units, while
                        // coarse profile selects one pair as a single sample.
                        if(counters || profile_period>0)local.baseline.total_tiles+=2;
                        if(measure)local.baseline.sampled_tiles+=2;
                        if(measure) {
                            if(counters)run_pair<true,true>(g,p,q,sched,w,first,batch,h,block,hbuf,cbuf,local);
                            else run_pair<true,false>(g,p,q,sched,w,first,batch,h,block,hbuf,cbuf,local);
                        } else {
                            if(counters)run_pair<false,true>(g,p,q,sched,w,first,batch,h,block,hbuf,cbuf,local);
                            else run_pair<false,false>(g,p,q,sched,w,first,batch,h,block,hbuf,cbuf,local);
                        }
                    }
                }
            }
            _tile_release();
        }
        if(counters || profile_period>0) {
            #pragma omp critical(icpp_pair_stats)
            combine(stats,local);
        }
    }
    if(permission_failed.load())throw std::runtime_error("ICPP pair AMX permission request failed on worker");
}

void layer(const Graph& g,const Param& p,const Prepared& q,const Schedule& sched,
           const std::vector<float>& input,Workspace& w,bool hidden,int block,
           int panel,Timing& t,Stats& stats,int profile_period,bool counters,
           const std::string& lr_policy) {
    validate_call(g,p,q,sched,w,block,panel,profile_period,counters);
    if(input.size()!=checked(g.n,p.in))throw std::runtime_error("ICPP pair input shape mismatch");
    const size_t blr_count=checked(p.in,2*p.heads);
    if(w.original.base.lr.size()!=checked(g.n,2*p.heads) ||
       (lr_policy=="fp32" && q.blr.size()!=blr_count) ||
       (lr_policy=="bf16" && q.blr_bf16.size()!=blr_count) ||
       (lr_policy=="avx" && q.blr_dot.size()!=blr_count) ||
       (lr_policy!="fp32" && lr_policy!="bf16" && lr_policy!="avx"))
        throw std::runtime_error("ICPP pair attention policy or prepared buffer mismatch");
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
