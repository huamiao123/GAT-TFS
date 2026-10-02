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
if [[ ${ICPP_HEADS_NUMA_APPLIED:-0} != 1 ]]; then
    export ICPP_HEADS_NUMA_APPLIED=1
    exec numactl --interleave="$NUMA_NODES" bash "$ROOT/scripts/icpp_preload_payload.sh" "$RUN"
fi
export OMP_NUM_THREADS="$THREADS" MKL_NUM_THREADS="$THREADS"
{
    echo "job=$SLURM_JOB_ID node=$SLURMD_NODENAME threads=$THREADS cpus=$CPUS"
    echo "NUMA=explicit_interleave nodes=$NUMA_NODES"
    echo 'purpose=preloaded_Phi_Plo_into_TMM6_TMM2_once_per_destination_keeping_DegreeSort_TR16_AMX_UW'
    echo 'controls=B0_FP32,B0_BF16,B1_TFS_PREMAX,ICPP_TFS_PAIR_B32,ICPP_TFS_BLOCK_B32;candidate=HEADS_AVX_B16_B32,HEADS_AMX_HILO_B16_B32,HEADS_AMX_PRELOAD_B16_B32'
    echo 'block_local_U=immediately_consumed_by_AMX;attention/W/m/l/V_independent;no_fullZ/U/e/alpha;V_worker_local_count_all_head_switch_traffic;new_PH_high_low_RNE_interface'
    echo 'model=8x32,8x32,1xC;seed=11;untrained=true;LR=fp32;blocks=16,32'
    echo 'timing=preallocated_three_layer_own_outputs;check_runs=1;warmups=1;repeats=3;alternating_order=true'
    echo 'profile=separate_source_index_score_max_exp_Hpack_Ppack_PHload_PHcompute_PHstore_UBconvert_UWload_UWcompute_Vstore_Vreload_rescale_transition;worker_sums_not_wall_percentages'
    echo 'correctness=old_heads_independent_oracle_plus_18operator_6layer_bitwise_preload_regression_before_speed;master_abs_gate=.003_unchanged'
    echo 'performance=EXPLORATORY;task_accuracy=UNVERIFIED'
    echo 'extra_diagnostics=same_input_contracted_attention_L2;profile_fingerprint_and_exact_work_identities;full_three_layer_own_output_E2E'
    numactl --show
    lscpu
    env | grep -E 'OMP_|MKL_' | sort
} > "$RUN/manifest.txt"
echo "START smoke $(date -Is)"
"$ROOT/build/icpp_preload" --smoke > "$RUN/smoke.log" 2>&1
echo "DONE smoke $(date -Is)"
for graph in arxiv products; do
    echo "START $graph $(date -Is)"
    stdbuf -oL "$ROOT/build/icpp_preload" "$ROOT/../data/$graph.gatbin" "$THREADS" 1 3 128 > "$RUN/$graph.log" 2>&1
    echo "DONE $graph $(date -Is)"
done
echo ICPP_HEADS_CLUSTER_COMPLETE
