#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$1
KIND=control
MODE=$2
RUN="$STUDY/logs/${KIND}-${SLURM_JOB_ID}"
mkdir -p "$RUN"
finish(){
 status=$?
 python3 - "$RUN" "$KIND" "$MODE" "$status" <<'PY'
import pathlib,json,sys,os
r=pathlib.Path(sys.argv[1]);c=json.loads((r/'completion.json').read_text()) if (r/'completion.json').exists() else {}
e=dict(id=r.parent.parent.name+'-'+r.name,kind='order_control_run',study=sys.argv[2],mode=sys.argv[3],status='PASS' if sys.argv[4]=='0' else 'FAILED',exit_status=int(sys.argv[4]),evidence=str(r),completion=c,job=os.environ.get('SLURM_JOB_ID'),node=os.uname().nodename,partition=os.environ.get('SLURM_JOB_PARTITION'),paper_eligible=False,issues='Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched',next='Numerical/work gate before interpretation, reconcile all three handoffs')
(r/'event.json').write_text(json.dumps(e,indent=2)+'\n');(r/'exit_status.txt').write_text(sys.argv[4]+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS OMP_DYNAMIC KMP_AFFINITY GOMP_CPU_AFFINITY
export OMP_NUM_THREADS=32
export OMP_PROC_BIND=close OMP_PLACES=cores PYTHONDONTWRITEBYTECODE=1
export TMPDIR="$RUN/tmp"
mkdir -p "$TMPDIR"
ulimit -c 0
hostname > "$RUN/node.txt"
lscpu > "$RUN/cpu.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1
numactl --hardware > "$RUN/numa_hardware.txt"
python3 - "$RUN" <<'PY'
import os,pathlib,json,sys
r=pathlib.Path(sys.argv[1]);cpus=sorted(os.sched_getaffinity(0));top=[]
for c in cpus:
 p=pathlib.Path('/sys/devices/system/cpu')/('cpu'+str(c))/'topology';top.append(dict(cpu=c,socket=int((p/'physical_package_id').read_text()),core=int((p/'core_id').read_text())))
info={a.strip():b.strip() for a,b in (x.split(':',1) for x in pathlib.Path('/proc/cpuinfo').read_text().split('\n\n',1)[0].splitlines() if ':' in x)}
state=dict(cpus=cpus,topology=top,threads=int(os.environ['OMP_NUM_THREADS']),cpuinfo=info)
(r/'affinity.json').write_text(json.dumps(state,indent=2))
assert len({x['socket'] for x in top})==1 and len({(x['socket'],x['core']) for x in top})==state['threads'],'Expected one-socket physical cores'
assert info.get('vendor_id')=='GenuineIntel' and info.get('cpu family')=='6' and info.get('model')=='143'
assert {'amx_tile','amx_bf16','avx512f','avx512_bf16'}.issubset(info['flags'].split())
keys=['OMP_NUM_THREADS','OMP_PROC_BIND','OMP_PLACES','MKL_NUM_THREADS','MKL_DYNAMIC','MKL_DOMAIN_NUM_THREADS','LD_LIBRARY_PATH','PATH','TMPDIR','SLURM_JOB_ID','SLURM_JOB_PARTITION']
(r/'environment.json').write_text(json.dumps({k:os.environ.get(k) for k in keys},indent=2))
PY
sha256sum "$STUDY/build/"* > "$RUN/build_files.sha256"
cp "$STUDY/source.sha256" "$RUN/source.sha256"
python3 "$STUDY/source_snapshot/order_control_suite.py" "$RUN" "$STUDY/build" "$MODE" > "$RUN/suite.log" 2>&1
