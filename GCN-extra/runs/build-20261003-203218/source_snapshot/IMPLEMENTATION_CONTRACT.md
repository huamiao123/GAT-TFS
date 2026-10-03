# GCN-extra inference experiment contract

This project implements the user-approved 2026-10-03 inference redundancy plan.
It is separate from TFS-Train and GAT. Training, backward, dropout, and dynamic
graphs are outside this experiment. The user explicitly requested the new project
under wzh/GCN-extra; all remote artifacts and handoffs stay in that directory.

## Frozen source and mathematics

- `original/amx_tfs_v3.cpp` is an unchanged copy of yx/TFS/code/amx_tfs_v3.cpp.
  SHA256: 4e574786055816df80beb6e68181fe19a8ee02cf4be1d43717bc1e79b88cab29.
- Initial shape: D=F=128, TR=16, R=64. Preserve original ascending DegreeSort
  permutation and original-node output order. All methods receive identical CSR.
- Initial operator: unweighted CSR sum, C=(AH)W. Every stored edge is retained,
  including existing self loops and duplicate entries. No self loops are added.
  CSR values must be exactly one (full scan), or the run rejects the graph.
- Two-layer inference: ReLU(C1), then the same CSR operator with independent W2.
  This reproduces the source's sum-aggregation experiment, not a checkpoint-based
  normalized GCN accuracy claim.
- Same FP32 master H/W; all AMX paths use the same source-compatible truncated
  BF16 H/W. FP32 MKL is the strong reference; a second MKL path receives the
  identical quantized inputs/weights cast back to FP32.

## Paths

- original: directly invoke the unchanged original kernel.
- original_nozero: generate the exact original function with only the initial
  global C memset removed. The original always stores every output row/panel,
  including empty rows. Verify bitwise equality after NaN-prefilling outputs.
- nozero local variants: remove the same initial memset, and owner threads zero
  any entirely empty destination tile. Compare against original_nozero to prevent
  attributing removal of redundant zeroing to the neighbor-reduction algorithm.
- block B fast: FP32 local sum, truncate the sum to BF16, AMX projection.
- block B accurate: truncate hi and residual lo, execute hi*W + lo*W.
- shared: one gather/reduction per block; local output tile spills/reloads between
  blocks and output panels. No global AH. Full has no inter-block output reload.
- replay: keep the original output-panel-outer execution; reduce the same input
  block again per panel. This isolates projection reduction from cross-panel
  gather reuse. Both variants preserve DegreeSort and tile-aware local fusion.

## Evidence and timing

Correctness is checked before timed repetitions. Full outputs are compared with
the BF16-input/weight MKL reference; a stratified row sample also uses FP64
aggregation/projection. Empty rows, negative/cancelling values, tails, and node
order are covered by fixtures. Fast gates rel-L2 and normalized max error at
1e-2; original/accurate at 1e-3. These are numerical gates, not task accuracy.
Both matched-precision and FP32-reference errors remain visible.

Uninstrumented median timings use one warmup and five measured repeats in formal
runs. Kernel includes C zeroing, OpenMP/AMX setup, scheduling and final output.
Prepared two-layer E2E includes both kernels, ReLU and intermediate BF16
conversion; original H conversion, W packing and DegreeSort are separately
reported preprocessing. Methods run in rotating order.

Profiling is a separate sampled run, not the speedup timing. Phase values are
sampled summed thread time and do not equal wall latency. Instrumented original
must match the unchanged kernel. Report sampled tile count and profiler overhead.
Logical feature bytes and modeled physical AMX work are not hardware counters.
For finite B: useful block rows=sum ceil(degree/B); physical block rows=
16*sum over scheduled tiles max ceil(degree/B). Count output panel passes,
hi/lo, local output spills/reloads, actual degree distribution and eta_B.

## Operations

Read current server AGENTS.md and tfs-research-engineering/SKILL.md before every
build/run, save contents and hashes, inspect queue/partition, and update all three
GCN-extra/docs/handoff files after each event. Original sources and yx are read
only. Use one node initially; never exceed the user's five-node limit. Shared
intel is for correctness; intel_expr exclusive is for formal performance.
Hardware/governor/NUMA policy is observed rather than changed cluster-wide.
The Xeon Max nodes expose four CPU NUMA domains per 32-core socket. Formal runs
use 32 physical cores on one socket and process-local interleaving across exactly
that socket's CPU NUMA domains, identically for all paths. This replaces the
plan's infeasible 32-core/single-NUMA assumption; policy and topology are saved.
All paper claims remain UNVERIFIED until evidence supports them.
