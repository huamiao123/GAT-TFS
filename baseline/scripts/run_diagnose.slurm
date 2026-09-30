#!/usr/bin/env bash
#SBATCH --job-name=gat-b0-diagnose
#SBATCH --partition=intel
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=16G
#SBATCH --hint=nomultithread
#SBATCH --time=00:15:00
#SBATCH --output=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline/runs/diagnose-%j.out
#SBATCH --error=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline/runs/diagnose-%j.err
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline
RUN="$ROOT/runs/diagnose-$SLURM_JOB_ID"
mkdir -p "$RUN"
trap 'status=$?; echo "exit_status=$status" > "$RUN/status.txt"' EXIT
RULES=/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md
SKILL=/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md
cat "$RULES" > "$RUN/AGENTS.read.txt"
cat "$SKILL" > "$RUN/SKILL.read.txt"
sha256sum "$RULES" "$SKILL" "$ROOT/IMPLEMENTATION_CONTRACT.md" > "$RUN/preflight.sha256"
find "$ROOT/src" "$ROOT/include" "$ROOT/bench" "$ROOT/scripts" -type f -exec sha256sum {} + > "$RUN/source.sha256"
sha256sum "$ROOT/build/gat_baseline" "$ROOT/build/test_boundaries" "$ROOT/../data/arxiv.gatbin" > "$RUN/artifact.sha256"
squeue -u hdacp1 > "$RUN/queue.txt"
sinfo -p intel > "$RUN/partition.txt"
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3
export TMPDIR="$ROOT/build/tmp" OMP_PROC_BIND=close OMP_PLACES=cores MKL_DYNAMIC=FALSE
srun --ntasks=1 --cpus-per-task="$SLURM_CPUS_PER_TASK" --cpu-bind=cores bash "$ROOT/scripts/diagnose_payload.sh" "$RUN"
