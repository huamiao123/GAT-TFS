#pragma once
#include "gat.hpp"

namespace gat::icpp_block {
// Diagnostic microbenchmark only: a maximum of 512 selected real destination
// rows, their first 32 neighbors, and eight independent L2 heads. It does not
// change the full-model kernel or certify model precision/performance.
void probe_head_PH(const Graph&,const Param&,const gat::Workspace&,const Schedule&);
}
