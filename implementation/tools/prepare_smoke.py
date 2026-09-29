#!/usr/bin/env python3
"""Deterministic GATBIN1 fixture with varied row degrees and self-loops."""
import struct
from pathlib import Path
import numpy as np

root = Path("/home/huangjianqiang_group/hdacp1/data/wzh/GAT")
out = root / "data" / "smoke1024.gatbin"
rng = np.random.default_rng(20260928)
n, d, c = 1024, 128, 40
degrees = [1, 2, 7, 15, 16, 17, 31, 32, 33, 63, 64, 65, 96, 127]
row = [0]
col = []
for i in range(n):
    deg = degrees[i % len(degrees)]
    neighbors = rng.choice(n, size=deg - 1, replace=False).astype(np.uint32).tolist()
    col.extend([i] + neighbors)
    row.append(len(col))
x = rng.normal(0, 0.5, size=(n, d)).astype("<f4")
with out.open("wb") as f:
    f.write(struct.pack("<8sQQII", b"GATBIN1\0", n, len(col), d, c))
    np.asarray(row, dtype="<u8").tofile(f)
    np.asarray(col, dtype="<u4").tofile(f)
    x.tofile(f)
print(out, "nodes", n, "edges", len(col))

