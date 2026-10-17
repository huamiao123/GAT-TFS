#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
EXP="$ROOT/experiments/delayed_residual_gate_20261005"
RUN="$ROOT/runs/delayed-residual-gate-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN/source_snapshot" "$RUN/build" "$RUN/logs" "$RUN/results"
build_finish(){
 s=$?
 python3 - "$RUN" "$s" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2]);e=dict(id=r.name+'-build',kind='delayed_residual_gate_build',status='PASS' if s==0 else 'FAILED',exit_status=s,evidence=str(r),paper_eligible=False,issues='Original TFS/ACCURATE source unchanged; exact hardware cancellation fixture',next='Hardware FP32 and final BF16 gate')
(r/'build/event.json').write_text(json.dumps(e,indent=2)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/build/event.json"
}
trap build_finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$EXP/residual_gate.cpp" "$EXP/run_gate.sh" "$EXP/PROTOCOL.md" "$RUN/source_snapshot/"
cp "$ROOT/original/gcn_e2e_v3.cpp" "$ROOT/src/paper_methods_kernels.hpp" "$ROOT/src/paper_methods_runtime.hpp" "$RUN/source_snapshot/"
cmp "$RUN/source_snapshot/gcn_e2e_v3.cpp" /home/huangjianqiang_group/hdacp1/data/yx/TFS/code/gcn_e2e_v3.cpp
FLAGS=(-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel)
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/residual_gate.cpp" -o "$RUN/build/residual_gate" > "$RUN/build/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/residual_gate.cpp" -o "$RUN/build/residual_gate" > "$RUN/build/compile.log" 2>&1
sha256sum "$RUN/build/residual_gate" > "$RUN/build/binary.sha256"
find "$RUN/source_snapshot" -type f -print0 | sort -z | xargs -0 sha256sum > "$RUN/source.sha256"
python3 - "$RUN" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);m=dict(parent_commit='33fb6b4270b94c1f38e8e9f3d94aed35595a384d',purpose='P3 exact cancellation AMX numerical counterexample',compiler=(r/'compiler.txt').read_text(),flags=(r/'build/commands.txt').read_text(),source_hashes=(r/'source.sha256').read_text(),binary_hashes=(r/'build/binary.sha256').read_text(),NUMA='default no interleave',input='exact BF16 four-neighbor scaled cancellation',accuracy_gate=0.001)
(r/'manifest.json').write_text(json.dumps(m,indent=2)+'\n')
PY
cat > "$RUN/job.sh" <<'SH'
#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
RUN=$1
finish(){
 s=$?
 python3 - "$RUN" "$s" <<'PY'
import pathlib,json,sys,os
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2]);e=dict(id=r.name,kind='delayed_residual_amx_counterexample',status='PASS' if s==0 else 'FAILED',job_id=os.environ.get('SLURM_JOB_ID'),node=os.uname().nodename,exit_status=s,evidence=str(r),paper_eligible=False,issues='D1 expected to fail frozen FP32 accurate numerical gate; BF16 final may hide it. D2 only checked on this counterexample, not general validation.',next='Do not time D1 as accepted ACCURATE; compare any D2 design to decoupled FULL ACCURATE')
(r/'event.json').write_text(json.dumps(e,indent=2)+'\n');(r/'exit_status.txt').write_text(str(s)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN/logs"
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS KMP_AFFINITY
export OMP_NUM_THREADS=4 OMP_PROC_BIND=close OMP_PLACES=cores
hostname > "$RUN/logs/node.txt"
lscpu > "$RUN/logs/cpu.txt"
env | sort > "$RUN/logs/environment.txt"
"$RUN/build/residual_gate" > "$RUN/results/gate.log" 2> "$RUN/results/gate.err"
SH
JOB=$(sbatch --parsable --partition=intel --nodes=1 --ntasks=1 --cpus-per-task=4 --sockets-per-node=1 --cores-per-socket=4 --threads-per-core=1 --distribution=block:block --time=00:10:00 --job-name=residual-gate --chdir="$ROOT" --output="$RUN/logs/slurm-%j.out" --error="$RUN/logs/slurm-%j.err" --wrap="srun --ntasks=1 --cpus-per-task=4 --cpu-bind=cores --hint=nomultithread bash $RUN/job.sh $RUN")
python3 - "$RUN" "$JOB" <<'PY'
import json,pathlib,sys
r=pathlib.Path(sys.argv[1]);e=dict(id=r.name+'-submit',kind='delayed_residual_gate_submission',status='SUBMITTED',evidence=str(r),job_id=sys.argv[2],paper_eligible=False,issues='Small hardware numerical counterexample under frozen accurate gate',next='Inspect both FP32 and BF16 boundaries')
(r/'submission.json').write_text(json.dumps(e,indent=2)+'\n')
PY
python3 "$ROOT/scripts/record_event.py" "$RUN/submission.json"
printf 'RESIDUAL_RUN=%s JOB=%s\n' "$RUN" "$JOB"
