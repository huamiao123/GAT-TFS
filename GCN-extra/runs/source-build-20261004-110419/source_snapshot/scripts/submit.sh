#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
MODE=$1
case "$MODE" in smoke|formal|suite) ;; *) exit 3;; esac
RUN="$ROOT/runs/submit-$MODE-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN"
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$ROOT/scripts/$MODE.slurm" "$RUN/"
sha256sum "$ROOT/scripts/$MODE.slurm" "$ROOT/scripts/job.sh" "$ROOT/scripts/payload.sh" > "$RUN/source.sha256"
JOB=$(sbatch --parsable --chdir="$ROOT" "$ROOT/scripts/$MODE.slurm")
printf '%s\n' "$JOB" > "$RUN/job_id.txt"
python3 - "$RUN" "$JOB" "$MODE" <<'PY'
import json,pathlib,sys
r=pathlib.Path(sys.argv[1]);event={'id':r.name,'kind':'submission','status':'SUBMITTED','job_id':sys.argv[2],'mode':sys.argv[3],'nodes':1,'evidence':str(r),'paper_eligible':False,'issues':'no new major issue','next':'Collect job status and correctness before reporting speedup'}
(r/'event.json').write_text(json.dumps(event,indent=2))
PY
python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
printf 'JOB_ID=%s\n' "$JOB"
