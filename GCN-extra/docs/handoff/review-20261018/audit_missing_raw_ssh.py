from pathlib import Path
import subprocess,json,csv,statistics,math,hashlib

remote = r'''
from pathlib import Path
import json,re,statistics,hashlib
root=Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs')
studies={
 'unpacked17': [root/'onemkl-bf16-twostep-20261015-081637'/x for x in ['diagnostic','remaining']],
 'packed4': [root/'onemkl-bf16-twostep-packed-20261015-163448/full17_4core']}
out={}
for key,dirs in studies.items():
 graphs={};metadata=[]
 for d in dirs:
  meta={}
  for name in ['node.txt','numa_policy.txt','environment.txt','event.json']:
   f=d/name
   if f.exists():meta[name]=f.read_text(errors='replace')
  metadata.append({'path':str(d),'files':meta})
  for f in sorted((d/'logs').glob('*.out')):
   txt=f.read_text(errors='replace');name=f.stem
   assert 'DONE graph='+name+' pass=1' in txt
   values={}
   for method,value in re.findall(r'TIME graph=\S+ method=(\S+) repeat=\d+ order=\d+ e2e_ms=(\S+)',txt):
    values.setdefault(method,[]).append(float(value))
   assert len(values)==(5 if key=='unpacked17' else 6)
   assert all(len(v)==5 for v in values.values())
   med={m:float(v) for m,v in re.findall(r'MEDIAN graph=\S+ method=(\S+) ms=(\S+)',txt)}
   assert all(statistics.median(v)==med[m] for m,v in values.items())
   checks=[line for line in txt.splitlines() if line.startswith('CHECK ')]
   assert all(('pass=1' in x) or ('bitwise=1' in x) for x in checks)
   assert name not in graphs
   graphs[name]={'path':str(f),'sha256':hashlib.sha256(f.read_bytes()).hexdigest(),'samples':values,'median':med,'checks':checks}
 assert len(graphs)==17
 source=dirs[0].parent/'source_snapshot/driver.cpp'
 out[key]={'graphs':graphs,'metadata':metadata,'driver_sha256':hashlib.sha256(source.read_bytes()).hexdigest()}
print(json.dumps(out))
'''
run=subprocess.run(['ssh','-o','BatchMode=yes','-o','ConnectTimeout=10','hdacp1@176.0.250.88','python3 -'],input=remote,text=True,encoding='utf-8',capture_output=True,check=True)
data=json.loads(run.stdout)
root=Path('C:/Users/花喵/tmp/tfs18')
studies={'unpacked17':('results/onemkl-bf16-twostep-20261015/fixed17.csv','experiments/onemkl_bf16_twostep_20261015/driver.cpp'),
 'packed4':('results/onemkl-bf16-twostep-packed-20261015/full17_4core.csv','experiments/onemkl_bf16_twostep_packed_20261015/driver.cpp')}
summary={}
for key,(csvfile,source) in studies.items():
 rows=list(csv.DictReader((root/csvfile).open(encoding='utf-8-sig')))
 mapping={'bf16_mkl_ms':'bf16_mkl_twostep','original_tfs_ms':'original_tfs','full_degree_ms':'full_degree','full_identity_ms':'full_identity','fp32_mkl_ms':'fp32_mkl_twostep'}
 if key=='packed4':mapping['bf16_mkl_packed_ms']='bf16_mkl_packed_twostep'
 diff=0.;stats=[]
 for row in rows:
  g=data[key]['graphs'][row['graph']]
  for col,method in mapping.items():
   delta=abs(float(row[col])-statistics.median(g['samples'][method]));diff=max(delta,diff)
   assert delta<1e-6,(key,row['graph'],col,delta)
  for method,v in g['samples'].items():
   stats.append({'graph':row['graph'],'method':method,'cv_pct':100*statistics.pstdev(v)/statistics.mean(v),'min':min(v),'max':max(v)})
 localsha=hashlib.sha256((root/source).read_bytes()).hexdigest()
 summary[key]={'graphs':len(rows),'method_groups':len(stats),'samples':len(stats)*5,'max_csv_median_difference_ms':diff,'driver_matches_github_snapshot':localsha==data[key]['driver_sha256'],'cv_gt5':sum(x['cv_pct']>5 for x in stats),'cv_gt10':sum(x['cv_pct']>10 for x in stats),'top_cv':sorted(stats,key=lambda x:x['cv_pct'],reverse=True)[:7]}
out=Path(__file__).parent
(out/'missing_raw_ssh_evidence.json').write_text(json.dumps(data,ensure_ascii=False,indent=2),encoding='utf-8')
(out/'missing_raw_ssh_summary.json').write_text(json.dumps(summary,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(summary,ensure_ascii=False,indent=2))
