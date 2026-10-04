#!/usr/bin/env python3
"""Import the verified source-protocol archive into local mirrors only."""
import hashlib
import json
import pathlib
import shutil
import tarfile

local=pathlib.Path(__file__).resolve().parents[1]
publication=pathlib.Path('C:/Users/花喵/GAT-TFS-publish-GCNextra-20261003/GCN-extra')
archive=local/'evidence-source-mkl-protocol-20261004.tar.gz'
expected='ad34614e9885ec5bba852ab2834da6c96992f12256172ab8c7c8b41fc7e2ba6c'
assert hashlib.sha256(archive.read_bytes()).hexdigest()==expected
with tarfile.open(archive,'r:gz') as tar:
    members=tar.getmembers()
    for m in members:
        p=pathlib.PurePosixPath(m.name)
        assert not p.is_absolute() and '..' not in p.parts and '\\' not in m.name
        assert m.isfile() or m.isdir(), 'Refuse symlinks, links or special members'
    history=tar.extractfile('docs/handoff/events.jsonl').read()
    previous=(publication/'docs/handoff/events.jsonl').read_bytes()
    assert history.startswith(previous), 'Do not replace published handoff history'
    for target in [local,publication]:
        target=target.resolve()
        for m in members:
            assert (target/m.name).resolve().is_relative_to(target)
        tar.extractall(target,filter='data')
        manifest=json.loads((target/'runs/source-formal-10864754/final_source_manifest.json').read_text())
        for record in manifest:
            assert hashlib.sha256((target/record['path']).read_bytes()).hexdigest()==record['sha256'],record['path']
        print('SOURCE_MIRROR_VERIFIED',target,'members',len(members),'manifest_files',len(manifest))
shutil.copyfile(archive,publication/archive.name)
shutil.copyfile(local/(archive.name+'.sha256'),publication/(archive.name+'.sha256'))
shutil.copyfile(pathlib.Path(__file__),publication/'scripts/mirror_source_evidence.py')
(publication/'docs/PUBLICATION_SOURCE_PROTOCOL_20261004.md').write_text('''# Original-source protocol publication (2026-10-04)

Branch: `gcn-extra-experiments-20261003`; preceding snapshot: `1de29e2`.
The current addition preserves all historical evidence and marks the previous
explicit NUMA interleave protocol as distinct from original source reproduction.

- Authority/code: `original/`, `src/source_protocol.cpp`, source protocol headers.
- Protocol: `docs/SOURCE_MKL_PROTOCOL_20261004.md`.
- Source/NUMA hashes: `docs/SOURCE_POLICY_AUDIT_20261004.json`.
- Results: `docs/SOURCE_MKL_RESULTS_20261004.md`.
- Evidence: `runs/source-formal-10864754`, `runs/source-formal-10864792`.
- Correctness: smoke 10864752 and post-launcher-fix smoke 10864830.
- Historical negative evidence: failed first generation build; first RGG range
  exclusion; FAILED main batch shell completion after all 16 graph programs
  completed successfully. The supplement adds RGG without changing the binary.
- Actual Slurm states: `runs/source-formal-10864754/source_slurm_accounting.txt`.

17 valid graphs, 765 numerical checks; eight excluded inputs. FP32 source MKL
versus BF16 source TFS/candidates, one warmup and minimum of five. Prepared
two-layer E2E excludes initial setup; random H/W is not trained task accuracy.
Every raw repetition, numerical comparison, stage/profile, runtime environment,
source snapshot and binary/input hash is retained. Standalone binaries are kept
inside the complete archive rather than duplicated as Git files.

Complete archive: `evidence-source-mkl-protocol-20261004.tar.gz`.
SHA256: `'''+expected+'''`.
The archive includes binaries and all source-protocol runs, failures and handoffs.
Local import validates safe relative paths, historical event prefix and all final
source manifest hashes; `.gitattributes` preserves byte hashes in this branch.
''',encoding='utf-8',newline='\n')
