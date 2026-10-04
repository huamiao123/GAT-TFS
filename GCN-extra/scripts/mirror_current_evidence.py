#!/usr/bin/env python3
"""Safely verify/extract cleaned evidence into an already cleaned local mirror.

Usage: python mirror_current_evidence.py ARCHIVE GCN_EXTRA_ROOT
Local deletions must first use the scoped PowerShell cleanup plan.
"""
import hashlib,json,pathlib,sys,tarfile
archive=pathlib.Path(sys.argv[1]).resolve();target=pathlib.Path(sys.argv[2]).resolve()
assert target.is_dir() and target.name=='GCN-extra'
expected=pathlib.Path(str(archive)+'.sha256').read_text().split()[0]
assert hashlib.sha256(archive.read_bytes()).hexdigest()==expected
with tarfile.open(archive,'r:gz') as tar:
    members=tar.getmembers()
    for m in members:
        p=pathlib.PurePosixPath(m.name)
        assert not p.is_absolute() and '..' not in p.parts and '\\' not in m.name and m.isfile()
        assert (target/m.name).resolve().is_relative_to(target)
    tar.extractall(target,filter='data')
manifest=json.loads((target/'runs/cleanup-current-20261004/archive_manifest.json').read_text())
for x in manifest:
    assert hashlib.sha256((target/x['path']).read_bytes()).hexdigest()==x['sha256'],x['path']
print(json.dumps({'mirror':str(target),'verified_files':len(manifest),'sha256':expected}))
