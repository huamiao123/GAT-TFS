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
if [[ ${JOINT_NUMA_APPLIED:-0} != 1 ]]; then
 export JOINT_NUMA_APPLIED=1
 exec numactl --interleave="$NUMA_NODES" bash "$ROOT/scripts/joint_payload.sh" "$RUN"
fi
export OMP_NUM_THREADS="$THREADS" MKL_NUM_THREADS="$THREADS"
{
 echo "job=$SLURM_JOB_ID node=$SLURMD_NODENAME threads=$THREADS cpus=$CPUS"
 echo "NUMA=explicit_interleave nodes=$NUMA_NODES"
 echo 'purpose=independent_head_source_reuse_FP32_local_U_online; baselines_unchanged'
 echo 'precision_joint=FP32; control=B0_FP32,B0_BF16,B1_TFS,old_local,joint_g1,g2,g4,g8'
 echo 'warmups=1 repeats=3 alternating_order; profile_separate; seed=11 untrained=true'
 echo 'master_reference_gate=UNVERIFIED; task_accuracy=UNVERIFIED; performance=EXPLORATORY'
 numactl --show
 lscpu
 env | grep -E 'OMP_|MKL_' | sort
} > "$RUN/manifest.txt"
"$ROOT/build/test_boundaries" "$RUN" > "$RUN/boundaries.log" 2>&1
"$ROOT/build/joint_online" --smoke > "$RUN/joint_smoke.log" 2>&1
"$ROOT/build/joint_online" "$RUN/oracle.gatbin" "$THREADS" 1 3 > "$RUN/fixture_model.log" 2>&1
for dataset in arxiv products; do
 echo "START $dataset $(date -Is)"
 stdbuf -oL "$ROOT/build/joint_online" "$ROOT/../data/$dataset.gatbin" "$THREADS" 1 3 > "$RUN/${dataset}_joint.log" 2>&1
 echo "DONE $dataset $(date -Is)"
done
echo JOINT_CLUSTER_COMPLETE
