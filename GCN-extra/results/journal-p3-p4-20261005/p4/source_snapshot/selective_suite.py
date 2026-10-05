import pathlib,sys,json,csv,shlex,struct,random
from evidence_utils import ROOT,save,sha,invoke,fixtures,csvfile
run,build=map(pathlib.Path,sys.argv[1:3]);mode=sys.argv[3]
def controlled():
    graphs=[];n=65536;degree=64;half=n//2;order=list(range(half));random.Random(20261005).shuffle(order)
    for tag,pool in [('bipartite_hot',512),('bipartite_spread',half)]:
        folder=run/'controlled'/tag;folder.mkdir(parents=True);path=folder/(tag+'.csrbin');row=[i*degree for i in range(n+1)];col=[]
        for i in range(n):
            offset=half if i<half else 0
            col.extend(offset+order[(i*degree+k)%pool] for k in range(degree))
        with path.open('wb') as f:
            f.write(struct.pack('<IIIQQQ',0,0,2,n,n,len(col)))
            f.write(struct.pack('<'+'I'*len(row),*row));f.write(struct.pack('<'+'I'*len(col),*col));f.write(struct.pack('<'+'f'*len(col),*([1.]*len(col))))
        graphs.append(dict(graph=tag,path=str(path),N=n,E=len(col),degree=degree,source_pool_each_side=pool,description='two-direction bipartite relation; same node/degree/edge count, no self-loops; not necessarily symmetric'))
    return graphs
if mode=='smoke':graphs=fixtures(run)
elif mode=='controlled':graphs=controlled()
else:graphs=[r for r in json.loads((ROOT/'docs/CURRENT_GRAPH_INVENTORY_20261004.json').read_text()) if r['graph'] in ['ogbn-products','reddit','mycielskian19','roadNet-CA','wiki-Talk']]
save(run/'graph_list.json',graphs);states=[];alltables={}
for graph in graphs:
    folder=run/graph['graph'];folder.mkdir();state=dict(graph)
    try:
        path=pathlib.Path(graph['path']);state['actual_sha256']=sha(path)
        if 'input_sha256' in graph:assert state['actual_sha256']==graph['input_sha256']
        print('SEL_START',graph['graph'],flush=True);log=invoke(folder,'selective',[build/'selective_shape',path.parent.parent,graph['graph']]);tables={}
        for line in log.splitlines():
            label,_,body=line.partition(' ')
            if not (label.startswith('SEL_') or label.startswith('METHOD_PMU')):continue
            tables.setdefault(label,[]).append(dict(x.split('=',1) for x in shlex.split(body) if '=' in x))
        assert len(tables['SEL_COMPLETE'])==2 and all(r['pass']=='1' for r in tables['SEL_COMPLETE'])
        assert len(tables['SEL_CHECK'])==54 and len(tables['SEL_ACCEPT'])==18
        assert all(r['pass']=='1' for r in tables['SEL_EQUIV']+tables.get('SEL_PROFILE_GATE',[])+tables.get('SEL_WORK',[]))
        for shape in ['128_32_32','128_128_128']:
            accepted={r['method'] for r in tables['SEL_ACCEPT'] if r['shape']==shape and r['accepted']=='1'}
            assert {r['method'] for r in tables['SEL_TIME'] if r['shape']==shape}==accepted|{'shape_mkl_fp32_original_style'}
            for method in accepted:assert len([r for r in tables['SEL_TIME'] if r['shape']==shape and r['method']==method])==5
        for key,rows in tables.items():csvfile(folder/(key.lower()+'.csv'),rows);alltables.setdefault(key,[]).extend(rows)
        save(folder/'parsed.json',tables);state.update(status='COMPLETE',rejected=[r for r in tables['SEL_ACCEPT'] if r['accepted']=='0'])
        print('SEL_DONE',graph['graph'],'rejections',len(state['rejected']),flush=True)
    except Exception as e:state.update(status='FAILED',reason=str(e));print('SEL_FAILED',graph['graph'],str(e),flush=True)
    states.append(state);save(run/'suite_status.json',states)
for key,rows in alltables.items():csvfile(run/(key.lower()+'.csv'),rows)
completion=dict(mode=mode,graphs=len(states),failed=sum(r['status']=='FAILED' for r in states),checks=len(alltables.get('SEL_CHECK',[])))
save(run/'completion.json',completion);print('SEL_SUITE_COMPLETE',json.dumps(completion),flush=True);sys.exit(2 if completion['failed'] else 0)
