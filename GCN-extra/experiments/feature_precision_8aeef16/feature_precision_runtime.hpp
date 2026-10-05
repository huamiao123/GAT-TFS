#pragma once
// Hooks follow the frozen original TFS/MKL comparisons. Performance BF16
// calls retain the frozen kernel; only source feature storage varies in FP32.
#include "frozen_source/paper_methods_runtime.hpp"
#include "feature_precision_kernels.hpp"
#include "feature_precision_profile.hpp"
#include "feature_precision_pmu.hpp"
#include <fstream>
#include <memory>

namespace gcn_extra_feature_precision {
using gcn_extra_paper::DIM;
using gcn_extra_paper::SourceGraphView;
struct Method { const char* name; int block; bool fp32; };
struct Totals { size_t checks=0,equivalence=0,differences=0,profile_gates=0,times=0,stages=0; };

template<class T> struct Buffer {
    T* p; size_t n;
    explicit Buffer(size_t count):p(static_cast<T*>(mkl_malloc(count*sizeof(T),64))),n(count) {
        if(!p)throw std::bad_alloc();
    }
    ~Buffer(){mkl_free(p);}
    Buffer(const Buffer&)=delete;Buffer& operator=(const Buffer&)=delete;
};
static gcn_extra_paper::Method frozen_method(const Method& m) {
    return {m.name,m.block,false,false,0,true};
}
static gcn_extra_feature_fp32::Method float_method(const Method& m) {
    return {m.name,m.block,false,false,0,true};
}
static gcn_extra_feature_fp32::SourceGraphView float_view(const SourceGraphView& g) {
    return {g.n,g.e,{g.row.p},{g.col.p}};
}
template<bool B16> static void layer(const SourceGraphView& g,const Method& m,
    const float* hf,const uint16_t* hb,const uint16_t* w,
    std::conditional_t<B16,uint16_t,float>* out,const int* perm) {
    if(m.fp32)gcn_extra_feature_fp32::block_kernel<false,B16>(float_view(g),hf,w,out,perm,float_method(m),nullptr);
    else gcn_extra_paper::block_kernel<false,B16>(g,hb,w,out,perm,frozen_method(m),nullptr);
}
template<bool B16> static void forward(const SourceGraphView& g,const Method& m,
    const float* h0,const uint16_t* h0b,const uint16_t* w1,const uint16_t* w2,
    float* h1,uint16_t* h1b,std::conditional_t<B16,uint16_t,float>* out,const int* perm) {
    layer<false>(g,m,h0,h0b,w1,h1,perm);::relu_f32(h1,size_t(g.n)*DIM);
    if(!m.fp32)::convert_f32_to_bf16(h1,h1b,size_t(g.n)*DIM);
    layer<B16>(g,m,h1,h1b,w2,out,perm);
}
static void expand_bf16(const uint16_t* src,float* dst,size_t n) {
    // The buffer is allocated without zeroing. Its first touch uses exactly
    // the same logical element partition as the original BF16 conversion.
    #pragma omp parallel for schedule(static)
    for(size_t i=0;i<n;i++)dst[i]=::bf16_to_f32(src[i]);
}
template<class T> static void nanfill(T* dst,size_t n) {
    #pragma omp parallel for schedule(static)
    for(size_t i=0;i<n;i++) {
        if constexpr(std::is_same_v<T,uint16_t>)dst[i]=uint16_t(0x7fc0);
        else dst[i]=std::numeric_limits<float>::quiet_NaN();
    }
}
template<class T> static uint64_t checksum(const T* ptr,size_t n) {
    // Noncryptographic, index-sensitive checksum; reductions are order
    // independent and cover every element without a serial bytewise pass.
    uint64_t result=0;
    #pragma omp parallel for reduction(^:result) schedule(static)
    for(size_t i=0;i<n;i++) {
        uint32_t raw=0;
        if constexpr(std::is_same_v<T,uint16_t>)raw=ptr[i];
        else std::memcpy(&raw,ptr+i,sizeof(float));
        uint64_t v=uint64_t(raw)+(uint64_t(i)+1)*UINT64_C(0x9e3779b97f4a7c15);
        v=(v^(v>>30))*UINT64_C(0xbf58476d1ce4e5b9);
        v=(v^(v>>27))*UINT64_C(0x94d049bb133111eb);
        result^=v^(v>>31);
    }
    return result;
}
template<class A,class B> static void assess(const char* label,const char* graph,const char* method,
    const char* boundary,const char* reference,const A* actual,const B* expected,size_t n,
    double gate,bool exact,Totals& totals) {
    const auto e=gcn_extra_paper::error(actual,expected,n);
    bool bitwise=false;
    if constexpr(std::is_same_v<A,B>)bitwise=std::memcmp(actual,expected,n*sizeof(A))==0;
    bool pass=e.finite&&(exact?bitwise:(gate==0 || (e.l2<gate&&e.nmax<gate)));
    printf("PRECISION_%s graph=%s method=%s boundary=%s reference=%s max_abs=%.12g mean_abs=%.12g relative_L2=%.12g normalized_max=%.12g finite=%d bitwise=%d gate=%.9g gate_type=%s checksum_actual=%016llx checksum_reference=%016llx checksum_algorithm=indexed_splitmix64_xor pass=%d\n",
        label,graph,method,boundary,reference,e.max_abs,e.mean_abs,e.l2,e.nmax,e.finite,bitwise,gate,
        exact?"bitwise":gate==0?"report_only":"relative_L2_and_normalized_max",
        (unsigned long long)checksum(actual,n),(unsigned long long)checksum(expected,n),pass);fflush(stdout);
    if(std::strcmp(label,"CHECK")==0)totals.checks++;
    else if(std::strcmp(label,"EQ")==0)totals.equivalence++;
    else if(std::strcmp(label,"DIFF")==0)totals.differences++;
    else if(std::strcmp(label,"PROFILE_GATE")==0)totals.profile_gates++;
    if(!pass)throw std::runtime_error("feature storage numerical/bitwise gate failed");
}
static void buffer_record(const char* graph,const char* name,const char* dtype,const void* ptr,
    size_t bytes,const char* initialization) {
    printf("PRECISION_BUFFER graph=%s name=%s dtype=%s pointer=%p bytes=%llu initialization=%s\n",
        graph,name,dtype,ptr,(unsigned long long)bytes,initialization);
}
static void numa_snapshot(const char* graph,const char* phase) {
    std::string file=std::string("precision_numa_maps_")+phase+".txt";
    std::ifstream in("/proc/self/numa_maps");std::ofstream out(file);
    bool available=in.good()&&out.good();if(available)out<<in.rdbuf();
    printf("PRECISION_NUMA graph=%s phase=%s file=%s available=%d policy=source_unchanged\n",graph,phase,file.c_str(),available);
}

// Independent mathematical oracle. It mirrors CSR addition order, DegreeSort
// tile maximum, block boundaries, partial high-word truncation, and quantized
// row-major W. Dense accumulation uses AVX-512 FP32 FMA, never AMX.
template<bool FP32_INPUT,bool B16> static void reference_layer(const SourceGraphView& g,
    const std::conditional_t<FP32_INPUT,float,uint16_t>* source,const float* quantized_w,
    std::conditional_t<B16,uint16_t,float>* output,const int* perm,int block) {
    #pragma omp parallel for schedule(dynamic,1)
    for(int ti=0;ti<g.n;ti+=TR) {
        int batch=std::min(TR,g.n-ti);uint32_t md=0;
        for(int r=0;r<batch;r++){int row=perm[ti+r];md=std::max(md,g.row[row+1]-g.row[row]);}
        uint32_t step=block?uint32_t(block):std::max(1u,md);
        for(int r=0;r<batch;r++) {
            int row=perm[ti+r];uint32_t begin=g.row[row],degree=g.row[row+1]-begin;
            __m512 acc[8];for(auto& a:acc)a=_mm512_setzero_ps();
            alignas(64) float quantized_partial[DIM];
            for(uint32_t start=0;start<md;start+=step) {
                __m512 sum[8];for(auto& s:sum)s=_mm512_setzero_ps();
                uint32_t end=std::min(degree,start+step);
                for(uint32_t k=start;k<end;k++) {
                    const auto* src=source+size_t(g.col[begin+k])*DIM;
                    for(int f=0;f<8;f++) {
                        __m512 value;
                        if constexpr(FP32_INPUT)value=_mm512_loadu_ps(src+f*16);
                        else value=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(src+f*16))),16));
                        sum[f]=_mm512_add_ps(sum[f],value);
                    }
                }
                for(int f=0;f<8;f++) {
                    __m512i high=_mm512_slli_epi32(_mm512_srli_epi32(_mm512_castps_si512(sum[f]),16),16);
                    _mm512_store_ps(quantized_partial+f*16,_mm512_castsi512_ps(high));
                }
                for(int k=0;k<DIM;k++) {
                    __m512 q=_mm512_set1_ps(quantized_partial[k]);
                    for(int f=0;f<8;f++)acc[f]=_mm512_fmadd_ps(q,_mm512_loadu_ps(quantized_w+k*DIM+f*16),acc[f]);
                }
            }
            for(int f=0;f<8;f++) {
                if constexpr(B16) {
                    alignas(64) float values[16];_mm512_store_ps(values,acc[f]);
                    for(int k=0;k<16;k++)output[size_t(row)*DIM+f*16+k]=::f32_to_bf16(values[k]);
                }else _mm512_storeu_ps(output+size_t(row)*DIM+f*16,acc[f]);
            }
        }
    }
}
template<bool B16> static void reference_forward(const SourceGraphView& g,const Method& m,
    const float* h0,const uint16_t* h0b,const float* qw1,const float* qw2,
    float* h1,uint16_t* h1b,std::conditional_t<B16,uint16_t,float>* output,const int* perm) {
    if(m.fp32)reference_layer<true,false>(g,h0,qw1,h1,perm,m.block);
    else reference_layer<false,false>(g,h0b,qw1,h1,perm,m.block);
    ::relu_f32(h1,size_t(g.n)*DIM);
    if(m.fp32)reference_layer<true,B16>(g,h1,qw2,output,perm,m.block);
    else {
        ::convert_f32_to_bf16(h1,h1b,size_t(g.n)*DIM);
        reference_layer<false,B16>(g,h1b,qw2,output,perm,m.block);
    }
}
static void profile_records(const char* graph,const Method& m,int li,
    const gcn_extra_feature_profile::Profile& p,double wall) {
    if(p.counters.source_gather_requested_bytes != p.counters.source_visits*(m.fp32?512u:256u)
       ||p.counters.source_decode_elements != p.counters.source_visits*(m.fp32?0u:128u)
       ||p.counters.feature_adds != p.counters.source_visits*128u)
        throw std::runtime_error("Feature load/decode/reduction operation count mismatch");
    for(int ph=0;ph<gcn_extra_paper::PHASES;ph++)
        printf("PRECISION_PROFILE graph=%s method=%s layer=%d phase=%s sampled_thread_ms=%.12g sampled_tiles=%llu wall_ms=%.12g\n",
            graph,m.name,li,gcn_extra_paper::phase_name[ph],p.sec[ph]*1000,(unsigned long long)p.tiles,wall);
    for(int ph=0;ph<gcn_extra_feature_profile::DETAIL_PHASES;ph++)
        printf("PRECISION_DETAIL graph=%s method=%s layer=%d phase=%s sampled_thread_ms=%.12g clocked_sections=%llu wall_ms=%.12g\n",
            graph,m.name,li,gcn_extra_feature_profile::detail_phase_name[ph],p.detail_sec[ph]*1000,
            (unsigned long long)p.detail_events[ph],wall);
    for(const auto& t:p.threads)
        printf("PRECISION_THREAD graph=%s method=%s layer=%d thread=%d kernel_call=%llu setup_ms=%.12g active_ms=%.12g release_ms=%.12g start_offset_ms=%.12g finish_offset_ms=%.12g completed_tiles=%llu completed_edges=%llu\n",
            graph,m.name,li,t.thread_id,(unsigned long long)t.kernel_call,t.setup_s*1000,t.kernel_active_s*1000,t.release_s*1000,
            t.start_offset_s*1000,t.finish_offset_s*1000,(unsigned long long)t.completed_tiles,(unsigned long long)t.completed_edges);
    printf("PRECISION_COUNTERS graph=%s method=%s layer=%d units=logical_requests_not_DRAM",graph,m.name,li);
    #define PRECISION_COUNT(F) printf(" " #F "=%llu",(unsigned long long)p.counters.F)
    PRECISION_COUNT(destination_tiles);PRECISION_COUNT(projection_scopes);PRECISION_COUNT(neighbor_blocks);
    PRECISION_COUNT(row_block_visits);PRECISION_COUNT(nonempty_row_blocks);PRECISION_COUNT(source_visits);
    PRECISION_COUNT(feature_adds);PRECISION_COUNT(source_gather_requested_bytes);PRECISION_COUNT(source_decode_elements);
    PRECISION_COUNT(partial_zero_bytes);PRECISION_COUNT(partial_state_store_bytes);PRECISION_COUNT(csr_prefetch_instructions);
    PRECISION_COUNT(prefetch_requested_bytes);PRECISION_COUNT(partial_packed_bytes);PRECISION_COUNT(projection_panels);
    PRECISION_COUNT(amx_input_loads);PRECISION_COUNT(amx_a_loads);PRECISION_COUNT(amx_b_loads);
    PRECISION_COUNT(amx_accumulator_loads);PRECISION_COUNT(amx_accumulator_stores);PRECISION_COUNT(amx_dpbf16ps);
    PRECISION_COUNT(amx_executed_flops);PRECISION_COUNT(amx_reload_bytes);PRECISION_COUNT(amx_spill_bytes);PRECISION_COUNT(output_store_bytes);
    #undef PRECISION_COUNT
    printf("\n");fflush(stdout);
}

static void run_methods(const csr_t& csr,const int* perm,const float* h0,const float* w1,const float* w2,
    const uint16_t* wv1,const uint16_t* wv2,const uint16_t* h0b,float* h1,uint16_t* h1b,uint16_t* h2b,float* h2f,
    sparse_matrix_t a,matrix_descr descr,float* z,float* mh1,float* mh2,const char* graph) {
    SourceGraphView g{csr.N,csr.nnz,{csr.indptr},{csr.indices}};size_t n=size_t(g.n)*DIM;
    if(!n)throw std::runtime_error("Empty node set is outside frozen original harness");
    for(uint64_t e=0;e<g.e;e++)if(csr.values[e]!=1.f)throw std::runtime_error("Nonunit graph gives unequal frozen source TFS/MKL operators");
    const std::array<Method,4> methods{{{"b64_bf16",64,false},{"b64_fp32",64,true},{"full_bf16",0,false},{"full_fp32",0,true}}};
    Totals totals;
    #pragma omp parallel
    {
        cpu_set_t mask;CPU_ZERO(&mask);sched_getaffinity(0,sizeof(mask),&mask);
        std::string affinity;
        for(int c=0;c<CPU_SETSIZE;c++)if(CPU_ISSET(c,&mask))affinity+=(affinity.empty()?"":",")+std::to_string(c);
        #pragma omp critical
        printf("PRECISION_AFFINITY graph=%s omp_thread=%d tid=%ld cpu=%d allowed_cpus=%s\n",graph,omp_get_thread_num(),syscall(SYS_gettid),sched_getcpu(),affinity.c_str());
    }
    printf("PRECISION_CONFIG graph=%s N=%d E=%llu D=128 F=128 TR=16 R=64 source_commit=8aeef16 methods=4 anchors=2 precision=FAST final_output=BF16 warmups_per_measurement=1 native_repeats=10 matched_repeats=10 preparation_repeats=5 stage_repeats=5 order=FRRFFRRFFR mkl_threads=%d mkl_dynamic=%d numa=source_unchanged input_initialization=source_srand_12345_fp32_parallel_static_copy oracle=CSR_FP32_sum_truncateBF16_AVX512_FMA_quantizedW\n",
        graph,g.n,(unsigned long long)g.e,mkl_get_max_threads(),mkl_get_dynamic());
    buffer_record(graph,"h0","FP32",h0,n*4,"source_serial_rand");
    buffer_record(graph,"h0b","BF16",h0b,n*2,"source_parallel_static_conversion");
    Buffer<float> prepared_fp32(n);
    #pragma omp parallel for schedule(static)
    for(size_t i=0;i<n;i++)prepared_fp32.p[i]=h0[i];
    if(std::memcmp(prepared_fp32.p,h0,n*sizeof(float))!=0)
        throw std::runtime_error("FP32 preparation changed source H values");
    buffer_record(graph,"h0_prepared_fp32","FP32",prepared_fp32.p,n*4,"parallel_static_exact_copy_first_touch");
    buffer_record(graph,"h1","FP32",h1,n*4,"source_dynamic_degree_scatter");
    buffer_record(graph,"h1b","BF16",h1b,n*2,"source_parallel_static_conversion");
    Buffer<float> q(n);expand_bf16(h0b,q.p,n);
    buffer_record(graph,"qFloat","FP32",q.p,n*4,"lossless_bf16_expand_parallel_static_first_touch");
    numa_snapshot(graph,"before_checks");
    {
        std::vector<float> original_final(h2f,h2f+n);
        std::vector<uint16_t> original_bfinal(h2b,h2b+n);
        ::tfs_v3_fp32out(g.row.p,g.col.p,h0b,wv1,h1,perm,g.n,64);
        std::vector<float> original_kernel(h1,h1+n);
        Buffer<float> rh1(n),expected(n),cross_l1(n),cross_final(n);
        Buffer<uint16_t> rh1b(n),expected_b(n),cross_b(n);
        alignas(64) float qw1[DIM*DIM],qw2[DIM*DIM];
        for(int i=0;i<DIM*DIM;i++){qw1[i]=::bf16_to_f32(::f32_to_bf16(w1[i]));qw2[i]=::bf16_to_f32(::f32_to_bf16(w2[i]));}
        for(const auto& m:methods) {
            const float* initial=m.fp32?prepared_fp32.p:h0;
            nanfill(h1,n);layer<false>(g,m,initial,h0b,wv1,h1,perm);
            if(m.fp32)reference_layer<true,false>(g,h0,qw1,expected.p,perm,m.block);
            else reference_layer<false,false>(g,h0b,qw1,expected.p,perm,m.block);
            assess("CHECK",graph,m.name,"layer1_FP32","own_math_oracle",h1,expected.p,n,.01,false,totals);
            if(!m.fp32) {
                assess("CHECK",graph,m.name,"kernel_FP32","original_TFS",h1,original_kernel.data(),n,.01,false,totals);
                std::memcpy(cross_l1.p,h1,n*sizeof(float));
            }else assess("DIFF",graph,m.name,"layer1_FP32","BF16_same_B_native",h1,cross_l1.p,n,0,false,totals);

            ::relu_f32(h1,n);if(!m.fp32)::convert_f32_to_bf16(h1,h1b,n);
            nanfill(h2f,n);layer<false>(g,m,h1,h1b,wv2,h2f,perm);
            if(m.fp32)reference_layer<true,false>(g,h1,qw2,expected.p,perm,m.block);
            else reference_layer<false,false>(g,h1b,qw2,expected.p,perm,m.block);
            assess("CHECK",graph,m.name,"layer2_FP32_actual_input","own_math_oracle",h2f,expected.p,n,.01,false,totals);

            nanfill(h2f,n);forward<false>(g,m,initial,h0b,wv1,wv2,h1,h1b,h2f,perm);
            reference_forward<false>(g,m,h0,h0b,qw1,qw2,rh1.p,rh1b.p,expected.p,perm);
            assess("CHECK",graph,m.name,"e2e_FP32_final","own_math_oracle",h2f,expected.p,n,.01,false,totals);
            if(!m.fp32) {
                assess("CHECK",graph,m.name,"e2e_FP32_final","original_TFS",h2f,original_final.data(),n,.01,false,totals);
                std::memcpy(cross_final.p,h2f,n*sizeof(float));
            }else assess("DIFF",graph,m.name,"e2e_FP32_final","BF16_same_B_native",h2f,cross_final.p,n,0,false,totals);

            nanfill(h2b,n);forward<true>(g,m,initial,h0b,wv1,wv2,h1,h1b,h2b,perm);
            reference_forward<true>(g,m,h0,h0b,qw1,qw2,rh1.p,rh1b.p,expected_b.p,perm);
            assess("CHECK",graph,m.name,"e2e_BF16_final","own_math_oracle",h2b,expected_b.p,n,.02,false,totals);
            if(!m.fp32) {
                assess("CHECK",graph,m.name,"e2e_BF16_final","original_TFS",h2b,original_bfinal.data(),n,.02,false,totals);
                assess("CHECK",graph,m.name,"e2e_BF16_final","source_MKL_FP32",h2b,mh2,n,.03,false,totals);
                std::memcpy(cross_b.p,h2b,n*sizeof(uint16_t));
            }else assess("DIFF",graph,m.name,"e2e_BF16_final","BF16_same_B_native",h2b,cross_b.p,n,0,false,totals);

            if(!m.fp32) {
                Method fm{m.block?"b64_fp32":"full_fp32",m.block,true};
                expand_bf16(h0b,q.p,n);nanfill(h1,n);layer<false>(g,m,h0,h0b,wv1,h1,perm);
                nanfill(h2f,n);layer<false>(g,fm,q.p,h0b,wv1,h2f,perm);
                assess("EQ",graph,fm.name,"matched_layer1_FP32",m.name,h2f,h1,n,0,true,totals);
                ::relu_f32(h1,n);::convert_f32_to_bf16(h1,h1b,n);expand_bf16(h1b,q.p,n);
                nanfill(expected.p,n);layer<false>(g,m,h1,h1b,wv2,expected.p,perm);
                nanfill(h2f,n);layer<false>(g,fm,q.p,h1b,wv2,h2f,perm);
                assess("EQ",graph,fm.name,"matched_layer2_FP32",m.name,h2f,expected.p,n,0,true,totals);
                nanfill(expected_b.p,n);layer<true>(g,m,h1,h1b,wv2,expected_b.p,perm);
                nanfill(h2b,n);layer<true>(g,fm,q.p,h1b,wv2,h2b,perm);
                assess("EQ",graph,fm.name,"matched_layer2_BF16",m.name,h2b,expected_b.p,n,0,true,totals);
            }
        }
    }
    if(totals.checks!=24||totals.equivalence!=6||totals.differences!=6)throw std::runtime_error("Unexpected correctness coverage");
    printf("PRECISION_GATE_COMPLETE graph=%s checks=%zu bitwise_checks=%zu report_only_differences=%zu pass=1\n",graph,totals.checks,totals.equivalence,totals.differences);fflush(stdout);
    // The same immutable allocation is reused. Refresh qFloat after the layer2
    // matched-input checks, without changing the native source input buffers.
    expand_bf16(h0b,q.p,n);numa_snapshot(graph,"before_primary");
    auto run=[&](int k,bool e2e,bool matched) {
        if(k<4) {
            const auto& m=methods[k];const float* input=matched?q.p:prepared_fp32.p;
            if(e2e)forward<true>(g,m,input,h0b,wv1,wv2,h1,h1b,h2b,perm);
            else layer<false>(g,m,input,h0b,wv1,h1,perm);
        }else if(k==4) {
            ::tfs_v3_fp32out(g.row.p,g.col.p,h0b,wv1,h1,perm,g.n,64);
            if(e2e){::relu_f32(h1,n);::convert_f32_to_bf16(h1,h1b,n);::tfs_v3_bf16out(g.row.p,g.col.p,h1b,wv2,h2b,perm,g.n,64);}
        }else {
            ::mkl_spmm_gemm(a,descr,h0,w1,z,mh1,g.n);
            if(e2e){::relu_f32(mh1,n);::mkl_spmm_gemm(a,descr,mh1,w2,z,mh2,g.n);}
        }
    };
    const bool reverse_order[10]={false,true,true,false,false,true,true,false,false,true};
    const char* anchor_names[2]={"original_TFS","source_MKL_FP32"};
    for(bool e2e:{false,true})for(int rep=0;rep<10;rep++) {
        for(int pos=0;pos<6;pos++) {
            int k=pos<4?(reverse_order[rep]?3-pos:pos):pos;
            run(k,e2e,false);double t=omp_get_wtime();run(k,e2e,false);double ms=(omp_get_wtime()-t)*1000;
            printf("PRECISION_TIME graph=%s method=%s kind=%s repeat=%d orientation=%s order=%d ms=%.12g warmups=1\n",
                graph,k<4?methods[k].name:anchor_names[k-4],e2e?"e2e":"kernel",rep,reverse_order[rep]?"reverse":"forward",pos,ms);totals.times++;
        }
    }
    for(int rep=0;rep<10;rep++)for(int pos=0;pos<4;pos++) {
        int k=reverse_order[rep]?3-pos:pos;run(k,false,true);double t=omp_get_wtime();run(k,false,true);double ms=(omp_get_wtime()-t)*1000;
        printf("PRECISION_TIME graph=%s method=%s kind=matched_kernel repeat=%d orientation=%s order=%d ms=%.12g warmups=1 input_values=lossless_expanded_source_BF16\n",
            graph,methods[k].name,rep,reverse_order[rep]?"reverse":"forward",pos,ms);totals.times++;
    }
    {
        Buffer<uint16_t> prepared(n);
        auto prepared_run=[&](const Method& m) {
            if(m.fp32) {
                #pragma omp parallel for schedule(static)
                for(size_t i=0;i<n;i++)prepared_fp32.p[i]=h0[i];
            } else ::convert_f32_to_bf16(h0,prepared.p,n);
            forward<true>(g,m,m.fp32?prepared_fp32.p:h0,m.fp32?h0b:prepared.p,wv1,wv2,h1,h1b,h2b,perm);
        };
        for(int rep=0;rep<5;rep++)for(int pos=0;pos<4;pos++) {
            int k=rep%2?3-pos:pos;const auto& m=methods[k];prepared_run(m);
            double t=omp_get_wtime();prepared_run(m);double ms=(omp_get_wtime()-t)*1000;
            printf("PRECISION_TIME graph=%s method=%s kind=preparation_inclusive repeat=%d orientation=%s order=%d ms=%.12g warmups=1 initial_conversion=%s fp32_copy=%d\n",
                graph,m.name,rep,rep%2?"reverse":"forward",pos,ms,m.fp32?"parallel_static_fp32_copy":"source_FP32_to_BF16",int(m.fp32));totals.times++;
        }
    }
    for(const auto& m:methods)for(int rep=0;rep<5;rep++) {
        const float* initial=m.fp32?prepared_fp32.p:h0;
        forward<true>(g,m,initial,h0b,wv1,wv2,h1,h1b,h2b,perm);
        double t0=omp_get_wtime();layer<false>(g,m,initial,h0b,wv1,h1,perm);double t1=omp_get_wtime();
        ::relu_f32(h1,n);double t2=omp_get_wtime();double t3=t2;
        if(!m.fp32){::convert_f32_to_bf16(h1,h1b,n);t3=omp_get_wtime();}
        layer<true>(g,m,h1,h1b,wv2,h2b,perm);double t4=omp_get_wtime();
        printf("PRECISION_STAGE graph=%s method=%s repeat=%d layer1_ms=%.12g relu_ms=%.12g interlayer_conversion_ms=%.12g layer2_ms=%.12g total_ms=%.12g conversion_bypassed=%d\n",
            graph,m.name,rep,(t1-t0)*1000,(t2-t1)*1000,m.fp32?0:(t3-t2)*1000,(t4-t3)*1000,(t4-t0)*1000,m.fp32);totals.stages++;
    }
    {
        Buffer<float> expected(n);Buffer<uint16_t> expected_b(n);
        for(const auto& m:methods) {
            const float* initial=m.fp32?prepared_fp32.p:h0;
            layer<false>(g,m,initial,h0b,wv1,expected.p,perm);nanfill(h1,n);
            gcn_extra_feature_profile::Profile p;double t=omp_get_wtime();
            if(m.fp32)gcn_extra_feature_profile::kernel<true,false>(g,prepared_fp32.p,wv1,h1,perm,frozen_method(m),&p);
            else gcn_extra_feature_profile::kernel<false,false>(g,h0b,wv1,h1,perm,frozen_method(m),&p);
            double wall=(omp_get_wtime()-t)*1000;
            assess("PROFILE_GATE",graph,m.name,"layer1_FP32","uninstrumented_same_method",h1,expected.p,n,0,true,totals);
            profile_records(graph,m,1,p,wall);
            ::relu_f32(h1,n);if(!m.fp32)::convert_f32_to_bf16(h1,h1b,n);
            layer<true>(g,m,h1,h1b,wv2,expected_b.p,perm);nanfill(h2b,n);
            p=gcn_extra_feature_profile::Profile{};t=omp_get_wtime();
            if(m.fp32)gcn_extra_feature_profile::kernel<true,true>(g,h1,wv2,h2b,perm,frozen_method(m),&p);
            else gcn_extra_feature_profile::kernel<false,true>(g,h1b,wv2,h2b,perm,frozen_method(m),&p);
            wall=(omp_get_wtime()-t)*1000;
            assess("PROFILE_GATE",graph,m.name,"layer2_BF16","uninstrumented_same_method",h2b,expected_b.p,n,0,true,totals);
            profile_records(graph,m,2,p,wall);
        }
    }
    // Separate PMU regions cannot enter the primary performance sample set.
    // Each set is a different pass; shared uncore traffic is not attributed.
    for(auto set:{gcn_extra_feature_pmu::GroupSet::BASIC,gcn_extra_feature_pmu::GroupSet::CACHE,
                  gcn_extra_feature_pmu::GroupSet::TLB,gcn_extra_feature_pmu::GroupSet::STALL})
        for(int rep=0;rep<3;rep++)for(int pos=0;pos<4;pos++) {
            int k=rep%2?3-pos:pos;
            run(k,true,false);
            gcn_extra_feature_pmu::Region region(set);region.start();
            run(k,true,false);
            auto result=region.stop();gcn_extra_feature_pmu::print(result,graph,methods[k].name,rep);
        }
    numa_snapshot(graph,"after_diagnostics");
    if(totals.times!=180||totals.stages!=20||totals.profile_gates!=8)throw std::runtime_error("Unexpected measurement coverage");
    rusage ru{};getrusage(RUSAGE_SELF,&ru);
    printf("PRECISION_COMPLETE graph=%s methods=4 anchors=2 checks=%zu bitwise_checks=%zu report_only_differences=%zu profile_bitwise_checks=%zu measured_repetitions=%zu stage_repetitions=%zu max_rss_kib=%ld pass=1\n",
        graph,totals.checks,totals.equivalence,totals.differences,totals.profile_gates,totals.times,totals.stages,ru.ru_maxrss);fflush(stdout);
}
}
