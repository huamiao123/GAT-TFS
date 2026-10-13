#!/usr/bin/env python3
import pathlib,sys,json,shlex,statistics,math,time,os,struct
from evidence_utils import ROOT,save,sha,invoke,fixtures,csvfile,baseline
run,build=map(pathlib.Path,sys.argv[1:3]);mode=sys.argv[3]
if mode=='smoke':
 graphs=fixtures(run)
 for n in [1,15,16,17,31,33,65]:
  name='state_tail_'+str(n);folder=run/'fixtures'/name;folder.mkdir();row=[0];col=[]
  degs=[0,1,16,32,63,64,65,127,128,255,256,257,512]
  for i in range(n):
   d=degs[(i+1)%len(degs)] if i%4!=0 or n==1 else 0
   col.extend((i*3+j*7)%n for j in range(d));row.append(len(col))
  f=folder/(name+'.csrbin')
  with f.open('wb') as o:
   o.write(struct.pack('<IIIQQQ',0,0,2,n,n,len(col)));o.write(struct.pack('<'+'I'*len(row),*row));o.write(struct.pack('<'+'I'*len(col),*col));o.write(struct.pack('<'+'f'*len(col),*([1.]*len(col))))
  graphs.append(dict(graph=name,path=str(f),N=n,E=len(col),fixture='empty_tail_unsorted_duplicate'))
else:
 inv={r['graph']:r for r in json.loads((ROOT/'docs/CURRENT_GRAPH_INVENTORY_20261004.json').read_text())};graphs=[inv[n] for n in mode.split(',')]
os.environ['STATE_SMOKE']='1' if mode=='smoke' else '0'
focus={'mycielskian19','reddit','roadNet-CA','amazon0601'}
save(run/'graph_list.json',graphs);states=[];alltables={};summary=[]
for graph in graphs:
 folder=run/graph['graph'];folder.mkdir();state=dict(graph);start=time.time();tables={}
 os.environ['STATE_ABLATIONS']='1' if mode=='smoke' or graph['graph'] in focus else '0'
 os.environ['STATE_PMU']='1' if graph['graph'] in focus else '0'
 error=None
 try:
  path=pathlib.Path(graph['path']);state['actual_input_sha256']=sha(path)
  if 'input_sha256' in graph:assert state['actual_input_sha256']==graph['input_sha256']
  print('STATE_START',graph['graph'],flush=True)
  if mode!='smoke':state['anchor']=baseline(invoke(folder,'original_raw',[build/'paper_original',path.parent.parent,graph['graph']]))
  invoke(folder,'methods',[build/'state_methods',path.parent.parent,graph['graph']])
 except Exception as e:error=e
 log=(folder/'methods.log').read_text(errors='replace') if (folder/'methods.log').exists() else ''
 for line in log.splitlines():
  label,_,body=line.partition(' ')
  if label.startswith(('METHOD_','GROUP_','STATE_')):tables.setdefault(label,[]).append(dict(x.split('=',1) for x in shlex.split(body) if '=' in x))
 for key,rows in tables.items():csvfile(folder/(key.lower()+'.csv'),rows)
 save(folder/'parsed.json',tables)
 try:
  if error:raise error
  assert len(tables.get('STATE_COMPLETE',[]))==1 and tables['STATE_COMPLETE'][0]['pass']=='1'
  assert len(tables.get('STATE_SEMANTIC',[]))==1 and tables['STATE_SEMANTIC'][0]['pass']=='1'
  cfg=tables['STATE_CONFIG'][0];assert all(cfg[k]=='128' for k in ['D','H','F']);assert cfg['numa']=='default'
  plans=tables['STATE_PLAN'];assert len(plans)==int(cfg['methods']);names={x['method'] for x in plans};assert len(names)==len(plans)
  for plan in plans:
   name=plan['method'];checks=[x for x in tables['METHOD_CHECK'] if x['method']==name]
   assert len(checks)==4 and all(x['pass']=='1' for x in checks),(name,'numeric')
   assert {(x['boundary'],x['reference']) for x in checks}=={('kernel_FP32','paper_tfs'),('e2e_FP32_final','paper_tfs'),('e2e_BF16_final','paper_tfs'),('e2e_BF16_final','source_MKL_FP32')}
   if int(plan['kind'])>=2:
    eq=[x for x in tables['STATE_EQUIV'] if x['method']==name]
    assert len(eq)==4 and {x['boundary'] for x in eq}=={'kernel_FP32','e2e_FP32','H1_BF16','e2e_BF16'} and all(x['bitwise']=='1' and x['pass']=='1' for x in eq),(name,'bitwise')
   if int(plan['kind'])==3:
    prof=[x for x in tables['STATE_PROFILE_GATE'] if x['method']==name]
    assert len(prof)==2 and {x['layer'] for x in prof}=={'1','2'} and all(x['pass']=='1' for x in prof)
  assert all(x['pass']=='1' for x in tables['METHOD_CHECK']+tables['STATE_EQUIV'])
  if mode=='smoke':
   extra=[x for x in tables['STATE_EQUIV'] if x['method'].startswith('smoke_')];assert len(extra)==42*4
   stress=[x for x in tables['STATE_EQUIV'] if x['method'].startswith('stress_')];assert len(stress)==18*4
  assert all(x['pass']=='1' for x in tables.get('STATE_WORK_GATE',[]))
  work=tables['STATE_STRUCTURE'];assert len(work)==6
  worklookup={(x['schedule'],x['scope']):x for x in work}
  for plan in plans:
   if int(plan['kind'])!=3:continue
   cs=[x for x in tables['STATE_COUNTERS'] if x['method']==plan['method']];wg=[x for x in tables['STATE_WORK_GATE'] if x['method']==plan['method']]
   assert len(cs)==len(wg)==2 and {x['layer'] for x in cs}=={'1','2'} and {x['layer'] for x in wg}=={'1','2'}
   for c in cs:
    w=worklookup[plan['schedule'],plan['scope']];assert c['source_visits']==w['edges'] and c['amx_tdp']==w['amx_tdp_fast'] and c['prefetch_requests']==w['prefetch_requests']
    if int(plan['pair'])>0:assert c['shared_heads']==w['shared_heads']
    assert c['decode_rows']==(w['shared_decode_rows'] if plan['pair']=='2' else w['edges'])
  for x in work:
   e=int(x['edges']);c=int(x['shared_heads']);assert e==int(cfg['E']) and int(x['scalar_adds'])==e*128 and int(x['merge_steps'])==e-c and int(x['shared_decode_rows'])==e-c
  if mode!='smoke':
   times={}
   for x in tables['STATE_TIME']:
    assert x['method'] in names|{'source_mkl_fp32'} and math.isfinite(float(x['ms'])) and float(x['ms'])>0
    times.setdefault((x['method'],x['kind']),[]).append(x)
   assert set(times)=={(m,k) for m in names|{'source_mkl_fp32'} for k in ['consecutive','alternating']}
   for (m,k),rows in times.items():
    nr=5 if k=='consecutive' else 10;assert len(rows)==nr and {int(x['repeat']) for x in rows}==set(range(nr))
    vals=[float(x['ms']) for x in rows];summary.append(dict(graph=graph['graph'],method=m,kind=k,median_ms=statistics.median(vals),min_ms=min(vals),max_ms=max(vals),repeats=nr))
  state.update(status='PASS',checks=len(tables['METHOD_CHECK']),equiv=len(tables['STATE_EQUIV']),profile=len(tables.get('STATE_PROFILE_GATE',[])))
  print('STATE_PASS',graph['graph'],flush=True)
 except Exception as exc:state.update(status='FAILED',reason=repr(exc));print('STATE_FAILED',graph['graph'],repr(exc),flush=True)
 state['wall_seconds']=time.time()-start;states.append(state);save(folder/'status.json',state);save(run/'suite_status.json',states)
 for key,rows in tables.items():alltables.setdefault(key,[]).extend(rows)
 for key,rows in alltables.items():csvfile(run/(key.lower()+'.csv'),rows)
 csvfile(run/'summary.csv',summary)
completion=dict(study='state',mode=mode,graphs=len(states),passed=sum(x['status']=='PASS' for x in states),failed=sum(x['status']!='PASS' for x in states),checks=sum(x.get('checks',0) for x in states),equiv=sum(x.get('equiv',0) for x in states))
save(run/'completion.json',completion);print('STATE_COMPLETE',json.dumps(completion),flush=True);sys.exit(2 if completion['failed'] else 0)
