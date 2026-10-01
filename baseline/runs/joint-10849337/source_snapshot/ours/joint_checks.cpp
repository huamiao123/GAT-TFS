#include "joint_checks.hpp"
#include <iomanip>
#include <ostream>
#include <set>

namespace gat::checks {
namespace {
uint64_t products_per_edge(const Param& p) {
    uint64_t result = uint64_t(p.heads);
    for (int value : {p.in, p.dim}) {
        if (result > UINT64_MAX / uint64_t(value))
            throw std::runtime_error("oracle product count overflow");
        result *= uint64_t(value);
    }
    return result;
}
void check_csr_shape(const Graph& g) {
    if (!g.n || g.n > UINT32_MAX || g.row.size() != g.n + 1 ||
        g.col.size() != g.e || g.row.front() != 0 || g.row.back() != g.e)
        throw std::runtime_error("oracle invalid CSR shape");
}
void check_attention(AttentionView view, const Graph& g, const Param& p) {
    if (view.nodes != g.n || view.heads != p.heads ||
        (!view.interleaved && (!view.left || !view.right)))
        throw std::runtime_error("oracle attention shape mismatch");
}
uint64_t row_degree(const Graph& g, uint32_t row) {
    if (row >= g.n || g.row[row] > g.row[row + 1] || g.row[row + 1] > g.e)
        throw std::runtime_error("oracle invalid CSR row");
    return g.row[row + 1] - g.row[row];
}
void check_selection(const Graph& g, const Param& p, const RowSelection& selection) {
    if (selection.graph_nodes != g.n || selection.graph_edges != g.e ||
        selection.rows.empty() || selection.rows.size() > selection.limits.max_rows)
        throw std::runtime_error("oracle invalid row selection");
    const uint64_t unit = products_per_edge(p);
    uint64_t total = 0;
    std::set<uint32_t> unique;
    for (uint32_t row : selection.rows) {
        if (!unique.insert(row).second)
            throw std::runtime_error("oracle duplicate selected row");
        const uint64_t degree = row_degree(g, row);
        if (degree > selection.effective_row_edge_limit ||
            degree > selection.limits.max_row_projection_products / unit ||
            degree > (selection.limits.max_total_projection_products - total) / unit)
            throw std::runtime_error("oracle selected row exceeds work budget");
        total += degree * unit;
    }
}
float edge_score(AttentionView view, uint32_t row, uint32_t source, int head) {
    const float left = view.L(row, head), right = view.R(source, head);
    const float result = leak(left + right);
    if (!std::isfinite(left) || !std::isfinite(right) || !std::isfinite(result))
        throw std::runtime_error("oracle nonfinite FP32 attention score");
    return result;
}
double finite_value(float value) {
    if (!std::isfinite(value)) throw std::runtime_error("oracle nonfinite input/weight");
    return double(value);
}
struct Metrics {
    double max_abs = 0;
    long double absolute_sum = 0, difference_sq = 0, reference_sq = 0;
    uint64_t count = 0;
    bool finite = true;
    std::vector<double> absolute;
    bool add(double reference, double actual) {
        ++count;
        if (!std::isfinite(reference) || !std::isfinite(actual)) {
            finite = false;
            return true;
        }
        const double difference = actual - reference, magnitude = std::abs(difference);
        if (!std::isfinite(difference)) { finite = false; return true; }
        const bool new_max = magnitude > max_abs;
        max_abs = std::max(max_abs, magnitude);
        absolute_sum += magnitude;
        difference_sq += static_cast<long double>(difference) * difference;
        reference_sq += static_cast<long double>(reference) * reference;
        absolute.push_back(magnitude);
        return new_max;
    }
    Error result() const {
        Error error;
        error.finite = finite;
        if (!finite) {
            error.max_abs = error.mean_abs = error.relative_l2 = error.rmse =
                error.p50 = error.p90 = error.p99 = INFINITY;
            return error;
        }
        if (!count) return error;
        error.max_abs = max_abs;
        error.mean_abs = double(absolute_sum / count);
        error.rmse = double(std::sqrt(difference_sq / count));
        error.relative_l2 = reference_sq > 0 ? double(std::sqrt(difference_sq / reference_sq)) :
                            difference_sq == 0 ? 0 : INFINITY;
        auto sorted = absolute;
        std::sort(sorted.begin(), sorted.end());
        auto quantile = [&](double fraction) {
            return sorted[std::min(sorted.size() - 1,
                size_t(std::ceil(sorted.size() * fraction) - 1))];
        };
        error.p50 = quantile(.5); error.p90 = quantile(.9); error.p99 = quantile(.99);
        return error;
    }
};
void print_metrics(std::ostream& out, const std::string& prefix, const Error& error) {
    out << ' ' << prefix << "_max_abs=" << error.max_abs
        << ' ' << prefix << "_mean_abs=" << error.mean_abs
        << ' ' << prefix << "_relative_L2=" << error.relative_l2
        << ' ' << prefix << "_finite=" << (error.finite ? "true" : "false");
}
} // namespace

AttentionView AttentionView::from_interleaved(const std::vector<float>& values,
                                             uint64_t n, int k) {
    if (k <= 0 || values.size() != checked(n, uint64_t(k) * 2))
        throw std::runtime_error("interleaved attention shape mismatch");
    AttentionView view; view.interleaved = values.data(); view.nodes = n; view.heads = k;
    return view;
}
AttentionView AttentionView::from_workspace(const Workspace& workspace, uint64_t n, int k) {
    if (k <= 0 || workspace.left.size() != checked(n, k) ||
        workspace.right.size() != checked(n, k))
        throw std::runtime_error("split attention shape mismatch");
    AttentionView view; view.left = workspace.left.data(); view.right = workspace.right.data();
    view.nodes = n; view.heads = k; return view;
}
float AttentionView::L(uint64_t row, int head) const {
    return interleaved ? interleaved[size_t(row) * 2 * heads + head] :
                         left[size_t(row) * heads + head];
}
float AttentionView::R(uint64_t row, int head) const {
    return interleaved ? interleaved[size_t(row) * 2 * heads + heads + head] :
                         right[size_t(row) * heads + head];
}

RowSelection choose_rows(const Graph& g, const Param& p, const Limits& limits) {
    p.validate(); check_csr_shape(g);
    if (!limits.max_rows || !limits.max_total_projection_products ||
        !limits.max_row_projection_products)
        throw std::runtime_error("oracle work limits must be positive");
    RowSelection selection; selection.graph_nodes = g.n; selection.graph_edges = g.e;
    selection.limits = limits;
    const uint64_t unit = products_per_edge(p);
    selection.effective_row_edge_limit = std::min({limits.max_row_edges,
        limits.max_row_projection_products / unit,
        limits.max_total_projection_products / unit});
    // The histogram is capped by the explicitly configured admissible degree.
    // Avoid allocating an arbitrary graph-sized degree/permutation array.
    if (selection.effective_row_edge_limit > 1000000)
        throw std::runtime_error("oracle degree histogram cap exceeds 1000000");
    const size_t bins = size_t(selection.effective_row_edge_limit) + 1;
    std::vector<uint64_t> histogram(bins, 0);
    std::vector<uint32_t> first(bins, UINT32_MAX);
    uint64_t admissible = 0;
    for (uint64_t i = 0; i < g.n; ++i) {
        const uint64_t degree = row_degree(g, uint32_t(i));
        if (degree > selection.global_max_degree) {
            selection.global_max_degree = degree; selection.global_max_degree_node = uint32_t(i);
        }
        if (degree > selection.effective_row_edge_limit) { ++selection.oversized_graph_rows; continue; }
        ++histogram[size_t(degree)]; ++admissible;
        if (first[size_t(degree)] == UINT32_MAX) first[size_t(degree)] = uint32_t(i);
    }
    if (!admissible) throw std::runtime_error("no complete row fits FP64 oracle budget");
    std::set<uint32_t> requested;
    auto request = [&](uint32_t row) {
        if (!requested.insert(row).second) return;
        const uint64_t degree = row_degree(g, row);
        std::string reason;
        if (degree > selection.effective_row_edge_limit) reason = "row_cost_limit";
        else if (selection.rows.size() >= limits.max_rows) reason = "row_count_limit";
        else if (degree > (limits.max_total_projection_products - selection.projection_products) / unit)
            reason = "total_cost_limit";
        if (!reason.empty()) { selection.skipped_candidates.push_back({row, degree, reason}); return; }
        selection.rows.push_back(row); selection.selected_edges += degree;
        selection.projection_products += degree * unit;
    };
    request(0);
    request(selection.global_max_degree_node); // Publish explicitly when skipped.
    for (size_t degree = bins; degree-- > 0;) if (histogram[degree]) { request(first[degree]); break; }
    for (double fraction : {0., .5, .9, .95, .99, 1.}) {
        const uint64_t target = std::max<uint64_t>(1, uint64_t(std::ceil(fraction * admissible)));
        uint64_t cumulative = 0;
        for (size_t degree = 0; degree < bins; ++degree) {
            cumulative += histogram[degree];
            if (cumulative >= target) { request(first[degree]); break; }
        }
    }
    for (uint64_t degree : {0ULL, 1ULL, 2ULL, 15ULL, 16ULL, 17ULL, 31ULL, 32ULL, 33ULL,
                            63ULL, 64ULL, 65ULL, 127ULL, 128ULL, 129ULL, 255ULL, 256ULL,
                            257ULL, 1023ULL, 1024ULL, 1025ULL})
        if (degree < bins && histogram[size_t(degree)]) request(first[size_t(degree)]);
    request(uint32_t(g.n - 1));
    if (g.n <= limits.max_rows) for (uint64_t i = 0; i < g.n; ++i) request(uint32_t(i));
    return selection;
}

Oracle stable_oracle(const Graph& g, const Param& p, const std::vector<float>& input,
                     AttentionView attention, const RowSelection& selection, bool hidden) {
    p.validate(); check_csr_shape(g); check_attention(attention, g, p);
    check_selection(g, p, selection);
    if (input.size() != checked(g.n, p.in)) throw std::runtime_error("oracle input shape mismatch");
    Oracle oracle; oracle.selection = selection; oracle.heads = p.heads; oracle.dim = p.dim;
    oracle.activation_applied = hidden;
    oracle.transform_first.assign(checked(selection.rows.size(), p.width()), 0.);
    oracle.aggregate_first.assign(oracle.transform_first.size(), 0.);
    Metrics metrics;
    for (size_t ri = 0; ri < selection.rows.size(); ++ri) {
        const uint32_t row = selection.rows[ri];
        const uint64_t begin = g.row[row], end = g.row[row + 1];
        for (int head = 0; head < p.heads; ++head) {
            std::vector<float> scores(size_t(end - begin), 0.f);
            float maximum = -INFINITY;
            for (uint64_t edge = begin; edge < end; ++edge) {
                const uint32_t source = g.col[edge];
                if (source >= g.n) throw std::runtime_error("oracle source out of bounds");
                const float score = edge_score(attention, row, source, head);
                scores[size_t(edge - begin)] = score; maximum = std::max(maximum, score);
            }
            std::vector<double> weighted_h(size_t(p.in), 0.);
            std::vector<double> weighted_z(size_t(p.dim), 0.);
            double denominator = 0;
            for (uint64_t edge = begin; edge < end; ++edge) {
                // Same FP64 weight is reused by both algebraic evaluation orders.
                const double weight = std::exp(double(scores[size_t(edge - begin)]) - double(maximum));
                denominator += weight;
                const float* feature = input.data() + size_t(g.col[edge]) * p.in;
                for (int k = 0; k < p.in; ++k) weighted_h[size_t(k)] += weight * finite_value(feature[k]);
                for (int dim = 0; dim < p.dim; ++dim) {
                    double projected = 0;
                    for (int k = 0; k < p.in; ++k)
                        projected += finite_value(feature[k]) * finite_value(p.w[size_t(k) * p.width() + head * p.dim + dim]);
                    weighted_z[size_t(dim)] += weight * projected;
                }
            }
            for (int dim = 0; dim < p.dim; ++dim) {
                double aggregated_projection = 0;
                for (int k = 0; k < p.in; ++k)
                    aggregated_projection += weighted_h[size_t(k)] * finite_value(p.w[size_t(k) * p.width() + head * p.dim + dim]);
                double transform = denominator > 0 ? weighted_z[size_t(dim)] / denominator : 0;
                double aggregate = denominator > 0 ? aggregated_projection / denominator : 0;
                if (hidden) {
                    if (transform < 0) transform = std::expm1(transform);
                    if (aggregate < 0) aggregate = std::expm1(aggregate);
                }
                const size_t index = ri * p.width() + head * p.dim + dim;
                oracle.transform_first[index] = transform; oracle.aggregate_first[index] = aggregate;
                if (metrics.add(transform, aggregate) || metrics.count == 1) {
                    oracle.associativity.node = row; oracle.associativity.head = head; oracle.associativity.dim = dim;
                    oracle.associativity.reference_at_max = transform; oracle.associativity.actual_at_max = aggregate;
                }
            }
        }
    }
    oracle.associativity.error = metrics.result(); oracle.associativity.compared_values = metrics.count;
    return oracle;
}
Oracle stable_oracle(const Graph& g, const Param& p, const std::vector<float>& input,
                     const std::vector<float>& lr, const RowSelection& selection, bool hidden) {
    return stable_oracle(g, p, input, AttentionView::from_interleaved(lr, g.n, p.heads), selection, hidden);
}
Comparison compare_sampled(const Oracle& oracle, const Graph& g, const Param& p,
                           const std::vector<float>& output, OracleBranch branch) {
    check_selection(g, p, oracle.selection);
    const auto& reference = branch == OracleBranch::TransformFirst ? oracle.transform_first : oracle.aggregate_first;
    if (oracle.heads != p.heads || oracle.dim != p.dim ||
        output.size() != checked(g.n, p.width()) ||
        reference.size() != checked(oracle.selection.rows.size(), p.width()))
        throw std::runtime_error("sample output shape mismatch");
    Metrics metrics; Comparison comparison;
    for (size_t ri = 0; ri < oracle.selection.rows.size(); ++ri) {
        const uint32_t node = oracle.selection.rows[ri];
        for (int head = 0; head < p.heads; ++head) for (int dim = 0; dim < p.dim; ++dim) {
            const double ref = reference[ri * p.width() + head * p.dim + dim];
            const double actual = output[size_t(node) * p.width() + head * p.dim + dim];
            if (metrics.add(ref, actual) || metrics.count == 1) {
                comparison.node = node; comparison.head = head; comparison.dim = dim;
                comparison.reference_at_max = ref; comparison.actual_at_max = actual;
            }
        }
    }
    comparison.error = metrics.result(); comparison.compared_values = metrics.count; return comparison;
}

AttentionDrift attention_drift(const Graph& g, const Param& p, AttentionView reference,
                              AttentionView actual, const RowSelection& selection) {
    p.validate(); check_csr_shape(g); check_selection(g, p, selection);
    check_attention(reference, g, p); check_attention(actual, g, p);
    AttentionDrift drift; Metrics left, right, scores, probability;
    for (uint32_t node : selection.rows) for (int head = 0; head < p.heads; ++head) {
        left.add(reference.L(node, head), actual.L(node, head));
        const uint64_t begin = g.row[node], end = g.row[node + 1];
        std::vector<float> ref_scores(size_t(end - begin)), actual_scores(ref_scores.size());
        float ref_max = -INFINITY, actual_max = -INFINITY;
        for (uint64_t edge = begin; edge < end; ++edge) {
            const uint32_t source = g.col[edge];
            if (source >= g.n) throw std::runtime_error("attention drift source out of bounds");
            right.add(reference.R(source, head), actual.R(source, head));
            const float r = edge_score(reference, node, source, head);
            const float a = edge_score(actual, node, source, head);
            ref_scores[size_t(edge - begin)] = r; actual_scores[size_t(edge - begin)] = a;
            ref_max = std::max(ref_max, r); actual_max = std::max(actual_max, a);
            if (scores.add(r, a) || scores.count == 1) {
                drift.max_score_node = node; drift.max_score_edge = edge; drift.max_score_head = head;
            }
        }
        double ref_den = 0, actual_den = 0;
        for (size_t index = 0; index < ref_scores.size(); ++index) {
            ref_den += std::exp(double(ref_scores[index]) - double(ref_max));
            actual_den += std::exp(double(actual_scores[index]) - double(actual_max));
        }
        for (size_t index = 0; index < ref_scores.size(); ++index) {
            const double r = std::exp(double(ref_scores[index]) - double(ref_max)) / ref_den;
            const double a = std::exp(double(actual_scores[index]) - double(actual_max)) / actual_den;
            if (probability.add(r, a) || probability.count == 1) {
                drift.max_probability_node = node; drift.max_probability_edge = begin + index;
                drift.max_probability_head = head;
            }
        }
    }
    drift.left = left.result(); drift.right = right.result(); drift.score = scores.result();
    drift.probability = probability.result(); drift.sampled_row_heads = left.count;
    drift.sampled_edge_heads = scores.count; return drift;
}

void print_selection(std::ostream& out, const std::string& label, int layer, const RowSelection& s) {
    out << "ORACLE_SELECTION path=" << label << " layer=" << layer
        << " graph_nodes=" << s.graph_nodes << " graph_edges=" << s.graph_edges
        << " selected_row_count=" << s.rows.size() << " selected_edges=" << s.selected_edges
        << " projection_products=" << s.projection_products
        << " max_projection_products=" << s.limits.max_total_projection_products
        << " effective_max_row_edges=" << s.effective_row_edge_limit
        << " oversized_graph_rows=" << s.oversized_graph_rows
        << " global_max_degree_node=" << s.global_max_degree_node
        << " global_max_degree=" << s.global_max_degree
        << " quantiles=admissible_rows full_neighborhood=true coverage=sampled_only row_ids=";
    for (size_t i = 0; i < s.rows.size(); ++i) { if (i) out << ','; out << s.rows[i]; }
    out << '\n';
    for (const auto& skipped : s.skipped_candidates)
        out << "ORACLE_SKIPPED path=" << label << " layer=" << layer << " node=" << skipped.node
            << " degree=" << skipped.degree << " reason=" << skipped.reason << '\n';
}
void print_comparison(std::ostream& out, const std::string& label, int layer, const Comparison& c) {
    out << std::setprecision(12) << "ORACLE_CHECK path=" << label << " layer=" << layer
        << " compared_values=" << c.compared_values;
    print_metrics(out, "output", c.error);
    out << " max_error_node=" << c.node << " head=" << c.head << " dim=" << c.dim
        << " reference_at_max=" << c.reference_at_max << " actual_at_max=" << c.actual_at_max
        << " score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only\n";
}
void print_attention_drift(std::ostream& out, const std::string& label, int layer, const AttentionDrift& d) {
    out << std::setprecision(12) << "ATTENTION_DRIFT path=" << label << " layer=" << layer
        << " row_heads=" << d.sampled_row_heads << " edge_heads=" << d.sampled_edge_heads;
    print_metrics(out, "left", d.left); print_metrics(out, "right_edge_weighted", d.right);
    print_metrics(out, "score", d.score); print_metrics(out, "probability", d.probability);
    out << " max_score_node=" << d.max_score_node << " max_score_edge=" << d.max_score_edge
        << " max_score_head=" << d.max_score_head << " max_probability_node=" << d.max_probability_node
        << " max_probability_edge=" << d.max_probability_edge << " max_probability_head=" << d.max_probability_head
        << " same_input_required=true softmax_eval=FP64_stable coverage=sampled_only\n";
}
} // namespace gat::checks
