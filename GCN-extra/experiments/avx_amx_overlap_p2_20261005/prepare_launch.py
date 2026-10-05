#!/usr/bin/env python3
"""Derive the isolated microbenchmark launcher from the numerical-gate runner."""
import pathlib
p=pathlib.Path(__file__).resolve().parent;root=p.parents[1]
s=(root/'experiments/delayed_residual_gate_20261005/run_gate.sh').read_text(encoding='utf-8')
for a,b in [
 ('delayed_residual_gate_20261005','avx_amx_overlap_p2_20261005'),('residual_gate.cpp','overlap_micro.cpp'),
 ('delayed-residual-gate','avx-amx-overlap'),('residual_gate','overlap_micro'),('delayed_residual','avx_amx_overlap'),
 ('residual-gate','overlap-p2'),('OMP_NUM_THREADS=4','OMP_NUM_THREADS=1'),('--cpus-per-task=4','--cpus-per-task=1'),
 ('run_gate.sh','run_overlap.sh'),('delayed_overlap_micro_build','avx_amx_overlap_build'),('delayed_overlap_micro_submission','avx_amx_overlap_submission'),('avx_amx_overlap_amx_counterexample','avx_amx_overlap_microbenchmark'),
 ('--cores-per-socket=4','--cores-per-socket=1'),('results/gate.log','results/overlap.log'),('results/gate.err','results/overlap.err'),
 ('RESIDUAL_RUN','OVERLAP_RUN'),('P3 exact cancellation AMX numerical counterexample','P2 fixed-work same-core serial/interleaved AVX-AMX'),
 ("input='exact BF16 four-neighbor scaled cancellation',accuracy_gate=0.001","input='five fixed CSR/pool cases, BF16 H/W',output_gate='memcmp full FP32 and finite'"),
 ('Original TFS/ACCURATE source unchanged; exact hardware cancellation fixture','Original source unchanged; fixed-work two-buffer handoff microbenchmark'),
 ('Hardware FP32 and final BF16 gate','FP32 output bitwise and sampled profile gate, paired timings'),
 ('D1 expected to fail frozen FP32 accurate numerical gate; BF16 final may hide it. D2 only checked on this counterexample, not general validation.','P2 microbenchmark only, not two-layer E2E; serial/interleaved fixed work, one physical core'),
 ('Do not time D1 as accepted ACCURATE; compare any D2 design to decoupled FULL ACCURATE','Assess whether fixed-work interleaving warrants a full GCN kernel'),
 ('Small hardware numerical counterexample under frozen accurate gate','One-core fixed-work AMX handoff microbenchmark'),
 ('Inspect both FP32 and BF16 boundaries','Compare paired serial/interleaved timing and PMU')]:s=s.replace(a,b)
anchor='FLAGS=(-O3'
pmu='''python3 - "$ROOT/src/projection_window_pmu.hpp" "$RUN/source_snapshot/micro_pmu.hpp" <<'PY'
import pathlib,sys
s=pathlib.Path(sys.argv[1]).read_text()
for a,b in [('STANDARD_EVENT_COUNT = 4','STANDARD_EVENT_COUNT = 5'),('EVENT_COUNT = 5','EVENT_COUNT = 6'),('CACHE_REFERENCES, AMX_BUSY','CACHE_REFERENCES, REF_CYCLES, AMX_BUSY'),('"cache_references", "amx_busy"','"cache_references", "ref_cycles", "amx_busy"'),('PERF_COUNT_HW_CACHE_REFERENCES, 0x02b7','PERF_COUNT_HW_CACHE_REFERENCES, PERF_COUNT_HW_REF_CPU_CYCLES, 0x02b7')]:
 assert a in s,a
 s=s.replace(a,b)
pathlib.Path(sys.argv[2]).write_text(s)
PY
'''
# Counts are replaced in order with an anchored declaration to avoid rewriting
# STANDARD_EVENT_COUNT after its own change.
pmu=pmu.replace("('EVENT_COUNT = 5','EVENT_COUNT = 6')","('constexpr size_t EVENT_COUNT = 5;','constexpr size_t EVENT_COUNT = 6;')")
assert s.count(anchor)==1;s=s.replace(anchor,pmu+anchor)
s=s.replace('sha256sum "$RUN/build/overlap_micro"','objdump -d -C "$RUN/build/overlap_micro" > "$RUN/build/disassembly.txt"\nsha256sum "$RUN/build/overlap_micro"')
anchor='env | sort > "$RUN/logs/environment.txt"'
audit='''python3 - "$RUN/logs" <<'PY'
import os,pathlib,json,sys
r=pathlib.Path(sys.argv[1]);cpus=sorted(os.sched_getaffinity(0));top=[]
for c in cpus:
 p=pathlib.Path('/sys/devices/system/cpu')/('cpu'+str(c))/'topology'
 top.append(dict(cpu=c,socket=int((p/'physical_package_id').read_text()),core=int((p/'core_id').read_text())))
assert len(cpus)==1 and len({(x['socket'],x['core']) for x in top})==1,'need one physical core'
(r/'affinity.json').write_text(json.dumps(dict(cpus=cpus,topology=top),indent=2))
PY
'''
assert s.count(anchor)==1;s=s.replace(anchor,anchor+'\n'+audit)
(p/'run_overlap.sh').write_bytes(s.replace('\r\n','\n').encode())
