#!/usr/bin/env python3
"""Summarize process-scoped AMX perf events from the original-source audit."""
import csv
import pathlib
import sys


def main() -> None:
    root = pathlib.Path(sys.argv[1])
    rows = []
    for mode in ("multi_event", "single_event"):
        for path in sorted((root / mode / "logs").glob("*.perf.csv")):
            parts = path.stem.removesuffix(".perf").split("_")
            graph, method, repeat = parts[0], "_".join(parts[1:-1]), int(parts[-1])
            event = [line.split(",") for line in path.read_text().splitlines()
                     if ",amx_ops_retired.bf16:u," in line]
            if len(event) != 1:
                raise ValueError(f"Expected one BF16 AMX event in {path}")
            fields = event[0]
            rows.append(dict(mode=mode, graph=graph, method=method, repeat=repeat,
                             amx_bf16_ops=int(fields[0]),
                             running_percent=float(fields[4]),
                             raw_perf_path=path.relative_to(root).as_posix()))
    if len(rows) != 16:
        raise ValueError(f"Expected 16 measurements, got {len(rows)}")
    for row in rows:
        if row["method"] == "mkl_baseline" and row["amx_bf16_ops"] != 0:
            raise ValueError(f"Nonzero MKL BF16 AMX event: {row}")
        if row["method"] == "gcn_e2e_v3" and row["amx_bf16_ops"] <= 0:
            raise ValueError(f"Missing TFS positive control: {row}")
        if row["mode"] == "single_event" and row["running_percent"] != 100.0:
            raise ValueError(f"Single-event measurement was multiplexed: {row}")
    with (root / "summary.csv").open("w", newline="") as fh:
        writer = csv.DictWriter(fh, fieldnames=list(rows[0]), lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)
    print(f"16/16 valid: MKL zero, original TFS positive; {root / 'summary.csv'}")


if __name__ == "__main__":
    main()
