#!/usr/bin/env bash
set -euo pipefail

STUDY=$1
MODE=$2
RUN=$3
mkdir -p "$RUN"

# Verify the policy in a child process. Both comparison paths then execute
# inside a single process started under this same policy.
numactl --interleave=all numactl --show >"$RUN/interleave_policy.txt" 2>&1
exec numactl --interleave=all bash "$STUDY/source_snapshot/job.sh" "$STUDY" "$MODE" "$RUN"
