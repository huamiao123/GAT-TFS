/**
 * Stage2 简化版：Gather 基准 + Gather+AMX 串行
 * 只测两种模式，确认基本流水线行为
 */
#include <immintrin.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <chrono>
#include <algorithm>
#include <vector>

typedef struct {
    uint8_t  palette_id, start_row;
    uint8_t  reserved0[14];
    uint16_t colsb[16];
    uint8_t  rows[16];
} __attribute__((packed)) tile_config_t;
static_assert(sizeof(tile_config_t) == 64, "");

static constexpr int M_TILE  = 16;
static constexpr int K_DIM   = 128;
static constexpr int N_DIM   = 128;
static constexpr int TK      = 32;
static constexpr int TN      = 16;
static constexpr int N_NODES = 1024;
static constexpr int BATCH   = 16;   // 每次处理 16 个邻居
static constexpr int N_ITERS = 5000;
static constexpr int WARMUP  = 200;

static inline uint16_t f32_to_bf16(float f) {
    uint32_t u; memcpy(&u, &f, 4); return (uint16_t)(u >> 16);
}

static void configure_tiles() {
    tile_config_t cfg = {};
    cfg.palette_id = 1;
    // tmm0, tmm1: C 累加器 FP32 (16行 × 16列)
    cfg.rows[0] = M_TILE; cfg.colsb[0] = TN * 4;
    cfg.rows[1] = M_TILE; cfg.colsb[1] = TN * 4;
    // tmm2: A BF16 (16行 × 32列)
    cfg.rows[2] = M_TILE; cfg.colsb[2] = TK * 2;
    // tmm3, tmm4: B VNNI (16行 × 32列 pairs)
    cfg.rows[3] = TK/2;   cfg.colsb[3] = TN * 4;
    cfg.rows[4] = TK/2;   cfg.colsb[4] = TN * 4;
    _tile_loadconfig(&cfg);
}

static void pack_W_vnni(const float* W, uint16_t* out) {
    int off = 0;
    for (int k0 = 0; k0 < K_DIM; k0 += TK)
        for (int n0 = 0; n0 < N_DIM; n0 += TN)
            for (int kp = 0; kp < TK/2; kp++)
                for (int n = 0; n < TN; n++) {
                    out[off++] = f32_to_bf16(W[(k0+2*kp)  *N_DIM+n0+n]);
                    out[off++] = f32_to_bf16(W[(k0+2*kp+1)*N_DIM+n0+n]);
                }
}

// gather 16 个 B 行到连续 buffer
static void gather_rows(
    const int* idx, int n,
    const uint16_t* B, uint16_t* buf)
{
    for (int k = 0; k < n; k++)
        memcpy(buf + k * K_DIM, B + (size_t)idx[k] * K_DIM,
               K_DIM * sizeof(uint16_t));
    // 不足 M_TILE 行的补零
    if (n < M_TILE)
        memset(buf + n * K_DIM, 0,
               (M_TILE - n) * K_DIM * sizeof(uint16_t));
}

// AMX 计算：C(16×128) = H_buf(16×128) × W(128×128)，结果写到 C
static void amx_fused(
    const uint16_t* H_buf,
    const uint16_t* W_vnni,
    float* C)
{
    const int BS = (TK/2) * TN * 2;  // B block size in uint16
    const int A_STR = K_DIM * 2;     // bytes
    const int B_STR = TN * 4;        // bytes (pairs)
    const int C_STR = N_DIM * 4;     // bytes

    for (int np = 0; np < N_DIM / (TN*2); np++) {
        _tile_zero(0);
        _tile_zero(1);
        for (int kp = 0; kp < K_DIM / TK; kp++) {
            _tile_loadd(2, H_buf + kp * TK, A_STR);
            _tile_loadd(3, W_vnni + (kp*(N_DIM/TN) + np*2  )*BS, B_STR);
            _tile_loadd(4, W_vnni + (kp*(N_DIM/TN) + np*2+1)*BS, B_STR);
            _tile_dpbf16ps(0, 2, 3);
            _tile_dpbf16ps(1, 2, 4);
        }
        _tile_stored(0, C + np*TN*2,      C_STR);
        _tile_stored(1, C + np*TN*2 + TN, C_STR);
    }
}

int main() {
    printf("=== Stage2: Gather + AMX Pipeline ===\n");
    printf("N_NODES=%d  K=%d  N=%d  BATCH=%d  N_ITERS=%d\n\n",
           N_NODES, K_DIM, N_DIM, BATCH, N_ITERS);

    syscall(SYS_arch_prctl, 0x1023, 18);

    // 分配内存
    size_t B_elems = (size_t)N_NODES * K_DIM;
    uint16_t* B     = (uint16_t*)aligned_alloc(64, B_elems * 2);
    float*    W     = (float*)   aligned_alloc(64, K_DIM * N_DIM * 4);
    uint16_t* W_v   = (uint16_t*)aligned_alloc(64, K_DIM * N_DIM * 2);
    uint16_t* H_buf = (uint16_t*)aligned_alloc(64, M_TILE * K_DIM * 2);
    // C tile：16行 × 128列 FP32
    float*    C_tile= (float*)   aligned_alloc(64, M_TILE * N_DIM * 4);

    if (!B||!W||!W_v||!H_buf||!C_tile) {
        printf("内存分配失败\n"); return 1;
    }

    // 初始化
    srand(42);
    for (size_t i = 0; i < B_elems; i++)
        B[i] = f32_to_bf16((rand()%200-100)/100.0f);
    for (int i = 0; i < K_DIM*N_DIM; i++)
        W[i] = (rand()%200-100)/100.0f;
    pack_W_vnni(W, W_v);
    memset(C_tile, 0, M_TILE * N_DIM * 4);

    // 生成随机邻居（每节点 BATCH 个）
    std::vector<std::vector<int>> nbrs(N_NODES, std::vector<int>(BATCH));
    for (int i = 0; i < N_NODES; i++)
        for (int k = 0; k < BATCH; k++)
            nbrs[i][k] = rand() % N_NODES;

    configure_tiles();

    // ── Mode A：纯 gather ───────────────────────────────────────
    printf("--- Mode A: 纯 gather（无 AMX 计算）---\n");
    for (int it = 0; it < WARMUP; it++)
        for (int i = 0; i < N_NODES; i++)
            gather_rows(nbrs[i].data(), BATCH, B, H_buf);

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int it = 0; it < N_ITERS; it++)
        for (int i = 0; i < N_NODES; i++)
            gather_rows(nbrs[i].data(), BATCH, B, H_buf);
    auto t1 = std::chrono::high_resolution_clock::now();

    double ms_A = std::chrono::duration<double,std::milli>(t1-t0).count() / N_ITERS;
    double bw_A = (double)N_NODES * BATCH * K_DIM * 2 / (ms_A * 1e6);
    printf("  %.4f ms/iter  带宽=%.1f GB/s  每次gather=%.1f ns\n\n",
           ms_A, bw_A, ms_A/N_NODES/BATCH*1e6);

    // ── Mode B：gather + AMX 串行 ───────────────────────────────
    printf("--- Mode B: gather + AMX 串行 ---\n");
    for (int it = 0; it < WARMUP; it++)
        for (int i = 0; i < N_NODES; i++) {
            gather_rows(nbrs[i].data(), BATCH, B, H_buf);
            amx_fused(H_buf, W_v, C_tile);
        }

    t0 = std::chrono::high_resolution_clock::now();
    for (int it = 0; it < N_ITERS; it++)
        for (int i = 0; i < N_NODES; i++) {
            gather_rows(nbrs[i].data(), BATCH, B, H_buf);
            amx_fused(H_buf, W_v, C_tile);
        }
    t1 = std::chrono::high_resolution_clock::now();

    double ms_B = std::chrono::duration<double,std::milli>(t1-t0).count() / N_ITERS;
    double flops = (double)N_NODES * 2 * M_TILE * K_DIM * N_DIM;
    double gflops_B = flops / (ms_B * 1e6);
    printf("  %.4f ms/iter  GFLOPS=%.1f\n\n", ms_B, gflops_B);

    // ── 汇总 ────────────────────────────────────────────────────
    double compute_ms = ms_B - ms_A;
    printf("======================================\n");
    printf("  gather 时间：    %.4f ms  (%.1f%%)\n",
           ms_A, ms_A/ms_B*100);
    printf("  compute 时间：   %.4f ms  (%.1f%%)\n",
           compute_ms, compute_ms/ms_B*100);
    printf("  总时间(串行)：   %.4f ms\n", ms_B);
    printf("  gather/compute： %.2f×\n", ms_A/compute_ms);
    printf("\n");

    if (ms_A / compute_ms < 1.5)
        printf("  结论：✅ gather ≈ compute，双缓冲流水线效率高，进入Stage3\n");
    else if (ms_A / compute_ms < 5.0)
        printf("  结论：⚠️  gather 略快，双缓冲有部分收益\n");
    else
        printf("  结论：❌ gather >> compute（%.1f×），AMX计算被淹没\n",
               ms_A/compute_ms);

    _tile_release();
    free(B); free(W); free(W_v); free(H_buf); free(C_tile);
    printf("\n=== Done ===\n");
    return 0;
}
