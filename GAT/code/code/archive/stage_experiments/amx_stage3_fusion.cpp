/**
 * Stage3: 真实 CSR 图上的融合 kernel
 * 计算：H_out[i,:] = sum_j A[i,j] * H[j,:] * W
 * 对比：FB-BF16（V17c 的 fallback 路径）vs 融合 kernel
 *
 * 实现策略：
 *   每个节点 i，把邻居按 BATCH=16 分组
 *   每组：gather 16行 H → AMX 计算 H_block(16×128) × W(128×128)
 *   尾部（<16 邻居）：补零后同样走 AMX
 *   结果在 C_tile 中累加，最终写回 H_out[i,:]
 */

#include <immintrin.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <omp.h>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>

// ── tile_config_t ────────────────────────────────────────────────
typedef struct {
    uint8_t  palette_id, start_row;
    uint8_t  reserved0[14];
    uint16_t colsb[16];
    uint8_t  rows[16];
} __attribute__((packed)) tile_config_t;
static_assert(sizeof(tile_config_t) == 64, "");

// ── 常量 ─────────────────────────────────────────────────────────
static constexpr int K_DIM  = 128;  // 输入特征维度
static constexpr int N_DIM  = 128;  // 输出特征维度
static constexpr int M_TILE = 16;   // AMX tile 行数
static constexpr int TK     = 32;   // K 分块
static constexpr int TN     = 16;   // N 分块
static constexpr int REPEAT = 10;   // 重复次数取最小
static constexpr int WARMUP = 3;

// ── BF16 工具 ────────────────────────────────────────────────────
static inline uint16_t f32_to_bf16(float f) {
    uint32_t u; memcpy(&u, &f, 4); return (uint16_t)(u >> 16);
}
static inline float bf16_to_f32(uint16_t b) {
    uint32_t u = (uint32_t)b << 16; float f; memcpy(&f, &u, 4); return f;
}

// ── CSR 矩阵 ─────────────────────────────────────────────────────
struct CSRMat {
    uint64_t nrow, ncol, nnz;
    std::vector<uint32_t> indptr;
    std::vector<uint32_t> indices;
};

static bool load_csrbin(const char* path, CSRMat& mat) {
    FILE* fp = fopen(path, "rb");
    if (!fp) { printf("无法打开文件：%s\n", path); return false; }
    uint32_t ptype, dtype, vtype;
    fread(&ptype, 4, 1, fp);
    fread(&dtype, 4, 1, fp);
    fread(&vtype, 4, 1, fp);
    fread(&mat.nrow, 8, 1, fp);
    fread(&mat.ncol, 8, 1, fp);
    fread(&mat.nnz,  8, 1, fp);
    mat.indptr.resize(mat.nrow + 1);
    mat.indices.resize(mat.nnz);
    fread(mat.indptr.data(),  4, mat.nrow + 1, fp);
    fread(mat.indices.data(), 4, mat.nnz,      fp);
    fclose(fp);
    return true;
}

// ── tile 配置 ────────────────────────────────────────────────────
static void configure_tiles() {
    tile_config_t cfg = {};
    cfg.palette_id = 1;
    cfg.rows[0] = M_TILE; cfg.colsb[0] = TN * 4;     // C0 FP32
    cfg.rows[1] = M_TILE; cfg.colsb[1] = TN * 4;     // C1 FP32
    cfg.rows[2] = M_TILE; cfg.colsb[2] = TK * 2;     // A  BF16
    cfg.rows[3] = TK/2;   cfg.colsb[3] = TN * 4;     // B0 VNNI
    cfg.rows[4] = TK/2;   cfg.colsb[4] = TN * 4;     // B1 VNNI
    _tile_loadconfig(&cfg);
}

// ── 打包 W 为 VNNI ───────────────────────────────────────────────
static uint16_t* pack_W_vnni(const float* W) {
    uint16_t* out = (uint16_t*)aligned_alloc(64, K_DIM * N_DIM * 2);
    int off = 0;
    for (int k0 = 0; k0 < K_DIM; k0 += TK)
        for (int n0 = 0; n0 < N_DIM; n0 += TN)
            for (int kp = 0; kp < TK/2; kp++)
                for (int n = 0; n < TN; n++) {
                    out[off++] = f32_to_bf16(W[(k0+2*kp)  *N_DIM+n0+n]);
                    out[off++] = f32_to_bf16(W[(k0+2*kp+1)*N_DIM+n0+n]);
                }
    return out;
}

// ── 把 H FP32 转为 BF16 存储 ─────────────────────────────────────
static uint16_t* convert_H_bf16(const float* H, uint64_t N, int K) {
    uint16_t* out = (uint16_t*)aligned_alloc(64, N * K * 2);
    for (uint64_t i = 0; i < (uint64_t)N * K; i++)
        out[i] = f32_to_bf16(H[i]);
    return out;
}

// ── AMX kernel：C(16×N) += H_buf(16×K) × W(K×N) ─────────────────
// 注意：这里是累加模式（zero_init=false 时从 C_tile 继续累加）
static inline void amx_tile_compute(
    const uint16_t* __restrict__ H_buf,   // [M_TILE × K_DIM] BF16
    const uint16_t* __restrict__ W_vnni,
    float*          __restrict__ C_tile,  // [M_TILE × N_DIM] FP32（累加器）
    bool zero_init)
{
    const int BS     = (TK/2) * TN * 2;
    const int A_STR  = K_DIM * 2;
    const int B_STR  = TN * 4;
    const int C_STR  = N_DIM * 4;

    for (int np = 0; np < N_DIM / (TN*2); np++) {
        if (zero_init) {
            _tile_zero(0);
            _tile_zero(1);
        } else {
            _tile_loadd(0, C_tile + np*TN*2,      C_STR);
            _tile_loadd(1, C_tile + np*TN*2 + TN, C_STR);
        }
        for (int kp = 0; kp < K_DIM / TK; kp++) {
            _tile_loadd(2, H_buf + kp * TK, A_STR);
            _tile_loadd(3, W_vnni + (kp*(N_DIM/TN) + np*2  ) * BS, B_STR);
            _tile_loadd(4, W_vnni + (kp*(N_DIM/TN) + np*2+1) * BS, B_STR);
            _tile_dpbf16ps(0, 2, 3);
            _tile_dpbf16ps(1, 2, 4);
        }
        _tile_stored(0, C_tile + np*TN*2,      C_STR);
        _tile_stored(1, C_tile + np*TN*2 + TN, C_STR);
    }
}

// ── 融合 kernel：完整图遍历 ──────────────────────────────────────
static void fusion_kernel(
    const CSRMat&   A,
    const uint16_t* H_bf16,   // [ncol × K_DIM] BF16
    const uint16_t* W_vnni,
    float*          H_out,    // [nrow × N_DIM] FP32 输出
    int             n_threads)
{
    omp_set_num_threads(n_threads);

    #pragma omp parallel
    {
        // 申请 AMX 权限（每个线程必须独立申请）
        syscall(SYS_arch_prctl, 0x1023, 18);
        configure_tiles();

        // 每个线程独立的 buffer
        uint16_t* H_buf  = (uint16_t*)aligned_alloc(64, M_TILE * K_DIM * 2);
        float*    C_tile = (float*)   aligned_alloc(64, M_TILE * N_DIM * 4);

        #pragma omp for schedule(dynamic, 64)
        for (int64_t i = 0; i < (int64_t)A.nrow; i++) {
            int64_t s = A.indptr[i];
            int64_t e = A.indptr[i+1];
            int64_t deg = e - s;

            if (deg == 0) {
                // 孤立节点：输出全零
                memset(H_out + i * N_DIM, 0, N_DIM * 4);
                continue;
            }

            // C_tile 存放第 i 行的输出累加器（M_TILE 行中只有第 0 行有意义）
            // 但 AMX 必须操作整个 16×128 tile，所以我们用第 0 行收集结果
            // 策略：把邻居按 M_TILE 分组，每组走一次 AMX
            // 最后把 C_tile 的 16 行对 N_DIM 维度做 reduction（只取第 0 行）
            //
            // 更优策略：把一个节点的所有邻居都打包成 [ceil(deg/16)×16, K] 的块
            // 然后做多次 AMX 调用，每次累加到同一个 C_tile
            // 这样 C_tile[0,:] = sum over all neighbors

            memset(C_tile, 0, M_TILE * N_DIM * 4);

            int64_t p = s;
            bool first = true;

            while (p < e) {
                int batch = (int)std::min((int64_t)M_TILE, e - p);

                // gather batch 个邻居的特征到 H_buf
                for (int b = 0; b < batch; b++) {
                    uint32_t j = A.indices[p + b];
                    memcpy(H_buf + b * K_DIM,
                           H_bf16 + (size_t)j * K_DIM,
                           K_DIM * 2);
                }
                // 不足 M_TILE 的补零
                if (batch < M_TILE)
                    memset(H_buf + batch * K_DIM, 0,
                           (M_TILE - batch) * K_DIM * 2);

                // AMX 计算：H_buf × W，累加到 C_tile
                // 第一次 zero_init=true，之后 zero_init=false（继续累加）
                amx_tile_compute(H_buf, W_vnni, C_tile, first);
                first = false;
                p += batch;
            }

            // C_tile 目前是 M_TILE 行的累加结果
            // 由于每批 gather 的邻居都在各自行累加，
            // 我们需要把 16 行 reduction 成 1 行
            // H_out[i,:] = sum of C_tile[0,:] + C_tile[1,:] + ... + C_tile[15,:]
            float* out_row = H_out + i * N_DIM;
            for (int n = 0; n < N_DIM; n++) {
                float acc = 0.0f;
                for (int r = 0; r < M_TILE; r++)
                    acc += C_tile[r * N_DIM + n];
                out_row[n] = acc;
            }
        }

        free(H_buf);
        free(C_tile);
        _tile_release();
    }
}

// ── FB-BF16 baseline（纯 AVX-512 FP32 累加，BF16 B）────────────────
static void fb_bf16_kernel(
    const CSRMat&   A,
    const uint16_t* H_bf16,
    float*          H_out,
    int             n_threads)
{
    omp_set_num_threads(n_threads);

    #pragma omp parallel for schedule(dynamic, 64)
    for (int64_t i = 0; i < (int64_t)A.nrow; i++) {
        float* out = H_out + i * K_DIM;
        memset(out, 0, K_DIM * 4);
        for (int64_t p = A.indptr[i]; p < (int64_t)A.indptr[i+1]; p++) {
            uint32_t j = A.indices[p];
            const uint16_t* hj = H_bf16 + (size_t)j * K_DIM;
            for (int k = 0; k < K_DIM; k++)
                out[k] += bf16_to_f32(hj[k]);
        }
    }
}

// ── 计时工具 ─────────────────────────────────────────────────────
template<typename Fn>
static double bench_ms(Fn fn, int warmup, int repeat) {
    for (int i = 0; i < warmup; i++) fn();
    double best = 1e18;
    for (int i = 0; i < repeat; i++) {
        auto t0 = std::chrono::high_resolution_clock::now();
        fn();
        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double,std::milli>(t1-t0).count();
        best = std::min(best, ms);
    }
    return best;
}

// ── 主函数 ───────────────────────────────────────────────────────
int main(int argc, char** argv) {
    const char* DEFAULT_MATRICES[] = {
        "/home/huangjianqiang_group/hdacp1/data/SpMM_project/data/web-Google/web-Google.csrbin",
        "/home/huangjianqiang_group/hdacp1/data/SpMM_project/data/amazon0601/amazon0601.csrbin",
        nullptr
    };

    int n_threads = 32;
    if (argc >= 2) n_threads = atoi(argv[1]);

    printf("=== Stage3: Fusion Kernel on Real CSR Graph ===\n");
    printf("K=%d  N=%d  threads=%d  repeat=%d\n\n",
           K_DIM, N_DIM, n_threads, REPEAT);

    // W 矩阵初始化（固定 seed，模拟 GNN 权重）
    float* W_fp32 = (float*)aligned_alloc(64, K_DIM * N_DIM * 4);
    srand(42);
    for (int i = 0; i < K_DIM * N_DIM; i++)
        W_fp32[i] = (rand()%200-100) / 100.0f;
    uint16_t* W_vnni = pack_W_vnni(W_fp32);
    free(W_fp32);

    printf("%-16s %8s %10s %12s %12s %8s\n",
           "矩阵", "NNZ(M)", "avg_deg",
           "FB-BF16(ms)", "Fusion(ms)", "加速比");
    printf("%s\n", std::string(72, '-').c_str());

    for (int mi = 0; DEFAULT_MATRICES[mi]; mi++) {
        const char* path = DEFAULT_MATRICES[mi];

        CSRMat A;
        if (!load_csrbin(path, A)) continue;

        double avg_deg = (double)A.nnz / A.nrow;
        printf("%-16s %8.1f %10.1f",
               strrchr(path, '/') + 1,
               A.nnz / 1e6,
               avg_deg);
        fflush(stdout);

        // H 矩阵（输入特征，随机初始化）
        float*    H_fp32 = (float*)   aligned_alloc(64, A.ncol * K_DIM * 4);
        for (size_t i = 0; i < A.ncol * K_DIM; i++)
            H_fp32[i] = (rand()%200-100) / 100.0f;
        uint16_t* H_bf16 = convert_H_bf16(H_fp32, A.ncol, K_DIM);
        free(H_fp32);

        // 输出矩阵
        float* H_out_fb     = (float*)aligned_alloc(64, A.nrow * K_DIM * 4);
        float* H_out_fusion = (float*)aligned_alloc(64, A.nrow * N_DIM * 4);
        memset(H_out_fb,     0, A.nrow * K_DIM * 4);
        memset(H_out_fusion, 0, A.nrow * N_DIM * 4);

        // ── FB-BF16 基准 ──────────────────────────────────────
        double ms_fb = bench_ms(
            [&]() { fb_bf16_kernel(A, H_bf16, H_out_fb, n_threads); },
            WARMUP, REPEAT);
        printf(" %12.2f", ms_fb);
        fflush(stdout);

        // ── 融合 kernel ───────────────────────────────────────
        double ms_fusion = bench_ms(
            [&]() { fusion_kernel(A, H_bf16, W_vnni, H_out_fusion, n_threads); },
            WARMUP, REPEAT);
        printf(" %12.2f", ms_fusion);

        double speedup = ms_fb / ms_fusion;
        printf(" %8.2f×\n", speedup);

        free(H_bf16);
        free(H_out_fb);
        free(H_out_fusion);
    }

    free(W_vnni);

    printf("\n注：融合 kernel 同时完成 SpMM + GeMM 两步\n");
    printf("    FB-BF16 只完成 SpMM（无 GeMM），对比是保守的\n");
    printf("    真实加速比应与 SpMM+GeMM 两步总时间对比\n");
    printf("\n=== Done ===\n");
    return 0;
}
