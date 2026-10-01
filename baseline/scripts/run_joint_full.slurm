#!/usr/bin/env bash
#SBATCH --job-name=gat-joint-full
#SBATCH --partition=intel
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=64G
#SBATCH --hint=nomultithread
#SBATCH --time=00:25:00
#SBATCH --output=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline/runs/joint-full-%j.out
#SBATCH --error=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline/runs/joint-full-%j.err
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline
RUN="$ROOT/runs/joint-full-$SLURM_JOB_ID"
mkdir -p "$RUN"
trap 'status=$?; echo "exit_status=$status" > "$RUN/status.txt"' EXIT
cat "$ROOT/../../AGENTS.md" > "$RUN/AGENTS.read.txt"
cat "$ROOT/../../skills/tfs-research-engineering/SKILL.md" > "$RUN/SKILL.read.txt"
sha256sum "$ROOT/../../AGENTS.md" "$ROOT/../../skills/tfs-research-engineering/SKILL.md" "$ROOT/IMPLEMENTATION_CONTRACT.md" > "$RUN/preflight.sha256"
sha256sum "$ROOT"/src/*.cpp "$ROOT/include/gat.hpp" "$ROOT"/ours/local_online.* "$ROOT"/ours/joint* "$ROOT/scripts/joint_full_payload.sh" "$ROOT/scripts/run_joint_full.slurm" > "$RUN/source.sha256"
sha256sum "$ROOT/build/joint_compare" "$ROOT/build/test_boundaries" "$ROOT/../data/arxiv.gatbin" "$ROOT/../data/products.gatbin" > "$RUN/artifact.sha256"
squeue -u hdacp1 > "$RUN/queue.txt"
sinfo -p intel > "$RUN/partition.txt"
scontrol show job "$SLURM_JOB_ID" > "$RUN/slurm_config.txt"
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3
export TMPDIR="$ROOT/build/tmp" OMP_PROC_BIND=close OMP_PLACES=cores MKL_DYNAMIC=FALSE
srun --ntasks=1 --cpus-per-task="$SLURM_CPUS_PER_TASK" --cpu-bind=cores bash "$ROOT/scripts/joint_full_payload.sh" "$RUN"
