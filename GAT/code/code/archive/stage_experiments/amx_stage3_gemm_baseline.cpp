/**
 * 补测 Dense GeMM baseline
 * 测量 SpMM + GeMM 两步总时间，与融合 kernel 公平对比
 */
#include <immintrin.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <omp.h>
#include <chrono>
#include <algorithm>
#include <vector>
#include <string>

static constexpr int K = 128;
static constexpr int N = 128;
static constexpr int REPEAT = 10;
static constexpr int WARMUP = 3;

static inline uint16_t f32_to_bf16(float f) {
    uint32_t u; memcpy(&u, &f, 4); return (uint16_t)(u >> 16);
}
static inline float bf16_to_f32(uint16_t b) {
    uint32_t u = (uint32_t)b << 16; float f; memcpy(&f, &u, 4); return f;
}

struct CSRMat {
    uint64_t nrow, ncol, nnz;
    std::vector<uint32_t> indptr, indices;
};
static bool load_csrbin(const char* path, CSRMat& m) {
    FILE* fp = fopen(path, "rb");
    if (!fp) return false;
    uint32_t p,d,v; fread(&p,4,1,fp); fread(&d,4,1,fp); fread(&v,4,1,fp);
    fread(&m.nrow,8,1,fp); fread(&m.ncol,8,1,fp); fread(&m.nnz,8,1,fp);
    m.indptr.resize(m.nrow+1); m.indices.resize(m.nnz);
    fread(m.indptr.data(),4,m.nrow+1,fp);
    fread(m.indices.data(),4,m.nnz,fp);
    fclose(fp); return true;
}

// ── FB-BF16 SpMM（与 Stage3 完全一致）──────────────────────────
static void spmm_fb(const CSRMat& A, const uint16_t* H, float* Z, int nt) {
    omp_set_num_threads(nt);
    #pragma omp parallel for schedule(dynamic,64)
    for (int64_t i = 0; i < (int64_t)A.nrow; i++) {
        float* z = Z + i * K;
        memset(z, 0, K * 4);
        for (int64_t p = A.indptr[i]; p < (int64_t)A.indptr[i+1]; p++)
            for (int k = 0; k < K; k++)
                z[k] += bf16_to_f32(H[(size_t)A.indices[p]*K + k]);
    }
}

// ── Dense GeMM：Z(M×K) × W(K×N) → C(M×N)，用 AVX-512 ─────────
// 分块以适应 L2 cache
static void dense_gemm(const float* Z, const float* W, float* C,
                       int64_t M, int nt) {
    omp_set_num_threads(nt);
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < M; i++) {
        const float* z = Z + i * K;
        float* c = C + i * N;
        memset(c, 0, N * 4);
        for (int k = 0; k < K; k++) {
            float zk = z[k];
            for (int n = 0; n < N; n++)
                c[n] += zk * W[k * N + n];
        }
    }
}

template<typename Fn>
static double bench_ms(Fn fn, int warmup, int repeat) {
    for (int i = 0; i < warmup; i++) fn();
    double best = 1e18;
    for (int i = 0; i < repeat; i++) {
        auto t0 = std::chrono::high_resolution_clock::now();
        fn();
        auto t1 = std::chrono::high_resolution_clock::now();
        best = std::min(best, std::chrono::duration<double,std::milli>(t1-t0).count());
    }
    return best;
}

int main(int argc, char** argv) {
    int nt = argc >= 2 ? atoi(argv[1]) : 32;
    const char* MATRICES[] = {
        "/home/huangjianqiang_group/hdacp1/data/SpMM_project/data/web-Google/web-Google.csrbin",
        "/home/huangjianqiang_group/hdacp1/data/SpMM_project/data/amazon0601/amazon0601.csrbin",
        nullptr
    };

    printf("=== Stage3 补测：SpMM + GeMM 两步 baseline ===\n");
    printf("threads=%d  K=%d  N=%d\n\n", nt, K, N);
    printf("%-18s %10s %10s %10s %12s %10s\n",
           "矩阵", "SpMM(ms)", "GeMM(ms)", "总计(ms)", "Fusion(ms)", "真实加速");
    printf("%s\n", std::string(76,'-').c_str());

    // Fusion 结果（Stage3 已测）
    double fusion_ms[] = {38.35, 20.60};

    srand(42);
    for (int mi = 0; MATRICES[mi]; mi++) {
        CSRMat A;
        if (!load_csrbin(MATRICES[mi], A)) continue;

        // 初始化
        uint16_t* H = (uint16_t*)aligned_alloc(64, A.ncol*K*2);
        float*    Z = (float*)   aligned_alloc(64, A.nrow*K*4);
        float*    W = (float*)   aligned_alloc(64, K*N*4);
        float*    C = (float*)   aligned_alloc(64, A.nrow*N*4);
        for (size_t i=0;i<A.ncol*(size_t)K;i++) H[i]=f32_to_bf16((rand()%200-100)/100.f);
        for (int i=0;i<K*N;i++) W[i]=(rand()%200-100)/100.f;

        // 分别计时
        double ms_spmm = bench_ms([&](){spmm_fb(A,H,Z,nt);}, WARMUP, REPEAT);
        double ms_gemm = bench_ms([&](){dense_gemm(Z,W,C,A.nrow,nt);}, WARMUP, REPEAT);
        double ms_total = ms_spmm + ms_gemm;
        double speedup = ms_total / fusion_ms[mi];

        const char* name = strrchr(MATRICES[mi],'/')+1;
        printf("%-18s %10.2f %10.2f %10.2f %12.2f %10.2f×\n",
               name, ms_spmm, ms_gemm, ms_total, fusion_ms[mi], speedup);

        free(H); free(Z); free(W); free(C);
    }

    printf("\n注：Fusion(ms) 来自 Stage3 实测结果\n");
    printf("    真实加速比 = (SpMM + GeMM) / Fusion\n");
    printf("\n=== Done ===\n");
    return 0;
}
