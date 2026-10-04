#!/usr/bin/env python3
import hashlib,json,pathlib,re,subprocess,sys
root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
commit=sys.argv[1];assert re.fullmatch('[0-9a-f]{40}',commit)
run=root/'runs/paper-method-publication-20261004';assert not run.exists();run.mkdir()
archive=root/'evidence-paper-methods-controlled-20261004.tar.gz'
digest=hashlib.sha256(archive.read_bytes()).hexdigest();assert digest==(root/(archive.name+'.sha256')).read_text().split()[0]
e=dict(id=run.name,kind='controlled_methods_github_publication',status='PUBLISHED_VERIFIED',source_evidence_commit=commit,branch='gcn-extra-experiments-20261003',repository='https://github.com/huamiao123/GAT-TFS',archive_sha256=digest,paper_eligible=True,evidence=str(run),issues='17 graphs, original paper v3/source MKL, both measurement orders, fixed gates and negative results published; archive captures pre-publication handoff snapshot and publication is a subsequent history event',next='Controlled remeasurement complete; future mechanism/PMU/shape/checkpoint experiments remain separate scope')
(run/'event.json').write_text(json.dumps(e,indent=2));subprocess.run(['python3',str(root/'scripts/record_event.py'),str(run/'event.json')],check=True)
print(json.dumps(e,indent=2))
