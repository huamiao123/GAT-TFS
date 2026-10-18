#!/bin/bash
#SBATCH --job-name=gcn_c4_mask_17
#SBATCH --partition=intel
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=16G
#SBATCH --time=01:30:00
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
EXP="$ROOT/experiments/consumer_mask_c4_20261018"
test -n "${RUN:?RUN required}"
[[ "$RUN" == "$ROOT"/runs/consumer_mask_c4_20261018-* ]]
export OMP_NUM_THREADS=1
export OPENBLAS_NUM_THREADS=1
export MKL_NUM_THREADS=1
printf '%s\n' "$SLURM_JOB_ID" > "$RUN/jobid.txt"
hostname > "$RUN/node.txt"
date --iso-8601=seconds > "$RUN/start_time.txt"
if python3 "$EXP/run_scan.py" "$ROOT" "$RUN" "$EXP/mask_scan"; then
    printf 'PASS\n' > "$RUN/status.txt"
else
    code=$?
    printf 'FAILED exit=%s\n' "$code" > "$RUN/status.txt"
    exit "$code"
fi
date --iso-8601=seconds > "$RUN/end_time.txt"
