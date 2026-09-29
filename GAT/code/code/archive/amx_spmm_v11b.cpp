/**
 * V11: POCM (Pack-Once, Compute-Many) + Adaptive Row Selection + ROI Threshold
 *
 * Breakthroughs:
 *   1. B_all pre-packing: B matrix pre-packed into VNNI tile format at setup
 *      → Runtime kernel is pure tile_loadd(A) + tile_loadd(B) + dpbf16ps
 *      → ZERO runtime Gather. ZERO cache pollution from random B access.
 *   2. Adaptive row selection: deg descending for high-degree graphs (shunting)
 *   3. ROI threshold: only accept panels with NNZ/B-reads > threshold
 *   4. Benchmark: single-shot + amortized (100 iters) for POCM evaluation
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

constexpr int TILE_R = 16;
constexpr int TILE_C = 32;
constexpr int K      = 32;
constexpr int KC     = 16;
constexpr int N_KC   = K / KC;
constexpr int BTILE  = 16 * 64;

constexpr float FILL_THR   = 0.0625f;
constexpr int   MAX_NTILES = 64;
constexpr float ROI_THR    = 1.0f;
constexpr float DEG_THR    = 50.0f;

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
    if (!f) { fprintf(stderr, "ERR open: %s\n", path); exit(1); }
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
    int rows[TILE_R];
    int nrows, U, ntiles, total_deg;
    int64_t A_offset;
    int64_t B_offset;
    std::vector<int> b_rows;
};

struct PrecompData {
    std::vector<Panel> panels;
    std::vector<int> avx_rows;
    uint16_t* A_all;
    uint8_t*  B_all;
    int64_t total_tiles;
    int64_t total_B_bytes;
    int64_t amx_nnz;
    bool desc_mode;
};

struct PackedNZ { uint8_t row, tile_idx; uint16_t tile_col, bf16_val; };

PrecompData build_panels(const CSR& csr, const CSC& csc) {
    PrecompData pd;
    int M=csr.M, N=csr.N;
    float avg_deg = (float)csr.nnz / csr.M;
    pd.desc_mode = (avg_deg > DEG_THR);
    printf("    avg_deg=%.1f -> row selection: %s\n",
        avg_deg, pd.desc_mode ? "DESCENDING (heavy first)" : "ASCENDING (light first)");

    std::vector<int> deg(M);
    for(int i=0;i<M;i++) deg[i]=csr.indptr[i+1]-csr.indptr[i];

    std::vector<int> col_order;
    for(int j=0;j<N;j++) if(csc.col_counts[j]>=TILE_R) col_order.push_back(j);
    std::sort(col_order.begin(),col_order.end(),
        [&](int a,int b){return csc.col_counts[a]>csc.col_counts[b];});

    std::vector<bool> assigned(M,false);
    std::vector<int> col_map(N,-1);

    struct PB { Panel panel; std::vector<PackedNZ> packed_nz; };
    std::vector<PB> builds;
    int64_t tile_offset=0, b_byte_offset=0;
    int rejected_fill=0, rejected_roi=0, rejected_nt=0;

    for(int col:col_order){
        if(csc.col_counts[col]<TILE_R) break;
        std::vector<int> avail;
        for(int64_t p=csc.col_ptr[col];p<csc.col_ptr[col+1];p++){
            int r=csc.col_rows[p]; if(!assigned[r]) avail.push_back(r);
        }
        if((int)avail.size()<TILE_R) continue;

        if((int)avail.size()>TILE_R){
            if(pd.desc_mode)
                std::partial_sort(avail.begin(),avail.begin()+TILE_R,avail.end(),
                    [&](int a,int b){return deg[a]>deg[b];});
            else
                std::partial_sort(avail.begin(),avail.begin()+TILE_R,avail.end(),
                    [&](int a,int b){return deg[a]<deg[b];});
        }

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

        if(fill<FILL_THR) { rejected_fill++; continue; }
        if(nt>MAX_NTILES) { rejected_nt++; continue; }
        float roi = (float)td / (nt * TILE_C);
        if(roi<ROI_THR) { rejected_roi++; continue; }

        for(int j=0;j<(int)cols.size();j++) col_map[cols[j]]=j;
        Panel panel;
        panel.nrows=TILE_R; panel.U=U; panel.ntiles=nt; panel.total_deg=td;
        panel.A_offset = tile_offset * TILE_R * TILE_C;
        panel.B_offset = b_byte_offset;
        for(int i=0;i<TILE_R;i++) panel.rows[i]=avail[i];

        std::vector<PackedNZ> pnz; pnz.reserve(td);
        for(int i=0;i<TILE_R;i++){
            int r=avail[i];
            for(uint32_t p=csr.indptr[r];p<csr.indptr[r+1];p++){
                int j=col_map[csr.indices[p]];
                pnz.push_back({(uint8_t)i,(uint8_t)(j/TILE_C),
                    (uint16_t)(j%TILE_C),f32_to_bf16(csr.values[p])});
            }
        }

        panel.b_rows.assign(nt*32,-1);
        for(int t=0;t<nt;t++) for(int p=0;p<16;p++){
            int je=t*TILE_C+p*2, jo=je+1;
            if(je<U) panel.b_rows[t*32+p*2]=cols[je];
            if(jo<U) panel.b_rows[t*32+p*2+1]=cols[jo];
        }
        for(int j=0;j<(int)cols.size();j++) col_map[cols[j]]=-1;
        for(int i=0;i<TILE_R;i++) assigned[avail[i]]=true;

        tile_offset += nt;
        b_byte_offset += (int64_t)nt * N_KC * BTILE;
        builds.push_back({std::move(panel), std::move(pnz)});
    }

    pd.total_tiles = tile_offset;
    pd.total_B_bytes = b_byte_offset;
    int64_t A_size = tile_offset * TILE_R * TILE_C;
    pd.A_all = (uint16_t*)aligned_alloc(64, std::max(A_size,(int64_t)1)*sizeof(uint16_t));
    memset(pd.A_all, 0, A_size * sizeof(uint16_t));

    pd.B_all = (uint8_t*)aligned_alloc(64, std::max(b_byte_offset,(int64_t)64));
    memset(pd.B_all, 0, std::max(b_byte_offset,(int64_t)64));

    pd.amx_nnz = 0;
    for(auto& pb : builds){
        uint16_t* base = pd.A_all + pb.panel.A_offset;
        for(auto& nz : pb.packed_nz)
            base[(int64_t)nz.tile_idx*TILE_R*TILE_C + nz.row*TILE_C + nz.tile_col] = nz.bf16_val;
        pd.amx_nnz += pb.panel.total_deg;
        pd.panels.push_back(std::move(pb.panel));
    }
    for(int i=0;i<M;i++) if(!assigned[i]) pd.avx_rows.push_back(i);

    printf("    Panels=%d Tiles=%ld A_all=%.1fMB B_all=%.1fMB\n",
        (int)pd.panels.size(), tile_offset,
        (double)A_size*2/1e6, (double)b_byte_offset/1e6);
    printf("    AMX_NNZ=%.1f%% AVX_rows=%d\n",
        100.0*pd.amx_nnz/csr.nnz, (int)pd.avx_rows.size());
    printf("    Rejected: fill=%d ntiles=%d roi=%d\n",
        rejected_fill, rejected_nt, rejected_roi);
    return pd;
}

void pack_B_all(const uint16_t* B_bf16, PrecompData& pd) {
    int np = (int)pd.panels.size();
    #pragma omp parallel for schedule(dynamic, 16)
    for(int pi=0; pi<np; pi++){
        const Panel& P = pd.panels[pi];
        for(int kc_idx=0; kc_idx<N_KC; kc_idx++){
            int kc = kc_idx * KC;
            for(int t=0; t<P.ntiles; t++){
                uint8_t* dst = pd.B_all + P.B_offset
                    + (int64_t)(kc_idx * P.ntiles + t) * BTILE;
                const int* br = &P.b_rows[t*32];
                for(int p=0; p<16; p++){
                    int re = br[p*2], ro = br[p*2+1];
                    uint32_t* pr = (uint32_t*)(dst + p*64);
                    if(re>=0 && ro>=0){
                        __m256i ve = _mm256_loadu_si256((const __m256i*)&B_bf16[(int64_t)re*K+kc]);
                        __m256i vo = _mm256_loadu_si256((const __m256i*)&B_bf16[(int64_t)ro*K+kc]);
                        __m512i ie = _mm512_cvtepu16_epi32(ve);
                        __m512i io = _mm512_cvtepu16_epi32(vo);
                        _mm512_stream_si512((__m512i*)pr,
                            _mm512_or_si512(ie, _mm512_slli_epi32(io,16)));
                    } else if(re>=0){
                        __m256i ve = _mm256_loadu_si256((const __m256i*)&B_bf16[(int64_t)re*K+kc]);
                        _mm512_stream_si512((__m512i*)pr, _mm512_cvtepu16_epi32(ve));
                    } else {
                        _mm512_stream_si512((__m512i*)pr, _mm512_setzero_si512());
                    }
                }
            }
        }
    }
    _mm_sfence();
}

void amx_v11_compute(float* C, const PrecompData& pd) {
    int np = (int)pd.panels.size();
    const uint16_t* A_all = pd.A_all;
    const uint8_t*  B_all = pd.B_all;

    #pragma omp parallel
    {
        syscall(SYS_arch_prctl, 0x1023, 18);
        tile_config_t cfg; memset(&cfg,0,64);
        cfg.palette_id=1;
        cfg.rows[0]=16; cfg.colsb[0]=KC*4;
        cfg.rows[1]=16; cfg.colsb[1]=64;
        cfg.rows[2]=16; cfg.colsb[2]=KC*4;
        _tile_loadconfig(&cfg);

        alignas(64) float Cb[16][KC];

        #pragma omp for schedule(dynamic,8)
        for(int pi=0; pi<np; pi++){
            const Panel& P = pd.panels[pi];
            for(int kc_idx=0; kc_idx<N_KC; kc_idx++){
                _tile_zero(0);
                for(int t=0; t<P.ntiles; t++){
                    _tile_loadd(1, A_all + P.A_offset + (size_t)t*TILE_R*TILE_C, 64);
                    _tile_loadd(2, B_all + P.B_offset
                        + (int64_t)(kc_idx*P.ntiles+t)*BTILE, 64);
                    _tile_dpbf16ps(0, 1, 2);
                }
                _tile_stored(0, Cb, KC*4);
                int kc = kc_idx * KC;
                for(int i=0; i<TILE_R; i++){
                    float* dst = C + (int64_t)P.rows[i]*K + kc;
                    for(int k=0; k<KC; k++) dst[k] = Cb[i][k];
                }
            }
        }
        _tile_release();
    }
}

void avx512_all(const CSR& csr, const float* B, float* C) {
    #pragma omp parallel for schedule(dynamic,256)
    for(int i=0; i<csr.M; i++){
        float* Cr = C + (int64_t)i*K;
        __m512 c0=_mm512_setzero_ps(), c1=_mm512_setzero_ps();
        for(uint32_t p=csr.indptr[i]; p<csr.indptr[i+1]; p++){
            __m512 a = _mm512_set1_ps(csr.values[p]);
            const float* Br = B + (int64_t)csr.indices[p]*K;
            c0 = _mm512_fmadd_ps(a, _mm512_loadu_ps(Br), c0);
            c1 = _mm512_fmadd_ps(a, _mm512_loadu_ps(Br+16), c1);
        }
        _mm512_storeu_ps(Cr, c0); _mm512_storeu_ps(Cr+16, c1);
    }
}

void avx512_rows(const CSR& csr, const float* B, float* C,
                 const std::vector<int>& rows) {
    int nr = (int)rows.size();
    #pragma omp parallel for schedule(dynamic,256)
    for(int idx=0; idx<nr; idx++){
        int i = rows[idx]; float* Cr = C + (int64_t)i*K;
        __m512 c0=_mm512_setzero_ps(), c1=_mm512_setzero_ps();
        for(uint32_t p=csr.indptr[i]; p<csr.indptr[i+1]; p++){
            __m512 a = _mm512_set1_ps(csr.values[p]);
            const float* Br = B + (int64_t)csr.indices[p]*K;
            c0 = _mm512_fmadd_ps(a, _mm512_loadu_ps(Br), c0);
            c1 = _mm512_fmadd_ps(a, _mm512_loadu_ps(Br+16), c1);
        }
        _mm512_storeu_ps(Cr, c0); _mm512_storeu_ps(Cr+16, c1);
    }
}

int main(int argc, char** argv) {
    if(argc<2){printf("Usage: %s <mat> [threads]\n",argv[0]); return 1;}
    int nt = (argc>=3) ? atoi(argv[2]) : omp_get_num_procs();
    omp_set_num_threads(nt);
    syscall(SYS_arch_prctl, 0x1023, 18);

    printf("=== AMX V11b: POCM + NT-Store (Pack-Once, Compute-Many) ===\n");
    printf("K=%d threads=%d  FILL=%.4f ROI=%.1f DEG_THR=%.0f MAXNT=%d\n\n",
        K, nt, FILL_THR, ROI_THR, DEG_THR, MAX_NTILES);

    CSR csr = read_csrbin(argv[1]);
    printf("[1] M=%d N=%d NNZ=%ld avg=%.1f\n", csr.M, csr.N, csr.nnz,
        (double)csr.nnz/csr.M);
    CSC csc = build_csc(csr);

    printf("[2] Build panels...\n");
    auto T0 = std::chrono::high_resolution_clock::now();
    PrecompData pd = build_panels(csr, csc);
    auto T1 = std::chrono::high_resolution_clock::now();
    printf("    Prep: %.2fs\n\n", std::chrono::duration<double>(T1-T0).count());

    float* B = (float*)aligned_alloc(64, (size_t)csr.N*K*sizeof(float));
    srand(12345);
    for(int64_t i=0; i<(int64_t)csr.N*K; i++) B[i] = (rand()%200-100)/100.0f;
    uint16_t* B_bf16 = (uint16_t*)aligned_alloc(64, (size_t)csr.N*K*sizeof(uint16_t));
    for(int64_t i=0; i<(int64_t)csr.N*K; i++) B_bf16[i] = f32_to_bf16(B[i]);

    float* C_ref = (float*)aligned_alloc(64, (size_t)csr.M*K*sizeof(float));
    float* C     = (float*)aligned_alloc(64, (size_t)csr.M*K*sizeof(float));

    printf("[3] Pure AVX-512 (%dT)...\n", nt);
    avx512_all(csr, B, C);
    double best_avx = 1e9;
    for(int t=0; t<5; t++){
        T0 = std::chrono::high_resolution_clock::now();
        avx512_all(csr, B, C);
        T1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double,std::milli>(T1-T0).count();
        if(ms < best_avx) best_avx = ms;
    }
    printf("    Best: %.2f ms\n\n", best_avx);
    memcpy(C_ref, C, (size_t)csr.M*K*sizeof(float));

    printf("[4] AMX V11 Single-Shot (Pack + Compute + FB)...\n");
    pack_B_all(B_bf16, pd);
    memset(C, 0, (size_t)csr.M*K*sizeof(float));
    amx_v11_compute(C, pd);
    avx512_rows(csr, B, C, pd.avx_rows);

    double best_pack=1e9, best_comp=1e9, best_fb=1e9, best_e2e=1e9;
    for(int t=0; t<5; t++){
        T0 = std::chrono::high_resolution_clock::now();
        pack_B_all(B_bf16, pd);
        auto Tp = std::chrono::high_resolution_clock::now();
        memset(C, 0, (size_t)csr.M*K*sizeof(float));
        amx_v11_compute(C, pd);
        auto Tc = std::chrono::high_resolution_clock::now();
        avx512_rows(csr, B, C, pd.avx_rows);
        T1 = std::chrono::high_resolution_clock::now();

        double pk = std::chrono::duration<double,std::milli>(Tp-T0).count();
        double cp = std::chrono::duration<double,std::milli>(Tc-Tp).count();
        double fb = std::chrono::duration<double,std::milli>(T1-Tc).count();
        printf("    [%d] pack=%.2f compute=%.2f fb=%.2f total=%.2f\n",
            t, pk, cp, fb, pk+cp+fb);
        if(pk+cp+fb < best_e2e){
            best_e2e=pk+cp+fb; best_pack=pk; best_comp=cp; best_fb=fb;
        }
    }
    printf("    Best: pack=%.2f + compute=%.2f + fb=%.2f = %.2f ms\n",
        best_pack, best_comp, best_fb, best_e2e);
    printf("    * Single-shot vs AVX: %.2fx\n\n", best_avx/best_e2e);

    printf("[5] AMX V11 Compute-Only (B_all pre-packed)...\n");
    pack_B_all(B_bf16, pd);
    double best_co=1e9, best_fb2=1e9;
    for(int t=0; t<5; t++){
        memset(C, 0, (size_t)csr.M*K*sizeof(float));
        T0 = std::chrono::high_resolution_clock::now();
        amx_v11_compute(C, pd);
        auto Tc = std::chrono::high_resolution_clock::now();
        avx512_rows(csr, B, C, pd.avx_rows);
        T1 = std::chrono::high_resolution_clock::now();
        double cp = std::chrono::duration<double,std::milli>(Tc-T0).count();
        double fb = std::chrono::duration<double,std::milli>(T1-Tc).count();
        printf("    [%d] compute=%.2f fb=%.2f total=%.2f\n", t, cp, fb, cp+fb);
        if(cp+fb < best_co+best_fb2){ best_co=cp; best_fb2=fb; }
    }
    printf("    Best: compute=%.2f + fb=%.2f = %.2f ms\n",
        best_co, best_fb2, best_co+best_fb2);
    printf("    * Compute-only vs AVX: %.2fx\n\n", best_avx/(best_co+best_fb2));

    printf("[6] Amortized (100 iters)...\n");
    const int NI=100;
    for(int w=0;w<2;w++){
        pack_B_all(B_bf16,pd);
        memset(C,0,(size_t)csr.M*K*sizeof(float));
        amx_v11_compute(C,pd);
        avx512_rows(csr,B,C,pd.avx_rows);
    }
    T0 = std::chrono::high_resolution_clock::now();
    for(int it=0;it<NI;it++){
        pack_B_all(B_bf16,pd);
        memset(C,0,(size_t)csr.M*K*sizeof(float));
        amx_v11_compute(C,pd);
        avx512_rows(csr,B,C,pd.avx_rows);
    }
    T1 = std::chrono::high_resolution_clock::now();
    double amx_tot = std::chrono::duration<double,std::milli>(T1-T0).count();

    for(int w=0;w<2;w++) avx512_all(csr,B,C);
    T0 = std::chrono::high_resolution_clock::now();
    for(int it=0;it<NI;it++) avx512_all(csr,B,C);
    T1 = std::chrono::high_resolution_clock::now();
    double avx_tot = std::chrono::duration<double,std::milli>(T1-T0).count();

    printf("    AMX: %.1fms total (%.2f ms/iter)\n", amx_tot, amx_tot/NI);
    printf("    AVX: %.1fms total (%.2f ms/iter)\n", avx_tot, avx_tot/NI);
    printf("    * Amortized speedup: %.2fx\n\n", avx_tot/amx_tot);

    printf("[7] Correctness...\n");
    pack_B_all(B_bf16, pd);
    memset(C, 0, (size_t)csr.M*K*sizeof(float));
    amx_v11_compute(C, pd);
    avx512_rows(csr, B, C, pd.avx_rows);
    int bad=0; float max_abs=0;
    for(int64_t i=0; i<(int64_t)csr.M*K; i++){
        float ae = fabsf(C_ref[i] - C[i]);
        if(ae > max_abs) max_abs = ae;
        float re = (fabsf(C_ref[i])>1e-6f) ? ae/fabsf(C_ref[i]) : ae;
        if(re>0.05f && fabsf(C_ref[i])>0.01f) bad++;
    }
    printf("    Max abs: %.3e  Bad: %d (%.4f%%)\n", max_abs, bad, 100.0*bad/(csr.M*K));
    printf("    %s\n\n", 100.0*bad/(csr.M*K)<1.0 ? "OK" : "FAIL");

    printf("[8] Per-NNZ:\n");
    printf("    AMX compute: %.2f ns/NNZ\n", best_co*1e6/pd.amx_nnz);
    printf("    Fallback:    %.2f ns/NNZ\n", best_fb2*1e6/(csr.nnz-pd.amx_nnz));
    printf("    Pure AVX:    %.2f ns/NNZ\n\n", best_avx*1e6/csr.nnz);

    printf("============ SUMMARY ============\n");
    printf("Pure AVX-512:     %.2f ms\n", best_avx);
    printf("V11 single-shot:  %.2f ms (%.2fx) [pack=%.2f comp=%.2f fb=%.2f]\n",
        best_e2e, best_avx/best_e2e, best_pack, best_comp, best_fb);
    printf("V11 compute-only: %.2f ms (%.2fx)\n",
        best_co+best_fb2, best_avx/(best_co+best_fb2));
    printf("V11 amortized:    %.2f ms/iter (%.2fx)\n",
        amx_tot/NI, avx_tot/amx_tot);

    free(B); free(B_bf16); free(C); free(C_ref);
    free(pd.A_all); free(pd.B_all);
    printf("\n=== Done ===\n");
    return 0;
}
