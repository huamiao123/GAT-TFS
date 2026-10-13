# Order and grouping balance experiment, 2026-10-07

Scope: independent GCN-extra inference computation experiment. This is not the
TFS-Train v2 training model. Older source and frozen evidence remain unchanged.
User authorizes new experiments; shared plus exclusive allocations must never
exceed 10 nodes in aggregate. Root orchestration owns jobs and preflight.
Default NUMA policy only; NUMA interleave is forbidden.

## Frozen matrix of candidates

Shapes: 128->32->32 and 128->128->128 only. Do not instantiate F=256: the inherited
AMX microkernel and second-layer template support only the stated shapes.

Each layer independently selects A, Pall or Pused:
A = locally fused FULL ACCURATE, AVX FP32 neighbor accumulation of BF16 H,
then BF16 hi/lo AMX projection. No global AH.
Pall = project every source once with AMX, storing FP32 Q in source-ID order;
AVX then reduces Q over exactly the original CSR edges.
Pused = same projected-source dataflow, only for sources consumed by the CSR.
Its Q is compact and ordered by descending consumer count / source-ID tie break.
Pused retains map-dependent indexing. This layout is an explicit confound to
isolate later, not evidence that compact projection is intrinsically inferior.

All 3x3 two-layer choices are compared, each with degree and qsource grouping:
18 predeclared candidates per shape, without a per-graph oracle selector.
qsource is the original P1 rule: first 64 neighbors' minimum mix(source-ID)
signature, stable rank tie break, within 4096-row segments of each q64 bucket.
Every position retains ceil(degree/64). Every CSR row and its neighbor sequence
remain unchanged. Audit permutation validity, full nonempty tile count, summed
tile visit rounds and summed row windows. Padded neighbor steps may change and
are reported, not falsely asserted invariant. No old hard-coded qpage key is used.
Each candidate uses the same grouping in both layers in this first experiment.
Choosing different groupings per layer is deferred, not silently oracle-tuned.

## Input and numerical contract

Use stored unit-edge CSR, without adding self-loops or normalization. Signed
LP64 limits are enforced. Use seed12345 and the original discrete rand()%200
generator; equal-width W1/W2 generation remains interleaved as in aligned P4.
Preserve BF16 bit truncation, packed W layout, TR16/R64, S64, dynamic(1), ReLU,
native BF16 interlayer conversion and final BF16 output. No task accuracy claim.

The mathematical MKL reference decodes the same BF16 H/W and includes the same
BF16 interlayer boundary. Check first-layer raw FP32, two-layer FP32-final and
actual BF16-final. Both relative L2 and normalized max must be below 1e-3 for
FP32 and 1e-2 for BF16-final; finite outputs required. No gate relaxation.
For each same-dataflow degree/qsource pair all three boundaries must also match
bitwise. Equal-width A_A_degree first layer must match frozen P0 FULL ACCURATE.
Keep rejected records and exclude them from all timing.
The original-style FP32 shape MKL is retained only as a separately labelled
anchor: it uses FP32 master H/W and FP32 interlayer math, not matched precision.

A's two BF16 components approximate a FP32 aggregate. P's direct original BF16
source projection needs one component. This is an intended precision-matched
tolerance comparison, not a claim of identical floating-point instruction paths.
Empirical gates do not prove arbitrary-input error or classification accuracy.

## Timing and scratch

Prepared forward includes both layers, every layer's Q rebuild, ReLU,
interlayer conversion and final BF16 output. Source count/rank/map preparation,
grouping, CSR loading, H/W setup and allocation are excluded and separately
reported. Q cannot be reused across forwards or layers with changed features.
Two shared full-capacity Q scratch buffers are reused by all candidates; there
are no 18 separately resident pairs of Q. Active cache bytes are also reported.

For each method: one rewarm plus five consecutive measured forwards.
Then one warmup per method and ten alternating-order rounds, i.e. five
forward/reverse pairs. Report both protocols independently, preserving every
sample. Include the separately labelled FP32 anchor in method ordering.
Two independent coarse-stage passes, one instrumented work/profile pass per
layer and one independent full-forward PMU region per accepted method are
diagnostics, never primary timing samples.

## Work accounting and falsifiable interpretation

For a layer D->F and E unit edges:
A sparse adds=E*D, logical feature reads=2*E*D bytes plus BF16 decode.
P sparse adds=E*F, logical feature reads=4*E*F bytes; Pused additionally maps IDs.
Changing only grouping keeps these effective amounts fixed, but can change
cache behavior, control work and thread completion times.

K=(D/32)*(F/16), T=nonempty destination tiles, C=ceil(projected_sources/16):
A TDP=2*T*K; P TDP=C*K. General hot/cold split would be (2*Tcold+C)*K.
Record actual instrumented edge/hot/cold/cache/cold-tile/parts/TDP counters and
check them against the independent topology plan. Record source-Q rebuild
requests/writes, active cache bytes, vector-add and decode counts. Pused demand
map lookups are E; lookahead-prefetch map lookups are E minus active row windows.
Window accumulator loads/stores are each row_windows*4*(A?D:F) bytes; this
excludes memset initialization and output traffic. Prefetch requests themselves
are not added to demand feature-byte counts.
Logical byte requests are not measured DRAM traffic. AMX_BUSY is not retired
TDP and is not AMX wall time. Sampled thread phases are not additive E2E time.

The objective is lower complete forward latency under unchanged gates, not
equal AVX/AMX utilization or equal stage duration. These kernels perform
sequential layer stages; no true AVX-AMX pipeline-overlap claim is made.
Any speedup must separate reduced feature dimension/work from improved
execution of unchanged work. Preserve negative results and overheads.

## Build handoff

Root freezes this directory's selective_shape.cpp, selective_kernels.hpp and
order_grouping.hpp plus GCN-extra/original/gcn_e2e_v3.cpp and the four src headers
paper_methods_kernels.hpp, paper_methods_runtime.hpp, projection_window_kernels.hpp,
projection_window_pmu.hpp into its unique run source_snapshot.
Compile on the authorized preflight environment with the original flags:
icpx -O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp
-qmkl=parallel source_snapshot/selective_shape.cpp -o build/selective_shape

The copied legacy build_selective.sh still names the old experiment and must
not be executed for this experiment; root owns the new build/job/suite wrapper.
The implementing subagent does not compile, execute kernels or submit jobs.
