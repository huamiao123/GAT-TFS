#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
EXP="$ROOT/experiments/journal_decoupling_a3_20261004"
RUN="$ROOT/runs/journal-a3-build-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN/source_snapshot" "$RUN/build" "$RUN/logs" "$RUN/results"
finish(){
 s=$?
 python3 - "$RUN" "$s" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2])
e=dict(id=r.name+'-build',kind='journal_a3_build',status='PASS' if s==0 else 'FAILED',exit_status=s,evidence=str(r),paper_eligible=False,issues='A3 row-order control; original frozen TFS/MKL/B64/FULL/A1 source remains unchanged; default NUMA and fixed numerics',next='Small partial/output bitwise gates before four diagnostic graphs')
(r/'build/event.json').write_text(json.dumps(e,indent=2)+'\n');(r/'build/exit_status.txt').write_text(str(s)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/build/event.json"
 printf 'JOURNAL_A3_BUILD=%s status=%s\n' "$RUN" "$s"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$EXP/"*.hpp "$EXP/"*.py "$EXP/"*.sh "$EXP/PROTOCOL.md" "$EXP/source_state.json" "$RUN/source_snapshot/"
cp "$ROOT/docs/TFS_JOURNAL_STUDY_INPUT_20261004.txt" "$RUN/source_snapshot/"
cp "$ROOT/original/gcn_e2e_v3.cpp" "$RUN/source_snapshot/gcn_e2e_v3.cpp"
cmp "$RUN/source_snapshot/gcn_e2e_v3.cpp" /home/huangjianqiang_group/hdacp1/data/yx/TFS/code/gcn_e2e_v3.cpp
python3 "$RUN/source_snapshot/generate_a3_driver.py" "$RUN/source_snapshot" "$ROOT" > "$RUN/build/generation.stdout.json"
python3 "$RUN/source_snapshot/make_a3_kernel.py" "$RUN/source_snapshot/projection_window_kernels.hpp" "$RUN/build/derived_a3.hpp" > "$RUN/build/a3_derivation.json"
cmp "$RUN/build/derived_a3.hpp" "$RUN/source_snapshot/projection_window_a3_kernels.hpp"
FLAGS=(-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel)
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/projection_methods.cpp" -o "$RUN/build/projection_methods" > "$RUN/build/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/projection_methods.cpp" -o "$RUN/build/projection_methods" > "$RUN/build/compile.log" 2>&1
printf '\n' >> "$RUN/build/commands.txt"
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/gcn_e2e_v3.cpp" -o "$RUN/build/paper_original" >> "$RUN/build/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/gcn_e2e_v3.cpp" -o "$RUN/build/paper_original" >> "$RUN/build/compile.log" 2>&1
find "$RUN/source_snapshot" -type f -print0 | sort -z | xargs -0 sha256sum > "$RUN/source.sha256"
sha256sum "$RUN/build/projection_methods" "$RUN/build/paper_original" > "$RUN/build/binary.sha256"
ldd "$RUN/build/projection_methods" > "$RUN/build/linked_libraries.txt"
objdump -d -C "$RUN/build/projection_methods" > "$RUN/build/disassembly.txt"
python3 - "$RUN" <<'PY'
import json,pathlib,sys
r=pathlib.Path(sys.argv[1]);state=json.loads((r/'source_snapshot/source_state.json').read_text())
manifest=dict(study=r.name,source_state=state,compiler=(r/'compiler.txt').read_text(),flags=['-O3','-march=sapphirerapids','-mamx-bf16','-mamx-tile','-mavx512bf16','-qopenmp','-qmkl=parallel'],numa='default no interleave',allocation='intel shared 32 physical cores on one socket',shape='128->128->128',seed=12345,primary_methods=['bfull_fast','s64_mfull_fast','b64_fast','s64_mfull_rowwise_fast'],speedup_denominator='same graph/process/node/forward protocol only',source_hashes=(r/'source.sha256').read_text(),binary_hashes=(r/'build/binary.sha256').read_text())
(r/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
(r/'README.md').write_text('# Journal A3 control\n\nA0 FULL, A1 visit64/projectFULL, A2 B64, A3 visit64/projectFULL with row-major windows. Only A3 changes the row/window loop order. Partial traces are instrumented in tiny fixtures and compiled out of performance calls.\n\nFrozen input plan: source_snapshot/TFS_JOURNAL_STUDY_INPUT_20261004.txt. Protocol: source_snapshot/PROTOCOL.md.\n')
PY
printf '%s\n' "$RUN" > "$ROOT/build/JOURNAL_A3_LATEST"
