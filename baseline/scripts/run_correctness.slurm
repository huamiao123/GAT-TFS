#!/usr/bin/env bash
#SBATCH --job-name=gat-baseline-check
#SBATCH --partition=intel
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=2
#SBATCH --mem=8G
#SBATCH --hint=nomultithread
#SBATCH --time=00:20:00
#SBATCH --output=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline/runs/check-%j.out
#SBATCH --error=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline/runs/check-%j.err
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline
RUN="$ROOT/runs/check-$SLURM_JOB_ID"
mkdir -p "$RUN"
trap 'status=$?; printf "exit_status=%s\n" "$status" > "$RUN/status.txt"' EXIT
export TMPDIR="$ROOT/build/tmp"
export PYTHONDONTWRITEBYTECODE=1
RULES=/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md
SKILL=/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md
cat "$RULES" > "$RUN/AGENTS.read.txt"
cat "$SKILL" > "$RUN/SKILL.read.txt"
sha256sum "$RULES" "$SKILL" "$ROOT/IMPLEMENTATION_CONTRACT.md" > "$RUN/preflight.sha256"
find "$ROOT/src" "$ROOT/include" "$ROOT/tests" "$ROOT/bench" "$ROOT/scripts" -type f -exec sha256sum {} + > "$RUN/source.sha256"
sha256sum "$ROOT"/build/gat_baseline* "$ROOT/build/test_boundaries" > "$RUN/binary.sha256"
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3
export OMP_NUM_THREADS=2 MKL_NUM_THREADS=2 MKL_DYNAMIC=FALSE
export OMP_PROC_BIND=close OMP_PLACES=cores
{
  echo "job=$SLURM_JOB_ID node=$SLURMD_NODENAME partition=$SLURM_JOB_PARTITION"
  echo 'scope=small-graph correctness; synthetic weights; not paper performance'
  echo 'accuracy_gate=unconfigured for BF16; numerical oracle gates in tests only'
  lscpu
  taskset -pc $$
  env | grep -E 'OMP_|MKL_|SLURM_CPUS' | sort
} > "$RUN/manifest.txt"
"$ROOT/build/test_boundaries" "$RUN" > "$RUN/boundaries.log" 2>&1
sha256sum "$RUN/oracle.gatbin" "$RUN/oracle.weights" > "$RUN/input.sha256"
"$ROOT/build/gat_baseline" --graph "$RUN/oracle.gatbin" --weights "$RUN/oracle.weights" --mode correctness --threads 2 --dump-prefix "$RUN/oracle" > "$RUN/correctness.log" 2>&1
python3 "$ROOT/tests/oracle.py" --graph "$RUN/oracle.gatbin" --weights "$RUN/oracle.weights" --dump-prefix "$RUN/oracle" > "$RUN/numpy_oracle.log" 2>&1
python3 "$ROOT/tests/evaluate_task_test.py" "$RUN/evaluator_fixture" > "$RUN/evaluator.log" 2>&1
for layer in 1 2 3; do
  "$ROOT/build/gat_baseline" --graph "$RUN/oracle.gatbin" --weights "$RUN/oracle.weights" --mode export-fixture --threads 2 --layer "$layer" --fixture "$RUN/layer${layer}.fixture" > "$RUN/export${layer}.log" 2>&1
  "$ROOT/build/gat_baseline" --graph "$RUN/oracle.gatbin" --weights "$RUN/oracle.weights" --mode fixed-p --threads 2 --layer "$layer" --fixture "$RUN/layer${layer}.fixture" --warmups 1 --repeats 3 > "$RUN/fixed_p${layer}.log" 2>&1
  MKL_VERBOSE=1 "$ROOT/build/gat_baseline" --graph "$ROOT/../data/smoke1024.gatbin" --seed 11 --mode micro --threads 2 --layer "$layer" --warmups 1 --repeats 3 > "$RUN/micro${layer}.log" 2>&1
done
for path in ref_fp32 standard_fp32 standard_bf16 tfs_bf16 matched_attention; do
  "$ROOT/build/gat_baseline" --graph "$ROOT/../data/smoke1024.gatbin" --seed 11 --mode benchmark --threads 2 --path "$path" --warmups 2 --repeats 5 > "$RUN/smoke_${path}.log" 2>&1
done
"$ROOT/build/gat_baseline_profile" --graph "$ROOT/../data/smoke1024.gatbin" --seed 11 --mode profile --threads 2 --path tfs_bf16 --warmups 1 --repeats 2 > "$RUN/profile.log" 2>&1
if command -v perf >/dev/null; then perf list 2>&1 | grep -i -A2 amx > "$RUN/perf_events.txt" || true; fi
echo 'CHECK_COMPLETE finite_and_oracle_tests=PASS BF16_task_gate=UNCONFIGURED'
