#pragma once
#include <algorithm>
#include <numeric>
#include <unordered_set>
#include <cstdint>
namespace gcn_extra_grouping {
struct Schedule { std::string name; std::vector<int> perm; double signature_ms=0, sort_ms=0; uint64_t moved=0; };
inline uint64_t mix(uint64_t x) {
    x+=0x9e3779b97f4a7c15ULL;x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;
    x=(x^(x>>27))*0x94d049bb133111ebULL;return x^(x>>31);
}
inline uint64_t q64(uint32_t d) { return (uint64_t(d)+63)/64; }
struct Budget { uint64_t full=0,b64=0,windows=0,state_load=0,state_store=0,padded=0; };
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
        b.full+=md>0;b.b64+=q64(md);b.windows+=q64(md);b.padded+=uint64_t(md)*16;
    }
    return b;
}
inline std::vector<Schedule> prepare(const gcn_extra_paper::SourceGraphView& g,const int* base,const char* graph) {
    const double begin=omp_get_wtime();
    std::vector<Schedule> out(4);
    const char* names[]={"degree","qshuffle","qsource","qpage"};
    for(int s=0;s<4;s++){out[s].name=names[s];out[s].perm.assign(base,base+g.n);}
    uint64_t native_hash=1469598103934665603ULL;
    for(int i=0;i<g.n;i++)native_hash=(native_hash^uint64_t(base[i]))*1099511628211ULL;
    printf("GROUP_NATIVE_PERM graph=%s fnv64=%llu\n",graph,(unsigned long long)native_hash);
    std::vector<uint64_t> source(g.n),page(g.n),shuffle(g.n);std::vector<int> rank(g.n);
    double t=omp_get_wtime(),allocation_ms=(t-begin)*1000;
    printf("GROUP_SETUP graph=%s phase=schedule_signature_allocation ms=%.12g\n",graph,allocation_ms);
    #pragma omp parallel for schedule(static)
    for(int pos=0;pos<g.n;pos++) {
        int row=base[pos];rank[row]=pos;shuffle[row]=mix(uint64_t(row)^0x20261005ULL);
        uint32_t start=g.row[row],d=g.row[row+1]-start;uint64_t a=UINT64_MAX,b=UINT64_MAX;
        for(uint32_t k=0;k<std::min(64u,d);k++) {
            uint32_t j=g.col[size_t(start)+k];a=std::min(a,mix(j));b=std::min(b,mix(j/16));
        }
        source[row]=a;page[row]=b;
    }
    double keys=(omp_get_wtime()-t)*1000;
    printf("GROUP_SETUP graph=%s phase=signature_pass ms=%.12g sample=first64 seed=fixed\n",graph,keys);
    t=omp_get_wtime();std::vector<std::pair<int,int>> segments;
    for(int i=0;i<g.n;) {
        int end=i+1;uint32_t d=g.row[base[i]+1]-g.row[base[i]];uint64_t q=q64(d);
        while(end<g.n&&q64(g.row[base[end]+1]-g.row[base[end]])==q)end++;
        for(int a=i;a<end;a+=4096)segments.emplace_back(a,std::min(end,a+4096));i=end;
    }
    double segments_ms=(omp_get_wtime()-t)*1000;
    printf("GROUP_SETUP graph=%s phase=segment_construction ms=%.12g segments=%zu cap=4096\n",graph,segments_ms,segments.size());
    Budget reference=budget(g,base);
    for(int s=1;s<4;s++) {
        const auto& key=s==1?shuffle:(s==2?source:page);t=omp_get_wtime();
        #pragma omp parallel for schedule(static)
        for(size_t seg=0;seg<segments.size();seg++) {
            auto bounds=segments[seg];auto& p=out[s].perm;
            std::sort(p.begin()+bounds.first,p.begin()+bounds.second,[&](int a,int b){return key[a]!=key[b]?key[a]<key[b]:rank[a]<rank[b];});
        }
        out[s].sort_ms=(omp_get_wtime()-t)*1000;out[s].signature_ms=keys;t=omp_get_wtime();
        std::vector<unsigned char> seen(g.n,0);uint64_t hash=1469598103934665603ULL;
        for(int pos=0;pos<g.n;pos++) {
            int row=out[s].perm[pos];if(row<0||row>=g.n||seen[row]++)throw std::runtime_error("invalid grouping permutation");
            if(q64(g.row[row+1]-g.row[row])!=q64(g.row[base[pos]+1]-g.row[base[pos]]))throw std::runtime_error("grouping changed per-slot q64");
            out[s].moved+=row!=base[pos];hash=(hash^uint64_t(row))*1099511628211ULL;
        }
        Budget v=budget(g,out[s].perm.data());
        if(v.full!=reference.full||v.b64!=reference.b64||v.windows!=reference.windows||v.state_load!=reference.state_load||v.state_store!=reference.state_store)throw std::runtime_error("grouping budget mismatch");
        double audit_ms=(omp_get_wtime()-t)*1000;
        printf("GROUP_SETUP graph=%s phase=schedule_sort schedule=%s ms=%.12g signature_ms=%.12g\n",graph,out[s].name.c_str(),out[s].sort_ms,out[s].signature_ms);
        printf("GROUP_AUDIT graph=%s schedule=%s pass=1 rows=%d moved=%llu fnv64=%llu full_projections=%llu b64_projections=%llu windows=%llu state_load_bytes=%llu state_store_bytes=%llu padded_steps=%llu audit_ms=%.12g candidate_setup_upper_ms=%.12g\n",graph,out[s].name.c_str(),g.n,(unsigned long long)out[s].moved,(unsigned long long)hash,(unsigned long long)v.full,(unsigned long long)v.b64,(unsigned long long)v.windows,(unsigned long long)v.state_load,(unsigned long long)v.state_store,(unsigned long long)v.padded,audit_ms,allocation_ms+keys+segments_ms+out[s].sort_ms+audit_ms);
    }
    printf("GROUP_SETUP graph=%s phase=all_schedules_total ms=%.12g\n",graph,(omp_get_wtime()-begin)*1000);
    return out;
}
inline void structure(const gcn_extra_paper::SourceGraphView& g,const int* p,const char* graph,const char* schedule) {
    int nt=(g.n+15)/16,limit=std::min(256,nt);std::vector<double> reuse,pages,occupancy;
    for(int q=0;q<limit;q++) {
        int tile=limit==1?0:int(uint64_t(q)*(nt-1)/(limit-1));uint32_t md=0;
        for(int r=tile*16;r<std::min(g.n,tile*16+16);r++)md=std::max(md,g.row[p[r]+1]-g.row[p[r]]);
        if(!md)continue;uint32_t nw=(md+63)/64;
        for(uint32_t wi:{0u,nw/2,nw-1}) {
            std::unordered_set<uint32_t> src,pg;uint64_t edges=0;
            for(int r=tile*16;r<std::min(g.n,tile*16+16);r++) {
                int row=p[r];uint32_t begin=g.row[row],d=g.row[row+1]-begin;
                for(uint32_t k=wi*64;k<std::min(d,(wi+1)*64);k++){uint32_t j=g.col[size_t(begin)+k];src.insert(j);pg.insert(j/16);edges++;}
            }
            if(edges){reuse.push_back(double(edges)/src.size());pages.push_back(pg.size());occupancy.push_back(double(edges)/1024);}
        }
    }
    auto qt=[](std::vector<double>& x,double q){if(x.empty())return 0.;std::sort(x.begin(),x.end());return x[size_t(q*(x.size()-1))];};
    printf("GROUP_STRUCTURE graph=%s schedule=%s window=64 kind=sampled_256_tiles sampled_windows=%zu reuse_p50=%.12g reuse_p90=%.12g source_pages_p50=%.12g occupancy_p50=%.12g\n",graph,schedule,reuse.size(),qt(reuse,.5),qt(reuse,.9),qt(pages,.5),qt(occupancy,.5));
}
}
