"""Summarize historical/current bridge logs without mixing timing boundaries."""
import json
import statistics
import sys
from pathlib import Path
root=Path(sys.argv[1])
report={}
for dataset in ('arxiv','products'):
    result={}
    f=root/f'{dataset}_bridge.log'
    if f.exists():
        rows=[json.loads(s) for s in f.read_text().splitlines() if s.startswith('{')]
        paths=sorted({r['path'] for r in rows})
        bridge={p:{k:statistics.median(r[k] for r in rows if r['path']==p)
            for k in ('total_ms','layer1_ms','layer2_ms','layer3_ms')} for p in paths}
        result['allocation_inclusive_bridge']=bridge
        if 'old_reference' in bridge and 'old_tfs_local_u' in bridge:
            result['old_tfs_speedup_vs_old_reference']=bridge['old_reference']['total_ms']/bridge['old_tfs_local_u']['total_ms']
        if 'old_panel_transform_first' in bridge and 'old_tfs_local_u' in bridge:
            result['old_tfs_speedup_vs_old_parallel_control']=bridge['old_panel_transform_first']['total_ms']/bridge['old_tfs_local_u']['total_ms']
    production={}
    for path in ('standard_fp32','standard_bf16','tfs_bf16'):
        f=root/f'{dataset}_{path}.log'
        if not f.exists():continue
        rows=[json.loads(s) for s in f.read_text().splitlines() if s.startswith('{')]
        e2e=[float(dict(v.split('=',1) for v in s.split()[1:])['total_ms']) for s in f.read_text().splitlines() if s.startswith('E2E ')]
        if not e2e:continue
        production[path]={'total_ms':statistics.median(e2e),'layer2':{k:statistics.median(r[k] for r in rows if r['layer']==2)
            for k in ('projection_ms','convert_ms','lr_ms','score_exp_aggregate_ms','normalize_ms','activation_ms','total_ms')}}
    result['preallocated_production']=production
    report[dataset]=result
print(json.dumps(report,indent=2))
