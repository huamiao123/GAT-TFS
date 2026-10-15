#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
EXP=$ROOT/experiments/onemkl_bf16_twostep_20261015
RUN=$1
mkdir -p "$RUN/source_snapshot" "$RUN/build" "$RUN/logs"
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$ROOT/original/gcn_e2e_v3.cpp" "$RUN/source_snapshot/original.cpp"
cmp "$RUN/source_snapshot/original.cpp" /home/huangjianqiang_group/hdacp1/data/yx/TFS/code/gcn_e2e_v3.cpp
cp "$ROOT/src/paper_methods_kernels.hpp" "$RUN/source_snapshot/paper_methods_kernels.hpp"
cp "$EXP"/{driver.cpp,build.sh,job.sh,summarize.py} "$RUN/source_snapshot/"
icpx -O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel \
  "$RUN/source_snapshot/driver.cpp" -o "$RUN/build/bench" >"$RUN/build/compile.log" 2>&1
find "$RUN/source_snapshot" -type f -print0 | sort -z | xargs -0 sha256sum >"$RUN/source.sha256"
sha256sum "$RUN/build/bench" >"$RUN/build/binary.sha256"
printf 'RUN=%s\n' "$RUN"
