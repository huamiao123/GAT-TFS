#pragma once
namespace gat::icpp_heads_hybrid {
// Independently selects PH arithmetic per original CSR row/block and simulates
// PH -> RNE(U_B) -> AMX UW, plus FP64 algebra, exact state, mixed work counts,
// profile/counter invariance and threshold endpoints. Master .003 unchanged.
void run_hybrid_smoke();
}
