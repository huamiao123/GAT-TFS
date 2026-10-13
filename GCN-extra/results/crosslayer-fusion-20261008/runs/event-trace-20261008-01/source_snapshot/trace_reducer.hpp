#pragma once
#include "trace_plan.hpp"

namespace gcn_extra_event_trace {
// MODE0 cached heads / independent expressions; MODE1 cached heads / shared
// decode; MODE2 representative current index / shared decode. No barriers,
// forced loads, floating-point reassociation, or runtime head comparisons.
template<bool P,int MODE>
inline void trace_pair_window(const SourceGraphView&g,const uint16_t*hb,
        const uint32_t*bases,const uint32_t*degrees,int n0,int n1,
        uint32_t ws,uint32_t we,uint32_t scope_end,bool first,
        float*partial,uint16_t*hi,uint16_t*lo,const Method&method,
        const uint8_t*&cursor,Profile&pr,bool sample) {
    using namespace gcn_extra_state;
    static_assert(MODE>=0&&MODE<=2,"unknown trace mode");
    const unsigned count=*cursor++;
    const uint8_t*payload=cursor;
    const unsigned bytes=(count+3)/4;cursor+=bytes;
    if constexpr(P){++pr.trace_logical.headers;pr.trace_logical.payload_bytes+=bytes;}
    const bool second=n1>=0;
    const uint32_t scope0=std::min(degrees[n0],scope_end);
    const uint32_t scope1=second?std::min(degrees[n1],scope_end):0;
    const uint32_t end0=std::min(scope0,we),end1=second?std::min(scope1,we):0;
    const bool visit0=first||ws<end0,visit1=second&&(first||ws<end1);
    // An exhausted pair still owns a count0 header in the immutable stream.
    if(!visit0&&!visit1)return;
    __m512 acc0[8],acc1[8];
    if(visit0)load_row<P>(partial,n0,first,acc0,pr,sample);
    if(visit1)load_row<P>(partial,n1,first,acc1,pr,sample);
    gcn_extra_pair_plan::count_row_window<P>(ws,end0,visit0,pr);
    if(second)gcn_extra_pair_plan::count_row_window<P>(ws,end1,visit1,pr);
    uint32_t k0=ws,k1=ws,head0=0,head1=0;
    if constexpr(MODE!=2) {
        if(k0<end0)head0=pair_read_head<P>(g,bases[n0],k0,pr,sample);
        if(second&&k1<end1)head1=pair_read_head<P>(g,bases[n1],k1,pr,sample);
    }
    // One source-level byte load for each <=4 event block; no load per event.
    for(unsigned block=0;block<count;block+=4) {
        uint8_t codes=payload[block/4];
        const unsigned limit=std::min(4u,count-block);
        for(unsigned z=0;z<limit;++z) {
            const unsigned take=codes&3u;codes>>=2;
            if constexpr(P) {
                ++pr.trace_logical.events;
                pr.trace_logical.take1+=take==1;
                pr.trace_logical.take2+=take==2;
                pr.trace_logical.take3+=take==3;
                pr.logical.pair_shared_sources+=take==3;
            }
            if constexpr(MODE==2) {
                if(take&1)head0=pair_read_head<P>(g,bases[n0],k0,pr,sample);
                else head1=pair_read_head<P>(g,bases[n1],k1,pr,sample);
            }
            if(take&1)pair_prefetch_next<P>(g,hb,bases[n0],k0,scope0,pr,sample);
            if(take&2)pair_prefetch_next<P>(g,hb,bases[n1],k1,scope1,pr,sample);
            if constexpr(MODE==0) {
                if(take&1)pair_add_source<P,false>(hb+size_t(head0)*DIM,acc0,nullptr,pr,sample);
                if(take&2)pair_add_source<P,false>(hb+size_t(head1)*DIM,acc1,nullptr,pr,sample);
            } else {
                if(take==3)pair_add_source<P,true>(hb+size_t(head0)*DIM,acc0,acc1,pr,sample);
                else if(take==1)pair_add_source<P,false>(hb+size_t(head0)*DIM,acc0,nullptr,pr,sample);
                else pair_add_source<P,false>(hb+size_t(head1)*DIM,acc1,nullptr,pr,sample);
            }
            if(take&1) {
                ++k0;
                if constexpr(MODE!=2)if(k0<end0)head0=pair_read_head<P>(g,bases[n0],k0,pr,sample);
            }
            if(take&2) {
                ++k1;
                if constexpr(MODE!=2)if(k1<end1)head1=pair_read_head<P>(g,bases[n1],k1,pr,sample);
            }
        }
    }
    if(visit0)finish_row<P>(acc0,n0,end0==scope0,partial,hi,lo,method,pr,sample);
    if(visit1)finish_row<P>(acc1,n1,end1==scope1,partial,hi,lo,method,pr,sample);
}

template<int MODE>
struct TraceReducer {
    static_assert(MODE>=0&&MODE<=2,"unknown trace mode");
    template<bool P>
    static inline void run(const SourceGraphView&g,const uint16_t*hb,
            const uint32_t*bases,const uint32_t*degrees,int batch,
            uint32_t scope_start,uint32_t scope_end,uint32_t window,
            float*partial,uint16_t*hi,uint16_t*lo,const Method&method,
            const TilePlan&tp,const TracePlan&trace,size_t tile,
            Profile&pr,bool sample) {
        using namespace gcn_extra_state;
        // kernel rejects M!=FULL and S!=64. All-empty tiles bypass this call.
        const uint8_t*cursor=trace.data.data()+trace.tile_offsets[tile];
        if constexpr(P)++pr.trace_logical.tile_offset_reads;
        init_scope<P>(partial,hi,lo,batch,method,pr,sample);
        for(uint32_t ws=scope_start;ws<scope_end;) {
            uint32_t we=ws+std::min(window,scope_end-ws);bool first=ws==scope_start;
            if constexpr(P)++pr.counters.neighbor_windows;
            for(int pair=0;pair<(batch+1)/2;pair++) {
                int n0=tp.order[2*pair],n1=2*pair+1<batch?tp.order[2*pair+1]:-1;
                if constexpr(P)pr.plan_logical.order_index_reads+=1+uint64_t(n1>=0);
                trace_pair_window<P,MODE>(g,hb,bases,degrees,n0,n1,ws,we,
                    scope_end,first,partial,hi,lo,method,cursor,pr,sample);
            }
            ws=we;
        }
        finish_scope<P>(partial,hi,lo,method,pr,sample);
    }
};
} // namespace gcn_extra_event_trace
