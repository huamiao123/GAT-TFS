/**
 * MKL Verification
 * 
 * Possible issues:
 *   1. MKL not using all threads?
 *   2. Wrong API parameters (ldb, ldc)?
 *   3. Inspector-executor overhead counted in timing?
 *   4. Should use mkl_scsrmm (older API, sometimes faster)?
 *   5. Need more warmup iterations?
 * 
 * This test isolates each possibility.
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
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
    uint32_t hdr[3]; uint64_t dims[3];
    fread(hdr,4,3,f); fread(dims,8,3,f);
    c.M=(int)dims[0]; c.N=(int)dims[1]; c.nnz=(int64_t)dims[2];
    c.indptr.resize(c.M+1); c.indices.resize(c.nnz); c.values.resize(c.nnz);
    fread(c.indptr.data(),4,c.M+1,f);
    fread(c.indices.data(),4,c.nnz,f);
    fread(c.values.data(),4,c.nnz,f);
    fclose(f); return c;
}

int main(int argc, char** argv) {
    if (argc < 2) { printf("Usage: %s <mat> [thr]\n", argv[0]); return 1; }
    int nt = (argc >= 3) ? atoi(argv[2]) : 32;

    printf("=== MKL Verification ===\n\n");

    // ============ CHECK 1: Thread settings ============
    omp_set_num_threads(nt);
    mkl_set_num_threads(nt);
    
    printf("[Check 1] Thread configuration:\n");
    printf("  omp_get_max_threads() = %d\n", omp_get_max_threads());
    printf("  mkl_get_max_threads() = %d\n", mkl_get_max_threads());
    printf("  MKL_NUM_THREADS env   = %s\n", getenv("MKL_NUM_THREADS") ? getenv("MKL_NUM_THREADS") : "(not set)");
    printf("  OMP_NUM_THREADS env   = %s\n", getenv("OMP_NUM_THREADS") ? getenv("OMP_NUM_THREADS") : "(not set)");
    
    // Verify actual parallel execution
    int actual_threads = 0;
    #pragma omp parallel
    {
        #pragma omp atomic
        actual_threads++;
    }
    printf("  Actual OMP threads    = %d\n\n", actual_threads);

    CSR csr = read_csrbin(argv[1]);
    printf("Matrix: M=%d N=%d NNZ=%ld avg_deg=%.1f\n\n",
        csr.M, csr.N, csr.nnz, (double)csr.nnz/csr.M);

    // Convert to MKL_INT
    std::vector<MKL_INT> ia(csr.M+1), ja(csr.nnz);
    for (int i = 0; i <= csr.M; i++) ia[i] = (MKL_INT)csr.indptr[i];
    for (int64_t i = 0; i < csr.nnz; i++) ja[i] = (MKL_INT)csr.indices[i];

    int K = 128;
    float* B = (float*)mkl_malloc((size_t)csr.N * K * sizeof(float), 64);
    float* C = (float*)mkl_malloc((size_t)csr.M * K * sizeof(float), 64);
    srand(12345);
    for (int64_t i = 0; i < (int64_t)csr.N * K; i++) B[i] = (rand()%200-100)/100.0f;

    // ============ CHECK 2: SpMV first (MKL should be fast here) ============
    printf("[Check 2] SpMV (K=1) — MKL should excel here:\n");
    {
        float* x = (float*)mkl_malloc(csr.N * sizeof(float), 64);
        float* y = (float*)mkl_malloc(csr.M * sizeof(float), 64);
        for (int i = 0; i < csr.N; i++) x[i] = B[i];

        sparse_matrix_t A;
        mkl_sparse_s_create_csr(&A, SPARSE_INDEX_BASE_ZERO,
            csr.M, csr.N, ia.data(), ia.data()+1, ja.data(),
            const_cast<float*>(csr.values.data()));
        struct matrix_descr descr; descr.type = SPARSE_MATRIX_TYPE_GENERAL;
        mkl_sparse_optimize(A);

        // warmup
        for(int t=0;t<5;t++)
            mkl_sparse_s_mv(SPARSE_OPERATION_NON_TRANSPOSE, 1.0f, A, descr, x, 0.0f, y);

        double best = 1e9;
        for (int t = 0; t < 20; t++) {
            auto T0 = std::chrono::high_resolution_clock::now();
            mkl_sparse_s_mv(SPARSE_OPERATION_NON_TRANSPOSE, 1.0f, A, descr, x, 0.0f, y);
            auto T1 = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(T1-T0).count();
            if (ms < best) best = ms;
        }
        printf("  MKL SpMV: %.2f ms  (%.2f GFLOP/s)\n", best,
            2.0*csr.nnz/best/1e6);

        mkl_sparse_destroy(A);
        mkl_free(x); mkl_free(y);
    }

    // ============ CHECK 3: New API (mkl_sparse_s_mm) with LOTS of warmup ============
    printf("\n[Check 3] mkl_sparse_s_mm with extended warmup (K=%d):\n", K);
    {
        sparse_matrix_t A;
        mkl_sparse_s_create_csr(&A, SPARSE_INDEX_BASE_ZERO,
            csr.M, csr.N, ia.data(), ia.data()+1, ja.data(),
            const_cast<float*>(csr.values.data()));
        struct matrix_descr descr; descr.type = SPARSE_MATRIX_TYPE_GENERAL;

        // Set hint and optimize
        mkl_sparse_set_mm_hint(A, SPARSE_OPERATION_NON_TRANSPOSE,
            descr, SPARSE_LAYOUT_ROW_MAJOR, K, 100);
        mkl_sparse_optimize(A);

        // LOTS of warmup (20 iterations)
        for (int t = 0; t < 20; t++)
            mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE, 1.0f, A,
                descr, SPARSE_LAYOUT_ROW_MAJOR, B, K, K, 0.0f, C, K);

        // Timed runs
        double best = 1e9;
        for (int t = 0; t < 20; t++) {
            auto T0 = std::chrono::high_resolution_clock::now();
            mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE, 1.0f, A,
                descr, SPARSE_LAYOUT_ROW_MAJOR, B, K, K, 0.0f, C, K);
            auto T1 = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(T1-T0).count();
            if (ms < best) best = ms;
        }
        printf("  mkl_sparse_s_mm (row, 20 warmup): %.2f ms\n", best);

        mkl_sparse_destroy(A);
    }

    // ============ CHECK 4: Old API (mkl_scsrmm) ============
    printf("\n[Check 4] mkl_scsrmm (legacy API, K=%d):\n", K);
    {
        // mkl_scsrmm: C := alpha*A*B + beta*C
        // A is M×N in CSR, B is N×K row-major, C is M×K row-major
        char transa = 'N';
        MKL_INT m = csr.M, n = K, k = csr.N;
        float alpha = 1.0f, beta = 0.0f;
        char matdescra[6] = "GXXF";  // General, -, -, Fortran-style (1-based?)
        
        // mkl_scsrmm needs 1-based indexing for matdescra 'F'
        // Or use 'C' for 0-based
        matdescra[3] = 'C';  // 0-based indexing

        // warmup
        for(int t=0;t<10;t++)
            mkl_scsrmm(&transa, &m, &n, &k, &alpha, matdescra,
                csr.values.data(), (MKL_INT*)ja.data(), (MKL_INT*)ia.data(), (MKL_INT*)(ia.data()+1),
                B, &n, &beta, C, &n);

        double best = 1e9;
        for (int t = 0; t < 20; t++) {
            auto T0 = std::chrono::high_resolution_clock::now();
            mkl_scsrmm(&transa, &m, &n, &k, &alpha, matdescra,
                csr.values.data(), (MKL_INT*)ja.data(), (MKL_INT*)ia.data(), (MKL_INT*)(ia.data()+1),
                B, &n, &beta, C, &n);
            auto T1 = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(T1-T0).count();
            if (ms < best) best = ms;
        }
        printf("  mkl_scsrmm (legacy): %.2f ms\n", best);
    }

    // ============ CHECK 5: cblas_sgemm control (dense × dense, M×N × N×K) ============
    // Just to verify MKL threads work at all
    printf("\n[Check 5] Dense GEMM control (1000×1000 × 1000×128):\n");
    {
        int D = 1000;
        float* DA = (float*)mkl_malloc(D*D*sizeof(float), 64);
        float* DB = (float*)mkl_malloc(D*K*sizeof(float), 64);
        float* DC = (float*)mkl_malloc(D*K*sizeof(float), 64);
        for(int i=0;i<D*D;i++) DA[i]=0.01f;
        for(int i=0;i<D*K;i++) DB[i]=0.01f;
        
        cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
            D, K, D, 1.0f, DA, D, DB, K, 0.0f, DC, K);
        
        double best=1e9;
        for(int t=0;t<10;t++){
            auto T0=std::chrono::high_resolution_clock::now();
            cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                D, K, D, 1.0f, DA, D, DB, K, 0.0f, DC, K);
            auto T1=std::chrono::high_resolution_clock::now();
            double ms=std::chrono::duration<double,std::milli>(T1-T0).count();
            if(ms<best)best=ms;
        }
        double gflops = 2.0*D*D*K/best/1e6;
        printf("  cblas_sgemm: %.2f ms (%.1f GFLOP/s)\n", best, gflops);
        printf("  → If GFLOP/s is reasonable, MKL threads are working\n");
        
        mkl_free(DA); mkl_free(DB); mkl_free(DC);
    }

    // ============ CHECK 6: Parameter sanity ============
    printf("\n[Check 6] API parameter verification (K=%d):\n", K);
    printf("  Row-major: B is %d×%d, ldB=%d, C is %d×%d, ldC=%d\n",
        csr.N, K, K, csr.M, K, K);
    printf("  Col-major: B is %d×%d, ldB=%d, C is %d×%d, ldC=%d\n",
        csr.N, K, csr.N, csr.M, K, csr.M);
    printf("  sizeof(MKL_INT) = %zu\n", sizeof(MKL_INT));
    printf("  ia[0]=%ld ia[M]=%ld (should be 0 and NNZ=%ld)\n",
        (long)ia[0], (long)ia[csr.M], (long)csr.nnz);

    // ============ CHECK 7: Multiple K values ============
    printf("\n[Check 7] mkl_sparse_s_mm across K values:\n");
    {
        sparse_matrix_t A;
        mkl_sparse_s_create_csr(&A, SPARSE_INDEX_BASE_ZERO,
            csr.M, csr.N, ia.data(), ia.data()+1, ja.data(),
            const_cast<float*>(csr.values.data()));
        struct matrix_descr descr; descr.type = SPARSE_MATRIX_TYPE_GENERAL;

        for (int kk : {1, 4, 16, 32, 64, 128}) {
            float* Bk = (float*)mkl_malloc((size_t)csr.N * kk * sizeof(float), 64);
            float* Ck = (float*)mkl_malloc((size_t)csr.M * kk * sizeof(float), 64);
            for(int64_t i=0;i<(int64_t)csr.N*kk;i++) Bk[i]=0.01f;

            mkl_sparse_set_mm_hint(A, SPARSE_OPERATION_NON_TRANSPOSE,
                descr, SPARSE_LAYOUT_ROW_MAJOR, kk, 50);
            mkl_sparse_optimize(A);

            // warmup
            for(int t=0;t<10;t++) {
                if(kk==1)
                    mkl_sparse_s_mv(SPARSE_OPERATION_NON_TRANSPOSE, 1.0f, A, descr, Bk, 0.0f, Ck);
                else
                    mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE, 1.0f, A,
                        descr, SPARSE_LAYOUT_ROW_MAJOR, Bk, kk, kk, 0.0f, Ck, kk);
            }

            double best=1e9;
            for(int t=0;t<20;t++){
                auto T0=std::chrono::high_resolution_clock::now();
                if(kk==1)
                    mkl_sparse_s_mv(SPARSE_OPERATION_NON_TRANSPOSE, 1.0f, A, descr, Bk, 0.0f, Ck);
                else
                    mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE, 1.0f, A,
                        descr, SPARSE_LAYOUT_ROW_MAJOR, Bk, kk, kk, 0.0f, Ck, kk);
                auto T1=std::chrono::high_resolution_clock::now();
                double ms=std::chrono::duration<double,std::milli>(T1-T0).count();
                if(ms<best)best=ms;
            }
            double gflops = 2.0*csr.nnz*kk/best/1e6;
            printf("  K=%-4d  %.2f ms  (%.2f GFLOP/s)  %s\n",
                kk, best, gflops, kk==1?"(SpMV)":"(SpMM)");

            mkl_free(Bk); mkl_free(Ck);
        }
        mkl_sparse_destroy(A);
    }

    mkl_free(B); mkl_free(C);
    printf("\n=== Done ===\n");
    return 0;
}
