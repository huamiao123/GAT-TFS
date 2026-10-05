#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
SRC="$ROOT/experiments/feature_precision_8aeef16"
RUN="$ROOT/runs/feature-precision-ablation-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN/source_snapshot" "$RUN/build" "$RUN/logs" "$RUN/results"
finish(){
 status=$?
 python3 - "$RUN" "$status" <<'PY'
import json,pathlib,sys
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2]);e=dict(id=r.name+'-build',kind='frozen_feature_precision_build',status='PASS' if s==0 else 'FAILED',exit_status=s,evidence=str(r),paper_eligible=False,issues='Frozen 8aeef16 BF16 control; minimal FP32 source loads; user authorizes shared nodes',next='Small-fixture correctness before five diagnostic graphs')
(r/'build/event.json').write_text(json.dumps(e,indent=2));(r/'build/exit_status.txt').write_text(str(s)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/build/event.json"
 printf 'FEATURE_PRECISION_BUILD=%s status=%s\n' "$RUN" "$status"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp -r "$SRC/frozen_source" "$SRC/frozen_original" "$SRC/frozen_docs" "$RUN/source_snapshot/"
cp "$SRC/"*.hpp "$SRC/"*.py "$SRC/"*.sh "$SRC/PROTOCOL.md" "$SRC/frozen_manifest.json" "$SRC/source_state.json" "$RUN/source_snapshot/"
cp "$ROOT/docs/CURRENT_GRAPH_INVENTORY_20261004.json" "$RUN/source_snapshot/"
cp -r "$ROOT/docs/references/perfmon_SPR_20261004" "$RUN/source_snapshot/pmu_event_sources"
cmp "$RUN/source_snapshot/frozen_source/gcn_e2e_v3.cpp" "$RUN/source_snapshot/frozen_original/gcn_e2e_v3.cpp"
cmp "$RUN/source_snapshot/frozen_source/gcn_e2e_v3.cpp" /home/huangjianqiang_group/hdacp1/data/yx/TFS/code/gcn_e2e_v3.cpp
python3 "$RUN/source_snapshot/generate_driver.py" "$RUN" > "$RUN/build/generation.stdout.json"
python3 "$RUN/source_snapshot/audit_frozen_control.py" "$RUN/source_snapshot" > "$RUN/build/minimal_edit_audit.json"
FLAGS=(-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel)
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/feature_precision_driver.cpp" -o "$RUN/build/feature_precision_driver" > "$RUN/build/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/feature_precision_driver.cpp" -o "$RUN/build/feature_precision_driver" > "$RUN/build/compile.log" 2>&1
printf '\n' >> "$RUN/build/commands.txt"
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/frozen_source/gcn_e2e_v3.cpp" -o "$RUN/build/paper_original" >> "$RUN/build/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/frozen_source/gcn_e2e_v3.cpp" -o "$RUN/build/paper_original" >> "$RUN/build/compile.log" 2>&1
find "$RUN/source_snapshot" -type f -print0 | sort -z | xargs -0 sha256sum > "$RUN/source.sha256"
sha256sum "$RUN/build/feature_precision_driver" "$RUN/build/paper_original" > "$RUN/build/binary.sha256"
ldd "$RUN/build/feature_precision_driver" > "$RUN/build/linked_libraries.txt"
objdump -d -C "$RUN/build/feature_precision_driver" > "$RUN/build/disassembly.txt"
python3 - "$RUN" <<'PY'
import pathlib,json,sys,hashlib
r=pathlib.Path(sys.argv[1]);state=json.loads((r/'source_snapshot/source_state.json').read_text())
manifest=dict(study=r.name,fixed_commit='8aeef1613fdeaeb929379874c4152eccde620f74',source_state=state,compiler=(r/'compiler.txt').read_text(),flags=['-O3','-march=sapphirerapids','-mamx-bf16','-mamx-tile','-mavx512bf16','-qopenmp','-qmkl=parallel'],generation=json.loads((r/'generation.json').read_text()),partition='intel shared by user instruction',NUMA_policy='default; no explicit interleave',seed=12345,shape=[128,128,128],methods=[dict(method=b+'_'+p,H_dtype=p,B=b) for b in ['b64','full'] for p in ['bf16','fp32']],prepared_e2e='exclude initial H preparation; include path-specific interlayer conversion',preparation_inclusive='separate secondary boundary',final_output='BF16',real_threads=32,socket_count=1,source_hashes=(r/'source.sha256').read_text(),binary_hashes=(r/'build/binary.sha256').read_text())
(r/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
(r/'README.md').write_text('# Feature precision ablation\n\nFrozen 8aeef16 BF16 reduction versus minimal FP32-input control, B64/FULL.\n\nFP32 H0 input is an exact parallel-static copy before prepared timing so page first-touch mirrors BF16 preparation. The prior serial-source first-touch run is retained as a confounded diagnostic under another study.\n\nProtocol: source_snapshot/PROTOCOL.md. Current phase outcomes live in logs/ and results/.\nShared-node 32-core, one-socket data are not mixed with earlier exclusive timings.\n')
PY
printf '%s\n' "$RUN" > "$ROOT/build/FEATURE_PRECISION_LATEST"
