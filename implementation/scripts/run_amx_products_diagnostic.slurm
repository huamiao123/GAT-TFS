#!/usr/bin/env bash
#SBATCH --job-name=gat-products-diag
#SBATCH --partition=intel
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=16
#SBATCH --mem=128G
#SBATCH --time=02:00:00
#SBATCH --output=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/runs/products-diag-slurm-%j.out
#SBATCH --error=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/runs/products-diag-slurm-%j.err
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT
GRAPH="$ROOT/data/products.gatbin"
RUN="$ROOT/runs/products-diagnostic-$SLURM_JOB_ID"
mkdir -p "$RUN" "$ROOT/build/tmp"
export TMPDIR="$ROOT/build/tmp"
RULES=/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md
SKILL=/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md
cat "$RULES" > "$RUN/AGENTS.read.txt"
cat "$SKILL" > "$RUN/SKILL.read.txt"
sha256sum "$RULES" "$SKILL" > "$RUN/preflight.sha256"
sha256sum "$GRAPH" "$ROOT"/src/*.cpp "$ROOT"/src/*.hpp \
  "$ROOT"/build/gat_tfs_online_speed "$ROOT"/build/gat_tfs_amx \
  "$ROOT"/build/gat_tfs_amx_speed "$ROOT/scripts/run_amx_products_diagnostic.slurm" \
  > "$RUN/artifact.sha256"
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3
export OMP_NUM_THREADS=16 MKL_NUM_THREADS=16
export OMP_PROC_BIND=close OMP_PLACES=cores
BLOCK=32
PANEL=64
{
  echo "job=$SLURM_JOB_ID node=$SLURMD_NODENAME partition=$SLURM_JOB_PARTITION"
  echo "cpus=$SLURM_CPUS_PER_TASK OMP=$OMP_NUM_THREADS MKL=$MKL_NUM_THREADS"
  echo "dataset=ogbn-products graph=$GRAPH block=$BLOCK panel=$PANEL"
  echo "boundary=three_layer_forward_only rep0=warmup rep1_rep2=measured"
  echo "prior_fp32_gate=FAILED job=10797365 reason=e2e_tfs_max_abs_0.00329780579_gt_0.003"
  echo "status=diagnostic_performance_only_not_paper_valid"
} > "$RUN/manifest.txt"
set +e
"$ROOT/build/gat_tfs_amx" "$GRAPH" "$BLOCK" "$PANEL" verify \
  > "$RUN/amx_correctness.log" 2>&1
VERIFY_RC=$?
set -e
echo "amx_verify_exit=$VERIFY_RC" >> "$RUN/manifest.txt"
for rep in 0 1 2; do
  for path in panel_transform_first_fp32 tfs_online_fused; do
    "$ROOT/build/gat_tfs_online_speed" "$GRAPH" "$BLOCK" benchmark "$PANEL" "$path" \
      > "$RUN/${path}_rep${rep}.log" 2>&1
  done
  "$ROOT/build/gat_tfs_amx_speed" "$GRAPH" "$BLOCK" "$PANEL" benchmark \
    > "$RUN/amx_head_row_rep${rep}.log" 2>&1
done
