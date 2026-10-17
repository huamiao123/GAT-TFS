# Original TFS V3 H-panel reuse: first experiment (2026-10-14)

## Status and provenance

- Source: `../../experiments/h_panel_reuse_20261014/h_panel_reuse.cpp`; frozen
  `original_gcn_e2e_v3.cpp` SHA-256
  `85c51bde92792f98223e80107e20b042f8f8d0e40068cdf6ce168143f0c7743f`.
- Final frozen build on server:
  `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/h-panel-reuse-build-20261014-231751`.
  Its binary SHA-256 is
  `eda207b215fa947cc4e314d1b9e77e90cf6bf573d8e14e2ac5f3b1ec5b0f65d7`,
  identical to the binary used for the smoke run.
- Smoke Slurm JobID `10964897`, `intel` shared node `qhcn018`, 4 threads,
  COMPLETED after 5 seconds. Run artifacts:
  `h-panel-reuse-10964897/`; mirrored from the server path of the same name.
- Both 37- and 137-node fixtures passed **all 7 bitwise checks** at L1 FP32,
  interlayer BF16, and L2 BF16. They exercise zero/short/long degrees and
  partial 16-row tiles. Correctness was checked before timings.
- Initial H conversion and W packing were outside timing. Prepared two-layer
  forward including L1, ReLU, interlayer conversion, L2 was timed five times,
  with forward/reverse method order. All times below are shared-node smoke
  measurements, not paper or real-graph performance evidence.

## Smoke timing, median of five, milliseconds

| Method | 37-node fixture | 137-node fixture |
|---|---:|---:|
| A0 original | 0.1130 | 0.1299 |
| A1 per-step H reuse | 0.1489 | 0.1521 |
| A2 window 8 | 0.1719 | 0.1541 |
| A3 window 16 | 0.1577 | 0.1421 |
| A4 window 32 | 0.1702 | 0.1321 |
| A5 window 64 | 0.1440 | 0.1370 |
| window 16 duplicate gather control | 0.2031 | 0.1909 |

These timings are dominated by tiny graph, OpenMP and kernel-entry overhead.
They cannot answer whether H reuse beats C staging on real graphs. Raw
per-repeat layer and E2E timing, min/max, checks, work ledgers, node and NUMA
state, and input hashes are under `h-panel-reuse-10964897/`.

## Mechanism and pending measurements

The work ledger confirms the intended logical operation counts on the 37-node
fixture: A0 gathers 2004 source vectors per layer (2×1002 real neighbors),
the shared-H candidates gather 1002, and every method issues 6464
`TDPBF16PS` operations. A1 adds 1592 C tile loads and stores each because it
stages after every neighbor step. Window 16 greatly reduces these requests.
These are software counts, **not measured cache misses or DRAM bytes**.

The four real graphs and PMU/stage diagnosis remain pending. The current
account queue occupied nine distinct compute nodes when checked after the
smoke. The existing maximum of five nodes therefore prevented another job
submission. The smoke was launched before that distinct-node count was
recognized; subsequent submissions are held until the account is within the
agreed bound. No other jobs were modified.

Next diagnostic launch order: `diagnostic1` (roadNet-CA, reddit), then
`diagnostic2` (ogbn-products, mycielskian19), each on one shared `intel` node
with 32 physical threads and all seven paths interleaved in the same process.
The scripts are frozen with the final build. Results remain exploratory until
exclusive-node repeats and PMU measurements are available.
