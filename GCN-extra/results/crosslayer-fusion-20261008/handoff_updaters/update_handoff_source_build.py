from pathlib import Path
root=Path('/home/huangjianqiang_group/hdacp1/data/wzh')
(root/'TFS-Train/docs/handoff/ALL_EXPERIMENTS.md').open('a').write(r'''

## 2026-10-08 source-overlap partition analyzer build

- Experiment ID: `source_partition_build_20261008`; isolated C++17 `-O3` analyzer build completed successfully. Evidence: `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-partition-build-20261008-210228/`.
- The analyzer compares DegreeSort, identity, degree-descending, and source-neighbor locality orderings for exact unique-source counts across 32 equal destination partitions. It is structural evidence only and paper-ineligible.
''')
(root/'TFS-Train/docs/handoff/EXPERIMENT_ISSUES.md').open('a').write(r'''

## 2026-10-08 source-overlap analyzer build: no new major issue

- The standalone structural analyzer compiled successfully with the required preflight and has not modified graph, feature, or canonical kernel sources.
''')
(root/'TFS-Train/docs/handoff/PROJECT_PROGRESS.md').open('a').write(r'''

### 2026-10-08 source-overlap audit ready

- The exact unique-source partition analyzer is built. The next task is its RoadNet-CA run to quantify whether source-aware destination schedules can reduce the 1.85x duplication observed in the 32-thread fusion prototype.
''')
