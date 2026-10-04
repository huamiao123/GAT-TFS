#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
RUN=$1
test -f "$RUN/reconciliation.event.json"
python3 "$ROOT/scripts/record_event.py" check
cd "$ROOT"
ARCHIVE="evidence-source-mkl-protocol-20261004.tar.gz"
test ! -e "$ARCHIVE"
python3 - "$ROOT" "$RUN" <<'PY'
import hashlib,json,pathlib,sys
root=pathlib.Path(sys.argv[1]);run=pathlib.Path(sys.argv[2])
paths=list(root.glob('src/source_protocol*'))+list(root.glob('scripts/*source*'))
paths+=list(root.glob('docs/SOURCE_*20261004*'))+list(root.glob('original/protocol_scripts/*'))
paths+=[root/'README.md',root/'original/mkl_baseline.cpp',root/'original/gcn_e2e_bench.cpp',root/'original/gcn_e2e_v3.cpp']
records=[{'path':str(p.relative_to(root)),'sha256':hashlib.sha256(p.read_bytes()).hexdigest()} for p in paths if p.is_file()]
(run/'final_source_manifest.json').write_text(json.dumps(records,indent=2)+'\n')
PY
tar -czf "$ARCHIVE" README.md .gitignore IMPLEMENTATION_CONTRACT.md src scripts original docs/handoff docs/SOURCE_*20261004* runs/source-*
sha256sum "$ARCHIVE" > "$ARCHIVE.sha256"
printf 'SOURCE_EVIDENCE=%s/%s\n' "$ROOT" "$ARCHIVE"
