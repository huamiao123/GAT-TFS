#!/usr/bin/env python3
"""Immutable graph runs and strict parsing for the frozen 2 x 2 study."""
import argparse
import array
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

METHODS = ('b64_bf16', 'b64_fp32', 'full_bf16', 'full_fp32')
ANCHORS = ('original_TFS', 'source_MKL_FP32')
DIAGNOSTIC = ('ogbn-products', 'reddit', 'mycielskian19', 'roadNet-CA', 'wiki-Talk')
KINDS = ('kernel', 'e2e', 'matched_kernel', 'preparation_inclusive')
EXPECTED = {'CHECK': 24, 'EQ': 6, 'DIFF': 6, 'PROFILE_GATE': 8, 'TIME': 180, 'STAGE': 20}
FROZEN_COMMIT = '8aeef1613fdeaeb929379874c4152eccde620f74'


def save(path, value):
    path.write_text(json.dumps(value, indent=2, allow_nan=False) + '\n')


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


def scalar(key, value):
    if key.startswith('checksum') or key in ('pointer', 'config', 'raw_event') or value.startswith('0x'):
        return value
    if re.fullmatch(r'[+-]?\d+', value):
        return int(value)
    if re.fullmatch(r'[+-]?(?:\d+\.\d*|\.\d+|\d+)(?:[eE][+-]?\d+)?', value):
        number = float(value)
        # JSON keeps unsupported/nonfinite diagnostic strings auditable.
        return number if math.isfinite(number) else value
    return value


def parse_records(folder, text):
    records = {}
    errors = []
    for line_number, line in enumerate(text.splitlines(), 1):
        if not line.startswith('PRECISION_'):
            continue
        label, _, body = line.partition(' ')
        try:
            row = {}
            for token in shlex.split(body):
                if '=' in token:
                    key, value = token.split('=', 1)
                    if key in row:
                        raise ValueError('Duplicate field ' + key)
                    row[key] = scalar(key, value)
            records.setdefault(label[len('PRECISION_'):], []).append(row)
        except Exception as exc:
            errors.append({'line_number': line_number, 'line': line, 'reason': str(exc)})
    save(folder / 'records.json', records)
    for label, rows in records.items():
        csvfile(folder / (label.lower() + '.csv'), rows)
    if errors:
        save(folder / 'parse_errors.json', errors)
        raise ValueError('Malformed PRECISION records; see parse_errors.json')
    return records


def graph_stats(path):
    with path.open('rb') as stream:
        header = stream.read(36)
        if len(header) != 36:
            raise ValueError('Short CSR header')
        a, b, c, n, nc, edges = struct.unpack('<IIIQQQ', header)
        if (a, b, c) != (0, 0, 2) or n != nc or n == 0:
            raise ValueError('Not the frozen nonempty square CSR format')
        rows = array.array('I')
        if rows.itemsize != 4:
            raise ValueError('Platform uint32 array width')
        rows.fromfile(stream, n + 1)
    if sys.byteorder != 'little':
        rows.byteswap()
    if rows[0] != 0 or rows[-1] != edges:
        raise ValueError('CSR row endpoint mismatch')
    minimum, maximum, empty, sumsq = edges, 0, 0, 0
    for previous, current in zip(rows, rows[1:]):
        degree = current - previous
        if degree < 0:
            raise ValueError('Nonmonotone CSR row pointers')
        minimum = min(minimum, degree)
        maximum = max(maximum, degree)
        empty += degree == 0
        sumsq += degree * degree
    mean = edges / n
    return {'N': n, 'E': edges, 'format': '0,0,2', 'avg_degree': mean,
            'min_degree': minimum, 'max_degree': maximum, 'empty_rows': empty,
            'empty_ratio': empty / n, 'degree_stddev': math.sqrt(max(0., sumsq / n - mean * mean))}


def smoke_graphs(run):
    degree_pattern = [0, 1, 7, 8, 9, 16, 31, 32, 33, 63, 64, 65,
                      127, 128, 129, 255, 256, 257, 511, 512, 513]
    result = []
    for name, n in (('precision_tail', 37), ('precision_high_tail', 137), ('precision_scope_tail', 521)):
        folder = run / 'fixtures' / name
        folder.mkdir(parents=True)
        rows, cols = [0], []
        for node in range(n):
            degree = degree_pattern[node % len(degree_pattern)]
            cols.extend((node * 7 + neighbor * 11) % n for neighbor in range(degree))
            rows.append(len(cols))
        path = folder / (name + '.csrbin')
        with path.open('wb') as stream:
            stream.write(struct.pack('<IIIQQQ', 0, 0, 2, n, n, len(cols)))
            stream.write(struct.pack('<' + 'I' * len(rows), *rows))
            stream.write(struct.pack('<' + 'I' * len(cols), *cols))
            stream.write(struct.pack('<' + 'f' * len(cols), *([1.] * len(cols))))
        result.append({'graph': name, 'path': str(path), 'N': n, 'E': len(cols),
                       'input_sha256': sha(path), 'fixture_degree_pattern': degree_pattern,
                       'duplicate_edges_permitted': True})
    return result


def require(condition, message):
    if not condition:
        raise ValueError(message)


def validate(records, graph, stats):
    for label, rows in records.items():
        require(all(row.get('graph', graph) == graph for row in rows), 'Cross-graph record: ' + label)
    for label, count in EXPECTED.items():
        require(len(records.get(label, [])) == count, label + ' record count')
    require(len(records.get('CONFIG', [])) == 1, 'Configuration count')
    config = records['CONFIG'][0]
    for key, value in {'N': stats['N'], 'E': stats['E'], 'D': 128, 'F': 128, 'TR': 16, 'R': 64,
                       'methods': 4, 'anchors': 2, 'precision': 'FAST', 'final_output': 'BF16',
                       'native_repeats': 10, 'matched_repeats': 10, 'preparation_repeats': 5,
                       'stage_repeats': 5, 'source_commit': '8aeef16'}.items():
        require(config.get(key) == value, 'Frozen configuration: ' + key)
    for label in ('CHECK', 'EQ', 'DIFF', 'PROFILE_GATE'):
        require(all(row.get('pass') == 1 and row.get('finite') == 1 for row in records[label]), label + ' failed/invalid')
        for row in records[label]:
            require(all(math.isfinite(float(row[key])) for key in ('max_abs', 'mean_abs', 'relative_L2', 'normalized_max')), label + ' nonfinite error metric')
    for method in METHODS:
        own = [row for row in records['CHECK'] if row['method'] == method and row['reference'] == 'own_math_oracle']
        require(len(own) == 4 and {row['boundary'] for row in own} == {
            'layer1_FP32', 'layer2_FP32_actual_input', 'e2e_FP32_final', 'e2e_BF16_final'}, 'Own reference coverage ' + method)
        for row in own:
            require(row['gate'] == (.02 if row['boundary'] == 'e2e_BF16_final' else .01), 'Own gate changed')
        source = [row for row in records['CHECK'] if row['method'] == method and row['reference'] != 'own_math_oracle']
        if method.endswith('_bf16'):
            wanted = {('kernel_FP32', 'original_TFS'), ('e2e_FP32_final', 'original_TFS'),
                      ('e2e_BF16_final', 'original_TFS'), ('e2e_BF16_final', 'source_MKL_FP32')}
            require(len(source) == 4 and {(row['boundary'], row['reference']) for row in source} == wanted, 'Original BF16 gates')
            for row in source:
                gate = .03 if row['reference'] == 'source_MKL_FP32' else .02 if row['boundary'] == 'e2e_BF16_final' else .01
                require(row['gate'] == gate, 'Original gate changed')
        else:
            require(not source, 'FP32 subjected to mismatched dtype source gate')
            eq = [row for row in records['EQ'] if row['method'] == method]
            require(len(eq) == 3 and {row['boundary'] for row in eq} == {
                'matched_layer1_FP32', 'matched_layer2_FP32', 'matched_layer2_BF16'}, 'Matched representation coverage')
            diff = [row for row in records['DIFF'] if row['method'] == method]
            require(len(diff) == 3 and all(row['gate'] == 0 and row['gate_type'] == 'report_only' for row in diff), 'Cross dtype report-only coverage')
    require(all(row['bitwise'] == 1 for row in records['EQ'] + records['PROFILE_GATE']), 'Bytewise check failed')
    profile_keys = [(row['method'], row['boundary']) for row in records['PROFILE_GATE']]
    require(len(set(profile_keys)) == 8 and set(profile_keys) == {
        (method, boundary) for method in METHODS for boundary in ('layer1_FP32', 'layer2_BF16')}, 'Profiling coverage')
    require(len(records.get('COUNTERS', [])) == 8, 'Counter coverage')
    require({(row['method'], row['layer']) for row in records['COUNTERS']} == {
        (method, layer) for method in METHODS for layer in (1, 2)}, 'Counter layer coverage')
    for row in records['COUNTERS']:
        fp32 = row['method'].endswith('_fp32')
        require(row['source_visits'] == stats['E'], 'Real neighbor count')
        require(row['feature_adds'] == stats['E'] * 128, 'FP32 feature addition count')
        require(row['source_gather_requested_bytes'] == stats['E'] * (512 if fp32 else 256), 'Requested source byte count')
        require(row['source_decode_elements'] == (0 if fp32 else stats['E'] * 128), 'Decode element count')
        require(row['amx_executed_flops'] == row['amx_dpbf16ps'] * 16384, 'Executed AMX work')
    stages = records['STAGE']
    for method in METHODS:
        selected = [row for row in stages if row['method'] == method]
        require(len(selected) == 5 and {row['repeat'] for row in selected} == set(range(5)), 'Stage repetitions')
        if method.endswith('_fp32'):
            require(all(row['interlayer_conversion_ms'] == 0 and row['conversion_bypassed'] == 1 for row in selected), 'FP32 conversion was not bypassed')
    require(len(records.get('GATE_COMPLETE', [])) == 1 and records['GATE_COMPLETE'][0]['pass'] == 1, 'Gate completion')
    require(len(records.get('COMPLETE', [])) == 1 and records['COMPLETE'][0]['pass'] == 1, 'Driver completion')
    complete = records['COMPLETE'][0]
    for key, value in {'checks': 24, 'bitwise_checks': 6, 'report_only_differences': 6,
                       'profile_bitwise_checks': 8, 'measured_repetitions': 180, 'stage_repetitions': 20}.items():
        require(complete.get(key) == value, 'Completion count: ' + key)


def percentile(values, fraction):
    ordered = sorted(values)
    position = (len(ordered) - 1) * fraction
    lo, hi = math.floor(position), math.ceil(position)
    return ordered[lo] + (ordered[hi] - ordered[lo]) * (position - lo)


def timing_summary(records, graph):
    groups = {}
    for row in records['TIME']:
        require(row['kind'] in KINDS, 'Unknown timing kind')
        require(math.isfinite(float(row['ms'])) and row['ms'] > 0, 'Invalid timing')
        require(row.get('warmups') == 1, 'Immediate warmup changed')
        groups.setdefault((row['method'], row['kind']), []).append(row)
    wanted = {(method, kind) for method in METHODS for kind in KINDS}
    wanted |= {(method, kind) for method in ANCHORS for kind in ('kernel', 'e2e')}
    require(set(groups) == wanted, 'Timing group coverage')
    result = []
    for (method, kind), rows in groups.items():
        count = 5 if kind == 'preparation_inclusive' else 10
        require(len(rows) == count and {row['repeat'] for row in rows} == set(range(count)), 'Timing repetition IDs')
        forward = [float(row['ms']) for row in rows if row['orientation'] == 'forward']
        reverse = [float(row['ms']) for row in rows if row['orientation'] == 'reverse']
        require(len(forward) == (3 if count == 5 else 5) and len(reverse) == (2 if count == 5 else 5), 'Directional coverage')
        values = [float(row['ms']) for row in rows]
        mean = statistics.mean(values)
        result.append({'graph': graph, 'method': method, 'kind': kind, 'repeats': count,
                       'min_ms': min(values), 'p25_ms': percentile(values, .25), 'median_ms': statistics.median(values),
                       'p75_ms': percentile(values, .75), 'max_ms': max(values), 'mean_ms': mean,
                       'cv': statistics.pstdev(values) / mean, 'cv_percent': statistics.pstdev(values) / mean * 100,
                       'forward_repeats': len(forward), 'reverse_repeats': len(reverse),
                       'forward_median_ms': statistics.median(forward), 'reverse_median_ms': statistics.median(reverse)})
    lookup = {(row['method'], row['kind']): row for row in result}
    for row in result:
        if row['kind'] in ('kernel', 'e2e'):
            row['speedup_median_vs_original_TFS'] = lookup['original_TFS', row['kind']]['median_ms'] / row['median_ms']
            row['speedup_median_vs_source_MKL_FP32'] = lookup['source_MKL_FP32', row['kind']]['median_ms'] / row['median_ms']
        if row['method'] in METHODS:
            bf16 = row['method'].split('_')[0] + '_bf16'
            row['speedup_median_vs_same_B_bf16'] = lookup[bf16, row['kind']]['median_ms'] / row['median_ms']
    return result


def original_anchor(text):
    groups, current = {}, None
    for line in text.splitlines():
        if line.startswith('=== TFS V3'):
            current = 'original_TFS'
        elif line.startswith('=== MKL FP32'):
            current = 'source_MKL_FP32'
        elif line.startswith('==='):
            current = None
        match = re.match(r'\s+run (\d+): ([0-9.]+) ms', line)
        if current and match:
            groups.setdefault(current, []).append({'repeat': int(match[1]), 'ms': float(match[2])})
    require(set(groups) == set(ANCHORS) and all(len(rows) == 5 for rows in groups.values()), 'Unchanged source timings missing')
    return {'printed_precision_decimal_places': 2, 'source_timings': groups,
            'policy': 'Separate original source sequence retained; supplemental ratios use same-process same-kind TIME records.'}


def prior_diagnostic(study, run, driver_hash, source_hash, explicit=None):
    candidates = [pathlib.Path(explicit).resolve()] if explicit else sorted((study / 'logs').glob('diagnostic-*'), key=lambda path: path.stat().st_mtime, reverse=True)
    audits, accepted = [], None
    for candidate in candidates:
        audit = {'run': str(candidate)}
        try:
            require(candidate.parent.resolve() == (study / 'logs').resolve(), 'Different study diagnostic')
            completion = json.loads((candidate / 'completion.json').read_text())
            require((candidate / 'exit_status.txt').read_text().strip() == '0', 'Diagnostic job not fully completed')
            require(completion['mode'] == 'diagnostic' and completion['status'] == 'PASS' and completion['passed'] == 5 and completion['failed'] == 0, 'Diagnostic did not pass all five')
            require(set(completion['graph_names']) == set(DIAGNOSTIC), 'Different diagnostic graph set')
            require(completion['driver_sha256'] == driver_hash and completion['source_manifest_sha256'] == source_hash, 'Diagnostic used different source/binary')
            require(pathlib.Path(completion['study']).resolve() == study.resolve(), 'Different diagnostic study identity')
            audit['status'] = 'PASS'
            if accepted is None:
                accepted = str(candidate)
        except Exception as exc:
            audit.update(status='INELIGIBLE', reason=str(exc))
        audits.append(audit)
    save(run / 'diagnostic_prerequisite.json', {'accepted_run': accepted, 'attempts': audits})
    require(accepted is not None, 'Extension requires completed same-study diagnostic PASS with identical source/binary')
    return accepted


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('run', type=pathlib.Path)
    parser.add_argument('study', type=pathlib.Path)
    parser.add_argument('mode', choices=('smoke', 'diagnostic', 'extension'))
    parser.add_argument('--diagnostic-run')
    args = parser.parse_args()
    run, study, mode = args.run.resolve(), args.study.resolve(), args.mode
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))
    driver = study / 'build/feature_precision_driver'
    states, allrecords, summaries = [], {}, []
    node, job = os.uname().nodename, os.environ.get('SLURM_JOB_ID')
    driver_hash, source_hash, graphs = None, None, []
    def flush():
        save(run / 'records.json', allrecords)
        for label, rows in allrecords.items():
            csvfile(run / (label.lower() + '.csv'), rows)
        csvfile(run / 'summary.csv', summaries)
        save(run / 'summary.json', {'mode': mode, 'study': str(study), 'node': node, 'job_id': job,
                                   'graph_states': states, 'timing_summary': summaries,
                                   'gates': {label: {'records': len(allrecords.get(label, [])),
                                                     'all_pass': all(row.get('pass') == 1 for row in allrecords.get(label, []))}
                                             for label in ('CHECK', 'EQ', 'DIFF', 'PROFILE_GATE')}})
        save(run / 'suite_status.json', states)
    try:
        driver_hash, source_hash = sha(driver), sha(study / 'source.sha256')
        inventory = json.loads((study / 'source_snapshot/CURRENT_GRAPH_INVENTORY_20261004.json').read_text())
        mapping = {row['graph']: row for row in inventory}
        if mode == 'smoke':
            graphs = smoke_graphs(run)
        elif mode == 'diagnostic':
            graphs = [mapping[name] for name in DIAGNOSTIC]
        else:
            prior_diagnostic(study, run, driver_hash, source_hash, args.diagnostic_run)
            graphs = [row for row in inventory if row['graph'] not in DIAGNOSTIC]
            require(len(graphs) == 12, 'Expected remaining twelve eligible graphs')
        save(run / 'graph_list.json', graphs)
        for graph in graphs:
            name = graph['graph']
            folder = run / name
            folder.mkdir()
            state = {'graph': name, 'node': node, 'job_id': job}
            records = {}
            start = time.monotonic()
            print('PRECISION_SUITE_START', name, flush=True)
            try:
                path = pathlib.Path(graph['path'])
                input_hash = sha(path)
                state['input_sha256'] = input_hash
                require(input_hash == graph['input_sha256'], 'Frozen input hash changed')
                stats = graph_stats(path)
                save(folder / 'graph_stats.json', stats)
                require(stats['N'] == graph['N'] and stats['E'] == graph['E'], 'Inventory/header mismatch')
                require(stats['E'] <= 2147483647 and stats['N'] * 128 <= 2147483648, 'Frozen LP64 bounds')
                manifest = {'graph': name, 'input_path': str(path), 'input_sha256': input_hash,
                            'input_size_bytes': path.stat().st_size, 'graph_stats': stats,
                            'driver_sha256': driver_hash, 'source_manifest_sha256': source_hash,
                            'frozen_commit': FROZEN_COMMIT, 'study': str(study), 'mode': mode,
                            'node': node, 'job_id': job, 'expected_gate_counts': EXPECTED,
                            'repetition_policy': 'Immediate warmup; 10 FRRFFRRFFR paired native/matched; 5 preparation; 5 stages',
                            'semantic_scope': 'source-random static two-layer 128->128->128 computation',
                            'NUMA_policy': 'source unchanged; buffer first-touch maps recorded by runtime'}
                save(folder / 'manifest.json', manifest)
                argv = [str(driver), str(path.parent.parent), name]
                save(folder / 'command.json', {'argv': argv, 'cwd': str(folder), 'env': {key: os.environ.get(key) for key in (
                    'OMP_NUM_THREADS', 'OMP_PROC_BIND', 'OMP_PLACES', 'MKL_NUM_THREADS', 'MKL_DYNAMIC', 'MKL_DOMAIN_NUM_THREADS',
                    'SLURM_JOB_ID', 'SLURM_CPUS_PER_TASK')}, 'graphfile': str(path), 'numa_policy': 'default'})
                launched = time.monotonic()
                with (folder / 'driver.stdout.log').open('wb') as out, (folder / 'driver.stderr.log').open('wb') as err:
                    try:
                        completed = subprocess.run(argv, cwd=folder, stdout=out, stderr=err, timeout=4200)
                        code, timed_out = completed.returncode, False
                    except subprocess.TimeoutExpired:
                        code, timed_out = None, True
                save(folder / 'driver.exit.json', {'returncode': code, 'timed_out': timed_out,
                                                 'wall_seconds': time.monotonic() - launched})
                state['driver_exit_code'] = code
                text = (folder / 'driver.stdout.log').read_text(errors='replace')
                records = parse_records(folder, text)
                require(not timed_out and code == 0, 'Driver timeout/exit ' + str(code))
                save(folder / 'original_source_anchor.json', original_anchor(text))
                validate(records, name, stats)
                timing = timing_summary(records, name)
                save(folder / 'timing_summary.json', timing)
                csvfile(folder / 'summary.csv', timing)
                summaries.extend(timing)
                state.update(status='PASS', graph_stats=stats, checks=24, bitwise_checks=6, profile_bitwise_checks=8)
                print('PRECISION_SUITE_PASS', name, json.dumps([row for row in timing if row['kind'] == 'e2e']), flush=True)
            except Exception as exc:
                state.update(status='FAILED', reason=str(exc), exception_type=type(exc).__name__)
                # Retain records written before a parse/validation failure.
                if not records and (folder / 'records.json').exists():
                    records = json.loads((folder / 'records.json').read_text())
                print('PRECISION_SUITE_FAILED', name, str(exc), flush=True)
            state['wall_seconds'] = time.monotonic() - start
            for label, rows in records.items():
                allrecords.setdefault(label, []).extend(rows)
            states.append(state)
            save(folder / 'status.json', state)
            flush()
        failed = sum(state['status'] != 'PASS' for state in states)
        completion = {'status': 'FAILED' if failed else 'PASS', 'mode': mode, 'study': str(study),
                      'node': node, 'job_id': job, 'passed': len(states) - failed, 'failed': failed,
                      'graphs': len(states), 'graph_names': [state['graph'] for state in states],
                      'driver_sha256': driver_hash, 'source_manifest_sha256': source_hash,
                      'counts': {label: len(allrecords.get(label, [])) for label in EXPECTED}}
    except Exception as exc:
        completion = {'status': 'FAILED', 'mode': mode, 'study': str(study), 'node': node, 'job_id': job,
                      'passed': 0, 'failed': 1, 'graphs': len(states), 'graph_names': [state['graph'] for state in states],
                      'driver_sha256': driver_hash, 'source_manifest_sha256': source_hash,
                      'setup_failure': True, 'reason': str(exc)}
        print('PRECISION_SUITE_SETUP_FAILED', str(exc), flush=True)
        flush()
    save(run / 'completion.json', completion)
    print('PRECISION_SUITE_COMPLETE', json.dumps(completion), flush=True)
    return 0 if completion['status'] == 'PASS' else 2


if __name__ == '__main__':
    sys.exit(main())
