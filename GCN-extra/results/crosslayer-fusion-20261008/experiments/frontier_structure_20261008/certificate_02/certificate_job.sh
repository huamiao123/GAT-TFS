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
e=dict(id='frontier-certificate-20261008-'+os.environ.get('SLURM_JOB_ID','unknown'),status='PASS' if s==0 and c.get('status')=='PASS' else 'FAILED',exit_code=s,evidence=str(r),completion=c,node=os.uname().nodename,job=os.environ.get('SLURM_JOB_ID'),partition=os.environ.get('SLURM_JOB_PARTITION'),paper_eligible=False,issues='Restricted state lower bound only; fixed node-ID order. No hardware speedup or unrestricted optimality claim.',next='Check Slurm and certificate PASS; compare verified lower bound to frozen constructive state peak, then decide whether complex state selection warrants a runtime.')
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
srun --ntasks=1 --cpus-per-task=1 --cpu-bind=cores python3 "$STUDY/source_snapshot/certificate_02/run_certificate.py" "$STUDY" "$RUN" > "$RUN/suite.log" 2>&1
