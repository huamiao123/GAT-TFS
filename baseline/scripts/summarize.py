"""Summarize raw benchmark logs without rerunning or changing measurements."""
import argparse
import json
import statistics
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("run")
args = parser.parse_args()
root = Path(args.run)
report = {}
for file in sorted(root.glob("model_*.log")):
    rows = [json.loads(s) for s in file.read_text().splitlines() if s.startswith("{")]
    e2e = []
    for line in file.read_text().splitlines():
        if line.startswith("E2E "):
            values = dict(s.split("=", 1) for s in line.split()[1:])
            e2e.append(float(values["total_ms"]))
    stages = ("convert_ms", "projection_ms", "lr_ms", "max_prescan_ms", "score_exp_aggregate_ms", "normalize_ms", "activation_ms", "total_ms")
    by_layer = {}
    for layer in (1, 2, 3):
        sample = [r for r in rows if r["layer"] == layer]
        by_layer[layer] = {k: statistics.median(r[k] for r in sample) for k in stages}
    report[file.stem] = dict(model_median_ms=statistics.median(e2e), stages=by_layer)
for file in sorted(root.glob("profile*.log")):
    rows = [json.loads(s) for s in file.read_text().splitlines() if s.startswith("{")]
    sample = [r for r in rows if r["layer"] == 2]
    if sample:
        report[file.stem+"_L2"] = {k: statistics.mean(r[k] for r in sample) for k in sample[0] if k.endswith("_ms") or k in ("neighbor_steps", "executed_fma", "padding_executed_flops")}
print(json.dumps(report, indent=2))
