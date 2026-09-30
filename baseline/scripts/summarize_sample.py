"""Report sampling perturbation and coarse sampled worker shares, not wall shares."""
import json
import statistics
import sys
from pathlib import Path
root=Path(sys.argv[1])
report={}
fields={'schedule':'schedule_sample_worker_ms','score':'score_sample_worker_ms',
        'staging':'staging_sample_worker_ms','matrix':'matrix_sample_worker_ms','output':'output_sample_worker_ms'}
for dataset in ('arxiv','products'):
    f=root/f'{dataset}_sample.log'
    if not f.exists():continue
    lines=f.read_text().splitlines()
    rows=[json.loads(s) for s in lines if s.startswith('{')]
    calibration=next((s for s in lines if s.startswith('CLOCK_FLOOR ')),None)
    floor=float(dict(p.split('=',1) for p in calibration.split()[1:])['median_ns'])/1e6 if calibration else 0
    result={'clock_floor_ms':floor,'complete':any(s.startswith('SAMPLE_COMPLETE') for s in lines)}
    totals={p:statistics.median(r['e2e_ms'] for r in rows if r['period']==p and r['layer']==1) for p in (-1,0,128,512) if any(r['period']==p for r in rows)}
    result['e2e_median_ms']=totals
    if -1 in totals: result['overhead_vs_original_pct']={p:100*(v/totals[-1]-1) for p,v in totals.items() if p!=-1}
    for layer in (1,2,3):
        originals=[r for r in rows if r['period']==-1 and r['layer']==layer]
        layer_result={'original_layer_median_ms':statistics.median(r['layer_ms'] for r in originals),
                      'original_kernel_median_ms':statistics.median(r['kernel_ms'] for r in originals)} if originals else {}
        for period in (128,512):
            rs=[r for r in rows if r['period']==period and r['layer']==layer]
            if not rs:continue
            adjusted=[];raw=[]
            for r in rs:
                counts={'schedule':r['sampled_steps']+r['sampled_tiles'],'score':r['sampled_steps'],
                        'staging':r['sampled_steps'],'matrix':r['sampled_steps'],'output':r['sampled_tiles']}
                a={k:max(0,r[v]-floor*counts[k]) for k,v in fields.items()}
                b={k:r[v] for k,v in fields.items()}
                if sum(a.values())==0 or sum(b.values())==0:continue
                adjusted.append({k:100*v/sum(a.values()) for k,v in a.items()})
                raw.append({k:100*v/sum(b.values()) for k,v in b.items()})
            if not adjusted:continue
            layer_result[period]={'kernel_median_ms':statistics.median(r['kernel_ms'] for r in rs),
                'sampled_tiles':[r['sampled_tiles'] for r in rs],
                'sampled_steps':[r['sampled_steps'] for r in rs],
                'raw_worker_share_pct':{k:statistics.median(r[k] for r in raw) for k in fields},
                'floor_adjusted_worker_share_pct':{k:statistics.median(r[k] for r in adjusted) for k in fields},
                'share_range_pct':{k:[min(r[k] for r in adjusted),max(r[k] for r in adjusted)] for k in fields}}
        result[f'layer{layer}']=layer_result
    report[dataset]=result
encoded=json.dumps(report,indent=2)+'\n'
if len(sys.argv)>2:Path(sys.argv[2]).write_bytes(encoded.encode('utf-8'))
else:print(encoded,end='')
