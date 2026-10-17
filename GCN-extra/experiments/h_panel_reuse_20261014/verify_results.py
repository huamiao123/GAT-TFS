#!/usr/bin/env python3
"""Audit saved runs; this does not execute an experiment."""
import csv,json,pathlib,re,sys

root=pathlib.Path(sys.argv[1]);runs=[root/x for x in
  ('h-panel-reuse-10964915','h-panel-reuse-10964919','h-panel-reuse-reverse-10964921')]
graphs={};binary=set();nodes=set();checks=0;timings=0
for run in runs:
    meta=json.loads((run/'manifest.json').read_text())
    assert meta['all_pass'] and meta['repeats']==5 and meta['threads']=='32'
    binary.add(meta['binary_sha256']);nodes.add(meta['hostname'])
    by_graph={}
    for c in csv.DictReader((run/'results'/'checks.csv').open(newline='')):
        checks+=1;assert c['pass']=='1'
        assert all(c[x]=='1' for x in ('l1_fp32_bits','interlayer_bf16_bits','l2_bf16_bits'))
        by_graph.setdefault(c['graph'],set()).add(c['method'])
    for g in meta['graphs']:
        assert g['pass_'] and len(by_graph[g['graph']])==7
        prior=graphs.setdefault(g['graph'],g['sha256'])
        assert prior==g['sha256']
    groups={}
    for t in csv.DictReader((run/'results'/'timings.csv').open(newline='')):
        timings+=1;groups.setdefault((t['graph'],t['method']),0)
        groups[(t['graph'],t['method'])]+=1
    assert all(n==5 for n in groups.values())
    work={}
    for w in csv.DictReader((run/'results'/'work.csv').open(newline='')):
        work.setdefault(w['graph'],[]).append(w)
    for graph,ws in work.items():
        assert len(ws)==7
        tdp={int(x['tdp']) for x in ws};assert len(tdp)==1
        e=int(next(x['gathered_neighbors'] for x in ws if x['method']=='A1_step'))
        for x in ws:
            expect=2*e if x['method'] in ('A0_original','control_window16_duplicate_gather') else e
            assert int(x['gathered_neighbors'])==expect
            if x['method']=='A0_original':assert int(x['c_state_load_tiles'])==0
    assert re.search(r'Core\(s\) per socket:\s+32\b',(run/'cpu.txt').read_text())
    assert 'physcpubind: 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63' in (run/'numa_policy.txt').read_text()
assert len(binary)==1 and len(nodes)==1 and len(graphs)==4
report=dict(status='PASS',real_runs=len(runs),graphs=graphs,checks=checks,
            timing_rows=timings,binary_sha256=next(iter(binary)),node=next(iter(nodes)))
(root/'audit_results.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
