#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
RUN=$1
MODE=$2
BUILD=$3
export OMP_NUM_THREADS="$SLURM_CPUS_PER_TASK" MKL_NUM_THREADS="$SLURM_CPUS_PER_TASK" OMP_PROC_BIND=close OMP_PLACES=cores MKL_DYNAMIC=FALSE
hostname > "$RUN/node.txt"
lscpu > "$RUN/lscpu.txt"
lscpu -e=CPU,CORE,SOCKET,NODE,ONLINE > "$RUN/cpu_layout.txt"
cat /proc/self/status > "$RUN/process_status.txt"
cat /proc/meminfo > "$RUN/meminfo.txt"
cat /proc/sys/kernel/perf_event_paranoid > "$RUN/perf_event_paranoid.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1 || true
numactl --hardware > "$RUN/numa_hardware.txt" 2>&1 || true
printenv OMP_NUM_THREADS MKL_NUM_THREADS OMP_PROC_BIND OMP_PLACES MKL_DYNAMIC > "$RUN/thread_env.txt"
python3 - "$RUN" "$MODE" <<'PY'
import json, os, pathlib, sys
allowed=sorted(os.sched_getaffinity(0));rows=[]
for cpu in allowed:
 p=pathlib.Path('/sys/devices/system/cpu')/f'cpu{cpu}'
 rows.append({'cpu':cpu,'core':int((p/'topology/core_id').read_text()),'socket':int((p/'topology/physical_package_id').read_text()),'nodes':[x.name for x in p.glob('node*')]})
data={'allowed_cpus':allowed,'cpu_topology':rows,'physical_cores':len({(x['socket'],x['core']) for x in rows}),'sockets':sorted({x['socket'] for x in rows})}
(pathlib.Path(sys.argv[1])/'frequency_observed.json').write_text(json.dumps({str(cpu):{name:(pathlib.Path('/sys/devices/system/cpu')/f'cpu{cpu}'/'cpufreq'/name).read_text().strip() for name in ['scaling_governor','scaling_cur_freq','scaling_max_freq'] if (pathlib.Path('/sys/devices/system/cpu')/f'cpu{cpu}'/'cpufreq'/name).exists()} for cpu in allowed},indent=2))
(pathlib.Path(sys.argv[1])/'affinity.json').write_text(json.dumps(data,indent=2))
if sys.argv[2]=='formal' and (len(data['sockets'])!=1 or data['physical_cores']<32):
 raise SystemExit('Formal gate requires 32 physical cores on one socket; inspect affinity.json')
PY
BENCH="$BUILD/gcn_bench"
if [ "$MODE" = smoke ]; then
    mkdir -p "$RUN/smoke"
    "$BENCH" --smoke --blocks 1,2,4,8,16,32,64,full --repeats 2 --warmups 1 --out "$RUN/smoke" > "$RUN/smoke/stdout.log" 2> "$RUN/smoke/stderr.log"
else
    DATA=/home/huangjianqiang_group/hdacp1/data/yx/TFS/data
    for GRAPH in reddit soc-Pokec; do
        mkdir -p "$RUN/$GRAPH"
        sha256sum "$DATA/$GRAPH/$GRAPH.csrbin" >> "$RUN/dataset.sha256"
        "$BENCH" --graph "$DATA/$GRAPH/$GRAPH.csrbin" --name "$GRAPH" --repeats 5 --warmups 1 --out "$RUN/$GRAPH" > "$RUN/$GRAPH/stdout.log" 2> "$RUN/$GRAPH/stderr.log"
    done
    for Q in 16 256; do
        SUB="$RUN/regular-q$Q"
        mkdir -p "$SUB"
        "$BENCH" --synthetic 262144 --degree "$Q" --seed 20261003 --repeats 5 --warmups 1 --out "$SUB" > "$SUB/stdout.log" 2> "$SUB/stderr.log"
    done
fi
python3 "$ROOT/scripts/summarize.py" "$RUN"
