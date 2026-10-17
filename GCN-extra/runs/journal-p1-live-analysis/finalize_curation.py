import pathlib,json,hashlib,sys
repo=pathlib.Path(sys.argv[1])
root=repo/'GCN-extra'
out=root/'results/journal-p1-20261005'
event=dict(id='journal-followup-interpretation-20261005',kind='journal_p1_p2_p3_interpretation',status='COMPLETE_CURRENT_ROUND',
    evidence='/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/docs/JOURNAL_FOLLOWUP_VALIDATION_20261005.md',paper_eligible=False,
    summary=dict(p1_graphs='17 real + 3 synthetic',p1_numeric=880,p1_same_scope_bitwise=480,p1_profile_bitwise=320,p1_same_work=320,
        p1_full_page_gmean_vs_degree=1.0125667474113085,p1_full_source_gmean_vs_degree=0.9994843472007671,
        p2='5 same-output fixed-work one-core cases; current interleaving slower in sparse-heavy cases',
        p3='D1 fails unchanged FP32 1e-3 gate on exact-BF16 hardware fixture; D2 only passes this fixture'),
    issues='Shared-node evidence; P1 setup charged separately, limited gains and all negative/noisy cases retained. P2 micro is not E2E. P3 experiment PASS denotes expected D1 rejection. Journal plan not fully validated.',
    next='Prioritize P0/P1 applicable-region and held-out selection evidence. P4, cross-shape/full-model tests and broad D2 validation remain incomplete.')
(out/'interpretation_event.json').write_text(json.dumps(event,indent=2)+'\n',encoding='utf-8')
for name in ['journal-p1-20261005','p2-overlap-20261005','p3-residual-gate-20261005']:
    folder=root/'results'/name
    hashes={p.relative_to(folder).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(folder.rglob('*')) if p.is_file() and p.name!='CURATED_ARTIFACT_HASHES.json'}
    (folder/'CURATED_ARTIFACT_HASHES.json').write_text(json.dumps(hashes,indent=2)+'\n')
    print(name,len(hashes))
