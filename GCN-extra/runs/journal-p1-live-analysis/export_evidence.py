import pathlib,tarfile,json,datetime
root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
export=root/'runs'/('evidence-export-'+datetime.datetime.now().strftime('%Y%m%d-%H%M%S'))
export.mkdir(exist_ok=False)
runs={'p1':'journal-p1-build-20261005-111037','p2':'avx-amx-overlap-20261005-112746','p3':'delayed-residual-gate-20261005-112212','p3_failed':'delayed-residual-gate-20261005-112010'}
records=[]
with tarfile.open(export/'evidence.tar.gz','w:gz') as archive:
    for label,name in runs.items():
        run=root/'runs'/name
        wanted=['source_snapshot','manifest.json','source.sha256','submission.json','event.json','exit_status.txt','README.md','build/commands.txt','build/compile.log','build/binary.sha256','build/event.json','build/linked_libraries.txt','build/generation.stdout.json']
        lognames=['node.txt','cpu.txt','compiler.txt','affinity.json','numa_policy.txt','numa_hardware.txt','preflight.sha256','exit_status.txt','event.json','slurm-10869361.err','slurm-10869361.out']
        if label=='p1':
            for log in sorted((run/'logs').iterdir()):
                if not log.is_dir():continue
                for file in lognames+['completion.json','graph_list.json','binary.sha256','source.sha256']:
                    wanted.append(str((log/file).relative_to(run)))
        else:
            wanted+=['logs/'+n for n in lognames]
        for rel in wanted:
            file=run/rel
            if file.exists():
                archive.add(file,arcname=label+'/'+rel)
                records.append(dict(study=label,source=str(file),archive_path=label+'/'+rel))
(export/'records.json').write_text(json.dumps(records,indent=2)+'\n')
print(str(export))
