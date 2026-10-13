#pragma once
#include "paper_methods_runtime.hpp"
#include "projection_window_kernels.hpp"
#include "projection_window_pmu.hpp"
#include <unordered_set>
namespace gcn_extra_projection_runtime {
using gcn_extra_paper::DIM;
using gcn_extra_paper::SourceGraphView;
struct Method { std::string name; int window, scope; bool accurate; int kind; };
static gcn_extra_paper::Method coupled(const Method& m) {
    return {m.name,m.scope,m.accurate,false,0,true};
}
static gcn_extra_projection::Method decoupled(const Method& m) {
    return {m.name,m.window,m.scope,m.accurate,true};
}
static void structure_probe(const SourceGraphView& g,const int* perm,const uint16_t* h0b,const char* graph) {
    int nt=(g.n+15)/16,limit=std::min(256,nt);
    for(int window:{32,64,128}) {
        std::vector<double> reuse,footprint,pages,occupancy;uint64_t visited=0;
        for(int q=0;q<limit;q++) {
            int tile=limit==1?0:int(uint64_t(q)*(nt-1)/(limit-1)),i=tile*16,batch=std::min(16,g.n-i);uint32_t md=0;
            for(int r=0;r<batch;r++)md=std::max(md,g.row[perm[i+r]+1]-g.row[perm[i+r]]);
            if(!md)continue;
            uint32_t nw=(md+window-1)/window;
            for(uint32_t wi:{0u,nw/2,nw-1}) {
                // Duplicate positions in short tiles are intentionally recorded;
                // these are deterministic sampled windows, not whole-graph totals.
                std::unordered_set<uint32_t> sources;std::unordered_set<uintptr_t> pageids;uint64_t edges=0;
                for(int r=0;r<batch;r++) {
                    uint32_t row=perm[i+r],begin=g.row[row],degree=g.row[row+1]-begin;
                    for(uint32_t k=wi*window;k<std::min(degree,(wi+1)*window);k++) {
                        uint32_t j=g.col[size_t(begin)+k];sources.insert(j);pageids.insert((uintptr_t(h0b)+uintptr_t(j)*256)/4096);edges++;
                    }
                }
                visited+=edges;if(!edges)continue;
                reuse.push_back(double(edges)/sources.size());footprint.push_back(sources.size()*256.);
                pages.push_back(pageids.size());occupancy.push_back(double(edges)/(16*window));
            }
        }
        auto quantile=[](std::vector<double>& v,double p) {if(v.empty())return 0.;std::sort(v.begin(),v.end());return v[size_t(p*(v.size()-1))];};
        printf("METHOD_STRUCTURE graph=%s window=%d kind=deterministic_sampled_256_sorted_tiles_3_offsets sampled_windows=%zu sampled_edge_visits=%llu reuse_p50=%.12g reuse_p90=%.12g reuse_p99=%.12g footprint_bytes_p50=%.12g footprint_bytes_p90=%.12g footprint_bytes_p99=%.12g source_pages_p50=%.12g source_pages_p99=%.12g row_step_occupancy_p50=%.12g\n",graph,window,reuse.size(),(unsigned long long)visited,quantile(reuse,.5),quantile(reuse,.9),quantile(reuse,.99),quantile(footprint,.5),quantile(footprint,.9),quantile(footprint,.99),quantile(pages,.5),quantile(pages,.99),quantile(occupancy,.5));
    }
}
template<bool B16> static void layer(const SourceGraphView& g,const Method& m,const uint16_t* h,const uint16_t* w,
    std::conditional_t<B16,uint16_t,float>* out,const int* perm) {
    if(m.kind==0) {
        if constexpr(B16)::tfs_v3_bf16out(g.row.p,g.col.p,h,w,out,perm,g.n,64);
        else ::tfs_v3_fp32out(g.row.p,g.col.p,h,w,out,perm,g.n,64);
    } else if(m.kind==1)gcn_extra_paper::block_kernel<false,B16>(g,h,w,out,perm,coupled(m),nullptr);
    else gcn_extra_projection::kernel<false,B16>(g,h,w,out,perm,decoupled(m),nullptr);
}
template<bool B16> static void forward(const SourceGraphView& g,const Method& m,const uint16_t* h0,const uint16_t* w1,const uint16_t* w2,
    float* h1,uint16_t* h1b,std::conditional_t<B16,uint16_t,float>* out,const int* perm) {
    layer<false>(g,m,h0,w1,h1,perm);::relu_f32(h1,size_t(g.n)*DIM);
    ::convert_f32_to_bf16(h1,h1b,size_t(g.n)*DIM);layer<B16>(g,m,h1b,w2,out,perm);
}
template<class T> static void equal(const char* graph,const Method& m,const char* boundary,const T* actual,const T* expected,size_t n) {
    auto e=gcn_extra_paper::error(actual,expected,n);bool bits=std::memcmp(actual,expected,n*sizeof(T))==0;
    printf("METHOD_EQUIV graph=%s method=%s boundary=%s scope=%d reference=coupled_same_scope max_abs=%.12g relative_L2=%.12g bitwise=%d pass=%d\n",graph,m.name.c_str(),boundary,m.scope,e.max_abs,e.l2,bits,bits&&e.finite);fflush(stdout);
    if(!bits||!e.finite)throw std::runtime_error("projection window changes same-scope numerical output");
}
static void run_methods(const csr_t& csr,const int* perm,const float* h0,const float* w1,const float* w2,
    const uint16_t* wv1,const uint16_t* wv2,const uint16_t* h0b,float* h1,uint16_t* h1b,uint16_t* h2b,float* h2f,
    sparse_matrix_t a,matrix_descr descr,float* z,float* mh1,float* mh2,const char* graph) {
    SourceGraphView g{csr.N,csr.nnz,{csr.indptr},{csr.indices}};size_t n=size_t(g.n)*DIM;
    for(uint64_t e=0;e<g.e;e++)if(csr.values[e]!=1.f)throw std::runtime_error("Original TFS requires unit values");
    std::vector<Method> methods{
        {"paper_tfs",1,1,false,0},
        {"b64_fast",64,64,false,1},{"b64_accurate",64,64,true,1},
        {"bfull_fast",0,0,false,1},{"bfull_accurate",0,0,true,1},
        {"b256_fast",256,256,false,1},{"b256_accurate",256,256,true,1},
        {"s64_m64_fast",64,64,false,2},
        {"s32_m64_fast",32,64,false,2},
        {"s32_m256_fast",32,256,false,2},{"s64_m256_fast",64,256,false,2},
        {"s32_mfull_fast",32,0,false,2},{"s64_mfull_fast",64,0,false,2},
        {"s128_mfull_fast",128,0,false,2},{"s64_mfull_accurate",64,0,true,2}
    };
    printf("METHOD_CONFIG graph=%s N=%d E=%u D=128 F=128 R=64 TR=16 warmups=1 repeats=5 final_output=BF16 mkl_threads=%d mkl_dynamic=%d numa=default controls=frozen source_srand=12345\n",graph,g.n,csr.nnz,mkl_get_max_threads(),mkl_get_dynamic());
    for(const auto& m:methods) {
        uint64_t steps=0,tiles=0,summax=0,padded=0;
        for(int i=0;i<g.n;i+=16) {
            uint32_t md=0;int batch=std::min(16,g.n-i);
            for(int j=0;j<batch;j++)md=std::max(md,g.row[perm[i+j]+1]-g.row[perm[i+j]]);
            tiles++;summax+=md;padded+=uint64_t(md)*16;
            steps+=m.kind==0?md:(md?(m.scope==0?1:(uint64_t(md)+m.scope-1)/m.scope):0);
        }
        uint64_t parts=m.accurate?2:1,tdp=steps*32*parts;
        printf("METHOD_MODEL graph=%s method=%s kind=analytic_per_layer window=%d scope=%d precision=%s destination_tiles=%llu sum_tile_max_degree=%llu padded_edge_steps=%llu valid_edges=%llu degree_step_occupancy=%.12g projection_scopes=%llu amx_dpbf16ps=%llu amx_executed_flops=%llu source_visits=%llu source_gather_requested_bytes=%llu wide_fp32_adds=%llu\n",graph,m.name.c_str(),m.window,m.scope,m.accurate?"hi_lo":"hi",(unsigned long long)tiles,(unsigned long long)summax,(unsigned long long)padded,(unsigned long long)g.e,padded?double(g.e)/padded:1.,(unsigned long long)steps,(unsigned long long)tdp,(unsigned long long)(tdp*16384),(unsigned long long)(g.e*(m.kind==0?2:1)),(unsigned long long)(g.e*256*(m.kind==0?2:1)),(unsigned long long)(m.kind==0?0:g.e*128));
    }
    {
        std::vector<float> final_ref(h2f,h2f+n);std::vector<uint16_t> final_bref(h2b,h2b+n);
        layer<false>(g,methods[0],h0b,wv1,h1,perm);std::vector<float> kernel_ref(h1,h1+n);
        for(const auto& m:methods) {
            bool original=m.kind==0;double gate=m.accurate?1e-3:1e-2;
            std::fill(h1,h1+n,std::numeric_limits<float>::quiet_NaN());layer<false>(g,m,h0b,wv1,h1,perm);
            gcn_extra_paper::check(graph,m.name.c_str(),"kernel_FP32","paper_tfs",h1,kernel_ref.data(),n,gate,original);
            forward<false>(g,m,h0b,wv1,wv2,h1,h1b,h2f,perm);
            gcn_extra_paper::check(graph,m.name.c_str(),"e2e_FP32_final","paper_tfs",h2f,final_ref.data(),n,gate,original);
            std::fill(h2b,h2b+n,uint16_t(0x7fc0));forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,perm);
            gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","paper_tfs",h2b,final_bref.data(),n,m.accurate?.01:.02,original);
            gcn_extra_paper::check(graph,m.name.c_str(),"e2e_BF16_final","source_MKL_FP32",h2b,mh2,n,.03);
            if(m.kind==2) {
                Method ref=m;ref.kind=1;
                layer<false>(g,m,h0b,wv1,h1,perm);std::vector<float> expected(h1,h1+n);
                layer<false>(g,ref,h0b,wv1,h1,perm);equal(graph,m,"kernel_FP32",expected.data(),h1,n);
                forward<false>(g,m,h0b,wv1,wv2,h1,h1b,h2f,perm);expected.assign(h2f,h2f+n);
                forward<false>(g,ref,h0b,wv1,wv2,h1,h1b,h2f,perm);equal(graph,m,"e2e_FP32_final",expected.data(),h2f,n);
                forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,perm);std::vector<uint16_t> eb(h2b,h2b+n);
                forward<true>(g,ref,h0b,wv1,wv2,h1,h1b,h2b,perm);equal(graph,m,"e2e_BF16_final",eb.data(),h2b,n);
            }
        }
    }
    printf("METHOD_GATE_COMPLETE graph=%s methods=%zu pass=1\n",graph,methods.size());fflush(stdout);
    auto run=[&](size_t k) {
        if(k==methods.size()) {::mkl_spmm_gemm(a,descr,h0,w1,z,mh1,g.n);::relu_f32(mh1,n);::mkl_spmm_gemm(a,descr,mh1,w2,z,mh2,g.n);}
        else forward<true>(g,methods[k],h0b,wv1,wv2,h1,h1b,h2b,perm);
    };
    // Same exact allocation/tensor/precision/NUMA policy; primary source-like immediate warmup.
    for(size_t k=0;k<=methods.size();k++) {
        run(k);
        for(int rep=0;rep<5;rep++) {double t=omp_get_wtime();run(k);double ms=(omp_get_wtime()-t)*1000;
            printf("METHOD_TIME graph=%s method=%s kind=consecutive_e2e repeat=%d order=%zu ms=%.12g\n",graph,k==methods.size()?"source_mkl_fp32":methods[k].name.c_str(),rep,k,ms);fflush(stdout);}
    }
    for(size_t k=0;k<=methods.size();k++)run(k);
    for(int rep=0;rep<5;rep++)for(size_t pos=0;pos<=methods.size();pos++) {
        size_t k=(pos+rep)%(methods.size()+1);double t=omp_get_wtime();run(k);double ms=(omp_get_wtime()-t)*1000;
        printf("METHOD_TIME graph=%s method=%s kind=rotating_e2e repeat=%d order=%zu ms=%.12g\n",graph,k==methods.size()?"source_mkl_fp32":methods[k].name.c_str(),rep,pos,ms);fflush(stdout);
    }
    for(const auto& m:methods) {
        for(int rep=0;rep<3;rep++) {
            double t0=omp_get_wtime();layer<false>(g,m,h0b,wv1,h1,perm);double t1=omp_get_wtime();
            ::relu_f32(h1,n);double t2=omp_get_wtime();::convert_f32_to_bf16(h1,h1b,n);double t3=omp_get_wtime();
            layer<true>(g,m,h1b,wv2,h2b,perm);double t4=omp_get_wtime();
            printf("METHOD_STAGE graph=%s method=%s repeat=%d layer1_ms=%.12g relu_ms=%.12g interlayer_conversion_ms=%.12g layer2_ms=%.12g total_ms=%.12g\n",graph,m.name.c_str(),rep,(t1-t0)*1000,(t2-t1)*1000,(t3-t2)*1000,(t4-t3)*1000,(t4-t0)*1000);
        }
        if(m.kind!=0)for(int li=0;li<2;li++) {
            gcn_extra_paper::Profile old;gcn_extra_projection::Profile p;double t=omp_get_wtime();
            std::vector<float> profile_fp32;std::vector<uint16_t> profile_bf16;
            if(li==0){layer<false>(g,m,h0b,wv1,h1,perm);profile_fp32.assign(h1,h1+n);}
            else {::relu_f32(h1,n);::convert_f32_to_bf16(h1,h1b,n);layer<true>(g,m,h1b,wv2,h2b,perm);profile_bf16.assign(h2b,h2b+n);}
            t=omp_get_wtime();
            if(li==0) {
                if(m.kind==1)gcn_extra_paper::block_kernel<true,false>(g,h0b,wv1,h1,perm,coupled(m),&old);
                else gcn_extra_projection::kernel<true,false>(g,h0b,wv1,h1,perm,decoupled(m),&p);
            }else {
                ::relu_f32(h1,n);::convert_f32_to_bf16(h1,h1b,n);t=omp_get_wtime();
                if(m.kind==1)gcn_extra_paper::block_kernel<true,true>(g,h1b,wv2,h2b,perm,coupled(m),&old);
                else gcn_extra_projection::kernel<true,true>(g,h1b,wv2,h2b,perm,decoupled(m),&p);
            }
            double wall=(omp_get_wtime()-t)*1000;const auto& base=m.kind==1?old:static_cast<const gcn_extra_paper::Profile&>(p);
            bool profile_equal=li==0?std::memcmp(h1,profile_fp32.data(),n*sizeof(float))==0:std::memcmp(h2b,profile_bf16.data(),n*sizeof(uint16_t))==0;
            printf("METHOD_PROFILE_GATE graph=%s method=%s layer=%d reference=uninstrumented bitwise=%d pass=%d\n",graph,m.name.c_str(),li+1,profile_equal,profile_equal);
            if(!profile_equal)throw std::runtime_error("instrumented kernel changes output");
            for(int ph=0;ph<gcn_extra_paper::PHASES;ph++)printf("METHOD_PROFILE graph=%s method=%s layer=%d phase=%s sampled_thread_ms=%.12g sampled_tiles=%llu wall_ms=%.12g\n",graph,m.name.c_str(),li+1,gcn_extra_paper::phase_name[ph],base.sec[ph]*1000,(unsigned long long)base.tiles,wall);
            if(m.kind==2) {
                for(int ph=0;ph<gcn_extra_projection::DETAIL_PHASES;ph++)
                    printf("METHOD_DETAIL graph=%s method=%s layer=%d phase=%s sampled_thread_ms=%.12g clocked_sections=%llu sampled_tiles=%llu parent_child_overlap=1 wall_ms=%.12g\n",graph,m.name.c_str(),li+1,gcn_extra_projection::detail_phase_name[ph],p.detail_sec[ph]*1000,(unsigned long long)p.detail_events[ph],(unsigned long long)p.tiles,wall);
                for(const auto& th:p.threads) {
                    printf("METHOD_THREAD graph=%s method=%s layer=%d thread=%d setup_ms=%.12g active_ms=%.12g release_ms=%.12g start_offset_ms=%.12g finish_offset_ms=%.12g completed_tiles=%llu completed_edges=%llu kind=instrumented\n",graph,m.name.c_str(),li+1,th.thread_id,th.setup_s*1000,th.kernel_active_s*1000,th.release_s*1000,th.start_offset_s*1000,th.finish_offset_s*1000,(unsigned long long)th.completed_tiles,(unsigned long long)th.completed_edges);
                    for(int ph=0;ph<gcn_extra_projection::DETAIL_PHASES;ph++)
                        printf("METHOD_THREAD_DETAIL graph=%s method=%s layer=%d thread=%d phase=%s sampled_thread_ms=%.12g clocked_sections=%llu\n",graph,m.name.c_str(),li+1,th.thread_id,gcn_extra_projection::detail_phase_name[ph],th.detail_sec[ph]*1000,(unsigned long long)th.detail_events[ph]);
                }
                printf("METHOD_PROFILE graph=%s method=%s layer=%d phase=window_state_load sampled_thread_ms=%.12g sampled_tiles=%llu wall_ms=%.12g\n",graph,m.name.c_str(),li+1,p.window_state_load_s*1000,(unsigned long long)p.tiles,wall);
                printf("METHOD_PROFILE graph=%s method=%s layer=%d phase=window_state_store sampled_thread_ms=%.12g sampled_tiles=%llu wall_ms=%.12g\n",graph,m.name.c_str(),li+1,p.window_state_store_s*1000,(unsigned long long)p.tiles,wall);
                printf("METHOD_COUNTERS graph=%s method=%s layer=%d unit=logical_requests_not_DRAM",graph,m.name.c_str(),li+1);
                #define PROJECTION_COUNT(F) printf(" " #F "=%llu",(unsigned long long)p.counters.F)
                PROJECTION_COUNT(destination_tiles);PROJECTION_COUNT(projection_scopes);PROJECTION_COUNT(neighbor_windows);
                PROJECTION_COUNT(row_window_visits);PROJECTION_COUNT(nonempty_row_windows);PROJECTION_COUNT(source_visits);
                PROJECTION_COUNT(feature_adds);PROJECTION_COUNT(source_decode_bytes);PROJECTION_COUNT(partial_zero_bytes);
                PROJECTION_COUNT(window_state_load_bytes);PROJECTION_COUNT(window_state_store_bytes);PROJECTION_COUNT(csr_prefetch_instructions);
                PROJECTION_COUNT(partial_packed_bytes);PROJECTION_COUNT(projection_panels);PROJECTION_COUNT(amx_input_loads);
                PROJECTION_COUNT(amx_accumulator_loads);PROJECTION_COUNT(amx_accumulator_stores);PROJECTION_COUNT(amx_dpbf16ps);
                PROJECTION_COUNT(amx_executed_flops);PROJECTION_COUNT(output_store_bytes);
                #undef PROJECTION_COUNT
                printf("\n");
                if(p.counters.source_visits!=g.e)throw std::runtime_error("source visit accounting mismatch");
            }
        }
    }
    // Diagnostic-only MKL decomposition: identical operations as source helper;
    // the baseline itself above remains the unmodified source function.
    for(int rep=0;rep<3;rep++) {
        double t0=omp_get_wtime();
        mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE,1.f,a,descr,SPARSE_LAYOUT_ROW_MAJOR,h0,DIM,DIM,0.f,z,DIM);
        double t1=omp_get_wtime();cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,g.n,DIM,DIM,1.f,z,DIM,w1,DIM,0.f,mh1,DIM);
        double t2=omp_get_wtime();::relu_f32(mh1,n);double t3=omp_get_wtime();
        mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE,1.f,a,descr,SPARSE_LAYOUT_ROW_MAJOR,mh1,DIM,DIM,0.f,z,DIM);
        double t4=omp_get_wtime();cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,g.n,DIM,DIM,1.f,z,DIM,w2,DIM,0.f,mh2,DIM);
        double t5=omp_get_wtime();
        printf("METHOD_MKL_STAGE graph=%s repeat=%d layer1_spmm_ms=%.12g layer1_gemm_ms=%.12g relu_ms=%.12g layer2_spmm_ms=%.12g layer2_gemm_ms=%.12g total_ms=%.12g\n",graph,rep,(t1-t0)*1000,(t2-t1)*1000,(t3-t2)*1000,(t4-t3)*1000,(t5-t4)*1000,(t5-t0)*1000);
    }
    for(size_t k=0;k<=methods.size();k++) {
        if(k<methods.size()&&methods[k].name!="paper_tfs"&&methods[k].name!="b64_fast"&&methods[k].name!="bfull_fast"&&methods[k].name!="b256_fast"&&methods[k].name!="s64_mfull_fast"&&methods[k].name!="s128_mfull_fast"&&methods[k].name!="s64_m256_fast"&&methods[k].name!="s64_mfull_accurate")continue;
        const char* name=k==methods.size()?"source_mkl_fp32":methods[k].name.c_str();
        for(int rep=0;rep<3;rep++) {
            run(k);gcn_extra_projection_pmu::Region region;region.start();double t=omp_get_wtime();run(k);double wall=(omp_get_wtime()-t)*1000;
            auto result=region.stop();gcn_extra_projection_pmu::print(result,graph,name,rep);
            printf("METHOD_PMU_WALL graph=%s method=%s repeat=%d ms=%.12g kind=diagnostic_not_primary\n",graph,name,rep,wall);
        }
    }
    structure_probe(g,perm,h0b,graph);
    rusage ru{};getrusage(RUSAGE_SELF,&ru);
    printf("METHOD_COMPLETE graph=%s methods=%zu checks=%zu pass=1 max_rss_kib=%ld\n",graph,methods.size(),methods.size()*4,ru.ru_maxrss);fflush(stdout);
}
}
