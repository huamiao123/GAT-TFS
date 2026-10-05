# Frozen feature storage × fusion granularity ablation

Source anchor: commit 8aeef1613fdeaeb929379874c4152eccde620f74, `GCN-extra/runs/paper-method-build-20261004-123445/source_snapshot`. Original TFS/MKL and frozen BF16 reduction remain byte for byte unchanged. New code and snapshots are isolated.

Primary factorial: B64/FULL × BF16-input/native FP32-input, FAST partial representation. D=F=128, TR16, KB4, NP2, ascending DegreeSort, R64 dynamic(1), original W packing, partial truncation to BF16, AMX configuration/instructions, C staging, scatter, seed and compiler flags remain fixed. FP32 changes only feature pointer/load/decode and the required two-layer activation dataflow. No S/M window variant participates.

The current four prefetch instructions at offsets 0,64,128,192 remain unchanged for both inputs. They cover the entire BF16 row but the first half of a FP32 row. This is a fixed prefetch instruction budget; an eight-line FP32 prefetch is not silently introduced.

## Runtime and scope

Explicit user steering authorizes shared `intel` nodes rather than waiting for exclusive `intel_expr`. Correctness smoke uses 4 physical cores; real graphs use 32 physical cores on one socket, OMP_PROC_BIND=close, OMP_PLACES=cores, original default NUMA policy, no interleave and no new MKL thread/dynamic override. Actual allowed CPUs and package/core IDs must verify the one-socket allocation before timing. Never use CPUs outside Slurm's allocated set. At most two nodes active, below the user's limit five. Shared-node results are labeled accordingly; old exclusive measurements are background, never speedup denominators.

This remains source-defined random H/W two-layer inference computation, not trained checkpoint classification. Graphs, stored loops/order/weights stay unchanged; only unit-valued safe LP64 inputs are eligible. First stage: products, Reddit, Mycielskian19, RoadNet-CA, Wiki-Talk. Expand the remaining twelve only after all five complete and correctness passes; retain failures and slowdowns.

## Correctness

Original numerical gates stay fixed: FAST FP32 relative L2 and normalized maximum <0.01; actual BF16 final <0.02 versus original TFS and <0.03 versus source MKL. No gate relaxation. BF16 controls repeat the original four checks per method.

Each input path also has its own independent reference: CSR per-row FP32 addition in the same B/tile grouping, truncate each FP32 partial to BF16, expand it and BF16-quantized W, perform independent FP32 dense FMA projection, accumulate, apply the path's ReLU and interlayer representation, and convert final output to BF16. Report L1/L2 FP32, FP32-final and BF16-final errors, finite values and checksum. Native FP32 and BF16 outputs may differ and are compared descriptively, without claiming classification accuracy.

For representation-only validation, losslessly expand BF16 features into FP32 and require bytewise kernel equality at actual layer1 and layer2 inputs/output types. Expanded features are prepared with the same parallel static row partition as the BF16 conversion. The matched-input kernel timing is supplementary and is kept separate from native two-layer E2E.

Native H0 FP32 is originally filled serially while H0 BF16 is produced in a parallel conversion. The first real-graph run exposed a large initial input-placement confound and is retained unchanged under its earlier study directory. In this revised primary 2x2, the FP32 input is an exact, bytewise-verified parallel-static copy of source H0 before prepared timing, using the same logical element partition and default NUMA policy as BF16 preparation. Source H0 remains unchanged for original MKL and the two initial representations. Preparation-inclusive timing explicitly includes this FP32 copy. H1 FP32 scatter and H1 BF16 conversion can still create different page distributions. Capture buffer addresses/numa_maps and report this limit; neither same logical partition nor default NUMA proves identical page locality. Matched-input kernel measurements remain a separate control for numerical values and input representation.

## Boundaries and repetitions

Prepared E2E excludes initial H preparation, graph loading, sorting and weight packing exactly as original. BF16 includes the interlayer conversion; native FP32 bypasses it. Report single-layer/kernel, complete prepared E2E, separate stage runs (L1/ReLU/conversion/L2/total), and optional preparation-inclusive E2E as distinct kinds.

Use ten paired repetitions with five forward and five reverse method orders, immediate warmup before a measurement. Record order/warmup/raw min/P25/median/P75/max/CV. Primary ratios BF16_ms/FP32_ms >1 mean FP32 is faster. Original source TFS/MKL remain measured with their original helper and precision. Same graph/process/node/protocol ratios only.

## Fine profiling and hardware counters

Only uninstrumented kernels determine performance. Independent sampled diagnostics separate feature load, BF16 expand/shift decode, FP32 reduction, partial store, partial BF16 conversion, AMX A/B loads/compute, C spill/reload, output conversion/scatter and thread setup/metadata. FP32 decode equals zero. Parent/child intervals overlap; sampled per-thread sums are not E2E wall percentages. Diagnostic outputs must match corresponding uninstrumented outputs byte for byte.

Per-TID PMU diagnostics retain raw/scaled counts, enabled/running time, coverage and enable/disable skew. On SPR CPU family6/model143, use officially defined basic, retired-load cache, TLB and load-stall groups. Unsupported or denied counters remain N/A. Generic caches are not all cache levels; stall events are not a complete TopDown metric. Shared uncore memory bandwidth cannot be attributed to this process and remains unavailable rather than fabricated from requested bytes.

Print N/E/average/max degree/empty ratio; logical real-neighbor count E; BF16 source bytes 256E and FP32 source bytes512E; decode expansions/shifts, FP32 loads/adds, partial conversion counts, actual padded TDP counts and C stores/reloads from code. Distinguish logical operations/requested bytes from retired instructions/measured DRAM traffic. Inspect compiled hot loops for memory-operand instruction fusion.

## Evidence

Unique `runs/feature-precision-ablation-<timestamp>` contains source_snapshot/build/logs/results/manifest.json/README.md, frozen-source/document hashes, actual worktree head/dirty status, flags/modules, Slurm/accounting, CPU/affinity/NUMA, graph hashes, raw outputs and all gates. Update all three project handoffs after each build/submission/completion. No frozen snapshot or previous valid results are overwritten.
