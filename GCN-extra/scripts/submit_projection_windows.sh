#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
MODE=$1
SNAP="$ROOT/runs/projection-window-submit-$MODE-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$SNAP/launcher"
cp "$ROOT/scripts/projection_window_job.sh" "$ROOT/scripts/projection_window_suite.py" "$SNAP/launcher/"
cp "$ROOT/build/PROJECTION_WINDOW_LATEST" "$SNAP/launcher/build_path.txt"
source "$ROOT/scripts/preflight.sh" "$SNAP"
cp "$ROOT/docs/PROJECTION_WINDOW_PROTOCOL_20261004.md" "$SNAP/protocol.read.md"
if [ "$MODE" = smoke ];then PART=intel;CPUS=4;LIMIT=00:10:00;else PART=intel;CPUS=32;LIMIT=01:30:00;fi
cat > "$SNAP/launcher/job.slurm" <<EOF
#!/usr/bin/env bash
exec srun --ntasks=1 --cpus-per-task=$CPUS --cpu-bind=cores --distribution=block:block --hint=nomultithread bash "$SNAP/launcher/projection_window_job.sh" "$MODE"
EOF
sha256sum "$SNAP/launcher/"* > "$SNAP/launcher.sha256"
JOB=$(sbatch --parsable --partition="$PART" --nodes=1 --ntasks=1 --cpus-per-task="$CPUS" --sockets-per-node=1 --cores-per-socket="$CPUS" --threads-per-core=1 --distribution=block:block --time="$LIMIT" --job-name="window-$MODE" --chdir="$ROOT" --output="$SNAP/slurm-%j.out" --error="$SNAP/slurm-%j.err" "$SNAP/launcher/job.slurm")
printf '%s\n' "$JOB" > "$SNAP/job_id.txt"
python3 - "$SNAP" "$JOB" "$MODE" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);e=dict(id=r.name,kind='projection_window_submission',status='SUBMITTED',job_id=sys.argv[2],mode=sys.argv[3],evidence=str(r),paper_eligible=False,issues='Shared intel node explicitly permitted by current user; one socket physical-core assertion; default NUMA no interleave',next='Correctness before formal' if sys.argv[3]=='smoke' else 'Same-process formal comparison; max two nodes')
(r/'event.json').write_text(json.dumps(e,indent=2))
PY
python3 "$ROOT/scripts/record_event.py" "$SNAP/event.json"
printf 'PROJECTION_JOB=%s MODE=%s SNAPSHOT=%s\n' "$JOB" "$MODE" "$SNAP"
