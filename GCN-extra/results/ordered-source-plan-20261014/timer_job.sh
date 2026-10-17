#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
BUILD=$1
RUN="$BUILD/logs/cal-${SLURM_JOB_ID}"
mkdir -p "$RUN"
finish(){
 status=$?
 python3 - "$RUN" "$status" <<'PY'
import json,pathlib,sys,os
p=pathlib.Path(sys.argv[1]);e=dict(id=p.parent.parent.name+'-'+p.name,kind='timer_calibration_run',status='PASS' if sys.argv[2]=='0' else 'FAILED',exit_status=int(sys.argv[2]),job=os.environ.get('SLURM_JOB_ID'),evidence=str(p),node=os.uname().nodename,issues='Empty tick/tock calibration only; never subtract mechanically from production or sampled phase times')
(p/'event.json').write_text(json.dumps(e,indent=2)+'\n');(p/'exit_status.txt').write_text(sys.argv[2]+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
}
trap finish EXIT
source /etc/profile.d/modules.sh
source "$ROOT/scripts/preflight.sh" "$RUN"
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS OMP_DYNAMIC KMP_AFFINITY GOMP_CPU_AFFINITY
export OMP_NUM_THREADS=32 OMP_PROC_BIND=close OMP_PLACES=cores
hostname > "$RUN/node.txt"
lscpu > "$RUN/cpu.txt"
numactl --show > "$RUN/numa.txt" 2>&1
python3 - "$RUN" <<'PY'
import os,pathlib,json,sys
cpus=sorted(os.sched_getaffinity(0));top=[]
for c in cpus:
 p=pathlib.Path('/sys/devices/system/cpu')/('cpu'+str(c))/'topology';top.append(dict(cpu=c,socket=int((p/'physical_package_id').read_text()),core=int((p/'core_id').read_text())))
assert len(set((x['socket'],x['core']) for x in top))==32 and len(set(x['socket'] for x in top))==1
pathlib.Path(sys.argv[1]).joinpath('affinity.json').write_text(json.dumps(top,indent=2)+'\n')
PY
"$BUILD/build/timer_calibration" > "$RUN/timer.csv"
