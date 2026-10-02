#pragma once

namespace gat::icpp_online {
// Correctness-only, deliberately outside all benchmark timing regions.
// Requires AVX-512/SVML and AMX on the machine that executes it. No tests or
// compilation are performed merely by including this declaration.
// Throws after a failed FP32 control, instruction-emulation, state/counter,
// permutation, or profiling-invariance gate. Master BF16 error is reported
// separately; passing these operator checks does not certify model accuracy.
void run_smoke();
}
