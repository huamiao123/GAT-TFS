/**
 * MKL SpMM Baseline
 * 
 * Uses mkl_sparse_s_mm (FP32, CSR format)
 * Fair comparison: same node, same threads, same matrices, same K
 * 
 * Also includes our AVX-512 FP32 baseline for cross-validation
 * (ensures our AVX baseline isn't artificially slow)
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <vector>
#include <chrono>
#include <mkl.h>
#include <mkl_spblas.h>
#include <immintrin.h>
#include <omp.h>

struct CSR {
    std::vector<uint32_t> indptr, indices;
    std::vector<float> values;
    int M, N; int64_t nnz;
};

CSR read_csrbin(const char* path) {
    CSR c; FILE* f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "ERR: %s\n", path); exit(1); }
    uint32_t hdr[3]; uint64_t dims[3];
    fread(hdr, 4, 3, f); fread(dims, 8, 3, f);
    c.M = (int)dims[0]; c.N = (int)dims[1]; c.nnz = (int64_t)dims[2];
    c.indptr.resize(c.M + 1); c.indices.resize(c.nnz); c.values.resize(c.nnz);
    fread(c.indptr.data(), 4, c.M + 1, f);
    fread(c.indices.data(), 4, c.nnz, f);
    fread(c.values.data(), 4, c.nnz, f);
    fclose(f); return c;
}

// Our AVX-512 baseline (same as in all our experiments)
template<int K>
void avx512_spmm(const CSR& csr, const float* B, float* C) {
    constexpr int NVEC = K / 16;
    #pragma omp parallel for schedule(dynamic, 256)
    for (int i = 0; i < csr.M; i++) {
        float* Cr = C + (int64_t)i * K;
        __m512 v[NVEC];
        for (int j = 0; j < NVEC; j++) v[j] = _mm512_setzero_ps();
        for (uint32_t p = csr.indptr[i]; p < csr.indptr[i + 1]; p++) {
            __m512 a = _mm512_set1_ps(csr.values[p]);
            const float* Br = B + (int64_t)csr.indices[p] * K;
            for (int j = 0; j < NVEC; j++)
                v[j] = _mm512_fmadd_ps(a, _mm512_loadu_ps(Br + j * 16), v[j]);
        }
        for (int j = 0; j < NVEC; j++) _mm512_storeu_ps(Cr + j * 16, v[j]);
    }
}

template<int K>
void bench_one_k(const CSR& csr, int threads) {
    printf("\n=============== K=%d ===============\n", K);

    // Allocate B (column-major for MKL, row-major for ours)
    // MKL sparse_s_mm: C = alpha * A * B + beta * C
    // B is dense: N × K (column-major) or N × K (row-major)
    // We test both layouts for MKL

    int64_t BN = (int64_t)csr.N * K;
    int64_t CM = (int64_t)csr.M * K;

    float* B_row = (float*)mkl_malloc(BN * sizeof(float), 64);   // row-major
    float* B_col = (float*)mkl_malloc(BN * sizeof(float), 64);   // col-major
    float* C_mkl = (float*)mkl_malloc(CM * sizeof(float), 64);
    float* C_avx = (float*)mkl_malloc(CM * sizeof(float), 64);

    srand(12345);
    for (int64_t i = 0; i < BN; i++) B_row[i] = (rand() % 200 - 100) / 100.0f;

    // Convert to column-major: B_col[j + n*K] = B_row[n*K + j]
    for (int n = 0; n < csr.N; n++)
        for (int j = 0; j < K; j++)
            B_col[j * csr.N + n] = B_row[n * K + j];

    // ==================== MKL SpMM ====================
    // Create sparse matrix handle
    // MKL uses MKL_INT for indexing - need to convert if necessary
    std::vector<MKL_INT> ia(csr.M + 1), ja(csr.nnz);
    for (int i = 0; i <= csr.M; i++) ia[i] = (MKL_INT)csr.indptr[i];
    for (int64_t i = 0; i < csr.nnz; i++) ja[i] = (MKL_INT)csr.indices[i];

    sparse_matrix_t A_mkl;
    sparse_status_t status;

    status = mkl_sparse_s_create_csr(&A_mkl, SPARSE_INDEX_BASE_ZERO,
        csr.M, csr.N, ia.data(), ia.data() + 1, ja.data(),
        const_cast<float*>(csr.values.data()));
    if (status != SPARSE_STATUS_SUCCESS) {
        printf("  MKL create_csr failed: %d\n", status);
        return;
    }

    // Optimize (inspector-executor)
    struct matrix_descr descr;
    descr.type = SPARSE_MATRIX_TYPE_GENERAL;
    
    // Hint for SpMM
    mkl_sparse_set_mm_hint(A_mkl, SPARSE_OPERATION_NON_TRANSPOSE,
        descr, SPARSE_LAYOUT_ROW_MAJOR, K, 10);
    mkl_sparse_optimize(A_mkl);

    // --- MKL row-major ---
    memset(C_mkl, 0, CM * sizeof(float));
    // warmup
    mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE, 1.0f, A_mkl,
        descr, SPARSE_LAYOUT_ROW_MAJOR, B_row, K, K,  // ldB = K
        0.0f, C_mkl, K);  // ldC = K

    double best_mkl_row = 1e9;
    for (int t = 0; t < 10; t++) {
        memset(C_mkl, 0, CM * sizeof(float));
        auto T0 = std::chrono::high_resolution_clock::now();
        mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE, 1.0f, A_mkl,
            descr, SPARSE_LAYOUT_ROW_MAJOR, B_row, K, K,
            0.0f, C_mkl, K);
        auto T1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(T1 - T0).count();
        if (ms < best_mkl_row) best_mkl_row = ms;
    }
    printf("  MKL  (row-major):  %8.2f ms\n", best_mkl_row);

    // --- MKL column-major ---
    float* C_mkl_col = (float*)mkl_malloc(CM * sizeof(float), 64);
    memset(C_mkl_col, 0, CM * sizeof(float));
    
    mkl_sparse_set_mm_hint(A_mkl, SPARSE_OPERATION_NON_TRANSPOSE,
        descr, SPARSE_LAYOUT_COLUMN_MAJOR, K, 10);
    mkl_sparse_optimize(A_mkl);

    mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE, 1.0f, A_mkl,
        descr, SPARSE_LAYOUT_COLUMN_MAJOR, B_col, K, csr.N,
        0.0f, C_mkl_col, csr.M);

    double best_mkl_col = 1e9;
    for (int t = 0; t < 10; t++) {
        memset(C_mkl_col, 0, CM * sizeof(float));
        auto T0 = std::chrono::high_resolution_clock::now();
        mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE, 1.0f, A_mkl,
            descr, SPARSE_LAYOUT_COLUMN_MAJOR, B_col, K, csr.N,
            0.0f, C_mkl_col, csr.M);
        auto T1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(T1 - T0).count();
        if (ms < best_mkl_col) best_mkl_col = ms;
    }
    printf("  MKL  (col-major):  %8.2f ms\n", best_mkl_col);
    double best_mkl = std::min(best_mkl_row, best_mkl_col);
    printf("  MKL  (best):       %8.2f ms\n", best_mkl);

    mkl_free(C_mkl_col);

    // ==================== Our AVX-512 ====================
    memset(C_avx, 0, CM * sizeof(float));
    avx512_spmm<K>(csr, B_row, C_avx);
    double best_avx = 1e9;
    for (int t = 0; t < 10; t++) {
        memset(C_avx, 0, CM * sizeof(float));
        auto T0 = std::chrono::high_resolution_clock::now();
        avx512_spmm<K>(csr, B_row, C_avx);
        auto T1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(T1 - T0).count();
        if (ms < best_avx) best_avx = ms;
    }
    printf("  AVX-512 (ours):    %8.2f ms\n", best_avx);

    // Cross-validation: compare MKL vs AVX results
    int bad = 0; float mx = 0;
    for (int64_t i = 0; i < CM; i++) {
        float ae = fabsf(C_mkl[i] - C_avx[i]);
        if (ae > mx) mx = ae;
        float ref = fabsf(C_mkl[i]);
        float re = (ref > 1e-6f) ? ae / ref : ae;
        if (re > 0.01f && ref > 0.01f) bad++;
    }
    printf("  MKL vs AVX diff:   max=%.3e bad=%.4f%%\n", mx, 100.0 * bad / CM);

    printf("  ─────────────────────────────────\n");
    printf("  AVX/MKL ratio:     %.2fx (>1 = our AVX faster)\n", best_mkl / best_avx);

    mkl_sparse_destroy(A_mkl);
    mkl_free(B_row); mkl_free(B_col); mkl_free(C_mkl); mkl_free(C_avx);
}

int main(int argc, char** argv) {
    if (argc < 2) { printf("Usage: %s <mat> [thr]\n", argv[0]); return 1; }
    int nt = (argc >= 3) ? atoi(argv[2]) : omp_get_num_procs();
    omp_set_num_threads(nt);
    mkl_set_num_threads(nt);

    printf("=== MKL SpMM Baseline Comparison ===\n");
    printf("threads=%d MKL=%s\n\n", nt, __INTEL_MKL__  ? "yes" : "no");

    CSR csr = read_csrbin(argv[1]);
    printf("Matrix: M=%d N=%d NNZ=%ld avg_deg=%.1f\n",
        csr.M, csr.N, csr.nnz, (double)csr.nnz / csr.M);

    bench_one_k<32>(csr, nt);
    bench_one_k<64>(csr, nt);
    bench_one_k<128>(csr, nt);

    printf("\n=== Done ===\n");
    return 0;
}
