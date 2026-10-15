#!/usr/bin/env python3
"""Derive same-process source-oneMKL ratios from the fixed 17-graph run logs."""
import csv
import math
import pathlib
import re
import statistics
import sys


def main() -> None:
    results = pathlib.Path(sys.argv[1])
    run_root = results / "full-degree-sort-ablation-20261015-113936" / "logs"
    with (results / "summary_17.csv").open(newline="", encoding="utf-8-sig") as fh:
        ablation = list(csv.DictReader(fh))
    if len(ablation) != 17 or len({r["graph"] for r in ablation}) != 17:
        raise ValueError("Expected exactly the fixed 17 unique graphs")

    rows = []
    for row in ablation:
        path = run_root / row["run"] / "logs" / (row["graph"] + ".out")
        text = path.read_text(errors="replace")
        section = re.search(r"=== MKL FP32 Two-Step ===(.*?)MKL E2E BEST:\s*([0-9.]+) ms", text, re.S)
        if section is None:
            raise ValueError(f"Missing source-oneMKL section: {path}")
        samples = [float(v) for v in re.findall(r"run [0-4]: ([0-9.]+) ms", section.group(1))]
        if len(samples) != 5 or abs(min(samples) - float(section.group(2))) > 0.01:
            raise ValueError(f"Invalid source-oneMKL samples: {path}")
        mkl = statistics.median(samples)
        degree = float(row["degree_median_ms"])
        identity = float(row["identity_median_ms"])
        rows.append(dict(
            graph=row["graph"], node=row["node"], run=row["run"],
            mkl_fp32_median_ms=mkl, mkl_fp32_min_ms=min(samples),
            mkl_fp32_max_ms=max(samples), full_degree_median_ms=degree,
            full_identity_median_ms=identity,
            mkl_over_full_degree=mkl / degree,
            mkl_over_full_identity=mkl / identity,
            mkl_samples=";".join(str(x) for x in samples),
        ))

    rows.sort(key=lambda r: r["graph"])
    output = results / "summary_17_vs_onemkl.csv"
    with output.open("w", newline="", encoding="utf-8") as fh:
        writer = csv.DictWriter(fh, fieldnames=list(rows[0]), lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)
    for name in ("mkl_over_full_degree", "mkl_over_full_identity"):
        mean = math.exp(sum(math.log(r[name]) for r in rows) / len(rows))
        print(f"{name} geometric_mean={mean:.12g} wins={sum(r[name] > 1 for r in rows)}/17")
    print(output)


if __name__ == "__main__":
    main()
