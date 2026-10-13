import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import time
import traceback

study = Path(sys.argv[1])
run = Path(sys.argv[2])
source = study / 'source_snapshot'
binary = run / 'build' / 'frontier_structure'

def save(path, value):
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')

def sha(path):
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(8*1024*1024), b''):
            h.update(block)
    return h.hexdigest()

def invoke(label, argv, cwd=None):
    save(run / (label + '.command.json'), [str(x) for x in argv])
    with (run / (label + '.stdout.log')).open('wb') as out, (run / (label + '.stderr.log')).open('wb') as err:
        ret = subprocess.run([str(x) for x in argv], stdout=out, stderr=err, cwd=cwd)
    save(run / (label + '.exit.json'), {'exit_code': ret.returncode})
    if ret.returncode:
        raise RuntimeError(label + ' failed: ' + str(ret.returncode))

def main():
    manifest = json.loads((study / 'snapshot_manifest.json').read_text())
    root=Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
    for path,digest in manifest['authority_sha256'].items():
        assert sha(Path(path))==digest,('authority changed',path)
    for name,digest in manifest['authority_spec_sha256'].items():
        assert sha(root.parent/'TFS-Train/docs/spec'/name)==digest,('spec changed',name)
    assert sha(root/'IMPLEMENTATION_CONTRACT.md')==manifest['contract_sha256']
    for name, digest in manifest['source_sha256'].items():
        assert sha(source / name) == digest, ('source hash', name)
    (run / 'build').mkdir()
    invoke('compile', ['icpx','-O3','-std=c++17','-qopenmp',
                       source / 'frontier_structure.cpp','-o',binary], cwd=run/'build')
    save(run / 'binary_identity.json', {'path': str(binary), 'sha256': sha(binary)})
    invoke('oracle', ['python3',source / 'audit_frontier.py','--binary',binary,'--out',run/'oracle'])
    oracle = json.loads((run / 'oracle' / 'status.json').read_text())
    assert oracle['status'] == 'PASS', oracle
    assert oracle['fixture_count']==81 and oracle['directed_fixtures']==17 and oracle['random_fixtures']==64
    assert oracle['malformed_count']==6
    assert oracle['script_sha256']==manifest['source_sha256']['audit_frontier.py']
    prior = json.loads((source / 'frontier_prior_inputs.json').read_text())
    items = prior['graphs']
    assert {g['graph'] for g in items} == {'mycielskian19','reddit','roadNet-CA','amazon0601'}
    # Small graphs first; no result-dependent change of cohort or policy.
    items = sorted(items, key=lambda g:g['E'])
    states=[]
    for graph in items:
        folder = run / graph['graph']
        folder.mkdir()
        path = Path(graph['path'])
        before = path.stat()
        actual = sha(path)
        assert actual == graph['input_sha256'], ('input hash', graph['graph'])
        save(folder / 'input_identity.json', dict(graph=graph, actual_sha256=actual,
             size=before.st_size, mtime_ns=before.st_mtime_ns, inode=before.st_ino))
        print('FRONTIER_START',graph['graph'],flush=True)
        cmd=[binary,'--csr',path,'--out',folder/'structure.json','--graph',graph['graph'],
             '--orders','nodeid,degree,qguard','--checkpoints','1,4,16,whole']
        save(folder/'command.json',[str(x) for x in cmd])
        start=time.monotonic()
        with (folder/'stdout.log').open('wb') as out, (folder/'stderr.log').open('wb') as err:
            ret=subprocess.run([str(x) for x in cmd],stdout=out,stderr=err)
        save(folder/'process_exit.json',{'exit_code':ret.returncode})
        assert ret.returncode==0,('analyzer',graph['graph'],ret.returncode)
        after=path.stat()
        assert (before.st_size,before.st_mtime_ns,before.st_ino)==(after.st_size,after.st_mtime_ns,after.st_ino)
        data=json.loads((folder/'structure.json').read_text())
        # Schema and previous schedule checks are delegated to a small separate
        # real-output validator, also frozen and independent of C++ analysis.
        with (folder/'validation.log').open('w') as audit_log:
            subprocess.run(['python3',str(source/'validate_real.py'),str(folder/'structure.json'),
                            str(source/'frontier_prior_inputs.json'),graph['graph']],check=True,
                            stdout=audit_log,stderr=subprocess.STDOUT)
        state=dict(graph=graph['graph'],status='PASS',input_sha256=actual,
                   elapsed_diagnostic_seconds=time.monotonic()-start,
                   structure_sha256=sha(folder/'structure.json'))
        save(folder/'status.json',state);states.append(state)
        print('FRONTIER_DONE',graph['graph'],flush=True)
    save(run/'completion.json',dict(status='PASS',scope='STRUCTURE_ONLY',graphs=states,
         oracle_status=oracle,source_manifest_sha256=sha(study/'snapshot_manifest.json'),
         node=os.uname().nodename,job=os.environ.get('SLURM_JOB_ID'),
         cpu_affinity=sorted(os.sched_getaffinity(0)),amx_performance_claim=False,desktop_write=False))

try:
    main()
except Exception:
    save(run/'failure.json',dict(status='FAILED',traceback=traceback.format_exc()))
    raise
