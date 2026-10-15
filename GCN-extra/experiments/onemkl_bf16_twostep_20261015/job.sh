#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
BASE=/home/huangjianqiang_group/hdacp1/data/yx/TFS/data
STUDY=$1
MODE=$2
RUN=$3
mkdir -p "$RUN/logs"
source "$ROOT/scripts/preflight.sh" "$RUN"
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS OMP_DYNAMIC KMP_AFFINITY GOMP_CPU_AFFINITY
export OMP_NUM_THREADS=${SLURM_CPUS_PER_TASK}
export OMP_PROC_BIND=close OMP_PLACES=cores PYTHONDONTWRITEBYTECODE=1
hostname >"$RUN/node.txt"
lscpu >"$RUN/lscpu.txt"
numactl --show >"$RUN/numa_policy.txt" 2>&1
env | sort >"$RUN/environment.txt"
if [[ $MODE == smoke ]];then graphs=(email-Enron)
elif [[ $MODE == diagnostic ]];then graphs=(ogbn-products reddit mycielskian19 roadNet-CA wiki-Talk)
elif [[ $MODE == remaining ]];then graphs=(hollywood-2009 indochina-2004 soc-Pokec cit-Patents soc-LiveJournal1 com-LiveJournal as-Skitter rgg_n_2_24_s0 web-Google email-Enron amazon0601 com-Youtube)
elif [[ $MODE == pmu ]];then graphs=(email-Enron web-Google)
else echo "Unknown mode $MODE" >&2;exit 2;fi
for graph in "${graphs[@]}";do
  sha256sum "$BASE/$graph/$graph.csrbin" >>"$RUN/graph.sha256"
  if [[ $MODE == pmu ]];then
    MKL_VERBOSE=1 perf stat --no-big-num -e amx_ops_retired.bf16 -o "$RUN/logs/$graph.perf.txt" -- \
      "$STUDY/build/bench" "$BASE" "$graph" bf16_only >"$RUN/logs/$graph.out" 2>"$RUN/logs/$graph.err"
  else
    "$STUDY/build/bench" "$BASE" "$graph" compare >"$RUN/logs/$graph.out" 2>"$RUN/logs/$graph.err"
    grep -E 'CHECK|DONE' "$RUN/logs/$graph.out" >>"$RUN/progress.log"
  fi
done
if [[ $MODE != pmu ]];then python3 "$STUDY/source_snapshot/summarize.py" "$RUN";fi
