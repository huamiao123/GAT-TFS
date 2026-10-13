from pathlib import Path
import json,subprocess,hashlib

root=Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
exp=root/'experiments/event_trace_20261008'
prev=root/'runs/access-prefetch-20261008-01/source_snapshot'
protocol='''# Compact consumption trace: registered protocol 2026-10-08

Inference-only GCN-extra; user authorizes shared intel and exclusive intel_expr experiments, <=10 requested nodes including pending. Target peak6. No NUMA interleave or changed feature first-touch. Fixed128->128->128, unit edges, original seeded H/W, DegreeSort/TR16/R64, BF16 truncation and original TFS/MKL immutable. Read wzh rules, skill, four training-v2 authority specs but do not change inference scope to training.

Question: can offline replay of the SAME legal dynamic pair consumption events realize shared-decode savings by removing online discovery? Encoding alone is existing inspector-executor practice and not claimed a new scientific principle. Negative results and setup/state cost are essential.

Fixed eight methods/order: pack_epi_degree, pack_epi_qguard, dyn_i, dyn_s, exact_s, trace_i, trace_s_cached, trace_s_rep. The two PACK baselines use existing degree/qguard schedules. Other methods use ONE adjacent degree pair plan, one immutable trace, unchanged PF1, S64/M=FULL and FAST. Trace-I and cached-S retain source-level old head reads; representative-S requests one current source index per event. No volatile/barriers to force artificial baseline work. Natural compiler load/decode reuse must be reported separately from source-level counters.

Trace: per tile offset, old window then pair iteration, each record uint8 count<=128 and ceil(count/4) bytes, 2-bit take1/2/3. Zero used codes invalid, unused bits zero. No source-index copy, graph sorting, deduplication, new pair selection or changed per-row addition order. Independent scalar validator checks exact greedy event identity, head equality on shared events, cursors/exhaustion, all boundaries and totals. Odd tail and exhausted pairs retained, all-empty tiles bypass. FULL is enforced; do not claim tested scope variants.

Correctness before timing: same original numerical thresholds; single-layerFP32/e2eFP32/H1BF16/e2eBF16 canonical bitwise and actual fixed-address timed-object bitwise gates; new shared trace vs frozen dyn_s; instrumentation output equality; independent CSR/trace work formulas, fixed slot/hash/address checks, ReLU semantics. Malformed trace fixtures must reject before timing. Existing13smoke fixtures cover empty, odd, duplicate/unsorted and degree tails; preserve every rejection/failure.

One warmup+five consecutive samples; ten forward/reverse alternating samples, one warmup on each switch. No plan copy needed: all planned paths reference identical immutable storage. Include complete two layers and activation/conversion; graph/setup/trace-build/input-weight preparation outside compute timing and reported separately. Record plan logical/capacity bytes and peak RSS. Three PMU repeats/method using unchanged32-worker event collection. PMU timings are not performance samples. All comparisons same graph/job/build/protocol and precision. Label shared/exclusive separately; retain all samples and variability.

Stage1 smoke on shared intel. Stage2 frozen3synthetic and original benchmark diagnostic4 {mycielskian19,reddit,roadNet-CA,amazon0601}, each allocation class. Myc is constructed; legacy real label is a cohort. Fixed expansion candidate trace_s_rep. Expand full17 only when every correctness gate passes and, for BOTH pack_epi_degree and pack_epi_qguard baselines, separately each protocol and allocation class has diagnostic4GM>=1.03 and wins>=3/4. No replacement by winning variant, per-graph oracle or threshold relaxation. Online-vs-replay gains alone are mechanism evidence, not practical superiority.

Primary contrasts: cached-S/dyn-S tests offline decision execution with same logical current-head requests; rep-S/cached-S adds representative-index change; cached-S/trace-I compares shared decode under same mask engine, subject to codegen; trace-S vs both PACK baselines tests net value. Report differences in branches, metadata and codegen, not all speedup as comparison elimination. No AVX-AMX overlap or DRAM/cache-level causal claims from these counters alone.

Build and each job reread/hash current rules and specs. Unique snapshots, manifests, raw logs, original source recovery, bindings and all Slurm results retained. Three inference handoffs synchronized using record_event.py after protocol/build/submission/completion/analysis. Source tree lacks .git; no initialization or push. Previous round complete and delivered, all jobs released.
'''
assert not (exp/'PROTOCOL.md').exists()
(exp/'PROTOCOL.md').write_text(protocol)
gen=(prev/'generate_prefetch_driver.py').read_text().replace('pf_runtime.hpp','trace_runtime.hpp').replace('gcn_extra_prefetch_runtime','gcn_extra_event_trace_runtime')
gen=gen.replace("'pf_reducer.hpp','pf_kernel.hpp','trace_runtime.hpp'", "'trace_plan.hpp','trace_reducer.hpp','trace_kernel.hpp','trace_runtime.hpp'")
gen=gen.replace(" (out/name).write_bytes((root/'src'/name).read_bytes())", " assert (out/name).is_file(),name")
(exp/'generate_trace_driver.py').write_text(gen)
job=(prev/'prefetch_job.sh').read_text().replace('KIND=prefetch','KIND=trace').replace('prefetch_run','trace_run').replace('pf_suite.py','trace_suite.py')
(exp/'trace_job.sh').write_text(job)
submit=(prev/'submit_prefetch.py').read_text().replace("kind='prefetch'","kind='trace'").replace('prefetch_job.sh','trace_job.sh').replace('prefetch_submission','trace_submission')
submit=submit.replace("'00:20:00' if mode=='smoke' else '01:30:00'", "'00:10:00' if mode=='smoke' else ('00:35:00' if 'mycielskian19' in mode else '00:12:00')")
(exp/'submit_trace.py').write_text(submit)
build=(prev/'build_prefetch.sh').read_text().replace('experiments/access_prefetch_20261008','experiments/event_trace_20261008').replace('prefetch_build','trace_build')
build=build.replace('pf_reducer.hpp pf_kernel.hpp check_pf_kernel.py PF_KERNEL_INTERFACE.md pf_runtime.hpp pf_suite.py', 'trace_plan.hpp trace_reducer.hpp trace_kernel.hpp check_trace_kernel.py KERNEL_INTERFACE.md trace_runtime.hpp trace_suite.py RUNTIME_INTERFACE.md')
build=build.replace('generate_prefetch_driver.py','generate_trace_driver.py').replace('build_prefetch.sh','build_trace.sh').replace('prefetch_job.sh','trace_job.sh').replace('submit_prefetch.py','submit_trace.py').replace('prefetch_methods','trace_methods').replace('check_pf_kernel.py','check_trace_kernel.py').replace('runs/order-share-factorial-20261007-01','runs/access-prefetch-20261008-01')
build=build.replace('cp "$ROOT/original/gcn_e2e_v3.cpp" "$ROOT/src/paper_methods_kernels.hpp" "$ROOT/src/paper_methods_runtime.hpp" "$ROOT/src/projection_window_kernels.hpp" "$ROOT/src/projection_window_pmu.hpp"', 'cp "$EXP/gcn_e2e_v3.cpp" "$EXP/paper_methods_kernels.hpp" "$EXP/paper_methods_runtime.hpp" "$EXP/projection_window_kernels.hpp" "$EXP/projection_window_pmu.hpp"')
(exp/'build_trace.sh').write_text(build)
event={'id':'event-trace-protocol-20261008','status':'PROTOCOL_FROZEN_IMPLEMENTATION_IN_PROGRESS','evidence':str(exp),'paper_eligible':False,'fixed_candidate':'trace_s_rep','dimensions':[128,128,128],'numa_interleave':False,'changed_first_touch':False,'user_node_cap':10,'planned_peak':6,'protocol_sha256':hashlib.sha256((exp/'PROTOCOL.md').read_bytes()).hexdigest(),'issues':'New probe isolates offline decision replay; no new result. Fixed PF1, one adjacent plan. Original and previous snapshots retained.','next':'Build trace plan/kernel/runtime, validate small fixtures, then fixed diagnostics.'}
(exp/'protocol_event.json').write_text(json.dumps(event,indent=2)+'\n')
subprocess.run(['python3',str(root/'scripts/record_event.py'),str(exp/'protocol_event.json')],check=True)
print('TRACE_BOOTSTRAP_READY')
