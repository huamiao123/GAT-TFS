#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$ROOT/experiments/recompute_window_parallel_20261008
BUILD=$(cat "$STUDY/LATEST_BUILD")
RUN="$ROOT/runs/recompute-window-parallel-cross-${SLURM_JOB_ID}"
mkdir -p "$RUN"
finish(){
  s=$?
  python3 - "$RUN" "$s" <<'PY'
import json,pathlib,re,sys,os
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2])
log=(r/'program.log').read_text(errors='replace') if (r/'program.log').exists() else ''
res=re.findall(r'PAR_RESULT .*',log); gates=re.findall(r'PAR_GATE .*',log)
e=dict(id=r.name,kind='recompute_window_parallel_cross',status='PASS' if s==0 and len(res)==6 and len(gates)==6 and all('pass=1' in x for x in gates) else 'FAILED',exit_status=s,job_id=os.environ.get('SLURM_JOB_ID'),node=os.uname().nodename,evidence=str(r),paper_eligible=False,results=res,gates=gates,issues='Exploratory cross-graph source-duplication boundary; no paper performance claim',next='Use the measured boundary to define an offline policy or close the candidate')
(r/'event.json').write_text(json.dumps(e,indent=2)+'\n')
(r/'exit_status.txt').write_text(str(s)+'\n')
PY
  python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$STUDY/PROTOCOL.md" "$RUN/road_protocol.read.md"
cp "$ROOT/experiments/recompute_window_parallel_20261008/recompute_window_parallel_cross_protocol.md" "$RUN/protocol.read.md"
cp "$BUILD/source.sha256" "$RUN/source.sha256"
cp "$BUILD/binary.sha256" "$RUN/binary.sha256"
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS OMP_DYNAMIC KMP_AFFINITY GOMP_CPU_AFFINITY
export OMP_NUM_THREADS=32 OMP_PROC_BIND=close OMP_PLACES=cores
hostname > "$RUN/node.txt"
lscpu > "$RUN/cpu.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1
printf '%s\n' amazon0601 reddit > "$RUN/graphs.txt"
while read -r graph; do
  printf '%q ' srun --ntasks=1 --cpus-per-task=32 --cpu-bind=cores "$BUILD/recompute_window_parallel" /home/huangjianqiang_group/hdacp1/data/yx/TFS/data "$graph" 32 > "$RUN/command-$graph.txt"
  srun --ntasks=1 --cpus-per-task=32 --cpu-bind=cores "$BUILD/recompute_window_parallel" /home/huangjianqiang_group/hdacp1/data/yx/TFS/data "$graph" 32 < /dev/null >> "$RUN/program.log" 2>&1
done < "$RUN/graphs.txt"
