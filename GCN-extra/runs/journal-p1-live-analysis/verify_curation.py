import pathlib,json,hashlib,subprocess
root=pathlib.Path('GCN-extra/results')
count=0
for folder in [root/n for n in ['journal-p1-20261005','p2-overlap-20261005','p3-residual-gate-20261005']]:
    hashes=json.loads((folder/'CURATED_ARTIFACT_HASHES.json').read_text())
    for rel,digest in hashes.items():
        p=folder/rel
        assert hashlib.sha256(p.read_bytes()).hexdigest()==digest,('working bytes',p)
        blob=subprocess.check_output(['git','cat-file','blob',':'+p.as_posix()])
        assert hashlib.sha256(blob).hexdigest()==digest,('staged bytes',p)
        count+=1
print('CURATED_HASHES_AND_STAGED_BYTES_MATCH',count)
baseline=pathlib.Path('GCN-extra/original/gcn_e2e_v3.cpp').read_bytes()
for folder in [root/n for n in ['journal-p1-20261005','p2-overlap-20261005','p3-residual-gate-20261005']]:
    assert (folder/'provenance/source_snapshot/gcn_e2e_v3.cpp').read_bytes()==baseline
print('ORIGINAL_TFS_MKL_SOURCE_BYTE_EQUAL_ALL_THREE_STUDIES')
