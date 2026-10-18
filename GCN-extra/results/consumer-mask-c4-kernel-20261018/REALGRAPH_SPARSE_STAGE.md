# C4 shared-mask AVX aggregation: exclusive real-graph follow-up

2026-10-18 Job `10996686`, `intel_expr --exclusive`, node `qhcn818`, Slurm `COMPLETED 0:0`, 00:01:51. Thirty-two recorded OpenMP worker IDs are CPU `0–31` on a single Xeon Max 9462 socket; default NUMA, no interleave. Original unit-weight CSR, Identity TR16, D=128 BF16 input, FP32 aggregate, one warmup and six balanced-order measured rounds. These are isolated sparse-stage times; no AMX or two-layer path.

| Real graph | Ideal additions saved | Actual plan | Build | Rowwise AVX median | Same-plan fanout | Same-plan shared | Fanout/shared |
|---|---:|---:|---:|---:|---:|---:|---:|
| indochina-2004 | 77.15% | 229.6 MB | 2.43 s | 1053.27 ms | 789.74 ms | 494.53 ms | 1.597× |
| as-Skitter | 18.02% | 112.2 MB | 2.76 s | 195.61 ms | 228.85 ms | 209.65 ms | 1.092× |
| soc-LiveJournal1 | 9.56% | 431.7 MB | 4.44 s | 597.10 ms | 716.82 ms | 714.34 ms | 1.003× |
| reddit | 4.86% | 868.0 MB | 8.16 s | 266.95 ms | 190.35 ms | 188.98 ms | 1.007× |

Indochina supplies the first strong **real-graph same-plan** evidence: combining sources of equal consumer mask saves about 37% of the fanout time. As-Skitter shows a smaller ~9% same-plan gain that fails to beat the independently implemented rowwise reference. Reddit's rowwise-relative gap is predominantly from the changed source traversal/plan, because shared and fanout times are nearly identical; do not attribute it to mask aggregation. The real graphs therefore have different bottlenecks despite their exact same kernel.

**New strong-baseline issue found during this run's audit:** the standalone rowwise reference calls an AVX add on a destination output vector per CSR edge, thereby reading/writing the output vector repeatedly. The canonical `src/paper_methods_kernels.hpp::reduce_tile` instead keeps eight `__m512` accumulators for each row across its neighbors and stores once. Therefore rowwise-relative speedups in this and earlier pilot reports are **not** speedups over canonical FULL Identity or even a matched register-resident sparse stage. Only the same-plan fanout/shared comparison directly isolates the shared subaggregation in the current implementation. A new register-resident rowwise control is mandatory before integration decisions. This issue does not invalidate the Indochina same-plan 1.597× mechanism result.

All four real inputs matched frozen inventory SHA-256; actual plan source count/addition savings matched the independent structural scan. Five directed/duplicate/rounding smoke cases passed. Shared relative L2 errors versus rowwise were `6.40e-8`, `6.62e-7`, `6.60e-8`, `1.92e-7`, and BF16 aggregate mismatches `1,1,0,51` for Indochina, as-Skitter, LiveJournal1, Reddit; all probe-specific gates passed. This is not two-layer GCN numerical validation. Raw six timing arrays, exact commands, CPU placement, source/binary/preflight hashes, environment, Slurm accounting, stdout/stderr reside at `runs/consumer_mask_c4_realgraphs_20261018-01/`.

Next: test the unchanged shared-mask implementation against a rowwise kernel that mirrors canonical eight-register reduction, within one exclusive job, at least Indochina, Mycielskian, Hollywood, as-Skitter and RoadNet. Then determine whether the value survives the strong baseline. AMX end-to-end work remains conditional. Paper eligibility false.
