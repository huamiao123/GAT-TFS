#pragma once

#include "joint_online.hpp"

namespace gat::joint_full {

using WorkspaceJoint = joint::WorkspaceJoint;
using Stats = joint::Stats;
using Timing = joint::Timing;

// Same FP32 dataflow and workspace as joint v1. A divisible head count
// selects Full=true so active-head guards are absent from that specialization.
void aggregate(const Graph&, const Param&, const local::PreparedLocal&,
               const Schedule&, const std::vector<float>&, WorkspaceJoint&,
               int head_group, int block, int panel, Stats&,
               uint64_t sample_period = 0, bool counters = false);

void layer(const Graph&, const Param&, const local::PreparedLocal&,
           const Schedule&, const std::vector<float>&, WorkspaceJoint&,
           bool hidden, int head_group, int block, int panel, Timing&,
           uint64_t sample_period = 0, bool counters = false);

} // namespace gat::joint_full
