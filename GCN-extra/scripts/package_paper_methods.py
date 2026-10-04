#!/usr/bin/env python3
import hashlib,json,pathlib,subprocess,tarfile
root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
run=root/'runs/paper-method-reconciled-20261004';assert json.loads((run/'audit_summary.json').read_text())['graphs']==17
cache=root/'runs/paper-cache-reconciled-20261004';assert json.loads((cache/'audit_summary.json').read_text())['graphs']==17
archive=root/'evidence-paper-methods-controlled-20261004.tar.gz';assert not archive.exists()
event=dict(id='paper-method-package-20261004',kind='controlled_method_archive',status='RECONCILED_READY_FOR_ARCHIVE',evidence=str(run),archive=str(archive),paper_eligible=True,issues='All numerical gates and slow cases retained; no cross-node or historical denominator',next='Verify local mirrors and publish existing isolated experiment branch')
(run/'package.event.json').write_text(json.dumps(event,indent=2));subprocess.run(['python3',str(root/'scripts/record_event.py'),str(run/'package.event.json')],check=True)
paths=[root/'README.md',root/'.gitignore',root/'docs/handoff',root/'docs/PAPER_METHODS_PROTOCOL_20261004.md',root/'docs/PAPER_METHODS_RESULTS_20261004.md',root/'src/paper_methods_runtime.hpp']
paths+=list((root/'runs').glob('paper-method-*'))
paths+=list((root/'runs').glob('paper-cache-*'))
paths+=list((root/'scripts').glob('*paper_methods*'))
paths+=list((root/'scripts').glob('*paper_cache_control*'))
paths+=[root/'scripts/analyze_paper_variability.py']
paths+=[root/'scripts/audit_paper_publication.py']
paths+=[root/'scripts/update_paper_methods_readme.py']
paths+=[root/'scripts/adjust_paper_cache_limits.sh',root/'scripts/paper_cache_progress.py']
paths+=[root/'scripts/record_paper_publication.py']
paths+=[root/'docs/PAPER_CACHE_CONTROL_RESULTS_20261004.md',root/'docs/PAPER_METHODS_INTERPRETATION_20261004.md',root/'docs/figures/paper_methods_20261004']
manifest=[]
for p in paths:
 for q in ([p] if p.is_file() else sorted(p.rglob('*'))):
  if q.is_file() and q.name!='.handoff.lock':manifest.append(dict(path=q.relative_to(root).as_posix(),sha256=hashlib.sha256(q.read_bytes()).hexdigest(),bytes=q.stat().st_size))
(run/'archive_manifest.json').write_text(json.dumps(manifest,indent=2))
with tarfile.open(str(archive),'w:gz') as tar:
 for p in paths:tar.add(str(p),arcname=p.relative_to(root).as_posix(),filter=lambda x:None if pathlib.PurePosixPath(x.name).name=='.handoff.lock' else x)
digest=hashlib.sha256(archive.read_bytes()).hexdigest();(root/(archive.name+'.sha256')).write_text(digest+'  '+archive.name+'\n')
print(json.dumps(dict(archive=str(archive),sha256=digest,bytes=archive.stat().st_size,manifest_files=len(manifest))))
