# Coverage audit evidence bundle

Read ../EXPERIMENT_COVERAGE_AUDIT_20261018.md first. Old review and experiment files are not modified.

- coverage_*.md: human-readable per-scope review, explicit errors, review depth and remaining limitations.
- coverage_*.json: per-experiment/run coverage and raw comparison results.
- coverage_machine_evidence.zip: full inventories, Git history/tree, source diffs, per-record extraction, and additional audit scripts. Internal sha256.json covers each entry.
- precleanup-8aeef16.tar.gz: GitHub archive of the pre-cleanup commit; original GCN-extra evidence was restored locally and all 3969 files checked against Git blob hashes. This archive is retained for recovery, not executed.
- coverage_history_raw.py: read-only parser with --root <restored-GCN-extra> --out <output-dir>. It reads existing logs and CSV; it does not run benchmarks. Other scripts in the ZIP retain the original local path constants and may need those constants adjusted before reuse.
- coverage_delivery_manifest.json: SHA256 of each delivered file. publish_coverage_server.py checks the manifest, preserves previous CURRENT_HANDOFF, appends the new event, then verifies all three persistent ledgers.

Inventory counts describe the pre-delivery server snapshot. This handoff itself adds files and an event, so a later filesystem count will differ. Timing rows are not experiment counts. Failure/partial/smoke results are not silently promoted to valid performance evidence.
