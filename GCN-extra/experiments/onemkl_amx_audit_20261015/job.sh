#!/usr/bin/env bash
set -uo pipefail
RUN=$1
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
BASE=/home/huangjianqiang_group/hdacp1/data/yx/TFS/data
source "$ROOT/scripts/preflight.sh" "$RUN"
export OMP_NUM_THREADS=${SLURM_CPUS_PER_TASK:-32}
export OMP_PROC_BIND=close OMP_PLACES=cores
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS OMP_DYNAMIC KMP_AFFINITY GOMP_CPU_AFFINITY
hostname >"$RUN/node.txt"
lscpu >"$RUN/lscpu.txt"
numactl --show >"$RUN/numa_policy.txt" 2>&1
env | sort >"$RUN/environment.txt"
perf --version >"$RUN/perf_version.txt" 2>&1
perf list >"$RUN/perf_list.txt" 2>&1
cat /proc/sys/kernel/perf_event_paranoid >"$RUN/perf_event_paranoid.txt"
EVENTS=${AUDIT_EVENTS:-amx_ops_retired.bf16,amx_ops_retired.int8,exe.amx_busy,cycles,instructions}
BIN_RUN=${AUDIT_BIN_RUN:-$RUN}
printf 'events=%s\n' "$EVENTS" >"$RUN/protocol.txt"
for graph in email-Enron web-Google; do
  sha256sum "$BASE/$graph/$graph.csrbin" >>"$RUN/graph.sha256"
  for method in mkl_baseline gcn_e2e_v3; do
    if [[ $method == mkl_baseline ]]; then suffix=(128); else suffix=(); fi
    for repeat in 1 2; do
      stem="$RUN/logs/${graph}_${method}_${repeat}"
      MKL_VERBOSE=1 perf stat --no-big-num -x, -e "$EVENTS" -o "$stem.perf.csv" -- \
        "$BIN_RUN/build/$method" "$BASE" "$graph" "${suffix[@]}" >"$stem.stdout" 2>"$stem.stderr"
      code=$?
      printf '%s,%s,%s,%s\n' "$graph" "$method" "$repeat" "$code" >>"$RUN/exit_status.csv"
    done
  done
done
