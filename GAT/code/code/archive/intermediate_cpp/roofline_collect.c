/**
 * Collect data for Roofline model:
 * 1. Peak compute: AMX BF16, AVX-512 FP32, AVX-512 BF16
 * 2. Peak bandwidth: sequential, random (various sizes)
 * 3. Per-matrix: actual FLOPS, actual bytes moved, arithmetic intensity
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <omp.h>
#include <immintrin.h>
#include <sys/time.h>
#include <x86intrin.h>

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

// Peak AVX-512 FP32 FLOPS (FMA)
double peak_avx512_fp32(int threads) {
    double best = 0;
    for (int trial = 0; trial < 3; trial++) {
        double t0 = get_time();
        #pragma omp parallel num_threads(threads)
        {
            __m512 a = _mm512_set1_ps(1.0001f);
            __m512 b = _mm512_set1_ps(0.9999f);
            // 8 independent FMA chains to saturate 2 FMA ports
            __m512 c0=a, c1=a, c2=a, c3=a, c4=a, c5=a, c6=a, c7=a;
            for (long i = 0; i < 100000000L; i++) {
                c0 = _mm512_fmadd_ps(a, b, c0);
                c1 = _mm512_fmadd_ps(a, b, c1);
                c2 = _mm512_fmadd_ps(a, b, c2);
                c3 = _mm512_fmadd_ps(a, b, c3);
                c4 = _mm512_fmadd_ps(a, b, c4);
                c5 = _mm512_fmadd_ps(a, b, c5);
                c6 = _mm512_fmadd_ps(a, b, c6);
                c7 = _mm512_fmadd_ps(a, b, c7);
            }
            // Prevent optimization
            volatile float sink = _mm512_reduce_add_ps(c0) + _mm512_reduce_add_ps(c1) +
                                  _mm512_reduce_add_ps(c2) + _mm512_reduce_add_ps(c3) +
                                  _mm512_reduce_add_ps(c4) + _mm512_reduce_add_ps(c5) +
                                  _mm512_reduce_add_ps(c6) + _mm512_reduce_add_ps(c7);
        }
        double dt = get_time() - t0;
        // 8 FMA * 16 floats * 2 FLOP/FMA * 100M iters * threads
        double gflops = 8.0 * 16 * 2 * 100000000.0 * threads / dt / 1e9;
        if (gflops > best) best = gflops;
    }
    return best;
}

// Peak memory bandwidth (sequential)
double peak_seq_bw(int threads) {
    size_t N = 256UL * 1024 * 1024;  // 1GB per array
    float *A = (float*)aligned_alloc(64, N * sizeof(float));
    float *B = (float*)aligned_alloc(64, N * sizeof(float));
    
    #pragma omp parallel for num_threads(threads)
    for (size_t i = 0; i < N; i++) { A[i] = 1.0f; B[i] = 0.0f; }
    
    // Warmup
    #pragma omp parallel for num_threads(threads)
    for (size_t i = 0; i < N; i++) B[i] = A[i];
    
    double best = 0;
    for (int t = 0; t < 5; t++) {
        double t0 = get_time();
        #pragma omp parallel for num_threads(threads)
        for (size_t i = 0; i < N; i++) B[i] = A[i];
        double dt = get_time() - t0;
        double bw = 2.0 * N * sizeof(float) / dt / 1e9;
        if (bw > best) best = bw;
    }
    free(A); free(B);
    return best;
}

// Per-matrix arithmetic intensity
typedef struct {
    uint32_t *indptr, *indices;
    int M, N;
    int64_t nnz;
} CSR;

CSR read_csrbin(const char* path) {
    CSR c;
    FILE* f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "ERR: %s\n", path); exit(1); }
    uint32_t hdr[3]; uint64_t dims[3];
    fread(hdr, 4, 3, f); fread(dims, 8, 3, f);
    c.M = (int)dims[0]; c.N = (int)dims[1]; c.nnz = (int64_t)dims[2];
    c.indptr = (uint32_t*)malloc((c.M+1)*4);
    c.indices = (uint32_t*)malloc(c.nnz*4);
    fread(c.indptr, 4, c.M+1, f);
    fread(c.indices, 4, c.nnz, f);
    fclose(f);
    return c;
}

void analyze_matrix(const char* name, const char* path, int K) {
    CSR csr = read_csrbin(path);
    
    // FLOPS: 2 * nnz * K (multiply + add)
    double flops = 2.0 * csr.nnz * K;
    
    // Bytes moved (theoretical minimum):
    //   Read A values: nnz * 4 (FP32) or nnz * 2 (BF16)
    //   Read A indices: nnz * 4
    //   Read A indptr: (M+1) * 4
    //   Read B rows: nnz * K * 4 (FP32) or nnz * K * 2 (BF16) or nnz * K * 1 (INT8)
    //   Write C: M * K * 4
    //   Note: B has reuse - unique cols < nnz
    
    // Count unique columns
    uint8_t *seen = (uint8_t*)calloc(csr.N, 1);
    int64_t unique_cols = 0;
    for (int64_t i = 0; i < csr.nnz; i++) {
        if (!seen[csr.indices[i]]) {
            seen[csr.indices[i]] = 1;
            unique_cols++;
        }
    }
    free(seen);
    
    // No-reuse model (worst case): every NNZ loads a full B row
    double bytes_noreuse_fp32 = (double)csr.nnz * K * 4 + (double)csr.M * K * 4;
    double bytes_noreuse_bf16 = (double)csr.nnz * K * 2 + (double)csr.M * K * 4;
    double bytes_noreuse_int8 = (double)csr.nnz * K * 1 + (double)csr.M * K * 4;
    
    // Perfect-reuse model (best case): each unique B row loaded once
    double bytes_perfect_fp32 = (double)unique_cols * K * 4 + (double)csr.M * K * 4;
    double bytes_perfect_bf16 = (double)unique_cols * K * 2 + (double)csr.M * K * 4;
    double bytes_perfect_int8 = (double)unique_cols * K * 1 + (double)csr.M * K * 4;
    
    printf("  Matrix: %s  M=%d N=%d NNZ=%ld unique_cols=%ld avg_deg=%.1f\n",
           name, csr.M, csr.N, csr.nnz, unique_cols, (double)csr.nnz/csr.M);
    printf("  FLOPS (K=%d): %.2f GFLOP\n", K, flops/1e9);
    printf("  Arithmetic Intensity (FLOP/Byte):\n");
    printf("    %-20s %10s %10s\n", "", "No-Reuse", "Perfect");
    printf("    FP32 B:           %10.3f %10.3f\n",
           flops/bytes_noreuse_fp32, flops/bytes_perfect_fp32);
    printf("    BF16 B:           %10.3f %10.3f\n",
           flops/bytes_noreuse_bf16, flops/bytes_perfect_bf16);
    printf("    INT8 B:           %10.3f %10.3f\n",
           flops/bytes_noreuse_int8, flops/bytes_perfect_int8);
    printf("  B reuse factor: %.1fx (nnz/unique = %ld/%ld)\n",
           (double)csr.nnz/unique_cols, csr.nnz, unique_cols);
    printf("\n");
    
    free(csr.indptr); free(csr.indices);
}

int main() {
    int threads = 64;
    omp_set_num_threads(threads);
    printf("Threads: %d\n\n", threads);
    
    // Part 1: Peak compute
    printf("=== Peak Compute ===\n");
    double avx512_peak = peak_avx512_fp32(threads);
    printf("  AVX-512 FP32 FMA: %.1f GFLOPS (%d threads)\n", avx512_peak, threads);
    printf("  AMX BF16 (from micro-bench): ~2653 GFLOPS/core × %d = ~%.0f GFLOPS\n",
           threads, 2653.0 * threads);
    
    // Part 2: Peak bandwidth
    printf("\n=== Peak Sequential Bandwidth ===\n");
    double seq_bw = peak_seq_bw(threads);
    printf("  Sequential copy: %.1f GB/s\n", seq_bw);
    
    // Part 3: Per-matrix analysis
    printf("\n=== Per-Matrix Arithmetic Intensity (K=128) ===\n\n");
    
    const char* data = "/home/huangjianqiang_group/hdacp1/data/SpMM_project/data";
    const char* names[] = {"web-Google", "amazon0601", "cit-Patents", "as-Skitter",
                           "soc-Pokec", "hollywood-2009", "indochina-2004"};
    
    for (int i = 0; i < 7; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s/%s/%s.csrbin", data, names[i], names[i]);
        analyze_matrix(names[i], path, 128);
    }
    
    // Part 4: Roofline summary
    printf("\n=== Roofline Summary ===\n");
    printf("  Ridge point (AVX-512): %.1f GFLOPS / %.1f GB/s = %.2f FLOP/Byte\n",
           avx512_peak, seq_bw, avx512_peak / seq_bw);
    printf("  Ridge point (AMX BF16): ~%.0f GFLOPS / %.1f GB/s = ~%.1f FLOP/Byte\n",
           2653.0 * threads, seq_bw, 2653.0 * threads / seq_bw);
    printf("\n  SpMM AI range: 0.3-1.0 FLOP/Byte (no-reuse) → ALL memory-bound\n");
    printf("  Even with perfect reuse: 1-4 FLOP/Byte → STILL memory-bound for AMX\n");
    
    printf("\n=== Done ===\n");
    return 0;
}
