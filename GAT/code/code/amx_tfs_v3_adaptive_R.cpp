/*
 * amx_tfs_v3_adaptive_R.cpp -- TFS V3 + Theorem S3 Adaptive R Selector
 *
 * Changes vs amx_tfs_v3.cpp:
 *   - Add select_R(indptr, N): classify degree ratio rho = d_max / d_avg into
 *     4 regimes per TPDS §5 Theorem S3, pick R accordingly
 *   - `R = 0` in CLI means "auto" (default for this binary); otherwise override
 *
 * Regime table (T=32, per Theorem S3 verdict 2026-04-17):
 *   rho < 32         UNIFORM     -> R = 128  (minimize scheduling overhead)
 *   32 <= rho < 1024 MODERATE    -> R = 64   (V3 default)
 *   1024 <= rho < 20000 HEAVY    -> R = 32   (balance imbalance and overhead)
 *   rho >= 20000     EXTREME     -> R = 16   (minimize imbalance)
 *
 * Expected wins (from fullsweep 4843954 R-sweep data):
 *   wiki-Talk (rho=47694): R=16 38.59ms vs R=64 54.55ms   -> 29% speedup
 *   as-Skitter (rho=2710):  R=16 48.62ms vs R=64 54.66ms   -> 11% speedup
 *   kron_g500 (rho=2464):   R=16 264.99ms vs R=64 372.86ms -> 29% speedup
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

#define K_IN   128
#define K_OUT  128
#define KB     4
#define NB     8
#define TR     16
#define NP     2

#define TC0  0
#define TC1  1
#define TC2  2
#define TC3  3
#define TA   4
#define TB0  5
#define TB1  6

struct __attribute__((aligned(64))) tilecfg_t {
    uint8_t  palette;
    uint8_t  start_row;
    uint8_t  reserved[14];
    uint16_t colsb[16];
    uint8_t  rows[16];
};

static void setup_tilecfg(tilecfg_t *cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    cfg->palette = 1;
    for (int t = 0; t < 7; t++) {
        cfg->rows[t]  = TR;
        cfg->colsb[t] = 64;
    }
}

static inline uint16_t f32_to_bf16(float f)
{
    uint32_t u;
    memcpy(&u, &f, 4);
    return (uint16_t)(u >> 16);
}

static inline float bf16_to_f32(uint16_t b)
{
    uint32_t u = (uint32_t)b << 16;
    float f;
    memcpy(&f, &u, 4);
    return f;
}

static void make_W_vnni(const float *W, uint16_t *Wv)
{
    for (int kb = 0; kb < KB; kb++) {
        for (int ob = 0; ob < NB; ob++) {
            for (int kp = 0; kp < 16; kp++) {
                for (int n = 0; n < 16; n++) {
                    int k0  = kb * 32 + kp * 2;
                    int k1  = k0 + 1;
                    int col = ob * 16 + n;
                    uint16_t v0 = f32_to_bf16(W[k0 * K_OUT + col]);
                    uint16_t v1 = f32_to_bf16(W[k1 * K_OUT + col]);
                    int base = ((kb * NB + ob) * 16 + kp) * 32 + n * 2;
                    memcpy(&Wv[base + 0], &v0, 2);
                    memcpy(&Wv[base + 1], &v1, 2);
                }
            }
        }
    }
}

static void convert_H_bf16(const float *H, uint16_t *Hb, int N)
{
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++) {
        for (int k = 0; k < K_IN; k++) {
            Hb[(size_t)i * K_IN + k] = f32_to_bf16(H[(size_t)i * K_IN + k]);
        }
    }
}

/* ================================================================
 * Theorem S3 Adaptive R Selector (TPDS §5.6)
 * Classify degree ratio rho = d_max / d_avg, pick R per regime.
 * ================================================================ */
static int select_R(const uint32_t *indptr, int N, const char **regime_out)
{
    uint32_t max_d = 0;
    double sum = 0;
    for (int i = 0; i < N; i++) {
        uint32_t d = indptr[i + 1] - indptr[i];
        if (d > max_d) max_d = d;
        sum += d;
    }
    double avg_d = sum / (double)N;
    double rho = (avg_d > 0) ? ((double)max_d / avg_d) : 1.0;

    int R;
    const char *regime;
    if (rho < 32.0)           { R = 128; regime = "UNIFORM";   }
    else if (rho < 1024.0)    { R = 64;  regime = "MODERATE";  }
    else if (rho < 20000.0)   { R = 32;  regime = "HEAVY";     }
    else                      { R = 16;  regime = "EXTREME";   }

    printf("  [AdaptiveR] rho=%.1f (max=%u, avg=%.1f) -> regime=%s, R=%d\n",
           rho, max_d, avg_d, regime, R);
    if (regime_out) *regime_out = regime;
    return R;
}

/* ================================================================
 * Degree-sorted row permutation (ascending)
 * perm[sorted_idx] = original_row
 * ================================================================ */
static int* make_degree_perm(const uint32_t *indptr, int N)
{
    int *perm = (int*)malloc((size_t)N * sizeof(int));
    if (!perm) { fprintf(stderr, "OOM perm\n"); exit(1); }
    for (int i = 0; i < N; i++) perm[i] = i;

    const uint32_t *ip = indptr;
    std::sort(perm, perm + N, [ip](int a, int b) {
        return (ip[a + 1] - ip[a]) < (ip[b + 1] - ip[b]);
    });

    return perm;
}

static void print_deg_stats(const uint32_t *indptr, int N)
{
    uint32_t min_d = ~0u, max_d = 0;
    double sum = 0;
    for (int i = 0; i < N; i++) {
        uint32_t d = indptr[i + 1] - indptr[i];
        if (d < min_d) min_d = d;
        if (d > max_d) max_d = d;
        sum += d;
    }
    printf("  deg: min=%u  max=%u  avg=%.1f  ratio=%.0f\n",
           min_d, max_d, sum / N, (double)max_d / (sum / N));
}

/* ================================================================
 * TFS V3 Kernel
 * ================================================================ */
static void tfs_v3(
    const uint32_t *indptr,
    const uint32_t *indices,
    const uint16_t *Hb,
    const uint16_t *Wv,
    float          *C,
    const int      *perm,
    int N, int R)
{
    memset(C, 0, (size_t)N * K_OUT * sizeof(float));

    #pragma omp parallel
    {
        if (syscall(SYS_arch_prctl, 0x1023, 18) != 0) {
            perror("arch_prctl"); exit(1);
        }
        tilecfg_t cfg;
        setup_tilecfg(&cfg);
        _tile_loadconfig(&cfg);

        uint16_t Hbuf[TR * K_IN]  __attribute__((aligned(64)));
        float    Ctmp[TR * 16]    __attribute__((aligned(64)));
        uint32_t base_local[TR];
        uint32_t deg_local[TR];
        int      orig_row[TR];

        #pragma omp for schedule(dynamic, 1) nowait
        for (int rg = 0; rg < N; rg += R) {
            int rg_end = (rg + R < N) ? (rg + R) : N;

            for (int i = rg; i < rg_end; i += TR) {
                int batch = ((i + TR) <= rg_end) ? TR : (rg_end - i);

                int max_deg = 0;
                for (int n = 0; n < batch; n++) {
                    int row = perm[i + n];
                    orig_row[n]   = row;
                    base_local[n] = indptr[row];
                    deg_local[n]  = indptr[row + 1] - indptr[row];
                    if ((int)deg_local[n] > max_deg)
                        max_deg = (int)deg_local[n];
                }

                for (int obp = 0; obp < NP; obp++) {
                    _tile_zero(TC0);
                    _tile_zero(TC1);
                    _tile_zero(TC2);
                    _tile_zero(TC3);

                    memset(Hbuf, 0, TR * K_IN * sizeof(uint16_t));
                    int active_from = 0;

                    for (int s = 0; s < max_deg; s++) {

                        /* Prefetch next step's H data into L1 */
                        if (s + 1 < max_deg) {
                            for (int n = active_from; n < batch; n++) {
                                if ((uint32_t)(s + 1) < deg_local[n]) {
                                    uint32_t j_next = indices[base_local[n] + s + 1];
                                    const char *addr =
                                        (const char*)&Hb[(size_t)j_next * K_IN];
                                    _mm_prefetch(addr,       _MM_HINT_T0);
                                    _mm_prefetch(addr + 64,  _MM_HINT_T0);
                                    _mm_prefetch(addr + 128, _MM_HINT_T0);
                                    _mm_prefetch(addr + 192, _MM_HINT_T0);
                                }
                            }
                        }

                        /* Zero rows that just became inactive */
                        while (active_from < batch &&
                               (uint32_t)s >= deg_local[active_from]) {
                            memset(&Hbuf[active_from * K_IN], 0,
                                   K_IN * sizeof(uint16_t));
                            active_from++;
                        }

                        /* Early termination */
                        if (active_from >= batch) break;

                        /* Gather active rows only */
                        for (int n = active_from; n < batch; n++) {
                            uint32_t j = indices[base_local[n] + s];
                            memcpy(&Hbuf[n * K_IN],
                                   &Hb[(size_t)j * K_IN],
                                   K_IN * sizeof(uint16_t));
                        }

                        /* KB-inner loop */
                        for (int kb = 0; kb < KB; kb++) {
                            _tile_loadd(TA,
                                        (const uint8_t*)Hbuf + kb * 64,
                                        K_IN * 2);

                            int ob0 = obp * 4;
                            const uint16_t *Wb = Wv;

                            #define WV_OFF(kb_, ob_) \
                                (((kb_) * NB + (ob_)) * 16 * 32)

                            _tile_loadd(TB0, &Wb[WV_OFF(kb, ob0 + 0)], 64);
                            _tile_loadd(TB1, &Wb[WV_OFF(kb, ob0 + 1)], 64);
                            _tile_dpbf16ps(TC0, TA, TB0);
                            _tile_dpbf16ps(TC1, TA, TB1);

                            _tile_loadd(TB0, &Wb[WV_OFF(kb, ob0 + 2)], 64);
                            _tile_loadd(TB1, &Wb[WV_OFF(kb, ob0 + 3)], 64);
                            _tile_dpbf16ps(TC2, TA, TB0);
                            _tile_dpbf16ps(TC3, TA, TB1);

                            #undef WV_OFF
                        }
                    }

                    /* Store C tiles to ORIGINAL row positions */
                    int col = obp * 64;

                    _tile_stored(TC0, Ctmp, 64);
                    for (int n = 0; n < batch; n++)
                        memcpy(&C[(size_t)orig_row[n] * K_OUT + col + 0],
                               &Ctmp[n * 16], 64);

                    _tile_stored(TC1, Ctmp, 64);
                    for (int n = 0; n < batch; n++)
                        memcpy(&C[(size_t)orig_row[n] * K_OUT + col + 16],
                               &Ctmp[n * 16], 64);

                    _tile_stored(TC2, Ctmp, 64);
                    for (int n = 0; n < batch; n++)
                        memcpy(&C[(size_t)orig_row[n] * K_OUT + col + 32],
                               &Ctmp[n * 16], 64);

                    _tile_stored(TC3, Ctmp, 64);
                    for (int n = 0; n < batch; n++)
                        memcpy(&C[(size_t)orig_row[n] * K_OUT + col + 48],
                               &Ctmp[n * 16], 64);
                }
            }
        }
        _tile_release();
    }
}

/* ================================================================
 * Reference two-step (scalar)
 * ================================================================ */
static void ref_twostep(
    const uint32_t *indptr, const uint32_t *indices,
    const uint16_t *Hb, const float *W,
    float *C_ref, int N)
{
    float *Z = (float*)calloc((size_t)N * K_IN, sizeof(float));
    if (!Z) { fprintf(stderr, "OOM Z\n"); exit(1); }

    for (int i = 0; i < N; i++) {
        for (uint32_t p = indptr[i]; p < indptr[i + 1]; p++) {
            uint32_t j = indices[p];
            for (int k = 0; k < K_IN; k++)
                Z[(size_t)i * K_IN + k] +=
                    bf16_to_f32(Hb[(size_t)j * K_IN + k]);
        }
    }
    for (int i = 0; i < N; i++) {
        for (int n = 0; n < K_OUT; n++) {
            float sum = 0.0f;
            for (int k = 0; k < K_IN; k++)
                sum += Z[(size_t)i * K_IN + k] * W[k * K_OUT + n];
            C_ref[(size_t)i * K_OUT + n] = sum;
        }
    }
    free(Z);
}

/* ================================================================
 * CSR loading -- .csrbin (36-byte header)
 * ================================================================ */
struct csr_t {
    uint32_t *indptr;
    uint32_t *indices;
    int N;
    uint32_t nnz;
};

static csr_t load_csrbin(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "Cannot open %s\n", path); exit(1); }

    uint8_t hdr[36];
    if (fread(hdr, 1, 36, f) != 36) {
        fprintf(stderr, "Failed to read header from %s\n", path); exit(1);
    }

    uint32_t type;
    uint64_t nrows64, ncols64, nnz64;
    memcpy(&type,    &hdr[8],  4);
    memcpy(&nrows64, &hdr[12], 8);
    memcpy(&ncols64, &hdr[20], 8);
    memcpy(&nnz64,   &hdr[28], 8);

    int N = (int)nrows64;
    uint32_t nnz = (uint32_t)nnz64;

    printf("  csrbin: type=%u  N=%d  ncols=%llu  NNZ=%u\n",
           type, N, (unsigned long long)ncols64, nnz);

    uint32_t *indptr = (uint32_t*)malloc((size_t)(N + 1) * sizeof(uint32_t));
    if (!indptr) { fprintf(stderr, "OOM indptr\n"); exit(1); }
    if (fread(indptr, sizeof(uint32_t), N + 1, f) != (size_t)(N + 1)) {
        fprintf(stderr, "Failed to read indptr\n"); exit(1);
    }

    uint32_t *indices = (uint32_t*)malloc((size_t)nnz * sizeof(uint32_t));
    if (!indices) { fprintf(stderr, "OOM indices\n"); exit(1); }
    if (fread(indices, sizeof(uint32_t), nnz, f) != (size_t)nnz) {
        fprintf(stderr, "Failed to read indices\n"); exit(1);
    }

    fclose(f);

    if (indptr[N] != nnz) {
        fprintf(stderr, "WARNING: indptr[N]=%u != header nnz=%u\n",
                indptr[N], nnz);
    }

    csr_t csr;
    csr.indptr  = indptr;
    csr.indices = indices;
    csr.N       = N;
    csr.nnz     = nnz;
    return csr;
}

/* ================================================================ */
static void selftest()
{
    printf("=== Self-test (N=256, avg_deg=8) ===\n");
    const int N = 256, avg_deg = 8;

    srand(42);
    uint32_t *indptr = (uint32_t*)malloc((N + 1) * sizeof(uint32_t));
    indptr[0] = 0;
    for (int i = 0; i < N; i++) {
        int deg = avg_deg / 2 + rand() % (avg_deg + 1);
        indptr[i + 1] = indptr[i] + deg;
    }
    uint32_t nnz = indptr[N];
    uint32_t *indices = (uint32_t*)malloc(nnz * sizeof(uint32_t));
    for (uint32_t p = 0; p < nnz; p++) indices[p] = rand() % N;

    float *H = (float*)malloc((size_t)N * K_IN * sizeof(float));
    float *W = (float*)malloc((size_t)K_IN * K_OUT * sizeof(float));
    for (int i = 0; i < N * K_IN; i++) H[i] = 0.01f * ((rand() % 200) - 100);
    for (int i = 0; i < K_IN * K_OUT; i++) W[i] = 0.01f * ((rand() % 200) - 100);

    uint16_t *Hb = (uint16_t*)aligned_alloc(64, (size_t)N * K_IN * 2);
    uint16_t *Wv = (uint16_t*)aligned_alloc(64, (size_t)KB * NB * 16 * 32 * 2);
    convert_H_bf16(H, Hb, N);
    make_W_vnni(W, Wv);

    int *perm = make_degree_perm(indptr, N);

    float *C_tfs = (float*)calloc((size_t)N * K_OUT, sizeof(float));
    tfs_v3(indptr, indices, Hb, Wv, C_tfs, perm, N, 64);

    float *C_ref = (float*)calloc((size_t)N * K_OUT, sizeof(float));
    ref_twostep(indptr, indices, Hb, W, C_ref, N);

    double max_d = 0, max_r = 0;
    for (size_t i = 0; i < (size_t)N * K_OUT; i++) {
        double d = fabs((double)C_tfs[i] - (double)C_ref[i]);
        double r = fabs((double)C_ref[i]);
        if (d > max_d) max_d = d;
        if (r > max_r) max_r = r;
    }
    double rel = (max_r > 0) ? max_d / max_r : 0;
    printf("  max|diff|=%.6e  max|ref|=%.6e  rel_err=%.6f\n", max_d, max_r, rel);
    printf("  %s\n", (rel < 0.02) ? "PASS" : "FAIL");
    printf("  TFS[0:8] =");
    for (int i = 0; i < 8; i++) printf(" %.4f", C_tfs[i]);
    printf("\n  Ref[0:8] =");
    for (int i = 0; i < 8; i++) printf(" %.4f", C_ref[i]);
    printf("\n");

    free(indptr); free(indices); free(H); free(W);
    free(Hb); free(Wv); free(C_tfs); free(C_ref); free(perm);
}

/* ================================================================ */
static void benchmark(const char *dir, const char *name, int R)
{
    char path[512];
    snprintf(path, sizeof(path), "%s/%s/%s.csrbin", dir, name, name);
    printf("Loading: %s\n", path);
    csr_t csr = load_csrbin(path);
    int N = csr.N;
    uint32_t nnz = csr.nnz;
    uint32_t *indptr  = csr.indptr;
    uint32_t *indices = csr.indices;

    double avg_deg = (double)nnz / N;
    const char *regime = "user-specified";
    if (R <= 0) {
        R = select_R(indptr, N, &regime);
    }
    printf("Matrix: %s  N=%d  NNZ=%u  avg_deg=%.1f  R=%d (%s)\n",
           name, N, nnz, avg_deg, R, regime);
    print_deg_stats(indptr, N);

    double t_sort0 = omp_get_wtime();
    int *perm = make_degree_perm(indptr, N);
    double t_sort1 = omp_get_wtime();
    printf("  sort time: %.2f ms\n", (t_sort1 - t_sort0) * 1000.0);

    srand(12345);
    float *H = (float*)malloc((size_t)N * K_IN * sizeof(float));
    float *W = (float*)malloc((size_t)K_IN * K_OUT * sizeof(float));
    for (size_t i = 0; i < (size_t)N * K_IN; i++)
        H[i] = 0.01f * ((rand() % 200) - 100);
    for (int i = 0; i < K_IN * K_OUT; i++)
        W[i] = 0.01f * ((rand() % 200) - 100);

    uint16_t *Hb = (uint16_t*)aligned_alloc(64, (size_t)N * K_IN * 2);
    uint16_t *Wv = (uint16_t*)aligned_alloc(64, (size_t)KB * NB * 16 * 32 * 2);
    convert_H_bf16(H, Hb, N);
    make_W_vnni(W, Wv);
    float *C = (float*)calloc((size_t)N * K_OUT, sizeof(float));

    tfs_v3(indptr, indices, Hb, Wv, C, perm, N, R);

    double best = 1e30;
    for (int run = 0; run < 5; run++) {
        double t0 = omp_get_wtime();
        tfs_v3(indptr, indices, Hb, Wv, C, perm, N, R);
        double t1 = omp_get_wtime();
        double ms = (t1 - t0) * 1000.0;
        printf("  run %d: %.2f ms\n", run, ms);
        if (ms < best) best = ms;
    }
    printf("  BEST: %.2f ms  (R=%d)\n\n", best, R);

    int ck = (N < 512) ? N : 512;
    float *C_ref = (float*)calloc((size_t)ck * K_OUT, sizeof(float));
    ref_twostep(indptr, indices, Hb, W, C_ref, ck);
    double md = 0, mr = 0;
    for (size_t i = 0; i < (size_t)ck * K_OUT; i++) {
        double d = fabs((double)C[i] - (double)C_ref[i]);
        double r = fabs((double)C_ref[i]);
        if (d > md) md = d;
        if (r > mr) mr = r;
    }
    double rel = (mr > 0) ? md / mr : 0;
    printf("  Correctness (first %d rows): rel_err=%.6f %s\n",
           ck, rel, (rel < 0.02) ? "PASS" : "FAIL");

    free(indptr); free(indices); free(H); free(W);
    free(Hb); free(Wv); free(C); free(C_ref); free(perm);
}

/* ================================================================ */
int main(int argc, char **argv)
{
    printf("TFS Fusion V3 + Adaptive R (Theorem S3)\n");
    printf("Threads: %d\n\n", omp_get_max_threads());

    if (argc >= 2 && strcmp(argv[1], "--selftest") == 0) {
        selftest();
        return 0;
    }
    if (argc < 3) {
        fprintf(stderr,
            "Usage:\n"
            "  %s --selftest\n"
            "  %s <data_dir> <matrix_name>         # R = auto (Theorem S3)\n"
            "  %s <data_dir> <matrix_name> <R>     # R = user override (>0)\n"
            "  %s <data_dir> <matrix_name> sweep   # test R=16,64,256\n"
            "\nExpects: <dir>/<n>/<n>.csrbin\n",
            argv[0], argv[0], argv[0], argv[0]);
        return 1;
    }

    const char *dir  = argv[1];
    const char *name = argv[2];
    int R = 0;  // default = auto
    bool sweep = false;
    if (argc >= 4) {
        if (strcmp(argv[3], "sweep") == 0) sweep = true;
        else R = atoi(argv[3]);
    }

    if (sweep) {
        int Rs[] = {16, 64, 256};
        for (int ri = 0; ri < 3; ri++)
            benchmark(dir, name, Rs[ri]);
    } else {
        benchmark(dir, name, R);  // R=0 triggers select_R inside benchmark()
    }

    return 0;
}
