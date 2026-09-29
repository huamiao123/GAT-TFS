#!/usr/bin/env bash
set -euo pipefail
ROOT="${GAT_ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
RUN="$(mktemp -d "$ROOT/runs/implementation-smoke-XXXXXXXX")"
RULES=/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md
SKILL=/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md
cat "$RULES" > "$RUN/AGENTS.read.txt"
cat "$SKILL" > "$RUN/SKILL.read.txt"
sha256sum "$RULES" "$SKILL" > "$RUN/preflight.sha256"
sha256sum "$ROOT"/src/*.cpp "$ROOT"/src/*.hpp "$ROOT"/tools/*probe.cpp \
  "$ROOT"/build/gat_tfs_online "$ROOT"/build/gat_tfs_online_speed \
  "$ROOT"/build/gat_tfs_amx "$ROOT"/build/amx_block_probe \
  "$ROOT"/build/state_protocol_probe "$ROOT"/data/smoke1024.gatbin \
  > "$RUN/artifact.sha256"
module load "${GAT_COMPILER_MODULE:-intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp}"
module load "${GAT_MKL_MODULE:-intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3}"
export OMP_NUM_THREADS=1 MKL_NUM_THREADS=1 OMP_PROC_BIND=close OMP_PLACES=cores
{
  echo "run_dir=$RUN"
  echo "node=$(hostname)"
  echo "threads=$OMP_NUM_THREADS"
  echo "scope=lightweight_1024_node_correctness_on_login"
  echo "amx_probe=skip_if_cpu_unavailable"
  echo "large_graph=not_run"
} > "$RUN/manifest.txt"
"$ROOT/build/state_protocol_probe" > "$RUN/state_protocol.log" 2>&1
"$ROOT/build/amx_block_probe" > "$RUN/amx_block.log" 2>&1
"$ROOT/build/gat_tfs_online" "$ROOT/data/smoke1024.gatbin" 32 profile 16 \
  > "$RUN/gat1024.log" 2>&1
grep -E 'STATE_PROTOCOL|AMX_BLOCK|CORRECTNESS' "$RUN"/*.log
echo "run_dir=$RUN"
