#pragma once

#include "local_online.hpp"

namespace gat::joint {

// U is worker-local [16 destination rows][independent heads][padded D].
// No complete Z, U, edge-score, or attention matrix is allocated.
struct WorkspaceJoint {
    std::vector<float> lr, out, u, c;
    std::vector<uint32_t> blocks, updates, rescales;
    int workers = 0;
    int padded_in = 0;
    void allocate(const Graph&, const Param&, bool counters = false);
};

// Sampled worker sums, not an additive wall-time decomposition. Rescale is
// fused into the accumulator load/update and is timed with weighted SpMM.
struct Stats {
    double init = 0, score = 0, spmm_rescale = 0, gemm = 0, output = 0;
    uint64_t sampled_tiles = 0;
    uint64_t sampled_neighbor_group_blocks = 0;
    uint64_t sampled_row_head_blocks = 0;
    uint64_t sampled_source_vector_loads = 0;
    uint64_t sampled_fma_vectors = 0;
};

struct Timing {
    double lr = 0, kernel = 0, activation = 0, total = 0;
    Stats sample;
};

// head_group is the requested maximum, one of 1/2/4/8. Layers with fewer
// heads dispatch a smaller specialization; the last group may be partial.
// PreparedLocal is reused for master-FP32 bL/bR and contiguous per-head W.
void aggregate(const Graph&, const Param&, const local::PreparedLocal&,
               const Schedule&, const std::vector<float>&, WorkspaceJoint&,
               int head_group, int block, int panel, Stats&,
               uint64_t sample_period = 0, bool counters = false);

void layer(const Graph&, const Param&, const local::PreparedLocal&,
           const Schedule&, const std::vector<float>&, WorkspaceJoint&,
           bool hidden, int head_group, int block, int panel, Timing&,
           uint64_t sample_period = 0, bool counters = false);

void print_stats(const WorkspaceJoint&, const Param&, int layer);

} // namespace gat::joint
