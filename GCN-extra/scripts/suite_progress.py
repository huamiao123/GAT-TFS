#!/usr/bin/env python3
"""Small read-only progress view of a running batch."""
import csv
import json
import pathlib
import statistics
import sys
run=pathlib.Path(sys.argv[1]);joblog=run.with_suffix('.out')
config=(run/'measurement_config.txt').read_text().splitlines() if (run/'measurement_config.txt').exists() else []
expected=int(next((s.split('=',1)[1] for s in config if s.startswith('repeats=')),'10'))
statuses=json.loads((run/'suite_status.json').read_text()) if (run/'suite_status.json').exists() else []
current=''
if joblog.exists():
    for line in joblog.read_text().splitlines():
        if line.startswith('START_GRAPH'):current=line.split()[1]
print('COMPLETED',len(statuses),'PASS',sum(x['status']=='PASS' for x in statuses),'CURRENT',current)
for r in statuses:
    if r['status']!='PASS':print(r['graph'],r['status'],r.get('reason',''));continue
    groups={}
    for t in csv.DictReader((run/r['graph']/'timings.csv').open()):
        if t['kind']=='e2e':groups.setdefault(t['method'],[]).append(float(t['ms']))
    med={m:statistics.median(v) for m,v in groups.items()}
    candidates=[m for m in med if m.startswith('shared_') and m.endswith('_fast_nozero')]
    best=min(candidates,key=med.get)
    print(r['graph'],'q',round(r['avg_degree'],2),'B8',round(med['shared_b8_fast_nozero'],3),'B16',round(med['shared_b16_fast_nozero'],3),
          'Full',round(med['shared_bfull_fast_nozero'],3),'best',best,round(med[best],3),'speedup',round(med['original_nozero']/med[best],3))
if current and (run/current/'stdout.log').exists():
    if (run/current/'timings.csv').exists():
        groups={}
        for r in csv.DictReader((run/current/'timings.csv').open()):
            if r.get('kind')=='kernel' and r.get('ms'):groups.setdefault(r['method'],[]).append(float(r['ms']))
        for m in ['original_nozero','shared_b8_fast_nozero','shared_b16_fast_nozero','shared_b64_fast_nozero','shared_bfull_fast_nozero']:
            if len(groups.get(m,[]))==expected:print('CURRENT_KERNEL_DONE',m,round(statistics.median(groups[m]),3))
    print('CURRENT_TAIL')
    print('\n'.join((run/current/'stdout.log').read_text().splitlines()[-2:]))
