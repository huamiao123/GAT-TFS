#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
EXP="$ROOT/experiments/journal_followup_p3_p4_20261005"
RUN="$ROOT/runs/selective-shape-build-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN/source_snapshot" "$RUN/build" "$RUN/logs"
finish(){
 status=$?
 python3 - "$RUN" "$status" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);e=dict(id=r.name,kind='selective_shape_build',status='PASS' if sys.argv[2]=='0' else 'FAILED',evidence=str(r),paper_eligible=False,issues='P4 isolated ACCURATE shape controls; source TFS/MKL unchanged',next='Smoke gates before diagnostic graphs')
(r/'build/event.json').write_text(json.dumps(e,indent=2)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/build/event.json"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$EXP/selective_kernels.hpp" "$EXP/selective_shape.cpp" "$EXP/selective_suite.py" "$EXP/build_selective.sh" "$EXP/selective_job.sh" "$EXP/selective_submit.sh" "$EXP/PROTOCOL.md" "$RUN/source_snapshot/"
cp "$ROOT/experiments/journal_grouping_p1_20261005/evidence_utils.py" "$RUN/source_snapshot/"
cp "$ROOT/original/gcn_e2e_v3.cpp" "$ROOT/src/paper_methods_kernels.hpp" "$ROOT/src/paper_methods_runtime.hpp" "$ROOT/src/projection_window_kernels.hpp" "$ROOT/src/projection_window_pmu.hpp" "$RUN/source_snapshot/"
cmp "$RUN/source_snapshot/gcn_e2e_v3.cpp" /home/huangjianqiang_group/hdacp1/data/yx/TFS/code/gcn_e2e_v3.cpp
FLAGS=(-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel)
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/selective_shape.cpp" -o "$RUN/build/selective_shape" > "$RUN/build/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/selective_shape.cpp" -o "$RUN/build/selective_shape" > "$RUN/build/compile.log" 2>&1
find "$RUN/source_snapshot" -type f -print0 | sort -z | xargs -0 sha256sum > "$RUN/source.sha256"
sha256sum "$RUN/build/selective_shape" > "$RUN/build/binary.sha256"
python3 - "$RUN" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);m=dict(parent_commit='af943926c7d4f8e68bcb95838924e21b14dd2e86',source_hashes=(r/'source.sha256').read_text(),binary_hashes=(r/'build/binary.sha256').read_text(),compiler=(r/'compiler.txt').read_text(),flags=(r/'build/commands.txt').read_text(),numa='default no interleave',gate='unchanged ACCURATE; rejects retained, not timed',scope='two-layer random H/W unit-adjacency computation')
(r/'manifest.json').write_text(json.dumps(m,indent=2)+'\n')
PY
printf '%s\n' "$RUN" > "$ROOT/build/SELECTIVE_SHAPE_LATEST"
printf 'BUILD=%s\n' "$RUN"
