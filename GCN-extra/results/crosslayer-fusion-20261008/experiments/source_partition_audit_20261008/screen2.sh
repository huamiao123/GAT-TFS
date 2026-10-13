#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$ROOT/experiments/source_partition_audit_20261008
BUILD=$(cat "$STUDY/LATEST_BUILD")
RUN="$ROOT/runs/source-partition-cross-${SLURM_JOB_ID}"
mkdir -p "$RUN"
finish(){
  s=$?
  python3 - "$RUN" "$s" <<'PY'
import json,pathlib,sys,os
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2])
lines=[x for x in (r/'program.log').read_text(errors='replace').splitlines() if x.startswith('PARTITION ')] if (r/'program.log').exists() else []
e=dict(id=r.name,kind='source_partition_screen_graphs',status='PASS' if s==0 and len(lines)==18 else 'FAILED',exit_status=s,job_id=os.environ.get('SLURM_JOB_ID'),node=os.uname().nodename,evidence=str(r),paper_eligible=False,partition_lines=lines,issues='Structural source-overlap audit; no runtime claim',next='Use low-duplication graphs for a controlled parallel fusion run')
(r/'event.json').write_text(json.dumps(e,indent=2)+'\n')
(r/'exit_status.txt').write_text(str(s)+'\n')
PY
  python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$STUDY/PROTOCOL.md" "$RUN/road_protocol.read.md"
cp "$ROOT/experiments/source_partition_audit_20261008/source_partition_cross_protocol.md" "$RUN/protocol.read.md"
cp "$BUILD/source.sha256" "$RUN/source.sha256"
cp "$BUILD/binary.sha256" "$RUN/binary.sha256"
unset OMP_NUM_THREADS MKL_NUM_THREADS MKL_DYNAMIC
export OMP_NUM_THREADS=1
hostname > "$RUN/node.txt"
lscpu > "$RUN/cpu.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1
printf '%s\n' hollywood-2009 indochina-2004 rgg_n_2_24_s0 > "$RUN/graphs.txt"
while read -r graph; do
  printf '%q ' srun --ntasks=1 --cpus-per-task=1 --cpu-bind=cores "$BUILD/source_partition_analyzer" "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/$graph/$graph.csrbin" 32 > "$RUN/command-$graph.txt"
  srun --ntasks=1 --cpus-per-task=1 --cpu-bind=cores "$BUILD/source_partition_analyzer" "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/$graph/$graph.csrbin" 32 < /dev/null >> "$RUN/program.log" 2>&1
done < "$RUN/graphs.txt"
