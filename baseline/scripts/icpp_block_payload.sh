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
print('ALLOCATION_SOCKET_CORE_COUNTS='+','.join(str(k)+':'+str(len(v)) for k,v in sorted(by.items())))
nodes={int(next(Path(f'/sys/devices/system/cpu/cpu{c}').glob('node[0-9]*')).name[4:]) for c in cpus}
print('CPUS='+','.join(map(str,cpus)))
print('THREADS='+str(len(cpus)))
print('NUMA_NODES='+','.join(map(str,sorted(nodes))))
PY
source "$RUN/pinning.env"
if [[ $THREADS != 16 ]]; then echo 'FAIL: need16physicalcores on one socket'; exit 1; fi
taskset -pc "$CPUS" $$ > "$RUN/affinity.txt"
if [[ ${ICPP_BLOCK_NUMA_APPLIED:-0} != 1 ]]; then
    export ICPP_BLOCK_NUMA_APPLIED=1
    exec numactl --interleave="$NUMA_NODES" bash "$ROOT/scripts/icpp_block_payload.sh" "$RUN"
fi
export OMP_NUM_THREADS="$THREADS" MKL_NUM_THREADS="$THREADS"
{
    echo "job=$SLURM_JOB_ID node=$SLURMD_NODENAME threads=$THREADS cpus=$CPUS"
    echo "NUMA=explicit_interleave nodes=$NUMA_NODES"
    echo 'purpose=block_SpMM_AMX_GeMM_fusion_retaining_DegreeSort_TR16_residentV'
    echo 'controls=B0_FP32,B0_BF16,B1_TFS_PREMAX,ICPP_TFS_ONLINE_B32;candidate=ICPP_TFS_BLOCK_B16_B32_B64'
    echo 'block_local_U=immediately_consumed_by_AMX;attention/W/m/l/V_independent;no_fullZ/U;new_block_RNE_interface'
    echo 'model=8x32,8x32,1xC;seed=11;untrained=true;LR=fp32;blocks=16,32,64'
    echo 'timing=preallocated_three_layer_own_outputs;warmups=1;repeats=3;alternating_order=true'
    echo 'profile=separate_coarse_staging_and_AMX_load_compute_worker_spans;not_wall_percentages'
    echo 'correctness=original_then_pair_then_block_independent_oracles_before_speed;master_abs_gate=.003_unchanged'
    echo 'performance=EXPLORATORY;task_accuracy=UNVERIFIED'
    echo 'extra_diagnostics=systematic_graph_reuse_samples_and_head_as_row_PH_microbench;not_E2E_speedup'
    numactl --show
    lscpu
    env | grep -E 'OMP_|MKL_' | sort
} > "$RUN/manifest.txt"
echo "START smoke $(date -Is)"
"$ROOT/build/icpp_block" --smoke > "$RUN/smoke.log" 2>&1
echo "DONE smoke $(date -Is)"
for graph in arxiv products; do
    echo "START $graph $(date -Is)"
    stdbuf -oL "$ROOT/build/icpp_block" "$ROOT/../data/$graph.gatbin" "$THREADS" 1 3 128 > "$RUN/$graph.log" 2>&1
    echo "DONE $graph $(date -Is)"
done
echo ICPP_BLOCK_CLUSTER_COMPLETE
