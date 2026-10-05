# Journal P0 reconciled evidence, 2026-10-04

These tables are copied from the immutable server study
`GCN-extra/runs/journal-a3-reconciled-20261004-230637`.
The A3 focus Slurm job was `10868162` on `qhcn064`; the thirteen-graph
extension was `10868206` on `qhcn165`. The earlier S/M study ran in jobs
`10867479` and `10867699`. Every per-graph method comparison is paired in
the same process and node. Measurements from different nodes are not mixed
into absolute-time denominators.

`summary.json` and `summary.csv` describe coverage and fixed policies.
`time.csv`, `stage.csv`, `profile.csv`, `detail.csv`, `thread.csv`, and
`thread_detail.csv` retain detailed timing. `check.csv`, `equiv.csv`, and
`profile_gate.csv` retain correctness. `counters.csv` and `model.csv` are
analytic work counters. `pmu*.csv` retain raw and summarized event readings.
`structure.csv` contains deterministic sampled graph-structure probes.

The immutable source/build manifests and full raw Slurm logs remain in the
server run directories. The frozen source is reproduced by
`GCN-extra/src/projection_window_*` plus
`GCN-extra/experiments/journal_decoupling_a3_20261004`.

The primary analysis is
`GCN-extra/docs/JOURNAL_P0_MECHANISM_20261005.md` and the complete timing
table is `GCN-extra/docs/JOURNAL_A3_RESULTS_20261004.md`.
