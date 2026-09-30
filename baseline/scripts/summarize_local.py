"""Summarize matched speed runs; sampled worker times are not wall fractions."""
import json
import statistics as st
import sys
from pathlib import Path
root=Path(sys.argv[1])
result={}
for dataset in ('arxiv','products'):
    path=root/f'{dataset}_local.log'
    if not path.exists():continue
    lines=path.read_text().splitlines()
    rows=[json.loads(s) for s in lines if s.startswith('{')]
    def kv(line):return dict(part.split('=',1) for part in line.split()[1:] if '=' in part)
    checks=[kv(s) for s in lines if s.startswith('CHECK ')]
    profiles=[kv(s) for s in lines if s.startswith('PROFILE ')]
    counters=[]
    active_profile=None
    for s in lines:
        if s.startswith('PROFILE '):active_profile=kv(s)['path']
        if s.startswith('ONLINE_STATS '):counters.append({'path':active_profile,**kv(s)})
    paths={}
    for name in sorted({r['path'] for r in rows}):
        a=[r['e2e_ms'] for r in rows if r['path']==name and r['layer']==1]
        perlayer={}
        for layer in (1,2,3):
            rs=[r for r in rows if r['path']==name and r['layer']==layer]
            perlayer[layer]={k:st.median(r[k] for r in rs) for k in rs[0] if k.endswith('_ms') and k!='e2e_ms'}
        paths[name]={'median_e2e_ms':st.median(a),'min_e2e_ms':min(a),'max_e2e_ms':max(a),'measured_repeats':len(a),'layers':perlayer}
    for p in profiles:
        fields=[k for k in p if k.endswith('_worker_ms')]
        total=sum(float(p[k]) for k in fields)
        p['raw_sample_worker_share_pct']={k:100*float(p[k])/total for k in fields} if total else {}
        speed_layer=paths[p['path']]['layers'][int(p['layer'])]['layer_ms']
        p['profile_plus_counters_change_vs_speed_pct']=100*(float(p['layer_wall_ms'])/speed_layer-1)
    speedups={name:{base:paths[base]['median_e2e_ms']/v['median_e2e_ms'] for base in ('B0_FP32','B0_BF16','B1_TFS')} for name,v in paths.items() if name.startswith('local_')}
    result[dataset]={'complete':any(s.startswith('LOCAL_COMPLETE') for s in lines),'paths':paths,'speedup_control_div_candidate':speedups,'errors_vs_B0_FP32':checks,'sample_profiles':profiles,'online_statistics':counters}
encoded=json.dumps(result,ensure_ascii=False,indent=2)+'\n'
if len(sys.argv)>2:Path(sys.argv[2]).write_bytes(encoded.encode('utf-8'))
else:print(encoded,end='')
