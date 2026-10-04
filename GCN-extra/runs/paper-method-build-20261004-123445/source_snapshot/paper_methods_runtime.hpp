#pragma once
#include "paper_methods_kernels.hpp"
#include <sstream>
#include <sys/resource.h>
namespace gcn_extra_paper {
struct Error { double max_abs,mean_abs,l2,nmax;bool finite; };
template<class A,class B> static Error error(const A* a,const B* b,size_t count) {
    double mx=0,scale=0,absolute=0,delta=0,ref=0;int bad=0;
    #pragma omp parallel for reduction(max:mx,scale) reduction(+:absolute,delta,ref,bad) schedule(static)
    for(size_t i=0;i<count;i++){
        double av,bv;
        if constexpr(std::is_same_v<A,uint16_t>)av=::bf16_to_f32(a[i]);else av=a[i];
        if constexpr(std::is_same_v<B,uint16_t>)bv=::bf16_to_f32(b[i]);else bv=b[i];
        double d=av-bv;
        if(!std::isfinite(av)||!std::isfinite(bv)){bad++;continue;}
        mx=std::max(mx,std::fabs(d));scale=std::max(scale,std::fabs(bv));
        absolute+=std::fabs(d);delta+=d*d;ref+=bv*bv;
    }
    return {mx,absolute/count,std::sqrt(delta/std::max(ref,1e-300)),mx/std::max(scale,1e-300),bad==0};
}
template<class A,class B> static void check(const char* graph,const char* method,const char* boundary,
    const char* ref,const A* a,const B* b,size_t count,double gate,bool exact=false){
    Error e=error(a,b,count);bool pass=e.finite&&(exact?e.max_abs==0:e.l2<gate&&e.nmax<gate);
    printf("METHOD_CHECK graph=%s method=%s boundary=%s reference=%s max_abs=%.12g mean_abs=%.12g relative_L2=%.12g normalized_max=%.12g gate=%.9g pass=%d\n",graph,method,boundary,ref,e.max_abs,e.mean_abs,e.l2,e.nmax,gate,pass);fflush(stdout);
    if(!pass)throw std::runtime_error("paper method numerical gate failed");
}
template<bool B16> static void layer(const SourceGraphView& g,const Method& m,const uint16_t* h,const uint16_t* w,
   std::conditional_t<B16,uint16_t,float>* out,const int* perm){
    if(m.name=="paper_tfs"){
        if constexpr(B16)::tfs_v3_bf16out(g.row.p,g.col.p,h,w,out,perm,g.n,64);
        else ::tfs_v3_fp32out(g.row.p,g.col.p,h,w,out,perm,g.n,64);
    }else block_kernel<false,B16>(g,h,w,out,perm,m,nullptr);
}
template<bool B16> static void forward(const SourceGraphView& g,const Method& m,const uint16_t* h0,const uint16_t* w1,const uint16_t* w2,
   float* h1,uint16_t* h1b,std::conditional_t<B16,uint16_t,float>* out,const int* perm){
    layer<false>(g,m,h0,w1,h1,perm);::relu_f32(h1,size_t(g.n)*DIM);
    ::convert_f32_to_bf16(h1,h1b,size_t(g.n)*DIM);layer<B16>(g,m,h1b,w2,out,perm);
}
static void run_methods(const csr_t& csr,const int* perm,const float* h0,const float* w1,const float* w2,
   const uint16_t* wv1,const uint16_t* wv2,const uint16_t* h0b,float* h1,uint16_t* h1b,uint16_t* h2b,float* h2f,
   sparse_matrix_t a,matrix_descr descr,float* z,float* mh1,float* mh2,const char* graph){
    SourceGraphView g{csr.N,csr.nnz,{csr.indptr},{csr.indices}};size_t count=size_t(g.n)*DIM;
    for(uint64_t edge=0;edge<g.e;edge++)if(csr.values[edge]!=1.f)throw std::runtime_error("Nonunit values: incomparable original operators");
    std::vector<Method> methods{{"paper_tfs",1,false,false,0,true}};
    std::stringstream tokens(getenv("GCN_PAPER_BLOCKS")?getenv("GCN_PAPER_BLOCKS"):"2,4,8,16,32,64,full");std::string token;
    while(std::getline(tokens,token,',')){
        int block=token=="full"?0:std::stoi(token);if(block<0||block>1048576)throw std::runtime_error("Invalid neighbor block size");
        for(bool accurate:{false,true})methods.push_back({"b"+token+(accurate?"_accurate":"_fast"),block,accurate,false,0,true});
    }
    printf("METHOD_CONFIG graph=%s N=%d E=%u D=128 F=128 R=64 TR=16 warmups=1 repeats=5 final_output=BF16 mkl_threads=%d mkl_dynamic=%d numa=default\n",graph,g.n,csr.nnz,mkl_get_max_threads(),mkl_get_dynamic());
    // Strict FP32 checks preserve the previous gates. BF16 checks allow one final
    // rounding bin in addition to these errors. FP32 MKL gate stays 0.03.
    {
        std::vector<float> final_fp32_ref(h2f,h2f+count);
        std::vector<uint16_t> final_bf16_ref(h2b,h2b+count);
        layer<false>(g,methods[0],h0b,wv1,h1,perm);
        std::vector<float> kernel_ref(h1,h1+count);
        for(const auto& m:methods){
            bool original=m.name=="paper_tfs";double gate=m.accurate?1e-3:1e-2;
            std::fill(h1,h1+count,std::numeric_limits<float>::quiet_NaN());
            layer<false>(g,m,h0b,wv1,h1,perm);
            check(graph,m.name.c_str(),"kernel_FP32","paper_tfs",h1,kernel_ref.data(),count,gate,original);
            forward<false>(g,m,h0b,wv1,wv2,h1,h1b,h2f,perm);
            check(graph,m.name.c_str(),"e2e_FP32_final","paper_tfs",h2f,final_fp32_ref.data(),count,gate,original);
            std::fill(h2b,h2b+count,uint16_t(0x7fc0));
            forward<true>(g,m,h0b,wv1,wv2,h1,h1b,h2b,perm);
            check(graph,m.name.c_str(),"e2e_BF16_final","paper_tfs",h2b,final_bf16_ref.data(),count,m.accurate?.01:.02,original);
            check(graph,m.name.c_str(),"e2e_BF16_final","source_MKL_FP32",h2b,mh2,count,.03);
        }
    }
    printf("METHOD_GATE_COMPLETE graph=%s methods=%zu pass=1\n",graph,methods.size());fflush(stdout);
    // Raw paper timing above is untouched. Supplemental fair comparisons rotate
    // every method and MKL in the same process, with identical original buffers.
    auto run=[&](size_t k,bool e2e){
        if(k==methods.size()){
            ::mkl_spmm_gemm(a,descr,h0,w1,z,mh1,g.n);
            if(e2e){::relu_f32(mh1,count);::mkl_spmm_gemm(a,descr,mh1,w2,z,mh2,g.n);}
        }else if(e2e)forward<true>(g,methods[k],h0b,wv1,wv2,h1,h1b,h2b,perm);
        else layer<false>(g,methods[k],h0b,wv1,h1,perm);
    };
    for(bool e2e:{false,true}){
        for(size_t k=0;k<=methods.size();k++)run(k,e2e);
        for(int rep=0;rep<5;rep++)for(size_t pos=0;pos<=methods.size();pos++){
            size_t k=(pos+rep)%(methods.size()+1);double t=omp_get_wtime();run(k,e2e);double ms=(omp_get_wtime()-t)*1000;
            printf("METHOD_TIME graph=%s method=%s kind=%s repeat=%d order=%zu ms=%.12g\n",graph,k==methods.size()?"source_mkl_fp32":methods[k].name.c_str(),e2e?"e2e":"kernel",rep,pos,ms);fflush(stdout);
        }
    }
    for(const auto& m:methods){
        for(int rep=0;rep<3;rep++){
            double t0=omp_get_wtime();layer<false>(g,m,h0b,wv1,h1,perm);double t1=omp_get_wtime();
            ::relu_f32(h1,count);double t2=omp_get_wtime();::convert_f32_to_bf16(h1,h1b,count);double t3=omp_get_wtime();
            layer<true>(g,m,h1b,wv2,h2b,perm);double t4=omp_get_wtime();
            printf("METHOD_STAGE graph=%s method=%s repeat=%d layer1_ms=%.12g relu_ms=%.12g interlayer_conversion_ms=%.12g layer2_ms=%.12g total_ms=%.12g\n",graph,m.name.c_str(),rep,(t1-t0)*1000,(t2-t1)*1000,(t3-t2)*1000,(t4-t3)*1000,(t4-t0)*1000);
        }
        if(m.name!="paper_tfs")for(int li=0;li<2;li++){
            Profile p;double t=omp_get_wtime();
            if(li==0)block_kernel<true,false>(g,h0b,wv1,h1,perm,m,&p);
            else {::relu_f32(h1,count);::convert_f32_to_bf16(h1,h1b,count);t=omp_get_wtime();block_kernel<true,true>(g,h1b,wv2,h2b,perm,m,&p);}
            double wall=(omp_get_wtime()-t)*1000;
            for(int ph=0;ph<PHASES;ph++)printf("METHOD_PROFILE graph=%s method=%s layer=%d phase=%s sampled_thread_ms=%.12g sampled_tiles=%llu wall_ms=%.12g\n",graph,m.name.c_str(),li+1,phase_name[ph],p.sec[ph]*1000,(unsigned long long)p.tiles,wall);
        }
    }
    rusage ru{};getrusage(RUSAGE_SELF,&ru);
    printf("METHOD_COMPLETE graph=%s methods=%zu checks=%zu pass=1 max_rss_kib=%ld\n",graph,methods.size(),methods.size()*4,ru.ru_maxrss);fflush(stdout);
}
}
