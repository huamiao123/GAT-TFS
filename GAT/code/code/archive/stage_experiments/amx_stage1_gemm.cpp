/**
 * Stage1: AMX Microkernel Benchmark
 * 计算 C(16×128) = H(16×128) × W(128×128)
 * BF16 输入，FP32 输出累加
 *
 * Tile 映射：
 *   tmm0, tmm1: C 累加器 (16行 × 16 FP32 = 64字节/行)
 *   tmm2:       A block  (16行 × 32 BF16 = 64字节/行)
 *   tmm3, tmm4: B VNNI   (16行 × 16 pairs = 64字节/行)
 * 每 pass 处理 32 个输出列，共 4 pass
 */

#include <immintrin.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <chrono>
#include <cmath>
#include <algorithm>

// ── tile_config_t（与 V17c 完全一致）────────────────────────────
typedef struct {
    uint8_t  palette_id;
    uint8_t  start_row;
    uint8_t  reserved0[14];
    uint16_t colsb[16];
    uint8_t  rows[16];
} __attribute__((packed)) tile_config_t;
static_assert(sizeof(tile_config_t) == 64, "tile_config size error");

// ── 常量 ──────────────────────────────────────────────────────────
static constexpr int M_TILE = 16;    // AMX tile 行数
static constexpr int K_DIM  = 128;   // 输入特征维度
static constexpr int N_DIM  = 128;   // 输出特征维度
static constexpr int TK     = 32;    // K 方向分块（32 BF16 = 64字节）
static constexpr int TN     = 16;    // N 方向分块（16 FP32 = 64字节）
static constexpr int WARMUP = 1000;
static constexpr int ITER   = 50000;

// ── BF16 工具 ─────────────────────────────────────────────────────
static inline uint16_t f32_to_bf16(float f) {
    uint32_t u; memcpy(&u, &f, 4); return uint16_t(u >> 16);
}
static inline float bf16_to_f32(uint16_t b) {
    uint32_t u = uint32_t(b) << 16; float f; memcpy(&f, &u, 4); return f;
}

// ── 配置 AMX tiles ────────────────────────────────────────────────
static void configure_tiles() {
    tile_config_t cfg = {};
    cfg.palette_id = 1;
    cfg.start_row  = 0;
    // tmm0, tmm1: C 累加器 (FP32: 16行 × 16列 = 64字节)
    cfg.rows[0] = M_TILE; cfg.colsb[0] = TN * sizeof(float);
    cfg.rows[1] = M_TILE; cfg.colsb[1] = TN * sizeof(float);
    // tmm2: A tile (BF16: 16行 × 32列 = 64字节)
    cfg.rows[2] = M_TILE; cfg.colsb[2] = TK * sizeof(uint16_t);
    // tmm3, tmm4: B VNNI (TK/2=16行 × 16 pairs = 64字节)
    cfg.rows[3] = TK / 2; cfg.colsb[3] = TN * 2 * sizeof(uint16_t);
    cfg.rows[4] = TK / 2; cfg.colsb[4] = TN * 2 * sizeof(uint16_t);
    _tile_loadconfig(&cfg);
}

// ── 打包 H 为 BF16 row-major ─────────────────────────────────────
static void pack_H_bf16(const float* H, uint16_t* H_out) {
    for (int i = 0; i < M_TILE * K_DIM; i++)
        H_out[i] = f32_to_bf16(H[i]);
}

// ── 打包 W 为 AMX VNNI 格式 ──────────────────────────────────────
// 输入：W[K_DIM][N_DIM] float，row-major
// 输出：按 (k_block, n_block) 分块，每块内按 VNNI 交织
//
// VNNI 格式：B tile 的 row i 对应 K 方向的第 2i 和 2i+1 行
// 每个 "cell" = (W[k_start+2i, n_start+n], W[k_start+2i+1, n_start+n]) 两个 BF16
// 内存布局：[k_block][n_block][TK/2 × TN × 2] uint16
static void pack_W_vnni(const float* W, uint16_t* W_out) {
    // 遍历所有 (k_block, n_block) 组合
    // k_block: K_DIM/TK = 4 个
    // n_block: N_DIM/TN = 8 个
    int offset = 0;
    for (int k0 = 0; k0 < K_DIM; k0 += TK) {
        for (int n0 = 0; n0 < N_DIM; n0 += TN) {
            // 填充一个 B tile：TK/2 行 × TN 列
            for (int kp = 0; kp < TK/2; kp++) {    // kp = k_pair 索引
                for (int n = 0; n < TN; n++) {
                    // 第 kp 行，第 n 列：存入 k=k0+2*kp 和 k=k0+2*kp+1 的值
                    W_out[offset++] = f32_to_bf16(W[(k0 + 2*kp)   * N_DIM + n0 + n]);
                    W_out[offset++] = f32_to_bf16(W[(k0 + 2*kp+1) * N_DIM + n0 + n]);
                }
            }
        }
    }
}

// ── AMX GEMM kernel：C(16×128) = H(16×128) × W(128×128) ─────────
// H_bf16: row-major BF16，stride = K_DIM * 2 字节
// W_vnni: 预打包好的 VNNI 格式
// C_out:  row-major FP32，stride = N_DIM * 4 字节
static void amx_gemm_kernel(
    const uint16_t* __restrict__ H_bf16,
    const uint16_t* __restrict__ W_vnni,
    float*          __restrict__ C_out)
{
    // 每个 B tile 占 (TK/2 × TN × 2) uint16 = TK*TN uint16
    const int B_BLOCK_SIZE = (TK/2) * TN * 2;  // = 16 × 16 × 2 = 512 uint16
    // stride for B tile tileload：TN 个 pairs × 4 bytes = TN*4 字节
    const int B_STRIDE = TN * 2 * sizeof(uint16_t);  // = 64 字节
    // stride for A tile tileload：K_DIM BF16 × 2 字节
    const int A_STRIDE = K_DIM * sizeof(uint16_t);    // = 256 字节
    // stride for C tile tilestore/tileload：N_DIM FP32 × 4 字节
    const int C_STRIDE = N_DIM * sizeof(float);       // = 512 字节

    // 4 passes：每 pass 处理 2×TN=32 个输出列
    for (int n_pass = 0; n_pass < N_DIM / (TN * 2); n_pass++) {
        int n0 = n_pass * TN * 2;

        // 初始化两个 C 累加器
        _tile_zero(0);   // cols n0 .. n0+TN-1
        _tile_zero(1);   // cols n0+TN .. n0+2*TN-1

        // 4 个 K 块
        for (int k_pass = 0; k_pass < K_DIM / TK; k_pass++) {
            int k0 = k_pass * TK;

            // 加载 A block：H 的第 [0..15] 行，列 [k0..k0+TK-1]
            _tile_loadd(2,
                H_bf16 + k0,    // 指向 H[0][k0]
                A_STRIDE);       // 行跨度 = K_DIM × 2 字节

            // 计算当前 (k_pass, n_pass) 对应的 B 块偏移
            // k_block = k_pass, n_block_base = n_pass*2
            int b0_off = (k_pass * (N_DIM/TN) + n_pass*2    ) * B_BLOCK_SIZE;
            int b1_off = (k_pass * (N_DIM/TN) + n_pass*2 + 1) * B_BLOCK_SIZE;

            // 加载 B0, B1
            _tile_loadd(3, W_vnni + b0_off, B_STRIDE);
            _tile_loadd(4, W_vnni + b1_off, B_STRIDE);

            // C0 += A × B0, C1 += A × B1
            _tile_dpbf16ps(0, 2, 3);
            _tile_dpbf16ps(1, 2, 4);
        }

        // 存结果到 C_out
        _tile_stored(0, C_out + n0,       C_STRIDE);
        _tile_stored(1, C_out + n0 + TN,  C_STRIDE);
    }
}

// ── 参考实现 (FP32 naive) ────────────────────────────────────────
static void naive_gemm(const float* H, const float* W, float* C) {
    memset(C, 0, M_TILE * N_DIM * sizeof(float));
    for (int i = 0; i < M_TILE; i++)
        for (int k = 0; k < K_DIM; k++)
            for (int j = 0; j < N_DIM; j++)
                C[i*N_DIM + j] += H[i*K_DIM + k] * W[k*N_DIM + j];
}

// ── 主函数 ───────────────────────────────────────────────────────
int main() {
    printf("=== Stage1: AMX Microkernel Benchmark ===\n");
    printf("C(%d×%d) = H(%d×%d) × W(%d×%d), BF16 input, FP32 output\n\n",
        M_TILE, N_DIM, M_TILE, K_DIM, K_DIM, N_DIM);

    // 分配对齐内存
    float*    H_fp32 = (float*)   aligned_alloc(64, M_TILE * K_DIM * sizeof(float));
    float*    W_fp32 = (float*)   aligned_alloc(64, K_DIM  * N_DIM * sizeof(float));
    float*    C_ref  = (float*)   aligned_alloc(64, M_TILE * N_DIM * sizeof(float));
    float*    C_amx  = (float*)   aligned_alloc(64, M_TILE * N_DIM * sizeof(float));
    uint16_t* H_bf16 = (uint16_t*)aligned_alloc(64, M_TILE * K_DIM * sizeof(uint16_t));
    uint16_t* W_vnni = (uint16_t*)aligned_alloc(64, K_DIM  * N_DIM * sizeof(uint16_t));

    if (!H_fp32 || !W_fp32 || !C_ref || !C_amx || !H_bf16 || !W_vnni) {
        printf("内存分配失败\n"); return 1;
    }

    // 初始化
    srand(42);
    for (int i = 0; i < M_TILE * K_DIM; i++) H_fp32[i] = (rand()%200-100)/100.0f;
    for (int i = 0; i < K_DIM  * N_DIM; i++) W_fp32[i] = (rand()%200-100)/100.0f;

    // 打包
    pack_H_bf16(H_fp32, H_bf16);
    pack_W_vnni(W_fp32, W_vnni);

    // 参考结果
    naive_gemm(H_fp32, W_fp32, C_ref);

    // AMX 初始化
    // 向 OS 申请 AMX 权限（必须，否则 Illegal instruction）
    // ARCH_REQ_XCOMP_PERM=0x1023, XFEATURE_XTILEDATA=18
    syscall(SYS_arch_prctl, 0x1023, 18);
    configure_tiles();

    // ── 正确性验证 ──────────────────────────────────────────────
    memset(C_amx, 0, M_TILE * N_DIM * sizeof(float));
    amx_gemm_kernel(H_bf16, W_vnni, C_amx);

    float max_abs = 0, max_rel = 0;
    for (int i = 0; i < M_TILE * N_DIM; i++) {
        float ae = fabsf(C_amx[i] - C_ref[i]);
        float re = ae / (fabsf(C_ref[i]) + 1e-6f);
        max_abs = std::max(max_abs, ae);
        max_rel = std::max(max_rel, re);
    }
    printf("[1] 正确性验证\n");
    printf("  最大绝对误差：%.3e\n", max_abs);
    printf("  最大相对误差：%.3e\n", max_rel);
    printf("  结论：%s\n\n", max_rel < 0.02f ? "✅ 通过（BF16精度损失在预期内）"
                                              : "❌ 误差过大，检查 tile 映射");

    // ── 打印前几个值对比 ─────────────────────────────────────────
    printf("[2] 前4个输出值对比\n");
    printf("  %-6s  %-12s  %-12s  %-10s\n", "idx", "ref(FP32)", "amx(BF16)", "rel_err");
    for (int i = 0; i < 4; i++) {
        float ae = fabsf(C_amx[i] - C_ref[i]);
        float re = ae / (fabsf(C_ref[i]) + 1e-6f);
        printf("  %-6d  %-12.6f  %-12.6f  %-10.4f%%\n",
               i, C_ref[i], C_amx[i], re*100);
    }
    printf("\n");

    // ── 性能测试 ────────────────────────────────────────────────
    // Warmup
    for (int i = 0; i < WARMUP; i++)
        amx_gemm_kernel(H_bf16, W_vnni, C_amx);

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITER; i++)
        amx_gemm_kernel(H_bf16, W_vnni, C_amx);
    auto t1 = std::chrono::high_resolution_clock::now();

    double ms_total  = std::chrono::duration<double,std::milli>(t1-t0).count();
    double ms_per    = ms_total / ITER;
    double flops     = 2.0 * M_TILE * K_DIM * N_DIM;  // 2×16×128×128 = 524288
    double gflops    = flops / (ms_per * 1e6);

    // 单核 AMX 峰值：64核共 167 TFLOPS → 单核 ≈ 2609 GFLOPS
    double peak_1core = 167000.0 / 64.0;
    double util       = gflops / peak_1core * 100.0;

    printf("[3] 性能测试（repeat=%d）\n", ITER);
    printf("  单次耗时：%.4f ms\n",   ms_per);
    printf("  GFLOPS：  %.1f\n",       gflops);
    printf("  单核峰值：%.0f GFLOPS\n", peak_1core);
    printf("  AMX利用率：%.1f%%\n",    util);
    printf("  结论：%s\n\n",
        util > 80.0 ? "✅ 优秀（>80%）：tile 映射正确，可进入 Stage2" :
        util > 50.0 ? "⚠️ 中等（50-80%）：有优化空间，检查 tileload 顺序" :
                      "❌ 差（<50%）：tile 映射有问题，需要重新设计");

    // ── Roofline 参考 ────────────────────────────────────────────
    printf("[4] Roofline 参考\n");
    printf("  FLOP/次：%.0f\n",               flops);
    printf("  峰值理论耗时：%.6f ms\n",        flops / (peak_1core * 1e6));
    printf("  80%%利用率耗时：%.6f ms\n",       flops / (peak_1core * 0.8 * 1e6));
    printf("  实测：%.6f ms\n",                ms_per);

    _tile_release();
    free(H_fp32); free(W_fp32); free(C_ref); free(C_amx);
    free(H_bf16); free(W_vnni);

    printf("\n=== Done ===\n");
    return 0;
}
