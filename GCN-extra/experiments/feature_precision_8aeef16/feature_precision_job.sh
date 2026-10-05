#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$1
MODE=$2
RUN="$STUDY/logs/$MODE-${SLURM_JOB_ID}"
mkdir -p "$RUN"
finish(){
 status=$?
 python3 - "$RUN" "$status" "$STUDY" "$MODE" <<'PY'
import pathlib,json,sys,os
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2]);c=json.loads((r/'completion.json').read_text()) if (r/'completion.json').exists() else {}
e=dict(id=r.parent.parent.name+'-'+r.name,kind='feature_precision_'+sys.argv[4],status='PASS' if s==0 else 'FAILED',exit_status=s,job_id=os.environ.get('SLURM_JOB_ID'),node=os.uname().nodename,study=sys.argv[3],evidence=str(r),completion=c,paper_eligible=False,issues='Shared node explicitly authorized; source controls frozen; no NUMA/MKL policy change',next='Five diagnostic graph gates before remaining-twelve extension')
(r/'event.json').write_text(json.dumps(e,indent=2));(r/'exit_status.txt').write_text(str(s)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS OMP_DYNAMIC KMP_AFFINITY GOMP_CPU_AFFINITY
export OMP_NUM_THREADS=${SLURM_CPUS_PER_TASK}
export OMP_PROC_BIND=close OMP_PLACES=cores
export PYTHONDONTWRITEBYTECODE=1
ulimit -c 0
hostname > "$RUN/node.txt"
lscpu > "$RUN/cpu.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1
numactl --hardware > "$RUN/numa_hardware.txt"
env | sort > "$RUN/environment.txt"
python3 - "$RUN" <<'PY'
import pathlib,json,os,sys
r=pathlib.Path(sys.argv[1]);cpus=sorted(os.sched_getaffinity(0));topology=[]
for c in cpus:
 p=pathlib.Path('/sys/devices/system/cpu')/('cpu'+str(c))/'topology'
 topology.append(dict(cpu=c,socket=int((p/'physical_package_id').read_text()),core=int((p/'core_id').read_text())))
state=dict(allowed_cpus=cpus,topology=topology,threads=int(os.environ['OMP_NUM_THREADS']),sockets=sorted({t['socket'] for t in topology}),physical_cores=len({(t['socket'],t['core']) for t in topology}))
first=pathlib.Path('/proc/cpuinfo').read_text().split('\n\n',1)[0]
cpuinfo={a.strip():b.strip() for a,b in (line.split(':',1) for line in first.splitlines() if ':' in line)}
state['cpuinfo']=cpuinfo
(r/'affinity.json').write_text(json.dumps(state,indent=2))
assert len(state['sockets'])==1 and state['physical_cores']==state['threads'],'Allocation is not the requested one-socket physical-core set'
assert cpuinfo.get('vendor_id')=='GenuineIntel' and cpuinfo.get('cpu family')=='6' and cpuinfo.get('model')=='143','CPU differs from paper-compatible Sapphire Rapids target'
assert {'amx_tile','amx_bf16','avx512f','avx512_bf16'}.issubset(cpuinfo['flags'].split()),'Required AMX/AVX512 ISA missing'
PY
sha256sum "$STUDY/build/feature_precision_driver" "$STUDY/build/paper_original" > "$RUN/binary.sha256"
cp "$STUDY/source.sha256" "$RUN/source.sha256"
python3 "$STUDY/source_snapshot/feature_precision_suite.py" "$RUN" "$STUDY" "$MODE" > "$RUN/suite.log" 2>&1
