#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
RUN=$1
mkdir -p "$RUN/source_snapshot" "$RUN/build" "$RUN/logs"
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$ROOT/original/mkl_baseline.cpp" "$RUN/source_snapshot/mkl_baseline.cpp"
cp "$ROOT/original/gcn_e2e_v3.cpp" "$RUN/source_snapshot/gcn_e2e_v3.cpp"
cmp "$RUN/source_snapshot/mkl_baseline.cpp" /home/huangjianqiang_group/hdacp1/data/yx/TFS/code/mkl_baseline.cpp
cmp "$RUN/source_snapshot/gcn_e2e_v3.cpp" /home/huangjianqiang_group/hdacp1/data/yx/TFS/code/gcn_e2e_v3.cpp
icpx -O3 -march=sapphirerapids -qopenmp -qmkl=parallel \
  "$RUN/source_snapshot/mkl_baseline.cpp" -o "$RUN/build/mkl_baseline" >"$RUN/build/mkl_compile.log" 2>&1
icpx -O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel \
  "$RUN/source_snapshot/gcn_e2e_v3.cpp" -o "$RUN/build/gcn_e2e_v3" >"$RUN/build/gcn_compile.log" 2>&1
sha256sum "$RUN/source_snapshot/"*.cpp "$RUN/build/mkl_baseline" "$RUN/build/gcn_e2e_v3" >"$RUN/source_binary.sha256"
printf 'RUN=%s\n' "$RUN"
