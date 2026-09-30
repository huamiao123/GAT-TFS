"""Exercise task evaluator input validation and a declared fixture accuracy gate."""
import json
import struct
import subprocess
import sys
from pathlib import Path

root = Path(sys.argv[1]).resolve()
root.mkdir(parents=True, exist_ok=True)
script = Path(__file__).resolve().parents[1] / "scripts" / "evaluate_task.py"
actual = root / "actual.f32"
reference = root / "reference.f32"
labels = root / "labels.csv"
split = root / "split.csv"
labels.write_text("0\n1\n0\n")
split.write_text("0\n1\n2\n")
reference.write_bytes(struct.pack("<6f", 2, 1, 3, 4, 7, 6))
actual.write_bytes(reference.read_bytes())
cmd = [sys.executable, str(script), "--output", str(actual), "--nodes", "3", "--classes", "2",
       "--labels", str(labels), "--split", str(split), "--reference", str(reference), "--max-accuracy-drop", "0.05"]

def run(name, expected):
    result = subprocess.run(cmd, capture_output=True, text=True)
    if (result.returncode == 0) != expected:
        raise AssertionError((name, result.returncode, result.stdout, result.stderr))
    print(f"EVALUATOR_TEST PASS {name}")
    return result

result = run("perfect_predictions", True)
assert json.loads(result.stdout)["accuracy"] == 1
actual.write_bytes(struct.pack("<6f", 1, 2, 3, 4, 7, 6))
result = run("accuracy_drop_gate", False)
assert json.loads(result.stdout)["gate"] == "FAIL"
actual.write_bytes(struct.pack("<6f", float("nan"), 2, 3, 4, 7, 6))
run("nonfinite_logits", False)
actual.write_bytes(reference.read_bytes()[:-4])
run("wrong_logits_size", False)
actual.write_bytes(reference.read_bytes())
split.write_text("0\n0\n2\n")
run("duplicate_split", False)
split.write_text("0\n1\n3\n")
run("out_of_range_split", False)
split.write_text("0\n1\n2\n")
labels.write_text("0\n1\n2\n")
run("out_of_range_label", False)
print("EVALUATOR_TESTS PASS task_fixture_only_not_real_checkpoint")
