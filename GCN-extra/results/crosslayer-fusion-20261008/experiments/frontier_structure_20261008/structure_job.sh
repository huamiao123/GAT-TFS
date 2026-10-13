#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$1
RUN="$STUDY/compute-${SLURM_JOB_ID}"
mkdir "$RUN"
finish(){
 status=$?
 python3 - "$RUN" "$status" <<'PY'
import json,os,pathlib,sys
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2]);c=json.loads((r/'completion.json').read_text()) if (r/'completion.json').exists() else {}
e=dict(id='frontier-structure-20261008-'+os.environ.get('SLURM_JOB_ID','unknown'),status='PASS' if s==0 else 'FAILED',exit_code=s,evidence=str(r),completion=c,node=os.uname().nodename,job=os.environ.get('SLURM_JOB_ID'),partition=os.environ.get('SLURM_JOB_PARTITION'),paper_eligible=False,issues='Graph-only diagnostic; no timing/performance interpretation. All malformed fixtures retained as expected rejects, any unexpected failure stops interpretation.',next='Independently analyze real graph state/resumption tradeoffs; compare ID and simple delayed consumers before developing a new kernel.')
(r/'event.json').write_text(json.dumps(e,indent=2)+'\n');(r/'exit_status.txt').write_text(str(s)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
export TMPDIR="$RUN/tmp"
mkdir "$TMPDIR"
export OMP_NUM_THREADS=1 OMP_PROC_BIND=close OMP_PLACES=cores PYTHONDONTWRITEBYTECODE=1
ulimit -c 0
hostname > "$RUN/node.txt"
lscpu > "$RUN/cpu.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1
python3 - "$RUN" <<'PY'
import json,os,pathlib,sys
r=pathlib.Path(sys.argv[1]);keys=['OMP_NUM_THREADS','OMP_PROC_BIND','OMP_PLACES','SLURM_JOB_ID','SLURM_JOB_PARTITION','SLURM_CPUS_PER_TASK','TMPDIR']
(r/'environment.json').write_text(json.dumps({k:os.environ.get(k) for k in keys},indent=2))
PY
srun --ntasks=1 --cpus-per-task=1 --cpu-bind=cores python3 "$STUDY/source_snapshot/run_structure.py" "$STUDY" "$RUN" > "$RUN/suite.log" 2>&1
