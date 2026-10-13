#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$ROOT/experiments/source_partition_audit_20261008
RUN=$ROOT/runs/source-partition-build-$(date +%Y%m%d-%H%M%S)
mkdir -p "$RUN/source_snapshot"
finish(){
  s=$?
  python3 - "$RUN" "$s" <<'PY'
import json,pathlib,sys
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2])
e=dict(id=r.name,kind='source_partition_build',status='PASS' if s==0 else 'FAILED',exit_status=s,evidence=str(r),paper_eligible=False,issues='Structural source-overlap audit; no runtime claim',next='Run analyzer on RoadNet-CA if build passes')
(r/'event.json').write_text(json.dumps(e,indent=2)+'\n');(r/'exit_status.txt').write_text(str(s)+'\n')
PY
  python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
  printf '%s\n' "$RUN" > "$STUDY/LATEST_BUILD"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$STUDY/PROTOCOL.md" "$RUN/protocol.read.md"
cp "$STUDY/source_snapshot/source_partition_analyzer.cpp" "$RUN/source_snapshot/"
sha256sum "$RUN/source_snapshot/source_partition_analyzer.cpp" > "$RUN/source.sha256"
printf '%q ' g++ -O3 -std=c++17 "$RUN/source_snapshot/source_partition_analyzer.cpp" -o "$RUN/source_partition_analyzer" > "$RUN/commands.txt"
g++ -O3 -std=c++17 "$RUN/source_snapshot/source_partition_analyzer.cpp" -o "$RUN/source_partition_analyzer" > "$RUN/build.log" 2>&1
sha256sum "$RUN/source_partition_analyzer" > "$RUN/binary.sha256"
