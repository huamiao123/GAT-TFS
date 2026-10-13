from pathlib import Path
root=Path('/home/huangjianqiang_group/hdacp1/data/wzh')
(root/'TFS-Train/docs/handoff/ALL_EXPERIMENTS.md').open('a').write(r'''

## 2026-10-08 32-thread consumer-driven fusion prototype (Job 10900662)

- Experiment ID: `recompute_window_parallel_20261008`; partition `intel`, node `qhcn003`, one task/32 CPUs, 16G, elapsed 00:00:08, exit 0:0. Default NUMA policy was used; no interleave or feature/graph reorder.
- RoadNet-CA, 128 -> 128 -> 128, three measured repetitions after one warmup, exact BF16 output equality passed. Materialized FULL median: `96.892118454 ms`; per-thread W=0 consumer-driven fusion median: `113.595008850 ms`; ratio `0.852961054`.
- Candidate generated `3,627,178` source rows, used `273,630` source projection groups, `123,206` second-layer projection groups, and reached a per-thread peak cache of `173,160` rows. The source-row count is about 1.85x the 1,965,206 used source rows in the materialized path, due to cross-partition duplication.
- Decision: exact dataflow PASS; simple per-thread cache partition is performance-rejected at 32 threads and remains paper-ineligible. Evidence: `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-parallel-10900662/`.
''')
(root/'TFS-Train/docs/handoff/EXPERIMENT_ISSUES.md').open('a').write(r'''

## 2026-10-08 parallel W=0 fusion loses to source duplication (Job 10900662)

- Symptom: the single-thread W=0 fusion signal did not survive 32-thread static destination partitioning; candidate median was 17.2% slower than materialized FULL.
- Evidence: candidate generated 3,627,178 source rows versus 1,965,206 used source rows in the same graph; final BF16 checksum matched exactly.
- Root cause: independent per-thread source caches recompute H1 rows appearing in multiple destination partitions. The dataflow has no shared producer ownership or source-overlap-aware partitioning.
- Resolution: reject this simple parallelization. Any continuation must measure source-cut duplication before kernel timing and either assign source ownership or partition destinations by source overlap.
- Validation status: negative result confirmed by three measured repetitions.
''')
(root/'TFS-Train/docs/handoff/PROJECT_PROGRESS.md').open('a').write(r'''

### 2026-10-08 parallel fusion boundary

- The 32-thread isolated W=0 prototype is numerically exact but 17.2% slower than materialized FULL because per-thread destination partitions duplicate H1 source generation by about 1.85x.
- This closes the naive AVX/AMX fusion implementation. The remaining research question is source-overlap-aware parallel ownership: can destination work be partitioned so source H1 rows are produced once or with bounded duplication while preserving AVX aggregation and AMX projection locality?
- Exact next task: run a source-cut analyzer over RoadNet-CA for DegreeSort, original-row, and source-overlap-aware destination partitions at 32-way parallelism. Do not implement another kernel until the source duplication bound is quantified.
''')
