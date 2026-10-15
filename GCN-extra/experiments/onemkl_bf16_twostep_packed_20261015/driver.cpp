// Original TFS functions are compiled verbatim in this translation unit.
#define main original_gcn_e2e_v3_main
#include "original.cpp"
#undef main
#include "paper_methods_kernels.hpp"
#include <algorithm>
#include <array>
#include <vector>
#include <limits>
#include <stdexcept>

struct Bf16Stages {
    double spmm1=0, z_to_bf16_1=0, gemm1=0, relu=0, interlayer_to_bf16=0;
    double interlayer_to_fp32=0, spmm2=0, z_to_bf16_2=0, gemm2=0, final_to_bf16=0;
    double total() const { return spmm1+z_to_bf16_1+gemm1+relu+interlayer_to_bf16+
        interlayer_to_fp32+spmm2+z_to_bf16_2+gemm2+final_to_bf16; }
};

static void decode_bf16(const uint16_t *src,float *dst,size_t n) {
    #pragma omp parallel for schedule(static)
    for(size_t i=0;i<n;i++) dst[i]=bf16_to_f32(src[i]);
}

static void* checked_alloc(size_t bytes) {
    void *p=aligned_alloc(64,(bytes+63)&~size_t(63));
    if(!p) throw std::bad_alloc();
    return p;
}

static double median(std::vector<double> v) {
    std::sort(v.begin(),v.end());
    return v[v.size()/2];
}

static void error_stats(const uint16_t *got,const uint16_t *ref,size_t n,
                        double &max_abs,double &rel_l2,double &norm_max,int &nonfinite) {
    double sq=0,ref_sq=0,max_ref=0;max_abs=0;nonfinite=0;
    for(size_t i=0;i<n;i++) {
        double a=bf16_to_f32(got[i]),b=bf16_to_f32(ref[i]);
        if(!std::isfinite(a)||!std::isfinite(b)){nonfinite++;continue;}
        double d=std::abs(a-b);max_abs=std::max(max_abs,d);
        max_ref=std::max(max_ref,std::abs(b));sq+=d*d;ref_sq+=b*b;
    }
    rel_l2=std::sqrt(sq/std::max(ref_sq,1e-300));
    norm_max=max_abs/std::max(max_ref,1e-300);
}

static void run_graph(const char *base,const char *name,const char *mode) {
    char path[512];snprintf(path,sizeof(path),"%s/%s/%s.csrbin",base,name,name);
    csr_t csr=load_csrbin(path);const int n=csr.N;const size_t cells=size_t(n)*K_DIM;
    for(uint32_t e=0;e<csr.nnz;e++) if(csr.values[e]!=1.0f)
        throw std::runtime_error("Original TFS requires a unit-weight graph");
    printf("GRAPH name=%s n=%d edges=%u dim=%d\n",name,n,csr.nnz,K_DIM);fflush(stdout);
    srand(12345);
    float *w1=(float*)checked_alloc(size_t(K_DIM)*K_DIM*4);
    float *w2=(float*)checked_alloc(size_t(K_DIM)*K_DIM*4);
    float *h0=(float*)checked_alloc(cells*4);
    for(int i=0;i<K_DIM*K_DIM;i++){w1[i]=0.01f*((rand()%200)-100);w2[i]=0.01f*((rand()%200)-100);}
    for(size_t i=0;i<cells;i++)h0[i]=0.01f*((rand()%200)-100);
    uint16_t *h0b=(uint16_t*)checked_alloc(cells*2),*w1b=(uint16_t*)checked_alloc(size_t(K_DIM)*K_DIM*2);
    uint16_t *w2b=(uint16_t*)checked_alloc(size_t(K_DIM)*K_DIM*2);
    uint16_t *wv1=(uint16_t*)checked_alloc(size_t(KB)*NB*16*32*2);
    uint16_t *wv2=(uint16_t*)checked_alloc(size_t(KB)*NB*16*32*2);
    float *h0q=(float*)checked_alloc(cells*4),*h1=(float*)checked_alloc(cells*4);
    float *h1q=(float*)checked_alloc(cells*4),*h2=(float*)checked_alloc(cells*4),*z=(float*)checked_alloc(cells*4);
    uint16_t *h1b=(uint16_t*)checked_alloc(cells*2),*h2b=(uint16_t*)checked_alloc(cells*2);
    uint16_t *zb=(uint16_t*)checked_alloc(cells*2),*reference=(uint16_t*)checked_alloc(cells*2);
    int *perm=make_degree_perm(csr.indptr,n);
    convert_f32_to_bf16(h0,h0b,cells);decode_bf16(h0b,h0q,cells);
    for(int i=0;i<K_DIM*K_DIM;i++){w1b[i]=f32_to_bf16(w1[i]);w2b[i]=f32_to_bf16(w2[i]);}
    const size_t packed_bytes=cblas_gemm_bf16bf16f32_pack_get_size(CblasBMatrix,n,K_DIM,K_DIM);
    if(packed_bytes==0)throw std::runtime_error("oneMKL BF16 B packing unavailable");
    MKL_BF16 *w1packed=(MKL_BF16*)checked_alloc(packed_bytes);
    MKL_BF16 *w2packed=(MKL_BF16*)checked_alloc(packed_bytes);
    cblas_gemm_bf16bf16f32_pack(CblasRowMajor,CblasBMatrix,CblasNoTrans,n,K_DIM,K_DIM,
        (const MKL_BF16*)w1b,K_DIM,w1packed);
    cblas_gemm_bf16bf16f32_pack(CblasRowMajor,CblasBMatrix,CblasNoTrans,n,K_DIM,K_DIM,
        (const MKL_BF16*)w2b,K_DIM,w2packed);
    make_W_vnni(w1,wv1);make_W_vnni(w2,wv2);
    MKL_INT *rs=(MKL_INT*)checked_alloc(size_t(n)*sizeof(MKL_INT));
    MKL_INT *re=(MKL_INT*)checked_alloc(size_t(n)*sizeof(MKL_INT));
    MKL_INT *ci=(MKL_INT*)checked_alloc(size_t(csr.nnz)*sizeof(MKL_INT));
    for(int i=0;i<n;i++){rs[i]=csr.indptr[i];re[i]=csr.indptr[i+1];}
    for(uint32_t i=0;i<csr.nnz;i++)ci[i]=csr.indices[i];
    sparse_matrix_t a;matrix_descr descr{};descr.type=SPARSE_MATRIX_TYPE_GENERAL;
    if(mkl_sparse_s_create_csr(&a,SPARSE_INDEX_BASE_ZERO,n,n,rs,re,ci,csr.values)!=SPARSE_STATUS_SUCCESS)
        throw std::runtime_error("mkl_sparse_s_create_csr failed");
    mkl_sparse_set_mm_hint(a,SPARSE_OPERATION_NON_TRANSPOSE,descr,SPARSE_LAYOUT_ROW_MAJOR,K_DIM,10);
    mkl_sparse_optimize(a);
    const gcn_extra_paper::SourceGraphView g{n,csr.nnz,{csr.indptr},{csr.indices}};
    const gcn_extra_paper::Method full{"bfull_fast",0,false,false,0,true};
    const std::vector<int> identity=[](int n){std::vector<int> v(n);for(int i=0;i<n;i++)v[i]=i;return v;}(n);
    auto tfs=[&]{tfs_v3_fp32out(csr.indptr,csr.indices,h0b,wv1,h1,perm,n,64);
        relu_f32(h1,cells);convert_f32_to_bf16(h1,h1b,cells);
        tfs_v3_bf16out(csr.indptr,csr.indices,h1b,wv2,h2b,perm,n,64);};
    auto full_forward=[&](const int *schedule){
        gcn_extra_paper::block_kernel<false,false>(g,h0b,wv1,h1,schedule,full,nullptr);
        relu_f32(h1,cells);convert_f32_to_bf16(h1,h1b,cells);
        gcn_extra_paper::block_kernel<false,true>(g,h1b,wv2,h2b,schedule,full,nullptr);};
    auto mkl_f32=[&]{mkl_spmm_gemm(a,descr,h0,w1,z,h1,n);relu_f32(h1,cells);
        mkl_spmm_gemm(a,descr,h1,w2,z,h2,n);convert_f32_to_bf16(h2,h2b,cells);};
    auto bf16_mkl=[&](Bf16Stages *s,bool packed){
        auto timed=[&](double &field,auto&& op){double t=omp_get_wtime();op();field=(omp_get_wtime()-t)*1000;};
        Bf16Stages tmp;Bf16Stages &x=s?*s:tmp;
        timed(x.spmm1,[&]{mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE,1.0f,a,descr,
            SPARSE_LAYOUT_ROW_MAJOR,h0q,K_DIM,K_DIM,0.0f,z,K_DIM);});
        timed(x.z_to_bf16_1,[&]{convert_f32_to_bf16(z,zb,cells);});
        timed(x.gemm1,[&]{if(packed)cblas_gemm_bf16bf16f32_compute(CblasRowMajor,CblasNoTrans,CblasPacked,
            n,K_DIM,K_DIM,1.0f,(const MKL_BF16*)zb,K_DIM,w1packed,K_DIM,0.0f,h1,K_DIM);
            else cblas_gemm_bf16bf16f32(CblasRowMajor,CblasNoTrans,CblasNoTrans,
            n,K_DIM,K_DIM,1.0f,(const MKL_BF16*)zb,K_DIM,(const MKL_BF16*)w1b,K_DIM,0.0f,h1,K_DIM);});
        timed(x.relu,[&]{relu_f32(h1,cells);});
        timed(x.interlayer_to_bf16,[&]{convert_f32_to_bf16(h1,h1b,cells);});
        timed(x.interlayer_to_fp32,[&]{decode_bf16(h1b,h1q,cells);});
        timed(x.spmm2,[&]{mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE,1.0f,a,descr,
            SPARSE_LAYOUT_ROW_MAJOR,h1q,K_DIM,K_DIM,0.0f,z,K_DIM);});
        timed(x.z_to_bf16_2,[&]{convert_f32_to_bf16(z,zb,cells);});
        timed(x.gemm2,[&]{if(packed)cblas_gemm_bf16bf16f32_compute(CblasRowMajor,CblasNoTrans,CblasPacked,
            n,K_DIM,K_DIM,1.0f,(const MKL_BF16*)zb,K_DIM,w2packed,K_DIM,0.0f,h2,K_DIM);
            else cblas_gemm_bf16bf16f32(CblasRowMajor,CblasNoTrans,CblasNoTrans,
            n,K_DIM,K_DIM,1.0f,(const MKL_BF16*)zb,K_DIM,(const MKL_BF16*)w2b,K_DIM,0.0f,h2,K_DIM);});
        timed(x.final_to_bf16,[&]{convert_f32_to_bf16(h2,h2b,cells);});
    };
    if(strcmp(mode,"bf16_only")==0){
        bf16_mkl(nullptr,false);
        for(int rep=0;rep<5;rep++){Bf16Stages st;bf16_mkl(&st,false);
            printf("BF16_ONLY graph=%s repeat=%d total_ms=%.9g spmm1=%.9g gemm1=%.9g spmm2=%.9g gemm2=%.9g checksum=%u\n",
                name,rep,st.total(),st.spmm1,st.gemm1,st.spmm2,st.gemm2,h2b[0]);fflush(stdout);}
    } else {
        tfs();memcpy(reference,h2b,cells*2);
        bf16_mkl(nullptr,false);double max_abs,rel_l2,norm_max;int nonfinite;
        error_stats(h2b,reference,cells,max_abs,rel_l2,norm_max,nonfinite);
        bool pass=nonfinite==0&&rel_l2<=0.03&&norm_max<=0.03;
        printf("CHECK graph=%s method=bf16_mkl_vs_tfs rel_l2=%.9g norm_max=%.9g max_abs=%.9g nonfinite=%d pass=%d\n",
            name,rel_l2,norm_max,max_abs,nonfinite,pass);fflush(stdout);
        if(!pass)throw std::runtime_error("BF16 oneMKL failed original paper numerical gate");
        bf16_mkl(nullptr,true);
        error_stats(h2b,reference,cells,max_abs,rel_l2,norm_max,nonfinite);
        pass=nonfinite==0&&rel_l2<=0.03&&norm_max<=0.03;
        printf("CHECK graph=%s method=bf16_mkl_packed_vs_tfs rel_l2=%.9g norm_max=%.9g max_abs=%.9g nonfinite=%d pass=%d\n",
            name,rel_l2,norm_max,max_abs,nonfinite,pass);fflush(stdout);
        if(!pass)throw std::runtime_error("Packed BF16 oneMKL failed original paper numerical gate");
        full_forward(perm);memcpy(reference,h2b,cells*2);
        full_forward(identity.data());
        bool exact=memcmp(reference,h2b,cells*2)==0;
        printf("CHECK graph=%s method=full_identity_vs_degree bitwise=%d\n",name,exact);fflush(stdout);
        if(!exact)throw std::runtime_error("FULL schedule changed output");
        const char *names[]={"original_tfs","full_degree","full_identity","bf16_mkl_twostep","bf16_mkl_packed_twostep","fp32_mkl_twostep"};
        std::array<std::vector<double>,6> times;
        auto call=[&](int k,Bf16Stages *s){
            if(k==0)tfs();else if(k==1)full_forward(perm);else if(k==2)full_forward(identity.data());
            else if(k==3)bf16_mkl(s,false);else if(k==4)bf16_mkl(s,true);else mkl_f32();};
        for(int k=0;k<6;k++)call(k,nullptr);
        for(int rep=0;rep<5;rep++)for(int order=0;order<6;order++) {
            int k=(rep+order)%6;Bf16Stages st;double begin=omp_get_wtime();call(k,(k==3||k==4)?&st:nullptr);
            double ms=(omp_get_wtime()-begin)*1000;times[k].push_back(ms);
            printf("TIME graph=%s method=%s repeat=%d order=%d e2e_ms=%.9g\n",name,names[k],rep,order,ms);
            if(k==3||k==4)printf("STAGE_BF16 graph=%s method=%s repeat=%d spmm1=%.9g z_to_bf16_1=%.9g gemm1=%.9g relu=%.9g interlayer_to_bf16=%.9g interlayer_to_fp32=%.9g spmm2=%.9g z_to_bf16_2=%.9g gemm2=%.9g final_to_bf16=%.9g\n",
                name,names[k],rep,st.spmm1,st.z_to_bf16_1,st.gemm1,st.relu,st.interlayer_to_bf16,
                st.interlayer_to_fp32,st.spmm2,st.z_to_bf16_2,st.gemm2,st.final_to_bf16);
            fflush(stdout);
        }
        for(int k=0;k<6;k++)printf("MEDIAN graph=%s method=%s ms=%.9g\n",name,names[k],median(times[k]));
        printf("DONE graph=%s pass=1\n",name);fflush(stdout);
    }
    mkl_sparse_destroy(a);
    free(rs);free(re);free(ci);free(perm);
    free(w1);free(w2);free(h0);free(h0b);free(w1b);free(w2b);free(w1packed);free(w2packed);free(wv1);free(wv2);
    free(h0q);free(h1);free(h1q);free(h2);free(z);free(h1b);free(h2b);free(zb);free(reference);
    free(csr.indptr);free(csr.indices);free(csr.values);
}

int main(int argc,char **argv) {
    if(argc!=4){fprintf(stderr,"Usage: %s <data_dir> <graph> <compare|bf16_only>\n",argv[0]);return 2;}
    try{run_graph(argv[1],argv[2],argv[3]);return 0;}
    catch(const std::exception &e){fprintf(stderr,"ERROR %s\n",e.what());return 1;}
}
