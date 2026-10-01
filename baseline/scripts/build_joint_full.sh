#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline
RUN="$ROOT/runs/joint-full-build-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN" "$ROOT/build/tmp"
trap 'status=$?; echo "$status" > "$RUN/status.txt"; echo "status=$status run=$RUN"' EXIT
cat "$ROOT/../../AGENTS.md" > "$RUN/AGENTS.read.txt"
cat "$ROOT/../../skills/tfs-research-engineering/SKILL.md" > "$RUN/SKILL.read.txt"
sha256sum "$ROOT/../../AGENTS.md" "$ROOT/../../skills/tfs-research-engineering/SKILL.md" "$ROOT/IMPLEMENTATION_CONTRACT.md" > "$RUN/preflight.sha256"
squeue -u hdacp1 > "$RUN/queue.txt"
sinfo -p intel > "$RUN/partition.txt"
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3
export TMPDIR="$ROOT/build/tmp"
icpx --version > "$RUN/compiler.txt"
sha256sum "$ROOT"/src/*.cpp "$ROOT/include/gat.hpp" "$ROOT"/ours/local_online.* "$ROOT"/ours/joint* "$ROOT/scripts/build_joint_full.sh" > "$RUN/source.sha256"
FLAGS=(-O3 -fp-model precise -std=c++17 -mavx512f -mavx512dq -mavx512bw -mavx512vl -mavx512bf16 -mamx-tile -mamx-bf16 -mfma -qopenmp -qmkl=parallel -I"$ROOT/include" -I"$ROOT/ours")
printf '%q ' icpx "${FLAGS[@]}" "$ROOT"/src/*.cpp "$ROOT/ours/local_online.cpp" "$ROOT/ours/joint_online.cpp" "$ROOT/ours/joint_full.cpp" "$ROOT/ours/joint_compare_main.cpp" -o "$ROOT/build/joint_compare" > "$RUN/command.txt"
icpx "${FLAGS[@]}" "$ROOT"/src/*.cpp "$ROOT/ours/local_online.cpp" "$ROOT/ours/joint_online.cpp" "$ROOT/ours/joint_full.cpp" "$ROOT/ours/joint_compare_main.cpp" -o "$ROOT/build/joint_compare" > "$RUN/build.log" 2>&1 || { cat "$RUN/build.log"; exit 1; }
cat "$RUN/build.log"
sha256sum "$ROOT/build/joint_compare" > "$RUN/binary.sha256"
icpx "${FLAGS[@]}" -S "$ROOT/ours/joint_full.cpp" -o "$RUN/joint_full.s" > "$RUN/assembly_build.log" 2>&1
grep -m 32 -E 'vfmadd|vmulps|zmm' "$RUN/joint_full.s" > "$RUN/vector_instructions.txt" || true
