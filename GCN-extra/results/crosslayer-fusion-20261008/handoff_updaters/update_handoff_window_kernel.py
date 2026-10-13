from pathlib import Path

root = Path('/home/huangjianqiang_group/hdacp1/data/wzh')
all_exp = root / 'TFS-Train/docs/handoff/ALL_EXPERIMENTS.md'
issues = root / 'TFS-Train/docs/handoff/EXPERIMENT_ISSUES.md'
progress = root / 'TFS-Train/docs/handoff/PROJECT_PROGRESS.md'

all_exp.open('a').write(r'''

## 2026-10-08 RoadNet-CA source-aware reuse-window kernel prototype (Job 10900289)

- Experiment ID: `recompute_window_kernel_20261008`; partition `intel`, node `qhcn022`, one task/one CPU, 16G, elapsed 00:00:09, exit 0:0. Default NUMA policy was recorded; no NUMA interleave, feature reorder, first-touch change, or graph reorder was used.
- Isolated shadow source: `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/recompute_window_kernel_20261008/source_snapshot/recompute_window_kernel.cpp`; binary SHA256 `bd8f2db5bdf352828c0ed26cd58d1550f35a85af1a5a337bed8b2ca53c4ae93d`; source SHA256 `47b89c062d088ae0212f5a662b6fea7e49a785dbdcff2f894338cd07d050f703`.
- Scope: RoadNet-CA, N=1,971,281, E=5,533,214, 128 -> 128 -> 128, DegreeSort logical schedule, BF16 H0/H1/output and packed weights, single-thread exploratory timing. The candidate generated H1 source rows on demand, retained them by source ID, reduced the second-layer neighbors with AVX and projected each 16-row destination tile with AMX.
- Exact numerical gate: W=0 and W=1024 final BF16 checksums both equal the materialized FULL FAST control (`12151273104846194773`); `WINDOW_GATE pass=1`.
- Timing/work: materialized FULL FAST `1237.267971 ms`; W=0 fused control `1057.199001 ms`; W=1024 `1267.936945 ms`. W=1024 generated 3,658,558 source rows (1,687,277 beyond the 1,965,206 used source rows), 275,383 source projection groups, 123,206 second-layer projection groups, and peak cache 2,902 rows. The W=1024 timing is 2.48% slower than the materialized control; this is exploratory, single-thread, and not paper eligible.
- Decision: numerical/dataflow feasibility PASS; performance promotion REJECTED for this implementation. W=0's one-shot improvement is a fusion signal only and requires repeated same-process measurement and a multi-thread design before interpretation.
- Evidence: `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-kernel-10900289/`; this record is not eligible for a paper table.
''')

issues.open('a').write(r'''

## 2026-10-08 reuse-window proxy versus legal kernel schedule

- Affected experiments: `recompute_window_20261008` structural scan and Job `10900289` source-aware kernel prototype.
- Symptom: the structural W=1024 audit predicted 173,153 extra regenerated source rows and peak source-plus-transient state 7,516 rows, while the legal DegreeSort consumer schedule in the prototype regenerated 1,687,277 extra source rows and measured peak cache 2,902 rows.
- Evidence: structural raw result `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/recompute_window_20261008/retry-04-analysis/raw_audit.json`; kernel log `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-kernel-10900289/program.log`.
- Root cause status: the structural scanner groups consumer ranks using source/CSR-derived keys and is explicitly a logical proxy; the prototype uses the actual DegreeSort destination batch rank and legal CSR traversal. The two rank definitions are not equivalent. The structural number must not be used as a runtime prediction until reconciled.
- Resolution/workaround: treat the prototype as the current source of legal-schedule work counts; audit and correct the structural scanner before using it to choose W. Keep the mismatch and negative timing visible.
- Prevention: every future window report must define producer rank, consumer rank, and the exact permutation/cursor order, and must compare the predicted regeneration count against an independently instrumented kernel trace.
- Validation status: numerical equality PASS; model reconciliation pending.
''')

progress.open('a').write(r'''

### 2026-10-08 source-aware reuse-window kernel checkpoint

- Completed: isolated single-thread RoadNet-CA prototype for W=0 and W=1024, preserving 128 -> 128 -> 128 and the canonical BF16/AMX boundary. Both candidate outputs are bitwise equal to materialized FULL FAST.
- Result: W=1024 reduces the measured cache peak to 2,902 source rows but is 2.48% slower than materialized FULL (`1267.936945` vs `1237.267971 ms`) because the legal schedule performs 1,687,277 extra source-row generations. W=0 is `1057.199001 ms` in one exploratory measurement and is not yet a performance claim.
- Important correction: the earlier structural audit's W=1024 extra-row estimate does not predict the legal DegreeSort schedule. Its rank construction must be reconciled before it can guide implementation.
- Current decision: do not claim a breakthrough or promote W=1024. The useful evidence is that cross-layer source-row reuse is numerically feasible and can trade state for recomputation, while the first legal implementation shows that state reduction alone does not produce speedup.
- Exact next task: run a same-process repeated bracket for materialized FULL, fused W=0, and W=1024; then sweep legal W values to locate the actual work/state frontier. Stop if W=0 is not stable or if no W improves the materialized control.
''')
