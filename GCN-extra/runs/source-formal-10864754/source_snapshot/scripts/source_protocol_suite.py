#!/usr/bin/env python3
"""Run source-faithful LP64 comparison; keep every exclusion and failure."""
import csv
import hashlib
import json
import os
import pathlib
import re
import resource
import signal
import statistics
import struct
import subprocess
import sys
import time

run, build = map(pathlib.Path, sys.argv[1:3])
mode = sys.argv[3]
root = pathlib.Path(__file__).resolve().parents[1]
resource.setrlimit(resource.RLIMIT_CORE, (0, 0))


def save(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n')


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(8 * 1024 * 1024), b''):
            h.update(chunk)
    return h.hexdigest()


def fixture(name, n, degrees):
    folder = run / 'fixtures' / name
    folder.mkdir(parents=True)
    row, col = [0], []
    for i in range(n):
        degree = degrees[i % len(degrees)]
        col.extend((i * 7 + j * 11) % n for j in range(degree))
        row.append(len(col))
    path = folder / (name + '.csrbin')
    with path.open('wb') as f:
        f.write(struct.pack('<IIIQQQ', 0, 0, 2, n, n, len(col)))
        f.write(struct.pack('<' + str(len(row)) + 'I', *row))
        f.write(struct.pack('<' + str(len(col)) + 'I', *col))
        f.write(struct.pack('<' + str(len(col)) + 'f', *([1.0] * len(col))))
    return dict(graph=name, path=str(path), N=n, E=len(col), avg_degree=len(col)/n,
                supported_header=True, dataset_sha256=digest(path))


def write_csv(path, rows):
    if rows:
        keys = list(dict.fromkeys(k for r in rows for k in r))
        with path.open('w', newline='') as f:
            w = csv.DictWriter(f, keys)
            w.writeheader()
            w.writerows(rows)


def parse(folder):
    tables = {k: [] for k in ['TIME', 'CORRECT', 'STAGE', 'PROFILE', 'CONFIG', 'INPUT', 'COMPLETE']}
    section, repeat = None, 0
    for line in (folder / 'stdout.log').read_text(errors='replace').splitlines():
        if line.startswith('=== TFS V3'):
            section, repeat = 'source_original_tfs', 0
        elif line.startswith('=== MKL FP32'):
            section, repeat = 'source_mkl_fp32', 0
        elif line.startswith('==='):
            section = None
        match = re.match(r'\s+run (\d+): ([0-9.]+) ms', line)
        if section and match:
            tables['TIME'].append(dict(method=section, kind='e2e', repeat=match[1],
                                       ms=match[2], precision='source_stdout_0.01ms'))
            repeat += 1
        if line.startswith('SOURCE_'):
            label, _, body = line.partition(' ')
            key = label[7:]
            if key in tables:
                tables[key].append(dict(item.split('=', 1) for item in body.split() if '=' in item))
    for key, rows in tables.items():
        write_csv(folder / (key.lower() + '.csv'), rows)
    save(folder / 'parsed.json', tables)
    if len(tables['COMPLETE']) != 1 or tables['COMPLETE'][0].get('pass') != '1':
        raise RuntimeError('Missing successful SOURCE_COMPLETE')
    if not tables['CORRECT'] or any(x.get('pass') != '1' for x in tables['CORRECT']):
        raise RuntimeError('Numerical check missing or failed')
    groups = {}
    for t in tables['TIME']:
        groups.setdefault((t['method'], t['kind']), []).append(float(t['ms']))
    if any(len(v) != 5 for v in groups.values()):
        raise RuntimeError('Expected exactly five measured repetitions')
    summaries = [dict(method=k[0], kind=k[1], min_ms=min(v), median_ms=statistics.median(v),
                      p25_ms=sorted(v)[1], p75_ms=sorted(v)[3], repeats=5) for k, v in groups.items()]
    baseline = {s['method']: s['min_ms'] for s in summaries if s['kind'] == 'e2e'}
    for s in summaries:
        if s['kind'] == 'e2e':
            s['speedup_vs_source_mkl'] = baseline['source_mkl_fp32'] / s['min_ms']
            s['speedup_vs_source_tfs'] = baseline['source_original_tfs'] / s['min_ms']
    write_csv(folder / 'summary.csv', summaries)
    return summaries


def execute(cmd, folder, label, env):
    save(folder / (label + '_command.json'), dict(argv=cmd, explicit_numa_policy=None,
         cwd=str(folder), thread_env={k: env.get(k) for k in
         ['OMP_NUM_THREADS', 'OMP_PROC_BIND', 'OMP_PLACES', 'MKL_NUM_THREADS', 'MKL_DYNAMIC']}))
    with (folder / (label + '.log')).open('w') as so, (folder / (label + '.err')).open('w') as se:
        result = subprocess.run(cmd, cwd=folder, env=env, stdout=so, stderr=se)
    if result.returncode:
        tail = (folder / (label + '.err')).read_text(errors='replace')[-2000:]
        if result.returncode < 0:
            tail += ' signal=' + signal.Signals(-result.returncode).name
        raise RuntimeError('exit=' + str(result.returncode) + ' ' + tail)


if mode == 'smoke':
    graphs = [fixture('source_tail', 37, [0, 1, 2, 7, 8, 9, 16, 31, 32, 33, 37]),
              fixture('source_high_tail', 137, [0, 1, 63, 64, 65, 127, 128, 130])]
else:
    historical = root / 'runs/formal-10862002'
    graphs = json.loads((historical / 'graph_list.json').read_text())
    known = {x['graph']: x for x in json.loads((historical / 'suite_status.json').read_text())}
    for g in graphs:
        g['dataset_sha256'] = known.get(g['graph'], {}).get('dataset_sha256')
    graphs.sort(key=lambda g: (-g['avg_degree'], g['graph']))
save(run / 'graph_list.json', graphs)
results, total_summaries = [], []
nonunit = {'kron_g500-logn21', 'cage15', 'FullChip', 'scircuit', 'sx-stackoverflow', 'rajat31'}
for graph in graphs:
    name = graph['graph']
    folder = run / name
    folder.mkdir()
    record = dict(graph)
    reason = None
    if name in nonunit:
        reason = 'Previously verified nonunit CSR values; original TFS ignores values while MKL uses them'
    elif graph['E'] > 2147483647 or graph['N'] * 128 >= 2147483648:
        reason = 'Original LP64 MKL range limit; ILP64 changes are outside source-faithful protocol'
    elif not graph['supported_header']:
        reason = 'Unsupported CSR header'
    if reason:
        record.update(status='SKIPPED', reason=reason)
    else:
        start = time.time()
        print('START_SOURCE_GRAPH', name, 'q', graph['avg_degree'], flush=True)
        try:
            path = pathlib.Path(graph['path'])
            actual = digest(path)
            if graph.get('dataset_sha256') and actual != graph['dataset_sha256']:
                raise RuntimeError('Dataset hash changed since frozen graph suite')
            record['dataset_sha256'] = actual
            with (run / 'dataset.sha256').open('a') as f:
                f.write(actual + '  ' + str(path) + '\n')
            env = dict(os.environ)
            env['GCN_SOURCE_BLOCKS'] = '1,2,4,8,16,32,64,full' if mode == 'smoke' else '2,4,8,16,32,64,full'
            data_root = str(path.parent.parent)
            # Untouched standalone binaries provide anchors on two representative real graphs.
            if mode == 'formal' and name in {'reddit', 'ogbn-products'}:
                for binary in ['source_mkl_raw', 'source_tfs_raw', 'source_e2e_raw']:
                    execute([str(build / binary), data_root, name], folder, binary, env)
            execute([str(build / 'source_candidates'), data_root, name], folder, 'stdout', env)
            summaries = parse(folder)
            total_summaries.extend(dict(graph=name, **x) for x in summaries)
            record.update(status='PASS', correctness_checks=len(json.loads((folder/'parsed.json').read_text())['CORRECT']))
        except Exception as e:
            record.update(status='FAILED', reason=str(e))
        record['elapsed_s'] = time.time() - start
        print('END_SOURCE_GRAPH', name, record['status'], record.get('reason', ''), flush=True)
    save(folder / 'status.json', record)
    results.append(record)
    save(run / 'suite_status.json', results)
    write_csv(run / 'summary.csv', total_summaries)

lines = ['# Original-source MKL/TFS protocol comparison', '',
         'All paths use default NUMA policy; no explicit interleave/bind/membind. Original source MKL is FP32; TFS/candidates use BF16 inputs and FP32 outputs.', '',
         'Primary statistic: minimum of five after one warmup. Secondary median and every raw repetition are retained. Prepared two-layer E2E includes ReLU and intermediate conversion, excludes loading/sorting/initial packing.', '',
         '| Graph | Method | E2E min ms | E2E median ms | vs source MKL | vs source TFS |',
         '|---|---|---:|---:|---:|---:|']
for x in total_summaries:
    if x['kind'] == 'e2e':
        lines.append('| {graph} | {method} | {min_ms:.3f} | {median_ms:.3f} | {speedup_vs_source_mkl:.3f}x | {speedup_vs_source_tfs:.3f}x |'.format(**x))
lines += ['', '## Status and exclusions', '']
lines.extend('- ' + x['graph'] + ': ' + x['status'] + (' — '+x['reason'] if 'reason' in x else '') for x in results)
lines += ['', 'Historical results with explicit NUMA interleave are a different protocol and must not be combined with this table.',
          'Baseline E2E values retain the original 0.01 ms stdout rounding. Stage/profile experiments are separate from primary timing; sampled thread time is not wall-time percentage.',
          'Random H/W exercise the source inference operator; trained-model accuracy and full application end-to-end are not claimed.', '']
(run / 'REPORT.md').write_text('\n'.join(lines))
print('SOURCE_SUITE_COMPLETE', len(results), 'passed', sum(x['status']=='PASS' for x in results), flush=True)
sys.exit(1 if any(x['status']=='FAILED' for x in results) else 0)
