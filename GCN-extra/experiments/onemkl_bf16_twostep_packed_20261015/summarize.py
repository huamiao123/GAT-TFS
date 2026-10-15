#!/usr/bin/env python3
"""Summarize original-source BF16 oneMKL two-step experiment logs."""
import csv
import pathlib
import re
import statistics
import sys


def main() -> None:
    run = pathlib.Path(sys.argv[1])
    rows = []
    for path in sorted((run / "logs").glob("*.out")):
        content = path.read_text(errors="replace")
        name = path.stem
        if f"DONE graph={name} pass=1" not in content:
            raise ValueError(f"Incomplete graph: {path}")
        med = dict((method, float(value)) for method, value in re.findall(
            rf"MEDIAN graph={re.escape(name)} method=(\S+) ms=([0-9.eE+-]+)", content))
        if len(med) != 6:
            raise ValueError(f"Expected six methods for {path}: {med}")
        check = re.search(rf"CHECK graph={re.escape(name)} method=bf16_mkl_vs_tfs rel_l2=([0-9.eE+-]+) norm_max=([0-9.eE+-]+) max_abs=([0-9.eE+-]+) nonfinite=(\d+) pass=(\d+)", content)
        if check is None or check.group(5) != "1":
            raise ValueError(f"BF16 correctness not passed: {path}")
        packed_check = re.search(rf"CHECK graph={re.escape(name)} method=bf16_mkl_packed_vs_tfs rel_l2=([0-9.eE+-]+) norm_max=([0-9.eE+-]+) max_abs=([0-9.eE+-]+) nonfinite=(\d+) pass=(\d+)", content)
        if packed_check is None or packed_check.group(5) != "1":
            raise ValueError(f"Packed BF16 correctness not passed: {path}")
        stages = {"bf16_mkl_twostep": {}, "bf16_mkl_packed_twostep": {}}
        for line in content.splitlines():
            if line.startswith(f"STAGE_BF16 graph={name} "):
                method = re.search(r"method=(\S+)", line).group(1)
                for key, value in re.findall(r"(\w+)=([0-9.eE+-]+)", line):
                    if key not in ("graph", "repeat"):
                        stages[method].setdefault(key, []).append(float(value))
        if any(len(d) != 10 or any(len(v) != 5 for v in d.values()) for d in stages.values()):
            raise ValueError(f"Incomplete stage timings: {path}")
        row = dict(graph=name, node=(run / "node.txt").read_text().strip(),
                   bf16_mkl_ms=med["bf16_mkl_twostep"],
                   bf16_mkl_packed_ms=med["bf16_mkl_packed_twostep"],
                   original_tfs_ms=med["original_tfs"],
                   full_degree_ms=med["full_degree"],
                   full_identity_ms=med["full_identity"],
                   fp32_mkl_ms=med["fp32_mkl_twostep"],
                   bf16_mkl_over_full_degree=med["bf16_mkl_twostep"] / med["full_degree"],
                   bf16_mkl_over_full_identity=med["bf16_mkl_twostep"] / med["full_identity"],
                   bf16_mkl_over_original_tfs=med["bf16_mkl_twostep"] / med["original_tfs"],
                   bf16_packed_over_full_identity=med["bf16_mkl_packed_twostep"] / med["full_identity"],
                   bf16_packed_over_original_tfs=med["bf16_mkl_packed_twostep"] / med["original_tfs"],
                   bf16_unpacked_over_packed=med["bf16_mkl_twostep"] / med["bf16_mkl_packed_twostep"],
                   rel_l2=float(check.group(1)),norm_max=float(check.group(2)),
                   max_abs=float(check.group(3)),nonfinite=int(check.group(4)),
                   packed_rel_l2=float(packed_check.group(1)),packed_norm_max=float(packed_check.group(2)),
                   packed_max_abs=float(packed_check.group(3)),packed_nonfinite=int(packed_check.group(4)))
        for method, sd in stages.items():
            prefix = "packed_" if method == "bf16_mkl_packed_twostep" else "unpacked_"
            row.update({prefix+key+"_ms":statistics.median(values) for key,values in sd.items()})
        rows.append(row)
    if not rows:raise ValueError("No graph logs")
    with (run / "summary.csv").open("w",newline="") as fh:
        writer=csv.DictWriter(fh,fieldnames=list(rows[0]),lineterminator="\n")
        writer.writeheader();writer.writerows(rows)
    print(f"SUMMARY graphs={len(rows)} path={run / 'summary.csv'}")


if __name__ == "__main__":main()
