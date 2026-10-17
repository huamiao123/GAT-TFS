#pragma once
#include "state_kernels.hpp"
#include "pair_reducer.hpp"
#include "pair_plan.hpp"

// Inference-only plan execution. Every order[] entry names the ORIGINAL AMX
// row slot; neither feature rows nor projection output slots are permuted.
// All counters describe source-level logical requests, never retired work.
namespace gcn_extra_pair_plan {
using gcn_extra_paper::SourceGraphView;
using gcn_extra_paper::DIM;
using gcn_extra_state::Method;
struct PlanLogical {
    uint64_t order_index_reads = 0;
    uint64_t exact_mask_bytes_read = 0;
    uint64_t exact_pair_flags_tested = 0;
    uint64_t exact_pair_windows = 0;
    uint64_t exact_pair_edges = 0;
    uint64_t exact_shared_edges = 0;
    uint64_t runtime_head_comparisons = 0;
};
struct Profile : gcn_extra_state::Profile { PlanLogical plan_logical; };
inline void add_plan_logical(PlanLogical& a, const PlanLogical& b) {
#define PLAN_ADD(X) a.X += b.X
    PLAN_ADD(order_index_reads); PLAN_ADD(exact_mask_bytes_read);
    PLAN_ADD(exact_pair_flags_tested); PLAN_ADD(exact_pair_windows);
    PLAN_ADD(exact_pair_edges); PLAN_ADD(exact_shared_edges);
    PLAN_ADD(runtime_head_comparisons);
#undef PLAN_ADD
}

template<bool P>
inline void count_row_window(uint32_t ws, uint32_t end, bool visit,
                             Profile& pr) {
    if constexpr(P) {
        pr.counters.row_window_visits += uint64_t(visit);
        if(ws < end) {
            const uint64_t edges = end - ws;
            ++pr.counters.nonempty_row_windows;
            pr.counters.source_visits += edges;
            pr.counters.feature_adds += edges * DIM;
        }
    }
}

// Used unchanged by MODE0 and by the unflagged fallback in MODE3/MODE4.
// A whole row window completes before proceeding to the second row.
template<bool P>
inline void scalar_row_window(const SourceGraphView& g, const uint16_t* hb,
        const uint32_t* bases, const uint32_t* degrees, int n,
        uint32_t ws, uint32_t we, uint32_t scope_end, bool first,
        float* partial, uint16_t* hi, uint16_t* lo,
        const Method& method, Profile& pr, bool sample) {
    using namespace gcn_extra_state;
    const uint32_t row_scope_end = std::min(degrees[n], scope_end);
    const uint32_t row_end = std::min(row_scope_end, we);
    const bool visit = first || ws < row_end;
    if(!visit) return;
    count_row_window<P>(ws, row_end, visit, pr);
    __m512 acc[8];
    load_row<P>(partial, n, first, acc, pr, sample);
    for(uint32_t k = ws; k < row_end; ++k) {
        const uint32_t j = pair_read_head<P>(g, bases[n], k, pr, sample);
        pair_prefetch_next<P>(g, hb, bases[n], k, row_scope_end, pr, sample);
        pair_add_source<P, false>(hb + size_t(j) * DIM,
                                  acc, nullptr, pr, sample);
    }
    finish_row<P>(acc, n, row_end == row_scope_end,
                  partial, hi, lo, method, pr, sample);
}

// Same current-head merge as the frozen run05 pair reducer. Only n0/n1
// come from immutable plan metadata instead of adjacent row numbers.
template<bool P, bool SHARE>
inline void dynamic_pair_window(const SourceGraphView& g, const uint16_t* hb,
        const uint32_t* bases, const uint32_t* degrees, int n0, int n1,
        uint32_t ws, uint32_t we, uint32_t scope_end, bool first,
        float* partial, uint16_t* hi, uint16_t* lo,
        const Method& method, Profile& pr, bool sample) {
    using namespace gcn_extra_state;
    const bool second = n1 >= 0;
    const uint32_t scope0 = std::min(degrees[n0], scope_end);
    const uint32_t scope1 = second ? std::min(degrees[n1], scope_end) : 0;
    const uint32_t end0 = std::min(scope0, we);
    const uint32_t end1 = second ? std::min(scope1, we) : 0;
    const bool visit0 = first || ws < end0;
    const bool visit1 = second && (first || ws < end1);
    if(!visit0 && !visit1) return;
    __m512 acc0[8], acc1[8];
    if(visit0) load_row<P>(partial, n0, first, acc0, pr, sample);
    if(visit1) load_row<P>(partial, n1, first, acc1, pr, sample);
    count_row_window<P>(ws, end0, visit0, pr);
    if(second) count_row_window<P>(ws, end1, visit1, pr);
    uint32_t k0 = ws, k1 = ws;
    uint32_t head0 = k0 < end0 ? pair_read_head<P>(g, bases[n0], k0, pr, sample) : 0;
    uint32_t head1 = second && k1 < end1 ?
        pair_read_head<P>(g, bases[n1], k1, pr, sample) : 0;
    while(k0 < end0 || (second && k1 < end1)) {
        const bool have0 = k0 < end0;
        const bool have1 = second && k1 < end1;
        const unsigned take = pair_take(have0, have1, head0, head1);
        if constexpr(P) {
            if(have0 && have1) {
                ++pr.logical.pair_comparisons;
                ++pr.plan_logical.runtime_head_comparisons;
            }
            if(take == 3u) ++pr.logical.pair_shared_sources;
        }
        if(take & 1u) pair_prefetch_next<P>(g, hb, bases[n0], k0, scope0, pr, sample);
        if(take & 2u) pair_prefetch_next<P>(g, hb, bases[n1], k1, scope1, pr, sample);
        if constexpr(SHARE) {
            if(take == 3u)
                pair_add_source<P, true>(hb + size_t(head0) * DIM,
                                          acc0, acc1, pr, sample);
            else if(take == 1u)
                pair_add_source<P, false>(hb + size_t(head0) * DIM,
                                           acc0, nullptr, pr, sample);
            else
                pair_add_source<P, false>(hb + size_t(head1) * DIM,
                                           acc1, nullptr, pr, sample);
        } else {
            if(take & 1u)
                pair_add_source<P, false>(hb + size_t(head0) * DIM,
                                           acc0, nullptr, pr, sample);
            if(take & 2u)
                pair_add_source<P, false>(hb + size_t(head1) * DIM,
                                           acc1, nullptr, pr, sample);
        }
        if(take & 1u) {
            ++k0;
            if(k0 < end0) head0 = pair_read_head<P>(g, bases[n0], k0, pr, sample);
        }
        if(take & 2u) {
            ++k1;
            if(k1 < end1) head1 = pair_read_head<P>(g, bases[n1], k1, pr, sample);
        }
    }
    if(visit0) finish_row<P>(acc0, n0, end0 == scope0,
                            partial, hi, lo, method, pr, sample);
    if(visit1) finish_row<P>(acc1, n1, end1 == scope1,
                            partial, hi, lo, method, pr, sample);
}

// Flag provenance is certified outside timing by positive equal lengths and
// exact CSR sequence comparison. No runtime per-edge matching is performed.
// S omits the second CURRENT index request; both rows' original next-index
// request and all four prefetches per eligible logical edge remain present.
template<bool P, bool SHARE>
inline void exact_pair_window(const SourceGraphView& g, const uint16_t* hb,
        const uint32_t* bases, const uint32_t* degrees, int n0, int n1,
        uint32_t ws, uint32_t we, uint32_t scope_end, bool first,
        float* partial, uint16_t* hi, uint16_t* lo,
        const Method& method, Profile& pr, bool sample) {
    using namespace gcn_extra_state;
    const uint32_t scope0 = std::min(degrees[n0], scope_end);
    const uint32_t scope1 = std::min(degrees[n1], scope_end);
    const uint32_t end0 = std::min(scope0, we);
    const uint32_t end1 = std::min(scope1, we);
    __m512 acc0[8], acc1[8];
    load_row<P>(partial, n0, first, acc0, pr, sample);
    load_row<P>(partial, n1, first, acc1, pr, sample);
    count_row_window<P>(ws, end0, true, pr);
    count_row_window<P>(ws, end1, true, pr);
    if constexpr(P) {
        ++pr.plan_logical.exact_pair_windows;
        pr.plan_logical.exact_pair_edges += end0 - ws;
        pr.logical.pair_shared_sources += end0 - ws;
        if constexpr(SHARE) pr.plan_logical.exact_shared_edges += end0 - ws;
    }
    for(uint32_t k = ws; k < end0; ++k) {
        const uint32_t j0 = pair_read_head<P>(g, bases[n0], k, pr, sample);
        pair_prefetch_next<P>(g, hb, bases[n0], k, scope0, pr, sample);
        pair_prefetch_next<P>(g, hb, bases[n1], k, scope1, pr, sample);
        if constexpr(SHARE) {
            pair_add_source<P, true>(hb + size_t(j0) * DIM,
                                      acc0, acc1, pr, sample);
        } else {
            // Separate CSR reads and separately expressed decodes. Ordinary
            // intrinsics permit natural compiler common-subexpression reuse.
            const uint32_t j1 = pair_read_head<P>(g, bases[n1], k, pr, sample);
            pair_add_source<P, false>(hb + size_t(j0) * DIM,
                                       acc0, nullptr, pr, sample);
            pair_add_source<P, false>(hb + size_t(j1) * DIM,
                                       acc1, nullptr, pr, sample);
        }
    }
    finish_row<P>(acc0, n0, end0 == scope0, partial, hi, lo, method, pr, sample);
    finish_row<P>(acc1, n1, end1 == scope1, partial, hi, lo, method, pr, sample);
}

template<int MODE>
struct PlannedReducer {
    static_assert(MODE >= 0 && MODE <= 4, "unknown planned reducer mode");
    template<bool P>
    static inline void run(const SourceGraphView& g, const uint16_t* hb,
            const uint32_t* bases, const uint32_t* degrees, int batch,
            uint32_t scope_start, uint32_t scope_end, uint32_t window,
            float* partial, uint16_t* hi, uint16_t* lo,
            const Method& method, const TilePlan& tp, const Plan& plan,
            Profile& pr, bool sample) {
        using namespace gcn_extra_state;
        init_scope<P>(partial, hi, lo, batch, method, pr, sample);
        for(uint32_t ws = scope_start; ws < scope_end;) {
            const uint32_t we = ws + std::min(window, scope_end - ws);
            const bool first = ws == scope_start;
            if constexpr(P) ++pr.counters.neighbor_windows;
            uint8_t mask = 0;
            if constexpr(MODE >= 3) {
                mask = plan.exact_masks[tp.mask_offset + ws / 64];
                if constexpr(P) ++pr.plan_logical.exact_mask_bytes_read;
            }
            for(int p = 0; p < (batch + 1) / 2; ++p) {
                const int n0 = tp.order[2 * p];
                const int n1 = 2 * p + 1 < batch ? tp.order[2 * p + 1] : -1;
                if constexpr(P)
                    pr.plan_logical.order_index_reads += 1 + uint64_t(n1 >= 0);
                if constexpr(MODE == 0) {
                    scalar_row_window<P>(g,hb,bases,degrees,n0,ws,we,scope_end,
                        first,partial,hi,lo,method,pr,sample);
                    if(n1 >= 0)
                        scalar_row_window<P>(g,hb,bases,degrees,n1,ws,we,scope_end,
                            first,partial,hi,lo,method,pr,sample);
                } else if constexpr(MODE <= 2) {
                    dynamic_pair_window<P, MODE == 2>(g,hb,bases,degrees,n0,n1,
                        ws,we,scope_end,first,partial,hi,lo,method,pr,sample);
                } else {
                    const bool flagged = n1 >= 0 && (mask & (uint8_t(1) << p));
                    if constexpr(P)
                        pr.plan_logical.exact_pair_flags_tested += uint64_t(n1 >= 0);
                    if(flagged) {
                        exact_pair_window<P, MODE == 4>(g,hb,bases,degrees,n0,n1,
                            ws,we,scope_end,first,partial,hi,lo,method,pr,sample);
                    } else {
                        scalar_row_window<P>(g,hb,bases,degrees,n0,ws,we,scope_end,
                            first,partial,hi,lo,method,pr,sample);
                        if(n1 >= 0)
                            scalar_row_window<P>(g,hb,bases,degrees,n1,ws,we,scope_end,
                                first,partial,hi,lo,method,pr,sample);
                    }
                }
            }
            ws = we;
        }
        finish_scope<P>(partial, hi, lo, method, pr, sample);
    }
};
} // namespace gcn_extra_pair_plan
