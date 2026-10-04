/*
 * gcn_e2e_bench.cpp — End-to-End 2-Layer GCN Inference Benchmark
 *
 * TFS V3 (BF16 AMX fusion) vs MKL FP32 two-step
 * Also measures BF16 vs FP32 precision impact
 *
 * Compile:
 *   icpx -O3 -march=sapphirerapids -mamx-bf16 -mamx-tile \
 *        -mavx512bf16 -qopenmp -qmkl=parallel \
 *        -o gcn_e2e gcn_e2e_bench.cpp
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

/* ================================================================
 * Constants & tile config
 * ================================================================ */
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
    memset(cfg, 0, sizeof(*cfg));
    cfg->palette = 1;
    for (int t = 0; t < 7; t++) { cfg->rows[t] = TR; cfg->colsb[t] = 64; }
}

/* ================================================================
 * BF16 helpers
 * ================================================================ */
static inline uint16_t f32_to_bf16(float f) {
    uint32_t u; memcpy(&u, &f, 4); return (uint16_t)(u >> 16);
}
static inline float bf16_to_f32(uint16_t b) {
    uint32_t u = (uint32_t)b << 16; float f; memcpy(&f, &u, 4); return f;
}

/* ================================================================
 * VNNI packing & BF16 conversion
 * ================================================================ */
static void make_W_vnni(const float *W, uint16_t *Wv) {
    for (int kb = 0; kb < KB; kb++)
        for (int ob = 0; ob < NB; ob++)
            for (int kp = 0; kp < 16; kp++)
                for (int n = 0; n < 16; n++) {
                    int k0 = kb*32+kp*2, k1 = k0+1, col = ob*16+n;
                    uint16_t v0 = f32_to_bf16(W[k0*K_DIM+col]);
                    uint16_t v1 = f32_to_bf16(W[k1*K_DIM+col]);
                    int base = ((kb*NB+ob)*16+kp)*32+n*2;
                    memcpy(&Wv[base], &v0, 2);
                    memcpy(&Wv[base+1], &v1, 2);
                }
}

static void convert_f32_to_bf16(const float *src, uint16_t *dst, size_t count) {
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < count; i++)
        dst[i] = f32_to_bf16(src[i]);
}

/* ================================================================
 * Degree-sorted permutation
 * ================================================================ */
static int* make_degree_perm(const uint32_t *indptr, int N) {
    int *perm = (int*)malloc((size_t)N * sizeof(int));
    for (int i = 0; i < N; i++) perm[i] = i;
    const uint32_t *ip = indptr;
    std::sort(perm, perm + N, [ip](int a, int b) {
        return (ip[a+1]-ip[a]) < (ip[b+1]-ip[b]);
    });
    return perm;
}

/* ================================================================
 * TFS V3 Kernel (embedded, same as amx_tfs_v3.cpp)
 * ================================================================ */
static void tfs_v3_kernel(
    const uint32_t *indptr, const uint32_t *indices,
    const uint16_t *Hb, const uint16_t *Wv,
    float *C, const int *perm, int N, int R)
{
    memset(C, 0, (size_t)N * K_DIM * sizeof(float));

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
                    _tile_zero(TC0); _tile_zero(TC1); _tile_zero(TC2); _tile_zero(TC3);
                    memset(Hbuf, 0, TR*K_DIM*sizeof(uint16_t));
                    int active_from = 0;
                    for (int s = 0; s < max_deg; s++) {
                        if (s+1 < max_deg) {
                            for (int n = active_from; n < batch; n++) {
                                if ((uint32_t)(s+1) < deg_local[n]) {
                                    uint32_t j_next = indices[base_local[n]+s+1];
                                    const char *addr = (const char*)&Hb[(size_t)j_next*K_DIM];
                                    _mm_prefetch(addr, _MM_HINT_T0);
                                    _mm_prefetch(addr+64, _MM_HINT_T0);
                                    _mm_prefetch(addr+128, _MM_HINT_T0);
                                    _mm_prefetch(addr+192, _MM_HINT_T0);
                                }
                            }
                        }
                        while (active_from < batch && (uint32_t)s >= deg_local[active_from]) {
                            memset(&Hbuf[active_from*K_DIM], 0, K_DIM*sizeof(uint16_t));
                            active_from++;
                        }
                        if (active_from >= batch) break;
                        for (int n = active_from; n < batch; n++) {
                            uint32_t j = indices[base_local[n]+s];
                            memcpy(&Hbuf[n*K_DIM], &Hb[(size_t)j*K_DIM], K_DIM*sizeof(uint16_t));
                        }
                        for (int kb = 0; kb < KB; kb++) {
                            _tile_loadd(TA, (const uint8_t*)Hbuf+kb*64, K_DIM*2);
                            int ob0 = obp*4;
                            const uint16_t *Wb = Wv;
                            #define WV_OFF(kb_, ob_) (((kb_)*NB+(ob_))*16*32)
                            _tile_loadd(TB0, &Wb[WV_OFF(kb,ob0+0)], 64);
                            _tile_loadd(TB1, &Wb[WV_OFF(kb,ob0+1)], 64);
                            _tile_dpbf16ps(TC0, TA, TB0);
                            _tile_dpbf16ps(TC1, TA, TB1);
                            _tile_loadd(TB0, &Wb[WV_OFF(kb,ob0+2)], 64);
                            _tile_loadd(TB1, &Wb[WV_OFF(kb,ob0+3)], 64);
                            _tile_dpbf16ps(TC2, TA, TB0);
                            _tile_dpbf16ps(TC3, TA, TB1);
                            #undef WV_OFF
                        }
                    }
                    int col = obp*64;
                    _tile_stored(TC0, Ctmp, 64);
                    for (int n=0;n<batch;n++) memcpy(&C[(size_t)orig_row[n]*K_DIM+col+0], &Ctmp[n*16], 64);
                    _tile_stored(TC1, Ctmp, 64);
                    for (int n=0;n<batch;n++) memcpy(&C[(size_t)orig_row[n]*K_DIM+col+16], &Ctmp[n*16], 64);
                    _tile_stored(TC2, Ctmp, 64);
                    for (int n=0;n<batch;n++) memcpy(&C[(size_t)orig_row[n]*K_DIM+col+32], &Ctmp[n*16], 64);
                    _tile_stored(TC3, Ctmp, 64);
                    for (int n=0;n<batch;n++) memcpy(&C[(size_t)orig_row[n]*K_DIM+col+48], &Ctmp[n*16], 64);
                }
            }
        }
        _tile_release();
    }
}

/* ================================================================
 * ReLU (in-place, FP32)
 * ================================================================ */
static void relu_f32(float *data, size_t count) {
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < count; i++)
        if (data[i] < 0.0f) data[i] = 0.0f;
}

/* ================================================================
 * CSR loading
 * ================================================================ */
struct csr_t { uint32_t *indptr, *indices; float *values; int N; uint32_t nnz; };

static csr_t load_csrbin(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", path); exit(1); }
    uint8_t hdr[36]; fread(hdr, 1, 36, f);
    uint64_t nr, nc, nz;
    memcpy(&nr, &hdr[12], 8); memcpy(&nc, &hdr[20], 8); memcpy(&nz, &hdr[28], 8);
    int N = (int)nr; uint32_t nnz = (uint32_t)nz;
    uint32_t *indptr = (uint32_t*)malloc((size_t)(N+1)*4); fread(indptr, 4, N+1, f);
    uint32_t *indices = (uint32_t*)malloc((size_t)nnz*4); fread(indices, 4, nnz, f);
    float *values = (float*)malloc((size_t)nnz*4);
    size_t rv = fread(values, 4, nnz, f);
    if (rv < (size_t)nnz) for (uint32_t i=0;i<nnz;i++) values[i]=1.0f;
    fclose(f);
    return {indptr, indices, values, N, nnz};
}

/* ================================================================
 * MKL two-step: SpMM + GeMM
 * ================================================================ */
static void mkl_spmm_gemm(
    sparse_matrix_t A, struct matrix_descr descr,
    const float *H, const float *W, float *Z, float *C, int N)
{
    mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE,
        1.0f, A, descr, SPARSE_LAYOUT_ROW_MAJOR,
        H, K_DIM, K_DIM, 0.0f, Z, K_DIM);
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
        N, K_DIM, K_DIM, 1.0f, Z, K_DIM, W, K_DIM, 0.0f, C, K_DIM);
}

/* ================================================================
 * Precision comparison utility
 * ================================================================ */
static void compare_outputs(const float *a, const float *b, size_t count,
                            const char *label) {
    double max_abs_diff = 0, max_abs_val = 0, sum_sq_diff = 0;
    for (size_t i = 0; i < count; i++) {
        double d = fabs((double)a[i] - (double)b[i]);
        double v = fabs((double)b[i]);
        if (d > max_abs_diff) max_abs_diff = d;
        if (v > max_abs_val) max_abs_val = v;
        sum_sq_diff += d * d;
    }
    double rel_err = (max_abs_val > 0) ? max_abs_diff / max_abs_val : 0;
    double rmse = sqrt(sum_sq_diff / count);
    printf("  %s: max_rel_err=%.6f  RMSE=%.6e  max|diff|=%.6e\n",
           label, rel_err, rmse, max_abs_diff);
}

/* ================================================================
 * E2E Benchmark
 * ================================================================ */
#include "source_protocol_runtime.hpp"

static void benchmark(const char *dir, const char *name) {
    /* Load graph */
    char path[512];
    snprintf(path, 512, "%s/%s/%s.csrbin", dir, name, name);
    printf("Loading: %s\n", path);
    csr_t csr = load_csrbin(path);
    int N = csr.N; uint32_t nnz = csr.nnz;
    printf("Matrix: %s  N=%d  NNZ=%u  avg_deg=%.1f\n\n",
           name, N, nnz, (double)nnz/N);

    /* Random weights for 2 layers */
    srand(12345);
    float *W1 = (float*)mkl_malloc((size_t)K_DIM*K_DIM*4, 64);
    float *W2 = (float*)mkl_malloc((size_t)K_DIM*K_DIM*4, 64);
    for (int i = 0; i < K_DIM*K_DIM; i++) {
        W1[i] = 0.01f * ((rand()%200)-100);
        W2[i] = 0.01f * ((rand()%200)-100);
    }

    /* Random initial features H0 */
    float *H0 = (float*)mkl_malloc((size_t)N*K_DIM*4, 64);
    for (size_t i = 0; i < (size_t)N*K_DIM; i++)
        H0[i] = 0.01f * ((rand()%200)-100);

    /* ============================================================
     * TFS V3 Path: BF16 AMX fusion
     * ============================================================ */
    printf("=== TFS V3 (BF16 AMX Fusion) ===\n");

    /* Prepare: degree sort, VNNI weights, BF16 features */
    int *perm = make_degree_perm(csr.indptr, N);

    uint16_t *Wv1 = (uint16_t*)aligned_alloc(64, (size_t)KB*NB*16*32*2);
    uint16_t *Wv2 = (uint16_t*)aligned_alloc(64, (size_t)KB*NB*16*32*2);
    make_W_vnni(W1, Wv1);
    make_W_vnni(W2, Wv2);

    uint16_t *H0_bf16 = (uint16_t*)aligned_alloc(64, (size_t)N*K_DIM*2);
    convert_f32_to_bf16(H0, H0_bf16, (size_t)N*K_DIM);

    float *H1_tfs = (float*)aligned_alloc(64, (size_t)N*K_DIM*4);
    float *H2_tfs = (float*)aligned_alloc(64, (size_t)N*K_DIM*4);
    uint16_t *H1_tfs_bf16 = (uint16_t*)aligned_alloc(64, (size_t)N*K_DIM*2);

    /* Warmup */
    tfs_v3_kernel(csr.indptr, csr.indices, H0_bf16, Wv1, H1_tfs, perm, N, 64);
    relu_f32(H1_tfs, (size_t)N*K_DIM);
    convert_f32_to_bf16(H1_tfs, H1_tfs_bf16, (size_t)N*K_DIM);
    tfs_v3_kernel(csr.indptr, csr.indices, H1_tfs_bf16, Wv2, H2_tfs, perm, N, 64);

    /* Benchmark */
    double best_tfs = 1e30;
    for (int run = 0; run < 5; run++) {
        /* Re-convert H0 each run (simulate fresh inference) */
        convert_f32_to_bf16(H0, H0_bf16, (size_t)N*K_DIM);

        double t0 = omp_get_wtime();

        /* Layer 1: SpMM+GeMM fusion → ReLU → BF16 convert */
        tfs_v3_kernel(csr.indptr, csr.indices, H0_bf16, Wv1, H1_tfs, perm, N, 64);
        relu_f32(H1_tfs, (size_t)N*K_DIM);
        convert_f32_to_bf16(H1_tfs, H1_tfs_bf16, (size_t)N*K_DIM);

        /* Layer 2: SpMM+GeMM fusion */
        tfs_v3_kernel(csr.indptr, csr.indices, H1_tfs_bf16, Wv2, H2_tfs, perm, N, 64);

        double t1 = omp_get_wtime();
        double ms = (t1-t0)*1000.0;
        printf("  run %d: %.2f ms\n", run, ms);
        if (ms < best_tfs) best_tfs = ms;
    }
    printf("  TFS V3 E2E BEST: %.2f ms\n\n", best_tfs);

    /* ============================================================
     * MKL Path: FP32 two-step
     * ============================================================ */
    printf("=== MKL FP32 Two-Step ===\n");

    /* Create MKL sparse handle */
    MKL_INT *rs = (MKL_INT*)malloc((size_t)N*sizeof(MKL_INT));
    MKL_INT *re = (MKL_INT*)malloc((size_t)N*sizeof(MKL_INT));
    MKL_INT *ci = (MKL_INT*)malloc((size_t)nnz*sizeof(MKL_INT));
    for (int i=0;i<N;i++) { rs[i]=csr.indptr[i]; re[i]=csr.indptr[i+1]; }
    for (uint32_t i=0;i<nnz;i++) ci[i]=csr.indices[i];

    sparse_matrix_t A_mkl;
    struct matrix_descr descr; descr.type = SPARSE_MATRIX_TYPE_GENERAL;
    sparse_status_t st = mkl_sparse_s_create_csr(&A_mkl, SPARSE_INDEX_BASE_ZERO,
        N, N, rs, re, ci, csr.values);
    if (st != SPARSE_STATUS_SUCCESS) {
        printf("  MKL create_csr failed (code=%d), skipping\n\n", st);
        goto precision_check;
    }
    mkl_sparse_set_mm_hint(A_mkl, SPARSE_OPERATION_NON_TRANSPOSE, descr,
        SPARSE_LAYOUT_ROW_MAJOR, K_DIM, 10);
    mkl_sparse_optimize(A_mkl);

    {
        float *Z_mkl  = (float*)mkl_malloc((size_t)N*K_DIM*4, 64);
        float *H1_mkl = (float*)mkl_malloc((size_t)N*K_DIM*4, 64);
        float *H2_mkl = (float*)mkl_malloc((size_t)N*K_DIM*4, 64);

        /* Warmup */
        mkl_spmm_gemm(A_mkl, descr, H0, W1, Z_mkl, H1_mkl, N);
        relu_f32(H1_mkl, (size_t)N*K_DIM);
        mkl_spmm_gemm(A_mkl, descr, H1_mkl, W2, Z_mkl, H2_mkl, N);

        /* Benchmark */
        double best_mkl = 1e30;
        for (int run = 0; run < 5; run++) {
            double t0 = omp_get_wtime();

            /* Layer 1: SpMM → GeMM → ReLU */
            mkl_spmm_gemm(A_mkl, descr, H0, W1, Z_mkl, H1_mkl, N);
            relu_f32(H1_mkl, (size_t)N*K_DIM);

            /* Layer 2: SpMM → GeMM */
            mkl_spmm_gemm(A_mkl, descr, H1_mkl, W2, Z_mkl, H2_mkl, N);

            double t1 = omp_get_wtime();
            double ms = (t1-t0)*1000.0;
            printf("  run %d: %.2f ms\n", run, ms);
            if (ms < best_mkl) best_mkl = ms;
        }
        printf("  MKL E2E BEST: %.2f ms\n\n", best_mkl);

        /* ============================================================
         * E2E Summary
         * ============================================================ */
        printf("=== E2E Summary ===\n");
        printf("  TFS V3:  %.2f ms\n", best_tfs);
        printf("  MKL:     %.2f ms\n", best_mkl);
        printf("  Speedup: %.2f×\n\n", best_mkl / best_tfs);

        /* ============================================================
         * Precision Analysis
         * ============================================================ */
        printf("=== Precision Analysis ===\n");

        /* Re-run both to get final outputs for comparison */
        convert_f32_to_bf16(H0, H0_bf16, (size_t)N*K_DIM);
        tfs_v3_kernel(csr.indptr, csr.indices, H0_bf16, Wv1, H1_tfs, perm, N, 64);
        relu_f32(H1_tfs, (size_t)N*K_DIM);
        convert_f32_to_bf16(H1_tfs, H1_tfs_bf16, (size_t)N*K_DIM);
        tfs_v3_kernel(csr.indptr, csr.indices, H1_tfs_bf16, Wv2, H2_tfs, perm, N, 64);

        mkl_spmm_gemm(A_mkl, descr, H0, W1, Z_mkl, H1_mkl, N);
        relu_f32(H1_mkl, (size_t)N*K_DIM);
        mkl_spmm_gemm(A_mkl, descr, H1_mkl, W2, Z_mkl, H2_mkl, N);

        /* Compare layer 1 outputs */
        printf("Layer 1 (after ReLU):\n");
        compare_outputs(H1_tfs, H1_mkl, (size_t)N*K_DIM, "TFS_BF16 vs MKL_FP32");

        /* Compare final outputs */
        printf("Layer 2 (final output):\n");
        compare_outputs(H2_tfs, H2_mkl, (size_t)N*K_DIM, "TFS_BF16 vs MKL_FP32");

        /* Print first few values for inspection */
        printf("\nFirst 8 output values:\n");
        printf("  TFS: ");
        for (int i=0;i<8;i++) printf("%.4f ", H2_tfs[i]);
        printf("\n  MKL: ");
        for (int i=0;i<8;i++) printf("%.4f ", H2_mkl[i]);
        printf("\n");

        gcn_extra_source::run_source_candidates(csr, perm, H0, W1, W2,
            Wv1, Wv2, H0_bf16, H1_tfs, H2_tfs, H1_tfs_bf16,
            A_mkl, descr, Z_mkl, H1_mkl, H2_mkl, name);

        mkl_free(Z_mkl); mkl_free(H1_mkl); mkl_free(H2_mkl);
        mkl_sparse_destroy(A_mkl);
    }

precision_check:
    /* Cleanup */
    free(rs); free(re); free(ci);
    free(perm);
    free(Wv1); free(Wv2);
    free(H0_bf16); free(H1_tfs); free(H2_tfs); free(H1_tfs_bf16);
    mkl_free(W1); mkl_free(W2); mkl_free(H0);
    free(csr.indptr); free(csr.indices); free(csr.values);
}

/* ================================================================
 * Main
 * ================================================================ */
int main(int argc, char **argv) {
    printf("GCN E2E Inference Benchmark (2-layer, K=%d)\n", K_DIM);
    printf("TFS V3 (BF16 AMX) vs MKL (FP32 two-step)\n");
    printf("Threads: %d\n\n", omp_get_max_threads());

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <data_dir> <matrix_name|ALL>\n", argv[0]);
        return 1;
    }

    const char *dir = argv[1], *name = argv[2];

    const char *all[] = {
        "web-Google", "amazon0601", "cit-Patents", "as-Skitter",
        "soc-Pokec", "hollywood-2009", "indochina-2004",
        "ogbn-products", "reddit", "com-LiveJournal"
    };

    if (strcmp(name, "ALL") == 0) {
        for (int i = 0; i < 10; i++) {
            printf("############################################\n");
            printf("# %s\n", all[i]);
            printf("############################################\n\n");
            benchmark(dir, all[i]);
            printf("\n");
        }
    } else {
        benchmark(dir, name);
    }

    return 0;
}
