#!/usr/bin/env python3
"""Final fixed-inventory audit. No jobs are submitted and input evidence is read-only.

python3 reconcile_balance.py --out ROOT/results/NEW
Optional --sacct-file FILE accepts pipe-separated sacct output with the fields
JobIDRaw,State,ExitCode,NodeList,Partition,AllocCPUS,NNodes,Elapsed,Start,End.
The script exits 2 when any required job, graph, gate or digest is incomplete.
"""
import argparse
import collections
import csv
import datetime
import hashlib
import io
import json
import math
import pathlib
import re
import statistics
import subprocess
import sys

ROOT = pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra').resolve()
SHAPES = ('128_32_32', '128_128_128')
SCHEDULES = ('degree', 'qsource', 'qpage', 'qsamplepage', 'qguard')
GROUP_METHODS = {'paper_tfs', 'b64_fast', 'b256_fast', 'bfull_fast', 'source_mkl_fp32'} | {
    's64_m%s_%s_fast' % (m, s) for m in ('64', '256', 'full') for s in SCHEDULES}
ORDER_CANDIDATES = {'%s_%s_%s' % (a, b, g) for a in ('A', 'Pall', 'Pused')
                    for b in ('A', 'Pall', 'Pused') for g in ('degree', 'qsource')}
ORDER_MKL = 'shape_mkl_fp32_original_style'
FIXTURES = ['window_tail', 'window_high_tail', 'window_scope_tail']
STUDIES = {'grouping': 'grouping-balance-20261007-01', 'order': 'order-balance-20261007-02'}
INVENTORY = [
    ('grouping', '10887774', 'smoke', FIXTURES),
    ('grouping', '10887784', 'shared_primary', ['mycielskian19']),
    ('grouping', '10887785', 'shared_primary', ['reddit', 'roadNet-CA', 'amazon0601']),
    ('grouping', '10887786', 'shared_primary', ['ogbn-products', 'hollywood-2009', 'indochina-2004', 'soc-Pokec']),
    ('grouping', '10887787', 'shared_primary', ['cit-Patents', 'soc-LiveJournal1', 'com-LiveJournal', 'as-Skitter']),
    ('grouping', '10887788', 'shared_primary', ['rgg_n_2_24_s0', 'web-Google', 'wiki-Talk', 'email-Enron', 'com-Youtube']),
    ('grouping', '10887789', 'synthetic_shared', ['regular128_reuse1', 'regular128_reuse4', 'regular128_reuse16']),
    ('grouping', '10887942', 'exclusive_intel_validation', ['mycielskian19', 'reddit', 'roadNet-CA', 'amazon0601']),
    ('order', '10887799', 'smoke', FIXTURES),
    ('order', '10887809', 'shared_primary', ['mycielskian19']),
    ('order', '10887810', 'shared_primary', ['reddit', 'ogbn-products']),
    ('order', '10887811', 'shared_primary', ['roadNet-CA', 'wiki-Talk', 'hollywood-2009']),
    ('order', '10887943', 'exclusive_intel_validation', ['reddit', 'ogbn-products', 'roadNet-CA', 'wiki-Talk']),
]
SKIPPED_JOB = '10887790'
SACCT_FIELDS = ['JobIDRaw', 'State', 'ExitCode', 'NodeList', 'Partition', 'AllocCPUS',
                'NNodes', 'Elapsed', 'Start', 'End']
INPUT_DIGESTS = {}


def require(condition, message):
    if not condition:
        raise ValueError(message)


def read_bytes(path):
    data = path.read_bytes()
    INPUT_DIGESTS[str(path)] = hashlib.sha256(data).hexdigest()
    return data


def read_json(path):
    return json.loads(read_bytes(path).decode('utf-8-sig'))


def read_csv(path):
    return list(csv.DictReader(io.StringIO(read_bytes(path).decode('utf-8-sig'))))


def digest_file(path):
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def digest_manifest(path, allowed, verify_files):
    parsed = {}
    for line in read_bytes(path).decode().splitlines():
        m = re.fullmatch(r'([0-9a-fA-F]{64})\s+[* ]?(.*)', line)
        require(m is not None, 'Malformed digest entry in ' + str(path))
        target = pathlib.Path(m.group(2)).resolve()
        require(any(target == root or root in target.parents for root in allowed),
                'Digest target outside source_snapshot/build: ' + str(target))
        require(str(target) not in parsed, 'Duplicate digest target ' + str(target))
        require(target.is_file(), 'Missing hashed file ' + str(target))
        if verify_files:
            require(digest_file(target) == m.group(1).lower(), 'Hash mismatch ' + str(target))
        parsed[str(target)] = m.group(1).lower()
    require(bool(parsed), 'Empty digest manifest ' + str(path))
    return parsed


def source_audit(study):
    result = {'study_path': str(study), 'status': 'FAIL'}
    try:
        source = digest_manifest(study / 'source.sha256', [study / 'source_snapshot'], True)
        binary = digest_manifest(study / 'build/binary.sha256', [study / 'build'], True)
        manifest = read_json(study / 'manifest.json')
        require(manifest.get('source_hashes', '').strip() == (study / 'source.sha256').read_text().strip(),
                'Source manifest embedded record differs')
        require(manifest.get('binary_hashes', '').strip() == (study / 'build/binary.sha256').read_text().strip(),
                'Binary manifest embedded record differs')
        command = study / 'build/commands.txt'
        require(command.is_file(), 'Build commands missing')
        read_bytes(command)
        result.update(status='PASS', source_hashes=source, binary_hashes=binary,
                      source_files=len(source), binary_files=len(binary),
                      compiler=manifest.get('compiler', ''), manifest_sha256=INPUT_DIGESTS[str(study / 'manifest.json')],
                      commands_sha256=INPUT_DIGESTS[str(command)])
    except (ValueError, OSError, KeyError, TypeError) as exc:
        result['error'] = str(exc)
    return result


def scheduler_snapshot(sacct_file):
    if sacct_file:
        raw = read_bytes(pathlib.Path(sacct_file).resolve()).decode()
        origin = {'kind': 'provided_sacct_snapshot', 'path': str(pathlib.Path(sacct_file).resolve())}
    else:
        ids = ','.join([r[1] for r in INVENTORY] + [SKIPPED_JOB])
        cmd = ['sacct', '-X', '-n', '-P', '-j', ids, '--format=' + ','.join(SACCT_FIELDS)]
        proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True, timeout=45)
        raw = proc.stdout
        origin = {'kind': 'live_sacct', 'command': cmd, 'exit_status': proc.returncode, 'stderr': proc.stderr}
        require(proc.returncode == 0, 'sacct failed: ' + proc.stderr)
    lines = [line for line in raw.splitlines() if line.strip()]
    has_header = bool(lines and lines[0].split('|')[0] == 'JobIDRaw')
    rows = list(csv.DictReader(lines, delimiter='|', fieldnames=None if has_header else SACCT_FIELDS))
    exact = {}
    for row in rows:
        job = str(row.get('JobIDRaw', '')).strip()
        if job in [r[1] for r in INVENTORY] + [SKIPPED_JOB]:
            require(job not in exact, 'Duplicate sacct allocation record ' + job)
            exact[job] = {k: str(row.get(k, '') or '').strip() for k in SACCT_FIELDS}
    return raw, origin, exact


def csv_matches(folder, parsed, key):
    rows = parsed.get(key, [])
    csv_rows = read_csv(folder / (key.lower() + '.csv'))
    keys = sorted(set().union(*(r.keys() for r in rows + csv_rows))) if rows or csv_rows else []
    normalize = lambda items: sorted(tuple(str(r.get(k, '')) for k in keys) for r in items)
    require(normalize(rows) == normalize(csv_rows), key + ' CSV/parsed mismatch')
    return rows


def gate(parsed, key, count, bitwise=False):
    rows = parsed.get(key, [])
    require(len(rows) == count, '%s expected %d rows, got %d' % (key, count, len(rows)))
    require(all(str(r.get('pass')) == '1' for r in rows), key + ' failed')
    if bitwise:
        require(all(str(r.get('bitwise')) == '1' for r in rows), key + ' not bitwise')


def numerical_rows(parsed, key, group):
    for r in parsed[key]:
        values = [float(r[k]) for k in ('max_abs', 'relative_L2', 'normalized_max')]
        expected = (.03 if r['reference'] == 'source_MKL_FP32' else
                    (.02 if r['boundary'] == 'e2e_BF16_final' else .01)) if group else (
                    .01 if r['boundary'] == 'e2e_BF16_final' else .001)
        require(all(math.isfinite(x) for x in values), key + ' nonfinite')
        require(float(r['gate']) == expected and values[1] < expected and values[2] < expected,
                key + ' changed/failed numerical threshold')
        if not group:
            require(r.get('finite') == '1' and r['reference'] == 'matched_quantized_MKL_math',
                    'Order reference/finite semantics changed')


def timing_summary(rows, shapes, methods, kinds, info, group):
    cells = collections.defaultdict(list)
    for r in rows:
        shape = '128_128_128' if group else r['shape']
        require(shape in shapes and r['method'] in methods and r['kind'] in kinds, 'Unexpected timing key')
        require(r['graph'] == info['graph'], 'Timing graph mismatch')
        ms = float(r['ms'])
        require(math.isfinite(ms) and ms > 0, 'Invalid timing')
        if not group:
            require(r.get('anchor_precision_mismatch') == ('1' if r['method'] == ORDER_MKL else '0'),
                    'Order source FP32 MKL label mismatch')
        cells[shape, r['method'], r['kind']].append(r)
    require(set(cells) == {(s, m, k) for s in shapes for m in methods for k in kinds}, 'Incomplete timing matrix')
    result = []
    for (shape, method, kind), rs in sorted(cells.items()):
        count = 5 if kind == kinds[0] else 10
        require(len(rs) == count and {int(r['repeat']) for r in rs} == set(range(count)),
                'Wrong repeats: ' + method + '/' + kind)
        if group and count == 10:
            require(all(r.get('orientation') == ('reverse' if int(r['repeat']) % 2 else 'forward') for r in rs),
                    'Alternating directions incomplete')
        values = [float(r['ms']) for r in sorted(rs, key=lambda r: int(r['repeat']))]
        mk = method == ('source_mkl_fp32' if group else ORDER_MKL)
        result.append(dict(info, shape=shape, method=method, kind=kind, primary=int(kind == kinds[0]),
            precision_label='SOURCE_FP32_MKL' if mk else ('BF16_FAST_AMX' if group else 'BF16_INPUT_ACCURATE_OR_FP32_PROJECTED_CACHE'),
            source_fp32_mkl=int(mk), samples=count, median_ms=statistics.median(values),
            min_ms=min(values), max_ms=max(values), cv=statistics.pstdev(values) / statistics.mean(values),
            samples_ms=json.dumps(values)))
    lookup = {(r['shape'], r['method'], r['kind']): r for r in result}
    for r in result:
        refs = {'source_fp32_mkl': 'source_mkl_fp32' if group else ORDER_MKL}
        if group:
            refs.update(tfs='paper_tfs', b256='b256_fast')
            m = re.fullmatch(r's64_m(64|256|full)_(.+)_fast', r['method'])
            if m:
                refs['degree_same_scope'] = 's64_m%s_degree_fast' % m.group(1)
        else:
            refs['A_A_degree'] = 'A_A_degree'
            if r['method'] != ORDER_MKL:
                refs['degree_same_dataflow'] = r['method'].rsplit('_', 1)[0] + '_degree'
        for label, method in refs.items():
            r['speedup_vs_' + label] = lookup[r['shape'], method, r['kind']]['median_ms'] / r['median_ms']
    return result


def graph_audit(folder, kind, info):
    state = read_json(folder / 'status.json')
    parsed = read_json(folder / 'parsed.json')
    require(state.get('status') == 'PASS', 'Graph status not PASS')
    digest = state.get('actual_input_sha256', '')
    require(re.fullmatch('[0-9a-f]{64}', digest) is not None, 'Missing recorded input SHA256')
    audit = dict(info, evidence=str(folder), input_sha256=digest, status='PASS')
    if kind == 'grouping':
        counts = {'METHOD_CHECK': 76, 'METHOD_EQUIV': 45, 'METHOD_PROFILE_GATE': 30,
                  'GROUP_WORK_GATE': 30, 'GROUP_AUDIT': 5}
        for key, count in counts.items():
            gate(parsed, key, count, key in ('METHOD_EQUIV', 'METHOD_PROFILE_GATE'))
            csv_matches(folder, parsed, key)
        candidates = GROUP_METHODS - {'source_mkl_fp32'}
        projection = {m for m in candidates if m.startswith('s64_m')}
        boundaries = ('kernel_FP32', 'e2e_FP32_final', 'e2e_BF16_final')
        check_keys = {(r['method'], r['boundary'], r['reference']) for r in parsed['METHOD_CHECK']}
        expected_keys = {(m, b, ref) for m in candidates for b, ref in [
            ('kernel_FP32', 'paper_tfs'), ('e2e_FP32_final', 'paper_tfs'),
            ('e2e_BF16_final', 'paper_tfs'), ('e2e_BF16_final', 'source_MKL_FP32')]}
        require(check_keys == expected_keys, 'Grouping numerical gate matrix incomplete')
        require({(r['method'], r['boundary']) for r in parsed['METHOD_EQUIV']} ==
                {(m, b) for m in projection for b in boundaries}, 'Grouping equivalence matrix incomplete')
        for label in ('METHOD_PROFILE_GATE', 'GROUP_WORK_GATE'):
            require({(r['method'], r['layer']) for r in parsed[label]} ==
                    {(m, layer) for m in projection for layer in ('1', '2')}, label + ' matrix incomplete')
        require({r['schedule'] for r in parsed['GROUP_AUDIT']} == set(SCHEDULES), 'Grouping audit schedules incomplete')
        numerical_rows(parsed, 'METHOD_CHECK', True)
        complete = parsed.get('METHOD_COMPLETE', [])
        require(len(complete) == 1 and complete[0].get('pass') == '1' and int(complete[0]['methods']) == 19,
                'Grouping completion invalid')
        config = parsed.get('METHOD_CONFIG', [])
        require(len(config) == 1 and all(config[0].get(k) == v for k, v in
                {'D': '128', 'F': '128', 'S': '64', 'TR': '16', 'R': '64', 'numa': 'default'}.items()),
                'Grouping configuration changed')
        require(len(parsed.get('METHOD_PLAN', [])) == 20 and
                {r['method'] for r in parsed['METHOD_PLAN']} == GROUP_METHODS, 'Grouping method plan incomplete')
        counters = parsed.get('METHOD_COUNTERS', [])
        require(len(counters) == 30, 'Grouping counters incomplete')
        by_key = {(r['method'], r['layer']): r for r in counters}
        require(len(by_key) == 30, 'Duplicate grouping counter key')
        for r in counters:
            require(int(r['source_visits']) == int(config[0]['E']), 'Grouping edge count mismatch')
            scope = re.fullmatch(r's64_m(64|256|full)_.+_fast', r['method']).group(1)
            ref = by_key['s64_m%s_degree_fast' % scope, r['layer']]
            require({k: v for k, v in r.items() if k != 'method'} ==
                    {k: v for k, v in ref.items() if k != 'method'}, 'Grouping same-work counters differ')
        times = csv_matches(folder, parsed, 'METHOD_TIME')
        require(len(times) == 300, 'Grouping requires 20 x 15 timings')
        rows = timing_summary(times, ('128_128_128',), GROUP_METHODS,
                              ('consecutive_e2e', 'alternating_e2e'), info, True)
    else:
        counts = {'ORDER_CHECK': 108, 'ORDER_EQUIV': 55, 'ORDER_PROFILE_GATE': 72,
                  'ORDER_WORK': 72, 'ORDER_GROUP': 2}
        for key, count in counts.items():
            gate(parsed, key, count, key in ('ORDER_EQUIV', 'ORDER_PROFILE_GATE'))
            csv_matches(folder, parsed, key)
        boundaries = ('kernel_FP32', 'e2e_FP32_final', 'e2e_BF16_final')
        require({(r['shape'], r['method'], r['boundary']) for r in parsed['ORDER_CHECK']} ==
                {(shape, m, b) for shape in SHAPES for m in ORDER_CANDIDATES for b in boundaries},
                'Order numerical gate matrix incomplete')
        expected_eq = {(shape, m, b, 'degree_same_dataflow') for shape in SHAPES
                       for m in ORDER_CANDIDATES if m.endswith('_qsource') for b in boundaries}
        expected_eq.add(('128_128_128', 'A_A_degree', 'kernel_FP32', 'frozen_P0_FULL_ACCURATE'))
        require({(r['shape'], r['method'], r['boundary'], r['reference']) for r in parsed['ORDER_EQUIV']} == expected_eq,
                'Order equivalence matrix incomplete')
        for label in ('ORDER_PROFILE_GATE', 'ORDER_WORK'):
            require({(r['shape'], r['method'], r['layer']) for r in parsed[label]} ==
                    {(shape, m, layer) for shape in SHAPES for m in ORDER_CANDIDATES for layer in ('1', '2')},
                    label + ' matrix incomplete')
        require({r['grouping'] for r in parsed['ORDER_GROUP']} == {'degree', 'qsource'}, 'Order grouping audit incomplete')
        numerical_rows(parsed, 'ORDER_CHECK', False)
        require(len(parsed.get('ORDER_ACCEPT', [])) == 36, 'Order acceptance count')
        for shape in SHAPES:
            accepts = [r for r in parsed['ORDER_ACCEPT'] if r['shape'] == shape]
            require(len(accepts) == 18 and {r['method'] for r in accepts} == ORDER_CANDIDATES and
                    all(r['accepted'] == '1' for r in accepts), 'Order candidate rejected/incomplete')
            complete = [r for r in parsed.get('ORDER_COMPLETE', []) if r['shape'] == shape]
            require(len(complete) == 1 and complete[0].get('execution_pass') == '1' and
                    complete[0].get('all_candidates_accepted') == '1' and int(complete[0]['rejected']) == 0,
                    'Order completion invalid')
            config = [r for r in parsed.get('ORDER_CONFIG', []) if r['shape'] == shape]
            require(len(config) == 1 and config[0].get('aggregate') == 'FULL_ACCURATE' and
                    config[0].get('numa') == 'default_no_interleave', 'Order config changed')
            for r in parsed['ORDER_WORK']:
                if r['shape'] == shape:
                    require(int(r['tdp']) == int(r['expected_tdp']) and int(r['edges']) == int(config[0]['E']),
                            'Order work count mismatch')
        # 27 grouping equalities per shape, plus the F128 frozen-P0 anchor.
        require(sum(r.get('reference') == 'frozen_P0_FULL_ACCURATE' for r in parsed['ORDER_EQUIV']) == 1,
                'Missing/duplicate F128 frozen anchor')
        times = csv_matches(folder, parsed, 'ORDER_TIME')
        require(len(times) == 570, 'Order requires 2 shapes x 19 x 15 timings')
        rows = timing_summary(times, SHAPES, ORDER_CANDIDATES | {ORDER_MKL},
                              ('consecutive', 'paired'), info, False)
    audit.update(gate_counts=counts, timings=len(times), summary_cells=len(rows))
    return audit, rows, state


def cohort_gmeans(rows):
    cells = collections.defaultdict(list)
    for row in rows:
        for metric, value in row.items():
            if metric.startswith('speedup_vs_'):
                key = tuple(row[k] for k in ('study', 'cohort', 'shape', 'kind', 'method')) + (metric,)
                cells[key].append(row)
    result = []
    for key, records in sorted(cells.items()):
        kind, cohort, shape, timing, method, metric = key
        targets = {g for k, j, c, gs in INVENTORY if k == kind and c == cohort for g in gs}
        graphs = [r['graph'] for r in records]
        duplicate = len(set(graphs)) != len(graphs)
        values = [float(r[metric]) for r in records]
        result.append(dict(study=kind, cohort=cohort, shape=shape, kind=timing, method=method, metric=metric,
            graphs=len(set(graphs)), expected_graphs=len(targets), graph_names=sorted(set(graphs)),
            complete=not duplicate and set(graphs) == targets,
            status='DUPLICATE_GRAPH' if duplicate else ('COMPLETE' if set(graphs) == targets else 'INCOMPLETE'),
            gmean=None if duplicate else math.exp(sum(math.log(v) for v in values) / len(values)),
            wins=None if duplicate else sum(v > 1 for v in values),
            jobs=sorted({r['job'] for r in records}), denominator='same graph/job/process only',
            aggregation='fixed method; one vote per graph; no oracle or cross-job time division'))
    return result


def write_csv(path, rows):
    keys = []
    for row in rows:
        for key in row:
            if key not in keys:
                keys.append(key)
    with path.open('x', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=keys or ['no_records'])
        writer.writeheader()
        for row in rows:
            writer.writerow({k: json.dumps(v) if isinstance(v, (dict, list)) else v for k, v in row.items()})


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--out', required=True)
    ap.add_argument('--sacct-file')
    args = ap.parse_args()
    out = pathlib.Path(args.out).resolve()
    require(not out.exists() and ROOT / 'results' in out.parents, '--out must be a new directory under ROOT/results')
    sources = {kind: source_audit(ROOT / 'runs' / name) for kind, name in STUDIES.items()}
    scheduler_error, raw_sacct, scheduler_origin, sacct = '', '', {}, {}
    try:
        raw_sacct, scheduler_origin, sacct = scheduler_snapshot(args.sacct_file)
    except (ValueError, OSError, subprocess.TimeoutExpired) as exc:
        scheduler_error = str(exc)
    jobs, graph_records, summaries = [], [], []
    seen_input_hashes = {}
    for kind, job, cohort, expected_graphs in INVENTORY:
        study = ROOT / 'runs' / STUDIES[kind]
        jobdir = study / 'logs' / (kind + '-' + job)
        account = sacct.get(job, {})
        jr = dict(study=kind, job=job, cohort=cohort, expected_graphs=expected_graphs,
                  evidence=str(jobdir), status='FAIL', sacct=account)
        try:
            require(sources[kind]['status'] == 'PASS', 'Study source/build digest audit failed')
            require(account.get('State') == 'COMPLETED' and account.get('ExitCode') == '0:0',
                    'Requires sacct COMPLETED and ExitCode 0:0')
            require(account.get('Partition') == 'intel' and int(account.get('NNodes', 0)) == 1, 'Allocation mismatch')
            event = read_json(jobdir / 'event.json')
            completion = read_json(jobdir / 'completion.json')
            states = read_json(jobdir / 'suite_status.json')
            graph_list = read_json(jobdir / 'graph_list.json')
            require(event.get('status') == 'PASS' and int(event.get('exit_status', -1)) == 0, 'Job event not PASS')
            require(str(event.get('job', event.get('job_id', ''))) == job and event.get('study') == kind,
                    'Event job/study identity mismatch')
            require(completion.get('study') == kind and int(completion.get('checks', -1)) ==
                    len(expected_graphs) * (76 if kind == 'grouping' else 108), 'Completion numerical count mismatch')
            require(int(completion.get('failed', -1)) == 0 and int(completion.get('passed', -1)) == len(expected_graphs)
                    and int(completion.get('graphs', -1)) == len(expected_graphs), 'Completion count mismatch')
            require(len(states) == len(expected_graphs) and {s['graph'] for s in states} == set(expected_graphs)
                    and all(s.get('status') == 'PASS' for s in states), 'Suite statuses incomplete/failed')
            require(len(graph_list) == len(expected_graphs) and {r['graph'] for r in graph_list} == set(expected_graphs),
                    'Unexpected graph inventory')
            recorded_source = digest_manifest(jobdir / 'source.sha256', [study / 'source_snapshot'], False)
            require(recorded_source == sources[kind]['source_hashes'], 'Job source digest record differs from build')
            captured_build = digest_manifest(jobdir / 'build_files.sha256', [study / 'build'], True)
            recorded_binary = {p: h for p, h in captured_build.items() if p in sources[kind]['binary_hashes']}
            require(recorded_binary == sources[kind]['binary_hashes'], 'Captured executable hashes differ from study build')
            needed = {'projection_methods', 'paper_original', 'generate_synthetic'} if kind == 'grouping' else {'selective_shape'}
            require(needed == {pathlib.Path(p).name for p in recorded_binary}, 'Required executable digest absent/unknown')
            binary_identity = sorted((pathlib.Path(p).name, h) for p, h in recorded_binary.items())
            jr['binary_identity'] = binary_identity
            affinity = read_json(jobdir / 'affinity.json')
            threads = int(affinity['threads'])
            require(threads == (4 if cohort == 'smoke' else 32), 'Unexpected compute thread count')
            require(len({t['socket'] for t in affinity['topology']}) == 1, 'Compute threads span sockets')
            require(len({(t['socket'], t['core']) for t in affinity['topology']}) == threads, 'Nonphysical-core affinity')
            submissions = []
            for path in study.glob('launcher/*/event.json'):
                submission = read_json(path)
                if str(submission.get('job', submission.get('job_id', ''))) == job:
                    submissions.append((path, submission))
            require(len(submissions) == 1, 'Submission provenance missing/ambiguous')
            path, submission = submissions[0]
            is_exclusive = bool(submission.get('exclusive', False))
            require(is_exclusive == (cohort == 'exclusive_intel_validation'), 'Exclusive/shared cohort mismatch')
            jr.update(node=event.get('node', account.get('NodeList', '')), threads=threads,
                      allocation_cpus=account.get('AllocCPUS', ''), exclusive=is_exclusive, tag=path.parent.name)
            graph_lookup = {r['graph']: r for r in graph_list}
            local_audits, local_summaries = [], []
            for graph in expected_graphs:
                info = dict(study=kind, job=job, cohort=cohort, graph=graph, node=jr['node'], threads=threads)
                ga, rows, state = graph_audit(jobdir / graph, kind, info)
                expected_digest = graph_lookup[graph].get('input_sha256')
                if expected_digest:
                    require(state['actual_input_sha256'] == expected_digest, 'Recorded graph digest mismatch')
                if cohort != 'smoke':
                    old = seen_input_hashes.setdefault(graph, state['actual_input_sha256'])
                    require(old == state['actual_input_sha256'], 'Same graph has different recorded hashes across jobs')
                local_audits.append(ga)
                local_summaries.extend(rows)
            graph_records.extend(local_audits)
            summaries.extend(local_summaries)
            jr.update(status='PASS', audited_graphs=len(local_audits),
                      numerical_checks=sum(r['gate_counts'].get('METHOD_CHECK', r['gate_counts'].get('ORDER_CHECK', 0)) for r in local_audits),
                      timing_rows=sum(r['timings'] for r in local_audits))
        except (ValueError, OSError, KeyError, TypeError) as exc:
            jr['error'] = str(exc)
        jobs.append(jr)
    skipped_account = sacct.get(SKIPPED_JOB, {})
    skipped = dict(job=SKIPPED_JOB, status='SKIPPED_CANCELLED', cohort='not_used',
                   reason='Cancelled dedicated intel_expr request; replaced by explicitly separate intel whole-node exclusive validation',
                   sacct=skipped_account, require_for_acceptance=False)
    all_pass = all(r['status'] == 'PASS' for r in jobs) and all(s['status'] == 'PASS' for s in sources.values())
    totals = collections.Counter()
    for record in graph_records:
        totals.update(record['gate_counts'])
        totals['raw_timing_rows'] += record['timings']
    audit = dict(created_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        status='PASS' if all_pass else 'FAIL', expected_required_jobs=len(INVENTORY),
        passing_jobs=sum(r['status'] == 'PASS' for r in jobs), jobs=jobs, skipped_jobs=[skipped],
        graph_audits=graph_records, sources=sources, scheduler_origin=scheduler_origin,
        scheduler_error=scheduler_error, gate_totals=dict(totals),
        graph_data_rehashed=False, input_digest_policy='Compare recorded graph SHA256 only; actual source/binary hashes checked',
        script_sha256=hashlib.sha256(pathlib.Path(__file__).read_bytes()).hexdigest(), input_evidence_sha256=INPUT_DIGESTS)
    aggregate = cohort_gmeans(summaries)
    summary = dict(status=audit['status'], gate_totals=dict(totals), source_binary_summary=sources,
        fixed_method_gmeans=aggregate, per_graph_matrix=summaries,
        precision_notes={'grouping': 'Source FP32 MKL timing anchor differs from BF16 FAST paths.',
                         'order': 'SOURCE_FP32_MKL timing anchor is not the matched-quantized MKL numerical reference.'},
        interpretation='All denominators are graph-local within-job medians. Shared/exclusive/smoke/synthetic cohorts separated. No best-per-graph oracle disguised as fixed method.',
        incomplete_jobs=[r['job'] for r in jobs if r['status'] != 'PASS'])
    out.mkdir(parents=True, exist_ok=False)
    (out / 'audit.json').write_text(json.dumps(audit, indent=2) + '\n')
    (out / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    (out / 'sacct.txt').write_text(raw_sacct)
    write_csv(out / 'jobs.csv', jobs + [skipped])
    for filename, kind, cohort in [
        ('group_primary.csv', 'grouping', 'shared_primary'), ('order_primary.csv', 'order', 'shared_primary'),
        ('group_exclusive.csv', 'grouping', 'exclusive_intel_validation'), ('order_exclusive.csv', 'order', 'exclusive_intel_validation'),
        ('group_synthetic.csv', 'grouping', 'synthetic_shared')]:
        write_csv(out / filename, [r for r in summaries if r['study'] == kind and r['cohort'] == cohort])
    (out / 'artifact_hashes.json').write_text(json.dumps(
        {p.name: digest_file(p) for p in out.iterdir() if p.is_file()}, indent=2) + '\n')
    print(json.dumps(dict(out=str(out), status=audit['status'], passing_jobs=audit['passing_jobs'],
                          expected_jobs=len(INVENTORY), graph_records=len(graph_records))))
    return 0 if all_pass else 2


if __name__ == '__main__':
    sys.exit(main())
