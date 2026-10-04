#!/usr/bin/env python3
"""Descriptive repeat variability; this is not a significance test."""
import collections,csv,json,pathlib,statistics
root=pathlib.Path(__file__).resolve().parents[1]
out=root/'docs/figures/paper_methods_20261004';out.mkdir(parents=True,exist_ok=True)
rows=[]
for protocol,run in [('interleaved','paper-method-reconciled-20261004'),('consecutive','paper-cache-reconciled-20261004')]:
 p=root/'runs'/run/'raw_repeats.csv'
 if not p.exists():continue
 groups=collections.defaultdict(list)
 for x in csv.DictReader(p.open()):groups[x['graph'],x['method'],x['kind']].append(float(x['ms']))
 for (g,m,k),v in groups.items():
  assert len(v)==5
  q=statistics.quantiles(v,n=4,method='inclusive');median=statistics.median(v)
  rows.append(dict(protocol=protocol,graph=g,method=m,kind=k,repeats=5,min_ms=min(v),p25_ms=q[0],median_ms=median,p75_ms=q[2],max_ms=max(v),range_fraction=(max(v)-min(v))/median,sample_sd_ms=statistics.stdev(v),raw_ms=json.dumps(v)))
with (out/'repeat_variability.csv').open('w',newline='') as f:
 w=csv.DictWriter(f,list(rows[0]));w.writeheader();w.writerows(rows)
print('DESCRIPTIVE_VARIABILITY',len(rows),'groups, raw repeats retained; no significance claim')
