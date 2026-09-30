#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline
RUN="$1"
taskset -pc $$ > "$RUN/step_affinity_before.txt"
python3 - <<'PY' > "$RUN/pinning.env"
import os
from pathlib import Path
by_socket = {}
for cpu in sorted(os.sched_getaffinity(0)):
    topo = Path(f'/sys/devices/system/cpu/cpu{cpu}/topology')
    socket = int((topo/'physical_package_id').read_text())
    core = int((topo/'core_id').read_text())
    by_socket.setdefault(socket, {}).setdefault(core, cpu)
socket = max(by_socket, key=lambda x: len(by_socket[x]))
cpus = sorted(by_socket[socket].values())[:int(os.environ['SLURM_CPUS_PER_TASK'])]
print('GAT_CPUS=' + ','.join(map(str, cpus)))
print('GAT_THREADS=' + str(len(cpus)))
print('GAT_SOCKET=' + str(socket))
PY
source "$RUN/pinning.env"
if [[ $GAT_THREADS -lt 2 ]]; then echo 'FAIL: requested multi-core validation received fewer than two cores' >&2; exit 1; fi
taskset -pc "$GAT_CPUS" $$ > "$RUN/affinity.txt"
# A socket has four NUMA domains on CPU Max 9462. Serial first-touch
# after path-dependent OpenMP initialization made earlier comparisons unfair.
# Apply one explicit page policy before ANY benchmark child allocates tensors.
if [[ ${GAT_NUMA_APPLIED:-0} != 1 ]]; then
  GAT_NUMA_NODES=$(python3 - <<'PY'
import os
from pathlib import Path
nodes={int(next(Path(f'/sys/devices/system/cpu/cpu{c}').glob('node[0-9]*')).name[4:]) for c in os.sched_getaffinity(0)}
print(','.join(map(str,sorted(nodes))))
PY
)
  export GAT_NUMA_NODES GAT_NUMA_APPLIED=1
  exec numactl --interleave="$GAT_NUMA_NODES" bash "$ROOT/scripts/arxiv_payload.sh" "$RUN"
fi
export OMP_NUM_THREADS="$GAT_THREADS" MKL_NUM_THREADS="$GAT_THREADS"
{
  echo "job=$SLURM_JOB_ID node=$SLURMD_NODENAME partition=$SLURM_JOB_PARTITION"
  echo "physical_threads=$GAT_THREADS socket=$GAT_SOCKET cpus=$GAT_CPUS"
  echo 'precision=RNE BF16, FP32 state; random checkpoint seed=11; task_gate=UNCONFIGURED'
  echo 'performance=exploratory_shared_node; warmups=2 repeats=7; not paper acceptance'
  echo "NUMA=explicit_interleave nodes=$GAT_NUMA_NODES"
  numactl --show
  lscpu
  env | grep -E 'OMP_|MKL_' | sort
} > "$RUN/manifest.txt"
# Regression precedes any speed measurement.
"$ROOT/build/test_boundaries" "$RUN" > "$RUN/boundaries.log" 2>&1
"$ROOT/build/gat_baseline" --graph "$RUN/oracle.gatbin" --weights "$RUN/oracle.weights" --mode correctness --threads "$GAT_THREADS" --dump-prefix "$RUN/oracle" > "$RUN/oracle_cpp.log" 2>&1
python3 "$ROOT/tests/oracle.py" --graph "$RUN/oracle.gatbin" --weights "$RUN/oracle.weights" --dump-prefix "$RUN/oracle" > "$RUN/oracle_numpy.log" 2>&1
python3 "$ROOT/tests/evaluate_task_test.py" "$RUN/evaluator_fixture" > "$RUN/evaluator.log" 2>&1
GRAPH="$ROOT/../data/arxiv.gatbin"
"$ROOT/build/gat_baseline" --graph "$GRAPH" --seed 11 --save-weights "$RUN/arxiv.weights" --mode correctness --threads "$GAT_THREADS" > "$RUN/arxiv_correctness.log" 2>&1
sha256sum "$RUN/arxiv.weights" > "$RUN/weights.sha256"
for layer in 1 2 3; do
  MKL_VERBOSE=1 "$ROOT/build/gat_baseline" --graph "$GRAPH" --weights "$RUN/arxiv.weights" --mode micro --threads "$GAT_THREADS" --layer "$layer" --warmups 2 --repeats 7 > "$RUN/micro${layer}.log" 2>&1
  "$ROOT/build/gat_baseline" --graph "$GRAPH" --weights "$RUN/arxiv.weights" --mode export-fixture --threads "$GAT_THREADS" --layer "$layer" --fixture "$RUN/layer${layer}.fixture" > "$RUN/export${layer}.log" 2>&1
  "$ROOT/build/gat_baseline" --graph "$GRAPH" --weights "$RUN/arxiv.weights" --mode fixed-p --threads "$GAT_THREADS" --layer "$layer" --fixture "$RUN/layer${layer}.fixture" --warmups 2 --repeats 7 > "$RUN/fixed_p${layer}.log" 2>&1
done
for path in standard_fp32 standard_bf16 tfs_bf16 matched_attention; do
  "$ROOT/build/gat_baseline" --graph "$GRAPH" --weights "$RUN/arxiv.weights" --mode benchmark --threads "$GAT_THREADS" --path "$path" --warmups 2 --repeats 7 > "$RUN/model_${path}.log" 2>&1
done
"$ROOT/build/gat_baseline_profile" --graph "$GRAPH" --weights "$RUN/arxiv.weights" --mode profile --threads "$GAT_THREADS" --path tfs_bf16 --warmups 1 --repeats 2 > "$RUN/profile.log" 2>&1
"$ROOT/build/gat_baseline_profile" --graph "$GRAPH" --weights "$RUN/arxiv.weights" --mode profile --threads "$GAT_THREADS" --path standard_bf16 --warmups 1 --repeats 2 > "$RUN/profile_standard.log" 2>&1
# Scope vendor AMX counter to a process that executes only the standard path.
# The TFS function is linked but never invoked in this process.
if command -v perf >/dev/null; then
  set +e
  perf stat -e amx_ops_retired.bf16 -o "$RUN/vendor_amx_perf.txt" "$ROOT/build/gat_baseline" --graph "$GRAPH" --weights "$RUN/arxiv.weights" --mode benchmark --path standard_bf16 --threads "$GAT_THREADS" --warmups 1 --repeats 2 > "$RUN/vendor_amx_stdout.log" 2>&1
  echo "perf_status=$?" > "$RUN/vendor_amx_status.txt"
  set -e
fi
echo 'ARXIV_COMPLETE structural_correctness=PASS BF16_task_gate=UNCONFIGURED'
