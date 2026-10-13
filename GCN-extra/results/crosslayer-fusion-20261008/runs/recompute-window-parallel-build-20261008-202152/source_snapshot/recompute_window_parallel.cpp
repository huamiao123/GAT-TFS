#define main serial_window_kernel_main
#include "recompute_window_kernel.cpp"
#undef main

struct ParallelStats {
    uint64_t source_rows = 0;
    uint64_t source_projection_groups = 0;
    uint64_t second_projection_groups = 0;
    uint64_t edges = 0;
    size_t peak_cache = 0;
};

static double run_parallel_w0(const csr_t& csr, const int* perm, const uint16_t* h0,
                              const uint16_t* w1, const uint16_t* w2, uint16_t* out,
                              ParallelStats& total) {
    SourceGraphView g{csr.N, csr.nnz, {csr.indptr}, {csr.indices}};
    const int batches = (csr.N + TR - 1) / TR;
    const int threads = omp_get_max_threads();
    std::vector<ParallelStats> locals(size_t(threads));
    const double t0 = omp_get_wtime();
#pragma omp parallel
    {
        const int tid = omp_get_thread_num();
        const int rb = (batches * tid) / threads;
        const int re = (batches * (tid + 1)) / threads;
        if (syscall(SYS_arch_prctl, 0x1023, 18) != 0) { perror("AMX permission"); abort(); }
        tilecfg_t cfg; setup_tilecfg(&cfg); _tile_loadconfig(&cfg);

        std::vector<uint8_t> mark(size_t(csr.N), 0);
        std::vector<uint32_t> unique;
        for (int r = rb; r < re; ++r) {
            int begin = r * TR;
            int end = std::min(begin + TR, csr.N);
            for (int pi = begin; pi < end; ++pi) {
                uint32_t row = uint32_t(perm[pi]);
                for (uint32_t e = csr.indptr[row]; e < csr.indptr[row + 1]; ++e) {
                    uint32_t src = csr.indices[e];
                    if (!mark[src]) { mark[src] = 1; unique.push_back(src); }
                }
            }
        }
        std::fill(mark.begin(), mark.end(), 0);
        SourceCache cache(csr.N, unique.size());
        std::vector<uint32_t> missing;
        alignas(64) uint16_t generated[TR * DIM];
        alignas(64) float partial[TR * DIM];
        alignas(64) uint16_t hi[TR * DIM];
        alignas(64) float tmp[TR * 16];
        Profile profile;
        for (int r = rb; r < re; ++r) {
            int begin = r * TR;
            int end = std::min(begin + TR, csr.N);
            missing.clear();
            for (int pi = begin; pi < end; ++pi) {
                uint32_t row = uint32_t(perm[pi]);
                for (uint32_t e = csr.indptr[row]; e < csr.indptr[row + 1]; ++e) {
                    uint32_t src = csr.indices[e];
                    if (!cache.has(src) && !mark[src]) { mark[src] = 1; missing.push_back(src); }
                }
            }
            for (uint32_t id : missing) mark[id] = 0;
            for (size_t off = 0; off < missing.size(); off += TR) {
                int count = int(std::min<size_t>(TR, missing.size() - off));
                compute_h1_group(g, h0, w1, missing.data() + off, count, generated,
                                 locals[tid].source_projection_groups);
                for (int n = 0; n < count; ++n) {
                    cache.put(missing[off + n], generated + n * DIM);
                    locals[tid].source_rows++;
                }
            }

            std::memset(partial, 0, sizeof(partial));
            for (int n = 0; n < end - begin; ++n) {
                __m512 acc[8];
                for (auto& x : acc) x = _mm512_setzero_ps();
                uint32_t row = uint32_t(perm[begin + n]);
                for (uint32_t e = csr.indptr[row]; e < csr.indptr[row + 1]; ++e) {
                    const uint16_t* h = cache.row(csr.indices[e]);
                    for (int f = 0; f < 8; ++f) {
                        __m256i b = _mm256_loadu_si256((const __m256i*)(h + f * 16));
                        __m512 val = _mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(b), 16));
                        acc[f] = _mm512_add_ps(acc[f], val);
                    }
                    locals[tid].edges++;
                }
                for (int f = 0; f < 8; ++f) _mm512_store_ps(partial + n * DIM + f * 16, acc[f]);
            }
            convert_partial_to_hi(partial, hi);
            for (int obp = 0; obp < NP; ++obp) {
                _tile_zero(TC0); _tile_zero(TC1); _tile_zero(TC2); _tile_zero(TC3);
                gcn_extra_paper::project_panel<false>(hi, w2, obp, profile, false);
                _tile_stored(TC0, tmp, 64);
                for (int n = 0; n < end - begin; ++n)
                    for (int j = 0; j < 16; ++j) out[size_t(perm[begin + n]) * DIM + obp * 64 + j] = f32_to_bf16(tmp[n * 16 + j]);
                _tile_stored(TC1, tmp, 64);
                for (int n = 0; n < end - begin; ++n)
                    for (int j = 0; j < 16; ++j) out[size_t(perm[begin + n]) * DIM + obp * 64 + 16 + j] = f32_to_bf16(tmp[n * 16 + j]);
                _tile_stored(TC2, tmp, 64);
                for (int n = 0; n < end - begin; ++n)
                    for (int j = 0; j < 16; ++j) out[size_t(perm[begin + n]) * DIM + obp * 64 + 32 + j] = f32_to_bf16(tmp[n * 16 + j]);
                _tile_stored(TC3, tmp, 64);
                for (int n = 0; n < end - begin; ++n)
                    for (int j = 0; j < 16; ++j) out[size_t(perm[begin + n]) * DIM + obp * 64 + 48 + j] = f32_to_bf16(tmp[n * 16 + j]);
            }
            locals[tid].second_projection_groups += (uint64_t(end - begin) + 15) / 16;
        }
        locals[tid].peak_cache = cache.peak;
        _tile_release();
    }
    for (const auto& x : locals) {
        total.source_rows += x.source_rows;
        total.source_projection_groups += x.source_projection_groups;
        total.second_projection_groups += x.second_projection_groups;
        total.edges += x.edges;
        total.peak_cache = std::max(total.peak_cache, x.peak_cache);
    }
    return (omp_get_wtime() - t0) * 1000.0;
}

static uint64_t parallel_checksum(const uint16_t* p, size_t n) {
    return checksum_bf16(p, n);
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::fprintf(stderr, "usage: %s DATA_DIR GRAPH THREADS\n", argv[0]);
        return 2;
    }
    const char* data_dir = argv[1];
    const char* graph_name = argv[2];
    const int threads = std::stoi(argv[3]);
    omp_set_num_threads(threads);
    std::string csr_path = std::string(data_dir) + "/" + graph_name + "/" + graph_name + ".csrbin";
    csr_t csr = load_csrbin(csr_path.c_str());
    for (uint32_t e = 0; e < csr.nnz; ++e)
        if (csr.values[e] != 1.0f) throw std::runtime_error("non-unit graph weights");
    std::srand(12345);
    float* W1 = (float*)mkl_malloc(size_t(DIM) * DIM * sizeof(float), 64);
    float* W2 = (float*)mkl_malloc(size_t(DIM) * DIM * sizeof(float), 64);
    float* H0 = (float*)mkl_malloc(size_t(csr.N) * DIM * sizeof(float), 64);
    for (int i = 0; i < DIM * DIM; ++i) { W1[i] = 0.01f * ((std::rand() % 200) - 100); W2[i] = 0.01f * ((std::rand() % 200) - 100); }
    for (size_t i = 0; i < size_t(csr.N) * DIM; ++i) H0[i] = 0.01f * ((std::rand() % 200) - 100);
    int* perm = make_degree_perm(csr.indptr, csr.N);
    uint16_t* H0b = (uint16_t*)aligned_alloc(64, size_t(csr.N) * DIM * sizeof(uint16_t));
    uint16_t* Wv1 = (uint16_t*)aligned_alloc(64, size_t(KB) * NB * 16 * 32 * sizeof(uint16_t));
    uint16_t* Wv2 = (uint16_t*)aligned_alloc(64, size_t(KB) * NB * 16 * 32 * sizeof(uint16_t));
    make_W_vnni(W1, Wv1); make_W_vnni(W2, Wv2);
    convert_f32_to_bf16(H0, H0b, size_t(csr.N) * DIM);
    float* H1 = (float*)aligned_alloc(64, size_t(csr.N) * DIM * sizeof(float));
    uint16_t* H1b = (uint16_t*)aligned_alloc(64, size_t(csr.N) * DIM * sizeof(uint16_t));
    uint16_t* base = (uint16_t*)aligned_alloc(64, size_t(csr.N) * DIM * sizeof(uint16_t));
    uint16_t* cand = (uint16_t*)aligned_alloc(64, size_t(csr.N) * DIM * sizeof(uint16_t));
    SourceGraphView g{csr.N, csr.nnz, {csr.indptr}, {csr.indices}};
    Method full{"full", 0, false, false, 0, true};

    for (int warm = 0; warm < 1; ++warm) {
        gcn_extra_paper::block_kernel<false, false>(g, H0b, Wv1, H1, perm, full, nullptr);
        relu_f32(H1, size_t(csr.N) * DIM); convert_f32_to_bf16(H1, H1b, size_t(csr.N) * DIM);
        gcn_extra_paper::block_kernel<false, true>(g, H1b, Wv2, base, perm, full, nullptr);
        ParallelStats warm_stats; run_parallel_w0(csr, perm, H0b, Wv1, Wv2, cand, warm_stats);
    }
    std::vector<double> base_ms, cand_ms;
    ParallelStats final_stats;
    for (int rep = 0; rep < 3; ++rep) {
        double t0 = omp_get_wtime();
        gcn_extra_paper::block_kernel<false, false>(g, H0b, Wv1, H1, perm, full, nullptr);
        relu_f32(H1, size_t(csr.N) * DIM); convert_f32_to_bf16(H1, H1b, size_t(csr.N) * DIM);
        gcn_extra_paper::block_kernel<false, true>(g, H1b, Wv2, base, perm, full, nullptr);
        base_ms.push_back((omp_get_wtime() - t0) * 1000.0);
        ParallelStats s;
        cand_ms.push_back(run_parallel_w0(csr, perm, H0b, Wv1, Wv2, cand, s));
        if (rep == 2) final_stats = s;
        std::printf("PAR_RUN rep=%d baseline_ms=%.9f candidate_ms=%.9f\n", rep, base_ms.back(), cand_ms.back());
    }
    std::sort(base_ms.begin(), base_ms.end()); std::sort(cand_ms.begin(), cand_ms.end());
    bool exact = std::memcmp(base, cand, size_t(csr.N) * DIM * sizeof(uint16_t)) == 0;
    std::printf("PAR_RESULT graph=%s threads=%d baseline_median_ms=%.9f candidate_median_ms=%.9f exact=%d source_rows=%llu source_projection_groups=%llu second_projection_groups=%llu peak_cache_rows=%zu checksum=%llu\n",
        graph_name, threads, base_ms[1], cand_ms[1], exact,
        (unsigned long long)final_stats.source_rows, (unsigned long long)final_stats.source_projection_groups,
        (unsigned long long)final_stats.second_projection_groups, final_stats.peak_cache,
        (unsigned long long)parallel_checksum(cand, size_t(csr.N) * DIM));
    std::printf("PAR_GATE pass=%d speedup=%.9f\n", exact, base_ms[1] / cand_ms[1]);
    std::fflush(stdout);
    free(perm); free(H0b); free(Wv1); free(Wv2); free(H1); free(H1b); free(base); free(cand);
    mkl_free(W1); mkl_free(W2); mkl_free(H0);
    free(csr.indptr); free(csr.indices); free(csr.values);
    return exact ? 0 : 3;
}
