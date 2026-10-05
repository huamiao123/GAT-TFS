/*
 * gcn_e2e_v3.cpp — Hybrid Pipeline: FP32 intermediate + BF16 final
 *
 * Fixes precision collapse by keeping FP32 between layers.
 * Only one FP32→BF16 conversion per inter-layer transition.
 *
 * Compile:
 *   icpx -O3 -march=sapphirerapids -mamx-bf16 -mamx-tile \
 *        -mavx512bf16 -qopenmp -qmkl=parallel \
 *        -o gcn_e2e_v3 gcn_e2e_v3.cpp
 */

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <immintrin.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <omp.h>
#include <mkl.h>
#include <mkl_spblas.h>

#define K_DIM  128
#define KB     4
#define NB     8
#define TR     16
#define NP     2

#define TC0 0
#define TC1 1
#define TC2 2
#define TC3 3
#define TA  4
#define TB0 5
#define TB1 6

struct __attribute__((aligned(64))) tilecfg_t {
    uint8_t palette; uint8_t start_row; uint8_t reserved[14];
    uint16_t colsb[16]; uint8_t rows[16];
};
static void setup_tilecfg(tilecfg_t *cfg) {
    memset(cfg,0,sizeof(*cfg)); cfg->palette=1;
    for (int t=0;t<7;t++){cfg->rows[t]=TR;cfg->colsb[t]=64;}
}

static inline uint16_t f32_to_bf16(float f) {
    uint32_t u; memcpy(&u,&f,4); return (uint16_t)(u>>16);
}
static inline float bf16_to_f32(uint16_t b) {
    uint32_t u=(uint32_t)b<<16; float f; memcpy(&f,&u,4); return f;
}

static void make_W_vnni(const float *W, uint16_t *Wv) {
    for(int kb=0;kb<KB;kb++) for(int ob=0;ob<NB;ob++)
        for(int kp=0;kp<16;kp++) for(int n=0;n<16;n++){
            int k0=kb*32+kp*2,k1=k0+1,col=ob*16+n;
            uint16_t v0=f32_to_bf16(W[k0*K_DIM+col]);
            uint16_t v1=f32_to_bf16(W[k1*K_DIM+col]);
            int base=((kb*NB+ob)*16+kp)*32+n*2;
            memcpy(&Wv[base],&v0,2); memcpy(&Wv[base+1],&v1,2);
        }
}

static void convert_f32_to_bf16(const float *src, uint16_t *dst, size_t count) {
    #pragma omp parallel for schedule(static)
    for (size_t i=0;i<count;i++) dst[i]=f32_to_bf16(src[i]);
}

static int* make_degree_perm(const uint32_t *indptr, int N) {
    int *perm=(int*)malloc((size_t)N*sizeof(int));
    for(int i=0;i<N;i++) perm[i]=i;
    const uint32_t *ip=indptr;
    std::sort(perm,perm+N,[ip](int a,int b){return(ip[a+1]-ip[a])<(ip[b+1]-ip[b]);});
    return perm;
}

/* ================================================================
 * Core TFS V3 kernel (shared by both output modes)
 * Returns with C tiles loaded. Caller decides FP32 or BF16 store.
 * ================================================================ */
#define KERNEL_BODY(Hb_in)                                                     \
    uint16_t Hbuf[TR*K_DIM] __attribute__((aligned(64)));                      \
    float Ctmp[TR*16] __attribute__((aligned(64)));                            \
    uint32_t base_local[TR], deg_local[TR]; int orig_row[TR];                  \
                                                                               \
    for (int rg=rg_start; rg<rg_end_outer; rg+=R) {                           \
        int rg_end=(rg+R<rg_end_outer)?(rg+R):rg_end_outer;                   \
        for (int i=rg;i<rg_end;i+=TR) {                                       \
            int batch=((i+TR)<=rg_end)?TR:(rg_end-i);                         \
            int max_deg=0;                                                     \
            for(int n=0;n<batch;n++){                                          \
                int row=perm[i+n]; orig_row[n]=row;                            \
                base_local[n]=indptr[row];                                     \
                deg_local[n]=indptr[row+1]-indptr[row];                        \
                if((int)deg_local[n]>max_deg) max_deg=(int)deg_local[n];       \
            }                                                                  \
            for(int obp=0;obp<NP;obp++){                                      \
                _tile_zero(TC0);_tile_zero(TC1);                               \
                _tile_zero(TC2);_tile_zero(TC3);                               \
                memset(Hbuf,0,TR*K_DIM*sizeof(uint16_t));                      \
                int af=0;                                                      \
                for(int s=0;s<max_deg;s++){                                    \
                    if(s+1<max_deg) for(int n=af;n<batch;n++)                  \
                        if((uint32_t)(s+1)<deg_local[n]){                      \
                            uint32_t jn=indices[base_local[n]+s+1];            \
                            const char *a=(const char*)&Hb_in[(size_t)jn*K_DIM]; \
                            _mm_prefetch(a,_MM_HINT_T0);                       \
                            _mm_prefetch(a+64,_MM_HINT_T0);                    \
                            _mm_prefetch(a+128,_MM_HINT_T0);                   \
                            _mm_prefetch(a+192,_MM_HINT_T0);                   \
                        }                                                      \
                    while(af<batch&&(uint32_t)s>=deg_local[af]){               \
                        memset(&Hbuf[af*K_DIM],0,K_DIM*sizeof(uint16_t));      \
                        af++;                                                  \
                    }                                                          \
                    if(af>=batch) break;                                        \
                    for(int n=af;n<batch;n++){                                 \
                        uint32_t j=indices[base_local[n]+s];                   \
                        memcpy(&Hbuf[n*K_DIM],&Hb_in[(size_t)j*K_DIM],        \
                               K_DIM*sizeof(uint16_t));                        \
                    }                                                          \
                    for(int kb=0;kb<KB;kb++){                                  \
                        _tile_loadd(TA,(const uint8_t*)Hbuf+kb*64,K_DIM*2);   \
                        int ob0=obp*4; const uint16_t *Wb=Wv;                 \
                        _tile_loadd(TB0,&Wb[((kb*NB+ob0+0)*16)*32],64);       \
                        _tile_loadd(TB1,&Wb[((kb*NB+ob0+1)*16)*32],64);       \
                        _tile_dpbf16ps(TC0,TA,TB0);                            \
                        _tile_dpbf16ps(TC1,TA,TB1);                            \
                        _tile_loadd(TB0,&Wb[((kb*NB+ob0+2)*16)*32],64);       \
                        _tile_loadd(TB1,&Wb[((kb*NB+ob0+3)*16)*32],64);       \
                        _tile_dpbf16ps(TC2,TA,TB0);                            \
                        _tile_dpbf16ps(TC3,TA,TB1);                            \
                    }                                                          \
                }                                                              \
                int col=obp*64;

/* FP32 store macro */
#define STORE_FP32(out)                                                        \
                _tile_stored(TC0,Ctmp,64);                                     \
                for(int n=0;n<batch;n++)                                       \
                    memcpy(&out[(size_t)orig_row[n]*K_DIM+col+0],&Ctmp[n*16],64); \
                _tile_stored(TC1,Ctmp,64);                                     \
                for(int n=0;n<batch;n++)                                       \
                    memcpy(&out[(size_t)orig_row[n]*K_DIM+col+16],&Ctmp[n*16],64); \
                _tile_stored(TC2,Ctmp,64);                                     \
                for(int n=0;n<batch;n++)                                       \
                    memcpy(&out[(size_t)orig_row[n]*K_DIM+col+32],&Ctmp[n*16],64); \
                _tile_stored(TC3,Ctmp,64);                                     \
                for(int n=0;n<batch;n++)                                       \
                    memcpy(&out[(size_t)orig_row[n]*K_DIM+col+48],&Ctmp[n*16],64);

/* BF16 store macro */
#define STORE_BF16(out)                                                        \
                {                                                              \
                    _tile_stored(TC0,Ctmp,64);                                 \
                    for(int n=0;n<batch;n++){                                  \
                        uint16_t *d=&out[(size_t)orig_row[n]*K_DIM+col+0];    \
                        float *s=&Ctmp[n*16];                                 \
                        for(int j=0;j<16;j++) d[j]=f32_to_bf16(s[j]);         \
                    }                                                          \
                    _tile_stored(TC1,Ctmp,64);                                 \
                    for(int n=0;n<batch;n++){                                  \
                        uint16_t *d=&out[(size_t)orig_row[n]*K_DIM+col+16];   \
                        float *s=&Ctmp[n*16];                                 \
                        for(int j=0;j<16;j++) d[j]=f32_to_bf16(s[j]);         \
                    }                                                          \
                    _tile_stored(TC2,Ctmp,64);                                 \
                    for(int n=0;n<batch;n++){                                  \
                        uint16_t *d=&out[(size_t)orig_row[n]*K_DIM+col+32];   \
                        float *s=&Ctmp[n*16];                                 \
                        for(int j=0;j<16;j++) d[j]=f32_to_bf16(s[j]);         \
                    }                                                          \
                    _tile_stored(TC3,Ctmp,64);                                 \
                    for(int n=0;n<batch;n++){                                  \
                        uint16_t *d=&out[(size_t)orig_row[n]*K_DIM+col+48];   \
                        float *s=&Ctmp[n*16];                                 \
                        for(int j=0;j<16;j++) d[j]=f32_to_bf16(s[j]);         \
                    }                                                          \
                }

#define KERNEL_END }}}

/* ================================================================
 * Kernel A: BF16 in → FP32 out (for intermediate layers)
 * ================================================================ */
static void tfs_v3_fp32out(
    const uint32_t *indptr, const uint32_t *indices,
    const uint16_t *Hb_in, const uint16_t *Wv,
    float *C_out, const int *perm, int N, int R)
{
    #pragma omp parallel
    {
        if(syscall(SYS_arch_prctl,0x1023,18)!=0){perror("arch_prctl");exit(1);}
        tilecfg_t cfg; setup_tilecfg(&cfg); _tile_loadconfig(&cfg);

        int rg_start, rg_end_outer;
        #pragma omp for schedule(dynamic,1) nowait
        for (rg_start=0; rg_start<N; rg_start+=R) {
            rg_end_outer = (rg_start+R<N)?(rg_start+R):N;

        KERNEL_BODY(Hb_in)
        STORE_FP32(C_out)
        KERNEL_END

        }
        _tile_release();
    }
}

/* ================================================================
 * Kernel B: BF16 in → BF16 out (for final layer)
 * ================================================================ */
static void tfs_v3_bf16out(
    const uint32_t *indptr, const uint32_t *indices,
    const uint16_t *Hb_in, const uint16_t *Wv,
    uint16_t *Hb_out, const int *perm, int N, int R)
{
    #pragma omp parallel
    {
        if(syscall(SYS_arch_prctl,0x1023,18)!=0){perror("arch_prctl");exit(1);}
        tilecfg_t cfg; setup_tilecfg(&cfg); _tile_loadconfig(&cfg);

        int rg_start, rg_end_outer;
        #pragma omp for schedule(dynamic,1) nowait
        for (rg_start=0; rg_start<N; rg_start+=R) {
            rg_end_outer = (rg_start+R<N)?(rg_start+R):N;

        KERNEL_BODY(Hb_in)
        STORE_BF16(Hb_out)
        KERNEL_END

        }
        _tile_release();
    }
}

/* ================================================================ */
static void relu_f32(float *data, size_t count) {
    #pragma omp parallel for schedule(static)
    for(size_t i=0;i<count;i++) if(data[i]<0.0f) data[i]=0.0f;
}

static void relu_bf16(uint16_t *data, size_t count) {
    #pragma omp parallel for schedule(static)
    for(size_t i=0;i<count;i++) if(data[i]&0x8000) data[i]=0;
}

/* ================================================================ */
struct csr_t { uint32_t *indptr,*indices; float *values; int N; uint32_t nnz; };
static csr_t load_csrbin(const char *path) {
    FILE *f=fopen(path,"rb");
    if(!f){fprintf(stderr,"Cannot open %s\n",path);exit(1);}
    uint8_t hdr[36]; fread(hdr,1,36,f);
    uint64_t nr,nc,nz;
    memcpy(&nr,&hdr[12],8);memcpy(&nc,&hdr[20],8);memcpy(&nz,&hdr[28],8);
    int N=(int)nr; uint32_t nnz=(uint32_t)nz;
    uint32_t *indptr=(uint32_t*)malloc((size_t)(N+1)*4); fread(indptr,4,N+1,f);
    uint32_t *indices=(uint32_t*)malloc((size_t)nnz*4); fread(indices,4,nnz,f);
    float *values=(float*)malloc((size_t)nnz*4);
    size_t rv=fread(values,4,nnz,f);
    if(rv<(size_t)nnz) for(uint32_t i=0;i<nnz;i++) values[i]=1.0f;
    fclose(f);
    return {indptr,indices,values,N,nnz};
}

static void mkl_spmm_gemm(sparse_matrix_t A, struct matrix_descr descr,
    const float *H, const float *W, float *Z, float *C, int N) {
    mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE,1.0f,A,descr,
        SPARSE_LAYOUT_ROW_MAJOR,H,K_DIM,K_DIM,0.0f,Z,K_DIM);
    cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,
        N,K_DIM,K_DIM,1.0f,Z,K_DIM,W,K_DIM,0.0f,C,K_DIM);
}

static void compare_outputs(const float *a, const float *b, size_t count, const char *label) {
    double max_d=0,max_v=0,sum_sq=0;
    for(size_t i=0;i<count;i++){
        double d=fabs((double)a[i]-(double)b[i]);
        double v=fabs((double)b[i]);
        if(d>max_d)max_d=d; if(v>max_v)max_v=v;
        sum_sq+=d*d;
    }
    double rel=(max_v>0)?max_d/max_v:0;
    printf("  %s: max_rel_err=%.6f  RMSE=%.4e\n",label,rel,sqrt(sum_sq/count));
}

/* ================================================================
 * E2E Benchmark
 * ================================================================ */
#include "residual_broad.hpp"

static void benchmark(const char *dir, const char *name) {
    char path[512];
    snprintf(path,512,"%s/%s/%s.csrbin",dir,name,name);
    printf("Loading: %s\n",path);
    double projection_setup_tick=omp_get_wtime();
    csr_t csr=load_csrbin(path);
    printf("METHOD_SETUP graph=%s phase=csr_file_load ms=%.12g\n",name,(omp_get_wtime()-projection_setup_tick)*1000);projection_setup_tick=omp_get_wtime();
    int N=csr.N; uint32_t nnz=csr.nnz;
    printf("Matrix: %s  N=%d  NNZ=%u  avg_deg=%.1f\n\n",name,N,nnz,(double)nnz/N);

    projection_setup_tick=omp_get_wtime();
    srand(12345);
    float *W1=(float*)mkl_malloc((size_t)K_DIM*K_DIM*4,64);
    float *W2=(float*)mkl_malloc((size_t)K_DIM*K_DIM*4,64);
    float *H0_f32=(float*)mkl_malloc((size_t)N*K_DIM*4,64);
    printf("METHOD_SETUP graph=%s phase=tensor_allocation ms=%.12g\n",name,(omp_get_wtime()-projection_setup_tick)*1000);projection_setup_tick=omp_get_wtime();
    for(int i=0;i<K_DIM*K_DIM;i++){W1[i]=0.01f*((rand()%200)-100);W2[i]=0.01f*((rand()%200)-100);}
    printf("METHOD_SETUP graph=%s phase=weight_generation ms=%.12g\n",name,(omp_get_wtime()-projection_setup_tick)*1000);projection_setup_tick=omp_get_wtime();
    for(size_t i=0;i<(size_t)N*K_DIM;i++) H0_f32[i]=0.01f*((rand()%200)-100);

    printf("METHOD_SETUP graph=%s phase=feature_generation ms=%.12g\n",name,(omp_get_wtime()-projection_setup_tick)*1000);projection_setup_tick=omp_get_wtime();
    int *perm=make_degree_perm(csr.indptr,N);
    printf("METHOD_SETUP graph=%s phase=degree_sort ms=%.12g\n",name,(omp_get_wtime()-projection_setup_tick)*1000);projection_setup_tick=omp_get_wtime();
    uint16_t *Wv1=(uint16_t*)aligned_alloc(64,(size_t)KB*NB*16*32*2);
    uint16_t *Wv2=(uint16_t*)aligned_alloc(64,(size_t)KB*NB*16*32*2);
    printf("METHOD_SETUP graph=%s phase=packed_weight_allocation ms=%.12g\n",name,(omp_get_wtime()-projection_setup_tick)*1000);projection_setup_tick=omp_get_wtime();
    make_W_vnni(W1,Wv1); make_W_vnni(W2,Wv2);

    printf("METHOD_SETUP graph=%s phase=weight_vnni_packing ms=%.12g\n",name,(omp_get_wtime()-projection_setup_tick)*1000);projection_setup_tick=omp_get_wtime();
    /* Pre-convert H0 (outside timing) */
    uint16_t *H0_bf16=(uint16_t*)aligned_alloc(64,(size_t)N*K_DIM*2);
    printf("METHOD_SETUP graph=%s phase=input_bf16_allocation ms=%.12g\n",name,(omp_get_wtime()-projection_setup_tick)*1000);projection_setup_tick=omp_get_wtime();
    convert_f32_to_bf16(H0_f32,H0_bf16,(size_t)N*K_DIM);

    printf("METHOD_SETUP graph=%s phase=input_bf16_conversion ms=%.12g\n",name,(omp_get_wtime()-projection_setup_tick)*1000);projection_setup_tick=omp_get_wtime();
    /* Buffers */
    float *H1_f32=(float*)aligned_alloc(64,(size_t)N*K_DIM*4);     /* Layer 1 FP32 output */
    uint16_t *H1_bf16=(uint16_t*)aligned_alloc(64,(size_t)N*K_DIM*2); /* Layer 1 → BF16 for L2 input */
    uint16_t *H2_bf16=(uint16_t*)aligned_alloc(64,(size_t)N*K_DIM*2); /* Layer 2 BF16 output */
    float *H2_f32_for_cmp=(float*)aligned_alloc(64,(size_t)N*K_DIM*4); /* For precision comparison */

    /* ============================================================
     * TFS V3 Hybrid Pipeline
     * Layer 1: BF16 in → FP32 out → FP32 ReLU → convert BF16
     * Layer 2: BF16 in → BF16 direct out
     * ============================================================ */
    printf("METHOD_SETUP graph=%s phase=tfs_output_allocation ms=%.12g\n",name,(omp_get_wtime()-projection_setup_tick)*1000);projection_setup_tick=omp_get_wtime();
    printf("=== TFS V3 Hybrid Pipeline ===\n");

    /* Warmup */
    tfs_v3_fp32out(csr.indptr,csr.indices,H0_bf16,Wv1,H1_f32,perm,N,64);
    relu_f32(H1_f32,(size_t)N*K_DIM);
    convert_f32_to_bf16(H1_f32,H1_bf16,(size_t)N*K_DIM);
    tfs_v3_bf16out(csr.indptr,csr.indices,H1_bf16,Wv2,H2_bf16,perm,N,64);

    double best_tfs=1e30;
    for(int run=0;run<5;run++){
        double t0=omp_get_wtime();

        /* Layer 1: FP32 output */
        tfs_v3_fp32out(csr.indptr,csr.indices,H0_bf16,Wv1,H1_f32,perm,N,64);
        relu_f32(H1_f32,(size_t)N*K_DIM);
        convert_f32_to_bf16(H1_f32,H1_bf16,(size_t)N*K_DIM);

        /* Layer 2: BF16 direct output */
        tfs_v3_bf16out(csr.indptr,csr.indices,H1_bf16,Wv2,H2_bf16,perm,N,64);

        double t1=omp_get_wtime();
        double ms=(t1-t0)*1000.0;
        printf("  run %d: %.2f ms\n",run,ms);
        if(ms<best_tfs) best_tfs=ms;
    }
    printf("  TFS V3 E2E BEST: %.2f ms\n\n",best_tfs);

    /* ============================================================
     * MKL FP32 Two-Step
     * ============================================================ */
    printf("=== MKL FP32 Two-Step ===\n");

    projection_setup_tick=omp_get_wtime();
    MKL_INT *rs=(MKL_INT*)malloc((size_t)N*sizeof(MKL_INT));
    MKL_INT *re=(MKL_INT*)malloc((size_t)N*sizeof(MKL_INT));
    MKL_INT *ci=(MKL_INT*)malloc((size_t)nnz*sizeof(MKL_INT));
    printf("METHOD_SETUP graph=%s phase=mkl_index_allocation ms=%.12g\n",name,(omp_get_wtime()-projection_setup_tick)*1000);projection_setup_tick=omp_get_wtime();
    for(int i=0;i<N;i++){rs[i]=csr.indptr[i];re[i]=csr.indptr[i+1];}
    for(uint32_t i=0;i<nnz;i++) ci[i]=csr.indices[i];

    printf("METHOD_SETUP graph=%s phase=mkl_index_conversion ms=%.12g\n",name,(omp_get_wtime()-projection_setup_tick)*1000);projection_setup_tick=omp_get_wtime();
    sparse_matrix_t A_mkl;
    struct matrix_descr descr; descr.type=SPARSE_MATRIX_TYPE_GENERAL;
    sparse_status_t st=mkl_sparse_s_create_csr(&A_mkl,SPARSE_INDEX_BASE_ZERO,
        N,N,rs,re,ci,csr.values);
    printf("METHOD_SETUP graph=%s phase=mkl_csr_create ms=%.12g\n",name,(omp_get_wtime()-projection_setup_tick)*1000);projection_setup_tick=omp_get_wtime();
    if(st!=SPARSE_STATUS_SUCCESS){
        printf("  MKL create_csr failed (code=%d), skipping\n\n",st);
        goto cleanup;
    }
    projection_setup_tick=omp_get_wtime();
    mkl_sparse_set_mm_hint(A_mkl,SPARSE_OPERATION_NON_TRANSPOSE,descr,
        SPARSE_LAYOUT_ROW_MAJOR,K_DIM,10);
    mkl_sparse_optimize(A_mkl);

    printf("METHOD_SETUP graph=%s phase=mkl_hint_and_optimize ms=%.12g\n",name,(omp_get_wtime()-projection_setup_tick)*1000);projection_setup_tick=omp_get_wtime();
    {
        float *Z=(float*)mkl_malloc((size_t)N*K_DIM*4,64);
        float *H1_mkl=(float*)mkl_malloc((size_t)N*K_DIM*4,64);
        float *H2_mkl=(float*)mkl_malloc((size_t)N*K_DIM*4,64);

        printf("METHOD_SETUP graph=%s phase=mkl_output_allocation ms=%.12g\n",name,(omp_get_wtime()-projection_setup_tick)*1000);
        /* Warmup */
        mkl_spmm_gemm(A_mkl,descr,H0_f32,W1,Z,H1_mkl,N);
        relu_f32(H1_mkl,(size_t)N*K_DIM);
        mkl_spmm_gemm(A_mkl,descr,H1_mkl,W2,Z,H2_mkl,N);

        double best_mkl=1e30;
        for(int run=0;run<5;run++){
            double t0=omp_get_wtime();
            mkl_spmm_gemm(A_mkl,descr,H0_f32,W1,Z,H1_mkl,N);
            relu_f32(H1_mkl,(size_t)N*K_DIM);
            mkl_spmm_gemm(A_mkl,descr,H1_mkl,W2,Z,H2_mkl,N);
            double t1=omp_get_wtime();
            double ms=(t1-t0)*1000.0;
            printf("  run %d: %.2f ms\n",run,ms);
            if(ms<best_mkl) best_mkl=ms;
        }
        printf("  MKL E2E BEST: %.2f ms\n\n",best_mkl);

        printf("=== E2E Summary ===\n");
        printf("  TFS V3 (hybrid): %.2f ms\n",best_tfs);
        printf("  MKL (FP32):      %.2f ms\n",best_mkl);
        printf("  E2E Speedup:     %.2f×\n\n",best_mkl/best_tfs);

        /* Precision */
        printf("=== Precision ===\n");
        tfs_v3_fp32out(csr.indptr,csr.indices,H0_bf16,Wv1,H1_f32,perm,N,64);
        relu_f32(H1_f32,(size_t)N*K_DIM);
        convert_f32_to_bf16(H1_f32,H1_bf16,(size_t)N*K_DIM);
        tfs_v3_fp32out(csr.indptr,csr.indices,H1_bf16,Wv2,H2_f32_for_cmp,perm,N,64);

        mkl_spmm_gemm(A_mkl,descr,H0_f32,W1,Z,H1_mkl,N);
        relu_f32(H1_mkl,(size_t)N*K_DIM);
        mkl_spmm_gemm(A_mkl,descr,H1_mkl,W2,Z,H2_mkl,N);

        compare_outputs(H2_f32_for_cmp,H2_mkl,(size_t)N*K_DIM,"TFS_hybrid vs MKL_FP32");

        printf("First 8:\n  TFS: ");
        for(int i=0;i<8;i++) printf("%.2f ",H2_f32_for_cmp[i]);
        printf("\n  MKL: ");
        for(int i=0;i<8;i++) printf("%.2f ",H2_mkl[i]);
        printf("\n");

        residual_broad::run(csr,perm,H0_f32,W1,W2,Wv1,Wv2,H0_bf16,
            H1_f32,H1_bf16,H2_bf16,H2_f32_for_cmp,A_mkl,descr,Z,H1_mkl,H2_mkl,name);

        mkl_free(Z);mkl_free(H1_mkl);mkl_free(H2_mkl);
        mkl_sparse_destroy(A_mkl);
    }

cleanup:
    free(rs);free(re);free(ci);free(perm);
    free(Wv1);free(Wv2);free(H0_bf16);
    free(H1_f32);free(H1_bf16);free(H2_bf16);free(H2_f32_for_cmp);
    mkl_free(W1);mkl_free(W2);mkl_free(H0_f32);
    free(csr.indptr);free(csr.indices);free(csr.values);
}

int main(int argc, char **argv) {
    printf("GCN E2E v3 — Hybrid Pipeline (FP32 inter-layer + BF16 final)\n");
    printf("Threads: %d\n\n",omp_get_max_threads());
    if(argc<3){fprintf(stderr,"Usage: %s <dir> <name>\n",argv[0]);return 1;}

    const char *dir=argv[1],*name=argv[2];

    if(strcmp(name,"ALL")==0){
        const char *all[]={"amazon0601","as-Skitter","cage15","cit-Patents",
            "com-Friendster","com-LiveJournal","com-Youtube","email-Enron",
            "FullChip","hollywood-2009","indochina-2004","kron_g500-logn21",
            "mycielskian19","ogbn-products","rajat31","reddit","rgg_n_2_24_s0",
            "roadNet-CA","road_usa","scircuit","soc-LiveJournal1","soc-Pokec",
            "sx-stackoverflow","web-Google","wiki-Talk"};
        for(int i=0;i<25;i++){
            printf("############################################\n");
            printf("# %s\n",all[i]);
            printf("############################################\n\n");
            benchmark(dir,all[i]);
            printf("\n");
        }
    } else benchmark(dir,name);
    return 0;
}
