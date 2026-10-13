# October 7/8 GCN cross-layer fusion experiments

Start with [the review and corrections](../../docs/CROSSLAYER_FUSION_REVIEW_20261013.md)
and [the original latest server handoff](handoff_snapshot/CURRENT_HANDOFF.md).

## Main implementation and result

- [Latest frozen adaptive C++](runs/recompute-window-adaptive-build-20261008-234221/source_snapshot/recompute_window_adaptive.cpp)
- [Source cache and H1 generation](runs/recompute-window-adaptive-build-20261008-234221/source_snapshot/recompute_window_kernel.cpp)
- [Adaptive launch/protocol](experiments/recompute_window_adaptive_20261008/)
- [Raw latest adaptive log](runs/recompute-window-adaptive-20261008-234255/program.log)
- [Recomputed ordinary medians](adaptive_recomputed_statistics.csv) and [all 18 samples](adaptive_raw_samples.csv)

## Complete history

- [All 19 experiment source/protocol directories](experiments/)
- [All 65 run/build directories](runs/), including failure and superseded records
- [Run inventory](run_inventory.json), [Slurm accounting](slurm_accounting.txt)
- [Published file origins and SHA256](exported_files.csv), [original manifest checks](original_manifest_verification.csv)
- [Dependency snapshot](reference_dependencies/), [handoff history](handoff_snapshot/), [manual update scripts](handoff_updaters/)
- [Excluded datasets/executables and other non-text artifacts](excluded_files.csv)

This is a publication of existing exploratory evidence, with no new compile
or benchmark. Unit-weight graphs, seeded random H/W and two layers128->128->128.
The latest adaptive denominator is same-order materialized FULL FAST, not
original TFS/MKL/FULL ACCURATE. All paper claims remain unverified.

The original source/log/status bytes are preserved. The reported six-sample
upper median differs from the conventional median; both are retained. Fallback
candidate time aliases baseline time, and selector/order preprocessing is outside
the timed region. These limitations are explicit in the review and derived CSV.
Two early source manifests referenced subsequently modified live prototype
files; their old expected and present hashes remain visible. Latest adaptive
source and executable hashes match. Original binaries/datasets stay on server.

`SERVER_ARTIFACT_HASHES.json` covers the exact server export before derived
publication tables. `PUBLICATION_HASHES.json` additionally covers review aids.
Frozen include/compile paths retain the original server layout; dependency
snapshots must be placed at their original relative positions for replay.
