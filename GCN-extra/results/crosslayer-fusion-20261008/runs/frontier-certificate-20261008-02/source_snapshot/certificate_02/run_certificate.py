import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import traceback

ROOT=Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
PARENT=ROOT/'runs/frontier-structure-20261008-01/compute-10898893'
study,run=map(Path,sys.argv[1:3])
source=study/'source_snapshot'
child=source/'certificate_02'

def save(path,obj):
    with path.open('x',encoding='utf-8') as stream:
        json.dump(obj,stream,indent=2);stream.write('\n')

def sha(path):
    h=hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda:stream.read(8*1024*1024),b''):h.update(block)
    return h.hexdigest()

def invoke(directory,label,argv):
    cmd=[str(x) for x in argv];save(directory/(label+'.command.json'),cmd)
    with (directory/(label+'.stdout.log')).open('wb') as out,(directory/(label+'.stderr.log')).open('wb') as err:
        ret=subprocess.run(cmd,stdout=out,stderr=err,cwd=run/'build')
    save(directory/(label+'.exit.json'),dict(exit_code=ret.returncode))
    assert ret.returncode==0,(label,ret.returncode)

def main():
    manifest=json.loads((study/'snapshot_manifest.json').read_text())
    for path,digest in manifest['authority_sha256'].items():assert sha(Path(path))==digest,path
    for name,digest in manifest['authority_spec_sha256'].items():
        assert sha(ROOT.parent/'TFS-Train/docs/spec'/name)==digest,name
    assert sha(ROOT/'IMPLEMENTATION_CONTRACT.md')==manifest['contract_sha256']
    for name,digest in manifest['source_sha256'].items():assert sha(source/name)==digest,name
    for path,digest in manifest['parent_evidence_sha256'].items():assert sha(Path(path))==digest,path
    (run/'build').mkdir()
    binary=run/'build/frontier_certificate';reference=run/'build/frontier_structure'
    for name,src,out in [('reference',source/'frontier_structure.cpp',reference),
                         ('certificate',child/'frontier_certificate.cpp',binary)]:
        invoke(run,'compile_'+name,['icpx','-O3','-std=c++17','-qopenmp',src,'-lstdc++fs','-o',out])
    save(run/'binary_identity.json',{str(p):sha(p) for p in [binary,reference]})
    invoke(run,'selftest',['python3',child/'verify_certificate.py','--self-test','--binary',binary,
                         '--reference-binary',reference,'--out',run/'selftest'])
    tests=json.loads((run/'selftest/status.json').read_text())
    assert tests['status']=='PASS' and len(tests['valid'])==5 and len(tests['mutations'])==5,tests
    assert all(x['status']=='PASS' for x in tests['valid'])
    assert all(x['status']=='REJECTED' for x in tests['mutations'])
    prior=json.loads((source/'frontier_prior_inputs.json').read_text())
    graphs=[g for g in prior['graphs'] if g['graph'] in ['mycielskian19','reddit']]
    assert len(graphs)==2
    states=[]
    for graph in sorted(graphs,key=lambda g:g['E']):
        name=graph['graph'];folder=run/name;folder.mkdir();path=Path(graph['path'])
        parent=PARENT/name;structure=json.loads((parent/'structure.json').read_text())
        order=next(o for o in structure['orders'] if o['name']=='nodeid')
        late=next(m for m in order['methods'] if m['name']=='row_complete')
        batches=order['producer_batches'];peak=late['peak_after_drain_rank']
        frontiers=sorted({max(1,min(batches,p)) for p in [batches//4,batches//2,3*batches//4,peak-1,peak,peak+1]})
        save(folder/'selection.json',dict(rule='quarter/half/threequarters and parent postdrain peak +/-1',
             frontiers=frontiers,parent_peak_after_drain_rank=peak,batches=batches))
        before=path.stat();actual=sha(path)
        assert actual==graph['input_sha256'],('input SHA',name)
        save(folder/'input_identity.json',dict(path=str(path),graph=graph,actual_sha256=actual,
             size=before.st_size,mtime_ns=before.st_mtime_ns,inode=before.st_ino))
        print('CERTIFICATE_START',name,frontiers,flush=True)
        invoke(folder,'native',[binary,'--csr',path,'--out',folder/'native',
               '--frontiers',','.join(map(str,frontiers)),'--graph',name])
        native=json.loads((folder/'native/summary.json').read_text())
        assert [f['p'] for f in native['frontiers']]==frontiers
        invoke(folder,'verify',['python3',child/'verify_certificate.py','--csr',path,
               '--certificate-dir',folder/'native','--structure',parent/'structure.json',
               '--structure-status',parent/'status.json','--input-identity',folder/'input_identity.json',
               '--out',folder/'verified.json'])
        after=path.stat()
        assert (before.st_size,before.st_mtime_ns,before.st_ino)==(after.st_size,after.st_mtime_ns,after.st_ino)
        verified=json.loads((folder/'verified.json').read_text());assert verified['status']=='PASS'
        state=dict(graph=name,status='PASS',frontiers=frontiers,input_sha256=actual,
                   verified_sha256=sha(folder/'verified.json'),summary_sha256=sha(folder/'native/summary.json'))
        save(folder/'status.json',state);states.append(state)
        print('CERTIFICATE_DONE',name,flush=True)
    save(run/'completion.json',dict(status='PASS',scope='RESTRICTED_STATE_LOWER_BOUND',graphs=states,
         self_test=tests,source_manifest_sha256=sha(study/'snapshot_manifest.json'),
         node=os.uname().nodename,job=os.environ.get('SLURM_JOB_ID'),
         cpu_affinity=sorted(os.sched_getaffinity(0)),desktop_write=False,numa_interleave=False,
         performance_claim=False))

try:main()
except Exception:
    save(run/'failure.json',dict(status='FAILED',traceback=traceback.format_exc()));raise
