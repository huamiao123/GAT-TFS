#pragma once
#include "projection_window_kernels.hpp"

// Isolated inference experiment. Logical counters below describe source-level
// requests, not retired instructions or measured cache/DRAM traffic.
namespace gcn_extra_state {
using namespace gcn_extra_projection;
using gcn_extra_paper::SourceGraphView;
using gcn_extra_paper::DIM;

struct Method {
    std::string name;
    int window = 64;
    int scope = 64; // 0 = FULL
    bool accurate = false;
    bool nozero_partial = false;
    bool direct_pack = false;
    bool relu_output = false; // Layer 1 BF16 output only.
};
struct Logical {
    uint64_t packing_partial_load_bytes = 0;
    uint64_t packed_tail_zero_bytes = 0;
    uint64_t direct_pack_rows = 0;
    uint64_t decoded_source_rows = 0;
    uint64_t csr_index_reads = 0;
    uint64_t prefetch_index_reads = 0;
    uint64_t pair_comparisons = 0;
    uint64_t pair_shared_sources = 0;
    uint64_t relu_elements = 0;
    uint64_t relu_negative_elements = 0;
};
struct Profile : gcn_extra_projection::Profile { Logical logical; };
inline void add_logical(Logical& a, const Logical& b) {
#define STATE_ADD(X) a.X += b.X
    STATE_ADD(packing_partial_load_bytes); STATE_ADD(packed_tail_zero_bytes);
    STATE_ADD(direct_pack_rows); STATE_ADD(decoded_source_rows);
    STATE_ADD(csr_index_reads); STATE_ADD(prefetch_index_reads);
    STATE_ADD(pair_comparisons); STATE_ADD(pair_shared_sources);
    STATE_ADD(relu_elements); STATE_ADD(relu_negative_elements);
#undef STATE_ADD
}

// Helpers are shared verbatim by ScalarReducer and the independent PairReducer.
// A first window initializes all real row accumulators, including exhausted rows.
// Only padding rows require an explicit memory initialization when NOZERO is set.
template<bool P>
inline void init_scope(float* partial, uint16_t* hi, uint16_t* lo, int batch,
                       const Method& method, Profile& pr, bool sample) {
    using namespace gcn_extra_paper;
    double t = tick<P>(sample);
    size_t zero_bytes = 0;
    if(!method.nozero_partial) {
        // Deliberately retained for the DIRECTPACK-only ablation.
        std::memset(partial, 0, TR * DIM * sizeof(float));
        zero_bytes = TR * DIM * sizeof(float);
    } else if(!method.direct_pack && batch < TR) {
        zero_bytes = size_t(TR - batch) * DIM * sizeof(float);
        std::memset(partial + size_t(batch) * DIM, 0, zero_bytes);
    }
    detail_tock<P>(pr, PARTIAL_INIT, t, sample);
    tock<P>(pr, REDUCE_ZERO, t, sample);
    if constexpr(P) {
        pr.counters.projection_scopes++;
        pr.counters.partial_zero_bytes += zero_bytes;
    }
    if(method.direct_pack && batch < TR) {
        const size_t bytes = size_t(TR - batch) * DIM * sizeof(uint16_t);
        std::memset(hi + size_t(batch) * DIM, 0, bytes);
        if(method.accurate) std::memset(lo + size_t(batch) * DIM, 0, bytes);
        if constexpr(P) {
            const size_t all_bytes = bytes * (method.accurate ? 2 : 1);
            pr.logical.packed_tail_zero_bytes += all_bytes;
            pr.counters.partial_packed_bytes += all_bytes;
        }
    }
}

template<bool P>
inline void load_row(const float* partial, int row, bool first_window,
                     __m512* acc, Profile& pr, bool sample) {
    using namespace gcn_extra_paper;
    if(first_window) {
        for(int f = 0; f < 8; f++) acc[f] = _mm512_setzero_ps();
    } else {
        double t = tick<P>(sample);
        for(int f = 0; f < 8; f++)
            acc[f] = _mm512_load_ps(partial + row * DIM + f * 16);
        detail_tock<P>(pr, PARTIAL_STATE_LOAD, t, sample);
        if constexpr(P) {
            if(sample) pr.window_state_load_s += omp_get_wtime() - t;
            pr.counters.window_state_load_bytes += DIM * sizeof(float);
        }
    }
}

template<bool P>
inline void pack_vector(__m512 a, uint16_t* hi, uint16_t* lo, bool accurate,
                        Profile& pr, bool sample) {
    using namespace gcn_extra_paper;
    double t = tick<P>(sample);
    const __m256i top = _mm512_cvtepi32_epi16(
        _mm512_srli_epi32(_mm512_castps_si512(a), 16));
    _mm256_store_si256(reinterpret_cast<__m256i*>(hi), top);
    detail_tock<P>(pr, PARTIAL_HI_PACK, t, sample);
    if(accurate) {
        t = tick<P>(sample);
        const __m512 high = _mm512_castsi512_ps(
            _mm512_slli_epi32(_mm512_cvtepu16_epi32(top), 16));
        const __m512 residual = _mm512_sub_ps(a, high);
        detail_tock<P>(pr, PARTIAL_LO_RESIDUAL, t, sample);
        t = tick<P>(sample);
        _mm256_store_si256(reinterpret_cast<__m256i*>(lo),
            _mm512_cvtepi32_epi16(_mm512_srli_epi32(
                _mm512_castps_si512(residual), 16)));
        detail_tock<P>(pr, PARTIAL_LO_PACK, t, sample);
    }
}

template<bool P>
inline void finish_row(const __m512* acc, int row, bool final_window,
                       float* partial, uint16_t* hi, uint16_t* lo,
                       const Method& method, Profile& pr, bool sample) {
    using namespace gcn_extra_paper;
    const double t = tick<P>(sample);
    if(method.direct_pack && final_window) {
        for(int f = 0; f < 8; f++)
            pack_vector<P>(acc[f], hi + row * DIM + f * 16,
                lo + row * DIM + f * 16, method.accurate, pr, sample);
        tock<P>(pr, CONVERT, t, sample);
        if constexpr(P) {
            pr.logical.direct_pack_rows++;
            pr.counters.partial_packed_bytes +=
                DIM * sizeof(uint16_t) * (method.accurate ? 2 : 1);
        }
    } else {
        for(int f = 0; f < 8; f++)
            _mm512_store_ps(partial + row * DIM + f * 16, acc[f]);
        detail_tock<P>(pr, PARTIAL_STATE_STORE, t, sample);
        if constexpr(P) {
            if(sample) pr.window_state_store_s += omp_get_wtime() - t;
            pr.counters.window_state_store_bytes += DIM * sizeof(float);
        }
    }
}

template<bool P>
inline void finish_scope(const float* partial, uint16_t* hi, uint16_t* lo,
                         const Method& method, Profile& pr, bool sample) {
    if(method.direct_pack) return;
    using namespace gcn_extra_paper;
    double t = tick<P>(sample);
    for(int v = 0; v < TR * DIM; v += 16)
        pack_vector<P>(_mm512_load_ps(partial + v), hi + v, lo + v,
                       method.accurate, pr, sample);
    tock<P>(pr, CONVERT, t, sample);
    if constexpr(P) {
        pr.logical.packing_partial_load_bytes += TR * DIM * sizeof(float);
        pr.counters.partial_packed_bytes +=
            TR * DIM * sizeof(uint16_t) * (method.accurate ? 2 : 1);
    }
}

struct ScalarReducer {
    template<bool P>
    static inline void run(const SourceGraphView& g, const uint16_t* hb,
                           const uint32_t* bases, const uint32_t* degrees,
                           int batch, uint32_t scope_start, uint32_t scope_end,
                           uint32_t window, float* partial, uint16_t* hi,
                           uint16_t* lo, const Method& method,
                           Profile& pr, bool sample) {
        using namespace gcn_extra_paper;
        init_scope<P>(partial, hi, lo, batch, method, pr, sample);
        for(uint32_t window_start = scope_start; window_start < scope_end;) {
            const uint32_t window_end =
                window_start + std::min(window, scope_end - window_start);
            const bool first_window = window_start == scope_start;
            if constexpr(P) pr.counters.neighbor_windows++;
            for(int n = 0; n < batch; n++) {
                const uint32_t row_scope_end = std::min(degrees[n], scope_end);
                const uint32_t row_window_end = std::min(row_scope_end, window_end);
                if(!first_window && window_start >= row_window_end) continue;
                __m512 acc[8];
                load_row<P>(partial, n, first_window, acc, pr, sample);
                if constexpr(P) {
                    pr.counters.row_window_visits++;
                    if(window_start < row_window_end) {
                        const uint64_t edges = row_window_end - window_start;
                        pr.counters.nonempty_row_windows++;
                        pr.counters.source_visits += edges;
                        pr.counters.feature_adds += edges * DIM;
                        pr.counters.source_decode_bytes += edges * DIM * sizeof(uint16_t);
                        pr.logical.decoded_source_rows += edges;
                        pr.logical.csr_index_reads += edges;
                        const uint64_t omitted = row_window_end == row_scope_end ? 1 : 0;
                        pr.counters.csr_prefetch_instructions += (edges - omitted) * 4;
                        pr.logical.prefetch_index_reads += edges - omitted;
                    }
                }
                for(uint32_t k = window_start; k < row_window_end; k++) {
                    double t = tick<P>(sample);
                    double d = tick<P>(sample);
                    const uint32_t j = g.col[size_t(bases[n]) + k];
                    detail_tock<P>(pr, CSR_INDEX_READ, d, sample);
                    if(k + 1 < row_scope_end) {
                        d = tick<P>(sample);
                        const uint32_t next_j = g.col[size_t(bases[n]) + k + 1];
                        detail_tock<P>(pr, CSR_NEXT_INDEX_READ, d, sample);
                        const char* next = reinterpret_cast<const char*>(hb + size_t(next_j) * DIM);
                        d = tick<P>(sample);
                        for(int o = 0; o < 256; o += 64)
                            _mm_prefetch(next + o, _MM_HINT_T0);
                        detail_tock<P>(pr, PREFETCH_ISSUE, d, sample);
                    }
                    tock<P>(pr, CSR_PREFETCH, t, sample);
                    t = tick<P>(sample);
                    __m512 val[8];
                    const uint16_t* src = hb + size_t(j) * DIM;
                    for(int f = 0; f < 8; f++)
                        val[f] = _mm512_castsi512_ps(_mm512_slli_epi32(
                            _mm512_cvtepu16_epi32(_mm256_loadu_si256(
                                reinterpret_cast<const __m256i*>(src + f * 16))), 16));
                    tock<P>(pr, GATHER_DECODE, t, sample);
                    t = tick<P>(sample);
                    for(int f = 0; f < 8; f++) acc[f] = _mm512_add_ps(acc[f], val[f]);
                    detail_tock<P>(pr, FEATURE_FP32_ADD, t, sample);
                    tock<P>(pr, REDUCE_ADD, t, sample);
                }
                // Every real row reaches this once at its final window, including
                // rows already exhausted before this scope (first window zeros).
                finish_row<P>(acc, n, row_window_end == row_scope_end,
                              partial, hi, lo, method, pr, sample);
            }
            window_start = window_end;
        }
        finish_scope<P>(partial, hi, lo, method, pr, sample);
    }
};

template<bool P, bool B16>
inline void scatter_state(std::conditional_t<B16, uint16_t, float>* dst,
                          const float* src, int count, const Method& method,
                          Profile& pr, bool sample) {
    if(!method.relu_output) {
        scatter_detailed<P, B16>(dst, src, count, pr, sample);
        return;
    }
    if constexpr(B16) {
        // Ordered x<0 preserves -0 and NaN payloads, exactly as relu_f32.
        // Integer truncation deliberately does not use RNE BF16 instructions.
        int k = 0;
        for(; k + 16 <= count; k += 16) {
            const __m512 v = _mm512_loadu_ps(src + k);
            const __mmask16 neg = _mm512_cmp_ps_mask(v, _mm512_setzero_ps(), _CMP_LT_OQ);
            // Keep the compared input's raw bits when the predicate is false.
            // Floating select was lowered to MAXPS by icpx; with DAZ that
            // flushed retained subnormals, unlike the original masked store.
            const __m512i raw = _mm512_castps_si512(v);
            const __m512i kept = _mm512_andnot_si512(_mm512_movm_epi32(neg), raw);
            _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst + k),
                _mm512_cvtepi32_epi16(_mm512_srli_epi32(kept, 16)));
            if constexpr(P) pr.logical.relu_negative_elements += __builtin_popcount(unsigned(neg));
        }
        for(; k < count; k++) {
            const float v = src[k];
            uint32_t raw;
            std::memcpy(&raw, &v, sizeof(raw));
            const uint32_t negative_mask = 0u - uint32_t(v < 0.0f);
            raw &= ~negative_mask;
            dst[k] = uint16_t(raw >> 16);
            if constexpr(P) pr.logical.relu_negative_elements += negative_mask != 0;
        }
        if constexpr(P) pr.logical.relu_elements += count;
        // Parent SCATTER clocks this fused conversion/store; no false split.
    } else {
        throw std::invalid_argument("relu_output requires BF16 output");
    }
}

template<bool P, bool B16 = false, class Reducer = ScalarReducer>
void kernel(const SourceGraphView& g, const uint16_t* hb, const uint16_t* w,
            std::conditional_t<B16, uint16_t, float>* c, const int* perm,
            const Method& method, Profile* result) {
    using namespace gcn_extra_paper;
    if(method.window != 64 || (method.scope != 0 && method.scope != 64 && method.scope != 256))
        throw std::invalid_argument("state experiment requires S64 and M64/M256/FULL");
    if constexpr(!B16) if(method.relu_output)
        throw std::invalid_argument("relu_output requires BF16 output");
    if constexpr(P) {
        if(result == nullptr)
            throw std::invalid_argument("profile result is required for kernel<true>");
    }

    constexpr int R = 64;
    std::vector<Profile> profiles(omp_get_max_threads());
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
                    Reducer::template run<P>(g, hb, bases, degrees, batch, start, end,
                                    uint32_t(method.window), partial, hi, lo,
                                    method, pr, sample);
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
                    scatter_state<P, B16>(c + size_t(rows[n]) * DIM,
                                            ctile + n * DIM, DIM, method, pr, sample);
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
            add_logical(result->logical, pr.logical);
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

} // namespace gcn_extra_state
