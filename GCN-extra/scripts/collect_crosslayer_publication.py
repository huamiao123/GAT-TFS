"""Export the October 7/8 journal evidence without modifying original runs."""
import csv
import datetime
import hashlib
import json
import pathlib
import re
import shutil
import subprocess
import tarfile

ROOT = pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
OUT = ROOT / 'runs/publication-crosslayer-20261013-review'
TEXT = {'.md','.txt','.json','.jsonl','.csv','.tsv','.log','.out','.err',
        '.cpp','.hpp','.h','.cc','.c','.py','.sh','.slurm','.sha256','.asm',
        '.yaml','.yml','.toml','.ini','.exit'}


def sha(p):
    h=hashlib.sha256()
    with p.open('rb') as f:
        for block in iter(lambda:f.read(1<<20),b''):h.update(block)
    return h.hexdigest()


def save(p,obj):
    p.write_text(json.dumps(obj,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')


def csvfile(p,rows):
    if not rows:return
    keys=list(dict.fromkeys(k for r in rows for k in r))
    with p.open('w',newline='',encoding='utf-8') as f:
        w=csv.DictWriter(f,keys);w.writeheader();w.writerows(rows)


def selected(name):
    return '20261007' in name or '20261008' in name


def main():
    assert OUT.resolve().is_relative_to(ROOT.resolve())
    OUT.mkdir(exist_ok=False)
    target=OUT/'payload';target.mkdir()
    captured=[];excluded=[];copied=[]

    def copy_tree(source,dest):
        for p in sorted(source.rglob('*')) if source.is_dir() else [source]:
            if not p.is_file():continue
            relative=p.relative_to(source) if source.is_dir() else pathlib.Path(p.name)
            q=dest/relative if source.is_dir() else dest
            # Runtime bytecode, datasets and executables stay on the server.
            # Source, stdout, table and status contents are exported unchanged.
            reason=None
            if p.is_symlink():reason='symlink_not_followed'
            elif '__pycache__' in p.parts or p.suffix=='.pyc':reason='runtime_bytecode'
            elif p.suffix not in TEXT and p.name not in ['LATEST_BUILD','LATEST_RUN','Makefile','CMakeLists.txt']:reason='non_text_artifact_or_dataset'
            if reason:
                excluded.append(dict(path=str(p),bytes=p.stat().st_size,reason=reason));continue
            with p.open('rb') as f:magic=f.read(4)
            if magic==b'\x7fELF':
                excluded.append(dict(path=str(p),bytes=p.stat().st_size,reason='ELF_benchmark_binary'));continue
            assert q.resolve().is_relative_to(target.resolve())
            q.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,q)
            digest=sha(q);assert digest==sha(p)
            captured.append(dict(source=str(p),published=q.relative_to(target).as_posix(),bytes=q.stat().st_size,sha256=digest))
            copied.append((p,q))

    exp=sorted(p for p in (ROOT/'experiments').iterdir() if p.is_dir() and selected(p.name))
    runs=sorted(p for p in (ROOT/'runs').iterdir() if p.is_dir() and
                (selected(p.name) or re.search(r'1090\d{4}',p.name)))
    for p in exp:copy_tree(p,target/'experiments'/p.name)
    for p in runs:copy_tree(p,target/'runs'/p.name)
    copy_tree(ROOT/'docs/handoff',target/'handoff_snapshot')
    for folder in ['original','src']:
        copy_tree(ROOT/folder,target/'reference_dependencies'/folder)
    for name in ['IMPLEMENTATION_CONTRACT.md','docs/CURRENT_GRAPH_INVENTORY_20261004.json',
                 'scripts/preflight.sh','scripts/record_event.py','scripts/collect_crosslayer_publication.py']:
        copy_tree(ROOT/name,target/'reference_dependencies'/name)
    for p in sorted(ROOT.parent.glob('update_handoff_*.py')):
        copy_tree(p,target/'handoff_updaters'/p.name)
    copied_sources=[]
    for p,q in copied:
        if p.name in ['source.sha256','binary.sha256']:
            for line in p.read_text(encoding='utf-8').splitlines():
                fields=line.split(maxsplit=1)
                if len(fields)!=2 or not re.fullmatch('[0-9a-f]{64}',fields[0]):continue
                original=pathlib.Path(fields[1].lstrip('*'))
                if not original.is_absolute():original=p.parent/original
                r=dict(manifest=str(p),path=str(original),expected_sha256=fields[0])
                if not original.resolve().is_relative_to(ROOT.resolve()):r['status']='OUTSIDE_ROOT_NOT_READ'
                elif not original.is_file():r['status']='MISSING_AT_PUBLICATION'
                else:
                    r['actual_sha256']=sha(original)
                    r['status']='MATCH' if r['actual_sha256']==r['expected_sha256'] else 'CHANGED_AT_PUBLICATION'
                copied_sources.append(r)
    # Latest adaptive build and canonical reference are required identity gates.
    latest='recompute-window-adaptive-build-20261008-234221'
    assert all(r['status']=='MATCH' for r in copied_sources if latest in r['manifest'])
    assert len([r for r in copied_sources if latest in r['manifest']])==3
    assert sha(ROOT/'src/paper_methods_kernels.hpp')=='35d291b886ee0f00e79e0e5d17c6226b2fa1ad64eb715ecbe7bcb1c3f02e7b85'
    inventory=[];jobs=set()
    for run in runs:
        events=[]
        for p in sorted(run.rglob('*event*.json')):
            try:
                event=json.loads(p.read_text(encoding='utf-8-sig'))
                if isinstance(event,dict):events.append(dict(path=str(p),id=event.get('id'),status=event.get('status'),kind=event.get('kind'),paper_eligible=event.get('paper_eligible')))
            except Exception:pass
        for p in run.rglob('*'):
            if p.is_file():
                for n in re.findall(r'(?<!\d)(10[89]\d{5})(?!\d)',p.name):jobs.add(n)
        for p in run.rglob('*jobs*.json'):
            try:
                for n in re.findall(r'(?<!\d)(10[89]\d{5})(?!\d)',p.read_text(encoding='utf-8-sig')):jobs.add(n)
            except Exception:pass
        # Recent flat run IDs and partition jobs are also visible in handoffs.
        for n in re.findall(r'(?<!\d)(1090\d{4})(?!\d)',run.name):jobs.add(n)
        inventory.append(dict(run=run.name,events=events,
            exported_files=sum(r['published'].startswith('runs/'+run.name+'/') for r in captured),
            has_program_log=(run/'program.log').exists(),has_source_snapshot=(run/'source_snapshot').exists()))
    # Event content covers IDs absent from flat file names (e.g. 10902324).
    for p in [ROOT/'docs/handoff/events.jsonl',ROOT/'docs/handoff/CURRENT_HANDOFF.md']:
        for n in re.findall(r'(?<!\d)(1090\d{4})(?!\d)',p.read_text(encoding='utf-8-sig')):jobs.add(n)
    accounting=subprocess.run(['sacct','-j',','.join(sorted(jobs)),
        '--format=JobID,JobName,Partition,State,ExitCode,Elapsed,AllocCPUS,NNodes,NodeList','-P'],capture_output=True,text=True)
    (target/'slurm_accounting.txt').write_text(accounting.stdout,encoding='utf-8')
    (target/'slurm_accounting.stderr.txt').write_text(accounting.stderr,encoding='utf-8')
    save(target/'run_inventory.json',inventory)
    csvfile(target/'exported_files.csv',captured);csvfile(target/'excluded_files.csv',excluded)
    csvfile(target/'original_manifest_verification.csv',copied_sources)
    save(target/'manifest.json',dict(
        publication_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        source_root=str(ROOT),source_days=['2026-10-07','2026-10-08'],
        git_publication_parent='672481d4ef34ec07f13f427485647e4def00062c',
        measured_workspace='standalone server workspace; no git status captured by this exporter',
        experiment_directories=[p.name for p in exp],run_directories=[p.name for p in runs],
        exported_files=len(captured),exported_bytes=sum(r['bytes'] for r in captured),
        excluded_files=len(excluded),sacct_exit_code=accounting.returncode,
        status='ARCHIVED_EXPLORATORY_EVIDENCE',paper_eligible=False,
        limitations=['Publication only; no new benchmark run.',
            'Raw source/status/timing files are retained byte-for-byte, including failures and superseded records.',
            'Interpretation corrections are separate; six-sample upper median and fallback alias timing are not rewritten.',
            'Compiled binaries/datasets/runtime bytecode are not included; binary identities and exclusions retained.',
            'Copied source snapshots retain original include paths and command layouts; see dependency snapshot for replay.',
            'User node ceiling is five; the source handoff ten-node sentence is historical and not authorization.']))
    save(target/'SERVER_ARTIFACT_HASHES.json',{p.relative_to(target).as_posix():sha(p)
        for p in target.rglob('*') if p.is_file()})
    with tarfile.open(OUT/'crosslayer-evidence.tar.gz','w:gz',compresslevel=1) as archive:
        archive.add(target,arcname='.')
    print(json.dumps(dict(experiments=len(exp),runs=len(runs),files=len(captured),
                         bytes=sum(r['bytes'] for r in captured),archive_bytes=(OUT/'crosslayer-evidence.tar.gz').stat().st_size,
                         excluded=len(excluded),manifest_hash_status={s:sum(r['status']==s for r in copied_sources) for s in sorted({r['status'] for r in copied_sources})})))


if __name__=='__main__':main()
