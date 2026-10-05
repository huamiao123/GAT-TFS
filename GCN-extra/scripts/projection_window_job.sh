#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
SNAPSHOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
MODE=$1
BUILD=$(cat "$SNAPSHOT/build_path.txt")
RUN="$ROOT/runs/projection-window-$MODE-${SLURM_JOB_ID}"
mkdir -p "$RUN/launcher_snapshot"
cp "$SNAPSHOT/"* "$RUN/launcher_snapshot/"
finish(){
 status=$?
 python3 - "$RUN" "$status" "$BUILD" <<'PY'
import pathlib,json,sys,os
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2]);c=json.loads((r/'completion.json').read_text()) if (r/'completion.json').exists() else {}
e=dict(id=r.name,kind='independent_projection_window',status='PASS' if s==0 else 'FAILED',exit_status=s,job_id=os.environ.get('SLURM_JOB_ID'),node=os.uname().nodename,build=sys.argv[3],evidence=str(r),completion=c,paper_eligible='smoke' not in r.name and s==0,issues='Same source/matrix/degree scheduling/NUMA/precision; same-M bitwise gates; both timing orders retained',next='Reconcile all raw results and retain negative outcomes')
(r/'event.json').write_text(json.dumps(e,indent=2));(r/'exit_status.txt').write_text(str(s)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$ROOT/docs/PROJECTION_WINDOW_PROTOCOL_20261004.md" "$RUN/protocol.read.md"
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS OMP_DYNAMIC KMP_AFFINITY GOMP_CPU_AFFINITY
export OMP_NUM_THREADS=${SLURM_CPUS_PER_TASK}
export OMP_PROC_BIND=close OMP_PLACES=cores
ulimit -c 0
hostname > "$RUN/node.txt"
lscpu > "$RUN/cpu.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1
numactl --hardware > "$RUN/numa_hardware.txt"
taskset -pc $$ > "$RUN/process_affinity.txt"
python3 - "$RUN" <<'PY'
import json,os,pathlib,sys
r=pathlib.Path(sys.argv[1]);cpus=sorted(os.sched_getaffinity(0));topology=[]
for c in cpus:
 p=pathlib.Path('/sys/devices/system/cpu')/('cpu'+str(c))/'topology'
 topology.append(dict(cpu=c,socket=int((p/'physical_package_id').read_text()),core=int((p/'core_id').read_text())))
first=pathlib.Path('/proc/cpuinfo').read_text().split('\n\n',1)[0]
info={a.strip():b.strip() for a,b in (line.split(':',1) for line in first.splitlines() if ':' in line)}
state=dict(allowed_cpus=cpus,topology=topology,OMP_NUM_THREADS=int(os.environ['OMP_NUM_THREADS']),cpuinfo=info)
(r/'affinity.json').write_text(json.dumps(state,indent=2))
assert len({v['socket'] for v in topology})==1 and len({(v['socket'],v['core']) for v in topology})==state['OMP_NUM_THREADS'],'Need one socket and one physical core per thread'
assert info.get('vendor_id')=='GenuineIntel' and info.get('cpu family')=='6' and info.get('model')=='143','CPU differs from paper-compatible Sapphire Rapids'
assert {'amx_tile','amx_bf16','avx512f','avx512_bf16'}.issubset(info['flags'].split()),'Required ISA missing'
PY
python3 - "$RUN" <<'PY'
import pathlib,json,sys,subprocess,shutil
r=pathlib.Path(sys.argv[1]);out={}
for p in pathlib.Path('/sys/devices/system/node').glob('node*/memory_side_cache/index*/*'):
 try:out[str(p)]=p.read_text().strip()
 except OSError as e:out[str(p)]=str(e)
out['dax_devices']=[str(p) for p in pathlib.Path('/sys/bus/dax/devices').glob('*')]
out['HMAT_present']=pathlib.Path('/sys/firmware/acpi/tables/HMAT').exists()
out['mode_inference']='UNKNOWN without positive mode evidence; missing sysfs information does not rule out HBM'
(r/'hbm_readonly_audit.json').write_text(json.dumps(out,indent=2))
if shutil.which('daxctl'):
 p=subprocess.run(['daxctl','list','-R','-D','-M'],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
 (r/'daxctl_list.json').write_text(json.dumps({'returncode':p.returncode,'stdout':p.stdout,'stderr':p.stderr},indent=2))
PY
env | sort > "$RUN/environment.txt"
cat /proc/sys/kernel/perf_event_paranoid > "$RUN/perf_event_paranoid.txt"
if command -v perf >/dev/null; then
 perf --version > "$RUN/perf_version.txt"
 perf list > "$RUN/perf_list.txt" 2>&1 || true
 if perf stat -e cycles,instructions,cache-misses -o "$RUN/perf_access.txt" true; then echo PASS > "$RUN/perf_access_status.txt";else echo DENIED_OR_UNSUPPORTED > "$RUN/perf_access_status.txt";fi
else echo UNAVAILABLE > "$RUN/perf_access_status.txt";fi
sha256sum "$BUILD/projection_methods" "$BUILD/paper_original" > "$RUN/binary.sha256"
cp "$BUILD/source.sha256" "$RUN/source.sha256"
python3 "$RUN/launcher_snapshot/projection_window_suite.py" "$RUN" "$BUILD" "$MODE" > "$RUN/suite.log" 2>&1
