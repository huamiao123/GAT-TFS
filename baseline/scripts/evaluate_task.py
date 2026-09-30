"""Node classification evaluation outside timing; supports npy or CSV(.gz) labels/split."""
import argparse
import json
import numpy as np

parser = argparse.ArgumentParser()
parser.add_argument("--output", required=True, help="FP32 row-major model logits")
parser.add_argument("--nodes", type=int, required=True)
parser.add_argument("--classes", type=int, required=True)
parser.add_argument("--labels", required=True)
parser.add_argument("--split", required=True, help="development/test node IDs, not a boolean mask")
parser.add_argument("--reference", help="FP32 R0 logits for accuracy-drop gate")
parser.add_argument("--max-accuracy-drop", type=float)
args = parser.parse_args()

def array(path):
    return np.load(path, allow_pickle=False) if path.endswith(".npy") else np.loadtxt(path, delimiter=",")

def logits(path):
    data = np.fromfile(path, dtype="<f4")
    if data.size != args.nodes * args.classes or not np.isfinite(data).all():
        raise ValueError("invalid/nonfinite logits")
    return data.reshape(args.nodes, args.classes)

labels = array(args.labels).reshape(-1)
split = array(args.split).reshape(-1)
if labels.size != args.nodes or not np.isfinite(labels).all() or not np.isfinite(split).all():
    raise ValueError("invalid label/split sizes or finite values")
if np.any(labels != labels.astype(np.int64)) or np.any(split != split.astype(np.int64)):
    raise ValueError("labels/split must be integers")
labels = labels.astype(np.int64); split = split.astype(np.int64)
if split.size == 0 or np.any(split < 0) or np.any(split >= args.nodes) or np.unique(split).size != split.size:
    raise ValueError("invalid or duplicate split IDs")
if np.any(labels[split] < 0) or np.any(labels[split] >= args.classes):
    raise ValueError("split labels out of bounds")
actual = logits(args.output)[split].argmax(axis=1)
accuracy = float((actual == labels[split]).mean())
report = dict(accuracy=accuracy, split_nodes=int(split.size), gate="UNCONFIGURED")
if args.reference:
    reference = logits(args.reference)[split].argmax(axis=1)
    reference_accuracy = float((reference == labels[split]).mean())
    report.update(reference_accuracy=reference_accuracy, accuracy_drop=reference_accuracy-accuracy,
                  prediction_agreement=float((actual == reference).mean()))
if args.max_accuracy_drop is not None:
    if not args.reference or not np.isfinite(args.max_accuracy_drop) or args.max_accuracy_drop < 0:
        raise ValueError("accuracy drop gate needs reference and a finite nonnegative threshold")
    passed = report["accuracy_drop"] <= args.max_accuracy_drop
    report["gate"] = "PASS" if passed else "FAIL"
    print(json.dumps(report)); raise SystemExit(0 if passed else 1)
print(json.dumps(report))
