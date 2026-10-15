#!/usr/bin/env python3
"""Record packed-BF16 control runs in the GCN-extra handoff."""
import json
import pathlib
import subprocess
import sys


def main() -> None:
    run = pathlib.Path(sys.argv[1])
    mode, job_id = sys.argv[2:4]
    if mode not in ("build_failed", "cancelled", "smoke", "diagnostic", "remaining", "full17"):
        raise ValueError(mode)
    if mode not in ("build_failed", "cancelled") and not (run / "summary.csv").exists():
        raise ValueError("Missing summary.csv")
    node = "none" if mode == "cancelled" else ("login" if mode == "build_failed" else (run / "node.txt").read_text().strip())
    event = dict(id=f"onemkl-bf16-packed-{job_id}",
                 kind="onemkl_bf16_twostep_packed_control", mode=mode,
                 status="CANCELLED" if mode == "cancelled" else ("FAILED" if mode == "build_failed" else "PASS"),
                 exit_status=1 if mode in ("build_failed", "cancelled") else 0,
                 job_id=job_id, node=node, evidence=str(run), paper_eligible=False,
                 issues=("Second shorter queue submission cancelled while pending to avoid duplicate 17-graph jobs."
                         if mode == "cancelled" else
                         "First build used nonexistent CblasBIdentifier; installed oneMKL 2023.2 header uses CblasBMatrix."
                         if mode == "build_failed" else
                         "Shared-node diagnostic; sparse step uses FP32 oneMKL Sparse BLAS on BF16-quantized features; BF16 GEMM W is packed before timing."),
                 next="Compare packed/unpacked oneMKL on all 17 fixed graphs and report full stage decomposition.")
    path = run / "event.json"
    path.write_text(json.dumps(event, indent=2) + "\n")
    root = pathlib.Path("/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra")
    subprocess.run([sys.executable, str(root / "scripts/record_event.py"), str(path)], check=True)
    subprocess.run([sys.executable, str(root / "scripts/record_event.py"), "check"], check=True)


if __name__ == "__main__":
    main()
