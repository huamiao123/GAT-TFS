#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$ROOT/experiments/recompute_window_kernel_20261008
BUILD=$(cat "$STUDY/LATEST_BUILD")
RUN="$ROOT/runs/recompute-window-kernel-${SLURM_JOB_ID}"
mkdir -p "$RUN"
finish(){
  s=$?
  python3 - "$RUN" "$s" <<'PY'
import json, pathlib, re, sys, os
r=pathlib.Path(sys.argv[1]); s=int(sys.argv[2])
log=(r/'program.log').read_text(errors='replace') if (r/'program.log').exists() else ''
gate=re.findall(r'WINDOW_GATE .*', log)
e=dict(id=r.name, kind='recompute_window_kernel', status='PASS' if s==0 and gate and 'pass=1' in gate[-1] else 'FAILED', exit_status=s, job_id=os.environ.get('SLURM_JOB_ID'), node=os.uname().nodename, evidence=str(r), paper_eligible=False, output_gate=gate[-1] if gate else None, issues='Single-thread isolated shadow prototype; no multi-thread or paper performance claim', next='If exact gate passes, analyze timing/work tradeoff and decide whether a parallel cache design is justified')
(r/'event.json').write_text(json.dumps(e,indent=2)+'\n')
(r/'exit_status.txt').write_text(str(s)+'\n')
PY
  python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$STUDY/PROTOCOL.md" "$RUN/protocol.read.md"
cp "$BUILD/source.sha256" "$RUN/source.sha256"
cp "$BUILD/binary.sha256" "$RUN/binary.sha256"
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS OMP_DYNAMIC KMP_AFFINITY GOMP_CPU_AFFINITY
export OMP_NUM_THREADS=1 OMP_PROC_BIND=close OMP_PLACES=cores
hostname > "$RUN/node.txt"
lscpu > "$RUN/cpu.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1
numactl --hardware > "$RUN/numa_hardware.txt"
env | sort > "$RUN/environment.txt"
printf '%q ' srun --ntasks=1 --cpus-per-task=1 --cpu-bind=cores "$BUILD/recompute_window_kernel" /home/huangjianqiang_group/hdacp1/data/yx/TFS/data roadNet-CA 1024 > "$RUN/command.txt"
srun --ntasks=1 --cpus-per-task=1 --cpu-bind=cores "$BUILD/recompute_window_kernel" /home/huangjianqiang_group/hdacp1/data/yx/TFS/data roadNet-CA 1024 > "$RUN/program.log" 2>&1
