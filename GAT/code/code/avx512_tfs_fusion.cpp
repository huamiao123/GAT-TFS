/*
 * avx512_tfs_fusion.cpp -- TFS Fusion with AVX-512 VDPBF16PS (no AMX)
 *
 * Same fusion idea, same BF16 precision, same VNNI W format.
 * Row-by-row processing: no batching, no sorting needed.
 * 512 VDPBF16PS per neighbor per row.
 */

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <immintrin.h>
#include <omp.h>

#define K_IN   128
#define K_OUT  128
#define KB     4
#define NB     8

static inline uint16_t f32_to_bf16(float f)
{
    uint32_t u; memcpy(&u, &f, 4);
    return (uint16_t)(u >> 16);
}

static inline float bf16_to_f32(uint16_t b)
{
    uint32_t u = (uint32_t)b << 16;
    float f; memcpy(&f, &u, 4);
    return f;
}

static void make_W_vnni(const float *W, uint16_t *Wv)
{
    for (int kb = 0; kb < KB; kb++)
        for (int ob = 0; ob < NB; ob++)
            for (int kp = 0; kp < 16; kp++)
                for (int n = 0; n < 16; n++) {
                    int k0 = kb*32+kp*2, k1 = k0+1, col = ob*16+n;
                    uint16_t v0 = f32_to_bf16(W[k0*K_OUT+col]);
                    uint16_t v1 = f32_to_bf16(W[k1*K_OUT+col]);
                    int base = ((kb*NB+ob)*16+kp)*32+n*2;
                    memcpy(&Wv[base], &v0, 2);
                    memcpy(&Wv[base+1], &v1, 2);
                }
}

static void convert_H_bf16(const float *H, uint16_t *Hb, int N)
{
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++)
        for (int k = 0; k < K_IN; k++)
            Hb[(size_t)i*K_IN+k] = f32_to_bf16(H[(size_t)i*K_IN+k]);
}

/* ================================================================
 * AVX-512 TFS Fusion Kernel
 *
 * Per row, per neighbor: broadcast BF16 pair from H, dpbf16ps with W.
 * 4 kb x 16 kp x 8 ob = 512 VDPBF16PS per neighbor per row.
 * ================================================================ */
static void tfs_avx512(
    const uint32_t *indptr, const uint32_t *indices,
    const uint16_t *Hb, const uint16_t *Wv,
    float *C, int N)
{
    memset(C, 0, (size_t)N * K_OUT * sizeof(float));

    #pragma omp parallel for schedule(dynamic, 64)
    for (int i = 0; i < N; i++) {
        __m512 c0 = _mm512_setzero_ps();
        __m512 c1 = _mm512_setzero_ps();
        __m512 c2 = _mm512_setzero_ps();
        __m512 c3 = _mm512_setzero_ps();
        __m512 c4 = _mm512_setzero_ps();
        __m512 c5 = _mm512_setzero_ps();
        __m512 c6 = _mm512_setzero_ps();
        __m512 c7 = _mm512_setzero_ps();

        uint32_t row_start = indptr[i];
        uint32_t row_end   = indptr[i + 1];

        for (uint32_t p = row_start; p < row_end; p++) {
            uint32_t j = indices[p];

            if (p + 1 < row_end) {
                uint32_t j_next = indices[p + 1];
                const char *addr = (const char*)&Hb[(size_t)j_next * K_IN];
                _mm_prefetch(addr,       _MM_HINT_T0);
                _mm_prefetch(addr + 64,  _MM_HINT_T0);
                _mm_prefetch(addr + 128, _MM_HINT_T0);
                _mm_prefetch(addr + 192, _MM_HINT_T0);
            }

            const uint16_t *h_base = &Hb[(size_t)j * K_IN];

            for (int kb = 0; kb < KB; kb++) {
                const uint16_t *h_ptr = h_base + kb * 32;

                for (int kp = 0; kp < 16; kp++) {
                    uint32_t pair;
                    memcpy(&pair, &h_ptr[kp * 2], 4);
                    __m512i h_bc = _mm512_set1_epi32(pair);

                    int wv_base = (kb * NB * 16 + kp) * 32;

                    #define DO_OB(C_REG, OB_IDX) {                          \
                        __m512i w = _mm512_loadu_si512(                      \
                            &Wv[wv_base + (OB_IDX) * 16 * 32]);            \
                        C_REG = _mm512_dpbf16_ps(C_REG,                     \
                            (__m512bh)h_bc, (__m512bh)w);                   \
                    }

                    DO_OB(c0, 0); DO_OB(c1, 1);
                    DO_OB(c2, 2); DO_OB(c3, 3);
                    DO_OB(c4, 4); DO_OB(c5, 5);
                    DO_OB(c6, 6); DO_OB(c7, 7);

                    #undef DO_OB
                }
            }
        }

        float *out = &C[(size_t)i * K_OUT];
        _mm512_storeu_ps(out +   0, c0);
        _mm512_storeu_ps(out +  16, c1);
        _mm512_storeu_ps(out +  32, c2);
        _mm512_storeu_ps(out +  48, c3);
        _mm512_storeu_ps(out +  64, c4);
        _mm512_storeu_ps(out +  80, c5);
        _mm512_storeu_ps(out +  96, c6);
        _mm512_storeu_ps(out + 112, c7);
    }
}

/* ================================================================
 * Reference
 * ================================================================ */
static void ref_twostep(
    const uint32_t *indptr, const uint32_t *indices,
    const uint16_t *Hb, const float *W,
    float *C_ref, int N)
{
    float *Z = (float*)calloc((size_t)N * K_IN, sizeof(float));
    if (!Z) { fprintf(stderr, "OOM\n"); exit(1); }
    for (int i = 0; i < N; i++)
        for (uint32_t p = indptr[i]; p < indptr[i+1]; p++) {
            uint32_t j = indices[p];
            for (int k = 0; k < K_IN; k++)
                Z[(size_t)i*K_IN+k] += bf16_to_f32(Hb[(size_t)j*K_IN+k]);
        }
    for (int i = 0; i < N; i++)
        for (int n = 0; n < K_OUT; n++) {
            float sum = 0;
            for (int k = 0; k < K_IN; k++)
                sum += Z[(size_t)i*K_IN+k] * W[k*K_OUT+n];
            C_ref[(size_t)i*K_OUT+n] = sum;
        }
    free(Z);
}

/* ================================================================ */
struct csr_t { uint32_t *indptr, *indices; int N; uint32_t nnz; };

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
    fclose(f);
    return {indptr, indices, N, nnz};
}

/* ================================================================ */
static void selftest()
{
    printf("=== Self-test (N=256, avg_deg=8) ===\n");
    const int N = 256;
    srand(42);
    uint32_t *indptr = (uint32_t*)malloc((N+1)*4);
    indptr[0] = 0;
    for (int i = 0; i < N; i++) indptr[i+1] = indptr[i] + 4 + rand()%9;
    uint32_t nnz = indptr[N];
    uint32_t *indices = (uint32_t*)malloc(nnz*4);
    for (uint32_t p = 0; p < nnz; p++) indices[p] = rand()%N;

    float *H = (float*)malloc((size_t)N*K_IN*4);
    float *W = (float*)malloc((size_t)K_IN*K_OUT*4);
    for (int i = 0; i < N*K_IN; i++) H[i] = 0.01f*((rand()%200)-100);
    for (int i = 0; i < K_IN*K_OUT; i++) W[i] = 0.01f*((rand()%200)-100);

    uint16_t *Hb = (uint16_t*)aligned_alloc(64,(size_t)N*K_IN*2);
    uint16_t *Wv = (uint16_t*)aligned_alloc(64,(size_t)KB*NB*16*32*2);
    convert_H_bf16(H, Hb, N);
    make_W_vnni(W, Wv);

    float *C = (float*)calloc((size_t)N*K_OUT, 4);
    tfs_avx512(indptr, indices, Hb, Wv, C, N);

    float *C_ref = (float*)calloc((size_t)N*K_OUT, 4);
    ref_twostep(indptr, indices, Hb, W, C_ref, N);

    double md=0, mr=0;
    for (size_t i = 0; i < (size_t)N*K_OUT; i++) {
        double d = fabs((double)C[i]-(double)C_ref[i]);
        double r = fabs((double)C_ref[i]);
        if (d>md) md=d; if (r>mr) mr=r;
    }
    double rel = (mr>0) ? md/mr : 0;
    printf("  rel_err=%.6f %s\n", rel, (rel<0.02)?"PASS":"FAIL");
    printf("  AVX[0:4] = %.4f %.4f %.4f %.4f\n", C[0],C[1],C[2],C[3]);
    printf("  Ref[0:4] = %.4f %.4f %.4f %.4f\n", C_ref[0],C_ref[1],C_ref[2],C_ref[3]);

    free(indptr);free(indices);free(H);free(W);
    free(Hb);free(Wv);free(C);free(C_ref);
}

/* ================================================================ */
static void benchmark(const char *dir, const char *name)
{
    char path[512];
    snprintf(path,512,"%s/%s/%s.csrbin",dir,name,name);
    printf("Loading: %s\n", path);
    csr_t csr = load_csrbin(path);
    int N=csr.N; uint32_t nnz=csr.nnz;
    printf("Matrix: %s  N=%d  NNZ=%u  avg_deg=%.1f\n",
           name, N, nnz, (double)nnz/N);

    srand(12345);
    float *H = (float*)malloc((size_t)N*K_IN*4);
    float *W = (float*)malloc((size_t)K_IN*K_OUT*4);
    for (size_t i=0;i<(size_t)N*K_IN;i++) H[i]=0.01f*((rand()%200)-100);
    for (int i=0;i<K_IN*K_OUT;i++) W[i]=0.01f*((rand()%200)-100);

    uint16_t *Hb = (uint16_t*)aligned_alloc(64,(size_t)N*K_IN*2);
    uint16_t *Wv = (uint16_t*)aligned_alloc(64,(size_t)KB*NB*16*32*2);
    convert_H_bf16(H, Hb, N);
    make_W_vnni(W, Wv);
    float *C = (float*)calloc((size_t)N*K_OUT, 4);

    tfs_avx512(csr.indptr, csr.indices, Hb, Wv, C, N);

    double best = 1e30;
    for (int run = 0; run < 5; run++) {
        double t0 = omp_get_wtime();
        tfs_avx512(csr.indptr, csr.indices, Hb, Wv, C, N);
        double t1 = omp_get_wtime();
        double ms = (t1-t0)*1000.0;
        printf("  run %d: %.2f ms\n", run, ms);
        if (ms<best) best=ms;
    }
    printf("  BEST: %.2f ms\n\n", best);

    int ck=(N<512)?N:512;
    float *Cr = (float*)calloc((size_t)ck*K_OUT,4);
    ref_twostep(csr.indptr, csr.indices, Hb, W, Cr, ck);
    double md=0,mr=0;
    for(size_t i=0;i<(size_t)ck*K_OUT;i++){
        double d=fabs((double)C[i]-(double)Cr[i]);
        double r=fabs((double)Cr[i]);
        if(d>md)md=d;if(r>mr)mr=r;
    }
    printf("  Correctness: rel_err=%.6f %s\n",
           (mr>0)?md/mr:0, ((mr>0?md/mr:0)<0.02)?"PASS":"FAIL");

    free(csr.indptr);free(csr.indices);free(H);free(W);
    free(Hb);free(Wv);free(C);free(Cr);
}

int main(int argc, char **argv)
{
    printf("TFS Fusion -- AVX-512 VDPBF16PS (no AMX)\n");
    printf("Threads: %d\n\n", omp_get_max_threads());

    if (argc>=2 && strcmp(argv[1],"--selftest")==0) { selftest(); return 0; }
    if (argc<3) {
        fprintf(stderr,"Usage: %s --selftest | %s <dir> <name>\n",argv[0],argv[0]);
        return 1;
    }
    benchmark(argv[1], argv[2]);
    return 0;
}
