# Controlled remeasurement of neighbor reduction against paper TFS v3

## Frozen controls

Reference is unmodified `original/gcn_e2e_v3.cpp`, not `gcn_e2e_bench.cpp`.
Every path uses identical graph, stored edges/self-loops, original node order,
ascending DegreeSort, R=64, TR=16, D=F=128, master H/W initialized by the original
srand(12345) sequence, BF16 truncation and VNNI W packing. No topology, graph
normalization, random initialization, bias or activation changes are allowed.

Both TFS and reduction paths use FP32 layer1 output, FP32 ReLU, original
FP32-to-BF16 interlayer conversion, and direct BF16 layer2 output. Reduction
does not materialize a full FP32 final output and convert it afterward.
Original MKL remains FP32, original allocations, original hint=10, original
`mkl_sparse_s_mm` then `cblas_sgemm`; no MKL implementation changes.

Runtime: same node and process for all paths on each graph, OMP_NUM_THREADS=32,
OMP_PROC_BIND=close, OMP_PLACES=cores, no explicit NUMA/interleave policy, no MKL
thread/dynamic overrides, original compiler flags and modules. Formal graphs
may be distributed across separate exclusive nodes; a speedup never divides
timings from different nodes. At most three nodes are planned, below limit five.

The two intended variables are neighbor block size {2,4,8,16,32,64,full} and
FP32-partial representation: FAST truncated BF16 vs ACCURATE hi+residual-lo.
The existing reduction, AMX projection, local accumulation, schedule, and
logical graph view are retained. Adaptation is limited to typed final stores
and the paper v3 harness; the generation manifest records the changes.

## Correctness gates fixed before measurements

Every method checks all output elements with identical input tensors.
FP32 kernel and two-layer FP32-final checks against paper v3 use the existing
FAST 1e-2 and ACCURATE 1e-3 gates for relative L2 and normalized max error.
BF16-final checks against paper BF16-final use 2e-2 FAST and 1e-2 ACCURATE,
allowing one final BF16 rounding bin in addition to the strict FP32 checks.
Actual BF16 final output vs original FP32 MKL retains the paper reproduction
gate of 3e-2 for both metrics. Original repeated output must be bitwise equal.
NaN-prefilled buffers check complete writes, including empty rows and tails.
Gate failures are retained; no numerical threshold may be relaxed afterward.

Only unit-valued graphs within original LP64 bounds are eligible. Six weighted
CSR graphs have unequal original TFS/MKL operators; Friendster and road_usa
exceed original LP64 limits. Their exclusions remain explicit.

## Timing and order

The original paper source is recovered byte for byte after removing two
insertions: a header and a candidate hook after original timings/precision.
An independently compiled unmodified paper binary remains the timing anchor.
Its original one-warmup, five-run minimum results are retained unchanged.

After all candidate numerical checks pass, supplemental measurements use one
warmup and five repeats for all methods and exact source MKL. Method order is
rotated between repeats, in the same process, using the original source buffers.
Report minimum and median. Primary speedups divide corresponding statistics
from this supplemental comparison; do not mix historical timings or nodes.
Single-layer measurements use FP32 output for all TFS methods. Two-layer E2E
includes both layers, ReLU, and interlayer conversion with BF16 final output.
Loading, sorting, initial H conversion and W packing remain outside timing.

Separate three-repeat stages report layer1, ReLU, conversion, layer2 and total.
Separate sampled profiles report scheduling, gather/decode, FP32 reduction,
partial conversion, AMX tile load/compute/store, and output store. Output-store
phase includes final conversion in the BF16 layer. Profile sums are thread
time, not wall latency, and must not be used for speedup calculations.

Negative results and block-size crossovers are evidence. An after-the-fact best
block is an oracle, not a deployed adaptive policy. Fixed B8, B16, B32, B64 and
FULL averages must remain visible for FAST and ACCURATE separately.
