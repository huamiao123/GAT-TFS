#!/usr/bin/env bash
set -euo pipefail
exec srun --ntasks=1 --cpus-per-task=32 --cpu-bind=cores --distribution=block:block --hint=nomultithread bash /home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/source_snapshot/plan_job.sh /home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01 web-Google,wiki-Talk,email-Enron,com-Youtube
