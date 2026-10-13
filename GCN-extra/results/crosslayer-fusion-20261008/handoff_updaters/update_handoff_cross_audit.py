from pathlib import Path
root=Path('/home/huangjianqiang_group/hdacp1/data/wzh')
(root/'TFS-Train/docs/handoff/ALL_EXPERIMENTS.md').open('a').write(r'''

## 2026-10-08 cross-graph source-overlap partition audit (Job 10901392)

- Experiment ID: `source_partition_cross_graph_20261008`; partition `intel`, node `qhcn006`, one task/one CPU, 16G, elapsed 00:00:09, exit 0:0. Exact analyzer ran on Amazon and Reddit under the default NUMA policy.
- Amazon duplication across 32 partitions: DegreeSort `4.4516x`, identity `4.2590x`, min-source `3.3726x`, max-source `4.0740x`.
- Reddit duplication: DegreeSort `21.6152x`, identity `28.0434x`, min-source `20.2700x`, max-source `20.4671x`.
- Evidence: `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-partition-cross-10901392/`. Structural only, paper-ineligible.
- Decision: RoadNet-CA is a low-duplication special case; Amazon and Reddit are negative structural candidates. A kernel run on them is retained only as a prediction/selection-boundary test.
''')
(root/'TFS-Train/docs/handoff/EXPERIMENT_ISSUES.md').open('a').write(r'''

## 2026-10-08 cross-graph source-overlap boundary

- Source-overlap-aware ordering does not generally produce low duplication: Amazon remains 3.37x and Reddit 20.27x even under the best tested simple order.
- This is a method boundary rather than an implementation failure. Any runtime policy must use an estimated source-duplication/cut metric and fall back to materialized FULL when the estimate is high.
- Validation status: structural boundary confirmed; kernel negative controls pending.
''')
(root/'TFS-Train/docs/handoff/PROJECT_PROGRESS.md').open('a').write(r'''

### 2026-10-08 cross-graph applicability boundary

- The source-aware fusion candidate is promising only when destination partitions have low source-set duplication. RoadNet-CA is 1.04x under min-source; Amazon is 3.37x and Reddit 20.27x.
- The method is now a conditional dataflow policy: estimate source duplication first, use source-aware fusion only below a threshold, and retain materialized FULL otherwise. Amazon/Reddit kernel runs will test this predictor.
''')
