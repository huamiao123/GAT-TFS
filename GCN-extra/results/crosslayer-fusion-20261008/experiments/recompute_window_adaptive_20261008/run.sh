#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$ROOT/experiments/recompute_window_adaptive_20261008
BIN=$(cat "$STUDY/LATEST_BUILD")/recompute_window_adaptive
RUN=$ROOT/runs/recompute-window-adaptive-$(date +%Y%m%d-%H%M%S)
mkdir -p "$RUN"
finish(){ s=$?; python3 - "$RUN" "$s" <<'PY'
import json,pathlib,sys,re
r=pathlib.Path(sys.argv[1]); s=int(sys.argv[2]); log=(r/'program.log').read_text() if (r/'program.log').exists() else ''; plans=re.findall(r'PLAN[^\n]+',log); results=re.findall(r'ADAPT_RESULT[^\n]+',log); ok=(s==0 and len(plans)==3 and len(results)==3 and all('exact=1' in x for x in results)); e=dict(id=r.name,kind='recompute_window_adaptive',status='PASS' if ok else 'FAILED',exit_status=s,evidence=str(r),paper_eligible=False,plans=plans,results=results,issues='Adaptive weighted-duplication dispatcher exploratory gate; no paper performance claim',next='Repeat on held-out graphs and calibrate threshold with weighted work model'); (r/'event.json').write_text(json.dumps(e,indent=2)+'\n'); (r/'exit_status.txt').write_text(str(s)+'\n')
PY
python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"; }
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$STUDY/PROTOCOL.md" "$RUN/protocol.read.md"
printf '%s\n' "$BIN" > "$RUN/binary.txt"
: > "$RUN/program.log"
for g in roadNet-CA amazon0601 reddit; do srun --exclusive -N1 -n1 -c32 --mem=16G --time=00:20:00 "$BIN" /home/huangjianqiang_group/hdacp1/data/yx/TFS/data "$g" 32 64 >> "$RUN/program.log" 2>&1; done