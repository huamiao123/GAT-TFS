#!/usr/bin/env bash
#SBATCH --job-name=gat-amx-audit
#SBATCH --partition=intel
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=64G
#SBATCH --time=02:00:00
#SBATCH --output=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/runs/amx-slurm-%j.out
#SBATCH --error=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/runs/amx-slurm-%j.err
set -euo pipefail
ROOT="${GAT_ROOT:-/home/huangjianqiang_group/hdacp1/data/wzh/GAT}"
RUN="$ROOT/runs/amx-arxiv-$SLURM_JOB_ID"
mkdir -p "$RUN" "$ROOT/build/tmp"
export TMPDIR="$ROOT/build/tmp"
RULES=/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md
SKILL=/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md
cat "$RULES" > "$RUN/AGENTS.read.txt"
cat "$SKILL" > "$RUN/SKILL.read.txt"
sha256sum "$RULES" "$SKILL" > "$RUN/preflight.sha256"
sha256sum "$ROOT"/src/*.cpp "$ROOT"/src/*.hpp \
  "$ROOT"/build/gat_tfs_online "$ROOT"/build/gat_tfs_online_speed \
  "$ROOT"/build/gat_tfs_amx "$ROOT"/build/gat_tfs_amx_speed \
  "$ROOT"/data/arxiv.gatbin "$ROOT/scripts/run_amx_arxiv.slurm" \
  > "$RUN/artifact.sha256"
module load "${GAT_COMPILER_MODULE:-intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp}"
module load "${GAT_MKL_MODULE:-intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3}"
export OMP_NUM_THREADS="${GAT_OMP_THREADS:-16}"
export MKL_NUM_THREADS="${GAT_MKL_THREADS:-16}"
export OMP_PROC_BIND=close OMP_PLACES=cores
BLOCK="${GAT_BLOCK_EDGES:-32}"
PANEL="${GAT_PANEL_R:-64}"
REPEATS="${GAT_SPEED_REPEATS:-2}"
{
  echo "job=$SLURM_JOB_ID"
  echo "node=$SLURMD_NODENAME"
  echo "partition=$SLURM_JOB_PARTITION"
  echo "cpus=$SLURM_CPUS_PER_TASK"
  echo "OMP_NUM_THREADS=$OMP_NUM_THREADS"
  echo "MKL_NUM_THREADS=$MKL_NUM_THREADS"
  echo "block=$BLOCK panel=$PANEL repeats=$REPEATS"
  echo "precision=amx_bf16_hi_lo_fp32_accumulation_fp32_gemm"
  echo "benchmark_boundary=three_layer_forward_only"
  echo "rep0=warmup"
} > "$RUN/manifest.txt"
"$ROOT/build/gat_tfs_online" "$ROOT/data/arxiv.gatbin" "$BLOCK" profile "$PANEL" \
  > "$RUN/fp32_correctness.log" 2>&1
"$ROOT/build/gat_tfs_amx" "$ROOT/data/arxiv.gatbin" "$BLOCK" "$PANEL" verify \
  > "$RUN/amx_correctness.log" 2>&1
for rep in $(seq 0 "$REPEATS"); do
  for path in panel_transform_first_fp32 tfs_online_fused; do
    "$ROOT/build/gat_tfs_online_speed" "$ROOT/data/arxiv.gatbin" "$BLOCK" benchmark "$PANEL" "$path" \
      > "$RUN/${path}_rep${rep}.log" 2>&1
  done
  "$ROOT/build/gat_tfs_amx_speed" "$ROOT/data/arxiv.gatbin" "$BLOCK" "$PANEL" benchmark \
    > "$RUN/amx_head_row_rep${rep}.log" 2>&1
done
