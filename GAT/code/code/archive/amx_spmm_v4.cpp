/**
 * Step 4.4: AMX SpMM V4 — 消灭 col_map
 *
 * 核心优化:
 *   1. 预计算 PackedNZ: 分组时记录每个非零的 (row, tile内位置, BF16值)
 *      → pack A 变为线性写入, 无随机访问, 无 col_map
 *   2. 预计算 B 行指针: 避免运行时 cu[je]*K 乘法
 *   3. Fallback 用 static 调度 + 更大 chunk
 *
 * Usage: ./amx_spmm_v4 <matrix.csrbin> [nthreads]
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
static_assert(sizeof(tile_config_t) == 64, "");

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
    CSR c; FILE* f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "ERR: %s\n", path); exit(1); }
    uint32_t hdr[3]; uint64_t dims[3];
    fread(hdr,4,3,f); fread(dims,8,3,f);
    c.M=(int)dims[0]; c.N=(int)dims[1]; c.nnz=(int64_t)dims[2];
    c.indptr.resize(c.M+1); c.indices.resize(c.nnz); c.values.resize(c.nnz);
    fread(c.indptr.data(),4,c.M+1,f);
    fread(c.indices.data(),4,c.nnz,f);
    fread(c.values.data(),4,c.nnz,f);
    fclose(f); return c;
}
struct CSC {
    std::vector<int64_t> col_ptr;
    std::vector<int> col_rows, col_counts;
};
CSC build_csc(const CSR& csr) {
    CSC sc; int N=csr.N; int64_t nnz=csr.nnz;
    sc.col_counts.assign(N,0);
    for(int64_t i=0;i<nnz;i++) sc.col_counts[csr.indices[i]]++;
    sc.col_ptr.assign(N+1,0);
    for(int j=0;j<N;j++) sc.col_ptr[j+1]=sc.col_ptr[j]+sc.col_counts[j];
    sc.col_rows.resize(nnz);
    std::vector<int64_t> pos(N,0);
    for(int i=0;i<csr.M;i++)
        for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
            int c=csr.indices[p];
            sc.col_rows[sc.col_ptr[c]+pos[c]++]=i;
        }
    return sc;
}

// ==================== 预计算数据结构 ====================
// 每个非零在 tile 内的坐标 + BF16 值
struct PackedNZ {
    uint8_t  row;       // 0..15: panel 内行号
    uint8_t  tile_idx;  // 哪个 A-tile
    uint16_t tile_col;  // tile 内列号 (0..31)
    uint16_t bf16_val;  // 预转 BF16
};

// B gather 需要的行指针: 按 (tile, pair) 排列
struct BGatherInfo {
    int ntiles;
    // b_rows[t][p*2+0] = even row in B, b_rows[t][p*2+1] = odd row
    // -1 表示该位置无效 (超出 U)
    std::vector<std::vector<int>> b_rows; // [ntiles][32]
};

struct Panel {
    int rows[TILE_R];
    int U, ntiles, total_deg;
    float fill;
    std::vector<PackedNZ> packed_nz;
    BGatherInfo bg;
};

// ==================== 分组 + 预计算 ====================
void column_anchor_precompute(const CSR& csr, const CSC& csc,
    std::vector<Panel>& amx_panels, std::vector<int>& avx_rows)
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

    // 单个临时 col_map, 只在预处理阶段用一次
    std::vector<int> col_map(N, -1);

    for (int col : col_order) {
        std::vector<int> avail;
        for (int64_t p = csc.col_ptr[col]; p < csc.col_ptr[col+1]; p++) {
            int r = csc.col_rows[p];
            if (!assigned[r]) avail.push_back(r);
        }
        if ((int)avail.size() < TILE_R) continue;

        std::partial_sort(avail.begin(), avail.begin()+TILE_R, avail.end(),
            [&](int a, int b){ return deg[a] < deg[b]; });

        // 构建列并集
        std::vector<int> cols;
        int total_deg = 0;
        for (int i = 0; i < TILE_R; i++) {
            int r = avail[i];
            total_deg += deg[r];
            for (uint32_t p = csr.indptr[r]; p < csr.indptr[r+1]; p++)
                cols.push_back(csr.indices[p]);
        }
        std::sort(cols.begin(), cols.end());
        cols.erase(std::unique(cols.begin(), cols.end()), cols.end());
        int U = (int)cols.size();
        int ntiles = (U + TILE_C - 1) / TILE_C;
        float fill = (float)total_deg / (TILE_R * TILE_C * ntiles);

        if (fill < FILL_THR || ntiles > MAX_NTILES) continue;

        // --- 预计算: 设置 col_map ---
        for (int j = 0; j < U; j++) col_map[cols[j]] = j;

        Panel panel;
        for (int i = 0; i < TILE_R; i++) panel.rows[i] = avail[i];
        panel.U = U;
        panel.ntiles = ntiles;
        panel.total_deg = total_deg;
        panel.fill = fill;

        // --- 预计算 PackedNZ ---
        panel.packed_nz.reserve(total_deg);
        for (int i = 0; i < TILE_R; i++) {
            int r = avail[i];
            for (uint32_t p = csr.indptr[r]; p < csr.indptr[r+1]; p++) {
                int j = col_map[csr.indices[p]];
                // j should always be valid since col_union covers all NNZ cols
                PackedNZ nz;
                nz.row      = (uint8_t)i;
                nz.tile_idx = (uint8_t)(j / TILE_C);
                nz.tile_col = (uint16_t)(j % TILE_C);
                nz.bf16_val = f32_to_bf16(csr.values[p]);
                panel.packed_nz.push_back(nz);
            }
        }

        // --- 预计算 BGatherInfo ---
        panel.bg.ntiles = ntiles;
        panel.bg.b_rows.resize(ntiles);
        for (int t = 0; t < ntiles; t++) {
            panel.bg.b_rows[t].assign(32, -1);
            for (int p = 0; p < 16; p++) {
                int je = t * TILE_C + p * 2;
                int jo = t * TILE_C + p * 2 + 1;
                if (je < U) panel.bg.b_rows[t][p*2]   = cols[je];
                if (jo < U) panel.bg.b_rows[t][p*2+1]  = cols[jo];
            }
        }

        // 清除 col_map
        for (int j = 0; j < U; j++) col_map[cols[j]] = -1;

        for (int i = 0; i < TILE_R; i++) assigned[avail[i]] = true;
        amx_panels.push_back(std::move(panel));
    }

    for (int i = 0; i < M; i++)
        if (!assigned[i]) avx_rows.push_back(i);
}

// ==================== Naive ====================
void naive_spmm(const CSR& csr, const float* B, float* C) {
    memset(C,0,(size_t)csr.M*K*sizeof(float));
    for(int i=0;i<csr.M;i++)
        for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
            float v=csr.values[p];
            const float*Br=B+(int64_t)csr.indices[p]*K;
            float*Cr=C+(int64_t)i*K;
            for(int k=0;k<K;k++) Cr[k]+=v*Br[k];
        }
}

// ==================== AVX-512 Fallback ====================
void avx512_fallback(const CSR& csr, const float* B, float* C,
                     const std::vector<int>& rows) {
    int nr = (int)rows.size();
    #pragma omp parallel for schedule(static)
    for (int idx = 0; idx < nr; idx++) {
        int i = rows[idx];
        float* Cr = C + (int64_t)i * K;
        __m512 c0 = _mm512_setzero_ps(), c1 = _mm512_setzero_ps();
        for (uint32_t p = csr.indptr[i]; p < csr.indptr[i+1]; p++) {
            __m512 a = _mm512_set1_ps(csr.values[p]);
            const float* Br = B + (int64_t)csr.indices[p] * K;
            c0 = _mm512_fmadd_ps(a, _mm512_loadu_ps(Br), c0);
            c1 = _mm512_fmadd_ps(a, _mm512_loadu_ps(Br+16), c1);
        }
        _mm512_storeu_ps(Cr, c0);
        _mm512_storeu_ps(Cr+16, c1);
    }
}

// ==================== AMX SpMM V4 ====================
void amx_spmm_v4(const CSR& csr, const float* B, float* C,
                  const std::vector<Panel>& panels)
{
    int np = (int)panels.size();

    #pragma omp parallel
    {
        syscall(SYS_arch_prctl, 0x1023, 18);
        tile_config_t cfg;
        memset(&cfg, 0, sizeof(cfg));
        cfg.palette_id = 1;
        cfg.rows[0]=16; cfg.colsb[0]=KC*4;
        cfg.rows[1]=16; cfg.colsb[1]=64;
        cfg.rows[2]=16; cfg.colsb[2]=KC*4;
        _tile_loadconfig(&cfg);

        // 每线程只需 A_tiles + B_buf + C_buf, 不需要 col_map!
        alignas(64) uint16_t A_tiles[MAX_NTILES][TILE_R][TILE_C];
        alignas(64) uint8_t  B_buf[16 * 64];
        alignas(64) float    C_buf[TILE_R][KC];

        #pragma omp for schedule(dynamic, 8)
        for (int pi = 0; pi < np; pi++) {
            const Panel& panel = panels[pi];
            int ntiles = panel.ntiles;

            // === Pack A: 纯线性扫描, 零随机访问 ===
            memset(A_tiles, 0, (size_t)ntiles * sizeof(A_tiles[0]));
            for (const auto& nz : panel.packed_nz) {
                A_tiles[nz.tile_idx][nz.row][nz.tile_col] = nz.bf16_val;
            }

            // === K-chunks ===
            for (int kc = 0; kc < K; kc += KC) {
                _tile_zero(0);
                for (int t = 0; t < ntiles; t++) {
                    const auto& br = panel.bg.b_rows[t];

                    // Gather B → VNNI
                    for (int p = 0; p < 16; p++) {
                        int re = br[p*2], ro = br[p*2+1];
                        uint32_t* pr = (uint32_t*)(B_buf + p * 64);
                        if (re >= 0 && ro >= 0) {
                            __m512 ve = _mm512_loadu_ps(&B[(int64_t)re*K+kc]);
                            __m512 vo = _mm512_loadu_ps(&B[(int64_t)ro*K+kc]);
                            __m512i ie = _mm512_srli_epi32(_mm512_castps_si512(ve),16);
                            __m512i io = _mm512_srli_epi32(_mm512_castps_si512(vo),16);
                            _mm512_store_si512((__m512i*)pr,
                                _mm512_or_si512(ie, _mm512_slli_epi32(io,16)));
                        } else if (re >= 0) {
                            __m512 ve = _mm512_loadu_ps(&B[(int64_t)re*K+kc]);
                            __m512i ie = _mm512_srli_epi32(_mm512_castps_si512(ve),16);
                            _mm512_store_si512((__m512i*)pr, ie);
                        } else {
                            _mm512_store_si512((__m512i*)pr, _mm512_setzero_si512());
                        }
                    }

                    _tile_loadd(1, A_tiles[t], 64);
                    _tile_loadd(2, B_buf, 64);
                    _tile_dpbf16ps(0, 1, 2);
                }

                _tile_stored(0, C_buf, KC*4);
                for (int i = 0; i < TILE_R; i++) {
                    float* dst = C + (int64_t)panel.rows[i] * K + kc;
                    for (int k = 0; k < KC; k++) dst[k] = C_buf[i][k];
                }
            }
        }
        _tile_release();
    }
}

// ==================== Main ====================
int main(int argc, char** argv) {
    if (argc < 2) { printf("Usage: %s <mat> [threads]\n",argv[0]); return 1; }
    int nt = (argc>=3) ? atoi(argv[2]) : omp_get_num_procs();
    omp_set_num_threads(nt);

    printf("=== AMX SpMM V4 (No col_map) ===\n");
    printf("K=%d threads=%d\n\n", K, nt);
    syscall(SYS_arch_prctl, 0x1023, 18);

    printf("[1] Read: %s\n", argv[1]);
    auto t0=std::chrono::high_resolution_clock::now();
    CSR csr = read_csrbin(argv[1]);
    auto t1=std::chrono::high_resolution_clock::now();
    printf("    M=%d N=%d NNZ=%ld avg=%.1f (%.0fms)\n\n",
        csr.M,csr.N,csr.nnz,(double)csr.nnz/csr.M,
        std::chrono::duration<double,std::milli>(t1-t0).count());

    printf("[2] CSC...\n");
    t0=std::chrono::high_resolution_clock::now();
    CSC csc = build_csc(csr);
    t1=std::chrono::high_resolution_clock::now();
    printf("    %.0fms\n\n", std::chrono::duration<double,std::milli>(t1-t0).count());

    printf("[3] Grouping + Precompute...\n");
    t0=std::chrono::high_resolution_clock::now();
    std::vector<Panel> amx_panels;
    std::vector<int> avx_rows;
    column_anchor_precompute(csr, csc, amx_panels, avx_rows);
    t1=std::chrono::high_resolution_clock::now();

    int64_t amx_nnz=0, precomp_bytes=0;
    for(auto&p:amx_panels){
        amx_nnz+=p.total_deg;
        precomp_bytes += p.packed_nz.size()*sizeof(PackedNZ);
        for(auto&v:p.bg.b_rows) precomp_bytes += v.size()*4;
    }
    printf("    AMX: %d panels, %ld NNZ (%.1f%%)\n",
        (int)amx_panels.size(), amx_nnz, 100.0*amx_nnz/csr.nnz);
    printf("    AVX: %d rows, %ld NNZ (%.1f%%)\n",
        (int)avx_rows.size(), csr.nnz-amx_nnz, 100.0*(csr.nnz-amx_nnz)/csr.nnz);
    printf("    Precomputed: %.1f MB\n", precomp_bytes/1e6);

    if(!amx_panels.empty()){
        std::vector<float> fills;
        for(auto&p:amx_panels) fills.push_back(p.fill);
        std::sort(fills.begin(),fills.end());
        int n=fills.size();
        printf("    Fill: mean=%.1f%% med=%.1f%%\n",
            std::accumulate(fills.begin(),fills.end(),0.0f)/n*100, fills[n/2]*100);
    }
    printf("    %.0fms\n\n", std::chrono::duration<double,std::milli>(t1-t0).count());

    printf("[4] B[%d x %d]\n", csr.N, K);
    float* B=(float*)aligned_alloc(64,(size_t)csr.N*K*sizeof(float));
    srand(12345);
    for(int64_t i=0;i<(int64_t)csr.N*K;i++) B[i]=(rand()%200-100)/100.0f;

    printf("[5] Naive FP32 (1T)...\n");
    float* C_ref=(float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));
    t0=std::chrono::high_resolution_clock::now();
    naive_spmm(csr,B,C_ref);
    t1=std::chrono::high_resolution_clock::now();
    double naive_ms=std::chrono::duration<double,std::milli>(t1-t0).count();
    printf("    %.1fms\n\n", naive_ms);

    float* C_amx=(float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));

    // warmup
    memset(C_amx,0,(size_t)csr.M*K*sizeof(float));
    amx_spmm_v4(csr,B,C_amx,amx_panels);
    avx512_fallback(csr,B,C_amx,avx_rows);

    printf("[6] AMX V4 (%dT)...\n", nt);
    double best_amx=1e9, best_avx=1e9;
    for(int trial=0;trial<5;trial++){
        memset(C_amx,0,(size_t)csr.M*K*sizeof(float));
        t0=std::chrono::high_resolution_clock::now();
        amx_spmm_v4(csr,B,C_amx,amx_panels);
        auto tm=std::chrono::high_resolution_clock::now();
        avx512_fallback(csr,B,C_amx,avx_rows);
        t1=std::chrono::high_resolution_clock::now();
        double a=std::chrono::duration<double,std::milli>(tm-t0).count();
        double f=std::chrono::duration<double,std::milli>(t1-tm).count();
        printf("    [%d] AMX=%.1f FB=%.1f T=%.1f ms\n",trial,a,f,a+f);
        if(a<best_amx)best_amx=a;
        if(f<best_avx)best_avx=f;
    }
    double best=best_amx+best_avx;
    printf("    Best: AMX=%.1f + FB=%.1f = %.1f ms\n", best_amx,best_avx,best);
    printf("    ★ vs Naive(1T): %.2fx\n\n", naive_ms/best);

    // Standalone AVX-512 baseline (same threads)
    printf("[6b] Pure AVX-512 (%dT)...\n", nt);
    float* C_avx=(float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));
    // warmup
    avx512_fallback(csr,B,C_avx,avx_rows); // reuse for warmup
    // full run: process ALL rows with AVX-512
    std::vector<int> all_rows(csr.M);
    std::iota(all_rows.begin(), all_rows.end(), 0);
    // warmup
    avx512_fallback(csr,B,C_avx,all_rows);
    double best_pure=1e9;
    for(int trial=0;trial<5;trial++){
        t0=std::chrono::high_resolution_clock::now();
        avx512_fallback(csr,B,C_avx,all_rows);
        t1=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(t1-t0).count();
        if(ms<best_pure)best_pure=ms;
    }
    printf("    Best: %.1f ms\n", best_pure);
    printf("    ★ AMX V4 vs Pure AVX-512: %.2fx\n\n", best_pure/best);

    // Correctness
    printf("[7] Correctness...\n");
    int bad=0;
    float max_abs=0;
    for(int64_t i=0;i<(int64_t)csr.M*K;i++){
        float ae=fabsf(C_ref[i]-C_amx[i]);
        float re=(fabsf(C_ref[i])>1e-6f)?ae/fabsf(C_ref[i]):ae;
        if(ae>max_abs)max_abs=ae;
        if(re>0.05f&&fabsf(C_ref[i])>0.01f) bad++;
    }
    printf("    Max abs: %.3e  Bad: %d (%.4f%%)\n", max_abs,bad,100.0*bad/(csr.M*K));
    printf("    %s\n", 100.0*bad/(csr.M*K)<1.0 ? "✅ OK" : "❌ FAIL");

    free(B);free(C_ref);free(C_amx);free(C_avx);
    printf("\n=== Done ===\n");
    return 0;
}
