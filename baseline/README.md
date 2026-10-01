# Vanilla GAT baseline suite

Research update (2026-10-01): [grouped-head Online results](docs/JOINT_ONLINE_RESULTS_20261001.md) and [method/literature audit](docs/METHOD_RESEARCH_20261001.md). Independent FP32 aggregate-first candidates retain Degree Sort, TR16, local U and block Online state; source loads are reused across independent heads. Two real-graph shared-node jobs preserve all original baselines. These candidates remain slower than strong B0 and have not passed the original master absolute-error gate; they are not a replacement BF16 B1.

Performance correction (2026-09-30): [B0 NUMA diagnosis](docs/B0_SLOWDOWN_DIAGNOSIS_20260930.md) and [B1 diagnosis](docs/B1_SLOWDOWN_DIAGNOSIS_20260930.md). Earlier comparisons had different NUMA first-touch placement. The full rerun arxiv-10809143 with explicit interleave on the same 16 cores measured BF16 B0 67.98 ms, FP32 B0 73.44 ms, B1 448.67 ms. The arxiv launcher applies the same interleave policy to all paths. Direct binary examples below require an equivalent explicit CPU/memory policy for performance comparisons. Results are exploratory shared-node measurements with random parameters, not formal task/performance acceptance.

Historical reconciliation: [same-graph comparison](docs/HISTORY_COMPARISON_20260930.md), history-10809340. Yesterday's local-U TFS still beats the old mostly single-thread sparse Reference, but loses to the matched parallel transform-first control on both official arxiv/products graphs. Current B1 uses a different neighbor-step projection order. Allocation-inclusive and preallocated times are reported separately. On products, current BF16 B0/B1 logits relative L2 errors are 2.54%/2.01% against FP32; their speed is diagnostic until precision/task accuracy is accepted. Current FP32 B0 remains close to FP32 Reference (relative L2 9.43e-7).

Authority: [IMPLEMENTATION_CONTRACT.md](IMPLEMENTATION_CONTRACT.md), supplied on 2026-09-30. This new baseline suite uses exact-max prescan + a second CSR pass. It supersedes the earlier online-only constraint for this suite; the older `implementation/` experiments remain separate.

## Paths

- `ref_fp32` (R0): SGEMM, explicit Z/score/p/alpha, scalar stable softmax and aggregation. Correctness oracle, excluded from primary speedup claims.
- `standard_bf16` (B0): one all-head oneMKL BF16 GEMM producing FP32 Z, AVX-512 fused L/R dots, exact-max prescan, multithreaded fused exp/denominator/weighted-Z aggregation. No TFS, global score, or alpha.
- `standard_fp32`: auxiliary B0 FP32 projection with the same optimized sparse path.
- `tfs_bf16` (B1): original TFS v3 degree-sorted ascending permutation, dynamic R panels, 16 destination rows, smart inactive-row zeroing, next-neighbor prefetch, independent heads and persistent OpenMP region. Each neighbor step supplies `RNE_BF16(p * FP32(X_bf16))` to AMX. All head output columns remain in TMM across neighbors. No local U+SGEMM, full Z, or global p/alpha.
- `matched_attention`: B1 reordered attention frontend with B0 projection/aggregation; diagnostic only.

The AMX output dimensions are currently supported for 1 <= d <= 64, 1 <= heads <= 8. Input D may have a tail and is padded to a multiple of 32. Output is padded to a multiple of 16. Graph dimensions/classes come from the graph header (arxiv C=40, products C=47), not a hardcoded dataset shape. The frozen model is Din -> 8x32 -> 8x32 -> 1xC; no dropout/residual/bias or training is added.

## Precision and graph contracts

FP32 master input, W, aL/aR are retained. X and W use BF16 RNE. B1 static bL/bR use master W/a and are optionally BF16-quantized for the LR GEMM. `--lr-policy fp32` is the FP32 contraction control; `avx` is an alternative FP32 dot frontend. `bf16` is the initial candidate, subject to microbenchmarks and accuracy review. B0 L/R naturally derives from BF16 projection. The LR policies and B1 additional pX quantization must be disclosed with each comparison.

All L/R, scores, max, exp, denominator and outputs are FP32. R0 graph ingestion is reused as a standalone validated GATBIN1 reader: destination-row CSR, uint64 rowptr, uint32 source indices. No graph mutation, deduplication or loop insertion occurs in this suite. Existing graph preparation policy must be documented in the dataset manifest. Empty rows yield zero denominator/output. Degree sorting only changes scheduling; output node order stays original.

`GATWGT1` checkpoint format: 8-byte magic, uint32 layer count (3), then per-layer uint32 D/K/d, FP32 row-major W[D,Kd], aL[Kd], aR[Kd]. Imported framework checkpoints must be exported with the same orientation. Synthetic weights require explicit `--seed`; logs always identify them as untrained fixtures.

## Build and execution

Server root: `/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline`. Build with `bash scripts/preflight_build.sh` after reading the current workspace rules and research skill. All build/temp/run files stay under this root.

```bash
./build/gat_baseline --graph ../data/smoke1024.gatbin --seed 11 \
  --mode correctness --threads 2
./build/gat_baseline --graph ../data/arxiv.gatbin --weights checkpoint.bin \
  --path standard_bf16 --mode benchmark --threads 16 --warmups 2 --repeats 7
./build/gat_baseline --graph ../data/arxiv.gatbin --weights checkpoint.bin \
  --path tfs_bf16 --lr-policy bf16 --mode benchmark --threads 16 --warmups 2 --repeats 7
./build/gat_baseline_profile --graph ../data/smoke1024.gatbin --seed 11 \
  --path tfs_bf16 --mode profile --threads 2 --warmups 1 --repeats 2
```

`--layer 1|2|3` measures one layer with its input generated by the R0 chain outside timing. `--layer 0` (default) passes each path's own outputs through all three layers. All workspaces are allocated before timing. Activation and input conversion are included in layer/model timing; output inspection and printing follow the model stopwatch. Steady-state excludes graph load, static degree sort, static W/B preparation and workspace allocation; these are reported separately along with first-forward time.

## Fixed-p and microbenchmarks

```bash
./build/gat_baseline --graph graph.gatbin --weights checkpoint.bin \
  --mode export-fixture --layer 2 --fixture layer2.fixture --threads 16
./build/gat_baseline --graph graph.gatbin --weights checkpoint.bin \
  --mode fixed-p --layer 2 --fixture layer2.fixture --threads 16 --warmups 2 --repeats 7
MKL_VERBOSE=1 ./build/gat_baseline --graph graph.gatbin --weights checkpoint.bin \
  --mode micro --layer 2 --threads 16 --warmups 2 --repeats 7
```

Fixed-p verifies fixture CSR and exact checkpoint identity, p in [0,1], denominator and all finite values. Both operators receive identical p/ell. Shared input conversion, attention frontend and p generation are excluded. A0 includes BF16 dense projection + AVX sparse aggregation + normalization. A1 includes weighted AMX TFS + normalization. AB/BA order alternates. The explicit pX scalar emulator runs only on small inputs; boundary tests exercise it across all supported shapes.

Micro mode covers conversion, BF16 projection, fused L/R vs skinny SGEMM, reordered LR BF16/FP32/AVX, max prescan, and score/exp/SpMM. Use representative node counts and the same thread/socket policy before freezing frontend choices. A tiny graph is insufficient to select a production LR policy.

## Correctness and evidence

Correctness mode compares Z, L/R, row max, denominator, layer output and path-specific E2E; prints max abs, mean abs, relative L2, RMSE and P50/P90/P99. It separates FP32 master, quantized-X/W FP32 reference, projection accumulation differences, FP32 attention reorder and BF16 full paths. Fixed-p checks master outputs, B0 vs B1 and BF16 weighted-edge emulation. NaN/Inf and size mismatches fail.

BF16 task acceptance is intentionally UNCONFIGURED until calibrated on a real checkpoint/development split. Optional `--max-abs` AND `--rel-l2` apply an explicitly supplied numerical gate, not a task accuracy gate. An exit status 0 without these flags means finite/structural checks ran; it does not certify BF16 task accuracy. The boundary tests' FP32 oracle/reduction tolerances are numerical implementation gates only.

`tests/boundaries.cpp` tests 64 D/K/d combinations, degrees 0/1/2/15/16/17/31/32/33/63/64/65/129, partial row tiles, tails, five score scenarios, BF16 ties, malformed CSR/parameters, overflow, NaN and Inf. `tests/oracle.py` independently checks all three R0 layers and per-edge values against NumPy FP64.

JSON layer logs expose wall stage times and profile-only nested worker sums. Profile worker fields sum elapsed time across workers and overlap the aggregate wall field; they must not be added together or treated as model latency. Speed binary compiles those timers/counters out. Performance reported from shared-node smoke runs is diagnostic only.

Profiles separate gather loads, pX expansion/multiplication/RNE stores, tile loads, AMX compute, tile stores, scheduling and output scatter. B0 profiles separate score/exp/denominator from weighted-Z loads/FMA. Subfields include timer overhead and overlap their parent fields. The profile binary is intentionally not used for speedups. Only TFS uses the panel_R/tile_rows metadata; early logs print these generic fields for other paths as well, where they do not indicate TFS execution.

Model stdout can be exported with `--output logits.f32` after timing. `scripts/evaluate_task.py` computes classification accuracy/agreement on an explicit labels/split file and supports a calibrated `--max-accuracy-drop` gate against R0 outputs. No training checkpoint or task threshold is invented. `scripts/summarize.py runs/arxiv-JOBID` computes stage medians from the original JSON logs (case-sensitive D/d keys).

Custom TFS AMX requires successful per-worker OS permission and direct TDPBF16PS execution. oneMKL BF16 API use is recorded separately: CPU flags and MKL_VERBOSE must be saved; vendor AMX dispatch remains UNVERIFIED unless corroborated by runtime/instruction evidence. Do not infer AMX execution from API name alone. Formal comparisons need fixed single-socket physical cores, affinity/NUMA evidence, calibrated accuracy and repeated median/P95 measurements.

Recorded evidence in arxiv-10808152 and arxiv-10808269: a standard_bf16-only process retired 27,571,749,888 AMX BF16 operations under perf. The custom TFS kernel was never called in that process, establishing that oneMKL projection executes AMX on the measured Xeon Max platform. Recheck dispatch on another CPU/library version. Actual CPU affinity must also be checked: Slurm batch CPUs do not automatically guarantee the srun step gets the same quota.
