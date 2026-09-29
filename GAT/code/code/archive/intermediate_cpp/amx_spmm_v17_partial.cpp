/**
 * V17: Two-Pass Partial Tile
 * 
 * Pass 1: TILE_R=16 (standard, high efficiency)
 * Pass 2: TILE_R=8  (partial, captures orphan rows)
 * 
 * AMX hardware supports configurable tile rows (1-16).
 * 8-row tiles have half compute density but eliminate fallback rows.
 * 
 * K=128 only for this experiment.
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <immintrin.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <omp.h>

constexpr int TILE_C = 32;
constexpr int K = 128;
constexpr int KC = 16;
constexpr int N_KC = K / KC;
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

struct Panel {
    int rows[16];  // max 16 rows
    int nrows;     // actual row count (16 or 8)
    int U, ntiles, total_deg;
    int64_t A_offset;
    std::vector<int> b_rows;
};

struct PrecompData {
    std::vector<Panel> panels_16;  // 16-row panels (pass 1)
    std::vector<Panel> panels_8;   // 8-row panels (pass 2)
    std::vector<int> avx_rows;
    uint16_t* A_all_16;
    uint16_t* A_all_8;
    int64_t total_tiles_16, total_tiles_8;
    int64_t amx_nnz_16, amx_nnz_8;
};

struct PackedNZ { uint8_t row, tile_idx; uint16_t tile_col, bf16_val; };

// Generic panel builder for given TILE_R
void build_panels(const CSR& csr, const CSC& csc, int TILE_R, int min_anchor_deg,
                  std::vector<bool>& assigned, std::vector<int>& col_map,
                  std::vector<Panel>& out_panels, uint16_t*& out_A,
                  int64_t& out_tiles, int64_t& out_nnz) {
    int M = csr.M, N = csr.N;
    std::vector<int> deg(M);
    for(int i=0;i<M;i++) deg[i] = csr.indptr[i+1] - csr.indptr[i];

    // Sort columns by degree (descending)
    std::vector<int> col_order;
    for(int j=0;j<N;j++) if(csc.col_counts[j] >= min_anchor_deg) col_order.push_back(j);
    std::sort(col_order.begin(), col_order.end(),
        [&](int a, int b){ return csc.col_counts[a] > csc.col_counts[b]; });

    struct PB { Panel panel; std::vector<PackedNZ> packed_nz; };
    std::vector<PB> builds;
    int64_t tile_offset = 0;

    for(int col : col_order) {
        if(csc.col_counts[col] < min_anchor_deg) break;

        std::vector<int> avail;
        for(int64_t p = csc.col_ptr[col]; p < csc.col_ptr[col+1]; p++) {
            int r = csc.col_rows[p];
            if(!assigned[r]) avail.push_back(r);
        }
        if((int)avail.size() < TILE_R) continue;

        // Select TILE_R rows with lowest degree (ascending = light rows)
        if((int)avail.size() > TILE_R)
            std::partial_sort(avail.begin(), avail.begin()+TILE_R, avail.end(),
                [&](int a, int b){ return deg[a] < deg[b]; });

        // Collect union of columns
        std::vector<int> cols; int td = 0;
        for(int i=0;i<TILE_R;i++) {
            int r = avail[i]; td += deg[r];
            for(uint32_t p = csr.indptr[r]; p < csr.indptr[r+1]; p++)
                cols.push_back(csr.indices[p]);
        }
        std::sort(cols.begin(), cols.end());
        cols.erase(std::unique(cols.begin(), cols.end()), cols.end());
        int U = (int)cols.size(), nt = (U + TILE_C - 1) / TILE_C;
        float fill = (float)td / (TILE_R * TILE_C * nt);
        if(fill < FILL_THR || nt > MAX_NTILES) continue;

        // Build panel
        for(int j=0; j<(int)cols.size(); j++) col_map[cols[j]] = j;

        Panel panel;
        panel.nrows = TILE_R;
        panel.U = U; panel.ntiles = nt; panel.total_deg = td;
        panel.A_offset = tile_offset * TILE_R * TILE_C;  // note: always use TILE_R for this panel
        for(int i=0;i<TILE_R;i++) panel.rows[i] = avail[i];

        std::vector<PackedNZ> pnz; pnz.reserve(td);
        for(int i=0;i<TILE_R;i++) {
            int r = avail[i];
            for(uint32_t p = csr.indptr[r]; p < csr.indptr[r+1]; p++) {
                int j = col_map[csr.indices[p]];
                pnz.push_back({(uint8_t)i, (uint8_t)(j/TILE_C), (uint16_t)(j%TILE_C), f32_to_bf16(csr.values[p])});
            }
        }
        panel.b_rows.assign(nt * 32, -1);
        for(int t=0;t<nt;t++) for(int p=0;p<16;p++) {
            int je = t*TILE_C + p*2, jo = je+1;
            if(je<U) panel.b_rows[t*32+p*2] = cols[je];
            if(jo<U) panel.b_rows[t*32+p*2+1] = cols[jo];
        }

        for(int j=0;j<(int)cols.size();j++) col_map[cols[j]] = -1;
        for(int i=0;i<TILE_R;i++) assigned[avail[i]] = true;
        tile_offset += nt;
        builds.push_back({std::move(panel), std::move(pnz)});
    }

    // Pack A tiles (using actual TILE_R for row dimension, but buffer is TILE_R × TILE_C per tile)
    out_tiles = tile_offset;
    int64_t A_size = tile_offset * TILE_R * TILE_C;
    out_A = (uint16_t*)aligned_alloc(64, std::max(A_size, (int64_t)1) * sizeof(uint16_t));
    memset(out_A, 0, std::max(A_size, (int64_t)1) * sizeof(uint16_t));
    out_nnz = 0;
    for(auto& pb : builds) {
        uint16_t* base = out_A + pb.panel.A_offset;
        for(auto& nz : pb.packed_nz)
            base[(int64_t)nz.tile_idx * TILE_R * TILE_C + nz.row * TILE_C + nz.tile_col] = nz.bf16_val;
        out_nnz += pb.panel.total_deg;
        out_panels.push_back(std::move(pb.panel));
    }
}

PrecompData build_two_pass(const CSR& csr, const CSC& csc) {
    PrecompData pd;
    int M = csr.M, N = csr.N;
    std::vector<bool> assigned(M, false);
    std::vector<int> col_map(N, -1);

    // Pass 1: TILE_R = 16 (standard)
    printf("  [Pass 1] TILE_R=16...\n");
    build_panels(csr, csc, 16, 16, assigned, col_map,
                 pd.panels_16, pd.A_all_16, pd.total_tiles_16, pd.amx_nnz_16);

    int assigned_after_p1 = 0;
    for(int i=0;i<M;i++) if(assigned[i]) assigned_after_p1++;
    printf("    Panels=%d Tiles=%ld AMX_NNZ=%.1f%% Rows=%d/%d (%.1f%%)\n",
        (int)pd.panels_16.size(), pd.total_tiles_16,
        100.0*pd.amx_nnz_16/csr.nnz, assigned_after_p1, M, 100.0*assigned_after_p1/M);

    // Pass 2: TILE_R = 8 (partial tiles for remaining rows)
    printf("  [Pass 2] TILE_R=8...\n");
    build_panels(csr, csc, 8, 8, assigned, col_map,
                 pd.panels_8, pd.A_all_8, pd.total_tiles_8, pd.amx_nnz_8);

    int assigned_after_p2 = 0;
    for(int i=0;i<M;i++) if(assigned[i]) assigned_after_p2++;
    int new_rows = assigned_after_p2 - assigned_after_p1;
    printf("    Panels=%d Tiles=%ld AMX_NNZ=%.1f%% New_rows=%d\n",
        (int)pd.panels_8.size(), pd.total_tiles_8,
        100.0*pd.amx_nnz_8/csr.nnz, new_rows);

    // Remaining → fallback
    for(int i=0;i<M;i++) if(!assigned[i]) pd.avx_rows.push_back(i);

    int64_t total_amx = pd.amx_nnz_16 + pd.amx_nnz_8;
    printf("  [Total] AMX_NNZ=%.1f%% (%ld+%ld) Fallback_rows=%d (%.1f%%)\n",
        100.0*total_amx/csr.nnz, pd.amx_nnz_16, pd.amx_nnz_8,
        (int)pd.avx_rows.size(), 100.0*pd.avx_rows.size()/M);

    return pd;
}

// Single-pass build (baseline, TILE_R=16 only)
PrecompData build_single_pass(const CSR& csr, const CSC& csc) {
    PrecompData pd;
    int M = csr.M, N = csr.N;
    std::vector<bool> assigned(M, false);
    std::vector<int> col_map(N, -1);
    build_panels(csr, csc, 16, 16, assigned, col_map,
                 pd.panels_16, pd.A_all_16, pd.total_tiles_16, pd.amx_nnz_16);
    pd.A_all_8 = nullptr; pd.total_tiles_8 = 0; pd.amx_nnz_8 = 0;
    for(int i=0;i<M;i++) if(!assigned[i]) pd.avx_rows.push_back(i);
    return pd;
}

// ==================== Prefetch + Gather ====================
static inline void prefetch_tile_B(const uint16_t*B_bf16, const int*br, int kc_base) {
    for(int p=0;p<16;p++){
        int re=br[p*2], ro=br[p*2+1];
        if(re>=0){const char*a=(const char*)&B_bf16[(int64_t)re*K+kc_base];_mm_prefetch(a,_MM_HINT_T1);_mm_prefetch(a+64,_MM_HINT_T1);}
        if(ro>=0){const char*a=(const char*)&B_bf16[(int64_t)ro*K+kc_base];_mm_prefetch(a,_MM_HINT_T1);_mm_prefetch(a+64,_MM_HINT_T1);}
    }
}

static inline void gather_fused_4kc(const uint16_t*B_bf16, const int*br, int kc_base,
    uint8_t*Bb0, uint8_t*Bb1, uint8_t*Bb2, uint8_t*Bb3) {
    for(int p=0;p<16;p++){
        int re=br[p*2],ro=br[p*2+1];
        uint32_t*p0=(uint32_t*)(Bb0+p*64);uint32_t*p1=(uint32_t*)(Bb1+p*64);
        uint32_t*p2=(uint32_t*)(Bb2+p*64);uint32_t*p3=(uint32_t*)(Bb3+p*64);
        if(re>=0&&ro>=0){
            const uint16_t*be=&B_bf16[(int64_t)re*K+kc_base];const uint16_t*bo=&B_bf16[(int64_t)ro*K+kc_base];
            __m512i fe0=_mm512_loadu_si512(be);__m512i fe1=_mm512_loadu_si512(be+32);
            __m512i fo0=_mm512_loadu_si512(bo);__m512i fo1=_mm512_loadu_si512(bo+32);
            __m512i ie,io;
            ie=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fe0));io=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fo0));
            _mm512_store_si512((__m512i*)p0,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
            ie=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fe0,1));io=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fo0,1));
            _mm512_store_si512((__m512i*)p1,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
            ie=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fe1));io=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fo1));
            _mm512_store_si512((__m512i*)p2,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
            ie=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fe1,1));io=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fo1,1));
            _mm512_store_si512((__m512i*)p3,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
        } else if(re>=0){
            const uint16_t*be=&B_bf16[(int64_t)re*K+kc_base];
            __m512i fe0=_mm512_loadu_si512(be);__m512i fe1=_mm512_loadu_si512(be+32);
            _mm512_store_si512((__m512i*)p0,_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fe0)));
            _mm512_store_si512((__m512i*)p1,_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fe0,1)));
            _mm512_store_si512((__m512i*)p2,_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fe1)));
            _mm512_store_si512((__m512i*)p3,_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fe1,1)));
        } else {
            __m512i z=_mm512_setzero_si512();
            _mm512_store_si512((__m512i*)p0,z);_mm512_store_si512((__m512i*)p1,z);
            _mm512_store_si512((__m512i*)p2,z);_mm512_store_si512((__m512i*)p3,z);
        }
    }
}

// ==================== AMX kernel for given panel list ====================
// tile_rows: 16 or 8 (configures AMX tile)
void amx_kernel(const uint16_t*B_bf16, float*C, const std::vector<Panel>& panels,
                const uint16_t* A_all, int tile_rows, int PF_DIST) {
    int np = (int)panels.size();
    if(np == 0) return;
    
    #pragma omp parallel
    {
        syscall(SYS_arch_prctl, 0x1023, 18);
        tile_config_t cfg; memset(&cfg, 0, 64); cfg.palette_id = 1;
        // tmm0: C acc (tile_rows × KC*4)
        cfg.rows[0] = tile_rows; cfg.colsb[0] = KC*4;
        // tmm1: A tile (tile_rows × 64)
        cfg.rows[1] = tile_rows; cfg.colsb[1] = 64;
        // tmm2: B tile (16 × KC*4) — B tile is always 16 rows
        cfg.rows[2] = 16; cfg.colsb[2] = KC*4;
        // tmm3,4,5: additional C accumulators
        cfg.rows[3] = tile_rows; cfg.colsb[3] = KC*4;
        cfg.rows[4] = tile_rows; cfg.colsb[4] = KC*4;
        cfg.rows[5] = tile_rows; cfg.colsb[5] = KC*4;
        _tile_loadconfig(&cfg);

        alignas(64) uint8_t Bb0[1024],Bb1[1024],Bb2[1024],Bb3[1024];
        alignas(64) float Cb0[16][KC],Cb1[16][KC],Cb2[16][KC],Cb3[16][KC];

        #pragma omp for schedule(dynamic, 8)
        for(int pi=0; pi<np; pi++) {
            const Panel& P = panels[pi];
            const uint16_t* preA = A_all + P.A_offset;

            for(int pass=0; pass<2; pass++) {
                int kc_base = pass * 64;
                _tile_zero(0); _tile_zero(3); _tile_zero(4); _tile_zero(5);

                for(int pf=0; pf<PF_DIST && pf<P.ntiles; pf++)
                    prefetch_tile_B(B_bf16, &P.b_rows[pf*32], kc_base);

                for(int t=0; t<P.ntiles; t++) {
                    if(t+PF_DIST < P.ntiles)
                        prefetch_tile_B(B_bf16, &P.b_rows[(t+PF_DIST)*32], kc_base);

                    gather_fused_4kc(B_bf16, &P.b_rows[t*32], kc_base, Bb0, Bb1, Bb2, Bb3);

                    // A tile stride = tile_rows * TILE_C (not always 16*32)
                    _tile_loadd(1, preA + (size_t)t * tile_rows * TILE_C, 64);
                    _tile_loadd(2, Bb0, 64); _tile_dpbf16ps(0, 1, 2);
                    _tile_loadd(2, Bb1, 64); _tile_dpbf16ps(3, 1, 2);
                    _tile_loadd(2, Bb2, 64); _tile_dpbf16ps(4, 1, 2);
                    _tile_loadd(2, Bb3, 64); _tile_dpbf16ps(5, 1, 2);
                }

                _tile_stored(0, Cb0, KC*4);
                _tile_stored(3, Cb1, KC*4);
                _tile_stored(4, Cb2, KC*4);
                _tile_stored(5, Cb3, KC*4);

                for(int i=0; i<P.nrows; i++) {
                    float* dst = C + (int64_t)P.rows[i]*K + kc_base;
                    for(int k=0;k<KC;k++) dst[k]     = Cb0[i][k];
                    for(int k=0;k<KC;k++) dst[k+KC]   = Cb1[i][k];
                    for(int k=0;k<KC;k++) dst[k+2*KC] = Cb2[i][k];
                    for(int k=0;k<KC;k++) dst[k+3*KC] = Cb3[i][k];
                }
            }
        }
        _tile_release();
    }
}

// ==================== AVX-512 ====================
void avx512_all(const CSR& csr, const float*B, float*C) {
    #pragma omp parallel for schedule(dynamic,256)
    for(int i=0;i<csr.M;i++){
        float*Cr=C+(int64_t)i*K;
        __m512 c[8]; for(int j=0;j<8;j++) c[j]=_mm512_setzero_ps();
        for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
            __m512 a=_mm512_set1_ps(csr.values[p]);
            const float*Br=B+(int64_t)csr.indices[p]*K;
            for(int j=0;j<8;j++) c[j]=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+j*16),c[j]);
        }
        for(int j=0;j<8;j++) _mm512_storeu_ps(Cr+j*16,c[j]);
    }
}
void avx512_rows(const CSR& csr, const float*B, float*C, const std::vector<int>& rows) {
    int nr=(int)rows.size();
    #pragma omp parallel for schedule(dynamic,256)
    for(int idx=0;idx<nr;idx++){
        int i=rows[idx]; float*Cr=C+(int64_t)i*K;
        __m512 c[8]; for(int j=0;j<8;j++) c[j]=_mm512_setzero_ps();
        for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
            __m512 a=_mm512_set1_ps(csr.values[p]);
            const float*Br=B+(int64_t)csr.indices[p]*K;
            for(int j=0;j<8;j++) c[j]=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+j*16),c[j]);
        }
        for(int j=0;j<8;j++) _mm512_storeu_ps(Cr+j*16,c[j]);
    }
}

int main(int argc, char** argv) {
    if(argc < 2) { printf("Usage: %s <mat> [thr]\n", argv[0]); return 1; }
    int nt = (argc >= 3) ? atoi(argv[2]) : omp_get_num_procs();
    omp_set_num_threads(nt);
    syscall(SYS_arch_prctl, 0x1023, 18);

    printf("=== V17 Two-Pass Partial Tile K=128 ===\n");
    printf("threads=%d\n\n", nt);

    CSR csr = read_csrbin(argv[1]);
    printf("M=%d N=%d NNZ=%ld avg=%.1f\n\n", csr.M, csr.N, csr.nnz, (double)csr.nnz/csr.M);
    CSC csc = build_csc(csr);

    // ===== Build single-pass baseline =====
    printf("[Baseline: single-pass TILE_R=16]\n");
    PrecompData pd1 = build_single_pass(csr, csc);
    printf("  Panels=%d Tiles=%ld AMX_NNZ=%.1f%% FB_rows=%d\n\n",
        (int)pd1.panels_16.size(), pd1.total_tiles_16,
        100.0*pd1.amx_nnz_16/csr.nnz, (int)pd1.avx_rows.size());

    // ===== Build two-pass =====
    printf("[Two-pass: TILE_R=16 + TILE_R=8]\n");
    PrecompData pd2 = build_two_pass(csr, csc);

    // Allocate B, C
    float* B = (float*)aligned_alloc(64, (size_t)csr.N*K*sizeof(float));
    srand(12345);
    for(int64_t i=0;i<(int64_t)csr.N*K;i++) B[i]=(rand()%200-100)/100.0f;
    uint16_t* B_bf16 = (uint16_t*)aligned_alloc(64, (size_t)csr.N*K*sizeof(uint16_t));
    for(int64_t i=0;i<(int64_t)csr.N*K;i++) B_bf16[i]=f32_to_bf16(B[i]);
    float* C = (float*)aligned_alloc(64, (size_t)csr.M*K*sizeof(float));
    float* C_ref = (float*)aligned_alloc(64, (size_t)csr.M*K*sizeof(float));

    // AVX baseline
    printf("\n[AVX-512 baseline]...\n");
    avx512_all(csr, B, C);
    double best_avx = 1e9;
    for(int t=0;t<5;t++){
        auto T0=std::chrono::high_resolution_clock::now();
        avx512_all(csr, B, C);
        auto T1=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(T1-T0).count();
        if(ms<best_avx) best_avx=ms;
    }
    printf("  Best: %.2f ms\n", best_avx);
    memcpy(C_ref, C, (size_t)csr.M*K*sizeof(float));

    // ===== Benchmark single-pass (V16-style) =====
    auto bench = [&](const char* name, PrecompData& pd) {
        // warmup
        memset(C, 0, (size_t)csr.M*K*sizeof(float));
        amx_kernel(B_bf16, C, pd.panels_16, pd.A_all_16, 16, 3);
        if(!pd.panels_8.empty())
            amx_kernel(B_bf16, C, pd.panels_8, pd.A_all_8, 8, 3);
        avx512_rows(csr, B, C, pd.avx_rows);

        double best=1e9, ba16=1e9, ba8=1e9, bf=1e9;
        for(int t=0;t<5;t++){
            memset(C, 0, (size_t)csr.M*K*sizeof(float));
            auto T0=std::chrono::high_resolution_clock::now();
            amx_kernel(B_bf16, C, pd.panels_16, pd.A_all_16, 16, 3);
            auto T1=std::chrono::high_resolution_clock::now();
            if(!pd.panels_8.empty())
                amx_kernel(B_bf16, C, pd.panels_8, pd.A_all_8, 8, 3);
            auto T2=std::chrono::high_resolution_clock::now();
            avx512_rows(csr, B, C, pd.avx_rows);
            auto T3=std::chrono::high_resolution_clock::now();
            double a16=std::chrono::duration<double,std::milli>(T1-T0).count();
            double a8=std::chrono::duration<double,std::milli>(T2-T1).count();
            double f=std::chrono::duration<double,std::milli>(T3-T2).count();
            if(a16+a8+f < best){best=a16+a8+f; ba16=a16; ba8=a8; bf=f;}
        }
        printf("  %-20s amx16=%7.2f amx8=%7.2f fb=%7.2f e2e=%7.2f (%.2fx)\n",
            name, ba16, ba8, bf, best, best_avx/best);
    };

    printf("\n[Benchmarking]...\n");
    bench("Single-pass(16):", pd1);
    bench("Two-pass(16+8):", pd2);

    // Correctness check
    memset(C, 0, (size_t)csr.M*K*sizeof(float));
    amx_kernel(B_bf16, C, pd2.panels_16, pd2.A_all_16, 16, 3);
    if(!pd2.panels_8.empty())
        amx_kernel(B_bf16, C, pd2.panels_8, pd2.A_all_8, 8, 3);
    avx512_rows(csr, B, C, pd2.avx_rows);
    int bad=0; float mx=0;
    for(int64_t i=0;i<(int64_t)csr.M*K;i++){
        float ae=fabsf(C_ref[i]-C[i]); if(ae>mx) mx=ae;
        float re=(fabsf(C_ref[i])>1e-6f)?ae/fabsf(C_ref[i]):ae;
        if(re>0.05f&&fabsf(C_ref[i])>0.01f) bad++;
    }
    printf("\n  Correct: max=%.3e bad=%.4f%% %s\n", mx, 100.0*bad/((double)csr.M*K), (100.0*bad/((double)csr.M*K))<1.0?"OK":"FAIL");

    free(B); free(B_bf16); free(C); free(C_ref);
    if(pd1.A_all_16) free(pd1.A_all_16);
    if(pd2.A_all_16) free(pd2.A_all_16);
    if(pd2.A_all_8) free(pd2.A_all_8);
    printf("\n=== Done ===\n");
    return 0;
}
