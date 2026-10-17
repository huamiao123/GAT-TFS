# P1: projection-budget-preserving destination grouping

This independent study continues the user-supplied journal study after P0.
Frozen parent commit: 33fb6b4270b94c1f38e8e9f3d94aed35595a384d.
No original kernel, paper-compatible baseline, or completed run is overwritten.

Every schedule preserves q64=ceil(degree/64) at every DegreeSort position.
Each contiguous q64 bucket is split into segments of at most 4096 rows.
Only rows inside a segment may be reordered. Tail and empty slots are kept;
permutation coverage, per-slot q64, FULL/B64 projection budgets, and logical
window work are checked before performance. CSR neighbor addition order and
original tensor order are unchanged. Output scattering still uses node IDs.

Schedules: original DegreeSort; deterministic row-hash shuffle (negative
control); min-hash of up to 64 initial neighbors' source IDs; min-hash of
their 4KiB feature-page IDs (16 BF16 128-wide features per page, assuming
relative feature address). Both signatures use the same fixed integer hash;
ties retain original DegreeSort position. Signature work is one parallel pass;
each bounded segment is independently sorted with the existing OpenMP team.
The 4096 cap, 64 sample length, and seeds are frozen before graph measurements.
These are candidate mechanisms, not a deployed adaptive policy.

For each schedule execute the frozen S64/MFULL and S64/M64 kernels without
changing their inner loops. Untouched source TFS/MKL, coupled FULL, and coupled
B64 are same-process anchors. Require bitwise equality at FP32 layer1,
FP32 final two-layer, and actual BF16 final boundaries to the same-scope
DegreeSort kernel, in addition to the existing numerical thresholds.
Every profiled kernel must match its own uninstrumented output bit for bit.

Use 32 physical cores on one SPR socket, shared intel as authorized, default
NUMA, close/core binding, seed12345, original compiler flags, TR16/R64,
BF16 H/W, unit CSR values, two-layer 128->128->128, no training/checkpoint.
Prepared E2E includes ReLU and interlayer conversion. Report preprocessing
signature allocation/pass, task construction, each sort and audit separately,
plus the break-even reuse count against the corresponding DegreeSort path.
Do not silently include preprocessing in paper-compatible time. Also report
one-forward cost with candidate preprocessing added, explicitly labelled.
The per-candidate setup sum is a conservative upper estimate: shared allocation
and a signature pass computing all candidate keys are charged to each candidate.
It is not a directly measured standalone one-candidate preprocessing path.

Primary: immediate warmup then five uninstrumented repeats. Order control:
five forward/reverse alternating complete method sweeps. Record min/median/
max/CV and all samples. Diagnostic runs retain two-layer stages, the inherited
24 sampled child intervals, thread completion/work records, logical counters,
three full-forward PMU regions (cycles/instructions/generic cache events/SPR
AMX busy). Sampled thread times and PMU are not additive wall decomposition;
logical requests and generic misses are not measured DRAM bytes.

Run three small fixtures first. Then Products, Reddit, Mycielskian19,
RoadNet-CA, wiki-Talk. Additionally run three controlled regular CSR graphs
(N=524288, degree=128) with tile-local source reuse 1,4,16. All use the same
source permutation and exactly the same global source in-degree (128), N/E,
row degrees and unit values. CSR rows are constructed relative to the actual
native DegreeSort tie permutation. No distinct graph outputs are compared
for equality; method comparisons are within one graph/process/node only.
Expand the remaining 12 real graphs after the diagnostic gate succeeds.

All writes on the server are under wzh/GCN-extra, yx remains read-only.
Run/source/build/launcher hashes, manifests, input hashes, failures and handoffs
are retained. At most two one-node jobs for this phase. No claims of general
cache causality, classification accuracy, or journal novelty from timing alone.
