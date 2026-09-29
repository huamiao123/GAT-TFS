#!/usr/bin/env bash
#SBATCH --job-name=gat-products-amx
#SBATCH --partition=intel
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=128G
#SBATCH --time=04:00:00
#SBATCH --output=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/runs/products-slurm-%j.out
#SBATCH --error=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/runs/products-slurm-%j.err
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT
SOURCE=/home/huangjianqiang_group/hdacp1/data/wzh/datasets/ogbn-products_official/products
RUN="$ROOT/runs/products-amx-$SLURM_JOB_ID"
GRAPH="$ROOT/data/products.gatbin"
mkdir -p "$RUN" "$ROOT/build/tmp"
export TMPDIR="$ROOT/build/tmp"
RULES=/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md
SKILL=/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md
cat "$RULES" > "$RUN/AGENTS.read.txt"
cat "$SKILL" > "$RUN/SKILL.read.txt"
sha256sum "$RULES" "$SKILL" > "$RUN/preflight.sha256"
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3
export OMP_NUM_THREADS=16 MKL_NUM_THREADS=16
export OMP_PROC_BIND=close OMP_PLACES=cores
export OPENBLAS_NUM_THREADS=1
BLOCK=32
PANEL=64
{
  echo "job=$SLURM_JOB_ID node=$SLURMD_NODENAME partition=$SLURM_JOB_PARTITION"
  echo "cpus=$SLURM_CPUS_PER_TASK OMP=$OMP_NUM_THREADS MKL=$MKL_NUM_THREADS"
  echo "dataset=ogbn-products source=$SOURCE graph=$GRAPH"
  echo "model=100-8x32-8x32-1x47 block=$BLOCK panel=$PANEL"
  echo "precision=amx_bf16_hi_lo_fp32_accumulation_fp32_gemm"
  echo "boundary=three_layer_forward_only rep0=warmup rep1_rep2=measured"
} > "$RUN/manifest.txt"
sha256sum "$SOURCE/raw/node-feat.csv.gz" "$SOURCE/raw/edge.csv.gz" \
  "$SOURCE/raw/node-label.csv.gz" > "$RUN/input_raw.sha256"
python3 "$ROOT/tools/prepare_arxiv.py" --dataset ogbn-products \
  --source "$SOURCE" --output "$GRAPH" > "$RUN/prepare.log" 2>&1
sha256sum "$GRAPH" "$ROOT/tools/prepare_arxiv.py" \
  "$ROOT"/src/*.cpp "$ROOT"/src/*.hpp \
  "$ROOT"/build/gat_tfs_online "$ROOT"/build/gat_tfs_online_speed \
  "$ROOT"/build/gat_tfs_amx "$ROOT"/build/gat_tfs_amx_speed \
  "$ROOT/scripts/run_amx_products.slurm" > "$RUN/artifact.sha256"
"$ROOT/build/gat_tfs_online" "$GRAPH" "$BLOCK" profile "$PANEL" \
  > "$RUN/fp32_correctness.log" 2>&1
"$ROOT/build/gat_tfs_amx" "$GRAPH" "$BLOCK" "$PANEL" verify \
  > "$RUN/amx_correctness.log" 2>&1
for rep in 0 1 2; do
  for path in panel_transform_first_fp32 tfs_online_fused; do
    "$ROOT/build/gat_tfs_online_speed" "$GRAPH" "$BLOCK" benchmark "$PANEL" "$path" \
      > "$RUN/${path}_rep${rep}.log" 2>&1
  done
  "$ROOT/build/gat_tfs_amx_speed" "$GRAPH" "$BLOCK" "$PANEL" benchmark \
    > "$RUN/amx_head_row_rep${rep}.log" 2>&1
done
