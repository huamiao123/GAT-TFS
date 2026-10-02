#pragma once
namespace gat::icpp_heads_preload {
// Bitwise regression against the independently checked frozen heads kernel.
// Run icpp_heads::run_heads_smoke() before these checks. This preserves the
// existing quantized PH/UW semantics; it does not certify master precision.
void run_preload_smoke();
}
