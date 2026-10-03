#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
RUN="$ROOT/runs/build-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN/source_snapshot/src" "$RUN/source_snapshot/original" "$RUN/source_snapshot/scripts"
finish() {
    status=$?
    printf 'exit_status=%s\n' "$status" > "$RUN/status.txt"
    python3 - "$RUN" "$status" <<'PY'
import json, pathlib, sys
r=pathlib.Path(sys.argv[1]);status=int(sys.argv[2])
event={'id':r.name,'kind':'build','status':'PASS' if status==0 else 'FAILED','exit_status':status,'evidence':str(r),'purpose':'Build unchanged Original TFS and block/local inference extension','source_hashes':'source.sha256','binary_hashes':'binary.sha256','compiler':'compiler.txt','timing_boundary':'not a benchmark','paper_eligible':False,'issues':'See build.log; no new major issue if exit=0','next':'shared intel correctness gate'}
(r/'event.json').write_text(json.dumps(event,indent=2))
PY
    python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
    echo "BUILD_RUN=$RUN status=$status"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$ROOT/src/bench.cpp" "$RUN/source_snapshot/src/"
cp "$ROOT/original/amx_tfs_v3.cpp" "$RUN/source_snapshot/original/"
cp "$ROOT/scripts/"*.sh "$ROOT/scripts/"*.py "$RUN/source_snapshot/scripts/"
cp "$ROOT/scripts/"*.slurm "$RUN/source_snapshot/scripts/"
cp "$ROOT/IMPLEMENTATION_CONTRACT.md" "$RUN/source_snapshot/"
sha256sum "$ROOT/src/bench.cpp" "$ROOT/original/amx_tfs_v3.cpp" "$ROOT/scripts/build.sh" "$ROOT/scripts/preflight.sh" > "$RUN/source.sha256"
FLAGS=(-O3 -fp-model precise -std=c++17 -mavx512f -mavx512dq -mavx512bw -mavx512vl -mavx512bf16 -mamx-tile -mamx-bf16 -mfma -qopenmp -qmkl=parallel)
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/src/bench.cpp" -o "$RUN/gcn_bench" > "$RUN/command.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/src/bench.cpp" -o "$RUN/gcn_bench" > "$RUN/build.log" 2>&1 || { cat "$RUN/build.log"; exit 1; }
sha256sum "$RUN/gcn_bench" > "$RUN/binary.sha256"
printf '%s\n' "$RUN" > "$ROOT/build/LATEST"
