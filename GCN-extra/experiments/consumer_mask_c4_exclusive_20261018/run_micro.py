#!/usr/bin/env python3
import csv
import hashlib
import json
import pathlib
import subprocess
import sys
import time

root=pathlib.Path(sys.argv[1]);run=pathlib.Path(sys.argv[2]);binary=pathlib.Path(sys.argv[3])
inventory={g['graph']:g for g in json.loads((root/'docs/CURRENT_GRAPH_INVENTORY_20261004.json').read_text())}
with (root/'runs/consumer_mask_c4_20261018-01/summary.csv').open() as f:
    scan={r['graph']:r for r in csv.DictReader(f) if r['order']=='identity'}
names=['amazon0601','roadNet-CA','hollywood-2009','mycielskian19']
raw=run/'raw';raw.mkdir(exist_ok=True)
results=[]
for name in names:
    g=inventory[name];p=pathlib.Path(g['path']);assert p.is_file() and p.stat().st_size==g['size_bytes']
    h=hashlib.sha256()
    with p.open('rb') as f:
        for block in iter(lambda:f.read(8*1024*1024),b''):h.update(block)
    assert h.hexdigest()==g['input_sha256'],name
    out=raw/(name+'.json')
    cmd=[str(binary),'--csr',str(p),'--graph',name,'--out',str(out),
         '--reserve',scan[name]['unique_tile_sources'],
         '--expected-save',scan[name]['saved_adds_per_feature'],'--repeats','6']
    (raw/(name+'.command.json')).write_text(json.dumps(cmd,indent=2))
    print('MICRO_START',name,flush=True);t=time.monotonic()
    cp=subprocess.run(cmd,capture_output=True,text=True)
    (raw/(name+'.stdout')).write_text(cp.stdout)
    (raw/(name+'.stderr')).write_text(cp.stderr)
    assert cp.returncode==0,(name,cp.returncode,cp.stderr)
    r=json.loads(out.read_text());r['input_sha256']=h.hexdigest();r['wall_s']=time.monotonic()-t
    assert r['schema']=='mask-kernel-exclusive-v1'
    assert r['N']==g['N'] and r['E']==g['E']
    assert r['plan_entries']==int(scan[name]['unique_tile_sources'])
    results.append(r);(run/'partial_summary.json').write_text(json.dumps(results,indent=2))
    print('MICRO_DONE',name,'build_s',round(r['plan_build_s'],3),'rowwise_s',r['rowwise_s'],
          'fanout_s',r['fanout_s'],'shared_s',r['shared_s'],flush=True)
(run/'summary.json').write_text(json.dumps(results,indent=2))
print('MICRO_ALL_PASS',len(results),flush=True)
