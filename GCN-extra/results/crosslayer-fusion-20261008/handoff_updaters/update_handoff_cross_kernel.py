from pathlib import Path
root=Path('/home/huangjianqiang_group/hdacp1/data/wzh')
(root/'TFS-Train/docs/handoff/ALL_EXPERIMENTS.md').open('a').write(r'''

## 2026-10-08 cross-graph source-aware fusion kernel gate (Job 10901444)

- Experiment ID: `recompute_window_parallel_cross_20261008`; partition `intel`, node `qhcn140`, one task/32 CPUs, 16G, elapsed 00:03:46, exit 0:0. Amazon and Reddit each ran DegreeSort, identity and min-source orders with three repetitions and exact BF16 gates.
- Amazon candidate/materialized ratios were 1.120x (DegreeSort), 1.260x (identity), and 1.456x (min-source), all exact. The baseline samples were highly variable (for example min-source 493.9--584.7 ms), so these shared-node ratios are diagnostic only and require exclusive-node confirmation.
- Reddit candidate/materialized ratios were 0.0991x (DegreeSort), 0.0994x (identity), and 0.0860x (min-source), all exact. Candidate times were 13.3--16.0 s versus 1.14--1.60 s materialized; this is a clear negative control consistent with extreme source duplication.
- Evidence: `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-parallel-cross-10901444/`; paper-ineligible.
''')
(root/'TFS-Train/docs/handoff/EXPERIMENT_ISSUES.md').open('a').write(r'''

## 2026-10-08 cross-graph kernel timing variability

- Amazon shared-node baseline timings varied substantially across repetitions, while candidate timings were more stable. This prevents using the apparent 1.12--1.46x ratios as formal evidence.
- Reddit is stable enough to serve as a negative control: all source orders are about an order of magnitude slower under the fused per-thread implementation.
- Resolution: repeat RoadNet-CA and Amazon on `intel_expr --exclusive` with the same data and order matrix before interpreting the positive boundary.
''')
(root/'TFS-Train/docs/handoff/PROJECT_PROGRESS.md').open('a').write(r'''

### 2026-10-08 cross-graph kernel boundary

- Reddit confirms the source-duplication failure mode: 20x structural duplication produces roughly 10x runtime slowdown despite exact output.
- Amazon appears positive in shared measurements but has large baseline variance; it is not accepted until an exclusive-node rerun. RoadNet min-source remains the leading candidate.
- Exact next task: exclusive-node repeated matrix for RoadNet-CA and Amazon, with the same-order materialized and fused paths and fixed 32-thread affinity.
''')
