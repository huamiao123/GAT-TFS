#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$(cat "$ROOT/build/FEATURE_PRECISION_LATEST")
MODE=$1
SUB="$STUDY/logs/submit-$MODE-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$SUB"
source "$ROOT/scripts/preflight.sh" "$SUB"
if [ "$MODE" = smoke ];then CPUS=4;LIMIT=00:10:00;else CPUS=32;LIMIT=01:30:00;fi
cat > "$SUB/job.slurm" <<EOF
#!/usr/bin/env bash
exec srun --ntasks=1 --cpus-per-task=$CPUS --cpu-bind=cores --distribution=block:block --hint=nomultithread bash "$STUDY/source_snapshot/feature_precision_job.sh" "$STUDY" "$MODE"
EOF
JOB=$(sbatch --parsable --partition=intel --nodes=1 --ntasks=1 --cpus-per-task="$CPUS" --sockets-per-node=1 --cores-per-socket="$CPUS" --threads-per-core=1 --distribution=block:block --time="$LIMIT" --job-name="feature-$MODE" --chdir="$ROOT" --output="$SUB/slurm-%j.out" --error="$SUB/slurm-%j.err" "$SUB/job.slurm")
printf '%s\n' "$JOB" > "$SUB/job_id.txt"
python3 - "$SUB" "$JOB" "$MODE" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);e=dict(id=r.parent.parent.name+'-'+r.name,kind='feature_precision_submission',status='SUBMITTED',job_id=sys.argv[2],mode=sys.argv[3],evidence=str(r),paper_eligible=False,issues='User explicitly authorizes shared intel node; 32 physical cores on one socket verified in job',next='Correctness before real-graph timing')
(r/'event.json').write_text(json.dumps(e,indent=2))
PY
python3 "$ROOT/scripts/record_event.py" "$SUB/event.json"
printf 'FEATURE_PRECISION_JOB=%s MODE=%s STUDY=%s SUB=%s\n' "$JOB" "$MODE" "$STUDY" "$SUB"
