import pathlib,sys,json,csv,shlex,subprocess,time
from evidence_utils import ROOT,save,sha,invoke,fixtures,csvfile
run,build=map(pathlib.Path,sys.argv[1:3]);mode=sys.argv[3]
names=['ogbn-products','reddit','mycielskian19','roadNet-CA','wiki-Talk']
graphs=fixtures(run) if mode=='smoke' else [r for r in json.loads((ROOT/'docs/CURRENT_GRAPH_INVENTORY_20261004.json').read_text()) if r['graph'] in names]
save(run/'graph_list.json',graphs);states=[];alltables={}
for g in graphs:
    folder=run/g['graph'];folder.mkdir();state=dict(g);t=time.time()
    try:
        print('D2_START',g['graph'],flush=True);path=pathlib.Path(g['path']);state['actual_sha256']=sha(path)
        if 'input_sha256' in g:assert state['actual_sha256']==g['input_sha256']
        log=invoke(folder,'residual',[build/'residual_methods',path.parent.parent,g['graph']]);tables={}
        for line in log.splitlines():
            label,_,body=line.partition(' ')
            if not (label.startswith('D2_') or label.startswith('METHOD_PMU')):continue
            tables.setdefault(label,[]).append(dict(x.split('=',1) for x in shlex.split(body) if '=' in x))
        assert len(tables['D2_COMPLETE'])==1 and tables['D2_COMPLETE'][0]['pass']=='1'
        assert len(tables['D2_CHECK'])==20 and len(tables['D2_ACCEPT'])==5
        assert all(r['pass']=='1' for r in tables.get('D2_PROFILE_GATE',[]))
        accepted={r['method'] for r in tables['D2_ACCEPT'] if r['accepted']=='1'}
        assert {r['method'] for r in tables['D2_TIME']}==accepted
        for m in accepted:assert {r['repeat'] for r in tables['D2_TIME'] if r['method']==m}==set(map(str,range(5)))
        for key,rows in tables.items():csvfile(folder/(key.lower()+'.csv'),rows);alltables.setdefault(key,[]).extend(rows)
        save(folder/'parsed.json',tables);state.update(status='COMPLETE',accepted=sorted(accepted),rejected=sorted({r['method'] for r in tables['D2_ACCEPT']}-accepted),wall_seconds=time.time()-t)
        print('D2_DONE',g['graph'],state['rejected'],flush=True)
    except Exception as e:state.update(status='FAILED',reason=str(e));print('D2_FAILED',g['graph'],str(e),flush=True)
    states.append(state);save(run/'suite_status.json',states)
for key,rows in alltables.items():csvfile(run/(key.lower()+'.csv'),rows)
completion=dict(mode=mode,graphs=len(states),failed=sum(r['status']=='FAILED' for r in states),checks=len(alltables.get('D2_CHECK',[])),rejections=[dict(graph=r['graph'],methods=r.get('rejected',[])) for r in states])
save(run/'completion.json',completion);print('D2_SUITE_COMPLETE',json.dumps(completion),flush=True)
sys.exit(2 if completion['failed'] else 0)
