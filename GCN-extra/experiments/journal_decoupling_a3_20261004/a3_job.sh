#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$1
MODE=$2
RUN="$STUDY/logs/$MODE-${SLURM_JOB_ID}"
mkdir -p "$RUN"
finish(){
 s=$?
 python3 - "$RUN" "$STUDY" "$MODE" "$s" <<'PY'
import json,pathlib,sys,os
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[4]);c=json.loads((r/'completion.json').read_text()) if (r/'completion.json').exists() else {}
e=dict(id=r.parent.parent.name+'-'+r.name,kind='journal_a3_'+sys.argv[3],status='PASS' if s==0 else 'FAILED',exit_status=s,job_id=os.environ.get('SLURM_JOB_ID'),node=os.uname().nodename,study=sys.argv[2],evidence=str(r),completion=c,paper_eligible=False,issues='Shared-node same-process method comparisons; no interleave; A3 control partial trace only in checks',next='Compare A0/A1/A2/A3 then extend after successful four-graph diagnostic')
(r/'event.json').write_text(json.dumps(e,indent=2)+'\n');(r/'exit_status.txt').write_text(str(s)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS OMP_DYNAMIC KMP_AFFINITY GOMP_CPU_AFFINITY
export OMP_NUM_THREADS=${SLURM_CPUS_PER_TASK}
export OMP_PROC_BIND=close OMP_PLACES=cores PYTHONDONTWRITEBYTECODE=1
ulimit -c 0
hostname > "$RUN/node.txt"
lscpu > "$RUN/cpu.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1
numactl --hardware > "$RUN/numa_hardware.txt"
env | sort > "$RUN/environment.txt"
python3 - "$RUN" <<'PY'
import os,pathlib,json,sys
r=pathlib.Path(sys.argv[1]);cpus=sorted(os.sched_getaffinity(0));top=[]
for c in cpus:
 p=pathlib.Path('/sys/devices/system/cpu')/('cpu'+str(c))/'topology'
 top.append(dict(cpu=c,socket=int((p/'physical_package_id').read_text()),core=int((p/'core_id').read_text())))
first=pathlib.Path('/proc/cpuinfo').read_text().split('\n\n',1)[0]
info={a.strip():b.strip() for a,b in (line.split(':',1) for line in first.splitlines() if ':' in line)}
state=dict(allowed_cpus=cpus,topology=top,threads=int(os.environ['OMP_NUM_THREADS']),cpuinfo=info)
(r/'affinity.json').write_text(json.dumps(state,indent=2))
assert len({x['socket'] for x in top})==1 and len({(x['socket'],x['core']) for x in top})==state['threads'],'One-socket physical-core allocation required'
assert info.get('vendor_id')=='GenuineIntel' and info.get('cpu family')=='6' and info.get('model')=='143','Need paper-compatible SPR CPU'
assert {'amx_tile','amx_bf16','avx512f','avx512_bf16'}.issubset(info['flags'].split()),'ISA missing'
PY
sha256sum "$STUDY/build/projection_methods" "$STUDY/build/paper_original" > "$RUN/binary.sha256"
cp "$STUDY/source.sha256" "$RUN/source.sha256"
python3 "$STUDY/source_snapshot/projection_window_a3_suite.py" "$RUN" "$STUDY/build" "$MODE" > "$RUN/suite.log" 2>&1
