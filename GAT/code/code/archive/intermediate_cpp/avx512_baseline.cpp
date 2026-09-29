#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <chrono>
#include <vector>
#include <immintrin.h>
#include <omp.h>

constexpr int K = 32;
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
int main(int argc, char** argv) {
    if (argc < 2) return 1;
    int nt = (argc>=3) ? atoi(argv[2]) : omp_get_num_procs();
    omp_set_num_threads(nt);
    CSR csr = read_csrbin(argv[1]);
    printf("  %s: M=%d NNZ=%ld avg=%.1f\n", argv[1], csr.M, csr.nnz, (double)csr.nnz/csr.M);
    float* B = (float*)aligned_alloc(64,(size_t)csr.N*K*sizeof(float));
    float* C = (float*)aligned_alloc(64,(size_t)csr.M*K*sizeof(float));
    srand(12345);
    for(int64_t i=0;i<(int64_t)csr.N*K;i++) B[i]=(rand()%200-100)/100.0f;

    // warmup
    #pragma omp parallel for schedule(dynamic,64)
    for(int i=0;i<csr.M;i++){
        float*Cr=C+(int64_t)i*K;
        __m512 c0=_mm512_setzero_ps(),c1=_mm512_setzero_ps();
        for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
            __m512 a=_mm512_set1_ps(csr.values[p]);
            const float*Br=B+(int64_t)csr.indices[p]*K;
            c0=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br),c0);
            c1=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+16),c1);
        }
        _mm512_storeu_ps(Cr,c0); _mm512_storeu_ps(Cr+16,c1);
    }

    // 3 trials
    double best = 1e9;
    for(int t=0;t<3;t++){
        auto t0=std::chrono::high_resolution_clock::now();
        #pragma omp parallel for schedule(dynamic,64)
        for(int i=0;i<csr.M;i++){
            float*Cr=C+(int64_t)i*K;
            __m512 c0=_mm512_setzero_ps(),c1=_mm512_setzero_ps();
            for(uint32_t p=csr.indptr[i];p<csr.indptr[i+1];p++){
                __m512 a=_mm512_set1_ps(csr.values[p]);
                const float*Br=B+(int64_t)csr.indices[p]*K;
                c0=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br),c0);
                c1=_mm512_fmadd_ps(a,_mm512_loadu_ps(Br+16),c1);
            }
            _mm512_storeu_ps(Cr,c0); _mm512_storeu_ps(Cr+16,c1);
        }
        auto t1=std::chrono::high_resolution_clock::now();
        double ms=std::chrono::duration<double,std::milli>(t1-t0).count();
        if(ms<best)best=ms;
    }
    printf("  AVX-512 %dT best: %.2f ms\n", nt, best);
    free(B); free(C); return 0;
}
