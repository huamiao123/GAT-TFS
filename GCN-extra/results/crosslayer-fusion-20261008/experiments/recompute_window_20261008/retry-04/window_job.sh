#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$1
RUN="$STUDY/compute-${SLURM_JOB_ID}"
mkdir "$RUN"
finish(){
  s=$?
  python3 - "$RUN" "$s" <<'PY'
import json,os,pathlib,sys
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2])
c=json.loads((r/'result/audit.json').read_text()) if (r/'result/audit.json').exists() else {}
event=dict(id='recompute-window-20261008-'+os.environ.get('SLURM_JOB_ID','unknown'),
 status='PASS' if s==0 and c.get('status')=='PASS' else 'FAILED', exit_code=s,
 evidence=str(r), completion=c, node=os.uname().nodename,
 job=os.environ.get('SLURM_JOB_ID'), paper_eligible=False,
 issues='Logical source reuse-window audit only; no speedup or DRAM claim.',
 next='Compare W tradeoffs with row-complete baseline; develop no kernel if AMX or AVX work erases capacity benefit.')
(r/'event.json').write_text(json.dumps(event,indent=2)+'\n')
(r/'exit_status.txt').write_text(str(s)+'\n')
PY
  python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
export TMPDIR="$RUN/tmp"
mkdir "$TMPDIR"
export OMP_NUM_THREADS=1 OMP_PROC_BIND=close OMP_PLACES=cores PYTHONDONTWRITEBYTECODE=1
hostname > "$RUN/node.txt"
lscpu > "$RUN/cpu.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1
srun --ntasks=1 --cpus-per-task=1 --cpu-bind=cores python3 "$STUDY/source_snapshot/run.py" "$STUDY" "$RUN" > "$RUN/suite.log" 2>&1
