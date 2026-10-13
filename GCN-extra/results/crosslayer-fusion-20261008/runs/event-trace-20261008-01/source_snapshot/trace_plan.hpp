#pragma once
#include "planned_reducer.hpp"
#include <array>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

// Static inference graph inspector. No source IDs, values or features are stored.
namespace gcn_extra_event_trace {
using gcn_extra_paper::SourceGraphView;
using gcn_extra_pair_plan::Plan;
using gcn_extra_pair_plan::TilePlan;
using gcn_extra_state::Method;
struct TracePlan {
    std::vector<uint64_t> tile_offsets;
    std::vector<uint8_t> data;
    std::array<uint64_t,129> event_histogram{};
    uint64_t source_edges=0,shared_heads=0,merge_steps=0,comparisons=0;
    uint64_t nonempty_tiles=0,windows=0,headers=0,payload_bytes=0;
    uint64_t data_bytes=0,offset_bytes=0,plan_bytes=0,allocated_bytes=0,fnv64=0;
    uint64_t take1=0,take2=0,take3=0;
    double build_ms=0,validate_ms=0,total_ms=0;
};
struct TraceLogical {
    uint64_t headers=0,payload_bytes=0,events=0;
    uint64_t take1=0,take2=0,take3=0,tile_offset_reads=0;
};
struct Profile : gcn_extra_pair_plan::Profile { TraceLogical trace_logical; };
inline void add_trace_logical(TraceLogical&a,const TraceLogical&b) {
#define TRACE_ADD(F) a.F+=b.F
    TRACE_ADD(headers);TRACE_ADD(payload_bytes);TRACE_ADD(events);
    TRACE_ADD(take1);TRACE_ADD(take2);TRACE_ADD(take3);TRACE_ADD(tile_offset_reads);
#undef TRACE_ADD
}
inline uint64_t trace_hash(const TracePlan&p) {
    uint64_t h=1469598103934665603ULL;
    auto byte=[&](uint8_t x){h=(h^x)*1099511628211ULL;};
    for(uint64_t x:p.tile_offsets)for(unsigned k=0;k<8;k++)byte(uint8_t(x>>(8*k)));
    for(uint8_t x:p.data)byte(x);
    return h;
}
// Validate addressable shape before either inspector reads a row. This does not
// validate the base plan's unrelated exact-mask payload, which trace never uses.
inline bool trace_shape_valid(const SourceGraphView&g,const int*perm,const Plan&base,
                              std::string*reason) {
    auto bad=[&](const char*s){if(reason)*reason=s;return false;};
    if(g.n<0 || !g.row.p || (g.n && !perm) || (g.e && !g.col.p))return bad("null/negative graph shape");
    if(g.row[0]!=0 || uint64_t(g.row[g.n])!=g.e)return bad("CSR endpoints");
    if(base.tiles.size()!=(size_t(g.n)+15)/16)return bad("base tile count");
    std::vector<uint8_t>seen(size_t(g.n),0);
    for(int i=0;i<g.n;i++) {
        if(g.row[i]>g.row[i+1])return bad("nonmonotone row pointers");
        if(perm[i]<0 || perm[i]>=g.n || seen[size_t(perm[i])]++)return bad("invalid permutation");
    }
    for(size_t ti=0;ti<base.tiles.size();ti++) {
        int batch=std::min(16,g.n-int(ti*16));bool slots[16]{};uint32_t mx=0;
        for(int n=0;n<batch;n++) {
            const unsigned slot=base.tiles[ti].order[n];
            if(slot>=unsigned(batch)||slots[slot])return bad("invalid plan row order");
            slots[slot]=true;int r=perm[ti*16+n];mx=std::max(mx,g.row[r+1]-g.row[r]);
        }
        for(int n=batch;n<16;n++)if(base.tiles[ti].order[n]!=255)return bad("invalid plan padded slot");
        if(base.tiles[ti].windows!=(uint64_t(mx)+63)/64)return bad("base window count");
    }
    return true;
}
struct TraceTileStats {
    uint64_t bytes=0,headers=0,payload=0,events=0,take1=0,take2=0,take3=0,comparisons=0;
};
// Builder alone uses the frozen pair_take. The validator below deliberately
// rederives each decision without calling this builder or pair_take.
template<bool FILL>
inline TraceTileStats build_trace_tile(const SourceGraphView&g,const int*perm,
        const Plan&base,size_t ti,uint8_t*dst,std::array<uint64_t,129>*hist=nullptr) {
    TraceTileStats st;const int batch=std::min(16,g.n-int(ti*16));
    uint32_t bases[16],degrees[16],mx=0;
    for(int n=0;n<batch;n++){int row=perm[ti*16+n];bases[n]=g.row[row];degrees[n]=g.row[row+1]-bases[n];mx=std::max(mx,degrees[n]);}
    for(uint32_t ws=0;ws<mx;) {
        const uint32_t we=ws+std::min(64u,mx-ws);
        for(int pair=0;pair<(batch+1)/2;pair++) {
            const int n0=base.tiles[ti].order[2*pair];
            const int n1=2*pair+1<batch?base.tiles[ti].order[2*pair+1]:-1;
            const uint32_t end0=std::min(degrees[n0],we),end1=n1>=0?std::min(degrees[n1],we):0;
            uint32_t k0=ws,k1=ws;unsigned count=0;uint8_t payload[32]{};
            uint32_t h0=k0<end0?g.col[size_t(bases[n0])+k0]:0;
            uint32_t h1=n1>=0&&k1<end1?g.col[size_t(bases[n1])+k1]:0;
            while(k0<end0 || (n1>=0&&k1<end1)) {
                const bool have0=k0<end0,have1=n1>=0&&k1<end1;
                const unsigned take=gcn_extra_state::pair_take(have0,have1,h0,h1);
                st.comparisons+=have0&&have1;
                st.take1+=take==1;st.take2+=take==2;st.take3+=take==3;
                if constexpr(FILL)payload[count/4]|=uint8_t(take<<(2*(count%4)));
                ++count;
                if(take&1){++k0;if(k0<end0)h0=g.col[size_t(bases[n0])+k0];}
                if(take&2){++k1;if(k1<end1)h1=g.col[size_t(bases[n1])+k1];}
            }
            const unsigned bytes=(count+3)/4;
            if(hist)++(*hist)[count];
            if constexpr(FILL){dst[st.bytes]=uint8_t(count);if(bytes)std::memcpy(dst+st.bytes+1,payload,bytes);}
            st.bytes+=1+bytes;++st.headers;st.payload+=bytes;st.events+=count;
        }
        ws=we;
    }
    return st;
}
inline bool trace_validate(const SourceGraphView&g,const int*perm,const Plan&base,
                           const TracePlan&p,std::string*reason=nullptr) {
    auto bad=[&](const char*s){if(reason)*reason=s;return false;};
    if(reason)reason->clear();
    if(!trace_shape_valid(g,perm,base,reason))return false;
    if(p.tile_offsets.size()!=base.tiles.size()+1 || p.tile_offsets.front()!=0 || p.tile_offsets.back()!=p.data.size())return bad("trace offset endpoints");
    for(size_t t=0;t<base.tiles.size();t++)if(p.tile_offsets[t]>p.tile_offsets[t+1]||p.tile_offsets[t+1]>p.data.size())return bad("trace offset bounds/order");
    uint64_t events=0,one=0,two=0,three=0,headers=0,payload=0,comparisons=0,windows=0,nonempty=0,edges=0;
    std::array<uint64_t,129> histogram{};
    for(size_t ti=0;ti<base.tiles.size();ti++) {
        const int batch=std::min(16,g.n-int(ti*16));uint32_t bases[16],degree[16],mx=0;
        for(int n=0;n<batch;n++){int r=perm[ti*16+n];bases[n]=g.row[r];degree[n]=g.row[r+1]-bases[n];mx=std::max(mx,degree[n]);}
        nonempty+=mx>0;uint64_t pos=p.tile_offsets[ti],limit=p.tile_offsets[ti+1];
        for(uint32_t ws=0;ws<mx;) {
            const uint32_t we=ws+std::min(64u,mx-ws);++windows;
            for(int pair=0;pair<(batch+1)/2;pair++) {
                int n0=base.tiles[ti].order[2*pair],n1=2*pair+1<batch?base.tiles[ti].order[2*pair+1]:-1;
                uint32_t a=ws,b=ws,ea=std::min(degree[n0],we),eb=n1>=0?std::min(degree[n1],we):0;
                if(pos>=limit)return bad("missing trace header");
                const unsigned count=p.data[size_t(pos++)],bytes=(count+3)/4;
                if(count>128 || uint64_t(bytes)>limit-pos)return bad("trace count/payload bounds");
                ++headers;payload+=bytes;++histogram[count];
                for(unsigned k=0;k<count;k++) {
                    unsigned code=(p.data[size_t(pos+k/4)]>>(2*(k%4)))&3u;
                    if(code==0)return bad("invalid zero event code");
                    bool have0=a<ea,have1=n1>=0&&b<eb;
                    if(!have0&&!have1)return bad("event after both rows exhausted");
                    uint32_t x=have0?g.col[size_t(bases[n0])+a]:0,y=have1?g.col[size_t(bases[n1])+b]:0;
                    if((have0&&x>=uint32_t(g.n))||(have1&&y>=uint32_t(g.n)))return bad("source ID out of range");
                    // Independent scalar reference: exhaustive three decisions.
                    unsigned expected;
                    if(!have0)expected=2;
                    else if(!have1)expected=1;
                    else if(x==y)expected=3;
                    else expected=x<y?1:2;
                    comparisons+=have0&&have1;
                    if(code!=expected)return bad("event differs from original greedy");
                    if((code&1)&&!have0)return bad("row0 cursor bounds");
                    if((code&2)&&!have1)return bad("row1 cursor bounds");
                    if(code==3 && x!=y)return bad("shared heads unequal");
                    if(code&1){++a;++edges;}if(code&2){++b;++edges;}
                    ++events;one+=code==1;two+=code==2;three+=code==3;
                }
                if(a!=std::max(ws,ea)||b!=std::max(ws,eb))return bad("row cursor not exactly exhausted");
                if(count%4 && (p.data[size_t(pos+bytes-1)]>>(2*(count%4)))!=0)return bad("nonzero trailing payload bits");
                pos+=bytes;
            }
            ws=we;
        }
        if(pos!=limit)return bad("extra tile trace bytes");
    }
    if(histogram!=p.event_histogram)return bad("record histogram mismatch");
    if(edges!=g.e||events!=edges-three||three!=base.dynamic_shared_heads||events!=base.merge_steps||comparisons!=base.comparisons||windows!=base.neighbor_windows||nonempty!=base.full_projections)return bad("trace vs base structural totals");
    if(p.source_edges!=edges||p.shared_heads!=three||p.merge_steps!=events||p.comparisons!=comparisons||p.nonempty_tiles!=nonempty||p.windows!=windows||p.headers!=headers||p.payload_bytes!=payload||p.take1!=one||p.take2!=two||p.take3!=three)return bad("trace scalar stats mismatch");
    if(p.data_bytes!=p.data.size()||p.data_bytes!=headers+payload||p.offset_bytes!=p.tile_offsets.size()*sizeof(uint64_t)||p.plan_bytes!=p.data_bytes+p.offset_bytes)return bad("trace persistent byte stats");
    // Capacity is an allocation diagnostic. A copied plan may have a different
    // capacity, so validator does not require its stored capacity statistic.
    if(trace_hash(p)!=p.fnv64)return bad("trace content hash");
    return true;
}
inline TracePlan build_trace(const SourceGraphView&g,const int*perm,const Plan&base,
                             const char*graph,bool emit_log=true) {
    const double begin=omp_get_wtime();std::string reason;
    if(!trace_shape_valid(g,perm,base,&reason))throw std::runtime_error("Trace inspector shape: "+reason);
    TracePlan p;const size_t nt=base.tiles.size();std::vector<TraceTileStats>stats(nt);
    #pragma omp parallel for schedule(static)
    for(size_t ti=0;ti<nt;ti++)stats[ti]=build_trace_tile<false>(g,perm,base,ti,nullptr);
    p.tile_offsets.resize(nt+1,0);
    for(size_t ti=0;ti<nt;ti++) {
        const auto&s=stats[ti];
        if(s.bytes>uint64_t(std::numeric_limits<size_t>::max())-p.tile_offsets[ti])throw std::length_error("Trace size overflow");
        p.tile_offsets[ti+1]=p.tile_offsets[ti]+s.bytes;
        p.headers+=s.headers;p.payload_bytes+=s.payload;p.merge_steps+=s.events;
        p.take1+=s.take1;p.take2+=s.take2;p.take3+=s.take3;p.comparisons+=s.comparisons;
        p.windows+=base.tiles[ti].windows;p.nonempty_tiles+=base.tiles[ti].windows>0;
    }
    p.data.resize(size_t(p.tile_offsets.back()));
    #pragma omp parallel
    {
        std::array<uint64_t,129> histogram{};
        #pragma omp for schedule(static)
        for(size_t ti=0;ti<nt;ti++) {
            if(p.tile_offsets[ti]!=p.tile_offsets[ti+1])
                build_trace_tile<true>(g,perm,base,ti,p.data.data()+p.tile_offsets[ti],&histogram);
        }
        #pragma omp critical(trace_histogram_merge)
        for(size_t h=0;h<histogram.size();h++)p.event_histogram[h]+=histogram[h];
    }
    p.source_edges=g.e;p.shared_heads=p.take3;p.data_bytes=p.data.size();p.offset_bytes=p.tile_offsets.size()*sizeof(uint64_t);
    p.plan_bytes=p.data_bytes+p.offset_bytes;p.allocated_bytes=p.data.capacity()+p.tile_offsets.capacity()*sizeof(uint64_t);
    p.fnv64=trace_hash(p);p.build_ms=(omp_get_wtime()-begin)*1000;
    double validation_begin=omp_get_wtime();bool ok=trace_validate(g,perm,base,p,&reason);
    p.validate_ms=(omp_get_wtime()-validation_begin)*1000;p.total_ms=(omp_get_wtime()-begin)*1000;
    if(emit_log) {
        printf("TRACE_PLAN_SETUP graph=%s source_edges=%llu shared_heads=%llu merge_steps=%llu comparisons=%llu nonempty_tiles=%llu windows=%llu headers=%llu payload_bytes=%llu data_bytes=%llu offset_bytes=%llu plan_bytes=%llu allocated_bytes=%llu take1=%llu take2=%llu take3=%llu fnv64=%llu build_ms=%.12g validate_ms=%.12g total_ms=%.12g format=count_u8_plus_lsb2 source_ids_stored=0 inspector_includes_full_validator=1 bytes_semantics=persistent_vector_payload object_bytes=%zu\n",graph,(unsigned long long)p.source_edges,(unsigned long long)p.shared_heads,(unsigned long long)p.merge_steps,(unsigned long long)p.comparisons,(unsigned long long)p.nonempty_tiles,(unsigned long long)p.windows,(unsigned long long)p.headers,(unsigned long long)p.payload_bytes,(unsigned long long)p.data_bytes,(unsigned long long)p.offset_bytes,(unsigned long long)p.plan_bytes,(unsigned long long)p.allocated_bytes,(unsigned long long)p.take1,(unsigned long long)p.take2,(unsigned long long)p.take3,(unsigned long long)p.fnv64,p.build_ms,p.validate_ms,p.total_ms,sizeof(TracePlan));
        printf("TRACE_PLAN_GATE graph=%s pass=%d independent_scalar_validator=1 original_greedy=1 source_ids_stored=0\n",graph,int(ok));fflush(stdout);
    }
    if(!ok)throw std::runtime_error("Trace validation: "+reason);
    return p;
}
} // namespace gcn_extra_event_trace
