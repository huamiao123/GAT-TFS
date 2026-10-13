/*
 * mkl_baseline.cpp -- MKL two-step: SpMM + GeMM
 * Compile: icpx -O3 -march=sapphirerapids -qopenmp -qmkl=parallel \
 *          -o mkl_baseline mkl_baseline.cpp
 */

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <mkl.h>
#include <mkl_spblas.h>
#include <omp.h>

struct csr_t {
    uint32_t *indptr, *indices;
    float *values;
    int N; uint32_t nnz;
};

static csr_t load_csrbin(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", path); exit(1); }
    uint8_t hdr[36];
    fread(hdr, 1, 36, f);
    uint32_t type; uint64_t nr, nc, nz;
    memcpy(&type, &hdr[8], 4);
    memcpy(&nr, &hdr[12], 8);
    memcpy(&nc, &hdr[20], 8);
    memcpy(&nz, &hdr[28], 8);
    int N = (int)nr; uint32_t nnz = (uint32_t)nz;
    printf("  csrbin: N=%d  NNZ=%u\n", N, nnz);

    uint32_t *indptr = (uint32_t*)malloc((size_t)(N+1)*4);
    fread(indptr, 4, N+1, f);
    uint32_t *indices = (uint32_t*)malloc((size_t)nnz*4);
    fread(indices, 4, nnz, f);
    float *values = (float*)malloc((size_t)nnz*4);
    size_t rv = fread(values, 4, nnz, f);
    if (rv < (size_t)nnz)
        for (uint32_t i = 0; i < nnz; i++) values[i] = 1.0f;
    fclose(f);

    return {indptr, indices, values, N, nnz};
}

static void benchmark(const char *dir, const char *name, int K)
{
    char path[512];
    snprintf(path, 512, "%s/%s/%s.csrbin", dir, name, name);
    printf("Loading: %s\n", path);
    csr_t csr = load_csrbin(path);
    int N = csr.N; uint32_t nnz = csr.nnz;
    printf("Matrix: %s  N=%d  NNZ=%u  avg_deg=%.1f  K=%d\n",
           name, N, nnz, (double)nnz/N, K);

    MKL_INT *rs = (MKL_INT*)malloc((size_t)N*sizeof(MKL_INT));
    MKL_INT *re = (MKL_INT*)malloc((size_t)N*sizeof(MKL_INT));
    MKL_INT *ci = (MKL_INT*)malloc((size_t)nnz*sizeof(MKL_INT));
    for (int i=0;i<N;i++) { rs[i]=csr.indptr[i]; re[i]=csr.indptr[i+1]; }
    for (uint32_t i=0;i<nnz;i++) ci[i]=csr.indices[i];

    sparse_matrix_t A;
    struct matrix_descr descr;
    descr.type = SPARSE_MATRIX_TYPE_GENERAL;

    sparse_status_t st = mkl_sparse_s_create_csr(
        &A, SPARSE_INDEX_BASE_ZERO, N, N, rs, re, ci, csr.values);
    if (st != SPARSE_STATUS_SUCCESS) {
        fprintf(stderr, "create_csr failed: %d\n", st); exit(1);
    }
    mkl_sparse_set_mm_hint(A, SPARSE_OPERATION_NON_TRANSPOSE, descr,
                           SPARSE_LAYOUT_ROW_MAJOR, K, 10);
    mkl_sparse_optimize(A);

    srand(12345);
    float *H = (float*)mkl_malloc((size_t)N*K*4, 64);
    float *W = (float*)mkl_malloc((size_t)K*K*4, 64);
    float *Z = (float*)mkl_malloc((size_t)N*K*4, 64);
    float *C = (float*)mkl_malloc((size_t)N*K*4, 64);
    if (!H||!W||!Z||!C) { fprintf(stderr,"OOM\n"); exit(1); }
    for (size_t i=0;i<(size_t)N*K;i++) H[i]=0.01f*((rand()%200)-100);
    for (int i=0;i<K*K;i++) W[i]=0.01f*((rand()%200)-100);

    /* Warmup */
    mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE,
                    1.0f, A, descr, SPARSE_LAYOUT_ROW_MAJOR,
                    H, K, K, 0.0f, Z, K);
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                N, K, K, 1.0f, Z, K, W, K, 0.0f, C, K);

    double best_spmm=1e30, best_gemm=1e30, best_total=1e30;
    for (int run=0; run<5; run++) {
        double t0 = omp_get_wtime();
        mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE,
                        1.0f, A, descr, SPARSE_LAYOUT_ROW_MAJOR,
                        H, K, K, 0.0f, Z, K);
        double t1 = omp_get_wtime();
        cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                    N, K, K, 1.0f, Z, K, W, K, 0.0f, C, K);
        double t2 = omp_get_wtime();
        double sm = (t1-t0)*1000, gm = (t2-t1)*1000, tot = sm+gm;
        printf("  run %d: SpMM=%.2f  GeMM=%.2f  Total=%.2f ms\n",
               run, sm, gm, tot);
        if (sm<best_spmm) best_spmm=sm;
        if (gm<best_gemm) best_gemm=gm;
        if (tot<best_total) best_total=tot;
    }
    printf("  BEST: SpMM=%.2f  GeMM=%.2f  Total=%.2f ms\n\n",
           best_spmm, best_gemm, best_total);

    mkl_sparse_destroy(A);
    mkl_free(H); mkl_free(W); mkl_free(Z); mkl_free(C);
    free(rs); free(re); free(ci);
    free(csr.indptr); free(csr.indices); free(csr.values);
}

int main(int argc, char **argv)
{
    printf("MKL Two-Step Baseline: SpMM + GeMM\n");
    printf("Threads: %d\n\n", omp_get_max_threads());

    if (argc < 3) {
        fprintf(stderr,
            "Usage: %s <dir> <name|ALL> [K=128|0=sweep]\n", argv[0]);
        return 1;
    }

    const char *dir=argv[1], *name=argv[2];
    int K = (argc>=4) ? atoi(argv[3]) : 128;

    const char *all[] = {
        "web-Google","amazon0601","cit-Patents","as-Skitter",
        "soc-Pokec","hollywood-2009","indochina-2004"
    };

    if (strcmp(name,"ALL")==0) {
        if (K==0) {
            int Ks[]={32,64,128,256};
            for (int ki=0;ki<4;ki++) {
                printf("########################################\n");
                printf("# K = %d\n", Ks[ki]);
                printf("########################################\n\n");
                for (int mi=0;mi<7;mi++) benchmark(dir,all[mi],Ks[ki]);
            }
        } else {
            for (int mi=0;mi<7;mi++) benchmark(dir,all[mi],K);
        }
    } else {
        if (K==0) {
            int Ks[]={32,64,128,256};
            for (int ki=0;ki<4;ki++) benchmark(dir,name,Ks[ki]);
        } else {
            benchmark(dir,name,K);
        }
    }
    return 0;
}
