import hashlib
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
study, run = map(Path, sys.argv[1:3])
src = study / 'source_snapshot'
build = run / 'build'
result = run / 'result'

def sha(path):
    h = hashlib.sha256()
    with Path(path).open('rb') as f:
        for block in iter(lambda: f.read(8 * 1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()

def save(path, value):
    with Path(path).open('x') as f:
        json.dump(value, f, indent=2)
        f.write('\n')

def call(label, argv, cwd=None):
    save(run / (label + '.command.json'), [str(x) for x in argv])
    with (run / (label + '.stdout.log')).open('wb') as out, (run / (label + '.stderr.log')).open('wb') as err:
        proc = subprocess.run([str(x) for x in argv], cwd=cwd, stdout=out, stderr=err)
    save(run / (label + '.exit.json'), {'exit_code': proc.returncode})
    assert proc.returncode == 0, (label, proc.returncode)

def main():
    manifest = json.loads((study / 'snapshot_manifest.json').read_text())
    for name, digest in manifest['source_sha256'].items():
        assert sha(src / name) == digest, name
    for path, digest in manifest['authority_sha256'].items():
        assert sha(path) == digest, path
    for name, digest in manifest['authority_spec_sha256'].items():
        assert sha(ROOT.parent / 'TFS-Train/docs/spec' / name) == digest, name
    assert sha(ROOT / 'IMPLEMENTATION_CONTRACT.md') == manifest['contract_sha256']
    build.mkdir()
    result.mkdir()
    binary = build / 'recompute_scan'
    call('compile', ['icpx', '-O3', '-std=c++17', src / 'recompute_scan.cpp', '-o', binary], cwd=build)
    save(run / 'binary_identity.json', {'path': str(binary), 'sha256': sha(binary)})
    prior = json.loads((src / 'frontier_prior_inputs.json').read_text())
    outputs = []
    for graph in sorted(prior['graphs'], key=lambda item: item['E']):
        output = result / (graph['graph'] + '.json')
        call('scan_' + graph['graph'], [binary, str(output), graph['path'], graph['graph']], cwd=build)
        data = json.loads(output.read_text())
        assert data['status'] == 'PASS'
        outputs.append(data)
        print('DONE', graph['graph'], flush=True)
    save(result / 'audit.json', {
        'schema': 'recompute-audit-v2', 'status': 'PASS', 'model': '128->128->128',
        'results': outputs, 'source_manifest_sha256': sha(study / 'snapshot_manifest.json'),
        'performance_claim': False, 'no_numa_interleave': True,
        'limitations': ['logical edge/projection work only', 'no DRAM/cache/synchronization',
                        'optimistic one regeneration per consumer rank', 'no legal AMX schedule proof']
    })

if __name__ == '__main__':
    main()
