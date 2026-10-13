#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
WZH=${ROOT%/*}
RUN=$1
cat "$WZH/AGENTS.md" > "$RUN/AGENTS.read.txt"
cat "$WZH/skills/tfs-research-engineering/SKILL.md" > "$RUN/SKILL.read.txt"
sha256sum "$WZH/AGENTS.md" "$WZH/skills/tfs-research-engineering/SKILL.md" "$ROOT/IMPLEMENTATION_CONTRACT.md" > "$RUN/preflight.sha256"
test "$(sha256sum "$WZH/AGENTS.md" | cut -d' ' -f1)" = 55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7
test "$(sha256sum "$WZH/skills/tfs-research-engineering/SKILL.md" | cut -d' ' -f1)" = 641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca
cat "$ROOT/IMPLEMENTATION_CONTRACT.md" > "$RUN/contract.read.txt"
cat "$WZH/TFS-Train/docs/spec/MATH_SPEC_V2.md" "$WZH/TFS-Train/docs/spec/KERNEL_LAYOUT_V2.md" "$WZH/TFS-Train/docs/spec/PYTORCH_EXTENSION_SPEC.md" "$WZH/TFS-Train/docs/spec/EXPERIMENT_SPEC_V2.md" > "$RUN/train_reference_specs.read.txt"
squeue -u hdacp1 > "$RUN/queue.txt"
sinfo -p intel,intel_expr > "$RUN/partitions.txt"
python3 "$ROOT/scripts/record_event.py" check
module load intel/intel-oneapi-compilers/2024.1.0/gcc8.5.0-5ndeojp
module load intel/intel-oneapi-mkl/2023.2.0/oneapi2023.2.0-vuihbr3
export TMPDIR="$ROOT/build/tmp"
icpx --version > "$RUN/compiler.txt"
