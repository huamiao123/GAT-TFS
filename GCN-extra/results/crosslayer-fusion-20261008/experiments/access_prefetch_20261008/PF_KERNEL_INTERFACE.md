# Prefetch kernel interface (inference-only, 2026-10-08)

New headers: `pf_reducer.hpp`, `pf_kernel.hpp`. Frozen existing headers are included unchanged.

API:
```cpp
#include "pf_kernel.hpp"
gcn_extra_prefetch::kernel<P, B16, MODE, PF>(
    graph, hb, packed_w, output, permutation, state_method, plan, profile_ptr);
```
`Plan`, `TilePlan`, and `Profile` are exactly the existing `gcn_extra_pair_plan` types, not a second layout. MODE accepts 0 (Scalar) or 4 (ExactS). PF accepts 0 (OFF), 1 (frozen next-edge prefetch), or 8 (matched-target lookahead eight). Scope 0/FULL, 64, and 256 retain the original S64 kernel contract.

PF8 has a prologue ONCE at the first window of EACH projection scope, separately for each row. For logical row interval [s,e), emit CSR positions s+1 through min(s+7,e-1). During the edge loop at k, emit k+8 iff k+8<e. Target lists equal PF1 exactly in ordered CSR positions: [s+1,...,e-1]. Every request loads its own CSR index and issues four T0 hints over one 256-byte feature row. ExactS retains both rows' requests even if feature addresses coincide. PF0 omits these prefetch requests entirely.

The original current-index reads, per-row CSR add order, BF16 decode sharing, direct packing, partial-buffer traffic, projection counts, AMX slot ownership, projection ordering, epilogue, first-touch, scheduling, and output placement are unchanged by PF policy at source level. PF policy can change compiler instruction scheduling and register allocation; assembly checks and old/new PF1 controls are required before interpreting timing.

Counts per layer: PF1/PF8 prefetch_index_reads = sum over row/scopes max(min(degree,scope_end)-scope_start-1,0), and csr_prefetch_instructions = four times that count. PF0 gives zero for those two fields. Other logical and work counters match the original kernel at the same MODE, Plan and Method.

Untimed runtime audit API:
```cpp
gcn_extra_prefetch::pf_visit_prologue_targets<PF>(ws,row_scope_end,first,emit);
gcn_extra_prefetch::pf_visit_steady_targets<PF>(k,row_scope_end,emit);
```
`emit(uint32_t target)` receives a CSR position relative to the row base. Call prologue at each visited row-window using first=(ws==scope_start), and steady only for consumed k < row_scope_end. These exact visitors drive PF8 production issue. PF1 production deliberately calls the frozen pair_prefetch_next helper verbatim; the PF1 visitor mirrors its bound for the audit. PF0 production and visitors issue nothing. Audit lengths around 0,1,7,8,9,63,64,65,127,128,129,255,256,257, exhausted rows, nonzero scope starts, duplicate/unsorted source IDs, and tails. Actual same-kernel profile request counts remain a separate required gate.

`check_pf_kernel.py` performs read-only source identity and independent position-model checks. It does not establish compiled instruction identity, bitwise numerical correctness, or performance. No compile, Slurm job, or timing was executed by the kernel author.
