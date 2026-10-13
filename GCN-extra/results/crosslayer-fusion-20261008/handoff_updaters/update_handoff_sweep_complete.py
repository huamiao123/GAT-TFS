from pathlib import Path

root=Path('/home/huangjianqiang_group/hdacp1/data/wzh')
(root/'TFS-Train/docs/handoff/ALL_EXPERIMENTS.md').open('a').write(r'''

## 2026-10-08 legal reuse-window sweep (Job 10900360)

- Experiment ID: `recompute_window_sweep_20261008`; partition `intel`, node `qhcn022`, one task/one CPU, 16G, elapsed 00:01:05, exit 0:0. The corrected launcher executed all planned windows `{1,4,16,64,256,512,1024,2048,4096}`; default NUMA policy and no-interleave controls were recorded.
- Every window and its W=0 control passed exact BF16 output equality against materialized FULL FAST. Evidence and raw logs: `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-sweep-10900360/`; machine summary: `summary.json`.
- Across the nine independent invocations, materialized FULL median was 1224.418 ms and W=0 fused control median was 1057.734 ms (exploratory single-thread cross-process bracket). All positive windows were slower than materialized FULL: W=1 1371.631 ms, W=4 1364.580 ms, W=16 1331.199 ms, W=64 1300.692 ms, W=256 1287.704 ms, W=512 1269.396 ms, W=1024 1266.910 ms, W=2048 1253.195 ms, W=4096 1257.085 ms.
- Legal-schedule W=1024 generated 1,687,277 extra source rows and peaked at 2,902 cached rows; W=4096 generated 1,563,643 extra rows and peaked at 9,404 rows. These counts differ from the earlier structural proxy and supersede it for legal scheduling analysis.
- Decision: the memory-saving recompute-window branch is rejected as a performance candidate on RoadNet-CA in this implementation. W=0 cross-layer fusion remains a hypothesis requiring same-process repeated timing and a parallel implementation; it is not paper eligible.
''')
(root/'TFS-Train/docs/handoff/EXPERIMENT_ISSUES.md').open('a').write(r'''

## 2026-10-08 corrected reuse-window sweep (Job 10900360): no new launcher issue

- The `/dev/null` stdin fix and expected-window postcondition were validated: all nine windows ran and all exact gates passed.
- The sweep confirms the earlier legal-schedule mismatch is substantive rather than timing noise: all W>0 candidates pay enough source regeneration to remain slower than materialized FULL. Keep the earlier structural proxy restricted to its stated logical scope.
''')
(root/'TFS-Train/docs/handoff/PROJECT_PROGRESS.md').open('a').write(r'''

### 2026-10-08 legal window frontier closed on RoadNet-CA

- Job 10900360 completed the corrected W sweep. Every W passed bitwise BF16 correctness, but every W>0 was slower than materialized FULL; W=2048 was the best candidate and still about 2.8% slower in its invocation.
- W=0, which avoids recomputation and fuses source generation with second-layer consumption, was consistently about 13.6% faster than materialized FULL in the exploratory single-thread bracket. This is the first real timing signal worth carrying forward, but it is not yet a paper result.
- Current decision: stop treating W as the main breakthrough. Reframe the next experiment around consumer-driven cross-layer fusion with explicit source lifetime, and test whether its W=0 benefit survives same-process repeats and parallel ownership. If it does not, close this branch with a useful negative result.
''')
