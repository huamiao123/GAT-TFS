#!/usr/bin/env bash
set -euo pipefail
ROOT="${GAT_ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
RUN="$(mktemp -d "$ROOT/runs/amx-smoke-XXXXXXXX")"
RULES=/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md
SKILL=/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md
cat "$RULES" > "$RUN/AGENTS.read.txt"
cat "$SKILL" > "$RUN/SKILL.read.txt"
sha256sum "$RULES" "$SKILL" > "$RUN/preflight.sha256"
sha256sum "$ROOT"/src/*.cpp "$ROOT"/src/*.hpp "$ROOT"/build/gat_tfs_amx \
  "$ROOT"/data/smoke1024.gatbin > "$RUN/artifact.sha256"
module load "${GAT_COMPILER_MODULE:-intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp}"
module load "${GAT_MKL_MODULE:-intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3}"
export OMP_NUM_THREADS=1 MKL_NUM_THREADS=1 OMP_PROC_BIND=close OMP_PLACES=cores
{
  echo "run_dir=$RUN"
  echo "node=$(hostname)"
  echo "threads=1"
  echo "scope=lightweight_1024_node_amx_correctness"
  echo "precision=bf16_weights_and_values_fp32_accumulation_and_gemm"
  echo "job=none_login_node"
} > "$RUN/manifest.txt"
set +e
"$ROOT/build/gat_tfs_amx" "$ROOT/data/smoke1024.gatbin" 32 16 verify \
  > "$RUN/amx_full.log" 2>&1
status=$?
set -e
echo "exit_code=$status" >> "$RUN/manifest.txt"
grep -E 'AMX_LAYER|AMX_E2E|AMX_VERIFY|ERROR' "$RUN/amx_full.log" || true
echo "run_dir=$RUN"
exit "$status"
