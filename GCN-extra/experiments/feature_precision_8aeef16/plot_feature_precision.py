#!/usr/bin/env python3
"""Plot paired BF16/FP32 E2E ratios from the completed precision study."""
import csv
import pathlib
import sys

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


def main(result_dir: pathlib.Path) -> None:
    rows = list(csv.DictReader((result_dir / "paired_comparisons.csv").open(
        encoding="utf-8-sig", newline="")))
    rows = [row for row in rows if row["kind"] == "e2e"]
    graphs = sorted({row["graph"] for row in rows})
    assert len(graphs) == 17 and len(rows) == 34
    by_key = {(row["graph"], row["block"]): row for row in rows}
    assert len(by_key) == 34
    graphs.sort(key=lambda graph: 1 / float(by_key[graph, "b64"]["paired_median"]))

    fig, ax = plt.subplots(figsize=(10.5, 7.2))
    for block, offset, color, marker in (
        ("b64", -0.16, "#175cd3", "o"),
        ("full", 0.16, "#e87912", "s"),
    ):
        center = [1 / float(by_key[graph, block]["paired_median"]) for graph in graphs]
        lower = [1 / float(by_key[graph, block]["paired_p75"]) for graph in graphs]
        upper = [1 / float(by_key[graph, block]["paired_p25"]) for graph in graphs]
        ax.errorbar(center, [index + offset for index in range(len(graphs))],
                    xerr=[[value - lo for value, lo in zip(center, lower)],
                          [hi - value for value, hi in zip(center, upper)]],
                    fmt=marker, color=color, ms=5, capsize=2, linewidth=1,
                    label=block.upper())
    ax.axvline(1, color="#555555", linestyle="--", linewidth=1)
    ax.set_xscale("log", base=2)
    ax.set_xticks([0.5, 1, 2, 4])
    ax.set_xticklabels(["0.5", "1", "2", "4"])
    ax.set_yticks(range(len(graphs)), labels=graphs)
    ax.set_xlabel("FP32-input / BF16-input E2E time (paired median; >1 favors BF16)")
    ax.set_title("Input precision and fusion granularity: 17 graphs")
    ax.grid(axis="x", alpha=0.2)
    ax.legend(loc="lower right", frameon=False)
    fig.text(0.5, 0.01,
             "Bars: paired P25–P75 across 10 ABBA measurements per method. "
             "Each graph uses one process and node.",
             ha="center", fontsize=8)
    fig.tight_layout(rect=(0, 0.035, 1, 1))
    fig.savefig(result_dir / "feature_precision_paired_e2e.png", dpi=220)
    fig.savefig(result_dir / "feature_precision_paired_e2e.pdf")
    plt.close(fig)


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]))
