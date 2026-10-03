#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
MODE=$1
RUN="$ROOT/runs/$MODE-$SLURM_JOB_ID"
mkdir -p "$RUN"
finish() {
    status=$?
    printf 'exit_status=%s\n' "$status" > "$RUN/status.txt"
    python3 - "$RUN" "$MODE" "$status" <<'PY'
import json, os, pathlib, socket, sys
r=pathlib.Path(sys.argv[1]);mode=sys.argv[2];status=int(sys.argv[3])
event={'id':r.name,'kind':mode,'status':'PASS' if status==0 else 'FAILED','exit_status':status,'job_id':os.getenv('SLURM_JOB_ID'),'partition':os.getenv('SLURM_JOB_PARTITION'),'node':socket.gethostname(),'threads':os.getenv('SLURM_CPUS_PER_TASK'),'evidence':str(r),'timing_boundary':'kernel includes original C-zero/AMX setup/output; prepared E2E includes 2 kernels+ReLU+intermediate BF16 conversion','warmups':1,'repeats':2 if mode=='smoke' else 5,'source_binary_hashes':'artifact.sha256 and build_source.sha256','input_hashes':'dataset.sha256 plus per-graph info.json','correctness':'per-graph correctness.csv; gate before timing','results':'SUMMARY.md and per-graph timings/work/profiles CSV','paper_eligible':False,'issues':'See stderr/status; sampled phase timings are advisory and sum thread time, not wall','next':'Inspect accuracy and speedup; full 25 graphs and shape sensitivity remain pending'}
(r/'event.json').write_text(json.dumps(event,indent=2))
PY
    python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
    echo "RUN=$RUN status=$status"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
BUILD=$(cat "$ROOT/build/LATEST")
case "$BUILD" in "$ROOT"/runs/build-*) ;; *) exit 3;; esac
cp -r "$BUILD/source_snapshot" "$RUN/"
cp "$BUILD/source.sha256" "$RUN/build_source.sha256"
sha256sum "$BUILD/gcn_bench" "$ROOT/scripts/job.sh" "$ROOT/scripts/payload.sh" > "$RUN/artifact.sha256"
printf '%s\n' "$BUILD" > "$RUN/build_path.txt"
scontrol show job "$SLURM_JOB_ID" > "$RUN/slurm_config.txt"
srun --ntasks=1 --cpus-per-task="$SLURM_CPUS_PER_TASK" --distribution=block:block --cpu-bind=verbose,cores bash "$ROOT/scripts/payload.sh" "$RUN" "$MODE" "$BUILD"
