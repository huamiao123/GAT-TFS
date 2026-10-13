#!/usr/bin/env bash
set -euo pipefail
ROOT=/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra
STUDY=$ROOT/experiments/recompute_window_kernel_20261008
BUILD=$(cat "$STUDY/LATEST_BUILD")
RUN="$ROOT/runs/recompute-window-sweep-${SLURM_JOB_ID}"
mkdir -p "$RUN"
finish(){
  s=$?
  python3 - "$RUN" "$s" <<'PY'
import json, pathlib, re, sys, os
r=pathlib.Path(sys.argv[1]); s=int(sys.argv[2])
rows=[]; gates=[]
for p in sorted(r.glob('window-*.log')):
    txt=p.read_text(errors='replace')
    base=re.search(r'BASELINE_FULL ms=([0-9.e+-]+) checksum=(\d+)',txt)
    w0=re.search(r'WINDOW_RESULT window=0 ms=([0-9.e+-]+).*?exact=(\d+) checksum=(\d+)',txt)
    wc=re.search(r'WINDOW_RESULT window=(\d+) ms=([0-9.e+-]+).*?extra_source_rows=(-?\d+).*?peak_cache_rows=(\d+).*?exact=(\d+) checksum=(\d+)',txt)
    gate=re.findall(r'WINDOW_GATE .*',txt)
    if gate: gates.extend(gate)
    rows.append(dict(file=p.name,baseline_ms=float(base.group(1)) if base else None,w0_ms=float(w0.group(1)) if w0 else None,w0_exact=int(w0.group(2)) if w0 else None,window=int(wc.group(1)) if wc else None,window_ms=float(wc.group(2)) if wc else None,extra_source_rows=int(wc.group(3)) if wc else None,peak_cache_rows=int(wc.group(4)) if wc else None,window_exact=int(wc.group(5)) if wc else None))
expected={int(x) for x in (r/'windows.txt').read_text().split()} if (r/'windows.txt').exists() else set()
got={x['window'] for x in rows if x['window'] is not None}
summary=dict(schema='recompute-window-sweep-v1',status='PASS' if s==0 and got==expected and len(gates)==len(rows) and all('pass=1' in x for x in gates) else 'FAILED',job_id=os.environ.get('SLURM_JOB_ID'),expected_windows=sorted(expected),rows=rows,gates=gates)
(r/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
e=dict(id=r.name,kind='recompute_window_sweep',status=summary['status'],exit_status=s,job_id=os.environ.get('SLURM_JOB_ID'),node=os.uname().nodename,evidence=str(r),paper_eligible=False,summary=summary,next='Use legal work/state frontier to choose a parallel design or close the window path')
(r/'event.json').write_text(json.dumps(e,indent=2)+'\n')
(r/'exit_status.txt').write_text(str(s)+'\n')
PY
  python3 "$ROOT/scripts/record_event.py" "$RUN/event.json"
}
trap finish EXIT
source "$ROOT/scripts/preflight.sh" "$RUN"
cp "$STUDY/PROTOCOL.md" "$RUN/protocol.read.md"
cp "$ROOT/experiments/recompute_window_kernel_20261008/recompute_window_sweep_protocol.md" "$RUN/sweep_protocol.read.md" 2>/dev/null || true
cp "$BUILD/source.sha256" "$RUN/source.sha256"
cp "$BUILD/binary.sha256" "$RUN/binary.sha256"
unset MKL_NUM_THREADS MKL_DYNAMIC MKL_DOMAIN_NUM_THREADS OMP_DYNAMIC KMP_AFFINITY GOMP_CPU_AFFINITY
export OMP_NUM_THREADS=1 OMP_PROC_BIND=close OMP_PLACES=cores
hostname > "$RUN/node.txt"
lscpu > "$RUN/cpu.txt"
numactl --show > "$RUN/numa_policy.txt" 2>&1
numactl --hardware > "$RUN/numa_hardware.txt"
env | sort > "$RUN/environment.txt"
printf '%s\n' 1 4 16 64 256 512 1024 2048 4096 > "$RUN/windows.txt"
while read -r w; do
  printf '%q ' srun --ntasks=1 --cpus-per-task=1 --cpu-bind=cores "$BUILD/recompute_window_kernel" /home/huangjianqiang_group/hdacp1/data/yx/TFS/data roadNet-CA "$w" > "$RUN/command-$w.txt"
  srun --ntasks=1 --cpus-per-task=1 --cpu-bind=cores "$BUILD/recompute_window_kernel" /home/huangjianqiang_group/hdacp1/data/yx/TFS/data roadNet-CA "$w" < /dev/null > "$RUN/window-$w.log" 2>&1
done < "$RUN/windows.txt"
