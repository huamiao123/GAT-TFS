#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
RUN="$ROOT/runs/projection-window-build-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN/source_snapshot"
finish(){
 status=$?
 python3 - "$RUN" "$status" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2]);e=dict(id=r.name,kind='projection_window_build',status='PASS' if s==0 else 'FAILED',exit_status=s,evidence=str(r),paper_eligible=False,issues='Frozen original source/control kernels; independent S/M; unchanged numerical gates plus bitwise equivalence',next='Shared correctness before exclusive formal')
(r/'event.json').write_text(json.dumps(e,indent=2));(r/'exit_status.txt').write_text(str(s)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
 printf 'PROJECTION_BUILD=%s status=%s\n' "$RUN" "$status"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$ROOT/docs/PROJECTION_WINDOW_PROTOCOL_20261004.md" "$RUN/protocol.read.md"
cp -r "$ROOT/docs/references/perfmon_SPR_20261004" "$RUN/pmu_event_sources"
cp "$ROOT/original/gcn_e2e_v3.cpp" "$ROOT/scripts/generate_projection_methods.py" "$ROOT/scripts/build_projection_methods.sh" "$RUN/source_snapshot/"
cmp "$RUN/source_snapshot/gcn_e2e_v3.cpp" /home/huangjianqiang_group/hdacp1/data/yx/TFS/code/gcn_e2e_v3.cpp
python3 "$ROOT/scripts/generate_projection_methods.py" "$RUN/source_snapshot" > "$RUN/generation.json"
FLAGS=(-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel)
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/projection_methods.cpp" -o "$RUN/projection_methods" > "$RUN/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/projection_methods.cpp" -o "$RUN/projection_methods" > "$RUN/build.log" 2>&1
printf '\n' >> "$RUN/commands.txt"
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/gcn_e2e_v3.cpp" -o "$RUN/paper_original" >> "$RUN/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/gcn_e2e_v3.cpp" -o "$RUN/paper_original" >> "$RUN/build.log" 2>&1
find "$RUN/source_snapshot" -type f -print0 | sort -z | xargs -0 sha256sum > "$RUN/source.sha256"
find "$RUN/pmu_event_sources" -type f -print0 | sort -z | xargs -0 sha256sum > "$RUN/pmu_events.sha256"
sha256sum "$RUN/projection_methods" "$RUN/paper_original" > "$RUN/binary.sha256"
ldd "$RUN/projection_methods" > "$RUN/linked_libraries.txt"
printf '%s\n' "$RUN" > "$ROOT/build/PROJECTION_WINDOW_LATEST"
