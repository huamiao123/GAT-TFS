#pragma once
#include "state_kernels.hpp"
#include "pair_reducer.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

// Immutable, tile-local plans. CSR order, tile membership and AMX row slots
// never change. All counters below count logical source requests, not DRAM.
namespace gcn_extra_pair_plan {
using gcn_extra_paper::SourceGraphView;
struct TilePlan { uint8_t order[16]; uint32_t windows=0; uint64_t mask_offset=0; };
struct Plan {
    std::string name;
    std::vector<TilePlan> tiles;
    std::vector<uint8_t> exact_masks;
    int kind=0;
    uint64_t source_edges=0,dynamic_shared_heads=0,exact_shared_edges=0;
    uint64_t merge_steps=0,comparisons=0,exact_windows=0,nonempty_pair_windows=0;
    uint64_t full_projections=0,b64_projections=0,b256_projections=0,neighbor_windows=0;
    // Persistent vector payload, including TilePlan padding; object/name overhead
    // is reported separately. Temporary construction buffers are not retained.
    uint64_t plan_bytes=0,allocated_bytes=0,sample_edge_probes=0;
    uint64_t full_verification_reads=0,audit_reads=0,fnv64=0;
    uint64_t signature_probes=0,score_probes=0,score_pairs=0;
    uint64_t merge_index_reads=0,exact_compare_reads=0,memcmp_requested_reads=0;
    uint64_t changed_tiles=0,zero_score_tiles=0,score_fallback_tiles=0;
    uint64_t adjacent_sample_score=0,selected_sample_score=0;
    double layout_ms=0,pairing_ms=0,verify_ms=0,audit_ms=0,total_ms=0;
};
inline uint64_t plan_mix(uint64_t x) {
    x+=0x9e3779b97f4a7c15ULL; x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;
    x=(x^(x>>27))*0x94d049bb133111ebULL; return x^(x>>31);
}
inline uint32_t plan_windows(uint32_t d) { return uint32_t((uint64_t(d)+63)/64); }
struct RowSample {
    std::array<uint64_t,4> signature{{UINT64_MAX,UINT64_MAX,UINT64_MAX,UINT64_MAX}};
    std::array<uint32_t,3> windows{{0,0,0}};
    uint32_t count=0;
};
struct BuildStats {
    uint64_t signature_probes=0,score_probes=0,score_pairs=0;
    uint64_t changed_tiles=0,zero_score_tiles=0,score_fallback_tiles=0;
    uint64_t adjacent_sample_score=0,selected_sample_score=0;
    uint64_t source_edges=0,dynamic_shared_heads=0,exact_shared_edges=0;
    uint64_t merge_steps=0,comparisons=0,exact_windows=0,nonempty_pair_windows=0;
    uint64_t merge_index_reads=0,exact_compare_reads=0,memcmp_requested_reads=0;
    uint64_t audit_reads=0,audit_edges=0,audit_full=0,audit_b64=0,audit_b256=0;
    uint64_t invalid=0;
};
inline void add_stats(BuildStats& a,const BuildStats& b) {
#define PAIRPLAN_ADD(F) a.F+=b.F
    PAIRPLAN_ADD(signature_probes);PAIRPLAN_ADD(score_probes);PAIRPLAN_ADD(score_pairs);
    PAIRPLAN_ADD(changed_tiles);PAIRPLAN_ADD(zero_score_tiles);PAIRPLAN_ADD(score_fallback_tiles);
    PAIRPLAN_ADD(adjacent_sample_score);PAIRPLAN_ADD(selected_sample_score);
    PAIRPLAN_ADD(source_edges);PAIRPLAN_ADD(dynamic_shared_heads);PAIRPLAN_ADD(exact_shared_edges);
    PAIRPLAN_ADD(merge_steps);PAIRPLAN_ADD(comparisons);PAIRPLAN_ADD(exact_windows);
    PAIRPLAN_ADD(nonempty_pair_windows);PAIRPLAN_ADD(merge_index_reads);
    PAIRPLAN_ADD(exact_compare_reads);PAIRPLAN_ADD(memcmp_requested_reads);
    PAIRPLAN_ADD(audit_reads);PAIRPLAN_ADD(audit_edges);PAIRPLAN_ADD(audit_full);
    PAIRPLAN_ADD(audit_b64);PAIRPLAN_ADD(audit_b256);PAIRPLAN_ADD(invalid);
#undef PAIRPLAN_ADD
}
inline void adjacent_order(TilePlan& tile,int batch) {
    for(int r=0;r<16;r++)tile.order[r]=r<batch?uint8_t(r):uint8_t(255);
}
inline uint64_t merge_hits(const SourceGraphView& g,uint32_t base0,uint32_t degree0,
        uint32_t base1,uint32_t degree1,uint32_t wi,uint64_t& reads,
        uint64_t* steps=nullptr,uint64_t* comparisons=nullptr) {
    const uint64_t ws=uint64_t(wi)*64;
    const uint64_t end0=std::min<uint64_t>(degree0,ws+64),end1=std::min<uint64_t>(degree1,ws+64);
    uint64_t k0=ws,k1=ws,hits=0;
    uint32_t h0=k0<end0?(++reads,g.col[size_t(base0)+size_t(k0)]):0;
    uint32_t h1=k1<end1?(++reads,g.col[size_t(base1)+size_t(k1)]):0;
    while(k0<end0||k1<end1) {
        const bool have0=k0<end0,have1=k1<end1;
        const unsigned take=gcn_extra_state::pair_take(have0,have1,h0,h1);
        if(steps)++*steps;
        if(comparisons&&have0&&have1)++*comparisons;
        hits+=take==3;
        if(take&1u){++k0;if(k0<end0){h0=g.col[size_t(base0)+size_t(k0)];++reads;}}
        if(take&2u){++k1;if(k1<end1){h1=g.col[size_t(base1)+size_t(k1)];++reads;}}
    }
    return hits;
}
inline RowSample make_sample(const SourceGraphView& g,uint32_t base,uint32_t degree,BuildStats& st) {
    RowSample sample;
    const uint32_t nw=plan_windows(degree);
    if(!nw)return sample;
    const uint32_t windows[3]={0,nw/2,nw-1};
    for(uint32_t wi:windows) {
        if(sample.count&&sample.windows[sample.count-1]==wi)continue;
        sample.windows[sample.count++]=wi;
        for(uint64_t k=uint64_t(wi)*64;k<std::min<uint64_t>(degree,uint64_t(wi+1)*64);k++) {
            const uint32_t j=g.col[size_t(base)+size_t(k)];++st.signature_probes;
            for(unsigned h=0;h<4;h++)sample.signature[h]=std::min(sample.signature[h],
                plan_mix(uint64_t(j)^plan_mix(20261007ULL+uint64_t(h))));
        }
    }
    return sample;
}
inline uint32_t sample_score(const SourceGraphView& g,uint32_t b0,uint32_t d0,
        uint32_t b1,uint32_t d1,const RowSample& a,const RowSample& b,BuildStats& st) {
    uint32_t windows[6],count=0;
    for(uint32_t k=0;k<a.count;k++)windows[count++]=a.windows[k];
    for(uint32_t k=0;k<b.count;k++)windows[count++]=b.windows[k];
    std::sort(windows,windows+count);
    uint32_t previous=UINT32_MAX,score=0;
    for(uint32_t k=0;k<count;k++) {
        if(windows[k]==previous)continue;
        previous=windows[k];
        score+=uint32_t(merge_hits(g,b0,d0,b1,d1,windows[k],st.score_probes));
    }
    ++st.score_pairs;
    return score;
}
inline void choose_order(const SourceGraphView& g,const uint32_t* bases,const uint32_t* degrees,
        int batch,uint64_t tile_index,int kind,TilePlan& tile,BuildStats& st) {
    adjacent_order(tile,batch);
    if(kind==0)return;
    if(kind==2) {
        uint64_t state=plan_mix(20261007ULL^tile_index);
        for(int n=batch-1;n>0;n--){state=plan_mix(state);std::swap(tile.order[n],tile.order[state%uint64_t(n+1)]);}
    } else {
        std::array<RowSample,16> samples;
        for(int r=0;r<batch;r++)samples[r]=make_sample(g,bases[r],degrees[r],st);
        bool candidate[16][16]={};
        uint32_t score[16][16]={};
        for(int r=0;r+1<batch;r+=2)candidate[r][r+1]=true;
        for(int r=0;r<batch;r++) {
            std::array<std::pair<int,int>,15> ranked;
            int count=0;
            for(int q=0;q<batch;q++)if(q!=r) {
                int equal=0;
                if(samples[r].count&&samples[q].count)
                    for(int h=0;h<4;h++)equal+=samples[r].signature[h]==samples[q].signature[h];
                // Zero signature agreement is not evidence for a partner.
                if(equal)ranked[count++]=std::make_pair(-equal,q);
            }
            std::sort(ranked.begin(),ranked.begin()+count);
            for(int k=0;k<std::min(3,count);k++) {
                int q=ranked[k].second;candidate[std::min(r,q)][std::max(r,q)]=true;
            }
        }
        struct Edge { uint32_t score;uint8_t a,b; };
        std::array<Edge,64> edges;int edge_count=0;
        uint64_t adjacent_score=0,selected_score=0;
        for(int r=0;r<batch;r++)for(int q=r+1;q<batch;q++)if(candidate[r][q]) {
            const uint32_t value=sample_score(g,bases[r],degrees[r],bases[q],degrees[q],samples[r],samples[q],st);
            score[r][q]=value;
            edges[edge_count++]={value,uint8_t(r),uint8_t(q)};
            if((r%2)==0&&q==r+1)adjacent_score+=value;
        }
        std::sort(edges.begin(),edges.begin()+edge_count,[](const Edge& a,const Edge& b){
            if(a.score!=b.score)return a.score>b.score;
            if(a.a!=b.a)return a.a<b.a;
            return a.b<b.b;
        });
        bool used[16]={};int next=0;
        for(int k=0;k<edge_count;k++) {
            const auto& e=edges[k];
            if(!e.score||used[e.a]||used[e.b])continue;
            used[e.a]=used[e.b]=true;tile.order[next++]=e.a;tile.order[next++]=e.b;
        }
        for(int r=0;r<batch;r++)if(!used[r])tile.order[next++]=uint8_t(r);
        // The leftover original-order pairing can itself be a new pair.
        // Score it only if needed; total scored candidates stays <= 64 per tile.
        for(int p=0;p+1<batch;p+=2) {
            int r=tile.order[p],q=tile.order[p+1];if(r>q)std::swap(r,q);
            if(!candidate[r][q]) {
                candidate[r][q]=true;
                score[r][q]=sample_score(g,bases[r],degrees[r],bases[q],degrees[q],samples[r],samples[q],st);
            }
            selected_score+=score[r][q];
        }
        st.adjacent_sample_score+=adjacent_score;
        if(edge_count==0||edges[0].score==0){adjacent_order(tile,batch);++st.zero_score_tiles;selected_score=adjacent_score;}
        else if(selected_score<adjacent_score){adjacent_order(tile,batch);++st.score_fallback_tiles;selected_score=adjacent_score;}
        st.selected_sample_score+=selected_score;
    }
    bool changed=false;for(int r=0;r<batch;r++)changed|=tile.order[r]!=r;
    st.changed_tiles+=changed;
}
inline Plan prepare_plan(const SourceGraphView& g,const int* perm,const std::string& name,
        int kind,const char* graph) {
    if(kind<0||kind>2||g.n<0||(!perm&&g.n))throw std::invalid_argument("invalid pair plan inputs");
    const double begin=omp_get_wtime();double phase=begin;
    Plan plan;plan.name=name;plan.kind=kind;
    const size_t nt=(size_t(g.n)+15)/16;
    plan.tiles.resize(nt);
    std::vector<uint8_t> seen(size_t(g.n),0);
    uint64_t reference_edges=0;
    for(int pos=0;pos<g.n;pos++) {
        const int row=perm[pos];
        if(row<0||row>=g.n||seen[size_t(row)]++)throw std::runtime_error("pair plan input perm is not a bijection");
        if(g.row[size_t(row)+1]<g.row[size_t(row)])throw std::runtime_error("pair plan invalid CSR row extent");
        reference_edges+=uint64_t(g.row[size_t(row)+1])-g.row[size_t(row)];
    }
    if(reference_edges!=g.e)throw std::runtime_error("pair plan graph E mismatch");
    #pragma omp parallel for schedule(static)
    for(size_t t=0;t<nt;t++) {
        const size_t first=t*16;const int batch=std::min<size_t>(16,size_t(g.n)-first);
        uint32_t md=0;
        for(int r=0;r<batch;r++){const int row=perm[first+size_t(r)];md=std::max(md,g.row[size_t(row)+1]-g.row[size_t(row)]);}
        adjacent_order(plan.tiles[t],batch);plan.tiles[t].windows=plan_windows(md);
    }
    for(size_t t=0;t<nt;t++) {
        auto& tile=plan.tiles[t];tile.mask_offset=plan.neighbor_windows;
        plan.neighbor_windows+=tile.windows;
        const size_t first=t*16;const int batch=std::min<size_t>(16,size_t(g.n)-first);
        uint32_t md=0;for(int r=0;r<batch;r++){int row=perm[first+size_t(r)];md=std::max(md,g.row[size_t(row)+1]-g.row[size_t(row)]);}
        plan.full_projections+=md>0;plan.b64_projections+=tile.windows;
        plan.b256_projections+=(uint64_t(md)+255)/256;
    }
    if(plan.neighbor_windows>std::numeric_limits<size_t>::max())throw std::overflow_error("pair plan mask allocation overflow");
    plan.exact_masks.assign(size_t(plan.neighbor_windows),0);
    plan.plan_bytes=uint64_t(plan.tiles.size())*sizeof(TilePlan)+plan.exact_masks.size();
    plan.allocated_bytes=uint64_t(plan.tiles.capacity())*sizeof(TilePlan)+plan.exact_masks.capacity();
    plan.layout_ms=(omp_get_wtime()-phase)*1000;phase=omp_get_wtime();
    BuildStats stats;
    #pragma omp parallel
    {
        BuildStats local;
        #pragma omp for schedule(static)
        for(size_t t=0;t<nt;t++) {
            const size_t first=t*16;const int batch=std::min<size_t>(16,size_t(g.n)-first);
            uint32_t bases[16]={},degrees[16]={};
            for(int r=0;r<batch;r++){int row=perm[first+size_t(r)];bases[r]=g.row[size_t(row)];degrees[r]=g.row[size_t(row)+1]-bases[r];}
            choose_order(g,bases,degrees,batch,t,kind,plan.tiles[t],local);
        }
        #pragma omp critical(pair_plan_stats)
        add_stats(stats,local);
    }
    plan.pairing_ms=(omp_get_wtime()-phase)*1000;phase=omp_get_wtime();
    #pragma omp parallel
    {
        BuildStats local;
        #pragma omp for schedule(static)
        for(size_t t=0;t<nt;t++) {
            const auto& tile=plan.tiles[t];const size_t first=t*16;
            const int batch=std::min<size_t>(16,size_t(g.n)-first);
            uint32_t bases[16]={},degrees[16]={};
            for(int r=0;r<batch;r++){int row=perm[first+size_t(r)];bases[r]=g.row[size_t(row)];degrees[r]=g.row[size_t(row)+1]-bases[r];}
            for(uint32_t wi=0;wi<tile.windows;wi++) {
                const uint64_t ws=uint64_t(wi)*64;uint8_t mask=0;
                for(int p=0;p<batch;p+=2) {
                    const int a=tile.order[p];const bool second=p+1<batch;
                    const int b=second?tile.order[p+1]:0;
                    const uint32_t d0=degrees[a],d1=second?degrees[b]:0;
                    const uint32_t l0=d0>ws?uint32_t(std::min<uint64_t>(64,uint64_t(d0)-ws)):0;
                    const uint32_t l1=d1>ws?uint32_t(std::min<uint64_t>(64,uint64_t(d1)-ws)):0;
                    local.source_edges+=uint64_t(l0)+l1;local.nonempty_pair_windows+=l0>0||l1>0;
                    local.dynamic_shared_heads+=merge_hits(g,bases[a],d0,second?bases[b]:0,d1,wi,
                        local.merge_index_reads,&local.merge_steps,&local.comparisons);
                    if(l0&&l0==l1) {
                        bool equal=true;
                        // Count explicit comparison loads; do not treat hash equality as proof.
                        for(uint32_t k=0;k<l0;k++) {
                            const uint32_t x=g.col[size_t(bases[a])+size_t(ws)+k];
                            const uint32_t y=g.col[size_t(bases[b])+size_t(ws)+k];
                            local.exact_compare_reads+=2;if(x!=y){equal=false;break;}
                        }
                        if(equal) {
                            // A memcmp is the final construction guard. Requested extent
                            // is reported separately, not claimed to be retired loads.
                            local.memcmp_requested_reads+=uint64_t(l0)*2;
                            if(std::memcmp(g.col.p+size_t(bases[a])+size_t(ws),
                                           g.col.p+size_t(bases[b])+size_t(ws),size_t(l0)*sizeof(uint32_t))==0) {
                                mask|=uint8_t(1u<<(p/2));++local.exact_windows;local.exact_shared_edges+=l0;
                            }
                        }
                    }
                }
                plan.exact_masks[size_t(tile.mask_offset)+wi]=mask;
            }
        }
        #pragma omp critical(pair_plan_stats)
        add_stats(stats,local);
    }
    plan.verify_ms=(omp_get_wtime()-phase)*1000;phase=omp_get_wtime();
    // Independent full plan audit: validate slot bijections and reconstruct every
    // exact flag directly, without signatures, merge hits or construction helper.
    #pragma omp parallel
    {
        BuildStats local;
        #pragma omp for schedule(static)
        for(size_t t=0;t<nt;t++) {
            const auto& tile=plan.tiles[t];const size_t first=t*16;
            const int batch=std::min<size_t>(16,size_t(g.n)-first);
            uint32_t slots=0,md=0;
            for(int r=0;r<16;r++) {
                const unsigned slot=tile.order[r];
                if(r>=batch){local.invalid+=slot!=255;continue;}
                if(slot>=unsigned(batch)){++local.invalid;continue;}
                local.invalid+=(slots&(1u<<slot))!=0;slots|=1u<<slot;
                const int row=perm[first+slot];const uint32_t d=g.row[size_t(row)+1]-g.row[size_t(row)];
                local.audit_edges+=d;md=std::max(md,d);
            }
            local.invalid+=slots!=((1u<<batch)-1);
            local.invalid+=tile.windows!=plan_windows(md);
            local.audit_full+=md>0;local.audit_b64+=plan_windows(md);local.audit_b256+=(uint64_t(md)+255)/256;
            if(slots!=((1u<<batch)-1))continue;
            for(uint32_t wi=0;wi<tile.windows;wi++) {
                const uint64_t ws=uint64_t(wi)*64;uint8_t expected=0;
                for(int p=0;p+1<batch;p+=2) {
                    const int row0=perm[first+tile.order[p]],row1=perm[first+tile.order[p+1]];
                    const uint32_t base0=g.row[size_t(row0)],base1=g.row[size_t(row1)];
                    const uint32_t d0=g.row[size_t(row0)+1]-base0,d1=g.row[size_t(row1)+1]-base1;
                    const uint32_t l0=d0>ws?uint32_t(std::min<uint64_t>(64,uint64_t(d0)-ws)):0;
                    const uint32_t l1=d1>ws?uint32_t(std::min<uint64_t>(64,uint64_t(d1)-ws)):0;
                    bool equal=l0>0&&l0==l1;
                    if(equal)for(uint32_t k=0;k<l0;k++) {
                        const uint32_t x=g.col[size_t(base0)+size_t(ws)+k];
                        const uint32_t y=g.col[size_t(base1)+size_t(ws)+k];
                        local.audit_reads+=2;if(x!=y){equal=false;break;}
                    }
                    if(equal)expected|=uint8_t(1u<<(p/2));
                }
                local.invalid+=expected!=plan.exact_masks[size_t(tile.mask_offset)+wi];
            }
        }
        #pragma omp critical(pair_plan_stats)
        add_stats(stats,local);
    }
#define PAIRPLAN_COPY(F) plan.F=stats.F
    PAIRPLAN_COPY(signature_probes);PAIRPLAN_COPY(score_probes);PAIRPLAN_COPY(score_pairs);
    PAIRPLAN_COPY(changed_tiles);PAIRPLAN_COPY(zero_score_tiles);PAIRPLAN_COPY(score_fallback_tiles);
    PAIRPLAN_COPY(adjacent_sample_score);PAIRPLAN_COPY(selected_sample_score);
    PAIRPLAN_COPY(source_edges);PAIRPLAN_COPY(dynamic_shared_heads);PAIRPLAN_COPY(exact_shared_edges);
    PAIRPLAN_COPY(merge_steps);PAIRPLAN_COPY(comparisons);PAIRPLAN_COPY(exact_windows);
    PAIRPLAN_COPY(nonempty_pair_windows);PAIRPLAN_COPY(merge_index_reads);
    PAIRPLAN_COPY(exact_compare_reads);PAIRPLAN_COPY(memcmp_requested_reads);PAIRPLAN_COPY(audit_reads);
#undef PAIRPLAN_COPY
    plan.sample_edge_probes=plan.signature_probes+plan.score_probes;
    plan.full_verification_reads=plan.merge_index_reads+plan.exact_compare_reads+plan.memcmp_requested_reads;
    const bool pass=!stats.invalid&&plan.source_edges==g.e&&stats.audit_edges==g.e&&
        plan.merge_index_reads==g.e&&plan.merge_steps+plan.dynamic_shared_heads==g.e&&
        plan.exact_shared_edges<=plan.dynamic_shared_heads&&
        stats.audit_full==plan.full_projections&&stats.audit_b64==plan.b64_projections&&
        stats.audit_b256==plan.b256_projections&&
        plan.signature_probes<=uint64_t(g.n)*192&&plan.score_pairs<=uint64_t(nt)*64&&
        (kind!=1||plan.selected_sample_score>=plan.adjacent_sample_score);
    uint64_t hash=1469598103934665603ULL;
    auto hash_byte=[&](uint8_t x){hash=(hash^x)*1099511628211ULL;};
    for(const auto& tile:plan.tiles) {
        for(uint8_t v:tile.order)hash_byte(v);
        for(unsigned k=0;k<4;k++)hash_byte(uint8_t(tile.windows>>(8*k)));
        for(unsigned k=0;k<8;k++)hash_byte(uint8_t(tile.mask_offset>>(8*k)));
    }
    for(uint8_t v:plan.exact_masks)hash_byte(v);plan.fnv64=hash;
    plan.audit_ms=(omp_get_wtime()-phase)*1000;plan.total_ms=(omp_get_wtime()-begin)*1000;
    printf("PAIRPLAN_SETUP graph=%s plan=%s kind=%d layout_ms=%.12g pairing_ms=%.12g verify_ms=%.12g audit_ms=%.12g total_ms=%.12g plan_bytes=%llu allocated_bytes=%llu sample_edge_probes=%llu signature_probes=%llu score_probes=%llu score_pairs=%llu full_verification_reads=%llu merge_index_reads=%llu exact_compare_reads=%llu memcmp_requested_reads=%llu audit_reads=%llu construction_includes_full_audit=1 uses_timing=0 seed=20261007 counter_semantics=logical_requests_not_retired_loads bytes_semantics=persistent_vector_payload_including_padding object_bytes=%zu name_capacity_bytes=%zu sampling_assignment_combined=1 score_pairs_per_tile_cap=64 signature_probes_per_row_cap=192\n",
        graph,name.c_str(),kind,plan.layout_ms,plan.pairing_ms,plan.verify_ms,plan.audit_ms,plan.total_ms,
        (unsigned long long)plan.plan_bytes,(unsigned long long)plan.allocated_bytes,
        (unsigned long long)plan.sample_edge_probes,(unsigned long long)plan.signature_probes,
        (unsigned long long)plan.score_probes,(unsigned long long)plan.score_pairs,
        (unsigned long long)plan.full_verification_reads,(unsigned long long)plan.merge_index_reads,
        (unsigned long long)plan.exact_compare_reads,(unsigned long long)plan.memcmp_requested_reads,
        (unsigned long long)plan.audit_reads,sizeof(Plan),plan.name.capacity()+1);
    printf("PAIRPLAN_STRUCTURE graph=%s plan=%s kind=%d tiles=%zu source_edges=%llu dynamic_shared_heads=%llu exact_shared_edges=%llu merge_steps=%llu comparisons=%llu exact_windows=%llu nonempty_pair_windows=%llu neighbor_windows=%llu full_projections=%llu b64_projections=%llu b256_projections=%llu changed_tiles=%llu zero_score_tiles=%llu score_fallback_tiles=%llu adjacent_sample_score=%llu selected_sample_score=%llu fnv64=%llu exact_masks_bytes=%zu\n",
        graph,name.c_str(),kind,plan.tiles.size(),(unsigned long long)plan.source_edges,
        (unsigned long long)plan.dynamic_shared_heads,(unsigned long long)plan.exact_shared_edges,
        (unsigned long long)plan.merge_steps,(unsigned long long)plan.comparisons,
        (unsigned long long)plan.exact_windows,(unsigned long long)plan.nonempty_pair_windows,
        (unsigned long long)plan.neighbor_windows,(unsigned long long)plan.full_projections,
        (unsigned long long)plan.b64_projections,(unsigned long long)plan.b256_projections,
        (unsigned long long)plan.changed_tiles,(unsigned long long)plan.zero_score_tiles,
        (unsigned long long)plan.score_fallback_tiles,(unsigned long long)plan.adjacent_sample_score,
        (unsigned long long)plan.selected_sample_score,(unsigned long long)plan.fnv64,plan.exact_masks.size());
    printf("PLAN_GATE graph=%s plan=%s pass=%d invalid=%llu source_edges=%llu expected_edges=%llu bijection=1 masks_independent=1 projection_budget_unchanged=1 feature_first_touch_changed=0\n",
        graph,name.c_str(),int(pass),(unsigned long long)stats.invalid,(unsigned long long)plan.source_edges,(unsigned long long)g.e);
    if(!pass)throw std::runtime_error("pair plan independent gate failed");
    return plan;
}
} // namespace gcn_extra_pair_plan
