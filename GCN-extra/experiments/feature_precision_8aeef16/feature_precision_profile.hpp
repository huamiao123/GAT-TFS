#pragma once
#include "frozen_source/paper_methods_kernels.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>

// Separate diagnostic implementation of the frozen coupled B64/FULL loop.
// It does not implement S/M decoupling and is never used as a performance
// numerator or denominator. Every invocation must be checked bitwise against
// its uninstrumented counterpart with exactly the same feature input.
namespace gcn_extra_feature_profile {

using gcn_extra_paper::DIM;
using gcn_extra_paper::SourceGraphView;
using gcn_extra_paper::Method;

enum DetailPhase {
    PARTIAL_INIT, CSR_INDEX_READ, CSR_NEXT_INDEX_READ, PREFETCH_ISSUE,
    SOURCE_FEATURE_LOAD, SOURCE_BF16_DECODE, FEATURE_FP32_ADD,
    PARTIAL_STATE_LOAD, PARTIAL_STATE_STORE, PARTIAL_HI_PACK,
    PARTIAL_LO_RESIDUAL, PARTIAL_LO_PACK, AMX_A_LOAD, AMX_B_LOAD,
    AMX_TDP_COMPUTE, AMX_C_ZERO, AMX_C_LOAD, AMX_C_STORE,
    OUTPUT_BF16_CONVERSION, OUTPUT_STORE, THREAD_PERMISSION,
    THREAD_TILE_CONFIG, THREAD_TILE_RELEASE, ROW_METADATA_READ, DETAIL_PHASES
};
inline constexpr const char* detail_phase_name[DETAIL_PHASES] = {
    "partial_init", "csr_index_read", "csr_next_index_read", "prefetch_issue",
    "source_feature_load", "source_bf16_decode", "feature_fp32_add",
    "partial_state_load", "partial_state_store", "partial_hi_pack",
    "partial_lo_residual", "partial_lo_pack", "amx_a_load", "amx_b_load",
    "amx_tdp_compute", "amx_c_zero", "amx_c_load", "amx_c_store",
    "output_bf16_conversion", "output_store", "thread_permission",
    "thread_tile_config", "thread_tile_release", "row_metadata_read"
};

// Complete logical counts, not sampled estimates or claims about DRAM traffic.
// The unchanged four hints prefetch 256 requested bytes per next source row:
// a whole BF16 row, but only the first half of an FP32 row.
struct Counters {
    uint64_t destination_tiles = 0;
    uint64_t projection_scopes = 0;
    uint64_t neighbor_blocks = 0;
    uint64_t row_block_visits = 0;
    uint64_t nonempty_row_blocks = 0;
    uint64_t source_visits = 0;
    uint64_t feature_adds = 0;
    uint64_t source_gather_requested_bytes = 0;
    uint64_t source_decode_elements = 0;
    uint64_t partial_zero_bytes = 0;
    uint64_t partial_state_store_bytes = 0;
    uint64_t csr_prefetch_instructions = 0;
    uint64_t prefetch_requested_bytes = 0;
    uint64_t partial_packed_bytes = 0;
    uint64_t projection_panels = 0;
    uint64_t amx_input_loads = 0;
    uint64_t amx_a_loads = 0;
    uint64_t amx_b_loads = 0;
    uint64_t amx_accumulator_loads = 0;
    uint64_t amx_accumulator_stores = 0;
    uint64_t amx_dpbf16ps = 0;
    uint64_t amx_executed_flops = 0;
    uint64_t amx_reload_bytes = 0;
    uint64_t amx_spill_bytes = 0;
    uint64_t output_store_bytes = 0;
};

struct ThreadProfile {
    int thread_id = 0;
    uint64_t kernel_call = 0;
    double setup_s = 0;
    double kernel_active_s = 0;
    double release_s = 0;
    double start_offset_s = 0;
    double finish_offset_s = 0;
    uint64_t completed_tiles = 0;
    uint64_t completed_edges = 0;
    Counters counters;
    std::array<double, gcn_extra_paper::PHASES> sec{};
    std::array<double, DETAIL_PHASES> detail_sec{};
    std::array<uint64_t, DETAIL_PHASES> detail_events{};
};

struct Profile : gcn_extra_paper::Profile {
    Counters counters;
    std::array<double, DETAIL_PHASES> detail_sec{};
    std::array<uint64_t, DETAIL_PHASES> detail_events{};
    uint64_t kernel_calls = 0;
    ThreadProfile thread;
    std::vector<ThreadProfile> threads;
};

// Coarse intervals contain their detail intervals, including timer overhead.
// Do not sum parent/child times or interpret their ratios as wall fractions.
// Fine timers can spill registers and change the sampling path. In particular,
// sampled BF16 loads and decoding are grouped to expose separate intervals;
// sampled BF16 scatter stages through a temporary to distinguish conversion
// from the destination write. All arithmetic and per-row addition order stay
// unchanged. The separate uninstrumented kernels supply all reported speedups.
inline double tick(bool sample) { return gcn_extra_paper::tick<true>(sample); }
inline void detail_tock(Profile& pr, DetailPhase phase, double t, bool sample) {
    if(sample) {
        pr.detail_sec[phase] += omp_get_wtime() - t;
        pr.detail_events[phase]++;
    }
}

inline void add_counters(Counters& dst, const Counters& src) {
    #define GCN_PRECISION_ADD(F) dst.F += src.F
    GCN_PRECISION_ADD(destination_tiles);
    GCN_PRECISION_ADD(projection_scopes);
    GCN_PRECISION_ADD(neighbor_blocks);
    GCN_PRECISION_ADD(row_block_visits);
    GCN_PRECISION_ADD(nonempty_row_blocks);
    GCN_PRECISION_ADD(source_visits);
    GCN_PRECISION_ADD(feature_adds);
    GCN_PRECISION_ADD(source_gather_requested_bytes);
    GCN_PRECISION_ADD(source_decode_elements);
    GCN_PRECISION_ADD(partial_zero_bytes);
    GCN_PRECISION_ADD(partial_state_store_bytes);
    GCN_PRECISION_ADD(csr_prefetch_instructions);
    GCN_PRECISION_ADD(prefetch_requested_bytes);
    GCN_PRECISION_ADD(partial_packed_bytes);
    GCN_PRECISION_ADD(projection_panels);
    GCN_PRECISION_ADD(amx_input_loads);
    GCN_PRECISION_ADD(amx_a_loads);
    GCN_PRECISION_ADD(amx_b_loads);
    GCN_PRECISION_ADD(amx_accumulator_loads);
    GCN_PRECISION_ADD(amx_accumulator_stores);
    GCN_PRECISION_ADD(amx_dpbf16ps);
    GCN_PRECISION_ADD(amx_executed_flops);
    GCN_PRECISION_ADD(amx_reload_bytes);
    GCN_PRECISION_ADD(amx_spill_bytes);
    GCN_PRECISION_ADD(output_store_bytes);
    #undef GCN_PRECISION_ADD
}

// Same A/B/C tile sequence, dimensions, packing and projection as frozen code.
inline void project_panel(const uint16_t* hb, const uint16_t* w, int obp,
                          Profile& pr, bool sample) {
    using namespace gcn_extra_paper;
    for(int kb = 0; kb < KB; kb++) {
        double t = gcn_extra_feature_profile::tick(sample);
        double d = gcn_extra_feature_profile::tick(sample);
        _tile_loadd(TA, reinterpret_cast<const uint8_t*>(hb) + kb * 64, DIM * 2);
        detail_tock(pr, AMX_A_LOAD, d, sample);
        const int ob0 = obp * 4;
        d = gcn_extra_feature_profile::tick(sample);
        _tile_loadd(TB0, w + ((kb * NB + ob0) * 16 * 32), 64);
        _tile_loadd(TB1, w + ((kb * NB + ob0 + 1) * 16 * 32), 64);
        detail_tock(pr, AMX_B_LOAD, d, sample);
        tock<true>(pr, TILE_LOAD, t, sample);

        t = gcn_extra_feature_profile::tick(sample);
        _tile_dpbf16ps(TC0, TA, TB0);
        _tile_dpbf16ps(TC1, TA, TB1);
        detail_tock(pr, AMX_TDP_COMPUTE, t, sample);
        tock<true>(pr, TILE_COMPUTE, t, sample);

        t = gcn_extra_feature_profile::tick(sample);
        _tile_loadd(TB0, w + ((kb * NB + ob0 + 2) * 16 * 32), 64);
        _tile_loadd(TB1, w + ((kb * NB + ob0 + 3) * 16 * 32), 64);
        detail_tock(pr, AMX_B_LOAD, t, sample);
        tock<true>(pr, TILE_LOAD, t, sample);

        t = gcn_extra_feature_profile::tick(sample);
        _tile_dpbf16ps(TC2, TA, TB0);
        _tile_dpbf16ps(TC3, TA, TB1);
        detail_tock(pr, AMX_TDP_COMPUTE, t, sample);
        tock<true>(pr, TILE_COMPUTE, t, sample);
    }
}

template<bool FP32_INPUT>
void reduce_tile(const SourceGraphView& g,
                 const std::conditional_t<FP32_INPUT, float, uint16_t>* hb,
                 const uint32_t* bases, const uint32_t* degrees, int batch,
                 uint32_t start, uint32_t count, float* partial, uint16_t* hi,
                 Profile& pr, bool sample) {
    using namespace gcn_extra_paper;
    double t = gcn_extra_feature_profile::tick(sample);
    std::memset(partial, 0, TR * DIM * sizeof(float));
    detail_tock(pr, PARTIAL_INIT, t, sample);
    tock<true>(pr, REDUCE_ZERO, t, sample);
    pr.counters.projection_scopes++;
    pr.counters.neighbor_blocks++;
    pr.counters.partial_zero_bytes += TR * DIM * sizeof(float);

    for(int n = 0; n < batch; n++) {
        const uint32_t end = std::min(degrees[n], start + count);
        __m512 acc[8];
        for(auto& v : acc) v = _mm512_setzero_ps();
        pr.counters.row_block_visits++;
        if(start < end) {
            const uint64_t edges = end - start;
            pr.counters.nonempty_row_blocks++;
            pr.counters.source_visits += edges;
            pr.counters.feature_adds += edges * DIM;
            pr.counters.source_gather_requested_bytes += edges * DIM * sizeof(*hb);
            if constexpr(!FP32_INPUT) pr.counters.source_decode_elements += edges * DIM;
            pr.counters.csr_prefetch_instructions += (edges - 1) * 4;
            pr.counters.prefetch_requested_bytes += (edges - 1) * 256;
        }

        for(uint32_t k = start; k < end; k++) {
            t = gcn_extra_feature_profile::tick(sample);
            double d = gcn_extra_feature_profile::tick(sample);
            const uint32_t j = g.col[bases[n] + k];
            detail_tock(pr, CSR_INDEX_READ, d, sample);
            if(k + 1 < end) {
                d = gcn_extra_feature_profile::tick(sample);
                const uint32_t next_j = g.col[bases[n] + k + 1];
                detail_tock(pr, CSR_NEXT_INDEX_READ, d, sample);
                const char* next = reinterpret_cast<const char*>(hb + size_t(next_j) * DIM);
                d = gcn_extra_feature_profile::tick(sample);
                // Fixed original instruction budget for both feature types.
                for(int o = 0; o < 256; o += 64) _mm_prefetch(next + o, _MM_HINT_T0);
                detail_tock(pr, PREFETCH_ISSUE, d, sample);
            }
            tock<true>(pr, CSR_PREFETCH, t, sample);

            t = gcn_extra_feature_profile::tick(sample);
            __m512 val[8];
            const auto* src = hb + size_t(j) * DIM;
            if constexpr(FP32_INPUT) {
                const double d = gcn_extra_feature_profile::tick(sample);
                for(int f = 0; f < 8; f++) val[f] = _mm512_loadu_ps(src + f * 16);
                detail_tock(pr, SOURCE_FEATURE_LOAD, d, sample);
                // No FP32 feature decode instruction or decode timing.
            } else if(sample) {
                __m256i raw[8];
                double d = gcn_extra_feature_profile::tick(sample);
                for(int f = 0; f < 8; f++)
                    raw[f] = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src + f * 16));
                detail_tock(pr, SOURCE_FEATURE_LOAD, d, sample);
                d = gcn_extra_feature_profile::tick(sample);
                for(int f = 0; f < 8; f++) val[f] = _mm512_castsi512_ps(
                    _mm512_slli_epi32(_mm512_cvtepu16_epi32(raw[f]), 16));
                detail_tock(pr, SOURCE_BF16_DECODE, d, sample);
            } else {
                for(int f = 0; f < 8; f++) val[f] = _mm512_castsi512_ps(_mm512_slli_epi32(
                    _mm512_cvtepu16_epi32(_mm256_loadu_si256(
                        reinterpret_cast<const __m256i*>(src + f * 16))), 16));
            }
            tock<true>(pr, GATHER_DECODE, t, sample);

            t = gcn_extra_feature_profile::tick(sample);
            for(int f = 0; f < 8; f++) acc[f] = _mm512_add_ps(acc[f], val[f]);
            detail_tock(pr, FEATURE_FP32_ADD, t, sample);
            tock<true>(pr, REDUCE_ADD, t, sample);
        }
        t = gcn_extra_feature_profile::tick(sample);
        for(int f = 0; f < 8; f++) _mm512_store_ps(partial + n * DIM + f * 16, acc[f]);
        detail_tock(pr, PARTIAL_STATE_STORE, t, sample);
        tock<true>(pr, REDUCE_ADD, t, sample);
        pr.counters.partial_state_store_bytes += DIM * sizeof(float);
    }

    // Original FAST high-part truncation. The residual stages are explicitly
    // zero for this 2x2 FAST experiment, with slots retained for later studies.
    t = gcn_extra_feature_profile::tick(sample);
    for(int v = 0; v < TR * DIM; v += 16) {
        const double d = gcn_extra_feature_profile::tick(sample);
        const __m512 a = _mm512_load_ps(partial + v);
        const __m512i bits = _mm512_castps_si512(a);
        const __m256i top = _mm512_cvtepi32_epi16(_mm512_srli_epi32(bits, 16));
        _mm256_store_si256(reinterpret_cast<__m256i*>(hi + v), top);
        detail_tock(pr, PARTIAL_HI_PACK, d, sample);
    }
    tock<true>(pr, CONVERT, t, sample);
    pr.counters.partial_packed_bytes += TR * DIM * sizeof(uint16_t);
}

template<bool B16>
inline void scatter_output(std::conditional_t<B16, uint16_t, float>* dst,
                           const float* src, int count, Profile& pr, bool sample) {
    if(sample) {
        if constexpr(B16) {
            alignas(64) uint16_t converted[DIM];
            double t = tick(sample);
            for(int k = 0; k < count; k++) converted[k] = ::f32_to_bf16(src[k]);
            detail_tock(pr, OUTPUT_BF16_CONVERSION, t, sample);
            t = tick(sample);
            std::memcpy(dst, converted, size_t(count) * sizeof(uint16_t));
            detail_tock(pr, OUTPUT_STORE, t, sample);
        } else {
            const double t = tick(sample);
            std::memcpy(dst, src, size_t(count) * sizeof(float));
            detail_tock(pr, OUTPUT_STORE, t, sample);
        }
    } else gcn_extra_paper::scatter_output<B16>(dst, src, count);
}

template<bool FP32_INPUT, bool B16 = false>
void kernel(const SourceGraphView& g,
            const std::conditional_t<FP32_INPUT, float, uint16_t>* hb,
            const uint16_t* w, std::conditional_t<B16, uint16_t, float>* c,
            const int* perm, const Method& method, Profile* result) {
    using namespace gcn_extra_paper;
    if(result == nullptr) throw std::invalid_argument("feature profile result is required");
    if(method.accurate || method.replay || !method.nozero ||
       (method.block != 0 && method.block != 64))
        throw std::invalid_argument("feature precision diagnostic accepts only FAST B64/FULL, nozero=true, replay=false");

    const int R = 64;
    std::vector<Profile> profiles(omp_get_max_threads());
    // No global output zero: the experiment uses the original nozero=true.
    const double epoch = gcn_extra_feature_profile::tick(true);
    #pragma omp parallel
    {
        Profile& pr = profiles[omp_get_thread_num()];
        pr.thread.thread_id = omp_get_thread_num();
        double t = gcn_extra_feature_profile::tick(true);
        double d = gcn_extra_feature_profile::tick(true);
        if(syscall(SYS_arch_prctl, 0x1023, 18) != 0) { perror("AMX permission"); abort(); }
        detail_tock(pr, THREAD_PERMISSION, d, true);
        d = gcn_extra_feature_profile::tick(true);
        tilecfg_t cfg;
        setup_tilecfg(&cfg);
        _tile_loadconfig(&cfg);
        detail_tock(pr, THREAD_TILE_CONFIG, d, true);
        tock<true>(pr, SETUP, t, true);
        pr.thread.setup_s = omp_get_wtime() - t;

        alignas(64) float partial[TR * DIM], ctile[TR * DIM];
        alignas(64) uint16_t hi[TR * DIM];
        uint32_t bases[TR], degrees[TR];
        int rows[TR];
        const double active_begin = gcn_extra_feature_profile::tick(true);
        pr.thread.start_offset_s = active_begin - epoch;

        #pragma omp for schedule(dynamic,1) nowait
        for(int rg = 0; rg < g.n; rg += R) {
            for(int i = rg; i < std::min(rg + R, g.n); i += TR) {
                const bool sample = ((i / TR) % PROFILE_STRIDE) == 0 || i + TR >= g.n;
                if(sample) pr.tiles++;
                pr.counters.destination_tiles++;
                t = gcn_extra_feature_profile::tick(sample);
                const int batch = std::min(TR, std::min(rg + R, g.n) - i);
                uint32_t maxdeg = 0;
                d = gcn_extra_feature_profile::tick(sample);
                for(int n = 0; n < batch; n++) {
                    rows[n] = perm[i + n];
                    bases[n] = g.row[rows[n]];
                    degrees[n] = g.row[rows[n] + 1] - bases[n];
                    maxdeg = std::max(maxdeg, degrees[n]);
                }
                detail_tock(pr, ROW_METADATA_READ, d, sample);
                const uint32_t step = method.block == 0 ? std::max(1u, maxdeg) : uint32_t(method.block);
                tock<true>(pr, SCHEDULE, t, sample);

                if(maxdeg == 0) {
                    t = gcn_extra_feature_profile::tick(sample);
                    for(int n = 0; n < batch; n++)
                        std::memset(c + size_t(rows[n]) * DIM, 0, DIM * sizeof(*c));
                    detail_tock(pr, OUTPUT_STORE, t, sample);
                    tock<true>(pr, SCATTER, t, sample);
                    pr.counters.output_store_bytes += uint64_t(batch) * DIM * sizeof(*c);
                    continue;
                }

                // Frozen B64/FULL: one reduce_tile and one packed projection
                // per start, with both output panels consuming that partial.
                for(uint32_t start = 0; start < maxdeg; start += step) {
                    gcn_extra_feature_profile::reduce_tile<FP32_INPUT>(
                        g, hb, bases, degrees, batch, start, step, partial, hi, pr, sample);
                    for(int obp = 0; obp < NP; obp++) {
                        t = gcn_extra_feature_profile::tick(sample);
                        if(start == 0) {
                            _tile_zero(TC0); _tile_zero(TC1);
                            _tile_zero(TC2); _tile_zero(TC3);
                            detail_tock(pr, AMX_C_ZERO, t, sample);
                        } else {
                            _tile_loadd(TC0, ctile + obp * 64, 512);
                            _tile_loadd(TC1, ctile + obp * 64 + 16, 512);
                            _tile_loadd(TC2, ctile + obp * 64 + 32, 512);
                            _tile_loadd(TC3, ctile + obp * 64 + 48, 512);
                            detail_tock(pr, AMX_C_LOAD, t, sample);
                            pr.counters.amx_accumulator_loads += 4;
                            pr.counters.amx_reload_bytes += 4 * uint64_t(TR * 64);
                        }
                        tock<true>(pr, start == 0 ? REDUCE_ZERO : TILE_LOAD, t, sample);

                        gcn_extra_feature_profile::project_panel(hi, w, obp, pr, sample);
                        pr.counters.projection_panels++;
                        pr.counters.amx_a_loads += KB;
                        pr.counters.amx_b_loads += KB * 4;
                        pr.counters.amx_input_loads += KB * 5;
                        pr.counters.amx_dpbf16ps += KB * 4;
                        pr.counters.amx_executed_flops += KB * 4 * uint64_t(2 * 16 * 16 * 32);

                        t = gcn_extra_feature_profile::tick(sample);
                        _tile_stored(TC0, ctile + obp * 64, 512);
                        _tile_stored(TC1, ctile + obp * 64 + 16, 512);
                        _tile_stored(TC2, ctile + obp * 64 + 32, 512);
                        _tile_stored(TC3, ctile + obp * 64 + 48, 512);
                        detail_tock(pr, AMX_C_STORE, t, sample);
                        tock<true>(pr, TILE_STORE, t, sample);
                        pr.counters.amx_accumulator_stores += 4;
                        pr.counters.amx_spill_bytes += 4 * uint64_t(TR * 64);
                    }
                }
                t = gcn_extra_feature_profile::tick(sample);
                for(int n = 0; n < batch; n++)
                    gcn_extra_feature_profile::scatter_output<B16>(
                        c + size_t(rows[n]) * DIM, ctile + n * DIM, DIM, pr, sample);
                tock<true>(pr, SCATTER, t, sample);
                pr.counters.output_store_bytes += uint64_t(batch) * DIM * sizeof(*c);
            }
        }
        const double active_end = gcn_extra_feature_profile::tick(true);
        pr.thread.kernel_active_s = active_end - active_begin;
        pr.thread.finish_offset_s = active_end - epoch;
        d = gcn_extra_feature_profile::tick(true);
        _tile_release();
        detail_tock(pr, THREAD_TILE_RELEASE, d, true);
        pr.thread.release_s = omp_get_wtime() - d;
        pr.thread.completed_tiles = pr.counters.destination_tiles;
        pr.thread.completed_edges = pr.counters.source_visits;
        pr.thread.counters = pr.counters;
        pr.thread.sec = pr.sec;
        pr.thread.detail_sec = pr.detail_sec;
        pr.thread.detail_events = pr.detail_events;
    }

    const uint64_t call = result->kernel_calls++;
    for(const auto& pr : profiles) {
        result->tiles += pr.tiles;
        for(int ph = 0; ph < PHASES; ph++) result->sec[ph] += pr.sec[ph];
        for(int ph = 0; ph < DETAIL_PHASES; ph++) {
            result->detail_sec[ph] += pr.detail_sec[ph];
            result->detail_events[ph] += pr.detail_events[ph];
        }
        add_counters(result->counters, pr.counters);
        if(pr.detail_events[THREAD_PERMISSION] != 0) {
            ThreadProfile thread = pr.thread;
            thread.kernel_call = call;
            result->threads.push_back(thread);
        }
    }
}

} // namespace gcn_extra_feature_profile
