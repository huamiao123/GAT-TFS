"""Independent NumPy FP64 three-layer and per-edge oracle for exported R0."""
import argparse
import struct
from pathlib import Path
import numpy as np

parser = argparse.ArgumentParser()
parser.add_argument("--graph", required=True)
parser.add_argument("--weights", required=True)
parser.add_argument("--dump-prefix", required=True)
args = parser.parse_args()
with open(args.graph, "rb") as f:
    assert f.read(8).startswith(b"GATBIN1")
    n, e, din, classes = struct.unpack("<QQII", f.read(24))
    row = np.fromfile(f, dtype="<u8", count=n+1)
    col = np.fromfile(f, dtype="<u4", count=e)
    x = np.fromfile(f, dtype="<f4", count=n*din).astype(np.float64).reshape(n, din)
with open(args.weights, "rb") as f:
    assert f.read(8).startswith(b"GATWGT1")
    assert struct.unpack("<I", f.read(4))[0] == 3
    model = []
    for layer in range(3):
        d_in, heads, d = struct.unpack("<III", f.read(12))
        w = np.fromfile(f, dtype="<f4", count=d_in*heads*d).astype(np.float64).reshape(d_in, heads*d)
        al = np.fromfile(f, dtype="<f4", count=heads*d).astype(np.float64).reshape(heads, d)
        ar = np.fromfile(f, dtype="<f4", count=heads*d).astype(np.float64).reshape(heads, d)
        model.append((w, al, ar))
for layer, (w, al, ar) in enumerate(model, 1):
    k, d = al.shape
    z = (x @ w).reshape(n, k, d)
    left = np.einsum("nhd,hd->nh", z, al)
    right = np.einsum("nhd,hd->nh", z, ar)
    score = np.empty((e, k)); p = np.empty_like(score); alpha = np.empty_like(score)
    maximum = np.zeros((n, k)); den = np.zeros_like(maximum); out = np.zeros((n, k, d))
    for i in range(n):
        b, end = map(int, row[i:i+2])
        if b == end:
            continue
        scores = left[i] + right[col[b:end]]
        scores = np.where(scores >= 0, scores, 0.2*scores)
        m = scores.max(axis=0)
        weights = np.exp(scores-m)
        denominator = weights.sum(axis=0)
        score[b:end] = scores; p[b:end] = weights; alpha[b:end] = weights/denominator
        maximum[i] = m; den[i] = denominator
        out[i] = np.einsum("eh,ehd->hd", alpha[b:end], z[col[b:end]])
    if layer < 3:
        out = np.where(out >= 0, out, np.expm1(np.minimum(out, 0)))
    expected = dict(z=z, left=left, right=right, max=maximum, den=den, score=score, p=p, alpha=alpha, output=out)
    for stage, ref in expected.items():
        actual = np.fromfile(f"{args.dump_prefix}_layer{layer}_{stage}.f32", dtype="<f4").reshape(ref.shape)
        assert np.isfinite(actual).all(), f"nonfinite {stage}"
        diff = actual-ref
        max_abs = np.abs(diff).max(initial=0)
        rel = np.linalg.norm(diff.ravel()) / max(np.linalg.norm(ref.ravel()), 1e-30)
        print(f"ORACLE layer={layer} stage={stage} max_abs={max_abs:.9g} relative_l2={rel:.9g}")
        # FP32-vs-FP64 implementation gate, not a BF16 task acceptance threshold.
        assert max_abs < 2e-4 and rel < 2e-4, (stage, max_abs, rel)
    x = out.reshape(n, k*d)
print("NUMPY_ORACLE PASS three_layer_chain=true")
