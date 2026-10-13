#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
BIN=$(cat "$ROOT/experiments/source_partition_weighted_20261008/LATEST_BUILD")/source_partition_weighted
RUN=$ROOT/runs/source-partition-weighted-roadusa-$(date +%Y%m%d-%H%M%S)
mkdir -p "$RUN"
finish(){ s=$?; python3 - "$RUN" "$s" <<'PY'
import json,pathlib,sys,re
r=pathlib.Path(sys.argv[1]); s=int(sys.argv[2]); log=(r/'program.log').read_text() if (r/'program.log').exists() else ''; lines=re.findall(r'PARTITION[^\n]+',log); ok=(s==0 and len(lines)==6); e=dict(id=r.name,kind='source_partition_weighted_roadusa',status='PASS' if ok else 'FAILED',exit_status=s,evidence=str(r),paper_eligible=False,partition_lines=lines,issues='Structural weighted source-overlap audit; no runtime claim',next='If weighted duplication is low, run adaptive held-out validation'); (r/'event.json').write_text(json.dumps(e,indent=2)+'\n'); (r/'exit_status.txt').write_text(str(s)+'\n')
PY
python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"; }
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
printf '%s\n' "$BIN" > "$RUN/binary.txt"
"$BIN" /home/huangjianqiang_group/hdacp1/data/yx/TFS/data/road_usa/road_usa.csrbin 32 > "$RUN/program.log" 2>&1