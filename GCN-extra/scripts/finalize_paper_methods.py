#!/usr/bin/env python3
"""Reconcile controlled, same-process comparisons, without selecting away losses."""
import csv,hashlib,json,math,pathlib,statistics,subprocess,sys
root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
runs=[root/'runs'/x for x in sys.argv[1:]]
assert len(runs)==3
allrows=[];allchecks=[];states=[];timings=[];stages=[];profiles=[];nodes={};binary_hashes=[]
def readcsv(p):return list(csv.DictReader(p.open()))
def writecsv(p,rows):
 keys=list(dict.fromkeys(k for x in rows for k in x))
 with p.open('w',newline='') as f:
  w=csv.DictWriter(f,keys);w.writeheader();w.writerows(rows)
def gm(v):return math.exp(sum(math.log(x) for x in v)/len(v))
for r in runs:
 c=json.loads((r/'completion.json').read_text());assert c['failed']==0 and (r/'exit_status.txt').read_text().strip()=='0'
 assert c['checks']==c['passed']*60
 node=(r/'node.txt').read_text().strip();nodes[r.name]=node
 binary_hashes.append([line.split()[0] for line in (r/'binary.sha256').read_text().splitlines()])
 rs=json.loads((r/'suite_status.json').read_text());assert all(x['status']=='PASS' for x in rs)
 for x in rs:
  folder=r/x['graph'];x['run']=r.name;x['node']=node;states.append(x)
  for label in ['original_raw','methods']:assert json.loads((folder/(label+'.status.json')).read_text())['returncode']==0
  times=readcsv(folder/'time.csv');assert len(times)==160
  checks=readcsv(folder/'check.csv');assert len(checks)==60 and all(x['pass']=='1' for x in checks)
  timings.extend(times);allchecks.extend(checks);stages.extend(readcsv(folder/'stage.csv'));profiles.extend(readcsv(folder/'profile.csv'))
 allrows.extend(readcsv(r/'summary.csv'))
assert all(x==binary_hashes[0] for x in binary_hashes)
assert len(states)==17 and len({x['graph'] for x in states})==17
assert len(allchecks)==1020 and len(allrows)==544 and len(timings)==2720
out=root/'runs/paper-method-reconciled-20261004';assert not out.exists();out.mkdir()
writecsv(out/'all_summary.csv',allrows);writecsv(out/'all_checks.csv',allchecks);writecsv(out/'raw_repeats.csv',timings);writecsv(out/'stages.csv',stages);writecsv(out/'sampled_profiles.csv',profiles)
(out/'graph_states.json').write_text(json.dumps(states,indent=2))
lookup={(x['graph'],x['method'],x['kind']):x for x in allrows}
variants=sorted({x['method'] for x in allrows if x['method'] not in ['paper_tfs','source_mkl_fp32']},key=lambda x:(999 if x.startswith('bfull') else int(x.split('_')[0][1:]),x.endswith('accurate')))
fixed=[]
for m in variants:
 rows=[x for x in allrows if x['method']==m and x['kind']=='e2e'];assert len(rows)==17
 fixed.append(dict(method=m,graphs=17,median_gmean_vs_tfs=gm([float(x['speedup_median_vs_tfs']) for x in rows]),median_gmean_vs_mkl=gm([float(x['speedup_median_vs_mkl']) for x in rows]),min_gmean_vs_tfs=gm([float(x['speedup_min_vs_tfs']) for x in rows]),wins_vs_tfs=sum(float(x['speedup_median_vs_tfs'])>1 for x in rows),wins_vs_mkl=sum(float(x['speedup_median_vs_mkl'])>1 for x in rows)))
writecsv(out/'fixed_method_averages.csv',fixed)
graph_rows=[];oracle=[]
for s in states:
 g=s['graph'];base=lookup[g,'paper_tfs','e2e'];mk=lookup[g,'source_mkl_fp32','e2e']
 fast=sorted([lookup[g,m,'e2e'] for m in variants if m.endswith('_fast')],key=lambda x:float(x['median_ms']))
 row=dict(graph=g,N=s['N'],E=s['E'],avg_degree=s['avg_degree'],node=s['node'],mkl_ms=mk['median_ms'],tfs_ms=base['median_ms'])
 for m in ['b8_fast','b16_fast','b64_fast','bfull_fast','bfull_accurate']:
  x=lookup[g,m,'e2e'];row[m+'_ms']=x['median_ms'];row[m+'_speedup_vs_tfs']=x['speedup_median_vs_tfs']
 row.update(best_fast=fast[0]['method'],best_fast_ms=fast[0]['median_ms'],best_fast_speedup_vs_tfs=fast[0]['speedup_median_vs_tfs'])
 oracle.append(float(fast[0]['speedup_median_vs_tfs']));graph_rows.append(row)
writecsv(out/'per_graph_e2e.csv',graph_rows)
anchor_audit=[]
for s in states:
 for a in s['anchor_audit']:
  anchor_audit.append(dict(graph=s['graph'],node=s['node'],method=a['method'],embedded_vs_raw_min_ratio=a['embedded_vs_raw_min_ratio'],rotating_vs_raw_median_ratio=float(a['rotating']['median_ms'])/a['raw_anchor']['median_ms']))
writecsv(out/'anchor_audit.csv',anchor_audit)
gate_max=[]
for m in ['paper_tfs']+variants:
 for boundary,ref in [('kernel_FP32','paper_tfs'),('e2e_FP32_final','paper_tfs'),('e2e_BF16_final','paper_tfs'),('e2e_BF16_final','source_MKL_FP32')]:
  subset=[x for x in allchecks if x['method']==m and x['boundary']==boundary and x['reference']==ref];assert len(subset)==17
  gate_max.append(dict(method=m,boundary=boundary,reference=ref,max_relative_L2=max(float(x['relative_L2']) for x in subset),max_normalized_max=max(float(x['normalized_max']) for x in subset),gate=subset[0]['gate']))
writecsv(out/'numerical_maxima.csv',gate_max)
summary=dict(graphs=17,methods=16,candidate_methods=14,numerical_checks=1020,numerical_failures=0,measured_repetitions=2720,nodes=nodes,same_binary_hashes=True,primary_statistic='median of five, same graph/process/buffers; corresponding original TFS/MKL medians',fixed=fixed,oracle_best_fast_gmean_vs_tfs=gm(oracle),oracle_best_fast_wins=sum(x>1 for x in oracle),oracle_is_not_implemented_adaptive_method=True,anchor_min_ratio_range=[min(x['embedded_vs_raw_min_ratio'] for x in anchor_audit),max(x['embedded_vs_raw_min_ratio'] for x in anchor_audit)])
(out/'audit_summary.json').write_text(json.dumps(summary,indent=2))
with (out/'slurm_accounting.txt').open('w') as f:
 subprocess.run(['sacct','-j',','.join(r.name.split('-')[-1] for r in runs),'-o','JobID,JobName,Partition,NodeList,State,ExitCode,Elapsed,AllocCPUS,MaxRSS','-P'],stdout=f,check=True)
doc=['# Controlled neighbor-reduction results against paper TFS v3 (2026-10-04)','',
'## Controls and scope','',
'17 real, unit-valued graphs; same CSR and source-seeded H/W, DegreeSort, original node order, TR=16, R=64, 128->128->128. Every graph compares all methods on one node and process. The three graph batches use disjoint exclusive nodes; speedups never mix nodes. All TFS methods have FP32 layer1, FP32 ReLU, original BF16 interlayer conversion and direct BF16 layer2 output. No NUMA/interleave policy, MKL helper/hint/allocation change, or replacement data. Original MKL remains FP32 as in the paper. Fixed variables and gates are in `PAPER_METHODS_PROTOCOL_20261004.md`.','',
'Existing neighbor reduction was retained; only output store typing and the paper-v3 harness were adapted. FAST uses one truncated BF16 partial; ACCURATE uses hi plus truncated residual lo. Every partial is projected immediately by AMX in a destination tile, with no global AH. Original paper v3 source is byte-recoverable after removing its include/hook, and an untouched independently compiled binary is retained as an anchor.','',
'## Complete evidence','',
'- Build: `runs/paper-method-build-20261004-123445`.',
'- Smoke 10864873: two empty-row/tail/group-boundary fixtures, 120 checks, PASS.',
'- Formal batches: '+', '.join(r.name+' on '+nodes[r.name] for r in runs)+'.',
'- Formal checks: 1020/1020 PASS; 2720 measured repetitions.',
'- Reconciled evidence: `'+str(out)+'`.',
'- Source and binary hashes, input full SHA256, exact argv, environment, graph exit statuses, Slurm accounting, raw times and profiles are retained.','',
'## Primary fixed-method results','',
'All-17 geometric means, median of five with one warmup. Faster than original TFS means speedup >1. No slow graphs are removed.','',
'| Method | vs paper TFS | vs source MKL | TFS wins /17 | MKL wins /17 |',
'|---|---:|---:|---:|---:|']
for x in fixed:doc.append(f"| {x['method']} | {x['median_gmean_vs_tfs']:.3f}x | {x['median_gmean_vs_mkl']:.3f}x | {x['wins_vs_tfs']} | {x['wins_vs_mkl']} |")
doc+=['','## Per-graph two-layer E2E','',
'Median milliseconds. FULL FAST/ACCURATE are fixed policies. Best FAST is selected after measurement and must be labelled an oracle.','',
'| Graph | Average degree | MKL | Paper TFS | B8 FAST | B16 FAST | B64 FAST | FULL FAST | FULL ACCURATE | Best FAST | Best/TFS speedup |',
'|---|---:|---:|---:|---:|---:|---:|---:|---:|---|---:|']
for x in graph_rows:doc.append('| '+x['graph']+' | '+f"{x['avg_degree']:.1f} | {float(x['mkl_ms']):.2f} | {float(x['tfs_ms']):.2f} | {float(x['b8_fast_ms']):.2f} | {float(x['b16_fast_ms']):.2f} | {float(x['b64_fast_ms']):.2f} | {float(x['bfull_fast_ms']):.2f} | {float(x['bfull_accurate_ms']):.2f} | {x['best_fast']} | {float(x['best_fast_speedup_vs_tfs']):.3f}x |")
doc+=['',f"After-the-fact best FAST oracle geometric mean vs paper TFS: {gm(oracle):.3f}x, wins {sum(x>1 for x in oracle)}/17. This is not an adaptive implementation and must not replace the fixed-method averages.",'',
'## Numerical controls','',
'Full-output strict FP32 checks retain FAST 1e-2 and ACCURATE 1e-3 thresholds. Actual BF16 final output vs paper BF16 output uses FAST 2e-2 and ACCURATE 1e-2 to allow one final BF16 rounding bin; FP32 MKL reference gate stays 3e-2. All thresholds were recorded before build or measurement, and none were relaxed. Original repeated outputs must be bitwise identical. See `numerical_maxima.csv` and every `check.csv`; these are numerical gates, not classification accuracy.','',
'## Timing, anchor audit, and limits','',
'Primary rotating measurements use one warmup plus five repetitions for every method, source MKL and paper TFS in the same process and original source buffers. Both minimum and median are retained. Raw unmodified anchor measurements and embedded source timings are separate from this primary comparison. `anchor_audit.csv` gives per-graph correspondence, rather than substituting a historical baseline.','',
f"Embedded original versus untouched original minimum-ratio range across TFS and MKL: {summary['anchor_min_ratio_range'][0]:.4f} to {summary['anchor_min_ratio_range'][1]:.4f}. All raw values remain visible for variability review.",'',
'Stage timing is separate: layer1, ReLU, BF16 interlayer conversion, layer2, total; three repeats each. Sampled hot-loop profiles cover row scheduling, partial zeroing, prefetch, feature gather/decode, FP32 reduction, partial BF16 conversion, tile load, tile compute, tile store, final scatter, and AMX setup. BF16 output scatter includes final BF16 conversion. Profile sums are sampled thread time, not additive wall latency, and do not determine speedups.','',
'E2E is prepared two-layer computational inference. Initial loading, DegreeSort, input conversion, and W packing are outside the interval. Random source-defined H/W are used; no trained classification checkpoint is claimed. Six nonunit graphs remain incomparable because original TFS ignores values while MKL uses them; Friendster and road_usa remain outside original LP64 safe bounds. No dataset was changed.','',
'## Reproduction','',
'`bash scripts/build_paper_methods.sh`; then shared `bash scripts/submit_paper_methods.sh smoke`; after the gate, exclusive `high`, `medium`, `low` submissions using the same script. Launcher scripts and pinned build paths are immutable per submission.','']
target=root/'docs/PAPER_METHODS_RESULTS_20261004.md';target.write_text('\n'.join(doc))
event=dict(id='paper-method-reconciliation-20261004',kind='controlled_paper_method_reconciliation',status='PASS',report=str(target),evidence=str(out),summary=summary,paper_eligible=True,issues='No gate relaxation or historical/cross-node denominator substitution; fixed-method losses and oracle selection retained',next='Interpret degree-dependent crossover before any further optimization')
(out/'event.json').write_text(json.dumps(event,indent=2));subprocess.run(['python3',str(root/'scripts/record_event.py'),str(out/'event.json')],check=True)
print(json.dumps(summary,indent=2))
