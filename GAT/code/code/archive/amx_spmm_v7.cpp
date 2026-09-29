/**
 * V7: 多轮分组 + 连续A + BF16 B + 快速预处理
 *
 * 改进:
 *   1. 多轮分组: Round1(freq>=16,16行) Round2(>=8) Round3(>=4)
 *   2. 连续A存储 (V6b验证有效)
 *   3. B存BF16
 *   4. 预处理加速: 不做partial_sort, 直接取前N个未分配行
 *      (partial_sort是as-Skitter 935秒的元凶)
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
    uint8_t palette_id, start_row, reserved0[14];
    uint16_t colsb[16]; uint8_t rows[16];
} tile_config_t;

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
    CSC sc; int N=csr.N;
    sc.col_counts.assign(N,0);
    for(int64_t i=0;i<csr.nnz;i++) sc.col_counts[csr.indices[i]]++;
    sc.col_ptr.assign(N+1,0);
    for(int j=0;j<N;j++) sc.col_ptr[j+1]=sc.col_ptr[j]+sc.col_counts[j];
    sc.col_rows.resize(csr.nnz);
    std::vector<int64_t> pos(N,0);
    for(int i=0;i<csr.M;i++)
        for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
            int c=csr.indices[p];
            sc.col_rows[sc.col_ptr[c]+pos[c]++]=i;
        }
    return sc;
}

struct PackedNZ { uint8_t row, tile_idx; uint16_t tile_col, bf16_val; };

struct Panel {
    int rows[TILE_R];  // 不满16行的填-1
    int nrows, U, ntiles, total_deg;
    int64_t A_offset;
    std::vector<int> b_rows;  // flat [ntiles*32]
};

struct PrecompData {
    std::vector<Panel> panels;
    std::vector<int> avx_rows;
    uint16_t* A_all;
    int64_t total_tiles;
    int64_t amx_nnz;
};

// 尝试从一列构建panel, min_rows=需要的最少行数
// 返回是否成功
bool try_build_panel(int col, int min_rows,
    const CSR& csr, const CSC& csc,
    const std::vector<int>& deg,
    std::vector<bool>& assigned,
    std::vector<int>& col_map,
    // 输出
    Panel& panel, std::vector<PackedNZ>& packed_nz,
    int64_t& tile_offset)
{
    // 收集未分配行
    std::vector<int> avail;
    avail.reserve(std::min((int)csc.col_counts[col], TILE_R * 2));
    for(int64_t p = csc.col_ptr[col]; p < csc.col_ptr[col+1]; p++){
        int r = csc.col_rows[p];
        if(!assigned[r]){
            avail.push_back(r);
            if((int)avail.size() >= TILE_R * 4) break; // 够了就停, 不遍历全部
        }
    }
    if((int)avail.size() < min_rows) return false;

    // ★ 快速行选择: 不排序! 直接取前 min(avail, 16) 行
    // 为什么可以不排序: partial_sort O(N*log16) 在大列上极慢
    // 不排序可能 U 稍大(fill稍低), 但预处理从935s→几秒
    int nrows = std::min((int)avail.size(), TILE_R);

    // 列并集
    std::vector<int> cols;
    int total_deg = 0;
    for(int i = 0; i < nrows; i++){
        int r = avail[i];
        total_deg += deg[r];
        for(uint32_t p = csr.indptr[r]; p < csr.indptr[r+1]; p++)
            cols.push_back(csr.indices[p]);
    }
    std::sort(cols.begin(), cols.end());
    cols.erase(std::unique(cols.begin(), cols.end()), cols.end());
    int U = (int)cols.size();
    int ntiles = (U + TILE_C - 1) / TILE_C;

    // fill 用 16 行计算 (AMX 永远用16行tile)
    float real_fill = (float)total_deg / (TILE_R * TILE_C * ntiles);
    if(real_fill < FILL_THR || ntiles > MAX_NTILES) return false;

    // 构建 panel
    for(int j = 0; j < (int)cols.size(); j++) col_map[cols[j]] = j;

    panel.nrows = nrows;
    panel.U = U;
    panel.ntiles = ntiles;
    panel.total_deg = total_deg;
    panel.A_offset = tile_offset * TILE_R * TILE_C;

    for(int i = 0; i < TILE_R; i++) panel.rows[i] = (i < nrows) ? avail[i] : avail[0]; // 填充行用row[0](写回时只写nrows行)
    // 注意: 填充行指向有效行, 这样scatter不会写到非法地址
    // 但实际值是0(A tile对应位置为0), 所以不影响结果

    packed_nz.clear();
    packed_nz.reserve(total_deg);
    for(int i = 0; i < nrows; i++){
        int r = avail[i];
        for(uint32_t p = csr.indptr[r]; p < csr.indptr[r+1]; p++){
            int j = col_map[csr.indices[p]];
            packed_nz.push_back({(uint8_t)i, (uint8_t)(j/TILE_C),
                (uint16_t)(j%TILE_C), f32_to_bf16(csr.values[p])});
        }
    }

    panel.b_rows.assign(ntiles * 32, -1);
    for(int t = 0; t < ntiles; t++)
        for(int p = 0; p < 16; p++){
            int je = t*TILE_C + p*2;
            int jo = je + 1;
            if(je < U) panel.b_rows[t*32+p*2]   = cols[je];
            if(jo < U) panel.b_rows[t*32+p*2+1] = cols[jo];
        }

    // cleanup
    for(int j = 0; j < (int)cols.size(); j++) col_map[cols[j]] = -1;
    for(int i = 0; i < nrows; i++) assigned[avail[i]] = true;
    tile_offset += ntiles;
    return true;
}

PrecompData build_all(const CSR& csr, const CSC& csc) {
    PrecompData pd;
    int M = csr.M, N = csr.N;
    std::vector<int> deg(M);
    for(int i=0;i<M;i++) deg[i]=csr.indptr[i+1]-csr.indptr[i];

    // 列按频率降序
    std::vector<int> col_order;
    col_order.reserve(N);
    for(int j=0;j<N;j++)
        if(csc.col_counts[j] >= 2) col_order.push_back(j);
    std::sort(col_order.begin(), col_order.end(),
        [&](int a,int b){return csc.col_counts[a]>csc.col_counts[b];});

    std::vector<bool> assigned(M, false);
    std::vector<int> col_map(N, -1);

    // Pass 1: 多轮分组, 收集 panel + packed_nz
    struct PanelBuild {
        Panel panel;
        std::vector<PackedNZ> packed_nz;
    };
    std::vector<PanelBuild> builds;
    int64_t tile_offset = 0;
    int r1=0, r2=0, r3=0;

    // Round 1: freq>=16, 需要16行
    for(int col : col_order){
        if(csc.col_counts[col] < TILE_R) break;
        PanelBuild pb;
        if(try_build_panel(col, TILE_R, csr, csc, deg, assigned, col_map,
                          pb.panel, pb.packed_nz, tile_offset)){
            builds.push_back(std::move(pb));
            r1++;
        }
    }

    // Round 2: freq>=8, 需要8行
    for(int col : col_order){
        if(csc.col_counts[col] < 8) break;
        PanelBuild pb;
        if(try_build_panel(col, 8, csr, csc, deg, assigned, col_map,
                          pb.panel, pb.packed_nz, tile_offset)){
            builds.push_back(std::move(pb));
            r2++;
        }
    }

    // Round 3: freq>=4, 需要4行
    for(int col : col_order){
        if(csc.col_counts[col] < 4) break;
        PanelBuild pb;
        if(try_build_panel(col, 4, csr, csc, deg, assigned, col_map,
                          pb.panel, pb.packed_nz, tile_offset)){
            builds.push_back(std::move(pb));
            r3++;
        }
    }

    printf("    Round1(16+): %d  Round2(8+): %d  Round3(4+): %d\n", r1, r2, r3);

    // Pass 2: 分配连续 A 并填充
    pd.total_tiles = tile_offset;
    int64_t A_size = tile_offset * TILE_R * TILE_C;
    pd.A_all = (uint16_t*)aligned_alloc(64, std::max(A_size, (int64_t)1) * sizeof(uint16_t));
    memset(pd.A_all, 0, A_size * sizeof(uint16_t));

    pd.amx_nnz = 0;
    for(auto& pb : builds){
        uint16_t* base = pd.A_all + pb.panel.A_offset;
        for(const auto& nz : pb.packed_nz)
            base[(int64_t)nz.tile_idx * TILE_R * TILE_C + nz.row * TILE_C + nz.tile_col] = nz.bf16_val;
        pd.amx_nnz += pb.panel.total_deg;
        pd.panels.push_back(std::move(pb.panel));
    }

    for(int i=0;i<M;i++) if(!assigned[i]) pd.avx_rows.push_back(i);

    printf("    Total tiles: %ld (%.1f MB)\n", tile_offset, (double)A_size*2/1e6);
    return pd;
}

// ==================== AVX-512 Fallback ====================
void avx512_fallback(const CSR& csr, const float* B, float* C,
                     const std::vector<int>& rows) {
    int nr=(int)rows.size();
    #pragma omp parallel for schedule(dynamic,256)
    for(int idx=0;idx<nr;idx++){
        int i=rows[idx]; float*Cr=C+(int64_t)i*K;
        __m512 c0=_mm512_setzero_ps(),c1=_mm512_setzero_ps();
        for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
            __m512 a=_mm512_set1_ps(csr.values[p]);
            const float*Br=B+(int64_t)csr.indices[p]*K;
            c0=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br),c0);
            c1=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+16),c1);
        }
        _mm512_storeu_ps(Cr,c0); _mm512_storeu_ps(Cr+16,c1);
    }
}

// ==================== AMX V7 ====================
void amx_v7(const uint16_t* B_bf16, float* C, const PrecompData& pd) {
    int np=(int)pd.panels.size();
    const uint16_t* A_all = pd.A_all;

    #pragma omp parallel
    {
        syscall(SYS_arch_prctl,0x1023,18);
        tile_config_t cfg; memset(&cfg,0,64);
        cfg.palette_id=1;
        cfg.rows[0]=16;cfg.colsb[0]=KC*4;
        cfg.rows[1]=16;cfg.colsb[1]=64;
        cfg.rows[2]=16;cfg.colsb[2]=KC*4;
        _tile_loadconfig(&cfg);

        alignas(64) uint8_t Bb[16*64];
        alignas(64) float Cb[16][KC];

        #pragma omp for schedule(dynamic,8)
        for(int pi=0;pi<np;pi++){
            const Panel& P=pd.panels[pi];
            const uint16_t* preA = A_all + P.A_offset;
            int nrows = P.nrows;

            for(int kc=0;kc<K;kc+=KC){
                _tile_zero(0);
                for(int t=0;t<P.ntiles;t++){
                    const int*br=&P.b_rows[t*32];
                    for(int p=0;p<16;p++){
                        int re=br[p*2],ro=br[p*2+1];
                        uint32_t*pr=(uint32_t*)(Bb+p*64);
                        if(re>=0&&ro>=0){
                            __m256i ve=_mm256_loadu_si256((const __m256i*)&B_bf16[(int64_t)re*K+kc]);
                            __m256i vo=_mm256_loadu_si256((const __m256i*)&B_bf16[(int64_t)ro*K+kc]);
                            __m512i ie=_mm512_cvtepu16_epi32(ve);
                            __m512i io=_mm512_cvtepu16_epi32(vo);
                            _mm512_store_si512((__m512i*)pr,
                                _mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
                        } else if(re>=0){
                            __m256i ve=_mm256_loadu_si256((const __m256i*)&B_bf16[(int64_t)re*K+kc]);
                            _mm512_store_si512((__m512i*)pr,_mm512_cvtepu16_epi32(ve));
                        } else {
                            _mm512_store_si512((__m512i*)pr,_mm512_setzero_si512());
                        }
                    }
                    _tile_loadd(1, preA+(size_t)t*TILE_R*TILE_C, 64);
                    _tile_loadd(2, Bb, 64);
                    _tile_dpbf16ps(0,1,2);
                }
                _tile_stored(0, Cb, KC*4);
                // 只写有效行
                for(int i=0;i<nrows;i++){
                    float*dst=C+(int64_t)P.rows[i]*K+kc;
                    for(int k=0;k<KC;k++) dst[k]=Cb[i][k];
                }
            }
        }
        _tile_release();
    }
}

// ==================== Naive (正确性参考) ====================
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

int main(int argc, char** argv) {
    if(argc<2){printf("Usage: %s <mat> [threads]\n",argv[0]);return 1;}
    int nt=(argc>=3)?atoi(argv[2]):omp_get_num_procs();
    omp_set_num_threads(nt);
    syscall(SYS_arch_prctl,0x1023,18);

    printf("=== AMX V7 (Multi-round + Contiguous A + BF16 B) ===\n");
    printf("K=%d threads=%d\n\n", K, nt);

    auto T0=std::chrono::high_resolution_clock::now();
    CSR csr=read_csrbin(argv[1]);
    auto T1=std::chrono::high_resolution_clock::now();
    printf("[1] M=%d N=%d NNZ=%ld avg=%.1f (%.0fms)\n\n",
        csr.M,csr.N,csr.nnz,(double)csr.nnz/csr.M,
        std::chrono::duration<double,std::milli>(T1-T0).count());

    CSC csc=build_csc(csr);

    printf("[2] Multi-round grouping + precompute...\n");
    T0=std::chrono::high_resolution_clock::now();
    PrecompData pd = build_all(csr, csc);
    T1=std::chrono::high_resolution_clock::now();
    double prep_s = std::chrono::duration<double>(T1-T0).count();
    printf("    AMX: %d panels, %ld NNZ (%.1f%%)\n",
        (int)pd.panels.size(), pd.amx_nnz, 100.0*pd.amx_nnz/csr.nnz);
    printf("    AVX: %d rows (%.1f%%), %ld NNZ (%.1f%%)\n",
        (int)pd.avx_rows.size(), 100.0*pd.avx_rows.size()/csr.M,
        csr.nnz-pd.amx_nnz, 100.0*(csr.nnz-pd.amx_nnz)/csr.nnz);
    printf("    Preprocess: %.2fs\n\n", prep_s);

    // B
    float* B=(float*)aligned_alloc(64,(size_t)csr.N*K*sizeof(float));
    srand(12345);
    for(int64_t i=0;i<(int64_t)csr.N*K;i++) B[i]=(rand()%200-100)/100.0f;
    uint16_t* B_bf16=(uint16_t*)aligned_alloc(64,(size_t)csr.N*K*sizeof(uint16_t));
    for(int64_t i=0;i<(int64_t)csr.N*K;i++) B_bf16[i]=f32_to_bf16(B[i]);

    // Naive
    printf("[3] Naive (1T)...\n");
    float* C_ref=(float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));
    T0=std::chrono::high_resolution_clock::now();
    naive_spmm(csr,B,C_ref);
    T1=std::chrono::high_resolution_clock::now();
    double naive_ms=std::chrono::duration<double,std::milli>(T1-T0).count();
    printf("    %.1fms\n\n", naive_ms);

    float* C=(float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));

    // Pure AVX-512
    printf("[4] Pure AVX-512 (%dT)...\n", nt);
    std::vector<int> all(csr.M); std::iota(all.begin(),all.end(),0);
    avx512_fallback(csr,B,C,all);
    double best_avx=1e9;
    for(int t=0;t<5;t++){
        T0=std::chrono::high_resolution_clock::now();
        avx512_fallback(csr,B,C,all);
        T1=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(T1-T0).count();
        if(ms<best_avx) best_avx=ms;
    }
    printf("    Best: %.2f ms\n\n", best_avx);

    // AMX V7
    printf("[5] AMX V7 (%dT)...\n", nt);
    memset(C,0,(size_t)csr.M*K*sizeof(float));
    amx_v7(B_bf16,C,pd);
    avx512_fallback(csr,B,C,pd.avx_rows); // warmup

    double best_e2e=1e9, best_amx=1e9, best_fb=1e9;
    for(int t=0;t<5;t++){
        memset(C,0,(size_t)csr.M*K*sizeof(float));
        T0=std::chrono::high_resolution_clock::now();
        amx_v7(B_bf16,C,pd);
        auto Tm=std::chrono::high_resolution_clock::now();
        avx512_fallback(csr,B,C,pd.avx_rows);
        T1=std::chrono::high_resolution_clock::now();
        double a=std::chrono::duration<double,std::milli>(Tm-T0).count();
        double f=std::chrono::duration<double,std::milli>(T1-Tm).count();
        printf("    [%d] AMX=%.2f FB=%.2f T=%.2f\n", t, a, f, a+f);
        if(a+f < best_e2e){ best_e2e=a+f; best_amx=a; best_fb=f; }
    }
    printf("    Best: AMX=%.2f + FB=%.2f = %.2f ms\n", best_amx, best_fb, best_e2e);
    printf("    ★ vs Naive(1T): %.2fx\n", naive_ms/best_e2e);
    printf("    ★ vs AVX-512(%dT): %.2fx\n\n", nt, best_avx/best_e2e);

    // Correctness
    printf("[6] Correctness...\n");
    memset(C,0,(size_t)csr.M*K*sizeof(float));
    amx_v7(B_bf16,C,pd);
    avx512_fallback(csr,B,C,pd.avx_rows);
    int bad=0; float max_abs=0;
    for(int64_t i=0;i<(int64_t)csr.M*K;i++){
        float ae=fabsf(C_ref[i]-C[i]);
        if(ae>max_abs) max_abs=ae;
        float re=(fabsf(C_ref[i])>1e-6f)?ae/fabsf(C_ref[i]):ae;
        if(re>0.05f&&fabsf(C_ref[i])>0.01f) bad++;
    }
    printf("    Max abs: %.3e  Bad: %d (%.4f%%)\n", max_abs, bad, 100.0*bad/(csr.M*K));
    printf("    %s\n", 100.0*bad/(csr.M*K)<1.0?"✅ OK":"❌ FAIL");

    printf("\n============ SUMMARY ============\n");
    printf("Naive (1T):         %.1f ms\n", naive_ms);
    printf("Pure AVX-512 (%dT): %.2f ms\n", nt, best_avx);
    printf("AMX V7 (%dT):       %.2f ms (%.2fx AVX)\n", nt, best_e2e, best_avx/best_e2e);

    free(B); free(B_bf16); free(C); free(C_ref); free(pd.A_all);
    printf("\n=== Done ===\n");
    return 0;
}
