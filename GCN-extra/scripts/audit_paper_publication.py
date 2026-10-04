#!/usr/bin/env python3
"""Check new environment snapshots without printing any values."""
import json,pathlib,re,sys
root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
bad=[];count=0
for pattern in ['paper-method-*','paper-cache-*']:
 for r in (root/'runs').glob(pattern):
  for p in r.rglob('environment.txt'):
   count+=1
   for line in p.read_text(errors='replace').splitlines():
    key,sep,_=line.partition('=')
    if sep and re.search(r'TOKEN|PASSWORD|SECRET|PRIVATE_KEY|ACCESS_KEY',key,re.I):bad.append(dict(file=str(p.relative_to(root)),key=key))
print(json.dumps(dict(environment_files=count,suspicious_keys=bad,values_printed=False)))
sys.exit(bool(bad))
