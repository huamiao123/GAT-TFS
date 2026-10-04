#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
MODE=$1
SNAP="$ROOT/runs/paper-method-submit-$MODE-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$SNAP/launcher"
cp "$ROOT/scripts/paper_methods_job.sh" "$ROOT/scripts/paper_methods_suite.py" "$SNAP/launcher/"
cp "$ROOT/build/PAPER_METHOD_LATEST" "$SNAP/launcher/build_path.txt"
source "$ROOT/scripts/preflight.sh" "$SNAP"
cp "$ROOT/docs/PAPER_METHODS_PROTOCOL_20261004.md" "$SNAP/protocol.read.md"
if [ "$MODE" = smoke ]; then PART=intel;CPUS=4;EXCLUSIVE=();else PART=intel_expr;CPUS=32;EXCLUSIVE=(--exclusive);fi
cat > "$SNAP/launcher/job.slurm" <<EOF
#!/usr/bin/env bash
exec bash "$SNAP/launcher/paper_methods_job.sh" "$MODE"
EOF
sha256sum "$SNAP/launcher/"* > "$SNAP/launcher.sha256"
JOB=$(sbatch --parsable --partition="$PART" "${EXCLUSIVE[@]}" --nodes=1 --ntasks=1 --cpus-per-task="$CPUS" --time=02:00:00 --job-name="method-$MODE" --chdir="$ROOT" --output="$SNAP/slurm-%j.out" --error="$SNAP/slurm-%j.err" "$SNAP/launcher/job.slurm")
printf '%s\n' "$JOB" > "$SNAP/job_id.txt"
python3 - "$SNAP" "$JOB" "$MODE" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);e=dict(id=r.name,kind='controlled_paper_method_submission',status='SUBMITTED',job_id=sys.argv[2],mode=sys.argv[3],evidence=str(r),paper_eligible=False,next='Correctness before formal' if sys.argv[3]=='smoke' else 'Within-graph same-process controlled comparison; never mix nodes in speedup')
(r/'event.json').write_text(json.dumps(e,indent=2))
PY
python3 "$ROOT/scripts/record_event.py" "$SNAP/event.json"
printf 'METHOD_JOB=%s MODE=%s SNAPSHOT=%s\n' "$JOB" "$MODE" "$SNAP"
