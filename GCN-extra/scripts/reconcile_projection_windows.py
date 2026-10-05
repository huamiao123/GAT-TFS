#!/usr/bin/env python3
"""Reconcile frozen-window runs; no cross-run speedup denominators."""
import csv,datetime,hashlib,json,pathlib,statistics,sys
root=pathlib.Path(__file__).resolve().parents[1]
runs=[root/'runs'/x for x in sys.argv[1:]]
assert len(runs)==2
out=root/'runs'/('projection-window-reconciled-'+datetime.datetime.now().strftime('%Y%m%d-%H%M%S'))
out.mkdir();rows=[];checks=[];equiv=[];tables={};states=[];hashes=[]
for run in runs:
 assert (run/'exit_status.txt').read_text().strip()=='0',run
 c=json.loads((run/'completion.json').read_text());assert c['failed']==0
 with (run/'summary.csv').open() as f:rows.extend(csv.DictReader(f))
 with (run/'all_checks.csv').open() as f:checks.extend(csv.DictReader(f))
 with (run/'all_equiv.csv').open() as f:equiv.extend(csv.DictReader(f))
 states.extend(json.loads((run/'suite_status.json').read_text()))
 hashes.append([x.split()[0] for x in (run/'binary.sha256').read_text().splitlines()])
 for g in json.loads((run/'graph_list.json').read_text()):
  parsed=json.loads((run/g['graph']/'parsed.json').read_text())
  for k,v in parsed.items():tables.setdefault(k,[]).extend(v)
assert hashes[0]==hashes[1]
inventory={x['graph']:x for x in json.loads((root/'docs/CURRENT_GRAPH_INVENTORY_20261004.json').read_text())}
assert {s['graph'] for s in states}==set(inventory) and len(states)==17
assert all(s['status']=='PASS' and s['input_sha256']==inventory[s['graph']]['input_sha256'] for s in states)
assert len(checks)==1020 and all(x['pass']=='1' for x in checks)
assert len(equiv)==408 and all(x['bitwise']=='1' and x['pass']=='1' for x in equiv)
assert len(rows)==17*16*2
assert tables['PROFILE_GATE'] and all(x['bitwise']=='1' and x['pass']=='1' for x in tables['PROFILE_GATE'])
def writecsv(path,data):
 if not data:return
 keys=list(dict.fromkeys(k for x in data for k in x))
 with path.open('w',newline='') as f:
  w=csv.DictWriter(f,keys);w.writeheader();w.writerows(data)
writecsv(out/'summary.csv',rows);writecsv(out/'checks.csv',checks);writecsv(out/'equiv.csv',equiv)
for key,value in tables.items():writecsv(out/(key.lower()+'.csv'),value)
gmean=lambda v:statistics.geometric_mean(v)
fixed=[]
for m in sorted({x['method'] for x in rows}):
 p=[x for x in rows if x['method']==m and x['kind']=='consecutive_e2e']
 s=[x for x in rows if x['method']==m and x['kind']=='rotating_e2e']
 assert len(p)==len(s)==17
 fixed.append(dict(method=m,graphs=17,consecutive_gmean_vs_tfs=gmean([float(x['speedup_median_vs_tfs']) for x in p]),
  consecutive_gmean_vs_mkl=gmean([float(x['speedup_median_vs_mkl']) for x in p]),
  rotating_gmean_vs_tfs=gmean([float(x['speedup_median_vs_tfs']) for x in s]),
  wins_vs_tfs=sum(float(x['speedup_median_vs_tfs'])>1 for x in p),wins_vs_mkl=sum(float(x['speedup_median_vs_mkl'])>1 for x in p)))
primary={(x['graph'],x['method']):x for x in rows if x['kind']=='consecutive_e2e'}
contrasts=[]
for graph in sorted(inventory):
 get=lambda m:float(primary[(graph,m)]['median_ms'])
 contrasts.append(dict(graph=graph,s64_full_vs_coupled_full=get('bfull_fast')/get('s64_mfull_fast'),
  s128_full_vs_coupled_full=get('bfull_fast')/get('s128_mfull_fast'),
  s64_full_vs_b64=get('b64_fast')/get('s64_mfull_fast'),
  s64_m256_vs_coupled_b256=get('b256_fast')/get('s64_m256_fast'),
  new_coupled64_vs_old=get('b64_fast')/get('s64_m64_fast')))
writecsv(out/'fixed.csv',fixed);writecsv(out/'contrasts.csv',contrasts)
pmu=tables.get('PMU',[])
summary=dict(graphs=17,numerical_checks=len(checks),bitwise_checks=len(equiv),profile_bitwise_checks=len(tables['PROFILE_GATE']),
 measured_repetitions=len(tables['TIME']),fixed=fixed,contrasts=contrasts,runs=[str(x) for x in runs],
 nodes={r.name:(r/'node.txt').read_text().strip() for r in runs},same_binary_hashes=True,
 pmu_regions=len(pmu),pmu_available=sum(x.get('available')=='1' for x in pmu),
 primary_statistic='median of five consecutive complete forwards, same graph/process/protocol baseline',
 model='two-layer 128->128->128 static inference compute with random source H/W; not checkpoint classification')
(out/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
report=['# Independent window / projection scope results','',summary['model'],'',
 'Controls and gates: [preregistered protocol](PROJECTION_WINDOW_PROTOCOL_20261004.md). Original source TFS and MKL unchanged; DegreeSort/TR16/R64/AMX/default NUMA preserved. Source TFS ends in BF16, source MKL in FP32.','',
 f"17 graphs; {len(checks)} standard gates, {len(equiv)} same-scope bitwise gates, {len(tables['PROFILE_GATE'])} profiling bitwise gates passed. Raw evidence: `{out.relative_to(root)}`.",'',
 '## Complete two-layer times','', '| Graph | Source TFS ms | Source MKL ms | B64 FAST ms | FULL FAST ms | S64/MFULL ms | S128/MFULL ms | S64/M256 ms |','|---|---:|---:|---:|---:|---:|---:|---:|']
for graph in sorted(inventory):
 vals=[float(primary[(graph,m)]['median_ms']) for m in ['paper_tfs','source_mkl_fp32','b64_fast','bfull_fast','s64_mfull_fast','s128_mfull_fast','s64_m256_fast']]
 report.append('| '+graph+' | '+' | '.join(f'{v:.3f}' for v in vals)+' |')
report+=['','## Fixed policy results','', '| Method | Gmean vs TFS | Gmean vs MKL | Wins vs TFS | Rotation vs TFS |','|---|---:|---:|---:|---:|']
for f in fixed:report.append(f"| {f['method']} | {f['consecutive_gmean_vs_tfs']:.4f} | {f['consecutive_gmean_vs_mkl']:.4f} | {f['wins_vs_tfs']}/17 | {f['rotating_gmean_vs_tfs']:.4f} |")
report+=['','## Measurement limits','',
 'Varying S at fixed M preserves bitwise output and executed AMX count. Varying M changes projection grouping and quantization points. Speedups are paired within this run; prior timing values are not denominators.',
 '', '24 sampled child intervals and per-thread records overlap with parent timers and include profiling perturbation. They are diagnostic records, not additive wall time fractions. Analytic/requested bytes are not measured DRAM traffic. PMU records retain per-thread coverage/scaling and enable/disable skew; generic cache events are CPU dependent. Performance counters are measured in separate complete-forward regions after primary timing.',
 '', 'No graph-wise oracle is reported as an implemented policy. A useful window or block mechanism alone does not establish a journal contribution; the actual AMX work, sparse dependency traversal, partial-state lifetime and numerical constraints need a validated general model.']
(root/'docs/PROJECTION_WINDOW_RESULTS_20261004.md').write_text('\n'.join(report)+'\n')
event=dict(id=out.name,kind='projection_window_reconciliation',status='PASS',evidence=str(out),report=str(root/'docs/PROJECTION_WINDOW_RESULTS_20261004.md'),paper_eligible=True,summary=summary,issues='All negative cases and both timing orders retained; same-process denominators; no novelty claim from caching alone',next='Analyze fixed-M contrasts, detailed timing, PMU and remaining limits')
(out/'event.json').write_text(json.dumps(event,indent=2)+'\n')
print(json.dumps(summary,indent=2))
