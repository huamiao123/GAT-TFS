#pragma once
#include "plan_runtime.hpp"
#include "trace_kernel.hpp"
#include "trace_opportunity.hpp"
#include <sstream>
namespace gcn_extra_event_trace_runtime {
using gcn_extra_paper::SourceGraphView;
using gcn_extra_paper::DIM;
using Plan=gcn_extra_pair_plan::Plan;
using TracePlan=gcn_extra_event_trace::TracePlan;
using gcn_extra_pair_plan_runtime::Work;
using gcn_extra_pair_plan_runtime::expected_work;
struct Method : gcn_extra_pair_plan_runtime::Method {
 Method(std::string n,int k=4,int sc=0,int sch=0,int pair_=0,int pi=-1,int md=0,bool ac=false)
 :gcn_extra_pair_plan_runtime::Method{std::move(n),k,sc,sch,pair_,pi,md,ac}{}
};
template<bool P,bool B16> inline void trace_layer(const SourceGraphView&g,const Method&m,const uint16_t*h,const uint16_t*w,std::conditional_t<B16,uint16_t,float>*out,const int*perm,bool relu,const Plan&base,const TracePlan&trace,gcn_extra_event_trace::Profile*profile){
 gcn_extra_state::Method c{m.name,64,m.scope,m.accurate,true,true,relu};
 switch(m.mode){
#define TRACE_CASE(M) case M:gcn_extra_event_trace::kernel<P,B16,M>(g,h,w,out,perm,c,base,trace,profile);break
 TRACE_CASE(0);TRACE_CASE(1);TRACE_CASE(2);
#undef TRACE_CASE
 default:throw std::runtime_error("Invalid trace mode");}
}
template<bool B16> inline void layer(const SourceGraphView&g,const Method&m,const uint16_t*h,const uint16_t*w,std::conditional_t<B16,uint16_t,float>*out,const int*perm,bool relu=false,const Plan*base=nullptr,const TracePlan*trace=nullptr){
 if(m.kind==5){if(!base||!trace)throw std::runtime_error("Missing trace plan");trace_layer<false,B16>(g,m,h,w,out,perm,relu,*base,*trace,nullptr);}
 else gcn_extra_pair_plan_runtime::layer<B16>(g,static_cast<const gcn_extra_pair_plan_runtime::Method&>(m),h,w,out,perm,relu,base);
}
template<bool B16> inline void forward(const SourceGraphView&g,const Method&m,const uint16_t*h0,const uint16_t*w1,const uint16_t*w2,float*h1,uint16_t*h1b,std::conditional_t<B16,uint16_t,float>*out,const int*perm,const Plan*base=nullptr,const TracePlan*trace=nullptr){
 if(m.kind>=3)layer<true>(g,m,h0,w1,h1b,perm,true,base,trace);
 else{layer<false>(g,m,h0,w1,h1,perm,false,base,trace);::relu_f32(h1,size_t(g.n)*DIM);::convert_f32_to_bf16(h1,h1b,size_t(g.n)*DIM);}
 layer<B16>(g,m,h1b,w2,out,perm,false,base,trace);
}
template<class T> inline void compare_result(const char*tag,const char*graph,const Method&label,const Method&tested,const char*reference,const char*boundary,const T*got,const T*expected,size_t n){
 bool pass=std::memcmp(got,expected,n*sizeof(T))==0;
 printf("%s graph=%s method=%s boundary=%s reference=%s kind=%d mode=%d execution_kind=%d execution_mode=%d execution_name=%s execution_method_ptr=%p scope=%d accurate=%d bitwise=%d pass=%d\n",tag,graph,label.name.c_str(),boundary,reference,label.kind,label.mode,tested.kind,tested.mode,tested.name.c_str(),(const void*)&tested,label.scope,int(label.accurate),pass,pass);fflush(stdout);
 if(!pass)throw std::runtime_error(std::string(tag)+"/"+label.name+"/"+boundary);
}
inline void compare_paths(const SourceGraphView&g,const Method&label,const Method&tested,const Method&reference,const int*tested_perm,const int*reference_perm,const uint16_t*h0,const uint16_t*w1,const uint16_t*w2,float*h1,uint16_t*h1b,float*h2f,uint16_t*h2b,const Plan&base,const TracePlan&trace,const char*graph,const char*tag){
 size_t n=size_t(g.n)*DIM;
 {layer<false>(g,reference,h0,w1,h1,reference_perm,false,&base,&trace);std::vector<float>v(h1,h1+n);std::fill(h1,h1+n,std::numeric_limits<float>::quiet_NaN());layer<false>(g,tested,h0,w1,h1,tested_perm,false,&base,&trace);compare_result(tag,graph,label,tested,reference.name.c_str(),"kernel_FP32",h1,v.data(),n);}
 {forward<false>(g,reference,h0,w1,w2,h1,h1b,h2f,reference_perm,&base,&trace);std::vector<float>v(h2f,h2f+n);std::fill(h2f,h2f+n,std::numeric_limits<float>::quiet_NaN());forward<false>(g,tested,h0,w1,w2,h1,h1b,h2f,tested_perm,&base,&trace);compare_result(tag,graph,label,tested,reference.name.c_str(),"e2e_FP32",h2f,v.data(),n);}
 {forward<true>(g,reference,h0,w1,w2,h1,h1b,h2b,reference_perm,&base,&trace);std::vector<uint16_t>a(h1b,h1b+n),b(h2b,h2b+n);std::fill(h1b,h1b+n,uint16_t(0x7fc0));std::fill(h2b,h2b+n,uint16_t(0x7fc0));forward<true>(g,tested,h0,w1,w2,h1,h1b,h2b,tested_perm,&base,&trace);compare_result(tag,graph,label,tested,reference.name.c_str(),"H1_BF16",h1b,a.data(),n);compare_result(tag,graph,label,tested,reference.name.c_str(),"e2e_BF16",h2b,b.data(),n);}
}
inline uint64_t base_hash(const Plan&p){uint64_t h=1469598103934665603ULL;auto add=[&](uint8_t b){h=(h^b)*1099511628211ULL;};for(const auto&t:p.tiles){for(uint8_t b:t.order)add(b);for(int k=0;k<4;k++)add(uint8_t(t.windows>>(8*k)));for(int k=0;k<8;k++)add(uint8_t(t.mask_offset>>(8*k)));}for(uint8_t b:p.exact_masks)add(b);return h;}
inline uint64_t trace_payload_hash(const TracePlan&t){uint64_t h=1469598103934665603ULL;auto add=[&](uint8_t b){h=(h^b)*1099511628211ULL;};for(uint64_t off:t.tile_offsets)for(int k=0;k<8;k++)add(uint8_t(off>>(8*k)));for(uint8_t b:t.data)add(b);return h;}
inline void require(bool condition,const char*message){if(!condition)throw std::runtime_error(message);}
inline void audit_trace(const SourceGraphView&g,const int*perm,const Plan&base,const TracePlan&t,const char*graph,bool smoke){
 double begin=omp_get_wtime();std::array<uint64_t,129>encoded{},replayed{};uint64_t events=0,takes[4]{},headers=0,payload=0,comparisons=0,nonempty=0,windows=0;
 require(t.tile_offsets.size()==base.tiles.size()+1,"Trace offsets size");require(t.tile_offsets.front()==0&&t.tile_offsets.back()==t.data.size(),"Trace offsets endpoints");
 for(size_t ti=0;ti<base.tiles.size();ti++){
  const auto&tile=base.tiles[ti];int batch=std::min(16,g.n-int(ti)*16);uint64_t cursor=t.tile_offsets[ti],end=t.tile_offsets[ti+1];require(cursor<=end&&end<=t.data.size(),"Trace tile offset range");nonempty+=tile.windows>0;windows+=tile.windows;
  for(uint32_t wi=0;wi<tile.windows;wi++)for(int pair=0;pair<batch;pair+=2){
   const uint64_t ws=uint64_t(wi)*64;int r0=perm[ti*16+tile.order[pair]],r1=pair+1<batch?perm[ti*16+tile.order[pair+1]]:-1;
   uint64_t b0=g.row[r0],b1=r1>=0?g.row[r1]:0,d0=g.row[r0+1]-b0,d1=r1>=0?g.row[r1+1]-b1:0;
   uint64_t k0=std::min(ws,d0),k1=std::min(ws,d1),e0=std::min(ws+64,d0),e1=std::min(ws+64,d1);unsigned want[128],count=0;
   while(k0<e0||k1<e1){bool a=k0<e0,b=k1<e1;unsigned take;if(a&&b){uint32_t x=g.col[b0+k0],y=g.col[b1+k1];comparisons++;take=x==y?3u:(x<y?1u:2u);}else take=a?1u:2u;require(count<128,"Trace record exceeds 128");want[count++]=take;takes[take]++;if(take&1)k0++;if(take&2)k1++;}
   replayed[count]++;require(cursor<end,"Trace missing header");unsigned stored=t.data[cursor++];require(stored<=128,"Trace invalid event count");encoded[stored]++;headers++;require(stored==count,"Trace record event count mismatch");uint64_t bytes=(stored+3)/4;require(cursor+bytes<=end,"Trace missing payload");payload+=bytes;
   for(unsigned k=0;k<stored;k++)require(((t.data[cursor+k/4]>>(2*(k%4)))&3u)==want[k],"Trace greedy event mismatch");
   if(stored%4)require((t.data[cursor+bytes-1]>>(2*(stored%4)))==0,"Trace nonzero padding");cursor+=bytes;events+=count;
  }
  require(cursor==end,"Trace tile exhaustion");
 }
 bool ok=encoded==replayed&&encoded==t.event_histogram&&events+takes[3]==g.e&&events==base.merge_steps&&takes[3]==base.dynamic_shared_heads&&comparisons==base.comparisons&&t.source_edges==g.e&&t.merge_steps==events&&t.shared_heads==takes[3]&&t.comparisons==comparisons&&t.take1==takes[1]&&t.take2==takes[2]&&t.take3==takes[3]&&t.headers==headers&&t.payload_bytes==payload&&t.data_bytes==headers+payload&&t.data_bytes==t.data.size()&&t.offset_bytes==t.tile_offsets.size()*sizeof(uint64_t)&&t.plan_bytes==t.data_bytes+t.offset_bytes&&t.allocated_bytes==t.data.capacity()+t.tile_offsets.capacity()*sizeof(uint64_t)&&t.nonempty_tiles==nonempty&&t.windows==windows;
 for(unsigned k=0;k<129;k++)printf("TRACE_PLAN_HIST graph=%s event_count=%u builder_records=%llu validator_records=%llu pass=%d\n",graph,k,(unsigned long long)t.event_histogram[k],(unsigned long long)replayed[k],t.event_histogram[k]==replayed[k]&&encoded[k]==replayed[k]);
 printf("TRACE_PLAN graph=%s tiles=%zu nonempty_tiles=%llu windows=%llu source_edges=%llu events=%llu shared_heads=%llu comparisons=%llu take1=%llu take2=%llu take3=%llu headers=%llu payload_bytes=%llu data_bytes=%zu offset_bytes=%llu plan_bytes=%llu allocated_bytes=%llu data_size=%zu data_capacity=%zu offsets_size=%zu offsets_capacity=%zu offset_element_bytes=%zu object_bytes=%zu plan_fnv64=%llu payload_hash=%llu independent_audit_ms=%.12g order_preserved=1 exact_exhaustion=1 padding_zero=1 header_hist_checked=1 pass=%d\n",graph,base.tiles.size(),(unsigned long long)nonempty,(unsigned long long)windows,(unsigned long long)g.e,(unsigned long long)events,(unsigned long long)takes[3],(unsigned long long)comparisons,(unsigned long long)takes[1],(unsigned long long)takes[2],(unsigned long long)takes[3],(unsigned long long)headers,(unsigned long long)payload,t.data.size(),(unsigned long long)t.offset_bytes,(unsigned long long)t.plan_bytes,(unsigned long long)t.allocated_bytes,t.data.size(),t.data.capacity(),t.tile_offsets.size(),t.tile_offsets.capacity(),sizeof(uint64_t),sizeof(TracePlan),(unsigned long long)t.fnv64,(unsigned long long)trace_payload_hash(t),(omp_get_wtime()-begin)*1000,ok);fflush(stdout);require(ok,"Independent trace structure mismatch");
 if(smoke){printf("TRACE_SMOKE_DUMP graph=%s degree_perm=",graph);for(int i=0;i<g.n;i++)printf("%s%d",i?",":"",perm[i]);if(g.n==0)printf("none");printf(" tile_offsets=");for(size_t i=0;i<t.tile_offsets.size();i++)printf("%s%llu",i?",":"",(unsigned long long)t.tile_offsets[i]);printf(" data_hex=");for(uint8_t b:t.data)printf("%02x",unsigned(b));if(t.data.empty())printf("none");printf("\n");}
}
inline void malformed_gate(const char*graph){
 uint32_t rows[]={0,1,2},cols[]={0,0};int perm[]={0,1};SourceGraphView g{2,2,{rows},{cols}};Plan b;b.name="fixture";b.tiles.resize(1);std::fill(std::begin(b.tiles[0].order),std::end(b.tiles[0].order),uint8_t(255));b.tiles[0].order[0]=0;b.tiles[0].order[1]=1;b.tiles[0].windows=1;b.exact_masks={1};b.source_edges=2;b.dynamic_shared_heads=1;b.merge_steps=1;b.comparisons=1;b.exact_shared_edges=1;b.exact_windows=1;b.nonempty_pair_windows=1;b.full_projections=b.b64_projections=b.b256_projections=b.neighbor_windows=1;
 auto good=gcn_extra_event_trace::build_trace(g,perm,b,graph,false);std::string reason;require(gcn_extra_event_trace::trace_validate(g,perm,b,good,&reason),"Malformed gate valid fixture failed");require(good.data.size()==2&&good.data[0]==1&&good.data[1]==3,"Malformed fixture layout changed");
 unsigned rejected=0;for(int k=0;k<2;k++){auto bad=good;if(k==0)bad.data[1]=0;else bad.tile_offsets.back()=bad.data.size()+1;reason.clear();bool pass=!gcn_extra_event_trace::trace_validate(g,perm,b,bad,&reason);rejected+=pass;printf("TRACE_REJECT_CASE graph=%s case=%s valid_fixture=1 rejected=%d reason_present=%d pass=%d\n",graph,k==0?"used_code_zero":"tile_offset_oob",pass,!reason.empty(),pass);require(pass,"Malformed trace accepted");}
 printf("TRACE_REJECT_AUDIT graph=%s cases=2 rejected=%u valid_fixture=1 pass=%d\n",graph,rejected,rejected==2);fflush(stdout);
}
inline void profile_method(const SourceGraphView&g,const Method&m,const uint16_t*h0,const uint16_t*w1,const uint16_t*w2,float*h1,uint16_t*h1b,uint16_t*h2b,const int*perm,const char*graph,const Plan*plan,const Plan&adj,const TracePlan&trace){
 size_t n=size_t(g.n)*DIM;forward<true>(g,m,h0,w1,w2,h1,h1b,h2b,perm,plan,&trace);std::vector<uint16_t>ref1(h1b,h1b+n),ref2(h2b,h2b+n);Work e=expected_work(g,perm,m.scope,m.accurate);
 for(int li=0;li<2;li++){gcn_extra_event_trace::Profile pr;const uint16_t*in=li==0?h0:h1b;const uint16_t*w=li==0?w1:w2;uint16_t*out=li==0?h1b:h2b;
 if(m.kind==5)trace_layer<true,true>(g,m,in,w,out,perm,li==0,*plan,trace,&pr);else if(m.kind==4)gcn_extra_pair_plan_runtime::planned_layer<true,true>(g,m,in,w,out,perm,li==0,*plan,&pr);else gcn_extra_pair_plan_runtime::state_layer<true,true>(g,m,in,w,out,perm,li==0,&pr);
 const auto&c=pr.counters;const auto&q=pr.logical;const auto&t=pr.plan_logical;
 uint64_t hits=0,expected_decode=g.e,expected_csr=g.e;
 if(m.kind==4){if(m.mode==1||m.mode==2)hits=plan->dynamic_shared_heads;if(m.mode==3||m.mode==4)hits=plan->exact_shared_edges;if(m.mode==2||m.mode==4)expected_decode-=hits;if(m.mode==4)expected_csr-=hits;}
 else if(m.kind==5){hits=trace.shared_heads;if(m.mode!=0)expected_decode-=hits;if(m.mode==2)expected_csr-=hits;}
 bool work=c.source_visits==g.e&&c.feature_adds==g.e*128&&c.projection_scopes==e.scopes&&c.neighbor_windows==e.windows&&c.amx_dpbf16ps==e.scopes*32*(m.accurate?2:1)&&q.decoded_source_rows==expected_decode&&q.csr_index_reads==expected_csr&&q.pair_shared_sources==hits&&c.csr_prefetch_instructions==e.prefetch&&c.partial_zero_bytes==0&&c.window_state_load_bytes==e.loads&&c.window_state_store_bytes==e.stores&&c.partial_packed_bytes==e.packed&&q.direct_pack_rows==e.direct&&q.packing_partial_load_bytes==0;
 work=work&&q.prefetch_index_reads*4==e.prefetch&&c.source_decode_bytes==expected_decode*128*2;
 if(m.kind>=4){uint64_t orders=0,flags=0;for(size_t ti=0;ti<plan->tiles.size();ti++){int batch=std::min(16,g.n-int(ti)*16);orders+=uint64_t(plan->tiles[ti].windows)*batch;flags+=uint64_t(plan->tiles[ti].windows)*(batch/2);}work=work&&t.order_index_reads==orders;
 if(m.kind==4&&(m.mode==1||m.mode==2))work=work&&t.runtime_head_comparisons==plan->comparisons&&q.pair_comparisons==plan->comparisons;
 else work=work&&t.runtime_head_comparisons==0&&q.pair_comparisons==0;
 if(m.kind==4&&m.mode>=3)work=work&&t.exact_pair_edges==plan->exact_shared_edges&&t.exact_pair_windows==plan->exact_windows&&t.exact_shared_edges==(m.mode==4?plan->exact_shared_edges:0)&&t.exact_mask_bytes_read==e.windows&&t.exact_pair_flags_tested==flags;
 else work=work&&t.exact_pair_edges==0&&t.exact_pair_windows==0&&t.exact_shared_edges==0&&t.exact_mask_bytes_read==0&&t.exact_pair_flags_tested==0;
 } else work=work&&q.pair_comparisons==0&&t.runtime_head_comparisons==0&&t.order_index_reads==0;
 const auto&tl=pr.trace_logical;bool is_trace=m.kind==5;
 bool trace_work=tl.headers==(is_trace?trace.headers:0)&&tl.payload_bytes==(is_trace?trace.payload_bytes:0)&&tl.events==(is_trace?trace.merge_steps:0)&&tl.take1==(is_trace?trace.take1:0)&&tl.take2==(is_trace?trace.take2:0)&&tl.take3==(is_trace?trace.take3:0)&&tl.tile_offset_reads==(is_trace?trace.nonempty_tiles:0);
 work=work&&trace_work;
 printf("TRACE_LOGICAL graph=%s method=%s layer=%d headers=%llu payload_bytes=%llu events=%llu take1=%llu take2=%llu take3=%llu tile_offset_reads=%llu expected_headers=%llu expected_payload_bytes=%llu expected_events=%llu expected_take1=%llu expected_take2=%llu expected_take3=%llu expected_tile_offset_reads=%llu pass=%d unit=source_level_not_retired\n",graph,m.name.c_str(),li+1,(unsigned long long)tl.headers,(unsigned long long)tl.payload_bytes,(unsigned long long)tl.events,(unsigned long long)tl.take1,(unsigned long long)tl.take2,(unsigned long long)tl.take3,(unsigned long long)tl.tile_offset_reads,(unsigned long long)(is_trace?trace.headers:0),(unsigned long long)(is_trace?trace.payload_bytes:0),(unsigned long long)(is_trace?trace.merge_steps:0),(unsigned long long)(is_trace?trace.take1:0),(unsigned long long)(is_trace?trace.take2:0),(unsigned long long)(is_trace?trace.take3:0),(unsigned long long)(is_trace?trace.nonempty_tiles:0),trace_work);
 printf("PAIRPLAN_COUNTERS graph=%s method=%s layer=%d scope=%d accurate=%d profile_path=production_BF16_epilogue unit=source_level_not_DRAM source_visits=%llu feature_adds=%llu decode_rows=%llu shared_heads=%llu projection_scopes=%llu windows=%llu amx_tdp=%llu partial_zero_bytes=%llu partial_load_bytes=%llu partial_store_bytes=%llu packing_load_bytes=%llu direct_rows=%llu packed_bytes=%llu prefetch_requests=%llu prefetch_index_reads=%llu csr_reads=%llu comparisons=%llu order_index_reads=%llu exact_mask_bytes_read=%llu exact_pair_flags_tested=%llu exact_pair_windows=%llu exact_pair_edges=%llu exact_shared_edges=%llu runtime_head_comparisons=%llu relu_elements=%llu expected_hits=%llu expected_decode=%llu expected_csr=%llu expected_scopes=%llu expected_windows=%llu expected_loads=%llu expected_stores=%llu expected_packed=%llu expected_direct=%llu expected_prefetch=%llu\n",graph,m.name.c_str(),li+1,m.scope,int(m.accurate),(unsigned long long)c.source_visits,(unsigned long long)c.feature_adds,(unsigned long long)q.decoded_source_rows,(unsigned long long)q.pair_shared_sources,(unsigned long long)c.projection_scopes,(unsigned long long)c.neighbor_windows,(unsigned long long)c.amx_dpbf16ps,(unsigned long long)c.partial_zero_bytes,(unsigned long long)c.window_state_load_bytes,(unsigned long long)c.window_state_store_bytes,(unsigned long long)q.packing_partial_load_bytes,(unsigned long long)q.direct_pack_rows,(unsigned long long)c.partial_packed_bytes,(unsigned long long)c.csr_prefetch_instructions,(unsigned long long)q.prefetch_index_reads,(unsigned long long)q.csr_index_reads,(unsigned long long)q.pair_comparisons,(unsigned long long)t.order_index_reads,(unsigned long long)t.exact_mask_bytes_read,(unsigned long long)t.exact_pair_flags_tested,(unsigned long long)t.exact_pair_windows,(unsigned long long)t.exact_pair_edges,(unsigned long long)t.exact_shared_edges,(unsigned long long)t.runtime_head_comparisons,(unsigned long long)q.relu_elements,(unsigned long long)hits,(unsigned long long)expected_decode,(unsigned long long)expected_csr,(unsigned long long)e.scopes,(unsigned long long)e.windows,(unsigned long long)e.loads,(unsigned long long)e.stores,(unsigned long long)e.packed,(unsigned long long)e.direct,(unsigned long long)e.prefetch);
 printf("PAIRPLAN_WORK_GATE graph=%s method=%s layer=%d pass=%d\n",graph,m.name.c_str(),li+1,work);fflush(stdout);if(!work)throw std::runtime_error("Pair plan work mismatch: "+m.name);
 bool bits=memcmp(out,li==0?ref1.data():ref2.data(),n*2)==0;printf("PAIRPLAN_PROFILE_GATE graph=%s method=%s layer=%d bitwise=%d pass=%d\n",graph,m.name.c_str(),li+1,bits,bits);fflush(stdout);if(!bits)throw std::runtime_error("Pair plan instrumentation changes output");
 }
}

inline void run_methods(const csr_t&csr,const int*perm,const float*h0,const float*w1,const float*w2,const uint16_t*wv1,const uint16_t*wv2,const uint16_t*h0b,float*h1,uint16_t*h1b,uint16_t*h2b,float*h2f,sparse_matrix_t a,matrix_descr descr,float*z,float*mh1,float*mh2,const char*graph){
 (void)a;(void)descr;(void)z;(void)mh1;SourceGraphView g{csr.N,csr.nnz,{csr.indptr},{csr.indices}};size_t n=size_t(g.n)*DIM;
 gcn_extra_pair_plan_runtime::semantic_gate(graph);for(uint64_t e=0;e<g.e;e++)require(csr.values[e]==1.f,"Nonunit graph");bool smoke=getenv("TRACE_SMOKE")&&std::string(getenv("TRACE_SMOKE"))=="1";if(smoke)malformed_gate(graph);
 double begin=omp_get_wtime();auto schedules=gcn_extra_grouping::prepare(g,perm,graph);double schedule_ms=(omp_get_wtime()-begin)*1000;
 require(schedules.size()==5&&schedules[0].name=="degree"&&schedules[4].name=="qguard"&&!memcmp(schedules[0].perm.data(),perm,size_t(g.n)*sizeof(int)),"Frozen schedule mismatch");
 printf("TRACE_SCHEDULE_SETUP graph=%s phase=all_five_frozen_schedules ms=%.12g included_in_primary=0\n",graph,schedule_ms);
 const Plan base=gcn_extra_pair_plan::prepare_plan(g,perm,"adj_degree",0,graph);const TracePlan trace=gcn_extra_event_trace::build_trace(g,perm,base,graph);audit_trace(g,perm,base,trace,graph,smoke);gcn_extra_event_trace::report_opportunity(g,perm,base,trace,graph);
 std::vector<Method> methods{{"pack_epi_degree",3,0,0,0,-1,0},{"pack_epi_qguard",3,0,4,0,-1,0},{"dyn_i",4,0,0,0,0,1},{"dyn_s",4,0,0,0,0,2},{"exact_s",4,0,0,0,0,4},{"trace_i",5,0,0,0,0,0},{"trace_s_cached",5,0,0,0,0,1},{"trace_s_rep",5,0,0,0,0,2}};
 printf("TRACE_CONFIG graph=%s N=%d E=%u D=128 H=128 F=128 S=64 M=FULL R=64 TR=16 methods=8 pf=1 warmup_consecutive=1 warmup_each_alternating_switch=1 consecutive=5 alternating=10 numa=default smoke=%d final_output=BF16 copy_in_compute=0 purpose=mechanism_not_deployment fixed_expansion_candidate=trace_s_rep trace_resident_for_all_methods=1\n",graph,g.n,csr.nnz,smoke);
 for(const auto&m:methods)printf("TRACE_METHOD graph=%s method=%s kind=%d mode=%d scope=0 schedule=%s schedule_index=%d plan_index=%d epilogue=1 accurate=0 pf=1\n",graph,m.name.c_str(),m.kind,m.mode,schedules[m.schedule].name.c_str(),m.schedule,m.plan_index);
 Method execution{"trace_exec",4,0,0,0,0,1};uint64_t serial=0;const uint64_t bh=base_hash(base),th=trace_payload_hash(trace);require(bh==base.fnv64,"Base hash mismatch");
 const void*fixed[]={h0,w1,w2,wv1,wv2,h0b,h1,h1b,h2b,h2f,perm,g.row.p,g.col.p,base.tiles.data(),base.exact_masks.data(),trace.tile_offsets.data(),trace.data.data(),schedules[4].perm.data()};
 const size_t sizes[]={base.tiles.size(),base.tiles.capacity(),base.exact_masks.size(),base.exact_masks.capacity(),trace.tile_offsets.size(),trace.tile_offsets.capacity(),trace.data.size(),trace.data.capacity(),schedules[4].perm.size(),schedules[4].perm.capacity()};
 auto immutable=[&](const char*phase){uint64_t now_bh=base_hash(base),now_th=trace_payload_hash(trace);bool pass=now_bh==bh&&now_th==th;printf("TRACE_IMMUTABLE graph=%s phase=%s base_hash=%llu expected_base_hash=%llu trace_hash=%llu expected_trace_hash=%llu hash_checked=1 pass=%d\n",graph,phase,(unsigned long long)now_bh,(unsigned long long)bh,(unsigned long long)now_th,(unsigned long long)th,pass);fflush(stdout);require(pass,"Plan content mutated");};immutable("before");
 auto select=[&](size_t k,const char*phase,int rep){const auto&m=methods[k];if(m.kind>=4){execution.kind=m.kind;execution.mode=m.mode;}const auto&actual=m.kind>=4?execution:m;const void*now[]={h0,w1,w2,wv1,wv2,h0b,h1,h1b,h2b,h2f,perm,g.row.p,g.col.p,base.tiles.data(),base.exact_masks.data(),trace.tile_offsets.data(),trace.data.data(),schedules[4].perm.data()};const size_t current[]={base.tiles.size(),base.tiles.capacity(),base.exact_masks.size(),base.exact_masks.capacity(),trace.tile_offsets.size(),trace.tile_offsets.capacity(),trace.data.size(),trace.data.capacity(),schedules[4].perm.size(),schedules[4].perm.capacity()};bool stable=!memcmp(fixed,now,sizeof(fixed))&&!memcmp(sizes,current,sizeof(sizes));bool pass=stable&&actual.kind==m.kind&&actual.mode==m.mode&&actual.scope==m.scope&&actual.accurate==m.accurate&&actual.schedule==m.schedule;
 printf("TRACE_SLOT graph=%s method=%s phase=%s repeat=%d serial=%llu kind=%d mode=%d execution_kind=%d execution_mode=%d execution_name=%s execution_method_ptr=%p schedule=%s schedule_ptr=%p base_plan_ptr=%p tiles_ptr=%p masks_ptr=%p trace_plan_ptr=%p offsets_ptr=%p data_ptr=%p tiles_size=%zu tiles_capacity=%zu masks_size=%zu masks_capacity=%zu offsets_size=%zu offsets_capacity=%zu data_size=%zu data_capacity=%zu h0_ptr=%p w1_ptr=%p w2_ptr=%p wv1_ptr=%p wv2_ptr=%p h0b_ptr=%p h1_ptr=%p h1b_ptr=%p h2b_ptr=%p h2f_ptr=%p perm_ptr=%p row_ptr=%p col_ptr=%p hash_checked=0 stable=%d pass=%d copy_in_compute=0\n",graph,m.name.c_str(),phase,rep,(unsigned long long)serial++,m.kind,m.mode,actual.kind,actual.mode,actual.name.c_str(),(const void*)&actual,schedules[m.schedule].name.c_str(),(const void*)schedules[m.schedule].perm.data(),(const void*)&base,(const void*)base.tiles.data(),(const void*)base.exact_masks.data(),(const void*)&trace,(const void*)trace.tile_offsets.data(),(const void*)trace.data.data(),base.tiles.size(),base.tiles.capacity(),base.exact_masks.size(),base.exact_masks.capacity(),trace.tile_offsets.size(),trace.tile_offsets.capacity(),trace.data.size(),trace.data.capacity(),(const void*)h0,(const void*)w1,(const void*)w2,(const void*)wv1,(const void*)wv2,(const void*)h0b,(void*)h1,(void*)h1b,(void*)h2b,(void*)h2f,(const void*)perm,(const void*)g.row.p,(const void*)g.col.p,stable,pass);fflush(stdout);require(pass,"Fixed execution storage/configuration changed");};
 Method tfs{"paper_tfs",0,1},canonical{"canonical_full",1,0},dyns{"dyn_s_reference",4,0,0,0,0,2};
 {std::vector<float>fr(h2f,h2f+n);std::vector<uint16_t>br(h2b,h2b+n);layer<false>(g,tfs,h0b,wv1,h1,perm);std::vector<float>kr(h1,h1+n);
 for(size_t k=0;k<methods.size();k++){select(k,"correctness",0);const auto&m=methods[k];const int*p=schedules[m.schedule].perm.data();
 layer<false>(g,m,h0b,wv1,h1,p,false,&base,&trace);gcn_extra_paper::check(graph,m.name.c_str(),"kernel_FP32","paper_tfs",h1,kr.data(),n,.01);
 forward<false>(g,m,h0b,wv1,wv2,h1,h1b,h2f,p,&base,&trace);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_FP32_final","paper_tfs",h2f,fr.data(),n,.01);
 forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,p,&base,&trace);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","paper_tfs",h2b,br.data(),n,.02);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","source_MKL_FP32",h2b,mh2,n,.03);
 compare_paths(g,m,m,canonical,p,perm,h0b,wv1,wv2,h1,h1b,h2f,h2b,base,trace,graph,"PAIRPLAN_EQUIV");
 if(m.kind==5&&m.mode!=0)compare_paths(g,m,m,dyns,p,perm,h0b,wv1,wv2,h1,h1b,h2f,h2b,base,trace,graph,"TRACE_DYNS_EQUIV");
 profile_method(g,m,h0b,wv1,wv2,h1,h1b,h2b,p,graph,&base,base,trace);const auto&actual=m.kind>=4?execution:m;
 compare_paths(g,m,actual,m,p,p,h0b,wv1,wv2,h1,h1b,h2f,h2b,base,trace,graph,"TRACE_EXEC_EQUIV");}
 }
 printf("TRACE_GATE_COMPLETE graph=%s pass=1\n",graph);fflush(stdout);
 auto run=[&](size_t k){const auto&m=methods[k];const auto&actual=m.kind>=4?execution:m;forward<true>(g,actual,h0b,wv1,wv2,h1,h1b,h2b,schedules[m.schedule].perm.data(),&base,&trace);};
 if(!smoke){for(size_t k=0;k<methods.size();k++){select(k,"consecutive",0);run(k);for(int rep=0;rep<5;rep++){double t=omp_get_wtime();run(k);double ms=(omp_get_wtime()-t)*1000;printf("TRACE_TIME graph=%s method=%s kind=consecutive repeat=%d order=%zu warmups_before_series=1 ms=%.12g\n",graph,methods[k].name.c_str(),rep,k,ms);fflush(stdout);}}
 for(int rep=0;rep<10;rep++)for(size_t pos=0;pos<methods.size();pos++){size_t k=rep%2?methods.size()-1-pos:pos;select(k,"alternating",rep);run(k);double t=omp_get_wtime();run(k);double ms=(omp_get_wtime()-t)*1000;printf("TRACE_TIME graph=%s method=%s kind=alternating repeat=%d order=%zu warmups_before_measure=1 ms=%.12g\n",graph,methods[k].name.c_str(),rep,pos,ms);fflush(stdout);}
 if(getenv("TRACE_PMU")&&std::string(getenv("TRACE_PMU"))=="1")for(size_t k=0;k<methods.size();k++)for(int rep=0;rep<3;rep++){select(k,"pmu",rep);run(k);gcn_extra_projection_pmu::Region region;region.start();double t=omp_get_wtime();run(k);double ms=(omp_get_wtime()-t)*1000;auto result=region.stop();gcn_extra_projection_pmu::print(result,graph,methods[k].name.c_str(),rep);printf("TRACE_PMU_WALL graph=%s method=%s repeat=%d ms=%.12g\n",graph,methods[k].name.c_str(),rep,ms);}
 }
 immutable("after");rusage ru{};getrusage(RUSAGE_SELF,&ru);printf("TRACE_COMPLETE graph=%s pass=1 methods=8 smoke=%d slot_checks=%llu max_rss_kib=%ld\n",graph,smoke,(unsigned long long)serial,ru.ru_maxrss);fflush(stdout);
}
}
