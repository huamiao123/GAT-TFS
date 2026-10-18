# C4 shared-mask AVX aggregation versus register-resident rowwise control

2026-10-18 Job `10996710`, `intel_expr --exclusive` node `qhcn818`, 32 workers all on physical CPUs `0–31`, Slurm `COMPLETED 0:0`, elapsed 00:03:49. BF16 input / FP32 sparse aggregation, D=128, original Identity TR16 unit-weight CSR, one warmup and six interleaved paired rounds; default NUMA, no interleave. **This is not a two-layer AMX result.**

The new rowwise control mirrors the sparse part of canonical FULL: each row keeps eight `__m512` FP32 accumulators, prefetches four cache lines of the next source row, and stores the aggregate once. Same-plan fanout and shared-mask kernels are byte-identical in algorithm to the previous exclusive probe; the source and binary are separately snapshotted here. Assembly audit (`rowwise_asm.txt`) confirms eight `vaddps` ZMM accumulator updates and four `prefetcht0` instructions per next source without loop-time accumulator spills.

| Graph | Register rowwise median | Same-plan fanout | Shared mask | Register/shared | Fanout/shared |
|---|---:|---:|---:|---:|---:|
| indochina-2004 | 714.88 ms | 724.41 ms | 471.19 ms | 1.517× | 1.537× |
| mycielskian19 (constructed) | 3386.28 ms | 2899.99 ms | 1404.96 ms | 2.410× | 2.064× |
| hollywood-2009 | 1488.74 ms | 1577.98 ms | 1500.55 ms | 0.992× | 1.052× |
| as-Skitter | 135.27 ms | 187.92 ms | 171.54 ms | 0.789× | 1.095× |
| reddit | 216.66 ms | 190.65 ms | 188.83 ms | 1.147× | 1.010× |
| roadNet-CA | 58.46 ms | 76.56 ms | 77.28 ms | 0.756× | 0.991× |

Indochina is now a **real-graph sparse-stage candidate** that survives a source- and assembly-audited eight-register prefetching rowwise control. Myciel is a stronger but constructed case. Reddit's apparent win remains dominated by plan/source traversal: shared beats same-plan fanout by only ~1%. Hollywood shared beats same-plan fanout by ~5% here but does not beat register rowwise. These are not general-graph gains. Output numerical gates passed: relative L2 against register rowwise ≤2.12e-6 across six graphs; BF16 aggregate mismatches are 1/552/49/1/51/0 in table order. Inputs and plan counts matched frozen 17-graph scan. Actual plan build for Indochina ~2.44 s and Myciel ~29.73 s, versus sparse-pass reductions of ~0.244 s and ~1.981 s, respectively; reuse on the order of 10–15 passes would be needed to amortize this particular build even before full-model costs.

**Remaining strong-baseline concern:** the new register control deliberately copied canonical next-row prefetch. Prefetch can be graph dependent; the current data do not show whether a register-resident *no-prefetch* rowwise implementation is faster. Compare both in one job and use their faster measured result as the conservative sparse-stage reference. Also, Hollywood's absolute times differ greatly from an earlier exclusive run even though the shared algorithm was nominally unchanged; node/run context or graph order may be responsible, so no cross-job ratio is used. Same-job six-repetition arrays remain low spread. Assembly confirms the intended register behavior, not a universal optimality claim.

Source, binary, compiler, protocol and instruction hashes, input hashes, exact commands, raw six times/errors, worker CPU placement, `rowwise_asm.txt`, Slurm stdout/stderr and accounting are under `runs/consumer_mask_c4_register_control_20261018-01/`. Paper eligibility false. Next gate: no-prefetch register ablation; later exact canonical FULL Identity plus AMX two-layer integration and plan amortization.
