#!/usr/bin/env python3
import datetime,hashlib,json,pathlib,subprocess,tarfile
root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
run=root/'runs/paper-formal-10864848';assert json.loads((run/'completion.json').read_text())['passed']==17
assert (run/'reconciliation.event.json').exists()
archive=root/'evidence-paper-reproduction-20261004.tar.gz';assert not archive.exists()
files=[root/'README.md',root/'.gitignore',root/'docs/PAPER_REPRODUCTION_RESULTS_20261004.md',root/'docs/handoff']
files+=list((root/'scripts').glob('*paper*'))
files+=list((root/'runs').glob('paper-*'))
if (root/'docs/paper_reference').exists():files.append(root/'docs/paper_reference')
manifest=[]
for p in files:
 for q in ([p] if p.is_file() else sorted(p.rglob('*'))):
  if q.is_file() and q.name!='.handoff.lock':
   manifest.append(dict(path=q.relative_to(root).as_posix(),sha256=hashlib.sha256(q.read_bytes()).hexdigest(),bytes=q.stat().st_size))
# Record packaging before capturing the final synchronized handoff history.
event=dict(id='paper-package-20261004-10864848',kind='paper_reproduction_archive',status='VERIFIED_RUN_READY_FOR_ARCHIVE',archive=str(archive),evidence=str(run),paper_eligible=True,issues='No new issue; baseline version correction and historical discrepancies retained',next='Mirror archive, verify hashes, publish on existing isolated experiment branch')
(run/'package.event.json').write_text(json.dumps(event,indent=2))
subprocess.run(['python3',str(root/'scripts/record_event.py'),str(run/'package.event.json')],check=True)
manifest=[]
for p in files:
 for q in ([p] if p.is_file() else sorted(p.rglob('*'))):
  if q.is_file() and q.name!='.handoff.lock':manifest.append(dict(path=q.relative_to(root).as_posix(),sha256=hashlib.sha256(q.read_bytes()).hexdigest(),bytes=q.stat().st_size))
(run/'archive_manifest.json').write_text(json.dumps(manifest,indent=2))
with tarfile.open(str(archive),'w:gz') as tar:
 for p in files:tar.add(str(p),arcname=p.relative_to(root).as_posix(),filter=lambda x: None if pathlib.PurePosixPath(x.name).name=='.handoff.lock' else x)
digest=hashlib.sha256(archive.read_bytes()).hexdigest()
(root/(archive.name+'.sha256')).write_text(digest+'  '+archive.name+'\n')
print(json.dumps(dict(archive=str(archive),sha256=digest,bytes=archive.stat().st_size,manifest_files=len(manifest))))
