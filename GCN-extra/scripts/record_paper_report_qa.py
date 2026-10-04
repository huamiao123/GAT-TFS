#!/usr/bin/env python3
"""Record reporting-only revisions after the immutable evidence archive."""
import hashlib,json,pathlib,subprocess
root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
manifest=json.loads((root/'runs/paper-method-reconciled-20261004/archive_manifest.json').read_text())
changed=[x['path'] for x in manifest if hashlib.sha256((root/x['path']).read_bytes()).hexdigest()!=x['sha256']]
allowed=['scripts/plot_paper_methods.py','docs/figures/paper_methods_20261004/execution_order_control.png','docs/figures/paper_methods_20261004/execution_order_control.pdf']
assert set(changed)==set(allowed),changed
run=root/'runs/paper-method-report-qa-20261004';assert not run.exists();run.mkdir()
hashes={p:hashlib.sha256((root/p).read_bytes()).hexdigest() for p in allowed}
e=dict(id=run.name,kind='post_archive_report_visual_qa',status='PASS',evidence=str(run),paper_eligible=False,changed_report_assets=hashes,benchmark_and_numerical_evidence_unchanged=True,mirror_verified_files=1176,issues='Initial local mirror hash check caught a concurrent plot layout edit; re-extraction succeeded and both mirrors verified all 1176 manifest files. Only then the legend was moved outside bars to expose final row numbers; PNG visually reviewed. A stalled upload connection left only the renderer updated, which the reporting-only hash gate rejected before recording success. Retry with BatchMode and ConnectTimeout=15 transferred both assets. Immutable archive retains its original reporting snapshot.',next='Publish verified benchmark evidence plus clearly recorded reporting-only revision')
(run/'event.json').write_text(json.dumps(e,indent=2));subprocess.run(['python3',str(root/'scripts/record_event.py'),str(run/'event.json')],check=True)
print(json.dumps(e,indent=2))
