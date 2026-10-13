from pathlib import Path

root = Path('/home/huangjianqiang_group/hdacp1/data/wzh')
(root/'TFS-Train/docs/handoff/ALL_EXPERIMENTS.md').open('a').write(r'''

## 2026-10-08 reuse-window sweep launcher failure (Job 10900343)

- Experiment ID: `recompute_window_sweep_20261008_partial`; partition `intel`, node `qhcn022`, one task/one CPU, 16G, elapsed 00:00:08, exit 0:0.
- The intended W set was `{1,4,16,64,256,512,1024,2048,4096}`, but only `window-1.log` was produced. Its W=1 numerical gate passed; the launcher allowed `srun` to inherit the `while read` pipe and consume the remaining input, so the sweep was incomplete.
- Evidence: `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-sweep-10900343/`. The partial result is preserved and is not paper eligible or a sweep conclusion.
''')
(root/'TFS-Train/docs/handoff/EXPERIMENT_ISSUES.md').open('a').write(r'''

## 2026-10-08 window-sweep launcher consumed loop input (Job 10900343)

- Symptom: only the first planned window ran even though the Slurm job exited 0:0.
- Root cause: `srun` inherited the loop's standard input, so the command consumed the remaining window list.
- Resolution: redirect each `srun` standard input from `/dev/null` and require the post-run summary to contain every requested window before marking PASS.
- Prevention: all sequential Slurm steps inside a `while read` loop must use `< /dev/null`; postconditions must validate expected file count and window set.
- Validation status: correction pending rerun.
''')
(root/'TFS-Train/docs/handoff/PROJECT_PROGRESS.md').open('a').write(r'''

### 2026-10-08 window sweep partial run

- Job 10900343 is preserved as an incomplete launcher diagnostic. W=1 passed exact output equality, but the remaining windows did not execute because `srun` consumed the loop input.
- The sweep is not accepted. The launcher is being corrected with `/dev/null` stdin and an explicit expected-window completeness check before interpretation.
''')
