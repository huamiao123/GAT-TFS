#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$ROOT/experiments/recompute_window_parallel_20261008
BUILD=$(cat "$STUDY/LATEST_BUILD")
RUN="$ROOT/runs/recompute-window-parallel-${SLURM_JOB_ID}"
mkdir -p "$RUN"
finish(){
  s=$?
  python3 - "$RUN" "$s" <<'PY'
import json,pathlib,re,sys,os
r=pathlib.Path(sys.argv[1]); s=int(sys.argv[2])
log=(r/'program.log').read_text(errors='replace') if (r/'program.log').exists() else ''
gate=re.findall(r'PAR_GATE .*',log)
result=re.findall(r'PAR_RESULT .*',log)
e=dict(id=r.name,kind='recompute_window_parallel',status='PASS' if s==0 and gate and 'pass=1' in gate[-1] else 'FAILED',exit_status=s,job_id=os.environ.get('SLURM_JOB_ID'),node=os.uname().nodename,evidence=str(r),paper_eligible=False,par_result=result[-1] if result else None,issues='Isolated 32-thread shadow prototype; no paper performance claim',next='Use exact result and source-duplication cost to decide whether to continue parallel fusion')
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
export OMP_NUM_THREADS=32 OMP_PROC_BIND=close OMP_PLACES=cores
hostname > "$RUN/node.txt"
lscpu > "$RUN/cpu.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1
numactl --hardware > "$RUN/numa_hardware.txt"
env | sort > "$RUN/environment.txt"
printf '%q ' srun --ntasks=1 --cpus-per-task=32 --cpu-bind=cores "$BUILD/recompute_window_parallel" /home/huangjianqiang_group/hdacp1/data/yx/TFS/data roadNet-CA 32 > "$RUN/command.txt"
srun --ntasks=1 --cpus-per-task=32 --cpu-bind=cores "$BUILD/recompute_window_parallel" /home/huangjianqiang_group/hdacp1/data/yx/TFS/data roadNet-CA 32 < /dev/null > "$RUN/program.log" 2>&1
