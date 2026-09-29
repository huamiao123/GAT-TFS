#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <vector>
#include <numeric>
#include <chrono>
#include <immintrin.h>
#include <omp.h>
constexpr int K=32;
struct CSR {
    std::vector<uint32_t> indptr, indices;
    std::vector<float> values;
    int M,N; int64_t nnz;
};
CSR read_csrbin(const char* p) {
    CSR c; FILE*f=fopen(p,"rb");
    uint32_t h[3];uint64_t d[3];
    fread(h,4,3,f);fread(d,8,3,f);
    c.M=d[0];c.N=d[1];c.nnz=d[2];
    c.indptr.resize(c.M+1);c.indices.resize(c.nnz);c.values.resize(c.nnz);
    fread(c.indptr.data(),4,c.M+1,f);
    fread(c.indices.data(),4,c.nnz,f);
    fread(c.values.data(),4,c.nnz,f);
    fclose(f); return c;
}
int main(int argc,char**argv){
    int nt=atoi(argv[2]); omp_set_num_threads(nt);
    CSR csr=read_csrbin(argv[1]);
    float*B=(float*)aligned_alloc(64,(size_t)csr.N*K*sizeof(float));
    float*C=(float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));
    srand(12345);
    for(int64_t i=0;i<(int64_t)csr.N*K;i++) B[i]=(rand()%200-100)/100.0f;
    std::vector<int> all(csr.M); std::iota(all.begin(),all.end(),0);
    // warmup
    int nr=csr.M;
    #pragma omp parallel for schedule(dynamic,256)
    for(int idx=0;idx<nr;idx++){
        int i=all[idx]; float*Cr=C+(int64_t)i*K;
        __m512 c0=_mm512_setzero_ps(),c1=_mm512_setzero_ps();
        for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
            __m512 a=_mm512_set1_ps(csr.values[p]);
            const float*Br=B+(int64_t)csr.indices[p]*K;
            c0=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br),c0);
            c1=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+16),c1);
        }
        _mm512_storeu_ps(Cr,c0);_mm512_storeu_ps(Cr+16,c1);
    }
    double best=1e9;
    for(int t=0;t<5;t++){
        auto t0=std::chrono::high_resolution_clock::now();
        #pragma omp parallel for schedule(dynamic,256)
        for(int idx=0;idx<nr;idx++){
            int i=all[idx]; float*Cr=C+(int64_t)i*K;
            __m512 c0=_mm512_setzero_ps(),c1=_mm512_setzero_ps();
            for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
                __m512 a=_mm512_set1_ps(csr.values[p]);
                const float*Br=B+(int64_t)csr.indices[p]*K;
                c0=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br),c0);
                c1=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+16),c1);
            }
            _mm512_storeu_ps(Cr,c0);_mm512_storeu_ps(Cr+16,c1);
        }
        auto t1=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(t1-t0).count();
        if(ms<best) best=ms;
    }
    printf("Pure_AVX512: %.2f ms (M=%d NNZ=%ld threads=%d)\n",best,csr.M,csr.nnz,nt);
    free(B);free(C);
}
