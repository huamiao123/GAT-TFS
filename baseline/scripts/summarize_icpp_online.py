#!/usr/bin/env python3
"""Derive median results from complete immutable ICPP Online raw logs."""
import argparse
import csv
import json
import statistics
from collections import defaultdict
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('run', type=Path)
parser.add_argument('--completion-marker', default='ICPP_ONLINE_COMPLETE')
args = parser.parse_args()
run = args.run.resolve()
records = []
checks = []
profiles = []
stats = []
access = []
diagnostics = {prefix: [] for prefix in (
    'BLOCK_WORK', 'REUSE_WINDOW', 'REUSE_BLOCK', 'HEAD_PH_PROBE_BEGIN',
    'HEAD_PH_TIME', 'HEAD_PH_DEN_CONTROL', 'HEAD_PH_ORACLE',
    'HEAD_PH_INSTRUCTION_GATE', 'HEAD_PH_ERROR', 'HEAD_PH_SUMMARY',
    'HEAD_PH_PROBE_END')}

def kv(line):
    result = {}
    for item in line.split()[1:]:
        if '=' in item:
            name, value = item.split('=', 1)
            result[name] = value
    return result

for graph in ('arxiv', 'products'):
    source = run / f'{graph}.log'
    lines = source.read_text(encoding='utf-8').splitlines()
    if not any(line.startswith(args.completion_marker + ' ') for line in lines):
        raise SystemExit(f'incomplete graph log: {source}')
    samples = defaultdict(list)
    for line in lines:
        if line.startswith('{'):
            value = json.loads(line)
            samples[(value['path'], value['layer'])].append(value)
        elif line.startswith('CHECK '):
            checks.append(dict(graph=graph, **kv(line)))
        elif line.startswith('PROFILE '):
            profiles.append(dict(graph=graph, **kv(line)))
        elif line.startswith('ONLINE_STATS '):
            stats.append(dict(graph=graph, **kv(line)))
        elif line.startswith('PAIR_ACCESS '):
            access.append(dict(graph=graph, **kv(line)))
        else:
            prefix = line.split(' ', 1)[0]
            if prefix in diagnostics:
                diagnostics[prefix].append(dict(graph=graph, **kv(line)))
    paths = sorted({path for path, _ in samples})
    e2e = {path: statistics.median(x['e2e_ms'] for x in samples[(path, 1)]) for path in paths}
    for (path, layer), values in sorted(samples.items()):
        if len(values) != 3 or sorted(x['rep'] for x in values) != [0, 1, 2]:
            raise SystemExit(f'expected three repetitions: {graph} {path} L{layer}')
        row = dict(graph=graph, path=path, layer=layer, repeats=len(values))
        for key in values[0]:
            if key.endswith('_ms'):
                row[key] = statistics.median(x[key] for x in values)
        row['speedup_vs_B0_FP32'] = e2e['B0_FP32'] / e2e[path]
        row['speedup_vs_B0_BF16'] = e2e['B0_BF16'] / e2e[path]
        row['speedup_vs_original_B1'] = e2e['B1_TFS_PREMAX'] / e2e[path]
        records.append(row)

for name, rows in [('summary.tsv', records), ('checks.tsv', checks),
                   ('profiles.tsv', profiles), ('online_stats.tsv', stats)]:
    target = run / name
    if target.exists():
        raise SystemExit(f'refusing to overwrite derived artifact: {target}')
    fields = list(dict.fromkeys(key for row in rows for key in row))
    with target.open('w', encoding='utf-8', newline='') as out:
        writer = csv.DictWriter(out, fields, delimiter='\t')
        writer.writeheader()
        writer.writerows(rows)

if access:
    target = run / 'access.tsv'
    if target.exists():
        raise SystemExit(f'refusing to overwrite derived artifact: {target}')
    fields = list(dict.fromkeys(key for row in access for key in row))
    with target.open('w', encoding='utf-8', newline='') as out:
        writer = csv.DictWriter(out, fields, delimiter='\t')
        writer.writeheader()
        writer.writerows(access)

for prefix, rows in diagnostics.items():
    if not rows:
        continue
    target = run / (prefix.lower() + '.tsv')
    if target.exists():
        raise SystemExit(f'refusing to overwrite derived artifact: {target}')
    fields = list(dict.fromkeys(key for row in rows for key in row))
    with target.open('w', encoding='utf-8', newline='') as out:
        writer = csv.DictWriter(out, fields, delimiter='\t')
        writer.writeheader()
        writer.writerows(rows)

print('graph\tpath\tthree_layer_median_ms\tvs_B0_FP32\tvs_B0_BF16\tvs_original_B1')
for row in records:
    if row['layer'] == 1:
        print('{graph}\t{path}\t{e2e_ms:.3f}\t{speedup_vs_B0_FP32:.5f}\t'
              '{speedup_vs_B0_BF16:.5f}\t{speedup_vs_original_B1:.5f}'.format(**row))
print('all_speedups=baseline_ms/candidate_ms; above_one_is_faster; timing=shared_node_exploratory')
