#!/usr/bin/env python3
"""User-authorized cleanup of superseded GCN-extra experiments (Linux only).

No benchmark is executed. Frozen current run evidence is hash-checked before
and after deletion. Local Windows mirrors use the same emitted deletion plan.
"""
import datetime, hashlib, json, os, pathlib, shutil, subprocess, sys, tarfile

ROOT = pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
PREFIXES = ('paper-method-', 'paper-cache-')
CLEAN_RUN = 'cleanup-current-20261004'
ARCHIVE = 'evidence-current-controlled-20261004-clean.tar.gz'
SCRIPTS = {
    'build_paper_methods.sh', 'generate_paper_methods.py', 'paper_methods_suite.py',
    'paper_methods_job.sh', 'submit_paper_methods.sh', 'build_paper_cache_control.sh',
    'submit_paper_cache_control.sh', 'finalize_paper_methods.py',
    'finalize_paper_cache_control.py', 'plot_paper_methods.py',
    'analyze_paper_variability.py', 'preflight.sh', 'record_event.py',
    'paper_methods_progress.py', 'paper_cache_progress.py',
    'audit_paper_publication.py', 'adjust_paper_cache_limits.sh',
    'cleanup_current_evidence.py', 'package_current_evidence.py',
    'mirror_current_evidence.py',
}
DOCS = {'PAPER_METHODS_PROTOCOL_20261004.md', 'PAPER_METHODS_RESULTS_20261004.md',
        'PAPER_CACHE_CONTROL_RESULTS_20261004.md', 'PAPER_METHODS_INTERPRETATION_20261004.md',
        'CURRENT_GRAPH_INVENTORY_20261004.json', 'CURRENT_EVIDENCE_20261004.md',
        'handoff', 'figures', 'paper_reference'}
ROOT_FILES = {'.gitattributes', '.gitignore', 'README.md', 'IMPLEMENTATION_CONTRACT.md',
              ARCHIVE, ARCHIVE+'.sha256', 'docs', 'runs', 'scripts', 'src', 'original', 'build'}

def sha(p):
    h=hashlib.sha256()
    with p.open('rb') as f:
        for b in iter(lambda:f.read(8*1024*1024),b''): h.update(b)
    return h.hexdigest()

def save(p,x): p.write_text(json.dumps(x,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')

def checked(root, rel):
    q=pathlib.PurePosixPath(rel)
    assert rel and not q.is_absolute() and '..' not in q.parts and '\\' not in rel
    p=root/q
    assert p.resolve().is_relative_to(root.resolve()) and p.resolve()!=root.resolve()
    assert not p.is_symlink()
    return p

def obsolete(root):
    paths=[]
    def select(sub, keep):
        folder=root/sub
        if folder.exists():
            paths.extend(p.relative_to(root).as_posix() for p in folder.iterdir() if not keep(p.name))
    select('', lambda n:n in ROOT_FILES)
    select('runs',lambda n:n.startswith(PREFIXES) or n==CLEAN_RUN)
    select('scripts',lambda n:n in SCRIPTS)
    select('docs',lambda n:n in DOCS)
    select('docs/figures',lambda n:n=='paper_methods_20261004')
    select('src',lambda n:n in {'paper_methods_runtime.hpp','paper_methods_kernels.hpp'})
    select('original',lambda n:n in {'amx_tfs_v3.cpp','mkl_baseline.cpp','gcn_e2e_v3.cpp'})
    select('build',lambda n:n in {'PAPER_METHOD_LATEST','PAPER_CACHE_LATEST','tmp'})
    select('build/tmp',lambda n:False)
    for rel in paths:
        p=checked(root,rel)
        if p.is_dir():
            for q in p.rglob('*'): assert not q.is_symlink() and q.resolve().is_relative_to(root.resolve())
    return sorted(paths)

def prepare(root):
    main=root/'runs/paper-method-build-20261004-123445/source_snapshot'
    cache=root/'runs/paper-cache-build-20261004-131154/source_snapshot'
    assert (main/'paper_methods_kernels.hpp').read_bytes()==(cache/'paper_methods_kernels.hpp').read_bytes()
    shutil.copyfile(main/'paper_methods_kernels.hpp',root/'src/paper_methods_kernels.hpp')
    assert (main/'paper_methods_runtime.hpp').read_bytes()==(root/'src/paper_methods_runtime.hpp').read_bytes()
    states=json.loads((root/'runs/paper-method-reconciled-20261004/graph_states.json').read_text())
    assert len(states)==17 and all(x['status']=='PASS' for x in states)
    fields=['graph','path','N','E','avg_degree','format','square','size_bytes','input_sha256']
    save(root/'docs/CURRENT_GRAPH_INVENTORY_20261004.json',[{k:x[k] for k in fields} for x in states])

    p=root/'scripts/generate_paper_methods.py';s=p.read_text()
    start=s.index("candidate=(root/'src/source_protocol_kernels.hpp').read_text()")
    end=s.index("marker='static void benchmark",start)
    s=s[:start]+"header=(root/'src/paper_methods_kernels.hpp').read_bytes()\n(out/'paper_methods_kernels.hpp').write_bytes(header)\n"+s[end:]
    s=s.replace("(root/'src/source_protocol_kernels.hpp').read_bytes()","(root/'src/paper_methods_kernels.hpp').read_bytes()")
    s=s.replace("changes=['typed destination output enables direct BF16 final store','FP32 reduction/hi-lo/AMX/CSR scheduling unchanged','remove obsolete H0 reconversion outside every repeat','source hooks after original MKL and TFS measurements; separate rotating measurements']","changes=['copy validated frozen kernel byte for byte; obsolete generator dependency removed','source hooks after original MKL and TFS measurements; separate rotating measurements']")
    p.write_text(s)
    p=root/'scripts/build_paper_methods.sh';s=p.read_text().replace('$ROOT/src/source_protocol_kernels.hpp','$ROOT/src/paper_methods_kernels.hpp');p.write_text(s)
    p=root/'scripts/paper_methods_suite.py';s=p.read_text()
    s=s.replace("root/'runs/formal-10862002/graph_list.json'","root/'docs/CURRENT_GRAPH_INVENTORY_20261004.json'")
    s=s.replace("root/'runs/formal-10862002/suite_status.json'","root/'docs/CURRENT_GRAPH_INVENTORY_20261004.json'")
    s=s.replace("known[g['graph']]['dataset_sha256']","known[g['graph']]['input_sha256']");p.write_text(s)
    p=root/'docs/PAPER_METHODS_INTERPRETATION_20261004.md';s=p.read_text(encoding='utf-8')
    s=s.replace('旧版本、此前显式 NUMA interleave 的实验和本轮结果分别保存。','此前不符合本轮控制变量的实验已按用户要求删除；仅保留本轮受控结果。')
    p.write_text(s,encoding='utf-8')

    handoff=root/'docs/handoff';events=[json.loads(l) for l in (handoff/'events.jsonl').read_text(encoding='utf-8').splitlines() if l.strip()]
    current=[e for e in events if e['id'].startswith(PREFIXES)]
    assert len(current)>=15
    event={'id':CLEAN_RUN,'date':datetime.datetime.now(datetime.timezone.utc).isoformat(),
           'kind':'user_authorized_historical_cleanup','status':'CLEANED',
           'scope':'GCN-extra only; server wzh and local mirrors; experiment Git branch latest tree',
           'kept':'Original TFS, exact source MKL and validated modified TFS; both current measurement orders, 17 graphs including negative cases',
           'evidence':str(root/'runs'/CLEAN_RUN),
           'issues':'Previous bundles and numerical claims removed from active files. Existing Git commits remain; no history rewrite. Current frozen source snapshots stay byte-for-byte intact.',
           'next':'Use current cleaned archive and CURRENT_EVIDENCE_20261004.md'}
    current.append(event)
    (handoff/'events.jsonl').write_text(''.join(json.dumps(e,ensure_ascii=False)+'\n' for e in current),encoding='utf-8')
    for name in ['ALL_EXPERIMENTS.md','EXPERIMENT_ISSUES.md','PROJECT_PROGRESS.md']:
        body='# '+name[:-3]+' — current controlled GCN-extra experiments\n\n'
        body+='Prior experiment entries removed by explicit user request. The retained run snapshots are immutable; older package hashes in current publication events describe superseded current-run bundles. See the cleaned archive manifest for active artifacts.\n'
        for e in current:body+='\n## '+e['id']+'\n\n```json\n'+json.dumps(e,ensure_ascii=False,indent=2)+'\n```\n'
        (handoff/name).write_text(body,encoding='utf-8')

    readme='''# GCN-extra: current controlled inference experiments

Only the controlled 2026-10-04 comparison is retained. Prior experimental runs,
result reports, obsolete implementation variants and archive bundles were
deleted at the user's request. Negative results from the current comparison
remain included. Existing Git commits were not rewritten.

## The three implementation roles

| Role | Source |
|---|---|
| Original TFS v3 | [original/amx_tfs_v3.cpp](original/amx_tfs_v3.cpp) |
| Original MKL | [original/mkl_baseline.cpp](original/mkl_baseline.cpp) |
| Actual paired two-layer paper TFS/MKL harness | [original/gcn_e2e_v3.cpp](original/gcn_e2e_v3.cpp) |
| Modified TFS (neighbor reduction + tile-local AMX projection) | [src/paper_methods_kernels.hpp](src/paper_methods_kernels.hpp) |
| Checks, timing and candidate integration | [src/paper_methods_runtime.hpp](src/paper_methods_runtime.hpp) |

The actual E2E comparison uses the unmodified helpers in `gcn_e2e_v3.cpp`.
Standalone TFS/MKL files are original reference sources. The canonical modified
kernel is byte-identical to both validated build snapshots. DegreeSort,
16-row tiles, 64-row dynamic scheduling, AMX, packed W and original output order
are retained. B2/4/8/16/32/64/FULL FAST/ACCURATE are parameters of this method.
No complete graph-wide AH intermediate is produced.

## Current controls and results

- [Protocol](docs/PAPER_METHODS_PROTOCOL_20261004.md)
- [Main controlled comparison](docs/PAPER_METHODS_RESULTS_20261004.md)
- [Consecutive measurement control](docs/PAPER_CACHE_CONTROL_RESULTS_20261004.md)
- [Mechanism and timing interpretation](docs/PAPER_METHODS_INTERPRETATION_20261004.md)
- [Current evidence map and cleanup record](docs/CURRENT_EVIDENCE_20261004.md)
- [17 graph paths and full input hashes](docs/CURRENT_GRAPH_INVENTORY_20261004.json)
- [Figures](docs/figures/paper_methods_20261004/)

17 real unit-valued graphs; two-layer 128 -> 128 -> 128 inference computation;
source `srand(12345)` H/W; exact original MKL helper and hints; 32 threads and
default NUMA policy; paired denominators from the same graph/node/process.
1020 main plus 612 supplemental formal numerical checks passed. Current
consecutive B64 FAST geometric means are 1.309x vs original TFS and 2.435x vs
source MKL, winning 11/17 and 17/17 respectively. Individual slow cases remain.
These are source-compatible random H/W computation-flow experiments, not
checkpoint classification accuracy measurements.

## Reproduction and immutable evidence

`scripts/build_paper_methods.sh` generates the source-compatible harness from
the original source and canonical validated kernel. Submit with
`scripts/submit_paper_methods.sh {smoke,high,medium,low}`. The shared smoke
precedes exclusive formal jobs. Workspace AGENTS/SKILL controls apply; maximum
five nodes. No explicit NUMA/interleave or MKL algorithm change is introduced.
`build_paper_cache_control.sh` uses the retained main frozen build and changes
only measurement order. Finalizers are one-shot reconciliation tools.

Main evidence: `runs/paper-method-reconciled-20261004/`.
Consecutive evidence: `runs/paper-cache-reconciled-20261004/`.
Original compiled sources, build commands, binary hashes, launchers, full raw
measurements, numerical checks and separate stages/profiles remain in
`runs/paper-method-*` and `runs/paper-cache-*` snapshots. These snapshots are
immutable; historical launcher copies may mention deleted prior inventory
paths. Active scripts use the independent current graph inventory.

Clean bundle: `evidence-current-controlled-20261004-clean.tar.gz`, its SHA256
sidecar and `runs/cleanup-current-20261004/archive_manifest.json`. The bundle
also contains the original compiled executables, which are ignored separately
by Git. Graph datasets and toolchains stay at their original read-only paths.
'''
    (root/'README.md').write_text(readme,encoding='utf-8')
    (root/'IMPLEMENTATION_CONTRACT.md').write_text('''# Current controlled implementation contract

Authoritative controls: docs/PAPER_METHODS_PROTOCOL_20261004.md.
Keep original gcn_e2e_v3.cpp and its source MKL helpers byte-for-byte unchanged.
Use the same graph, source-seeded H/W, 128->128->128, DegreeSort, TR=16, R=64,
packed weights, activation, output precision and timing boundaries.
32 OpenMP threads on formal exclusive nodes; source compilation flags/modules;
default NUMA policy; no MKL thread/dynamic override; at most five nodes.
Write only within wzh; yx is read-only. Read applicable AGENTS.md/SKILL.md.
Modified TFS parameters are neighbor block and FAST/ACCURATE representation.
Use the canonical validated paper_methods_kernels.hpp. No numerical gate
relaxation, per-graph oracle treated as deployed policy, or cross-run timing
denominator. Preserve all current negative cases and raw repeated measurements.
No training, dynamic graph or checkpoint classification is claimed.
Previous result sets were deleted by explicit user request on 2026-10-04.
Current frozen build/launcher snapshots remain immutable evidence of actual runs.
''',encoding='utf-8')

def cleanup(root):
    assert os.name=='posix' and root==ROOT and not root.is_symlink()
    assert root.resolve().parent==pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh').resolve()
    assert str(root).startswith('/home/huangjianqiang_group/hdacp1/data/wzh/')
    r=root/'runs'/CLEAN_RUN;assert not r.exists();r.mkdir()
    before=[]
    for run in sorted((root/'runs').iterdir()):
        if run.name.startswith(PREFIXES):
            for p in sorted(run.rglob('*')):
                if p.is_file():before.append({'path':p.relative_to(root).as_posix(),'sha256':sha(p),'bytes':p.stat().st_size})
    for p in sorted((root/'original').glob('*.cpp')):
        if p.name in {'amx_tfs_v3.cpp','mkl_baseline.cpp','gcn_e2e_v3.cpp'}:
            before.append({'path':p.relative_to(root).as_posix(),'sha256':sha(p),'bytes':p.stat().st_size})
    save(r/'preserved_before.json',before)
    prepare(root)
    plan=obsolete(root);items=[]
    for rel in plan:
        p=checked(root,rel);files=[p] if p.is_file() else list(p.rglob('*'))
        items.append({'path':rel,'files':sum(x.is_file() for x in files),'bytes':sum(x.stat().st_size for x in files if x.is_file())})
    save(r/'deletion_plan.json',items)
    for rel in plan:
        p=checked(root,rel)
        if p.is_dir():shutil.rmtree(p)
        else:p.unlink()
    assert obsolete(root)==[]
    for x in before:assert sha(root/x['path'])==x['sha256'],x['path']
    generated=r/'generated-source-check';generated.mkdir()
    subprocess.run([sys.executable,str(root/'scripts/generate_paper_methods.py'),str(generated)],check=True,stdout=subprocess.PIPE)
    snap=root/'runs/paper-method-build-20261004-123445/source_snapshot'
    for n in ['paper_methods.cpp','paper_methods_kernels.hpp']:
        assert (generated/n).read_bytes()==(snap/n).read_bytes(),n
    subprocess.run([sys.executable,str(root/'scripts/record_event.py'),'check'],check=True)
    summary={'status':'PASS','deleted_entries':len(items),'deleted_files':sum(x['files'] for x in items),
             'deleted_bytes':sum(x['bytes'] for x in items),'preserved_hashes':len(before),
             'current_formal_checks':1632,'graphs':17,'frozen_evidence_unchanged':True,
             'generated_source_and_kernel_match_compiled_snapshot':True,'benchmarks_executed':False,
             'git_history_rewritten':False}
    save(r/'cleanup_summary.json',summary)
    (root/'docs/CURRENT_EVIDENCE_20261004.md').write_text('''# Current evidence and authorized cleanup

Retained: original TFS, exact original source MKL, modified TFS; 17-graph main
controlled sweep plus consecutive-measurement control, including all slow cases.
Main raw data: ../runs/paper-method-reconciled-20261004/.
Supplement raw data: ../runs/paper-cache-reconciled-20261004/.
Sources and reproducible active scripts are linked in ../README.md.

Removed: superseded experiment runs, reports, root source variants, their
launch/build scripts, all superseded archives and historical result entries
in the active handoffs. Graph data and toolchains were not touched. Unrelated
GAT work was not touched. The Git experiment branch will record ordinary
deletions; previous commits remain available, without force push/history rewrite.

Before/after hashes protect every retained current run file and all three
original source files. Active generated .cpp and kernel bytes match the
actually compiled main snapshot. Graph inventory now contains independent
full input hashes; no active launcher depends on a deleted old run directory.
No performance measurement was repeated or changed during cleanup.

Cleanup inventory and verification: ../runs/cleanup-current-20261004/.
Clean archive manifest there supersedes the old current-run packaging manifest,
which is retained only as immutable provenance of the earlier packaging step.
Frozen snapshots retain actual generation inputs/launchers even when they are
obsolete today; they are not alternative active implementations.
''',encoding='utf-8')
    (root/'.gitattributes').write_text('** -text\n')
    print(json.dumps(summary))
    subprocess.run([sys.executable,str(root/'scripts/package_current_evidence.py')],check=True)

if __name__=='__main__': cleanup(ROOT)
