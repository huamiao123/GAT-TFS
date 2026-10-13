"""Verify provenance and package derived evidence without changing raw files."""
import csv
import datetime
import hashlib
import json
import pathlib
import subprocess
import sys
import tarfile

root=pathlib.Path(sys.argv[1]).resolve()
repo=pathlib.Path(sys.argv[2]).resolve()


def verify_inventory(folder, filename):
    inventory=json.loads((folder/filename).read_text(encoding='utf-8'))
    for name,digest in inventory.items():
        assert hashlib.sha256((folder/name).read_bytes()).hexdigest()==digest, name


verify_inventory(root,'artifact_hashes.json')
verify_inventory(root/'superseded_smoke','artifact_hashes.json')
metadata=[]
for part in ['p3','p4']:
    for run in sorted((root/'protocol_metadata'/part).iterdir()):
        assert (run/'exit_status.txt').read_text().strip()=='0'
        affinity=json.loads((run/'affinity.json').read_text())
        topology=affinity['topology']
        assert len({c['socket'] for c in topology})==1
        assert len({(c['socket'],c['core']) for c in topology})==len(topology)
        assert len(topology)==(4 if run.name.startswith('smoke') else 32)
        assert 'policy: default' in (run/'numa_policy.txt').read_text()
        graph_list=json.loads((run/'graph_list.json').read_text())
        status=json.loads((run/'suite_status.json').read_text())
        for r in status:
            assert r['status']=='COMPLETE'
            if 'input_sha256' in r:
                assert r['actual_sha256']==r['input_sha256']
            metadata.append(dict(study=part,run=run.name,graph=r['graph'],
                                 path=r['path'],actual_sha256=r['actual_sha256'],
                                 N=r['N'],E=r['E']))
        assert {r['graph'] for r in graph_list}=={r['graph'] for r in status}
bygraph={}
for r in metadata:
    if r['run'].startswith('focus'):
        bygraph.setdefault(r['graph'],set()).add(r['actual_sha256'])
assert all(len(hashes)==1 for hashes in bygraph.values())
with (root/'graph_provenance.csv').open('w',newline='',encoding='utf-8') as f:
    w=csv.DictWriter(f,list(metadata[0]));w.writeheader();w.writerows(metadata)
archive_counts={}
for name in ['RAW_LOGS.tar.gz','superseded_smoke/RAW_LOGS.tar.gz']:
    with tarfile.open(root/name) as archive:
        members=archive.getmembers()
        assert all(not pathlib.PurePosixPath(m.name).is_absolute() and '..' not in pathlib.PurePosixPath(m.name).parts for m in members)
        # Read every file to ensure the entire gzip stream is valid, no extraction.
        for m in members:
            if m.isfile():
                f=archive.extractfile(m)
                while f.read(1<<20):
                    pass
        archive_counts[name]=len(members)
state=dict(git_parent_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=repo,text=True).strip(),
           branch=subprocess.check_output(['git','branch','--show-current'],cwd=repo,text=True).strip(),
           recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
           dirty_before_publication=True,
           dirty_status_before_publication=subprocess.check_output(['git','status','--porcelain','--','GCN-extra/experiments/journal_followup_p3_p4_20261005','GCN-extra/docs/JOURNAL_P3_P4_INTERPRETATION_20261005.md','GCN-extra/results/journal-p3-p4-20261005','GCN-extra/IMPLEMENTATION_CONTRACT.md'],cwd=repo,text=True).splitlines(),
           measured_source_state='Independent uncommitted source snapshots; build manifests record parent commit and exact source/binary hashes. No remote git status was captured at execution.',
           graph_hashes_and_physical_core_binding_verified=True,
           raw_archive_members_verified=archive_counts,
           publish_scope='P3/P4 measured evidence, source, interpretation and synchronized handoffs; frozen Python runtime caches retained to match inventories; no main datasets, toolchains or benchmark executables')
(root/'publication_state.json').write_text(json.dumps(state,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
(root/'README.md').write_text('''# P3/P4 journal follow-up evidence

Parent commit: af943926c7d4f8e68bcb95838924e21b14dd2e86.
Read [the explanation](../../docs/JOURNAL_P3_P4_INTERPRETATION_20261005.md)
and [all fixed-method results](JOURNAL_P3_P4_RESULTS_20261005.md).

P3: D2/D3 broad correctness and performance against original TFS, B64 and
S64/FULL ACCURATE. P4: source-consumer selection, selective source projection
plus cold-source TFS fusion, strong project-all/project-used/mixed controls,
two shapes, two controlled relations and five real graphs. No paper baseline
replacement, cross-harness denominator, normalized GCN classification claim
or universal numerical guarantee. Shared-node measurements are diagnostic.

`p3/` and `p4/` retain the exact compiled `source_snapshot/`, manifests, build
commands/logs/hashes, every CSV, detailed sampled phases and per-TID PMU.
The three generated Python runtime caches in the frozen snapshots are also
retained to match the original inventories, despite the usual Git ignore.
`protocol_metadata/` retains input paths/hashes, CPU/affinity and NUMA evidence.
`graph_provenance.csv` matches graph hashes across P3/P4. Prepared timing
includes every layer's cache rebuild; topology setup is separately charged.
`superseded_smoke/` is preserved and excluded from main tables. Archive files
contain raw stdout and metadata, not compiled executables or graph datasets
in the main archive. The small superseded smoke includes generated fixtures.

`artifact_hashes.json` is the original reconciliation inventory;
`FINAL_ARTIFACT_HASHES.json` covers the subsequently added analysis/metadata.
`publication_state.json` distinguishes measured snapshots from publication git
state. See `ANALYSIS.json` for gate maxima and audit counts. Fine child phases
are thread-local sampled diagnostics, not additive wall time. PMU misses and
logical request bytes are not DRAM bandwidth measurements.
''',encoding='utf-8')
event=dict(id='journal-p3-p4-interpretation-20261005',kind='journal_followup_final_audit',status='COMPLETE',
           evidence='GCN-extra/results/journal-p3-p4-20261005',
           source='GCN-extra/experiments/journal_followup_p3_p4_20261005',
           report='GCN-extra/docs/JOURNAL_P3_P4_INTERPRETATION_20261005.md',
           paper_eligible=False,
           results='P3:160/P4:540 numerical checks passed; zero candidate rejection; 660 PMU regions. D2/D3 0/5 wins vs FULL. Partial P4 fractions do not beat all global/mixed controls.',
           issues='Analysis initially stopped on missing P3 work pass-column; schema corrected and complete audit rerun. Controlled small graphs have high CV. P3 fine profile is layer1 only. Source-cache layout/map dependence is not isolated. No universal accuracy or task/novelty claims.',
           resolution='All raw measurements retained; source/input hashes, Slurm exit, physical binding, NUMA policy, gates and PMU scaling reconciled. Superseded first P4 smoke remains outside main tables.',
           next='Prioritize P0/P1 predictive conditions and held-out selection evidence; isolate same-set Q layout before claiming a selective materialization mechanism.')
(root/'final_event.json').write_text(json.dumps(event,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
(root/'FINAL_ARTIFACT_HASHES.json').write_text(json.dumps({p.relative_to(root).as_posix():hashlib.sha256(p.read_bytes()).hexdigest()
    for p in root.rglob('*') if p.is_file() and p.name!='FINAL_ARTIFACT_HASHES.json'},indent=2)+'\n',encoding='utf-8')
print('FINAL_EVIDENCE_VERIFIED',len(metadata),'graph records',archive_counts)
