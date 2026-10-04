#pragma once
#include "source_protocol_kernels.hpp"
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <sys/resource.h>

namespace gcn_extra_source {
struct NumericalError { double maximum,mean,relative_l2,normalized_max; bool finite; };
static NumericalError numerical_error(const float* a,const float* b,size_t count) {
    double mx=0,scale=0,absolute=0,delta=0,reference=0;int bad=0;
    #pragma omp parallel for reduction(max:mx,scale) reduction(+:absolute,delta,reference,bad) schedule(static)
    for(size_t i=0;i<count;i++) {
        double av=a[i],bv=b[i],d=av-bv;
        if(!std::isfinite(av)||!std::isfinite(bv)){bad++;continue;}
        mx=std::max(mx,std::fabs(d));scale=std::max(scale,std::fabs(bv));
        absolute+=std::fabs(d);delta+=d*d;reference+=bv*bv;
    }
    return {mx,absolute/count,std::sqrt(delta/std::max(reference,1e-300)),mx/std::max(scale,1e-300),bad==0};
}
static void emit_error(const char* graph,const Method& m,const char* boundary,const char* ref,
                       const float* a,const float* b,size_t count,double gate,bool identity=false) {
    auto e=numerical_error(a,b,count);
    bool pass=e.finite && (identity?e.maximum==0:(e.relative_l2<gate && e.normalized_max<gate));
    printf("SOURCE_CORRECT graph=%s method=%s boundary=%s reference=%s max_abs=%.12g mean_abs=%.12g relative_l2=%.12g normalized_max=%.12g pass=%d\n",
           graph,m.name.c_str(),boundary,ref,e.maximum,e.mean,e.relative_l2,e.normalized_max,pass);
    fflush(stdout);if(!pass)throw std::runtime_error("source protocol numerical gate failed");
}
static void run_kernel(const SourceGraphView& g,const Method& m,const uint16_t* hb,
                       const uint16_t* w,float* out,const int* perm) {
    if(m.name=="original_nozero")tfs_v3_kernel_nozero(g.row.p,g.col.p,hb,w,out,perm,g.n,64);
    else block_kernel<false>(g,hb,w,out,perm,m,nullptr);
}
static void forward(const SourceGraphView& g,const Method& m,const uint16_t* h0,
                    const uint16_t* w1,const uint16_t* w2,float* h1,float* h2,
                    uint16_t* h1b,const int* perm) {
    run_kernel(g,m,h0,w1,h1,perm);
    ::relu_f32(h1,size_t(g.n)*DIM);
    ::convert_f32_to_bf16(h1,h1b,size_t(g.n)*DIM);
    run_kernel(g,m,h1b,w2,h2,perm);
}
static uint64_t fingerprint(const void* pointer,size_t bytes) {
    uint64_t h=1469598103934665603ULL;auto p=(const uint8_t*)pointer;
    for(size_t i=0;i<bytes;i++){h^=p[i];h*=1099511628211ULL;}return h;
}
static void run_source_candidates(const csr_t& csr,const int* perm,const float* h0,
                                 const float* w1,const float* w2,const uint16_t* wv1,
                                 const uint16_t* wv2,uint16_t* h0b,float* h1,float* h2,
                                 uint16_t* h1b,sparse_matrix_t a,matrix_descr descr,
                                 float* z,float* mkl_h1,float* mkl_h2,const char* graph) {
    SourceGraphView g{csr.N,csr.nnz,{csr.indptr},{csr.indices}};size_t count=size_t(g.n)*DIM;
    for(uint64_t edge=0;edge<g.e;edge++)if(csr.values[edge]!=1.0f)
        throw std::runtime_error("Nonunit values: original TFS and source MKL are different operators");
    std::vector<Method> methods{{"original_nozero",1,false,true,0,true}};
    const char* setting=getenv("GCN_SOURCE_BLOCKS");std::stringstream tokens(setting?setting:"2,4,8,16,32,64,full");std::string token;
    while(std::getline(tokens,token,',')) {
        int b=token=="full"?0:std::stoi(token);
        if(b<0 || b>1048576)throw std::runtime_error("Bad source protocol block size");
        for(bool accurate:{false,true})methods.push_back({"shared_b"+token+(accurate?"_accurate_nozero":"_fast_nozero"),b,accurate,false,0,true});
    }
    printf("SOURCE_CONFIG graph=%s N=%d E=%u D=128 F=128 R=64 TR=16 repeats=5 warmups=1 mkl_max_threads=%d mkl_dynamic=%d policy=default_no_numactl\n",
           graph,g.n,csr.nnz,mkl_get_max_threads(),mkl_get_dynamic());
    printf("SOURCE_INPUT graph=%s H0=%llu W1=%llu W2=%llu\n",graph,
           (unsigned long long)fingerprint(h0,count*4),(unsigned long long)fingerprint(w1,DIM*DIM*4),(unsigned long long)fingerprint(w2,DIM*DIM*4));
    // Reference buffers exist only during checks, after unchanged source MKL timing.
    // Candidate performance reuses the original TFS H1/H2/H1_bf16 allocations.
    {
        std::vector<float> e2e_reference(h2,h2+count);
        ::tfs_v3_kernel(csr.indptr,csr.indices,h0b,wv1,h1,perm,g.n,64);
        std::vector<float> kernel_reference(h1,h1+count);
        for(const Method& m:methods) {
            double gate=m.name=="original_nozero"?0:(m.accurate?1e-3:1e-2);
            std::fill(h1,h1+count,std::numeric_limits<float>::quiet_NaN());
            run_kernel(g,m,h0b,wv1,h1,perm);
            emit_error(graph,m,"kernel","source_original_tfs",h1,kernel_reference.data(),count,gate,m.name=="original_nozero");
            forward(g,m,h0b,wv1,wv2,h1,h2,h1b,perm);
            emit_error(graph,m,"e2e","source_original_tfs",h2,e2e_reference.data(),count,gate,m.name=="original_nozero");
            emit_error(graph,m,"e2e","source_mkl_fp32",h2,mkl_h2,count,0.03);
        }
    }
    // Single-layer MKL uses the literal original helper, unchanged FP32 inputs.
    ::mkl_spmm_gemm(a,descr,h0,w1,z,mkl_h1,g.n);
    for(int r=0;r<5;r++) {
        double t=omp_get_wtime();::mkl_spmm_gemm(a,descr,h0,w1,z,mkl_h1,g.n);double elapsed=(omp_get_wtime()-t)*1000;
        printf("SOURCE_TIME graph=%s method=source_mkl_fp32 kind=kernel repeat=%d ms=%.12g\n",graph,r,elapsed);
    }
    ::tfs_v3_kernel(csr.indptr,csr.indices,h0b,wv1,h1,perm,g.n,64);
    for(int r=0;r<5;r++) {
        double t=omp_get_wtime();::tfs_v3_kernel(csr.indptr,csr.indices,h0b,wv1,h1,perm,g.n,64);double elapsed=(omp_get_wtime()-t)*1000;
        printf("SOURCE_TIME graph=%s method=source_original_tfs kind=kernel repeat=%d ms=%.12g\n",graph,r,elapsed);
    }
    for(const Method& m:methods) {
        run_kernel(g,m,h0b,wv1,h1,perm);
        for(int r=0;r<5;r++) {
            double t=omp_get_wtime();run_kernel(g,m,h0b,wv1,h1,perm);double elapsed=(omp_get_wtime()-t)*1000;
            printf("SOURCE_TIME graph=%s method=%s kind=kernel repeat=%d ms=%.12g\n",graph,m.name.c_str(),r,elapsed);fflush(stdout);
        }
        forward(g,m,h0b,wv1,wv2,h1,h2,h1b,perm);
        for(int r=0;r<5;r++) {
            // The original E2E source reconverts H0 before t0 on every TFS repetition.
            ::convert_f32_to_bf16(h0,h0b,count);
            double t=omp_get_wtime();forward(g,m,h0b,wv1,wv2,h1,h2,h1b,perm);double elapsed=(omp_get_wtime()-t)*1000;
            printf("SOURCE_TIME graph=%s method=%s kind=e2e repeat=%d ms=%.12g\n",graph,m.name.c_str(),r,elapsed);fflush(stdout);
        }
        for(int r=0;r<3;r++) {
            double t0=omp_get_wtime();run_kernel(g,m,h0b,wv1,h1,perm);double t1=omp_get_wtime();
            ::relu_f32(h1,count);double t2=omp_get_wtime();::convert_f32_to_bf16(h1,h1b,count);double t3=omp_get_wtime();
            run_kernel(g,m,h1b,wv2,h2,perm);double t4=omp_get_wtime();
            printf("SOURCE_STAGE graph=%s method=%s repeat=%d layer1_ms=%.12g relu_ms=%.12g conversion_ms=%.12g layer2_ms=%.12g total_ms=%.12g\n",
                   graph,m.name.c_str(),r,(t1-t0)*1000,(t2-t1)*1000,(t3-t2)*1000,(t4-t3)*1000,(t4-t0)*1000);
        }
        if(m.name!="original_nozero") {
            Profile profile;double t=omp_get_wtime();block_kernel<true>(g,h0b,wv1,h1,perm,m,&profile);double elapsed=(omp_get_wtime()-t)*1000;
            for(int phase=0;phase<PHASES;phase++)printf("SOURCE_PROFILE graph=%s method=%s phase=%s sampled_thread_ms=%.12g sampled_tiles=%llu profile_wall_ms=%.12g\n",
                graph,m.name.c_str(),phase_name[phase],profile.sec[phase]*1000,(unsigned long long)profile.tiles,elapsed);
        }
    }
    rusage usage{};getrusage(RUSAGE_SELF,&usage);
    printf("SOURCE_COMPLETE graph=%s pass=1 max_rss_kib=%ld\n",graph,usage.ru_maxrss);fflush(stdout);
}
}
