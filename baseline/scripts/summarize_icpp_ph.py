#!/usr/bin/env python3
"""Parse complete two-layout PH micro logs with explicit layout provenance."""
import argparse
import csv
from pathlib import Path
parser=argparse.ArgumentParser()
parser.add_argument('run',type=Path)
run=parser.parse_args().run.resolve()
groups={key:[] for key in ('HEAD_PH_SUMMARY','HEAD_PH_TIME','HEAD_PH_ERROR',
                         'HEAD_PH_INSTRUCTION_GATE','HEAD_PH_DEN_CONTROL','HEAD_PH_ORACLE')}
for graph in ('arxiv','products'):
    lines=(run/f'{graph}.log').read_text(encoding='utf-8').splitlines()
    if not any(line.startswith('ICPP_PH_COMPLETE ') for line in lines):
        raise SystemExit('incomplete '+graph)
    variant=None
    for line in lines:
        prefix=line.split(' ',1)[0]
        values=dict(item.split('=',1) for item in line.split()[1:] if '=' in item)
        if prefix=='PH_LAYOUT_CONTROL':variant=values['variant']
        if prefix in groups:
            if variant is None:raise SystemExit('missing layout provenance')
            groups[prefix].append(dict(graph=graph,variant=variant,**values))
for prefix,rows in groups.items():
    target=run/(prefix.lower()+'.tsv')
    if target.exists():raise SystemExit('refusing overwrite '+str(target))
    keys=list(dict.fromkeys(key for row in rows for key in row))
    with target.open('w',encoding='utf-8',newline='') as out:
        writer=csv.DictWriter(out,keys,delimiter='\t');writer.writeheader();writer.writerows(rows)
print('graph\tlayout\tmethod\tmedian_ms\tvs_same_layout_AVX')
for row in groups['HEAD_PH_SUMMARY']:
    print('\t'.join(row[key] for key in ('graph','variant','method','median_ms','micro_speed_vs_AVX')))
print('scope=sampled_hot_instrumented_firstblock_micro; not fullmodel acceleration')
