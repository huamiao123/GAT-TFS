#pragma once
#include "paper_methods_kernels.hpp"
#include <numeric>
#include <vector>
#include <algorithm>
#include <stdexcept>

static void run_full_sort_ablation(const csr_t& csr, const int* degree_perm,
    const uint16_t* h0, const uint16_t* w1, const uint16_t* w2,
    float* h1, uint16_t* h1b, uint16_t* h2, const char* graph,
    double degree_sort_ms) {
    const int n=csr.N;
    const size_t cells=size_t(n)*K_DIM;
    gcn_extra_paper::SourceGraphView g{n,csr.nnz,{csr.indptr},{csr.indices}};
    std::vector<int> identity(n);
    double t=omp_get_wtime();
    std::iota(identity.begin(),identity.end(),0);
    const double identity_setup_ms=(omp_get_wtime()-t)*1000;
    gcn_extra_paper::Method method{"bfull_fast",0,false,false,0,true};
    auto layer1=[&](const int* perm) {
        gcn_extra_paper::block_kernel<false,false>(g,h0,w1,h1,perm,method,nullptr);
    };
    auto forward=[&](const int* perm) {
        layer1(perm);
        relu_f32(h1,cells);
        convert_f32_to_bf16(h1,h1b,cells);
        gcn_extra_paper::block_kernel<false,true>(g,h1b,w2,h2,perm,method,nullptr);
    };
    layer1(degree_perm);
    std::vector<float> l1_ref(h1,h1+cells);
    layer1(identity.data());
    const bool l1_equal=memcmp(h1,l1_ref.data(),cells*sizeof(float))==0;
    forward(degree_perm);
    std::vector<uint16_t> l2_ref(h2,h2+cells);
    forward(identity.data());
    const bool l2_equal=memcmp(h2,l2_ref.data(),cells*sizeof(uint16_t))==0;
    printf("ABLATION_CHECK graph=%s l1_bitwise=%d l2_bitwise=%d pass=%d\n",graph,l1_equal,l2_equal,l1_equal&&l2_equal);
    fflush(stdout);
    if(!l1_equal||!l2_equal)throw std::runtime_error("FULL schedule changed output");
    auto structure=[&](const char* name,const int* perm) {
        uint64_t nonempty=0,padded_steps=0,active_steps=0;
        uint32_t max_degree=0;
        for(int i=0;i<n;i+=TR) {
            int batch=std::min(TR,n-i);uint32_t md=0;
            for(int r=0;r<batch;r++) {
                int row=perm[i+r],d=csr.indptr[row+1]-csr.indptr[row];
                md=std::max(md,uint32_t(d));max_degree=std::max(max_degree,uint32_t(d));
                active_steps+=d;
            }
            nonempty+=md>0;padded_steps+=uint64_t(md)*batch;
        }
        printf("ABLATION_WORK graph=%s schedule=%s nonempty_tiles=%llu padded_row_steps=%llu real_edges=%u max_degree=%u amx_dpbf16ps_per_layer=%llu\n",
            graph,name,(unsigned long long)nonempty,(unsigned long long)padded_steps,csr.nnz,max_degree,
            (unsigned long long)(nonempty*KB*NP*4));
    };
    structure("degree",degree_perm);structure("identity",identity.data());
    printf("ABLATION_SETUP graph=%s degree_sort_ms=%.9g identity_init_ms=%.9g included_in_prepared_e2e=0\n",graph,degree_sort_ms,identity_setup_ms);
    fflush(stdout);
    const int* perms[2]={degree_perm,identity.data()};
    const char* names[2]={"degree","identity"};
    for(int repeat=0;repeat<8;repeat++) {
        int first=(repeat%2==0)?0:1;
        for(int pos=0;pos<2;pos++) {
            int k=(pos==0)?first:1-first;
            forward(perms[k]);
            double start=omp_get_wtime();forward(perms[k]);double end=omp_get_wtime();
            printf("ABLATION_TIME graph=%s schedule=%s repeat=%d order=%d prepared_e2e_ms=%.9g\n",graph,names[k],repeat,pos,(end-start)*1000);
            fflush(stdout);
        }
    }
    for(int k=0;k<2;k++)for(int repeat=0;repeat<3;repeat++) {
        double a=omp_get_wtime();layer1(perms[k]);double b=omp_get_wtime();
        relu_f32(h1,cells);double c=omp_get_wtime();
        convert_f32_to_bf16(h1,h1b,cells);double d=omp_get_wtime();
        gcn_extra_paper::block_kernel<false,true>(g,h1b,w2,h2,perms[k],method,nullptr);double e=omp_get_wtime();
        printf("ABLATION_STAGE graph=%s schedule=%s repeat=%d layer1_ms=%.9g relu_ms=%.9g interlayer_ms=%.9g layer2_ms=%.9g total_ms=%.9g\n",
            graph,names[k],repeat,(b-a)*1000,(c-b)*1000,(d-c)*1000,(e-d)*1000,(e-a)*1000);
    }
    printf("ABLATION_COMPLETE graph=%s pass=1\n",graph);fflush(stdout);
}
