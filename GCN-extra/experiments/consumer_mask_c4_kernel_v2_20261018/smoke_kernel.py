#!/usr/bin/env python3
import json
import pathlib
import struct
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
name='interleaved_rounding_stress';n=2304;adj=[]
for row in range(n):
    if row>=16:adj.append([]);continue
    values=[]
    for j in range(2048):
        values.append(j)
        if j%128==0:values.append(2048+row*16+j//128)
    adj.append(values)
ptr=[0];col=[]
for a in adj:col.extend(a);ptr.append(len(col))
p=run/(name+'.csrbin');out=run/(name+'.json')
p.write_bytes(struct.pack('<IIIQQQ',0,0,2,n,n,len(col))
              +struct.pack('<'+'I'*len(ptr),*ptr)
              +struct.pack('<'+'I'*len(col),*col)
              +struct.pack('<'+'f'*len(col),*([1.0]*len(col))))
cp=subprocess.run([str(binary),'--csr',str(p),'--graph',name,'--out',str(out),
                   '--expected-save',str((2048-1)*(16-1)),'--repeats','1'],capture_output=True,text=True)
(run/(name+'.stdout')).write_text(cp.stdout);(run/(name+'.stderr')).write_text(cp.stderr)
assert cp.returncode==0,(name,cp.stderr)
r=json.loads(out.read_text())
assert r['shared_error']['rel_l2']>0,('numerical stress did not expose reorder rounding',r['shared_error'])
assert r['shared_error']['rel_l2']<=1e-2
print('PASS interleaved_rounding_stress shared_rel_l2',r['shared_error']['rel_l2'],flush=True)
print('KERNEL_SMOKE_ALL_PASS',flush=True)
