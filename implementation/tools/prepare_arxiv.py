#!/usr/bin/env python3
"""Create a destination-row, undirected, self-looped CSR from OGB raw CSV."""
import argparse
import gzip
import json
import struct
from pathlib import Path

import numpy as np
from scipy import sparse


def load_csv(path, dtype):
    with gzip.open(path, "rt") as stream:
        return np.loadtxt(stream, delimiter=",", dtype=dtype)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--dataset", default="ogbn-arxiv")
    args = parser.parse_args()
    raw = args.source / "raw"
    x = np.asarray(load_csv(raw / "node-feat.csv.gz", np.float32), dtype="<f4", order="C")
    edges = load_csv(raw / "edge.csv.gz", np.int64)
    labels = load_csv(raw / "node-label.csv.gz", np.int64)
    n, d = x.shape
    assert edges.shape[1] == 2 and labels.shape[0] == n
    src, dst = edges[:, 0], edges[:, 1]
    assert src.min() >= 0 and dst.min() >= 0 and src.max() < n and dst.max() < n
    row = np.concatenate((dst, src, np.arange(n)))
    col = np.concatenate((src, dst, np.arange(n)))
    graph = sparse.coo_matrix((np.ones(row.size, dtype=np.uint8), (row, col)),
                              shape=(n, n)).tocsr()
    graph.sum_duplicates()
    graph.sort_indices()
    rowptr = np.asarray(graph.indptr, dtype="<u8")
    indices = np.asarray(graph.indices, dtype="<u4")
    classes = int(labels.max() + 1)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("wb") as out:
        out.write(struct.pack("<8sQQII", b"GATBIN1\0", n, indices.size, d, classes))
        rowptr.tofile(out)
        indices.tofile(out)
        x.tofile(out)
    meta = dict(dataset=args.dataset, source=str(args.source), nodes=n,
                edges=int(indices.size), in_features=d, classes=classes,
                graph="undirected, deduplicated, one self-loop per node",
                dtype="float32", format="GATBIN1")
    args.output.with_suffix(".json").write_text(json.dumps(meta, indent=2) + "\n")
    print(json.dumps(meta))


if __name__ == "__main__":
    main()
