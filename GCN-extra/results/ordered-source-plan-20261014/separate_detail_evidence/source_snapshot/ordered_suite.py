#!/usr/bin/env python3
import pathlib,json,csv,struct,subprocess,statistics,hashlib,os,sys,shlex,time
ROOT=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
run,build=map(pathlib.Path,sys.argv[1:3]);mode=sys.argv[3]
def save(p,d):p.write_text(json.dumps(d,indent=2)+'\n')
def sha(p):
 h=hashlib.sha256()
 with p.open('rb') as f:
  for b in iter(lambda:f.read(8*1024*1024),b''):h.update(b)
 return h.hexdigest()
def csvfile(p,rr):
 if rr:
  with p.open('w',newline='') as f:
   w=csv.DictWriter(f,list(dict.fromkeys(k for r in rr for k in r)));w.writeheader();w.writerows(rr)
def fixture(name,n,adj):
 p=run/'fixtures'/name;p.mkdir(parents=True);row=[0];col=[]
 for a in adj:col.extend(a);row.append(len(col))
 f=p/(name+'.csrbin')
 with f.open('wb') as s:
  s.write(struct.pack('<IIIQQQ',0,0,2,n,n,len(col)));s.write(struct.pack('<'+'I'*len(row),*row));s.write(struct.pack('<'+'I'*len(col),*col));s.write(struct.pack('<'+'f'*len(col),*([1.]*len(col))))
 return dict(graph=name,path=str(f),N=n,E=len(col))
inv={r['graph']:r for r in json.loads((ROOT/'docs/CURRENT_GRAPH_INVENTORY_20261004.json').read_text())}
diagnosis=[inv[g] for g in ['ogbn-products','reddit','mycielskian19','roadNet-CA','wiki-Talk']]
packets=[];tables_all={};summary=[]
graphs=[]
if mode in ['smoke_structure','smoke_detail']:
 for n in [1,17,33,65]:
  ds=[0,1,63,64,65,255,256,257,511,512,513]
  adj=[[(i*3+k*7)%n for k in range(513 if n==1 else ds[(i+1)%len(ds)])] for i in range(n)]
  graphs.append((fixture('ordered_tail_'+str(n),n,adj),'smoke'))
 n=65
 adj=[]
 for i in range(n):
  if i==64:adj.append([])
  else:adj.append(list(range(32))+[60+(i%2)]*32+[(k*3)%n for k in range(64)])
 graphs.append((fixture('ordered_partial_runs',n,adj),'smoke'))
 adj=[(([1,2]*32) if i%2==0 else ([2,1]*32)) for i in range(33)]
 graphs.append((fixture('ordered_crossing',33,adj),'smoke'))
 graphs.extend((g,'structure') for g in diagnosis)
elif mode=='detail':graphs=[(g,'diagnostic') for g in diagnosis]
else:graphs=[(inv[g],'performance') for g in mode.split(',')]
save(run/'graph_list.json',[dict(g,phase=phase) for g,phase in graphs])
for graph,phase in graphs:
 p=run/(phase+'-'+graph['graph']);p.mkdir();f=pathlib.Path(graph['path'])
 packet=dict(graph,phase=phase,actual_input_sha256=sha(f))
 if 'input_sha256' in graph:assert packet['actual_input_sha256']==graph['input_sha256']
 if phase=='diagnostic':os.environ['EVENT_DIAGNOSTIC_ONLY']='1'
 else:os.environ.pop('EVENT_DIAGNOSTIC_ONLY',None)
 os.environ['STATE_SMOKE']='1' if phase=='smoke' else '0';os.environ['STATE_ABLATIONS']='1'
 os.environ['STATE_PMU']='1' if phase=='performance' else '0'
 if phase=='structure':os.environ['EVENT_STRUCTURE_ONLY']='1'
 else:os.environ.pop('EVENT_STRUCTURE_ONLY',None)
 argv=[str(build/'ordered_methods'),str(f.parent.parent),graph['graph']]
 save(p/'command.json',dict(argv=argv,cwd=str(p),phase=phase))
 print('ORDERED_START',graph['graph'],phase,flush=True);t=time.time()
 with (p/'program.log').open('w') as out,(p/'program.err').open('w') as err:
  rc=subprocess.run(argv,cwd=p,stdout=out,stderr=err,timeout=2700).returncode
 tt={}
 for line in (p/'program.log').read_text(errors='replace').splitlines():
  key,_,body=line.partition(' ')
  if key.startswith(('STATE_','EVENT_','METHOD_','GROUP_')):
   rr=dict(x.split('=',1) for x in shlex.split(body) if '=' in x);rr['phase_kind']=phase
   tt.setdefault(key,[]).append(rr)
 for key,rr in tt.items():tables_all.setdefault(key,[]).extend(rr)
 packet.update(exit_code=rc,elapsed_seconds=time.time()-t)
 try:
  assert rc==0,(rc,(p/'program.err').read_text())
  assert len(tt.get('EVENT_PLAN',[]))==1 and tt['EVENT_PLAN'][0]['pass']=='1'
  if phase!='structure':
   for key in ['STATE_COMPLETE','METHOD_CHECK','STATE_EQUIV','STATE_SEMANTIC','STATE_WORK_GATE','STATE_PROFILE_GATE']:
    assert tt.get(key) and all(r['pass']=='1' for r in tt[key]),key
   assert len(tt['STATE_PLAN'])==8
   for m in [r['method'] for r in tt['STATE_PLAN']]:
    assert len([r for r in tt['METHOD_CHECK'] if r['method']==m])==4
   counters=tt['EVENT_COUNTERS']
   for r in counters:
    ref=next(x for x in counters if x['method']=='pack_epi_degree' and x['layer']==r['layer'])
    for k in ['edges','adds','tdp','prefetch','partial_load_bytes','partial_store_bytes','packed_bytes']:
     assert r[k]==ref[k],(r['method'],k,r[k],ref[k])
   if graph['graph']=='ordered_partial_runs':
    assert int(tt['EVENT_PLAN'][0]['partial_matches'])>0,'partial matching fixture not covered'
    for r in counters:
     if r['method']=='event_share':assert int(r['saved_decode_rows'])>0 and int(r['head_comparisons'])==0
   if phase=='performance':
    times=tt['STATE_TIME'];assert len(times)==9*15
    for m in [r['method'] for r in tt['STATE_PLAN']]+['source_mkl_fp32']:
     for kind,n in [('alternating',10),('consecutive',5)]:
      v=[float(r['ms']) for r in times if r['method']==m and r['kind']==kind];assert len(v)==n
      summary.append(dict(graph=graph['graph'],method=m,protocol=kind,median_ms=statistics.median(v),min_ms=min(v),max_ms=max(v),CV=statistics.stdev(v)/statistics.mean(v),samples=json.dumps(v)))
  packet['status']='PASS'
 except Exception as e:packet.update(status='FAILED',error=repr(e))
 packets.append(packet);save(p/'status.json',packet)
 for key,rr in tables_all.items():csvfile(run/(key.lower()+'.csv'),rr)
 csvfile(run/'timing_summary.csv',summary)
 save(run/'completion.json',dict(status=packet['status'],graphs=packets,counts={k:len(v) for k,v in tables_all.items()}))
 print('ORDERED_FINISH',graph['graph'],phase,packet['status'],flush=True)
 if packet['status']!='PASS':raise RuntimeError(packet)
