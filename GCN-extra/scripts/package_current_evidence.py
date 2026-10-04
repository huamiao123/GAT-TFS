#!/usr/bin/env python3
"""Package only the cleaned current source and controlled experiment evidence."""
import hashlib,json,pathlib,tarfile,sys
sys.dont_write_bytecode=True
from cleanup_current_evidence import ROOT,ARCHIVE,CLEAN_RUN,obsolete,sha
assert not obsolete(ROOT)
archive=ROOT/ARCHIVE
assert not archive.exists()
manifest_path=ROOT/'runs'/CLEAN_RUN/'archive_manifest.json'
files=[p for p in sorted(ROOT.rglob('*')) if p.is_file() and
       p.name!='.handoff.lock' and '__pycache__' not in p.parts and
       p!=manifest_path and p!=archive and p!=ROOT/(ARCHIVE+'.sha256')]
manifest=[{'path':p.relative_to(ROOT).as_posix(),'sha256':sha(p),'bytes':p.stat().st_size} for p in files]
manifest_path.write_text(json.dumps(manifest,indent=2)+'\n')
with tarfile.open(archive,'w:gz') as tar:
    for p in files+[manifest_path]:tar.add(p,arcname=p.relative_to(ROOT).as_posix(),recursive=False)
digest=sha(archive);(ROOT/(ARCHIVE+'.sha256')).write_text(digest+'  '+ARCHIVE+'\n')
with tarfile.open(archive,'r:gz') as tar:
    assert len(tar.getmembers())==len(manifest)+1
    for x in manifest:
        assert hashlib.sha256(tar.extractfile(x['path']).read()).hexdigest()==x['sha256'],x['path']
print(json.dumps({'archive':str(archive),'sha256':digest,'manifest_files':len(manifest),'bytes':archive.stat().st_size}))
