#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
RUN="$ROOT/runs/paper-method-build-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN/source_snapshot"
finish(){
 status=$?
 python3 - "$RUN" "$status" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2]);e=dict(id=r.name,kind='paper_method_build',status='PASS' if s==0 else 'FAILED',exit_status=s,evidence=str(r),paper_eligible=False,issues='Controls fixed in PAPER_METHODS_PROTOCOL_20261004.md; no numerical gate changes allowed',next='Shared smoke before exclusive controlled comparisons')
(r/'event.json').write_text(json.dumps(e,indent=2));(r/'exit_status.txt').write_text(str(s)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
 printf 'METHOD_BUILD=%s status=%s\n' "$RUN" "$status"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$ROOT/docs/PAPER_METHODS_PROTOCOL_20261004.md" "$RUN/protocol.read.md"
cp "$ROOT/src/paper_methods_runtime.hpp" "$ROOT/src/paper_methods_kernels.hpp" "$ROOT/original/gcn_e2e_v3.cpp" "$ROOT/scripts/generate_paper_methods.py" "$ROOT/scripts/build_paper_methods.sh" "$RUN/source_snapshot/"
cmp "$RUN/source_snapshot/gcn_e2e_v3.cpp" /home/huangjianqiang_group/hdacp1/data/yx/TFS/code/gcn_e2e_v3.cpp
python3 "$ROOT/scripts/generate_paper_methods.py" "$RUN/source_snapshot" > "$RUN/generation.json"
FLAGS=(-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel)
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/paper_methods.cpp" -o "$RUN/paper_methods" > "$RUN/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/paper_methods.cpp" -o "$RUN/paper_methods" > "$RUN/build.log" 2>&1
printf '\n' >> "$RUN/commands.txt"
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/gcn_e2e_v3.cpp" -o "$RUN/paper_original" >> "$RUN/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/gcn_e2e_v3.cpp" -o "$RUN/paper_original" >> "$RUN/build.log" 2>&1
find "$RUN/source_snapshot" -type f -print0 | sort -z | xargs -0 sha256sum > "$RUN/source.sha256"
sha256sum "$RUN/paper_methods" "$RUN/paper_original" > "$RUN/binary.sha256"
ldd "$RUN/paper_methods" > "$RUN/linked_libraries.txt"
printf '%s\n' "$RUN" > "$ROOT/build/PAPER_METHOD_LATEST"
