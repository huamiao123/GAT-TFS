#!/usr/bin/env bash
set -euo pipefail
exec srun --ntasks=1 --cpus-per-task=32 --cpu-bind=cores --distribution=block:block --hint=nomultithread bash /home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01/source_snapshot/trace_job.sh /home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01 mycielskian19
