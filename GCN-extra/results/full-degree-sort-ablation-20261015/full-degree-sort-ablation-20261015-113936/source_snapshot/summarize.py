#!/usr/bin/env python3
import csv,json,pathlib,statistics,sys
run=pathlib.Path(sys.argv[1]);records=[]
for file in sorted((run/'logs').glob('*.out')):
    graph=file.stem;rows=[];checks=[];work=[];setup=[];stages=[]
    for line in file.read_text(errors='replace').splitlines():
        if not line.startswith('ABLATION_'):continue
        tag,_,body=line.partition(' ')
        kv=dict(part.split('=',1) for part in body.split() if '=' in part)
        if tag=='ABLATION_TIME':rows.append(kv)
        elif tag=='ABLATION_CHECK':checks.append(kv)
        elif tag=='ABLATION_WORK':work.append(kv)
        elif tag=='ABLATION_SETUP':setup.append(kv)
        elif tag=='ABLATION_STAGE':stages.append(kv)
    assert len(checks)==1 and checks[0]['pass']=='1' and len(rows)==16 and len(work)==2 and len(setup)==1 and len(stages)==6,(graph,len(checks),len(rows),len(work),len(stages))
    assert 'ABLATION_COMPLETE graph='+graph+' pass=1' in file.read_text(errors='replace')
    by={k:[float(r['prepared_e2e_ms']) for r in rows if r['schedule']==k] for k in ('degree','identity')}
    assert len(by['degree'])==len(by['identity'])==8
    summary={k:dict(median_ms=statistics.median(v),min_ms=min(v),max_ms=max(v),cv=statistics.pstdev(v)/statistics.mean(v)) for k,v in by.items()}
    summary['speedup_degree_over_identity']=summary['degree']['median_ms']/summary['identity']['median_ms']
    records.append(dict(graph=graph,summary=summary,checks=checks[0],work=work,setup=setup[0],stages=stages,times=rows))
(run/'results.json').write_text(json.dumps(records,indent=2)+'\n')
with (run/'summary.csv').open('w',newline='') as f:
    w=csv.writer(f);w.writerow(['graph','degree_median_ms','identity_median_ms','degree_over_identity','degree_sort_ms','identity_init_ms','degree_nonempty_tiles','identity_nonempty_tiles','degree_padded_steps','identity_padded_steps','bitwise'])
    for x in records:
        s=x['summary'];work={r['schedule']:r for r in x['work']};setup=x['setup']
        w.writerow([x['graph'],s['degree']['median_ms'],s['identity']['median_ms'],s['speedup_degree_over_identity'],setup['degree_sort_ms'],setup['identity_init_ms'],work['degree']['nonempty_tiles'],work['identity']['nonempty_tiles'],work['degree']['padded_row_steps'],work['identity']['padded_row_steps'],x['checks']['pass']])
print((run/'summary.csv').read_text())
