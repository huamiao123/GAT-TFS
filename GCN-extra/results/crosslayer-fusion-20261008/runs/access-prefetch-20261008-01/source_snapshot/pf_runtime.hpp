#pragma once
#include "plan_runtime.hpp"
#include "pf_kernel.hpp"
namespace gcn_extra_prefetch_runtime {
using namespace gcn_extra_pair_plan_runtime;
// Keep the frozen original Method unchanged. PF dispatch precedes the kernel.
struct Method : gcn_extra_pair_plan_runtime::Method {
 int pf=1;
 Method(std::string n,int k=4,int sc=0,int sched=0,int pair_=0,int pi=-1,int md=0,bool ac=false,int policy=1)
 :gcn_extra_pair_plan_runtime::Method{std::move(n),k,sc,sched,pair_,pi,md,ac},pf(policy){}
};
template<bool P,bool B16,int MODE> void pf_layer(const SourceGraphView&g,const Method&m,const uint16_t*h,const uint16_t*w,std::conditional_t<B16,uint16_t,float>*out,const int*perm,bool relu,const Plan&plan,gcn_extra_pair_plan::Profile*pr){
 gcn_extra_state::Method c{m.name,64,m.scope,m.accurate,true,true,relu};
 switch(m.pf){
#define PF_CASE(F) case F:gcn_extra_prefetch::kernel<P,B16,MODE,F>(g,h,w,out,perm,c,plan,pr);break
 PF_CASE(0);PF_CASE(1);PF_CASE(8);
#undef PF_CASE
 default:throw std::runtime_error("Invalid prefetch policy");}
}
template<bool P,bool B16> void planned_layer(const SourceGraphView&g,const Method&m,const uint16_t*h,const uint16_t*w,std::conditional_t<B16,uint16_t,float>*out,const int*perm,bool relu,const Plan&plan,gcn_extra_pair_plan::Profile*pr){
 switch(m.mode){case 0:pf_layer<P,B16,0>(g,m,h,w,out,perm,relu,plan,pr);break;case 4:pf_layer<P,B16,4>(g,m,h,w,out,perm,relu,plan,pr);break;default:throw std::runtime_error("Invalid prefetch mode");}
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
inline void profile_method(const SourceGraphView&g,const Method&m,const uint16_t*h0,const uint16_t*w1,const uint16_t*w2,float*h1,uint16_t*h1b,uint16_t*h2b,const int*perm,const char*graph,const Plan*plan,const Plan&adj){
 size_t n=size_t(g.n)*DIM;forward<true>(g,m,h0,w1,w2,h1,h1b,h2b,perm,plan);std::vector<uint16_t>ref1(h1b,h1b+n),ref2(h2b,h2b+n);Work e=expected_work(g,perm,m.scope,m.accurate);if(m.kind==4&&m.pf==0)e.prefetch=0;
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
inline uint64_t slot_hash(const Plan&p){uint64_t h=1469598103934665603ULL;auto byte=[&](uint8_t x){h=(h^x)*1099511628211ULL;};for(const auto&t:p.tiles){for(uint8_t x:t.order)byte(x);for(int k=0;k<4;k++)byte(uint8_t(t.windows>>(8*k)));for(int k=0;k<8;k++)byte(uint8_t(t.mask_offset>>(8*k)));}for(uint8_t x:p.exact_masks)byte(x);return h;}
inline Plan shuffled_pairs(const SourceGraphView&g,const int*perm,const Plan&adj,const char*graph,const std::string&name,uint64_t seed){
 double begin=omp_get_wtime();Plan p=adj;p.name=name;uint64_t changed=0;const uint64_t salt=0x504149524f524452ULL;
 for(size_t ti=0;ti<p.tiles.size();ti++){int batch=std::min(16,g.n-int(ti)*16),np=batch/2;int map[8];for(int j=0;j<np;j++)map[j]=j;uint64_t x=gcn_extra_grouping::mix(seed^salt^uint64_t(ti));for(int j=np-1;j>0;j--){x=gcn_extra_grouping::mix(x);std::swap(map[j],map[x%uint64_t(j+1)]);}
 const auto&src=adj.tiles[ti];auto&dst=p.tiles[ti];for(int j=0;j<np;j++){dst.order[2*j]=src.order[2*map[j]];dst.order[2*j+1]=src.order[2*map[j]+1];}if(batch%2)dst.order[batch-1]=src.order[batch-1];bool different=false;for(int j=0;j<batch;j++)different=different||src.order[j]!=dst.order[j];changed+=different;
 for(uint32_t w=0;w<src.windows;w++){uint8_t old=adj.exact_masks[src.mask_offset+w],bits=0;for(int j=0;j<np;j++)bits|=((old>>map[j])&1u)<<j;p.exact_masks[dst.mask_offset+w]=bits;}
 }
 double derive=(omp_get_wtime()-begin)*1000;begin=omp_get_wtime();uint64_t checked_pairs=0,checked_windows=0,invalid=0,actual_edges=0,actual_windows=0;
 for(size_t ti=0;ti<p.tiles.size();ti++){int batch=std::min(16,g.n-int(ti)*16),np=batch/2;const auto&a=adj.tiles[ti];const auto&b=p.tiles[ti];invalid+=a.windows!=b.windows||a.mask_offset!=b.mask_offset;bool used[8]{};int inverse[8]{};bool seen[16]{};
 for(int j=0;j<batch;j++){invalid+=b.order[j]>=batch;if(b.order[j]<batch){invalid+=seen[b.order[j]];seen[b.order[j]]=true;}}
 for(int j=batch;j<16;j++)invalid+=b.order[j]!=255;
 for(int j=0;j<np;j++){int found=-1;for(int old=0;old<np;old++)if(b.order[2*j]==a.order[2*old]&&b.order[2*j+1]==a.order[2*old+1])found=old;invalid+=found<0;if(found>=0){invalid+=used[found];used[found]=true;inverse[j]=found;}checked_pairs++;}
 if(batch%2)invalid+=a.order[batch-1]!=b.order[batch-1];
 for(uint32_t w=0;w<a.windows;w++){uint8_t old=adj.exact_masks[a.mask_offset+w],now=p.exact_masks[b.mask_offset+w];
 for(int j=0;j<np;j++){
 invalid+=((old>>inverse[j])&1u)!=((now>>j)&1u);
 if(b.order[2*j]>=batch||b.order[2*j+1]>=batch){invalid++;continue;}
 int row0=perm[ti*16+b.order[2*j]],row1=perm[ti*16+b.order[2*j+1]];uint32_t base0=g.row[row0],base1=g.row[row1],d0=g.row[row0+1]-base0,d1=g.row[row1+1]-base1,start=w*64;
 uint32_t len0=d0>start?std::min(64u,d0-start):0,len1=d1>start?std::min(64u,d1-start):0;
 bool exact=len0>0&&len0==len1&&std::memcmp(g.col.p+size_t(base0)+start,g.col.p+size_t(base1)+start,size_t(len0)*sizeof(uint32_t))==0;
 invalid+=exact!=bool((now>>j)&1u);if(exact){actual_edges+=len0;actual_windows++;}
 }
 invalid+=np<8&&(now>>np)!=0;checked_windows++;}
 }
 invalid+=actual_edges!=adj.exact_shared_edges||actual_windows!=adj.exact_windows;p.fnv64=slot_hash(p);
 printf("FACT_PAIR_PROOF graph=%s plan=%s seed=%llu salt=%llu tiles=%zu changed_tiles=%llu checked_pairs=%llu checked_windows=%llu pair_set_preserved=1 pair_direction_preserved=1 odd_row_last=1 exact_mask_remapped=1 shared_heads_preserved_by_pair_set=1 actual_exact_shared_edges=%llu actual_exact_windows=%llu expected_exact_shared_edges=%llu expected_exact_windows=%llu invalid=%llu pass=%d derive_ms=%.12g audit_ms=%.12g fnv64=%llu\n",graph,p.name.c_str(),(unsigned long long)seed,(unsigned long long)salt,p.tiles.size(),(unsigned long long)changed,(unsigned long long)checked_pairs,(unsigned long long)checked_windows,(unsigned long long)actual_edges,(unsigned long long)actual_windows,(unsigned long long)adj.exact_shared_edges,(unsigned long long)adj.exact_windows,(unsigned long long)invalid,invalid==0,derive,(omp_get_wtime()-begin)*1000,(unsigned long long)p.fnv64);fflush(stdout);if(invalid)throw std::runtime_error("Pair shuffle proof failed");return p;
}
template<class T> inline void execution_equal(const char*graph,const Method&label,const Method&execution,const char*boundary,const T*got,const T*ref,size_t n){bool bits=std::memcmp(got,ref,n*sizeof(T))==0;printf("FACT_EXEC_EQUIV graph=%s method=%s boundary=%s mode=%d execution_mode=%d pf=%d execution_pf=%d execution_name=%s execution_method_ptr=%p bitwise=%d pass=%d\n",graph,label.name.c_str(),boundary,label.mode,execution.mode,label.pf,execution.pf,execution.name.c_str(),(const void*)&execution,bits,bits);fflush(stdout);if(!bits)throw std::runtime_error("Actual timed execution differs: "+label.name+"/"+boundary);}
inline void execution_equivalence(const SourceGraphView&g,const Method&label,const Method&execution,const uint16_t*h0,const uint16_t*w1,const uint16_t*w2,float*h1,uint16_t*h1b,float*h2f,uint16_t*h2b,const int*perm,const char*graph,const Plan*plan){
 size_t n=size_t(g.n)*DIM;
 {layer<false>(g,label,h0,w1,h1,perm,false,plan);std::vector<float>ref(h1,h1+n);std::fill(h1,h1+n,std::numeric_limits<float>::quiet_NaN());layer<false>(g,execution,h0,w1,h1,perm,false,plan);execution_equal(graph,label,execution,"kernel_FP32",h1,ref.data(),n);}
 {forward<false>(g,label,h0,w1,w2,h1,h1b,h2f,perm,plan);std::vector<float>ref(h2f,h2f+n);std::fill(h2f,h2f+n,std::numeric_limits<float>::quiet_NaN());std::fill(h1b,h1b+n,uint16_t(0x7fc0));forward<false>(g,execution,h0,w1,w2,h1,h1b,h2f,perm,plan);execution_equal(graph,label,execution,"e2e_FP32",h2f,ref.data(),n);}
 {forward<true>(g,label,h0,w1,w2,h1,h1b,h2b,perm,plan);std::vector<uint16_t>ref1(h1b,h1b+n),ref2(h2b,h2b+n);std::fill(h1b,h1b+n,uint16_t(0x7fc0));std::fill(h2b,h2b+n,uint16_t(0x7fc0));forward<true>(g,execution,h0,w1,w2,h1,h1b,h2b,perm,plan);execution_equal(graph,label,execution,"H1_BF16",h1b,ref1.data(),n);execution_equal(graph,label,execution,"e2e_BF16",h2b,ref2.data(),n);}
}

template<class T> inline void identity_equal(const char*graph,const Method&m,const char*boundary,const T*got,const T*ref,size_t n){bool bits=std::memcmp(got,ref,n*sizeof(T))==0;printf("PF_IDENTITY graph=%s method=%s boundary=%s pf=1 mode=%d bitwise=%d pass=%d reference=frozen_planned_kernel\n",graph,m.name.c_str(),boundary,m.mode,bits,bits);if(!bits)throw std::runtime_error("PF1 old-kernel identity failed");}
inline void pf_identity(const SourceGraphView&g,const Method&m,const uint16_t*h0,const uint16_t*w1,const uint16_t*w2,float*h1,uint16_t*h1b,float*h2f,uint16_t*h2b,const int*perm,const char*graph,const Plan*plan){
 if(m.kind!=4||m.pf!=1)return;size_t n=size_t(g.n)*DIM;const auto&old=static_cast<const gcn_extra_pair_plan_runtime::Method&>(m);
 {layer<false>(g,m,h0,w1,h1,perm,false,plan);std::vector<float>ref(h1,h1+n);gcn_extra_pair_plan_runtime::layer<false>(g,old,h0,w1,h1,perm,false,plan);identity_equal(graph,m,"kernel_FP32",h1,ref.data(),n);}
 {forward<false>(g,m,h0,w1,w2,h1,h1b,h2f,perm,plan);std::vector<float>ref(h2f,h2f+n);gcn_extra_pair_plan_runtime::forward<false>(g,old,h0,w1,w2,h1,h1b,h2f,perm,plan);identity_equal(graph,m,"e2e_FP32",h2f,ref.data(),n);}
 {forward<true>(g,m,h0,w1,w2,h1,h1b,h2b,perm,plan);std::vector<uint16_t>a(h1b,h1b+n),b(h2b,h2b+n);gcn_extra_pair_plan_runtime::forward<true>(g,old,h0,w1,w2,h1,h1b,h2b,perm,plan);identity_equal(graph,m,"H1_BF16",h1b,a.data(),n);identity_equal(graph,m,"e2e_BF16",h2b,b.data(),n);}
 for(int li=0;li<2;li++){gcn_extra_pair_plan::Profile a,b;const uint16_t*in=li?h1b:h0;const uint16_t*w=li?w2:w1;uint16_t*out=li?h2b:h1b;
 planned_layer<true,true>(g,m,in,w,out,perm,li==0,*plan,&a);std::vector<uint16_t>ref(out,out+n);
 gcn_extra_pair_plan_runtime::planned_layer<true,true>(g,old,in,w,out,perm,li==0,*plan,&b);bool bits=std::memcmp(ref.data(),out,n*2)==0;uint64_t mismatches=0,fields=0;
#define PF_SAME(S,F) fields++;mismatches+=a.S.F!=b.S.F
 PF_SAME(counters,destination_tiles);PF_SAME(counters,projection_scopes);PF_SAME(counters,neighbor_windows);PF_SAME(counters,row_window_visits);PF_SAME(counters,nonempty_row_windows);PF_SAME(counters,source_visits);PF_SAME(counters,feature_adds);PF_SAME(counters,source_decode_bytes);PF_SAME(counters,partial_zero_bytes);PF_SAME(counters,window_state_load_bytes);PF_SAME(counters,window_state_store_bytes);PF_SAME(counters,csr_prefetch_instructions);PF_SAME(counters,partial_packed_bytes);PF_SAME(counters,projection_panels);PF_SAME(counters,amx_input_loads);PF_SAME(counters,amx_accumulator_loads);PF_SAME(counters,amx_accumulator_stores);PF_SAME(counters,amx_dpbf16ps);PF_SAME(counters,amx_executed_flops);PF_SAME(counters,output_store_bytes);
 PF_SAME(logical,packing_partial_load_bytes);PF_SAME(logical,packed_tail_zero_bytes);PF_SAME(logical,direct_pack_rows);PF_SAME(logical,decoded_source_rows);PF_SAME(logical,csr_index_reads);PF_SAME(logical,prefetch_index_reads);PF_SAME(logical,pair_comparisons);PF_SAME(logical,pair_shared_sources);PF_SAME(logical,relu_elements);PF_SAME(logical,relu_negative_elements);
 PF_SAME(plan_logical,order_index_reads);PF_SAME(plan_logical,exact_mask_bytes_read);PF_SAME(plan_logical,exact_pair_flags_tested);PF_SAME(plan_logical,exact_pair_windows);PF_SAME(plan_logical,exact_pair_edges);PF_SAME(plan_logical,exact_shared_edges);PF_SAME(plan_logical,runtime_head_comparisons);
#undef PF_SAME
 bool pass=bits&&!mismatches;printf("PF_IDENTITY_PROFILE graph=%s method=%s layer=%d pf=1 mode=%d fields=%llu mismatches=%llu bitwise=%d pass=%d prefetch_requests=%llu reference_prefetch_requests=%llu prefetch_index_reads=%llu reference_prefetch_index_reads=%llu\n",graph,m.name.c_str(),li+1,m.mode,(unsigned long long)fields,(unsigned long long)mismatches,bits,pass,(unsigned long long)a.counters.csr_prefetch_instructions,(unsigned long long)b.counters.csr_prefetch_instructions,(unsigned long long)a.logical.prefetch_index_reads,(unsigned long long)b.logical.prefetch_index_reads);if(!pass)throw std::runtime_error("PF1 profile identity failed");
 }fflush(stdout);
}
template<int PF> inline std::vector<uint32_t> pf_targets(uint32_t degree,uint32_t scope){std::vector<uint32_t>x;for(uint32_t start=0;start<degree;){uint32_t end=start+std::min(scope?scope:degree,degree-start);for(uint32_t ws=start;ws<end;){uint32_t we=ws+std::min(64u,end-ws);auto emit=[&](uint32_t t){x.push_back(t);};gcn_extra_prefetch::pf_visit_prologue_targets<PF>(ws,end,ws==start,emit);for(uint32_t k=ws;k<we;k++)gcn_extra_prefetch::pf_visit_steady_targets<PF>(k,end,emit);ws=we;}start=end;}return x;}
inline void pf_target_audit(const char*graph){
 const uint32_t degrees[]={0,1,7,8,9,63,64,65,127,128,129,257};uint64_t cases=0,total=0,requests=0,crossings=0;
 for(uint32_t degree:degrees)for(uint32_t scope:{0u,64u,256u}){
 std::vector<uint32_t>csr(degree);for(uint32_t k=0;k<degree;k++)csr[k]=(k*17u+5u)%23u; // Deliberate duplicate and unsorted sources.
 auto off=pf_targets<0>(degree,scope),one=pf_targets<1>(degree,scope),eight=pf_targets<8>(degree,scope);std::vector<uint32_t>expected;
 for(uint32_t start=0;start<degree;){uint32_t end=start+std::min(scope?scope:degree,degree-start);for(uint32_t target=start+1;target<end;target++)expected.push_back(target);start=end;}
 bool pass=off.empty()&&one==expected&&eight==expected;uint64_t hash1=1469598103934665603ULL,hash8=hash1;
 auto address_hash=[&](const std::vector<uint32_t>&v,uint64_t&hash){for(uint32_t t:v){if(t>=degree){pass=false;continue;}for(uint32_t line=0;line<4;line++){uint64_t byte_address=uint64_t(csr[t])*256+line*64;hash=(hash^byte_address)*1099511628211ULL;}}};address_hash(one,hash1);address_hash(eight,hash8);pass=pass&&hash1==hash8;
 for(uint32_t t:expected)crossings+=t%64==0;cases++;total+=one.size();requests+=one.size()*4;
 printf("PF_TARGET_CASE graph=%s degree=%u scope=%u window=64 off_targets=%zu p1_targets=%zu p8_targets=%zu expected_targets=%zu p1_address_hash=%llu p8_address_hash=%llu matched_ordered_positions=%d matched_ordered_addresses=%d pass=%d\n",graph,degree,scope,off.size(),one.size(),eight.size(),expected.size(),(unsigned long long)hash1,(unsigned long long)hash8,one==eight,hash1==hash8,pass);if(!pass)throw std::runtime_error("Prefetch target audit failed");
 }
 printf("PF_TARGET_AUDIT graph=%s cases=%llu p1_targets=%llu p8_targets=%llu prefetch_index_reads=%llu prefetch_requests=%llu off_targets=0 cross_window_targets=%llu matched_ordered_positions=1 matched_ordered_addresses=1 pf8_production_visitor=1 pf1_frozen_formula=1 pass=1\n",graph,(unsigned long long)cases,(unsigned long long)total,(unsigned long long)total,(unsigned long long)total,(unsigned long long)requests,(unsigned long long)crossings);fflush(stdout);
}
inline void run_methods(const csr_t&csr,const int*perm,const float*h0,const float*w1,const float*w2,const uint16_t*wv1,const uint16_t*wv2,const uint16_t*h0b,float*h1,uint16_t*h1b,uint16_t*h2b,float*h2f,sparse_matrix_t a,matrix_descr descr,float*z,float*mh1,float*mh2,const char*graph){
 (void)a;(void)descr;(void)z;(void)mh1;SourceGraphView g{csr.N,csr.nnz,{csr.indptr},{csr.indices}};size_t n=size_t(g.n)*DIM;semantic_gate(graph);pf_target_audit(graph);for(uint64_t e=0;e<g.e;e++)if(csr.values[e]!=1.f)throw std::runtime_error("Nonunit graph");bool smoke=getenv("FACT_SMOKE")&&std::string(getenv("FACT_SMOKE"))=="1";
 const uint64_t seeds[2]={0,20261007};const char*order_names[]={"adj","pairs_s1"};const char*labels[]={"adj","s1"};
 std::array<Plan,2>plans;plans[0]=gcn_extra_pair_plan::prepare_plan(g,perm,"adj_degree",0,graph);for(int i=1;i<2;i++)plans[i]=shuffled_pairs(g,perm,plans[0],graph,order_names[i],seeds[i]);
 std::array<uint64_t,2>hashes;for(int i=0;i<2;i++){hashes[i]=slot_hash(plans[i]);if(plans[i].tiles.size()!=plans[0].tiles.size()||plans[i].exact_masks.size()!=plans[0].exact_masks.size())throw std::runtime_error("Plan layout size differs");printf("FACT_PLAN graph=%s plan=%s slot_hash=%llu tiles=%zu mask_bytes=%zu shared_heads=%llu exact_shared_edges=%llu order_index=%d order_seed=%llu exact_windows=%llu source=immutable_prepared_plan\n",graph,plans[i].name.c_str(),(unsigned long long)hashes[i],plans[i].tiles.size(),plans[i].exact_masks.size(),(unsigned long long)plans[i].dynamic_shared_heads,(unsigned long long)plans[i].exact_shared_edges,i,(unsigned long long)seeds[i],(unsigned long long)plans[i].exact_windows);}
 Plan scratch=plans[0];scratch.name="same_slot";auto*tiles0=scratch.tiles.data();auto*masks0=scratch.exact_masks.data();const size_t tiles_cap=scratch.tiles.capacity(),masks_cap=scratch.exact_masks.capacity(),tiles_size=scratch.tiles.size(),masks_size=scratch.exact_masks.size();
 std::vector<Method>methods{{"pack_epi_degree",3,0,0,0,-1,0}};for(int pi=0;pi<2;pi++)for(int mode:{0,4})for(int pf:{0,1,8}){std::string suffix=mode==0?"Scalar":"ExactS";std::string ps=pf==0?"Off":(pf==1?"P1":"P8");methods.push_back({std::string(labels[pi])+suffix+ps,4,0,0,0,pi,mode,false,pf});}
 printf("FACT_CONFIG graph=%s N=%d E=%u D=128 H=128 F=128 S=64 M=FULL R=64 TR=16 methods=13 modes=0,4 pf_policies=0,1,8 prefetch_matching=ordered_targets_count_hint warmup_consecutive=1 warmup_each_alternating_switch=1 consecutive=5 alternating=10 numa=default smoke=%d final_output=BF16 copy_in_compute=0 purpose=causal_control_not_deployment\n",graph,g.n,csr.nnz,smoke);
 for(const auto&m:methods)printf("FACT_METHOD graph=%s method=%s kind=%d mode=%d pf=%d scope=0 plan_index=%d order_index=%d order_seed=%llu epilogue=1 accurate=0\n",graph,m.name.c_str(),m.kind,m.mode,m.pf,m.plan_index,m.plan_index,(unsigned long long)(m.plan_index>=0?seeds[m.plan_index]:0));
 Method slot_execution{"slot_exec",4,0,0,0,0,0};
 const void*fixed[]={h0,w1,w2,wv1,wv2,h0b,h1,h1b,h2b,h2f,perm,g.row.p,g.col.p};uint64_t serial=0;
 auto select=[&](size_t k,const char*phase,int rep){auto&m=methods[k];if(m.plan_index>=0){slot_execution.mode=m.mode;slot_execution.pf=m.pf;}double copy_ms=0;uint64_t expected=slot_hash(scratch);if(m.plan_index>=0){const auto&p=plans[m.plan_index];double t=omp_get_wtime();std::copy(p.tiles.begin(),p.tiles.end(),scratch.tiles.begin());std::copy(p.exact_masks.begin(),p.exact_masks.end(),scratch.exact_masks.begin());copy_ms=(omp_get_wtime()-t)*1000;expected=hashes[m.plan_index];}uint64_t actual=slot_hash(scratch);const void*now[]={h0,w1,w2,wv1,wv2,h0b,h1,h1b,h2b,h2f,perm,g.row.p,g.col.p};bool stable=memcmp(fixed,now,sizeof(fixed))==0&&scratch.tiles.data()==tiles0&&scratch.exact_masks.data()==masks0&&scratch.tiles.capacity()==tiles_cap&&scratch.exact_masks.capacity()==masks_cap&&scratch.tiles.size()==tiles_size&&scratch.exact_masks.size()==masks_size;bool exec_ok=m.plan_index<0||(slot_execution.kind==m.kind&&slot_execution.scope==m.scope&&slot_execution.schedule==m.schedule&&slot_execution.pair==m.pair&&slot_execution.mode==m.mode&&slot_execution.pf==m.pf&&slot_execution.accurate==m.accurate&&slot_execution.name=="slot_exec");bool pass=stable&&actual==expected&&exec_ok;
 printf("FACT_SLOT graph=%s method=%s phase=%s repeat=%d serial=%llu uses_slot=%d mode=%d execution_mode=%d pf=%d execution_pf=%d order_index=%d order_seed=%llu execution_method_ptr=%p execution_name=%s plan_ptr=%p tiles_ptr=%p mask_ptr=%p tiles_size=%zu mask_size=%zu tiles_capacity=%zu mask_capacity=%zu h0_ptr=%p w1_ptr=%p w2_ptr=%p wv1_ptr=%p wv2_ptr=%p h0b_ptr=%p h1_ptr=%p h1b_ptr=%p h2b_ptr=%p h2f_ptr=%p perm_ptr=%p row_ptr=%p col_ptr=%p slot_hash=%llu expected_hash=%llu stable=%d pass=%d copy_ms=%.12g copy_in_compute=0\n",graph,m.name.c_str(),phase,rep,(unsigned long long)serial++,m.plan_index>=0,m.mode,m.plan_index>=0?slot_execution.mode:methods[0].mode,m.pf,m.plan_index>=0?slot_execution.pf:methods[0].pf,m.plan_index,(unsigned long long)(m.plan_index>=0?seeds[m.plan_index]:0),m.plan_index>=0?(const void*)&slot_execution:(const void*)&methods[0],m.plan_index>=0?slot_execution.name.c_str():methods[0].name.c_str(),(void*)&scratch,(void*)scratch.tiles.data(),(void*)scratch.exact_masks.data(),tiles_size,masks_size,tiles_cap,masks_cap,(const void*)h0,(const void*)w1,(const void*)w2,(const void*)wv1,(const void*)wv2,(const void*)h0b,(void*)h1,(void*)h1b,(void*)h2b,(void*)h2f,(const void*)perm,(const void*)g.row.p,(const void*)g.col.p,(unsigned long long)actual,(unsigned long long)expected,stable,pass,copy_ms);fflush(stdout);if(!pass)throw std::runtime_error("Same slot address/hash proof failed");};
 Method tfs{"paper_tfs",0,1};{std::vector<float>fr(h2f,h2f+n);std::vector<uint16_t>br(h2b,h2b+n);layer<false>(g,tfs,h0b,wv1,h1,perm);std::vector<float>kr(h1,h1+n);
 for(size_t k=0;k<methods.size();k++){select(k,"correctness",0);auto&m=methods[k];const Plan*pl=m.plan_index>=0?&scratch:nullptr;
 layer<false>(g,m,h0b,wv1,h1,perm,false,pl);gcn_extra_paper::check(graph,m.name.c_str(),"kernel_FP32","paper_tfs",h1,kr.data(),n,.01);
 forward<false>(g,m,h0b,wv1,wv2,h1,h1b,h2f,perm,pl);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_FP32_final","paper_tfs",h2f,fr.data(),n,.01);
 forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,perm,pl);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","paper_tfs",h2b,br.data(),n,.02);gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","source_MKL_FP32",h2b,mh2,n,.03);
 pf_identity(g,m,h0b,wv1,wv2,h1,h1b,h2f,h2b,perm,graph,pl);equivalence(g,m,h0b,wv1,wv2,h1,h1b,h2f,h2b,perm,perm,graph,pl);profile_method(g,m,h0b,wv1,wv2,h1,h1b,h2b,perm,graph,pl,plans[0]);const Method&actual_execution=m.plan_index>=0?slot_execution:methods[0];execution_equivalence(g,m,actual_execution,h0b,wv1,wv2,h1,h1b,h2f,h2b,perm,graph,pl);}
 }
 printf("FACT_GATE_COMPLETE graph=%s pass=1\n",graph);fflush(stdout);
 auto run=[&](size_t k){const bool uses_slot=methods[k].plan_index>=0;const Method&execution=uses_slot?slot_execution:methods[0];forward<true>(g,execution,h0b,wv1,wv2,h1,h1b,h2b,perm,uses_slot?&scratch:nullptr);};
 if(!smoke){for(size_t k=0;k<methods.size();k++){select(k,"same_slot_consecutive",0);run(k);for(int rep=0;rep<5;rep++){double t=omp_get_wtime();run(k);double ms=(omp_get_wtime()-t)*1000;printf("FACT_TIME graph=%s method=%s kind=same_slot_consecutive repeat=%d order=%zu warmups_before_series=1 ms=%.12g\n",graph,methods[k].name.c_str(),rep,k,ms);fflush(stdout);}}
 for(int rep=0;rep<10;rep++)for(size_t pos=0;pos<methods.size();pos++){size_t k=rep%2?methods.size()-1-pos:pos;select(k,"same_slot_alternating",rep);run(k);double t=omp_get_wtime();run(k);double ms=(omp_get_wtime()-t)*1000;printf("FACT_TIME graph=%s method=%s kind=same_slot_alternating repeat=%d order=%zu warmups_before_measure=1 ms=%.12g\n",graph,methods[k].name.c_str(),rep,pos,ms);fflush(stdout);}
 if(getenv("FACT_PMU")&&std::string(getenv("FACT_PMU"))=="1")for(size_t k=0;k<methods.size();k++)for(int rep=0;rep<3;rep++){select(k,"pmu",rep);run(k);gcn_extra_projection_pmu::Region region;region.start();double t=omp_get_wtime();run(k);double ms=(omp_get_wtime()-t)*1000;auto result=region.stop();gcn_extra_projection_pmu::print(result,graph,methods[k].name.c_str(),rep);printf("FACT_PMU_WALL graph=%s method=%s repeat=%d ms=%.12g\n",graph,methods[k].name.c_str(),rep,ms);}
 }
 rusage ru{};getrusage(RUSAGE_SELF,&ru);printf("FACT_COMPLETE graph=%s pass=1 methods=13 smoke=%d slot_checks=%llu max_rss_kib=%ld\n",graph,smoke,(unsigned long long)serial,ru.ru_maxrss);fflush(stdout);
}
}
