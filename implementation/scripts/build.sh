#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT
mkdir -p "$ROOT/build/tmp"
export TMPDIR="$ROOT/build/tmp"
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3
icpx -O3 -fp-model precise -std=c++17 -mavx512f -mavx512dq -mavx512bw -mfma -qmkl=sequential \
  "$ROOT/src/gat_online.cpp" -o "$ROOT/build/gat_online"

icpx -O3 -fp-model precise -std=c++17 -mavx512f -mavx512dq -mavx512bw -mfma -qmkl=sequential -DGAT_NO_FINE_TIMING \
  "$ROOT/src/gat_online.cpp" -o "$ROOT/build/gat_online_speed"
