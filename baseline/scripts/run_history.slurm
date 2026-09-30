#!/usr/bin/env bash
#SBATCH --job-name=gat-history-compare
#SBATCH --partition=intel
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=64G
#SBATCH --hint=nomultithread
#SBATCH --time=00:30:00
#SBATCH --output=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline/runs/history-%j.out
#SBATCH --error=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline/runs/history-%j.err
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline
RUN="$ROOT/runs/history-$SLURM_JOB_ID"
mkdir -p "$RUN"
trap 'status=$?; echo "exit_status=$status" > "$RUN/status.txt"' EXIT
RULES="$ROOT/../../AGENTS.md"
SKILL="$ROOT/../../skills/tfs-research-engineering/SKILL.md"
cat "$RULES" > "$RUN/AGENTS.read.txt"
cat "$SKILL" > "$RUN/SKILL.read.txt"
sha256sum "$RULES" "$SKILL" "$ROOT/IMPLEMENTATION_CONTRACT.md" > "$RUN/preflight.sha256"
sha256sum "$ROOT"/src/*.cpp "$ROOT/include/gat.hpp" "$ROOT/bench/compare_history.cpp" "$ROOT/../src/gat_online.cpp" "$ROOT/../src/gat_tfs_online.cpp" "$ROOT/scripts/history_payload.sh" "$ROOT/scripts/run_history.slurm" > "$RUN/source.sha256"
sha256sum "$ROOT/build/compare_history" "$ROOT/build/gat_baseline" "$ROOT/build/test_boundaries" "$ROOT/../data/arxiv.gatbin" "$ROOT/../data/products.gatbin" > "$RUN/artifact.sha256"
squeue -u hdacp1 > "$RUN/queue.txt"
sinfo -p intel > "$RUN/partition.txt"
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3
export TMPDIR="$ROOT/build/tmp" OMP_PROC_BIND=close OMP_PLACES=cores MKL_DYNAMIC=FALSE
srun --ntasks=1 --cpus-per-task="$SLURM_CPUS_PER_TASK" --cpu-bind=cores bash "$ROOT/scripts/history_payload.sh" "$RUN"
