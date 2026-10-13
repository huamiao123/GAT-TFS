#define main frozen_gcn_e2e_main
#include "../../../original/gcn_e2e_v3.cpp"
#undef main

#include "../../../src/paper_methods_kernels.hpp"

#include <atomic>
#include <chrono>
#include <climits>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using gcn_extra_paper::DIM;
using gcn_extra_paper::SourceGraphView;
using gcn_extra_paper::Profile;
using gcn_extra_paper::Method;

static constexpr uint32_t INF_RANK = UINT32_MAX;

struct SourceCache {
    static constexpr int SLOT_NONE = -1;
    size_t cap;
    std::vector<uint16_t> data;
    std::vector<uint32_t> ids;
    std::vector<int32_t> slot_of;
    std::vector<uint32_t> free_slots;
    size_t peak = 0;

    SourceCache(uint32_t n, size_t capacity)
        : cap(capacity), data(capacity * DIM), ids(capacity, UINT32_MAX),
          slot_of(n, SLOT_NONE) {
        free_slots.reserve(capacity);
        for (size_t i = capacity; i > 0; --i) free_slots.push_back(uint32_t(i - 1));
    }

    bool has(uint32_t id) const { return slot_of[id] != SLOT_NONE; }
    uint16_t* row(uint32_t id) {
        int32_t s = slot_of[id];
        if (s == SLOT_NONE) throw std::runtime_error("source cache miss");
        return data.data() + size_t(s) * DIM;
    }
    void put(uint32_t id, const uint16_t* src) {
        if (has(id)) return;
        if (free_slots.empty()) throw std::runtime_error("source cache capacity exceeded");
        uint32_t s = free_slots.back();
        free_slots.pop_back();
        slot_of[id] = int32_t(s);
        ids[s] = id;
        std::memcpy(data.data() + size_t(s) * DIM, src, DIM * sizeof(uint16_t));
        size_t live = cap - free_slots.size();
        if (live > peak) peak = live;
    }
    void erase(uint32_t id) {
        int32_t s = slot_of[id];
        if (s == SLOT_NONE) return;
        slot_of[id] = SLOT_NONE;
        ids[size_t(s)] = UINT32_MAX;
        free_slots.push_back(uint32_t(s));
    }
};

static void compute_h1_group(const SourceGraphView& g, const uint16_t* h0,
                             const uint16_t* wv, const uint32_t* ids, int count,
                             uint16_t* out, uint64_t& projection_groups) {
    alignas(64) uint32_t bases[TR]{};
    alignas(64) uint32_t degrees[TR]{};
    uint32_t maxdeg = 0;
    for (int n = 0; n < count; ++n) {
        uint32_t id = ids[n];
        bases[n] = g.row[id];
        degrees[n] = g.row[id + 1] - bases[n];
        maxdeg = std::max(maxdeg, degrees[n]);
    }
    alignas(64) float partial[TR * DIM];
    alignas(64) uint16_t hi[TR * DIM];
    alignas(64) uint16_t lo[TR * DIM];
    alignas(64) float result[TR * DIM];
    Profile profile;
    gcn_extra_paper::reduce_tile<false>(g, h0, bases, degrees, count, 0, maxdeg,
                                        partial, hi, lo, false, profile, false);
    std::memset(result, 0, sizeof(result));
    for (int obp = 0; obp < NP; ++obp) {
        _tile_zero(TC0); _tile_zero(TC1); _tile_zero(TC2); _tile_zero(TC3);
        gcn_extra_paper::project_panel<false>(hi, wv, obp, profile, false);
        alignas(64) float tmp[TR * 16];
        _tile_stored(TC0, tmp, 64);
        for (int n = 0; n < count; ++n)
            std::memcpy(result + n * DIM + obp * 64 + 0, tmp + n * 16, 16 * sizeof(float));
        _tile_stored(TC1, tmp, 64);
        for (int n = 0; n < count; ++n)
            std::memcpy(result + n * DIM + obp * 64 + 16, tmp + n * 16, 16 * sizeof(float));
        _tile_stored(TC2, tmp, 64);
        for (int n = 0; n < count; ++n)
            std::memcpy(result + n * DIM + obp * 64 + 32, tmp + n * 16, 16 * sizeof(float));
        _tile_stored(TC3, tmp, 64);
        for (int n = 0; n < count; ++n)
            std::memcpy(result + n * DIM + obp * 64 + 48, tmp + n * 16, 16 * sizeof(float));
    }
    for (int n = 0; n < count; ++n) {
        for (int f = 0; f < DIM; ++f) {
            float x = result[n * DIM + f];
            if (x < 0.0f) x = 0.0f;
            out[n * DIM + f] = f32_to_bf16(x);
        }
    }
    projection_groups += (uint64_t(count) + 15) / 16;
}

static void convert_partial_to_hi(const float* partial, uint16_t* hi) {
    for (int v = 0; v < TR * DIM; v += 16) {
        __m512 a = _mm512_load_ps(partial + v);
        __m512i bits = _mm512_castps_si512(a);
        __m256i top = _mm512_cvtepi32_epi16(_mm512_srli_epi32(bits, 16));
        _mm256_store_si256((__m256i*)(hi + v), top);
    }
}

struct CandidateStats {
    uint64_t source_rows = 0;
    uint64_t source_projection_groups = 0;
    uint64_t second_projection_groups = 0;
    uint64_t extra_edges = 0;
    uint64_t second_edges = 0;
    size_t peak_cache = 0;
};

static void build_next_use(const csr_t& csr, const int* perm, std::vector<uint32_t>& next_use) {
    next_use.assign(csr.nnz, INF_RANK);
    std::vector<uint32_t> last(csr.N, INF_RANK);
    int batches = (csr.N + TR - 1) / TR;
    for (int r = batches - 1; r >= 0; --r) {
        int begin = r * TR;
        int end = std::min(begin + TR, csr.N);
        for (int pi = begin; pi < end; ++pi) {
            uint32_t row = uint32_t(perm[pi]);
            for (uint32_t e = csr.indptr[row]; e < csr.indptr[row + 1]; ++e) {
                uint32_t src = csr.indices[e];
                next_use[e] = last[src];
                last[src] = uint32_t(r);
            }
        }
    }
}

static double run_window(const csr_t& csr, const int* perm, const uint16_t* h0,
                         const uint16_t* w1, const uint16_t* w2, uint32_t window,
                         size_t cache_capacity, uint16_t* out, CandidateStats& stats) {
    SourceGraphView g{csr.N, csr.nnz, {csr.indptr}, {csr.indices}};
    std::vector<uint32_t> next_use;
    build_next_use(csr, perm, next_use);
    SourceCache cache(csr.N, cache_capacity);
    std::vector<uint8_t> marked(csr.N, 0);
    alignas(64) uint16_t generated[TR * DIM];
    alignas(64) uint32_t miss[TR * 8];
    alignas(64) float partial[TR * DIM];
    alignas(64) uint16_t hi[TR * DIM];
    alignas(64) float tmp[TR * 16];
    Profile profile;
    const bool keep_all = window == 0;
    const int batches = (csr.N + TR - 1) / TR;
    double t0 = omp_get_wtime();
    for (int r = 0; r < batches; ++r) {
        int begin = r * TR;
        int end = std::min(begin + TR, csr.N);
        std::vector<uint32_t> missing;
        for (int pi = begin; pi < end; ++pi) {
            uint32_t row = uint32_t(perm[pi]);
            for (uint32_t e = csr.indptr[row]; e < csr.indptr[row + 1]; ++e) {
                uint32_t src = csr.indices[e];
                if (!cache.has(src) && !marked[src]) {
                    marked[src] = 1;
                    missing.push_back(src);
                }
            }
        }
        for (uint32_t id : missing) marked[id] = 0;
        for (size_t off = 0; off < missing.size(); off += TR) {
            int count = int(std::min<size_t>(TR, missing.size() - off));
            for (int n = 0; n < count; ++n) miss[n] = missing[off + n];
            compute_h1_group(g, h0, w1, miss, count, generated, stats.source_projection_groups);
            for (int n = 0; n < count; ++n) {
                cache.put(miss[n], generated + n * DIM);
                stats.source_rows++;
            }
        }

        std::memset(partial, 0, sizeof(partial));
        for (int n = 0; n < end - begin; ++n) {
            __m512 acc[8];
            for (auto& x : acc) x = _mm512_setzero_ps();
            uint32_t row = uint32_t(perm[begin + n]);
            for (uint32_t e = csr.indptr[row]; e < csr.indptr[row + 1]; ++e) {
                uint32_t src = csr.indices[e];
                const uint16_t* h = cache.row(src);
                for (int f = 0; f < 8; ++f) {
                    __m256i b = _mm256_loadu_si256((const __m256i*)(h + f * 16));
                    __m512 val = _mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(b), 16));
                    acc[f] = _mm512_add_ps(acc[f], val);
                }
                stats.second_edges++;
                stats.extra_edges += (next_use[e] != INF_RANK && next_use[e] - uint32_t(r) > window) ?
                                     uint64_t(csr.indptr[src + 1] - csr.indptr[src]) : 0;
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
        stats.second_projection_groups += (uint64_t(end - begin) + 15) / 16;

        for (int pi = begin; pi < end; ++pi) {
            uint32_t row = uint32_t(perm[pi]);
            for (uint32_t e = csr.indptr[row]; e < csr.indptr[row + 1]; ++e) {
                uint32_t src = csr.indices[e];
                uint32_t next = next_use[e];
                if (!keep_all && (next == INF_RANK || next - uint32_t(r) > window)) cache.erase(src);
            }
        }
        stats.peak_cache = std::max(stats.peak_cache, cache.peak);
    }
    stats.peak_cache = cache.peak;
    return (omp_get_wtime() - t0) * 1000.0;
}

static uint64_t checksum_bf16(const uint16_t* p, size_t n) {
    uint64_t h = 1469598103934665603ULL;
    for (size_t i = 0; i < n; ++i) { h ^= p[i]; h *= 1099511628211ULL; }
    return h;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::fprintf(stderr, "usage: %s DATA_DIR GRAPH [WINDOW]\n", argv[0]);
        return 2;
    }
    const char* data_dir = argv[1];
    const char* graph_name = argv[2];
    uint32_t window = argc >= 4 ? uint32_t(std::stoul(argv[3])) : 1024;
    std::string csr_path = std::string(data_dir) + "/" + graph_name + "/" + graph_name + ".csrbin";
    csr_t csr = load_csrbin(csr_path.c_str());
    if (csr.N <= 0 || csr.nnz == 0) throw std::runtime_error("empty graph");
    for (uint32_t e = 0; e < csr.nnz; ++e)
        if (csr.values[e] != 1.0f) throw std::runtime_error("non-unit graph weights");
    omp_set_num_threads(1);
    std::printf("WINDOW_CONFIG graph=%s N=%d E=%u D=128 F=128 threads=1 window=%u no_numa_interleave=1\n",
                graph_name, csr.N, csr.nnz, window);

    std::srand(12345);
    float* W1 = (float*)mkl_malloc(size_t(DIM) * DIM * sizeof(float), 64);
    float* W2 = (float*)mkl_malloc(size_t(DIM) * DIM * sizeof(float), 64);
    float* H0 = (float*)mkl_malloc(size_t(csr.N) * DIM * sizeof(float), 64);
    for (int i = 0; i < DIM * DIM; ++i) {
        W1[i] = 0.01f * ((std::rand() % 200) - 100);
        W2[i] = 0.01f * ((std::rand() % 200) - 100);
    }
    for (size_t i = 0; i < size_t(csr.N) * DIM; ++i) H0[i] = 0.01f * ((std::rand() % 200) - 100);
    int* perm = make_degree_perm(csr.indptr, csr.N);
    uint16_t* H0b = (uint16_t*)aligned_alloc(64, size_t(csr.N) * DIM * sizeof(uint16_t));
    uint16_t* Wv1 = (uint16_t*)aligned_alloc(64, size_t(KB) * NB * 16 * 32 * sizeof(uint16_t));
    uint16_t* Wv2 = (uint16_t*)aligned_alloc(64, size_t(KB) * NB * 16 * 32 * sizeof(uint16_t));
    make_W_vnni(W1, Wv1); make_W_vnni(W2, Wv2);
    convert_f32_to_bf16(H0, H0b, size_t(csr.N) * DIM);
    float* H1 = (float*)aligned_alloc(64, size_t(csr.N) * DIM * sizeof(float));
    uint16_t* H1b = (uint16_t*)aligned_alloc(64, size_t(csr.N) * DIM * sizeof(uint16_t));
    uint16_t* base_out = (uint16_t*)aligned_alloc(64, size_t(csr.N) * DIM * sizeof(uint16_t));
    uint16_t* w0_out = (uint16_t*)aligned_alloc(64, size_t(csr.N) * DIM * sizeof(uint16_t));
    uint16_t* w_out = (uint16_t*)aligned_alloc(64, size_t(csr.N) * DIM * sizeof(uint16_t));
    SourceGraphView g{csr.N, csr.nnz, {csr.indptr}, {csr.indices}};
    Method full{"full", 0, false, false, 0, true};

    tilecfg_t cfg; setup_tilecfg(&cfg); _tile_loadconfig(&cfg);
    double t0 = omp_get_wtime();
    gcn_extra_paper::block_kernel<false, false>(g, H0b, Wv1, H1, perm, full, nullptr);
    relu_f32(H1, size_t(csr.N) * DIM);
    convert_f32_to_bf16(H1, H1b, size_t(csr.N) * DIM);
    gcn_extra_paper::block_kernel<false, true>(g, H1b, Wv2, base_out, perm, full, nullptr);
    double baseline_ms = (omp_get_wtime() - t0) * 1000.0;
    std::printf("BASELINE_FULL ms=%.6f checksum=%llu\n", baseline_ms,
                (unsigned long long)checksum_bf16(base_out, size_t(csr.N) * DIM));
    _tile_release();

    tilecfg_t candidate_cfg; setup_tilecfg(&candidate_cfg); _tile_loadconfig(&candidate_cfg);
    size_t cap = window == 0 ? size_t(csr.N) : 20000;
    CandidateStats s0;
    double w0_ms = run_window(csr, perm, H0b, Wv1, Wv2, 0, size_t(csr.N), w0_out, s0);
    CandidateStats sw;
    double w_ms = run_window(csr, perm, H0b, Wv1, Wv2, window, cap, w_out, sw);
    _tile_release();
    bool exact0 = std::memcmp(base_out, w0_out, size_t(csr.N) * DIM * sizeof(uint16_t)) == 0;
    bool exactw = std::memcmp(base_out, w_out, size_t(csr.N) * DIM * sizeof(uint16_t)) == 0;
    std::printf("WINDOW_RESULT window=0 ms=%.6f source_rows=%llu source_projection_groups=%llu second_projection_groups=%llu peak_cache_rows=%zu exact=%d checksum=%llu\n",
        w0_ms, (unsigned long long)s0.source_rows, (unsigned long long)s0.source_projection_groups,
        (unsigned long long)s0.second_projection_groups, s0.peak_cache, exact0,
        (unsigned long long)checksum_bf16(w0_out, size_t(csr.N) * DIM));
    std::printf("WINDOW_RESULT window=%u ms=%.6f source_rows=%llu extra_source_rows=%lld source_projection_groups=%llu second_projection_groups=%llu peak_cache_rows=%zu exact=%d checksum=%llu\n",
        window, w_ms, (unsigned long long)sw.source_rows,
        (long long)sw.source_rows - csr.N, (unsigned long long)sw.source_projection_groups,
        (unsigned long long)sw.second_projection_groups, sw.peak_cache, exactw,
        (unsigned long long)checksum_bf16(w_out, size_t(csr.N) * DIM));
    std::printf("WINDOW_GATE pass=%d baseline_ms=%.6f window_ms=%.6f\n", int(exact0 && exactw), baseline_ms, w_ms);
    std::fflush(stdout);

    free(perm); free(H0b); free(Wv1); free(Wv2); free(H1); free(H1b); free(base_out); free(w0_out); free(w_out);
    mkl_free(W1); mkl_free(W2); mkl_free(H0);
    free(csr.indptr); free(csr.indices); free(csr.values);
    return (exact0 && exactw) ? 0 : 3;
}
