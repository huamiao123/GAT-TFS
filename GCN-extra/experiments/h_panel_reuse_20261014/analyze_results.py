#!/usr/bin/env python3
"""Summarize each independent run without pooling unlike nodes or orders."""
import csv, json, pathlib, statistics, sys

root=pathlib.Path(sys.argv[1])
output=[]
for run in sorted(root.glob('h-panel-reuse-*')):
    manifest=run/'manifest.json'
    timings=run/'results'/'timings.csv'
    if not manifest.exists() or not timings.exists():continue
    meta=json.loads(manifest.read_text())
    groups={}
    for row in csv.DictReader(timings.open(newline='')):
        groups.setdefault((row['graph'],row['method']),[]).append(row)
    for (graph,method),items in groups.items():
        vals=sorted(float(x['total_ms']) for x in items)
        baseline=sorted(float(x['total_ms']) for x in groups[(graph,'A0_original')])
        out=dict(run=run.name,node=meta['hostname'],graph=graph,method=method,
                 repeats=len(vals),min_ms=vals[0],p25_ms=statistics.median(vals[:2]),
                 median_ms=statistics.median(vals),p75_ms=statistics.median(vals[-2:]),
                 max_ms=vals[-1],cv_pct=100*statistics.pstdev(vals)/statistics.mean(vals),
                 speedup_vs_A0=statistics.median(baseline)/statistics.median(vals))
        for stage in ('layer1_ms','relu_ms','conversion_ms','layer2_ms'):
            out[stage+'_median']=statistics.median(float(x[stage]) for x in items)
        output.append(out)
if output:
    with (root/'summary_by_run.csv').open('w',newline='') as f:
        writer=csv.DictWriter(f,fieldnames=output[0]);writer.writeheader();writer.writerows(output)
    (root/'summary_by_run.json').write_text(json.dumps(output,indent=2)+'\n')
print(json.dumps(dict(runs=sorted({x['run'] for x in output}),records=len(output)),indent=2))
