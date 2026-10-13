from pathlib import Path
root=Path('/home/huangjianqiang_group/hdacp1/data/wzh')
(root/'TFS-Train/docs/handoff/ALL_EXPERIMENTS.md').open('a').write(r'''

## 2026-10-08 source-overlap-aware parallel fusion orders (Job 10901373)

- Experiment ID: `recompute_window_parallel_order_20261008`; partition `intel`, node `qhcn002`, one task/32 CPUs, 16G, elapsed 00:00:08, exit 0:0. RoadNet-CA, 128 -> 128 -> 128, one warmup and three measured repetitions per order, default NUMA policy, no interleave or feature reorder.
- DegreeSort: materialized FULL median `39.365053177 ms`; per-thread fusion `69.551944733 ms`; ratio `0.565980625`; source rows `3,627,178`.
- Identity order: materialized FULL `31.201124191 ms`; fusion `30.153036118 ms`; ratio `1.034758957`; source rows `2,088,038`.
- Minimum-neighbor-source order: materialized FULL `32.449007034 ms`; fusion `29.790878296 ms`; ratio `1.089226263`; source rows `2,043,040`; per-thread peak cache `66,277` rows.
- All three orders passed exact BF16 equality with checksum `12151273104846194773`. Evidence: `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-parallel-10901373/`; exploratory and paper-ineligible.
- Decision: source-overlap-aware scheduling plus consumer-driven fusion is the first candidate with a positive 32-thread same-order result. It requires cross-graph, shape, repeat, and baseline validation before any paper claim.
''')
(root/'TFS-Train/docs/handoff/EXPERIMENT_ISSUES.md').open('a').write(r'''

## 2026-10-08 source-overlap order changes the fusion result (Job 10901373)

- The same fused kernel is 43.4% slower under DegreeSort, 3.5% faster under identity order, and 8.9% faster under minimum-neighbor-source order relative to same-order materialized FULL controls.
- Evidence: exact output equality holds for all three orders; source duplication tracks the independent structural audit (DegreeSort about 1.79x, identity about 1.06x, min-source about 1.04x).
- Interpretation: destination schedule and cross-layer fusion are coupled. DegreeSort remains the controlled paper baseline, but it is not automatically the best schedule for the fused dataflow.
- Validation status: positive RoadNet-CA candidate only; cross-graph and stronger repeated validation pending.
''')
(root/'TFS-Train/docs/handoff/PROJECT_PROGRESS.md').open('a').write(r'''

### 2026-10-08 first positive source-aware fusion signal

- A 32-thread RoadNet-CA experiment found a positive same-order result for source-overlap-aware destination scheduling: min-source fusion `29.79 ms` versus min-source materialized FULL `32.45 ms` (1.089x), with exact BF16 equality. Identity gives 1.035x; DegreeSort gives 0.566x.
- This is the current main research candidate: jointly choose the destination schedule to minimize cross-thread H1 source duplication, then fuse AVX source aggregation with AMX projection. The schedule preserves original output order through scatter; its effect is execution order and source locality.
- Exact next task: repeat the structural source audit and parallel kernel gate on Amazon and Reddit, then test whether the positive mechanism survives graphs with different degree distributions. No final breakthrough claim yet.
''')
