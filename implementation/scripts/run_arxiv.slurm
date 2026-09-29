#!/usr/bin/env bash
#SBATCH --job-name=gat-online
#SBATCH --partition=intel
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=64G
#SBATCH --time=02:00:00
#SBATCH --output=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/runs/slurm-%j.out
#SBATCH --error=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/runs/slurm-%j.err
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT
RUN="$ROOT/runs/arxiv-$SLURM_JOB_ID"
mkdir -p "$RUN" "$ROOT/build/tmp"
export TMPDIR="$ROOT/build/tmp"
export MKL_NUM_THREADS=16
export OMP_NUM_THREADS=16
cat /home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md > "$RUN/AGENTS.read.txt"
cat /home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md > "$RUN/SKILL.read.txt"
sha256sum /home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md /home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md > "$RUN/preflight.sha256"
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3
icpx --version > "$RUN/compiler.txt"
sha256sum "$ROOT/src/gat_online.cpp" "$ROOT/tools/prepare_arxiv.py"  "$ROOT/build/gat_online" "$ROOT/build/gat_online_speed" "$ROOT/scripts/run_arxiv.slurm" > "$RUN/artifact.sha256"
{
  echo "job=$SLURM_JOB_ID"
  echo "node=$SLURMD_NODENAME"
  echo "partition=$SLURM_JOB_PARTITION"
  echo "cpus=$SLURM_CPUS_PER_TASK"
  echo "MKL_NUM_THREADS=$MKL_NUM_THREADS"
  echo "OMP_NUM_THREADS=$OMP_NUM_THREADS"
  echo "block=$(printenv BLOCK_EDGES || echo 32)"
} > "$RUN/manifest.txt"
python3 "$ROOT/tools/prepare_arxiv.py" \
  --source /home/huangjianqiang_group/hdacp1/data/wzh/TFS-Train/runs/c5_fullstep_arxiv_20260804_v3/dataset/arxiv \
  --output "$ROOT/data/arxiv.gatbin" | tee "$RUN/prepare.log"
"$ROOT/build/gat_online" "$ROOT/data/arxiv.gatbin" "$(printenv BLOCK_EDGES || echo 32)" "$RUN/arxiv" | tee "$RUN/benchmark.log"

for rep in 0 1 2 3; do
  "$ROOT/build/gat_online_speed" "$ROOT/data/arxiv.gatbin" "$(printenv BLOCK_EDGES || echo 32)" "$RUN/arxiv_speed_rep$rep" | tee "$RUN/benchmark_speed_rep$rep.log"
done
