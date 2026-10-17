# Fixed-quantization partial bundle experiment, 2026-10-13

User explicitly requests implementing and trying the audited improvement.
Static unit-weight inference, random source H/W seed12345, 128->128->128.
This inference scope supersedes the training-only architecture scope without
changing training sources. Maximum two single-node jobs in this round; overall
our wzh experiment node cap remains five. Other account/yx jobs are untouched.

Frozen passing state-reuse run05 supplies ScalarReducer, direct packing,
nozero, precise integer-mask ReLU/BF16 EPI, DegreeSort/TR16/R64 dynamic(1),
VNNI weights, tile configuration, and the exact original TFS/MKL driver.
New executor retains S64 and separate M64/M256/FULL and FAST/ACCURATE.
Only G=1/2/4 changes lifetime of already-quantized partials and C.
No scope sums, quantization boundaries, output TDP order, normalization,
prefetch, NUMA, thread or compiler policy change. Original snapshots immutable.

For each finite M and precision compare canonical PACK_EPI, derived G1/G2/G4.
FULL canonical vs G4 is the Q=1 negative/shape control. P0 FULL and source
TFS/MKL remain anchors; attribution uses matched M/precision canonical.
Strong control does not write full FP32 H1 in production; first-layer ReLU
and BF16 truncation are inside the output EPI. All paths produce final BF16.

Before timing: original numerical gates (FAST FP32 .01, ACCURATE .001,
BF16 FAST .02/ACCURATE .01 vs TFS, BF16 .03 vs original FP32 MKL), finite
outputs and four-boundary exact same-scope checks (kernelFP32, finalFP32,
H1BF16, finalBF16). Keep 19-value EPI semantics fixture. Small fixtures cover
empty/unsorted/duplicate rows, tails, 63/64/65,255/256/257,511/512/513 degrees,
cancellation and incomplete bundles. All controls must pass before timing.
Instrumented production outputs must match uninstrumented outputs bitwise.

Independent work gates derive Q per DegreeSort TR16 tile, bundles ceil(Q/G),
E real reads,128E adds, 32Q/64Q TDP; C stores8*bundles and reloads
8*(bundles-nonempty_tiles). Source/partial work stays equal across G.
Logical requested bytes and source-level TDP counts are not PMU/DRAM bytes.

Shared intel accepted exploratory:32 physical cores,one socket,close/cores,
default NUMA,no interleave, unchanged MKL runtime environment. Correctness
may start at one physical core; that timing is excluded from main results.
Prepared E2E excludes graph/initial H/W/schedule setup, includes both layers,
EPI and final conversion. One warmup,5 consecutive +10 forward/reverse
samples; median,min/max,CV,raw samples retained, no oracle G selection.
Separate stages and sampled24 child intervals/per-thread diagnostics; sums
are not wall percentages. PMU optional independent full-forward regions.

First smoke then five diagnostic graphs:Products,Reddit,Myciel,RoadNet,wiki.
If quota delays 32-core allocation, retain PENDING and do not bypass Slurm
or run graph kernels on login nodes. One graph's methods compare in one
process/node; shared/exclusive and differing thread counts never mixed.
Detailed gates/raw logs/manifest/source+binary+graph hashes and three GCN
handoff updates required. This round makes no classification accuracy claim.
