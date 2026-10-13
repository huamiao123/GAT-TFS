#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$ROOT/experiments/recompute_window_interleave_20261008
BIN=$(cat "$STUDY/LATEST_BUILD")/recompute_window_interleave
RUN=$ROOT/runs/recompute-window-interleave-$(date +%Y%m%d-%H%M%S)
mkdir -p "$RUN"
finish(){ s=$?; python3 - "$RUN" "$s" <<'PY'
import json,pathlib,sys,re
r=pathlib.Path(sys.argv[1]); s=int(sys.argv[2]); log=(r/'program.log').read_text() if (r/'program.log').exists() else ''; results=re.findall(r'PAR_RESULT[^\n]+',log); gates=re.findall(r'PAR_GATE[^\n]+',log); passed=(s==0 and len(results)==3 and len(gates)==3 and all('exact=1' in x for x in results) and all('pass=1' in x for x in gates)); e=dict(id=r.name,kind='recompute_window_interleave',status='PASS' if passed else 'FAILED',exit_status=s,evidence=str(r),paper_eligible=False,results=results,gates=gates,issues='Interleaved exclusive-node exploratory timing; no paper performance claim',next='Compare interleaved ratios and finalize source-overlap/packing policy'); (r/'event.json').write_text(json.dumps(e,indent=2)+'\n'); (r/'exit_status.txt').write_text(str(s)+'\n')
PY
python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"; }
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$STUDY/PROTOCOL.md" "$RUN/protocol.read.md"
printf '%s\n' "$BIN" > "$RUN/binary.txt"
srun --exclusive -N1 -n1 -c32 --mem=16G --time=00:20:00 "$BIN" /home/huangjianqiang_group/hdacp1/data/yx/TFS/data amazon0601 32 16 > "$RUN/program.log" 2>&1