from pathlib import Path
root=Path('/home/huangjianqiang_group/hdacp1/data/wzh')
(root/'TFS-Train/docs/handoff/ALL_EXPERIMENTS.md').open('a').write(r'''

## 2026-10-08 source-order parallel fusion build correction

- `recompute-window-parallel-build-20261008-211935` failed during compilation because the new identity/min-source order helper used `std::iota` without including `<numeric>`. Evidence is preserved at `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-parallel-build-20261008-211935/`.
''')
(root/'TFS-Train/docs/handoff/EXPERIMENT_ISSUES.md').open('a').write(r'''

## 2026-10-08 source-order helper missing include

- The parallel source-order extension initially failed to compile because `<numeric>` was missing for `std::iota`. The fix is isolated to the shadow source and requires a rebuild before runtime testing.
''')
(root/'TFS-Train/docs/handoff/PROJECT_PROGRESS.md').open('a').write(r'''

### 2026-10-08 source-order fusion build correction

- The first build of the identity/min-source schedule extension failed on a missing standard header. The source is corrected and will be rebuilt before any job submission.
''')
