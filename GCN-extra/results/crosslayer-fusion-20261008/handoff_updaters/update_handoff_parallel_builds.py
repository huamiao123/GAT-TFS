from pathlib import Path
root=Path('/home/huangjianqiang_group/hdacp1/data/wzh')
(root/'TFS-Train/docs/handoff/ALL_EXPERIMENTS.md').open('a').write(r'''

## 2026-10-08 parallel fusion prototype build attempts

- `recompute-window-parallel-build-20261008-202152` failed during compile. The isolated source initially had a vexing-parse declaration for the per-thread statistics vector and embedded the serial driver's `main`; no binary or performance evidence was produced. Evidence: `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-parallel-build-20261008-202152/`.
- `recompute-window-parallel-build-20261008-202420` completed successfully with oneAPI `icpx -O3 -std=c++17 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel`. It is an isolated shadow build; no paper claim. Evidence: `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-parallel-build-20261008-202420/`.
''')
(root/'TFS-Train/docs/handoff/EXPERIMENT_ISSUES.md').open('a').write(r'''

## 2026-10-08 parallel prototype compile repair

- The first parallel prototype compile failed because the embedded serial source exposed its `main` and `std::vector<ParallelStats> locals(size_t(threads))` was parsed as a function declaration.
- Resolution: guard the serial driver's main with `WINDOW_KERNEL_NO_MAIN` for the isolated include and use an unambiguous vector construction. The corrected build passed; no runtime result was inferred from the failed attempt.
''')
(root/'TFS-Train/docs/handoff/PROJECT_PROGRESS.md').open('a').write(r'''

### 2026-10-08 parallel W=0 fusion build

- The isolated 32-thread consumer-driven fusion prototype now builds successfully. It statically partitions destination rank ranges, uses per-thread H1 source caches, and counts source duplication across partitions.
- The matching RoadNet-CA correctness/timing gate is the next task; the canonical paper-method and original baseline sources remain unchanged.
''')
