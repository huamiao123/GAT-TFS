#pragma once

#include "paper_methods_kernels.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>

// Isolated experimental extension of the validated paper_methods kernel.
// Only the interleaving of the destination rows inside one projection scope
// changes. The CSR order of the FP32 additions for each row is unchanged.
namespace gcn_extra_projection_a3 {

using gcn_extra_paper::DIM;
using gcn_extra_paper::SourceGraphView;
using gcn_extra_paper::scatter_output;

struct Method {
    std::string name;
    int window = 64;          // S: neighbors visited per row before rotating rows.
    int scope = 64;           // M: neighbors combined before projection; 0 = FULL.
    bool accurate = false;   // Original high-part + residual BF16 representation.
    bool nozero = true;      // Every destination output is explicitly written.
    float* debug_partials = nullptr; // Instrumented FULL partial trace only.
};

// Counts describe executed instructions and logical staging traffic, not PMU
// events or DRAM traffic. They are complete counts (not sampled estimates).
// They are collected only by kernel<true>, separately from timed kernel<false>.
struct Counters {
    uint64_t destination_tiles = 0;
    uint64_t projection_scopes = 0;
    uint64_t neighbor_windows = 0;
    uint64_t row_window_visits = 0;
    uint64_t nonempty_row_windows = 0;
    uint64_t source_visits = 0;
    uint64_t feature_adds = 0;
    uint64_t source_decode_bytes = 0;
    uint64_t partial_zero_bytes = 0;
    uint64_t window_state_load_bytes = 0;
    uint64_t window_state_store_bytes = 0;
    uint64_t csr_prefetch_instructions = 0;
    uint64_t partial_packed_bytes = 0;
    uint64_t projection_panels = 0;
    uint64_t amx_input_loads = 0;
    uint64_t amx_accumulator_loads = 0;
    uint64_t amx_accumulator_stores = 0;
    uint64_t amx_dpbf16ps = 0;
    uint64_t amx_executed_flops = 0;
    uint64_t output_store_bytes = 0;
};

// These are child intervals of the existing phases. Parent and child times
// overlap and must not be added together. All clocks are restricted to sampled
// tiles in kernel<true>; kernel<false> never executes them. detail_events records
// the number of clocked sections, permitting inspection of timer granularity.
enum DetailPhase {
    PARTIAL_INIT,
    CSR_INDEX_READ,
    CSR_NEXT_INDEX_READ,
    PREFETCH_ISSUE,
    SOURCE_BF16_LOAD,
    SOURCE_BF16_DECODE,
    FEATURE_FP32_ADD,
    PARTIAL_STATE_LOAD,
    PARTIAL_STATE_STORE,
    PARTIAL_HI_PACK,
    PARTIAL_LO_RESIDUAL,
    PARTIAL_LO_PACK,
    AMX_A_LOAD,
    AMX_B_LOAD,
    AMX_TDP_COMPUTE,
    AMX_C_ZERO,
    AMX_C_LOAD,
    AMX_C_STORE,
    OUTPUT_BF16_CONVERSION,
    OUTPUT_STORE,
    THREAD_PERMISSION,
    THREAD_TILE_CONFIG,
    THREAD_TILE_RELEASE,
    ROW_METADATA_READ,
    DETAIL_PHASES
};

inline constexpr const char* detail_phase_name[DETAIL_PHASES] = {
    "partial_init", "csr_index_read", "csr_next_index_read", "prefetch_issue",
    "source_bf16_load", "source_bf16_decode", "feature_fp32_add",
    "partial_state_load", "partial_state_store", "partial_hi_pack",
    "partial_lo_residual", "partial_lo_pack", "amx_a_load", "amx_b_load",
    "amx_tdp_compute", "amx_c_zero", "amx_c_load", "amx_c_store",
    "output_bf16_conversion", "output_store", "thread_permission",
    "thread_tile_config", "thread_tile_release", "row_metadata_read"
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

// Keeping the base type preserves the original coarse timing API. The detailed
// projection helper below retains the original AMX instruction sequence.
struct Profile : gcn_extra_paper::Profile {
    double window_state_load_s = 0;
    double window_state_store_s = 0;
    Counters counters;
    std::array<double, DETAIL_PHASES> detail_sec{};
    std::array<uint64_t, DETAIL_PHASES> detail_events{};
    uint64_t kernel_calls = 0;
    ThreadProfile thread;
    std::vector<ThreadProfile> threads;
};

template<bool P>
inline void detail_tock(Profile& pr, DetailPhase phase, double start, bool sample) {
    if constexpr(P) {
        if(sample) {
            pr.detail_sec[phase] += omp_get_wtime() - start;
            pr.detail_events[phase]++;
        }
    }
}

// Same tile instructions, layout and order as project_panel, with the A load,
// B loads and TDP operations individually exposed in the profiling variant.
template<bool P>
inline void project_panel_detailed(const uint16_t* hb, const uint16_t* w,
                                   int obp, Profile& pr, bool sample) {
    using namespace gcn_extra_paper;
    for(int kb = 0; kb < KB; kb++) {
        double t = tick<P>(sample);
        double d = tick<P>(sample);
        _tile_loadd(TA, reinterpret_cast<const uint8_t*>(hb) + kb * 64, DIM * 2);
        detail_tock<P>(pr, AMX_A_LOAD, d, sample);
        const int ob0 = obp * 4;
        d = tick<P>(sample);
        _tile_loadd(TB0, w + ((kb * NB + ob0) * 16 * 32), 64);
        _tile_loadd(TB1, w + ((kb * NB + ob0 + 1) * 16 * 32), 64);
        detail_tock<P>(pr, AMX_B_LOAD, d, sample);
        tock<P>(pr, TILE_LOAD, t, sample);

        t = tick<P>(sample);
        _tile_dpbf16ps(TC0, TA, TB0);
        _tile_dpbf16ps(TC1, TA, TB1);
        detail_tock<P>(pr, AMX_TDP_COMPUTE, t, sample);
        tock<P>(pr, TILE_COMPUTE, t, sample);

        t = tick<P>(sample);
        _tile_loadd(TB0, w + ((kb * NB + ob0 + 2) * 16 * 32), 64);
        _tile_loadd(TB1, w + ((kb * NB + ob0 + 3) * 16 * 32), 64);
        detail_tock<P>(pr, AMX_B_LOAD, t, sample);
        tock<P>(pr, TILE_LOAD, t, sample);

        t = tick<P>(sample);
        _tile_dpbf16ps(TC2, TA, TB0);
        _tile_dpbf16ps(TC3, TA, TB1);
        detail_tock<P>(pr, AMX_TDP_COMPUTE, t, sample);
        tock<P>(pr, TILE_COMPUTE, t, sample);
    }
}

template<bool P, bool B16>
inline void scatter_detailed(std::conditional_t<B16, uint16_t, float>* dst,
                             const float* src, int count, Profile& pr, bool sample) {
    using gcn_extra_paper::tick;
    if constexpr(P) {
        if(sample) {
            if constexpr(B16) {
                // Profiling only: conversion and the destination store need
                // independent intervals. The temporary preserves every output
                // bit; the performance variant retains the direct scatter.
                alignas(64) uint16_t converted[DIM];
                double t = tick<P>(sample);
                for(int k = 0; k < count; k++) converted[k] = ::f32_to_bf16(src[k]);
                detail_tock<P>(pr, OUTPUT_BF16_CONVERSION, t, sample);
                t = tick<P>(sample);
                std::memcpy(dst, converted, size_t(count) * sizeof(uint16_t));
                detail_tock<P>(pr, OUTPUT_STORE, t, sample);
            } else {
                const double t = tick<P>(sample);
                std::memcpy(dst, src, size_t(count) * sizeof(float));
                detail_tock<P>(pr, OUTPUT_STORE, t, sample);
            }
            return;
        }
    }
    scatter_output<B16>(dst, src, count);
}

inline void add_counters(Counters& dst, const Counters& src) {
    #define GCN_EXTRA_ADD_COUNTER(FIELD) dst.FIELD += src.FIELD
    GCN_EXTRA_ADD_COUNTER(destination_tiles);
    GCN_EXTRA_ADD_COUNTER(projection_scopes);
    GCN_EXTRA_ADD_COUNTER(neighbor_windows);
    GCN_EXTRA_ADD_COUNTER(row_window_visits);
    GCN_EXTRA_ADD_COUNTER(nonempty_row_windows);
    GCN_EXTRA_ADD_COUNTER(source_visits);
    GCN_EXTRA_ADD_COUNTER(feature_adds);
    GCN_EXTRA_ADD_COUNTER(source_decode_bytes);
    GCN_EXTRA_ADD_COUNTER(partial_zero_bytes);
    GCN_EXTRA_ADD_COUNTER(window_state_load_bytes);
    GCN_EXTRA_ADD_COUNTER(window_state_store_bytes);
    GCN_EXTRA_ADD_COUNTER(csr_prefetch_instructions);
    GCN_EXTRA_ADD_COUNTER(partial_packed_bytes);
    GCN_EXTRA_ADD_COUNTER(projection_panels);
    GCN_EXTRA_ADD_COUNTER(amx_input_loads);
    GCN_EXTRA_ADD_COUNTER(amx_accumulator_loads);
    GCN_EXTRA_ADD_COUNTER(amx_accumulator_stores);
    GCN_EXTRA_ADD_COUNTER(amx_dpbf16ps);
    GCN_EXTRA_ADD_COUNTER(amx_executed_flops);
    GCN_EXTRA_ADD_COUNTER(output_store_bytes);
    #undef GCN_EXTRA_ADD_COUNTER
}

template<bool P>
inline void reduce_scope(const SourceGraphView& g, const uint16_t* hb,
                         const uint32_t* bases, const uint32_t* degrees,
                         int batch, uint32_t scope_start, uint32_t scope_end,
                         uint32_t window, float* partial, uint16_t* hi,
                         uint16_t* lo, bool accurate, Profile& pr, bool sample) {
    using namespace gcn_extra_paper;
    double t = tick<P>(sample);
    std::memset(partial, 0, TR * DIM * sizeof(float));
    detail_tock<P>(pr, PARTIAL_INIT, t, sample);
    tock<P>(pr, REDUCE_ZERO, t, sample);
    if constexpr(P) {
        pr.counters.projection_scopes++;
        pr.counters.partial_zero_bytes += TR * DIM * sizeof(float);
    }

    // The end calculations use subtraction before addition, so even large
    // uint32_t degrees and a nondivisible final scope/window cannot overflow.
    if constexpr(P) pr.counters.neighbor_windows +=
        1 + (scope_end - scope_start - 1) / window;
    for(int n = 0; n < batch; n++) {
        for(uint32_t window_start = scope_start; window_start < scope_end;
            window_start += std::min(window, scope_end - window_start)) {
            const uint32_t window_end = window_start +
                std::min(window, scope_end - window_start);
            const bool first_window = window_start == scope_start;
            const uint32_t row_scope_end = std::min(degrees[n], scope_end);
            const uint32_t row_window_end = std::min(row_scope_end, window_end);
            // The initial row store is retained for the coupled S=M control.
            // Exhausted rows need no subsequent state reload or store.
            if(!first_window && window_start >= row_window_end) continue;

            __m512 acc[8];
            if(first_window) {
                for(auto& v : acc) v = _mm512_setzero_ps();
            } else {
                t = tick<P>(sample);
                for(int f = 0; f < 8; f++)
                    acc[f] = _mm512_load_ps(partial + n * DIM + f * 16);
                detail_tock<P>(pr, PARTIAL_STATE_LOAD, t, sample);
                if constexpr(P) {
                    if(sample) pr.window_state_load_s += omp_get_wtime() - t;
                    pr.counters.window_state_load_bytes += DIM * sizeof(float);
                }
            }

            if constexpr(P) {
                pr.counters.row_window_visits++;
                if(window_start < row_window_end) {
                    const uint64_t edges = row_window_end - window_start;
                    pr.counters.nonempty_row_windows++;
                    pr.counters.source_visits += edges;
                    pr.counters.feature_adds += edges * DIM;
                    pr.counters.source_decode_bytes += edges * DIM * sizeof(uint16_t);
                    // Preserve the original scope's prefetch boundary. A
                    // window boundary does not add/remove prefetches.
                    const uint64_t last_prefetch_omitted =
                        row_window_end == row_scope_end ? 1 : 0;
                    pr.counters.csr_prefetch_instructions +=
                        (edges - last_prefetch_omitted) * 4;
                }
            }

            for(uint32_t k = window_start; k < row_window_end; k++) {
                t = tick<P>(sample);
                double d = tick<P>(sample);
                const uint32_t j = g.col[size_t(bases[n]) + k];
                detail_tock<P>(pr, CSR_INDEX_READ, d, sample);
                if(k + 1 < row_scope_end) {
                    d = tick<P>(sample);
                    const uint32_t next_j = g.col[size_t(bases[n]) + k + 1];
                    detail_tock<P>(pr, CSR_NEXT_INDEX_READ, d, sample);
                    const char* next = reinterpret_cast<const char*>(
                        hb + size_t(next_j) * DIM);
                    d = tick<P>(sample);
                    for(int o = 0; o < 256; o += 64)
                        _mm_prefetch(next + o, _MM_HINT_T0);
                    detail_tock<P>(pr, PREFETCH_ISSUE, d, sample);
                }
                tock<P>(pr, CSR_PREFETCH, t, sample);

                t = tick<P>(sample);
                __m512 val[8];
                const uint16_t* src = hb + size_t(j) * DIM;
                if constexpr(P) {
                    if(sample) {
                        // Only the sampled variant groups the independent
                        // loads to distinguish load and decode intervals.
                        // The timed variant below keeps the original order.
                        __m256i raw[8];
                        d = tick<P>(sample);
                        for(int f = 0; f < 8; f++)
                            raw[f] = _mm256_loadu_si256(
                                reinterpret_cast<const __m256i*>(src + f * 16));
                        detail_tock<P>(pr, SOURCE_BF16_LOAD, d, sample);
                        d = tick<P>(sample);
                        for(int f = 0; f < 8; f++)
                            val[f] = _mm512_castsi512_ps(_mm512_slli_epi32(
                                _mm512_cvtepu16_epi32(raw[f]), 16));
                        detail_tock<P>(pr, SOURCE_BF16_DECODE, d, sample);
                    } else {
                        for(int f = 0; f < 8; f++)
                            val[f] = _mm512_castsi512_ps(_mm512_slli_epi32(
                                _mm512_cvtepu16_epi32(_mm256_loadu_si256(
                                    reinterpret_cast<const __m256i*>(src + f * 16))), 16));
                    }
                } else {
                    for(int f = 0; f < 8; f++)
                        val[f] = _mm512_castsi512_ps(_mm512_slli_epi32(
                            _mm512_cvtepu16_epi32(_mm256_loadu_si256(
                                reinterpret_cast<const __m256i*>(src + f * 16))), 16));
                }
                tock<P>(pr, GATHER_DECODE, t, sample);

                t = tick<P>(sample);
                for(int f = 0; f < 8; f++)
                    acc[f] = _mm512_add_ps(acc[f], val[f]);
                detail_tock<P>(pr, FEATURE_FP32_ADD, t, sample);
                tock<P>(pr, REDUCE_ADD, t, sample);
            }

            t = tick<P>(sample);
            for(int f = 0; f < 8; f++)
                _mm512_store_ps(partial + n * DIM + f * 16, acc[f]);
            detail_tock<P>(pr, PARTIAL_STATE_STORE, t, sample);
            if constexpr(P) {
                if(sample) pr.window_state_store_s += omp_get_wtime() - t;
                pr.counters.window_state_store_bytes += DIM * sizeof(float);
            }
        }
    }

    // Identical truncation and residual arithmetic to reduce_tile. No native
    // BF16 rounding instruction and no reassociation across projection scopes.
    t = tick<P>(sample);
    for(int v = 0; v < TR * DIM; v += 16) {
        double d = tick<P>(sample);
        const __m512 a = _mm512_load_ps(partial + v);
        const __m512i bits = _mm512_castps_si512(a);
        const __m256i top = _mm512_cvtepi32_epi16(_mm512_srli_epi32(bits, 16));
        _mm256_store_si256(reinterpret_cast<__m256i*>(hi + v), top);
        detail_tock<P>(pr, PARTIAL_HI_PACK, d, sample);
        if(accurate) {
            d = tick<P>(sample);
            const __m512 quant = _mm512_castsi512_ps(_mm512_slli_epi32(
                _mm512_cvtepu16_epi32(top), 16));
            const __m512 residual = _mm512_sub_ps(a, quant);
            detail_tock<P>(pr, PARTIAL_LO_RESIDUAL, d, sample);
            d = tick<P>(sample);
            _mm256_store_si256(reinterpret_cast<__m256i*>(lo + v),
                _mm512_cvtepi32_epi16(_mm512_srli_epi32(
                    _mm512_castps_si512(residual), 16)));
            detail_tock<P>(pr, PARTIAL_LO_PACK, d, sample);
        }
    }
    tock<P>(pr, CONVERT, t, sample);
    if constexpr(P) {
        pr.counters.partial_packed_bytes +=
            TR * DIM * sizeof(uint16_t) * (accurate ? 2 : 1);
    }
}

template<bool P, bool B16 = false>
void kernel(const SourceGraphView& g, const uint16_t* hb, const uint16_t* w,
            std::conditional_t<B16, uint16_t, float>* c, const int* perm,
            const Method& method, Profile* result) {
    using namespace gcn_extra_paper;
    if(method.window <= 0 || method.scope < 0)
        throw std::invalid_argument("projection window must be positive; scope must be nonnegative");
    if constexpr(P) {
        if(result == nullptr)
            throw std::invalid_argument("profile result is required for kernel<true>");
    }

    constexpr int R = 64;
    std::vector<Profile> profiles(omp_get_max_threads());
    const double z = tick<P>(true);
    if(!method.nozero) std::memset(c, 0, size_t(g.n) * DIM * sizeof(*c));
    if constexpr(P) profiles[0].sec[ZERO] = omp_get_wtime() - z;
    const double parallel_epoch = tick<P>(true);

    #pragma omp parallel
    {
        Profile& pr = profiles[omp_get_thread_num()];
        double t = tick<P>(true);
        if constexpr(P) pr.thread.thread_id = omp_get_thread_num();
        double d = tick<P>(true);
        if(syscall(SYS_arch_prctl, 0x1023, 18) != 0) {
            perror("AMX permission");
            abort();
        }
        detail_tock<P>(pr, THREAD_PERMISSION, d, true);
        d = tick<P>(true);
        tilecfg_t cfg;
        setup_tilecfg(&cfg);
        _tile_loadconfig(&cfg);
        detail_tock<P>(pr, THREAD_TILE_CONFIG, d, true);
        tock<P>(pr, SETUP, t, true);
        if constexpr(P) pr.thread.setup_s = omp_get_wtime() - t;

        alignas(64) float partial[TR * DIM], ctile[TR * DIM];
        alignas(64) uint16_t hi[TR * DIM], lo[TR * DIM];
        uint32_t bases[TR], degrees[TR];
        int rows[TR];
        const double active_begin = tick<P>(true);
        if constexpr(P) pr.thread.start_offset_s = active_begin - parallel_epoch;

        #pragma omp for schedule(dynamic,1) nowait
        for(int rg = 0; rg < g.n; rg += R) {
            for(int i = rg; i < std::min(rg + R, g.n); i += TR) {
                const bool sample = P &&
                    (((i / TR) % PROFILE_STRIDE) == 0 || i + TR >= g.n);
                if(sample) pr.tiles++;
                if constexpr(P) pr.counters.destination_tiles++;

                t = tick<P>(sample);
                const int batch = std::min(TR, std::min(rg + R, g.n) - i);
                uint32_t maxdeg = 0;
                d = tick<P>(sample);
                for(int n = 0; n < batch; n++) {
                    rows[n] = perm[i + n];
                    bases[n] = g.row[rows[n]];
                    degrees[n] = g.row[rows[n] + 1] - bases[n];
                    maxdeg = std::max(maxdeg, degrees[n]);
                }
                detail_tock<P>(pr, ROW_METADATA_READ, d, sample);
                const uint32_t scope = method.scope == 0 ?
                    std::max(1u, maxdeg) : uint32_t(method.scope);
                tock<P>(pr, SCHEDULE, t, sample);

                if(maxdeg == 0) {
                    t = tick<P>(sample);
                    for(int n = 0; n < batch; n++)
                        std::memset(c + size_t(rows[n]) * DIM, 0, DIM * sizeof(*c));
                    detail_tock<P>(pr, OUTPUT_STORE, t, sample);
                    tock<P>(pr, SCATTER, t, sample);
                    if constexpr(P)
                        pr.counters.output_store_bytes += uint64_t(batch) * DIM * sizeof(*c);
                    continue;
                }

                for(uint32_t start = 0; start < maxdeg;) {
                    const uint32_t end = start + std::min(scope, maxdeg - start);
                    reduce_scope<P>(g, hb, bases, degrees, batch, start, end,
                                    uint32_t(method.window), partial, hi, lo,
                                    method.accurate, pr, sample);
                    if constexpr(P) {
                        if(method.debug_partials != nullptr && method.scope == 0)
                            for(int r = 0; r < batch; r++)
                                std::memcpy(method.debug_partials + size_t(rows[r]) * DIM,
                                            partial + size_t(r) * DIM,
                                            DIM * sizeof(float));
                    }
                    for(int obp = 0; obp < NP; obp++) {
                        t = tick<P>(sample);
                        if(start == 0) {
                            _tile_zero(TC0); _tile_zero(TC1);
                            _tile_zero(TC2); _tile_zero(TC3);
                            detail_tock<P>(pr, AMX_C_ZERO, t, sample);
                        } else {
                            _tile_loadd(TC0, ctile + obp * 64, 512);
                            _tile_loadd(TC1, ctile + obp * 64 + 16, 512);
                            _tile_loadd(TC2, ctile + obp * 64 + 32, 512);
                            _tile_loadd(TC3, ctile + obp * 64 + 48, 512);
                            detail_tock<P>(pr, AMX_C_LOAD, t, sample);
                        }
                        tock<P>(pr, start == 0 ? REDUCE_ZERO : TILE_LOAD, t, sample);

                        project_panel_detailed<P>(hi, w, obp, pr, sample);
                        if(method.accurate) project_panel_detailed<P>(lo, w, obp, pr, sample);
                        if constexpr(P) {
                            const uint64_t parts = method.accurate ? 2 : 1;
                            pr.counters.projection_panels++;
                            pr.counters.amx_input_loads += parts * KB * 5;
                            pr.counters.amx_accumulator_loads += start == 0 ? 0 : 4;
                            pr.counters.amx_accumulator_stores += 4;
                            pr.counters.amx_dpbf16ps += parts * KB * 4;
                            // One TDPBF16PS computes a 16 x 16 x 32 product.
                            // This includes padded destination rows and zeros.
                            pr.counters.amx_executed_flops +=
                                parts * KB * 4 * uint64_t(2 * 16 * 16 * 32);
                        }

                        t = tick<P>(sample);
                        _tile_stored(TC0, ctile + obp * 64, 512);
                        _tile_stored(TC1, ctile + obp * 64 + 16, 512);
                        _tile_stored(TC2, ctile + obp * 64 + 32, 512);
                        _tile_stored(TC3, ctile + obp * 64 + 48, 512);
                        detail_tock<P>(pr, AMX_C_STORE, t, sample);
                        tock<P>(pr, TILE_STORE, t, sample);
                    }
                    start = end;
                }

                t = tick<P>(sample);
                for(int n = 0; n < batch; n++)
                    scatter_detailed<P, B16>(c + size_t(rows[n]) * DIM,
                                            ctile + n * DIM, DIM, pr, sample);
                tock<P>(pr, SCATTER, t, sample);
                if constexpr(P)
                    pr.counters.output_store_bytes += uint64_t(batch) * DIM * sizeof(*c);
            }
        }
        const double active_end = tick<P>(true);
        if constexpr(P) {
            pr.thread.kernel_active_s = active_end - active_begin;
            pr.thread.finish_offset_s = active_end - parallel_epoch;
        }
        d = tick<P>(true);
        _tile_release();
        detail_tock<P>(pr, THREAD_TILE_RELEASE, d, true);
        if constexpr(P) {
            pr.thread.release_s = omp_get_wtime() - d;
            pr.thread.completed_tiles = pr.counters.destination_tiles;
            pr.thread.completed_edges = pr.counters.source_visits;
            pr.thread.counters = pr.counters;
            pr.thread.sec = pr.sec;
            pr.thread.detail_sec = pr.detail_sec;
            pr.thread.detail_events = pr.detail_events;
        }
    }

    if constexpr(P) {
        const uint64_t call = result->kernel_calls++;
        for(const auto& pr : profiles) {
            result->tiles += pr.tiles;
            for(int s = 0; s < PHASES; s++) result->sec[s] += pr.sec[s];
            result->window_state_load_s += pr.window_state_load_s;
            result->window_state_store_s += pr.window_state_store_s;
            add_counters(result->counters, pr.counters);
            for(int s = 0; s < DETAIL_PHASES; s++) {
                result->detail_sec[s] += pr.detail_sec[s];
                result->detail_events[s] += pr.detail_events[s];
            }
            // OpenMP may create fewer threads than omp_get_max_threads().
            // A participating thread always records one permission section.
            if(pr.detail_events[THREAD_PERMISSION] != 0) {
                ThreadProfile thread = pr.thread;
                thread.kernel_call = call;
                result->threads.push_back(thread);
            }
        }
    }
}

} // namespace gcn_extra_projection_a3
