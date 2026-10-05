#define main original_source_main
#include "gcn_e2e_v3.cpp"
#undef main
#include "paper_methods_runtime.hpp"
#include "projection_window_kernels.hpp"
#include "projection_window_pmu.hpp"
#include "selective_kernels.hpp"
#include <fstream>
#include <random>
#include <memory>
namespace selective {
template<class T>struct Buffer{
 T* p;size_t n;Buffer(size_t size):n(size){p=(T*)mkl_malloc(std::max(size_t(64),n*sizeof(T)),64);if(!p)throw std::bad_alloc();}
 ~Buffer(){mkl_free(p);}Buffer(const Buffer&)=delete;T& operator[](size_t i){return p[i];}
};
struct Choice{std::string name;int kind,plan,second=-1;};
template<int F,bool B16>void forward(const csr_t& g,const int* perm,const Choice& m,const Plan& plan,const uint16_t* h0,const uint16_t* w1,const uint16_t* w2,float* q1,float* q2,float* h1,uint16_t* h1b,std::conditional_t<B16,uint16_t,float>* out){
 layer<128,F,false,false>(g,perm,h0,w1,plan,q1,h1,m.kind,nullptr);relu_f32(h1,size_t(g.N)*F);convert_f32_to_bf16(h1,h1b,size_t(g.N)*F);layer<F,F,false,B16>(g,perm,h1b,w2,plan,q2,out,m.second<0?m.kind:m.second,nullptr);
}
template<class A,class B>bool observe(const char* graph,int f,const Choice& m,const char* boundary,const char* ref,const A* a,const B* b,size_t n,double gate){
 auto e=gcn_extra_paper::error(a,b,n);bool pass=e.finite&&e.l2<gate&&e.nmax<gate;
 printf("SEL_CHECK graph=%s shape=128_%d_%d method=%s boundary=%s reference=%s max_abs=%.12g relative_L2=%.12g normalized_max=%.12g finite=%d gate=%.9g pass=%d\n",graph,f,f,m.name.c_str(),boundary,ref,e.max_abs,e.l2,e.nmax,e.finite,gate,pass);fflush(stdout);return pass;
}
template<int D,int F>void mkl_one(sparse_matrix_t a,matrix_descr desc,const float* h,const float* w,float* z,float* out,int n){
 auto status=mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE,1.f,a,desc,SPARSE_LAYOUT_ROW_MAJOR,h,D,D,0.f,z,D);
 if(status!=SPARSE_STATUS_SUCCESS)throw std::runtime_error("shape MKL sparse status");
 cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,n,F,D,1.f,z,D,w,F,0.f,out,F);
}
template<int F>void run(csr_t& g,const int* perm,std::vector<Plan>& pl,const char* graph){
 const size_t nd=size_t(g.N)*128,nf=size_t(g.N)*F;
 Buffer<float> h0(nd),w1(128*F),w2(F*F),h0q(nd),w1q(128*F),w2q(F*F),h1(nf),out(nf),ref1(nf),ref2(nf),mkl1(nf),mkl2(nf),z(nd),mkh1q(nf);
 Buffer<uint16_t> hb(nd),wb1(128*F),wb2(F*F),h1b(nf),final(nf),refb(nf);
 srand(12345);for(size_t k=0;k<w1.n;k++){w1[k]=.01f*((rand()%200)-100);if(k<w2.n)w2[k]=.01f*((rand()%200)-100);}
 for(size_t k=0;k<nd;k++)h0[k]=.01f*((rand()%200)-100);
 convert_f32_to_bf16(h0.p,hb.p,nd);pack<128,F>(w1.p,wb1.p);pack<F,F>(w2.p,wb2.p);
 #pragma omp parallel for schedule(static)
 for(size_t k=0;k<nd;k++)h0q[k]=bf16_to_f32(hb[k]);
 for(size_t k=0;k<w1.n;k++)w1q[k]=bf16_to_f32(f32_to_bf16(w1[k]));for(size_t k=0;k<w2.n;k++)w2q[k]=bf16_to_f32(f32_to_bf16(w2[k]));
 std::vector<MKL_INT> rs(g.N),re(g.N),ci(g.nnz);for(int i=0;i<g.N;i++){rs[i]=g.indptr[i];re[i]=g.indptr[i+1];}for(size_t e=0;e<g.nnz;e++)ci[e]=g.indices[e];
 sparse_matrix_t a;matrix_descr desc{};desc.type=SPARSE_MATRIX_TYPE_GENERAL;
 if(mkl_sparse_s_create_csr(&a,SPARSE_INDEX_BASE_ZERO,g.N,g.N,rs.data(),re.data(),ci.data(),g.values)!=SPARSE_STATUS_SUCCESS)throw std::runtime_error("MKL create");
 mkl_sparse_set_mm_hint(a,SPARSE_OPERATION_NON_TRANSPOSE,desc,SPARSE_LAYOUT_ROW_MAJOR,128,10);mkl_sparse_optimize(a);
 // Separate matched-input numerical reference: both H/W quantized, and the
 // native BF16 interlayer boundary preserved. Not a timed baseline claim.
 mkl_one<128,F>(a,desc,h0q.p,w1q.p,z.p,ref1.p,g.N);memcpy(mkl1.p,ref1.p,nf*4);relu_f32(mkl1.p,nf);convert_f32_to_bf16(mkl1.p,h1b.p,nf);
 #pragma omp parallel for schedule(static)
 for(size_t k=0;k<nf;k++)mkh1q[k]=bf16_to_f32(h1b[k]);
 mkl_one<F,F>(a,desc,mkh1q.p,w2q.p,z.p,ref2.p,g.N);convert_f32_to_bf16(ref2.p,refb.p,nf);
 std::vector<Choice> methods{{"fused_full_accurate",0,0},{"hybrid_none",1,0},{"top_1_64",1,1},{"top_1_16",1,2},{"top_1_4",1,3},{"hybrid_all",1,4},{"project_all",2,4},{"project_used",3,5},{"project_used_L1_full_L2",3,5,0}};
 std::vector<std::unique_ptr<Buffer<float>>> q1,q2;for(auto& m:methods){size_t ns=pl[m.plan].sources.size();q1.emplace_back(new Buffer<float>(ns*F));q2.emplace_back(new Buffer<float>(ns*F));}
 printf("SEL_CONFIG graph=%s shape=128_%d_%d N=%d E=%u threads=%d seed=12345 TR=16 R=64 S=64 partial=ACCURATE cache=FP32 final=BF16 repeats=5 numa=default\n",graph,F,F,g.N,g.nnz,omp_get_max_threads());
 std::vector<bool> accepted(methods.size(),true);std::vector<float> full1(nf),all1(nf),all_final(nf);
 for(size_t k=0;k<methods.size();k++){
  auto& m=methods[k];auto& p=pl[m.plan];std::fill(h1.p,h1.p+nf,NAN);layer<128,F,false,false>(g,perm,hb.p,wb1.p,p,q1[k]->p,h1.p,m.kind,nullptr);
  accepted[k]=observe(graph,F,m,"kernel_FP32","matched_quantized_MKL_math",h1.p,ref1.p,nf,.001);
  if(k==0)full1.assign(h1.p,h1.p+nf);if(k==5)all1.assign(h1.p,h1.p+nf);
  if(k==1||k==6){auto& expected=k==1?full1:all1;bool eq=memcmp(expected.data(),h1.p,nf*4)==0;printf("SEL_EQUIV graph=%s shape=128_%d_%d method=%s boundary=kernel_FP32 bitwise=%d pass=%d\n",graph,F,F,m.name.c_str(),eq,eq);if(!eq)throw std::runtime_error("selector degenerate control gate");}
  if constexpr(F==128){if(k==0){gcn_extra_paper::SourceGraphView view{g.N,g.nnz,{g.indptr},{g.indices}};gcn_extra_projection::Method frozen{"s64_mfull_accurate",64,0,true,true};gcn_extra_projection::kernel<false,false>(view,hb.p,wb1.p,out.p,perm,frozen,nullptr);bool eq=memcmp(out.p,h1.p,nf*4)==0;printf("SEL_EQUIV graph=%s shape=128_128_128 method=fused_full_accurate boundary=frozen_P0_kernel_FP32 bitwise=%d pass=%d\n",graph,eq,eq);if(!eq)throw std::runtime_error("generic/frozen P0 mismatch");}}
  forward<F,false>(g,perm,m,p,hb.p,wb1.p,wb2.p,q1[k]->p,q2[k]->p,h1.p,h1b.p,out.p);
  accepted[k]=observe(graph,F,m,"e2e_FP32_final","matched_quantized_MKL_math",out.p,ref2.p,nf,.001)&&accepted[k];
  forward<F,true>(g,perm,m,p,hb.p,wb1.p,wb2.p,q1[k]->p,q2[k]->p,h1.p,h1b.p,final.p);
  accepted[k]=observe(graph,F,m,"e2e_BF16_final","matched_quantized_MKL_math",final.p,refb.p,nf,.01)&&accepted[k];
  printf("SEL_ACCEPT graph=%s shape=128_%d_%d method=%s accepted=%d\n",graph,F,F,m.name.c_str(),bool(accepted[k]));
 }
 auto call=[&](size_t k){if(k==methods.size()){mkl_one<128,F>(a,desc,h0.p,w1.p,z.p,mkl1.p,g.N);relu_f32(mkl1.p,nf);mkl_one<F,F>(a,desc,mkl1.p,w2.p,z.p,mkl2.p,g.N);}else {auto& m=methods[k];forward<F,true>(g,perm,m,pl[m.plan],hb.p,wb1.p,wb2.p,q1[k]->p,q2[k]->p,h1.p,h1b.p,final.p);}};
 for(size_t k=0;k<=methods.size();k++)if(k==methods.size()||accepted[k])call(k);
 for(int rep=0;rep<5;rep++)for(size_t pos=0;pos<=methods.size();pos++){size_t k=rep%2?methods.size()-pos:pos;if(k<methods.size()&&!accepted[k])continue;double t=omp_get_wtime();call(k);printf("SEL_TIME graph=%s shape=128_%d_%d method=%s repeat=%d order=%zu ms=%.12g kind=prepared_two_layer_cache_rebuilt\n",graph,F,F,k==methods.size()?"shape_mkl_fp32_original_style":methods[k].name.c_str(),rep,pos,(omp_get_wtime()-t)*1000);fflush(stdout);}
 for(size_t k=0;k<methods.size();k++)if(accepted[k]){
  auto& m=methods[k];auto& p=pl[m.plan];for(int rep=0;rep<3;rep++){
   double t=omp_get_wtime();if(m.kind&&!p.sources.empty())cache<128,F,false>(hb.p,wb1.p,q1[k]->p,p,nullptr);double c1=omp_get_wtime();
   if(m.kind==0)reduce<128,F,0,false,false>(g,perm,hb.p,wb1.p,p,q1[k]->p,h1.p,nullptr);else if(m.kind==1)reduce<128,F,1,false,false>(g,perm,hb.p,wb1.p,p,q1[k]->p,h1.p,nullptr);else if(m.kind==2)reduce<128,F,2,false,false>(g,perm,hb.p,wb1.p,p,q1[k]->p,h1.p,nullptr);else reduce<128,F,3,false,false>(g,perm,hb.p,wb1.p,p,q1[k]->p,h1.p,nullptr);double l1=omp_get_wtime();relu_f32(h1.p,nf);double rel=omp_get_wtime();convert_f32_to_bf16(h1.p,h1b.p,nf);double cv=omp_get_wtime();
   int k2=m.second<0?m.kind:m.second;if(k2&&!p.sources.empty())cache<F,F,false>(h1b.p,wb2.p,q2[k]->p,p,nullptr);double c2=omp_get_wtime();
   if(k2==0)reduce<F,F,0,false,true>(g,perm,h1b.p,wb2.p,p,q2[k]->p,final.p,nullptr);else if(k2==1)reduce<F,F,1,false,true>(g,perm,h1b.p,wb2.p,p,q2[k]->p,final.p,nullptr);else if(k2==2)reduce<F,F,2,false,true>(g,perm,h1b.p,wb2.p,p,q2[k]->p,final.p,nullptr);else reduce<F,F,3,false,true>(g,perm,h1b.p,wb2.p,p,q2[k]->p,final.p,nullptr);double end=omp_get_wtime();
   printf("SEL_STAGE graph=%s shape=128_%d_%d method=%s repeat=%d cache1_ms=%.12g reduce_project1_ms=%.12g relu_ms=%.12g interlayer_ms=%.12g cache2_ms=%.12g reduce_project2_ms=%.12g total_ms=%.12g\n",graph,F,F,m.name.c_str(),rep,(c1-t)*1000,(l1-c1)*1000,(rel-l1)*1000,(cv-rel)*1000,(c2-cv)*1000,(end-c2)*1000,(end-t)*1000);
  }
  for(int li=0;li<2;li++){
   forward<F,true>(g,perm,m,p,hb.p,wb1.p,wb2.p,q1[k]->p,q2[k]->p,h1.p,h1b.p,final.p);Stats s;bool eq;
   if(li==0){std::vector<float> expected(h1.p,h1.p+nf); // h1 is ReLU after forward; obtain raw layer reference first.
    layer<128,F,false,false>(g,perm,hb.p,wb1.p,p,q1[k]->p,h1.p,m.kind,nullptr);expected.assign(h1.p,h1.p+nf);layer<128,F,true,false>(g,perm,hb.p,wb1.p,p,q1[k]->p,h1.p,m.kind,&s);eq=memcmp(expected.data(),h1.p,nf*4)==0;
   }else{std::vector<uint16_t> expected(final.p,final.p+nf);layer<F,F,true,true>(g,perm,h1b.p,wb2.p,p,q2[k]->p,final.p,m.second<0?m.kind:m.second,&s);eq=memcmp(expected.data(),final.p,nf*2)==0;}
   printf("SEL_PROFILE_GATE graph=%s shape=128_%d_%d method=%s layer=%d bitwise=%d pass=%d\n",graph,F,F,m.name.c_str(),li+1,eq,eq);if(!eq)throw std::runtime_error("selector profile gate");
   for(int phase=0;phase<PHASES;phase++)printf("SEL_DETAIL graph=%s shape=128_%d_%d method=%s layer=%d phase=%s sampled_thread_ms=%.12g clocked_sections=%llu additive_wall=0\n",graph,F,F,m.name.c_str(),li+1,phase_names[phase],s.sec[phase]*1000,(unsigned long long)s.calls[phase]);
   for(auto& th:s.thread_rows)printf("SEL_THREAD graph=%s shape=128_%d_%d method=%s layer=%d stage=%s thread=%.0f instrumented_active_ms=%.12g edges=%.0f hot=%.0f cold=%.0f\n",graph,F,F,m.name.c_str(),li+1,th[0]==1?"cache":"reduce",th[1],th[2],th[3],th[4],th[5]);
   int d=li?F:128,kind=li&&m.second>=0?m.second:m.kind;uint64_t expected_cold=kind==0?g.nnz:(kind>=2?0:g.nnz-p.hot_edges),expected_hot=g.nnz-expected_cold;
   uint64_t expected_tdp=(s.cache_tiles+2*s.cold_tiles)*(d/32)*(F/16);
   bool work=s.edges==g.nnz&&s.cold==expected_cold&&s.hot==expected_hot&&s.tdp==expected_tdp;
   printf("SEL_WORK graph=%s shape=128_%d_%d method=%s layer=%d edges=%llu hot=%llu cold=%llu cold_tiles=%llu cache_tiles=%llu parts=%llu tdp=%llu cold_H_request_bytes=%llu hot_Q_request_bytes=%llu cache_storage_bytes=%llu pass=%d\n",graph,F,F,m.name.c_str(),li+1,(unsigned long long)s.edges,(unsigned long long)s.hot,(unsigned long long)s.cold,(unsigned long long)s.cold_tiles,(unsigned long long)s.cache_tiles,(unsigned long long)s.parts,(unsigned long long)s.tdp,(unsigned long long)(s.cold*2*d),(unsigned long long)(s.hot*4*F),(unsigned long long)(p.sources.size()*4*F),work);if(!work)throw std::runtime_error("selector work gate");
  }
  for(int rep=0;rep<3;rep++){call(k);gcn_extra_projection_pmu::Region pm;pm.start();call(k);auto result=pm.stop();std::string label="shape128_"+std::to_string(F)+"_"+m.name;gcn_extra_projection_pmu::print(result,graph,label.c_str(),rep);}
 }
 mkl_sparse_destroy(a);printf("SEL_COMPLETE graph=%s shape=128_%d_%d methods=9 rejected=%d pass=1\n",graph,F,F,int(std::count(accepted.begin(),accepted.end(),false)));fflush(stdout);
}
}
int main(int argc,char** argv){
 try{
  if(argc!=3)throw std::runtime_error("usage: selective_shape dir graph");char path[2048];snprintf(path,sizeof(path),"%s/%s/%s.csrbin",argv[1],argv[2],argv[2]);auto g=load_csrbin(path);
  for(size_t e=0;e<g.nnz;e++)if(g.values[e]!=1.f||g.indices[e]>=uint32_t(g.N))throw std::runtime_error("requires unit square LP64 CSR");
  int* perm=make_degree_perm(g.indptr,g.N);auto plans=selective::plans(g,perm,argv[2]);selective::run<32>(g,perm,plans,argv[2]);selective::run<128>(g,perm,plans,argv[2]);
  free(perm);free(g.indptr);free(g.indices);free(g.values);return 0;
 }catch(const std::exception& e){fprintf(stderr,"SELECTIVE_ERROR %s\n",e.what());return 2;}
}
