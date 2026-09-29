#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cmath>
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
    int rows[TILE_R]; int nrows, U, ntiles, total_deg;
    std::vector<PackedNZ> packed_nz;
    std::vector<int> b_rows;
};

void build_panels(const CSR& csr, const CSC& csc,
    std::vector<Panel>& panels, std::vector<int>& avx_rows)
{
    int M=csr.M, N=csr.N;
    std::vector<int> deg(M);
    for(int i=0;i<M;i++) deg[i]=csr.indptr[i+1]-csr.indptr[i];
    std::vector<int> col_order;
    for(int j=0;j<N;j++) if(csc.col_counts[j]>=TILE_R) col_order.push_back(j);
    std::sort(col_order.begin(),col_order.end(),
        [&](int a,int b){return csc.col_counts[a]>csc.col_counts[b];});
    std::vector<bool> assigned(M,false);
    std::vector<int> col_map(N,-1);

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
        Panel panel; panel.nrows=TILE_R; panel.U=U; panel.ntiles=nt; panel.total_deg=td;
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
        panels.push_back(std::move(panel));
    }
    for(int i=0;i<M;i++) if(!assigned[i]) avx_rows.push_back(i);
}

// AMX kernel — K 参数化
void amx_kernel(const float* B, float* C, const std::vector<Panel>& panels, int K_val) {
    int np=(int)panels.size();
    #pragma omp parallel
    {
        syscall(SYS_arch_prctl, 0x1023, 18);
        tile_config_t cfg; memset(&cfg,0,64);
        cfg.palette_id=1;
        cfg.rows[0]=16; cfg.colsb[0]=KC*4;
        cfg.rows[1]=16; cfg.colsb[1]=64;
        cfg.rows[2]=16; cfg.colsb[2]=KC*4;
        _tile_loadconfig(&cfg);

        alignas(64) uint16_t At[MAX_NTILES][16][32];
        alignas(64) uint8_t Bb[16*64];
        alignas(64) float Cb[16][KC];

        #pragma omp for schedule(dynamic,8)
        for(int pi=0;pi<np;pi++){
            const Panel& P=panels[pi];
            memset(At,0,(size_t)P.ntiles*sizeof(At[0]));
            for(auto&nz:P.packed_nz)
                At[nz.tile_idx][nz.row][nz.tile_col]=nz.bf16_val;

            for(int kc=0;kc<K_val;kc+=KC){
                _tile_zero(0);
                for(int t=0;t<P.ntiles;t++){
                    const int*br=&P.b_rows[t*32];
                    for(int p=0;p<16;p++){
                        int re=br[p*2],ro=br[p*2+1];
                        uint32_t*pr=(uint32_t*)(Bb+p*64);
                        if(re>=0&&ro>=0){
                            __m512 ve=_mm512_loadu_ps(&B[(int64_t)re*K_val+kc]);
                            __m512 vo=_mm512_loadu_ps(&B[(int64_t)ro*K_val+kc]);
                            __m512i ie=_mm512_srli_epi32(_mm512_castps_si512(ve),16);
                            __m512i io=_mm512_srli_epi32(_mm512_castps_si512(vo),16);
                            _mm512_store_si512((__m512i*)pr,_mm512_or_si512(ie,_mm512_slli_epi32(io,16)));
                        } else if(re>=0){
                            __m512 ve=_mm512_loadu_ps(&B[(int64_t)re*K_val+kc]);
                            _mm512_store_si512((__m512i*)pr,_mm512_srli_epi32(_mm512_castps_si512(ve),16));
                        } else {
                            _mm512_store_si512((__m512i*)pr,_mm512_setzero_si512());
                        }
                    }
                    _tile_loadd(1,At[t],64);
                    _tile_loadd(2,Bb,64);
                    _tile_dpbf16ps(0,1,2);
                }
                _tile_stored(0,Cb,KC*4);
                for(int i=0;i<TILE_R;i++){
                    float*dst=C+(int64_t)P.rows[i]*K_val+kc;
                    for(int k=0;k<KC;k++) dst[k]=Cb[i][k];
                }
            }
        }
        _tile_release();
    }
}

// AVX-512 kernel — K 参数化
void avx_kernel(const CSR& csr, const float* B, float* C,
                const std::vector<int>& rows, int K_val) {
    int nr=(int)rows.size();
    int nregs=K_val/16;
    #pragma omp parallel for schedule(dynamic,256)
    for(int idx=0;idx<nr;idx++){
        int i=rows[idx];
        float*Cr=C+(int64_t)i*K_val;
        __m512 c[16]; // max K=256 → 16 regs
        for(int r=0;r<nregs;r++) c[r]=_mm512_setzero_ps();
        for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
            __m512 a=_mm512_set1_ps(csr.values[p]);
            const float*Br=B+(int64_t)csr.indices[p]*K_val;
            for(int r=0;r<nregs;r++)
                c[r]=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+r*16),c[r]);
        }
        for(int r=0;r<nregs;r++) _mm512_storeu_ps(Cr+r*16,c[r]);
    }
}

int main(int argc, char** argv) {
    if(argc<2){printf("Usage: %s <mat> [threads]\n",argv[0]);return 1;}
    int nt=(argc>=3)?atoi(argv[2]):omp_get_num_procs();
    omp_set_num_threads(nt);
    syscall(SYS_arch_prctl,0x1023,18);

    CSR csr=read_csrbin(argv[1]);
    printf("Matrix: M=%d NNZ=%ld avg=%.1f  Threads=%d\n",
        csr.M,csr.nnz,(double)csr.nnz/csr.M,nt);

    CSC csc=build_csc(csr);
    std::vector<Panel> panels; std::vector<int> avx_rows;
    auto t0=std::chrono::high_resolution_clock::now();
    build_panels(csr,csc,panels,avx_rows);
    auto t1=std::chrono::high_resolution_clock::now();
    int64_t amx_nnz=0;
    for(auto&p:panels) amx_nnz+=p.total_deg;
    printf("Panels: %d, AMX NNZ: %.1f%%, Grouping: %.1fs\n\n",
        (int)panels.size(), 100.0*amx_nnz/csr.nnz,
        std::chrono::duration<double>(t1-t0).count());

    // Test K = 32, 64, 128, 256
    int K_vals[] = {32, 64, 128, 256};

    printf("%-6s %10s %10s %10s %8s\n", "K", "AMX(ms)", "AVX(ms)", "Total(ms)", "AMX/AVX");
    printf("------ ---------- ---------- ---------- --------\n");

    for (int K_val : K_vals) {
        float* B = (float*)aligned_alloc(64, (size_t)csr.N * K_val * sizeof(float));
        float* C1 = (float*)aligned_alloc(64, (size_t)csr.M * K_val * sizeof(float));
        float* C2 = (float*)aligned_alloc(64, (size_t)csr.M * K_val * sizeof(float));
        srand(12345);
        for(int64_t i=0;i<(int64_t)csr.N*K_val;i++) B[i]=(rand()%200-100)/100.0f;

        // warmup
        memset(C1,0,(size_t)csr.M*K_val*sizeof(float));
        amx_kernel(B,C1,panels,K_val);
        avx_kernel(csr,B,C1,avx_rows,K_val);

        // AMX + fallback: 3 trials
        double best_amx_t=1e9;
        for(int t=0;t<3;t++){
            memset(C1,0,(size_t)csr.M*K_val*sizeof(float));
            t0=std::chrono::high_resolution_clock::now();
            amx_kernel(B,C1,panels,K_val);
            avx_kernel(csr,B,C1,avx_rows,K_val);
            t1=std::chrono::high_resolution_clock::now();
            double ms=std::chrono::duration<double,std::milli>(t1-t0).count();
            if(ms<best_amx_t) best_amx_t=ms;
        }

        // Pure AVX-512: 3 trials
        std::vector<int> all(csr.M); std::iota(all.begin(),all.end(),0);
        avx_kernel(csr,B,C2,all,K_val); // warmup
        double best_avx_t=1e9;
        for(int t=0;t<3;t++){
            t0=std::chrono::high_resolution_clock::now();
            avx_kernel(csr,B,C2,all,K_val);
            t1=std::chrono::high_resolution_clock::now();
            double ms=std::chrono::duration<double,std::milli>(t1-t0).count();
            if(ms<best_avx_t) best_avx_t=ms;
        }

        printf("%-6d %10.1f %10.1f %10.1f %7.2fx\n",
            K_val, best_amx_t, best_avx_t, best_amx_t, best_avx_t/best_amx_t);

        free(B); free(C1); free(C2);
    }
    printf("\n=== Done ===\n");
    return 0;
}
