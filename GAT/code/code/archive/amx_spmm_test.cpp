/**
 * Step 4.2: AMX SpMM on Real Sparse Matrix
 *
 * Pipeline: read .csrbin → column-anchor grouping → AMX panels + scalar fallback
 * Verify correctness against naive FP32 SpMM
 *
 * Usage: ./amx_spmm_test <matrix.csrbin> [max_panels]
 *   max_panels: limit AMX panels for quick test (default: all)
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <immintrin.h>
#include <sys/syscall.h>
#include <unistd.h>

// ==================== Constants ====================
constexpr int TILE_R = 16;    // AMX tile rows
constexpr int TILE_C = 32;    // AMX A-tile cols (BF16 pairs)
constexpr int K      = 32;    // Dense matrix B width
constexpr int KC     = 16;    // K-chunk size (AMX C tile cols)
constexpr float FILL_THR = 0.0625f;  // 盈亏线 1/16

// ==================== tile_config_t (踩过坑!) ====================
typedef struct __attribute__((aligned(64))) {
    uint8_t  palette_id;
    uint8_t  start_row;
    uint8_t  reserved0[14];
    uint16_t colsb[16];
    uint8_t  rows[16];
} tile_config_t;
static_assert(sizeof(tile_config_t) == 64, "must be 64 bytes");

// ==================== BF16 conversion ====================
static inline uint16_t f32_to_bf16(float f) {
    uint32_t u; memcpy(&u, &f, 4);
    u += 0x7FFF + ((u >> 16) & 1);  // round-to-nearest-even
    return (uint16_t)(u >> 16);
}

// ==================== Data structures ====================
struct Panel {
    int rows[TILE_R];
    std::vector<int> col_union;  // sorted
    int U, total_deg;
    float fill;
};

// ==================== Read .csrbin ====================
// Header: ptype(u32) dtype(u32) vtype(u32) nrow(u64) ncol(u64) nnz(u64) = 36 bytes
// Data: indptr[nrow+1](u32) indices[nnz](u32) values[nnz](f32)
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

// ==================== Build CSC ====================
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

// ==================== Column-Anchor Grouping ====================
void column_anchor_group(const CSR& csr, const CSC& csc,
    std::vector<Panel>& amx_panels, std::vector<int>& avx_rows,
    int max_panels)
{
    int M = csr.M, N = csr.N;
    std::vector<int> deg(M);
    for (int i = 0; i < M; i++) deg[i] = csr.indptr[i+1] - csr.indptr[i];

    // 按列频率降序排列
    std::vector<int> col_order;
    col_order.reserve(N);
    for (int j = 0; j < N; j++)
        if (csc.col_counts[j] >= TILE_R) col_order.push_back(j);
    std::sort(col_order.begin(), col_order.end(),
        [&](int a, int b){ return csc.col_counts[a] > csc.col_counts[b]; });

    std::vector<bool> assigned(M, false);

    for (int col : col_order) {
        if (max_panels > 0 && (int)amx_panels.size() >= max_panels) break;

        // 收集未分配的行
        std::vector<int> avail;
        for (int64_t p = csc.col_ptr[col]; p < csc.col_ptr[col+1]; p++) {
            int r = csc.col_rows[p];
            if (!assigned[r]) avail.push_back(r);
        }
        if ((int)avail.size() < TILE_R) continue;

        // 度数最小的 16 行
        std::partial_sort(avail.begin(), avail.begin() + TILE_R, avail.end(),
            [&](int a, int b){ return deg[a] < deg[b]; });

        // 构建 panel: 计算列并集
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

        if (panel.fill >= FILL_THR) {
            for (int i = 0; i < TILE_R; i++) assigned[panel.rows[i]] = true;
            amx_panels.push_back(std::move(panel));
        }
    }

    for (int i = 0; i < M; i++)
        if (!assigned[i]) avx_rows.push_back(i);
}

// ==================== Naive FP32 SpMM (reference) ====================
void naive_spmm(const CSR& csr, const float* B, float* C) {
    memset(C, 0, (size_t)csr.M * K * sizeof(float));
    for (int i = 0; i < csr.M; i++)
        for (uint32_t p = csr.indptr[i]; p < csr.indptr[i+1]; p++) {
            float v = csr.values[p];
            const float* Brow = B + (int64_t)csr.indices[p] * K;
            float* Crow = C + (int64_t)i * K;
            for (int k = 0; k < K; k++) Crow[k] += v * Brow[k];
        }
}

// ==================== Scalar fallback ====================
void scalar_fallback(const CSR& csr, const float* B, float* C,
                     const std::vector<int>& rows) {
    for (int i : rows) {
        float* Crow = C + (int64_t)i * K;
        for (int k = 0; k < K; k++) Crow[k] = 0;
        for (uint32_t p = csr.indptr[i]; p < csr.indptr[i+1]; p++) {
            float v = csr.values[p];
            const float* Brow = B + (int64_t)csr.indices[p] * K;
            for (int k = 0; k < K; k++) Crow[k] += v * Brow[k];
        }
    }
}

// ==================== AMX SpMM for panels ====================
void amx_spmm_panels(const CSR& csr, const float* B, float* C,
                      const std::vector<Panel>& panels)
{
    // 固定 tile 配置 (所有 panel 共用)
    // tmm0: C累加器 16×16 FP32, tmm1: A 16×32 BF16, tmm2: B 16pairs×16 VNNI
    tile_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.palette_id = 1;
    cfg.rows[0] = 16; cfg.colsb[0] = KC * 4;   // tmm0
    cfg.rows[1] = 16; cfg.colsb[1] = 64;        // tmm1
    cfg.rows[2] = 16; cfg.colsb[2] = KC * 4;    // tmm2
    _tile_loadconfig(&cfg);

    alignas(64) uint16_t A_buf[TILE_R][TILE_C];  // 16×32 BF16
    alignas(64) uint8_t  B_buf[16 * 64];          // 16 VNNI pairs
    alignas(64) float    C_buf[TILE_R][KC];        // 16×16 FP32

    int done = 0;
    for (const auto& panel : panels) {
        int ntiles = (panel.U + TILE_C - 1) / TILE_C;
        const int* cu = panel.col_union.data();
        int cu_sz = panel.U;

        // 对每个 K-chunk (K=32 → 2 chunks of 16)
        for (int kc = 0; kc < K; kc += KC) {
            _tile_zero(0);  // C 累加器清零

            for (int t = 0; t < ntiles; t++) {
                int col_start = t * TILE_C;
                int col_end = std::min(col_start + TILE_C, cu_sz);

                // --- Pack A tile ---
                // A_buf[i][j] = BF16(A[row_i][col_union[col_start+j]])
                memset(A_buf, 0, sizeof(A_buf));
                for (int i = 0; i < TILE_R; i++) {
                    int row = panel.rows[i];
                    // 遍历该行非零, binary search 定位在 col_union 中的位置
                    for (uint32_t p = csr.indptr[row]; p < csr.indptr[row+1]; p++) {
                        int c = csr.indices[p];
                        // binary search in col_union[col_start..col_end)
                        const int* it = std::lower_bound(cu + col_start, cu + col_end, c);
                        if (it != cu + col_end && *it == c) {
                            int j = (int)(it - cu) - col_start;
                            A_buf[i][j] = f32_to_bf16(csr.values[p]);
                        }
                    }
                }

                // --- Gather B → VNNI ---
                // pair p → even=col_start+2p, odd=col_start+2p+1
                memset(B_buf, 0, sizeof(B_buf));
                for (int p = 0; p < 16; p++) {
                    int je = col_start + p * 2;
                    int jo = col_start + p * 2 + 1;
                    uint32_t* pr = (uint32_t*)(B_buf + p * 64);
                    for (int k = 0; k < KC; k++) {
                        uint16_t be = (je < cu_sz) ?
                            f32_to_bf16(B[(int64_t)cu[je] * K + kc + k]) : 0;
                        uint16_t bo = (jo < cu_sz) ?
                            f32_to_bf16(B[(int64_t)cu[jo] * K + kc + k]) : 0;
                        pr[k] = ((uint32_t)bo << 16) | (uint32_t)be;
                    }
                }

                _tile_loadd(1, A_buf, 64);
                _tile_loadd(2, B_buf, 64);
                _tile_dpbf16ps(0, 1, 2);
            }

            // --- Scatter C ---
            _tile_stored(0, C_buf, KC * 4);
            for (int i = 0; i < TILE_R; i++) {
                float* dst = C + (int64_t)panel.rows[i] * K + kc;
                for (int k = 0; k < KC; k++) dst[k] = C_buf[i][k];
            }
        }

        done++;
        if (done % 5000 == 0)
            printf("    ... %d / %d panels done\n", done, (int)panels.size());
    }
    _tile_release();
}

// ==================== Main ====================
int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <matrix.csrbin> [max_panels]\n", argv[0]);
        return 1;
    }
    int max_panels = (argc >= 3) ? atoi(argv[2]) : -1;

    printf("=== Step 4.2: AMX SpMM on Real Matrix ===\n");
    printf("K=%d, tile=%dx%d, fill_thr=%.2f%%\n\n", K, TILE_R, TILE_C, FILL_THR*100);

    syscall(SYS_arch_prctl, 0x1023, 18);

    // --- [1] Read matrix ---
    printf("[1] Reading: %s\n", argv[1]);
    auto t0 = std::chrono::high_resolution_clock::now();
    CSR csr = read_csrbin(argv[1]);
    auto t1 = std::chrono::high_resolution_clock::now();
    printf("    M=%d  N=%d  NNZ=%ld  avg_deg=%.1f  (%.0f ms)\n\n",
        csr.M, csr.N, csr.nnz, (double)csr.nnz/csr.M,
        std::chrono::duration<double,std::milli>(t1-t0).count());

    // --- [2] Build CSC ---
    printf("[2] Building CSC...\n");
    t0 = std::chrono::high_resolution_clock::now();
    CSC csc = build_csc(csr);
    t1 = std::chrono::high_resolution_clock::now();
    printf("    Done (%.0f ms)\n\n",
        std::chrono::duration<double,std::milli>(t1-t0).count());

    // --- [3] Column-anchor grouping ---
    printf("[3] Column-anchor grouping%s...\n",
        max_panels > 0 ? (", max=" + std::to_string(max_panels)).c_str() : "");
    t0 = std::chrono::high_resolution_clock::now();
    std::vector<Panel> amx_panels;
    std::vector<int> avx_rows;
    column_anchor_group(csr, csc, amx_panels, avx_rows, max_panels);
    t1 = std::chrono::high_resolution_clock::now();

    int64_t amx_nnz = 0;
    for (auto& p : amx_panels) amx_nnz += p.total_deg;
    int amx_rc = (int)amx_panels.size() * TILE_R;

    printf("    AMX: %d panels, %d rows (%.1f%%), %ld NNZ (%.1f%%)\n",
        (int)amx_panels.size(), amx_rc, 100.0*amx_rc/csr.M,
        amx_nnz, 100.0*amx_nnz/csr.nnz);
    printf("    AVX: %d rows (%.1f%%), %ld NNZ (%.1f%%)\n",
        (int)avx_rows.size(), 100.0*avx_rows.size()/csr.M,
        csr.nnz - amx_nnz, 100.0*(csr.nnz - amx_nnz)/csr.nnz);

    if (!amx_panels.empty()) {
        std::vector<float> fills;
        std::vector<int> Us;
        for (auto& p : amx_panels) { fills.push_back(p.fill); Us.push_back(p.U); }
        std::sort(fills.begin(), fills.end());
        std::sort(Us.begin(), Us.end());
        int n = (int)fills.size();
        float fmean = std::accumulate(fills.begin(), fills.end(), 0.0f) / n;
        printf("    Fill: mean=%.1f%% med=%.1f%% P95=%.1f%%\n",
            fmean*100, fills[n/2]*100, fills[(int)(n*0.95)]*100);
        double umean = std::accumulate(Us.begin(), Us.end(), 0.0) / n;
        int u32cnt = (int)std::count_if(Us.begin(), Us.end(), [](int u){return u<=32;});
        printf("    U: mean=%.0f med=%d U<=32: %.1f%%\n", umean, Us[n/2], 100.0*u32cnt/n);
    }
    printf("    Grouping: %.0f ms\n\n",
        std::chrono::duration<double,std::milli>(t1-t0).count());

    // --- [4] Generate B ---
    printf("[4] Generating B[%d x %d]...\n", csr.N, K);
    float* B = (float*)aligned_alloc(64, (size_t)csr.N * K * sizeof(float));
    srand(12345);
    for (int64_t i = 0; i < (int64_t)csr.N * K; i++)
        B[i] = (rand() % 200 - 100) / 100.0f;

    // --- [5] Naive FP32 reference ---
    printf("[5] Naive FP32 SpMM...\n");
    float* C_ref = (float*)aligned_alloc(64, (size_t)csr.M * K * sizeof(float));
    t0 = std::chrono::high_resolution_clock::now();
    naive_spmm(csr, B, C_ref);
    t1 = std::chrono::high_resolution_clock::now();
    double naive_ms = std::chrono::duration<double,std::milli>(t1-t0).count();
    printf("    %.0f ms\n\n", naive_ms);

    // --- [6] AMX SpMM ---
    printf("[6] AMX SpMM (%d panels + %d fallback)...\n",
        (int)amx_panels.size(), (int)avx_rows.size());
    float* C_amx = (float*)aligned_alloc(64, (size_t)csr.M * K * sizeof(float));
    memset(C_amx, 0, (size_t)csr.M * K * sizeof(float));

    t0 = std::chrono::high_resolution_clock::now();
    amx_spmm_panels(csr, B, C_amx, amx_panels);
    auto t_mid = std::chrono::high_resolution_clock::now();
    scalar_fallback(csr, B, C_amx, avx_rows);
    t1 = std::chrono::high_resolution_clock::now();

    double amx_ms = std::chrono::duration<double,std::milli>(t_mid-t0).count();
    double avx_ms = std::chrono::duration<double,std::milli>(t1-t_mid).count();
    printf("    AMX: %.0f ms  Fallback: %.0f ms  Total: %.0f ms\n", amx_ms, avx_ms, amx_ms+avx_ms);
    printf("    vs Naive: %.2fx %s\n\n",
        naive_ms / (amx_ms + avx_ms),
        naive_ms > (amx_ms + avx_ms) ? "(faster)" : "(slower)");

    // --- [7] Correctness ---
    printf("[7] Correctness check...\n");
    float max_abs = 0, max_rel = 0;
    double sum_abs = 0;
    int bad = 0, total = csr.M * K;
    for (int64_t i = 0; i < (int64_t)csr.M * K; i++) {
        float ae = fabsf(C_ref[i] - C_amx[i]);
        float re = (fabsf(C_ref[i]) > 1e-6f) ? ae / fabsf(C_ref[i]) : ae;
        sum_abs += ae;
        if (ae > max_abs) max_abs = ae;
        if (re > max_rel) max_rel = re;
        if (re > 0.05f && fabsf(C_ref[i]) > 0.01f) bad++;
    }
    printf("    Max abs err: %.3e  Max rel err: %.3e  Mean abs: %.3e\n",
        max_abs, max_rel, sum_abs / total);
    printf("    Bad (>5%% rel, |ref|>0.01): %d / %d (%.4f%%)\n",
        bad, total, 100.0 * bad / total);
    printf("    %s\n", bad == 0 ? "✅ PASS" :
        (100.0*bad/total < 0.1 ? "⚠️ MARGINAL" : "❌ FAIL"));

    // Sample
    printf("\n    Sample C[0][0..7]:\n");
    printf("      ref:");
    for (int k = 0; k < 8; k++) printf(" %8.4f", C_ref[k]);
    printf("\n      amx:");
    for (int k = 0; k < 8; k++) printf(" %8.4f", C_amx[k]);
    printf("\n");

    free(B); free(C_ref); free(C_amx);
    printf("\n=== Step 4.2 Complete ===\n");
    return 0;
}
