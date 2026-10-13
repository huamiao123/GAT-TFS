#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <list>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace gcn_extra_grouping {
struct Schedule { std::string name; std::vector<int> perm; double signature_ms=0, sort_ms=0; uint64_t moved=0; };
inline uint64_t mix(uint64_t x) {
    x+=0x9e3779b97f4a7c15ULL;x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;
    x=(x^(x>>27))*0x94d049bb133111ebULL;return x^(x>>31);
}
inline uint64_t q64(uint32_t d) { return (uint64_t(d)+63)/64; }
struct Budget { uint64_t full=0,b64=0,b256=0,windows=0,state_load=0,state_store=0,padded=0; };
inline Budget budget(const gcn_extra_paper::SourceGraphView& g,const int* p) {
    Budget b;
    for(int i=0;i<g.n;i+=16) {
        uint32_t md=0;int batch=std::min(16,g.n-i);
        for(int r=0;r<batch;r++) {
            uint32_t row=p[i+r],d=g.row[row+1]-g.row[row];md=std::max(md,d);
        }
        if(md)for(int r=0;r<batch;r++) {
            uint32_t row=p[i+r],d=g.row[row+1]-g.row[row];
            if(d)b.state_load+=(q64(d)-1)*512;
            b.state_store+=std::max<uint64_t>(1,q64(d))*512;
        }
        b.full+=md>0;b.b64+=q64(md);b.b256+=(uint64_t(md)+255)/256;
        b.windows+=q64(md);b.padded+=uint64_t(md)*16;
    }
    return b;
}
// The probe is a bounded model of a concatenated, sampled source trace.
// Its capacities are feature rows, not a claim about physical cache capacity.
struct FeatureRowLRU {
    size_t capacity=0;
    uint64_t misses=0;
    std::list<uint32_t> order;
    std::unordered_map<uint32_t,std::list<uint32_t>::iterator> lookup;
    void init(size_t n) { capacity=n;lookup.reserve(n*2); }
    void touch(uint32_t row) {
        auto it=lookup.find(row);
        if(it!=lookup.end()) { order.splice(order.begin(),order,it->second);return; }
        misses++;
        if(order.size()==capacity) { lookup.erase(order.back());order.pop_back(); }
        order.push_front(row);lookup.emplace(row,order.begin());
    }
};
struct GuardProbe {
    uint64_t requests=0,windows=0;
    std::array<uint64_t,3> misses{};
};
inline int guarded_row(const gcn_extra_paper::SourceGraphView& g,const int* base,
                       const int* candidate,int pos) {
    int row=base[pos];uint32_t d=g.row[row+1]-g.row[row];
    return q64(d)<=1?row:candidate[pos];
}
inline GuardProbe guard_probe(const gcn_extra_paper::SourceGraphView& g,
                              const int* base,const int* candidate,
                              const std::vector<int>& tiles) {
    const size_t capacities[3]={256,1024,4096};
    std::array<FeatureRowLRU,3> caches;
    for(int c=0;c<3;c++)caches[c].init(capacities[c]);
    GuardProbe p;
    for(int tile:tiles) {
        const int first=tile*16,last=std::min(g.n,first+16);
        uint64_t nw=0;
        for(int pos=first;pos<last;pos++) {
            int row=base[pos];nw=std::max(nw,q64(g.row[row+1]-g.row[row]));
        }
        if(!nw)continue;
        const uint64_t windows[3]={0,nw/2,nw-1};
        uint64_t previous=UINT64_MAX;
        for(uint64_t wi:windows) {
            if(wi==previous)continue;
            previous=wi;p.windows++;
            const uint64_t window_start=wi*64,window_end=window_start+64;
            for(int pos=first;pos<last;pos++) {
                int row=guarded_row(g,base,candidate,pos);
                uint32_t begin=g.row[row],d=g.row[row+1]-begin;
                for(uint64_t k=window_start;k<std::min<uint64_t>(d,window_end);k++) {
                    uint32_t j=g.col[size_t(begin)+size_t(k)];p.requests++;
                    for(auto& cache:caches)cache.touch(j);
                }
            }
        }
    }
    for(int c=0;c<3;c++)p.misses[c]=caches[c].misses;
    return p;
}
inline std::vector<Schedule> prepare(const gcn_extra_paper::SourceGraphView& g,const int* base,const char* graph) {
    const double begin=omp_get_wtime();
    std::vector<Schedule> out(5);
    const char* names[]={"degree","qsource","qpage","qsamplepage","qguard"};
    for(int s=0;s<5;s++){out[s].name=names[s];out[s].perm.assign(base,base+g.n);}
    uint64_t native_hash=1469598103934665603ULL;
    for(int i=0;i<g.n;i++)native_hash=(native_hash^uint64_t(base[i]))*1099511628211ULL;
    printf("GROUP_NATIVE_PERM graph=%s fnv64=%llu\n",graph,(unsigned long long)native_hash);
    std::vector<uint64_t> source(g.n),page(g.n),samplepage(g.n);std::vector<int> rank(g.n);
    double t=omp_get_wtime(),allocation_ms=(t-begin)*1000;
    printf("GROUP_SETUP graph=%s phase=schedule_signature_allocation ms=%.12g\n",graph,allocation_ms);
    #pragma omp parallel for schedule(static)
    for(int pos=0;pos<g.n;pos++) {
        int row=base[pos];rank[row]=pos;
        uint32_t start=g.row[row],d=g.row[row+1]-start;
        uint64_t a=UINT64_MAX,b=UINT64_MAX,c=UINT64_MAX;
        const uint32_t count=std::min(64u,d);
        for(uint32_t k=0;k<count;k++) {
            uint32_t j=g.col[size_t(start)+k];a=std::min(a,mix(j));b=std::min(b,mix(j/16));
        }
        // Endpoints included; at most 64 distinct, deterministic CSR positions.
        for(uint32_t k=0;k<count;k++) {
            const uint32_t offset=count<=1?0:uint32_t(uint64_t(k)*(d-1)/(count-1));
            uint32_t j=g.col[size_t(start)+offset];
            c=std::min(c,mix(uint64_t(j/16)^0x20261007ULL));
        }
        source[row]=a;page[row]=b;samplepage[row]=c;
    }
    double keys=(omp_get_wtime()-t)*1000;
    printf("GROUP_SETUP graph=%s phase=signature_pass ms=%.12g sample=first64_and_uniform64 seed=0x20261007 shared_candidates=3 page_model=relative_source_id_div16\n",graph,keys);
    t=omp_get_wtime();std::vector<std::pair<int,int>> segments;
    for(int i=0;i<g.n;) {
        int end=i+1;uint32_t d=g.row[base[i]+1]-g.row[base[i]];uint64_t q=q64(d);
        while(end<g.n&&q64(g.row[base[end]+1]-g.row[base[end]])==q)end++;
        for(int a=i;a<end;a+=4096)segments.emplace_back(a,std::min(end,a+4096));i=end;
    }
    double segments_ms=(omp_get_wtime()-t)*1000;
    printf("GROUP_SETUP graph=%s phase=segment_construction ms=%.12g segments=%zu cap=4096\n",graph,segments_ms,segments.size());
    for(int s=1;s<=3;s++) {
        const auto& key=s==1?source:(s==2?page:samplepage);t=omp_get_wtime();
        #pragma omp parallel for schedule(static)
        for(size_t seg=0;seg<segments.size();seg++) {
            auto bounds=segments[seg];auto& p=out[s].perm;
            std::sort(p.begin()+bounds.first,p.begin()+bounds.second,[&](int a,int b){return key[a]!=key[b]?key[a]<key[b]:rank[a]<rank[b];});
        }
        out[s].sort_ms=(omp_get_wtime()-t)*1000;out[s].signature_ms=keys;
        printf("GROUP_SETUP graph=%s phase=schedule_sort schedule=%s ms=%.12g signature_ms=%.12g shared_signature_pass=1\n",graph,out[s].name.c_str(),out[s].sort_ms,out[s].signature_ms);
    }
    t=omp_get_wtime();
    std::vector<int> high_tiles;
    for(int first=0;first<g.n;first+=16) {
        bool high=false;
        for(int pos=first;pos<std::min(g.n,first+16);pos++) {
            int row=base[pos];if(q64(g.row[row+1]-g.row[row])>1){high=true;break;}
        }
        if(high)high_tiles.push_back(first/16);
    }
    const int limit=std::min<size_t>(128,high_tiles.size());
    std::vector<int> sampled_tiles;sampled_tiles.reserve(limit);
    for(int i=0;i<limit;i++)sampled_tiles.push_back(high_tiles[limit==1?0:size_t(uint64_t(i)*(high_tiles.size()-1)/(limit-1))]);
    std::array<GuardProbe,4> probes;
    for(int s=0;s<=3;s++)probes[s]=guard_probe(g,base,out[s].perm.data(),sampled_tiles);
    int selected=0;double best_score=1.;
    const auto& ref=probes[0];
    for(int s=0;s<=3;s++) {
        const auto& p=probes[s];bool no_worse=p.requests>0&&ref.requests>0;int improves=0;
        std::array<double,3> ratios{{1.,1.,1.}};double score=0.;
        for(int c=0;c<3;c++) {
            if(p.requests&&ref.requests&&ref.misses[c]) {
                ratios[c]=(double(p.misses[c])/p.requests)/(double(ref.misses[c])/ref.requests);
                // At most 128*3*16*64 requests: these exact cross-products fit uint64_t.
                const uint64_t lhs=p.misses[c]*ref.requests,rhs=ref.misses[c]*p.requests;
                no_worse=no_worse&&(lhs<=rhs);
                if(lhs*100<=rhs*95)improves++;
            } else no_worse=false;
            score+=ratios[c]/3.;
        }
        const bool eligible=s>0&&no_worse&&improves>=2;
        printf("GROUP_GUARD_PROBE graph=%s candidate=%s sampled_tiles=%d sampled_windows=%llu requests=%llu misses256=%llu misses1024=%llu misses4096=%llu ratio256=%.12g ratio1024=%.12g ratio4096=%.12g mean_ratio=%.12g no_worse=%d improvements_ge5pct=%d eligible=%d low_q_fallback=1 model=sampled_trace_LRU_not_hardware_cache\n",graph,out[s].name.c_str(),limit,(unsigned long long)p.windows,(unsigned long long)p.requests,(unsigned long long)p.misses[0],(unsigned long long)p.misses[1],(unsigned long long)p.misses[2],ratios[0],ratios[1],ratios[2],score,no_worse,improves,eligible);
        if(eligible&&score<best_score){selected=s;best_score=score;}
    }
    const double probe_ms=(omp_get_wtime()-t)*1000;t=omp_get_wtime();
    for(int pos=0;pos<g.n;pos++)out[4].perm[pos]=guarded_row(g,base,out[selected].perm.data(),pos);
    out[4].sort_ms=(omp_get_wtime()-t)*1000;out[4].signature_ms=keys;
    printf("GROUP_GUARD_SELECT graph=%s schedule=qguard selected=%s predicted_mean_miss_rate_ratio=%.12g sampled_tiles=%d high_q_tiles=%zu low_q_fallback=1 uses_timing=0 probe_ms=%.12g assembly_ms=%.12g\n",graph,out[selected].name.c_str(),best_score,limit,high_tiles.size(),probe_ms,out[4].sort_ms);
    printf("GROUP_SETUP graph=%s phase=guard_probe ms=%.12g\n",graph,probe_ms);
    printf("GROUP_SETUP graph=%s phase=guard_schedule_assembly ms=%.12g\n",graph,out[4].sort_ms);
    const Budget reference=budget(g,base);
    for(int s=0;s<5;s++) {
        t=omp_get_wtime();std::vector<unsigned char> seen(g.n,0);uint64_t hash=1469598103934665603ULL;
        for(int pos=0;pos<g.n;pos++) {
            int row=out[s].perm[pos];if(row<0||row>=g.n||seen[row]++)throw std::runtime_error("invalid grouping permutation");
            if(q64(g.row[row+1]-g.row[row])!=q64(g.row[base[pos]+1]-g.row[base[pos]]))throw std::runtime_error("grouping changed per-slot q64");
            out[s].moved+=row!=base[pos];hash=(hash^uint64_t(row))*1099511628211ULL;
        }
        const Budget v=budget(g,out[s].perm.data());
        if(v.full!=reference.full||v.b64!=reference.b64||v.b256!=reference.b256||v.windows!=reference.windows||v.state_load!=reference.state_load||v.state_store!=reference.state_store)throw std::runtime_error("grouping budget mismatch");
        const double audit_ms=(omp_get_wtime()-t)*1000;
        printf("GROUP_AUDIT graph=%s schedule=%s pass=1 rows=%d moved=%llu fnv64=%llu full_projections=%llu b64_projections=%llu b256_projections=%llu windows=%llu state_load_bytes=%llu state_store_bytes=%llu padded_steps=%llu audit_ms=%.12g\n",graph,out[s].name.c_str(),g.n,(unsigned long long)out[s].moved,(unsigned long long)hash,(unsigned long long)v.full,(unsigned long long)v.b64,(unsigned long long)v.b256,(unsigned long long)v.windows,(unsigned long long)v.state_load,(unsigned long long)v.state_store,(unsigned long long)v.padded,audit_ms);
    }
    // This is measured construction of the entire five-schedule pipeline.
    // It is not an isolated candidate cost or a mathematical runtime upper bound.
    printf("GROUP_SETUP graph=%s phase=all_schedules_total ms=%.12g measurement=actual_complete_construction includes_all_candidates_and_guard=1\n",graph,(omp_get_wtime()-begin)*1000);
    return out;
}
inline void structure(const gcn_extra_paper::SourceGraphView& g,const int* p,const char* graph,const char* schedule) {
    int nt=(g.n+15)/16,limit=std::min(256,nt);std::vector<double> reuse,pages,occupancy;
    for(int q=0;q<limit;q++) {
        int tile=limit==1?0:int(uint64_t(q)*(nt-1)/(limit-1));uint32_t md=0;
        for(int r=tile*16;r<std::min(g.n,tile*16+16);r++)md=std::max(md,g.row[p[r]+1]-g.row[p[r]]);
        if(!md)continue;uint32_t nw=uint32_t(q64(md));
        for(uint32_t wi:{0u,nw/2,nw-1}) {
            std::unordered_set<uint32_t> src,pg;uint64_t edges=0;
            for(int r=tile*16;r<std::min(g.n,tile*16+16);r++) {
                int row=p[r];uint32_t begin=g.row[row],d=g.row[row+1]-begin;
                for(uint64_t k=uint64_t(wi)*64;k<std::min<uint64_t>(d,(uint64_t(wi)+1)*64);k++){uint32_t j=g.col[size_t(begin)+size_t(k)];src.insert(j);pg.insert(j/16);edges++;}
            }
            if(edges){reuse.push_back(double(edges)/src.size());pages.push_back(pg.size());occupancy.push_back(double(edges)/1024);}
        }
    }
    auto qt=[](std::vector<double>& x,double q){if(x.empty())return 0.;std::sort(x.begin(),x.end());return x[size_t(q*(x.size()-1))];};
    printf("GROUP_STRUCTURE graph=%s schedule=%s window=64 kind=sampled_256_tiles sampled_windows=%zu reuse_p50=%.12g reuse_p90=%.12g source_pages_p50=%.12g occupancy_p50=%.12g\n",graph,schedule,reuse.size(),qt(reuse,.5),qt(reuse,.9),qt(pages,.5),qt(occupancy,.5));
}
}
