#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline
RUN="$1"
python3 - <<'PY' > "$RUN/pinning.env"
import os
from pathlib import Path
by={}
for c in sorted(os.sched_getaffinity(0)):
 s=int(Path(f'/sys/devices/system/cpu/cpu{c}/topology/physical_package_id').read_text())
 by.setdefault(s,[]).append(c)
cpus=max(by.values(),key=len)[:16]
print('CPUS='+','.join(map(str,cpus)))
print('THREADS='+str(len(cpus)))
PY
source "$RUN/pinning.env"
taskset -pc "$CPUS" $$ > "$RUN/affinity.txt"
export OMP_NUM_THREADS="$THREADS" MKL_NUM_THREADS="$THREADS"
lscpu > "$RUN/lscpu.txt"
env | sort > "$RUN/environment.txt"
"$ROOT/build/test_boundaries" "$RUN" > "$RUN/boundaries.log" 2>&1
if [[ ${GAT_DIAG_MEMORY:-0} == 1 ]]; then
  python3 - <<'PY' > "$RUN/memory.env"
import os
from pathlib import Path
by={}
for c in sorted(os.sched_getaffinity(0)):
 nodes=list(Path(f'/sys/devices/system/cpu/cpu{c}').glob('node[0-9]*'))
 n=int(nodes[0].name[4:])
 by.setdefault(n,[]).append(c)
print('NODES='+','.join(map(str,sorted(by))))
print('LARGEST_NODE='+str(max(by,key=lambda n:len(by[n]))))
PY
  source "$RUN/memory.env"
  for policy in bind interleave; do
    if [[ $policy == bind ]]; then memory=(--membind="$LARGEST_NODE"); else memory=(--interleave="$NODES"); fi
    numactl "${memory[@]}" --show > "$RUN/${policy}_numactl.txt" 2>&1
    for pair in '0 standard_fp32' '1 standard_bf16' '2 standard_bf16' '3 standard_fp32'; do
      read -r order path <<< "$pair"
      numactl "${memory[@]}" "$ROOT/build/gat_baseline" --graph "$ROOT/../data/arxiv.gatbin" --seed 11 --mode benchmark --path "$path" --threads "$THREADS" --init-policy warm --diagnostics "$RUN/${policy}_${order}_${path}" --warmups 3 --repeats 9 --output "$RUN/${policy}_${order}_${path}.f32" > "$RUN/${policy}_${order}_${path}.log" 2>&1
    done
  done
  sha256sum "$RUN"/*.f32 > "$RUN/output.sha256"
  echo MEMORY_DIAGNOSE_COMPLETE
  exit 0
fi
for init in original warm; do
  # ABBA reduces bias from sequential shared-node load drift.
  for pair in '0 standard_fp32' '1 standard_bf16' '2 standard_bf16' '3 standard_fp32'; do
    read -r order path <<< "$pair"
    "$ROOT/build/gat_baseline" --graph "$ROOT/../data/arxiv.gatbin" --seed 11 --mode benchmark --path "$path" --threads "$THREADS" --init-policy "$init" --diagnostics "$RUN/${init}_${order}_${path}" --warmups 3 --repeats 9 > "$RUN/${init}_${order}_${path}.log" 2>&1
  done
done
echo DIAGNOSE_COMPLETE
