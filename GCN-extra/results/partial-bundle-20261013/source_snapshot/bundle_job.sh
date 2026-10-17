#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$1
MODE=$2
THREADS=$3
RUN="$STUDY/logs/bundle-${SLURM_JOB_ID}"
mkdir -p "$RUN/tmp"
finish(){
 status=$?
 python3 - "$RUN" "$MODE" "$status" <<'PY'
import pathlib,json,sys,os
r=pathlib.Path(sys.argv[1]);c=json.loads((r/'completion.json').read_text()) if (r/'completion.json').exists() else {}
e=dict(id=r.parent.parent.name+'-'+r.name,kind='partial_bundle_run',mode=sys.argv[2],status='PASS' if sys.argv[3]=='0' else 'FAILED',exit_status=int(sys.argv[3]),evidence=str(r),completion=c,job=os.environ.get('SLURM_JOB_ID'),node=os.uname().nodename,partition=os.environ.get('SLURM_JOB_PARTITION'),paper_eligible=False,issues='Same-process DegreeSort controls; default NUMA; shared exploratory timing; no new major issue if PASS',next='Retain all negative outcomes and reconcile source/work/timing evidence')
(r/'event.json').write_text(json.dumps(e,indent=2)+'\n');(r/'exit_status.txt').write_text(sys.argv[3]+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
}
trap finish EXIT
source /etc/profile.d/modules.sh
source "$ROOT/scripts/preflight.sh" "$RUN"
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS OMP_DYNAMIC KMP_AFFINITY GOMP_CPU_AFFINITY
export OMP_NUM_THREADS="$THREADS" OMP_PROC_BIND=close OMP_PLACES=cores
export PYTHONDONTWRITEBYTECODE=1 TMPDIR="$RUN/tmp"
ulimit -c 0
hostname > "$RUN/node.txt"
lscpu > "$RUN/cpu.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1
python3 - "$RUN" "$THREADS" <<'PY'
import os,pathlib,json,sys
r=pathlib.Path(sys.argv[1]);cpus=sorted(os.sched_getaffinity(0));top=[]
for c in cpus:
 p=pathlib.Path('/sys/devices/system/cpu')/('cpu'+str(c))/'topology'
 top.append(dict(cpu=c,socket=int((p/'physical_package_id').read_text()),core=int((p/'core_id').read_text())))
state=dict(cpus=cpus,topology=top,threads=int(sys.argv[2]))
(r/'affinity.json').write_text(json.dumps(state,indent=2)+'\n')
assert len({x['socket'] for x in top})==1
assert len({(x['socket'],x['core']) for x in top})==state['threads']
keys=['OMP_NUM_THREADS','OMP_PROC_BIND','OMP_PLACES','MKL_NUM_THREADS','MKL_DYNAMIC','MKL_DOMAIN_NUM_THREADS','LD_LIBRARY_PATH','TMPDIR','SLURM_JOB_ID','SLURM_JOB_PARTITION']
(r/'environment.json').write_text(json.dumps({k:os.environ.get(k) for k in keys},indent=2)+'\n')
PY
cp "$STUDY/source.sha256" "$RUN/source.sha256"
sha256sum "$STUDY/build/bundle_methods" "$STUDY/build/paper_original" > "$RUN/binary.sha256"
python3 "$STUDY/source_snapshot/bundle_suite.py" "$RUN" "$STUDY/build" "$MODE" > "$RUN/suite.log" 2>&1
