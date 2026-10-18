from pathlib import Path
import csv,json,re,statistics,math
p=Path('C:/Users/花喵/tmp/tfs18')
def rows(rel): return list(csv.DictReader((p/rel).open(encoding='utf-8-sig',newline='')))
def gm(v): return math.exp(statistics.mean(map(math.log,v)))
def dump(tag,v): print(tag,json.dumps(v,ensure_ascii=False))
fs=rows('results/full-degree-sort-ablation-20261015/summary_17.csv')
dump('FULL-summary',dict(n=len(fs),gm=gm([float(x['degree_median_ms'])/float(x['identity_median_ms']) for x in fs]),wins=sum(float(x['degree_median_ms'])>float(x['identity_median_ms']) for x in fs),gt20=sum(float(x['speedup_degree_over_identity'])>1.2 for x in fs)))
alllog=list((p/'results/full-degree-sort-ablation-20261015').rglob('results.json'))
checked=0
for f in alllog:
 data=json.loads(f.read_text());
 for d in data:
  t=d['times'];s=d['summary'];
  for k in ['degree','identity']:
   v=[float(x['prepared_e2e_ms']) for x in t if x['schedule']==k]
   assert abs(statistics.median(v)-s[k]['median_ms'])<1e-6
  checked+=1
dump('FULL-raw-validated',dict(result_files=len(alllog),graph_runs=checked))
for rel in ['results/onemkl-bf16-twostep-20261015/fixed17.csv','results/onemkl-bf16-twostep-packed-20261015/full17_32core.csv','results/onemkl-bf16-twostep-packed-20261015/full17_4core.csv','results/onemkl-bf16-interleave-20261015/full17_32core.csv']:
 rr=rows(rel);out={'n':len(rr)}
 for col in ['bf16_mkl_ms','bf16_mkl_packed_ms']:
  if col not in rr[0]:continue
  for den in ['original_tfs_ms','full_identity_ms']:
   v=[float(x[col])/float(x[den]) for x in rr];out[col+'/'+den]={'gm':gm(v),'gt1':sum(a>1 for a in v)}
 if 'bf16_mkl_packed_ms' in rr[0]:
  v=[float(x['bf16_mkl_packed_ms'])/float(x['bf16_mkl_ms']) for x in rr];out['packed/unpacked']={'gm':gm(v),'packedwins':sum(a<1 for a in v)}
 dump(rel,out)
 rrraw=[]
 for f in (p/'results/onemkl-bf16-twostep-packed-20261015/logs').glob('*.out'):
  text=f.read_text();med={m:float(v) for m,v in re.findall(r'MEDIAN graph=\S+ method=(\S+) ms=(\S+)',text)}
  t={m:[] for m in med}
  for m,v in re.findall(r'TIME graph=\S+ method=(\S+) repeat=\d+ order=\d+ e2e_ms=(\S+)',text):t[m].append(float(v))
  assert all(len(v)==5 and abs(statistics.median(v)-med[m])<1e-5 for m,v in t.items())
  for m,v in t.items():rrraw.append({'graph':f.stem,'method':m,'cv_pct':100*statistics.pstdev(v)/statistics.mean(v),'min':min(v),'median':statistics.median(v),'max':max(v)})
dump('packed32-raw-topCV',sorted(rrraw,key=lambda x:x['cv_pct'],reverse=True)[:15])
dump('packed32-raw-CV-summary',dict(n=len(rrraw),gt5=sum(x['cv_pct']>5 for x in rrraw),gt10=sum(x['cv_pct']>10 for x in rrraw)))
h=rows('results/h-panel-reuse-20261014/summary_by_run.csv')
for rev in [False,True]:
 r=[x for x in h if x['graph'] not in ['tail_37','tail_137'] and ('reverse' in x['run'])==rev and x['method']=='A5_window64']
 dump('HPanel-W64-'+str(rev),dict(n=len(r),gm=gm([float(x['speedup_vs_A0']) for x in r]),ratios={x['graph']:x['speedup_vs_A0'] for x in r}))
wf=p/'results/h-panel-reuse-20261014/h-panel-reuse-10964915/results/work.csv'
for x in csv.DictReader(wf.open()):
 if x['graph']=='roadNet-CA' and x['method'] in ['A3_window16','A4_window32','control_window16_duplicate_gather']:dump('HPanel-bug-evidence',x)
