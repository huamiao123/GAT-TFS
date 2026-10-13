from pathlib import Path
root=Path('/home/huangjianqiang_group/hdacp1/data/wzh')
(root/'TFS-Train/docs/handoff/ALL_EXPERIMENTS.md').open('a').write(r'''

## 2026-10-08 source-order parallel fusion build success

- `recompute-window-parallel-build-20261008-212208` rebuilt the identity/min-source schedule extension successfully after adding `<numeric>`. Evidence: `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-parallel-build-20261008-212208/`.
''')
(root/'TFS-Train/docs/handoff/EXPERIMENT_ISSUES.md').open('a').write(r'''

## 2026-10-08 source-order build correction: resolved

- The `<numeric>` include repair compiled successfully; no runtime conclusion is attached to the build artifact.
''')
(root/'TFS-Train/docs/handoff/PROJECT_PROGRESS.md').open('a').write(r'''

### 2026-10-08 source-order schedule gate ready

- The parallel fusion binary now includes DegreeSort, identity, and min-source destination orders. The next task is the 32-thread RoadNet-CA exact/timing gate for all three orders.
''')
