#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
RUN=$1
STUDY=$(cat "$ROOT/build/FULL_DEGREE_SORT_LATEST")
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
expected=int(os.environ['SLURM_CPUS_PER_TASK'])
assert len(cpus)==expected and len(set(top))==expected and len({x[0] for x in top})==1,(cpus,top)
(r/'affinity.json').write_text(json.dumps(dict(cpus=cpus,topology=top),indent=2))
PY
if [[ ${2:-real} == smoke ]]; then
  base="$ROOT/runs/h-panel-reuse-10964897/fixtures"
  graphs=(tail_37 tail_137)
elif [[ ${2:-real} == reverse ]]; then
  base=/home/huangjianqiang_group/hdacp1/data/yx/TFS/data
  graphs=(mycielskian19 ogbn-products reddit roadNet-CA)
elif [[ ${2:-real} == validation_small ]]; then
  base=/home/huangjianqiang_group/hdacp1/data/yx/TFS/data
  graphs=(hollywood-2009 indochina-2004 soc-Pokec cit-Patents soc-LiveJournal1 com-LiveJournal as-Skitter web-Google wiki-Talk email-Enron amazon0601 com-Youtube)
elif [[ ${2:-real} == validation_large ]]; then
  base=/home/huangjianqiang_group/hdacp1/data/yx/TFS/data
  graphs=(rgg_n_2_24_s0)
elif [[ ${2:-real} == web_repeat ]]; then
  base=/home/huangjianqiang_group/hdacp1/data/yx/TFS/data
  graphs=(web-Google)
else
  base=/home/huangjianqiang_group/hdacp1/data/yx/TFS/data
  graphs=(roadNet-CA reddit ogbn-products mycielskian19)
fi
for graph in "${graphs[@]}"; do
  input="$base/$graph/$graph.csrbin"
  sha256sum "$input" >> "$RUN/graph.sha256"
  echo "START $graph $(date -Is)" | tee -a "$RUN/progress.log"
  "$STUDY/build/ablation" "$base" "$graph" > "$RUN/logs/$graph.out" 2> "$RUN/logs/$graph.err"
  grep -E 'ABLATION_CHECK|ABLATION_COMPLETE' "$RUN/logs/$graph.out" >> "$RUN/progress.log"
  echo "DONE $graph $(date -Is)" | tee -a "$RUN/progress.log"
done
python3 "$STUDY/source_snapshot/summarize.py" "$RUN"
python3 - "$RUN" <<'PY'
import json,pathlib,sys,os
r=pathlib.Path(sys.argv[1]);e=dict(id=r.name,kind='full_degree_sort_ablation',status='PASS',job_id=os.environ.get('SLURM_JOB_ID'),node=os.uname().nodename,evidence=str(r),paper_eligible=False,issues='shared node diagnostic; only FULL destination schedule changed',next='exclusive rerun and PMU if result warrants')
(r/'event.json').write_text(json.dumps(e,indent=2)+'\n')
PY
python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
