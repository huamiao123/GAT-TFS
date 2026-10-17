#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
RUN=$1
mkdir -p "$RUN/logs"
source "$ROOT/scripts/preflight.sh" "$RUN"
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS OMP_DYNAMIC KMP_AFFINITY GOMP_CPU_AFFINITY
export OMP_NUM_THREADS=${SLURM_CPUS_PER_TASK}
export OMP_PROC_BIND=close OMP_PLACES=cores PYTHONDONTWRITEBYTECODE=1
hostname > "$RUN/node.txt"
lscpu > "$RUN/lscpu.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1
env | sort > "$RUN/environment.txt"
python3 - "$RUN" <<'PY'
import os,pathlib,json,sys
r=pathlib.Path(sys.argv[1]);cpus=sorted(os.sched_getaffinity(0));top=[]
for c in cpus:
 p=pathlib.Path('/sys/devices/system/cpu')/f'cpu{c}'/'topology'
 top.append((int((p/'physical_package_id').read_text()),int((p/'core_id').read_text())))
assert len(cpus)==32 and len(set(top))==32 and len({x[0] for x in top})==1,(cpus,top)
(r/'affinity.json').write_text(json.dumps(dict(cpus=cpus,topology=top),indent=2))
PY
for graph in roadNet-CA reddit ogbn-products mycielskian19; do
  input="/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/$graph/$graph.csrbin"
  sha256sum "$input" >> "$RUN/graph.sha256"
  echo "START $graph $(date -Is)" | tee -a "$RUN/progress.log"
  "$RUN/build/ablation" "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data" "$graph" > "$RUN/logs/$graph.out" 2> "$RUN/logs/$graph.err"
  rg 'ABLATION_CHECK|ABLATION_COMPLETE' "$RUN/logs/$graph.out" >> "$RUN/progress.log"
  echo "DONE $graph $(date -Is)" | tee -a "$RUN/progress.log"
done
python3 "$RUN/source_snapshot/summarize.py" "$RUN"
python3 - "$RUN" <<'PY'
import json,pathlib,sys,os
r=pathlib.Path(sys.argv[1]);e=dict(id=r.name,kind='full_degree_sort_ablation',status='PASS',job_id=os.environ.get('SLURM_JOB_ID'),node=os.uname().nodename,evidence=str(r),paper_eligible=False,issues='shared node diagnostic; only FULL destination schedule changed',next='exclusive rerun and PMU if result warrants')
(r/'event.json').write_text(json.dumps(e,indent=2)+'\n')
PY
python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
