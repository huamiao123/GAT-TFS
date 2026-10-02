#!/usr/bin/env python3
"""Summarize same-run wall timings; keep sampled worker spans separate."""
import json,csv,statistics,sys
from pathlib import Path
run=Path(sys.argv[1])
rows=[];profiles=[];checks=[];work=[]
for graph in ('arxiv','products'):
    text=(run/f'{graph}.log').read_text(encoding='utf-8')
    if 'ICPP_HEADS_COMPLETE ' not in text:raise SystemExit(f'incomplete graph: {graph}')
    groups={}
    for line in text.splitlines():
        if line.startswith('{"path"'):
            r=json.loads(line);r['graph']=graph;groups.setdefault((r['path'],r['layer']),[]).append(r)
        elif line.startswith(('PROFILE ','HEAD_WORK ','ONLINE_STATS ','CHECK ')):
            r={'graph':graph,'record':line.split()[0]}
            for item in line.split()[1:]:
                k,v=item.split('=',1);r[k]=v
            (profiles if r['record']=='PROFILE' else checks if r['record']=='CHECK' else work).append(r)
    for (path,layer),rep in groups.items():
        if len(rep)!=3:raise SystemExit(f'{graph} {path} L{layer}: expected3reps')
        r={'graph':graph,'path':path,'layer':layer,'repeats':len(rep)}
        for key in rep[0]:
            if key.endswith('_ms'):r[key]=statistics.median(x[key] for x in rep)
        r['e2e_min_ms']=min(x['e2e_ms'] for x in rep);r['e2e_max_ms']=max(x['e2e_ms'] for x in rep)
        rows.append(r)
    baseline={r['path']:r['e2e_ms'] for r in rows if r['graph']==graph and r['layer']==3}
    for r in rows:
        if r['graph']==graph:
            r['speedup_vs_B0_BF16']=baseline['B0_BF16']/r['e2e_ms']
            r['speedup_vs_B1_PREMAX']=baseline['B1_TFS_PREMAX']/r['e2e_ms']
            r['speedup_vs_BLOCK_B32']=baseline['ICPP_TFS_BLOCK_B32']/r['e2e_ms']
            r['speedup_vs_PAIR_B32']=baseline['ICPP_TFS_PAIR_B32']/r['e2e_ms']
def save(name,data):
    keys=list(dict.fromkeys(k for r in data for k in r))
    with (run/name).open('w',encoding='utf-8',newline='') as f:
        w=csv.DictWriter(f,fieldnames=keys,delimiter='\t');w.writeheader();w.writerows(data)
save('timing_summary.tsv',rows);save('profiles.tsv',profiles);save('checks.tsv',checks);save('work.tsv',work)
for graph in ('arxiv','products'):
    for r in rows:
        if r['graph']==graph and r['layer']==3:print(graph,r['path'],f"E2E={r['e2e_ms']:.3f}ms",f"vsB0={r['speedup_vs_B0_BF16']:.4f}x",f"vsB1={r['speedup_vs_B1_PREMAX']:.4f}x",f"vsBlock={r['speedup_vs_BLOCK_B32']:.4f}x")
