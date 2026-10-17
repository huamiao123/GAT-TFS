#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
EXP="$ROOT/experiments/ordered_source_plan_20261014"
PARENT=$(cat "$EXP/LATEST_BUILD")
RUN="$ROOT/runs/ordered-source-detail-build-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN/build" "$RUN/logs"
finish(){
 status=$?
 python3 - "$RUN" "$status" <<'PY'
import pathlib,json,sys
p=pathlib.Path(sys.argv[1]);e=dict(id=p.name+'-build',kind='diagnostic_build',status='PASS' if sys.argv[2]=='0' else 'FAILED',exit_status=int(sys.argv[2]),evidence=str(p),issues='Main performance snapshot immutable; separate sampled-only load/decode timing',next='Small correctness then five-graph detailed diagnostics, no E2E speed claim from this build')
(p/'build_event.json').write_text(json.dumps(e,indent=2)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/build_event.json"
}
trap finish EXIT
source /etc/profile.d/modules.sh
source "$ROOT/scripts/preflight.sh" "$RUN"
python3 "$EXP/generate_detail.py" "$PARENT/source_snapshot" "$RUN/source_snapshot" > "$RUN/generation.log"
cp "$EXP/generate_detail.py" "$EXP/build_detail.sh" "$RUN/source_snapshot/"
icpx -O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel "$RUN/source_snapshot/ordered_methods.cpp" -o "$RUN/build/ordered_methods" > "$RUN/compile.log" 2>&1
cp "$PARENT/build/paper_original" "$RUN/build/paper_original"
find "$RUN/source_snapshot" -maxdepth 1 -type f -print0 | sort -z | xargs -0 sha256sum > "$RUN/source.sha256"
sha256sum "$RUN/build/ordered_methods" "$RUN/build/paper_original" > "$RUN/binary.sha256"
icpx --version > "$RUN/compiler.txt"
printf '%s\n' 'icpx -O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel source_snapshot/ordered_methods.cpp -o build/ordered_methods' > "$RUN/commands.txt"
python3 - "$RUN" "$PARENT" <<'PY'
import pathlib,json,sys
p=pathlib.Path(sys.argv[1]);(p/'manifest.json').write_text(json.dumps(dict(parent=sys.argv[2],purpose='separate sampled diagnostic only',source_hashes=(p/'source.sha256').read_text(),binary_hashes=(p/'binary.sha256').read_text(),compiler=(p/'compiler.txt').read_text(),flags=(p/'commands.txt').read_text(),numa='default',main_timing_unchanged=True),indent=2)+'\n')
PY
printf '%s\n' "$RUN" > "$EXP/LATEST_DETAIL_BUILD"
printf 'DETAIL_BUILD=%s\n' "$RUN"
