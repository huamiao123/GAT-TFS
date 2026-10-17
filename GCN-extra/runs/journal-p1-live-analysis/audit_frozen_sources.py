import pathlib,json,hashlib,tarfile
root=pathlib.Path('GCN-extra/results')
counts={}
for name in ['journal-p1-20261005','p2-overlap-20261005','p3-residual-gate-20261005']:
    folder=root/name/'provenance'
    n=0
    for line in (folder/'source.sha256').read_text().splitlines():
        digest,path=line.split(maxsplit=1)
        actual=folder/'source_snapshot'/pathlib.PurePosixPath(path).name
        assert actual.is_file(),actual
        assert hashlib.sha256(actual.read_bytes()).hexdigest()==digest,actual
        n+=1
    counts[name]=n
p=root/'journal-p1-20261005'
index=json.loads((p/'RAW_LOGS_INDEX.json').read_text())
assert hashlib.sha256((p/'RAW_LOGS.tar.gz').read_bytes()).hexdigest()==index['archive_sha256']
with tarfile.open(p/'RAW_LOGS.tar.gz') as archive:
    assert len(archive.getmembers())==len(index['records'])
    for r in index['records']:
        data=archive.extractfile(r['path']).read()
        assert len(data)==r['bytes']
        assert hashlib.sha256(data).hexdigest()==r['sha256'],r['path']
result=dict(status='PASS',compiled_source_hashes=counts,raw_archive_members=len(index['records']),raw_archive_sha256=index['archive_sha256'],original_source_tfs_mkl='unchanged in all three frozen snapshots',scope='Artifact integrity only. Numerical/performance gates are recorded separately.')
(p/'evidence_audit.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result))
