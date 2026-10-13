#define main original_source_main
#include "gcn_e2e_v3.cpp"
#undef main
#include "paper_methods_runtime.hpp"
#include "projection_window_kernels.hpp"
#include "projection_window_pmu.hpp"
#include "selective_kernels.hpp"
#include "order_grouping.hpp"
#include <memory>
#include <string>
#include <cmath>
namespace order_balance {
template<class T>struct Buffer{
 T* p;size_t n;explicit Buffer(size_t count):n(count){p=(T*)mkl_malloc(std::max(size_t(64),n*sizeof(T)),64);if(!p)throw std::bad_alloc();}
 ~Buffer(){mkl_free(p);}Buffer(const Buffer&)=delete;T& operator[](size_t i){return p[i];}
};
struct Choice{std::string name;int first,second,group;};
inline int kind(int plan){return plan==0?0:(plan==1?2:3);}
template<int D,int F,bool P,bool B16>
void reduce_only(const csr_t& g,const int* perm,const uint16_t* h,const uint16_t* w,const selective::Plan& p,const float* q,std::conditional_t<B16,uint16_t,float>* out,int plan,selective::Stats* s){
 if(plan==0)selective::reduce<D,F,0,P,B16>(g,perm,h,w,p,q,out,s);
 else if(plan==1)selective::reduce<D,F,2,P,B16>(g,perm,h,w,p,q,out,s);
 else selective::reduce<D,F,3,P,B16>(g,perm,h,w,p,q,out,s);
}
template<int F,bool B16>
void forward(const csr_t& g,const std::vector<Group>& gs,const Choice& m,const std::vector<selective::Plan>& plans,const uint16_t* h0,const uint16_t* w1,const uint16_t* w2,float* q1,float* q2,float* h1,uint16_t* h1b,std::conditional_t<B16,uint16_t,float>* out){
 const int* perm=gs[m.group].perm.data();
 selective::layer<128,F,false,false>(g,perm,h0,w1,plans[m.first],q1,h1,kind(m.first),nullptr);
 relu_f32(h1,size_t(g.N)*F);convert_f32_to_bf16(h1,h1b,size_t(g.N)*F);
 selective::layer<F,F,false,B16>(g,perm,h1b,w2,plans[m.second],q2,out,kind(m.second),nullptr);
}
template<class A,class B>bool observe(const char* graph,int f,const Choice& m,const char* boundary,const A* a,const B* b,size_t n,double gate){
 auto e=gcn_extra_paper::error(a,b,n);bool pass=e.finite&&e.l2<gate&&e.nmax<gate;
 printf("ORDER_CHECK graph=%s shape=128_%d_%d method=%s boundary=%s reference=matched_quantized_MKL_math max_abs=%.12g relative_L2=%.12g normalized_max=%.12g finite=%d gate=%.9g pass=%d\n",graph,f,f,m.name.c_str(),boundary,e.max_abs,e.l2,e.nmax,e.finite,gate,pass);fflush(stdout);return pass;
}
template<class T>bool equivalent(const char* graph,int f,const Choice& m,const char* boundary,const T* a,const T* b,size_t n,const char* ref="degree_same_dataflow"){
 bool bits=memcmp(a,b,n*sizeof(T))==0;printf("ORDER_EQUIV graph=%s shape=128_%d_%d method=%s boundary=%s reference=%s bitwise=%d pass=%d\n",graph,f,f,m.name.c_str(),boundary,ref,bits,bits);fflush(stdout);return bits;
}
template<int D,int F>void mkl_one(sparse_matrix_t a,matrix_descr desc,const float* h,const float* w,float* z,float* out,int n){
 auto status=mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE,1.f,a,desc,SPARSE_LAYOUT_ROW_MAJOR,h,D,D,0.f,z,D);
 if(status!=SPARSE_STATUS_SUCCESS)throw std::runtime_error("order MKL sparse status");
 cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,n,F,D,1.f,z,D,w,F,0.f,out,F);
}
template<int F>void run(csr_t& g,const std::vector<Group>& groups,const std::vector<selective::Plan>& plans,const char* graph){
 const size_t nd=size_t(g.N)*128,nf=size_t(g.N)*F;
 Buffer<float> h0(nd),w1(128*F),w2(F*F),h0q(nd),w1q(128*F),w2q(F*F),h1(nf),out(nf),ref1(nf),ref2(nf),mkl1(nf),mkl2(nf),z(nd),mkh1q(nf),q1(nf),q2(nf);
 Buffer<uint16_t> hb(nd),wb1(128*F),wb2(F*F),h1b(nf),final(nf),refb(nf);
 Buffer<float> degree_kernel(nf),degree_final(nf);Buffer<uint16_t> degree_bf16(nf);
 // Preserve P4 aligned input generator, including W1/W2 interleaving at F=128.
 srand(12345);for(size_t k=0;k<w1.n;k++){w1[k]=.01f*((rand()%200)-100);if(k<w2.n)w2[k]=.01f*((rand()%200)-100);}
 for(size_t k=0;k<nd;k++)h0[k]=.01f*((rand()%200)-100);
 convert_f32_to_bf16(h0.p,hb.p,nd);selective::pack<128,F>(w1.p,wb1.p);selective::pack<F,F>(w2.p,wb2.p);
 #pragma omp parallel for schedule(static)
 for(size_t k=0;k<nd;k++)h0q[k]=bf16_to_f32(hb[k]);
 for(size_t k=0;k<w1.n;k++)w1q[k]=bf16_to_f32(f32_to_bf16(w1[k]));
 for(size_t k=0;k<w2.n;k++)w2q[k]=bf16_to_f32(f32_to_bf16(w2[k]));
 std::vector<MKL_INT> rs(g.N),re(g.N),ci(g.nnz);for(int i=0;i<g.N;i++){rs[i]=g.indptr[i];re[i]=g.indptr[i+1];}for(size_t e=0;e<g.nnz;e++)ci[e]=g.indices[e];
 sparse_matrix_t a;matrix_descr desc{};desc.type=SPARSE_MATRIX_TYPE_GENERAL;
 if(mkl_sparse_s_create_csr(&a,SPARSE_INDEX_BASE_ZERO,g.N,g.N,rs.data(),re.data(),ci.data(),g.values)!=SPARSE_STATUS_SUCCESS)throw std::runtime_error("order MKL create");
 mkl_sparse_set_mm_hint(a,SPARSE_OPERATION_NON_TRANSPOSE,desc,SPARSE_LAYOUT_ROW_MAJOR,128,10);mkl_sparse_optimize(a);
 // Numerical reference only: BF16-decoded H/W and the native interlayer BF16 boundary.
 mkl_one<128,F>(a,desc,h0q.p,w1q.p,z.p,ref1.p,g.N);memcpy(mkl1.p,ref1.p,nf*4);relu_f32(mkl1.p,nf);convert_f32_to_bf16(mkl1.p,h1b.p,nf);
 #pragma omp parallel for schedule(static)
 for(size_t k=0;k<nf;k++)mkh1q[k]=bf16_to_f32(h1b[k]);
 mkl_one<F,F>(a,desc,mkh1q.p,w2q.p,z.p,ref2.p,g.N);convert_f32_to_bf16(ref2.p,refb.p,nf);
 std::vector<Choice> methods;const char* names[]={"A","Pall","Pused"};
 for(int x=0;x<3;x++)for(int y=0;y<3;y++)for(int s=0;s<2;s++)methods.push_back({std::string(names[x])+"_"+names[y]+"_"+groups[s].name,x,y,s});
 printf("ORDER_CONFIG graph=%s shape=128_%d_%d N=%d E=%u threads=%d seed=12345 TR=16 R=64 S=64 aggregate=FULL_ACCURATE projected_cache=FP32 final=BF16 methods=18 warmups=1 consecutive=5 paired_rounds=10 stages=2 pmu_repeats=1 numa=default_no_interleave shared_Q_buffers=2 input_generator=source_discrete_aligned\n",graph,F,F,g.N,g.nnz,omp_get_max_threads());fflush(stdout);
 std::vector<bool> accepted(methods.size(),true);
 for(size_t k=0;k<methods.size();k++){
  const auto& m=methods[k];const int* perm=groups[m.group].perm.data();const auto& p=plans[m.first];bool eq=true;
  std::fill(h1.p,h1.p+nf,NAN);selective::layer<128,F,false,false>(g,perm,hb.p,wb1.p,p,q1.p,h1.p,kind(m.first),nullptr);
  accepted[k]=observe(graph,F,m,"kernel_FP32",h1.p,ref1.p,nf,.001);
  if(m.group==0)memcpy(degree_kernel.p,h1.p,nf*4);else eq=equivalent(graph,F,m,"kernel_FP32",h1.p,degree_kernel.p,nf)&&eq;
  if constexpr(F==128){if(k==0){gcn_extra_paper::SourceGraphView view{g.N,g.nnz,{g.indptr},{g.indices}};gcn_extra_projection::Method frozen{"s64_mfull_accurate",64,0,true,true};gcn_extra_projection::kernel<false,false>(view,hb.p,wb1.p,out.p,perm,frozen,nullptr);eq=equivalent(graph,F,m,"kernel_FP32",h1.p,out.p,nf,"frozen_P0_FULL_ACCURATE")&&eq;}}
  forward<F,false>(g,groups,m,plans,hb.p,wb1.p,wb2.p,q1.p,q2.p,h1.p,h1b.p,out.p);
  accepted[k]=observe(graph,F,m,"e2e_FP32_final",out.p,ref2.p,nf,.001)&&accepted[k];
  if(m.group==0)memcpy(degree_final.p,out.p,nf*4);else eq=equivalent(graph,F,m,"e2e_FP32_final",out.p,degree_final.p,nf)&&eq;
  forward<F,true>(g,groups,m,plans,hb.p,wb1.p,wb2.p,q1.p,q2.p,h1.p,h1b.p,final.p);
  accepted[k]=observe(graph,F,m,"e2e_BF16_final",final.p,refb.p,nf,.01)&&accepted[k];
  if(m.group==0)memcpy(degree_bf16.p,final.p,nf*2);else eq=equivalent(graph,F,m,"e2e_BF16_final",final.p,degree_bf16.p,nf)&&eq;
  accepted[k]=accepted[k]&&eq;
  printf("ORDER_ACCEPT graph=%s shape=128_%d_%d method=%s first=%s second=%s grouping=%s accepted=%d numerical_and_grouping_gate=1\n",graph,F,F,m.name.c_str(),names[m.first],names[m.second],groups[m.group].name.c_str(),bool(accepted[k]));fflush(stdout);
 }
 const size_t anchor=methods.size();
 auto label=[&](size_t k)->const char*{return k==anchor?"shape_mkl_fp32_original_style":methods[k].name.c_str();};
 auto call=[&](size_t k){
  if(k==anchor){mkl_one<128,F>(a,desc,h0.p,w1.p,z.p,mkl1.p,g.N);relu_f32(mkl1.p,nf);mkl_one<F,F>(a,desc,mkl1.p,w2.p,z.p,mkl2.p,g.N);}
  else forward<F,true>(g,groups,methods[k],plans,hb.p,wb1.p,wb2.p,q1.p,q2.p,h1.p,h1b.p,final.p);
 };
 auto timed=[&](size_t k,int rep,size_t pos,const char* protocol){
  double begin=omp_get_wtime();call(k);double ms=(omp_get_wtime()-begin)*1000;
  printf("ORDER_TIME graph=%s shape=128_%d_%d method=%s repeat=%d pair=%d order=%zu kind=%s ms=%.12g boundary=prepared_forward_Q_rebuilt anchor_precision_mismatch=%d\n",graph,F,F,label(k),rep,rep/2,pos,protocol,ms,k==anchor);fflush(stdout);
 };
 // Five consecutive forwards per method. Rewarm each method before its block.
 for(size_t k=0;k<=anchor;k++)if(k==anchor||accepted[k]){call(k);for(int rep=0;rep<5;rep++)timed(k,rep,k,"consecutive");}
 // Ten rounds = five forward/reverse pairs. No samples are removed.
 for(size_t k=0;k<=anchor;k++)if(k==anchor||accepted[k])call(k);
 for(int rep=0;rep<10;rep++)for(size_t pos=0;pos<=anchor;pos++){size_t k=rep%2?anchor-pos:pos;if(k<anchor&&!accepted[k])continue;timed(k,rep,pos,"paired");}
 for(size_t k=0;k<methods.size();k++)if(accepted[k]){
  const auto& m=methods[k];const int* perm=groups[m.group].perm.data();const auto& p1=plans[m.first];const auto& p2=plans[m.second];
  for(int rep=0;rep<2;rep++){
   double t=omp_get_wtime();if(m.first&&!p1.sources.empty())selective::cache<128,F,false>(hb.p,wb1.p,q1.p,p1,nullptr);double c1=omp_get_wtime();
   reduce_only<128,F,false,false>(g,perm,hb.p,wb1.p,p1,q1.p,h1.p,m.first,nullptr);double l1=omp_get_wtime();
   relu_f32(h1.p,nf);double rel=omp_get_wtime();convert_f32_to_bf16(h1.p,h1b.p,nf);double cv=omp_get_wtime();
   if(m.second&&!p2.sources.empty())selective::cache<F,F,false>(h1b.p,wb2.p,q2.p,p2,nullptr);double c2=omp_get_wtime();
   reduce_only<F,F,false,true>(g,perm,h1b.p,wb2.p,p2,q2.p,final.p,m.second,nullptr);double end=omp_get_wtime();
   printf("ORDER_STAGE graph=%s shape=128_%d_%d method=%s repeat=%d cache1_ms=%.12g reduce_project1_ms=%.12g relu_ms=%.12g interlayer_ms=%.12g cache2_ms=%.12g reduce_project2_ms=%.12g total_ms=%.12g kind=independent_stage_not_main\n",graph,F,F,m.name.c_str(),rep,(c1-t)*1000,(l1-c1)*1000,(rel-l1)*1000,(cv-rel)*1000,(c2-cv)*1000,(end-c2)*1000,(end-t)*1000);
  }
  for(int li=0;li<2;li++){
   forward<F,true>(g,groups,m,plans,hb.p,wb1.p,wb2.p,q1.p,q2.p,h1.p,h1b.p,final.p);
   selective::Stats s;bool eq;
   if(li==0){
    selective::layer<128,F,false,false>(g,perm,hb.p,wb1.p,p1,q1.p,h1.p,kind(m.first),nullptr);memcpy(degree_kernel.p,h1.p,nf*4);
    selective::layer<128,F,true,false>(g,perm,hb.p,wb1.p,p1,q1.p,h1.p,kind(m.first),&s);eq=memcmp(h1.p,degree_kernel.p,nf*4)==0;
   }else{
    memcpy(degree_bf16.p,final.p,nf*2);selective::layer<F,F,true,true>(g,perm,h1b.p,wb2.p,p2,q2.p,final.p,kind(m.second),&s);eq=memcmp(final.p,degree_bf16.p,nf*2)==0;
   }
   printf("ORDER_PROFILE_GATE graph=%s shape=128_%d_%d method=%s layer=%d bitwise=%d pass=%d\n",graph,F,F,m.name.c_str(),li+1,eq,eq);if(!eq)throw std::runtime_error("order instrumented output mismatch");
   int d=li?F:128,plan=li?m.second:m.first;const auto& p=plans[plan];
   uint64_t expected_cache=plan?(p.sources.size()+15)/16:0,expected_cold=plan?0:groups[m.group].full_tiles;
   uint64_t expected_parts=expected_cache+2*expected_cold,expected_tdp=expected_parts*(d/32)*(F/16);
   bool work=s.edges==g.nnz&&s.cold==(plan?0:g.nnz)&&s.hot==(plan?g.nnz:0)&&s.cold_tiles==expected_cold&&s.cache_tiles==expected_cache&&s.parts==expected_parts&&s.tdp==expected_tdp;
   uint64_t adds=s.cold*d+s.hot*F,raw_bytes=s.cold*2*d+s.hot*4*F;
   printf("ORDER_WORK graph=%s shape=128_%d_%d method=%s layer=%d D=%d F=%d dataflow=%s edges=%llu hot=%llu cold=%llu cold_tiles=%llu cache_tiles=%llu parts=%llu tdp=%llu expected_tdp=%llu sparse_scalar_adds=%llu sparse_vector_adds=%llu sparse_feature_request_bytes=%llu bf16_decode_vectors=%llu map_lookups=%llu cache_H_request_bytes=%llu cache_Q_write_bytes=%llu cache_storage_bytes=%llu logical_requests_not_DRAM=1 pass=%d\n",graph,F,F,m.name.c_str(),li+1,d,F,names[plan],(unsigned long long)s.edges,(unsigned long long)s.hot,(unsigned long long)s.cold,(unsigned long long)s.cold_tiles,(unsigned long long)s.cache_tiles,(unsigned long long)s.parts,(unsigned long long)s.tdp,(unsigned long long)expected_tdp,(unsigned long long)adds,(unsigned long long)(adds/16),(unsigned long long)raw_bytes,(unsigned long long)(s.cold*d/16),(unsigned long long)(plan==2?g.nnz:0),(unsigned long long)(p.sources.size()*2*d),(unsigned long long)(p.sources.size()*4*F),(unsigned long long)(p.sources.size()*4*F),work);
   if(!work)throw std::runtime_error("order work count mismatch");
   for(int ph=0;ph<selective::PHASES;ph++)printf("ORDER_DETAIL graph=%s shape=128_%d_%d method=%s layer=%d phase=%s sampled_thread_ms=%.12g clocked_sections=%llu additive_wall=0\n",graph,F,F,m.name.c_str(),li+1,selective::phase_names[ph],s.sec[ph]*1000,(unsigned long long)s.calls[ph]);
   for(auto& th:s.thread_rows)printf("ORDER_THREAD graph=%s shape=128_%d_%d method=%s layer=%d stage=%s thread=%.0f instrumented_active_ms=%.12g edges=%.0f hot=%.0f cold=%.0f\n",graph,F,F,m.name.c_str(),li+1,th[0]==1?"cache":"reduce",th[1],th[2],th[3],th[4],th[5]);
  }
  call(k);gcn_extra_projection_pmu::Region pm;double pstart=omp_get_wtime();pm.start();call(k);auto result=pm.stop();
  std::string pm_label="shape128_"+std::to_string(F)+"_"+m.name;gcn_extra_projection_pmu::print(result,graph,pm_label.c_str(),0);
  printf("ORDER_PMU_WALL graph=%s shape=128_%d_%d method=%s repeat=0 ms=%.12g kind=independent_diagnostic_not_main\n",graph,F,F,m.name.c_str(),(omp_get_wtime()-pstart)*1000);
 }
 mkl_sparse_destroy(a);int rejected=int(std::count(accepted.begin(),accepted.end(),false));
 printf("ORDER_COMPLETE graph=%s shape=128_%d_%d methods=18 rejected=%d numerical_records=54 grouping_records=27 all_candidates_accepted=%d execution_pass=1\n",graph,F,F,rejected,rejected==0);fflush(stdout);
}
}
int main(int argc,char** argv){
 try{
  if(argc!=3)throw std::runtime_error("usage: selective_shape dataset_root graph");
  char path[2048];snprintf(path,sizeof(path),"%s/%s/%s.csrbin",argv[1],argv[2],argv[2]);auto g=load_csrbin(path);
  if(g.N<=0||g.nnz>2147483647u)throw std::runtime_error("order requires nonempty square LP64 CSR");
  for(size_t e=0;e<g.nnz;e++)if(g.values[e]!=1.f||g.indices[e]>=uint32_t(g.N))throw std::runtime_error("order requires unit square LP64 CSR");
  int* degree=make_degree_perm(g.indptr,g.N);auto gs=order_balance::groups(g,degree,argv[2]);auto ps=order_balance::source_plans(g,argv[2]);
  order_balance::run<32>(g,gs,ps,argv[2]);order_balance::run<128>(g,gs,ps,argv[2]);
  free(degree);free(g.indptr);free(g.indices);free(g.values);return 0;
 }catch(const std::exception& e){fprintf(stderr,"ORDER_ERROR %s\n",e.what());return 2;}
}
