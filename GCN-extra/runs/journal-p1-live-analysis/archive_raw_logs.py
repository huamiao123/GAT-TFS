import pathlib,tarfile,hashlib,json
study=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p1-build-20261005-111037')
out=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p1-reconciled-20261005-115500')
records=[]
with tarfile.open(out/'RAW_LOGS.tar.gz','w:gz') as archive:
    for job in sorted((study/'logs').iterdir()):
        if not job.is_dir():continue
        for graph in sorted(job.iterdir()):
            if not graph.is_dir():continue
            for p in sorted(graph.iterdir()):
                if not p.is_file() or p.name in ['parsed.json','summary.csv']:continue
                rel=p.relative_to(study).as_posix()
                archive.add(p,arcname=rel)
                records.append(dict(path=rel,sha256=hashlib.sha256(p.read_bytes()).hexdigest(),bytes=p.stat().st_size))
(out/'RAW_LOGS_INDEX.json').write_text(json.dumps(dict(study=str(study),records=records,archive_sha256=hashlib.sha256((out/'RAW_LOGS.tar.gz').read_bytes()).hexdigest()),indent=2)+'\n')
print('ARCHIVED',len(records),'FILES', (out/'RAW_LOGS.tar.gz').stat().st_size,'BYTES')
