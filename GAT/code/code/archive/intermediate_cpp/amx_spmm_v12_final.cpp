/**
 * V12-Final: KC-Fusion + Adaptive Row Selection
 * - Config A: ascending degree (light rows) — wins on high-avg-deg graphs
 * - Config B: descending degree (heavy rows) — wins on low-avg-deg graphs
 * - KC-Fusion: kc inner loop, B cache line loaded once
 * - K parameterized at compile time via -DK_VAL=xxx
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

#ifndef K_VAL
#define K_VAL 32
#endif

constexpr int TILE_R = 16;
constexpr int TILE_C = 32;
constexpr int K      = K_VAL;
constexpr int KC     = 16;
constexpr int N_KC   = K / KC;
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
    int rows[TILE_R];
    int nrows, U, ntiles, total_deg;
    int64_t A_offset;
    std::vector<int> b_rows;
};
struct PrecompData {
    std::vector<Panel> panels;
    std::vector<int> avx_rows;
    uint16_t* A_all;
    int64_t total_tiles, amx_nnz;
};
struct PackedNZ { uint8_t row, tile_idx; uint16_t tile_col, bf16_val; };

// ascending=true → Config A (light rows), ascending=false → Config B (heavy rows)
PrecompData build_all(const CSR& csr, const CSC& csc, bool ascending) {
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
    struct PB { Panel panel; std::vector<PackedNZ> packed_nz; };
    std::vector<PB> builds;
    int64_t tile_offset=0;
    for(int col:col_order){
        if(csc.col_counts[col]<TILE_R) break;
        std::vector<int> avail;
        for(int64_t p=csc.col_ptr[col];p<csc.col_ptr[col+1];p++){
            int r=csc.col_rows[p]; if(!assigned[r]) avail.push_back(r);
        }
        if((int)avail.size()<TILE_R) continue;
        if((int)avail.size()>TILE_R){
            if(ascending)
                std::partial_sort(avail.begin(),avail.begin()+TILE_R,avail.end(),
                    [&](int a,int b){return deg[a]<deg[b];});
            else
                std::partial_sort(avail.begin(),avail.begin()+TILE_R,avail.end(),
                    [&](int a,int b){return deg[a]>deg[b];});
        }
        std::vector<int> cols; int td=0;
        for(int i=0;i<TILE_R;i++){
            int r=avail[i]; td+=deg[r];
            for(uint32_t p=csr.indptr[r];p<csr.indptr[r+1];p++)
                cols.push_back(csr.indices[p]);
        }
        std::sort(cols.begin(),cols.end());
        cols.erase(std::unique(cols.begin(),cols.end()),cols.end());
        int U=(int)cols.size(),nt=(U+TILE_C-1)/TILE_C;
        float fill=(float)td/(TILE_R*TILE_C*nt);
        if(fill<FILL_THR||nt>MAX_NTILES) continue;
        for(int j=0;j<(int)cols.size();j++) col_map[cols[j]]=j;
        Panel panel;
        panel.nrows=TILE_R;panel.U=U;panel.ntiles=nt;panel.total_deg=td;
        panel.A_offset=tile_offset*TILE_R*TILE_C;
        for(int i=0;i<TILE_R;i++) panel.rows[i]=avail[i];
        std::vector<PackedNZ> pnz; pnz.reserve(td);
        for(int i=0;i<TILE_R;i++){
            int r=avail[i];
            for(uint32_t p=csr.indptr[r];p<csr.indptr[r+1];p++){
                int j=col_map[csr.indices[p]];
                pnz.push_back({(uint8_t)i,(uint8_t)(j/TILE_C),(uint16_t)(j%TILE_C),f32_to_bf16(csr.values[p])});
            }
        }
        panel.b_rows.assign(nt*32,-1);
        for(int t=0;t<nt;t++) for(int p=0;p<16;p++){
            int je=t*TILE_C+p*2,jo=je+1;
            if(je<U) panel.b_rows[t*32+p*2]=cols[je];
            if(jo<U) panel.b_rows[t*32+p*2+1]=cols[jo];
        }
        for(int j=0;j<(int)cols.size();j++) col_map[cols[j]]=-1;
        for(int i=0;i<TILE_R;i++) assigned[avail[i]]=true;
        tile_offset+=nt;
        builds.push_back({std::move(panel),std::move(pnz)});
    }
    pd.total_tiles=tile_offset;
    int64_t A_size=tile_offset*TILE_R*TILE_C;
    pd.A_all=(uint16_t*)aligned_alloc(64,std::max(A_size,(int64_t)1)*sizeof(uint16_t));
    memset(pd.A_all,0,A_size*sizeof(uint16_t));
    pd.amx_nnz=0;
    for(auto&pb:builds){
        uint16_t*base=pd.A_all+pb.panel.A_offset;
        for(auto&nz:pb.packed_nz)
            base[(int64_t)nz.tile_idx*TILE_R*TILE_C+nz.row*TILE_C+nz.tile_col]=nz.bf16_val;
        pd.amx_nnz+=pb.panel.total_deg;
        pd.panels.push_back(std::move(pb.panel));
    }
    for(int i=0;i<M;i++) if(!assigned[i]) pd.avx_rows.push_back(i);
    return pd;
}

// ==================== Gather helper (one pass = 4 kc chunks fused) ====================
static inline void gather_fused_4kc(const uint16_t*B_bf16, const int*br, int kc_base,
    uint8_t*Bb0, uint8_t*Bb1, uint8_t*Bb2, uint8_t*Bb3) {
    for(int p=0;p<16;p++){
        int re=br[p*2],ro=br[p*2+1];
        uint32_t*p0=(uint32_t*)(Bb0+p*64);
        uint32_t*p1=(uint32_t*)(Bb1+p*64);
        uint32_t*p2=(uint32_t*)(Bb2+p*64);
        uint32_t*p3=(uint32_t*)(Bb3+p*64);
        if(re>=0&&ro>=0){
            const uint16_t*be=&B_bf16[(int64_t)re*K+kc_base];
            const uint16_t*bo=&B_bf16[(int64_t)ro*K+kc_base];
            __m512i fe0=_mm512_loadu_si512(be);
            __m512i fe1=_mm512_loadu_si512(be+32);
            __m512i fo0=_mm512_loadu_si512(bo);
            __m512i fo1=_mm512_loadu_si512(bo+32);
            __m512i ie,io;
            ie=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fe0));
            io=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fo0));
            _mm512_store_si512((__m512i*)p0,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
            ie=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fe0,1));
            io=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fo0,1));
            _mm512_store_si512((__m512i*)p1,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
            ie=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fe1));
            io=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fo1));
            _mm512_store_si512((__m512i*)p2,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
            ie=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fe1,1));
            io=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fo1,1));
            _mm512_store_si512((__m512i*)p3,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
        } else if(re>=0){
            const uint16_t*be=&B_bf16[(int64_t)re*K+kc_base];
            __m512i fe0=_mm512_loadu_si512(be);
            __m512i fe1=_mm512_loadu_si512(be+32);
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

// K=32 specialization: 1 pass of 2 kc (simpler, uses 256-bit gather)
static inline void gather_fused_2kc(const uint16_t*B_bf16, const int*br,
    uint8_t*Bb0, uint8_t*Bb1) {
    for(int p=0;p<16;p++){
        int re=br[p*2],ro=br[p*2+1];
        uint32_t*p0=(uint32_t*)(Bb0+p*64);
        uint32_t*p1=(uint32_t*)(Bb1+p*64);
        if(re>=0&&ro>=0){
            __m512i fe=_mm512_loadu_si512(&B_bf16[(int64_t)re*K]);
            __m512i fo=_mm512_loadu_si512(&B_bf16[(int64_t)ro*K]);
            __m512i ie,io;
            ie=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fe));
            io=_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fo));
            _mm512_store_si512((__m512i*)p0,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
            ie=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fe,1));
            io=_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fo,1));
            _mm512_store_si512((__m512i*)p1,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
        } else if(re>=0){
            __m512i fe=_mm512_loadu_si512(&B_bf16[(int64_t)re*K]);
            _mm512_store_si512((__m512i*)p0,_mm512_cvtepu16_epi32(_mm512_castsi512_si256(fe)));
            _mm512_store_si512((__m512i*)p1,_mm512_cvtepu16_epi32(_mm512_extracti64x4_epi64(fe,1)));
        } else {
            __m512i z=_mm512_setzero_si512();
            _mm512_store_si512((__m512i*)p0,z);_mm512_store_si512((__m512i*)p1,z);
        }
    }
}

// ==================== V12 KC-Fusion kernel (generic K) ====================
void amx_v12(const uint16_t*B_bf16, float*C, const PrecompData&pd) {
    int np=(int)pd.panels.size();
    #pragma omp parallel
    {
        syscall(SYS_arch_prctl,0x1023,18);
        tile_config_t cfg;memset(&cfg,0,64);cfg.palette_id=1;
        cfg.rows[0]=16;cfg.colsb[0]=KC*4; // tmm0: C acc 0
        cfg.rows[1]=16;cfg.colsb[1]=64;    // tmm1: A
        cfg.rows[2]=16;cfg.colsb[2]=KC*4;  // tmm2: B
        cfg.rows[3]=16;cfg.colsb[3]=KC*4;  // tmm3: C acc 1
        cfg.rows[4]=16;cfg.colsb[4]=KC*4;  // tmm4: C acc 2
        cfg.rows[5]=16;cfg.colsb[5]=KC*4;  // tmm5: C acc 3
        _tile_loadconfig(&cfg);

        alignas(64) uint8_t Bb0[1024],Bb1[1024],Bb2[1024],Bb3[1024];
        alignas(64) float Cb0[16][KC],Cb1[16][KC],Cb2[16][KC],Cb3[16][KC];

        #pragma omp for schedule(dynamic,8)
        for(int pi=0;pi<np;pi++){
            const Panel&P=pd.panels[pi];
            const uint16_t*preA=pd.A_all+P.A_offset;

            // Process in passes of 4 kc chunks (64 bf16 elements = 128 bytes = 2 cache lines)
            constexpr int KC_PER_PASS = 4;
            constexpr int N_PASSES = (N_KC + KC_PER_PASS - 1) / KC_PER_PASS;

            for(int pass=0; pass<N_PASSES; pass++){
                int kc_base = pass * KC_PER_PASS * KC;
                int kc_count = std::min(KC_PER_PASS, N_KC - pass*KC_PER_PASS);

                _tile_zero(0);
                if(kc_count>=2) _tile_zero(3);
                if(kc_count>=3) _tile_zero(4);
                if(kc_count>=4) _tile_zero(5);

                for(int t=0;t<P.ntiles;t++){
                    const int*br=&P.b_rows[t*32];

                    if(kc_count==2){
                        gather_fused_2kc(B_bf16+kc_base,br,Bb0,Bb1);
                    } else {
                        gather_fused_4kc(B_bf16,br,kc_base,Bb0,Bb1,Bb2,Bb3);
                    }

                    _tile_loadd(1,preA+(size_t)t*TILE_R*TILE_C,64);
                    _tile_loadd(2,Bb0,64); _tile_dpbf16ps(0,1,2);
                    if(kc_count>=2){_tile_loadd(2,Bb1,64); _tile_dpbf16ps(3,1,2);}
                    if(kc_count>=3){_tile_loadd(2,Bb2,64); _tile_dpbf16ps(4,1,2);}
                    if(kc_count>=4){_tile_loadd(2,Bb3,64); _tile_dpbf16ps(5,1,2);}
                }

                _tile_stored(0,Cb0,KC*4);
                if(kc_count>=2) _tile_stored(3,Cb1,KC*4);
                if(kc_count>=3) _tile_stored(4,Cb2,KC*4);
                if(kc_count>=4) _tile_stored(5,Cb3,KC*4);

                for(int i=0;i<TILE_R;i++){
                    float*dst=C+(int64_t)P.rows[i]*K+kc_base;
                    for(int k=0;k<KC;k++) dst[k]=Cb0[i][k];
                    if(kc_count>=2) for(int k=0;k<KC;k++) dst[k+KC]=Cb1[i][k];
                    if(kc_count>=3) for(int k=0;k<KC;k++) dst[k+2*KC]=Cb2[i][k];
                    if(kc_count>=4) for(int k=0;k<KC;k++) dst[k+3*KC]=Cb3[i][k];
                }
            }
        }
        _tile_release();
    }
}

// ==================== AVX-512 (generic K) ====================
void avx512_all(const CSR&csr, const float*B, float*C) {
    constexpr int NVEC = K/16;
    #pragma omp parallel for schedule(dynamic,256)
    for(int i=0;i<csr.M;i++){
        float*Cr=C+(int64_t)i*K;
        __m512 c[NVEC]; for(int v=0;v<NVEC;v++) c[v]=_mm512_setzero_ps();
        for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
            __m512 a=_mm512_set1_ps(csr.values[p]);
            const float*Br=B+(int64_t)csr.indices[p]*K;
            for(int v=0;v<NVEC;v++) c[v]=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+v*16),c[v]);
        }
        for(int v=0;v<NVEC;v++) _mm512_storeu_ps(Cr+v*16,c[v]);
    }
}
void avx512_rows(const CSR&csr, const float*B, float*C, const std::vector<int>&rows) {
    constexpr int NVEC = K/16;
    int nr=(int)rows.size();
    #pragma omp parallel for schedule(dynamic,256)
    for(int idx=0;idx<nr;idx++){
        int i=rows[idx]; float*Cr=C+(int64_t)i*K;
        __m512 c[NVEC]; for(int v=0;v<NVEC;v++) c[v]=_mm512_setzero_ps();
        for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
            __m512 a=_mm512_set1_ps(csr.values[p]);
            const float*Br=B+(int64_t)csr.indices[p]*K;
            for(int v=0;v<NVEC;v++) c[v]=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+v*16),c[v]);
        }
        for(int v=0;v<NVEC;v++) _mm512_storeu_ps(Cr+v*16,c[v]);
    }
}

struct RunResult {
    double amx_ms, fb_ms, e2e_ms;
};

RunResult benchmark(const CSR&csr, const uint16_t*B_bf16, const float*B, float*C,
                    const PrecompData&pd, int nwarmup, int nruns) {
    // Warmup
    for(int t=0;t<nwarmup;t++){
        memset(C,0,(size_t)csr.M*K*sizeof(float));
        amx_v12(B_bf16,C,pd);
        avx512_rows(csr,B,C,pd.avx_rows);
    }
    RunResult best={1e9,1e9,1e9};
    for(int t=0;t<nruns;t++){
        memset(C,0,(size_t)csr.M*K*sizeof(float));
        auto T0=std::chrono::high_resolution_clock::now();
        amx_v12(B_bf16,C,pd);
        auto T1=std::chrono::high_resolution_clock::now();
        avx512_rows(csr,B,C,pd.avx_rows);
        auto T2=std::chrono::high_resolution_clock::now();
        double a=std::chrono::duration<double,std::milli>(T1-T0).count();
        double f=std::chrono::duration<double,std::milli>(T2-T1).count();
        if(a+f<best.e2e_ms){best.amx_ms=a;best.fb_ms=f;best.e2e_ms=a+f;}
    }
    return best;
}

int main(int argc, char**argv) {
    if(argc<2){printf("Usage: %s <mat> [threads]\n",argv[0]);return 1;}
    int nt=(argc>=3)?atoi(argv[2]):omp_get_num_procs();
    omp_set_num_threads(nt);
    syscall(SYS_arch_prctl,0x1023,18);

    printf("=== V12-Final: KC-Fusion + Adaptive ===\n");
    printf("K=%d threads=%d\n",K,nt);

    CSR csr=read_csrbin(argv[1]);
    double avg_deg=(double)csr.nnz/csr.M;
    printf("M=%d N=%d NNZ=%ld avg_deg=%.1f\n",csr.M,csr.N,csr.nnz,avg_deg);
    CSC csc=build_csc(csr);

    // Build Config A and Config B
    printf("\n[Build Config A (ascending/light)]...\n");
    auto T0=std::chrono::high_resolution_clock::now();
    PrecompData pdA=build_all(csr,csc,true);
    auto T1=std::chrono::high_resolution_clock::now();
    printf("  Panels=%d Tiles=%ld (%.1fMB) AMX_NNZ=%.1f%% AVX_rows=%d prep=%.2fs\n",
        (int)pdA.panels.size(),pdA.total_tiles,(double)pdA.total_tiles*TILE_R*TILE_C*2/1e6,
        100.0*pdA.amx_nnz/csr.nnz,(int)pdA.avx_rows.size(),
        std::chrono::duration<double>(T1-T0).count());

    printf("[Build Config B (descending/heavy)]...\n");
    T0=std::chrono::high_resolution_clock::now();
    PrecompData pdB=build_all(csr,csc,false);
    T1=std::chrono::high_resolution_clock::now();
    printf("  Panels=%d Tiles=%ld (%.1fMB) AMX_NNZ=%.1f%% AVX_rows=%d prep=%.2fs\n",
        (int)pdB.panels.size(),pdB.total_tiles,(double)pdB.total_tiles*TILE_R*TILE_C*2/1e6,
        100.0*pdB.amx_nnz/csr.nnz,(int)pdB.avx_rows.size(),
        std::chrono::duration<double>(T1-T0).count());

    // Allocate B and C
    float*B=(float*)aligned_alloc(64,(size_t)csr.N*K*sizeof(float));
    srand(12345);
    for(int64_t i=0;i<(int64_t)csr.N*K;i++) B[i]=(rand()%200-100)/100.0f;
    uint16_t*B_bf16=(uint16_t*)aligned_alloc(64,(size_t)csr.N*K*sizeof(uint16_t));
    for(int64_t i=0;i<(int64_t)csr.N*K;i++) B_bf16[i]=f32_to_bf16(B[i]);
    float*C_ref=(float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));
    float*C=(float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));

    // AVX-512 baseline
    printf("\n[AVX-512]...\n");
    avx512_all(csr,B,C);
    double best_avx=1e9;
    for(int t=0;t<5;t++){
        T0=std::chrono::high_resolution_clock::now();
        avx512_all(csr,B,C);
        T1=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(T1-T0).count();
        if(ms<best_avx) best_avx=ms;
    }
    printf("  Best: %.2f ms\n",best_avx);
    memcpy(C_ref,C,(size_t)csr.M*K*sizeof(float));

    // Config A
    printf("\n[V12 Config A]...\n");
    RunResult rA=benchmark(csr,B_bf16,B,C,pdA,1,5);
    printf("  Best: amx=%.2f fb=%.2f e2e=%.2f (%.2fx)\n",rA.amx_ms,rA.fb_ms,rA.e2e_ms,best_avx/rA.e2e_ms);

    // Config B
    printf("[V12 Config B]...\n");
    RunResult rB=benchmark(csr,B_bf16,B,C,pdB,1,5);
    printf("  Best: amx=%.2f fb=%.2f e2e=%.2f (%.2fx)\n",rB.amx_ms,rB.fb_ms,rB.e2e_ms,best_avx/rB.e2e_ms);

    // Pick best
    bool useA = rA.e2e_ms <= rB.e2e_ms;
    RunResult best = useA ? rA : rB;
    const char* winner = useA ? "A(light)" : "B(heavy)";

    // Correctness check with winner config
    PrecompData& pdW = useA ? pdA : pdB;
    memset(C,0,(size_t)csr.M*K*sizeof(float));
    amx_v12(B_bf16,C,pdW);
    avx512_rows(csr,B,C,pdW.avx_rows);
    int bad=0;float mx=0;
    for(int64_t i=0;i<(int64_t)csr.M*K;i++){
        float ae=fabsf(C_ref[i]-C[i]);if(ae>mx)mx=ae;
        float re=(fabsf(C_ref[i])>1e-6f)?ae/fabsf(C_ref[i]):ae;
        if(re>0.05f&&fabsf(C_ref[i])>0.01f) bad++;
    }
    double bad_pct=100.0*bad/((double)csr.M*K);

    printf("\n========== RESULT ==========\n");
    printf("AVX-512:     %.2f ms\n",best_avx);
    printf("V12-A:       %.2f ms (%.2fx) [amx=%.2f fb=%.2f]\n",rA.e2e_ms,best_avx/rA.e2e_ms,rA.amx_ms,rA.fb_ms);
    printf("V12-B:       %.2f ms (%.2fx) [amx=%.2f fb=%.2f]\n",rB.e2e_ms,best_avx/rB.e2e_ms,rB.amx_ms,rB.fb_ms);
    printf("Winner:      %s → %.2f ms (%.2fx)\n",winner,best.e2e_ms,best_avx/best.e2e_ms);
    printf("Correct:     max_abs=%.3e bad=%.4f%% %s\n",mx,bad_pct,bad_pct<1.0?"OK":"FAIL");

    free(B);free(B_bf16);free(C);free(C_ref);free(pdA.A_all);free(pdB.A_all);
    return 0;
}
