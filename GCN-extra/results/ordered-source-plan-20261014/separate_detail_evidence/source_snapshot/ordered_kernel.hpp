#pragma once
#include "ordered_plan.hpp"
namespace gcn_extra_ordered {
template<bool P, bool B16 = false, class Reducer = gcn_extra_ordered::Reducer<3>>
void kernel(const SourceGraphView& g, const uint16_t* hb, const uint16_t* w,
            std::conditional_t<B16, uint16_t, float>* c, const int* perm,
            const Method& method, const Plan& plan, Profile* result) {
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
        Reducer::bind(plan);
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

                Reducer::set_tile(i/TR);
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
            gcn_extra_pair_plan::add_plan_logical(result->plan_logical,pr.plan_logical);
            add_event(result->event,pr.event);
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


}
