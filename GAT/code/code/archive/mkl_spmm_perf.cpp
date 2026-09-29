#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <mkl.h>
#include <omp.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <csrbin_file> <k> [iterations=50]" << std::endl;
        return 1;
    }

    const char* filename = argv[1];
    int k = atoi(argv[2]);
    int iters = argc > 3 ? atoi(argv[3]) : 50;

    std::ifstream fin(filename, std::ios::binary);
    if (!fin.is_open()) {
        std::cerr << "Cannot open: " << filename << std::endl;
        return 1;
    }

    uint32_t ptype, dtype, vtype;
    fin.read((char*)&ptype, 4);
    fin.read((char*)&dtype, 4);
    fin.read((char*)&vtype, 4);

    uint64_t nrow, ncol, nnz;
    fin.read((char*)&nrow, 8);
    fin.read((char*)&ncol, 8);
    fin.read((char*)&nnz, 8);

    std::cout << "File: " << filename
              << " | types=(" << ptype << "," << dtype << "," << vtype << ")"
              << " | M=" << nrow << ", N=" << ncol << ", NNZ=" << nnz
              << ", k=" << k << ", iters=" << iters << std::endl;

    // 读 indptr
    MKL_INT* rowptr = (MKL_INT*)mkl_malloc((nrow + 1) * sizeof(MKL_INT), 64);
    if (ptype == 0) {
        std::vector<uint32_t> tmp(nrow + 1);
        fin.read((char*)tmp.data(), (nrow + 1) * 4);
        for (uint64_t i = 0; i <= nrow; i++) rowptr[i] = tmp[i];
    } else {
        std::vector<uint64_t> tmp(nrow + 1);
        fin.read((char*)tmp.data(), (nrow + 1) * 8);
        for (uint64_t i = 0; i <= nrow; i++) rowptr[i] = tmp[i];
    }

    // 读 indices
    MKL_INT* colidx = (MKL_INT*)mkl_malloc(nnz * sizeof(MKL_INT), 64);
    if (dtype == 0) {
        std::vector<uint32_t> tmp(nnz);
        fin.read((char*)tmp.data(), nnz * 4);
        for (uint64_t i = 0; i < nnz; i++) colidx[i] = tmp[i];
    } else {
        std::vector<uint64_t> tmp(nnz);
        fin.read((char*)tmp.data(), nnz * 8);
        for (uint64_t i = 0; i < nnz; i++) colidx[i] = tmp[i];
    }

    // 读 values
    float* values = (float*)mkl_malloc(nnz * sizeof(float), 64);
    if (vtype == 2) {
        fin.read((char*)values, nnz * 4);
    } else {
        std::vector<double> tmp(nnz);
        fin.read((char*)tmp.data(), nnz * 8);
        for (uint64_t i = 0; i < nnz; i++) values[i] = (float)tmp[i];
    }
    fin.close();

    std::cout << "CSR loaded | rowptr[end]=" << rowptr[nrow]
              << " (should equal NNZ=" << nnz << ") | dtype=FP32" << std::endl;

    sparse_matrix_t A;
    mkl_sparse_s_create_csr(&A, SPARSE_INDEX_BASE_ZERO, nrow, ncol,
                            rowptr, rowptr + 1, colidx, values);

    struct matrix_descr descr;
    descr.type = SPARSE_MATRIX_TYPE_GENERAL;

    mkl_sparse_set_mm_hint(A, SPARSE_OPERATION_NON_TRANSPOSE, descr,
                           SPARSE_LAYOUT_ROW_MAJOR, k, iters + 60);
    mkl_sparse_optimize(A);

    // 关键：用 mkl_malloc 分配 64 字节对齐的 B 和 C
    float* B = (float*)mkl_malloc(sizeof(float) * ncol * k, 64);
    float* C = (float*)mkl_malloc(sizeof(float) * nrow * k, 64);

    // 关键：并行初始化 → First-Touch NUMA 分布
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < (int64_t)ncol * k; i++) B[i] = 1.0f;
    #pragma omp parallel for schedule(static)
    for (int64_t i = 0; i < (int64_t)nrow * k; i++) C[i] = 0.0f;

    // Warmup
    for (int i = 0; i < 50; i++) {
        mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE, 1.0f, A, descr,
                        SPARSE_LAYOUT_ROW_MAJOR, B, k, k,
                        0.0f, C, k);
    }

    // Benchmark
    double t0 = omp_get_wtime();
    for (int i = 0; i < iters; i++) {
        mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE, 1.0f, A, descr,
                        SPARSE_LAYOUT_ROW_MAJOR, B, k, k,
                        0.0f, C, k);
    }
    double t1 = omp_get_wtime();
    double avg_ms = (t1 - t0) / iters * 1000.0;
    double gflops = 2.0 * nnz * k / avg_ms / 1e6;

    std::cout << "Time: " << avg_ms << " ms | GFLOPS: " << gflops << std::endl;

    mkl_sparse_destroy(A);
    mkl_free(rowptr); mkl_free(colidx); mkl_free(values);
    mkl_free(B); mkl_free(C);
    return 0;
}
