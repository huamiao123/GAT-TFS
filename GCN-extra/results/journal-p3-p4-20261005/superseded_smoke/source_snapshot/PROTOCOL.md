# Journal follow-up P3 / P4

Parent: af943926c7d4f8e68bcb95838924e21b14dd2e86. Original paper TFS/MKL,
P0 kernels, and all earlier frozen evidence remain unchanged. The supplied
journal study authorizes independent new dataflows and shape controls;
these results do not replace the paper-compatible reproduction table.

P3 first broadens the actual-AMX D2 gate to three smoke fixtures and five
diagnostic real graphs. B64 ACCURATE and S64/MFULL ACCURATE are frozen
strong controls. D2 sums FP32 residuals across all blocks and corrects using
hi+lo; D3 corrects with the same two components every four B64 blocks.
Same source seed/graph/W, 128->128->128, original layer transitions and
unchanged ACCURATE gates. Rejected candidates retain their numerical
records and never enter accepted performance tables. Timing follows gate
acceptance, warmup plus five alternating-order forwards, three stages/PMU.
Passing these fixtures is empirical, not a general error guarantee.

P4 examines source-consumer-count selection, partial source projection
materialization and a cold-source locally fused reduction. Use a separate
shape harness, initially 128->32->32 and 128->128->128. Cache projections
remain FP32: hot-edge logical bytes are 4F, cold-edge BF16 H bytes 2D.
Hot membership is based on source consumer counts, not destination degree.
Fixed fractions, including none and all, must be reported; preprocessing,
cache rebuild and merge are charged separately. Cache H*W is rebuilt on
every layer/forward, since features change. No stale cross-inference cache.
Cold partials use ACCURATE hi/lo, with 1e-3 FP32 and 1e-2 BF16-final gates
against same-input quantized-H/W MKL mathematical references. Rejected paths
are not timed. Degenerate none/all paths must be bitwise identical to their
fused/project-first controls. For width128 the fused layer must match frozen
P0 ACCURATE bitwise. The optional original-style FP32 shape MKL is separately
labelled; its dtype and interlayer math differ from matched-input references.
Keep both global project-all and project-only-used-source strong controls.
The latter avoids making unreferenced source projections a weak baseline.

All P4 methods use the same stored unit adjacency, H/W BF16 truncation,
TR16/R64, S64, DegreeSort, dynamic(1) scheduling, packed weights, ReLU,
native interlayer conversion and final BF16 output. Generic shape kernels
derive the original AMX tile/weight packing layout without changing the
paper source. Equal-width controls must align with frozen kernels; new-shape
results are explicitly labelled shape experiments, not original TFS.

Use 32 physical cores on one socket, shared intel as authorized, close/cores,
default NUMA/no interleave, original compiler modules/flags. Read AGENTS and
research skill before each build/run and append every result/failure to all
three GCN-extra handoffs. At most two one-node jobs in this round. Detailed
sampled/thread-local phases are diagnostics, not additive wall decomposition.

Before expanded P4 real graphs, pass small same-operator gates and controlled
source-sharing fixtures. Preserve negative results, numerical rejections and
counterexample boundaries. No classifier checkpoint/accuracy is claimed.
