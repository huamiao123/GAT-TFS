from pathlib import Path
import subprocess,json,csv,collections,statistics,hashlib,math

audit=Path(__file__).parent
local=Path(r'C:\Users\花喵\tmp\tfs18')
remote=r'''
from pathlib import Path
import json,hashlib
root=Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs')
logs=[]
for study in root.glob('journal-a3-build-*'):
 for f in (study/'logs').glob('*/*/methods.log'):
  lines=[line for line in f.read_text(errors='replace').splitlines() if line.startswith('METHOD_TIME ')]
  if lines:logs.append({'path':str(f),'sha256':hashlib.sha256(f.read_bytes()).hexdigest(),'time_lines':lines})
print(json.dumps(logs))
'''
p=subprocess.run(['ssh','-o','BatchMode=yes','-o','ConnectTimeout=10','hdacp1@176.0.250.88','python3 -'],input=remote,text=True,encoding='utf8',capture_output=True,check=True)
logs=json.loads(p.stdout)
(audit/'p0_remote_raw_evidence.json').write_text(json.dumps(logs,indent=2),encoding='utf8')
cols=['graph','method','kind','repeat','order','ms']
key=lambda row:tuple(row[c] for c in cols)
raw=collections.Counter()
for log in logs:
 for line in log['time_lines']:
  row=dict(part.split('=',1) for part in line.split()[1:]);raw[key(row)]+=1
want=collections.Counter(key(row) for row in csv.DictReader((local/'results/journal-p0-20261004/time.csv').open(encoding='utf8')))
missing=want-raw
out={'p0':{'remote_raw_logs':len(logs),'remote_time_lines':sum(raw.values()),'github_csv_rows':sum(want.values()),'missing_exact_rows':sum(missing.values()),'missing_examples':list(missing.items())[:3]}}

base=local/'results/crosslayer-fusion-20261008/runs/access-prefetch-20261008-01/logs'
groups=collections.defaultdict(list)
for f in base.glob('prefetch-*/fact_time.csv'):
 job=f.parent.name.split('-')[-1]
 for r in csv.DictReader(f.open(encoding='utf8')):
  groups[(job,r['graph'],r['method'],r['kind'])].append(float(r['ms']))
med={k:statistics.median(v) for k,v in groups.items()}
case=[]
for partition,jobs in [('intel_expr',{'10895500','10895501'}),('intel',{'10895503','10895504'})]:
 for kind in ['same_slot_consecutive','same_slot_alternating']:
  ratios=[]
  for (job,graph,method,k),t in med.items():
   if job in jobs and method=='s1ExactSP8' and k==kind:
    ratios.append({'graph':graph,'ratio':med[(job,graph,'pack_epi_degree',kind)]/t})
  case.append({'partition':partition,'kind':kind,'ratios':ratios,'gm':math.exp(sum(math.log(x['ratio']) for x in ratios)/len(ratios))})
out['prefetch']={'samples':sum(map(len,groups.values())),'groups':len(groups),'expansion_candidate_cohorts':case}
(audit/'final_supplement_summary.json').write_text(json.dumps(out,indent=2),encoding='utf8')
print(json.dumps(out,indent=2))
