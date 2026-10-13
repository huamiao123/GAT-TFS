#!/usr/bin/env bash
set -euo pipefail
exec srun --ntasks=1 --cpus-per-task=32 --cpu-bind=cores --distribution=block:block --hint=nomultithread bash /home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-02/source_snapshot/state_job.sh /home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-02 smoke
