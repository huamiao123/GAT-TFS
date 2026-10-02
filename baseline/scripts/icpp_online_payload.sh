#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline
RUN="$1"
python3 - <<'PY' > "$RUN/pinning.env"
import os
from pathlib import Path
by={}
for c in sorted(os.sched_getaffinity(0)):
    socket=int(Path(f'/sys/devices/system/cpu/cpu{c}/topology/physical_package_id').read_text())
    core=int(Path(f'/sys/devices/system/cpu/cpu{c}/topology/core_id').read_text())
    by.setdefault(socket,{}).setdefault(core,c)
cpus=sorted(max(by.values(),key=len).values())[:16]
nodes={int(next(Path(f'/sys/devices/system/cpu/cpu{c}').glob('node[0-9]*')).name[4:]) for c in cpus}
print('CPUS='+','.join(map(str,cpus)))
print('THREADS='+str(len(cpus)))
print('NUMA_NODES='+','.join(map(str,sorted(nodes))))
PY
source "$RUN/pinning.env"
if [[ $THREADS != 16 ]]; then echo 'FAIL: need 16 physical cores on one socket'; exit 1; fi
taskset -pc "$CPUS" $$ > "$RUN/affinity.txt"
if [[ ${ICPP_ONLINE_NUMA_APPLIED:-0} != 1 ]]; then
    export ICPP_ONLINE_NUMA_APPLIED=1
    exec numactl --interleave="$NUMA_NODES" bash "$ROOT/scripts/icpp_online_payload.sh" "$RUN"
fi
export OMP_NUM_THREADS="$THREADS" MKL_NUM_THREADS="$THREADS"
{
    echo "job=$SLURM_JOB_ID node=$SLURMD_NODENAME threads=$THREADS cpus=$CPUS"
    echo "NUMA=explicit_interleave nodes=$NUMA_NODES"
    echo 'purpose=preserve_ICPP_TFS_neighbor_step_AMX_fusion_with_block_Online_output_V_rescale'
    echo 'controls=B0_FP32,B0_BF16,B1_TFS_PREMAX; candidates=ICPP_TFS_ONLINE_B16,B32,B64'
    echo 'original_baselines_unchanged=true seed=11 untrained=true warmups=1 repeats=3'
    echo 'timing=preallocated_full_three_layer_own_outputs; static_prepare_sort_and_checks_excluded'
    echo 'correctness=FP32_V_simulator_before_AMX_emulator_before_real_graph; original_master_abs_gate=.003'
    echo 'profile=sampled_worker_sums_separate_from_speed; counters=exact; task_accuracy=UNVERIFIED; performance=EXPLORATORY'
    numactl --show
    lscpu
    env | grep -E 'OMP_|MKL_' | sort
} > "$RUN/manifest.txt"
echo "START smoke $(date -Is)"
"$ROOT/build/icpp_online" --smoke > "$RUN/smoke.log" 2>&1
echo "DONE smoke $(date -Is)"
echo "START arxiv $(date -Is)"
stdbuf -oL "$ROOT/build/icpp_online" "$ROOT/../data/arxiv.gatbin" "$THREADS" 1 3 16,32,64 64 > "$RUN/arxiv.log" 2>&1
echo "DONE arxiv $(date -Is)"
echo "START products $(date -Is)"
stdbuf -oL "$ROOT/build/icpp_online" "$ROOT/../data/products.gatbin" "$THREADS" 1 3 32 128 > "$RUN/products.log" 2>&1
echo "DONE products $(date -Is)"
echo ICPP_ONLINE_CLUSTER_COMPLETE
