// Independent FP32 grouped-head aggregate-first Online GAT candidate.
// A source H vector is loaded once for a head group; attention and online
// state remain independent. Rescale shares the U load/store with weighted FMA.
#include "joint_online.hpp"
#include <array>
#include <iomanip>
#include <iostream>

namespace gat::joint {

void WorkspaceJoint::allocate(const Graph& g, const Param& p, bool counters) {
    if (p.in < 1 || p.heads < 1 || p.heads > 8 || p.dim < 1 ||
        p.in > std::numeric_limits<int>::max() - 31)
        throw std::runtime_error("invalid joint workspace shape");
    workers = omp_get_max_threads();
    padded_in = (p.in + 31) / 32 * 32;
    lr.resize(checked(g.n, 2 * p.heads));
    out.resize(checked(g.n, p.width()));
    u.resize(checked(workers, checked(16 * p.heads, padded_in)));
    c.resize(checked(workers, checked(16, p.dim)));
    if (counters) {
        const size_t n = checked(g.n, p.heads);
        blocks.resize(n);
        updates.resize(n);
        rescales.resize(n);
    } else {
        blocks.clear(); updates.clear(); rescales.clear();
    }
}

static uint64_t mix(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

template<bool Measure>
static inline Clock::time_point now() {
    if constexpr (Measure) return Clock::now();
    else return {};
}

template<int G, bool Measure, bool Counters>
static void tile(const Graph& g, const Param& p, const local::PreparedLocal& q,
                 const Schedule& sched, const std::vector<float>& x,
                 WorkspaceJoint& w, uint64_t base, int block,
                 float* u, float* c, Stats& stats) {
    static_assert(G == 1 || G == 2 || G == 4 || G == 8);
    const int rows = int(std::min<uint64_t>(16, g.n - base));
    const int dp = q.base.padded_in;
    const size_t row_stride = size_t(p.heads) * dp;
    alignas(64) float den[16][8] = {};
    uint32_t rowids[16];
    auto begin_time = now<Measure>();
    std::fill(u, u + 16 * row_stride, 0.f);
    if constexpr (Measure) {
        stats.init += seconds(begin_time, now<Measure>());
        ++stats.sampled_tiles;
    }

    for (int ri = 0; ri < rows; ++ri) {
        const uint32_t row = sched.perm[base + ri];
        rowids[ri] = row;
        for (int hb = 0; hb < p.heads; hb += G) {
            const int active = std::min(G, p.heads - hb);
            std::array<float, G> m, l{}, left{}, scale{}, sum{}, next{};
            std::array<bool, G> changed{}, rescale{};
            std::array<uint32_t, G> nb{}, nu{}, nr{};
            m.fill(-std::numeric_limits<float>::infinity());
            #pragma unroll
            for (int h = 0; h < G; ++h)
                if (h < active) left[h] = w.lr[size_t(row) * 2 * p.heads + hb + h];

            for (uint64_t eb = g.row[row]; eb < g.row[row + 1]; eb += block) {
                begin_time = now<Measure>();
                const int count = int(std::min<uint64_t>(block, g.row[row + 1] - eb));
                alignas(64) float scores[G][64], weights[G][64];
                uint32_t sources[64];
                for (int e = 0; e < count; ++e) sources[e] = g.col[eb + e];

                #pragma unroll
                for (int h = 0; h < G; ++h) {
                    if (h >= active) continue;
                    float bm = -std::numeric_limits<float>::infinity();
                    for (int e = 0; e < count; ++e)
                        scores[h][e] = w.lr[size_t(sources[e]) * 2 * p.heads + p.heads + hb + h];
                    for (int e = 0; e < count; e += 16) {
                        const auto mask = tail_mask(std::min(16, count - e));
                        const __m512 v = leaky(_mm512_add_ps(_mm512_set1_ps(left[h]),
                            _mm512_maskz_loadu_ps(mask, scores[h] + e)));
                        _mm512_mask_storeu_ps(scores[h] + e, mask, v);
                        bm = std::max(bm, _mm512_mask_reduce_max_ps(mask, v));
                    }
                    next[h] = std::max(m[h], bm);
                    changed[h] = next[h] > m[h];
                    rescale[h] = l[h] > 0 && changed[h];
                    scale[h] = rescale[h] ? std::exp(m[h] - next[h]) : (l[h] > 0 ? 1.f : 0.f);
                    sum[h] = 0;
                    for (int e = 0; e < count; e += 16) {
                        const auto mask = tail_mask(std::min(16, count - e));
                        const __m512 z = _mm512_mask_loadu_ps(_mm512_set1_ps(next[h]), mask, scores[h] + e);
                        const __m512 v = _mm512_exp_ps(_mm512_sub_ps(z, _mm512_set1_ps(next[h])));
                        _mm512_mask_storeu_ps(weights[h] + e, mask, v);
                        sum[h] += _mm512_mask_reduce_add_ps(mask, v);
                    }
                }
                if constexpr (Measure) stats.score += seconds(begin_time, now<Measure>());
                begin_time = now<Measure>();

                for (int d = 0; d < p.in; d += 16) {
                    const auto mask = tail_mask(std::min(16, p.in - d));
                    // G is a compile-time constant: at most eight feature vectors
                    // live while one shared H vector feeds independent head FMAs.
                    std::array<__m512, G> values;
                    #pragma unroll
                    for (int h = 0; h < G; ++h) {
                        values[h] = _mm512_setzero_ps();
                        if (h < active) {
                            float* acc = u + size_t(ri) * row_stride + size_t(hb + h) * dp + d;
                            values[h] = _mm512_maskz_loadu_ps(mask, acc);
                            if (rescale[h]) values[h] = _mm512_mul_ps(values[h], _mm512_set1_ps(scale[h]));
                        }
                    }
                    for (int e = 0; e < count; ++e) {
                        const __m512 source = _mm512_maskz_loadu_ps(mask,
                            x.data() + size_t(sources[e]) * p.in + d);
                        #pragma unroll
                        for (int h = 0; h < G; ++h)
                            if (h < active) values[h] = _mm512_fmadd_ps(_mm512_set1_ps(weights[h][e]), source, values[h]);
                    }
                    #pragma unroll
                    for (int h = 0; h < G; ++h)
                        if (h < active) _mm512_mask_storeu_ps(
                            u + size_t(ri) * row_stride + size_t(hb + h) * dp + d, mask, values[h]);
                }

                if constexpr (Measure) {
                    stats.spmm_rescale += seconds(begin_time, now<Measure>());
                    ++stats.sampled_neighbor_group_blocks;
                    stats.sampled_row_head_blocks += active;
                    const uint64_t vectors = uint64_t(count) * ((p.in + 15) / 16);
                    stats.sampled_source_vector_loads += vectors;
                    stats.sampled_fma_vectors += vectors * active;
                }
                #pragma unroll
                for (int h = 0; h < G; ++h) {
                    if (h >= active) continue;
                    if constexpr (Counters) {
                        ++nb[h]; nu[h] += changed[h]; nr[h] += rescale[h];
                    }
                    l[h] = scale[h] * l[h] + sum[h];
                    m[h] = next[h];
                }
            }

            #pragma unroll
            for (int h = 0; h < G; ++h) {
                if (h >= active) continue;
                den[ri][hb + h] = l[h];
                if constexpr (Counters) {
                    const size_t ix = size_t(row) * p.heads + hb + h;
                    w.blocks[ix] = nb[h]; w.updates[ix] = nu[h]; w.rescales[ix] = nr[h];
                }
            }
        }
    }

    for (int h = 0; h < p.heads; ++h) {
        begin_time = now<Measure>();
        // Each head consumes its own strided U immediately inside this tile.
        cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                    rows, p.dim, p.in, 1.f, u + size_t(h) * dp, int(row_stride),
                    q.wh.data() + size_t(h) * p.in * p.dim, p.dim, 0.f, c, p.dim);
        if constexpr (Measure) stats.gemm += seconds(begin_time, now<Measure>());
        begin_time = now<Measure>();
        for (int ri = 0; ri < rows; ++ri) {
            const float inv = den[ri][h] > 0 ? 1.f / den[ri][h] : 0.f;
            float* dest = w.out.data() + size_t(rowids[ri]) * p.width() + h * p.dim;
            for (int d = 0; d < p.dim; d += 16) {
                const auto mask = tail_mask(std::min(16, p.dim - d));
                _mm512_mask_storeu_ps(dest + d, mask, _mm512_mul_ps(
                    _mm512_maskz_loadu_ps(mask, c + size_t(ri) * p.dim + d), _mm512_set1_ps(inv)));
            }
        }
        if constexpr (Measure) stats.output += seconds(begin_time, now<Measure>());
    }
}

template<int G, bool Counters>
static void run(const Graph& g, const Param& p, const local::PreparedLocal& q,
                const Schedule& sched, const std::vector<float>& x,
                WorkspaceJoint& w, int block, int panel, Stats& result, uint64_t period) {
    #pragma omp parallel
    {
        Stats stats;
        const int id = omp_get_thread_num();
        const int old_threads = mkl_set_num_threads_local(1);
        float* u = w.u.data() + size_t(id) * 16 * p.heads * q.base.padded_in;
        float* c = w.c.data() + size_t(id) * 16 * p.dim;
        #pragma omp for schedule(dynamic, 1)
        for (uint64_t rg = 0; rg < g.n; rg += panel)
            for (uint64_t base = rg; base < std::min<uint64_t>(g.n, rg + panel); base += 16) {
                // Tile selection is independent of group size for comparable samples.
                const bool measure = period && mix(base / 16) % period == 0;
                if (measure) tile<G, true, Counters>(g, p, q, sched, x, w, base, block, u, c, stats);
                else tile<G, false, Counters>(g, p, q, sched, x, w, base, block, u, c, stats);
            }
        mkl_set_num_threads_local(old_threads);
        #pragma omp critical(gat_joint_stats)
        {
            result.init += stats.init; result.score += stats.score;
            result.spmm_rescale += stats.spmm_rescale;
            result.gemm += stats.gemm; result.output += stats.output;
            result.sampled_tiles += stats.sampled_tiles;
            result.sampled_neighbor_group_blocks += stats.sampled_neighbor_group_blocks;
            result.sampled_row_head_blocks += stats.sampled_row_head_blocks;
            result.sampled_source_vector_loads += stats.sampled_source_vector_loads;
            result.sampled_fma_vectors += stats.sampled_fma_vectors;
        }
    }
}

static void validate(const Graph& g, const Param& p, const local::PreparedLocal& q,
                     const Schedule& sched, const std::vector<float>& x,
                     const WorkspaceJoint& w, int group, int block, int panel, bool counters) {
    if ((group != 1 && group != 2 && group != 4 && group != 8) || block < 1 || block > 64 ||
        panel < 16 || panel % 16 || p.in < 1 || p.heads < 1 || p.heads > 8 || p.dim < 1 ||
        g.n > uint64_t(std::numeric_limits<int>::max()) || q.base.padded_in < p.in ||
        q.base.padded_in % 32 || size_t(p.heads) * q.base.padded_in > size_t(std::numeric_limits<int>::max()) ||
        x.size() != checked(g.n, p.in) || sched.perm.size() != g.n ||
        q.wh.size() != checked(p.in, p.width()) || q.base.blr.size() != checked(p.in, 2 * p.heads) ||
        w.workers < omp_get_max_threads() || w.padded_in != q.base.padded_in ||
        w.lr.size() != checked(g.n, 2 * p.heads) || w.out.size() != checked(g.n, p.width()) ||
        w.u.size() < checked(w.workers, checked(16 * p.heads, q.base.padded_in)) ||
        w.c.size() < checked(w.workers, checked(16, p.dim)))
        throw std::runtime_error("invalid joint configuration or workspace");
    if (counters && (w.blocks.size() != checked(g.n, p.heads) ||
        w.updates.size() != checked(g.n, p.heads) || w.rescales.size() != checked(g.n, p.heads)))
        throw std::runtime_error("joint counter workspace not allocated");
}

void aggregate(const Graph& g, const Param& p, const local::PreparedLocal& q,
               const Schedule& sched, const std::vector<float>& x, WorkspaceJoint& w,
               int head_group, int block, int panel, Stats& stats, uint64_t period, bool counters) {
    validate(g, p, q, sched, x, w, head_group, block, panel, counters);
    while (head_group > p.heads) head_group /= 2;
    if (counters) {
        switch (head_group) {
            case 1: run<1, true>(g, p, q, sched, x, w, block, panel, stats, period); break;
            case 2: run<2, true>(g, p, q, sched, x, w, block, panel, stats, period); break;
            case 4: run<4, true>(g, p, q, sched, x, w, block, panel, stats, period); break;
            case 8: run<8, true>(g, p, q, sched, x, w, block, panel, stats, period); break;
        }
    } else {
        switch (head_group) {
            case 1: run<1, false>(g, p, q, sched, x, w, block, panel, stats, period); break;
            case 2: run<2, false>(g, p, q, sched, x, w, block, panel, stats, period); break;
            case 4: run<4, false>(g, p, q, sched, x, w, block, panel, stats, period); break;
            case 8: run<8, false>(g, p, q, sched, x, w, block, panel, stats, period); break;
        }
    }
}

void layer(const Graph& g, const Param& p, const local::PreparedLocal& q,
           const Schedule& sched, const std::vector<float>& x, WorkspaceJoint& w,
           bool hidden, int head_group, int block, int panel, Timing& t, uint64_t period, bool counters) {
    validate(g, p, q, sched, x, w, head_group, block, panel, counters);
    const auto begin = Clock::now();
    auto a = begin;
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, int(g.n), 2 * p.heads, p.in,
                1.f, x.data(), p.in, q.base.blr.data(), 2 * p.heads, 0.f, w.lr.data(), 2 * p.heads);
    t.lr = seconds(a, Clock::now());
    a = Clock::now();
    aggregate(g, p, q, sched, x, w, head_group, block, panel, t.sample, period, counters);
    t.kernel = seconds(a, Clock::now());
    a = Clock::now();
    if (hidden) {
        #pragma omp parallel for schedule(static)
        for (size_t i = 0; i < w.out.size(); i += 16) {
            const auto mask = tail_mask(int(std::min<size_t>(16, w.out.size() - i)));
            _mm512_mask_storeu_ps(w.out.data() + i, mask, activation(_mm512_maskz_loadu_ps(mask, w.out.data() + i)));
        }
    }
    t.activation = seconds(a, Clock::now());
    t.total = seconds(begin, Clock::now());
}

void print_stats(const WorkspaceJoint& w, const Param& p, int layer) {
    if (w.blocks.size() != w.updates.size() || w.blocks.size() != w.rescales.size())
        throw std::runtime_error("invalid joint counter shape");
    uint64_t nb = 0, nu = 0, nr = 0;
    std::vector<float> ratios;
    ratios.reserve(w.blocks.size());
    for (size_t i = 0; i < w.blocks.size(); ++i) {
        nb += w.blocks[i]; nu += w.updates[i]; nr += w.rescales[i];
        ratios.push_back(w.blocks[i] ? float(w.rescales[i]) / w.blocks[i] : 0.f);
    }
    std::sort(ratios.begin(), ratios.end());
    const auto pct = [&](double f) {
        return ratios.empty() ? 0.f : ratios[std::min(ratios.size() - 1, size_t(std::ceil(ratios.size() * f) - 1))];
    };
    std::cout << std::setprecision(10) << "JOINT_ONLINE_STATS layer=" << layer
        << " blocks=" << nb << " running_max_updates_including_initial=" << nu
        << " rescales=" << nr << " rescaled_feature_elements=" << nr * uint64_t(p.in)
        << " rescale_per_block=" << (nb ? double(nr) / nb : 0)
        << " row_head_ratio_P50=" << pct(.5) << " P90=" << pct(.9)
        << " P95=" << pct(.95) << " P99=" << pct(.99)
        << " rescale_timing=fused_with_weighted_spmm\n";
}

} // namespace gat::joint
