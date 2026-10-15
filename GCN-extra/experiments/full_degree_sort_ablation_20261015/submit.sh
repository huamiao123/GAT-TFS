#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$(cat "$ROOT/build/FULL_DEGREE_SORT_LATEST")
MODE=${1:?smoke or real}
EXP=$ROOT/experiments/full_degree_sort_ablation_20261015
RUN="$STUDY/logs/$MODE-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN"
if [[ $MODE == smoke ]];then CPUS=4;LIMIT=00:10:00;MEM=16000M
elif [[ $MODE == validation_large ]];then CPUS=32;LIMIT=00:30:00;MEM=128000M
elif [[ $MODE == validation_small ]];then CPUS=32;LIMIT=01:00:00;MEM=64000M
else CPUS=32;LIMIT=00:30:00;MEM=64000M;fi
NODE_ARGS=()
if [[ $MODE == web_repeat ]];then NODE_ARGS=(--nodelist=qhcn251);fi
JOB=$(sbatch --parsable "${NODE_ARGS[@]}" --partition=intel --nodes=1 --ntasks=1 \
  --cpus-per-task="$CPUS" --sockets-per-node=1 --cores-per-socket="$CPUS" \
  --mem="$MEM" \
  --threads-per-core=1 --hint=nomultithread --distribution=block:block \
  --time="$LIMIT" --job-name="full-sort-$MODE" --chdir="$ROOT" \
  --output="$RUN/slurm-%j.out" --error="$RUN/slurm-%j.err" \
  "$EXP/job.sh" "$RUN" "$MODE")
printf '%s\n' "$JOB" > "$RUN/job_id.txt"
printf 'JOB=%s RUN=%s\n' "$JOB" "$RUN"
