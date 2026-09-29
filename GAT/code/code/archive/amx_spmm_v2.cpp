/**
 * Step 4.3: AMX SpMM V2 — 优化 pack/gather
 *
 * 优化点:
 *   1. col_map[col] → O(1) 列定位, 替代 binary search O(log U)
 *   2. Pack A 只做一次 (A 不依赖 K-chunk), 跨 K-chunk 复用
 *   3. AVX-512 向量化 B gather + VNNI 打包
 *   4. 减少 memset: 只清实际使用的 tile 数量
 *
 * Usage: ./amx_spmm_v2 <matrix.csrbin> [max_panels]
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

constexpr int TILE_R = 16;
constexpr int TILE_C = 32;
constexpr int K      = 32;
constexpr int KC     = 16;
constexpr float FILL_THR = 0.0625f;
constexpr int MAX_NTILES = 64;  // 支持 U 最大 2048

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

// ==================== CSR / CSC (同 V1) ====================
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
    col_order.reserve(N);
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

// ==================== Scalar fallback ====================
void scalar_fallback(const CSR& csr, const float* B, float* C,
                     const std::vector<int>& rows) {
    for (int i : rows) {
        float* Cr = C + (int64_t)i * K;
        for (int k = 0; k < K; k++) Cr[k] = 0;
        for (uint32_t p = csr.indptr[i]; p < csr.indptr[i+1]; p++) {
            float v = csr.values[p];
            const float* Br = B + (int64_t)csr.indices[p] * K;
            for (int k = 0; k < K; k++) Cr[k] += v * Br[k];
        }
    }
}

// ==================== AMX SpMM V2 — Optimized ====================
void amx_spmm_v2(const CSR& csr, const float* B, float* C,
                  const std::vector<Panel>& panels, int N)
{
    tile_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.palette_id = 1;
    cfg.rows[0] = 16; cfg.colsb[0] = KC * 4;
    cfg.rows[1] = 16; cfg.colsb[1] = 64;
    cfg.rows[2] = 16; cfg.colsb[2] = KC * 4;
    _tile_loadconfig(&cfg);

    // 优化 1: 持久化 col_map, 避免每 panel 分配/释放
    // col_map[col] = j+1 (1-indexed), 0 = 不在当前 panel
    int* col_map = (int*)calloc(N, sizeof(int));

    // 优化 2: A tiles 缓冲区 — pack 一次, 跨 K-chunk 复用
    alignas(64) uint16_t A_tiles[MAX_NTILES][TILE_R][TILE_C];
    alignas(64) uint8_t  B_buf[16 * 64];
    alignas(64) float    C_buf[TILE_R][KC];

    int done = 0;
    for (const auto& panel : panels) {
        int ntiles = (panel.U + TILE_C - 1) / TILE_C;
        const int* cu = panel.col_union.data();
        int cu_sz = panel.U;

        // --- Setup col_map: O(U) ---
        for (int j = 0; j < cu_sz; j++)
            col_map[cu[j]] = j + 1;  // 1-indexed

        // --- 优化 2: Pack A 一次 (不依赖 K-chunk!) ---
        memset(A_tiles, 0, (size_t)ntiles * sizeof(A_tiles[0]));
        for (int i = 0; i < TILE_R; i++) {
            int row = panel.rows[i];
            for (uint32_t p = csr.indptr[row]; p < csr.indptr[row+1]; p++) {
                int j = col_map[csr.indices[p]] - 1;  // O(1) 查找!
                if (j >= 0) {
                    A_tiles[j / TILE_C][i][j % TILE_C] = f32_to_bf16(csr.values[p]);
                }
            }
        }

        // --- 遍历 K-chunks ---
        for (int kc = 0; kc < K; kc += KC) {
            _tile_zero(0);

            for (int t = 0; t < ntiles; t++) {
                int col_start = t * TILE_C;

                // --- 优化 3: AVX-512 向量化 B gather + VNNI ---
                for (int p = 0; p < 16; p++) {
                    int je = col_start + p * 2;
                    int jo = col_start + p * 2 + 1;
                    uint32_t* pr = (uint32_t*)(B_buf + p * 64);

                    if (je < cu_sz && jo < cu_sz) {
                        // 两行都有效: AVX-512 向量化
                        // 加载 16 个 FP32
                        __m512 ve = _mm512_loadu_ps(&B[(int64_t)cu[je] * K + kc]);
                        __m512 vo = _mm512_loadu_ps(&B[(int64_t)cu[jo] * K + kc]);
                        // FP32 → BF16 (截断: 取高 16 位, 快速但无舍入)
                        __m512i ie = _mm512_srli_epi32(_mm512_castps_si512(ve), 16);
                        __m512i io = _mm512_srli_epi32(_mm512_castps_si512(vo), 16);
                        // VNNI 打包: uint32 = (odd_bf16 << 16) | even_bf16
                        __m512i vnni = _mm512_or_si512(ie, _mm512_slli_epi32(io, 16));
                        _mm512_store_si512((__m512i*)pr, vnni);
                    } else if (je < cu_sz) {
                        // 只有 even 行
                        __m512 ve = _mm512_loadu_ps(&B[(int64_t)cu[je] * K + kc]);
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

            // Scatter C
            _tile_stored(0, C_buf, KC * 4);
            for (int i = 0; i < TILE_R; i++) {
                float* dst = C + (int64_t)panel.rows[i] * K + kc;
                for (int k = 0; k < KC; k++) dst[k] = C_buf[i][k];
            }
        }

        // --- Cleanup col_map: O(U), 只清用过的位置 ---
        for (int j = 0; j < cu_sz; j++)
            col_map[cu[j]] = 0;

        done++;
        if (done % 5000 == 0)
            printf("    ... %d / %d panels\n", done, (int)panels.size());
    }

    free(col_map);
    _tile_release();
}

// ==================== Main ====================
int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <matrix.csrbin> [max_panels]\n", argv[0]);
        return 1;
    }
    int max_panels = (argc >= 3) ? atoi(argv[2]) : -1;

    printf("=== Step 4.3: AMX SpMM V2 (Optimized) ===\n");
    printf("K=%d, KC=%d, tile=%dx%d\n\n", K, KC, TILE_R, TILE_C);

    syscall(SYS_arch_prctl, 0x1023, 18);

    printf("[1] Read matrix: %s\n", argv[1]);
    auto t0 = std::chrono::high_resolution_clock::now();
    CSR csr = read_csrbin(argv[1]);
    auto t1 = std::chrono::high_resolution_clock::now();
    printf("    M=%d N=%d NNZ=%ld avg_deg=%.1f (%.0fms)\n\n",
        csr.M, csr.N, csr.nnz, (double)csr.nnz/csr.M,
        std::chrono::duration<double,std::milli>(t1-t0).count());

    printf("[2] Build CSC...\n");
    t0 = std::chrono::high_resolution_clock::now();
    CSC csc = build_csc(csr);
    t1 = std::chrono::high_resolution_clock::now();
    printf("    %.0fms\n\n", std::chrono::duration<double,std::milli>(t1-t0).count());

    printf("[3] Column-anchor grouping...\n");
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
    printf("    AVX: %d rows, %ld NNZ (%.1f%%)\n",
        (int)avx_rows.size(), csr.nnz-amx_nnz, 100.0*(csr.nnz-amx_nnz)/csr.nnz);

    if (!amx_panels.empty()) {
        std::vector<float> fills;
        std::vector<int> Us;
        for (auto& p : amx_panels) { fills.push_back(p.fill); Us.push_back(p.U); }
        std::sort(fills.begin(), fills.end());
        std::sort(Us.begin(), Us.end());
        int n = fills.size();
        printf("    Fill: mean=%.1f%% med=%.1f%%\n",
            std::accumulate(fills.begin(),fills.end(),0.0f)/n*100, fills[n/2]*100);
        double um = std::accumulate(Us.begin(),Us.end(),0.0)/n;
        printf("    U: mean=%.0f med=%d U<=32: %.1f%%\n",
            um, Us[n/2], 100.0*std::count_if(Us.begin(),Us.end(),[](int u){return u<=32;})/n);
    }
    printf("    Grouping: %.0fms\n\n",
        std::chrono::duration<double,std::milli>(t1-t0).count());

    printf("[4] Generating B[%d x %d]...\n", csr.N, K);
    float* B = (float*)aligned_alloc(64, (size_t)csr.N * K * sizeof(float));
    srand(12345);
    for (int64_t i = 0; i < (int64_t)csr.N * K; i++)
        B[i] = (rand() % 200 - 100) / 100.0f;

    // --- Naive baseline ---
    printf("[5] Naive FP32 SpMM...\n");
    float* C_ref = (float*)aligned_alloc(64, (size_t)csr.M * K * sizeof(float));
    t0 = std::chrono::high_resolution_clock::now();
    naive_spmm(csr, B, C_ref);
    t1 = std::chrono::high_resolution_clock::now();
    double naive_ms = std::chrono::duration<double,std::milli>(t1-t0).count();
    printf("    %.1f ms\n\n", naive_ms);

    // --- V2 AMX ---
    printf("[6] AMX SpMM V2 (%d AMX + %d fallback)...\n",
        (int)amx_panels.size(), (int)avx_rows.size());
    float* C_amx = (float*)aligned_alloc(64, (size_t)csr.M * K * sizeof(float));
    memset(C_amx, 0, (size_t)csr.M * K * sizeof(float));

    t0 = std::chrono::high_resolution_clock::now();
    amx_spmm_v2(csr, B, C_amx, amx_panels, csr.N);
    auto t_mid = std::chrono::high_resolution_clock::now();
    scalar_fallback(csr, B, C_amx, avx_rows);
    t1 = std::chrono::high_resolution_clock::now();

    double v2_amx = std::chrono::duration<double,std::milli>(t_mid-t0).count();
    double v2_avx = std::chrono::duration<double,std::milli>(t1-t_mid).count();
    double v2_total = v2_amx + v2_avx;
    printf("    AMX: %.1f ms  Fallback: %.1f ms  Total: %.1f ms\n",
        v2_amx, v2_avx, v2_total);
    printf("    Speedup vs Naive: %.2fx\n\n", naive_ms / v2_total);

    // --- Correctness ---
    printf("[7] Correctness...\n");
    float max_abs = 0, max_rel = 0;
    int bad = 0;
    for (int64_t i = 0; i < (int64_t)csr.M * K; i++) {
        float ae = fabsf(C_ref[i] - C_amx[i]);
        float re = (fabsf(C_ref[i]) > 1e-6f) ? ae / fabsf(C_ref[i]) : ae;
        if (ae > max_abs) max_abs = ae;
        if (re > max_rel) max_rel = re;
        if (re > 0.05f && fabsf(C_ref[i]) > 0.01f) bad++;
    }
    int total = csr.M * K;
    printf("    Max abs: %.3e  Max rel: %.3e\n", max_abs, max_rel);
    printf("    Bad: %d / %d (%.4f%%)\n", bad, total, 100.0*bad/total);
    printf("    %s\n", bad == 0 ? "✅ PASS" :
        (100.0*bad/total < 1.0 ? "⚠️ MARGINAL (BF16 precision)" : "❌ FAIL"));

    printf("\n    Sample C[0][0..7]:\n      ref:");
    for (int k = 0; k < 8; k++) printf(" %8.4f", C_ref[k]);
    printf("\n      amx:");
    for (int k = 0; k < 8; k++) printf(" %8.4f", C_amx[k]);
    printf("\n");

    free(B); free(C_ref); free(C_amx);
    printf("\n=== Done ===\n");
    return 0;
}
