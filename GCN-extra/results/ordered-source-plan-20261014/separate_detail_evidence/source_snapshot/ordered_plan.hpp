#pragma once
#include "planned_reducer.hpp"
#include <chrono>

namespace gcn_extra_ordered {
using namespace gcn_extra_state;
using gcn_extra_paper::SourceGraphView;
using gcn_extra_paper::DIM;
using gcn_extra_state::Method;
struct Run { uint8_t mask, count; };
static_assert(sizeof(Run)==2);
struct Window { uint64_t offset=0; uint16_t nruns=0; uint8_t n0=0,n1=0,flags=0; };
struct Tile { uint64_t offset=0; uint32_t windows=0; uint8_t flags=0; };
struct Plan {
    std::vector<Tile> tiles;
    std::vector<Window> windows;
    std::vector<Run> runs;
    uint64_t edges=0,all_matches=0,eligible_matches=0,exact_matches=0;
    uint64_t partial_matches=0,eligible_windows=0,exact_windows=0,total_pair_windows=0;
    uint64_t audit_occurrences=0;
    double construct_ms=0,audit_ms=0;
};

inline Plan build_plan(const SourceGraphView& g,const int* perm,const char* graph) {
    const double begin=omp_get_wtime(); Plan p;
    p.tiles.resize((size_t(g.n)+15)/16);
    for(int ti=0;ti<int(p.tiles.size());ti++) {
        uint32_t bases[16]{},deg[16]{},md=0;
        int batch=std::min(16,g.n-ti*16);
        for(int r=0;r<batch;r++){int row=perm[ti*16+r];bases[r]=g.row[row];deg[r]=g.row[row+1]-bases[r];md=std::max(md,deg[r]);}
        uint32_t nw=(uint64_t(md)+63)/64;
        std::vector<Window> local(size_t(nw)*8);
        std::vector<Run> local_runs;
        uint8_t tile_flags=0;
        for(uint32_t wi=0;wi<nw;wi++)for(int pair=0;pair<(batch+1)/2;pair++) {
            int n0=pair*2,n1=n0+1;uint32_t ws=wi*64;
            uint32_t e0=std::min(deg[n0],ws+64),e1=n1<batch?std::min(deg[n1],ws+64):ws;
            uint32_t k0=ws,k1=ws;std::array<Run,128> rr{};uint16_t nr=0;uint32_t matches=0;
            while(k0<e0||k1<e1) {
                bool h0=k0<e0,h1=k1<e1;
                uint32_t j0=h0?g.col[size_t(bases[n0])+k0]:0;
                uint32_t j1=h1?g.col[size_t(bases[n1])+k1]:0;
                unsigned take=gcn_extra_state::pair_take(h0,h1,j0,j1);
                if(nr&&rr[nr-1].mask==take)rr[nr-1].count++;
                else rr[nr++]={uint8_t(take),1};
                if(take&1)k0++;if(take&2)k1++;if(take==3)matches++;
            }
            Window& w=local[size_t(wi)*8+pair];
            w.n0=uint8_t(e0>ws?e0-ws:0);w.n1=uint8_t(e1>ws?e1-ws:0);
            p.edges+=uint64_t(w.n0)+w.n1;p.all_matches+=matches;p.total_pair_windows++;
            const bool exact=w.n0>0&&w.n0==w.n1&&matches==w.n0&&nr==1&&rr[0].mask==3;
            const bool eligible=matches>=8&&nr<=8;
            w.flags=uint8_t(eligible)|(uint8_t(exact)<<1);
            tile_flags|=w.flags;
            if(exact){p.exact_matches+=matches;p.exact_windows++;}
            if(eligible){
                p.eligible_matches+=matches;p.eligible_windows++;
                if(!exact)p.partial_matches+=matches;
                w.offset=local_runs.size();w.nruns=nr;
                local_runs.insert(local_runs.end(),rr.begin(),rr.begin()+nr);
            }
        }
        if(tile_flags) {
            Tile& t=p.tiles[ti];t.offset=p.windows.size();t.windows=nw;t.flags=tile_flags;
            for(auto& w:local)if(w.flags&1)w.offset+=p.runs.size();
            p.windows.insert(p.windows.end(),local.begin(),local.end());
            p.runs.insert(p.runs.end(),local_runs.begin(),local_runs.end());
        }
    }
    p.construct_ms=(omp_get_wtime()-begin)*1000;
    if(p.edges!=g.e)throw std::runtime_error("Planner lost occurrences");
    const double at=omp_get_wtime();
    // Independent expansion: verify actual CSR IDs, occurrence order, and flags.
    for(int ti=0;ti<int(p.tiles.size());ti++) {
        const auto& t=p.tiles[ti];if(!t.flags)continue;
        int batch=std::min(16,g.n-ti*16);
        for(uint32_t wi=0;wi<t.windows;wi++)for(int pair=0;pair<(batch+1)/2;pair++) {
            const auto& w=p.windows[t.offset+size_t(wi)*8+pair];
            int r0=perm[ti*16+pair*2];int r1=pair*2+1<batch?perm[ti*16+pair*2+1]:-1;
            uint32_t b0=g.row[r0],b1=r1>=0?g.row[r1]:0,ws=wi*64,k0=0,k1=0;
            uint32_t d0=g.row[r0+1]-b0,d1=r1>=0?g.row[r1+1]-b1:0;
            if(w.n0!=(d0>ws?std::min(64u,d0-ws):0)||w.n1!=(d1>ws?std::min(64u,d1-ws):0))throw std::runtime_error("Planner lengths invalid");
            if(w.flags&2)for(uint32_t k=0;k<w.n0;k++)if(g.col[size_t(b0)+ws+k]!=g.col[size_t(b1)+ws+k])throw std::runtime_error("False exact flag");
            if(!(w.flags&1))continue;
            for(uint32_t ri=0;ri<w.nruns;ri++) {
                const auto& r=p.runs[w.offset+ri];if(r.mask<1||r.mask>3||!r.count)throw std::runtime_error("Invalid run");
                for(uint32_t k=0;k<r.count;k++) {
                    if((r.mask&1)&&k0>=w.n0)throw std::runtime_error("Run overreads row0");
                    if((r.mask&2)&&k1>=w.n1)throw std::runtime_error("Run overreads row1");
                    if(r.mask==3&&g.col[size_t(b0)+ws+k0]!=g.col[size_t(b1)+ws+k1])throw std::runtime_error("Illegal shared decode");
                    if(r.mask&1){k0++;p.audit_occurrences++;}if(r.mask&2){k1++;p.audit_occurrences++;}
                }
            }
            if(k0!=w.n0||k1!=w.n1)throw std::runtime_error("Run skips occurrence");
        }
    }
    p.audit_ms=(omp_get_wtime()-at)*1000;
    uint64_t bytes=p.tiles.size()*sizeof(Tile)+p.windows.size()*sizeof(Window)+p.runs.size()*sizeof(Run);
    uint64_t allocated=p.tiles.capacity()*sizeof(Tile)+p.windows.capacity()*sizeof(Window)+p.runs.capacity()*sizeof(Run);
    printf("EVENT_PLAN graph=%s E=%llu total_pair_windows=%llu all_matches=%llu eligible_matches=%llu partial_matches=%llu exact_matches=%llu eligible_windows=%llu exact_windows=%llu run_count=%llu plan_bytes=%llu allocated_bytes=%llu construct_ms=%.12g audit_ms=%.12g min_matches=8 max_runs=8 pass=1\n",graph,
      (unsigned long long)p.edges,(unsigned long long)p.total_pair_windows,(unsigned long long)p.all_matches,
      (unsigned long long)p.eligible_matches,(unsigned long long)p.partial_matches,(unsigned long long)p.exact_matches,
      (unsigned long long)p.eligible_windows,(unsigned long long)p.exact_windows,(unsigned long long)p.runs.size(),
      (unsigned long long)bytes,(unsigned long long)allocated,p.construct_ms,p.audit_ms);fflush(stdout);
    return p;
}

struct EventLogical { uint64_t tile_fallback_scopes=0,window_entries=0,selected_windows=0,run_dispatches=0,run_bytes=0; };
struct Profile:gcn_extra_pair_plan::Profile { EventLogical event; };
inline void add_event(EventLogical& a,const EventLogical& b){
    a.tile_fallback_scopes+=b.tile_fallback_scopes;a.window_entries+=b.window_entries;
    a.selected_windows+=b.selected_windows;a.run_dispatches+=b.run_dispatches;a.run_bytes+=b.run_bytes;
}

template<int MODE> struct Reducer {
    inline static thread_local const Plan* plan=nullptr;
    inline static thread_local int ti=0;
    static void bind(const Plan& p){plan=&p;}
    static void set_tile(int i){ti=i;}
    template<bool P> static void run(const SourceGraphView& g,const uint16_t* hb,
        const uint32_t* bases,const uint32_t* degrees,int batch,uint32_t st,uint32_t en,
        uint32_t window,float* partial,uint16_t* hi,uint16_t* lo,
        const Method& method,Profile& pr,bool sample) {
        using namespace gcn_extra_state;
        const Tile& tile=plan->tiles[ti];
        const bool active=MODE==5?(tile.flags&2):(tile.flags&1);
        if(!active) {
            if constexpr(P)pr.event.tile_fallback_scopes++;
            ScalarReducer::run<P>(g,hb,bases,degrees,batch,st,en,window,partial,hi,lo,method,pr,sample);
            return;
        }
        init_scope<P>(partial,hi,lo,batch,method,pr,sample);
        for(uint32_t ws=st;ws<en;) {
            uint32_t we=ws+std::min(window,en-ws);bool first=ws==st;
            if constexpr(P)pr.counters.neighbor_windows++;
            for(int pair=0;pair<(batch+1)/2;pair++) {
                int n0=pair*2,n1=n0+1<batch?n0+1:-1;
                const Window& w=plan->windows[tile.offset+size_t(ws/64)*8+pair];
                if constexpr(P)pr.event.window_entries++;
                bool selected=n1>=0&&(MODE==5?(w.flags&2):(w.flags&1));
                if(!selected) {
                    gcn_extra_pair_plan::scalar_row_window<P>(g,hb,bases,degrees,n0,ws,we,en,first,partial,hi,lo,method,pr,sample);
                    if(n1>=0)gcn_extra_pair_plan::scalar_row_window<P>(g,hb,bases,degrees,n1,ws,we,en,first,partial,hi,lo,method,pr,sample);
                    continue;
                }
                if constexpr(P)pr.event.selected_windows++;
                if constexpr(MODE==4) {
                    gcn_extra_pair_plan::dynamic_pair_window<P,true>(g,hb,bases,degrees,n0,n1,ws,we,en,first,partial,hi,lo,method,pr,sample);
                } else if constexpr(MODE==5) {
                    gcn_extra_pair_plan::exact_pair_window<P,true>(g,hb,bases,degrees,n0,n1,ws,we,en,first,partial,hi,lo,method,pr,sample);
                } else {
                    uint32_t s0=std::min(degrees[n0],en),s1=std::min(degrees[n1],en);
                    uint32_t e0=std::min(s0,we),e1=std::min(s1,we),k0=ws,k1=ws;
                    __m512 a0[8],a1[8];load_row<P>(partial,n0,first,a0,pr,sample);load_row<P>(partial,n1,first,a1,pr,sample);
                    gcn_extra_pair_plan::count_row_window<P>(ws,e0,true,pr);
                    gcn_extra_pair_plan::count_row_window<P>(ws,e1,true,pr);
                    if constexpr(P){pr.event.run_dispatches+=w.nruns;pr.event.run_bytes+=w.nruns*sizeof(Run);}
                    for(uint32_t ri=0;ri<w.nruns;ri++) {
                        const Run& r=plan->runs[w.offset+ri];
                        if(r.mask==1)for(uint32_t k=0;k<r.count;k++,k0++) {
                            uint32_t j=pair_read_head<P>(g,bases[n0],k0,pr,sample);
                            pair_prefetch_next<P>(g,hb,bases[n0],k0,s0,pr,sample);
                            pair_add_source<P,false>(hb+size_t(j)*DIM,a0,nullptr,pr,sample);
                        } else if(r.mask==2)for(uint32_t k=0;k<r.count;k++,k1++) {
                            uint32_t j=pair_read_head<P>(g,bases[n1],k1,pr,sample);
                            pair_prefetch_next<P>(g,hb,bases[n1],k1,s1,pr,sample);
                            pair_add_source<P,false>(hb+size_t(j)*DIM,a1,nullptr,pr,sample);
                        } else {
                            if constexpr(P)pr.logical.pair_shared_sources+=r.count;
                            for(uint32_t k=0;k<r.count;k++,k0++,k1++) {
                                uint32_t j0=pair_read_head<P>(g,bases[n0],k0,pr,sample);
                                pair_prefetch_next<P>(g,hb,bases[n0],k0,s0,pr,sample);
                                pair_prefetch_next<P>(g,hb,bases[n1],k1,s1,pr,sample);
                                if constexpr(MODE==3)pair_add_source<P,true>(hb+size_t(j0)*DIM,a0,a1,pr,sample);
                                else {
                                    uint32_t j1=pair_read_head<P>(g,bases[n1],k1,pr,sample);
                                    pair_add_source<P,false>(hb+size_t(j0)*DIM,a0,nullptr,pr,sample);
                                    pair_add_source<P,false>(hb+size_t(j1)*DIM,a1,nullptr,pr,sample);
                                }
                            }
                        }
                    }
                    finish_row<P>(a0,n0,e0==s0,partial,hi,lo,method,pr,sample);
                    finish_row<P>(a1,n1,e1==s1,partial,hi,lo,method,pr,sample);
                }
            }
            ws=we;
        }
        finish_scope<P>(partial,hi,lo,method,pr,sample);
    }
};
} // namespace gcn_extra_ordered
