#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RUN="$ROOT/runs/build-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN"
RULES=/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md
SKILL=/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md
cat "$RULES" > "$RUN/AGENTS.read.txt"
cat "$SKILL" > "$RUN/SKILL.read.txt"
sha256sum "$RULES" "$SKILL" "$ROOT/IMPLEMENTATION_CONTRACT.md" > "$RUN/preflight.sha256"
squeue -u hdacp1 > "$RUN/queue.txt"
sinfo -p intel > "$RUN/partition.txt"
find "$ROOT/include" "$ROOT/src" "$ROOT/bench" "$ROOT/tests" "$ROOT/scripts" -type f -exec sha256sum {} + > "$RUN/source.sha256"
printf '%s\n' 'scope=GAT forward; TFS-Train GCN architecture specifications do not apply' 'targets=baseline/build and baseline/runs only' > "$RUN/manifest.txt"
set +e
bash "$ROOT/scripts/build.sh" > "$RUN/build.log" 2>&1
status=$?
set -e
cat "$RUN/build.log"
echo "status=$status run=$RUN"
echo "$status" > "$RUN/exit_status.txt"
if [[ $status == 0 ]]; then sha256sum "$ROOT"/build/gat_baseline* "$ROOT/build/test_boundaries" > "$RUN/binary.sha256"; fi
exit "$status"
