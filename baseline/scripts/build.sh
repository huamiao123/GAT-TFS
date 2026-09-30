#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
mkdir -p "$ROOT/build/tmp"
export TMPDIR="$ROOT/build/tmp"
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3
FLAGS=(-O3 -fp-model precise -std=c++17 -mavx512f -mavx512dq -mavx512bw -mavx512vl -mavx512bf16 -mamx-tile -mamx-bf16 -mfma -qopenmp -qmkl=parallel -I"$ROOT/include")
SOURCES=("$ROOT/src/common.cpp" "$ROOT/src/ref_fp32.cpp" "$ROOT/src/baseline_standard.cpp" "$ROOT/src/baseline_tfs.cpp")
icpx "${FLAGS[@]}" "${SOURCES[@]}" "$ROOT/bench/main.cpp" -o "$ROOT/build/gat_baseline"
icpx "${FLAGS[@]}" -DGAT_PROFILE "${SOURCES[@]}" "$ROOT/bench/main.cpp" -o "$ROOT/build/gat_baseline_profile"
icpx "${FLAGS[@]}" "${SOURCES[@]}" "$ROOT/tests/boundaries.cpp" -o "$ROOT/build/test_boundaries"
icpx "${FLAGS[@]}" -S "$ROOT/src/baseline_tfs.cpp" -o "$ROOT/build/baseline_tfs.s"
grep -m 8 -E 'tdpbf16ps|tileloadd|tilestored' "$ROOT/build/baseline_tfs.s"
objdump -d "$ROOT/build/gat_baseline" | grep -m 8 -E 'tdpbf16ps|tileloadd|tilestored' || true
