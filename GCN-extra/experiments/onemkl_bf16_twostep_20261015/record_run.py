#!/usr/bin/env python3
"""Record a completed GCN-extra run in the durable server handoff."""
import json
import pathlib
import subprocess
import sys


def main() -> None:
    run = pathlib.Path(sys.argv[1])
    mode, job_id = sys.argv[2:4]
    if mode != "pmu" and not (run / "summary.csv").exists():
        raise ValueError("Missing summarized comparison results")
    if mode == "pmu" and len(list((run / "logs").glob("*.perf.txt"))) != 2:
        raise ValueError("Missing BF16 oneMKL PMU files")
    node = (run / "node.txt").read_text().strip()
    event = dict(id=f"onemkl-bf16-twostep-{job_id}",kind="onemkl_bf16_twostep",
                 status="PASS",exit_status=0,job_id=job_id,node=node,mode=mode,
                 evidence=str(run),paper_eligible=False,
                 issues="Shared-node diagnostic; BF16 oneMKL uses FP32 Sparse BLAS with BF16-quantized input and BF16 GEMM after materializing and quantizing SpMM output.",
                 next="Complete fixed 17-graph comparison, numerical analysis, and AMX PMU confirmation.")
    path = run / "event.json"
    path.write_text(json.dumps(event,indent=2)+"\n")
    root = pathlib.Path("/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra")
    subprocess.run([sys.executable,str(root / "scripts/record_event.py"),str(path)],check=True)
    subprocess.run([sys.executable,str(root / "scripts/record_event.py"),"check"],check=True)


if __name__ == "__main__":main()
