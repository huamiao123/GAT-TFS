#!/usr/bin/env python3
"""Safe, hash-checked Windows import, preserving published experiment history."""
import hashlib,json,pathlib,shutil,tarfile
local=pathlib.Path(__file__).resolve().parents[1]
publication=local.parent.parent/'GAT-TFS-publish-GCNextra-20261003/GCN-extra'
archive=local/'evidence-paper-reproduction-20261004.tar.gz'
expected=(local/(archive.name+'.sha256')).read_text().split()[0]
assert hashlib.sha256(archive.read_bytes()).hexdigest()==expected
with tarfile.open(archive,'r:gz') as tar:
 members=tar.getmembers()
 for m in members:
  p=pathlib.PurePosixPath(m.name)
  assert not p.is_absolute() and '..' not in p.parts and '\\' not in m.name
  assert m.isfile() or m.isdir()
 history=tar.extractfile('docs/handoff/events.jsonl').read()
 assert history.startswith((publication/'docs/handoff/events.jsonl').read_bytes())
 for target in [local,publication]:
  target=target.resolve()
  for m in members:assert (target/m.name).resolve().is_relative_to(target)
  tar.extractall(target,filter='data')
  manifest=json.loads((target/'runs/paper-formal-10864848/archive_manifest.json').read_text())
  for r in manifest:assert hashlib.sha256((target/r['path']).read_bytes()).hexdigest()==r['sha256'],r['path']
  print('PAPER_MIRROR_VERIFIED',target,'members',len(members),'hashes',len(manifest))
shutil.copyfile(archive,publication/archive.name)
shutil.copyfile(local/(archive.name+'.sha256'),publication/(archive.name+'.sha256'))
