import pathlib, hashlib, json, subprocess, datetime, os

root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
wzh=root.parent.resolve()
assert os.path.commonpath([str(root.resolve()),str(wzh)])==str(wzh)
handoff=root/'docs/handoff'
bundle=handoff/'review-20261018'
manifest=json.loads((bundle/'delivery_manifest.json').read_text(encoding='utf8'))
for name,expected in manifest.items():
    got=hashlib.sha256((bundle/name).read_bytes()).hexdigest()
    assert got==expected,(name,got,expected)

report=handoff/'GITHUB_DEEP_REVIEW_20261018.md'
report_bytes=(bundle/report.name).read_bytes()
if report.exists():
    assert report.read_bytes()==report_bytes,'Refuse overwriting a different existing review'
else:
    report.write_bytes(report_bytes)

current=handoff/'CURRENT_HANDOFF.md'
backup=bundle/'CURRENT_HANDOFF.before_review_20261018.md'
if not backup.exists():
    backup.write_bytes(current.read_bytes())
prefix=(bundle/'CURRENT_HANDOFF_REVIEW_PREFIX.md').read_text(encoding='utf8')
current.write_text(prefix+backup.read_text(encoding='utf8'),encoding='utf8')

event_id='github-deep-review-20261018-9369b9e'
event={
  'id':event_id,
  'date':datetime.datetime.now(datetime.timezone.utc).isoformat(),
  'status':'READ_ONLY_REVIEW_COMPLETED',
  'kind':'evidence_audit_and_research_synthesis',
  'paper_eligible':False,
  'github_commit':'9369b9e0491d59e328a99153b6d97314c62285da',
  'scope':'All 15 top-level result families plus original/cache/projection runs and 19 crosslayer subdirections; trace depth and unavailable raw evidence stated in report.',
  'snapshot_files_checked':16442,
  'snapshot_missing':0,
  'snapshot_blob_mismatches':0,
  'cluster_jobs_submitted':0,
  'kernel_edits':0,
  'report':'docs/handoff/GITHUB_DEEP_REVIEW_20261018.md',
  'report_sha256':hashlib.sha256(report_bytes).hexdigest(),
  'attachments':'docs/handoff/review-20261018/',
  'findings':[
    'Default-NUMA FULL Identity remains a strong baseline, including completed BF16 oneMKL and packed comparisons; shared-node and 4-core limits retained.',
    'Exclude onemkl_bf16_interleave_20261015 performance from user-compliant claims because launcher explicitly enables NUMA interleave.',
    'Correct fallback timing interpretation, median convention, H-panel window accounting and early crosslayer structural-analysis errors.',
    'Distinguish rowwise FULL, S64/MFULL, FAST/ACCURATE and PACK_EPI; do not combine speedups across runs.',
    'B changes projection count and access timing, not total E*D additions in non-replay B/FULL aggregation.',
    'New candidate C1/C2/C4 mechanisms remain unverified; missing same-aggregator materialization ablation is identified.'
  ],
  'constraints':['No NUMA interleave','128->128->128 canonical comparison','No desktop TXT','No benchmark rerun in this audit','Server writes confined to wzh'],
  'delivery_manifest':manifest
}
event_path=bundle/'event.json'
events=handoff/'events.jsonl'
existing=[json.loads(x) for x in events.read_text(encoding='utf8').splitlines() if x.strip()] if events.exists() else []
found=[x for x in existing if x.get('id')==event_id]
if not found:
    event_path.write_text(json.dumps(event,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
    subprocess.run(['python3',str(root/'scripts/record_event.py'),str(event_path)],check=True)
else:
    assert len(found)==1 and found[0]['report_sha256']==event['report_sha256']

subprocess.run(['python3',str(root/'scripts/record_event.py'),'check'],check=True)
for name in ['ALL_EXPERIMENTS.md','EXPERIMENT_ISSUES.md','PROJECT_PROGRESS.md']:
    text=(handoff/name).read_text(encoding='utf8')
    assert text.count('## '+event_id+'\n')==1,(name,'event duplicate/missing')
    tail=text.split('## '+event_id+'\n',1)[1]
    assert event['report_sha256'] in tail,(name,'report hash not recorded')
assert current.read_text(encoding='utf8').startswith(prefix)
assert report.read_bytes()==report_bytes
verification={'event_id':event_id,'report_sha256':event['report_sha256'],'files_delivered':len(manifest),'handoff_ledgers_verified':3,'current_handoff_updated':True,'historical_current_preserved':str(backup),'cluster_jobs_submitted':0}
(bundle/'server_delivery_verification.json').write_text(json.dumps(verification,indent=2)+'\n',encoding='utf8')
print(json.dumps(verification))
