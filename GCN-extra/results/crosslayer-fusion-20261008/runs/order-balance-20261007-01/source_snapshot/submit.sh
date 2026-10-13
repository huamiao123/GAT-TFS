#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$1
MODE=$2
CORES=32
if [ "$MODE" = smoke ]; then CORES=4; fi
JOB=$(sbatch --parsable --partition=intel --nodes=1 --ntasks=1 --cpus-per-task="$CORES" --sockets-per-node=1 --cores-per-socket="$CORES" --threads-per-core=1 --distribution=block:block --time=01:00:00 --job-name="residual-$MODE" --chdir="$ROOT" --output="$STUDY/logs/slurm-%j.out" --error="$STUDY/logs/slurm-%j.err" --wrap="RUN=$STUDY/logs/$MODE-\$SLURM_JOB_ID; mkdir -p \$RUN; srun --ntasks=1 --cpus-per-task=$CORES --cpu-bind=cores --hint=nomultithread bash $STUDY/source_snapshot/job.sh \$RUN $STUDY $MODE")
python3 - "$STUDY" "$MODE" "$JOB" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);e=dict(id=r.name+'-'+sys.argv[2]+'-submit-'+sys.argv[3],kind='residual_broad_submission',status='SUBMITTED',study=str(r),mode=sys.argv[2],job=sys.argv[3],paper_eligible=False,issues='Shared single-socket physical cores, default NUMA',next='Numerical gates before accepted timing')
p=r/'logs'/('submission-'+sys.argv[3]+'.json');p.write_text(json.dumps(e,indent=2)+'\n');print(str(p))
PY
python3 "$ROOT/scripts/record_event.py" "$STUDY/logs/submission-$JOB.json"
printf 'JOB=%s MODE=%s\n' "$JOB" "$MODE"
