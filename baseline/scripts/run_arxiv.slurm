#!/usr/bin/env bash
#SBATCH --job-name=gat-baseline-arxiv
#SBATCH --partition=intel
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=32G
#SBATCH --hint=nomultithread
#SBATCH --time=00:30:00
#SBATCH --output=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline/runs/arxiv-%j.out
#SBATCH --error=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline/runs/arxiv-%j.err
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline
RUN="$ROOT/runs/arxiv-$SLURM_JOB_ID"
mkdir -p "$RUN"
trap 'status=$?; printf "exit_status=%s\n" "$status" > "$RUN/status.txt"' EXIT
RULES=/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md
SKILL=/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md
cat "$RULES" > "$RUN/AGENTS.read.txt"
cat "$SKILL" > "$RUN/SKILL.read.txt"
sha256sum "$RULES" "$SKILL" "$ROOT/IMPLEMENTATION_CONTRACT.md" > "$RUN/preflight.sha256"
find "$ROOT/src" "$ROOT/include" "$ROOT/tests" "$ROOT/bench" "$ROOT/scripts" -type f -exec sha256sum {} + > "$RUN/source.sha256"
sha256sum "$ROOT"/build/gat_baseline* "$ROOT/build/test_boundaries" "$ROOT/build/baseline_tfs.s" "$ROOT/../data/arxiv.gatbin" > "$RUN/artifact.sha256"
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3
export TMPDIR="$ROOT/build/tmp" PYTHONDONTWRITEBYTECODE=1
export OMP_PROC_BIND=close OMP_PLACES=cores MKL_DYNAMIC=FALSE
srun --ntasks=1 --cpus-per-task="$SLURM_CPUS_PER_TASK" --cpu-bind=cores bash "$ROOT/scripts/arxiv_payload.sh" "$RUN"
