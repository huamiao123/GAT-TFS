/**
 * Step 4.3b: AMX SpMM V3 — OpenMP Parallel
 *
 * V2 基础上加多线程:
 *   - Panel 间完全独立, 按 panel 并行
 *   - 每线程: 自己的 col_map + A_tiles + AMX tile config
 *   - Fallback 也并行
 *   - AMX 权限: 每线程需单独请求 (per-thread XSTATE)
 *
 * Usage: ./amx_spmm_v3 <matrix.csrbin> [nthreads]
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <immintrin.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <omp.h>

constexpr int TILE_R = 16;
constexpr int TILE_C = 32;
constexpr int K      = 32;
constexpr int KC     = 16;
constexpr float FILL_THR = 0.0625f;
constexpr int MAX_NTILES = 64;

typedef struct __attribute__((aligned(64))) {
    uint8_t  palette_id;
    uint8_t  start_row;
    uint8_t  reserved0[14];
    uint16_t colsb[16];
    uint8_t  rows[16];
} tile_config_t;
static_assert(sizeof(tile_config_t) == 64, "must be 64 bytes");

static inline uint16_t f32_to_bf16(float f) {
    uint32_t u; memcpy(&u, &f, 4);
    u += 0x7FFF + ((u >> 16) & 1);
    return (uint16_t)(u >> 16);
}

// ==================== CSR / CSC ====================
struct CSR {
    std::vector<uint32_t> indptr, indices;
    std::vector<float> values;
    int M, N; int64_t nnz;
};

CSR read_csrbin(const char* path) {
    CSR c;
    FILE* f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "ERROR: cannot open %s\n", path); exit(1); }
    uint32_t hdr[3]; uint64_t dims[3];
    fread(hdr, 4, 3, f); fread(dims, 8, 3, f);
    c.M = (int)dims[0]; c.N = (int)dims[1]; c.nnz = (int64_t)dims[2];
    c.indptr.resize(c.M + 1);
    c.indices.resize(c.nnz);
    c.values.resize(c.nnz);
    fread(c.indptr.data(), 4, c.M + 1, f);
    fread(c.indices.data(), 4, c.nnz, f);
    fread(c.values.data(), 4, c.nnz, f);
    fclose(f);
    return c;
}

struct CSC {
    std::vector<int64_t> col_ptr;
    std::vector<int> col_rows, col_counts;
};

CSC build_csc(const CSR& csr) {
    CSC sc;
    int N = csr.N; int64_t nnz = csr.nnz;
    sc.col_counts.assign(N, 0);
    for (int64_t i = 0; i < nnz; i++) sc.col_counts[csr.indices[i]]++;
    sc.col_ptr.assign(N + 1, 0);
    for (int j = 0; j < N; j++) sc.col_ptr[j+1] = sc.col_ptr[j] + sc.col_counts[j];
    sc.col_rows.resize(nnz);
    std::vector<int64_t> pos(N, 0);
    for (int i = 0; i < csr.M; i++)
        for (uint32_t p = csr.indptr[i]; p < csr.indptr[i+1]; p++) {
            int c = csr.indices[p];
            sc.col_rows[sc.col_ptr[c] + pos[c]++] = i;
        }
    return sc;
}

// ==================== Panel ====================
struct Panel {
    int rows[TILE_R];
    std::vector<int> col_union;
    int U, total_deg;
    float fill;
};

void column_anchor_group(const CSR& csr, const CSC& csc,
    std::vector<Panel>& amx_panels, std::vector<int>& avx_rows,
    int max_panels)
{
    int M = csr.M, N = csr.N;
    std::vector<int> deg(M);
    for (int i = 0; i < M; i++) deg[i] = csr.indptr[i+1] - csr.indptr[i];

    std::vector<int> col_order;
    for (int j = 0; j < N; j++)
        if (csc.col_counts[j] >= TILE_R) col_order.push_back(j);
    std::sort(col_order.begin(), col_order.end(),
        [&](int a, int b){ return csc.col_counts[a] > csc.col_counts[b]; });

    std::vector<bool> assigned(M, false);

    for (int col : col_order) {
        if (max_panels > 0 && (int)amx_panels.size() >= max_panels) break;
        std::vector<int> avail;
        for (int64_t p = csc.col_ptr[col]; p < csc.col_ptr[col+1]; p++) {
            int r = csc.col_rows[p];
            if (!assigned[r]) avail.push_back(r);
        }
        if ((int)avail.size() < TILE_R) continue;

        std::partial_sort(avail.begin(), avail.begin() + TILE_R, avail.end(),
            [&](int a, int b){ return deg[a] < deg[b]; });

        Panel panel;
        panel.total_deg = 0;
        std::vector<int> cols;
        for (int i = 0; i < TILE_R; i++) {
            panel.rows[i] = avail[i];
            panel.total_deg += deg[avail[i]];
            int r = avail[i];
            for (uint32_t p = csr.indptr[r]; p < csr.indptr[r+1]; p++)
                cols.push_back(csr.indices[p]);
        }
        std::sort(cols.begin(), cols.end());
        cols.erase(std::unique(cols.begin(), cols.end()), cols.end());
        panel.col_union = std::move(cols);
        panel.U = (int)panel.col_union.size();
        int ntiles = (panel.U + TILE_C - 1) / TILE_C;
        panel.fill = (float)panel.total_deg / (TILE_R * TILE_C * ntiles);

        if (panel.fill >= FILL_THR && ntiles <= MAX_NTILES) {
            for (int i = 0; i < TILE_R; i++) assigned[panel.rows[i]] = true;
            amx_panels.push_back(std::move(panel));
        }
    }
    for (int i = 0; i < M; i++)
        if (!assigned[i]) avx_rows.push_back(i);
}

// ==================== Naive reference ====================
void naive_spmm(const CSR& csr, const float* B, float* C) {
    memset(C, 0, (size_t)csr.M * K * sizeof(float));
    for (int i = 0; i < csr.M; i++)
        for (uint32_t p = csr.indptr[i]; p < csr.indptr[i+1]; p++) {
            float v = csr.values[p];
            const float* Br = B + (int64_t)csr.indices[p] * K;
            float* Cr = C + (int64_t)i * K;
            for (int k = 0; k < K; k++) Cr[k] += v * Br[k];
        }
}

// ==================== AVX-512 fallback (向量化) ====================
void avx512_fallback(const CSR& csr, const float* B, float* C,
                     const std::vector<int>& rows) {
    #pragma omp parallel for schedule(dynamic, 64)
    for (int idx = 0; idx < (int)rows.size(); idx++) {
        int i = rows[idx];
        float* Cr = C + (int64_t)i * K;
        // K=32 → 两个 zmm 寄存器
        __m512 c0 = _mm512_setzero_ps();
        __m512 c1 = _mm512_setzero_ps();
        for (uint32_t p = csr.indptr[i]; p < csr.indptr[i+1]; p++) {
            __m512 a = _mm512_set1_ps(csr.values[p]);
            const float* Br = B + (int64_t)csr.indices[p] * K;
            c0 = _mm512_fmadd_ps(a, _mm512_loadu_ps(Br),      c0);
            c1 = _mm512_fmadd_ps(a, _mm512_loadu_ps(Br + 16),  c1);
        }
        _mm512_storeu_ps(Cr,      c0);
        _mm512_storeu_ps(Cr + 16, c1);
    }
}

// ==================== AMX SpMM V3 — Parallel ====================
void amx_spmm_v3(const CSR& csr, const float* B, float* C,
                  const std::vector<Panel>& panels, int N)
{
    int np = (int)panels.size();

    #pragma omp parallel
    {
        // --- Per-thread: AMX 权限 + tile config ---
        syscall(SYS_arch_prctl, 0x1023, 18);

        tile_config_t cfg;
        memset(&cfg, 0, sizeof(cfg));
        cfg.palette_id = 1;
        cfg.rows[0] = 16; cfg.colsb[0] = KC * 4;
        cfg.rows[1] = 16; cfg.colsb[1] = 64;
        cfg.rows[2] = 16; cfg.colsb[2] = KC * 4;
        _tile_loadconfig(&cfg);

        // --- Per-thread buffers ---
        int* col_map = (int*)calloc(N, sizeof(int));
        alignas(64) uint16_t A_tiles[MAX_NTILES][TILE_R][TILE_C];
        alignas(64) uint8_t  B_buf[16 * 64];
        alignas(64) float    C_buf[TILE_R][KC];

        #pragma omp for schedule(dynamic, 4)
        for (int pi = 0; pi < np; pi++) {
            const Panel& panel = panels[pi];
            int ntiles = (panel.U + TILE_C - 1) / TILE_C;
            const int* cu = panel.col_union.data();
            int cu_sz = panel.U;

            // col_map setup
            for (int j = 0; j < cu_sz; j++)
                col_map[cu[j]] = j + 1;

            // Pack A (once)
            memset(A_tiles, 0, (size_t)ntiles * sizeof(A_tiles[0]));
            for (int i = 0; i < TILE_R; i++) {
                int row = panel.rows[i];
                for (uint32_t p = csr.indptr[row]; p < csr.indptr[row+1]; p++) {
                    int j = col_map[csr.indices[p]] - 1;
                    if (j >= 0)
                        A_tiles[j / TILE_C][i][j % TILE_C] = f32_to_bf16(csr.values[p]);
                }
            }

            // K-chunks
            for (int kc = 0; kc < K; kc += KC) {
                _tile_zero(0);
                for (int t = 0; t < ntiles; t++) {
                    int col_start = t * TILE_C;
                    // B gather + VNNI
                    for (int p = 0; p < 16; p++) {
                        int je = col_start + p * 2;
                        int jo = col_start + p * 2 + 1;
                        uint32_t* pr = (uint32_t*)(B_buf + p * 64);
                        if (je < cu_sz && jo < cu_sz) {
                            __m512 ve = _mm512_loadu_ps(&B[(int64_t)cu[je]*K+kc]);
                            __m512 vo = _mm512_loadu_ps(&B[(int64_t)cu[jo]*K+kc]);
                            __m512i ie = _mm512_srli_epi32(_mm512_castps_si512(ve), 16);
                            __m512i io = _mm512_srli_epi32(_mm512_castps_si512(vo), 16);
                            _mm512_store_si512((__m512i*)pr,
                                _mm512_or_si512(ie, _mm512_slli_epi32(io, 16)));
                        } else if (je < cu_sz) {
                            __m512 ve = _mm512_loadu_ps(&B[(int64_t)cu[je]*K+kc]);
                            __m512i ie = _mm512_srli_epi32(_mm512_castps_si512(ve), 16);
                            _mm512_store_si512((__m512i*)pr, ie);
                        } else {
                            _mm512_store_si512((__m512i*)pr, _mm512_setzero_si512());
                        }
                    }
                    _tile_loadd(1, A_tiles[t], 64);
                    _tile_loadd(2, B_buf, 64);
                    _tile_dpbf16ps(0, 1, 2);
                }
                _tile_stored(0, C_buf, KC * 4);
                for (int i = 0; i < TILE_R; i++) {
                    float* dst = C + (int64_t)panel.rows[i] * K + kc;
                    for (int k = 0; k < KC; k++) dst[k] = C_buf[i][k];
                }
            }

            // col_map cleanup
            for (int j = 0; j < cu_sz; j++)
                col_map[cu[j]] = 0;
        }

        free(col_map);
        _tile_release();
    }  // end omp parallel
}

// ==================== Main ====================
int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <matrix.csrbin> [nthreads]\n", argv[0]);
        return 1;
    }
    int nthreads = (argc >= 3) ? atoi(argv[2]) : omp_get_num_procs();
    omp_set_num_threads(nthreads);

    printf("=== AMX SpMM V3 (OpenMP Parallel) ===\n");
    printf("K=%d, threads=%d\n\n", K, nthreads);

    syscall(SYS_arch_prctl, 0x1023, 18);

    printf("[1] Read: %s\n", argv[1]);
    auto t0 = std::chrono::high_resolution_clock::now();
    CSR csr = read_csrbin(argv[1]);
    auto t1 = std::chrono::high_resolution_clock::now();
    printf("    M=%d N=%d NNZ=%ld avg=%.1f (%.0fms)\n\n",
        csr.M, csr.N, csr.nnz, (double)csr.nnz/csr.M,
        std::chrono::duration<double,std::milli>(t1-t0).count());

    printf("[2] CSC...\n");
    t0 = std::chrono::high_resolution_clock::now();
    CSC csc = build_csc(csr);
    t1 = std::chrono::high_resolution_clock::now();
    printf("    %.0fms\n\n", std::chrono::duration<double,std::milli>(t1-t0).count());

    printf("[3] Grouping...\n");
    t0 = std::chrono::high_resolution_clock::now();
    std::vector<Panel> amx_panels;
    std::vector<int> avx_rows;
    column_anchor_group(csr, csc, amx_panels, avx_rows, -1);
    t1 = std::chrono::high_resolution_clock::now();

    int64_t amx_nnz = 0;
    for (auto& p : amx_panels) amx_nnz += p.total_deg;
    printf("    AMX: %d panels, %ld NNZ (%.1f%%)\n",
        (int)amx_panels.size(), amx_nnz, 100.0*amx_nnz/csr.nnz);
    printf("    AVX: %d rows, %ld NNZ (%.1f%%)\n",
        (int)avx_rows.size(), csr.nnz-amx_nnz, 100.0*(csr.nnz-amx_nnz)/csr.nnz);

    if (!amx_panels.empty()) {
        std::vector<float> fills;
        for (auto& p : amx_panels) fills.push_back(p.fill);
        std::sort(fills.begin(), fills.end());
        int n = fills.size();
        printf("    Fill: mean=%.1f%% med=%.1f%%\n",
            std::accumulate(fills.begin(),fills.end(),0.0f)/n*100, fills[n/2]*100);
    }
    printf("    %.0fms\n\n", std::chrono::duration<double,std::milli>(t1-t0).count());

    printf("[4] B[%d x %d]\n", csr.N, K);
    float* B = (float*)aligned_alloc(64, (size_t)csr.N * K * sizeof(float));
    srand(12345);
    for (int64_t i = 0; i < (int64_t)csr.N * K; i++)
        B[i] = (rand() % 200 - 100) / 100.0f;

    // --- Naive (单线程作为 baseline) ---
    printf("[5] Naive FP32 (single thread)...\n");
    float* C_ref = (float*)aligned_alloc(64, (size_t)csr.M * K * sizeof(float));
    t0 = std::chrono::high_resolution_clock::now();
    naive_spmm(csr, B, C_ref);
    t1 = std::chrono::high_resolution_clock::now();
    double naive_ms = std::chrono::duration<double,std::milli>(t1-t0).count();
    printf("    %.1f ms\n\n", naive_ms);

    // --- V3 多线程, 多次取最佳 ---
    float* C_amx = (float*)aligned_alloc(64, (size_t)csr.M * K * sizeof(float));

    printf("[6] AMX V3 (%d threads)...\n", nthreads);
    // warmup
    memset(C_amx, 0, (size_t)csr.M * K * sizeof(float));
    amx_spmm_v3(csr, B, C_amx, amx_panels, csr.N);
    avx512_fallback(csr, B, C_amx, avx_rows);

    // 3 次取最佳
    double best_amx = 1e9, best_avx = 1e9;
    for (int trial = 0; trial < 3; trial++) {
        memset(C_amx, 0, (size_t)csr.M * K * sizeof(float));

        t0 = std::chrono::high_resolution_clock::now();
        amx_spmm_v3(csr, B, C_amx, amx_panels, csr.N);
        auto tm = std::chrono::high_resolution_clock::now();
        avx512_fallback(csr, B, C_amx, avx_rows);
        t1 = std::chrono::high_resolution_clock::now();

        double a = std::chrono::duration<double,std::milli>(tm-t0).count();
        double f = std::chrono::duration<double,std::milli>(t1-tm).count();
        printf("    [%d] AMX=%.1f Fallback=%.1f Total=%.1f ms\n", trial, a, f, a+f);
        if (a < best_amx) best_amx = a;
        if (f < best_avx) best_avx = f;
    }
    double best_total = best_amx + best_avx;
    printf("    Best: AMX=%.1f + Fallback=%.1f = %.1f ms\n", best_amx, best_avx, best_total);
    printf("    ★ Speedup vs Naive(1T): %.2fx\n\n", naive_ms / best_total);

    // --- Correctness ---
    printf("[7] Correctness...\n");
    float max_abs = 0;
    int bad = 0, total = csr.M * K;
    for (int64_t i = 0; i < (int64_t)csr.M * K; i++) {
        float ae = fabsf(C_ref[i] - C_amx[i]);
        float re = (fabsf(C_ref[i]) > 1e-6f) ? ae / fabsf(C_ref[i]) : ae;
        if (ae > max_abs) max_abs = ae;
        if (re > 0.05f && fabsf(C_ref[i]) > 0.01f) bad++;
    }
    printf("    Max abs: %.3e  Bad: %d (%.4f%%)\n", max_abs, bad, 100.0*bad/total);
    printf("    %s\n", 100.0*bad/total < 1.0 ? "✅ OK (BF16 precision)" : "❌ FAIL");

    free(B); free(C_ref); free(C_amx);
    printf("\n=== Done ===\n");
    return 0;
}
