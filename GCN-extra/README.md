# GCN-extra: current controlled inference experiments

The controlled 2026-10-04 comparison and subsequent isolated journal studies
are retained. Prior experimental runs,
result reports, obsolete implementation variants and archive bundles were
deleted at the user's request. Negative results from the current comparison
remain included. Existing Git commits were not rewritten.

## The three implementation roles

| Role | Source |
|---|---|
| Original TFS v3 | [original/amx_tfs_v3.cpp](original/amx_tfs_v3.cpp) |
| Original MKL | [original/mkl_baseline.cpp](original/mkl_baseline.cpp) |
| Actual paired two-layer paper TFS/MKL harness | [original/gcn_e2e_v3.cpp](original/gcn_e2e_v3.cpp) |
| Modified TFS (neighbor reduction + tile-local AMX projection) | [src/paper_methods_kernels.hpp](src/paper_methods_kernels.hpp) |
| Checks, timing and candidate integration | [src/paper_methods_runtime.hpp](src/paper_methods_runtime.hpp) |

The actual E2E comparison uses the unmodified helpers in `gcn_e2e_v3.cpp`.
Standalone TFS/MKL files are original reference sources. The canonical modified
kernel is byte-identical to both validated build snapshots. DegreeSort,
16-row tiles, 64-row dynamic scheduling, AMX, packed W and original output order
are retained. B2/4/8/16/32/64/FULL FAST/ACCURATE are parameters of this method.
No complete graph-wide AH intermediate is produced.

## Current controls and results

- [Latest journal P3/P4 validation and interpretation](docs/JOURNAL_P3_P4_INTERPRETATION_20261005.md)
- [P3/P4 frozen sources, raw samples and PMU](results/journal-p3-p4-20261005/)
- [Earlier P1/P2/P3 evidence and remaining requirements](docs/JOURNAL_FOLLOWUP_VALIDATION_20261005.md)
- [Protocol](docs/PAPER_METHODS_PROTOCOL_20261004.md)
- [Main controlled comparison](docs/PAPER_METHODS_RESULTS_20261004.md)
- [Consecutive measurement control](docs/PAPER_CACHE_CONTROL_RESULTS_20261004.md)
- [Mechanism and timing interpretation](docs/PAPER_METHODS_INTERPRETATION_20261004.md)
- [Current evidence map and cleanup record](docs/CURRENT_EVIDENCE_20261004.md)
- [17 graph paths and full input hashes](docs/CURRENT_GRAPH_INVENTORY_20261004.json)
- [Figures](docs/figures/paper_methods_20261004/)

17 real unit-valued graphs; two-layer 128 -> 128 -> 128 inference computation;
source `srand(12345)` H/W; exact original MKL helper and hints; 32 threads and
default NUMA policy; paired denominators from the same graph/node/process.
1020 main plus 612 supplemental formal numerical checks passed. Current
consecutive B64 FAST geometric means are 1.309x vs original TFS and 2.435x vs
source MKL, winning 11/17 and 17/17 respectively. Individual slow cases remain.
These are source-compatible random H/W computation-flow experiments, not
checkpoint classification accuracy measurements.

## Reproduction and immutable evidence

`scripts/build_paper_methods.sh` generates the source-compatible harness from
the original source and canonical validated kernel. Submit with
`scripts/submit_paper_methods.sh {smoke,high,medium,low}`. The shared smoke
precedes exclusive formal jobs. Workspace AGENTS/SKILL controls apply; maximum
five nodes. No explicit NUMA/interleave or MKL algorithm change is introduced.
`build_paper_cache_control.sh` uses the retained main frozen build and changes
only measurement order. Finalizers are one-shot reconciliation tools.

Main evidence: `runs/paper-method-reconciled-20261004/`.
Consecutive evidence: `runs/paper-cache-reconciled-20261004/`.
Original compiled sources, build commands, binary hashes, launchers, full raw
measurements, numerical checks and separate stages/profiles remain in
`runs/paper-method-*` and `runs/paper-cache-*` snapshots. These snapshots are
immutable; historical launcher copies may mention deleted prior inventory
paths. Active scripts use the independent current graph inventory.

Clean bundle: `evidence-current-controlled-20261004-clean.tar.gz`, its SHA256
sidecar and `runs/cleanup-current-20261004/archive_manifest.json`. The bundle
also contains the original compiled executables, which are ignored separately
by Git. Graph datasets and toolchains stay at their original read-only paths.
