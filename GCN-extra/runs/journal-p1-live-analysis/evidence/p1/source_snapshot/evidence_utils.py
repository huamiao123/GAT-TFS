#!/usr/bin/env python3
"""Run the frozen source harness and the S/M window experiment without log loss.

The primary protocol is consecutive E2E; rotating E2E is a separate control.
All baseline ratios are formed within one graph, protocol, node, and process.
"""
import csv
import hashlib
import json
import math
import os
import pathlib
import re
import resource
import shlex
import statistics
import struct
import subprocess
import sys
import time

ROOT = pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
KINDS = ('consecutive_e2e', 'rotating_e2e')
CHECK_PAIRS = {
    ('kernel_FP32', 'paper_tfs'),
    ('e2e_FP32_final', 'paper_tfs'),
    ('e2e_BF16_final', 'paper_tfs'),
    ('e2e_BF16_final', 'source_MKL_FP32'),
}
EQ_BOUNDARIES = {'kernel_FP32', 'e2e_FP32_final', 'e2e_BF16_final'}
PARTS = {
    'focus': ['ogbn-products', 'reddit', 'mycielskian19', 'roadNet-CA'],
    'validation': [
        'hollywood-2009', 'indochina-2004', 'soc-Pokec', 'cit-Patents',
        'soc-LiveJournal1', 'com-LiveJournal', 'as-Skitter', 'rgg_n_2_24_s0',
        'web-Google', 'wiki-Talk', 'email-Enron', 'amazon0601',
        'com-Youtube',
    ],
}


def save(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n')


def sha(path):
    digest = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(8 * 1024 * 1024), b''):
            digest.update(block)
    return digest.hexdigest()


def csvfile(path, rows):
    if not rows:
        return
    keys = list(dict.fromkeys(key for row in rows for key in row))
    with path.open('w', newline='') as stream:
        writer = csv.DictWriter(stream, keys)
        writer.writeheader()
        writer.writerows(rows)


def invoke(folder, label, argv):
    save(folder / (label + '.command.json'), {
        'argv': list(map(str, argv)), 'cwd': str(folder), 'numa_policy': None,
        'env': {key: os.environ.get(key) for key in (
            'OMP_NUM_THREADS', 'OMP_PROC_BIND', 'OMP_PLACES', 'MKL_NUM_THREADS',
            'MKL_DYNAMIC', 'GCN_PROJECTION_WINDOWS', 'GCN_PROJECTION_SCOPES',
            'GCN_PROJECTION_METHODS', 'GCN_PAPER_BLOCKS',
        )},
    })
    start = time.time()
    try:
        with (folder / (label + '.log')).open('w') as out, (folder / (label + '.err')).open('w') as err:
            result = subprocess.run(list(map(str, argv)), cwd=str(folder),
                                    stdout=out, stderr=err, timeout=2400)
    except subprocess.TimeoutExpired:
        save(folder / (label + '.status.json'), {
            'returncode': None, 'timed_out': True,
            'wall_seconds': time.time() - start,
        })
        raise
    save(folder / (label + '.status.json'), {
        'returncode': result.returncode, 'wall_seconds': time.time() - start,
    })
    if result.returncode:
        raise RuntimeError(label + ' exit ' + str(result.returncode))
    return (folder / (label + '.log')).read_text(errors='replace')


def baseline(log):
    groups = {}
    current = None
    for line in log.splitlines():
        if line.startswith('=== TFS V3'):
            current = 'paper_tfs'
        elif line.startswith('=== MKL FP32'):
            current = 'source_mkl_fp32'
        elif line.startswith('==='):
            current = None
        match = re.match(r'\s+run (\d+): ([0-9.]+) ms', line)
        if current and match:
            groups.setdefault(current, []).append(float(match[2]))
    assert set(groups) == {'paper_tfs', 'source_mkl_fp32'}, 'Missing raw source baseline'
    assert all(len(values) == 5 for values in groups.values()), 'Raw baseline repeat count'
    return {key: {
        'min_ms': min(values), 'median_ms': statistics.median(values),
        'raw_ms': values, 'printed_precision_decimal_places': 2,
    } for key, values in groups.items()}


def parse(folder, log, raw_anchor):
    tables = {key: [] for key in (
        'CHECK', 'EQUIV', 'TIME', 'STAGE', 'PROFILE', 'CONFIG', 'MODEL',
        'COUNTERS', 'GATE_COMPLETE', 'COMPLETE',
    )}
    for line in log.splitlines():
        if not line.startswith('METHOD_'):
            continue
        label, _, body = line.partition(' ')
        key = label[7:]
        # Preserve new METHOD records rather than silently dropping metadata.
        tables.setdefault(key, []).append(dict(
            item.split('=', 1) for item in shlex.split(body) if '=' in item
        ))
    # Write parsed evidence before assertions, so failed runs remain auditable.
    for key, rows in tables.items():
        csvfile(folder / (key.lower() + '.csv'), rows)
    save(folder / 'parsed.json', tables)

    assert len(tables['CONFIG']) == 1, 'Missing or duplicate run configuration'
    config = tables['CONFIG'][0]
    assert all(config.get(key) == value for key, value in {
        'D': '128', 'F': '128', 'R': '64', 'TR': '16',
        'warmups': '1', 'repeats': '5', 'final_output': 'BF16', 'numa': 'default',
    }.items()), 'Frozen configuration changed'
    assert all(row.get('graph', folder.name) == folder.name
               for rows in tables.values() for row in rows), 'Cross-graph records'
    assert len(tables['COMPLETE']) == 1 and tables['COMPLETE'][0]['pass'] == '1'
    assert len(tables['GATE_COMPLETE']) == 1 and tables['GATE_COMPLETE'][0]['pass'] == '1'
    count = int(tables['COMPLETE'][0]['methods'])
    checks = tables['CHECK']
    assert len(checks) == count * 4 and all(row['pass'] == '1' for row in checks)
    method_names = {row['method'] for row in checks}
    assert len(method_names) == count and 'paper_tfs' in method_names
    for method in method_names:
        rows = [row for row in checks if row['method'] == method]
        assert len(rows) == 4 and {
            (row['boundary'], row['reference']) for row in rows
        } == CHECK_PAIRS, 'Incorrect per-method numerical gate coverage: ' + method
        accurate = method.endswith('_accurate')
        for row in rows:
            if row['reference'] == 'source_MKL_FP32':
                expected_gate = .03
            elif row['boundary'] == 'e2e_BF16_final':
                expected_gate = .01 if accurate else .02
            else:
                expected_gate = 1e-3 if accurate else 1e-2
            assert float(row['gate']) == expected_gate, 'Frozen numerical gate changed: ' + method

    equiv = tables['EQUIV']
    assert equiv and all(row.get('bitwise') == '1' and row.get('pass') == '1' for row in equiv), 'Non-bitwise S/M equivalence'
    eq_methods = {row['method'] for row in equiv}
    # Unchanged source/coupled baselines are exempt; every decoupled S/M path is not.
    expected_eq_methods = {
        method for method in method_names
        if re.fullmatch(r's\d+_m(?:\d+|full)(?:_rowwise)?_(?:fast|accurate)', method)
    }
    assert expected_eq_methods and eq_methods == expected_eq_methods, 'Missing coupled-M equivalence method'
    for method in eq_methods:
        rows = [row for row in equiv if row['method'] == method]
        assert len(rows) == 3 and {row['boundary'] for row in rows} == EQ_BOUNDARIES, 'Incorrect equivalence boundaries: ' + method
    expected_eq = len(expected_eq_methods) * 3
    assert len(equiv) == expected_eq
    a1 = {int(row['layer']): row for row in tables['COUNTERS'] if row['method'] == 's64_mfull_fast'}
    a3 = {int(row['layer']): row for row in tables.get('A3_COUNTERS', []) if row['method'] == 's64_mfull_rowwise_fast'}
    assert set(a1) == set(a3) == {1, 2}, 'A1/A3 counter layer coverage'
    for layer in (1, 2):
        for first, second in (('source_visits', 'source_visits'),
                              ('projection_scopes', 'projection_scopes'),
                              ('neighbor_windows', 'neighbor_windows'),
                              ('window_state_load_bytes', 'state_load_bytes'),
                              ('window_state_store_bytes', 'state_store_bytes'),
                              ('amx_dpbf16ps', 'amx_dpbf16ps')):
            assert a1[layer][first] == a3[layer][second], 'A1/A3 work mismatch: ' + first
    if 'smoke' in folder.parent.name.split('-'):
        partial = tables.get('PARTIAL_EQUIV', [])
        assert len(partial) == 2 and all(row.get('method') == 's64_mfull_rowwise_fast' and row.get('bitwise') == '1' and row.get('pass') == '1' for row in partial), 'Missing A3 FULL partial checks'
    if 'equiv_checks' in tables['COMPLETE'][0]:
        assert int(tables['COMPLETE'][0]['equiv_checks']) == expected_eq

    groups = {}
    for row in tables['TIME']:
        assert row['kind'] in KINDS, 'Unexpected timing protocol: ' + row['kind']
        value = float(row['ms'])
        assert math.isfinite(value) and value > 0, 'Invalid timing value'
        groups.setdefault((row['method'], row['kind']), []).append(row)
    expected_keys = {(method, kind) for method in method_names | {'source_mkl_fp32'} for kind in KINDS}
    assert set(groups) == expected_keys, 'Missing or extra timed method/protocol'
    assert all(len(rows) == 5 and {int(row['repeat']) for row in rows} == set(range(5)) for rows in groups.values()), 'Timing repetitions must be unique 0..4'
    summaries = []
    for (method, kind), rows in groups.items():
        values = [float(row['ms']) for row in rows]
        summaries.append({
            'graph': folder.name, 'method': method, 'kind': kind,
            'min_ms': min(values), 'median_ms': statistics.median(values),
            'max_ms': max(values), 'repeats': 5,
            'primary_protocol': int(kind == 'consecutive_e2e'),
        })
    lookup = {(row['method'], row['kind']): row for row in summaries}
    for row in summaries:
        for stat in ('min', 'median'):
            denominator = row[stat + '_ms']
            row['speedup_' + stat + '_vs_mkl'] = lookup['source_mkl_fp32', row['kind']][stat + '_ms'] / denominator
            row['speedup_' + stat + '_vs_tfs'] = lookup['paper_tfs', row['kind']][stat + '_ms'] / denominator
    csvfile(folder / 'summary.csv', summaries)
    embedded = baseline(log)
    audit = []
    for method in ('paper_tfs', 'source_mkl_fp32'):
        raw_min = raw_anchor[method]['min_ms']
        audit.append({
            'method': method, 'raw_anchor': raw_anchor[method],
            'embedded_original': embedded[method],
            'consecutive_e2e': lookup[method, 'consecutive_e2e'],
            'rotating_e2e': lookup[method, 'rotating_e2e'],
            'embedded_vs_raw_min_ratio': embedded[method]['min_ms'] / raw_min if raw_min else None,
            'warning': 'Raw anchor is a separate process with source two-decimal printed times; primary speedups use same-process protocol baselines.',
        })
    save(folder / 'baseline_anchor_audit.json', audit)
    return summaries, tables, audit


def fixtures(run):
    graphs = []
    degrees = [0, 1, 7, 8, 9, 16, 31, 32, 33, 63, 64, 65,
               127, 128, 129, 255, 256, 257, 511, 512, 513]
    for name, n in (('window_tail', 37), ('window_high_tail', 137), ('window_scope_tail', 521)):
        folder = run / 'fixtures' / name
        folder.mkdir(parents=True)
        row, col = [0], []
        for node in range(n):
            degree = degrees[node % len(degrees)]
            col.extend((node * 7 + neighbor * 11) % n for neighbor in range(degree))
            row.append(len(col))
        path = folder / (name + '.csrbin')
        with path.open('wb') as stream:
            stream.write(struct.pack('<IIIQQQ', 0, 0, 2, n, n, len(col)))
            stream.write(struct.pack('<' + 'I' * len(row), *row))
            stream.write(struct.pack('<' + 'I' * len(col), *col))
            stream.write(struct.pack('<' + 'f' * len(col), *([1.] * len(col))))
        graphs.append({
            'graph': name, 'path': str(path), 'N': n, 'E': len(col),
            'avg_degree': len(col) / n, 'fixture_degree_pattern': degrees,
            'duplicate_source_edges_allowed': True,
        })
    return graphs


def main():
    run, build = map(pathlib.Path, sys.argv[1:3])
    mode = sys.argv[3]
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    if mode == 'smoke':
        graphs = fixtures(run)
        known = {}
    else:
        assert mode in PARTS, 'Unsupported graph mode: ' + mode
        inventory = json.loads((ROOT / 'docs/CURRENT_GRAPH_INVENTORY_20261004.json').read_text())
        known = {row['graph']: row for row in inventory}
        graphs = [known[name] for name in PARTS[mode]]
    save(run / 'graph_list.json', graphs)
    states, allrows, allchecks, allequiv, failures = [], [], [], [], 0
    for graph in graphs:
        folder = run / graph['graph']
        folder.mkdir()
        state = dict(graph)
        try:
            print('METHOD_START', graph['graph'], flush=True)
            start = time.time()
            path = pathlib.Path(graph['path'])
            state['input_sha256'] = sha(path)
            if mode != 'smoke':
                assert state['input_sha256'] == known[graph['graph']]['input_sha256'], 'Graph hash changed'
                assert graph['E'] <= 2147483647 and graph['N'] * 128 <= 2147483648, 'Original LP64 limit'
            data = path.parent.parent
            anchor = baseline(invoke(folder, 'original_raw', [build / 'paper_original', data, graph['graph']]))
            log = invoke(folder, 'methods', [build / 'projection_methods', data, graph['graph']])
            summaries, tables, audit = parse(folder, log, anchor)
            state.update({
                'status': 'PASS', 'wall_seconds': time.time() - start,
                'checks': len(tables['CHECK']), 'equiv_checks': len(tables['EQUIV']),
                'anchor_audit': audit,
            })
            allrows.extend(summaries)
            allchecks.extend(tables['CHECK'])
            allequiv.extend(tables['EQUIV'])
            print('METHOD_PASS', graph['graph'], json.dumps([
                row for row in summaries if row['kind'] == 'consecutive_e2e'
            ]), flush=True)
        except Exception as exc:
            failures += 1
            state.update(status='FAILED', reason=str(exc))
            print('METHOD_FAILED', graph['graph'], str(exc), flush=True)
        states.append(state)
        save(folder / 'status.json', state)
        save(run / 'suite_status.json', states)
        csvfile(run / 'summary.csv', allrows)
        csvfile(run / 'all_checks.csv', allchecks)
        csvfile(run / 'all_equiv.csv', allequiv)
    completion = {
        'passed': sum(state['status'] == 'PASS' for state in states),
        'failed': failures, 'checks': len(allchecks),
        'equiv_checks': len(allequiv), 'mode': mode,
        'primary_protocol': 'consecutive_e2e',
        'secondary_protocol': 'rotating_e2e',
    }
    save(run / 'completion.json', completion)
    print('METHOD_SUITE_COMPLETE', json.dumps(completion), flush=True)
    return 2 if failures else 0


if __name__ == '__main__':
    sys.exit(main())
