#pragma once
#include "trace_plan.hpp"

// Read-only structural diagnostic, never executed in a timed forward.
// Full-row greedy is pair-optimal for sorted sequences. For unsorted CSR it is
// only a specific legal schedule and may share LESS than synchronous S64.
namespace gcn_extra_event_trace {
struct OpportunityStats {
    uint64_t rows=0,sorted_rows=0,unsorted_rows=0,pairs=0,odd_rows=0;
    uint64_t sorted_pairs=0,unsorted_pairs=0,gain_pairs=0,loss_pairs=0,tie_pairs=0;
    uint64_t sorted_gain_pairs=0,sorted_loss_pairs=0,sorted_tie_pairs=0;
    uint64_t shared_s64=0,shared_full=0,sorted_shared_s64=0,sorted_shared_full=0;
    uint64_t unsorted_shared_s64=0,unsorted_shared_full=0;
    uint64_t source_reads=0,events=0,comparisons=0,invalid=0;
};
inline void opportunity_add(OpportunityStats& a,const OpportunityStats& b) {
#define OPP_ADD(F) a.F+=b.F
    OPP_ADD(rows);OPP_ADD(sorted_rows);OPP_ADD(unsorted_rows);OPP_ADD(pairs);OPP_ADD(odd_rows);
    OPP_ADD(sorted_pairs);OPP_ADD(unsorted_pairs);OPP_ADD(gain_pairs);OPP_ADD(loss_pairs);OPP_ADD(tie_pairs);
    OPP_ADD(sorted_gain_pairs);OPP_ADD(sorted_loss_pairs);OPP_ADD(sorted_tie_pairs);
    OPP_ADD(shared_s64);OPP_ADD(shared_full);OPP_ADD(sorted_shared_s64);OPP_ADD(sorted_shared_full);
    OPP_ADD(unsorted_shared_s64);OPP_ADD(unsorted_shared_full);OPP_ADD(source_reads);
    OPP_ADD(events);OPP_ADD(comparisons);OPP_ADD(invalid);
#undef OPP_ADD
}
inline void report_opportunity(const SourceGraphView& g,const int* perm,const Plan& base,
                               const TracePlan& trace,const char* graph) {
    const double begin=omp_get_wtime();
    std::vector<OpportunityStats> thread(omp_get_max_threads());
    #pragma omp parallel
    {
        OpportunityStats& st=thread[omp_get_thread_num()];
        #pragma omp for schedule(static)
        for(size_t ti=0;ti<base.tiles.size();ti++) {
            const auto& tile=base.tiles[ti];
            const int batch=std::min(16,g.n-int(ti*16)),np=(batch+1)/2;
            uint64_t window_shared[8]{};
            uint64_t pos=trace.tile_offsets[ti],limit=trace.tile_offsets[ti+1];
            // Count 11 codes directly from the already independently validated
            // encoded stream; this does not rerun the S64 builder/merge.
            for(uint32_t w=0;w<tile.windows;w++)for(int p=0;p<np;p++) {
                if(pos>=limit){++st.invalid;continue;}
                unsigned count=trace.data[size_t(pos++)],bytes=(count+3)/4;
                if(bytes>limit-pos){++st.invalid;pos=limit;continue;}
                for(unsigned b=0;b<bytes;b++) {
                    unsigned x=trace.data[size_t(pos++)];
                    window_shared[p]+=unsigned(__builtin_popcount(x&(x>>1)&0x55u));
                }
            }
            st.invalid+=pos!=limit;
            for(int p=0;p<np;p++) {
                const int n0=tile.order[2*p],n1=2*p+1<batch?tile.order[2*p+1]:-1;
                const int r0=perm[ti*16+n0],r1=n1>=0?perm[ti*16+n1]:-1;
                const uint32_t b0=g.row[r0],b1=r1>=0?g.row[r1]:0;
                const uint32_t d0=g.row[r0+1]-b0,d1=r1>=0?g.row[r1+1]-b1:0;
                uint32_t a=0,b=0,h0=d0?g.col[b0]:0,h1=d1?g.col[b1]:0;
                st.source_reads+=uint64_t(d0>0)+uint64_t(d1>0);
                bool sorted0=true,sorted1=true;
                uint64_t shared=0;
                while(a<d0||b<d1) {
                    const bool have0=a<d0,have1=b<d1;
                    unsigned take;
                    if(!have0)take=2;
                    else if(!have1)take=1;
                    else if(h0==h1)take=3;
                    else take=h0<h1?1:2;
                    st.comparisons+=have0&&have1;
                    ++st.events;shared+=take==3;
                    if(take&1u){++a;if(a<d0){uint32_t x=g.col[size_t(b0)+a];++st.source_reads;sorted0=sorted0&&(x>=h0);h0=x;}}
                    if(take&2u){++b;if(b<d1){uint32_t x=g.col[size_t(b1)+b];++st.source_reads;sorted1=sorted1&&(x>=h1);h1=x;}}
                }
                ++st.rows;st.sorted_rows+=sorted0;st.unsorted_rows+=!sorted0;
                if(r1>=0){++st.rows;st.sorted_rows+=sorted1;st.unsorted_rows+=!sorted1;}
                st.shared_s64+=window_shared[p];st.shared_full+=shared;
                if(r1<0){++st.odd_rows;st.invalid+=shared!=0||window_shared[p]!=0;continue;}
                ++st.pairs;st.gain_pairs+=shared>window_shared[p];
                st.loss_pairs+=shared<window_shared[p];st.tie_pairs+=shared==window_shared[p];
                if(sorted0&&sorted1) {
                    ++st.sorted_pairs;st.sorted_shared_s64+=window_shared[p];st.sorted_shared_full+=shared;
                    st.sorted_gain_pairs+=shared>window_shared[p];st.sorted_loss_pairs+=shared<window_shared[p];st.sorted_tie_pairs+=shared==window_shared[p];
                    st.invalid+=shared<window_shared[p];
                } else {
                    ++st.unsorted_pairs;st.unsorted_shared_s64+=window_shared[p];st.unsorted_shared_full+=shared;
                }
            }
        }
    }
    OpportunityStats st;
    for(const auto& s:thread)opportunity_add(st,s);
    st.invalid+=st.rows!=uint64_t(g.n)||st.source_reads!=g.e||st.events!=g.e-st.shared_full;
    st.invalid+=st.shared_s64!=base.dynamic_shared_heads||st.shared_s64!=trace.shared_heads;
    st.invalid+=st.sorted_pairs+st.unsorted_pairs!=st.pairs;
    st.invalid+=st.pairs*2+st.odd_rows!=st.rows;
    const double ms=(omp_get_wtime()-begin)*1000;
    printf("TRACE_OPPORTUNITY graph=%s diagnostic_only=1 semantics=full_row_greedy source_level=1 diagnostic_ms=%.12g",graph,ms);
#define OPP_PRINT(F) printf(" " #F "=%llu",(unsigned long long)st.F)
    OPP_PRINT(rows);OPP_PRINT(sorted_rows);OPP_PRINT(unsorted_rows);OPP_PRINT(pairs);OPP_PRINT(odd_rows);
    OPP_PRINT(sorted_pairs);OPP_PRINT(unsorted_pairs);OPP_PRINT(gain_pairs);OPP_PRINT(loss_pairs);OPP_PRINT(tie_pairs);
    OPP_PRINT(sorted_gain_pairs);OPP_PRINT(sorted_loss_pairs);OPP_PRINT(sorted_tie_pairs);
    OPP_PRINT(shared_s64);OPP_PRINT(shared_full);OPP_PRINT(sorted_shared_s64);OPP_PRINT(sorted_shared_full);
    OPP_PRINT(unsorted_shared_s64);OPP_PRINT(unsorted_shared_full);OPP_PRINT(source_reads);OPP_PRINT(events);OPP_PRINT(comparisons);OPP_PRINT(invalid);
#undef OPP_PRINT
    printf(" pass=%d\n",int(st.invalid==0));fflush(stdout);
    if(st.invalid)throw std::runtime_error("Full-row opportunity diagnostic mismatch");
}
}
