#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <vector>
#include <chrono>
#include <random>
#include <cstdint>
#include <immintrin.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <omp.h>
using namespace std;
using hrc = chrono::high_resolution_clock;

#define TMM_C0 0
#define TMM_C1 1
#define TMM_A  2
#define TMM_B0 3
#define TMM_B1 4

static inline uint16_t f32_to_bf16_u16(float f) {
    uint32_t u; memcpy(&u, &f, 4); return (uint16_t)(u >> 16);
}
static inline float bf16_u16_to_f32(uint16_t h) {
    uint32_t u = (uint32_t)h << 16; float f; memcpy(&f, &u, 4); return f;
}

struct alignas(64) TileCfg {
    uint8_t  palette_id, start_row, reserved[14];
    uint16_t colsb[16];
    uint8_t  rows[16];
};
static void amx_enable() {
    if (syscall(SYS_arch_prctl, 0x1023, 18) != 0) {
        perror("arch_prctl"); exit(1);
    }
}
static void amx_init_tiles() {
    TileCfg cfg{};
    cfg.palette_id = 1;
    for (int t : {0,1,2,3,4}) { cfg.rows[t]=16; cfg.colsb[t]=64; }
    _tile_loadconfig(&cfg);
}

struct CSR { int nrow,ncol,nnz; uint32_t *indptr,*indices; };
static CSR load_csr(const char* path) {
    FILE* f = fopen(path,"rb");
    if (!f) { fprintf(stderr,"无法打开 %s\n",path); exit(1); }
    uint32_t hdr[3]; fread(hdr,4,3,f);
    uint64_t dim[3]; fread(dim,8,3,f);
    CSR m; m.nrow=(int)dim[0]; m.ncol=(int)dim[1]; m.nnz=(int)dim[2];
    m.indptr  = (uint32_t*)malloc((m.nrow+1)*4);
    m.indices = (uint32_t*)malloc(m.nnz*4);
    fread(m.indptr,4,m.nrow+1,f);
    fread(m.indices,4,m.nnz,f);
    fclose(f); return m;
}

// W[K_in, K_out] BF16 → VNNI格式 [KB][NB][16 kpair][32 BF16]
// 用 memcpy 避免 icpx __bf16 直接赋值的类型转换 bug
static uint16_t* make_W_vnni(const uint16_t* W, int K_in, int K_out) {
    int KB = K_in/32, NB = K_out/16;
    uint16_t* Wv = (uint16_t*)aligned_alloc(64, (size_t)KB*NB*16*32*2);
    for (int kb=0; kb<KB; kb++)
        for (int ob=0; ob<NB; ob++) {
            uint16_t* dst = Wv + (kb*NB+ob)*16*32;
            for (int kp=0; kp<16; kp++) {
                int k0=kb*32+kp*2, k1=k0+1;
                for (int n=0; n<16; n++) {
                    uint16_t v0, v1;
                    memcpy(&v0, &W[k0*K_out+ob*16+n], 2);
                    memcpy(&v1, &W[k1*K_out+ob*16+n], 2);
                    memcpy(&dst[kp*32+n*2+0], &v0, 2);
                    memcpy(&dst[kp*32+n*2+1], &v1, 2);
                }
            }
        }
    return Wv;
}

static void tfs_fusion(const CSR& A, const uint16_t* H, const uint16_t* Wv,
                       float* Cout, int Kin=128, int Kout=128, int R=128) {
    const int KB=Kin/32, NB=Kout/16, NP=NB/2;
    const int Crbytes = Kout*4;
    alignas(64) static const uint16_t zero_row[128]={};

    #pragma omp parallel
    {
        amx_enable();
        amx_init_tiles();
        alignas(64) uint16_t Abuf[16*32];
        float* Cg = (float*)aligned_alloc(64, (size_t)R*Kout*4+64);

        #pragma omp for schedule(dynamic,1)
        for (int rg=0; rg<A.nrow; rg+=R) {
            int rend = min(rg+R, A.nrow);
            int rlen = rend-rg;

            for (int obp=0; obp<NP; obp++) {
                int ob0=obp*2, ob1=ob0+1;

                for (int i=rg; i<rend; i+=16) {
                    int bs = min(i+16, rend)-i;

                    _tile_zero(TMM_C0);
                    _tile_zero(TMM_C1);

                    int maxd=0;
                    for (int r=0; r<bs; r++) {
                        int d=(int)(A.indptr[i+r+1]-A.indptr[i+r]);
                        if (d>maxd) maxd=d;
                    }

                    for (int kb=0; kb<KB; kb++) {
                        const uint16_t* B0 = Wv+(kb*NB+ob0)*16*32;
                        const uint16_t* B1 = Wv+(kb*NB+ob1)*16*32;
                        _tile_loadd(TMM_B0, B0, 64);
                        _tile_loadd(TMM_B1, B1, 64);

                        for (int step=0; step<maxd; step++) {
                            for (int r=0; r<16; r++) {
                                const uint16_t* src = zero_row;
                                if (r<bs) {
                                    uint32_t p0  = A.indptr[i+r];
                                    uint32_t deg = A.indptr[i+r+1]-p0;
                                    if ((uint32_t)step<deg)
                                        src = H+(int64_t)A.indices[p0+step]*Kin+kb*32;
                                }
                                __m512i v=_mm512_loadu_si512((const __m512i*)src);
                                _mm512_store_si512((__m512i*)(Abuf+r*32),v);
                            }
                            _tile_loadd(TMM_A, Abuf, 64);
                            _tile_dpbf16ps(TMM_C0, TMM_A, TMM_B0);
                            _tile_dpbf16ps(TMM_C1, TMM_A, TMM_B1);
                        }
                    }
                    int li = i-rg;
                    _tile_stored(TMM_C0, Cg+(int64_t)li*Kout+ob0*16, Crbytes);
                    _tile_stored(TMM_C1, Cg+(int64_t)li*Kout+ob1*16, Crbytes);
                }
            }
            memcpy(Cout+(int64_t)rg*Kout, Cg, (size_t)rlen*Crbytes);
        }
        free(Cg);
        _tile_release();
    }
}

static void ref_twostep(const CSR& A, const uint16_t* H, const uint16_t* W,
                        float* Cout, int Kin=128, int Kout=128) {
    vector<float> Z((int64_t)A.nrow*Kin, 0.f);
    #pragma omp parallel for schedule(dynamic,64)
    for (int i=0; i<A.nrow; i++) {
        float* zi=Z.data()+(int64_t)i*Kin;
        for (uint32_t p=A.indptr[i]; p<A.indptr[i+1]; p++) {
            const uint16_t* hj=H+(int64_t)A.indices[p]*Kin;
            for (int k=0; k<Kin; k++) zi[k]+=bf16_u16_to_f32(hj[k]);
        }
    }
    #pragma omp parallel for schedule(static)
    for (int i=0; i<A.nrow; i++) {
        float* ci=Cout+(int64_t)i*Kout;
        float* zi=Z.data()+(int64_t)i*Kin;
        for (int n=0; n<Kout; n++) ci[n]=0.f;
        for (int k=0; k<Kin; k++) {
            float zk=zi[k];
            const uint16_t* wk=W+(int64_t)k*Kout;
            for (int n=0; n<Kout; n++) ci[n]+=zk*bf16_u16_to_f32(wk[n]);
        }
    }
}

static void verify(const float* a, const float* b, int64_t n) {
    double err=0, ref=0;
    for (int64_t i=0; i<n; i++) {
        double d=a[i]-b[i]; err+=d*d; ref+=(double)b[i]*b[i];
    }
    float rel=(float)(sqrt(err)/(sqrt(ref)+1e-8));
    printf("  相对误差=%.6f  %s\n", rel, rel<0.02f?"✅ PASS":"❌ FAIL");
}
static double now_ms() {
    return chrono::duration<double,milli>(hrc::now().time_since_epoch()).count();
}

int main(int argc, char* argv[]) {
    if (argc<2) {
        fprintf(stderr,"用法: %s <matrix.csrbin> [K=128] [R=128] [warmup=5] [repeat=20]\n",argv[0]);
        return 1;
    }
    int K=argc>2?atoi(argv[2]):128;
    int R=argc>3?atoi(argv[3]):128;
    int warmup=argc>4?atoi(argv[4]):5;
    int repeat=argc>5?atoi(argv[5]):20;
    if (K!=128) { fprintf(stderr,"当前仅支持K=128\n"); return 1; }

    int thr=omp_get_max_threads();
    printf("=================================================================\n");
    printf("TFS Fusion  K=%d  R=%d  threads=%d  warmup=%d  repeat=%d\n",K,R,thr,warmup,repeat);

    CSR A=load_csr(argv[1]);
    printf("矩阵=%s\nnrow=%d  ncol=%d  nnz=%d  avg_deg=%.1f\n",
           argv[1],A.nrow,A.ncol,A.nnz,(float)A.nnz/A.nrow);

    int64_t Hn=(int64_t)A.ncol*K, Wn=(int64_t)K*K;
    uint16_t* H=(uint16_t*)aligned_alloc(64,Hn*2);
    uint16_t* W=(uint16_t*)aligned_alloc(64,Wn*2);
    { mt19937 rng(42); normal_distribution<float> nd(0.f,0.02f);
      for (int64_t i=0; i<Hn; i++) H[i]=f32_to_bf16_u16(nd(rng));
      for (int64_t i=0; i<Wn; i++) W[i]=f32_to_bf16_u16(nd(rng)); }

    uint16_t* Wv=make_W_vnni(W,K,K);
    int64_t Cn=(int64_t)A.nrow*K;
    float* Ctfs=(float*)aligned_alloc(64,Cn*4);
    float* Cref=(float*)aligned_alloc(64,Cn*4);
    memset(Ctfs,0,Cn*4); memset(Cref,0,Cn*4);

    if (A.nrow<=1000000) {
        printf("\n--- 正确性验证 ---\n");
        ref_twostep(A,H,W,Cref,K,K);
        tfs_fusion(A,H,Wv,Ctfs,K,K,R);
        verify(Ctfs,Cref,Cn);
        memset(Ctfs,0,Cn*4);
    }

    printf("\n--- TFS Fusion 计时 ---\n");
    for (int i=0;i<warmup;i++){memset(Ctfs,0,Cn*4);tfs_fusion(A,H,Wv,Ctfs,K,K,R);}
    vector<double> tt(repeat);
    for (int i=0;i<repeat;i++){memset(Ctfs,0,Cn*4);double t0=now_ms();tfs_fusion(A,H,Wv,Ctfs,K,K,R);tt[i]=now_ms()-t0;}
    sort(tt.begin(),tt.end());

    printf("--- 两步法 Ref 计时 ---\n");
    for (int i=0;i<warmup;i++){memset(Cref,0,Cn*4);ref_twostep(A,H,W,Cref,K,K);}
    vector<double> tr(repeat);
    for (int i=0;i<repeat;i++){memset(Cref,0,Cn*4);double t0=now_ms();ref_twostep(A,H,W,Cref,K,K);tr[i]=now_ms()-t0;}
    sort(tr.begin(),tr.end());

    printf("\n=== 结果 ===\n");
    printf("%-12s  %10s  %10s  %8s\n","","TFS Fusion","Ref两步法","加速比");
    printf("%-12s  %10.2f  %10.2f  %8.2f×\n","min(ms)",   tt[0],        tr[0],        tr[0]/tt[0]);
    printf("%-12s  %10.2f  %10.2f  %8.2f×\n","median(ms)",tt[repeat/2], tr[repeat/2], tr[repeat/2]/tt[repeat/2]);
    printf("注：Ref=朴素两步法，真实基线(FB-BF16+GeMM)见bench_gemm结果\n");

    free(H);free(W);free(Wv);free(Ctfs);free(Cref);free(A.indptr);free(A.indices);
    return 0;
}
