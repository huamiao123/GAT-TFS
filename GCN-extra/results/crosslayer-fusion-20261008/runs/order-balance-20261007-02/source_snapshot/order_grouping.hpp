#pragma once
#include <algorithm>
#include <numeric>
#include <array>
#include <cstdint>
namespace order_balance {
struct Group {
 std::string name;std::vector<int> perm;
 uint64_t full_tiles=0,visit_rounds=0,row_windows=0,padded_steps=0,hash=0;
};
inline uint64_t mix(uint64_t x){x+=0x9e3779b97f4a7c15ULL;x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;x=(x^(x>>27))*0x94d049bb133111ebULL;return x^(x>>31);}
inline uint64_t q64(uint32_t d){return (uint64_t(d)+63)/64;}
inline void audit_group(const csr_t& g,Group& s){
 std::vector<unsigned char> seen(g.N,0);s.hash=1469598103934665603ULL;
 for(int pos=0;pos<g.N;pos++){int r=s.perm[pos];if(r<0||r>=g.N||seen[r]++)throw std::runtime_error("order grouping is not permutation");s.hash=(s.hash^uint64_t(r))*1099511628211ULL;s.row_windows+=q64(g.indptr[r+1]-g.indptr[r]);}
 for(int i=0;i<g.N;i+=16){uint32_t md=0;for(int r=i;r<std::min(g.N,i+16);r++)md=std::max(md,g.indptr[s.perm[r]+1]-g.indptr[s.perm[r]]);s.full_tiles+=md>0;s.visit_rounds+=q64(md);s.padded_steps+=uint64_t(md)*16;}
}
inline std::vector<Group> groups(const csr_t& g,const int* degree,const char* graph){
 double begin=omp_get_wtime();std::vector<Group> out(2);out[0].name="degree";out[1].name="qsource";
 for(auto& s:out)s.perm.assign(degree,degree+g.N);
 std::vector<uint64_t> keys(g.N,UINT64_MAX);std::vector<int> rank(g.N);
 #pragma omp parallel for schedule(static)
 for(int pos=0;pos<g.N;pos++){int r=degree[pos];rank[r]=pos;uint32_t d=g.indptr[r+1]-g.indptr[r];for(uint32_t k=0;k<std::min(64u,d);k++)keys[r]=std::min(keys[r],mix(g.indices[size_t(g.indptr[r])+k]));}
 double sig=omp_get_wtime();std::vector<std::pair<int,int>> segments;
 for(int i=0;i<g.N;){int end=i+1;uint64_t q=q64(g.indptr[degree[i]+1]-g.indptr[degree[i]]);while(end<g.N&&q64(g.indptr[degree[end]+1]-g.indptr[degree[end]])==q)end++;for(int a=i;a<end;a+=4096)segments.emplace_back(a,std::min(end,a+4096));i=end;}
 #pragma omp parallel for schedule(static)
 for(size_t i=0;i<segments.size();i++){auto b=segments[i];std::sort(out[1].perm.begin()+b.first,out[1].perm.begin()+b.second,[&](int a,int c){return keys[a]!=keys[c]?keys[a]<keys[c]:rank[a]<rank[c];});}
 double sorted=omp_get_wtime();
 for(auto& s:out){audit_group(g,s);for(int pos=0;pos<g.N;pos++)if(q64(g.indptr[s.perm[pos]+1]-g.indptr[s.perm[pos]])!=q64(g.indptr[degree[pos]+1]-g.indptr[degree[pos]]))throw std::runtime_error("order grouping changed per-slot q64");}
 if(out[0].full_tiles!=out[1].full_tiles||out[0].visit_rounds!=out[1].visit_rounds||out[0].row_windows!=out[1].row_windows)throw std::runtime_error("order grouping budget changed");
 printf("ORDER_GROUP_SETUP graph=%s sample=first64 segment_cap=4096 signature_ms=%.12g sorting_ms=%.12g audit_ms=%.12g total_ms=%.12g\n",graph,(sig-begin)*1000,(sorted-sig)*1000,(omp_get_wtime()-sorted)*1000,(omp_get_wtime()-begin)*1000);
 for(auto& s:out)printf("ORDER_GROUP graph=%s grouping=%s rows=%d full_tiles=%llu visit_rounds=%llu row_windows=%llu padded_steps=%llu perm_hash=%llu same_per_slot_q64=1 pass=1\n",graph,s.name.c_str(),g.N,(unsigned long long)s.full_tiles,(unsigned long long)s.visit_rounds,(unsigned long long)s.row_windows,(unsigned long long)s.padded_steps,(unsigned long long)s.hash);
 return out;
}
// Independent plans: 0=A (no cache), 1=Pall (source ID order), 2=Pused
// (nonzero consumer count, descending frequency then original source ID).
inline std::vector<selective::Plan> source_plans(const csr_t& g,const char* graph){
 double begin=omp_get_wtime();std::vector<uint64_t> count(g.N,0);
 #pragma omp parallel for schedule(static)
 for(uint64_t e=0;e<g.nnz;e++){
  #pragma omp atomic update
  count[g.indices[e]]++;
 }
 double counted=omp_get_wtime();std::vector<selective::Plan> p(3);p[0].name="A";p[1].name="Pall";p[2].name="Pused";
 p[1].sources.resize(g.N);std::iota(p[1].sources.begin(),p[1].sources.end(),0);p[1].hot_edges=g.nnz;
 for(int i=0;i<g.N;i++)if(count[i])p[2].sources.push_back(i);
 std::sort(p[2].sources.begin(),p[2].sources.end(),[&](int a,int b){return count[a]!=count[b]?count[a]>count[b]:a<b;});
 p[2].map.assign(g.N,-1);for(size_t i=0;i<p[2].sources.size();i++)p[2].map[p[2].sources[i]]=int(i);p[2].hot_edges=g.nnz;
 double end=omp_get_wtime();
 printf("ORDER_PLAN_SETUP graph=%s count_ms=%.12g plans_ms=%.12g total_ms=%.12g excluded_from_prepared=1\n",graph,(counted-begin)*1000,(end-counted)*1000,(end-begin)*1000);
 for(auto& s:p)printf("ORDER_PLAN graph=%s plan=%s sources=%zu cache_tiles=%zu map_entries=%zu hot_edges=%llu\n",graph,s.name.c_str(),s.sources.size(),(s.sources.size()+15)/16,s.map.size(),(unsigned long long)s.hot_edges);
 return p;
}
}
