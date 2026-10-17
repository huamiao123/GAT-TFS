#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
SRC="$ROOT/experiments/h_panel_reuse_20261014"
BUILD=$(cat "$SRC/LATEST_BUILD")
RUN="$ROOT/runs/h-panel-reuse-reverse-${SLURM_JOB_ID:-manual}"
mkdir -p "$RUN/fixtures" "$RUN/logs" "$RUN/results" "$RUN/source_snapshot"
cp "$BUILD/source_snapshot/"* "$RUN/source_snapshot/"
cp "$SRC/suite.py" "$SRC/reverse_run.sh" "$SRC/reverse.slurm" "$RUN/source_snapshot/"
source "$ROOT/scripts/preflight.sh" "$RUN"
export OMP_NUM_THREADS=${SLURM_CPUS_PER_TASK:-32}
export OMP_PROC_BIND=close OMP_PLACES=cores
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS OMP_DYNAMIC KMP_AFFINITY GOMP_CPU_AFFINITY
ulimit -c 0
hostname > "$RUN/node.txt"
lscpu > "$RUN/cpu.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1
env | sort > "$RUN/environment.txt"
sha256sum "$BUILD/h_panel_reuse" "$RUN/source_snapshot/"* > "$RUN/hashes.sha256"
python3 "$RUN/source_snapshot/suite.py" "$RUN" "$BUILD/h_panel_reuse" reverse > "$RUN/suite.log" 2>&1
