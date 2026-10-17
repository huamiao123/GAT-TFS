#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
SRC="$ROOT/experiments/h_panel_reuse_20261014"
RUN="$ROOT/runs/h-panel-reuse-build-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN/source_snapshot"
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$SRC/h_panel_reuse.cpp" "$SRC/original_gcn_e2e_v3.cpp" "$SRC/build.sh" "$SRC/run.sh" "$RUN/source_snapshot/"
cmp "$RUN/source_snapshot/original_gcn_e2e_v3.cpp" "$ROOT/original/gcn_e2e_v3.cpp"
FLAGS=(-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel)
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/h_panel_reuse.cpp" -o "$RUN/h_panel_reuse" > "$RUN/command.txt"
printf '\n' >> "$RUN/command.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/h_panel_reuse.cpp" -o "$RUN/h_panel_reuse" > "$RUN/compile.log" 2>&1
sha256sum "$RUN/source_snapshot/"* "$RUN/h_panel_reuse" > "$RUN/hashes.sha256"
ldd "$RUN/h_panel_reuse" > "$RUN/linked_libraries.txt"
printf '%s\n' "$RUN" > "$SRC/LATEST_BUILD"
python3 - "$RUN" <<'PY'
import json,pathlib,sys
r=pathlib.Path(sys.argv[1])
e=dict(id=r.name,kind='h_panel_reuse_build',status='PASS',evidence=str(r),
       source_sha256=(r/'hashes.sha256').read_text(),paper_eligible=False,
       purpose='Build same-work original TFS versus per-step and H-window panel loop ordering',
       notes='No correctness/performance conclusion from build alone; original source snapshot byte-identical.')
(r/'event.json').write_text(json.dumps(e,indent=2)+'\n')
PY
python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
printf 'BUILD_PATH=%s\n' "$RUN"
