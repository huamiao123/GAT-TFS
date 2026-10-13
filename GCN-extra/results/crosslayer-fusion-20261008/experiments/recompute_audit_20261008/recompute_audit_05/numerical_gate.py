import hashlib
import json
import struct
from pathlib import Path

import numpy as np

SEED = 20261008
DIM = 128

def bf16_trunc(x):
    a = np.asarray(x, dtype=np.float32)
    return (a.view(np.uint32) >> 16).astype(np.uint16)

def bf16_decode(x):
    return (np.asarray(x, dtype=np.uint32) << 16).view(np.float32)

def make_graph(n=64):
    rng = np.random.default_rng(SEED)
    rows = []
    for d in range(n):
        degree = int(rng.integers(1, min(10, n) + 1))
        rows.append(sorted(set(int(x) for x in rng.choice(n, degree, replace=False))))
    return rows

def h1_row(j, rows, x_bf16, w0_bf16):
    acc = np.zeros(DIM, dtype=np.float32)
    for src in rows[j]:
        acc += bf16_decode(x_bf16[src])
    partial_bf16 = bf16_trunc(acc)
    projected = np.maximum(bf16_decode(partial_bf16) @ bf16_decode(w0_bf16), 0).astype(np.float32)
    return bf16_trunc(projected)

def second_row(d, rows, source_provider, w1_bf16):
    acc = np.zeros(DIM, dtype=np.float32)
    for src in rows[d]:
        acc += bf16_decode(source_provider(src))
    partial_bf16 = bf16_trunc(acc)
    return bf16_trunc(bf16_decode(partial_bf16) @ bf16_decode(w1_bf16))

def run():
    rows = make_graph()
    n = len(rows)
    rng = np.random.default_rng(SEED + 1)
    x_bf16 = bf16_trunc(rng.standard_normal((n, DIM), dtype=np.float32))
    w0_bf16 = bf16_trunc(rng.standard_normal((DIM, DIM), dtype=np.float32))
    w1_bf16 = bf16_trunc(rng.standard_normal((DIM, DIM), dtype=np.float32))
    # H1 source is generated once in the reference path.
    h1 = [h1_row(j, rows, x_bf16, w0_bf16) for j in range(n)]
    reference = np.stack([second_row(d, rows, lambda j: h1[j], w1_bf16) for d in range(n)])
    rank = [max(rows[d]) // 16 + 1 for d in range(n)]
    consumers = [sorted(set(rank[d] for d in range(n) if j in rows[d])) for j in range(n)]
    calls = {j: 0 for j in range(n)}
    by_rank = {}
    for d, p in enumerate(rank):
        by_rank.setdefault(p, []).append(d)
    regenerated = {}
    candidate = np.zeros_like(reference)
    for p in sorted(by_rank):
        # Rows in one consumer rank share a temporary source cache. The cache
        # is dropped after the rank, matching the cost model's optimistic plan.
        cache = {}
        for d in by_rank[p]:
            def provider(j):
                if j not in cache:
                    cache[j] = h1_row(j, rows, x_bf16, w0_bf16)
                    calls[j] += 1
                return cache[j]
            candidate[d] = second_row(d, rows, provider, w1_bf16)
        for j in cache:
            regenerated[j] = regenerated.get(j, 0) + 1
    assert np.array_equal(reference, candidate), 'recompute changed output boundary'
    extra_edges = sum((len(consumers[j]) - 1) * len(rows[j]) for j in range(n))
    expected_calls = sum(len(c) for c in consumers)
    observed_calls = sum(calls.values())
    assert expected_calls == observed_calls
    return {
        'status': 'PASS', 'seed': SEED, 'n': n, 'dim': DIM,
        'reference_candidate_equal': True, 'reference_sha256': hashlib.sha256(reference.tobytes()).hexdigest(),
        'candidate_sha256': hashlib.sha256(candidate.tobytes()).hexdigest(),
        'extra_first_layer_edge_visits': int(extra_edges),
        'source_consumer_rank_events': int(expected_calls),
        'source_rows': n, 'row_completion_ranks': sorted(set(rank)),
        'quantization': 'uint32 top16 truncation at aggregate and output boundaries',
        'scope': 'toy semantic gate; no performance claim'
    }

if __name__ == '__main__':
    out = run()
    print(json.dumps(out, indent=2))
