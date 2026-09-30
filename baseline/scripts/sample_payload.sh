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
if [[ $THREADS != 16 ]]; then echo 'FAIL: need 16 physical cores on one socket'; exit 1; fi
taskset -pc "$CPUS" $$ > "$RUN/affinity.txt"
if [[ ${SAMPLE_NUMA_APPLIED:-0} != 1 ]]; then
 export SAMPLE_NUMA_APPLIED=1
 exec numactl --interleave="$NUMA_NODES" bash "$ROOT/scripts/sample_payload.sh" "$RUN"
fi
export OMP_NUM_THREADS="$THREADS" MKL_NUM_THREADS="$THREADS"
{
 echo "job=$SLURM_JOB_ID node=$SLURMD_NODENAME threads=$THREADS cpus=$CPUS"
 echo "NUMA=explicit_interleave nodes=$NUMA_NODES"
 echo 'purpose=B1_coarse_sample_attribution precision=BF16_RNE_FP32_state random_seed=11'
 echo 'control=original_B1,sampler_no_clocks; periods=128,512; warmups=1 repeats=3'
 echo 'phases=setup_prefetch,score_den,gather_weight_RNE_store,tile_load_plus_compute,output'
 echo 'all_worker_times_are_sample_sums_not_wall; task_accuracy=NOT_ACCEPTED'
 numactl --show
 lscpu
 env | grep -E 'OMP_|MKL_' | sort
} > "$RUN/manifest.txt"
"$ROOT/build/test_boundaries" "$RUN" > "$RUN/boundaries.log" 2>&1
"$ROOT/build/sample_tfs" "$RUN/oracle.gatbin" "$THREADS" 0 1 > "$RUN/smoke.log" 2>&1
for dataset in arxiv products; do
 echo "START $dataset $(date -Is)"
 stdbuf -oL "$ROOT/build/sample_tfs" "$ROOT/../data/$dataset.gatbin" "$THREADS" 1 3 > "$RUN/${dataset}_sample.log" 2>&1
 echo "DONE $dataset $(date -Is)"
done
if command -v perf >/dev/null; then
 set +e
 perf record -e cycles:u -F 199 -o "$RUN/products_cycles.data" -- "$ROOT/build/gat_baseline" --graph "$ROOT/../data/products.gatbin" --seed 11 --mode benchmark --path tfs_bf16 --threads "$THREADS" --warmups 1 --repeats 3 > "$RUN/perf_stdout.log" 2>&1
 status=$?
 echo "$status" > "$RUN/perf_status.txt"
 if [[ $status == 0 ]]; then
   perf report -i "$RUN/products_cycles.data" --stdio --sort symbol --percent-limit 0.5 > "$RUN/perf_symbols.txt" 2>&1
   perf annotate -i "$RUN/products_cycles.data" --stdio --stdio-color never > "$RUN/perf_annotate.txt" 2>&1
   echo "$?" > "$RUN/perf_annotate_status.txt"
 fi
 set -e
fi
echo SAMPLE_COMPLETE
