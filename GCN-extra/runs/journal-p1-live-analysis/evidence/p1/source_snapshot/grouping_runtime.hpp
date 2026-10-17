#pragma once
#include "paper_methods_runtime.hpp"
#include "projection_window_kernels.hpp"
#include "projection_window_pmu.hpp"
#include "grouping_schedule.hpp"
namespace gcn_extra_grouping_runtime {
using gcn_extra_paper::DIM;
using gcn_extra_paper::SourceGraphView;
struct Method { std::string name; int kind,scope,schedule; };
template<bool B16> void layer(const SourceGraphView& g,const Method& m,const uint16_t* h,const uint16_t* w,
    std::conditional_t<B16,uint16_t,float>* out,const int* p) {
    if(m.kind==0) {
        if constexpr(B16)::tfs_v3_bf16out(g.row.p,g.col.p,h,w,out,p,g.n,64);
        else ::tfs_v3_fp32out(g.row.p,g.col.p,h,w,out,p,g.n,64);
    } else if(m.kind==1) {
        gcn_extra_paper::Method c{m.name,m.scope,false,false,0,true};
        gcn_extra_paper::block_kernel<false,B16>(g,h,w,out,p,c,nullptr);
    } else {
        gcn_extra_projection::Method c{m.name,64,m.scope,false,true};
        gcn_extra_projection::kernel<false,B16>(g,h,w,out,p,c,nullptr);
    }
}
template<bool B16> void forward(const SourceGraphView& g,const Method& m,const uint16_t* h0,const uint16_t* w1,const uint16_t* w2,
    float* h1,uint16_t* h1b,std::conditional_t<B16,uint16_t,float>* out,const int* p) {
    layer<false>(g,m,h0,w1,h1,p);::relu_f32(h1,size_t(g.n)*DIM);
    ::convert_f32_to_bf16(h1,h1b,size_t(g.n)*DIM);layer<B16>(g,m,h1b,w2,out,p);
}
template<class T> void equal(const char* graph,const Method& m,const char* boundary,const T* actual,const T* ref,size_t n) {
    auto e=gcn_extra_paper::error(actual,ref,n);bool bits=std::memcmp(actual,ref,n*sizeof(T))==0;
    printf("METHOD_EQUIV graph=%s method=%s boundary=%s scope=%d reference=degree_same_scope max_abs=%.12g relative_L2=%.12g bitwise=%d pass=%d\n",graph,m.name.c_str(),boundary,m.scope,e.max_abs,e.l2,bits,bits&&e.finite);fflush(stdout);
    if(!bits||!e.finite)throw std::runtime_error("grouping changes same-scope numerical output");
}
inline void print_profile(const gcn_extra_projection::Profile& p,const char* graph,const Method& m,int li,double wall) {
    using namespace gcn_extra_projection;
    for(int ph=0;ph<gcn_extra_paper::PHASES;ph++)printf("METHOD_PROFILE graph=%s method=%s layer=%d phase=%s sampled_thread_ms=%.12g sampled_tiles=%llu wall_ms=%.12g\n",graph,m.name.c_str(),li+1,gcn_extra_paper::phase_name[ph],p.sec[ph]*1000,(unsigned long long)p.tiles,wall);
    for(int ph=0;ph<DETAIL_PHASES;ph++)printf("METHOD_DETAIL graph=%s method=%s layer=%d phase=%s sampled_thread_ms=%.12g clocked_sections=%llu parent_child_overlap=1 wall_ms=%.12g\n",graph,m.name.c_str(),li+1,detail_phase_name[ph],p.detail_sec[ph]*1000,(unsigned long long)p.detail_events[ph],wall);
    for(const auto& th:p.threads) {
        printf("METHOD_THREAD graph=%s method=%s layer=%d thread=%d setup_ms=%.12g active_ms=%.12g release_ms=%.12g start_offset_ms=%.12g finish_offset_ms=%.12g completed_tiles=%llu completed_edges=%llu kind=instrumented\n",graph,m.name.c_str(),li+1,th.thread_id,th.setup_s*1000,th.kernel_active_s*1000,th.release_s*1000,th.start_offset_s*1000,th.finish_offset_s*1000,(unsigned long long)th.completed_tiles,(unsigned long long)th.completed_edges);
        for(int ph=0;ph<DETAIL_PHASES;ph++)printf("METHOD_THREAD_DETAIL graph=%s method=%s layer=%d thread=%d phase=%s sampled_thread_ms=%.12g clocked_sections=%llu\n",graph,m.name.c_str(),li+1,th.thread_id,detail_phase_name[ph],th.detail_sec[ph]*1000,(unsigned long long)th.detail_events[ph]);
    }
    printf("METHOD_COUNTERS graph=%s method=%s layer=%d unit=logical_requests_not_DRAM",graph,m.name.c_str(),li+1);
    #define COUNT(F) printf(" " #F "=%llu",(unsigned long long)p.counters.F)
    COUNT(destination_tiles);COUNT(projection_scopes);COUNT(neighbor_windows);COUNT(row_window_visits);
    COUNT(nonempty_row_windows);COUNT(source_visits);COUNT(feature_adds);COUNT(source_decode_bytes);
    COUNT(partial_zero_bytes);COUNT(window_state_load_bytes);COUNT(window_state_store_bytes);COUNT(csr_prefetch_instructions);
    COUNT(partial_packed_bytes);COUNT(projection_panels);COUNT(amx_input_loads);COUNT(amx_accumulator_loads);
    COUNT(amx_accumulator_stores);COUNT(amx_dpbf16ps);COUNT(amx_executed_flops);COUNT(output_store_bytes);
    #undef COUNT
    printf("\n");
}
inline void same_work(const gcn_extra_projection::Counters& a,const gcn_extra_projection::Counters& b) {
    #define SAME(F) if(a.F!=b.F)throw std::runtime_error("grouping changes work " #F)
    SAME(destination_tiles);SAME(projection_scopes);SAME(neighbor_windows);SAME(row_window_visits);
    SAME(nonempty_row_windows);SAME(source_visits);SAME(feature_adds);SAME(source_decode_bytes);
    SAME(partial_zero_bytes);SAME(window_state_load_bytes);SAME(window_state_store_bytes);SAME(csr_prefetch_instructions);
    SAME(partial_packed_bytes);SAME(projection_panels);SAME(amx_input_loads);SAME(amx_accumulator_loads);
    SAME(amx_accumulator_stores);SAME(amx_dpbf16ps);SAME(amx_executed_flops);SAME(output_store_bytes);
    #undef SAME
}
inline void run_methods(const csr_t& csr,const int* perm,const float* h0,const float* w1,const float* w2,
    const uint16_t* wv1,const uint16_t* wv2,const uint16_t* h0b,float* h1,uint16_t* h1b,uint16_t* h2b,float* h2f,
    sparse_matrix_t a,matrix_descr descr,float* z,float* mh1,float* mh2,const char* graph) {
    SourceGraphView g{csr.N,csr.nnz,{csr.indptr},{csr.indices}};size_t n=size_t(g.n)*DIM;
    for(uint64_t e=0;e<g.e;e++)if(csr.values[e]!=1.f)throw std::runtime_error("Original TFS requires unit values");
    auto schedules=gcn_extra_grouping::prepare(g,perm,graph);
    std::vector<Method> methods{{"paper_tfs",0,1,0},{"b64_fast",1,64,0},{"bfull_fast",1,0,0}};
    for(int scope:{0,64})for(int s=0;s<4;s++)methods.push_back({std::string(scope?"s64_m64_":"s64_mfull_")+schedules[s].name+"_fast",2,scope,s});
    printf("METHOD_CONFIG graph=%s N=%d E=%u D=128 F=128 R=64 TR=16 warmups=1 repeats=5 final_output=BF16 mkl_threads=%d mkl_dynamic=%d numa=default controls=frozen source_srand=12345 grouping_segment=4096 signature_neighbors=64\n",graph,g.n,csr.nnz,mkl_get_max_threads(),mkl_get_dynamic());
    {
        std::vector<float> final_ref(h2f,h2f+n);std::vector<uint16_t> final_bref(h2b,h2b+n);
        layer<false>(g,methods[0],h0b,wv1,h1,perm);std::vector<float> kernel_ref(h1,h1+n);
        for(const auto& m:methods) {
            const int* p=schedules[m.schedule].perm.data();bool original=m.kind==0;
            std::fill(h1,h1+n,std::numeric_limits<float>::quiet_NaN());layer<false>(g,m,h0b,wv1,h1,p);
            gcn_extra_paper::check(graph,m.name.c_str(),"kernel_FP32","paper_tfs",h1,kernel_ref.data(),n,.01,original);
            forward<false>(g,m,h0b,wv1,wv2,h1,h1b,h2f,p);
            gcn_extra_paper::check(graph,m.name.c_str(),"e2e_FP32_final","paper_tfs",h2f,final_ref.data(),n,.01,original);
            std::fill(h2b,h2b+n,uint16_t(0x7fc0));forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,p);
            gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","paper_tfs",h2b,final_bref.data(),n,.02,original);
            gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","source_MKL_FP32",h2b,mh2,n,.03);
            if(m.kind==2) {
                Method ref=m;ref.kind=1;ref.schedule=0;
                layer<false>(g,m,h0b,wv1,h1,p);std::vector<float> expected(h1,h1+n);
                layer<false>(g,ref,h0b,wv1,h1,perm);equal(graph,m,"kernel_FP32",expected.data(),h1,n);
                forward<false>(g,m,h0b,wv1,wv2,h1,h1b,h2f,p);expected.assign(h2f,h2f+n);
                forward<false>(g,ref,h0b,wv1,wv2,h1,h1b,h2f,perm);equal(graph,m,"e2e_FP32_final",expected.data(),h2f,n);
                forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,p);std::vector<uint16_t> eb(h2b,h2b+n);
                forward<true>(g,ref,h0b,wv1,wv2,h1,h1b,h2b,perm);equal(graph,m,"e2e_BF16_final",eb.data(),h2b,n);
            }
        }
    }
    printf("METHOD_GATE_COMPLETE graph=%s methods=%zu pass=1\n",graph,methods.size());fflush(stdout);
    auto run=[&](size_t k) {
        if(k==methods.size()){::mkl_spmm_gemm(a,descr,h0,w1,z,mh1,g.n);::relu_f32(mh1,n);::mkl_spmm_gemm(a,descr,mh1,w2,z,mh2,g.n);}
        else {auto& m=methods[k];forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,schedules[m.schedule].perm.data());}
    };
    for(size_t k=0;k<=methods.size();k++) {
        run(k);for(int rep=0;rep<5;rep++){double t=omp_get_wtime();run(k);
            printf("METHOD_TIME graph=%s method=%s kind=consecutive_e2e repeat=%d order=%zu ms=%.12g\n",graph,k==methods.size()?"source_mkl_fp32":methods[k].name.c_str(),rep,k,(omp_get_wtime()-t)*1000);fflush(stdout);}
    }
    for(size_t k=0;k<=methods.size();k++)run(k);
    for(int rep=0;rep<5;rep++)for(size_t pos=0;pos<=methods.size();pos++) {
        size_t k=rep%2?methods.size()-pos:pos;double t=omp_get_wtime();run(k);
        printf("METHOD_TIME graph=%s method=%s kind=alternating_e2e repeat=%d order=%zu ms=%.12g\n",graph,k==methods.size()?"source_mkl_fp32":methods[k].name.c_str(),rep,pos,(omp_get_wtime()-t)*1000);fflush(stdout);
    }
    gcn_extra_projection::Counters work[2][2];
    for(const auto& m:methods) {
        const int* p=schedules[m.schedule].perm.data();
        for(int rep=0;rep<3;rep++) {
            double t0=omp_get_wtime();layer<false>(g,m,h0b,wv1,h1,p);double t1=omp_get_wtime();
            ::relu_f32(h1,n);double t2=omp_get_wtime();::convert_f32_to_bf16(h1,h1b,n);double t3=omp_get_wtime();
            layer<true>(g,m,h1b,wv2,h2b,p);double t4=omp_get_wtime();
            printf("METHOD_STAGE graph=%s method=%s repeat=%d layer1_ms=%.12g relu_ms=%.12g interlayer_conversion_ms=%.12g layer2_ms=%.12g total_ms=%.12g\n",graph,m.name.c_str(),rep,(t1-t0)*1000,(t2-t1)*1000,(t3-t2)*1000,(t4-t3)*1000,(t4-t0)*1000);
        }
        if(m.kind!=2)continue;
        for(int li=0;li<2;li++) {
            layer<false>(g,m,h0b,wv1,h1,p);std::vector<float> ref_fp;std::vector<uint16_t> ref_bf;
            if(li==0)ref_fp.assign(h1,h1+n);
            else {::relu_f32(h1,n);::convert_f32_to_bf16(h1,h1b,n);layer<true>(g,m,h1b,wv2,h2b,p);ref_bf.assign(h2b,h2b+n);}
            gcn_extra_projection::Profile profile;gcn_extra_projection::Method config{m.name,64,m.scope,false,true};double t=omp_get_wtime();
            if(li==0)gcn_extra_projection::kernel<true,false>(g,h0b,wv1,h1,p,config,&profile);
            else gcn_extra_projection::kernel<true,true>(g,h1b,wv2,h2b,p,config,&profile);
            double wall=(omp_get_wtime()-t)*1000;bool bits=li==0?std::memcmp(h1,ref_fp.data(),n*sizeof(float))==0:std::memcmp(h2b,ref_bf.data(),n*sizeof(uint16_t))==0;
            printf("METHOD_PROFILE_GATE graph=%s method=%s layer=%d reference=uninstrumented bitwise=%d pass=%d\n",graph,m.name.c_str(),li+1,bits,bits);
            if(!bits)throw std::runtime_error("grouping profile changes output");print_profile(profile,graph,m,li,wall);
            if(profile.counters.source_visits!=g.e)throw std::runtime_error("grouping edge count differs");
            int scope_index=m.scope?1:0;if(m.schedule==0)work[scope_index][li]=profile.counters;
            else same_work(work[scope_index][li],profile.counters);
            printf("GROUP_WORK_GATE graph=%s method=%s layer=%d reference=degree_same_scope pass=1\n",graph,m.name.c_str(),li+1);
        }
    }
    for(size_t k=0;k<=methods.size();k++)for(int rep=0;rep<3;rep++) {
        run(k);gcn_extra_projection_pmu::Region region;region.start();double t=omp_get_wtime();run(k);double wall=(omp_get_wtime()-t)*1000;
        auto result=region.stop();const char* name=k==methods.size()?"source_mkl_fp32":methods[k].name.c_str();
        gcn_extra_projection_pmu::print(result,graph,name,rep);printf("METHOD_PMU_WALL graph=%s method=%s repeat=%d ms=%.12g kind=diagnostic_not_primary\n",graph,name,rep,wall);
    }
    for(const auto& schedule:schedules)gcn_extra_grouping::structure(g,schedule.perm.data(),graph,schedule.name.c_str());
    rusage ru{};getrusage(RUSAGE_SELF,&ru);printf("METHOD_COMPLETE graph=%s methods=%zu checks=%zu pass=1 max_rss_kib=%ld\n",graph,methods.size(),methods.size()*4,ru.ru_maxrss);fflush(stdout);
}
}
