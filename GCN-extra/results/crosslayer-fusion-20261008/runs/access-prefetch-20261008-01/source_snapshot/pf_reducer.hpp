#pragma once
#include "planned_reducer.hpp"

// Inference-only plan execution. Every order[] entry names the ORIGINAL AMX
// row slot; neither feature rows nor projection output slots are permuted.
// All counters describe source-level logical requests, never retired work.
namespace gcn_extra_prefetch {
using gcn_extra_paper::SourceGraphView;
using gcn_extra_paper::DIM;
using gcn_extra_state::Method;
using gcn_extra_pair_plan::Plan;
using gcn_extra_pair_plan::TilePlan;
using gcn_extra_pair_plan::Profile;
using gcn_extra_pair_plan::add_plan_logical;

// PF=1 calls the frozen helper verbatim. PF=8 issues the same ordered list
// of CSR-index targets per row/scope, with a seven-target prologue followed
// by lookahead eight. Every target issues four T0 hints; no deduplication.
template<bool P>
inline void pf_issue_target(const SourceGraphView& g, const uint16_t* hb,
        uint32_t base, uint32_t target, Profile& pr, bool sample) {
    const double t = gcn_extra_paper::tick<P>(sample);
    const uint32_t next_j = g.col[size_t(base) + target];
    gcn_extra_projection::detail_tock<P>(
        pr, gcn_extra_projection::CSR_NEXT_INDEX_READ, t, sample);
    const char* next = reinterpret_cast<const char*>(hb + size_t(next_j) * DIM);
    const double d = gcn_extra_paper::tick<P>(sample);
    for(int o = 0; o < 256; o += 64)
        _mm_prefetch(next + o, _MM_HINT_T0);
    gcn_extra_projection::detail_tock<P>(
        pr, gcn_extra_projection::PREFETCH_ISSUE, d, sample);
    gcn_extra_paper::tock<P>(pr, gcn_extra_paper::CSR_PREFETCH, t, sample);
    if constexpr(P) {
        ++pr.logical.prefetch_index_reads;
        pr.counters.csr_prefetch_instructions += 4;
    }
}

// The PF8 production emitter and the untimed target audit share these
// visitors. PF1 production deliberately retains its frozen helper; its
// audit visitor mirrors that helper's original k+1 bound.
template<int PF, class Emit>
inline void pf_visit_prologue_targets(uint32_t ws, uint32_t row_scope_end,
                                     bool first, Emit emit) {
    static_assert(PF == 0 || PF == 1 || PF == 8, "unknown prefetch policy");
    if constexpr(PF == 8) {
        if(first && ws < row_scope_end) {
            const uint32_t count = std::min(7u, row_scope_end - ws - 1);
            for(uint32_t q = 1; q <= count; ++q) emit(ws + q);
        }
    }
}

template<int PF, class Emit>
inline void pf_visit_steady_targets(uint32_t k, uint32_t row_scope_end,
                                   Emit emit) {
    static_assert(PF == 0 || PF == 1 || PF == 8, "unknown prefetch policy");
    if constexpr(PF == 1) {
        if(k + 1 < row_scope_end) emit(k + 1);
    } else if constexpr(PF == 8) {
        if(row_scope_end - k > 8) emit(k + 8);
    }
}

template<bool P, int PF>
inline void pf_prologue(const SourceGraphView& g, const uint16_t* hb,
        uint32_t base, uint32_t ws, uint32_t row_scope_end, bool first,
        Profile& pr, bool sample) {
    static_assert(PF == 0 || PF == 1 || PF == 8, "unknown prefetch policy");
    // first is ws == scope_start, never merely a window boundary.
    pf_visit_prologue_targets<PF>(ws, row_scope_end, first,
        [&](uint32_t target) { pf_issue_target<P>(g, hb, base, target, pr, sample); });
}

template<bool P, int PF>
inline void pf_steady(const SourceGraphView& g, const uint16_t* hb,
        uint32_t base, uint32_t k, uint32_t row_scope_end,
        Profile& pr, bool sample) {
    static_assert(PF == 0 || PF == 1 || PF == 8, "unknown prefetch policy");
    if constexpr(PF == 1) {
        gcn_extra_state::pair_prefetch_next<P>(
            g, hb, base, k, row_scope_end, pr, sample);
    } else if constexpr(PF == 8) {
        pf_visit_steady_targets<PF>(k, row_scope_end,
            [&](uint32_t target) { pf_issue_target<P>(g, hb, base, target, pr, sample); });
    }
}

template<bool P>
inline void pf_count_row_window(uint32_t ws, uint32_t end, bool visit,
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

// Arithmetic is shared by MODE0 and the unflagged fallback in MODE4.
// A whole row window completes before proceeding to the second row.
template<bool P, int PF>
inline void pf_scalar_row_window(const SourceGraphView& g, const uint16_t* hb,
        const uint32_t* bases, const uint32_t* degrees, int n,
        uint32_t ws, uint32_t we, uint32_t scope_end, bool first,
        float* partial, uint16_t* hi, uint16_t* lo,
        const Method& method, Profile& pr, bool sample) {
    using namespace gcn_extra_state;
    const uint32_t row_scope_end = std::min(degrees[n], scope_end);
    const uint32_t row_end = std::min(row_scope_end, we);
    const bool visit = first || ws < row_end;
    if(!visit) return;
    pf_count_row_window<P>(ws, row_end, visit, pr);
    __m512 acc[8];
    load_row<P>(partial, n, first, acc, pr, sample);
    pf_prologue<P, PF>(g, hb, bases[n], ws, row_scope_end, first, pr, sample);
    for(uint32_t k = ws; k < row_end; ++k) {
        const uint32_t j = pair_read_head<P>(g, bases[n], k, pr, sample);
        pf_steady<P, PF>(g, hb, bases[n], k, row_scope_end, pr, sample);
        pair_add_source<P, false>(hb + size_t(j) * DIM,
                                  acc, nullptr, pr, sample);
    }
    finish_row<P>(acc, n, row_end == row_scope_end,
                  partial, hi, lo, method, pr, sample);
}

// Flag provenance is certified outside timing by positive equal lengths and
// exact CSR sequence comparison. No runtime per-edge matching is performed.
// S omits the second CURRENT index request; both rows' original next-index
// request remains for both rows under PF1/PF8; PF0 omits software prefetch.
template<bool P, bool SHARE, int PF>
inline void pf_exact_pair_window(const SourceGraphView& g, const uint16_t* hb,
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
    pf_count_row_window<P>(ws, end0, true, pr);
    pf_count_row_window<P>(ws, end1, true, pr);
    if constexpr(P) {
        ++pr.plan_logical.exact_pair_windows;
        pr.plan_logical.exact_pair_edges += end0 - ws;
        pr.logical.pair_shared_sources += end0 - ws;
        if constexpr(SHARE) pr.plan_logical.exact_shared_edges += end0 - ws;
    }
    pf_prologue<P, PF>(g, hb, bases[n0], ws, scope0, first, pr, sample);
    pf_prologue<P, PF>(g, hb, bases[n1], ws, scope1, first, pr, sample);
    for(uint32_t k = ws; k < end0; ++k) {
        const uint32_t j0 = pair_read_head<P>(g, bases[n0], k, pr, sample);
        pf_steady<P, PF>(g, hb, bases[n0], k, scope0, pr, sample);
        pf_steady<P, PF>(g, hb, bases[n1], k, scope1, pr, sample);
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

template<int MODE, int PF>
struct PlannedReducer {
    static_assert(MODE == 0 || MODE == 4, "only Scalar and ExactS are supported");
    static_assert(PF == 0 || PF == 1 || PF == 8, "unknown prefetch policy");
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
                    pf_scalar_row_window<P, PF>(g,hb,bases,degrees,n0,ws,we,scope_end,
                        first,partial,hi,lo,method,pr,sample);
                    if(n1 >= 0)
                        pf_scalar_row_window<P, PF>(g,hb,bases,degrees,n1,ws,we,scope_end,
                            first,partial,hi,lo,method,pr,sample);
                } else {
                    const bool flagged = n1 >= 0 && (mask & (uint8_t(1) << p));
                    if constexpr(P)
                        pr.plan_logical.exact_pair_flags_tested += uint64_t(n1 >= 0);
                    if(flagged) {
                        pf_exact_pair_window<P, MODE == 4, PF>(g,hb,bases,degrees,n0,n1,
                            ws,we,scope_end,first,partial,hi,lo,method,pr,sample);
                    } else {
                        pf_scalar_row_window<P, PF>(g,hb,bases,degrees,n0,ws,we,scope_end,
                            first,partial,hi,lo,method,pr,sample);
                        if(n1 >= 0)
                            pf_scalar_row_window<P, PF>(g,hb,bases,degrees,n1,ws,we,scope_end,
                                first,partial,hi,lo,method,pr,sample);
                    }
                }
            }
            ws = we;
        }
        finish_scope<P>(partial, hi, lo, method, pr, sample);
    }
};
} // namespace gcn_extra_prefetch
