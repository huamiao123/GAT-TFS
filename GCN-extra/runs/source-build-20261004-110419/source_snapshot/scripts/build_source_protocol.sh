#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
RUN="$ROOT/runs/source-build-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN/source_snapshot/src" "$RUN/source_snapshot/original" "$RUN/source_snapshot/scripts" "$RUN/source_snapshot/docs"
finish() {
    status=$?
    python3 - "$RUN" "$status" <<'PY'
import json,pathlib,sys
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2]);e={'id':r.name,'kind':'source_protocol_build','status':'PASS' if s==0 else 'FAILED','exit_status':s,'evidence':str(r),'paper_eligible':False,'issues':'See build.log; source baseline and generated runner compile with original flags','next':'shared source-protocol correctness gate'}
(r/'event.json').write_text(json.dumps(e,indent=2));(r/'status.txt').write_text('exit_status='+str(s)+'\n')
PY
    python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
    printf 'SOURCE_BUILD=%s status=%s\n' "$RUN" "$status"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cat "$ROOT/docs/SOURCE_MKL_PROTOCOL_20261004.md" > "$RUN/source_protocol.read.md"
python3 "$ROOT/scripts/generate_source_protocol.py" > "$RUN/generation.json"
cp "$ROOT/src/"*.cpp "$ROOT/src/"*.hpp "$RUN/source_snapshot/src/"
cp "$ROOT/original/"*.cpp "$RUN/source_snapshot/original/"
cp "$ROOT/scripts/"*.sh "$ROOT/scripts/"*.py "$ROOT/scripts/"*.slurm "$RUN/source_snapshot/scripts/"
cp "$ROOT/docs/SOURCE_MKL_PROTOCOL_20261004.md" "$RUN/source_snapshot/docs/"
find "$RUN/source_snapshot" -type f -print0 | sort -z | xargs -0 sha256sum > "$RUN/source.sha256"
FLAGS=(-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp)
printf 'LP64\n' > "$RUN/mkl_integer_model.txt"
printf '%q ' icpx "${FLAGS[@]}" -qmkl=parallel "$RUN/source_snapshot/src/source_protocol.cpp" -o "$RUN/source_candidates" > "$RUN/candidate_command.txt"
icpx "${FLAGS[@]}" -qmkl=parallel "$RUN/source_snapshot/src/source_protocol.cpp" -o "$RUN/source_candidates" > "$RUN/build.log" 2>&1
icpx "${FLAGS[@]}" -qmkl=parallel "$RUN/source_snapshot/original/gcn_e2e_bench.cpp" -o "$RUN/source_e2e_raw" >> "$RUN/build.log" 2>&1
icpx -O3 -march=sapphirerapids -qopenmp -qmkl=parallel "$RUN/source_snapshot/original/mkl_baseline.cpp" -o "$RUN/source_mkl_raw" >> "$RUN/build.log" 2>&1
icpx "${FLAGS[@]}" "$RUN/source_snapshot/original/amx_tfs_v3.cpp" -o "$RUN/source_tfs_raw" >> "$RUN/build.log" 2>&1
sha256sum "$RUN/source_candidates" "$RUN/source_e2e_raw" "$RUN/source_mkl_raw" "$RUN/source_tfs_raw" > "$RUN/binary.sha256"
ldd "$RUN/source_candidates" > "$RUN/linked_libraries.txt"
printf '%s\n' "$RUN" > "$ROOT/build/SOURCE_PROTOCOL_LATEST"
