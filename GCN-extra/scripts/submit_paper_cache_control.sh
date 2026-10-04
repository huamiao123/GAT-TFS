#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
MODE=$1
SNAP="$ROOT/runs/paper-cache-submit-$MODE-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$SNAP/launcher"
cp "$ROOT/scripts/paper_methods_job.sh" "$ROOT/scripts/paper_methods_suite.py" "$SNAP/launcher/"
cp "$ROOT/build/PAPER_CACHE_LATEST" "$SNAP/launcher/build_path.txt"
python3 - "$SNAP/launcher" <<'PY'
import pathlib,sys
p=pathlib.Path(sys.argv[1]);job=p/'paper_methods_job.sh';s=job.read_text().replace('paper-method-$MODE-','paper-cache-$MODE-').replace('GCN_PAPER_BLOCKS=2,4,8,16,32,64,full','GCN_PAPER_BLOCKS=8,16,64,full').replace("kind='controlled_paper_method_comparison'","kind='source_warm_cache_control'").replace('original source anchor and rotated method order','original source anchor and immediate warmup plus consecutive method repeats').replace('Reconcile all three disjoint graph batches against raw evidence','Reconcile cache control with retained interleaved measurements');job.write_text(s)
q=p/'paper_methods_suite.py';s=q.read_text().replace("assert len(groups)==(count+1)*2 and all(len(v)==5 for v in groups.values())","assert len(groups)==count+1 and all(k[1]=='e2e' and len(v)==5 for k,v in groups.items())");q.write_text(s)
PY
source "$ROOT/scripts/preflight.sh" "$SNAP"
if [ "$MODE" = smoke ]; then PART=intel;CPUS=4;EXCLUSIVE=();else PART=intel_expr;CPUS=32;EXCLUSIVE=(--exclusive);fi
cat > "$SNAP/launcher/job.slurm" <<EOF
#!/usr/bin/env bash
exec bash "$SNAP/launcher/paper_methods_job.sh" "$MODE"
EOF
sha256sum "$SNAP/launcher/"* > "$SNAP/launcher.sha256"
JOB=$(sbatch --parsable --partition="$PART" "${EXCLUSIVE[@]}" --nodes=1 --ntasks=1 --cpus-per-task="$CPUS" --time=02:00:00 --job-name="cache-$MODE" --chdir="$ROOT" --output="$SNAP/slurm-%j.out" --error="$SNAP/slurm-%j.err" "$SNAP/launcher/job.slurm")
printf '%s\n' "$JOB" > "$SNAP/job_id.txt"
python3 - "$SNAP" "$JOB" "$MODE" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);e=dict(id=r.name,kind='source_warm_cache_control_submission',status='SUBMITTED',job_id=sys.argv[2],mode=sys.argv[3],evidence=str(r),paper_eligible=False,issues='Same frozen kernels/gates/inputs/output precision; only measured execution order differs',next='Check consecutive vs interleaved sensitivity, retaining both')
(r/'event.json').write_text(json.dumps(e,indent=2))
PY
python3 "$ROOT/scripts/record_event.py" "$SNAP/event.json"
printf 'CACHE_JOB=%s MODE=%s\n' "$JOB" "$MODE"
