#!/usr/bin/env python3
"""Synchronize a NUMA-interleaved oneMKL/TFS comparison with GCN-extra handoff."""
import json
import pathlib
import subprocess
import sys


def main() -> None:
    run = pathlib.Path(sys.argv[1])
    mode, job_id = sys.argv[2:4]
    if mode not in ("smoke", "full17"):
        raise ValueError(mode)
    if not (run / "summary.csv").exists():
        raise ValueError("Missing summary.csv")
    policy = (run / "interleave_policy.txt").read_text()
    if "policy: interleave" not in policy:
        raise ValueError("NUMA interleave policy not confirmed")
    root = pathlib.Path("/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra")
    parent = run.parent
    event = dict(
        id=f"onemkl-bf16-interleave-{job_id}",
        kind="onemkl_bf16_tfs_interleave_control", status="PASS", exit_status=0,
        mode=mode, job_id=job_id, node=(run / "node.txt").read_text().strip(),
        numa_policy="numactl --interleave=all for both oneMKL and original TFS in one process",
        threads=4 if mode == "smoke" else 32,
        source_hashes=str(parent / "source.sha256"),
        binary_hash=str(parent / "binary.sha256"),
        evidence=str(run), paper_eligible=False,
        issues="Shared-node diagnostic; oneMKL sparse step is FP32 Sparse BLAS on BF16-quantized input, GEMM is BF16 AMX.",
        next="Compare 17 graph within-process BF16 oneMKL versus original TFS under the same confirmed interleave policy.",
    )
    path = run / "event.json"
    path.write_text(json.dumps(event, indent=2) + "\n")
    subprocess.run([sys.executable, str(root / "scripts/record_event.py"), str(path)], check=True)
    subprocess.run([sys.executable, str(root / "scripts/record_event.py"), "check"], check=True)


if __name__ == "__main__":
    main()
