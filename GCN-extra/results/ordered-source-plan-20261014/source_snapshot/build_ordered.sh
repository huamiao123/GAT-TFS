#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
EXP="$ROOT/experiments/ordered_source_plan_20261014"
RUN="$ROOT/runs/ordered-source-build-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN/source_snapshot" "$RUN/build" "$RUN/logs" "$RUN/launcher"
finish(){
 status=$?
 python3 - "$RUN" "$status" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);e=dict(id=r.name+'-build',kind='ordered_source_build',status='PASS' if sys.argv[2]=='0' else 'FAILED',exit_status=int(sys.argv[2]),evidence=str(r),paper_eligible=False,issues='Isolated frozen dependency derivation; no baseline or old snapshot changes; preserve failed builds',next='Small same-scope numerical/work gates before diagnostic performance')
(r/'build/event.json').write_text(json.dumps(e,indent=2)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/build/event.json"
 printf 'ORDERED_BUILD=%s status=%s\n' "$RUN" "$status"
}
trap finish EXIT
source /etc/profile.d/modules.sh
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$EXP/"*.py "$EXP/"*.sh "$EXP/PROTOCOL.md" "$RUN/source_snapshot/"
cp "$EXP/ordered_plan.hpp" "$RUN/source_snapshot/"
python3 "$RUN/source_snapshot/generate_ordered.py" "$RUN/source_snapshot" "$ROOT/runs/pair-plan-20261007-01/source_snapshot" > "$RUN/build/generation.stdout.json"
cmp "$RUN/source_snapshot/gcn_e2e_v3.cpp" "$ROOT/original/gcn_e2e_v3.cpp"
cmp "$RUN/source_snapshot/gcn_e2e_v3.cpp" /home/huangjianqiang_group/hdacp1/data/yx/TFS/code/gcn_e2e_v3.cpp
export PYTHONDONTWRITEBYTECODE=1
FLAGS=(-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel)
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/ordered_methods.cpp" -o "$RUN/build/ordered_methods" > "$RUN/build/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/ordered_methods.cpp" -o "$RUN/build/ordered_methods" > "$RUN/build/compile.log" 2>&1
printf '\n' >> "$RUN/build/commands.txt"
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/gcn_e2e_v3.cpp" -o "$RUN/build/paper_original" >> "$RUN/build/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/gcn_e2e_v3.cpp" -o "$RUN/build/paper_original" >> "$RUN/build/compile.log" 2>&1
find "$RUN/source_snapshot" -type f -print0 | sort -z | xargs -0 sha256sum > "$RUN/source.sha256"
sha256sum "$RUN/build/ordered_methods" "$RUN/build/paper_original" > "$RUN/build/binary.sha256"
ldd "$RUN/build/ordered_methods" > "$RUN/build/linked_libraries.txt"
python3 - "$RUN" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);(r/'manifest.json').write_text(json.dumps(dict(root=str(r),source_hashes=(r/'source.sha256').read_text(),binary_hashes=(r/'build/binary.sha256').read_text(),compiler=(r/'compiler.txt').read_text(),commands=(r/'build/commands.txt').read_text(),generation=json.loads((r/'source_snapshot/generation.json').read_text()),dimensions=[128,128,128],S=64,performance_M=0,smoke_M=[64,256,0],modes=list(range(6)),node_cap=5,numa='default',old_evidence_unchanged=True),indent=2)+'\n')
PY
printf '%s\n' "$RUN" > "$EXP/LATEST_BUILD"
