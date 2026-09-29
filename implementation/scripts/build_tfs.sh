#!/usr/bin/env bash
set -euo pipefail
ROOT="${GAT_ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)}"
mkdir -p "$ROOT/build/tmp"
export TMPDIR="$ROOT/build/tmp"
module load "${GAT_COMPILER_MODULE:-intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp}"
module load "${GAT_MKL_MODULE:-intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3}"
icpx -O3 -fp-model precise -std=c++17 -mavx512f -mavx512dq -mavx512bw -mfma -qopenmp -qmkl=parallel \
  "$ROOT/src/gat_tfs_online.cpp" -o "$ROOT/build/gat_tfs_online"
icpx -O3 -fp-model precise -std=c++17 -mavx512f -mavx512dq -mavx512bw -mfma -qopenmp -qmkl=parallel -DGAT_NO_FINE_TIMING \
  "$ROOT/src/gat_tfs_online.cpp" -o "$ROOT/build/gat_tfs_online_speed"
icpx -O2 -fp-model precise -std=c++17 -mamx-tile -mamx-bf16 \
  "$ROOT/tools/amx_block_probe.cpp" -o "$ROOT/build/amx_block_probe"
icpx -O3 -fp-model precise -std=c++17 -mavx512f -mavx512dq -mavx512bw -mfma \
  -mamx-tile -mamx-bf16 -qopenmp -qmkl=parallel \
  "$ROOT/src/gat_tfs_amx.cpp" -o "$ROOT/build/gat_tfs_amx"
icpx -O3 -fp-model precise -std=c++17 -mavx512f -mavx512dq -mavx512bw -mfma \
  -mamx-tile -mamx-bf16 -qopenmp -qmkl=parallel -DGAT_NO_FINE_TIMING \
  "$ROOT/src/gat_tfs_amx.cpp" -o "$ROOT/build/gat_tfs_amx_speed"
icpx -O2 -fp-model precise -std=c++17 "$ROOT/tools/state_protocol_probe.cpp" -o "$ROOT/build/state_protocol_probe"
