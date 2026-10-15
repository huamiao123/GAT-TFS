#!/usr/bin/env python3
"""Merge the fixed 5+12 graph runs and compute within-graph speedups."""
import csv
import math
import pathlib
import sys


GRAPHS = (
    "ogbn-products", "reddit", "mycielskian19", "hollywood-2009",
    "indochina-2004", "soc-Pokec", "cit-Patents", "soc-LiveJournal1",
    "com-LiveJournal", "as-Skitter", "rgg_n_2_24_s0", "web-Google",
    "roadNet-CA", "wiki-Talk", "email-Enron", "amazon0601", "com-Youtube",
)


def main() -> None:
    root = pathlib.Path(sys.argv[1])
    rows = []
    for file in ("diagnostic.csv", "remaining.csv"):
        with (root / file).open(newline="") as fh:
            for row in csv.DictReader(fh):
                if row["graph"] in {r["graph"] for r in rows}:
                    raise ValueError(f"Duplicate graph: {row['graph']}")
                rows.append(row)
    if {r["graph"] for r in rows} != set(GRAPHS):
        raise ValueError("Fixed 17-graph set mismatch")
    by_name = {r["graph"]:r for r in rows}
    result = []
    for graph in GRAPHS:
        r = by_name[graph]
        bf16 = float(r["bf16_mkl_ms"])
        entry = dict(graph=graph,node=r["node"],bf16_mkl_ms=bf16,
                     original_tfs_ms=float(r["original_tfs_ms"]),
                     full_degree_ms=float(r["full_degree_ms"]),
                     full_identity_ms=float(r["full_identity_ms"]),
                     fp32_mkl_ms=float(r["fp32_mkl_ms"]),
                     bf16_over_tfs=bf16/float(r["original_tfs_ms"]),
                     bf16_over_full_degree=bf16/float(r["full_degree_ms"]),
                     bf16_over_full_identity=bf16/float(r["full_identity_ms"]),
                     fp32_over_bf16=float(r["fp32_mkl_ms"])/bf16,
                     bf16_spmm12_ms=float(r["spmm1_ms"])+float(r["spmm2_ms"]),
                     bf16_gemm12_ms=float(r["gemm1_ms"])+float(r["gemm2_ms"]),
                     bf16_z_convert12_ms=float(r["z_to_bf16_1_ms"])+float(r["z_to_bf16_2_ms"]),
                     rel_l2=float(r["rel_l2"]),norm_max=float(r["norm_max"]))
        result.append(entry)
    output = root / "fixed17.csv"
    with output.open("w",newline="") as fh:
        writer=csv.DictWriter(fh,fieldnames=list(result[0]),lineterminator="\n")
        writer.writeheader();writer.writerows(result)
    for key in ("bf16_over_tfs","bf16_over_full_degree","bf16_over_full_identity","fp32_over_bf16"):
        gm=math.exp(sum(math.log(r[key]) for r in result)/len(result))
        print(f"{key} GM={gm:.6f} wins={sum(r[key]>1 for r in result)}/17")
    print(f"max_rel_l2={max(r['rel_l2'] for r in result):.9g}")
    print(output)


if __name__ == "__main__":main()
