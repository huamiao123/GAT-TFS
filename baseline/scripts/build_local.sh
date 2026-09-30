#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline
RUN="$ROOT/runs/local-build-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN"
cat "$ROOT/../../AGENTS.md" > "$RUN/AGENTS.read.txt"
cat "$ROOT/../../skills/tfs-research-engineering/SKILL.md" > "$RUN/SKILL.read.txt"
sha256sum "$ROOT/../../AGENTS.md" "$ROOT/../../skills/tfs-research-engineering/SKILL.md" "$ROOT/IMPLEMENTATION_CONTRACT.md" > "$RUN/preflight.sha256"
squeue -u hdacp1 > "$RUN/queue.txt"
sinfo -p intel > "$RUN/partition.txt"
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3
export TMPDIR="$ROOT/build/tmp"
sha256sum "$ROOT"/src/*.cpp "$ROOT/include/gat.hpp" "$ROOT"/ours/* "$ROOT/scripts/build_local.sh" > "$RUN/source.sha256"
FLAGS=(-O3 -fp-model precise -std=c++17 -mavx512f -mavx512dq -mavx512bw -mavx512vl -mavx512bf16 -mamx-tile -mamx-bf16 -mfma -qopenmp -qmkl=parallel -I"$ROOT/include" -I"$ROOT/ours")
set +e
icpx "${FLAGS[@]}" "$ROOT"/src/*.cpp "$ROOT/ours/local_online.cpp" "$ROOT/ours/main.cpp" -o "$ROOT/build/local_online" > "$RUN/build.log" 2>&1
status=$?
set -e
cat "$RUN/build.log"
echo "status=$status run=$RUN"
echo "$status" > "$RUN/status.txt"
if [[ $status == 0 ]]; then
 sha256sum "$ROOT/build/local_online" > "$RUN/binary.sha256"
 icpx "${FLAGS[@]}" -S "$ROOT/ours/local_online.cpp" -o "$RUN/local_online.s"
 grep -m 12 -E 'tdpbf16ps|tileloadd|tilestored' "$RUN/local_online.s" > "$RUN/amx_instructions.txt"
fi
exit "$status"
