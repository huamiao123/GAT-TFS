# P1 immutable measurement and reconciliation

Server build: `runs/journal-p1-build-20261005-111037`.
Postprocessing: `runs/journal-p1-reconciled-20261005-115500`.
Read `JOURNAL_P1_RESULTS_20261005.md` for complete fixed-policy tables;
read `../../docs/JOURNAL_FOLLOWUP_VALIDATION_20261005.md` for interpretation
and the remaining plan items. New grouping source is in
`../../experiments/journal_grouping_p1_20261005/`.

17 real + 3 controlled graphs; smoke records are separate. All same-scope
FP32/BF16 output and enumerated logical-work gates passed. Shared nodes,
32 physical cores/one socket, default NUMA, prepared two-layer random H/W
compute. No task accuracy or trained model checkpoint is claimed.

All parsed raw records are retained in CSV, including each repetition,
alternating-order control, layer stages, sampled child/per-thread timing,
logical work, PMU and schedule audit/structure. `provenance/` copies the
actual frozen source snapshot, build metadata and each job's environment
and completion evidence. Executable binaries and input graph data remain
on the server; their hashes are recorded. Source TFS/MKL are unmodified.

`artifact_hashes.json` is the original reconciliation inventory. The later
curation adds provenance, interpretation event and this README;
`CURATED_ARTIFACT_HASHES.json` covers the complete curated directory
except that inventory itself. Frozen measurements are not edited.
