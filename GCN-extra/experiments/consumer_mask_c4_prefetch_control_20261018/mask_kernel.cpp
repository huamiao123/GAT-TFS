#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <immintrin.h>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <fcntl.h>
#include <omp.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
using U=uint64_t;
static void check(bool b,const char* s){if(!b)throw std::runtime_error(s);}
struct Graph {
 int fd=-1;void* ptr=MAP_FAILED;size_t bytes=0;U n=0,e=0;
 const uint32_t *row=nullptr,*col=nullptr;
 explicit Graph(const std::string& path){
  fd=open(path.c_str(),O_RDONLY);check(fd>=0,"open CSR failed");
  struct stat st{};check(fstat(fd,&st)==0&&st.st_size>=36,"bad CSR size");
  bytes=size_t(st.st_size);ptr=mmap(nullptr,bytes,PROT_READ,MAP_PRIVATE,fd,0);check(ptr!=MAP_FAILED,"mmap failed");
  auto p=static_cast<const unsigned char*>(ptr);uint32_t fmt[3];std::memcpy(fmt,p,12);
  U nc;std::memcpy(&n,p+12,8);std::memcpy(&nc,p+20,8);std::memcpy(&e,p+28,8);
  check(fmt[0]==0&&fmt[1]==0&&fmt[2]==2&&n==nc,"unsupported CSR");
  check(n<=U(std::numeric_limits<int>::max())&&e<=UINT32_MAX,"CSR index overflow");
  check(36+4*(n+1)+8*e==bytes,"CSR file length mismatch");
  row=reinterpret_cast<const uint32_t*>(p+36);
  col=reinterpret_cast<const uint32_t*>(p+36+4*(n+1));
  check(row[0]==0&&row[n]==e,"CSR endpoints");
  for(U i=0;i<n;i++)check(row[i]<=row[i+1]&&row[i+1]<=e,"CSR row bounds");
  for(U i=0;i<e;i++)check(col[i]<n,"CSR col bounds");
 }
 ~Graph(){if(ptr!=MAP_FAILED)munmap(ptr,bytes);if(fd>=0)close(fd);}
};
struct Entry {uint32_t src;uint16_t mask;uint16_t pad=0;};
static_assert(sizeof(Entry)==8,"entry size");
struct Plan {
 std::vector<Entry> data;std::vector<U> offsets;
 U saved=0,duplicate=0,groups=0;double build_s=0;
 Plan(const Graph&g,U reserve_count){
  auto begin=omp_get_wtime();data.reserve(reserve_count);offsets.reserve((g.n+15)/16+1);offsets.push_back(0);
  std::unordered_map<uint32_t,uint32_t> map;std::vector<Entry> tile;
  for(U start=0;start<g.n;start+=16){
   map.clear();tile.clear();
   for(U pos=start;pos<std::min(g.n,start+16);pos++){
    uint32_t d=uint32_t(pos),bit=1u<<(pos-start);
    for(U x=g.row[d];x<g.row[d+1];x++){
     auto [it,inserted]=map.try_emplace(g.col[x],0);
     if(it->second&bit)duplicate++;
     it->second|=bit;
    }
   }
   check(duplicate==0,"duplicate edge requires multiplicity plan; aborting this isolated kernel");
   tile.reserve(map.size());
   for(auto [source,mask]:map)tile.push_back({source,uint16_t(mask),0});
   std::sort(tile.begin(),tile.end(),[](const Entry&a,const Entry&b){return a.mask!=b.mask?a.mask<b.mask:a.src<b.src;});
   for(size_t a=0;a<tile.size();){
    size_t b=a+1;while(b<tile.size()&&tile[b].mask==tile[a].mask)b++;
    U nn=b-a,k=__builtin_popcount((unsigned)tile[a].mask);groups++;
    if(nn>=2&&k>=2)saved+=(nn-1)*(k-1);
    a=b;
   }
   data.insert(data.end(),tile.begin(),tile.end());offsets.push_back(data.size());
  }
  build_s=omp_get_wtime()-begin;
 }
 U bytes()const{return data.size()*sizeof(Entry)+offsets.size()*sizeof(U);}
};
static inline __m512 decode16(const uint16_t*p){
 __m256i x=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(p));
 return _mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(x),16));
}
static inline void add_bf16(float*d,const uint16_t*s){
 for(int f=0;f<128;f+=16){__m512 a=_mm512_loadu_ps(d+f);_mm512_storeu_ps(d+f,_mm512_add_ps(a,decode16(s+f)));}
}
static inline void add_float(float*d,const float*s){
 for(int f=0;f<128;f+=16){__m512 a=_mm512_loadu_ps(d+f);__m512 b=_mm512_loadu_ps(s+f);_mm512_storeu_ps(d+f,_mm512_add_ps(a,b));}
}
static inline void copy_bf16(float*d,const uint16_t*s){
 for(int f=0;f<128;f+=16)_mm512_storeu_ps(d+f,decode16(s+f));
}
template<bool PREFETCH> static void rowwise(const Graph&g,const uint16_t*h,float*out){
 #pragma omp parallel for schedule(static)
 for(U d=0;d<g.n;d++){
  U begin=g.row[d],end=g.row[d+1];if(begin==end)continue;
  __m512 acc[8];for(auto& v:acc)v=_mm512_setzero_ps();
  for(U x=begin;x<end;x++){
   const uint16_t* src=h+U(g.col[x])*128;
   if constexpr(PREFETCH) if(x+1<end){
    const char* next=reinterpret_cast<const char*>(h+U(g.col[x+1])*128);
    for(int off=0;off<256;off+=64)_mm_prefetch(next+off,_MM_HINT_T0);
   }
   for(int f=0;f<8;f++)acc[f]=_mm512_add_ps(acc[f],decode16(src+f*16));
  }
  for(int f=0;f<8;f++)_mm512_storeu_ps(out+d*128+f*16,acc[f]);
 }
}
static void plan_compute(const Graph&g,const Plan&p,const uint16_t*h,float*out,bool shared){
 (void)g;
 #pragma omp parallel for schedule(static)
 for(U t=0;t<p.offsets.size()-1;t++){
  alignas(64) float tmp[128];U start=t*16;
  for(U a=p.offsets[t];a<p.offsets[t+1];){
   U b=a+1;uint16_t mask=p.data[a].mask;
   while(b<p.offsets[t+1]&&p.data[b].mask==mask)b++;
   int k=__builtin_popcount((unsigned)mask);
   if(shared&&b-a>=2&&k>=2){
    copy_bf16(tmp,h+U(p.data[a].src)*128);
    for(U z=a+1;z<b;z++)add_bf16(tmp,h+U(p.data[z].src)*128);
    for(int bit=0;bit<16;bit++)if(mask&(1u<<bit))add_float(out+(start+bit)*128,tmp);
   } else {
    for(U z=a;z<b;z++)for(int bit=0;bit<16;bit++)if(mask&(1u<<bit))
      add_bf16(out+(start+bit)*128,h+U(p.data[z].src)*128);
   }
   a=b;
  }
 }
}
static std::vector<uint16_t> features(U n){
 std::vector<uint16_t> h(n*128);
 for(U i=0;i<n;i++)for(U f=0;f<128;f++){
  U x=(i+1)*0x9e3779b97f4a7c15ULL+(f+7)*0xbf58476d1ce4e5b9ULL;
  x^=x>>30;x*=0xbf58476d1ce4e5b9ULL;x^=x>>27;
  uint32_t a=uint32_t((x>>8)&0x00ffffffu);
  float v=float(a)/16777216.0f;
  uint32_t bits;std::memcpy(&bits,&v,4);
  h[i*128+f]=uint16_t(bits>>16);
 }
 return h;
}
struct Error{double rel_l2=0,norm_max=0,max_abs=0;U bf16_mismatches=0;};
static Error compare(const std::vector<float>&ref,const std::vector<float>&got){
 check(ref.size()==got.size(),"output size mismatch");
 long double d2=0,r2=0;double ma=0,mr=0;U mism=0;
 for(size_t i=0;i<ref.size();i++){
  check(std::isfinite(ref[i])&&std::isfinite(got[i]),"nonfinite output");
  double d=double(got[i])-ref[i];d2+=d*d;r2+=double(ref[i])*ref[i];
  ma=std::max(ma,std::abs(d));mr=std::max(mr,std::abs(double(ref[i])));
  uint32_t x,y;std::memcpy(&x,&ref[i],4);std::memcpy(&y,&got[i],4);mism+=(x>>16)!=(y>>16);
 }
 return {std::sqrt(double(d2/r2)),ma/std::max(1.0,mr),ma,mism};
}
static void err_json(std::ostream&o,const Error&e){
 o<<"{\"rel_l2\":"<<e.rel_l2<<",\"norm_max\":"<<e.norm_max
  <<",\"max_abs\":"<<e.max_abs<<",\"bf16_mismatches\":"<<e.bf16_mismatches<<'}';
}
static std::vector<double> time_method(const Graph&g,const Plan&p,const uint16_t*h,std::vector<float>&out,
                                        const std::string&method,int warmups,int repeats){
 std::vector<double> v;
 for(int rep=-warmups;rep<repeats;rep++){
  std::fill(out.begin(),out.end(),0.0f);double start=omp_get_wtime();
  if(method=="rowwise_np")rowwise<false>(g,h,out.data());
  else if(method=="rowwise_pf")rowwise<true>(g,h,out.data());
  else plan_compute(g,p,h,out.data(),method=="shared");
  double elapsed=omp_get_wtime()-start;if(rep>=0)v.push_back(elapsed);
 }
 return v;
}
static void array(std::ostream&o,const std::vector<double>&v){o<<'[';for(size_t i=0;i<v.size();i++){if(i)o<<',';o<<v[i];}o<<']';}
int main(int argc,char**argv){
 try{
  std::string path,label="graph",output;U reserve=0,expected=UINT64_MAX;int repeats=3;
  for(int i=1;i<argc;i++){
   check(i+1<argc,"option value missing");std::string k=argv[i],v=argv[++i];
   if(k=="--csr")path=v;else if(k=="--graph")label=v;else if(k=="--out")output=v;
   else if(k=="--reserve")reserve=std::stoull(v);else if(k=="--expected-save")expected=std::stoull(v);
   else if(k=="--repeats")repeats=std::stoi(v);else throw std::runtime_error("unknown option "+k);
  }
  check(!path.empty()&&!output.empty()&&repeats>=1&&repeats<=8,"bad arguments");
  Graph g(path);Plan p(g,reserve);check(expected==UINT64_MAX||p.saved==expected,"saved-add count mismatch against scan");
  auto h=features(g.n);std::vector<float> ref(g.n*128),got(g.n*128);
  std::vector<int> cpu(omp_get_max_threads(),-1);
  #pragma omp parallel
  cpu[omp_get_thread_num()]=sched_getcpu();
  rowwise<false>(g,h.data(),ref.data());rowwise<true>(g,h.data(),got.data());Error prefetch_error=compare(ref,got);
  check(prefetch_error.rel_l2==0&&prefetch_error.max_abs==0,"prefetch changed rowwise output");
  std::fill(got.begin(),got.end(),0.0f);plan_compute(g,p,h.data(),got.data(),false);Error fanout_error=compare(ref,got);
  std::fill(got.begin(),got.end(),0.0f);plan_compute(g,p,h.data(),got.data(),true);Error shared_error=compare(ref,got);
  check(fanout_error.rel_l2<=1e-2&&fanout_error.norm_max<=1e-2,"fanout accuracy gate failed");
  check(shared_error.rel_l2<=1e-2&&shared_error.norm_max<=1e-2,"shared accuracy gate failed");
  const char* methods[]={"rowwise_np","rowwise_pf","fanout","shared"};
  const int order[8][4]={{0,1,2,3},{1,2,3,0},{2,3,0,1},{3,0,1,2},
                         {3,2,1,0},{2,1,0,3},{1,0,3,2},{0,3,2,1}};
  std::vector<double> times[4];
  for(int m=0;m<4;m++)time_method(g,p,h.data(),got,methods[m],1,0);
  for(int rep=0;rep<repeats;rep++)for(int slot=0;slot<4;slot++){
   int m=order[rep][slot];auto x=time_method(g,p,h.data(),got,methods[m],0,1);
   times[m].push_back(x.at(0));
  }
  std::ofstream f(output);check(bool(f),"output open failed");
  f<<"{\"schema\":\"mask-kernel-prefetch-control-v1\",\"feature_distribution\":\"deterministic_uniform_24bit_then_bf16_truncate\",\"graph\":\""<<label<<"\",\"N\":"<<g.n
   <<",\"E\":"<<g.e<<",\"D\":128,\"order\":\"identity\",\"threads\":"<<omp_get_max_threads()
   <<",\"plan_entries\":"<<p.data.size()<<",\"plan_groups\":"<<p.groups
   <<",\"plan_bytes\":"<<p.bytes()<<",\"plan_build_s\":"<<p.build_s
   <<",\"duplicate_edges\":"<<p.duplicate<<",\"saved_adds_per_feature\":"<<p.saved
   <<",\"prefetch_error\":";err_json(f,prefetch_error);
  f<<",\"fanout_error\":";err_json(f,fanout_error);f<<",\"shared_error\":";err_json(f,shared_error);
  f<<",\"cpu_placement\":[";for(size_t i=0;i<cpu.size();i++){if(i)f<<',';f<<cpu[i];}f<<']';
  f<<",\"measurement_orders\":[[0,1,2,3],[1,2,3,0],[2,3,0,1],[3,0,1,2],[3,2,1,0],[2,1,0,3],[1,0,3,2],[0,3,2,1]]";
  f<<",\"warmups\":1,\"repeats\":"<<repeats<<",\"rowwise_np_s\":";array(f,times[0]);
  f<<",\"rowwise_pf_s\":";array(f,times[1]);
  f<<",\"fanout_s\":";array(f,times[2]);f<<",\"shared_s\":";array(f,times[3]);f<<"}\n";
  check(bool(f),"output write failed");
  std::cout<<"KERNEL_PASS graph="<<label<<" entries="<<p.data.size()<<" plan_bytes="<<p.bytes()
           <<" build_s="<<p.build_s<<" save="<<p.saved<<" rel_l2="<<shared_error.rel_l2<<'\n';
  return 0;
 }catch(const std::exception&e){std::cerr<<"ERROR: "<<e.what()<<'\n';return 2;}
}
