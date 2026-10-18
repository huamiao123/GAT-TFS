#!/usr/bin/env python3
import json
import pathlib
import subprocess
import sys

old=pathlib.Path(sys.argv[1]);run=pathlib.Path(sys.argv[2]);binary=pathlib.Path(sys.argv[3])
run.mkdir(parents=True,exist_ok=True)
for name,expected in [('high_overlap',105),('no_overlap',0),('tile_boundary',0)]:
    p=old/(name+'.csrbin');out=run/(name+'.json')
    cp=subprocess.run([str(binary),'--csr',str(p),'--graph',name,'--out',str(out),
                       '--expected-save',str(expected),'--repeats','1'],capture_output=True,text=True)
    (run/(name+'.stdout')).write_text(cp.stdout);(run/(name+'.stderr')).write_text(cp.stderr)
    assert cp.returncode==0,(name,cp.stderr)
    r=json.loads(out.read_text());assert r['saved_adds_per_feature']==expected
    assert r['fanout_error']['rel_l2']<=1e-2 and r['shared_error']['rel_l2']<=1e-2
    print('PASS',name,'shared_rel_l2',r['shared_error']['rel_l2'],flush=True)
name='overlap_dup';p=old/(name+'.csrbin');out=run/(name+'.json')
cp=subprocess.run([str(binary),'--csr',str(p),'--graph',name,'--out',str(out),
                   '--repeats','1'],capture_output=True,text=True)
(run/(name+'.stdout')).write_text(cp.stdout);(run/(name+'.stderr')).write_text(cp.stderr)
assert cp.returncode!=0 and 'duplicate edge' in cp.stderr and not out.exists()
print('PASS duplicate_rejection',flush=True)
print('KERNEL_SMOKE_ALL_PASS',flush=True)
