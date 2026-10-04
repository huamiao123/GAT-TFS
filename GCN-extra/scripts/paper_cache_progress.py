#!/usr/bin/env python3
import csv,json,pathlib
root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs')
for name in ['paper-cache-high-10864906','paper-cache-medium-10864907','paper-cache-low-10864908']:
 r=root/name
 if not r.exists():print(name,'PENDING');continue
 print(name,(r/'node.txt').read_text().strip() if (r/'node.txt').exists() else 'STARTING')
 states=json.loads((r/'suite_status.json').read_text()) if (r/'suite_status.json').exists() else []
 print('GRAPH_STATES',[(x['graph'],x['status']) for x in states])
 if (r/'summary.csv').exists():
  rows=list(csv.DictReader((r/'summary.csv').open()));graphs=list(dict.fromkeys(x['graph'] for x in rows))
  for g in graphs:
   selected=[dict(method=x['method'],ms=round(float(x['median_ms']),2),vs_tfs=round(float(x['speedup_median_vs_tfs']),3)) for x in rows if x['graph']==g and x['kind']=='e2e' and x['method'] in ['paper_tfs','source_mkl_fp32','b8_fast','b16_fast','b64_fast','bfull_fast','bfull_accurate']]
   print(g,json.dumps(selected))
 if (r/'suite.log').exists():
  lines=(r/'suite.log').read_text().splitlines()
  if lines and lines[-1].startswith('METHOD_START'):print('ACTIVE',lines[-1])
 if (r/'completion.json').exists():print('COMPLETE',(r/'completion.json').read_text())
