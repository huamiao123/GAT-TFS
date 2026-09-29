/**
 * Step 4.1: Single Panel AMX SpMM — 正确性验证
 * 
 * 验证数据通路: pack A(BF16) → gather B(VNNI BF16) → TDPBF16PS → C(FP32)
 * 
 * 测试场景: 16 行, 列并集 U=40 (需要 2 个 A-tile), K=16
 * 与 FP32 naive 和 BF16 参考对比
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <immintrin.h>
#include <sys/syscall.h>
#include <unistd.h>

// === AMX tile 配置结构体 (踩过坑: 不加 reserved1, sizeof 必须 = 64!) ===
typedef struct __attribute__((aligned(64))) {
    uint8_t  palette_id;      // byte 0, 必须 = 1
    uint8_t  start_row;       // byte 1
    uint8_t  reserved0[14];   // bytes 2-15
    uint16_t colsb[16];       // bytes 16-47: 每个 tile 的列字节数
    uint8_t  rows[16];        // bytes 48-63: 每个 tile 的行数
} tile_config_t;

// === BF16 ↔ FP32 转换 ===
// BF16 = FP32 的高 16 位 (1 sign + 8 exp + 7 mantissa)
static inline uint16_t fp32_to_bf16(float f) {
    uint32_t u;
    memcpy(&u, &f, 4);
    // Round-to-nearest-even: 加 0x7FFF + 第16位(guard bit)
    u += 0x7FFF + ((u >> 16) & 1);
    return (uint16_t)(u >> 16);
}

static inline float bf16_to_fp32(uint16_t b) {
    uint32_t u = ((uint32_t)b) << 16;
    float f;
    memcpy(&f, &u, 4);
    return f;
}

int main() {
    printf("=== Step 4.1: Single Panel AMX SpMM Correctness Test ===\n\n");

    // 检查 tile_config_t 大小
    static_assert(sizeof(tile_config_t) == 64, "tile_config_t must be 64 bytes!");

    // 请求 AMX 权限: ARCH_REQ_XCOMP_PERM(0x1023), XFEATURE_XTILEDATA(18)
    long ret = syscall(SYS_arch_prctl, 0x1023, 18);
    printf("AMX permission: %s (ret=%ld)\n", ret == 0 ? "granted" : "already/error", ret);

    // ============================================================
    // 参数
    // ============================================================
    constexpr int ROWS    = 16;   // Panel 行数 (= AMX tile 行数)
    constexpr int U       = 40;   // 列并集大小
    constexpr int K       = 16;   // 稠密矩阵 B 的宽度 (= AMX C tile 列数)
    constexpr int NTILES  = (U + 31) / 32;  // A-tile 数量 = ceil(40/32) = 2
    constexpr int STRIDE  = 64;   // 所有 tile 行距 = 64 bytes

    printf("Panel: %d rows, U=%d → %d A-tiles, K=%d\n", ROWS, U, NTILES, K);

    // ============================================================
    // 构造测试数据
    // ============================================================

    // 列并集: 散布在 [7, 982] 范围内的列索引
    int col_union[U];
    for (int i = 0; i < U; i++) col_union[i] = i * 25 + 7;
    int N_cols = col_union[U - 1] + 1;  // B 的行数上限

    // 稀疏 A[16][U], ~30% 填充
    float A_sp[ROWS][U];
    memset(A_sp, 0, sizeof(A_sp));
    srand(42);
    int nnz = 0;
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < U; j++)
            if (rand() % 100 < 30) {
                A_sp[i][j] = (rand() % 100 - 50) / 10.0f;
                nnz++;
            }

    float fill_pct = 100.0f * nnz / (ROWS * 32 * NTILES);
    printf("NNZ=%d, Fill=%.1f%% (capacity=%d)\n\n", nnz, fill_pct, ROWS * 32 * NTILES);

    // 稠密 B[N_cols][K], 随机值
    float* B = (float*)aligned_alloc(64, (size_t)N_cols * K * sizeof(float));
    for (int i = 0; i < N_cols * K; i++)
        B[i] = (rand() % 100 - 50) / 50.0f;

    // ============================================================
    // 参考结果 1: FP32 naive SpMM
    // ============================================================
    float C_ref[ROWS][K];
    memset(C_ref, 0, sizeof(C_ref));
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < U; j++)
            if (A_sp[i][j] != 0.0f)
                for (int k = 0; k < K; k++)
                    C_ref[i][k] += A_sp[i][j] * B[col_union[j] * K + k];

    // ============================================================
    // 参考结果 2: BF16 截断后的精确值 (模拟 AMX 应该得到的结果)
    // AMX 将 A、B 都以 BF16 读入, 但累加用 FP32
    // ============================================================
    float C_bf16[ROWS][K];
    memset(C_bf16, 0, sizeof(C_bf16));
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < U; j++) {
            float a = bf16_to_fp32(fp32_to_bf16(A_sp[i][j]));
            if (a != 0.0f)
                for (int k = 0; k < K; k++) {
                    float b = bf16_to_fp32(fp32_to_bf16(B[col_union[j] * K + k]));
                    C_bf16[i][k] += a * b;
                }
        }

    // ============================================================
    // AMX 路径 Step 1: Pack A → BF16 tiles
    // A_tiles[t][row][col] = BF16, 16 rows × 32 cols per tile
    // 列并集中 j 位于 tile t = j/32, tile 内偏移 = j%32
    // 超出 U 的位置保持 0 (零填充)
    // ============================================================
    alignas(64) uint16_t A_tiles[NTILES][ROWS][32];  // BF16
    memset(A_tiles, 0, sizeof(A_tiles));
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < U; j++)
            A_tiles[j / 32][i][j % 32] = fp32_to_bf16(A_sp[i][j]);

    printf("[Pack A] %d tiles × (%d × 32) BF16, stride=%d bytes/row\n",
           NTILES, ROWS, STRIDE);

    // ============================================================
    // AMX 路径 Step 2: Gather B → VNNI BF16 tiles
    //
    // VNNI 格式 (AMX TDPBF16PS 要求):
    //   B tile 有 16 "行"(pair), 每 pair 对应原始 B 的 2 行
    //   每个位置存 uint32_t = (B_odd_bf16 << 16) | B_even_bf16
    //
    // 对于 A-tile t, pair p:
    //   even_row = col_union[32t + 2p]
    //   odd_row  = col_union[32t + 2p + 1]
    //   B_vnni[t][p][k] = pack(B[even_row][k], B[odd_row][k])
    // ============================================================
    alignas(64) uint8_t B_vnni[NTILES][16 * STRIDE];
    memset(B_vnni, 0, sizeof(B_vnni));

    for (int t = 0; t < NTILES; t++) {
        for (int p = 0; p < 16; p++) {
            int j_even = t * 32 + p * 2;
            int j_odd  = t * 32 + p * 2 + 1;
            uint32_t* pair_row = (uint32_t*)(B_vnni[t] + p * STRIDE);
            for (int k = 0; k < K; k++) {
                uint16_t be = (j_even < U) ?
                    fp32_to_bf16(B[col_union[j_even] * K + k]) : 0;
                uint16_t bo = (j_odd < U) ?
                    fp32_to_bf16(B[col_union[j_odd]  * K + k]) : 0;
                // VNNI pack: low 16 = even, high 16 = odd
                pair_row[k] = ((uint32_t)bo << 16) | (uint32_t)be;
            }
        }
    }

    printf("[Gather B] %d VNNI tiles × (16 pairs × %d cols), stride=%d\n",
           NTILES, K, STRIDE);

    // ============================================================
    // AMX 路径 Step 3: 配置 tiles + TDPBF16PS
    //
    // Tile 分配:
    //   tmm0: C 累加器  16 rows × K*4 bytes (16×16 FP32)
    //   tmm1: A tile    16 rows × 64 bytes  (16×32 BF16)
    //   tmm2: B tile    16 rows × K*4 bytes (16 pairs × 16 VNNI)
    //
    // TDPBF16PS 语义:
    //   C[m][n] += Σ_{k=0..15} (A[m][2k]·B[k][2n] + A[m][2k+1]·B[k][2n+1])
    //   其中 B[k][2n], B[k][2n+1] 是 VNNI pair 中的 even/odd BF16
    // ============================================================
    tile_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.palette_id = 1;
    cfg.rows[0] = ROWS;   cfg.colsb[0] = K * 4;  // tmm0: C
    cfg.rows[1] = ROWS;   cfg.colsb[1] = 64;      // tmm1: A
    cfg.rows[2] = 16;     cfg.colsb[2] = K * 4;   // tmm2: B

    _tile_loadconfig(&cfg);

    // C = 0
    alignas(64) float C_amx[ROWS][K];
    memset(C_amx, 0, sizeof(C_amx));
    _tile_zero(0);

    // 累加: 遍历每个 A-tile
    printf("[Compute] ");
    for (int t = 0; t < NTILES; t++) {
        _tile_loadd(1, A_tiles[t], STRIDE);   // 加载 A tile
        _tile_loadd(2, B_vnni[t],  STRIDE);   // 加载 B VNNI tile
        _tile_dpbf16ps(0, 1, 2);               // C += A × B
        printf("tile%d ", t);
    }
    printf("→ done\n");

    // 写回 C
    _tile_stored(0, C_amx, K * 4);
    _tile_release();

    printf("[Scatter C] 16 × %d FP32 written\n\n", K);

    // ============================================================
    // 正确性检查
    // ============================================================

    // (1) AMX vs BF16 参考 — 应该几乎完全一致
    printf("--- Check 1: AMX vs BF16 Reference (should match closely) ---\n");
    float max_abs1 = 0, max_rel1 = 0;
    int mismatch1 = 0;
    for (int i = 0; i < ROWS; i++)
        for (int k = 0; k < K; k++) {
            float ref = C_bf16[i][k], amx = C_amx[i][k];
            float ae = fabsf(ref - amx);
            float re = (fabsf(ref) > 1e-6f) ? ae / fabsf(ref) : ae;
            if (ae > max_abs1) max_abs1 = ae;
            if (re > max_rel1) max_rel1 = re;
            if (re > 0.005f && ae > 1e-5f) {
                if (mismatch1 < 5)
                    printf("  [%d][%d] ref=%.6f amx=%.6f abs=%.2e rel=%.2e\n",
                           i, k, ref, amx, ae, re);
                mismatch1++;
            }
        }
    printf("  Max abs err: %.2e   Max rel err: %.2e\n", max_abs1, max_rel1);
    printf("  Mismatches (>0.5%%): %d / %d\n", mismatch1, ROWS * K);

    if (mismatch1 == 0)
        printf("  ✅ PASS\n");
    else
        printf("  ❌ FAIL (%d mismatches)\n", mismatch1);

    // (2) AMX vs FP32 参考 — 量化 BF16 截断的影响
    printf("\n--- Check 2: BF16 Truncation Impact (AMX vs FP32 ref) ---\n");
    float max_abs2 = 0, max_rel2 = 0;
    for (int i = 0; i < ROWS; i++)
        for (int k = 0; k < K; k++) {
            float ae = fabsf(C_ref[i][k] - C_amx[i][k]);
            float re = (fabsf(C_ref[i][k]) > 1e-6f) ?
                       ae / fabsf(C_ref[i][k]) : ae;
            if (ae > max_abs2) max_abs2 = ae;
            if (re > max_rel2) max_rel2 = re;
        }
    printf("  Max abs err: %.2e   Max rel err: %.2e\n", max_abs2, max_rel2);

    // (3) 样本值对比
    printf("\n--- Sample Values (row 0, cols 0-7) ---\n");
    printf("  FP32 ref:");
    for (int k = 0; k < 8; k++) printf(" %8.4f", C_ref[0][k]);
    printf("\n  BF16 ref:");
    for (int k = 0; k < 8; k++) printf(" %8.4f", C_bf16[0][k]);
    printf("\n  AMX out: ");
    for (int k = 0; k < 8; k++) printf(" %8.4f", C_amx[0][k]);
    printf("\n");

    free(B);
    printf("\n=== Step 4.1 Complete ===\n");
    return (mismatch1 == 0) ? 0 : 1;
}
