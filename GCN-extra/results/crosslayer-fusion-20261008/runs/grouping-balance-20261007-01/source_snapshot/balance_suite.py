#!/usr/bin/env python3
import pathlib,sys,json,shlex,statistics,math,time
from evidence_utils import ROOT,save,sha,invoke,fixtures,csvfile,baseline
run,build=map(pathlib.Path,sys.argv[1:3]);study,mode=sys.argv[3:5]
if mode=='smoke': graphs=fixtures(run)
elif mode=='synthetic':
    text=invoke(run,'generate_synthetic',[build/'generate_synthetic',run/'synthetic']);graphs=[]
    for line in text.splitlines():
        if line.startswith('SYNTHETIC '):
            d=dict(x.split('=',1) for x in line.split()[1:]);d['path']=str(run/'synthetic'/d['graph']/(d['graph']+'.csrbin'));graphs.append(d)
    assert len(graphs)==3
else:
    inv={r['graph']:r for r in json.loads((ROOT/'docs/CURRENT_GRAPH_INVENTORY_20261004.json').read_text())}
    graphs=[inv[n] for n in mode.split(',')]
save(run/'graph_list.json',graphs);states=[];alltables={}
for graph in graphs:
    folder=run/graph['graph'];folder.mkdir();state=dict(graph);start=time.time();tables={}
    try:
        path=pathlib.Path(graph['path']);state['actual_input_sha256']=sha(path)
        if 'input_sha256' in graph: assert state['actual_input_sha256']==graph['input_sha256']
        print('BALANCE_START',study,graph['graph'],flush=True)
        if study=='grouping':
            state['anchor']=baseline(invoke(folder,'original_raw',[build/'paper_original',path.parent.parent,graph['graph']]))
            program=build/'projection_methods'
        else: program=build/'selective_shape'
        log=invoke(folder,'methods',[program,path.parent.parent,graph['graph']])
        for line in log.splitlines():
            label,_,body=line.partition(' ')
            if label.startswith(('METHOD_','GROUP_','ORDER_','SEL_')):
                tables.setdefault(label,[]).append(dict(x.split('=',1) for x in shlex.split(body) if '=' in x))
        for key,rows in tables.items(): csvfile(folder/(key.lower()+'.csv'),rows)
        save(folder/'parsed.json',tables)
        if study=='grouping':
            assert len(tables['METHOD_COMPLETE'])==1 and tables['METHOD_COMPLETE'][0]['pass']=='1'
            for key,count in [('METHOD_CHECK',76),('METHOD_EQUIV',45),('METHOD_PROFILE_GATE',30),('GROUP_WORK_GATE',30)]:
                assert len(tables[key])==count,(key,len(tables.get(key,[])))
                assert all(r['pass']=='1' for r in tables[key]),key
            assert all(r['bitwise']=='1' for r in tables['METHOD_EQUIV']+tables['METHOD_PROFILE_GATE'])
            assert len(tables['METHOD_PLAN'])==20
            names={r['method'] for r in tables['METHOD_PLAN']}
            samples={}
            for r in tables['METHOD_TIME']:
                assert r['method'] in names and math.isfinite(float(r['ms'])) and float(r['ms'])>0
                samples.setdefault((r['method'],r['kind']),[]).append(r)
            assert len(samples)==40
            for (method,kind),rows in samples.items():
                repeats=5 if kind=='consecutive_e2e' else 10
                assert len(rows)==repeats and {int(r['repeat']) for r in rows}==set(range(repeats))
            state['checks']=len(tables['METHOD_CHECK']);state['equiv']=len(tables['METHOD_EQUIV'])
        else:
            assert len(tables['ORDER_COMPLETE'])==2 and all(r['pass']=='1' for r in tables['ORDER_COMPLETE'])
            assert all(r['pass']=='1' for r in tables.get('ORDER_EQUIV',[]))
            for shape in ['128_32_32','128_128_128']:
                accepts=[r for r in tables['ORDER_ACCEPT'] if r['shape']==shape]
                assert len(accepts)==18
                accepted={r['method'] for r in accepts if r['accepted']=='1'}
                assert 'A_A_degree' in accepted
                samples={}
                for r in tables['ORDER_TIME']:
                    if r['shape']!=shape: continue
                    assert r['method'] in accepted or 'mkl' in r['method'].lower()
                    assert math.isfinite(float(r['ms'])) and float(r['ms'])>0
                    samples.setdefault((r['method'],r['kind']),[]).append(r)
                for (method,kind),rows in samples.items():
                    repeats=5 if kind=='consecutive' else 10
                    assert len(rows)==repeats,(method,kind,len(rows))
                for method in accepted: assert (method,'consecutive') in samples and (method,'paired') in samples
            state['checks']=len(tables.get('ORDER_CHECK',[]));state['rejections']=[r for r in tables['ORDER_ACCEPT'] if r['accepted']=='0']
        state['status']='PASS';print('BALANCE_PASS',graph['graph'],flush=True)
    except Exception as exc:
        state.update(status='FAILED',reason=repr(exc));print('BALANCE_FAILED',graph['graph'],repr(exc),flush=True)
    state['wall_seconds']=time.time()-start
    for key,rows in tables.items(): alltables.setdefault(key,[]).extend(rows)
    states.append(state);save(folder/'status.json',state);save(run/'suite_status.json',states)
    for key,rows in alltables.items(): csvfile(run/(key.lower()+'.csv'),rows)
completion=dict(study=study,mode=mode,graphs=len(states),passed=sum(r['status']=='PASS' for r in states),failed=sum(r['status']!='PASS' for r in states),checks=sum(r.get('checks',0) for r in states))
save(run/'completion.json',completion);print('BALANCE_COMPLETE',json.dumps(completion),flush=True)
sys.exit(2 if completion['failed'] else 0)
