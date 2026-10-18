#!/usr/bin/env python3
import csv
import hashlib
import json
import pathlib
import subprocess
import sys
import time

root=pathlib.Path(sys.argv[1]);run=pathlib.Path(sys.argv[2]);binary=pathlib.Path(sys.argv[3])
inventory=json.loads((root/'docs/CURRENT_GRAPH_INVENTORY_20261004.json').read_text())
assert len(inventory)==17
raw=run/'raw';raw.mkdir(exist_ok=True)
summary=[]
for i,g in enumerate(inventory):
    path=pathlib.Path(g['path']);assert path.is_file() and path.stat().st_size==g['size_bytes'],g['graph']
    t=time.monotonic();h=hashlib.sha256()
    with path.open('rb') as f:
        for block in iter(lambda:f.read(8*1024*1024),b''):h.update(block)
    actual=h.hexdigest();assert actual==g['input_sha256'],(g['graph'],actual,g['input_sha256'])
    out=raw/(g['graph']+'.jsonl')
    command=[str(binary),'--csr',str(path),'--graph',g['graph'],'--out',str(out)]
    print('SCAN_START',i+1,17,g['graph'],flush=True)
    cp=subprocess.run(command,capture_output=True,text=True)
    (raw/(g['graph']+'.stdout')).write_text(cp.stdout)
    (raw/(g['graph']+'.stderr')).write_text(cp.stderr)
    assert cp.returncode==0,(g['graph'],cp.returncode,cp.stderr)
    rows=[json.loads(line) for line in out.read_text().splitlines()]
    assert {r['order'] for r in rows}=={'identity','degree'}
    for r in rows:
        assert r['N']==g['N'] and r['E']==g['E'] and r['edge_occurrences']==g['E']
        r['input_sha256']=actual
        r['elapsed_graph_s']=time.monotonic()-t
        summary.append(r)
    (run/'partial_summary.json').write_text(json.dumps(summary,indent=2))
    print('SCAN_DONE',g['graph'],[(r['order'],r['saving_fraction']) for r in rows],
          'elapsed_s',round(time.monotonic()-t,3),flush=True)

(run/'summary.json').write_text(json.dumps(summary,indent=2))
fields=['graph','order','N','E','saved_adds_per_feature','saving_fraction','eligible_groups',
        'eligible_edges','duplicate_occurrences','duplicate_sources_excluded',
        'unique_tile_sources','group_records','plan_naive_6byte_source_plus_4byte_group_bytes',
        'plan_8byte_source_plus_4byte_group_bytes','max_tile_edges','max_tile_sources','sort_ms','scan_ms',
        'perm_fnv64','input_sha256']
with (run/'summary.csv').open('w',newline='') as f:
    writer=csv.DictWriter(f,fields);writer.writeheader();writer.writerows({k:r[k] for k in fields} for r in summary)
print('SCAN_ALL_PASS',len(summary),flush=True)
