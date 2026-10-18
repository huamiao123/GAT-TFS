"""Run on the server after copying the frozen coverage bundle. No kernels run."""
import pathlib, hashlib, json, subprocess, datetime, os

root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
wzh=root.parent.resolve()
bundle=root/'docs/handoff/coverage-audit-20261018'
assert os.path.commonpath([str(bundle.resolve()),str(wzh)])==str(wzh)
manifest=json.loads((bundle/'coverage_delivery_manifest.json').read_text(encoding='utf8'))
for name,expected in manifest.items():
    got=hashlib.sha256((bundle/name).read_bytes()).hexdigest()
    assert got==expected,(name,got,expected)
report=root/'docs/handoff/EXPERIMENT_COVERAGE_AUDIT_20261018.md'
report_data=(bundle/report.name).read_bytes()
if report.exists():
    assert report.read_bytes()==report_data,'Refuse changing an existing different coverage report'
else:
    report.write_bytes(report_data)

event_id='experiment-coverage-audit-20261018-history-errata'
current=root/'docs/handoff/CURRENT_HANDOFF.md'
backup=bundle/'CURRENT_HANDOFF.before_coverage_20261018.md'
prefix=(bundle/'CURRENT_HANDOFF_COVERAGE_PREFIX.md').read_text(encoding='utf8')
if not backup.exists(): backup.write_bytes(current.read_bytes())
old=backup.read_text(encoding='utf8')
assert event_id not in old,'Unexpected existing prefix in backup'
current.write_text(prefix+old,encoding='utf8')

event={
    'id':event_id,
    'date':datetime.datetime.now(datetime.timezone.utc).isoformat(),
    'kind':'historical_experiment_coverage_audit_and_erratum',
    'status':'COVERAGE_AUDIT_DELIVERED_WITH_EXPLICIT_REVIEW_DEPTH_LIMITS',
    'paper_eligible':False,
    'supersedes_scope_claim_of':'github-deep-review-20261018-9369b9e',
    'github_head':'9369b9e0491d59e328a99153b6d97314c62285da',
    'restored_precleanup_commit':'8aeef1613fdeaeb929379874c4152eccde620f74',
    'restored_files':3969,
    'historical_paths_absent_in_head':2789,
    'server_organization_units':36,
    'server_run_containers_in_inventory':140,
    'report':'docs/handoff/EXPERIMENT_COVERAGE_AUDIT_20261018.md',
    'report_sha256':hashlib.sha256(report_data).hexdigest(),
    'attachments':'docs/handoff/coverage-audit-20261018/',
    'corrections':[
        'Previous review did not establish complete historical coverage; HEAD directory count was insufficient.',
        'Withdraw audit_projection.md:114 native FP32 serial first-touch assertion: final runtime uses prepared_fp32 parallel static exact copy; earlier confounded run was cancelled.',
        'Initial S/M 2720 timings are a distinct round from A3/P0 2890.',
        'Replay was implemented and timed before cleanup; not a wholly new candidate.',
        'Oct7-8 eight families include thirteen versions, sixty-two executed jobs and 37110 checked primary timings; state semantic failures and strict control chain preserved.',
        'Do not infer completed kernels or performance failure from generic event labels; inspect actual launcher/errors/results.'
    ],
    'limitations':[
        'No performance rerun; no claim that every disassembly instruction or every PMU sample has been manually interpreted.',
        'Targeted 856 text-file content reconciliation completed; full-server content hash attempt timed out and is not counted as verified.',
        'Review depth and remaining independent validation are stated per report; cancelled partial samples excluded from performance conclusions.'
    ],
    'cluster_jobs_submitted':0,
    'kernel_edits':0,
    'old_frozen_evidence_modified':False,
    'numa_interleave_used':False,
    'issues':'Prior coverage overclaim and feature first-touch misread corrected; actual failure/version lineage documented in attached audits.',
    'resolution':'Restored pre-cleanup commit; separated experimental units from builds/submissions; reconciled runtime/log/CSV and retained failures; linked explicit errata in CURRENT_HANDOFF.',
    'next':'Use per-unit coverage ledger before selecting new mechanisms; do not rename already-tested replay/pair/prefetch/trace as untested ideas.',
    'delivery_manifest':manifest
}
events=root/'docs/handoff/events.jsonl'
existing=[json.loads(x) for x in events.read_text(encoding='utf8').splitlines() if x.strip()]
found=[x for x in existing if x.get('id')==event_id]
event_path=bundle/'event.json'
if not found:
    event_path.write_text(json.dumps(event,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    subprocess.run(['python3',str(root/'scripts/record_event.py'),str(event_path)],check=True)
else:
    assert len(found)==1 and found[0]['report_sha256']==event['report_sha256']
subprocess.run(['python3',str(root/'scripts/record_event.py'),'check'],check=True)
for name in ['ALL_EXPERIMENTS.md','EXPERIMENT_ISSUES.md','PROJECT_PROGRESS.md']:
    txt=(root/'docs/handoff'/name).read_text(encoding='utf8')
    assert txt.count('## '+event_id+'\n')==1,(name,'missing/duplicated event')
    assert event['report_sha256'] in txt.split('## '+event_id+'\n',1)[1],(name,'hash absent')
assert current.read_text(encoding='utf8').startswith(prefix)
verification={'event_id':event_id,'report_sha256':event['report_sha256'],'manifest_files_verified':len(manifest),'persistent_ledgers_verified':3,'current_handoff_updated':True,'previous_review_preserved':True,'cluster_jobs_submitted':0}
(bundle/'coverage_server_delivery_verification.json').write_text(json.dumps(verification,indent=2)+'\n',encoding='utf8')
print(json.dumps(verification))
