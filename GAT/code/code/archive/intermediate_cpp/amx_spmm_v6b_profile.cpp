/**
 * V6b: precomp_A 连续存储 + 精确 profiling
 *
 * 优化1: 所有 panel 的 A tiles 存储在一个连续大数组中
 *         → _tile_loadd 流式访问, 极大减少 cache miss
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
constexpr int TILE_BYTES = TILE_R * TILE_C * 2; // 1024 bytes per tile

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
    int rows[TILE_R];
    int nrows, U, ntiles, total_deg;
    int64_t A_offset;  // ★ offset into global precomp_A array (in uint16_t units)
    std::vector<PackedNZ> packed_nz;
    std::vector<int> b_rows;  // flat: [ntiles * 32]
};

// ★ 全局连续 A 存储
struct PrecompData {
    std::vector<Panel> panels;
    std::vector<int> avx_rows;
    uint16_t* A_all;       // aligned, contiguous: [total_tiles * 16 * 32]
    int64_t total_tiles;
    int64_t amx_nnz;
};

PrecompData build_all(const CSR& csr, const CSC& csc) {
    PrecompData pd;
    int M=csr.M, N=csr.N;
    std::vector<int> deg(M);
    for(int i=0;i<M;i++) deg[i]=csr.indptr[i+1]-csr.indptr[i];
    std::vector<int> col_order;
    for(int j=0;j<N;j++) if(csc.col_counts[j]>=TILE_R) col_order.push_back(j);
    std::sort(col_order.begin(),col_order.end(),
        [&](int a,int b){return csc.col_counts[a]>csc.col_counts[b];});
    std::vector<bool> assigned(M,false);
    std::vector<int> col_map(N,-1);

    // Pass 1: build panels, record packed_nz
    int64_t total_tiles = 0;
    for(int col:col_order){
        std::vector<int> avail;
        for(int64_t p=csc.col_ptr[col];p<csc.col_ptr[col+1];p++){
            int r=csc.col_rows[p]; if(!assigned[r]) avail.push_back(r);
        }
        if((int)avail.size()<TILE_R) continue;
        std::partial_sort(avail.begin(),avail.begin()+TILE_R,avail.end(),
            [&](int a,int b){return deg[a]<deg[b];});
        std::vector<int> cols; int td=0;
        for(int i=0;i<TILE_R;i++){
            int r=avail[i]; td+=deg[r];
            for(uint32_t p=csr.indptr[r];p<csr.indptr[r+1];p++)
                cols.push_back(csr.indices[p]);
        }
        std::sort(cols.begin(),cols.end());
        cols.erase(std::unique(cols.begin(),cols.end()),cols.end());
        int U=(int)cols.size(), nt=(U+TILE_C-1)/TILE_C;
        float fill=(float)td/(TILE_R*TILE_C*nt);
        if(fill<FILL_THR||nt>MAX_NTILES) continue;

        for(int j=0;j<U;j++) col_map[cols[j]]=j;
        Panel panel;
        panel.nrows=TILE_R; panel.U=U; panel.ntiles=nt; panel.total_deg=td;
        panel.A_offset = total_tiles * TILE_R * TILE_C;  // ★
        total_tiles += nt;

        for(int i=0;i<TILE_R;i++) panel.rows[i]=avail[i];
        panel.packed_nz.reserve(td);
        for(int i=0;i<TILE_R;i++){
            int r=avail[i];
            for(uint32_t p=csr.indptr[r];p<csr.indptr[r+1];p++){
                int j=col_map[csr.indices[p]];
                panel.packed_nz.push_back({(uint8_t)i,(uint8_t)(j/TILE_C),
                    (uint16_t)(j%TILE_C),f32_to_bf16(csr.values[p])});
            }
        }
        panel.b_rows.assign(nt*32,-1);
        for(int t=0;t<nt;t++) for(int p=0;p<16;p++){
            int je=t*32+p*2, jo=je+1;
            if(je<U) panel.b_rows[t*32+p*2]=cols[je];
            if(jo<U) panel.b_rows[t*32+p*2+1]=cols[jo];
        }
        for(int j=0;j<U;j++) col_map[cols[j]]=-1;
        for(int i=0;i<TILE_R;i++) assigned[avail[i]]=true;
        pd.panels.push_back(std::move(panel));
    }
    for(int i=0;i<M;i++) if(!assigned[i]) pd.avx_rows.push_back(i);

    // Pass 2: allocate contiguous A and fill
    pd.total_tiles = total_tiles;
    int64_t A_size = total_tiles * TILE_R * TILE_C;
    pd.A_all = (uint16_t*)aligned_alloc(64, A_size * sizeof(uint16_t));
    memset(pd.A_all, 0, A_size * sizeof(uint16_t));

    for(const auto& P : pd.panels){
        uint16_t* base = pd.A_all + P.A_offset;
        for(const auto& nz : P.packed_nz){
            base[(int64_t)nz.tile_idx * TILE_R * TILE_C
                 + nz.row * TILE_C + nz.tile_col] = nz.bf16_val;
        }
    }

    pd.amx_nnz = 0;
    for(auto& p : pd.panels) pd.amx_nnz += p.total_deg;
    printf("    Total tiles: %ld (%.1f MB contiguous A)\n",
        total_tiles, (double)A_size*2/1e6);
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

// ==================== AMX V6b: B=BF16, A连续 ====================
struct ProfileResult {
    double gather_ms, compute_ms, scatter_ms;
};

ProfileResult amx_v6b(const uint16_t* B_bf16, float* C,
    const PrecompData& pd)
{
    int np=(int)pd.panels.size();
    const uint16_t* A_all = pd.A_all;
    int max_threads = omp_get_max_threads();
    std::vector<double> tg(max_threads,0), tc(max_threads,0), ts(max_threads,0);

    #pragma omp parallel
    {
        syscall(SYS_arch_prctl,0x1023,18);
        tile_config_t cfg; memset(&cfg,0,64);
        cfg.palette_id=1;
        cfg.rows[0]=16;cfg.colsb[0]=KC*4;
        cfg.rows[1]=16;cfg.colsb[1]=64;
        cfg.rows[2]=16;cfg.colsb[2]=KC*4;
        _tile_loadconfig(&cfg);

        int tid=omp_get_thread_num();
        alignas(64) uint8_t Bb[16*64];
        alignas(64) float Cb[16][KC];

        #pragma omp for schedule(dynamic,8)
        for(int pi=0;pi<np;pi++){
            const Panel& P=pd.panels[pi];
            const uint16_t* preA = A_all + P.A_offset;  // ★ 连续内存

            for(int kc=0;kc<K;kc+=KC){
                // --- Gather B ---
                double g0=omp_get_wtime();
                for(int t=0;t<P.ntiles;t++){
                    const int*br=&P.b_rows[t*32];
                    for(int p=0;p<16;p++){
                        int re=br[p*2],ro=br[p*2+1];
                        uint32_t*pr=(uint32_t*)(Bb+p*64);
                        if(re>=0&&ro>=0){
                            const uint16_t* be=&B_bf16[(int64_t)re*K+kc];
                            const uint16_t* bo=&B_bf16[(int64_t)ro*K+kc];
                            __m256i ve=_mm256_loadu_si256((const __m256i*)be);
                            __m256i vo=_mm256_loadu_si256((const __m256i*)bo);
                            __m512i ie=_mm512_cvtepu16_epi32(ve);
                            __m512i io=_mm512_cvtepu16_epi32(vo);
                            _mm512_store_si512((__m512i*)pr,
                                _mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
                        } else if(re>=0){
                            const uint16_t* be=&B_bf16[(int64_t)re*K+kc];
                            __m256i ve=_mm256_loadu_si256((const __m256i*)be);
                            __m512i ie=_mm512_cvtepu16_epi32(ve);
                            _mm512_store_si512((__m512i*)pr,ie);
                        } else {
                            _mm512_store_si512((__m512i*)pr,_mm512_setzero_si512());
                        }
                    }
                }
                double g1=omp_get_wtime();
                tg[tid]+=g1-g0;

                // --- Compute ---
                double c0t=omp_get_wtime();
                _tile_zero(0);
                for(int t=0;t<P.ntiles;t++){
                    _tile_loadd(1, preA + (size_t)t*TILE_R*TILE_C, 64);
                    _tile_loadd(2, Bb, 64);
                    _tile_dpbf16ps(0,1,2);
                }
                double c1t=omp_get_wtime();
                tc[tid]+=c1t-c0t;

                // --- Scatter ---
                double s0=omp_get_wtime();
                _tile_stored(0, Cb, KC*4);
                for(int i=0;i<TILE_R;i++){
                    float*dst=C+(int64_t)P.rows[i]*K+kc;
                    for(int k=0;k<KC;k++) dst[k]=Cb[i][k];
                }
                double s1=omp_get_wtime();
                ts[tid]+=s1-s0;
            }
        }
        _tile_release();
    }

    double mg=0,mc=0,ms=0;
    for(int i=0;i<max_threads;i++){
        mg=std::max(mg,tg[i]); mc=std::max(mc,tc[i]); ms=std::max(ms,ts[i]);
    }
    return {mg*1000, mc*1000, ms*1000};
}

// ==================== AMX V6b 不计时版 (端到端计时用) ====================
void amx_v6b_noprofile(const uint16_t* B_bf16, float* C,
    const PrecompData& pd)
{
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

            for(int kc=0;kc<K;kc+=KC){
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
                    _tile_loadd(1, preA + (size_t)t*TILE_R*TILE_C, 64);
                    _tile_loadd(2, Bb, 64);
                    _tile_dpbf16ps(0,1,2);
                }
                _tile_stored(0, Cb, KC*4);
                for(int i=0;i<TILE_R;i++){
                    float*dst=C+(int64_t)P.rows[i]*K+kc;
                    for(int k=0;k<KC;k++) dst[k]=Cb[i][k];
                }
            }
        }
        _tile_release();
    }
}

int main(int argc, char** argv) {
    if(argc<2){printf("Usage: %s <mat> [threads]\n",argv[0]);return 1;}
    int nt=(argc>=3)?atoi(argv[2]):omp_get_num_procs();
    omp_set_num_threads(nt);
    syscall(SYS_arch_prctl,0x1023,18);

    printf("=== AMX V6b Profiling (Contiguous A) ===\n");
    printf("K=%d threads=%d\n\n", K, nt);

    CSR csr=read_csrbin(argv[1]);
    printf("[1] M=%d N=%d NNZ=%ld avg=%.1f\n", csr.M,csr.N,csr.nnz,(double)csr.nnz/csr.M);
    CSC csc=build_csc(csr);

    printf("[2] Grouping + Precompute...\n");
    auto t0=std::chrono::high_resolution_clock::now();
    PrecompData pd = build_all(csr, csc);
    auto t1=std::chrono::high_resolution_clock::now();
    printf("    Panels=%d AMX_NNZ=%.1f%% AVX_rows=%d Prep=%.1fs\n\n",
        (int)pd.panels.size(),100.0*pd.amx_nnz/csr.nnz,
        (int)pd.avx_rows.size(),
        std::chrono::duration<double>(t1-t0).count());

    // B
    float* B=(float*)aligned_alloc(64,(size_t)csr.N*K*sizeof(float));
    srand(12345);
    for(int64_t i=0;i<(int64_t)csr.N*K;i++) B[i]=(rand()%200-100)/100.0f;
    uint16_t* B_bf16=(uint16_t*)aligned_alloc(64,(size_t)csr.N*K*sizeof(uint16_t));
    for(int64_t i=0;i<(int64_t)csr.N*K;i++) B_bf16[i]=f32_to_bf16(B[i]);

    float* C=(float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));

    // ===== Pure AVX-512 =====
    printf("=== Pure AVX-512 (%dT) ===\n", nt);
    std::vector<int> all(csr.M); std::iota(all.begin(),all.end(),0);
    avx512_fallback(csr,B,C,all);
    double best_avx=1e9;
    for(int t=0;t<5;t++){
        t0=std::chrono::high_resolution_clock::now();
        avx512_fallback(csr,B,C,all);
        t1=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(t1-t0).count();
        if(ms<best_avx) best_avx=ms;
    }
    printf("  Best: %.2f ms\n\n", best_avx);

    // ===== Fallback only =====
    printf("=== Fallback Only (%dT) ===\n", nt);
    avx512_fallback(csr,B,C,pd.avx_rows);
    double best_fb=1e9;
    for(int t=0;t<5;t++){
        t0=std::chrono::high_resolution_clock::now();
        avx512_fallback(csr,B,C,pd.avx_rows);
        t1=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(t1-t0).count();
        if(ms<best_fb) best_fb=ms;
    }
    int64_t fb_nnz=csr.nnz-pd.amx_nnz;
    printf("  Best: %.2f ms (%d rows, %ld NNZ)\n\n", best_fb,
        (int)pd.avx_rows.size(), fb_nnz);

    // ===== AMX V6b: Profile版 =====
    printf("=== AMX V6b Profile (BF16, Contiguous A) ===\n");
    memset(C,0,(size_t)csr.M*K*sizeof(float));
    amx_v6b(B_bf16,C,pd); // warmup

    ProfileResult best_r = {1e9,1e9,1e9};
    for(int t=0;t<5;t++){
        memset(C,0,(size_t)csr.M*K*sizeof(float));
        auto r = amx_v6b(B_bf16,C,pd);
        printf("  [%d] gather=%.2f compute=%.2f scatter=%.2f\n",
            t, r.gather_ms, r.compute_ms, r.scatter_ms);
        double total = r.gather_ms+r.compute_ms+r.scatter_ms;
        double best_total = best_r.gather_ms+best_r.compute_ms+best_r.scatter_ms;
        if(total < best_total) best_r = r;
    }
    double amx_part = best_r.gather_ms+best_r.compute_ms+best_r.scatter_ms;
    printf("  Best: gather=%.2f + compute=%.2f + scatter=%.2f = %.2f ms\n",
        best_r.gather_ms, best_r.compute_ms, best_r.scatter_ms, amx_part);

    // ===== AMX V6b: 端到端 (无 omp_get_wtime 开销) =====
    printf("\n=== AMX V6b End-to-End (no per-op timing) ===\n");
    memset(C,0,(size_t)csr.M*K*sizeof(float));
    amx_v6b_noprofile(B_bf16,C,pd);
    avx512_fallback(csr,B,C,pd.avx_rows); // warmup

    double best_e2e=1e9;
    for(int t=0;t<5;t++){
        memset(C,0,(size_t)csr.M*K*sizeof(float));
        t0=std::chrono::high_resolution_clock::now();
        amx_v6b_noprofile(B_bf16,C,pd);
        avx512_fallback(csr,B,C,pd.avx_rows);
        t1=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(t1-t0).count();
        printf("  [%d] %.2f ms\n", t, ms);
        if(ms<best_e2e) best_e2e=ms;
    }
    printf("  Best: %.2f ms\n", best_e2e);

    // ===== 汇总 =====
    printf("\n============ SUMMARY ============\n");
    printf("Pure AVX-512:       %.2f ms (baseline)\n", best_avx);
    printf("AMX V6b + FB (e2e): %.2f ms (%.2fx AVX)\n", best_e2e, best_avx/best_e2e);
    printf("  AMX part (prof):  %.2f ms\n", amx_part);
    printf("  Fallback:         %.2f ms\n", best_fb);
    printf("\nBreakdown (AMX BF16, contiguous A):\n");
    printf("  Gather B:  %.2f ms (%.0f%%)\n", best_r.gather_ms,
        100*best_r.gather_ms/amx_part);
    printf("  Compute:   %.2f ms (%.0f%%)\n", best_r.compute_ms,
        100*best_r.compute_ms/amx_part);
    printf("  Scatter:   %.2f ms (%.0f%%)\n", best_r.scatter_ms,
        100*best_r.scatter_ms/amx_part);

    free(B); free(B_bf16); free(C); free(pd.A_all);
    printf("\n=== Done ===\n");
    return 0;
}
