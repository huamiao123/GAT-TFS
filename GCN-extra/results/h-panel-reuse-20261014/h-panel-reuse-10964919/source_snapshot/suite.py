#!/usr/bin/env python3
import csv, hashlib, json, os, pathlib, statistics, struct, subprocess, sys, time

run, binary, mode = pathlib.Path(sys.argv[1]), pathlib.Path(sys.argv[2]), sys.argv[3]
root = pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
graph_root = pathlib.Path('/home/huangjianqiang_group/hdacp1/data/yx/TFS/data')

def sha(p):
    h=hashlib.sha256()
    with p.open('rb') as f:
        for b in iter(lambda:f.read(8*1024*1024),b''):h.update(b)
    return h.hexdigest()

def fixture(name,n):
    folder=run/'fixtures'/name;folder.mkdir(parents=True)
    row=[0];col=[]
    ds=[0,1,2,7,8,9,15,16,17,31,32,33,63,64,65,129]
    for i in range(n):
        d=ds[i%len(ds)]
        col.extend((i*7+j*11)%n for j in range(d));row.append(len(col))
    path=folder/(name+'.csrbin')
    with path.open('wb') as f:
        f.write(struct.pack('<IIIQQQ',0,0,2,n,n,len(col)))
        f.write(struct.pack('<'+'I'*len(row),*row))
        f.write(struct.pack('<'+'I'*len(col),*col))
        f.write(struct.pack('<'+'f'*len(col),*([1.]*len(col))))
    return folder.parent,name,path

targets = [fixture('tail_37',37),fixture('tail_137',137)] if mode=='smoke' else [
    (graph_root,name,graph_root/name/(name+'.csrbin')) for name in
    (['roadNet-CA','reddit'] if mode=='diagnostic1' else ['ogbn-products','mycielskian19'])]
summary=[];work=[];checks=[];manifest=[];failed=False
for parent,name,path in targets:
    print('START',name,flush=True)
    gdir=run/'logs'/name;gdir.mkdir(parents=True,exist_ok=True)
    argv=[str(binary),str(parent),name,'5']
    start=time.time()
    result=subprocess.run(argv,stdout=(gdir/'stdout.txt').open('w'),
                          stderr=(gdir/'stderr.txt').open('w'),timeout=1800)
    lines=(gdir/'stdout.txt').read_text(errors='replace').splitlines()
    def parse(prefix):
        return [dict(x.split('=',1) for x in line.split()[1:] if '=' in x)
                for line in lines if line.startswith(prefix+' ')]
    rows=parse('TIME');wc=parse('WORK');cc=parse('CHECK');complete=parse('COMPLETE')
    valid=result.returncode==0 and len(complete)==1 and complete[0].get('pass')=='1' and \
        len(cc)==7 and all(c.get('pass')=='1' for c in cc) and len(rows)==35
    for x in rows:x['graph']=name
    for x in wc:x['graph']=name
    for x in cc:x['graph']=name
    summary.extend(rows);work.extend(wc);checks.extend(cc)
    metadata=dict(graph=name,path=str(path),sha256=sha(path),argv=argv,exit_code=result.returncode,
                  elapsed_s=time.time()-start,checks=len(cc),time_rows=len(rows),pass_=valid)
    manifest.append(metadata);print('RESULT',json.dumps(metadata),flush=True)
    if not valid:failed=True
    if valid:
        groups={}
        for x in rows:groups.setdefault(x['method'],[]).append(float(x['total_ms']))
        print('MEDIANS',name,json.dumps({k:statistics.median(v) for k,v in groups.items()}),flush=True)
        print('SPEEDUP',name,json.dumps({k:statistics.median(groups['A0_original'])/statistics.median(v) for k,v in groups.items()}),flush=True)

for label,rows in [('timings.csv',summary),('work.csv',work),('checks.csv',checks)]:
    if rows:
        with (run/'results'/label).open('w',newline='') as f:
            w=csv.DictWriter(f,fieldnames=rows[0].keys());w.writeheader();w.writerows(rows)
manifest_record=dict(mode=mode,graphs=manifest,hostname=os.uname().nodename,
    job_id=os.environ.get('SLURM_JOB_ID'),shared=True,threads=os.environ.get('OMP_NUM_THREADS'),
    bind=os.environ.get('OMP_PROC_BIND'),places=os.environ.get('OMP_PLACES'),
    source_build=str(binary.parent),original_source_sha256=sha(run/'source_snapshot'/'original_gcn_e2e_v3.cpp'),
    binary_sha256=sha(binary),input_initial_H_prepared_outside_time=True,repeats=5,
    order='forward/reverse alternating',all_pass=not failed,paper_eligible=False)
(run/'manifest.json').write_text(json.dumps(manifest_record,indent=2)+'\n')
event=dict(id=run.name,kind='h_panel_reuse_shared_'+mode,status='PASS' if not failed else 'FAILED',
    job_id=os.environ.get('SLURM_JOB_ID'),node=os.uname().nodename,evidence=str(run),
    purpose='H random gather versus C tile residency in original TFS',
    graph_results=manifest,source_build=str(binary.parent),paper_eligible=False,
    notes='Shared node diagnostic; uninstrumented prepared two-layer alternating 5 repeats; A0 original included source.')
(run/'event.json').write_text(json.dumps(event,indent=2)+'\n')
subprocess.run(['python3',str(root/'scripts'/'record_event.py'),str(run/'event.json')],check=True)
print('COMPLETE',json.dumps(dict(mode=mode,pass_=not failed)),flush=True)
sys.exit(1 if failed else 0)
