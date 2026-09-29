#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <immintrin.h>
#include <sys/time.h>

// Test multiple array sizes to see L4/HBM cache effects
// Small = fits in L3 (75MB), Medium = fits in HBM (64GB), Large = exceeds HBM
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

// Sequential bandwidth (STREAM Copy)
double test_seq_bw(size_t N, int threads) {
    float *A = (float*)aligned_alloc(64, N * sizeof(float));
    float *B = (float*)aligned_alloc(64, N * sizeof(float));
    
    #pragma omp parallel for num_threads(threads)
    for (size_t i = 0; i < N; i++) { A[i] = 1.0f; B[i] = 0.0f; }
    
    // Warmup
    #pragma omp parallel for num_threads(threads)
    for (size_t i = 0; i < N; i++) B[i] = A[i];
    
    double best = 1e9;
    for (int t = 0; t < 5; t++) {
        double t0 = get_time();
        #pragma omp parallel for num_threads(threads)
        for (size_t i = 0; i < N; i++) B[i] = A[i];
        double dt = get_time() - t0;
        if (dt < best) best = dt;
    }
    
    free(A); free(B);
    return 2.0 * N * sizeof(float) / best / 1e9;  // GB/s (read + write)
}

// Random gather bandwidth (simulates SpMM B access)
double test_gather_bw(size_t B_rows, int K, size_t n_accesses, int threads) {
    size_t B_size = B_rows * K;
    float *B = (float*)aligned_alloc(64, B_size * sizeof(float));
    uint32_t *idx = (uint32_t*)malloc(n_accesses * sizeof(uint32_t));
    float *out = (float*)aligned_alloc(64, n_accesses * K * sizeof(float));
    
    // Init
    #pragma omp parallel for num_threads(threads)
    for (size_t i = 0; i < B_size; i++) B[i] = 1.0f;
    
    // Random indices
    srand(42);
    for (size_t i = 0; i < n_accesses; i++) idx[i] = rand() % B_rows;
    
    // Warmup
    #pragma omp parallel for num_threads(threads)
    for (size_t i = 0; i < n_accesses; i++) {
        const float *src = B + (size_t)idx[i] * K;
        float *dst = out + i * K;
        memcpy(dst, src, K * sizeof(float));
    }
    
    double best = 1e9;
    for (int t = 0; t < 5; t++) {
        double t0 = get_time();
        #pragma omp parallel for num_threads(threads)
        for (size_t i = 0; i < n_accesses; i++) {
            const float *src = B + (size_t)idx[i] * K;
            float *dst = out + i * K;
            memcpy(dst, src, K * sizeof(float));
        }
        double dt = get_time() - t0;
        if (dt < best) best = dt;
    }
    
    free(B); free(idx); free(out);
    return (double)n_accesses * K * sizeof(float) / best / 1e9;  // GB/s (read only)
}

int main() {
    int threads = omp_get_num_procs();
    printf("Threads: %d\n\n", threads);
    
    // Part 1: Sequential STREAM bandwidth at different sizes
    printf("=== Sequential Bandwidth (STREAM Copy) ===\n");
    printf("%-20s %12s %12s\n", "Array Size", "Total (MB)", "BW (GB/s)");
    printf("─────────────────────────────────────────────\n");
    
    size_t sizes[] = {
        1UL*1024*1024,      // 4MB (fits L3)
        4UL*1024*1024,      // 16MB (fits L3)
        16UL*1024*1024,     // 64MB (near L3 boundary)
        64UL*1024*1024,     // 256MB (should be in HBM if cache mode)
        256UL*1024*1024,    // 1GB
        1024UL*1024*1024,   // 4GB
        4096UL*1024*1024,   // 16GB (fits HBM)
        16384UL*1024*1024   // 64GB (near HBM capacity)
    };
    int n_sizes = 8;
    
    for (int s = 0; s < n_sizes; s++) {
        size_t N = sizes[s];
        double total_mb = 2.0 * N * sizeof(float) / (1024*1024);
        if (total_mb > 120000) { printf("SKIP %zu (too large)\n", N); continue; }
        double bw = test_seq_bw(N, threads);
        printf("%-20zu %10.0f MB %10.1f GB/s\n", N, total_mb, bw);
        fflush(stdout);
    }
    
    // Part 2: Random gather bandwidth at different B sizes
    printf("\n=== Random Gather Bandwidth (K=128, simulates SpMM) ===\n");
    printf("%-15s %12s %12s %12s\n", "B_rows", "B Size (MB)", "Accesses", "BW (GB/s)");
    printf("─────────────────────────────────────────────────────────\n");
    
    int K = 128;
    // Different B sizes to see HBM cache effect
    size_t row_counts[] = {
        10000,       // ~5MB (fits L3)
        100000,      // ~49MB (near L3)
        500000,      // ~244MB (should be in HBM)
        1000000,     // ~488MB
        5000000,     // ~2.4GB
        10000000     // ~4.9GB
    };
    int n_rows = 6;
    
    for (int r = 0; r < n_rows; r++) {
        size_t B_rows = row_counts[r];
        double b_mb = (double)B_rows * K * sizeof(float) / (1024*1024);
        size_t n_acc = B_rows * 10;  // 10x accesses
        if (n_acc > 100000000) n_acc = 100000000;
        double bw = test_gather_bw(B_rows, K, n_acc, threads);
        printf("%-15zu %10.0f MB %12zu %10.1f GB/s\n", B_rows, b_mb, n_acc, bw);
        fflush(stdout);
    }
    
    // Part 3: BF16 vs FP32 gather comparison
    printf("\n=== BF16 vs FP32 Gather (B_rows=1M, K=128) ===\n");
    {
        size_t B_rows = 1000000;
        size_t n_acc = 10000000;
        
        // FP32
        double bw_fp32 = test_gather_bw(B_rows, 128, n_acc, threads);
        
        // BF16 (half the bytes)
        // Simulate by using K=64 with FP32 (same byte count as K=128 BF16)
        double bw_bf16_equiv = test_gather_bw(B_rows, 64, n_acc, threads);
        
        printf("  FP32 (512 B/row): %.1f GB/s\n", bw_fp32);
        printf("  BF16-equiv (256 B/row): %.1f GB/s\n", bw_bf16_equiv);
        printf("  INT8-equiv (128 B/row): ");
        double bw_int8_equiv = test_gather_bw(B_rows, 32, n_acc, threads);
        printf("%.1f GB/s\n", bw_int8_equiv);
    }
    
    printf("\n=== Done ===\n");
    return 0;
}
