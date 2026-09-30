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
nodes={int(next(Path(f'/sys/devices/system/cpu/cpu{c}').glob('node[0-9]*')).name[4:]) for c in cpus}
print('CPUS='+','.join(map(str,cpus)))
print('THREADS='+str(len(cpus)))
print('NUMA_NODES='+','.join(map(str,sorted(nodes))))
PY
source "$RUN/pinning.env"
if [[ $THREADS != 16 ]]; then echo 'FAIL: comparison requires 16 physical cores on one socket'; exit 1; fi
taskset -pc "$CPUS" $$ > "$RUN/affinity.txt"
if [[ ${HISTORY_NUMA_APPLIED:-0} != 1 ]]; then
  export HISTORY_NUMA_APPLIED=1
  exec numactl --interleave="$NUMA_NODES" bash "$ROOT/scripts/history_payload.sh" "$RUN"
fi
export OMP_NUM_THREADS="$THREADS" MKL_NUM_THREADS="$THREADS"
{
 echo "job=$SLURM_JOB_ID node=$SLURMD_NODENAME partition=$SLURM_JOB_PARTITION"
 echo "threads=$THREADS cpus=$CPUS numa_interleave=$NUMA_NODES"
 echo 'real_graphs=ogbn-arxiv,ogbn-products parameters=synthetic_seed_11_not_trained'
 echo 'bridge_boundary=allocation_and_weight_preparation_inclusive_forward'
 echo 'production_boundary=preallocated_forward; timing_boundaries_must_not_be_mixed'
 echo 'warmups=1 repeats=3 shared_node_not_paper_acceptance'
 numactl --show
 lscpu
 env | grep -E 'OMP_|MKL_' | sort
} > "$RUN/manifest.txt"
"$ROOT/build/test_boundaries" "$RUN" > "$RUN/boundaries.log" 2>&1
"$ROOT/build/compare_history" "$RUN/oracle.gatbin" "$THREADS" 0 1 > "$RUN/smoke_bridge.log" 2>&1
for dataset in arxiv products; do
 graph="$ROOT/../data/$dataset.gatbin"
 echo "START $dataset bridge $(date -Is)"
 "$ROOT/build/compare_history" "$graph" "$THREADS" 1 3 > "$RUN/${dataset}_bridge.log" 2>&1
 echo "DONE $dataset bridge $(date -Is)"
 for path in standard_fp32 standard_bf16 tfs_bf16; do
   echo "START $dataset production $path $(date -Is)"
   "$ROOT/build/gat_baseline" --graph "$graph" --seed 11 --path "$path" --mode benchmark --threads "$THREADS" --warmups 1 --repeats 3 > "$RUN/${dataset}_${path}.log" 2>&1
 done
done
echo HISTORY_COMPLETE
