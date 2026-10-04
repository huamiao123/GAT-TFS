#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
SNAPSHOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
MODE=$1
BUILD=$(cat "$SNAPSHOT/build_path.txt")
RUN="$ROOT/runs/paper-method-$MODE-${SLURM_JOB_ID}"
mkdir -p "$RUN/launcher_snapshot"
cp "$SNAPSHOT/"* "$RUN/launcher_snapshot/"
finish(){
 status=$?
 python3 - "$RUN" "$status" "$BUILD" <<'PY'
import pathlib,json,sys,os
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2]);c=json.loads((r/'completion.json').read_text()) if (r/'completion.json').exists() else {}
e=dict(id=r.name,kind='controlled_paper_method_comparison',status='PASS' if s==0 else 'FAILED',exit_status=s,job_id=os.environ.get('SLURM_JOB_ID'),node=os.uname().nodename,build=sys.argv[3],evidence=str(r),completion=c,paper_eligible='smoke' not in r.name and s==0,issues='Same graph/tensors/thread binding/NUMA policy/output precision/timing; original source anchor and rotated method order; frozen numerical gates',next='Reconcile all three disjoint graph batches against raw evidence')
(r/'event.json').write_text(json.dumps(e,indent=2));(r/'exit_status.txt').write_text(str(s)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$ROOT/docs/PAPER_METHODS_PROTOCOL_20261004.md" "$RUN/protocol.read.md"
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS OMP_DYNAMIC KMP_AFFINITY GOMP_CPU_AFFINITY
export OMP_NUM_THREADS=${SLURM_CPUS_PER_TASK}
export OMP_PROC_BIND=close OMP_PLACES=cores GCN_PAPER_BLOCKS=2,4,8,16,32,64,full
ulimit -c 0
hostname > "$RUN/node.txt"
lscpu > "$RUN/cpu.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1
numactl --hardware > "$RUN/numa_hardware.txt"
env | sort > "$RUN/environment.txt"
sha256sum "$BUILD/paper_methods" "$BUILD/paper_original" > "$RUN/binary.sha256"
cp "$BUILD/source.sha256" "$RUN/source.sha256"
python3 "$RUN/launcher_snapshot/paper_methods_suite.py" "$RUN" "$BUILD" "$MODE" > "$RUN/suite.log" 2>&1
