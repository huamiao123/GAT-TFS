/**
 * Test software prefetch on AVX-512 SpMM (Fallback path)
 * 
 * Strategy: while processing row i's NNZ, prefetch B rows for row i+AHEAD
 * Test AHEAD = 1, 2, 4, 8, 16
 * Also test prefetch hint: T0 (L1), T1 (L2), T2 (L3), NTA (non-temporal)
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <vector>
#include <chrono>
#include <immintrin.h>
#include <omp.h>

struct CSR {
    std::vector<uint32_t> indptr, indices;
    int M, N; int64_t nnz;
};

CSR read_csrbin(const char* path) {
    CSR c; FILE* f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "ERR: %s\n", path); exit(1); }
    uint32_t hdr[3]; uint64_t dims[3];
    fread(hdr, 4, 3, f); fread(dims, 8, 3, f);
    c.M = (int)dims[0]; c.N = (int)dims[1]; c.nnz = (int64_t)dims[2];
    c.indptr.resize(c.M + 1); c.indices.resize(c.nnz);
    fread(c.indptr.data(), 4, c.M + 1, f);
    fread(c.indices.data(), 4, c.nnz, f);
    fclose(f);
    return c;
}

// Baseline: no prefetch
void spmm_baseline(const CSR& csr, const float* B, float* C, int K) {
    constexpr int NVEC = 128 / 16;  // K=128
    #pragma omp parallel for schedule(dynamic, 256)
    for (int i = 0; i < csr.M; i++) {
        float* Cr = C + (int64_t)i * K;
        __m512 v[NVEC];
        for (int j = 0; j < NVEC; j++) v[j] = _mm512_setzero_ps();
        for (uint32_t p = csr.indptr[i]; p < csr.indptr[i+1]; p++) {
            __m512 a = _mm512_set1_ps(1.0f);
            const float* Br = B + (int64_t)csr.indices[p] * K;
            for (int j = 0; j < NVEC; j++)
                v[j] = _mm512_fmadd_ps(a, _mm512_loadu_ps(Br + j*16), v[j]);
        }
        for (int j = 0; j < NVEC; j++) _mm512_storeu_ps(Cr + j*16, v[j]);
    }
}

// With prefetch: prefetch B rows for future NNZ within same row
template<int HINT>  // 0=T0, 1=T1, 2=T2, 3=NTA
void spmm_prefetch_intra(const CSR& csr, const float* B, float* C, int K, int ahead) {
    constexpr int NVEC = 128 / 16;
    #pragma omp parallel for schedule(dynamic, 256)
    for (int i = 0; i < csr.M; i++) {
        float* Cr = C + (int64_t)i * K;
        __m512 v[NVEC];
        for (int j = 0; j < NVEC; j++) v[j] = _mm512_setzero_ps();
        
        uint32_t start = csr.indptr[i], end = csr.indptr[i+1];
        for (uint32_t p = start; p < end; p++) {
            // Prefetch ahead
            if (p + ahead < end) {
                const float* Br_future = B + (int64_t)csr.indices[p + ahead] * K;
                for (int j = 0; j < NVEC; j++) {
                    if constexpr (HINT == 0)
                        _mm_prefetch((const char*)(Br_future + j*16), _MM_HINT_T0);
                    else if constexpr (HINT == 1)
                        _mm_prefetch((const char*)(Br_future + j*16), _MM_HINT_T1);
                    else if constexpr (HINT == 2)
                        _mm_prefetch((const char*)(Br_future + j*16), _MM_HINT_T2);
                    else
                        _mm_prefetch((const char*)(Br_future + j*16), _MM_HINT_NTA);
                }
            }
            
            __m512 a = _mm512_set1_ps(1.0f);
            const float* Br = B + (int64_t)csr.indices[p] * K;
            for (int j = 0; j < NVEC; j++)
                v[j] = _mm512_fmadd_ps(a, _mm512_loadu_ps(Br + j*16), v[j]);
        }
        for (int j = 0; j < NVEC; j++) _mm512_storeu_ps(Cr + j*16, v[j]);
    }
}

// With inter-row prefetch: prefetch first B row of row i+AHEAD
void spmm_prefetch_inter(const CSR& csr, const float* B, float* C, int K, int ahead) {
    constexpr int NVEC = 128 / 16;
    #pragma omp parallel for schedule(dynamic, 256)
    for (int i = 0; i < csr.M; i++) {
        // Prefetch first B row of future row
        if (i + ahead < csr.M && csr.indptr[i+ahead] < csr.indptr[i+ahead+1]) {
            const float* Br_future = B + (int64_t)csr.indices[csr.indptr[i+ahead]] * K;
            for (int j = 0; j < NVEC; j++)
                _mm_prefetch((const char*)(Br_future + j*16), _MM_HINT_T1);
        }
        
        float* Cr = C + (int64_t)i * K;
        __m512 v[NVEC];
        for (int j = 0; j < NVEC; j++) v[j] = _mm512_setzero_ps();
        
        uint32_t start = csr.indptr[i], end = csr.indptr[i+1];
        for (uint32_t p = start; p < end; p++) {
            // Also intra-row prefetch ahead=4
            if (p + 4 < end) {
                const float* Br_f = B + (int64_t)csr.indices[p+4] * K;
                for (int j = 0; j < NVEC; j++)
                    _mm_prefetch((const char*)(Br_f + j*16), _MM_HINT_T0);
            }
            __m512 a = _mm512_set1_ps(1.0f);
            const float* Br = B + (int64_t)csr.indices[p] * K;
            for (int j = 0; j < NVEC; j++)
                v[j] = _mm512_fmadd_ps(a, _mm512_loadu_ps(Br + j*16), v[j]);
        }
        for (int j = 0; j < NVEC; j++) _mm512_storeu_ps(Cr + j*16, v[j]);
    }
}

double bench(const CSR& csr, const float* B, float* C, int K, 
             void(*fn)(const CSR&, const float*, float*, int), int warmup=2, int iters=10) {
    for (int w = 0; w < warmup; w++) fn(csr, B, C, K);
    double best = 1e9;
    for (int t = 0; t < iters; t++) {
        memset(C, 0, (int64_t)csr.M * K * sizeof(float));
        auto t0 = std::chrono::high_resolution_clock::now();
        fn(csr, B, C, K);
        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1-t0).count();
        if (ms < best) best = ms;
    }
    return best;
}

int main(int argc, char** argv) {
    if (argc < 2) { printf("Usage: %s <mat> [thr]\n", argv[0]); return 1; }
    int nt = (argc >= 3) ? atoi(argv[2]) : omp_get_num_procs();
    omp_set_num_threads(nt);
    
    CSR csr = read_csrbin(argv[1]);
    int K = 128;
    printf("=== Prefetch Test K=%d threads=%d ===\n", K, nt);
    printf("M=%d N=%d NNZ=%ld avg=%.1f\n\n", csr.M, csr.N, csr.nnz, (double)csr.nnz/csr.M);
    
    // Allocate
    float* B = (float*)aligned_alloc(64, (int64_t)csr.N * K * sizeof(float));
    float* C = (float*)aligned_alloc(64, (int64_t)csr.M * K * sizeof(float));
    srand(42);
    for (int64_t i = 0; i < (int64_t)csr.N * K; i++) B[i] = (rand() % 200 - 100) / 100.0f;
    
    // Baseline
    double t_base = bench(csr, B, C, K, spmm_baseline);
    printf("Baseline (no prefetch):  %8.2f ms\n\n", t_base);
    
    // Intra-row prefetch with different ahead values and hints
    printf("--- Intra-row prefetch ---\n");
    printf("%-20s %10s %10s\n", "Config", "Time (ms)", "Speedup");
    
    auto bench_pf = [&](const char* name, auto fn) {
        // Warmup
        for (int w = 0; w < 2; w++) fn(csr, B, C, K);
        double best = 1e9;
        for (int t = 0; t < 10; t++) {
            memset(C, 0, (int64_t)csr.M * K * sizeof(float));
            auto t0 = std::chrono::high_resolution_clock::now();
            fn(csr, B, C, K);
            auto t1 = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t1-t0).count();
            if (ms < best) best = ms;
        }
        printf("%-20s %10.2f %9.2fx\n", name, best, t_base / best);
    };
    
    // T0 hint with different ahead
    bench_pf("T0 ahead=1", [](const CSR& c, const float* b, float* cc, int k){ 
        spmm_prefetch_intra<0>(c,b,cc,k,1); });
    bench_pf("T0 ahead=2", [](const CSR& c, const float* b, float* cc, int k){ 
        spmm_prefetch_intra<0>(c,b,cc,k,2); });
    bench_pf("T0 ahead=4", [](const CSR& c, const float* b, float* cc, int k){ 
        spmm_prefetch_intra<0>(c,b,cc,k,4); });
    bench_pf("T0 ahead=8", [](const CSR& c, const float* b, float* cc, int k){ 
        spmm_prefetch_intra<0>(c,b,cc,k,8); });
    
    // T1 hint
    bench_pf("T1 ahead=4", [](const CSR& c, const float* b, float* cc, int k){ 
        spmm_prefetch_intra<1>(c,b,cc,k,4); });
    
    // NTA hint
    bench_pf("NTA ahead=4", [](const CSR& c, const float* b, float* cc, int k){ 
        spmm_prefetch_intra<3>(c,b,cc,k,4); });
    
    // Inter+Intra combined
    printf("\n--- Inter+Intra row prefetch ---\n");
    bench_pf("Inter+Intra a=4", [](const CSR& c, const float* b, float* cc, int k){ 
        spmm_prefetch_inter(c,b,cc,k,4); });
    bench_pf("Inter+Intra a=8", [](const CSR& c, const float* b, float* cc, int k){ 
        spmm_prefetch_inter(c,b,cc,k,8); });
    bench_pf("Inter+Intra a=16", [](const CSR& c, const float* b, float* cc, int k){ 
        spmm_prefetch_inter(c,b,cc,k,16); });
    
    free(B); free(C);
    printf("\n=== Done ===\n");
    return 0;
}
