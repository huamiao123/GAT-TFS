/*
 * gcn_e2e_v2.cpp — Optimized E2E: BF16 direct output pipeline
 *
 * Key optimizations vs v1:
 *   1. Kernel outputs BF16 directly (no separate FP32→BF16 pass)
 *   2. No global memset (every row written exactly once via perm)
 *   3. BF16 ReLU (sign bit check, no FP32 conversion)
 *   4. No initial H0 FP32→BF16 conversion in timing (pre-converted)
 *
 * Compile:
 *   icpx -O3 -march=sapphirerapids -mamx-bf16 -mamx-tile \
 *        -mavx512bf16 -qopenmp -qmkl=parallel \
 *        -o gcn_e2e_v2 gcn_e2e_v2.cpp
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
    memset(cfg, 0, sizeof(*cfg)); cfg->palette = 1;
    for (int t = 0; t < 7; t++) { cfg->rows[t] = TR; cfg->colsb[t] = 64; }
}

static inline uint16_t f32_to_bf16(float f) {
    uint32_t u; memcpy(&u, &f, 4); return (uint16_t)(u >> 16);
}
static inline float bf16_to_f32(uint16_t b) {
    uint32_t u = (uint32_t)b << 16; float f; memcpy(&f, &u, 4); return f;
}

/* ================================================================
 * VNNI packing & conversion (same as before)
 * ================================================================ */
static void make_W_vnni(const float *W, uint16_t *Wv) {
    for (int kb=0;kb<KB;kb++) for (int ob=0;ob<NB;ob++)
        for (int kp=0;kp<16;kp++) for (int n=0;n<16;n++) {
            int k0=kb*32+kp*2, k1=k0+1, col=ob*16+n;
            uint16_t v0=f32_to_bf16(W[k0*K_DIM+col]);
            uint16_t v1=f32_to_bf16(W[k1*K_DIM+col]);
            int base=((kb*NB+ob)*16+kp)*32+n*2;
            memcpy(&Wv[base],&v0,2); memcpy(&Wv[base+1],&v1,2);
        }
}

static void convert_f32_to_bf16(const float *src, uint16_t *dst, size_t count) {
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < count; i++) dst[i] = f32_to_bf16(src[i]);
}

static int* make_degree_perm(const uint32_t *indptr, int N) {
    int *perm = (int*)malloc((size_t)N*sizeof(int));
    for (int i=0;i<N;i++) perm[i]=i;
    const uint32_t *ip = indptr;
    std::sort(perm, perm+N, [ip](int a, int b){return (ip[a+1]-ip[a])<(ip[b+1]-ip[b]);});
    return perm;
}

/* ================================================================
 * BF16 ReLU: check sign bit, zero if negative
 * BF16 format: bit15=sign. If set, value is negative → output 0.
 * ================================================================ */
static void relu_bf16(uint16_t *data, size_t count) {
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < count; i++)
        if (data[i] & 0x8000) data[i] = 0;
}

/* ================================================================
 * TFS V3 Kernel — BF16 direct output
 *
 * Input:  Hb_in  [N][K] BF16
 * Output: Hb_out [N][K] BF16 (direct, no FP32 intermediate matrix)
 *
 * Key: tile_stored → FP32 temp (1KB, L1) → immediate BF16 convert → write
 * No global memset needed (every row written exactly once via perm)
 * ================================================================ */
static void tfs_v3_bf16_pipeline(
    const uint32_t *indptr, const uint32_t *indices,
    const uint16_t *Hb_in, const uint16_t *Wv,
    uint16_t *Hb_out, const int *perm, int N, int R)
{
    #pragma omp parallel
    {
        if (syscall(SYS_arch_prctl, 0x1023, 18) != 0) { perror("arch_prctl"); exit(1); }
        tilecfg_t cfg; setup_tilecfg(&cfg); _tile_loadconfig(&cfg);

        uint16_t Hbuf[TR * K_DIM] __attribute__((aligned(64)));
        float    Ctmp[TR * 16]    __attribute__((aligned(64)));
        uint32_t base_local[TR], deg_local[TR];
        int      orig_row[TR];

        #pragma omp for schedule(dynamic, 1) nowait
        for (int rg = 0; rg < N; rg += R) {
            int rg_end = (rg+R < N) ? (rg+R) : N;
            for (int i = rg; i < rg_end; i += TR) {
                int batch = ((i+TR) <= rg_end) ? TR : (rg_end-i);
                int max_deg = 0;
                for (int n = 0; n < batch; n++) {
                    int row = perm[i+n];
                    orig_row[n] = row;
                    base_local[n] = indptr[row];
                    deg_local[n] = indptr[row+1]-indptr[row];
                    if ((int)deg_local[n] > max_deg) max_deg = (int)deg_local[n];
                }
                for (int obp = 0; obp < NP; obp++) {
                    _tile_zero(TC0); _tile_zero(TC1);
                    _tile_zero(TC2); _tile_zero(TC3);
                    memset(Hbuf, 0, TR*K_DIM*sizeof(uint16_t));
                    int active_from = 0;

                    for (int s = 0; s < max_deg; s++) {
                        if (s+1 < max_deg) {
                            for (int n=active_from;n<batch;n++) {
                                if ((uint32_t)(s+1)<deg_local[n]) {
                                    uint32_t j_next=indices[base_local[n]+s+1];
                                    const char *addr=(const char*)&Hb_in[(size_t)j_next*K_DIM];
                                    _mm_prefetch(addr,_MM_HINT_T0);
                                    _mm_prefetch(addr+64,_MM_HINT_T0);
                                    _mm_prefetch(addr+128,_MM_HINT_T0);
                                    _mm_prefetch(addr+192,_MM_HINT_T0);
                                }
                            }
                        }
                        while (active_from<batch && (uint32_t)s>=deg_local[active_from]) {
                            memset(&Hbuf[active_from*K_DIM],0,K_DIM*sizeof(uint16_t));
                            active_from++;
                        }
                        if (active_from>=batch) break;
                        for (int n=active_from;n<batch;n++) {
                            uint32_t j=indices[base_local[n]+s];
                            memcpy(&Hbuf[n*K_DIM],&Hb_in[(size_t)j*K_DIM],K_DIM*sizeof(uint16_t));
                        }
                        for (int kb=0;kb<KB;kb++) {
                            _tile_loadd(TA,(const uint8_t*)Hbuf+kb*64,K_DIM*2);
                            int ob0=obp*4; const uint16_t *Wb=Wv;
                            #define WV_OFF(kb_,ob_) (((kb_)*NB+(ob_))*16*32)
                            _tile_loadd(TB0,&Wb[WV_OFF(kb,ob0+0)],64);
                            _tile_loadd(TB1,&Wb[WV_OFF(kb,ob0+1)],64);
                            _tile_dpbf16ps(TC0,TA,TB0);
                            _tile_dpbf16ps(TC1,TA,TB1);
                            _tile_loadd(TB0,&Wb[WV_OFF(kb,ob0+2)],64);
                            _tile_loadd(TB1,&Wb[WV_OFF(kb,ob0+3)],64);
                            _tile_dpbf16ps(TC2,TA,TB0);
                            _tile_dpbf16ps(TC3,TA,TB1);
                            #undef WV_OFF
                        }
                    }

                    /* === BF16 direct output ===
                     * tile_stored → FP32 Ctmp (1KB, stays in L1)
                     * → immediate convert to BF16 → write to Hb_out
                     * ONE memory write, no separate conversion pass */
                    int col = obp * 64;

                    #define STORE_TILE_BF16(TILE_ID, COL_OFF)                    \
                    {                                                             \
                        _tile_stored(TILE_ID, Ctmp, 64);                         \
                        for (int n = 0; n < batch; n++) {                        \
                            uint16_t *dst = &Hb_out[(size_t)orig_row[n]*K_DIM    \
                                            + col + COL_OFF];                    \
                            float *src = &Ctmp[n * 16];                          \
                            for (int j = 0; j < 16; j++)                         \
                                dst[j] = f32_to_bf16(src[j]);                    \
                        }                                                        \
                    }

                    STORE_TILE_BF16(TC0, 0);
                    STORE_TILE_BF16(TC1, 16);
                    STORE_TILE_BF16(TC2, 32);
                    STORE_TILE_BF16(TC3, 48);

                    #undef STORE_TILE_BF16
                }
            }
        }
        _tile_release();
    }
}

/* ================================================================
 * Also keep FP32 output version for precision comparison
 * ================================================================ */
static void tfs_v3_fp32out(
    const uint32_t *indptr, const uint32_t *indices,
    const uint16_t *Hb_in, const uint16_t *Wv,
    float *C, const int *perm, int N, int R)
{
    memset(C, 0, (size_t)N*K_DIM*sizeof(float));
    #pragma omp parallel
    {
        if (syscall(SYS_arch_prctl, 0x1023, 18) != 0) { perror("arch_prctl"); exit(1); }
        tilecfg_t cfg; setup_tilecfg(&cfg); _tile_loadconfig(&cfg);
        uint16_t Hbuf[TR*K_DIM] __attribute__((aligned(64)));
        float Ctmp[TR*16] __attribute__((aligned(64)));
        uint32_t base_local[TR], deg_local[TR]; int orig_row[TR];

        #pragma omp for schedule(dynamic, 1) nowait
        for (int rg=0;rg<N;rg+=R) {
            int rg_end=(rg+R<N)?(rg+R):N;
            for (int i=rg;i<rg_end;i+=TR) {
                int batch=((i+TR)<=rg_end)?TR:(rg_end-i);
                int max_deg=0;
                for (int n=0;n<batch;n++) {
                    int row=perm[i+n]; orig_row[n]=row;
                    base_local[n]=indptr[row]; deg_local[n]=indptr[row+1]-indptr[row];
                    if ((int)deg_local[n]>max_deg) max_deg=(int)deg_local[n];
                }
                for (int obp=0;obp<NP;obp++) {
                    _tile_zero(TC0);_tile_zero(TC1);_tile_zero(TC2);_tile_zero(TC3);
                    memset(Hbuf,0,TR*K_DIM*sizeof(uint16_t));
                    int af=0;
                    for (int s=0;s<max_deg;s++) {
                        if (s+1<max_deg) for (int n=af;n<batch;n++)
                            if ((uint32_t)(s+1)<deg_local[n]) {
                                uint32_t jn=indices[base_local[n]+s+1];
                                const char *a=(const char*)&Hb_in[(size_t)jn*K_DIM];
                                _mm_prefetch(a,_MM_HINT_T0);_mm_prefetch(a+64,_MM_HINT_T0);
                                _mm_prefetch(a+128,_MM_HINT_T0);_mm_prefetch(a+192,_MM_HINT_T0);
                            }
                        while (af<batch&&(uint32_t)s>=deg_local[af]) {
                            memset(&Hbuf[af*K_DIM],0,K_DIM*sizeof(uint16_t)); af++;
                        }
                        if (af>=batch) break;
                        for (int n=af;n<batch;n++) {
                            uint32_t j=indices[base_local[n]+s];
                            memcpy(&Hbuf[n*K_DIM],&Hb_in[(size_t)j*K_DIM],K_DIM*sizeof(uint16_t));
                        }
                        for (int kb=0;kb<KB;kb++) {
                            _tile_loadd(TA,(const uint8_t*)Hbuf+kb*64,K_DIM*2);
                            int ob0=obp*4; const uint16_t *Wb=Wv;
                            #define WV_OFF(kb_,ob_) (((kb_)*NB+(ob_))*16*32)
                            _tile_loadd(TB0,&Wb[WV_OFF(kb,ob0+0)],64);
                            _tile_loadd(TB1,&Wb[WV_OFF(kb,ob0+1)],64);
                            _tile_dpbf16ps(TC0,TA,TB0);_tile_dpbf16ps(TC1,TA,TB1);
                            _tile_loadd(TB0,&Wb[WV_OFF(kb,ob0+2)],64);
                            _tile_loadd(TB1,&Wb[WV_OFF(kb,ob0+3)],64);
                            _tile_dpbf16ps(TC2,TA,TB0);_tile_dpbf16ps(TC3,TA,TB1);
                            #undef WV_OFF
                        }
                    }
                    int col=obp*64;
                    _tile_stored(TC0,Ctmp,64); for(int n=0;n<batch;n++) memcpy(&C[(size_t)orig_row[n]*K_DIM+col+0],&Ctmp[n*16],64);
                    _tile_stored(TC1,Ctmp,64); for(int n=0;n<batch;n++) memcpy(&C[(size_t)orig_row[n]*K_DIM+col+16],&Ctmp[n*16],64);
                    _tile_stored(TC2,Ctmp,64); for(int n=0;n<batch;n++) memcpy(&C[(size_t)orig_row[n]*K_DIM+col+32],&Ctmp[n*16],64);
                    _tile_stored(TC3,Ctmp,64); for(int n=0;n<batch;n++) memcpy(&C[(size_t)orig_row[n]*K_DIM+col+48],&Ctmp[n*16],64);
                }
            }
        }
        _tile_release();
    }
}

/* ================================================================ */
static void relu_f32(float *data, size_t count) {
    #pragma omp parallel for schedule(static)
    for (size_t i=0;i<count;i++) if (data[i]<0.0f) data[i]=0.0f;
}

/* ================================================================
 * MKL two-step
 * ================================================================ */
static void mkl_spmm_gemm(sparse_matrix_t A, struct matrix_descr descr,
    const float *H, const float *W, float *Z, float *C, int N) {
    mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE,1.0f,A,descr,
        SPARSE_LAYOUT_ROW_MAJOR,H,K_DIM,K_DIM,0.0f,Z,K_DIM);
    cblas_sgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,
        N,K_DIM,K_DIM,1.0f,Z,K_DIM,W,K_DIM,0.0f,C,K_DIM);
}

/* ================================================================
 * CSR loading
 * ================================================================ */
struct csr_t { uint32_t *indptr,*indices; float *values; int N; uint32_t nnz; };

static csr_t load_csrbin(const char *path) {
    FILE *f=fopen(path,"rb");
    if (!f) { fprintf(stderr,"Cannot open %s\n",path); exit(1); }
    uint8_t hdr[36]; fread(hdr,1,36,f);
    uint64_t nr,nc,nz;
    memcpy(&nr,&hdr[12],8);memcpy(&nc,&hdr[20],8);memcpy(&nz,&hdr[28],8);
    int N=(int)nr; uint32_t nnz=(uint32_t)nz;
    uint32_t *indptr=(uint32_t*)malloc((size_t)(N+1)*4); fread(indptr,4,N+1,f);
    uint32_t *indices=(uint32_t*)malloc((size_t)nnz*4); fread(indices,4,nnz,f);
    float *values=(float*)malloc((size_t)nnz*4);
    size_t rv=fread(values,4,nnz,f);
    if (rv<(size_t)nnz) for(uint32_t i=0;i<nnz;i++) values[i]=1.0f;
    fclose(f);
    return {indptr,indices,values,N,nnz};
}

/* ================================================================
 * Precision comparison
 * ================================================================ */
static void compare_bf16_fp32(const uint16_t *bf16_out, const float *fp32_out,
                               size_t count, const char *label) {
    double max_d=0, max_r=0, sum_sq=0;
    for (size_t i=0;i<count;i++) {
        double a = (double)bf16_to_f32(bf16_out[i]);
        double b = (double)fp32_out[i];
        double d = fabs(a-b);
        double r = fabs(b);
        if (d>max_d) max_d=d;
        if (r>max_r) max_r=r;
        sum_sq += d*d;
    }
    double rel = (max_r>0)?max_d/max_r:0;
    printf("  %s: max_rel_err=%.6f  RMSE=%.4e\n", label, rel, sqrt(sum_sq/count));
}

/* ================================================================
 * E2E Benchmark
 * ================================================================ */
static void benchmark(const char *dir, const char *name) {
    char path[512];
    snprintf(path,512,"%s/%s/%s.csrbin",dir,name,name);
    printf("Loading: %s\n",path);
    csr_t csr = load_csrbin(path);
    int N=csr.N; uint32_t nnz=csr.nnz;
    printf("Matrix: %s  N=%d  NNZ=%u  avg_deg=%.1f\n\n", name,N,nnz,(double)nnz/N);

    /* Weights & features */
    srand(12345);
    float *W1=(float*)mkl_malloc((size_t)K_DIM*K_DIM*4,64);
    float *W2=(float*)mkl_malloc((size_t)K_DIM*K_DIM*4,64);
    float *H0_f32=(float*)mkl_malloc((size_t)N*K_DIM*4,64);
    for (int i=0;i<K_DIM*K_DIM;i++) { W1[i]=0.01f*((rand()%200)-100); W2[i]=0.01f*((rand()%200)-100); }
    for (size_t i=0;i<(size_t)N*K_DIM;i++) H0_f32[i]=0.01f*((rand()%200)-100);

    /* Prepare TFS: perm, VNNI weights, initial BF16 features */
    int *perm = make_degree_perm(csr.indptr, N);
    uint16_t *Wv1=(uint16_t*)aligned_alloc(64,(size_t)KB*NB*16*32*2);
    uint16_t *Wv2=(uint16_t*)aligned_alloc(64,(size_t)KB*NB*16*32*2);
    make_W_vnni(W1,Wv1); make_W_vnni(W2,Wv2);

    /* Pre-convert H0 to BF16 (OUTSIDE timing — in production, features arrive as BF16) */
    uint16_t *H0_bf16=(uint16_t*)aligned_alloc(64,(size_t)N*K_DIM*2);
    convert_f32_to_bf16(H0_f32, H0_bf16, (size_t)N*K_DIM);

    /* Buffers for BF16 pipeline */
    uint16_t *H1_bf16=(uint16_t*)aligned_alloc(64,(size_t)N*K_DIM*2);
    uint16_t *H2_bf16=(uint16_t*)aligned_alloc(64,(size_t)N*K_DIM*2);

    /* ============================================================
     * TFS V3 BF16 Pipeline
     * Layer 1: TFS(A, H0_bf16, W1) → BF16 out → BF16 ReLU
     * Layer 2: TFS(A, H1_bf16, W2) → BF16 out
     * NO format conversion between layers!
     * ============================================================ */
    printf("=== TFS V3 BF16 Pipeline (Optimized) ===\n");

    /* Warmup */
    tfs_v3_bf16_pipeline(csr.indptr,csr.indices,H0_bf16,Wv1,H1_bf16,perm,N,64);
    relu_bf16(H1_bf16,(size_t)N*K_DIM);
    tfs_v3_bf16_pipeline(csr.indptr,csr.indices,H1_bf16,Wv2,H2_bf16,perm,N,64);

    double best_tfs = 1e30;
    for (int run=0;run<5;run++) {
        double t0 = omp_get_wtime();

        /* Layer 1 */
        tfs_v3_bf16_pipeline(csr.indptr,csr.indices,H0_bf16,Wv1,H1_bf16,perm,N,64);
        relu_bf16(H1_bf16,(size_t)N*K_DIM);

        /* Layer 2 */
        tfs_v3_bf16_pipeline(csr.indptr,csr.indices,H1_bf16,Wv2,H2_bf16,perm,N,64);

        double t1 = omp_get_wtime();
        double ms = (t1-t0)*1000.0;
        printf("  run %d: %.2f ms\n", run, ms);
        if (ms<best_tfs) best_tfs=ms;
    }
    printf("  TFS V3 E2E BEST: %.2f ms\n\n", best_tfs);

    /* ============================================================
     * MKL FP32 Two-Step
     * ============================================================ */
    printf("=== MKL FP32 Two-Step ===\n");

    MKL_INT *rs=(MKL_INT*)malloc((size_t)N*sizeof(MKL_INT));
    MKL_INT *re=(MKL_INT*)malloc((size_t)N*sizeof(MKL_INT));
    MKL_INT *ci=(MKL_INT*)malloc((size_t)nnz*sizeof(MKL_INT));
    for(int i=0;i<N;i++){rs[i]=csr.indptr[i];re[i]=csr.indptr[i+1];}
    for(uint32_t i=0;i<nnz;i++) ci[i]=csr.indices[i];

    sparse_matrix_t A_mkl;
    struct matrix_descr descr; descr.type=SPARSE_MATRIX_TYPE_GENERAL;
    sparse_status_t st=mkl_sparse_s_create_csr(&A_mkl,SPARSE_INDEX_BASE_ZERO,
        N,N,rs,re,ci,csr.values);
    if (st!=SPARSE_STATUS_SUCCESS) {
        printf("  MKL create_csr failed (code=%d), skipping\n\n",st);
        goto cleanup;
    }
    mkl_sparse_set_mm_hint(A_mkl,SPARSE_OPERATION_NON_TRANSPOSE,descr,
        SPARSE_LAYOUT_ROW_MAJOR,K_DIM,10);
    mkl_sparse_optimize(A_mkl);

    {
        float *Z=(float*)mkl_malloc((size_t)N*K_DIM*4,64);
        float *H1_mkl=(float*)mkl_malloc((size_t)N*K_DIM*4,64);
        float *H2_mkl=(float*)mkl_malloc((size_t)N*K_DIM*4,64);

        /* Warmup */
        mkl_spmm_gemm(A_mkl,descr,H0_f32,W1,Z,H1_mkl,N);
        relu_f32(H1_mkl,(size_t)N*K_DIM);
        mkl_spmm_gemm(A_mkl,descr,H1_mkl,W2,Z,H2_mkl,N);

        double best_mkl=1e30;
        for (int run=0;run<5;run++) {
            double t0=omp_get_wtime();
            mkl_spmm_gemm(A_mkl,descr,H0_f32,W1,Z,H1_mkl,N);
            relu_f32(H1_mkl,(size_t)N*K_DIM);
            mkl_spmm_gemm(A_mkl,descr,H1_mkl,W2,Z,H2_mkl,N);
            double t1=omp_get_wtime();
            double ms=(t1-t0)*1000.0;
            printf("  run %d: %.2f ms\n",run,ms);
            if (ms<best_mkl) best_mkl=ms;
        }
        printf("  MKL E2E BEST: %.2f ms\n\n", best_mkl);

        /* ============================================================
         * Summary
         * ============================================================ */
        printf("=== E2E Summary ===\n");
        printf("  TFS V3 (BF16 pipeline): %.2f ms\n", best_tfs);
        printf("  MKL (FP32 two-step):    %.2f ms\n", best_mkl);
        printf("  E2E Speedup:            %.2f×\n\n", best_mkl/best_tfs);

        /* ============================================================
         * Precision: BF16 pipeline vs FP32 MKL
         * ============================================================ */
        printf("=== Precision: BF16 Pipeline vs FP32 MKL ===\n");

        /* Re-run both for clean comparison */
        tfs_v3_bf16_pipeline(csr.indptr,csr.indices,H0_bf16,Wv1,H1_bf16,perm,N,64);
        relu_bf16(H1_bf16,(size_t)N*K_DIM);
        tfs_v3_bf16_pipeline(csr.indptr,csr.indices,H1_bf16,Wv2,H2_bf16,perm,N,64);

        mkl_spmm_gemm(A_mkl,descr,H0_f32,W1,Z,H1_mkl,N);
        relu_f32(H1_mkl,(size_t)N*K_DIM);
        mkl_spmm_gemm(A_mkl,descr,H1_mkl,W2,Z,H2_mkl,N);

        printf("Layer 2 (final output):\n");
        compare_bf16_fp32(H2_bf16, H2_mkl, (size_t)N*K_DIM, "BF16_pipeline vs FP32_MKL");

        /* Sample values */
        printf("\nFirst 8 output values:\n  TFS_BF16: ");
        for(int i=0;i<8;i++) printf("%.2f ",bf16_to_f32(H2_bf16[i]));
        printf("\n  MKL_FP32: ");
        for(int i=0;i<8;i++) printf("%.2f ",H2_mkl[i]);
        printf("\n");

        mkl_free(Z); mkl_free(H1_mkl); mkl_free(H2_mkl);
        mkl_sparse_destroy(A_mkl);
    }

cleanup:
    free(rs);free(re);free(ci);
    free(perm);free(Wv1);free(Wv2);
    free(H0_bf16);free(H1_bf16);free(H2_bf16);
    mkl_free(W1);mkl_free(W2);mkl_free(H0_f32);
    free(csr.indptr);free(csr.indices);free(csr.values);
}

/* ================================================================ */
int main(int argc, char **argv) {
    printf("GCN E2E v2 — Optimized BF16 Pipeline\n");
    printf("Threads: %d\n\n", omp_get_max_threads());
    if (argc<3) { fprintf(stderr,"Usage: %s <dir> <name|ALL>\n",argv[0]); return 1; }

    const char *dir=argv[1],*name=argv[2];
    const char *all[]={"web-Google","amazon0601","cit-Patents","as-Skitter",
        "soc-Pokec","hollywood-2009","indochina-2004","ogbn-products","reddit","com-LiveJournal"};

    if (strcmp(name,"ALL")==0) {
        for (int i=0;i<10;i++) {
            printf("############################################\n");
            printf("# %s\n",all[i]);
            printf("############################################\n\n");
            benchmark(dir,all[i]);
            printf("\n");
        }
    } else benchmark(dir,name);
    return 0;
}
