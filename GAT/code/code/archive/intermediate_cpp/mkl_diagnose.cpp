#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <vector>
#include <chrono>
#include <mkl.h>
#include <mkl_spblas.h>
#include <omp.h>

struct CSR {
    std::vector<uint32_t> indptr, indices;
    std::vector<float> values;
    int M, N; int64_t nnz;
};

CSR read_csrbin(const char* path) {
    CSR c; FILE* f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "ERR: %s\n", path); exit(1); }
    
    // 获取文件大小
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    uint32_t hdr[3]; uint64_t dims[3];
    fread(hdr, 4, 3, f); fread(dims, 8, 3, f);
    c.M = (int)dims[0]; c.N = (int)dims[1]; c.nnz = (int64_t)dims[2];
    
    long header_size = 12 + 24; // 3*4 + 3*8
    long indptr_size = (c.M + 1) * 4;
    long indices_size = c.nnz * 4;
    long values_size = c.nnz * 4;
    long expected_with_values = header_size + indptr_size + indices_size + values_size;
    long expected_without_values = header_size + indptr_size + indices_size;
    
    printf("  File size: %ld bytes\n", fsize);
    printf("  Expected with values: %ld bytes\n", expected_with_values);
    printf("  Expected without values: %ld bytes\n", expected_without_values);
    
    c.indptr.resize(c.M + 1); c.indices.resize(c.nnz); c.values.resize(c.nnz);
    fread(c.indptr.data(), 4, c.M + 1, f);
    fread(c.indices.data(), 4, c.nnz, f);
    
    // 检查是否还有数据可读（values段）
    long pos_before_values = ftell(f);
    size_t vals_read = fread(c.values.data(), 4, c.nnz, f);
    printf("  Values read: %zu / %ld (expected)\n", vals_read, c.nnz);
    
    if (vals_read == 0) {
        printf("  *** WARNING: No values in file! Generating random values ***\n");
        srand(42);
        for (int64_t i = 0; i < c.nnz; i++)
            c.values[i] = (rand() % 200 - 100) / 100.0f;
    }
    
    // 检查 values 是否全0
    float vsum = 0;
    for (int64_t i = 0; i < std::min(c.nnz, (int64_t)1000); i++)
        vsum += fabsf(c.values[i]);
    printf("  Values sample sum (first 1000): %f\n", vsum);
    if (vsum == 0) printf("  *** WARNING: Values appear to be all zeros! ***\n");
    
    fclose(f); return c;
}

int main(int argc, char** argv) {
    if (argc < 2) { printf("Usage: %s <mat> [thr]\n", argv[0]); return 1; }
    int nt = (argc >= 3) ? atoi(argv[2]) : 32;
    
    printf("=== MKL Diagnostic ===\n\n");
    
    // 诊断 1: 线程配置
    printf("[1] Thread Configuration:\n");
    omp_set_num_threads(nt);
    mkl_set_num_threads(nt);
    mkl_set_dynamic(0);
    printf("  Requested threads: %d\n", nt);
    printf("  mkl_get_max_threads(): %d\n", mkl_get_max_threads());
    printf("  omp_get_max_threads(): %d\n", omp_get_max_threads());
    printf("  mkl_get_dynamic(): %d\n", mkl_get_dynamic());
    
    #pragma omp parallel
    {
        #pragma omp single
        printf("  Actual OMP threads: %d\n", omp_get_num_threads());
    }
    
    // 诊断 2: 读矩阵（带 values 检查）
    printf("\n[2] Matrix Loading:\n");
    CSR csr = read_csrbin(argv[1]);
    printf("  M=%d N=%d NNZ=%ld avg_deg=%.1f\n",
        csr.M, csr.N, csr.nnz, (double)csr.nnz / csr.M);
    
    // 诊断 3: MKL 创建和优化
    printf("\n[3] MKL Sparse Setup:\n");
    std::vector<MKL_INT> ia(csr.M + 1), ja(csr.nnz);
    for (int i = 0; i <= csr.M; i++) ia[i] = (MKL_INT)csr.indptr[i];
    for (int64_t i = 0; i < csr.nnz; i++) ja[i] = (MKL_INT)csr.indices[i];
    
    sparse_matrix_t A_mkl;
    mkl_sparse_s_create_csr(&A_mkl, SPARSE_INDEX_BASE_ZERO,
        csr.M, csr.N, ia.data(), ia.data() + 1, ja.data(), csr.values.data());
    
    struct matrix_descr descr;
    descr.type = SPARSE_MATRIX_TYPE_GENERAL;
    
    mkl_sparse_set_mm_hint(A_mkl, SPARSE_OPERATION_NON_TRANSPOSE,
        descr, SPARSE_LAYOUT_ROW_MAJOR, 128, 10);
    sparse_status_t opt_status = mkl_sparse_optimize(A_mkl);
    printf("  optimize status: %d (0=success)\n", opt_status);
    
    // 诊断 4: MKL_VERBOSE 运行
    printf("\n[4] MKL SpMM Performance (K=128):\n");
    int K = 128;
    int64_t BN = (int64_t)csr.N * K;
    int64_t CM = (int64_t)csr.M * K;
    float* B = (float*)mkl_malloc(BN * sizeof(float), 64);
    float* C = (float*)mkl_malloc(CM * sizeof(float), 64);
    srand(12345);
    for (int64_t i = 0; i < BN; i++) B[i] = (rand() % 200 - 100) / 100.0f;
    
    // Warmup 5 次
    for (int w = 0; w < 5; w++) {
        memset(C, 0, CM * sizeof(float));
        mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE, 1.0f, A_mkl,
            descr, SPARSE_LAYOUT_ROW_MAJOR, B, K, K, 0.0f, C, K);
    }
    printf("  Warmup done (5 iterations)\n");
    
    // 精确计时 20 次
    double times[20];
    for (int t = 0; t < 20; t++) {
        memset(C, 0, CM * sizeof(float));
        auto T0 = std::chrono::high_resolution_clock::now();
        mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE, 1.0f, A_mkl,
            descr, SPARSE_LAYOUT_ROW_MAJOR, B, K, K, 0.0f, C, K);
        auto T1 = std::chrono::high_resolution_clock::now();
        times[t] = std::chrono::duration<double, std::milli>(T1 - T0).count();
    }
    
    // 排序取中位数
    for (int i = 0; i < 19; i++)
        for (int j = i+1; j < 20; j++)
            if (times[j] < times[i]) { double tmp=times[i]; times[i]=times[j]; times[j]=tmp; }
    
    printf("  MKL K=128 (20 runs):\n");
    printf("    min=%.2f  median=%.2f  max=%.2f ms\n",
        times[0], times[10], times[19]);
    printf("    All: ");
    for (int t = 0; t < 20; t++) printf("%.1f ", times[t]);
    printf("\n");
    
    // 诊断 5: 简单的行并行 SpMM 作为 sanity check
    printf("\n[5] Naive parallel SpMM (sanity check):\n");
    float* C2 = (float*)mkl_malloc(CM * sizeof(float), 64);
    
    // warmup
    memset(C2, 0, CM * sizeof(float));
    #pragma omp parallel for schedule(dynamic, 256)
    for (int i = 0; i < csr.M; i++) {
        for (uint32_t p = csr.indptr[i]; p < csr.indptr[i+1]; p++) {
            float a = csr.values[p];
            int col = csr.indices[p];
            for (int k = 0; k < K; k++)
                C2[(int64_t)i*K + k] += a * B[(int64_t)col*K + k];
        }
    }
    
    double best_naive = 1e9;
    for (int t = 0; t < 5; t++) {
        memset(C2, 0, CM * sizeof(float));
        auto T0 = std::chrono::high_resolution_clock::now();
        #pragma omp parallel for schedule(dynamic, 256)
        for (int i = 0; i < csr.M; i++) {
            for (uint32_t p = csr.indptr[i]; p < csr.indptr[i+1]; p++) {
                float a = csr.values[p];
                int col = csr.indices[p];
                for (int k = 0; k < K; k++)
                    C2[(int64_t)i*K + k] += a * B[(int64_t)col*K + k];
            }
        }
        auto T1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(T1 - T0).count();
        if (ms < best_naive) best_naive = ms;
    }
    printf("  Naive parallel: %.2f ms\n", best_naive);
    printf("  MKL / Naive ratio: %.2fx\n", times[0] / best_naive);
    
    // 检查 MKL vs Naive 结果一致性
    float mx = 0; int bad = 0;
    for (int64_t i = 0; i < CM; i++) {
        float ae = fabsf(C[i] - C2[i]);
        if (ae > mx) mx = ae;
        if (fabsf(C[i]) > 0.01f && ae / fabsf(C[i]) > 0.01f) bad++;
    }
    printf("  MKL vs Naive diff: max=%.3e bad=%d\n", mx, bad);
    
    mkl_sparse_destroy(A_mkl);
    mkl_free(B); mkl_free(C); mkl_free(C2);
    printf("\n=== Done ===\n");
    return 0;
}
