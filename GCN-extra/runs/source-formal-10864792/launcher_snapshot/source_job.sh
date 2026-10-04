#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
MODE=$1
RUN="$ROOT/runs/source-$MODE-$SLURM_JOB_ID"
mkdir -p "$RUN"
ulimit -c 0
finish() {
    status=$?
    python3 - "$RUN" "$MODE" "$status" <<'PY'
import json,pathlib,os,socket,sys
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[3]);cases=json.loads((r/'suite_status.json').read_text()) if (r/'suite_status.json').exists() else []
bad=[x for x in cases if x['status']=='FAILED'];e={'id':r.name,'kind':'source_protocol_'+sys.argv[2],'status':'PASS' if s==0 and not bad else 'FAILED','exit_status':s,'datasets':cases,'job_id':os.getenv('SLURM_JOB_ID'),'node':socket.gethostname(),'threads':os.getenv('SLURM_CPUS_PER_TASK'),'partition':os.getenv('SLURM_JOB_PARTITION'),'warmups':1,'repeats':5,'statistic':'source min-of-five, median secondary','timing_boundary':'literal original prepared two-layer source MKL/TFS; separate candidate stages','evidence':str(r),'paper_eligible':False,'issues':bad or 'No new major issue; all methods use default NUMA policy, no explicit MKL thread controls','next':'Compare original source MKL, source TFS and candidates; trained-model accuracy still unverified'}
(r/'event.json').write_text(json.dumps(e,indent=2));(r/'status.txt').write_text('exit_status='+str(s)+'\n')
PY
    python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
    printf 'SOURCE_RUN=%s status=%s\n' "$RUN" "$status"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
BUILD=$(cat "$ROOT/build/SOURCE_PROTOCOL_LATEST")
printf '%s\n' "$BUILD" > "$RUN/build_path.txt"
cp "$BUILD/source.sha256" "$RUN/build_source.sha256"
cp "$BUILD/binary.sha256" "$RUN/binary.sha256"
cp -r "$BUILD/source_snapshot" "$RUN/source_snapshot"
mkdir -p "$RUN/launcher_snapshot"
cp "$ROOT/scripts/source_protocol_suite.py" "$ROOT/scripts/source_job.sh" "$ROOT/scripts/source_$MODE.slurm" "$RUN/launcher_snapshot/"
sha256sum "$RUN/launcher_snapshot/"* > "$RUN/launcher_source.sha256"
cp "$ROOT/docs/SOURCE_MKL_PROTOCOL_20261004.md" "$RUN/source_protocol.read.md"
python3 - "$RUN" <<'PY'
import json,pathlib,os,sys
r=pathlib.Path(sys.argv[1]);keys=['OMP_NUM_THREADS','OMP_PROC_BIND','OMP_PLACES','OMP_DYNAMIC','MKL_NUM_THREADS','MKL_DYNAMIC','MKL_DOMAIN_NUM_THREADS','KMP_AFFINITY']
(r/'inherited_thread_env.json').write_text(json.dumps({k:os.environ.get(k) for k in keys},indent=2))
PY
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS OMP_DYNAMIC KMP_AFFINITY
export OMP_NUM_THREADS="$SLURM_CPUS_PER_TASK" OMP_PROC_BIND=close OMP_PLACES=cores
hostname > "$RUN/node.txt"
lscpu > "$RUN/lscpu.txt"
lscpu -e=CPU,CORE,SOCKET,NODE,ONLINE > "$RUN/cpu_layout.txt"
numactl --show > "$RUN/numa_policy_observed.txt" 2>&1 || true
numactl --hardware > "$RUN/numa_hardware.txt"
cat /proc/self/status > "$RUN/process_status.txt"
cat /proc/meminfo > "$RUN/meminfo.txt"
python3 - "$RUN" <<'PY'
import json,pathlib,os,sys
r=pathlib.Path(sys.argv[1]);keys=['OMP_NUM_THREADS','OMP_PROC_BIND','OMP_PLACES','OMP_DYNAMIC','MKL_NUM_THREADS','MKL_DYNAMIC','MKL_DOMAIN_NUM_THREADS','KMP_AFFINITY']
(r/'thread_env.json').write_text(json.dumps({k:os.environ.get(k) for k in keys},indent=2));(r/'affinity.json').write_text(json.dumps({'allowed_cpus':sorted(os.sched_getaffinity(0))},indent=2))
PY
export GCN_EXTRA_ROOT="$ROOT"
python3 "$RUN/launcher_snapshot/source_protocol_suite.py" "$RUN" "$BUILD" "$MODE"
