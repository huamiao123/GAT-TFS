#!/usr/bin/env python3
import pathlib,json,sys,os,shlex,statistics,math,time,subprocess
from evidence_utils import ROOT,save,sha,csvfile,invoke,baseline,fixtures
PARTS={'focus':['ogbn-products','reddit','mycielskian19','roadNet-CA','wiki-Talk'],'validation':['hollywood-2009','indochina-2004','soc-Pokec','cit-Patents','soc-LiveJournal1','com-LiveJournal','as-Skitter','rgg_n_2_24_s0','web-Google','email-Enron','amazon0601','com-Youtube']}
def parse(folder,log):
    tables={}
    for line in log.splitlines():
        label,_,body=line.partition(' ')
        if not (label.startswith('METHOD_') or label.startswith('GROUP_')):continue
        key=label.replace('METHOD_','',1) if label.startswith('METHOD_') else label
        tables.setdefault(key,[]).append(dict(item.split('=',1) for item in shlex.split(body) if '=' in item))
    for key,rows in tables.items():csvfile(folder/(key.lower()+'.csv'),rows)
    save(folder/'parsed.json',tables)
    assert len(tables['COMPLETE'])==1 and tables['COMPLETE'][0]['pass']=='1'
    methods=int(tables['COMPLETE'][0]['methods']);assert methods==11
    assert len(tables['CHECK'])==44 and all(r['pass']=='1' for r in tables['CHECK'])
    assert len(tables['EQUIV'])==24 and all(r['pass']==r['bitwise']=='1' for r in tables['EQUIV'])
    assert len(tables['PROFILE_GATE'])==16 and all(r['pass']==r['bitwise']=='1' for r in tables['PROFILE_GATE'])
    assert len(tables['GROUP_WORK_GATE'])==16 and all(r['pass']=='1' for r in tables['GROUP_WORK_GATE'])
    assert len(tables['GROUP_AUDIT'])==3 and all(r['pass']=='1' for r in tables['GROUP_AUDIT'])
    config=tables['CONFIG'][0]
    assert all(config[k]==v for k,v in {'D':'128','F':'128','TR':'16','R':'64','numa':'default','grouping_segment':'4096','signature_neighbors':'64'}.items())
    names={r['method'] for r in tables['CHECK']}|{'source_mkl_fp32'}
    groups={}
    for r in tables['TIME']:
        assert r['method'] in names and r['kind'] in ('consecutive_e2e','alternating_e2e')
        value=float(r['ms']);assert math.isfinite(value) and value>0
        groups.setdefault((r['method'],r['kind']),[]).append(r)
    assert set(groups)=={(m,k) for m in names for k in ('consecutive_e2e','alternating_e2e')}
    summaries=[]
    for (m,k),rows in groups.items():
        assert len(rows)==5 and {int(r['repeat']) for r in rows}==set(range(5))
        vs=[float(r['ms']) for r in rows];mean=statistics.mean(vs)
        summaries.append(dict(graph=folder.name,method=m,kind=k,median_ms=statistics.median(vs),min_ms=min(vs),max_ms=max(vs),cv=statistics.pstdev(vs)/mean,p25_ms=sorted(vs)[1],p75_ms=sorted(vs)[3]))
    lookup={(r['method'],r['kind']):r for r in summaries}
    for r in summaries:
        r['speedup_vs_tfs']=lookup['paper_tfs',r['kind']]['median_ms']/r['median_ms']
        r['speedup_vs_mkl']=lookup['source_mkl_fp32',r['kind']]['median_ms']/r['median_ms']
        if '_q' in r['method']:
            base=r['method'].split('_q',1)[0]+'_degree_fast';r['speedup_vs_degree']=lookup[base,r['kind']]['median_ms']/r['median_ms']
    csvfile(folder/'summary.csv',summaries);return summaries,tables
def main():
    run,build=map(pathlib.Path,sys.argv[1:3]);mode=sys.argv[3]
    if mode=='smoke':graphs=fixtures(run)
    elif mode=='synthetic':
        synthetic=run/'synthetic';t=time.time();log=invoke(run,'generate_synthetic',[build/'generate_synthetic',synthetic]);graphs=[]
        for line in log.splitlines():
            if line.startswith('SYNTHETIC '):
                d=dict(x.split('=',1) for x in line.split()[1:]);d['path']=str(synthetic/d['graph']/(d['graph']+'.csrbin'));graphs.append(d)
        assert len(graphs)==3;save(run/'synthetic_generation.json',dict(wall_seconds=time.time()-t,graphs=graphs))
    else:
        inv={r['graph']:r for r in json.loads((ROOT/'docs/CURRENT_GRAPH_INVENTORY_20261004.json').read_text())};graphs=[inv[n] for n in PARTS[mode]]
    save(run/'graph_list.json',graphs);states=[];summaries=[];checks=[];eq=[]
    for graph in graphs:
        folder=run/graph['graph'];folder.mkdir();state=dict(graph);t=time.time()
        try:
            print('P1_START',graph['graph'],flush=True);path=pathlib.Path(graph['path']);state['actual_input_sha256']=sha(path)
            if 'input_sha256' in graph:assert state['actual_input_sha256']==graph['input_sha256']
            anchor=baseline(invoke(folder,'original_raw',[build/'paper_original',path.parent.parent,graph['graph']]))
            log=invoke(folder,'methods',[build/'projection_methods',path.parent.parent,graph['graph']]);rows,tables=parse(folder,log)
            if mode=='synthetic':assert tables['GROUP_NATIVE_PERM'][0]['fnv64']==graph['native_perm_fnv64'],'Native DegreeSort differs from generator'
            state.update(status='PASS',checks=len(tables['CHECK']),equiv_checks=len(tables['EQUIV']),profile_checks=len(tables['PROFILE_GATE']),anchor=anchor,wall_seconds=time.time()-t)
            summaries.extend(rows);checks.extend(tables['CHECK']);eq.extend(tables['EQUIV']);print('P1_PASS',graph['graph'],json.dumps([r for r in rows if r['kind']=='consecutive_e2e']),flush=True)
        except Exception as exc:
            state.update(status='FAILED',reason=str(exc),wall_seconds=time.time()-t);print('P1_FAILED',graph['graph'],str(exc),flush=True)
        states.append(state);save(folder/'status.json',state);save(run/'suite_status.json',states)
        csvfile(run/'summary.csv',summaries);csvfile(run/'all_checks.csv',checks);csvfile(run/'all_equiv.csv',eq)
    completion=dict(mode=mode,passed=sum(r['status']=='PASS' for r in states),failed=sum(r['status']!='PASS' for r in states),checks=len(checks),equiv_checks=len(eq))
    save(run/'completion.json',completion);print('P1_COMPLETE',json.dumps(completion),flush=True);return bool(completion['failed'])*2
if __name__=='__main__':sys.exit(main())
