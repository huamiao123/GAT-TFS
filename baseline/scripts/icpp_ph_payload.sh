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
if [[ ${ICPP_PH_NUMA_APPLIED:-0} != 1 ]]; then
    export ICPP_PH_NUMA_APPLIED=1
    exec numactl --interleave="$NUMA_NODES" bash "$ROOT/scripts/icpp_ph_payload.sh" "$RUN"
fi
export OMP_NUM_THREADS="$THREADS" MKL_NUM_THREADS="$THREADS"
{
    echo "job=$SLURM_JOB_ID node=$SLURMD_NODENAME threads=$THREADS cpus=$CPUS"
    echo "NUMA=explicit_interleave nodes=$NUMA_NODES"
    echo 'purpose=head_as_row_PH_layout_control_micro_only'
    echo 'variants=original16_scalar_pack,compact8_direct_vector_BF16_pack'
    echo 'arithmetic=independent8heads;FP32_p/den;BF16_high_or_highlow_PH'
    echo 'upstream=B64_own_L1;micro_neighbors=first32;maxsamples=512;warmup=1;reps=3'
    echo 'timing=score_exp_Hgather_Ppack_AMXconfig_loadcompute_store_scatter_included'
    echo 'performance=sampled_hot_instrumented_micro;fullmodel_speedup=NOT_MEASURED'
    echo 'acceptance=independent_quantized_FP64_instruction_gate;master_gate=NOT_EVALUATED'
    numactl --show
    lscpu
    env | grep -E 'OMP_|MKL_' | sort
} > "$RUN/manifest.txt"
for graph in arxiv products; do
    echo "START $graph $(date -Is)"
    stdbuf -oL "$ROOT/build/icpp_ph" "$ROOT/../data/$graph.gatbin" > "$RUN/$graph.log" 2>&1
    echo "DONE $graph $(date -Is)"
done
echo ICPP_PH_CLUSTER_COMPLETE
