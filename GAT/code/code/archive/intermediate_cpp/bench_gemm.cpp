#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <immintrin.h>
#include <omp.h>
using namespace std;
using hrc = chrono::high_resolution_clock;
static double now_ms(){
    return chrono::duration<double,milli>(hrc::now().time_since_epoch()).count();
}
struct CSR{int nrow,ncol,nnz;uint32_t *indptr,*indices;};
static CSR load_csr(const char* path){
    FILE* f=fopen(path,"rb");if(!f){fprintf(stderr,"cant open %s\n",path);exit(1);}
    uint32_t hdr[3];fread(hdr,4,3,f);
    uint64_t dim[3];fread(dim,8,3,f);
    CSR m;m.nrow=(int)dim[0];m.ncol=(int)dim[1];m.nnz=(int)dim[2];
    m.indptr=(uint32_t*)malloc((m.nrow+1)*4);
    m.indices=(uint32_t*)malloc(m.nnz*4);
    fread(m.indptr,4,m.nrow+1,f);fread(m.indices,4,m.nnz,f);
    fclose(f);return m;
}
static inline uint16_t f32_to_bf16(float f){uint32_t u;memcpy(&u,&f,4);return(uint16_t)(u>>16);}
static void fb_spmm(const CSR& A,const uint16_t* H,float* Z,int K=128){
    #pragma omp parallel for schedule(dynamic,64)
    for(int i=0;i<A.nrow;i++){
        float* zi=Z+(int64_t)i*K;
        for(int k=0;k<K;k++) zi[k]=0.f;
        for(uint32_t p=A.indptr[i];p<A.indptr[i+1];p++){
            const uint16_t* hj=H+(int64_t)A.indices[p]*K;
            for(int k=0;k<K;k+=32){
                __m512i r0=_mm512_loadu_si512(hj+k);
                __m256i lo=_mm512_castsi512_si256(r0);
                __m256i hi=_mm512_extracti32x8_epi32(r0,1);
                __m512 f0=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(lo),16));
                __m512 f1=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(hi),16));
                _mm512_storeu_ps(zi+k,   _mm512_add_ps(_mm512_loadu_ps(zi+k),   f0));
                _mm512_storeu_ps(zi+k+16,_mm512_add_ps(_mm512_loadu_ps(zi+k+16),f1));
            }
        }
    }
}
static void dense_gemm(const float* Z,const float* W,float* C,int nrow,int K=128){
    #pragma omp parallel for schedule(static)
    for(int i=0;i<nrow;i++){
        const float* zi=Z+(int64_t)i*K;
        float*       ci=C+(int64_t)i*K;
        for(int n=0;n<K;n+=16){
            __m512 acc=_mm512_setzero_ps();
            for(int k=0;k<K;k++)
                acc=_mm512_fmadd_ps(_mm512_set1_ps(zi[k]),
                                    _mm512_loadu_ps(W+k*K+n),acc);
            _mm512_storeu_ps(ci+n,acc);
        }
    }
}
int main(int argc,char* argv[]){
    if(argc<2){fprintf(stderr,"用法: %s <matrix.csrbin> [repeat=20]\n",argv[0]);return 1;}
    int repeat=argc>2?atoi(argv[2]):20;
    int K=128,thr=omp_get_max_threads();
    CSR A=load_csr(argv[1]);
    printf("==================================================\n");
    printf("矩阵=%s\nnrow=%d nnz=%d avg_deg=%.1f threads=%d\n",
           argv[1],A.nrow,A.nnz,(float)A.nnz/A.nrow,thr);
    int64_t Hn=(int64_t)A.ncol*K,Wn=(int64_t)K*K,Zn=(int64_t)A.nrow*K;
    uint16_t* H=(uint16_t*)aligned_alloc(64,Hn*2);
    float* W=(float*)aligned_alloc(64,Wn*4);
    float* Z=(float*)aligned_alloc(64,Zn*4);
    float* C=(float*)aligned_alloc(64,Zn*4);
    {mt19937 rng(42);normal_distribution<float> nd(0.f,0.02f);
     for(int64_t i=0;i<Hn;i++) H[i]=f32_to_bf16(nd(rng));
     for(int64_t i=0;i<Wn;i++) W[i]=nd(rng);
     for(int64_t i=0;i<Zn;i++) Z[i]=nd(rng);}
    for(int i=0;i<3;i++){
        memset(Z,0,Zn*4);fb_spmm(A,H,Z,K);
        memset(C,0,Zn*4);dense_gemm(Z,W,C,A.nrow,K);
    }
    vector<double> ts(repeat),tg(repeat),t2(repeat);
    for(int i=0;i<repeat;i++){
        memset(Z,0,Zn*4);
        double t0=now_ms();fb_spmm(A,H,Z,K);ts[i]=now_ms()-t0;
    }
    for(int i=0;i<repeat;i++){
        memset(C,0,Zn*4);
        double t0=now_ms();dense_gemm(Z,W,C,A.nrow,K);tg[i]=now_ms()-t0;
    }
    for(int i=0;i<repeat;i++){
        memset(Z,0,Zn*4);memset(C,0,Zn*4);
        double t0=now_ms();
        fb_spmm(A,H,Z,K);dense_gemm(Z,W,C,A.nrow,K);
        t2[i]=now_ms()-t0;
    }
    sort(ts.begin(),ts.end());
    sort(tg.begin(),tg.end());
    sort(t2.begin(),t2.end());
    printf("FB-BF16 SpMM : min=%.2fms  median=%.2fms\n",ts[0],ts[repeat/2]);
    printf("Dense GeMM   : min=%.2fms  median=%.2fms\n",tg[0],tg[repeat/2]);
    printf("两步法合计   : min=%.2fms  median=%.2fms  ← TFS要打败这个\n",t2[0],t2[repeat/2]);
    printf("==================================================\n");
    free(H);free(W);free(Z);free(C);free(A.indptr);free(A.indices);
    return 0;
}
