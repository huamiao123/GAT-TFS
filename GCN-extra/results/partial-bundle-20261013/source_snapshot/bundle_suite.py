#!/usr/bin/env python3
import csv
import hashlib
import json
import math
import os
import pathlib
import shlex
import statistics
import struct
import subprocess
import sys
import time

ROOT = pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
run, build = map(pathlib.Path, sys.argv[1:3])
mode = sys.argv[3]


def save(p, x):
    p.write_text(json.dumps(x, indent=2) + '\n')


def sha(p):
    h = hashlib.sha256()
    with p.open('rb') as stream:
        for b in iter(lambda: stream.read(8 * 1024 * 1024), b''):
            h.update(b)
    return h.hexdigest()


def table(p, rows):
    if not rows:
        return
    keys = list(dict.fromkeys(k for r in rows for k in r))
    with p.open('w', newline='') as stream:
        w = csv.DictWriter(stream, keys)
        w.writeheader()
        w.writerows(rows)


graphs = []
if mode == 'smoke':
    for n in [1, 15, 17, 33, 65]:
        name = 'bundle_tail_' + str(n)
        folder = run / 'fixtures' / name
        folder.mkdir(parents=True)
        row, col = [0], []
        degrees = [0, 1, 63, 64, 65, 255, 256, 257, 511, 512, 513, 769, 1025]
        for i in range(n):
            d = degrees[(i + 1) % len(degrees)] if i % 4 else 0
            if n == 1:
                d = 513
            col.extend((i * 3 + j * 7) % n for j in range(d))
            row.append(len(col))
        f = folder / (name + '.csrbin')
        with f.open('wb') as stream:
            stream.write(struct.pack('<IIIQQQ', 0, 0, 2, n, n, len(col)))
            stream.write(struct.pack('<' + 'I' * len(row), *row))
            stream.write(struct.pack('<' + 'I' * len(col), *col))
            stream.write(struct.pack('<' + 'f' * len(col), *([1.] * len(col))))
        graphs.append({'graph': name, 'path': str(f), 'N': n, 'E': len(col),
                       'fixture': 'empty_tail_unsorted_duplicate_scope_and_bundle_boundaries'})
else:
    inventory = {r['graph']: r for r in json.loads((ROOT / 'docs/CURRENT_GRAPH_INVENTORY_20261004.json').read_text())}
    graphs = [inventory[g] for g in mode.split(',')]

os.environ['STATE_SMOKE'] = '1' if mode == 'smoke' else '0'
os.environ['STATE_ABLATIONS'] = '1'
os.environ['STATE_PMU'] = '0' if mode == 'smoke' else '1'
save(run / 'graph_list.json', graphs)
packets, all_tables, summaries = [], {}, []
for graph in graphs:
    name = graph['graph']
    folder = run / name
    folder.mkdir()
    path = pathlib.Path(graph['path'])
    packet = dict(graph)
    packet['actual_input_sha256'] = sha(path)
    if 'input_sha256' in graph:
        assert packet['actual_input_sha256'] == graph['input_sha256']
    argv = [str(build / 'bundle_methods'), str(path.parent.parent), name]
    save(folder / 'command.json', {'argv': argv, 'cwd': str(folder), 'mode': mode})
    print('BUNDLE_START', name, flush=True)
    t = time.time()
    with (folder / 'program.log').open('w') as out, (folder / 'program.err').open('w') as err:
        code = subprocess.run(argv, cwd=folder, stdout=out, stderr=err, timeout=2700).returncode
    tables = {}
    for line in (folder / 'program.log').read_text(errors='replace').splitlines():
        label, _, body = line.partition(' ')
        if label.startswith(('STATE_', 'BUNDLE_', 'METHOD_', 'GROUP_')):
            tables.setdefault(label, []).append(dict(x.split('=', 1) for x in shlex.split(body) if '=' in x))
    for key, rows in tables.items():
        all_tables.setdefault(key, []).extend(rows)
    packet['exit_code'] = code
    packet['elapsed_seconds'] = time.time() - t
    try:
        assert code == 0, ('process', code)
        assert len(tables.get('STATE_COMPLETE', [])) == 1 and tables['STATE_COMPLETE'][0]['pass'] == '1'
        for key in ['METHOD_CHECK', 'STATE_EQUIV', 'STATE_SEMANTIC', 'STATE_PROFILE_GATE', 'STATE_WORK_GATE']:
            assert tables.get(key) and all(r['pass'] == '1' for r in tables[key]), key
        plans = tables['STATE_PLAN']
        assert len(plans) == 22 and len({r['method'] for r in plans}) == 22
        for p in plans:
            m = p['method']
            checks = [r for r in tables['METHOD_CHECK'] if r['method'] == m]
            assert len(checks) == 4, (m, 'numeric boundaries')
            if int(p['kind']) >= 2:
                eq = [r for r in tables['STATE_EQUIV'] if r['method'] == m]
                assert len(eq) == 4 and all(r['bitwise'] == '1' for r in eq), m
            if int(p['kind']) == 3:
                assert len([r for r in tables['STATE_PROFILE_GATE'] if r['method'] == m]) == 2
                assert len([r for r in tables['STATE_WORK_GATE'] if r['method'] == m]) == 2
        # Independently verify counts and same-scope invariants from model records.
        model = {r['method']: r for r in tables['BUNDLE_MODEL']}
        counted = tables['BUNDLE_COUNTERS']
        for r in counted:
            q = model[r['method']]
            assert int(r['source_visits']) == int(q['E'])
            assert int(r['tdp']) == int(q['tdp'])
            assert int(r['C_store_tiles']) * 1024 == int(q['C_store_bytes'])
            assert int(r['C_reload_tiles']) * 1024 == int(q['C_reload_bytes'])
            p = next(p for p in plans if p['method'] == r['method'])
            control = 'pack_epi_' + ('m' + p['scope'] if p['scope'] != '0' else 'full') + ('_accurate' if p['accurate'] == '1' else '_fast')
            ref = next(x for x in counted if x['method'] == control and x['layer'] == r['layer'])
            for k in ['source_visits', 'feature_adds', 'decode_rows', 'scopes', 'tdp',
                      'partial_load_bytes', 'partial_store_bytes', 'packed_bytes',
                      'csr_reads', 'prefetch_requests', 'output_bytes']:
                assert r[k] == ref[k], (r['method'], k, r[k], ref[k])
        if mode != 'smoke':
            times = tables['STATE_TIME']
            assert len(times) == 23 * 15
            for m in [p['method'] for p in plans] + ['source_mkl_fp32']:
                for kind, count in [('consecutive', 5), ('alternating', 10)]:
                    vals = [float(r['ms']) for r in times if r['method'] == m and r['kind'] == kind]
                    assert len(vals) == count
                    mean = statistics.mean(vals)
                    summaries.append({'graph': name, 'method': m, 'protocol': kind,
                                      'median_ms': statistics.median(vals), 'min_ms': min(vals), 'max_ms': max(vals),
                                      'CV': statistics.stdev(vals) / mean, 'samples': json.dumps(vals), 'repeats': count})
        packet['status'] = 'PASS'
    except Exception as exc:
        packet['status'] = 'FAILED'
        packet['error'] = repr(exc)
    packets.append(packet)
    save(folder / 'status.json', packet)
    print('BUNDLE_FINISH', name, packet['status'], flush=True)
    # Keep parsable partial evidence even when a later graph or gate fails.
    for key, rows in all_tables.items():
        table(run / (key.lower() + '.csv'), rows)
    table(run / 'timing_summary.csv', summaries)
    save(run / 'completion.json', {'status': packet['status'], 'graphs': packets,
                                   'counts': {k: len(v) for k, v in all_tables.items()}})
    if packet['status'] != 'PASS':
        raise RuntimeError(packet)
