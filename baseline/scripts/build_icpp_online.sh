#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline
RUN="$ROOT/runs/icpp-online-build-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN" "$ROOT/build/tmp"
trap 'status=$?; echo "exit_status=$status" > "$RUN/status.txt"; echo "status=$status run=$RUN"' EXIT
cat "$ROOT/../../AGENTS.md" > "$RUN/AGENTS.read.txt"
cat "$ROOT/../../skills/tfs-research-engineering/SKILL.md" > "$RUN/SKILL.read.txt"
sha256sum "$ROOT/../../AGENTS.md" "$ROOT/../../skills/tfs-research-engineering/SKILL.md" "$ROOT/IMPLEMENTATION_CONTRACT.md" > "$RUN/preflight.sha256"
squeue -u hdacp1 > "$RUN/queue.txt"
sinfo -p intel > "$RUN/partition.txt"
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3
export TMPDIR="$ROOT/build/tmp"
icpx --version > "$RUN/compiler.txt"
sha256sum "$ROOT"/src/*.cpp "$ROOT/include/gat.hpp" "$ROOT"/tfs_online/* "$ROOT/scripts/build_icpp_online.sh" > "$RUN/source.sha256"
FLAGS=(-O3 -fp-model precise -std=c++17 -mavx512f -mavx512dq -mavx512bw -mavx512vl -mavx512bf16 -mamx-tile -mamx-bf16 -mfma -qopenmp -qmkl=parallel -I"$ROOT/include" -I"$ROOT/tfs_online")
SOURCES=("$ROOT"/src/*.cpp "$ROOT/tfs_online/icpp_online.cpp" "$ROOT/tfs_online/checks.cpp" "$ROOT/tfs_online/main.cpp")
printf '%q ' icpx "${FLAGS[@]}" "${SOURCES[@]}" -o "$ROOT/build/icpp_online" > "$RUN/command.txt"
icpx "${FLAGS[@]}" "${SOURCES[@]}" -o "$ROOT/build/icpp_online" > "$RUN/build.log" 2>&1 || { cat "$RUN/build.log"; exit 1; }
cat "$RUN/build.log"
sha256sum "$ROOT/build/icpp_online" > "$RUN/binary.sha256"
icpx "${FLAGS[@]}" -S "$ROOT/tfs_online/icpp_online.cpp" -o "$RUN/icpp_online.s" > "$RUN/assembly_build.log" 2>&1
icpx "${FLAGS[@]}" -S "$ROOT/tfs_online/checks.cpp" -o "$RUN/checks.s" >> "$RUN/assembly_build.log" 2>&1
grep -m 48 -E 'tdpbf16ps|tileload|tilestore|vmulps' "$RUN/icpp_online.s" > "$RUN/tile_instructions.txt" || true
mkdir -p "$RUN/source_snapshot"
cp -r "$ROOT/tfs_online" "$ROOT/src" "$ROOT/include" "$RUN/source_snapshot/"
