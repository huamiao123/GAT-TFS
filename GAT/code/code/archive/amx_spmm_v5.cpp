/**
 * Step 4.5: AMX SpMM V5 — 多轮分组, 最大化 AMX 覆盖
 *
 * 核心改动:
 *   1. 多轮分组: Round1 freq>=16选16行, Round2 freq>=2选2~15行
 *      不满16行的 panel 用零行填充, tile config 不变
 *   2. 预处理优化: 简化行选择 (不 partial_sort, 直接取前16)
 *   3. Fallback 改 dynamic 调度
 *
 * Usage: ./amx_spmm_v5 <matrix.csrbin> [nthreads]
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

struct PackedNZ {
    uint8_t  row;
    uint8_t  tile_idx;
    uint16_t tile_col;
    uint16_t bf16_val;
};

struct Panel {
    int rows[TILE_R];    // 不满16行的位置填 -1
    int nrows;           // 实际行数 (2~16)
    int U, ntiles, total_deg;
    float fill;
    std::vector<PackedNZ> packed_nz;
    // B gather: b_rows[t*32 + p*2+{0,1}] = row in B, -1 if invalid
    std::vector<int> b_rows;
};

// ==================== 多轮分组 ====================
void multi_round_grouping(const CSR& csr, const CSC& csc,
    std::vector<Panel>& amx_panels, std::vector<int>& avx_rows)
{
    int M = csr.M, N = csr.N;
    std::vector<int> deg(M);
    for (int i = 0; i < M; i++) deg[i] = csr.indptr[i+1] - csr.indptr[i];

    // 按列频率降序
    std::vector<int> col_order;
    col_order.reserve(N);
    for (int j = 0; j < N; j++)
        if (csc.col_counts[j] >= 2) col_order.push_back(j);  // 门槛降到 2
    std::sort(col_order.begin(), col_order.end(),
        [&](int a, int b){ return csc.col_counts[a] > csc.col_counts[b]; });

    std::vector<bool> assigned(M, false);
    std::vector<int> col_map(N, -1);  // 预处理阶段用

    // 辅助函数: 尝试从一列构建 panel
    auto try_build_panel = [&](int col, int min_rows) -> bool {
        // 收集未分配行
        std::vector<int> avail;
        avail.reserve(csc.col_counts[col]);
        for (int64_t p = csc.col_ptr[col]; p < csc.col_ptr[col+1]; p++) {
            int r = csc.col_rows[p];
            if (!assigned[r]) avail.push_back(r);
        }
        if ((int)avail.size() < min_rows) return false;

        // 选行: 取度数最小的 min(avail, 16) 行
        int nrows = std::min((int)avail.size(), TILE_R);
        std::partial_sort(avail.begin(), avail.begin()+nrows, avail.end(),
            [&](int a, int b){ return deg[a] < deg[b]; });

        // 列并集
        std::vector<int> cols;
        int total_deg = 0;
        for (int i = 0; i < nrows; i++) {
            int r = avail[i];
            total_deg += deg[r];
            for (uint32_t p = csr.indptr[r]; p < csr.indptr[r+1]; p++)
                cols.push_back(csr.indices[p]);
        }
        std::sort(cols.begin(), cols.end());
        cols.erase(std::unique(cols.begin(), cols.end()), cols.end());
        int U = (int)cols.size();
        int ntiles = (U + TILE_C - 1) / TILE_C;

        // 盈亏线: 用实际行数计算 fill (不是 16)
        // 但 AMX 永远用 16 行 tile, 所以真实 fill = total_deg / (16 * 32 * ntiles)
        float real_fill = (float)total_deg / (TILE_R * TILE_C * ntiles);
        if (real_fill < FILL_THR || ntiles > MAX_NTILES) return false;

        // 构建 panel
        for (int j = 0; j < (int)cols.size(); j++) col_map[cols[j]] = j;

        Panel panel;
        panel.nrows = nrows;
        panel.total_deg = total_deg;
        panel.U = U;
        panel.ntiles = ntiles;
        panel.fill = real_fill;

        for (int i = 0; i < TILE_R; i++) panel.rows[i] = -1;
        for (int i = 0; i < nrows; i++) panel.rows[i] = avail[i];

        // PackedNZ
        panel.packed_nz.reserve(total_deg);
        for (int i = 0; i < nrows; i++) {
            int r = avail[i];
            for (uint32_t p = csr.indptr[r]; p < csr.indptr[r+1]; p++) {
                int j = col_map[csr.indices[p]];
                PackedNZ nz;
                nz.row = (uint8_t)i;
                nz.tile_idx = (uint8_t)(j / TILE_C);
                nz.tile_col = (uint16_t)(j % TILE_C);
                nz.bf16_val = f32_to_bf16(csr.values[p]);
                panel.packed_nz.push_back(nz);
            }
        }

        // B gather info (flat array)
        panel.b_rows.assign(ntiles * 32, -1);
        for (int t = 0; t < ntiles; t++) {
            for (int p = 0; p < 16; p++) {
                int je = t*TILE_C + p*2;
                int jo = t*TILE_C + p*2 + 1;
                if (je < U) panel.b_rows[t*32 + p*2]   = cols[je];
                if (jo < U) panel.b_rows[t*32 + p*2+1] = cols[jo];
            }
        }

        // cleanup col_map
        for (int j = 0; j < (int)cols.size(); j++) col_map[cols[j]] = -1;

        // mark assigned
        for (int i = 0; i < nrows; i++) assigned[avail[i]] = true;
        amx_panels.push_back(std::move(panel));
        return true;
    };

    // Round 1: 高频列, 16 行 panel
    int r1_count = 0;
    for (int col : col_order) {
        if (csc.col_counts[col] < TILE_R) break;  // 频率不够, 后面的更少
        if (try_build_panel(col, TILE_R)) r1_count++;
    }

    // Round 2: 中频列, 8~15 行 panel
    int r2_count = 0;
    for (int col : col_order) {
        if (csc.col_counts[col] < 8) break;
        if (try_build_panel(col, 8)) r2_count++;
    }

    // Round 3: 低频列, 4~7 行 panel
    int r3_count = 0;
    for (int col : col_order) {
        if (csc.col_counts[col] < 4) break;
        if (try_build_panel(col, 4)) r3_count++;
    }

    printf("    Round1(16+): %d  Round2(8+): %d  Round3(4+): %d\n",
           r1_count, r2_count, r3_count);

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
    #pragma omp parallel for schedule(dynamic, 256)
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

// ==================== AMX V5 ====================
void amx_spmm_v5(const float* B, float* C,
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

        alignas(64) uint16_t A_tiles[MAX_NTILES][TILE_R][TILE_C];
        alignas(64) uint8_t  B_buf[16 * 64];
        alignas(64) float    C_buf[TILE_R][KC];

        #pragma omp for schedule(dynamic, 8)
        for (int pi = 0; pi < np; pi++) {
            const Panel& panel = panels[pi];
            int ntiles = panel.ntiles;
            int nrows = panel.nrows;

            // Pack A
            memset(A_tiles, 0, (size_t)ntiles * sizeof(A_tiles[0]));
            for (const auto& nz : panel.packed_nz)
                A_tiles[nz.tile_idx][nz.row][nz.tile_col] = nz.bf16_val;

            for (int kc = 0; kc < K; kc += KC) {
                _tile_zero(0);
                for (int t = 0; t < ntiles; t++) {
                    const int* br = &panel.b_rows[t * 32];
                    for (int p = 0; p < 16; p++) {
                        int re = br[p*2], ro = br[p*2+1];
                        uint32_t* pr = (uint32_t*)(B_buf + p*64);
                        if (re >= 0 && ro >= 0) {
                            __m512 ve = _mm512_loadu_ps(&B[(int64_t)re*K+kc]);
                            __m512 vo = _mm512_loadu_ps(&B[(int64_t)ro*K+kc]);
                            __m512i ie = _mm512_srli_epi32(_mm512_castps_si512(ve),16);
                            __m512i io = _mm512_srli_epi32(_mm512_castps_si512(vo),16);
                            _mm512_store_si512((__m512i*)pr,
                                _mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
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
                // 只写有效行
                for (int i = 0; i < nrows; i++) {
                    int row = panel.rows[i];
                    float* dst = C + (int64_t)row * K + kc;
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

    printf("=== AMX SpMM V5 (Multi-round Grouping) ===\n");
    printf("K=%d threads=%d\n\n", K, nt);
    syscall(SYS_arch_prctl, 0x1023, 18);

    CSR csr = read_csrbin(argv[1]);
    printf("[1] M=%d N=%d NNZ=%ld avg=%.1f\n\n", csr.M,csr.N,csr.nnz,(double)csr.nnz/csr.M);

    printf("[2] CSC...\n");
    auto t0=std::chrono::high_resolution_clock::now();
    CSC csc = build_csc(csr);
    auto t1=std::chrono::high_resolution_clock::now();
    printf("    %.0fms\n\n", std::chrono::duration<double,std::milli>(t1-t0).count());

    printf("[3] Multi-round grouping...\n");
    t0=std::chrono::high_resolution_clock::now();
    std::vector<Panel> amx_panels;
    std::vector<int> avx_rows;
    multi_round_grouping(csr, csc, amx_panels, avx_rows);
    t1=std::chrono::high_resolution_clock::now();

    int64_t amx_nnz=0;
    for(auto&p:amx_panels) amx_nnz+=p.total_deg;
    printf("    AMX: %d panels, %ld NNZ (%.1f%%)\n",
        (int)amx_panels.size(), amx_nnz, 100.0*amx_nnz/csr.nnz);
    printf("    AVX: %d rows (%.1f%%), %ld NNZ (%.1f%%)\n",
        (int)avx_rows.size(), 100.0*avx_rows.size()/csr.M,
        csr.nnz-amx_nnz, 100.0*(csr.nnz-amx_nnz)/csr.nnz);
    printf("    Preprocess: %.1fs\n\n",
        std::chrono::duration<double>(t1-t0).count());

    printf("[4] B[%d x %d]\n", csr.N, K);
    float* B=(float*)aligned_alloc(64,(size_t)csr.N*K*sizeof(float));
    srand(12345);
    for(int64_t i=0;i<(int64_t)csr.N*K;i++) B[i]=(rand()%200-100)/100.0f;

    printf("[5] Naive (1T)...\n");
    float* C_ref=(float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));
    t0=std::chrono::high_resolution_clock::now();
    naive_spmm(csr,B,C_ref);
    t1=std::chrono::high_resolution_clock::now();
    double naive_ms=std::chrono::duration<double,std::milli>(t1-t0).count();
    printf("    %.1fms\n\n", naive_ms);

    float* C_amx=(float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));
    // warmup
    memset(C_amx,0,(size_t)csr.M*K*sizeof(float));
    amx_spmm_v5(B,C_amx,amx_panels);
    avx512_fallback(csr,B,C_amx,avx_rows);

    printf("[6] AMX V5 (%dT)...\n", nt);
    double best_amx=1e9, best_fb=1e9;
    for(int trial=0;trial<5;trial++){
        memset(C_amx,0,(size_t)csr.M*K*sizeof(float));
        t0=std::chrono::high_resolution_clock::now();
        amx_spmm_v5(B,C_amx,amx_panels);
        auto tm=std::chrono::high_resolution_clock::now();
        avx512_fallback(csr,B,C_amx,avx_rows);
        t1=std::chrono::high_resolution_clock::now();
        double a=std::chrono::duration<double,std::milli>(tm-t0).count();
        double f=std::chrono::duration<double,std::milli>(t1-tm).count();
        printf("    [%d] AMX=%.1f FB=%.1f T=%.1f\n",trial,a,f,a+f);
        if(a<best_amx)best_amx=a;
        if(f<best_fb)best_fb=f;
    }
    double best=best_amx+best_fb;
    printf("    Best: AMX=%.1f + FB=%.1f = %.1f ms\n", best_amx,best_fb,best);
    printf("    ★ vs Naive(1T): %.2fx\n\n", naive_ms/best);

    // Pure AVX-512 baseline
    printf("[6b] Pure AVX-512 (%dT)...\n", nt);
    float* C_avx=(float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));
    std::vector<int> all(csr.M); std::iota(all.begin(),all.end(),0);
    avx512_fallback(csr,B,C_avx,all); // warmup
    double best_pure=1e9;
    for(int t=0;t<5;t++){
        t0=std::chrono::high_resolution_clock::now();
        avx512_fallback(csr,B,C_avx,all);
        t1=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(t1-t0).count();
        if(ms<best_pure)best_pure=ms;
    }
    printf("    Best: %.1f ms\n", best_pure);
    printf("    ★ AMX V5 vs AVX-512: %.2fx\n\n", best_pure/best);

    // Correctness
    printf("[7] Correctness...\n");
    int bad=0; float max_abs=0;
    for(int64_t i=0;i<(int64_t)csr.M*K;i++){
        float ae=fabsf(C_ref[i]-C_amx[i]);
        float re=(fabsf(C_ref[i])>1e-6f)?ae/fabsf(C_ref[i]):ae;
        if(ae>max_abs)max_abs=ae;
        if(re>0.05f&&fabsf(C_ref[i])>0.01f)bad++;
    }
    printf("    Max abs: %.3e  Bad: %d (%.4f%%)\n", max_abs,bad,100.0*bad/(csr.M*K));
    printf("    %s\n", 100.0*bad/(csr.M*K)<1.0?"✅ OK":"❌ FAIL");

    free(B);free(C_ref);free(C_amx);free(C_avx);
    printf("\n=== Done ===\n");
    return 0;
}
