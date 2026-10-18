# C4 same-plan AVX sparse-aggregation pilot, v1

2026-10-18, Job `10996305`, shared `intel` node `qhcn053`, 16 CPUs, 32 GiB request, `COMPLETED 0:0`, 00:01:47. This isolated program performs one BF16-input/FP32-output sparse aggregation. It does not include AMX, the two-layer epilogue, or the canonical FULL kernel. One warmup and three measured repetitions per method, fixed method order, output zeroing outside each timer. No NUMA interleave.

| Graph | Ideal additions saved | Plan bytes | Plan build | Rowwise AVX | Same-plan fanout | Same-plan shared mask |
|---|---:|---:|---:|---:|---:|---:|
| amazon0601 | 9.45% | 19.8 MB | 0.16 s | 8.51 ms | 9.64 ms | 9.97 ms |
| roadNet-CA | 0.79% | 34.2 MB | 0.25 s | 26.49 ms | 37.27 ms | 37.37 ms |
| hollywood-2009 | 43.57% | 408.4 MB | 4.47 s | 392.8 ms | 456.1 ms | 406.78 ms |
| mycielskian19 | 67.11% | 2.355 GB | 30.25 s | 4735.95 ms | 3497.1 ms | 1623.89 ms |

Times are arithmetic means of the three raw repetitions for readability; raw arrays are in `runs/consumer_mask_c4_kernel_20261018-01/summary.json`. Shared-mask beats the exact same precomputed plan's per-source fanout by ~2.15× on Mycielskian19 and ~1.12× on Hollywood. Against the standalone rowwise AVX reference it is ~2.92× faster on Mycielskian19, but slower on the other three. The constructed Mycielskian case therefore identifies a real execution opportunity, **not** a robust journal result. Its 30.25 s build would require roughly ten repeated sparse passes to amortize even under these pilot numbers, ignoring storage and model-level costs.

All four inputs were SHA-256 verified against the frozen inventory; the plan entry counts and algebraic addition savings matched the independent 17-graph scan exactly. Directed high-overlap, no-overlap, tile-boundary and duplicate-rejection fixture checks passed. The first source compilation failed on a container-to-pointer API mismatch; corrected source compiled and all smoke checks passed before submitting. See `BUILD_ISSUE.md` in the run directory.

**Numerical gate limitation discovered after completion:** the generated BF16 values came from small integer multiples of `1/1024`. On these four graphs, reordered FP32 aggregation matched rowwise exactly, including zero BF16-output mismatches. This input distribution fails to stress FP32 addition-order and BF16 threshold sensitivity. The code's 1e-2 gate passed, but cannot be taken as meaningful numerical validation. A new source/binary/run snapshot with broad-mantissa random BF16 inputs is required; do not edit or reinterpret this frozen v1 run.

Shared-node timing order was fixed, not randomized, and sparse aggregation is only one part of FULL+AMX. No paper-eligible speedup or completed precision claim follows. V2 must remeasure numerical error and timing with more realistic random BF16 inputs; subsequent work must beat canonical two-layer FULL Identity while including conversion, AMX, plan build/amortization, and correct activation boundary.
