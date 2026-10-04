#!/usr/bin/env python3
"""Unmodified paper sources; validation first, raw repeats and both min/median."""
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
 save(folder/(label+'.command.json'),dict(argv=list(map(str,argv)),cwd=str(folder),numa_policy=None,env={k:os.environ.get(k) for k in ['OMP_NUM_THREADS','OMP_PROC_BIND','OMP_PLACES','MKL_NUM_THREADS','MKL_DYNAMIC']}))
 start=time.time()
 with (folder/(label+'.log')).open('w') as out,(folder/(label+'.err')).open('w') as err:
  result=subprocess.run(list(map(str,argv)),cwd=str(folder),stdout=out,stderr=err,timeout=1200)
 save(folder/(label+'.status.json'),dict(returncode=result.returncode,wall_seconds=time.time()-start))
 if result.returncode:raise RuntimeError(label+' exit '+str(result.returncode))
 return (folder/(label+'.log')).read_text(errors='replace')
def metrics(log):
 m=re.search(r'PAPER_FINAL_CHECK (.+)',log)
 if not m:raise RuntimeError('full BF16 final validation missing')
 d=dict(x.split('=',1) for x in m.group(1).split());assert d['pass']=='1';return d
def parse_e2e(log):
 groups={};current=None
 for line in log.splitlines():
  if line.startswith('=== TFS V3'):current='tfs_e2e'
  elif line.startswith('=== MKL FP32'):current='mkl_e2e'
  elif line.startswith('==='):current=None
  m=re.match(r'\s+run (\d+): ([0-9.]+) ms',line)
  if current and m:groups.setdefault(current,[]).append(float(m[2]))
 assert set(groups)=={'tfs_e2e','mkl_e2e'} and all(len(v)==5 for v in groups.values())
 return groups
def parse_single(log,kind):
 pat=r'\s+run \d+: ([0-9.]+) ms' if kind=='tfs_kernel' else r'\s+run \d+: SpMM=[0-9.]+\s+GeMM=[0-9.]+\s+Total=([0-9.]+) ms'
 v=[float(x) for x in re.findall(pat,log)];assert len(v)==5;return {kind:v}
paper={
 'amazon0601':(12.65,25.62,2.88),'as-Skitter':(100.52,162.63,2.18),
 'cit-Patents':(77.05,271.32,4.80),'com-LiveJournal':(237.90,646.45,3.68),
 'com-Youtube':(47.33,54.17,1.41),'email-Enron':(2.30,1.43,.70),
 'hollywood-2009':(298.88,392.32,1.88),'indochina-2004':(347.33,696.06,2.29),
 'mycielskian19':(2132,4814,2.74),'ogbn-products':(329.69,1348,6.67),
 'reddit':(292.82,441.37,2.36),'rgg_n_2_24_s0':(885.20,2022,3.41),
 'roadNet-CA':(35.25,66.23,2.24),'soc-LiveJournal1':(245.07,680,3.70),
 'soc-Pokec':(96.50,331.18,5.05),'web-Google':(20.66,46.73,2.55),
 'wiki-Talk':(106.69,90.34,.94)}
if mode=='smoke':
 graphs=[]
 for name,n in [('paper_tail',37),('paper_high_tail',137)]:
  p=run/'fixtures'/name;p.mkdir(parents=True);row=[0];col=[]
  for i in range(n):
   d=[0,1,7,16,32,65,129][i%7];col.extend((i*7+j*11)%n for j in range(d));row.append(len(col))
  path=p/(name+'.csrbin')
  with path.open('wb') as f:
   f.write(struct.pack('<IIIQQQ',0,0,2,n,n,len(col)))
   f.write(struct.pack('<'+'I'*len(row),*row));f.write(struct.pack('<'+'I'*len(col),*col));f.write(struct.pack('<'+'f'*len(col),*([1.]*len(col))))
  graphs.append(dict(graph=name,path=str(path),N=n,E=len(col),avg_degree=len(col)/n))
else:
 graphs=json.loads((root/'runs/formal-10862002/graph_list.json').read_text())
 # Important representative results first, then all eligible graphs.
 order=['ogbn-products','reddit','soc-Pokec','cit-Patents','soc-LiveJournal1','web-Google','roadNet-CA','wiki-Talk']
 graphs.sort(key=lambda x:order.index(x['graph']) if x['graph'] in order else len(order))
 known={x['graph']:x for x in json.loads((root/'runs/formal-10862002/suite_status.json').read_text())}
save(run/'graph_list.json',graphs)
states=[];summaries=[];raw=[];failures=0
for g in graphs:
 name=g['graph'];folder=run/name;folder.mkdir();state=dict(g)
 if mode!='smoke' and name not in paper:
  reason='Original TFS ignores nonunit CSR values; unlike MKL' if name in ['cage15','FullChip','kron_g500-logn21','rajat31','scircuit','sx-stackoverflow'] else 'Original LP64 overflow; preserve historical failure without rerunning a known unsafe case'
  state.update(status='EXCLUDED',reason=reason)
 else:
  try:
   print('PAPER_START',name,flush=True);start=time.time();path=pathlib.Path(g['path'])
   state['input_sha256']=sha(path)
   if mode!='smoke':
    expected=known[name].get('dataset_sha256');state['historical_input_sha256']=expected
    if expected and expected!=state['input_sha256']:raise RuntimeError('Dataset hash differs from full unit-value scan evidence')
   data=path.parent.parent
   check=invoke(folder,'correctness',[build/'paper_check',data,name]);state['final_bf16_correctness']=metrics(check)
   log=invoke(folder,'e2e',[build/'paper_e2e_raw',data,name]);groups=parse_e2e(log)
   log=invoke(folder,'tfs_kernel',[build/'paper_tfs_raw',data,name,'64'])
   if ' PASS' not in log or ' FAIL' in log:raise RuntimeError('Original standalone TFS check did not pass')
   groups.update(parse_single(log,'tfs_kernel'))
   log=invoke(folder,'mkl_kernel',[build/'paper_mkl_raw',data,name,'128']);groups.update(parse_single(log,'mkl_kernel'))
   row=dict(graph=name,N=g['N'],E=g['E'],avg_degree=g['avg_degree'])
   for method,v in groups.items():
    row[method+'_min_ms']=min(v);row[method+'_median_ms']=statistics.median(v)
    raw.extend(dict(graph=name,method=method,repeat=i,ms=x) for i,x in enumerate(v))
   for stat in ['min','median']:
    for kind in ['kernel','e2e']:row[kind+'_'+stat+'_speedup']=row['mkl_'+kind+'_'+stat+'_ms']/row['tfs_'+kind+'_'+stat+'_ms']
   if name in paper:
    p=paper[name];row.update(paper_tfs_e2e_ms=p[0],paper_mkl_e2e_ms=p[1],paper_kernel_speedup=p[2],paper_e2e_speedup=p[1]/p[0])
   row.update(state['final_bf16_correctness']);summaries.append(row)
   state.update(status='PASS',wall_seconds=time.time()-start)
   print('PAPER_PASS',name,'kernel',row['kernel_min_speedup'],'E2E',row['e2e_min_speedup'],flush=True)
  except Exception as e:
   failures+=1;state.update(status='FAILED',reason=str(e));print('PAPER_FAILED',name,str(e),flush=True)
 states.append(state);save(folder/'status.json',state);save(run/'suite_status.json',states);csvfile(run/'summary.csv',summaries);csvfile(run/'raw_repeats.csv',raw)
save(run/'completion.json',dict(passed=sum(x['status']=='PASS' for x in states),excluded=sum(x['status']=='EXCLUDED' for x in states),failed=failures,mode=mode))
print('PAPER_COMPLETE',json.dumps(json.loads((run/'completion.json').read_text())),flush=True)
sys.exit(2 if failures else 0)
