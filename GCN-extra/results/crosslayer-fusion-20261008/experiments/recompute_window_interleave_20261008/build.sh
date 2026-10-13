#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$ROOT/experiments/recompute_window_interleave_20261008
RUN=$ROOT/runs/recompute-window-interleave-build-$(date +%Y%m%d-%H%M%S)
mkdir -p "$RUN/source_snapshot"
finish(){ s=$?; python3 - "$RUN" "$s" <<'PY'
import json,pathlib,sys
r=pathlib.Path(sys.argv[1]); s=int(sys.argv[2]); e=dict(id=r.name,kind='recompute_window_interleave_build',status='PASS' if s==0 else 'FAILED',exit_status=s,evidence=str(r),paper_eligible=False,issues='Interleaved timing shadow prototype; no paper performance claim',next='Run exclusive interleaved timing gate if build passes'); (r/'event.json').write_text(json.dumps(e,indent=2)+'\n'); (r/'exit_status.txt').write_text(str(s)+'\n')
PY
python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"; printf '%s\n' "$RUN" > "$STUDY/LATEST_BUILD"; }
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$STUDY/PROTOCOL.md" "$RUN/protocol.read.md"
cp "$STUDY/source_snapshot/recompute_window_interleave.cpp" "$RUN/source_snapshot/"
cp "$STUDY/source_snapshot/recompute_window_kernel.cpp" "$RUN/source_snapshot/"
sha256sum "$RUN/source_snapshot"/* > "$RUN/source.sha256"
FLAGS=(-O3 -std=c++17 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel)
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/recompute_window_interleave.cpp" -o "$RUN/recompute_window_interleave" > "$RUN/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/recompute_window_interleave.cpp" -o "$RUN/recompute_window_interleave" > "$RUN/build.log" 2>&1
sha256sum "$RUN/recompute_window_interleave" > "$RUN/binary.sha256"
ldd "$RUN/recompute_window_interleave" > "$RUN/linked_libraries.txt"