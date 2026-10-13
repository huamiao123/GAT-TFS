#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$ROOT/experiments/source_partition_hot_20261008
RUN=$ROOT/runs/source-partition-weighted-build-$(date +%Y%m%d-%H%M%S)
mkdir -p "$RUN/source_snapshot"
finish(){ s=$?; python3 - "$RUN" "$s" <<'PY'
import json,pathlib,sys
r=pathlib.Path(sys.argv[1]); s=int(sys.argv[2]); e=dict(id=r.name,kind='source_partition_hot_build',status='PASS' if s==0 else 'FAILED',exit_status=s,evidence=str(r),paper_eligible=False,issues='Weighted source duplication structural analyzer; no runtime claim',next='Run weighted graph screening if build passes'); (r/'event.json').write_text(json.dumps(e,indent=2)+'\n'); (r/'exit_status.txt').write_text(str(s)+'\n')
PY
python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"; printf '%s\n' "$RUN" > "$STUDY/LATEST_BUILD"; }
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$STUDY/PROTOCOL.md" "$RUN/protocol.read.md"
cp "$STUDY/source_snapshot/source_partition_hot.cpp" "$RUN/source_snapshot/"
sha256sum "$RUN/source_snapshot"/* > "$RUN/source.sha256"
source /etc/profile.d/modules.sh 2>/dev/null || true
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
icpx -O3 -std=c++17 "$RUN/source_snapshot/source_partition_hot.cpp" -o "$RUN/source_partition_hot" > "$RUN/build.log" 2>&1
sha256sum "$RUN/source_partition_hot" > "$RUN/binary.sha256"