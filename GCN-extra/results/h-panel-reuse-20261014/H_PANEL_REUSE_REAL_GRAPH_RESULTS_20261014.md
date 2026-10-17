# Original TFS V3: H reuse versus accumulator residency

Date: 2026-10-14. Status: completed shared-node diagnostic; not paper-table evidence.

## Question and implementation

Original TFS V3 traverses the same CSR neighbors once for each of two 64-column
output panels. Our isolated experiment retains the original DegreeSort,
`TR=16`, `R=64`, BF16 input and packed W, four `kb` steps, the original
`TDPBF16PS` arithmetic order within each panel, and FP32 accumulators. It
compares:

- **A0**: original TFS panel-first kernel from the byte-identical source.
- **A1**: neighbor-step-first, gather H once, switch panels after each step.
- **A2–A5**: gather each H window once; panels replay that window. Window sizes
  8, 16, 32, 64.
- **D16**: window-16 duplicate-gather control. It uses the same window and C
  staging as A3, but gathers H again before panel 1.

The source is `../../experiments/h_panel_reuse_20261014/h_panel_reuse.cpp`.
The included original `gcn_e2e_v3.cpp` has SHA-256
`85c51bde92792f98223e80107e20b042f8f8d0e40068cdf6ce168143f0c7743f`.
The tested binary has SHA-256
`eda207b215fa947cc4e314d1b9e77e90cf6bf573d8e14e2ac5f3b1ec5b0f65d7`.
Build: `icpx` 2024.1, `-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile
-mavx512bf16 -qopenmp -qmkl=parallel`, same as the original source build.

## Protocol and correctness

Both real-graph jobs and the reverse-order repetition ran on shared `qhcn132`.
The Slurm allocation pinned 32 physical CPUs 32–63, one socket, with
`OMP_PROC_BIND=close`, `OMP_PLACES=cores`, and default NUMA policy. The same
process generated the same seed-12345 H/W, degree permutation, CSR and W VNNI
packing for all seven methods. Each method was warmed up and measured five
times in alternating forward/reverse order; the next job reversed graph order.

Initial FP32→BF16 H and W packing were outside the prepared two-layer timer.
Layer 1 BF16→FP32, FP32 ReLU, interlayer FP32→BF16, and Layer 2 BF16→BF16
were inside it. Every method on each graph passed bitwise checks for L1 FP32,
interlayer BF16, and final L2 BF16 against A0. This is **algorithmic and
bitwise equivalence**, not a classification-accuracy claim.
The artifact audit passed 56 real-graph method checks and verified 280 raw
timing rows, identical graph hashes on repeats, one binary hash and one node.

Slurm jobs: `10964915` (RoadNet, Reddit), `10964919` (Products, Myciel),
`10964921` (four graphs in reverse order). All completed successfully.
The first small-fixture JobID `10964897` tested empty rows, short/high degrees
and partial row tiles. The run directories below contain raw logs, manifests,
source snapshots, commands, hashes, node topology, NUMA state, and timings.

## Two-layer E2E, median of five, milliseconds

Ratios are `A0 time / candidate time`; values above 1 mean the candidate is
faster. The first and reverse measurements are independent same-node runs.

| Graph | Run | A0 | A1 | W16 | W32 | W64 | D16 | W64 vs A0 |
|---|---|---:|---:|---:|---:|---:|---:|---:|
| roadNet-CA | first | 36.91 | 33.84 | 32.09 | 32.21 | 31.97 | 36.53 | 1.155× |
| roadNet-CA | reverse | 36.63 | 33.62 | 32.15 | 32.23 | 32.81 | 36.69 | 1.117× |
| reddit | first | 305.92 | 373.97 | 259.03 | 239.03 | 235.57 | 340.22 | 1.299× |
| reddit | reverse | 311.50 | 362.90 | 253.41 | 241.31 | 236.70 | 335.48 | 1.316× |
| ogbn-products | first | 338.63 | 437.34 | 319.09 | 306.71 | 300.65 | 397.69 | 1.126× |
| ogbn-products | reverse | 338.37 | 440.13 | 318.02 | 308.89 | 297.78 | 399.39 | 1.136× |
| mycielskian19 | first | 2235.12 | 1789.26 | 1815.87 | 1690.73 | 1705.79 | 2291.78 | 1.310× |
| mycielskian19 | reverse | 2194.17 | 1804.04 | 1786.88 | 1681.58 | 1661.70 | 2346.09 | 1.320× |

A fixed W64 gives a four-graph geometric mean speedup of **1.220× first / 
1.219× reverse**. This is a same-node shared allocation result, not a formal
exclusive-node speedup. W32 and W64 on Myciel swap rank across runs; RoadNet
W16/W32/W64 are also too close to declare one stable best window.

## Layer timing evidence

Median stage times from the first job, milliseconds. These independently
computed medians need not sum exactly to the median E2E time.

| Graph | Method | L1 | ReLU | H1 convert | L2 | E2E |
|---|---|---:|---:|---:|---:|---:|
| roadNet-CA | A0 | 15.1 | 6.1 | 4.2 | 11.8 | 36.9 |
| roadNet-CA | W64 | 12.4 | 5.7 | 4.1 | 9.8 | 32.0 |
| reddit | A0 | 149.0 | 0.4 | 1.7 | 154.5 | 305.9 |
| reddit | W64 | 114.9 | 0.4 | 1.9 | 116.1 | 235.6 |
| ogbn-products | A0 | 164.9 | 4.6 | 5.1 | 163.2 | 338.6 |
| ogbn-products | W64 | 146.3 | 4.8 | 5.2 | 144.4 | 300.7 |
| mycielskian19 | A0 | 1114.0 | 0.7 | 0.8 | 1100.1 | 2235.1 |
| mycielskian19 | W64 | 853.9 | 0.7 | 0.9 | 845.8 | 1705.8 |

The benefit is inside both TFS layers. ReLU and interlayer conversion remain
similar between paths, so they do not explain these speedups.

## Work ledger, per layer

Counts come from the CSR and exact loop bounds; they are logical instruction/
byte requests and **must not be interpreted as measured DRAM traffic**.

| Graph | N | E | A0 H copies | shared H copies | TDP for every method | W32 extra C tile loads | W64 extra C tile loads |
|---|---:|---:|---:|---:|---:|---:|---:|
| roadNet-CA | 1,971,281 | 5,533,214 | 11,066,428 | 5,533,214 | 11,066,976 | 0 | 0 |
| reddit | 232,965 | 114,615,892 | 229,231,784 | 114,615,892 | 230,000,448 | 1,739,328 | 843,432 |
| ogbn-products | 2,449,029 | 123,718,152 | 247,436,304 | 123,718,152 | 248,012,384 | 1,437,944 | 543,832 |
| mycielskian19 | 393,215 | 903,194,710 | 1,806,389,420 | 903,194,710 | 1,810,038,496 | 14,046,240 | 6,979,872 |

Each source H copy is 256 BF16 bytes. The number of AMX H tile loads and W
tile loads is unchanged across methods. Each extra C tile load has a matching
store; each 16×16 FP32 tile is 1024 bytes. A1 pays, respectively, 1,784,128;
57,383,624; 60,802,784; and 452,313,016 extra C tile **loads and stores**
on the four graphs. RoadNet has zero extra C staging at W32/W64 because every
sorted 16-row batch fits in one window of those sizes.

## What the controls establish

1. **Naive per-step loop interchange is graph-dependent.** A1 beats A0 on
   RoadNet and Myciel, but is roughly 16–30% slower on Reddit and Products. Its large
   C staging count makes a universal A1 speedup claim untenable.
2. **Windowing consistently beats A0 in this set.** W64 is faster in both runs
   on all four graphs; W32 gives nearly the same range. These preserve all
   original per-neighbor AMX projection work.
3. **H reuse has net value at fixed W16 staging.** W16 beats D16 on every graph
   in both runs. D16 re-gathers H but otherwise uses the same H buffer and C
   staging, so the difference includes avoided CSR/source-copy/prefetch work
   and its resulting cache behavior. It does not isolate DRAM bytes alone.
4. **The optimum window is unresolved.** W32/W64 ranks change on Myciel and
   RoadNet. A paper policy must account for cache footprint and graph degree
   distribution rather than choose a per-graph oracle from these results.

## Limits and next diagnostic

The node was shared, so neighbor jobs can perturb frequency and cache state.
No PMU cache-miss, memory-bandwidth or AMX counters were collected
in this round. The measured stage split covers L1/ReLU/conversion/L2; it does
not yet give direct wall-time attribution for H copy, prefetch, C Tile load/
store, AMX loads or TDP. Logical access counts support the mechanism but do
not prove which cache level served H or C. An independent exclusive-node
repeat and a sampled phase/PMU run are required before paper-level claims.

The original kernel source was not modified. No graph semantics, feature
precision, projection count, activation, output ordering or NUMA policy was
changed. These results compare the original TFS family only; B64/FULL FAST
change arithmetic work and remain a separate, non-single-factor reference.

## Artifact index

Server run directories are under `GCN-extra/runs/`; copies are beside this
report in the local workspace. Each run has raw `logs/*/stdout.txt`,
`results/timings.csv`, `results/checks.csv`, `results/work.csv`, and a manifest.

- `h-panel-reuse-10964897/`: correctness smoke, 37/137-node fixtures.
- `h-panel-reuse-10964915/`: RoadNet, Reddit, seven methods.
- `h-panel-reuse-10964919/`: Products, Myciel, seven methods.
- `h-panel-reuse-reverse-10964921/`: reverse graph order, all four graphs.
- `summary_by_run.csv` and `.json`: raw-run statistics without pooling nodes.
- `audit_results.json`: cross-run hash, topology, work-count and correctness
  audit, generated by `verify_results.py`.
- `../../experiments/h_panel_reuse_20261014/PROTOCOL.md`: fixed controls.
