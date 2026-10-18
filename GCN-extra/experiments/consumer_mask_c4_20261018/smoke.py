#!/usr/bin/env python3
import collections
import json
import pathlib
import struct
import subprocess
import sys

run = pathlib.Path(sys.argv[1]); binary = pathlib.Path(sys.argv[2]); run.mkdir(parents=True, exist_ok=True)
fixtures = {
    'overlap_dup': [[3,2,2,1],[1,2,3],[1,3]] + [[] for _ in range(13)],
    'no_overlap': [[i] for i in range(16)],
    'tile_boundary': [[1,2,3]] + [[] for _ in range(15)] + [[1,2,3]] + [[] for _ in range(15)],
    'high_overlap': [list(range(8)) for _ in range(16)],
}

def oracle(adj):
    e=sum(map(len,adj)); unique=dups=bad=eligible_groups=eligible_sources=eligible_edges=saving=groups=0
    hist=[0]*17
    for start in range(0,len(adj),16):
        src=collections.defaultdict(lambda:[0,False])
        for bit,row in enumerate(adj[start:start+16]):
            for j in row:
                if src[j][0]&(1<<bit):src[j][1]=True;dups+=1
                src[j][0]|=1<<bit
        unique+=len(src)
        bad+=sum(x[1] for x in src.values())
        freq=collections.Counter(mask for mask,duplicate in src.values() if not duplicate)
        groups+=len(freq)
        for mask,n in freq.items():
            k=bin(mask).count('1');hist[k]+=n
            if n>=2 and k>=2:
                eligible_groups+=1;eligible_sources+=n;eligible_edges+=n*k
                saving+=(n-1)*(k-1)
    return dict(edge_occurrences=e,unique_tile_sources=unique,duplicate_occurrences=dups,
                duplicate_sources_excluded=bad,eligible_groups=eligible_groups,
                eligible_sources=eligible_sources,eligible_edges=eligible_edges,
                saved_adds_per_feature=saving,group_records=groups,mask_popcount_source_hist=hist)

for name,adj in fixtures.items():
    n=len(adj);row=[0];col=[]
    for a in adj:col.extend(a);row.append(len(col))
    path=run/(name+'.csrbin');out=run/(name+'.jsonl')
    path.write_bytes(struct.pack('<IIIQQQ',0,0,2,n,n,len(col))
                     +struct.pack('<'+'I'*len(row),*row)
                     +struct.pack('<'+'I'*len(col),*col)
                     +struct.pack('<'+'f'*len(col),*([1.0]*len(col))))
    cp=subprocess.run([str(binary),'--csr',str(path),'--graph',name,'--out',str(out)],capture_output=True,text=True)
    assert cp.returncode==0,(name,cp.stdout,cp.stderr)
    records=[json.loads(line) for line in out.read_text().splitlines()]
    assert {r['order'] for r in records}=={'identity','degree'}
    ref=oracle(adj)
    for field,value in ref.items():
        assert records[0][field]==value,(name,field,value,records[0][field])
    for r in records:
        assert r['edge_occurrences']==len(col)
        assert r['saved_adds_per_feature']<=len(col)
    print('PASS',name, 'identity_saving',records[0]['saved_adds_per_feature'],flush=True)
assert json.loads((run/'overlap_dup.jsonl').read_text().splitlines()[0])['saved_adds_per_feature']==2
assert json.loads((run/'high_overlap.jsonl').read_text().splitlines()[0])['saved_adds_per_feature']==105
print('SMOKE_ALL_PASS')
