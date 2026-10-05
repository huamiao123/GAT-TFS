# P3/P4 journal follow-up evidence

Parent commit: af943926c7d4f8e68bcb95838924e21b14dd2e86.
Read [the explanation](../../docs/JOURNAL_P3_P4_INTERPRETATION_20261005.md)
and [all fixed-method results](JOURNAL_P3_P4_RESULTS_20261005.md).

P3: D2/D3 broad correctness and performance against original TFS, B64 and
S64/FULL ACCURATE. P4: source-consumer selection, selective source projection
plus cold-source TFS fusion, strong project-all/project-used/mixed controls,
two shapes, two controlled relations and five real graphs. No paper baseline
replacement, cross-harness denominator, normalized GCN classification claim
or universal numerical guarantee. Shared-node measurements are diagnostic.

`p3/` and `p4/` retain the exact compiled `source_snapshot/`, manifests, build
commands/logs/hashes, every CSV, detailed sampled phases and per-TID PMU.
The three generated Python runtime caches in the frozen snapshots are also
retained to match the original inventories, despite the usual Git ignore.
`protocol_metadata/` retains input paths/hashes, CPU/affinity and NUMA evidence.
`graph_provenance.csv` matches graph hashes across P3/P4. Prepared timing
includes every layer's cache rebuild; topology setup is separately charged.
`superseded_smoke/` is preserved and excluded from main tables. Archive files
contain raw stdout and metadata, not compiled executables or graph datasets
in the main archive. The small superseded smoke includes generated fixtures.

`artifact_hashes.json` is the original reconciliation inventory;
`FINAL_ARTIFACT_HASHES.json` covers the subsequently added analysis/metadata.
`publication_state.json` distinguishes measured snapshots from publication git
state. See `ANALYSIS.json` for gate maxima and audit counts. Fine child phases
are thread-local sampled diagnostics, not additive wall time. PMU misses and
logical request bytes are not DRAM bandwidth measurements.
