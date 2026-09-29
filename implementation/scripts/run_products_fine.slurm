#!/usr/bin/env bash
#SBATCH --job-name=gat-fine-timing
#SBATCH --partition=intel
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=128G
#SBATCH --time=01:30:00
#SBATCH --output=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/runs/fine-slurm-%j.out
#SBATCH --error=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/runs/fine-slurm-%j.err
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT
RUN="$ROOT/runs/fine-timing-$SLURM_JOB_ID"
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
  "$ROOT"/data/smoke1024.gatbin "$ROOT"/data/arxiv.gatbin \
  "$ROOT"/data/products.gatbin "$ROOT/scripts/run_products_fine.slurm" \
  > "$RUN/artifact.sha256"
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3
export OMP_NUM_THREADS=16 MKL_NUM_THREADS=16
export OMP_PROC_BIND=close OMP_PLACES=cores
{
  echo "job=$SLURM_JOB_ID node=$SLURMD_NODENAME partition=$SLURM_JOB_PARTITION"
  echo "cpus=$SLURM_CPUS_PER_TASK OMP=$OMP_NUM_THREADS MKL=$MKL_NUM_THREADS"
  echo "datasets=smoke1024,ogbn-arxiv,ogbn-products block=32 panel=64"
  echo "fine_profile=worker_accumulated_times plus layer/parallel wall; speed=timers_disabled"
  echo "benchmark_boundary=three_layer_forward_only rep0=warmup rep1_rep2=measured"
  echo "precision_gate=max_abs_le_0.003_and_relative_L2_le_1e-4"
} > "$RUN/manifest.txt"
"$ROOT/build/gat_tfs_online" "$ROOT/data/smoke1024.gatbin" 32 profile 64 \
  > "$RUN/smoke_fp32.log" 2>&1
"$ROOT/build/gat_tfs_amx" "$ROOT/data/smoke1024.gatbin" 32 64 verify \
  > "$RUN/smoke_amx.log" 2>&1
for dataset in arxiv products; do
  graph="$ROOT/data/${dataset}.gatbin"
  set +e
  "$ROOT/build/gat_tfs_online" "$graph" 32 profile 64 \
    > "$RUN/${dataset}_fp32_profile.log" 2>&1
  fp32_rc=$?
  "$ROOT/build/gat_tfs_amx" "$graph" 32 64 verify \
    > "$RUN/${dataset}_amx_verify.log" 2>&1
  amx_rc=$?
  set -e
  echo "${dataset}_fp32_exit=$fp32_rc ${dataset}_amx_exit=$amx_rc" >> "$RUN/manifest.txt"
  for rep in 0 1 2; do
    for path in panel_transform_first_fp32 tfs_online_fused; do
      "$ROOT/build/gat_tfs_online_speed" "$graph" 32 benchmark 64 "$path" \
        > "$RUN/${dataset}_${path}_rep${rep}.log" 2>&1
    done
    "$ROOT/build/gat_tfs_amx_speed" "$graph" 32 64 benchmark \
      > "$RUN/${dataset}_amx_head_row_rep${rep}.log" 2>&1
  done
done
