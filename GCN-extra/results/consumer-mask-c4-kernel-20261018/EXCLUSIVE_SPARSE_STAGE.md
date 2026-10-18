# C4 consumer-mask AVX aggregation: exclusive 32-core sparse-stage replication

2026-10-18 Job `10996654`, `intel_expr --exclusive`, node `qhcn818`, Slurm `COMPLETED 0:0`, 00:02:14. One 32-core OpenMP process, all recorded worker CPU IDs exactly `0–31` on one socket of Intel Xeon Max 9462. Default NUMA policy, **no interleave**. Original unit-weight CSR, Identity TR16 plan, D=128, deterministic random BF16 input, FP32 aggregate output. One warmup per method and six interleaved measurements with all six method orders; output zeroing outside timers. This is an isolated sparse stage; AMX and two-layer GCN are absent.

| Graph | Rowwise AVX median | Same-plan fanout median | Same-plan shared median | Rowwise/shared | Fanout/shared | Ideal additions saved |
|---|---:|---:|---:|---:|---:|---:|
| amazon0601 | 15.159 ms | 18.354 ms | 18.958 ms | 0.800× | 0.968× | 9.45% |
| roadNet-CA | 57.997 ms | 76.591 ms | 77.761 ms | 0.746× | 0.985× | 0.79% |
| hollywood-2009 | 457.534 ms | 538.831 ms | 489.412 ms | 0.935× | 1.101× | 43.57% |
| mycielskian19 | 3976.985 ms | 2884.335 ms | 1403.980 ms | 2.833× | 2.054× | 67.11% |

The six raw timings per method are in `runs/consumer_mask_c4_exclusive_20261018-01/summary.json` and per-graph raw JSON. On Mycielskian19, all six shared times lie around 1.404 s while all six fanout times lie around 2.88–2.90 s and rowwise around 3.91–4.02 s. The same-plan comparison therefore confirms a large effect from shared aggregation on this **constructed** graph. Hollywood also confirms an approximately 10% same-plan gain, but its plan overhead and altered traversal leave it approximately 7% slower than rowwise. Amazon and RoadNet are negative controls. Savings in ideal additions are not sufficient by themselves to predict wall time.

All 17-graph inventory SHA-256 inputs in this four-graph subset, stage-one plan source count and addition savings matched; five directed/duplicate/rounding smoke gates passed. Shared relative L2 errors versus rowwise are `1.54e-10` (Amazon), `0` (RoadNet), `1.13e-7` (Hollywood), `2.11e-6` (Myciel); BF16 aggregate mismatches are `0,0,49,552` respectively. All probe-specific 1e-2 checks passed. These are not the canonical two-layer error checks. The actual plan for Myciel remains ~2.355 GB and takes ~30.33 s to build; its sparse-pass saving versus rowwise is ~2.57 s, implying at least roughly 12 repeated sparse passes to repay *this* build cost if the same structural plan is reusable. Memory capacity and full-model cost are additional constraints.

This exclusive replication resolves the shared-node CPU placement concern for **this sparse-stage comparison**. It does not validate the 2-layer AMX runtime. The rowwise baseline is an independently implemented AVX reference, not the canonical fused FULL Identity. Performance is paper-ineligible. The next discriminating test is the preregistered Identity-mask opportunity on real Indochina-2004 (77.15%), plus medium-overlap real graphs. A real-graph win and later canonical AMX integration are necessary before treating C4 as a journal candidate.

Evidence: protocol/source/binary SHA-256 and preflight snapshots in `runs/consumer_mask_c4_exclusive_20261018-01/`, exact per-graph command files, `summary.json`, `lscpu.txt`, `numa_policy.txt`, `sacct.txt`, `slurm-10996654.*` and `results.sha256`.
