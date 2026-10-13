#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$ROOT/experiments/source_partition_hot_20261008
BIN=$(cat "$STUDY/LATEST_BUILD")/source_partition_hot
RUN=$ROOT/runs/source-partition-weighted-$(date +%Y%m%d-%H%M%S)
mkdir -p "$RUN"
finish(){ s=$?; python3 - "$RUN" "$s" <<'PY'
import json,pathlib,sys,re
r=pathlib.Path(sys.argv[1]); s=int(sys.argv[2]); log=(r/'program.log').read_text() if (r/'program.log').exists() else ''; lines=re.findall(r'PARTITION[^\n]+',log); ok=(s==0 and len(lines)==21); e=dict(id=r.name,kind='source_partition_hot',status='PASS' if ok else 'FAILED',exit_status=s,evidence=str(r),paper_eligible=False,partition_lines=lines,issues='Structural weighted source-overlap audit; no runtime claim',next='Use weighted duplication with AMX group utilization in an adaptive plan'); (r/'event.json').write_text(json.dumps(e,indent=2)+'\n'); (r/'exit_status.txt').write_text(str(s)+'\n')
PY
python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"; }
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$STUDY/PROTOCOL.md" "$RUN/protocol.read.md"
printf '%s\n' "$BIN" > "$RUN/binary.txt"
: > "$RUN/program.log"
for g in roadNet-CA amazon0601 reddit; do "$BIN" /home/huangjianqiang_group/hdacp1/data/yx/TFS/data/$g/$g.csrbin 32 >> "$RUN/program.log" 2>&1; done