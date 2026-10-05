#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
JOB_RUN=$1
STUDY=$2
MODE=$3
RUN=$JOB_RUN
finish(){
 status=$?
 python3 - "$JOB_RUN" "$status" "$STUDY" "$MODE" <<'PY'
import pathlib,json,sys,os
r=pathlib.Path(sys.argv[1]);e=dict(id=r.name,kind='residual_broad_run',status='COMPLETE' if sys.argv[2]=='0' else 'FAILED',exit_status=int(sys.argv[2]),evidence=str(r),study=sys.argv[3],mode=sys.argv[4],job=os.getenv('SLURM_JOB_ID'),node=os.uname().nodename,paper_eligible=False,issues='Candidate numeric rejection remains evidence; no relaxed gates or cross-node ratios',next='Inspect accepted candidates against decoupled FULL ACCURATE')
if (r/'completion.json').exists():e['completion']=json.loads((r/'completion.json').read_text())
(r/'event.json').write_text(json.dumps(e,indent=2)+'\n');(r/'exit_status.txt').write_text(sys.argv[2]+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$JOB_RUN/event.json"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
RUN=$JOB_RUN
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS KMP_AFFINITY
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK OMP_PROC_BIND=close OMP_PLACES=cores
python3 - "$RUN" <<'PY'
import pathlib,os,json,sys
r=pathlib.Path(sys.argv[1]);cpus=sorted(os.sched_getaffinity(0));top=[]
for c in cpus:
 p=pathlib.Path('/sys/devices/system/cpu')/('cpu'+str(c))/'topology';top.append(dict(cpu=c,socket=int((p/'physical_package_id').read_text()),core=int((p/'core_id').read_text())))
assert len({x['socket'] for x in top})==1 and len({(x['socket'],x['core']) for x in top})==int(os.environ['SLURM_CPUS_PER_TASK'])
(r/'affinity.json').write_text(json.dumps(dict(cpus=cpus,topology=top),indent=2))
PY
hostname > "$RUN/node.txt"
lscpu > "$RUN/cpu.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1
sha256sum "$STUDY/build/residual_methods" > "$RUN/binary.sha256"
python3 "$STUDY/source_snapshot/residual_suite.py" "$RUN" "$STUDY/build" "$MODE" > "$RUN/suite.log" 2>&1
