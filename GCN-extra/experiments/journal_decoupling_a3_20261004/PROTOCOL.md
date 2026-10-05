# A3 row-order control for the journal decoupling study

Input: the user-supplied Desktop TFS journal study document,
archived byte-for-byte as `GCN-extra/docs/TFS_JOURNAL_STUDY_INPUT_20261004.txt`
(SHA256 `50f954813f76a5e9043cec7b3da4835ffd5135161ed23418af715150f57a7b4a`).
This is W3 of that plan, after initial A1 results on Reddit and Myciel showed
an access/project decoupling benefit. The earlier S/M source and its running
formal job remain frozen. This is a separate, unique build and run.

The primary three-way mechanisms are A0 `bfull_fast`, A1
`s64_mfull_fast`, A2 `b64_fast`; A3
`s64_mfull_rowwise_fast` is an additional control. A1 and A3 both use a 64
neighbor access boundary, preserve one projection per nonempty destination
tile, use identical BF16 partial packing, and perform the same FP32 partial
store/load at every active row/window. A1 interleaves destination rows after
each 64-neighbor window. A3 completes all windows for a destination row before
moving to the next. Within each row the CSR addition order remains unchanged.
No global AH is written. The same DegreeSort, TR16, R64 scheduling, AMX tile
configuration, output panel, prefetch boundary and compiler flags apply.

The generated A3 header is a mechanical clone of the prior S/M kernel with a
namespace change and the row/window loop nesting exchanged. Its derivation
is regenerated from the frozen base and byte compared in every build. The
partial trace exists only inside `kernel<true>` for small correctness fixtures;
the compiler discards that branch for uninstrumented performance calls.

Before timing, require the original four numerical gates per method plus
three bitwise output gates at the same projection scope. On all three small
fixtures, additionally require the A3 FULL FP32 partial at layer 1 and layer
2 to match an independent sequential CSR sum bit for bit. Profiled A3 output
must match uninstrumented A3 output bit for bit. Validate real source visits,
projection scopes, TDP count, partial state bytes, and per-method runtime.
All failed cases remain visible.

Primary time is five repeated prepared two-layer complete forwards after
immediate warmup, with independent five-order rotating control and retained
min/median/max/raw values. First run four diagnostic graphs: Products,
Reddit, Mycielskian19, RoadNet-CA. Extend remaining thirteen eligible graphs
after these gates succeed. A3 ratios use the same graph/node/process and
measurement order as A0/A1/A2. A/B/C phase times are diagnostic and sampled
thread times cannot be added into wall time. A3 is a mechanism control, not
an assumed faster method.

Use 32 physical cores on one Sapphire Rapids socket, shared `intel` node as
explicitly authorized, `OMP_PROC_BIND=close`, `OMP_PLACES=cores`, default NUMA
policy, no `numactl --interleave`, no MKL thread override, no change to BF16
numerical thresholds, and at most two one-node jobs concurrently. Original
TFS and original source MKL remain same-process anchors. The source model is
random H/W, unit CSR, 128→128→128 static two-layer computation; no trained
classification result is claimed.

A0/A1/A3 fixed projection and numerical path isolate loop order plus the
resulting compiler/cache behavior. Equal analytical instruction counts do
not prove equal retired instructions or DRAM traffic. If A3 reproduces A1's
improvement, cross-row interleaving is not necessary for the observed gain;
if A3 tracks A0, interleaving becomes a stronger candidate mechanism, subject
to PMU/assembly and repeatability checks. No conclusion about cache or
bandwidth is made solely from timing.
