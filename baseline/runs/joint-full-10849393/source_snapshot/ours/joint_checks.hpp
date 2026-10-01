#pragma once
#include "gat.hpp"
#include <iosfwd>

// Correctness diagnostics only. Call outside every benchmark timing interval.
// The oracle has no dependency on joint_online.hpp or an optimized kernel.
namespace gat::checks {

// A non-owning view: the backing workspace/vector must remain alive.
// Interleaved layout is [node][L_0..L_K-1,R_0..R_K-1].
struct AttentionView {
    const float* interleaved = nullptr;
    const float* left = nullptr;
    const float* right = nullptr;
    uint64_t nodes = 0;
    int heads = 0;
    static AttentionView from_interleaved(const std::vector<float>&, uint64_t, int);
    static AttentionView from_workspace(const Workspace&, uint64_t, int);
    float L(uint64_t node, int head) const;
    float R(uint64_t node, int head) const;
};

struct Limits {
    size_t max_rows = 64;
    uint64_t max_row_edges = 4096;
    // Products count the expensive transform-first FP64 H_j W^h oracle.
    // These limits do not mean truncated neighborhoods: oversized rows are skipped.
    uint64_t max_row_projection_products = 16000000;
    uint64_t max_total_projection_products = 100000000;
};
struct SkippedRow {
    uint32_t node = 0;
    uint64_t degree = 0;
    std::string reason;
};
struct RowSelection {
    std::vector<uint32_t> rows;
    std::vector<SkippedRow> skipped_candidates;
    uint64_t graph_nodes = 0, graph_edges = 0;
    uint64_t selected_edges = 0, projection_products = 0;
    uint64_t effective_row_edge_limit = 0, oversized_graph_rows = 0;
    uint32_t global_max_degree_node = 0;
    uint64_t global_max_degree = 0;
    Limits limits;
};

// Deterministic selection: node 0, highest admissible degree, degree quantiles
// over admissible rows, block-boundary degrees, last node. Small graphs also
// request all rows. Every omitted candidate and the global degree limit are public.
RowSelection choose_rows(const Graph&, const Param&, const Limits& = Limits{});

struct Comparison {
    Error error;
    uint64_t compared_values = 0;
    uint32_t node = 0;
    int head = 0, dim = 0;
    double reference_at_max = 0, actual_at_max = 0;
};
enum class OracleBranch { TransformFirst, AggregateFirst };
struct Oracle {
    RowSelection selection;
    int heads = 0, dim = 0;
    bool activation_applied = false;
    // [selection row index][head][dim], never a full N x D matrix.
    std::vector<double> transform_first, aggregate_first;
    Comparison associativity;
};

// One FP32 add/LeakyReLU score definition, one FP64 stable exp/denominator,
// and the SAME weights for A(HW) and (AH)W. H and W are widened from FP32.
// Full original CSR neighborhoods are evaluated; duplicate edges are preserved.
// hidden=false compares pre-activation output. Empty rows yield zero.
Oracle stable_oracle(const Graph&, const Param&, const std::vector<float>& input,
                     AttentionView, const RowSelection&, bool hidden = false);
Oracle stable_oracle(const Graph&, const Param&, const std::vector<float>& input,
                     const std::vector<float>& interleaved_lr,
                     const RowSelection&, bool hidden = false);

// Compare a full ORIGINAL-NODE-ORDER optimized output with selected FP64 outputs.
Comparison compare_sampled(const Oracle&, const Graph&, const Param&,
                           const std::vector<float>& full_output,
                           OracleBranch = OracleBranch::AggregateFirst);

struct AttentionDrift {
    Error left, right, score, probability;
    uint64_t sampled_row_heads = 0, sampled_edge_heads = 0;
    uint32_t max_score_node = 0, max_probability_node = 0;
    uint64_t max_score_edge = 0, max_probability_edge = 0;
    int max_score_head = 0, max_probability_head = 0;
};
// Used after computing BOTH L/R implementations on the SAME layer input.
// Left is sampled at destination rows; right is edge-weighted (not unique-node
// weighted). Probabilities use FP64 stable softmax for each respective L/R.
AttentionDrift attention_drift(const Graph&, const Param&, AttentionView reference,
                              AttentionView actual, const RowSelection&);

void print_selection(std::ostream&, const std::string& label, int layer,
                     const RowSelection&);
void print_comparison(std::ostream&, const std::string& label, int layer,
                      const Comparison&);
void print_attention_drift(std::ostream&, const std::string& label, int layer,
                           const AttentionDrift&);
} // namespace gat::checks
