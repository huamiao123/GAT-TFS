# C4 same-plan AVX sparse aggregation: numerical-stress rerun

2026-10-18, Job `10996636`, shared `intel` node `qhcn085`, 16 CPUs, 32 GiB requested, `COMPLETED 0:0`, 00:03:43. Source, binary and run differ from v1 only in BF16 input generation and evidence logging. Synthetic FP32 values are deterministically drawn from 24-bit `[0,1)` values and truncated to BF16. This remains a single sparse aggregation stage, without AMX projection, ReLU, interlayer conversion or two-layer end-to-end timing. Default NUMA policy; **no NUMA interleave**.

| Graph | Rowwise AVX | Same-plan fanout | Same-plan shared mask | Shared / rowwise speedup | Shared / fanout speedup | Shared relative L2 | BF16 mismatches |
|---|---:|---:|---:|---:|---:|---:|---:|
| amazon0601 | 88.0 ms | 92.1 ms | 95.9 ms | 0.918× | 0.961× | 1.54e-10 | 0 |
| roadNet-CA | 59.4 ms | 61.3 ms | 61.8 ms | 0.962× | 0.992× | 0 | 0 |
| hollywood-2009 | 2861.6 ms | 2329.5 ms | 2194.1 ms | 1.304× | 1.062× | 1.13e-7 | 49 |
| mycielskian19 | 14550.3 ms | 6330.3 ms | 4313.2 ms | 3.373× | 1.468× | 2.11e-6 | 552 |

The table shows means of three measurements after one warmup. Output zeroing is outside every sparse-kernel timer. All raw repetitions, input SHA-256 hashes, plan counts and outputs are in `runs/consumer_mask_c4_kernel_v2_20261018-01/summary.json` and `raw/`. The plan is **the same** for fanout and shared; thus their within-job contrast identifies benefit from sharing after plan construction. Rowwise is an independently implemented AVX sparse reference; comparison to it also includes plan layout, source reuse and traversal differences. BF16 mismatch counts compare the FP32 aggregates truncated to BF16; they are not two-layer output errors.

Numerical stress is now nontrivial: an interleaved high-degree fixture produced nonzero relative L2 error before real runs; Hollywood and Myciel also showed nonzero FP32 and BF16 differences. Shared max normalized error was 1.43e-6 (Hollywood) and 2.21e-5 (Myciel), both under the probe-specific 1e-2 gate. Myciel maximum absolute FP32 difference was 2.17188 because aggregates can be large; its relative measures are small. The canonical two-layer paper gate remains untested.

Actual plan storage/build were unchanged in principle: Myciel 2.355 GB and ~30.24 s, Hollywood 408.4 MB and ~4.47 s, Amazon 19.8 MB/~0.16 s, RoadNet 34.2 MB/~0.26 s. The candidate therefore has a material memory and preprocessing bill. The input SHA-256 matched the frozen inventory; plan entries and algebraic saved additions exactly matched the independent 17-graph structural scan. All five smoke checks passed, including duplicate rejection and the new nonzero-rounding fixture. `sacct.txt` records successful Slurm accounting.

**Do not compare v1 and v2 absolute times as a feature-value effect.** V1 Job10996305 ran on `qhcn053` with allowed CPUs `32-47`; v2 ran on `qhcn085` with allowed CPUs `0-7,56-63`. Both are Xeon Max 9462 under default NUMA policy, but the v2 16 CPUs span two sockets and the jobs were shared. The different affinity/layout, node state and data values are confounded. Within-job comparisons are the only provisional timing evidence. Fixed-order measurement also limits timing inference.

Conclusion: same-mask aggregation has a real execution signal on the constructed Mycielskian case and a smaller one on Hollywood, with numerical behavior visible and within this probe's tolerance. It regresses or ties on two lower-opportunity graphs. This is **not yet an AVX–AMX or journal breakthrough**. Next gates: exclusive one-socket 32-core paired sparse-stage rerun; then integration with canonical FULL Identity and two-layer AMX, exact epilogue/precision control, plan-build amortization, and broader real graph coverage. Paper eligibility remains false.
