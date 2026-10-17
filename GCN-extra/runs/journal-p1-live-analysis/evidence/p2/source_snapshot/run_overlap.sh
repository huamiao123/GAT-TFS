#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
EXP="$ROOT/experiments/avx_amx_overlap_p2_20261005"
RUN="$ROOT/runs/avx-amx-overlap-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN/source_snapshot" "$RUN/build" "$RUN/logs" "$RUN/results"
build_finish(){
 s=$?
 python3 - "$RUN" "$s" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2]);e=dict(id=r.name+'-build',kind='avx_amx_overlap_build',status='PASS' if s==0 else 'FAILED',exit_status=s,evidence=str(r),paper_eligible=False,issues='Original source unchanged; fixed-work two-buffer handoff microbenchmark',next='FP32 output bitwise and sampled profile gate, paired timings')
(r/'build/event.json').write_text(json.dumps(e,indent=2)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/build/event.json"
}
trap build_finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$EXP/overlap_micro.cpp" "$EXP/run_overlap.sh" "$EXP/PROTOCOL.md" "$RUN/source_snapshot/"
cp "$ROOT/original/gcn_e2e_v3.cpp" "$ROOT/src/paper_methods_kernels.hpp" "$ROOT/src/paper_methods_runtime.hpp" "$RUN/source_snapshot/"
cmp "$RUN/source_snapshot/gcn_e2e_v3.cpp" /home/huangjianqiang_group/hdacp1/data/yx/TFS/code/gcn_e2e_v3.cpp
python3 - "$ROOT/src/projection_window_pmu.hpp" "$RUN/source_snapshot/micro_pmu.hpp" <<'PY'
import pathlib,sys
s=pathlib.Path(sys.argv[1]).read_text()
for a,b in [('STANDARD_EVENT_COUNT = 4','STANDARD_EVENT_COUNT = 5'),('constexpr size_t EVENT_COUNT = 5;','constexpr size_t EVENT_COUNT = 6;'),('CACHE_REFERENCES, AMX_BUSY','CACHE_REFERENCES, REF_CYCLES, AMX_BUSY'),('"cache_references", "amx_busy"','"cache_references", "ref_cycles", "amx_busy"'),('PERF_COUNT_HW_CACHE_REFERENCES, 0x02b7','PERF_COUNT_HW_CACHE_REFERENCES, PERF_COUNT_HW_REF_CPU_CYCLES, 0x02b7')]:
 assert a in s,a
 s=s.replace(a,b)
pathlib.Path(sys.argv[2]).write_text(s)
PY
FLAGS=(-O3 -march=sapphirerapids -mamx-bf16 -mamx-tile -mavx512bf16 -qopenmp -qmkl=parallel)
printf '%q ' icpx "${FLAGS[@]}" "$RUN/source_snapshot/overlap_micro.cpp" -o "$RUN/build/overlap_micro" > "$RUN/build/commands.txt"
icpx "${FLAGS[@]}" "$RUN/source_snapshot/overlap_micro.cpp" -o "$RUN/build/overlap_micro" > "$RUN/build/compile.log" 2>&1
objdump -d -C "$RUN/build/overlap_micro" > "$RUN/build/disassembly.txt"
sha256sum "$RUN/build/overlap_micro" > "$RUN/build/binary.sha256"
find "$RUN/source_snapshot" -type f -print0 | sort -z | xargs -0 sha256sum > "$RUN/source.sha256"
python3 - "$RUN" <<'PY'
import pathlib,json,sys
r=pathlib.Path(sys.argv[1]);m=dict(parent_commit='33fb6b4270b94c1f38e8e9f3d94aed35595a384d',purpose='P2 fixed-work same-core serial/interleaved AVX-AMX',compiler=(r/'compiler.txt').read_text(),flags=(r/'build/commands.txt').read_text(),source_hashes=(r/'source.sha256').read_text(),binary_hashes=(r/'build/binary.sha256').read_text(),NUMA='default no interleave',input='five fixed CSR/pool cases, BF16 H/W',output_gate='memcmp full FP32 and finite')
(r/'manifest.json').write_text(json.dumps(m,indent=2)+'\n')
PY
cat > "$RUN/job.sh" <<'SH'
#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
GATE_RUN=$1
RUN=$GATE_RUN
finish(){
 s=$?
 python3 - "$RUN" "$s" <<'PY'
import pathlib,json,sys,os
r=pathlib.Path(sys.argv[1]);s=int(sys.argv[2]);e=dict(id=r.name,kind='avx_amx_overlap_microbenchmark',status='PASS' if s==0 else 'FAILED',job_id=os.environ.get('SLURM_JOB_ID'),node=os.uname().nodename,exit_status=s,evidence=str(r),paper_eligible=False,issues='P2 microbenchmark only, not two-layer E2E; serial/interleaved fixed work, one physical core',next='Assess whether fixed-work interleaving warrants a full GCN kernel')
(r/'event.json').write_text(json.dumps(e,indent=2)+'\n');(r/'exit_status.txt').write_text(str(s)+'\n')
PY
 python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN/logs"
RUN=$GATE_RUN
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS KMP_AFFINITY
export OMP_NUM_THREADS=1 OMP_PROC_BIND=close OMP_PLACES=cores
hostname > "$RUN/logs/node.txt"
lscpu > "$RUN/logs/cpu.txt"
env | sort > "$RUN/logs/environment.txt"
python3 - "$RUN/logs" <<'PY'
import os,pathlib,json,sys
r=pathlib.Path(sys.argv[1]);cpus=sorted(os.sched_getaffinity(0));top=[]
for c in cpus:
 p=pathlib.Path('/sys/devices/system/cpu')/('cpu'+str(c))/'topology'
 top.append(dict(cpu=c,socket=int((p/'physical_package_id').read_text()),core=int((p/'core_id').read_text())))
assert len(cpus)==1 and len({(x['socket'],x['core']) for x in top})==1,'need one physical core'
(r/'affinity.json').write_text(json.dumps(dict(cpus=cpus,topology=top),indent=2))
PY

"$RUN/build/overlap_micro" > "$RUN/results/overlap.log" 2> "$RUN/results/overlap.err"
SH
JOB=$(sbatch --parsable --partition=intel --nodes=1 --ntasks=1 --cpus-per-task=1 --sockets-per-node=1 --cores-per-socket=1 --threads-per-core=1 --distribution=block:block --time=00:10:00 --job-name=overlap-p2 --chdir="$ROOT" --output="$RUN/logs/slurm-%j.out" --error="$RUN/logs/slurm-%j.err" --wrap="srun --ntasks=1 --cpus-per-task=1 --cpu-bind=cores --hint=nomultithread bash $RUN/job.sh $RUN")
python3 - "$RUN" "$JOB" <<'PY'
import json,pathlib,sys
r=pathlib.Path(sys.argv[1]);e=dict(id=r.name+'-submit',kind='avx_amx_overlap_submission',status='SUBMITTED',evidence=str(r),job_id=sys.argv[2],paper_eligible=False,issues='One-core fixed-work AMX handoff microbenchmark',next='Compare paired serial/interleaved timing and PMU')
(r/'submission.json').write_text(json.dumps(e,indent=2)+'\n')
PY
python3 "$ROOT/scripts/record_event.py" "$RUN/submission.json"
printf 'OVERLAP_RUN=%s JOB=%s\n' "$RUN" "$JOB"
