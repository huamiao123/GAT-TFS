#!/usr/bin/env python3
"""Run all canonical graphs with the same benchmark binary; retain failures."""
import csv
import hashlib
import json
import os
import pathlib
import resource
import signal
import subprocess
import sys
import time

run, build = map(pathlib.Path,sys.argv[1:3])
resource.setrlimit(resource.RLIMIT_CORE,(0,0))
local_nodes = sys.argv[3]
graphs = json.loads((run/'graph_list.json').read_text(encoding='utf-8'))
# Finish smaller graphs before the largest; otherwise descending average degree.
graphs.sort(key=lambda x:(x['budget_gib']>200,-x['avg_degree']))
results=[]
model=(build/'mkl_integer_model.txt').read_text().strip() if (build/'mkl_integer_model.txt').exists() else 'LP64'
for r in graphs:
    name=r['graph'];sub=run/name;sub.mkdir()
    if model=='LP64' and (r['E']>2147483647 or r['N']*128>2147483648):
        st=dict(r,status='SKIPPED',reason='LP64 index boundary; select a validated ILP64 build before loading large arrays')
        (sub/'status.json').write_text(json.dumps(st,indent=2),encoding='utf-8');results.append(st)
        (run/'suite_status.json').write_text(json.dumps(results,indent=2),encoding='utf-8');continue
    if not r['supported_header'] or r['budget_gib']>440:
        r=dict(r,status='SKIPPED',reason='unsupported header or conservative memory budget >440 GiB')
        (sub/'status.json').write_text(json.dumps(r,indent=2),encoding='utf-8');results.append(r);continue
    nodes=local_nodes
    if r['budget_gib']>200:
        # Large dense validation arrays exceed one socket's capacity. All methods
        # receive the same process-local interleave policy across online domains.
        nodes=','.join(p.name[4:] for p in sorted(pathlib.Path('/sys/devices/system/node').glob('node[0-9]*')) if (p/'cpulist').read_text().strip())
    cmd=['numactl','--interleave='+nodes,str(build/'gcn_bench'),'--graph',r['path'],'--name',name,
         '--block-controls','--blocks',os.getenv('GCN_BLOCKS','2,4,8,16,32,64,full'),
         '--no-replay','--repeats',os.getenv('GCN_REPEATS','10'),'--warmups','1','--out',str(sub)]
    (sub/'command.json').write_text(json.dumps({'argv':cmd,'memory_nodes':nodes,'estimated_memory_gib':r['budget_gib']},indent=2),encoding='utf-8')
    start=time.time();print('START_GRAPH',name,'q',r['avg_degree'],'budget_gib',r['budget_gib'],'nodes',nodes,flush=True)
    sha=hashlib.sha256()
    with open(r['path'],'rb') as f:
        while True:
            chunk=f.read(8*1024*1024)
            if not chunk:break
            sha.update(chunk)
    with (run/'dataset.sha256').open('a',encoding='utf-8') as f:f.write(sha.hexdigest()+'  '+r['path']+'\n')
    with (sub/'stdout.log').open('w') as so,(sub/'stderr.log').open('w') as se:
        p=subprocess.run(cmd,stdout=so,stderr=se,cwd=sub)
    status=dict(r,exit_code=p.returncode,status='PASS' if p.returncode==0 else 'FAILED',elapsed_s=time.time()-start,memory_nodes=nodes,dataset_sha256=sha.hexdigest())
    if p.returncode!=0:
        status['reason']=(sub/'stderr.log').read_text(errors='replace').strip()[-2000:]
        if not status['reason'] and p.returncode<0:status['reason']='terminated by signal '+signal.Signals(-p.returncode).name
    (sub/'status.json').write_text(json.dumps(status,indent=2),encoding='utf-8');results.append(status)
    (run/'suite_status.json').write_text(json.dumps(results,indent=2),encoding='utf-8')
    print('END_GRAPH',name,status['status'],status['elapsed_s'],status.get('reason',''),flush=True)
# Batch completion is not the same as all datasets passing; list every failure.
(run/'suite_status.json').write_text(json.dumps(results,indent=2),encoding='utf-8')
print('SUITE_COMPLETE',len(results),'passed',sum(r['status']=='PASS' for r in results),flush=True)
