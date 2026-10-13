#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
KIND=$1
RUN=$2
EXP="$ROOT/experiments/${KIND}_balance_20261007"
mkdir -p "$RUN/build" "$RUN/logs" "$RUN/launcher"
finish(){
 status=$?
 python3 - "$RUN" "$KIND" "$status" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);e=dict(id=r.name+'-build',kind='balance_build',study=sys.argv[2],status='PASS' if sys.argv[3]=='0' else 'FAILED',exit_status=int(sys.argv[3]),evidence=str(r),paper_eligible=False,issues='New isolated source snapshot; previous sources and frozen evidence unchanged; user node cap10; no NUMA interleave',next='Smoke numerical/equivalence/work gates before performance')
(r/'build/event.json').write_text(json.dumps(e,indent=2)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/build/event.json"
 printf 'BALANCE_BUILD=%s status=%s\n' "$RUN" "$status"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
mkdir "$RUN/source_snapshot"
cp "$EXP/"*.hpp "$EXP/"*.cpp "$EXP/"*.py "$EXP/"*.sh "$EXP/PROTOCOL.md" "$RUN/source_snapshot/"
cp "$ROOT/original/gcn_e2e_v3.cpp" "$ROOT/src/paper_methods_kernels.hpp" "$ROOT/src/paper_methods_runtime.hpp" "$ROOT/src/projection_window_kernels.hpp" "$ROOT/src/projection_window_pmu.hpp" "$RUN/source_snapshot/"
cmp "$RUN/source_snapshot/gcn_e2e_v3.cpp" /home/huangjianqiang_group/hdacp1/data/yx/TFS/code/gcn_e2e_v3.cpp
export PYTHONDONTWRITEBYTECODE=1
FLAGS=(-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel)
if [ "$KIND" = grouping ]; then
 python3 "$RUN/source_snapshot/generate_grouping_driver.py" "$RUN/source_snapshot" "$ROOT" > "$RUN/build/generation.stdout.json"
 printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/projection_methods.cpp" -o "$RUN/build/projection_methods" > "$RUN/build/commands.txt"
 icpx "${FLAGS[@]}" "$RUN/source_snapshot/projection_methods.cpp" -o "$RUN/build/projection_methods" > "$RUN/build/compile.log" 2>&1
 icpx "${FLAGS[@]}" "$RUN/source_snapshot/gcn_e2e_v3.cpp" -o "$RUN/build/paper_original" >> "$RUN/build/compile.log" 2>&1
 icpx "${FLAGS[@]}" "$RUN/source_snapshot/generate_synthetic.cpp" -lstdc++fs -o "$RUN/build/generate_synthetic" >> "$RUN/build/compile.log" 2>&1
else
 printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/selective_shape.cpp" -o "$RUN/build/selective_shape" > "$RUN/build/commands.txt"
 icpx "${FLAGS[@]}" "$RUN/source_snapshot/selective_shape.cpp" -o "$RUN/build/selective_shape" > "$RUN/build/compile.log" 2>&1
fi
find "$RUN/source_snapshot" -type f -print0 | sort -z | xargs -0 sha256sum > "$RUN/source.sha256"
find "$RUN/build" -maxdepth 1 -type f -executable -print0 | sort -z | xargs -0 sha256sum > "$RUN/build/binary.sha256"
python3 - "$RUN" "$KIND" <<'PY'
import json,pathlib,sys
r=pathlib.Path(sys.argv[1]);m=dict(study=sys.argv[2],reviewed_github_head='672481d4ef34ec07f13f427485647e4def00062c',root=str(r),compiler=(r/'compiler.txt').read_text(),commands=(r/'build/commands.txt').read_text(),source_hashes=(r/'source.sha256').read_text(),binary_hashes=(r/'build/binary.sha256').read_text(),numa='default, no interleave',user_node_cap=10,precision='as frozen PROTOCOL; no gate relaxation',previous_evidence_unchanged=True)
(r/'manifest.json').write_text(json.dumps(m,indent=2)+'\n')
PY
