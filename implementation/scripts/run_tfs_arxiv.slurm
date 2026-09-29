#!/usr/bin/env bash
#SBATCH --job-name=gat-tfs-online
#SBATCH --partition=intel
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=64G
#SBATCH --time=02:00:00
#SBATCH --output=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/runs/tfs-slurm-%j.out
#SBATCH --error=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/runs/tfs-slurm-%j.err
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT
RUN="$ROOT/runs/tfs-arxiv-$SLURM_JOB_ID"
mkdir -p "$RUN" "$ROOT/build/tmp"
export TMPDIR="$ROOT/build/tmp"
export OMP_NUM_THREADS="$(printenv TFS_OMP_THREADS || echo 16)"
export MKL_NUM_THREADS="$(printenv TFS_MKL_THREADS || echo 16)"
export OMP_PROC_BIND=close
export OMP_PLACES=cores
cat /home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md > "$RUN/AGENTS.read.txt"
cat /home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md > "$RUN/SKILL.read.txt"
sha256sum /home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md /home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md > "$RUN/preflight.sha256"
sha256sum "$ROOT/src/gat_online.cpp" "$ROOT/src/gat_tfs_online.cpp" "$ROOT/build/gat_tfs_online" "$ROOT/build/gat_tfs_online_speed" "$ROOT/data/arxiv.gatbin" "$ROOT/scripts/run_tfs_arxiv.slurm" > "$RUN/artifact.sha256"
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3
{
  echo "job=$SLURM_JOB_ID"
  echo "node=$SLURMD_NODENAME"
  echo "partition=$SLURM_JOB_PARTITION"
  echo "cpus=$SLURM_CPUS_PER_TASK"
  echo "OMP_NUM_THREADS=$OMP_NUM_THREADS"
  echo "MKL_NUM_THREADS=$MKL_NUM_THREADS"
  echo "BLOCK_EDGES=$(printenv BLOCK_EDGES || echo 32)"
  echo "PANEL_R=$(printenv PANEL_R || echo auto)"
  echo "SPEED_REPEATS=$(printenv SPEED_REPEATS || echo 2)"
} > "$RUN/manifest.txt"
"$ROOT/build/gat_tfs_online" "$ROOT/data/arxiv.gatbin" "$(printenv BLOCK_EDGES || echo 32)" profile "$(printenv PANEL_R || echo auto)" | tee "$RUN/profile.log"
for rep in $(seq 0 "$(printenv SPEED_REPEATS || echo 2)"); do
  "$ROOT/build/gat_tfs_online_speed" "$ROOT/data/arxiv.gatbin" "$(printenv BLOCK_EDGES || echo 32)" speed "$(printenv PANEL_R || echo auto)" | tee "$RUN/speed_rep$rep.log"
done

