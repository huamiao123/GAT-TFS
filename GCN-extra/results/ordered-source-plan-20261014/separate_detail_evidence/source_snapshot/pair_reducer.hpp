#pragma once

#include "state_kernels.hpp"
#include <algorithm>
#include <cstdint>

// This header changes neither graph grouping nor any per-row summation order.
// All counts below are source-level logical requests. In particular, ordinary
// intrinsics allow a compiler to CSE INTERLEAVE's repeated load/decode; only
// assembly inspection can establish the emitted instruction count.
namespace gcn_extra_state {

// A current-head merge is valid even for unsorted/duplicate CSR: each take
// advances exactly one position in that row. Sorting is not required or done.
inline unsigned pair_take(bool have0, bool have1, uint32_t head0, uint32_t head1) {
    if(!have0) return have1 ? 2u : 0u;
    if(!have1) return 1u;
    if(head0 < head1) return 1u;
    if(head1 < head0) return 2u;
    return 3u;
}

struct PairStructure {
    uint64_t edge_visits = 0;
    uint64_t shared_heads = 0;
    uint64_t merge_steps = 0;
    uint64_t frontier_comparisons = 0;
    uint64_t prefetch_index_reads = 0;
    uint64_t row_window_visits = 0;
    uint64_t nonempty_row_windows = 0;
    uint64_t neighbor_windows = 0;

    uint64_t interleave_decode_rows() const { return edge_visits; }
    uint64_t share_decode_rows() const { return edge_visits - shared_heads; }
};

// Read-only structural pass, to be called OUTSIDE all primary timed regions.
// The caller supplies the SAME TR16 bases/degrees, projection scope and S64
// window as kernel(). Add this result across scopes/tiles to obtain graph totals.
inline PairStructure count_pair_scope(const SourceGraphView& g,
        const uint32_t* bases, const uint32_t* degrees, int batch,
        uint32_t scope_start, uint32_t scope_end, uint32_t window) {
    if(window == 0 || batch < 0 || batch > TR ||
       scope_end < scope_start)
        throw std::invalid_argument("invalid pair structure bounds");
    PairStructure out;
    for(uint32_t ws = scope_start; ws < scope_end;) {
        const uint32_t we = ws + std::min(window, scope_end - ws);
        const bool first = ws == scope_start;
        ++out.neighbor_windows;
        for(int n = 0; n < batch; n += 2) {
            const bool second = n + 1 < batch;
            const uint32_t scope0 = std::min(degrees[n], scope_end);
            const uint32_t scope1 = second ? std::min(degrees[n+1], scope_end) : 0;
            const uint32_t end0 = std::min(scope0, we);
            const uint32_t end1 = second ? std::min(scope1, we) : 0;
            if(first || ws < end0) ++out.row_window_visits;
            if(second && (first || ws < end1)) ++out.row_window_visits;
            if(ws < end0) {
                const uint64_t edges = end0 - ws;
                out.edge_visits += edges;
                ++out.nonempty_row_windows;
                out.prefetch_index_reads += edges - uint64_t(end0 == scope0);
            }
            if(second && ws < end1) {
                const uint64_t edges = end1 - ws;
                out.edge_visits += edges;
                ++out.nonempty_row_windows;
                out.prefetch_index_reads += edges - uint64_t(end1 == scope1);
            }
            uint32_t k0 = ws, k1 = ws;
            uint32_t head0 = k0 < end0 ? g.col[size_t(bases[n]) + k0] : 0;
            uint32_t head1 = second && k1 < end1 ?
                g.col[size_t(bases[n+1]) + k1] : 0;
            while(k0 < end0 || (second && k1 < end1)) {
                const bool have0 = k0 < end0;
                const bool have1 = second && k1 < end1;
                const unsigned take = pair_take(have0, have1, head0, head1);
                ++out.merge_steps;
                if(have0 && have1) ++out.frontier_comparisons;
                if(take == 3u) ++out.shared_heads;
                if(take & 1u) {
                    ++k0;
                    if(k0 < end0) head0 = g.col[size_t(bases[n]) + k0];
                }
                if(take & 2u) {
                    ++k1;
                    if(k1 < end1) head1 = g.col[size_t(bases[n+1]) + k1];
                }
            }
        }
        ws = we;
    }
    return out;
}

template<bool P>
inline uint32_t pair_read_head(const SourceGraphView& g, uint32_t base,
        uint32_t k, Profile& pr, bool sample) {
    const double t = gcn_extra_paper::tick<P>(sample);
    const uint32_t head = g.col[size_t(base) + k];
    gcn_extra_projection::detail_tock<P>(
        pr, gcn_extra_projection::CSR_INDEX_READ, t, sample);
    if constexpr(P) ++pr.logical.csr_index_reads;
    return head;
}

template<bool P>
inline void pair_prefetch_next(const SourceGraphView& g, const uint16_t* hb,
        uint32_t base, uint32_t k, uint32_t row_scope_end,
        Profile& pr, bool sample) {
    // Keep the original next_j request and four cache-line prefetches for
    // EACH consumed edge, including across S64 window boundaries. A shared
    // current head never removes either row's prefetch.
    if(k + 1 < row_scope_end) {
        const double t = gcn_extra_paper::tick<P>(sample);
        const uint32_t next_j = g.col[size_t(base) + k + 1];
        gcn_extra_projection::detail_tock<P>(
            pr, gcn_extra_projection::CSR_NEXT_INDEX_READ, t, sample);
        const char* next = reinterpret_cast<const char*>(
            hb + size_t(next_j) * gcn_extra_paper::DIM);
        const double d = gcn_extra_paper::tick<P>(sample);
        for(int o = 0; o < 256; o += 64)
            _mm_prefetch(next + o, _MM_HINT_T0);
        gcn_extra_projection::detail_tock<P>(
            pr, gcn_extra_projection::PREFETCH_ISSUE, d, sample);
        gcn_extra_paper::tock<P>(
            pr, gcn_extra_paper::CSR_PREFETCH, t, sample);
        if constexpr(P) {
            ++pr.logical.prefetch_index_reads;
            pr.counters.csr_prefetch_instructions += 4;
        }
    }
}

inline __m512 pair_decode16(const uint16_t* src) {
    return _mm512_castsi512_ps(_mm512_slli_epi32(
        _mm512_cvtepu16_epi32(_mm256_loadu_si256(
            reinterpret_cast<const __m256i*>(src))), 16));
}

// Ordinary intrinsics in BOTH modes: no volatile loads, inline assembly,
// optimizer barrier or artificial restriction on compiler common subexpressions.
// Timed source uses sixteen accumulator vectors and one decoded temporary;
// actual register allocation/spills must be checked in the generated assembly.
// Instrumented sampled code deliberately has different lifetime/timing overhead;
// its output must pass the independent profile-bitwise gate.
template<bool P, bool BOTH>
inline void pair_add_source(const uint16_t* src, __m512* acc0, __m512* acc1,
                           Profile& pr, bool sample) {
    using namespace gcn_extra_paper;
    if constexpr(P) {
        ++pr.logical.decoded_source_rows;
        pr.counters.source_decode_bytes += DIM * sizeof(uint16_t);
        if(sample) {
            __m512 val[8];
            double t = tick<P>(sample);
            __m256i raw[8];
            double d = tick<P>(sample);
            for(int f=0;f<8;f++)raw[f]=_mm256_loadu_si256(reinterpret_cast<const __m256i*>(src+f*16));
            gcn_extra_projection::detail_tock<P>(pr,gcn_extra_projection::SOURCE_BF16_LOAD,d,sample);
            d=tick<P>(sample);
            for(int f=0;f<8;f++)val[f]=_mm512_castsi512_ps(_mm512_slli_epi32(_mm512_cvtepu16_epi32(raw[f]),16));
            gcn_extra_projection::detail_tock<P>(pr,gcn_extra_projection::SOURCE_BF16_DECODE,d,sample);
            // Diagnostic sampled branch only: split load/decode, original add below.
            tock<P>(pr, GATHER_DECODE, t, sample);
            t = tick<P>(sample);
            for(int f = 0; f < 8; ++f) {
                acc0[f] = _mm512_add_ps(acc0[f], val[f]);
                if constexpr(BOTH) acc1[f] = _mm512_add_ps(acc1[f], val[f]);
            }
            gcn_extra_projection::detail_tock<P>(
                pr, gcn_extra_projection::FEATURE_FP32_ADD, t, sample);
            tock<P>(pr, REDUCE_ADD, t, sample);
            return;
        }
    }
    for(int f = 0; f < 8; ++f) {
        const __m512 val = pair_decode16(src + f * 16);
        acc0[f] = _mm512_add_ps(acc0[f], val);
        if constexpr(BOTH) acc1[f] = _mm512_add_ps(acc1[f], val);
    }
}

template<bool SHARE>
struct PairReducer {
    template<bool P>
    static inline void run(const SourceGraphView& g, const uint16_t* hb,
            const uint32_t* bases, const uint32_t* degrees, int batch,
            uint32_t scope_start, uint32_t scope_end, uint32_t window,
            float* partial, uint16_t* hi, uint16_t* lo,
            const Method& method, Profile& pr, bool sample) {
        using namespace gcn_extra_paper;
        // Shared helpers own initialization, NOZERO, direct packing, tails,
        // high-word truncation/residual semantics and their logical counters.
        init_scope<P>(partial, hi, lo, batch, method, pr, sample);
        for(uint32_t ws = scope_start; ws < scope_end;) {
            const uint32_t we = ws + std::min(window, scope_end - ws);
            const bool first = ws == scope_start;
            if constexpr(P) ++pr.counters.neighbor_windows;
            for(int n = 0; n < batch; n += 2) {
                const bool second = n + 1 < batch;
                const uint32_t scope0 = std::min(degrees[n], scope_end);
                const uint32_t scope1 = second ?
                    std::min(degrees[n+1], scope_end) : 0;
                const uint32_t end0 = std::min(scope0, we);
                const uint32_t end1 = second ? std::min(scope1, we) : 0;
                const bool visit0 = first || ws < end0;
                const bool visit1 = second && (first || ws < end1);
                if(!visit0 && !visit1) continue;

                __m512 acc0[8], acc1[8];
                if(visit0) load_row<P>(partial, n, first, acc0, pr, sample);
                if(visit1) load_row<P>(partial, n+1, first, acc1, pr, sample);
                if constexpr(P) {
                    pr.counters.row_window_visits += uint64_t(visit0) + uint64_t(visit1);
                    if(ws < end0) {
                        const uint64_t edges = end0 - ws;
                        ++pr.counters.nonempty_row_windows;
                        pr.counters.source_visits += edges;
                        pr.counters.feature_adds += edges * DIM;
                    }
                    if(second && ws < end1) {
                        const uint64_t edges = end1 - ws;
                        ++pr.counters.nonempty_row_windows;
                        pr.counters.source_visits += edges;
                        pr.counters.feature_adds += edges * DIM;
                    }
                }
                uint32_t k0 = ws, k1 = ws;
                uint32_t head0 = k0 < end0 ?
                    pair_read_head<P>(g, bases[n], k0, pr, sample) : 0;
                uint32_t head1 = second && k1 < end1 ?
                    pair_read_head<P>(g, bases[n+1], k1, pr, sample) : 0;
                while(k0 < end0 || (second && k1 < end1)) {
                    const bool have0 = k0 < end0;
                    const bool have1 = second && k1 < end1;
                    const unsigned take = pair_take(have0, have1, head0, head1);
                    if constexpr(P) {
                        if(have0 && have1) ++pr.logical.pair_comparisons;
                        if(take == 3u) ++pr.logical.pair_shared_sources;
                    }
                    if(take & 1u)
                        pair_prefetch_next<P>(g, hb, bases[n], k0, scope0, pr, sample);
                    if(take & 2u)
                        pair_prefetch_next<P>(g, hb, bases[n+1], k1, scope1, pr, sample);

                    if constexpr(SHARE) {
                        if(take == 3u) {
                            pair_add_source<P, true>(
                                hb + size_t(head0) * DIM, acc0, acc1, pr, sample);
                        } else if(take == 1u) {
                            pair_add_source<P, false>(
                                hb + size_t(head0) * DIM, acc0, nullptr, pr, sample);
                        } else {
                            pair_add_source<P, false>(
                                hb + size_t(head1) * DIM, acc1, nullptr, pr, sample);
                        }
                    } else {
                        // The same traversal/prefetch decisions, independently
                        // expressed loads/decodes. Natural compiler CSE is allowed.
                        if(take & 1u)
                            pair_add_source<P, false>(
                                hb + size_t(head0) * DIM, acc0, nullptr, pr, sample);
                        if(take & 2u)
                            pair_add_source<P, false>(
                                hb + size_t(head1) * DIM, acc1, nullptr, pr, sample);
                    }
                    if(take & 1u) {
                        ++k0;
                        if(k0 < end0)
                            head0 = pair_read_head<P>(g, bases[n], k0, pr, sample);
                    }
                    if(take & 2u) {
                        ++k1;
                        if(k1 < end1)
                            head1 = pair_read_head<P>(g, bases[n+1], k1, pr, sample);
                    }
                }
                // Final is PER ROW, including a valid row already exhausted
                // before this scope. Such a first-window row must pack +0.
                if(visit0)
                    finish_row<P>(acc0, n, end0 == scope0, partial, hi, lo,
                                  method, pr, sample);
                if(visit1)
                    finish_row<P>(acc1, n+1, end1 == scope1, partial, hi, lo,
                                  method, pr, sample);
            }
            ws = we;
        }
        finish_scope<P>(partial, hi, lo, method, pr, sample);
    }
};

using PairInterleaveReducer = PairReducer<false>;
using PairShareReducer = PairReducer<true>;

} // namespace gcn_extra_state
