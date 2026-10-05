#pragma once
#include <vector>
#include <numeric>
#include <array>
namespace selective {
enum Phase { SETUP, META, INIT, INDEX, LOOKUP, PREFETCH, HLOAD, DECODE, COLD_ADD,
 QLOAD, HOT_ADD, STATE_STORE, HI_PACK, LO_PACK, ALOAD, BLOAD, TDP, CSTORE, MERGE,
 OUTPUT, CACHE_GATHER, CACHE_STORE, RELEASE, PHASES };
constexpr const char* phase_names[]={"thread_amx_setup","row_metadata","partial_init","csr_index",
 "cache_membership","prefetch","BF16_H_load","BF16_decode","cold_FP32_reduce",
 "FP32_Q_load","hot_FP32_reduce","partial_state_store","partial_hi_pack","partial_lo_pack",
 "AMX_A_load","AMX_B_load","AMX_compute","C_store","hot_cold_merge","output_scatter",
 "cache_H_gather","cache_Q_store","thread_amx_release"};
struct Stats{std::array<double,PHASES> sec{};std::array<uint64_t,PHASES> calls{};
 uint64_t edges=0,hot=0,cold=0,cold_tiles=0,cache_tiles=0,parts=0,tdp=0,threads=0;};
template<bool P>inline double tick(bool s){if constexpr(P)if(s)return omp_get_wtime();return 0;}
template<bool P>inline void tock(Stats& p,int phase,double t,bool s){if constexpr(P)if(s){p.sec[phase]+=omp_get_wtime()-t;p.calls[phase]++;}}
inline void add(Stats& a,const Stats& b){for(int k=0;k<PHASES;k++){a.sec[k]+=b.sec[k];a.calls[k]+=b.calls[k];}a.edges+=b.edges;a.hot+=b.hot;a.cold+=b.cold;a.cold_tiles+=b.cold_tiles;a.cache_tiles+=b.cache_tiles;a.parts+=b.parts;a.tdp+=b.tdp;a.threads+=b.threads;}
struct Plan{std::string name;std::vector<int> map,sources;uint64_t hot_edges=0,covered_rows=0,cold_tiles=0;double setup_ms=0;};
inline std::vector<Plan> plans(const csr_t& g,const int* perm,const char* graph){
 double t=omp_get_wtime();std::vector<uint64_t> count(g.N,0);
 #pragma omp parallel for schedule(static)
 for(uint64_t e=0;e<g.nnz;e++){
  #pragma omp atomic update
  count[g.indices[e]]++;
 }
 double counted=omp_get_wtime();std::vector<int> rank(g.N);std::iota(rank.begin(),rank.end(),0);
 std::sort(rank.begin(),rank.end(),[&](int a,int b){return count[a]!=count[b]?count[a]>count[b]:a<b;});
 double sorted=omp_get_wtime();printf("SEL_SETUP graph=%s phase=source_consumers ms=%.12g\n",graph,(counted-t)*1000);printf("SEL_SETUP graph=%s phase=source_rank ms=%.12g\n",graph,(sorted-counted)*1000);
 std::vector<Plan> out;const char* names[]={"hybrid_none","top_1_64","top_1_16","top_1_4","hybrid_all","project_used"};int denoms[]={0,64,16,4,1,-1};
 for(int method=0;method<6;method++){
  double a=omp_get_wtime();Plan p;p.name=names[method];p.map.assign(g.N,-1);int ns=denoms[method]>0?(g.N+denoms[method]-1)/denoms[method]:(method==5?g.N:0);
  if(method!=4)while(ns>0&&!count[rank[ns-1]])ns--;
  p.sources.assign(rank.begin(),rank.begin()+ns);if(method==4)std::iota(p.sources.begin(),p.sources.end(),0);for(int i=0;i<ns;i++)p.map[p.sources[i]]=i;
  for(int i=0;i<ns;i++)p.hot_edges+=count[p.sources[i]];
  std::vector<unsigned char> cold_row(g.N,0);
  #pragma omp parallel for schedule(static)
  for(int i=0;i<g.N;i++){for(uint32_t e=g.indptr[i];e<g.indptr[i+1];e++)if(p.map[g.indices[e]]<0){cold_row[i]=1;break;}}
  uint64_t nonempty=0;for(int i=0;i<g.N;i++)if(g.indptr[i+1]>g.indptr[i]){nonempty++;p.covered_rows+=!cold_row[i];}
  for(int i=0;i<g.N;i+=16){bool any=false;for(int r=i;r<std::min(g.N,i+16);r++)any|=cold_row[perm[r]];p.cold_tiles+=any;}
  p.setup_ms=(sorted-t+omp_get_wtime()-a)*1000;
  printf("SEL_PLAN graph=%s method=%s selected_sources=%d hot_edges=%llu cold_edges=%llu covered_nonempty_rows=%llu nonempty_rows=%llu cold_projection_tiles=%llu setup_cost_estimate_ms=%.12g\n",graph,p.name.c_str(),ns,(unsigned long long)p.hot_edges,(unsigned long long)(g.nnz-p.hot_edges),(unsigned long long)p.covered_rows,(unsigned long long)nonempty,(unsigned long long)p.cold_tiles,p.setup_ms);out.push_back(std::move(p));
 }
 return out;
}
template<int D,int F>void pack(const float* w,uint16_t* dst){
 for(int kb=0;kb<D/32;kb++)for(int ob=0;ob<F/16;ob++)for(int pair=0;pair<16;pair++)for(int f=0;f<16;f++){
  size_t off=((kb*(F/16)+ob)*16+pair)*32+f*2;
  dst[off]=f32_to_bf16(w[(kb*32+2*pair)*F+ob*16+f]);dst[off+1]=f32_to_bf16(w[(kb*32+2*pair+1)*F+ob*16+f]);
 }
}
template<int D,int F,bool P>inline void project(const uint16_t* x,const uint16_t* w,const uint16_t* lo,float* c,Stats& p,bool sample){
 constexpr int NB_SHAPE=F/16;
 for(int panel=0;panel<(NB_SHAPE+3)/4;panel++){
  _tile_zero(TC0);_tile_zero(TC1);if constexpr(F==128){_tile_zero(TC2);_tile_zero(TC3);}
  for(int component=0;component<(lo?2:1);component++)for(int kb=0;kb<D/32;kb++){
   const auto* a=component?lo:x;double t=tick<P>(sample);_tile_loadd(TA,a+kb*32,D*2);tock<P>(p,ALOAD,t,sample);
   int ob=panel*4;t=tick<P>(sample);_tile_loadd(TB0,w+(kb*NB_SHAPE+ob)*512,64);_tile_loadd(TB1,w+(kb*NB_SHAPE+ob+1)*512,64);tock<P>(p,BLOAD,t,sample);
   t=tick<P>(sample);_tile_dpbf16ps(TC0,TA,TB0);_tile_dpbf16ps(TC1,TA,TB1);tock<P>(p,TDP,t,sample);if constexpr(P)p.tdp+=2;
   if constexpr(F==128){t=tick<P>(sample);_tile_loadd(TB0,w+(kb*NB_SHAPE+ob+2)*512,64);_tile_loadd(TB1,w+(kb*NB_SHAPE+ob+3)*512,64);tock<P>(p,BLOAD,t,sample);t=tick<P>(sample);_tile_dpbf16ps(TC2,TA,TB0);_tile_dpbf16ps(TC3,TA,TB1);tock<P>(p,TDP,t,sample);if constexpr(P)p.tdp+=2;}
  }
  double t=tick<P>(sample);_tile_stored(TC0,c+panel*64,F*4);_tile_stored(TC1,c+panel*64+16,F*4);if constexpr(F==128){_tile_stored(TC2,c+panel*64+32,F*4);_tile_stored(TC3,c+panel*64+48,F*4);}tock<P>(p,CSTORE,t,sample);
 }
 if constexpr(P)p.parts+=lo?2:1;
}
template<int D,int F,bool P>void cache(const uint16_t* h,const uint16_t* w,float* q,const Plan& plan,Stats* result){
 std::vector<Stats> stats(omp_get_max_threads());
 #pragma omp parallel
 {
  Stats& p=stats[omp_get_thread_num()];double t=tick<P>(true);if(syscall(SYS_arch_prctl,0x1023,18)!=0)abort();tilecfg_t cfg;setup_tilecfg(&cfg);_tile_loadconfig(&cfg);tock<P>(p,SETUP,t,true);
  alignas(64) uint16_t input[16*D];alignas(64) float c[16*F];
  #pragma omp for schedule(dynamic,1) nowait
  for(size_t rg=0;rg<plan.sources.size();rg+=64)for(size_t i=rg;i<std::min(plan.sources.size(),rg+64);i+=16){
   bool sample=P&&((i/16)%256==0||i+16>=plan.sources.size());int n=std::min(size_t(16),plan.sources.size()-i);
   double a=tick<P>(sample);memset(input,0,sizeof(input));for(int r=0;r<n;r++)memcpy(input+r*D,h+size_t(plan.sources[i+r])*D,D*2);tock<P>(p,CACHE_GATHER,a,sample);
   project<D,F,P>(input,w,nullptr,c,p,sample);a=tick<P>(sample);for(int r=0;r<n;r++)memcpy(q+(i+r)*F,c+r*F,F*4);tock<P>(p,CACHE_STORE,a,sample);if constexpr(P)p.cache_tiles++;
  }
  t=tick<P>(true);_tile_release();tock<P>(p,RELEASE,t,true);if constexpr(P)p.threads++;
 }
 if constexpr(P)for(auto& p:stats)add(*result,p);
}
// Kind0: fused FULL. Kind1: selective hot/cold. Kind2: all-source projected
// cache, direct source indexing; same source layout as the all-source plan.
template<int D,int F,int Kind,bool P,bool B16>void reduce(const csr_t& g,const int* perm,const uint16_t* h,const uint16_t* w,const Plan& plan,const float* q,std::conditional_t<B16,uint16_t,float>* out,Stats* result){
 std::vector<Stats> stats(omp_get_max_threads());
 #pragma omp parallel
 {
  Stats& p=stats[omp_get_thread_num()];double t=tick<P>(true);if(syscall(SYS_arch_prctl,0x1023,18)!=0)abort();tilecfg_t cfg;setup_tilecfg(&cfg);_tile_loadconfig(&cfg);tock<P>(p,SETUP,t,true);
  alignas(64) float cold[16*D],hot[16*F],c[16*F];alignas(64) uint16_t hi[16*D],lo[16*D];int rows[16];uint32_t base[16],degree[16];
  #pragma omp for schedule(dynamic,1) nowait
  for(int rg=0;rg<g.N;rg+=64)for(int i=rg;i<std::min(g.N,rg+64);i+=16){
   bool sample=P&&((i/16)%256==0||i+16>=g.N);int n=std::min(16,g.N-i);uint32_t md=0;bool has_cold=false;
   t=tick<P>(sample);for(int r=0;r<n;r++){rows[r]=perm[i+r];base[r]=g.indptr[rows[r]];degree[r]=g.indptr[rows[r]+1]-base[r];md=std::max(md,degree[r]);}tock<P>(p,META,t,sample);
   t=tick<P>(sample);if constexpr(Kind<=1)memset(cold,0,sizeof(cold));if constexpr(Kind!=0)memset(hot,0,sizeof(hot));memset(c,0,sizeof(c));tock<P>(p,INIT,t,sample);
   for(uint32_t b=0;b<md;b+=64)for(int r=0;r<n;r++){
    uint32_t end=std::min(degree[r],b+64);if(b>=end)continue;
    __m512 ac[D/16],aq[F/16];if constexpr(Kind<=1)for(int f=0;f<D/16;f++)ac[f]=_mm512_load_ps(cold+r*D+f*16);
    if constexpr(Kind!=0)for(int f=0;f<F/16;f++)aq[f]=_mm512_load_ps(hot+r*F+f*16);
    for(uint32_t k=b;k<end;k++){
     t=tick<P>(sample);uint32_t j=g.indices[size_t(base[r])+k];tock<P>(p,INDEX,t,sample);
     int ix=-1;if constexpr(Kind==2)ix=int(j);else if constexpr(Kind!=0){t=tick<P>(sample);ix=plan.map[j];tock<P>(p,LOOKUP,t,sample);}
     t=tick<P>(sample);if(k+1<end){uint32_t next=g.indices[size_t(base[r])+k+1];int nx=-1;if constexpr(Kind==2)nx=int(next);else if constexpr(Kind!=0)nx=plan.map[next];const char* ptr=nx>=0?(const char*)(q+size_t(nx)*F):(const char*)(h+size_t(next)*D);int bytes=nx>=0?4*F:2*D;for(int off=0;off<bytes;off+=64)_mm_prefetch(ptr+off,_MM_HINT_T0);}tock<P>(p,PREFETCH,t,sample);
     if constexpr(Kind!=0){if(ix>=0){
      __m512 val[F/16];t=tick<P>(sample);for(int f=0;f<F/16;f++)val[f]=_mm512_loadu_ps(q+size_t(ix)*F+16*f);tock<P>(p,QLOAD,t,sample);
      t=tick<P>(sample);for(int f=0;f<F/16;f++)aq[f]=_mm512_add_ps(aq[f],val[f]);tock<P>(p,HOT_ADD,t,sample);if constexpr(P){p.hot++;p.edges++;}continue;
     }}
     if constexpr(Kind<=1){
      has_cold=true;__m256i raw[D/16];t=tick<P>(sample);for(int f=0;f<D/16;f++)raw[f]=_mm256_loadu_si256((const __m256i*)(h+size_t(j)*D+16*f));tock<P>(p,HLOAD,t,sample);
      __m512 val[D/16];t=tick<P>(sample);for(int f=0;f<D/16;f++)val[f]=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(raw[f]),16));tock<P>(p,DECODE,t,sample);
      t=tick<P>(sample);for(int f=0;f<D/16;f++)ac[f]=_mm512_add_ps(ac[f],val[f]);tock<P>(p,COLD_ADD,t,sample);if constexpr(P){p.cold++;p.edges++;}
     }else throw std::runtime_error("all-source plan has a cache miss");
    }
    t=tick<P>(sample);if constexpr(Kind<=1)for(int f=0;f<D/16;f++)_mm512_store_ps(cold+r*D+f*16,ac[f]);if constexpr(Kind!=0)for(int f=0;f<F/16;f++)_mm512_store_ps(hot+r*F+f*16,aq[f]);tock<P>(p,STATE_STORE,t,sample);
   }
   if constexpr(Kind<=1){if(has_cold){
    for(int v=0;v<16*D;v+=16){__m512 a=_mm512_load_ps(cold+v);t=tick<P>(sample);__m256i top=_mm512_cvtepi32_epi16(_mm512_srli_epi32(_mm512_castps_si512(a),16));_mm256_store_si256((__m256i*)(hi+v),top);tock<P>(p,HI_PACK,t,sample);
     t=tick<P>(sample);__m512 high=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(top),16));__m512 delta=_mm512_sub_ps(a,high);_mm256_store_si256((__m256i*)(lo+v),_mm512_cvtepi32_epi16(_mm512_srli_epi32(_mm512_castps_si512(delta),16)));tock<P>(p,LO_PACK,t,sample);}
    project<D,F,P>(hi,w,lo,c,p,sample);if constexpr(P)p.cold_tiles++;
   }}
   for(int r=0;r<n;r++){
    if constexpr(Kind!=0){t=tick<P>(sample);for(int f=0;f<F/16;f++)_mm512_store_ps(c+r*F+16*f,_mm512_add_ps(_mm512_load_ps(c+r*F+16*f),_mm512_load_ps(hot+r*F+16*f)));tock<P>(p,MERGE,t,sample);}
    t=tick<P>(sample);gcn_extra_paper::scatter_output<B16>(out+size_t(rows[r])*F,c+r*F,F);tock<P>(p,OUTPUT,t,sample);
   }
  }
  t=tick<P>(true);_tile_release();tock<P>(p,RELEASE,t,true);if constexpr(P)p.threads++;
 }
 if constexpr(P)for(auto& p:stats)add(*result,p);
}
template<int D,int F,bool P,bool B16>void layer(const csr_t& g,const int* perm,const uint16_t* h,const uint16_t* w,const Plan& plan,float* q,std::conditional_t<B16,uint16_t,float>* out,int kind,Stats* s){
 if(kind&&!plan.sources.empty())cache<D,F,P>(h,w,q,plan,s);
 if(kind==0)reduce<D,F,0,P,B16>(g,perm,h,w,plan,q,out,s);
 else if(kind==2)reduce<D,F,2,P,B16>(g,perm,h,w,plan,q,out,s);
 else if(kind==3)reduce<D,F,3,P,B16>(g,perm,h,w,plan,q,out,s);
 else reduce<D,F,1,P,B16>(g,perm,h,w,plan,q,out,s);
}
}
