#pragma once
#include "projection_window_kernels.hpp"
#include "paper_methods_runtime.hpp"
#include "projection_window_pmu.hpp"
namespace residual_broad {
using namespace gcn_extra_paper;
enum { META, INIT, CSR, PREFETCH, HLOAD, DECODE, ADD, PSTORE, HI,
 RLOAD, RDECODE, RSUB, RADD, RSTORE, RHI, RLO, CZERO, CLOAD, ALOAD,
 BLOAD, COMPUTE, CSTORE, SCATTER_OUT, PERMISSION, CONFIG, RELEASE, PHASES_R };
constexpr const char* phases[]={"row_metadata","state_init","csr_index","prefetch",
 "source_load","source_decode","fp32_reduce","partial_store","partial_hi_pack",
 "residual_load","partial_hi_decode","residual_subtract","residual_add",
 "residual_store","correction_hi_pack","correction_lo_pack","C_zero","C_load",
 "AMX_A_load","AMX_B_load","AMX_compute","C_store","output_scatter",
 "AMX_permission","AMX_config","AMX_release"};
struct Stats { std::array<double,PHASES_R> sec{}; std::array<uint64_t,PHASES_R> calls{};
 uint64_t tiles=0,edges=0,blocks=0,corrections=0,parts=0,tdp=0; };
template<bool P> inline double start(bool sample){if constexpr(P)if(sample)return omp_get_wtime();return 0;}
template<bool P> inline void stop(Stats& p,int phase,double t,bool sample){if constexpr(P)if(sample){p.sec[phase]+=omp_get_wtime()-t;p.calls[phase]++;}}
template<bool P> inline void project(const uint16_t* x,const uint16_t* w,int panel,Stats& p,bool s) {
 for(int kb=0;kb<4;kb++){
  double t=start<P>(s);_tile_loadd(TA,x+kb*32,256);stop<P>(p,ALOAD,t,s);
  int ob=panel*4;t=start<P>(s);_tile_loadd(TB0,w+(kb*8+ob)*512,64);_tile_loadd(TB1,w+(kb*8+ob+1)*512,64);stop<P>(p,BLOAD,t,s);
  t=start<P>(s);_tile_dpbf16ps(TC0,TA,TB0);_tile_dpbf16ps(TC1,TA,TB1);stop<P>(p,COMPUTE,t,s);
  t=start<P>(s);_tile_loadd(TB0,w+(kb*8+ob+2)*512,64);_tile_loadd(TB1,w+(kb*8+ob+3)*512,64);stop<P>(p,BLOAD,t,s);
  t=start<P>(s);_tile_dpbf16ps(TC2,TA,TB0);_tile_dpbf16ps(TC3,TA,TB1);stop<P>(p,COMPUTE,t,s);
  if constexpr(P)p.tdp+=4;
 }
}
template<bool P,bool B16> void kernel(const SourceGraphView& g,const uint16_t* h,const uint16_t* w,
 std::conditional_t<B16,uint16_t,float>* out,const int* perm,int period,Stats* result){
 std::vector<Stats> stats(omp_get_max_threads());
 #pragma omp parallel
 {
  Stats& pr=stats[omp_get_thread_num()];double t=start<P>(true);
  if(syscall(SYS_arch_prctl,0x1023,18)!=0)abort();stop<P>(pr,PERMISSION,t,true);
  tilecfg_t cfg;setup_tilecfg(&cfg);t=start<P>(true);_tile_loadconfig(&cfg);stop<P>(pr,CONFIG,t,true);
  alignas(64) float partial[2048],residual[2048],ctile[2048];
  alignas(64) uint16_t hi[2048],lo[2048];uint32_t bases[16],degrees[16];int rows[16];
  #pragma omp for schedule(dynamic,1) nowait
  for(int rg=0;rg<g.n;rg+=64)for(int i=rg;i<std::min(g.n,rg+64);i+=16){
   bool sample=P&&((i/16)%256==0||i+16>=g.n);int batch=std::min(16,g.n-i);uint32_t md=0;
   t=start<P>(sample);for(int r=0;r<batch;r++){rows[r]=perm[i+r];bases[r]=g.row[rows[r]];degrees[r]=g.row[rows[r]+1]-bases[r];md=std::max(md,degrees[r]);}
   stop<P>(pr,META,t,sample);if constexpr(P){pr.tiles++;for(int r=0;r<batch;r++)pr.edges+=degrees[r];}
   t=start<P>(sample);memset(ctile,0,sizeof(ctile));memset(residual,0,sizeof(residual));stop<P>(pr,INIT,t,sample);
   bool first=true;uint32_t block=0;
   auto consume=[&](const uint16_t* x,const uint16_t* second){
    for(int panel=0;panel<2;panel++){
     double q=start<P>(sample);
     if(first){_tile_zero(TC0);_tile_zero(TC1);_tile_zero(TC2);_tile_zero(TC3);stop<P>(pr,CZERO,q,sample);}
     else{_tile_loadd(TC0,ctile+panel*64,512);_tile_loadd(TC1,ctile+panel*64+16,512);_tile_loadd(TC2,ctile+panel*64+32,512);_tile_loadd(TC3,ctile+panel*64+48,512);stop<P>(pr,CLOAD,q,sample);}
     project<P>(x,w,panel,pr,sample);if(second)project<P>(second,w,panel,pr,sample);
     q=start<P>(sample);_tile_stored(TC0,ctile+panel*64,512);_tile_stored(TC1,ctile+panel*64+16,512);_tile_stored(TC2,ctile+panel*64+32,512);_tile_stored(TC3,ctile+panel*64+48,512);stop<P>(pr,CSTORE,q,sample);
    }first=false;if constexpr(P)pr.parts+=second?2:1;
   };
   for(uint32_t b=0;b<md;b+=64){
    if constexpr(P)pr.blocks++;block++;
    t=start<P>(sample);memset(partial,0,sizeof(partial));stop<P>(pr,INIT,t,sample);
    for(int r=0;r<batch;r++){
     __m512 acc[8];for(auto& a:acc)a=_mm512_setzero_ps();uint32_t end=std::min(degrees[r],b+64);
     for(uint32_t k=b;k<end;k++){
      t=start<P>(sample);uint32_t j=g.col[size_t(bases[r])+k];stop<P>(pr,CSR,t,sample);
      t=start<P>(sample);if(k+1<end){const char* next=(const char*)(h+size_t(g.col[size_t(bases[r])+k+1])*128);for(int f=0;f<256;f+=64)_mm_prefetch(next+f,_MM_HINT_T0);}stop<P>(pr,PREFETCH,t,sample);
      __m256i raw[8];t=start<P>(sample);for(int f=0;f<8;f++)raw[f]=_mm256_loadu_si256((const __m256i*)(h+size_t(j)*128+16*f));stop<P>(pr,HLOAD,t,sample);
      __m512 v[8];t=start<P>(sample);for(int f=0;f<8;f++)v[f]=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(raw[f]),16));stop<P>(pr,DECODE,t,sample);
      t=start<P>(sample);for(int f=0;f<8;f++)acc[f]=_mm512_add_ps(acc[f],v[f]);stop<P>(pr,ADD,t,sample);
     }
     t=start<P>(sample);for(int f=0;f<8;f++)_mm512_store_ps(partial+r*128+f*16,acc[f]);stop<P>(pr,PSTORE,t,sample);
    }
    for(int v=0;v<2048;v+=16){
     t=start<P>(sample);__m512 a=_mm512_load_ps(partial+v);__m256i top=_mm512_cvtepi32_epi16(_mm512_srli_epi32(_mm512_castps_si512(a),16));_mm256_store_si256((__m256i*)(hi+v),top);stop<P>(pr,HI,t,sample);
     t=start<P>(sample);__m512 old=_mm512_load_ps(residual+v);stop<P>(pr,RLOAD,t,sample);
     t=start<P>(sample);__m512 q=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(top),16));stop<P>(pr,RDECODE,t,sample);
     t=start<P>(sample);__m512 delta=_mm512_sub_ps(a,q);stop<P>(pr,RSUB,t,sample);
     t=start<P>(sample);__m512 sum=_mm512_add_ps(old,delta);stop<P>(pr,RADD,t,sample);
     t=start<P>(sample);_mm512_store_ps(residual+v,sum);stop<P>(pr,RSTORE,t,sample);
    }
    consume(hi,nullptr);
    if(b+64>=md||(period>0&&block%period==0)){
     if constexpr(P)pr.corrections++;
     for(int v=0;v<2048;v+=16){
      __m512 a=_mm512_load_ps(residual+v);t=start<P>(sample);__m256i top=_mm512_cvtepi32_epi16(_mm512_srli_epi32(_mm512_castps_si512(a),16));_mm256_store_si256((__m256i*)(hi+v),top);stop<P>(pr,RHI,t,sample);
      t=start<P>(sample);__m512 q=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(top),16));__m512 d=_mm512_sub_ps(a,q);_mm256_store_si256((__m256i*)(lo+v),_mm512_cvtepi32_epi16(_mm512_srli_epi32(_mm512_castps_si512(d),16)));stop<P>(pr,RLO,t,sample);
     }
     consume(hi,lo);t=start<P>(sample);memset(residual,0,sizeof(residual));stop<P>(pr,INIT,t,sample);
    }
   }
   t=start<P>(sample);for(int r=0;r<batch;r++)scatter_output<B16>(out+size_t(rows[r])*128,ctile+r*128,128);stop<P>(pr,SCATTER_OUT,t,sample);
  }
  t=start<P>(true);_tile_release();stop<P>(pr,RELEASE,t,true);
 }
 if constexpr(P)for(auto& p:stats){for(int k=0;k<PHASES_R;k++){result->sec[k]+=p.sec[k];result->calls[k]+=p.calls[k];}result->tiles+=p.tiles;result->edges+=p.edges;result->blocks+=p.blocks;result->corrections+=p.corrections;result->parts+=p.parts;result->tdp+=p.tdp;}
}
struct Choice{const char* name;int kind,period;};
template<bool B16>void one(const SourceGraphView& g,const Choice& m,const uint16_t* h,const uint16_t* w,std::conditional_t<B16,uint16_t,float>* out,const int* perm){
 if(m.kind==0){if constexpr(B16)tfs_v3_bf16out(g.row.p,g.col.p,h,w,out,perm,g.n,64);else tfs_v3_fp32out(g.row.p,g.col.p,h,w,out,perm,g.n,64);}
 else if(m.kind==1){gcn_extra_paper::Method c{m.name,64,true,false,0,true};block_kernel<false,B16>(g,h,w,out,perm,c,nullptr);}
 else if(m.kind==2){gcn_extra_projection::Method c{m.name,64,0,true,true};gcn_extra_projection::kernel<false,B16>(g,h,w,out,perm,c,nullptr);}
 else kernel<false,B16>(g,h,w,out,perm,m.period,nullptr);
}
template<bool B16>void forward(const SourceGraphView& g,const Choice& m,const uint16_t* h0,const uint16_t* w1,const uint16_t* w2,float* h1,uint16_t* h1b,std::conditional_t<B16,uint16_t,float>* out,const int* p){one<false>(g,m,h0,w1,h1,p);relu_f32(h1,size_t(g.n)*128);convert_f32_to_bf16(h1,h1b,size_t(g.n)*128);one<B16>(g,m,h1b,w2,out,p);}
template<class A,class B>bool observe(const char* graph,const Choice& m,const char* boundary,const char* ref,const A* a,const B* b,size_t n,double gate){
 auto e=error(a,b,n);bool pass=e.finite&&e.l2<gate&&e.nmax<gate;
 printf("D2_CHECK graph=%s method=%s boundary=%s reference=%s max_abs=%.12g relative_L2=%.12g normalized_max=%.12g finite=%d gate=%.9g pass=%d\n",graph,m.name,boundary,ref,e.max_abs,e.l2,e.nmax,e.finite,gate,pass);fflush(stdout);return pass;
}
inline void run(const csr_t& csr,const int* perm,const float*,const float*,const float*,const uint16_t* w1,const uint16_t* w2,const uint16_t* h0,float* h1,uint16_t* h1b,uint16_t* h2b,float* h2f,sparse_matrix_t,matrix_descr,float*,float*,float* mkl2,const char* graph){
 SourceGraphView g{csr.N,csr.nnz,{csr.indptr},{csr.indices}};size_t n=size_t(g.n)*128;
 std::vector<Choice> choices{{"paper_tfs",0,0},{"b64_accurate",1,0},{"s64_mfull_accurate",2,0},{"d2_b64_all",3,0},{"d3_b64_period4",3,4}};
 std::vector<float> ref(h2f,h2f+n),kernel_ref(n);std::vector<uint16_t> bref(h2b,h2b+n);
 one<false>(g,choices[0],h0,w1,kernel_ref.data(),perm);std::vector<bool> accepted(choices.size(),true);
 printf("D2_CONFIG graph=%s N=%d E=%llu D=128 F=128 seed=12345 window=64 repeats=5 NUMA=default threads=%d\n",graph,g.n,(unsigned long long)g.e,omp_get_max_threads());
 for(size_t k=0;k<choices.size();k++){
  auto m=choices[k];std::fill(h1,h1+n,NAN);one<false>(g,m,h0,w1,h1,perm);accepted[k]=observe(graph,m,"kernel_FP32","paper_tfs",h1,kernel_ref.data(),n,.001);
  forward<false>(g,m,h0,w1,w2,h1,h1b,h2f,perm);accepted[k]=observe(graph,m,"e2e_FP32_final","paper_tfs",h2f,ref.data(),n,.001)&&accepted[k];
  forward<true>(g,m,h0,w1,w2,h1,h1b,h2b,perm);accepted[k]=observe(graph,m,"e2e_BF16_final","paper_tfs",h2b,bref.data(),n,.01)&&accepted[k];
  accepted[k]=observe(graph,m,"e2e_BF16_final","source_MKL_FP32",h2b,mkl2,n,.03)&&accepted[k];
  printf("D2_ACCEPT graph=%s method=%s accepted=%d\n",graph,m.name,bool(accepted[k]));
  if(k<3&&!accepted[k])throw std::runtime_error("frozen reference gate failed");
 }
 for(size_t k=0;k<choices.size();k++)if(accepted[k])forward<true>(g,choices[k],h0,w1,w2,h1,h1b,h2b,perm);
 for(int rep=0;rep<5;rep++)for(size_t pos=0;pos<choices.size();pos++){
  size_t k=rep%2?choices.size()-1-pos:pos;if(!accepted[k])continue;auto m=choices[k];double t=omp_get_wtime();forward<true>(g,m,h0,w1,w2,h1,h1b,h2b,perm);
  printf("D2_TIME graph=%s method=%s repeat=%d order=%zu kind=alternating_e2e ms=%.12g\n",graph,m.name,rep,pos,(omp_get_wtime()-t)*1000);fflush(stdout);
 }
 for(size_t k=0;k<choices.size();k++)if(accepted[k]){
  auto m=choices[k];for(int rep=0;rep<3;rep++){double a=omp_get_wtime();one<false>(g,m,h0,w1,h1,perm);double b=omp_get_wtime();relu_f32(h1,n);double c=omp_get_wtime();convert_f32_to_bf16(h1,h1b,n);double d=omp_get_wtime();one<true>(g,m,h1b,w2,h2b,perm);double e=omp_get_wtime();printf("D2_STAGE graph=%s method=%s repeat=%d layer1_ms=%.12g relu_ms=%.12g conversion_ms=%.12g layer2_ms=%.12g total_ms=%.12g\n",graph,m.name,rep,(b-a)*1000,(c-b)*1000,(d-c)*1000,(e-d)*1000,(e-a)*1000);}
  if(m.kind==3){
   one<false>(g,m,h0,w1,h1,perm);std::vector<float> expected(h1,h1+n);Stats p;double t=omp_get_wtime();kernel<true,false>(g,h0,w1,h1,perm,m.period,&p);
   bool bits=memcmp(h1,expected.data(),n*4)==0;printf("D2_PROFILE_GATE graph=%s method=%s bitwise=%d pass=%d wall_ms=%.12g\n",graph,m.name,bits,bits,(omp_get_wtime()-t)*1000);if(!bits)throw std::runtime_error("profile gate");
   for(int ph=0;ph<PHASES_R;ph++)printf("D2_DETAIL graph=%s method=%s phase=%s sampled_thread_ms=%.12g clocked_sections=%llu additive_wall=0\n",graph,m.name,phases[ph],p.sec[ph]*1000,(unsigned long long)p.calls[ph]);
   printf("D2_WORK graph=%s method=%s edges=%llu blocks=%llu corrections=%llu parts=%llu tdp=%llu residual_state_bytes_per_thread=8192\n",graph,m.name,(unsigned long long)p.edges,(unsigned long long)p.blocks,(unsigned long long)p.corrections,(unsigned long long)p.parts,(unsigned long long)p.tdp);
   if(p.edges!=g.e||p.tdp!=32*p.parts)throw std::runtime_error("D2 work gate");
  }
  for(int rep=0;rep<3;rep++){gcn_extra_projection_pmu::Region pmu;pmu.start();forward<true>(g,m,h0,w1,w2,h1,h1b,h2b,perm);auto pm=pmu.stop();gcn_extra_projection_pmu::print(pm,graph,m.name,rep);}
 }
 printf("D2_COMPLETE graph=%s methods=5 rejected=%d pass=1\n",graph,int(std::count(accepted.begin(),accepted.end(),false)));fflush(stdout);
}
}
