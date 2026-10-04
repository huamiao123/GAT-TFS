#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
RUN="$ROOT/runs/paper-cache-scheduling-adjustment-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN"
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$ROOT/scripts/adjust_paper_cache_limits.sh" "$RUN/"
python3 - "$RUN" <<'PY'
import pathlib,subprocess,json,sys,re
r=pathlib.Path(sys.argv[1]);jobs=['10864906','10864907','10864908']
before={}
for j in jobs:
 s=subprocess.check_output(['scontrol','show','job','-o',j],text=True)
 assert 'UserId=hdacp1(' in s and re.search(r'JobName=cache-(high|medium|low)\b',s)
 assert 'JobState=PENDING' in s
 before[j]=s
(r/'before.json').write_text(json.dumps(before,indent=2))
for j in jobs:subprocess.run(['scontrol','update','JobId='+j,'TimeLimit=00:30:00'],check=True)
after={j:subprocess.check_output(['scontrol','show','job','-o',j],text=True) for j in jobs}
for s in after.values():assert 'TimeLimit=00:30:00' in s
(r/'after.json').write_text(json.dumps(after,indent=2))
e=dict(id=r.name,kind='cache_control_scheduler_limit_only',status='PASS',job_ids=jobs,evidence=str(r),paper_eligible=False,issues='Original two-hour timeout overstates scheduling duration. Complete larger main sweep took at most 16m43s. Supplemental scope is smaller. Only scheduler timeout changes to 30 minutes; frozen kernels/gates/data/threads/affinity/memory/partition/exclusivity/timing boundaries unchanged.',next='Retain timeout outcomes if any; reconcile all cache-control outputs')
(r/'event.json').write_text(json.dumps(e,indent=2))
PY
python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
squeue --start -u hdacp1
