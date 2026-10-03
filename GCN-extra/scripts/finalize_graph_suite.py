#!/usr/bin/env python3
"""Reconcile completed coverage and bundle its immutable experiment evidence."""
import hashlib
import json
import pathlib
import subprocess

root = pathlib.Path(__file__).resolve().parents[1]
jobs = ['10862001','10862002','10862091','10862093','10862158','10862162']
event_path = root/'docs/GRAPH_SUITE_RESULTS_20261003.event.json'
event = json.loads(event_path.read_text(encoding='utf-8'))
assert (event['attempted_graphs'],event['passed_graphs'],event['correctness_records'],event['failed_correctness_records']) == (25,19,2028,0)
for job in ['10862158','10862162']:
    status = json.loads((root/f'runs/formal-{job}/event.json').read_text(encoding='utf-8'))
    assert status['status']=='PASS' and status['exit_status']==0, status

def digest(path):
    h=hashlib.sha256()
    with path.open('rb') as f:
        for block in iter(lambda:f.read(1024*1024),b''):h.update(block)
    return h.hexdigest()

assert digest(root/'src/bench.cpp')=='b33c962bac335ff3059e03f8cc735e97932ec08921c322f76148490eb0a9c0c6'
original_hash='4e574786055816df80beb6e68181fe19a8ee02cf4be1d43717bc1e79b88cab29'
assert digest(root/'original/amx_tfs_v3.cpp')==original_hash
assert digest(root.parent.parent/'yx/TFS/code/amx_tfs_v3.cpp')==original_hash
accounting=subprocess.check_output(['sacct','-j',','.join(jobs),'--format=JobID,State,Elapsed,ExitCode,MaxRSS','-P'],text=True)
assert '10862158|COMPLETED|' in accounting and '10862162|COMPLETED|' in accounting
account_path=root/'docs/GRAPH_SUITE_ACCOUNTING_20261003.txt'
account_path.write_text(accounting,encoding='utf-8')
event.update({
    'accounting':str(account_path),
    'figures':str(root/'docs/figures/graph_suite'),
    'performance':{'best_fast_posthoc_geomean':1.350,'fixed_full_fast_geomean':1.290,'best_accurate_posthoc_geomean':1.297,'fast_faster_graphs':17,'completed_graphs':19},
    'resolved_issues':[
        'Road-USA and Friendster passed unchanged AMX/local kernels after MKL ILP64 recovery; historical LP64 preparation crashes remain preserved. Exact MKL fault line is unverified.',
        'Future LP64 supervisor skips E or N*128 beyond signed 32-bit limits.',
        'Future submissions explicitly set workspace cwd; child core dumps are disabled.',
        'Full does not dominate B64 on Mycielskian19; no claim of automatic B selection.',
        'Total gains include grouping, cross-output-panel gather reuse and empty-row output handling; causal shares need ablation.'
    ],
    'next':'Completed all 25 canonical attempts; six nonunit-value inputs remain excluded. Investigate Full/B64 crossover and task/shape validation before paper claims.'
})
event_path.write_text(json.dumps(event,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
subprocess.run(['python3',str(root/'scripts/record_event.py'),str(event_path)],check=True)
subprocess.run(['python3',str(root/'scripts/record_event.py'),'check'],check=True)
manifest_paths=[root/'src/bench.cpp',root/'src/original_nozero.hpp',root/'original/amx_tfs_v3.cpp',root/'README.md',account_path,event_path]
manifest_paths += sorted((root/'scripts').glob('*.*'))
manifest_paths += [root/'docs/GRAPH_SUITE_RESULTS_20261003.md',root/'docs/GRAPH_SUITE_RESULTS_20261003.csv']
manifest_paths += sorted((root/'docs/figures/graph_suite').glob('*'))
manifest_paths += sorted((root/'docs/handoff').glob('*.md'))+[root/'docs/handoff/events.jsonl']
manifest=root/'docs/GRAPH_SUITE_FINAL_ARTIFACTS_20261003.sha256'
manifest.write_text(''.join(f'{digest(p)}  {p.relative_to(root).as_posix()}\n' for p in manifest_paths if p.is_file()),encoding='utf-8')
archive=root/'evidence-graph-suite-completed-20261003.tar.gz'
assert not archive.exists(), 'Do not overwrite completed archives'
subprocess.run(['tar','-czf',str(archive),'-C',str(root),'src','original','scripts','docs','runs','README.md','IMPLEMENTATION_CONTRACT.md'],check=True)
archive_hash=digest(archive)
archive.with_name(archive.name+'.sha256').write_text(f'{archive_hash}  {archive.name}\n',encoding='utf-8')
print(json.dumps({'archive':str(archive),'sha256':archive_hash,'successful_graphs':19,'correctness_records':2028},ensure_ascii=False))
