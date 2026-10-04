#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
MODE=$1
case "$MODE" in smoke|formal) ;; *) exit 3;; esac
RUN="$ROOT/runs/source-submit-$MODE-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN"
source "$ROOT/scripts/preflight.sh" "$RUN"
cat "$ROOT/docs/SOURCE_MKL_PROTOCOL_20261004.md" > "$RUN/source_protocol.read.md"
test -f "$ROOT/build/SOURCE_PROTOCOL_LATEST"
mkdir -p "$RUN/launcher"
cp "$ROOT/scripts/source_$MODE.slurm" "$ROOT/scripts/source_job.sh" "$ROOT/scripts/source_protocol_suite.py" "$RUN/launcher/"
cp "$ROOT/build/SOURCE_PROTOCOL_LATEST" "$RUN/launcher/build_path.txt"
python3 - "$RUN" "$MODE" "$ROOT" <<'PY'
import json,pathlib,os,sys
r=pathlib.Path(sys.argv[1]);mode=sys.argv[2];root=sys.argv[3];p=r/'launcher'/('source_'+mode+'.slurm')
text=p.read_text();old='bash '+root+'/scripts/source_job.sh '+mode
assert text.count(old)==1
p.write_text(text.replace(old,'bash '+str(r/'launcher/source_job.sh')+' '+mode))
(r/'submission_config.json').write_text(json.dumps({'mode':mode,'graphs':os.getenv('GCN_SOURCE_GRAPHS'),'build_path':(r/'launcher/build_path.txt').read_text().strip(),'launcher_is_immutable':True},indent=2))
PY
sha256sum "$RUN/launcher/"* > "$RUN/source.sha256"
JOB=$(sbatch --parsable --chdir="$ROOT" "$RUN/launcher/source_$MODE.slurm")
printf '%s\n' "$JOB" > "$RUN/job_id.txt"
python3 - "$RUN" "$JOB" <<'PY'
import json,pathlib,sys
r=pathlib.Path(sys.argv[1]);e={'id':r.name,'kind':'source_protocol_submission','status':'SUBMITTED','job_id':sys.argv[2],'evidence':str(r),'paper_eligible':False,'issues':'Original MKL/TFS environment, no numactl interleave for any path','next':'Validate source protocol correctness before performance'}
(r/'event.json').write_text(json.dumps(e,indent=2))
PY
python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
printf 'SOURCE_JOB=%s\n' "$JOB"
