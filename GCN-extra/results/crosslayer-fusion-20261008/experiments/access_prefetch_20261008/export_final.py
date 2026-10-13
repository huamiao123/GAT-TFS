from pathlib import Path
import hashlib, json, shutil
root=Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
res=root/'results/access_prefetch_20261008'
a=res/'analysis-01'
d=res/'final'
report=json.loads((a/'audit.json').read_text())
assert report['analysis_pass'] and len(report['packets'])==27
assert len(report['jobs'])==7 and all(j['valid'] for j in report['jobs'])
assert report['counts']['timings']==2730
assert not d.exists(), 'New delivery directory required'
d.mkdir()
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
for name in ('summary.csv','ratios.csv','interactions.csv','pmu_summary.csv','setup.csv','expansion_gate.json'):
    shutil.copy2(a/name,d/name)
jobs=[{k:j.get(k) for k in ('job','node','partition','exclusive','mode','valid','slurm')} for j in report['jobs']]
compact=dict(analysis_pass=True,packet_count=len(report['packets']),counts=report['counts'],jobs=jobs,
             provenance=dict(audit_sha256=sha(a/'audit.json'),analysis_script_sha256=report['source_script_sha256'],
                             helper=report['helper'],build_fingerprint=report['build']['build_fingerprint'],
                             binary_hashes=report['build']['manifest']['binary_hashes'],
                             assembly_identity_sha256=sha(root/'experiments/access_prefetch_20261008/assembly_identity.json')))
(d/'compact_audit.json').write_text(json.dumps(compact,ensure_ascii=False,indent=2)+'\n')
manifest={p.name:sha(p) for p in d.iterdir() if p.is_file()}
(d/'delivery_input_hashes.json').write_text(json.dumps(manifest,sort_keys=True,indent=2)+'\n')
print(json.dumps(dict(output=str(d),analysis_pass=True,input_hashes=manifest),ensure_ascii=False))
