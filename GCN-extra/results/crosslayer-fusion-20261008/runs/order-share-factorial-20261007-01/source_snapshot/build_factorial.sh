#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
EXP="$ROOT/experiments/order_share_factorial_20261007"
RUN=$1
test ! -e "$RUN"
mkdir -p "$RUN/build" "$RUN/logs" "$RUN/launcher"
finish(){
 status=$?
 python3 - "$RUN" "$status" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);e=dict(id=r.name+'-build',kind='factorial_build',status='PASS' if sys.argv[2]=='0' else 'FAILED',exit_status=int(sys.argv[2]),evidence=str(r),paper_eligible=False,issues='Isolated immutable build; 128x128x128; default NUMA; no new major issue if PASS',next='Smoke all gates before performance')
(r/'build/event.json').write_text(json.dumps(e,indent=2))
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/build/event.json"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
mkdir "$RUN/source_snapshot"
for f in state_kernels.hpp pair_reducer.hpp state_runtime.hpp grouping_schedule.hpp pair_plan.hpp planned_reducer.hpp planned_kernel.hpp plan_runtime.hpp factorial_runtime.hpp factorial_suite.py evidence_utils.py generate_factorial_driver.py build_factorial.sh factorial_job.sh submit_factorial.py PROTOCOL.md INTERFACE.md; do
  cp "$EXP/$f" "$RUN/source_snapshot/"
done
cp "$ROOT/original/gcn_e2e_v3.cpp" "$ROOT/src/paper_methods_kernels.hpp" "$ROOT/src/paper_methods_runtime.hpp" "$ROOT/src/projection_window_kernels.hpp" "$ROOT/src/projection_window_pmu.hpp" "$RUN/source_snapshot/"
cmp "$RUN/source_snapshot/gcn_e2e_v3.cpp" /home/huangjianqiang_group/hdacp1/data/yx/TFS/code/gcn_e2e_v3.cpp
for f in state_kernels.hpp pair_reducer.hpp state_runtime.hpp grouping_schedule.hpp pair_plan.hpp planned_reducer.hpp planned_kernel.hpp plan_runtime.hpp paper_methods_kernels.hpp paper_methods_runtime.hpp projection_window_kernels.hpp projection_window_pmu.hpp; do
  cmp "$RUN/source_snapshot/$f" "$ROOT/runs/pair-order-control-20261007-01/source_snapshot/$f"
done
export PYTHONDONTWRITEBYTECODE=1
FLAGS=(-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel)
python3 "$RUN/source_snapshot/generate_factorial_driver.py" "$RUN/source_snapshot" "$ROOT" > "$RUN/build/generation.stdout.json"
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/projection_methods.cpp" -o "$RUN/build/factorial_methods" > "$RUN/build/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/projection_methods.cpp" -o "$RUN/build/factorial_methods" > "$RUN/build/compile.log" 2>&1
icpx "${FLAGS[@]}" "$RUN/source_snapshot/gcn_e2e_v3.cpp" -o "$RUN/build/paper_original" >> "$RUN/build/compile.log" 2>&1
objdump -d -C "$RUN/build/factorial_methods" > "$RUN/build/disassembly.txt"
find "$RUN/source_snapshot" -type f -print0 | sort -z | xargs -0 sha256sum > "$RUN/source.sha256"
sha256sum "$RUN/build/factorial_methods" "$RUN/build/paper_original" > "$RUN/build/binary.sha256"
python3 - "$RUN" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);(r/'manifest.json').write_text(json.dumps(dict(root=str(r),source_hashes=(r/'source.sha256').read_text(),binary_hashes=(r/'build/binary.sha256').read_text(),compiler=(r/'compiler.txt').read_text(),commands=(r/'build/commands.txt').read_text(),dimensions=[128,128,128],node_cap=10,numa_interleave=False,old_evidence_unchanged=True),indent=2))
PY
