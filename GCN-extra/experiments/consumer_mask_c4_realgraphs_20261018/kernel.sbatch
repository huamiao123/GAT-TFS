#!/bin/bash
#SBATCH --job-name=gcn_c4_real_x
#SBATCH --partition=intel_expr
#SBATCH --nodes=1
#SBATCH --exclusive
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --mem=64G
#SBATCH --time=02:00:00
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
EXP="$ROOT/experiments/consumer_mask_c4_realgraphs_20261018"
[[ "${RUN:?RUN required}" == "$ROOT"/runs/consumer_mask_c4_realgraphs_20261018-* ]]
export OMP_NUM_THREADS=32
export OMP_PROC_BIND=close
export OMP_PLACES=cores
export OPENBLAS_NUM_THREADS=1
printf '%s\n' "$SLURM_JOB_ID" > "$RUN/jobid.txt"
hostname > "$RUN/node.txt"
date --iso-8601=seconds > "$RUN/start_time.txt"
lscpu > "$RUN/lscpu.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1 || true
if python3 "$EXP/run_micro.py" "$ROOT" "$RUN" "$EXP/mask_kernel"; then
  printf 'PASS\n' > "$RUN/status.txt"
else
  code=$?;printf 'FAILED exit=%s\n' "$code" > "$RUN/status.txt";exit "$code"
fi
date --iso-8601=seconds > "$RUN/end_time.txt"
