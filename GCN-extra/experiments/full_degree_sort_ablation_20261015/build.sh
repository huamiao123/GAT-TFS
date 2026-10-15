#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
EXP=$ROOT/experiments/full_degree_sort_ablation_20261015
RUN=$ROOT/runs/full-degree-sort-ablation-$(date +%Y%m%d-%H%M%S)
mkdir -p "$RUN/source_snapshot" "$RUN/build" "$RUN/logs"
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$EXP"/{ablation.hpp,generate.py,job.sh,summarize.py,PROTOCOL.md,build.sh} "$RUN/source_snapshot/"
cp "$ROOT/original/gcn_e2e_v3.cpp" "$RUN/source_snapshot/original.cpp"
cp "$ROOT/src/paper_methods_kernels.hpp" "$RUN/source_snapshot/paper_methods_kernels.hpp"
python3 "$RUN/source_snapshot/generate.py" "$ROOT" "$RUN/source_snapshot" > "$RUN/generation.log"
cmp "$RUN/source_snapshot/original.cpp" /home/huangjianqiang_group/hdacp1/data/yx/TFS/code/gcn_e2e_v3.cpp
icpx -O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel \
    "$RUN/source_snapshot/driver.cpp" -o "$RUN/build/ablation" > "$RUN/build/compile.log" 2>&1
find "$RUN/source_snapshot" -type f -print0 | sort -z | xargs -0 sha256sum > "$RUN/source.sha256"
sha256sum "$RUN/build/ablation" > "$RUN/build/binary.sha256"
printf '%s\n' "$RUN" > "$ROOT/build/FULL_DEGREE_SORT_LATEST"
printf 'RUN=%s\n' "$RUN"
