#!/usr/bin/env bash
#SBATCH --job-name=gat-pure-baseline
#SBATCH --partition=intel
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=128G
#SBATCH --time=02:00:00
#SBATCH --output=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/runs/pure-gat-slurm-%j.out
#SBATCH --error=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/runs/pure-gat-slurm-%j.err
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT
RUN="$ROOT/runs/pure-gat-baseline-$SLURM_JOB_ID"
mkdir -p "$RUN" "$ROOT/build/tmp"
export TMPDIR="$ROOT/build/tmp"
RULES=/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md
SKILL=/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md
cat "$RULES" > "$RUN/AGENTS.read.txt"
cat "$SKILL" > "$RUN/SKILL.read.txt"
sha256sum "$RULES" "$SKILL" > "$RUN/preflight.sha256"
sha256sum "$ROOT"/src/*.cpp "$ROOT"/src/*.hpp \
  "$ROOT"/build/gat_tfs_online_speed "$ROOT"/build/gat_tfs_amx_speed \
  "$ROOT"/data/arxiv.gatbin "$ROOT"/data/products.gatbin \
  "$ROOT/scripts/run_pure_gat_baseline.slurm" > "$RUN/artifact.sha256"
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3
export OMP_NUM_THREADS=16 MKL_NUM_THREADS=16
export OMP_PROC_BIND=close OMP_PLACES=cores
{
  echo "job=$SLURM_JOB_ID node=$SLURMD_NODENAME partition=$SLURM_JOB_PARTITION"
  echo "cpus=$SLURM_CPUS_PER_TASK OMP=$OMP_NUM_THREADS MKL=$MKL_NUM_THREADS"
  echo "primary_baseline=reference_standard_vanilla_GAT_stable_softmax"
  echo "secondary_controls=online_fp32,online_avx512,panel_transform_first_fp32"
  echo "datasets=ogbn-arxiv,ogbn-products block=32 panel=64"
  echo "precision_evidence=fine-timing-10797770"
  echo "arxiv_precision=pass products_precision=fails_fixed_max_abs_gate"
  echo "benchmark_boundary=three_layer_forward_only rep0=warmup rep1_rep2=measured"
} > "$RUN/manifest.txt"
for dataset in arxiv products; do
  graph="$ROOT/data/${dataset}.gatbin"
  for rep in 0 1 2; do
    for path in reference online_fp32 online_avx512 panel_transform_first_fp32 tfs_online_fused; do
      "$ROOT/build/gat_tfs_online_speed" "$graph" 32 benchmark 64 "$path" \
        > "$RUN/${dataset}_${path}_rep${rep}.log" 2>&1
    done
    "$ROOT/build/gat_tfs_amx_speed" "$graph" 32 64 benchmark \
      > "$RUN/${dataset}_amx_head_row_rep${rep}.log" 2>&1
  done
done
