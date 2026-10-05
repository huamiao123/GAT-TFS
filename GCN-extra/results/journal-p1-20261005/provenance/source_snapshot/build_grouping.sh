#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
EXP="$ROOT/experiments/journal_grouping_p1_20261005"
RUN="$ROOT/runs/journal-p1-build-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN/source_snapshot" "$RUN/build" "$RUN/logs" "$RUN/results"
finish(){
 s=$?
 python3 - "$RUN" "$s" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2]);e=dict(id=r.name+'-build',kind='journal_p1_build',status='PASS' if s==0 else 'FAILED',exit_status=s,evidence=str(r),paper_eligible=False,issues='P1 budget-preserving grouping, unchanged P0 kernels/TFS/MKL, no interleave',next='Smoke correctness/work gates then five diagnostic graphs and controlled sharing graphs')
(r/'build/event.json').write_text(json.dumps(e,indent=2)+'\n');(r/'build/exit_status.txt').write_text(str(s)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/build/event.json"
 printf 'JOURNAL_P1_BUILD=%s status=%s\n' "$RUN" "$s"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$EXP/"*.hpp "$EXP/"*.cpp "$EXP/"*.py "$EXP/"*.sh "$EXP/PROTOCOL.md" "$EXP/source_state.json" "$RUN/source_snapshot/"
cp "$ROOT/docs/TFS_JOURNAL_STUDY_INPUT_20261004.txt" "$RUN/source_snapshot/"
cp "$ROOT/original/gcn_e2e_v3.cpp" "$RUN/source_snapshot/gcn_e2e_v3.cpp"
cmp "$RUN/source_snapshot/gcn_e2e_v3.cpp" /home/huangjianqiang_group/hdacp1/data/yx/TFS/code/gcn_e2e_v3.cpp
python3 "$RUN/source_snapshot/generate_grouping_driver.py" "$RUN/source_snapshot" "$ROOT" > "$RUN/build/generation.stdout.json"
FLAGS=(-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel)
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/projection_methods.cpp" -o "$RUN/build/projection_methods" > "$RUN/build/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/projection_methods.cpp" -o "$RUN/build/projection_methods" > "$RUN/build/compile.log" 2>&1
printf '\n' >> "$RUN/build/commands.txt"
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/gcn_e2e_v3.cpp" -o "$RUN/build/paper_original" >> "$RUN/build/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/gcn_e2e_v3.cpp" -o "$RUN/build/paper_original" >> "$RUN/build/compile.log" 2>&1
printf '\n' >> "$RUN/build/commands.txt"
printf '%q ' icpx "${FLAGS[@]}" -lstdc++fs "$RUN/source_snapshot/generate_synthetic.cpp" -o "$RUN/build/generate_synthetic" >> "$RUN/build/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/generate_synthetic.cpp" -lstdc++fs -o "$RUN/build/generate_synthetic" >> "$RUN/build/compile.log" 2>&1
find "$RUN/source_snapshot" -type f -print0 | sort -z | xargs -0 sha256sum > "$RUN/source.sha256"
sha256sum "$RUN/build/projection_methods" "$RUN/build/paper_original" "$RUN/build/generate_synthetic" > "$RUN/build/binary.sha256"
ldd "$RUN/build/projection_methods" > "$RUN/build/linked_libraries.txt"
objdump -d -C "$RUN/build/projection_methods" > "$RUN/build/disassembly.txt"
python3 - "$RUN" <<'PY'
import json,pathlib,sys
r=pathlib.Path(sys.argv[1]);state=json.loads((r/'source_snapshot/source_state.json').read_text());m=dict(study=r.name,source_state=state,compiler=(r/'compiler.txt').read_text(),flags=['-O3','-march=sapphirerapids','-mamx-bf16','-mamx-tile','-mavx512bf16','-qopenmp','-qmkl=parallel'],numa='default no interleave',allocation='intel shared physical cores on one socket',shape='128->128->128',seed=12345,segment_rows=4096,signature_neighbors=64,source_hashes=(r/'source.sha256').read_text(),binary_hashes=(r/'build/binary.sha256').read_text())
(r/'manifest.json').write_text(json.dumps(m,indent=2)+'\n');(r/'README.md').write_text('# Journal P1 grouping\n\nProtocol: source_snapshot/PROTOCOL.md. Same-q64 bounded destination grouping, frozen P0 S64/M64 and S64/MFULL kernels, same-source TFS/MKL anchors.\n')
PY
printf '%s\n' "$RUN" > "$ROOT/build/JOURNAL_P1_LATEST"
