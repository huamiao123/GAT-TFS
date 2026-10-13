#pragma once
#include "paper_methods_runtime.hpp"
#include "projection_window_kernels.hpp"
#include "projection_window_pmu.hpp"
#include "grouping_schedule.hpp"
#include "state_kernels.hpp"
#include "pair_reducer.hpp"
#include "pair_plan.hpp"
#include "planned_kernel.hpp"
#include <cstdlib>
#include <array>
namespace gcn_extra_pair_plan_runtime {
using gcn_extra_paper::DIM;
using gcn_extra_paper::SourceGraphView;
using Plan=gcn_extra_pair_plan::Plan;
struct Method { std::string name; int kind=4,scope=0,schedule=0,pair=0,plan_index=-1,mode=0; bool accurate=false; };
inline std::vector<Method> candidates(bool full) {
 std::vector<Method>a{{"paper_tfs",0,1},{"pack_epi_degree",3,0,0,0},{"legacy_s_degree",3,0,0,2}};
 const char* modes[]={"scalar","dyn_i","dyn_s","exact_i","exact_s"};
 auto add=[&](const char* prefix,int pi,int schedule,const std::vector<int>& mm){for(int mode:mm)a.push_back({std::string(prefix)+"_"+modes[mode]+(schedule?"_qguard":"_degree"),4,0,schedule,0,pi,mode});};
 if(full){a.insert(a.begin()+2,{"legacy_i_degree",3,0,0,1});add("adj",0,0,{0,1,2,3,4});add("opt",1,0,{0,1,2,3,4});add("rand",2,0,{0,2,4});}
 else {add("adj",0,0,{4});add("opt",1,0,{1,2,3,4});}
 a.push_back({"pack_epi_qguard",3,0,4,0});add("opt",3,4,full?std::vector<int>{0,1,2,3,4}:std::vector<int>{1,2,3,4});return a;
}
template<bool P,bool B16> void state_layer(const SourceGraphView&g,const Method&m,const uint16_t*h,const uint16_t*w,std::conditional_t<B16,uint16_t,float>*out,const int*perm,bool relu,gcn_extra_state::Profile*pr){
 gcn_extra_state::Method c{m.name,64,m.scope,m.accurate,true,true,relu};
 if(m.pair==1)gcn_extra_state::kernel<P,B16,gcn_extra_state::PairReducer<false>>(g,h,w,out,perm,c,pr);
 else if(m.pair==2)gcn_extra_state::kernel<P,B16,gcn_extra_state::PairReducer<true>>(g,h,w,out,perm,c,pr);
 else gcn_extra_state::kernel<P,B16>(g,h,w,out,perm,c,pr);
}
template<bool P,bool B16> void planned_layer(const SourceGraphView&g,const Method&m,const uint16_t*h,const uint16_t*w,std::conditional_t<B16,uint16_t,float>*out,const int*perm,bool relu,const Plan&plan,gcn_extra_pair_plan::Profile*pr){
 gcn_extra_state::Method c{m.name,64,m.scope,m.accurate,true,true,relu};
 switch(m.mode){
#define PLAN_CASE(M) case M:gcn_extra_pair_plan::kernel<P,B16,M>(g,h,w,out,perm,c,plan,pr);break
 PLAN_CASE(0);PLAN_CASE(1);PLAN_CASE(2);PLAN_CASE(3);PLAN_CASE(4);
#undef PLAN_CASE
 default:throw std::runtime_error("Invalid plan kernel mode");}
}
template<bool B16> void layer(const SourceGraphView&g,const Method&m,const uint16_t*h,const uint16_t*w,std::conditional_t<B16,uint16_t,float>*out,const int*perm,bool relu=false,const Plan*plan=nullptr){
 if(m.kind==0){if constexpr(B16)::tfs_v3_bf16out(g.row.p,g.col.p,h,w,out,perm,g.n,64);else ::tfs_v3_fp32out(g.row.p,g.col.p,h,w,out,perm,g.n,64);}
 else if(m.kind==1){gcn_extra_paper::Method c{m.name,m.scope,m.accurate,false,0,true};gcn_extra_paper::block_kernel<false,B16>(g,h,w,out,perm,c,nullptr);}
 else if(m.kind==3)state_layer<false,B16>(g,m,h,w,out,perm,relu,nullptr);
 else {if(!plan)throw std::runtime_error("Missing plan");planned_layer<false,B16>(g,m,h,w,out,perm,relu,*plan,nullptr);}
}
template<bool B16> void forward(const SourceGraphView&g,const Method&m,const uint16_t*h0,const uint16_t*w1,const uint16_t*w2,float*h1,uint16_t*h1b,std::conditional_t<B16,uint16_t,float>*out,const int*perm,const Plan*plan=nullptr){
 if(m.kind>=3)layer<true>(g,m,h0,w1,h1b,perm,true,plan);
 else {layer<false>(g,m,h0,w1,h1,perm);::relu_f32(h1,size_t(g.n)*DIM);::convert_f32_to_bf16(h1,h1b,size_t(g.n)*DIM);}
 layer<B16>(g,m,h1b,w2,out,perm,false,plan);
}
template<class T> void equal(const char*graph,const Method&m,const char*boundary,const T*got,const T*ref,size_t n){bool bits=std::memcmp(got,ref,n*sizeof(T))==0;printf("PAIRPLAN_EQUIV graph=%s method=%s boundary=%s scope=%d accurate=%d bitwise=%d pass=%d\n",graph,m.name.c_str(),boundary,m.scope,int(m.accurate),bits,bits);fflush(stdout);if(!bits)throw std::runtime_error("Pair plan equality: "+m.name+"/"+boundary);}
inline void equivalence(const SourceGraphView&g,const Method&m,const uint16_t*h0,const uint16_t*w1,const uint16_t*w2,float*h1,uint16_t*h1b,float*h2f,uint16_t*h2b,const int*perm,const int*degree,const char*graph,const Plan*plan){
 size_t n=size_t(g.n)*DIM;Method ref=m;ref.kind=1;ref.schedule=0;ref.pair=0;
 {std::fill(h1,h1+n,std::numeric_limits<float>::quiet_NaN());layer<false>(g,m,h0,w1,h1,perm,false,plan);std::vector<float>v(h1,h1+n);layer<false>(g,ref,h0,w1,h1,degree);equal(graph,m,"kernel_FP32",v.data(),h1,n);}
 {std::fill(h2f,h2f+n,std::numeric_limits<float>::quiet_NaN());forward<false>(g,m,h0,w1,w2,h1,h1b,h2f,perm,plan);std::vector<float>v(h2f,h2f+n);forward<false>(g,ref,h0,w1,w2,h1,h1b,h2f,degree);equal(graph,m,"e2e_FP32",v.data(),h2f,n);}
 {std::fill(h1b,h1b+n,uint16_t(0x7fc0));std::fill(h2b,h2b+n,uint16_t(0x7fc0));forward<true>(g,m,h0,w1,w2,h1,h1b,h2b,perm,plan);std::vector<uint16_t>v(h2b,h2b+n),u(h1b,h1b+n);forward<true>(g,ref,h0,w1,w2,h1,h1b,h2b,degree);equal(graph,m,"H1_BF16",u.data(),h1b,n);equal(graph,m,"e2e_BF16",v.data(),h2b,n);}
}
struct Work {uint64_t edges=0,scopes=0,windows=0,loads=0,stores=0,packed=0,direct=0,prefetch=0;};
inline Work expected_work(const SourceGraphView&g,const int*perm,int scope,bool accurate){
 Work x;for(int i=0;i<g.n;i+=16){int batch=std::min(16,g.n-i);uint32_t d[16]{},mx=0;for(int r=0;r<batch;r++){int row=perm[i+r];d[r]=g.row[row+1]-g.row[row];mx=std::max(mx,d[r]);}
 if(!mx)continue;uint32_t span=scope?scope:mx;for(uint32_t st=0;st<mx;){uint32_t en=st+std::min(span,mx-st);x.scopes++;x.packed+=16*128*2*(accurate?2:1);
 for(uint32_t ws=st;ws<en;){uint32_t we=ws+std::min(64u,en-ws);x.windows++;for(int r=0;r<batch;r++){uint32_t re=std::min(d[r],en),end=std::min(re,we);if(ws!=st&&ws>=end)continue;if(ws!=st)x.loads+=512;if(end==re)x.direct++;else x.stores+=512;if(end>ws){uint64_t e=end-ws;x.edges+=e;x.prefetch+=(e-(end==re?1:0))*4;}}ws=we;}st=en;}}
 if(x.edges!=g.e)throw std::runtime_error("Independent work does not cover graph");return x;
}
inline void profile_method(const SourceGraphView&g,const Method&m,const uint16_t*h0,const uint16_t*w1,const uint16_t*w2,float*h1,uint16_t*h1b,uint16_t*h2b,const int*perm,const char*graph,const Plan*plan,const Plan&adj){
 size_t n=size_t(g.n)*DIM;forward<true>(g,m,h0,w1,w2,h1,h1b,h2b,perm,plan);std::vector<uint16_t>ref1(h1b,h1b+n),ref2(h2b,h2b+n);Work e=expected_work(g,perm,m.scope,m.accurate);
 for(int li=0;li<2;li++){gcn_extra_pair_plan::Profile pr;const uint16_t*in=li==0?h0:h1b;const uint16_t*w=li==0?w1:w2;uint16_t*out=li==0?h1b:h2b;
 if(m.kind==4)planned_layer<true,true>(g,m,in,w,out,perm,li==0,*plan,&pr);else state_layer<true,true>(g,m,in,w,out,perm,li==0,&pr);
 const auto&c=pr.counters;const auto&q=pr.logical;const auto&t=pr.plan_logical;
 uint64_t hits=0,expected_decode=g.e,expected_csr=g.e;
 if(m.kind==4){if(m.mode==1||m.mode==2)hits=plan->dynamic_shared_heads;if(m.mode==3||m.mode==4)hits=plan->exact_shared_edges;if(m.mode==2||m.mode==4)expected_decode-=hits;if(m.mode==4)expected_csr-=hits;}
 else if(m.pair){hits=adj.dynamic_shared_heads;if(m.pair==2)expected_decode-=hits;}
 bool work=c.source_visits==g.e&&c.feature_adds==g.e*128&&c.projection_scopes==e.scopes&&c.neighbor_windows==e.windows&&c.amx_dpbf16ps==e.scopes*32*(m.accurate?2:1)&&q.decoded_source_rows==expected_decode&&q.csr_index_reads==expected_csr&&q.pair_shared_sources==hits&&c.csr_prefetch_instructions==e.prefetch&&c.partial_zero_bytes==0&&c.window_state_load_bytes==e.loads&&c.window_state_store_bytes==e.stores&&c.partial_packed_bytes==e.packed&&q.direct_pack_rows==e.direct&&q.packing_partial_load_bytes==0;
 work=work&&q.prefetch_index_reads*4==e.prefetch&&c.source_decode_bytes==expected_decode*128*2;
 if(m.kind==4){uint64_t orders=0,flags=0;for(size_t ti=0;ti<plan->tiles.size();ti++){int batch=std::min(16,g.n-int(ti)*16);orders+=uint64_t(plan->tiles[ti].windows)*batch;flags+=uint64_t(plan->tiles[ti].windows)*(batch/2);}work=work&&t.order_index_reads==orders;
 if(m.mode==1||m.mode==2)work=work&&t.runtime_head_comparisons==plan->comparisons&&q.pair_comparisons==plan->comparisons;
 else work=work&&t.runtime_head_comparisons==0&&q.pair_comparisons==0;
 if(m.mode>=3)work=work&&t.exact_pair_edges==plan->exact_shared_edges&&t.exact_pair_windows==plan->exact_windows&&t.exact_shared_edges==(m.mode==4?plan->exact_shared_edges:0)&&t.exact_mask_bytes_read==e.windows&&t.exact_pair_flags_tested==flags;
 else work=work&&t.exact_pair_edges==0&&t.exact_pair_windows==0&&t.exact_shared_edges==0&&t.exact_mask_bytes_read==0&&t.exact_pair_flags_tested==0;
 } else if(m.pair)work=work&&q.pair_comparisons==adj.comparisons;
 printf("PAIRPLAN_COUNTERS graph=%s method=%s layer=%d scope=%d accurate=%d profile_path=production_BF16_epilogue unit=source_level_not_DRAM source_visits=%llu feature_adds=%llu decode_rows=%llu shared_heads=%llu projection_scopes=%llu windows=%llu amx_tdp=%llu partial_zero_bytes=%llu partial_load_bytes=%llu partial_store_bytes=%llu packing_load_bytes=%llu direct_rows=%llu packed_bytes=%llu prefetch_requests=%llu prefetch_index_reads=%llu csr_reads=%llu comparisons=%llu order_index_reads=%llu exact_mask_bytes_read=%llu exact_pair_flags_tested=%llu exact_pair_windows=%llu exact_pair_edges=%llu exact_shared_edges=%llu runtime_head_comparisons=%llu relu_elements=%llu expected_hits=%llu expected_decode=%llu expected_csr=%llu expected_scopes=%llu expected_windows=%llu expected_loads=%llu expected_stores=%llu expected_packed=%llu expected_direct=%llu expected_prefetch=%llu\n",graph,m.name.c_str(),li+1,m.scope,int(m.accurate),(unsigned long long)c.source_visits,(unsigned long long)c.feature_adds,(unsigned long long)q.decoded_source_rows,(unsigned long long)q.pair_shared_sources,(unsigned long long)c.projection_scopes,(unsigned long long)c.neighbor_windows,(unsigned long long)c.amx_dpbf16ps,(unsigned long long)c.partial_zero_bytes,(unsigned long long)c.window_state_load_bytes,(unsigned long long)c.window_state_store_bytes,(unsigned long long)q.packing_partial_load_bytes,(unsigned long long)q.direct_pack_rows,(unsigned long long)c.partial_packed_bytes,(unsigned long long)c.csr_prefetch_instructions,(unsigned long long)q.prefetch_index_reads,(unsigned long long)q.csr_index_reads,(unsigned long long)q.pair_comparisons,(unsigned long long)t.order_index_reads,(unsigned long long)t.exact_mask_bytes_read,(unsigned long long)t.exact_pair_flags_tested,(unsigned long long)t.exact_pair_windows,(unsigned long long)t.exact_pair_edges,(unsigned long long)t.exact_shared_edges,(unsigned long long)t.runtime_head_comparisons,(unsigned long long)q.relu_elements,(unsigned long long)hits,(unsigned long long)expected_decode,(unsigned long long)expected_csr,(unsigned long long)e.scopes,(unsigned long long)e.windows,(unsigned long long)e.loads,(unsigned long long)e.stores,(unsigned long long)e.packed,(unsigned long long)e.direct,(unsigned long long)e.prefetch);
 printf("PAIRPLAN_WORK_GATE graph=%s method=%s layer=%d pass=%d\n",graph,m.name.c_str(),li+1,work);fflush(stdout);if(!work)throw std::runtime_error("Pair plan work mismatch: "+m.name);
 bool bits=memcmp(out,li==0?ref1.data():ref2.data(),n*2)==0;printf("PAIRPLAN_PROFILE_GATE graph=%s method=%s layer=%d bitwise=%d pass=%d\n",graph,m.name.c_str(),li+1,bits,bits);fflush(stdout);if(!bits)throw std::runtime_error("Pair plan instrumentation changes output");
 }
}
__attribute__((noinline)) inline void semantic_runtime_scatter(uint16_t* got,const float* input,int count){
    gcn_extra_state::Method m{"semantic",64,0,false,false,false,true};gcn_extra_state::Profile pr;
    gcn_extra_state::scatter_state<false,true>(got,input,count,m,pr,false);
}
inline void semantic_gate(const char* graph){
    const uint32_t bits[]={0u,0x80000000u,0x3f800000u,0xbf800000u,0x7fc12345u,0xffc12345u,0x7f800000u,0xff800000u,1u,0x80000001u,0x3f807fffu,0x3f808000u,0xbf808000u,0x00010000u,0x80010000u,0x7fa12345u,0xffa12345u,0x3eabcdefu,0xbeabcdefu};
    alignas(64) float input[19],ref[19];alignas(64) uint16_t got[19],expected[19];
    memcpy(input,bits,sizeof(bits));memcpy(ref,bits,sizeof(bits));::relu_f32(ref,19);::convert_f32_to_bf16(ref,expected,19);
    gcn_extra_state::Method m{"semantic",64,0,false,false,false,true};gcn_extra_state::Profile pr;
    semantic_runtime_scatter(got,input,19);
    bool pass=memcmp(got,expected,sizeof(got))==0; if(!pass)for(int i=0;i<19;i++)printf("PAIRPLAN_SEMANTIC_DIFF graph=%s index=%d input=%08x got=%04x expected=%04x mxcsr=%08x\n",graph,i,bits[i],unsigned(got[i]),unsigned(expected[i]),_mm_getcsr());printf("PAIRPLAN_SEMANTIC graph=%s values=19 bitwise=%d pass=%d kind=relu_truncation_signedzero_nan_vector_tail\n",graph,pass,pass);fflush(stdout);if(!pass)throw std::runtime_error("ReLU conversion semantics changed");
}
inline void run_methods(const csr_t&csr,const int*perm,const float*h0,const float*w1,const float*w2,const uint16_t*wv1,const uint16_t*wv2,const uint16_t*h0b,float*h1,uint16_t*h1b,uint16_t*h2b,float*h2f,sparse_matrix_t a,matrix_descr descr,float*z,float*mh1,float*mh2,const char*graph){
 SourceGraphView g{csr.N,csr.nnz,{csr.indptr},{csr.indices}};size_t n=size_t(g.n)*DIM;semantic_gate(graph);for(uint64_t e=0;e<g.e;e++)if(csr.values[e]!=1.f)throw std::runtime_error("Nonunit source graph");
 bool smoke=getenv("PAIRPLAN_SMOKE")&&std::string(getenv("PAIRPLAN_SMOKE"))=="1";bool full=smoke||!getenv("PAIRPLAN_ABLATIONS")||std::string(getenv("PAIRPLAN_ABLATIONS"))=="1";
 double ts=omp_get_wtime();auto schedules=gcn_extra_grouping::prepare(g,perm,graph);double setup=(omp_get_wtime()-ts)*1000;
 if(schedules.size()!=5||schedules[0].name!="degree"||schedules[4].name!="qguard"||memcmp(schedules[0].perm.data(),perm,size_t(g.n)*sizeof(int)))throw std::runtime_error("Frozen schedule mismatch");
 printf("PAIRPLAN_SCHEDULE_SETUP graph=%s phase=all_five_frozen_schedules ms=%.12g included_in_primary=0\n",graph,setup);
 std::array<Plan,4>plans;plans[0]=gcn_extra_pair_plan::prepare_plan(g,perm,"adj_degree",0,graph);plans[1]=gcn_extra_pair_plan::prepare_plan(g,perm,"opt_degree",1,graph);if(full)plans[2]=gcn_extra_pair_plan::prepare_plan(g,perm,"rand_degree",2,graph);plans[3]=gcn_extra_pair_plan::prepare_plan(g,schedules[4].perm.data(),"opt_qguard",1,graph);
 if(smoke){std::string gn(graph);bool require_hit=gn=="plan_exact_tail"||gn=="plan_exact_unsorteddup",require_zero=gn=="plan_exact_mismatch";bool pass=(!require_hit||plans[0].exact_shared_edges>0)&&(!require_zero||plans[0].exact_shared_edges==0);printf("PAIRPLAN_COVERAGE graph=%s required=%s adj_exact_shared_edges=%llu pass=%d\n",graph,require_hit?"positive":(require_zero?"zero":"unconstrained"),(unsigned long long)plans[0].exact_shared_edges,pass);if(!pass)throw std::runtime_error("Exact fixture path coverage missing");}
 auto methods=candidates(full);printf("PAIRPLAN_CONFIG graph=%s N=%d E=%u D=128 H=128 F=128 S=64 R=64 TR=16 methods=%zu warmups=1 consecutive=5 alternating=10 numa=default smoke=%d ablations=%d mkl_threads=%d mkl_dynamic=%d final_output=BF16\n",graph,g.n,csr.nnz,methods.size(),smoke,full,mkl_get_max_threads(),mkl_get_dynamic());
 for(auto&m:methods)printf("PAIRPLAN_PLAN graph=%s method=%s kind=%d scope=%d schedule=%s pair=%d plan_index=%d plan=%s mode=%d accurate=%d epilogue=%d\n",graph,m.name.c_str(),m.kind,m.scope,schedules[m.schedule].name.c_str(),m.pair,m.plan_index,m.plan_index<0?"none":plans[m.plan_index].name.c_str(),m.mode,m.accurate,m.kind>=3);
 auto numeric=[&](const Method&m,const float*kr,const float*fr,const uint16_t*br){const int*p=schedules[m.schedule].perm.data();const Plan*pl=m.plan_index<0?nullptr:&plans[m.plan_index];bool exact=m.kind==0;layer<false>(g,m,h0b,wv1,h1,p,false,pl);gcn_extra_paper::check(graph,m.name.c_str(),"kernel_FP32","paper_tfs",h1,kr,n,m.accurate?.001:.01,exact);forward<false>(g,m,h0b,wv1,wv2,h1,h1b,h2f,p,pl);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_FP32_final","paper_tfs",h2f,fr,n,m.accurate?.001:.01,exact);forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,p,pl);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","paper_tfs",h2b,br,n,m.accurate?.01:.02,exact);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","source_MKL_FP32",h2b,mh2,n,.03);};
 {std::vector<float>fr(h2f,h2f+n);std::vector<uint16_t>br(h2b,h2b+n);layer<false>(g,methods[0],h0b,wv1,h1,perm);std::vector<float>kr(h1,h1+n);
 for(auto&m:methods){const int*p=schedules[m.schedule].perm.data();const Plan*pl=m.plan_index<0?nullptr:&plans[m.plan_index];numeric(m,kr.data(),fr.data(),br.data());if(m.kind>=3){equivalence(g,m,h0b,wv1,wv2,h1,h1b,h2f,h2b,p,perm,graph,pl);profile_method(g,m,h0b,wv1,wv2,h1,h1b,h2b,p,graph,pl,plans[0]);}}
 if(smoke)for(bool ac:{false,true})for(int sc:{64,256,0})for(int pi=0;pi<3;pi++)for(int md=0;md<5;md++){
 Method m{"smoke_m"+std::to_string(sc)+"_a"+std::to_string(ac)+"_p"+std::to_string(pi)+"_v"+std::to_string(md),4,sc,0,0,pi,md,ac};numeric(m,kr.data(),fr.data(),br.data());equivalence(g,m,h0b,wv1,wv2,h1,h1b,h2f,h2b,perm,perm,graph,&plans[pi]);profile_method(g,m,h0b,wv1,wv2,h1,h1b,h2b,perm,graph,&plans[pi],plans[0]);}
 }
 if(smoke){std::vector<uint16_t>sh(n);const float vals[]={1.f,-1.f,256.f,-256.f,.00390625f,-.00390625f,0.f};for(size_t i=0;i<n;i++)sh[i]=::f32_to_bf16(vals[(i/128+i%128)%7]);for(bool ac:{false,true})for(int sc:{64,256,0})for(int pi=0;pi<3;pi++)for(int md=0;md<5;md++){Method m{"stress_m"+std::to_string(sc)+"_a"+std::to_string(ac)+"_p"+std::to_string(pi)+"_v"+std::to_string(md),4,sc,0,0,pi,md,ac};equivalence(g,m,sh.data(),wv1,wv2,h1,h1b,h2f,h2b,perm,perm,graph,&plans[pi]);}}
 printf("PAIRPLAN_GATE_COMPLETE graph=%s pass=1\n",graph);fflush(stdout);
 auto run=[&](size_t k){if(k==methods.size()){::mkl_spmm_gemm(a,descr,h0,w1,z,mh1,g.n);::relu_f32(mh1,n);::mkl_spmm_gemm(a,descr,mh1,w2,z,mh2,g.n);}else{auto&m=methods[k];forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,schedules[m.schedule].perm.data(),m.plan_index<0?nullptr:&plans[m.plan_index]);}};
 if(!smoke){for(size_t k=0;k<=methods.size();k++){run(k);for(int rep=0;rep<5;rep++){double t=omp_get_wtime();run(k);double ms=(omp_get_wtime()-t)*1000;printf("PAIRPLAN_TIME graph=%s method=%s kind=consecutive repeat=%d order=%zu ms=%.12g\n",graph,k==methods.size()?"source_mkl_fp32":methods[k].name.c_str(),rep,k,ms);fflush(stdout);}}
 for(size_t k=0;k<=methods.size();k++)run(k);for(int rep=0;rep<10;rep++)for(size_t pos=0;pos<=methods.size();pos++){size_t k=rep%2?methods.size()-pos:pos;double t=omp_get_wtime();run(k);double ms=(omp_get_wtime()-t)*1000;printf("PAIRPLAN_TIME graph=%s method=%s kind=alternating repeat=%d order=%zu ms=%.12g\n",graph,k==methods.size()?"source_mkl_fp32":methods[k].name.c_str(),rep,pos,ms);fflush(stdout);}}
 if(!smoke&&getenv("PAIRPLAN_PMU")&&std::string(getenv("PAIRPLAN_PMU"))=="1")for(size_t k=0;k<methods.size();k++){auto&m=methods[k];if(m.name!="pack_epi_degree"&&m.name!="legacy_s_degree"&&m.name!="opt_dyn_s_degree"&&m.name!="opt_exact_s_degree")continue;for(int rep=0;rep<3;rep++){run(k);gcn_extra_projection_pmu::Region region;region.start();double t=omp_get_wtime();run(k);double ms=(omp_get_wtime()-t)*1000;auto result=region.stop();gcn_extra_projection_pmu::print(result,graph,m.name.c_str(),rep);printf("PAIRPLAN_PMU_WALL graph=%s method=%s repeat=%d ms=%.12g\n",graph,m.name.c_str(),rep,ms);}}
 rusage ru{};getrusage(RUSAGE_SELF,&ru);printf("PAIRPLAN_COMPLETE graph=%s pass=1 methods=%zu smoke=%d max_rss_kib=%ld\n",graph,methods.size(),smoke,ru.ru_maxrss);fflush(stdout);
}
}
