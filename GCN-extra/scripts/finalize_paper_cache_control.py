#!/usr/bin/env python3
"""Retain both execution-order protocols, with same-run denominators only."""
import csv,json,math,pathlib,subprocess,sys
root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
main=root/'runs/paper-method-reconciled-20261004'
runs=[root/'runs'/x for x in sys.argv[1:]]
assert len(runs)==3
def read(p):return list(csv.DictReader(p.open()))
def write(p,rows):
 with p.open('w',newline='') as f:
  w=csv.DictWriter(f,list(dict.fromkeys(k for x in rows for k in x)));w.writeheader();w.writerows(rows)
def gm(xs):return math.exp(sum(math.log(x) for x in xs)/len(xs))
rows=[];checks=[];times=[];states=[];hashes=[]
old={x['graph']:x for x in json.loads((main/'graph_states.json').read_text())}
for r in runs:
 c=json.loads((r/'completion.json').read_text());assert c['failed']==0 and c['checks']==c['passed']*36
 assert (r/'exit_status.txt').read_text().strip()=='0'
 hashes.append([x.split()[0] for x in (r/'binary.sha256').read_text().splitlines()])
 for s in json.loads((r/'suite_status.json').read_text()):
  assert s['status']=='PASS' and s['input_sha256']==old[s['graph']]['input_sha256']
  for k in ['N','E','path']:assert s[k]==old[s['graph']][k]
  s.update(run=r.name,node=(r/'node.txt').read_text().strip());states.append(s)
  p=r/s['graph'];cs=read(p/'check.csv');ts=read(p/'time.csv')
  assert len(cs)==36 and all(x['pass']=='1' for x in cs) and len(ts)==50
  for label in ['original_raw','methods']:assert json.loads((p/(label+'.status.json')).read_text())['returncode']==0
  checks+=cs;times+=ts
 rows+=read(r/'summary.csv')
assert all(h==hashes[0] for h in hashes) and len(states)==17 and len({s['graph'] for s in states})==17
assert len(checks)==612 and len(times)==850 and len(rows)==170
out=root/'runs/paper-cache-reconciled-20261004';assert not out.exists();out.mkdir()
for name,rs in [('all_summary',rows),('all_checks',checks),('raw_repeats',times)]:write(out/(name+'.csv'),rs)
(out/'graph_states.json').write_text(json.dumps(states,indent=2))
lookup={(x['graph'],x['method']):x for x in rows}
oldrows={(x['graph'],x['method']):x for x in read(main/'all_summary.csv') if x['kind']=='e2e'}
methods=['b8_fast','b8_accurate','b16_fast','b16_accurate','b64_fast','b64_accurate','bfull_fast','bfull_accurate']
fixed=[]
for m in methods:
 rs=[x for x in rows if x['method']==m]
 fixed.append(dict(method=m,graphs=17,consecutive_gmean_vs_tfs=gm([float(x['speedup_median_vs_tfs']) for x in rs]),consecutive_gmean_vs_mkl=gm([float(x['speedup_median_vs_mkl']) for x in rs]),interleaved_gmean_vs_tfs=gm([float(oldrows[x['graph'],m]['speedup_median_vs_tfs']) for x in rs]),interleaved_gmean_vs_mkl=gm([float(oldrows[x['graph'],m]['speedup_median_vs_mkl']) for x in rs]),wins_vs_tfs=sum(float(x['speedup_median_vs_tfs'])>1 for x in rs),wins_vs_mkl=sum(float(x['speedup_median_vs_mkl'])>1 for x in rs)))
write(out/'protocol_comparison.csv',fixed)
graphs=[]
for s in states:
 g=s['graph'];d=dict(graph=g,N=s['N'],E=s['E'],avg_degree=s['avg_degree'],node=s['node'])
 for m in ['paper_tfs','source_mkl_fp32']+methods:
  x=lookup[g,m];d[m+'_ms']=x['median_ms'];d[m+'_vs_tfs']=x['speedup_median_vs_tfs'];d[m+'_vs_mkl']=x['speedup_median_vs_mkl']
  d[m+'_interleaved_ms']=oldrows[g,m]['median_ms']
 graphs.append(d)
write(out/'per_graph_e2e.csv',graphs)
audit=dict(graphs=17,numerical_checks=612,numerical_failures=0,measured_repetitions=850,source_input_hashes_match_main=True,same_binary_hashes=True,fixed=fixed,primary_statistic='median of five consecutive repetitions after immediate warmup; same-process TFS/MKL denominator',nodes={r.name:(r/'node.txt').read_text().strip() for r in runs})
(out/'audit_summary.json').write_text(json.dumps(audit,indent=2))
with (out/'slurm_accounting.txt').open('w') as f:subprocess.run(['sacct','-j',','.join(r.name.split('-')[-1] for r in runs),'-o','JobID,Partition,NodeList,State,ExitCode,Elapsed,AllocCPUS','-P'],stdout=f,check=True)
doc=['# Execution-order control (2026-10-04)','',
'17 real graphs, 612/612 numerical checks passed, 850 measured two-layer E2E repetitions. This supplement was planned after observing small-graph cache sensitivity in interleaved execution. Every method receives an immediate warmup and five consecutive measurements, matching the original source timing order. The previous interleaved results remain intact.','',
'## Frozen controls','',
'Same full CSR hashes, source-seeded H/W, shapes 128→128→128, DegreeSort, original node order, AMX tiles, 32 physical threads, exclusive allocation, default NUMA policy, compiler flags, original MKL helper/hint, FP32 intermediate and direct BF16 final output. No kernel change, parameter selection, numerical gate relaxation, or input substitution. `cache_generation.json` records the frozen kernel hash and the timing-only insertion. Original source MKL stays FP32 as in the paper. The shared smoke had 72/72 checks.','',
'Measured protocols ran at different times/nodes; within each graph all methods share one node/process. Ratios use the TFS/MKL measured in the same protocol and process. Between-protocol differences include run variability and cannot all be causally assigned to cache.','',
'## Fixed policies: all-17 geometric means','',
'| Method | Consecutive / TFS | Consecutive / MKL | Interleaved / TFS | Interleaved / MKL | TFS wins | MKL wins |',
'|---|---:|---:|---:|---:|---:|---:|']
for x in fixed:doc.append(f"| {x['method']} | {x['consecutive_gmean_vs_tfs']:.3f}x | {x['consecutive_gmean_vs_mkl']:.3f}x | {x['interleaved_gmean_vs_tfs']:.3f}x | {x['interleaved_gmean_vs_mkl']:.3f}x | {x['wins_vs_tfs']}/17 | {x['wins_vs_mkl']}/17 |")
doc+=['','## Consecutive E2E milliseconds','',
'| Graph | MKL | Original TFS | B8 FAST | B16 FAST | B64 FAST | FULL FAST | FULL ACCURATE |',
'|---|---:|---:|---:|---:|---:|---:|---:|']
for x in graphs:doc.append('| '+x['graph']+' | '+' | '.join(f"{float(x[m+'_ms']):.2f}" for m in ['source_mkl_fp32','paper_tfs','b8_fast','b16_fast','b64_fast','bfull_fast','bfull_accurate'])+' |')
doc+=['','## Boundaries and retained evidence','',
'No losing graph is removed. FULL is not universally faster; compare Mycielskian19 with bounded blocks. Average degree alone does not select the best policy. There is no implemented adaptive selector. FAST/ACCURATE are numerical error controls, not measured classification accuracy. Random source-defined H/W remain in use.','',
'Prepared two-layer computational inference includes layer1, ReLU, intermediate BF16 conversion, layer2 and final output. It excludes initial graph loading, DegreeSort, feature conversion and weight packing exactly as the original source. Detailed stage and sampled hot-loop profiles remain in the main experiment, with their instrumentation limits. Weighted graphs and original LP64-incompatible shapes retain the prior explicit exclusions.','',
'Build: `runs/paper-cache-build-20261004-131154`. Runs: '+', '.join(r.name for r in runs)+'. Reconciled CSV, checks, raw repetitions, source/binary hashes and Slurm accounting are retained under `runs/paper-cache-reconciled-20261004`. Scheduler timeout alone was reduced from two hours to 30 minutes while jobs were pending, based on the larger completed main sweep taking at most 16m43s; before/after requests are recorded in `runs/paper-cache-scheduling-adjustment-20261004-133536`. No measured interval, kernel, thread, memory, affinity or exclusivity setting changed. The parser retains the historical JSON key `rotating` for the supplemental anchor object; in these cache runs it contains consecutive measurements.','',
'Reproduce: restore the hash-verified evidence archive for a source-only Git checkout (compiled binaries are archived, not tracked as separate Git files). The cache build intentionally pins the frozen `paper-method-build-20261004-123445`, rather than silently picking a newly generated source. Then `bash scripts/build_paper_cache_control.sh`, shared `bash scripts/submit_paper_cache_control.sh smoke`, and authorized exclusive `high`, `medium`, `low` using the same script.','']
target=root/'docs/PAPER_CACHE_CONTROL_RESULTS_20261004.md';target.write_text('\n'.join(doc))
event=dict(id='paper-cache-reconciliation-20261004',kind='execution_order_control_reconciliation',status='PASS',report=str(target),evidence=str(out),summary=audit,paper_eligible=True,issues='Both protocols retained; same-process denominator, frozen kernel/gates and input hashes; cache effects not isolated from cross-run variability',next='Archive and publish all main and supplemental evidence')
(out/'event.json').write_text(json.dumps(event,indent=2));subprocess.run(['python3',str(root/'scripts/record_event.py'),str(out/'event.json')],check=True)
print(json.dumps(audit,indent=2))
