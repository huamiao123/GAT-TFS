#!/usr/bin/env python3
"""Summarize the preserved three-path adaptive GAT run without changing raw logs."""
import csv
import json
import statistics
import sys
from pathlib import Path

run = Path(sys.argv[1]).resolve()
if (run / "status.txt").read_text().strip() != "exit_status=0":
    raise SystemExit("incomplete job")
paths = ("B0_BF16", "HYBRID_T16_B32", "TFS_L12_B0_L3_MATCHED")
fields = ("e2e_ms", "layer_ms", "projection_ms", "conversion_ms", "lr_ms",
          "max_prescan_ms", "kernel_ms", "normalization_ms", "activation_ms")
rows = []
checks = []
for graph in ("arxiv", "products"):
    log = (run / f"{graph}.log").read_text()
    if "ICPP_ADAPTIVE_COMPLETE" not in log or "ICPP_HEADS_COMPLETE" not in log:
        raise SystemExit(f"{graph}: missing completion markers")
    raw = []
    for line in log.splitlines():
        if line.startswith('{"path":'):
            item = json.loads(line)
            if item["path"] not in paths:
                raise SystemExit(f"{graph}: unexpected path {item['path']}")
            raw.append(item)
        elif line.startswith("CHECK "):
            item = dict(token.split("=", 1) for token in line.split()[1:])
            checks.append({"graph": graph, **item})
    if len(raw) != len(paths) * 3 * 3:
        raise SystemExit(f"{graph}: expected 27 timing rows, got {len(raw)}")
    by = {(path, layer): [r for r in raw if r["path"] == path and r["layer"] == layer]
          for path in paths for layer in (1, 2, 3)}
    for path in paths:
        e2e = [r["e2e_ms"] for r in by[path, 1]]
        for layer in (1, 2, 3):
            group = by[path, layer]
            if sorted(r["rep"] for r in group) != [0, 1, 2]:
                raise SystemExit(f"{graph}/{path}/{layer}: invalid repetitions")
            row = {"graph": graph, "path": path, "layer": layer, "repeats": 3,
                   "e2e_min_ms": min(e2e), "e2e_max_ms": max(e2e)}
            row.update({key: statistics.median(r[key] for r in group) for key in fields})
            rows.append(row)

for graph in ("arxiv", "products"):
    base = next(r["e2e_ms"] for r in rows if r["graph"] == graph and r["path"] == paths[0])
    full = next(r["e2e_ms"] for r in rows if r["graph"] == graph and r["path"] == paths[1])
    for row in rows:
        if row["graph"] == graph:
            row["speedup_vs_B0_BF16"] = base / row["e2e_ms"]
            row["speedup_vs_full_TFS"] = full / row["e2e_ms"]
            if row["layer"] == 1:
                print(f"{graph} {row['path']} E2E={row['e2e_ms']:.3f}ms "
                      f"vsB0={row['speedup_vs_B0_BF16']:.4f}x "
                      f"vsFullTFS={row['speedup_vs_full_TFS']:.4f}x")

with (run / "timing_summary.tsv").open("w", newline="", encoding="utf-8") as stream:
    writer = csv.DictWriter(stream, fieldnames=list(rows[0]), delimiter="\t")
    writer.writeheader()
    writer.writerows(rows)
with (run / "checks.tsv").open("w", newline="", encoding="utf-8") as stream:
    writer = csv.DictWriter(stream, fieldnames=list(checks[0]), delimiter="\t")
    writer.writeheader()
    writer.writerows(checks)
