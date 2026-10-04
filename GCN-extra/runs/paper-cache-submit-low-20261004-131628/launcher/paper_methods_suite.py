#!/usr/bin/env python3
import csv,hashlib,json,os,pathlib,re,resource,statistics,struct,subprocess,sys,time
root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
run,build=map(pathlib.Path,sys.argv[1:3]);mode=sys.argv[3]
resource.setrlimit(resource.RLIMIT_CORE,(0,0))
def save(p,x):p.write_text(json.dumps(x,indent=2)+'\n')
def sha(p):
 h=hashlib.sha256()
 with p.open('rb') as f:
  for b in iter(lambda:f.read(8*1024*1024),b''):h.update(b)
 return h.hexdigest()
def csvfile(p,rows):
 if not rows:return
 keys=list(dict.fromkeys(k for x in rows for k in x))
 with p.open('w',newline='') as f:
  w=csv.DictWriter(f,keys);w.writeheader();w.writerows(rows)
def invoke(folder,label,argv):
 save(folder/(label+'.command.json'),dict(argv=list(map(str,argv)),cwd=str(folder),numa_policy=None,env={k:os.environ.get(k) for k in ['OMP_NUM_THREADS','OMP_PROC_BIND','OMP_PLACES','MKL_NUM_THREADS','MKL_DYNAMIC','GCN_PAPER_BLOCKS']}))
 start=time.time()
 with (folder/(label+'.log')).open('w') as so,(folder/(label+'.err')).open('w') as se:
  r=subprocess.run(list(map(str,argv)),cwd=str(folder),stdout=so,stderr=se,timeout=2400)
 save(folder/(label+'.status.json'),dict(returncode=r.returncode,wall_seconds=time.time()-start))
 if r.returncode:raise RuntimeError(label+' exit '+str(r.returncode))
 return (folder/(label+'.log')).read_text(errors='replace')
def baseline(log):
 groups={};current=None
 for line in log.splitlines():
  if line.startswith('=== TFS V3'):current='paper_tfs'
  elif line.startswith('=== MKL FP32'):current='source_mkl_fp32'
  elif line.startswith('==='):current=None
  m=re.match(r'\s+run (\d+): ([0-9.]+) ms',line)
  if current and m:groups.setdefault(current,[]).append(float(m[2]))
 assert set(groups)=={'paper_tfs','source_mkl_fp32'} and all(len(v)==5 for v in groups.values())
 return {k:dict(min_ms=min(v),median_ms=statistics.median(v),raw_ms=v) for k,v in groups.items()}
def parse(folder,log,raw_anchor):
 tables={k:[] for k in ['CHECK','TIME','STAGE','PROFILE','CONFIG','GATE_COMPLETE','COMPLETE']}
 for line in log.splitlines():
  if line.startswith('METHOD_'):
   label,_,body=line.partition(' ');key=label[7:]
   if key in tables:tables[key].append(dict(x.split('=',1) for x in body.split() if '=' in x))
 assert len(tables['COMPLETE'])==1 and tables['COMPLETE'][0]['pass']=='1'
 assert len(tables['GATE_COMPLETE'])==1 and tables['GATE_COMPLETE'][0]['pass']=='1'
 assert tables['CHECK'] and all(x['pass']=='1' for x in tables['CHECK'])
 count=int(tables['COMPLETE'][0]['methods']);assert len(tables['CHECK'])==count*4
 for key,v in tables.items():csvfile(folder/(key.lower()+'.csv'),v)
 save(folder/'parsed.json',tables)
 groups={}
 for x in tables['TIME']:groups.setdefault((x['method'],x['kind']),[]).append(float(x['ms']))
 assert len(groups)==count+1 and all(k[1]=='e2e' and len(v)==5 for k,v in groups.items())
 sums=[dict(graph=folder.name,method=k[0],kind=k[1],min_ms=min(v),median_ms=statistics.median(v),max_ms=max(v),repeats=5) for k,v in groups.items()]
 anchors=baseline(log)
 for s in sums:
  for stat in ['min','median']:
   mk=next(x for x in sums if x['method']=='source_mkl_fp32' and x['kind']==s['kind'])[stat+'_ms']
   tf=next(x for x in sums if x['method']=='paper_tfs' and x['kind']==s['kind'])[stat+'_ms']
   s['speedup_'+stat+'_vs_mkl']=mk/s[stat+'_ms'];s['speedup_'+stat+'_vs_tfs']=tf/s[stat+'_ms']
 csvfile(folder/'summary.csv',sums)
 comparison=[]
 for method in ['paper_tfs','source_mkl_fp32']:
  comparison.append(dict(method=method,raw_anchor=raw_anchor[method],embedded_original=anchors[method],rotating=next(x for x in sums if x['method']==method and x['kind']=='e2e'),embedded_vs_raw_min_ratio=anchors[method]['min_ms']/raw_anchor[method]['min_ms']))
 save(folder/'baseline_anchor_audit.json',comparison)
 return sums,tables['CHECK'],comparison
parts={
 'high':['ogbn-products','reddit','mycielskian19','hollywood-2009','indochina-2004'],
 'medium':['soc-Pokec','cit-Patents','soc-LiveJournal1','com-LiveJournal','as-Skitter'],
 'low':['rgg_n_2_24_s0','web-Google','roadNet-CA','wiki-Talk','email-Enron','amazon0601','com-Youtube']}
if mode=='smoke':
 graphs=[]
 for name,n in [('method_tail',37),('method_high_tail',137)]:
  p=run/'fixtures'/name;p.mkdir(parents=True);row=[0];col=[]
  for i in range(n):
   d=[0,1,2,7,8,9,16,31,32,33,63,64,65,129][i%14];col.extend((i*7+j*11)%n for j in range(d));row.append(len(col))
  path=p/(name+'.csrbin')
  with path.open('wb') as f:
   f.write(struct.pack('<IIIQQQ',0,0,2,n,n,len(col)));f.write(struct.pack('<'+'I'*len(row),*row));f.write(struct.pack('<'+'I'*len(col),*col));f.write(struct.pack('<'+'f'*len(col),*([1.]*len(col))))
  graphs.append(dict(graph=name,path=str(path),N=n,E=len(col),avg_degree=len(col)/n))
else:
 allgraphs=json.loads((root/'runs/formal-10862002/graph_list.json').read_text());mapping={x['graph']:x for x in allgraphs}
 graphs=[mapping[x] for x in parts[mode]]
 known={x['graph']:x for x in json.loads((root/'runs/formal-10862002/suite_status.json').read_text())}
save(run/'graph_list.json',graphs);states=[];allrows=[];allchecks=[];failures=0
for g in graphs:
 folder=run/g['graph'];folder.mkdir();state=dict(g)
 try:
  print('METHOD_START',g['graph'],flush=True);start=time.time();path=pathlib.Path(g['path']);state['input_sha256']=sha(path)
  if mode!='smoke':
   assert state['input_sha256']==known[g['graph']]['dataset_sha256'];assert g['E']<=2147483647 and g['N']*128<=2147483648
  data=path.parent.parent
  anchor=baseline(invoke(folder,'original_raw',[build/'paper_original',data,g['graph']]))
  log=invoke(folder,'methods',[build/'paper_methods',data,g['graph']]);sums,checks,audit=parse(folder,log,anchor)
  state.update(status='PASS',wall_seconds=time.time()-start,checks=len(checks),anchor_audit=audit)
  allrows.extend(sums);allchecks.extend(checks)
  show=[x for x in sums if x['kind']=='e2e' and x['method'] in ['paper_tfs','source_mkl_fp32','bfull_fast','bfull_accurate','b8_fast','b16_fast','b64_fast']]
  print('METHOD_PASS',g['graph'],json.dumps(show),flush=True)
 except Exception as e:
  failures+=1;state.update(status='FAILED',reason=str(e));print('METHOD_FAILED',g['graph'],str(e),flush=True)
 states.append(state);save(folder/'status.json',state);save(run/'suite_status.json',states);csvfile(run/'summary.csv',allrows);csvfile(run/'all_checks.csv',allchecks)
save(run/'completion.json',dict(passed=sum(x['status']=='PASS' for x in states),failed=failures,checks=len(allchecks),mode=mode))
print('METHOD_SUITE_COMPLETE',json.dumps(json.loads((run/'completion.json').read_text())),flush=True)
sys.exit(2 if failures else 0)
