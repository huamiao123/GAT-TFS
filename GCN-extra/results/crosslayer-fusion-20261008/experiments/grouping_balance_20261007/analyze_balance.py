#!/usr/bin/env python3
"""Read immutable grouping-balance evidence; write a new analysis directory.

Usage: python3 analyze_balance.py RUN [RUN ...] --out ROOT/results/NEW_DIR
Optional: --cohort JOB=LABEL (repeatable). Ratios are always formed within one
graph/process/job. Cohort geometric means combine those ratios, never raw times.
"""
import argparse
import collections
import csv
import datetime
import hashlib
import json
import math
import pathlib
import re
import statistics
import sys

ROOT = pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra').resolve()
DIAGNOSTIC = {'reddit', 'mycielskian19', 'roadNet-CA', 'amazon0601'}
REAL_GRAPHS = DIAGNOSTIC | {
    'ogbn-products', 'wiki-Talk', 'hollywood-2009', 'indochina-2004',
    'soc-Pokec', 'cit-Patents', 'soc-LiveJournal1', 'com-LiveJournal',
    'as-Skitter', 'rgg_n_2_24_s0', 'web-Google', 'email-Enron', 'com-Youtube'}
SCOPES = ('64', '256', 'full')
SCHEDULES = ('degree', 'qsource', 'qpage', 'qsamplepage', 'qguard')
KINDS = ('consecutive_e2e', 'alternating_e2e')
ANCHORS = ('paper_tfs', 'b64_fast', 'b256_fast', 'bfull_fast', 'source_mkl_fp32')
METHODS = set(ANCHORS) | {'s64_m%s_%s_fast' % (m, s) for m in SCOPES for s in SCHEDULES}
EXPECTED_GATES = {'METHOD_CHECK': 76, 'METHOD_EQUIV': 45,
                  'METHOD_PROFILE_GATE': 30, 'GROUP_WORK_GATE': 30, 'GROUP_AUDIT': 5}
META = ('run', 'job', 'node', 'partition', 'tag', 'cohort', 'threads', 'binary_set')
HASHES = {}


def read_json(path, default=None):
    if not path.is_file():
        if default is not None:
            return default
        raise ValueError('Missing ' + str(path))
    data = path.read_bytes()
    HASHES[str(path)] = hashlib.sha256(data).hexdigest()
    return json.loads(data.decode('utf-8-sig'))


def read_csv(path):
    data = path.read_bytes()
    HASHES[str(path)] = hashlib.sha256(data).hexdigest()
    return list(csv.DictReader(data.decode('utf-8-sig').splitlines()))


def optional_text(path):
    return path.read_text(encoding='utf-8-sig').strip() if path.is_file() else ''


def write_csv(path, rows, minimum=()):
    keys = list(minimum)
    for row in rows:
        for key in row:
            if key not in keys:
                keys.append(key)
    with path.open('x', newline='', encoding='utf-8') as stream:
        writer = csv.DictWriter(stream, fieldnames=keys or ['no_records'])
        writer.writeheader()
        writer.writerows(rows)


def table(parsed, name):
    # Both historical short keys and the new balance_suite full labels work.
    return parsed.get(name, parsed.get(name[7:] if name.startswith('METHOD_') else name, []))


def gm(values):
    return math.exp(sum(math.log(x) for x in values) / len(values))


def percentile(values, q):
    v = sorted(values)
    pos = q * (len(v) - 1)
    lo = int(pos)
    hi = min(lo + 1, len(v) - 1)
    return v[lo] + (v[hi] - v[lo]) * (pos - lo)


def scope_schedule(method):
    m = re.fullmatch(r's64_m(64|256|full)_(degree|qsource|qpage|qsamplepage|qguard)_fast', method)
    return m.groups() if m else ('', '')


def find_study(path):
    for candidate in (path,) + tuple(path.parents):
        if (candidate / 'source_snapshot').is_dir() and (candidate / 'logs').is_dir():
            return candidate
    raise ValueError('Cannot find study root for ' + str(path))


def discover(arguments):
    found = set()
    for arg in arguments:
        p = pathlib.Path(arg).resolve()
        if (p / 'parsed.json').is_file() and (p / 'status.json').is_file():
            found.add(p)
        elif (p / 'logs').is_dir():
            found.update(q.parent for q in p.glob('logs/*/*/status.json'))
        elif p.name == 'logs':
            found.update(q.parent for q in p.glob('*/*/status.json'))
        else:
            found.update(q.parent for q in p.glob('*/status.json'))
    return sorted(found)


def metadata(folder, overrides, launcher_cache):
    jobdir = folder.parent
    study = find_study(folder)
    event = read_json(jobdir / 'event.json', {})
    env = read_json(jobdir / 'environment.json', {})
    affinity = read_json(jobdir / 'affinity.json', {})
    job = str(event.get('job', event.get('job_id', env.get('SLURM_JOB_ID', jobdir.name.rsplit('-', 1)[-1]))))
    if study not in launcher_cache:
        lookup = {}
        for path in study.glob('launcher/*/event.json'):
            item = read_json(path)
            jid = str(item.get('job', item.get('job_id', '')))
            if jid:
                lookup[jid] = (item, path.parent.name)
        launcher_cache[study] = lookup
    submission, directory_tag = launcher_cache[study].get(job, ({}, ''))
    tag = str(submission.get('tag', event.get('tag', directory_tag)))
    partition = str(event.get('partition', submission.get('partition', env.get('SLURM_JOB_PARTITION', 'unknown'))))
    mode = str(event.get('mode', submission.get('mode', '')))
    if job in overrides:
        cohort = overrides[job]
    elif 'smoke' in mode:
        cohort = 'smoke_' + partition
    elif 'synthetic' in mode or 'synthetic' in tag:
        cohort = 'synthetic_' + partition
    elif tag.startswith(('gfocus', 'gval')):
        cohort = 'shared_primary' if partition == 'intel' else 'unexpected_partition_' + job
    elif tag.startswith('gexclusive') or partition == 'intel_expr':
        cohort = 'exclusive_validation'
    elif tag.startswith(('grepeat', 'greplic', 'gsharedrep')):
        # Distinct replication tags stay distinct; callers can join them explicitly.
        cohort = 'shared_replication_' + re.split(r'-20\d{6}', tag)[0]
    else:
        cohort = 'unclassified_job_' + job
    # New balance_job.sh captures build_files.sha256, not binary.sha256.
    # Only executable basenames + content digests identify a build. Metadata
    # paths, event files, logs and job IDs must never split same-build cohorts.
    executable_names = {'projection_methods', 'paper_original', 'generate_synthetic'}
    def executable_digests(path):
        if not path.is_file():
            raise ValueError('Missing required executable digest evidence: ' + str(path))
        data = path.read_bytes()
        HASHES[str(path)] = hashlib.sha256(data).hexdigest()
        found = {}
        for line in data.decode('utf-8-sig').splitlines():
            match = re.fullmatch(r'([0-9a-fA-F]{64})\s+[* ]?(.*)', line)
            if not match:
                raise ValueError('Malformed digest entry in ' + str(path))
            target = pathlib.Path(match.group(2)).resolve()
            if target.name not in executable_names:
                continue
            if target.parent != (study / 'build').resolve() or target.name in found:
                raise ValueError('Ambiguous executable identity in ' + str(path))
            found[target.name] = match.group(1).lower()
        if set(found) != executable_names:
            raise ValueError('Incomplete known executable identities in ' + str(path))
        return found
    expected_binary = executable_digests(study / 'build/binary.sha256')
    captured_binary = executable_digests(jobdir / 'build_files.sha256')
    if captured_binary != expected_binary:
        raise ValueError('Job executable hashes differ from its frozen study build: ' + job)
    canonical_binary = json.dumps(sorted(captured_binary.items()), separators=(',', ':'))
    binary_set = hashlib.sha256(canonical_binary.encode()).hexdigest()
    return dict(run=study.name, job=job, node=str(event.get('node') or optional_text(jobdir / 'node.txt') or 'unknown'),
                partition=partition, tag=tag, cohort=cohort,
                threads=str(affinity.get('threads', env.get('OMP_NUM_THREADS', 'unknown'))),
                binary_set=binary_set, job_status=str(event.get('status', 'INCOMPLETE')))


def check_record(folder, parsed, times):
    for name, count in EXPECTED_GATES.items():
        rows = table(parsed, name)
        if len(rows) != count or any(str(r.get('pass')) != '1' for r in rows):
            raise ValueError('%s requires %d passing rows' % (name, count))
    for name in ('METHOD_EQUIV', 'METHOD_PROFILE_GATE'):
        if any(str(r.get('bitwise')) != '1' for r in table(parsed, name)):
            raise ValueError(name + ' is not bitwise')
    complete = table(parsed, 'METHOD_COMPLETE')
    if len(complete) != 1 or complete[0].get('pass') != '1' or int(complete[0]['methods']) != 19:
        raise ValueError('METHOD_COMPLETE invalid')
    config = table(parsed, 'METHOD_CONFIG')
    if len(config) != 1 or any(config[0].get(k) != v for k, v in
            {'D': '128', 'F': '128', 'S': '64', 'TR': '16', 'R': '64',
             'numa': 'default', 'final_output': 'BF16', 'methods': '19'}.items()):
        raise ValueError('Unexpected computation/configuration')
    for row in table(parsed, 'METHOD_CHECK'):
        metrics = [float(row[k]) for k in ('max_abs', 'relative_L2', 'normalized_max')]
        if not all(math.isfinite(x) for x in metrics):
            raise ValueError('Nonfinite numerical check')
        expected = .03 if row['reference'] == 'source_MKL_FP32' else (.02 if row['boundary'] == 'e2e_BF16_final' else .01)
        if float(row['gate']) != expected or metrics[1] >= expected or metrics[2] >= expected:
            raise ValueError('Changed/failed FAST threshold')
    columns = ('graph', 'method', 'kind', 'repeat', 'order', 'ms', 'orientation')
    canonical = lambda rows: sorted(tuple(str(r.get(k, '')) for k in columns) for r in rows)
    if canonical(times) != canonical(table(parsed, 'METHOD_TIME')):
        raise ValueError('CSV / parsed METHOD_TIME mismatch')
    groups = collections.defaultdict(list)
    for row in times:
        if row['graph'] != folder.name or row['method'] not in METHODS or row['kind'] not in KINDS:
            raise ValueError('Unexpected timing key')
        ms = float(row['ms'])
        if not math.isfinite(ms) or ms <= 0:
            raise ValueError('Invalid timing')
        groups[row['method'], row['kind']].append(row)
    if set(groups) != {(m, k) for m in METHODS for k in KINDS}:
        raise ValueError('Incomplete method/kind matrix')
    for (method, kind), rows in groups.items():
        count = 5 if kind == KINDS[0] else 10
        if len(rows) != count or {int(r['repeat']) for r in rows} != set(range(count)):
            raise ValueError('Incomplete/duplicate repeats ' + method)
        if kind == KINDS[1] and any(r.get('orientation') != ('reverse' if int(r['repeat']) % 2 else 'forward') for r in rows):
            raise ValueError('Unbalanced alternating orientation')
    return groups


def analyze_record(meta, folder, parsed, groups):
    per_graph, interactions, selections, probes = [], [], [], []
    summary = {}
    for (method, kind), rows in groups.items():
        values = [float(r['ms']) for r in sorted(rows, key=lambda r: int(r['repeat']))]
        m, schedule = scope_schedule(method)
        row = dict(meta, graph=folder.name, method=method, kind=kind,
                   primary=int(kind == KINDS[0]), scope=m, schedule=schedule,
                   samples=len(values), median_ms=statistics.median(values),
                   min_ms=min(values), max_ms=max(values), mean_ms=statistics.mean(values),
                   cv=statistics.pstdev(values) / statistics.mean(values),
                   p25_ms=percentile(values, .25), p75_ms=percentile(values, .75),
                   samples_ms=json.dumps(values), evidence=str(folder))
        summary[method, kind] = row
    setup_rows = table(parsed, 'METHOD_GROUPING_SETUP')
    setup = float(setup_rows[0]['ms']) if len(setup_rows) == 1 else None
    for (method, kind), row in summary.items():
        references = {'tfs': 'paper_tfs', 'mkl': 'source_mkl_fp32', 'b256': 'b256_fast'}
        if row['scope']:
            references['degree'] = 's64_m%s_degree_fast' % row['scope']
        for label, reference in references.items():
            row['speedup_vs_' + label] = summary[reference, kind]['median_ms'] / row['median_ms']
            if kind == KINDS[1]:
                a = {int(r['repeat']): float(r['ms']) for r in groups[reference, kind]}
                b = {int(r['repeat']): float(r['ms']) for r in groups[method, kind]}
                ratios = [a[i] / b[i] for i in range(10)]
                row['paired_median_vs_' + label] = statistics.median(ratios)
                row['forward_paired_median_vs_' + label] = statistics.median(ratios[0::2])
                row['reverse_paired_median_vs_' + label] = statistics.median(ratios[1::2])
        row['all_schedules_setup_ms'] = setup if setup is not None else ''
        per_graph.append(row)
    for kind in KINDS:
        for schedule in SCHEDULES:
            a = summary['s64_m64_%s_fast' % schedule, kind]
            b = summary['s64_mfull_%s_fast' % schedule, kind]
            d64 = summary['s64_m64_degree_fast', kind]['median_ms']
            dfull = summary['s64_mfull_degree_fast', kind]['median_ms']
            interactions.append(dict(meta, graph=folder.name, kind=kind, schedule=schedule,
                m64_ms=a['median_ms'], mfull_ms=b['median_ms'],
                m64_to_full_speedup=a['median_ms'] / b['median_ms'],
                degree_m64_to_full_speedup=d64 / dfull,
                grouping_speedup_m64=a['speedup_vs_degree'],
                grouping_speedup_mfull=b['speedup_vs_degree'],
                interaction_full_over_m64=b['speedup_vs_degree'] / a['speedup_vs_degree'],
                interpretation='above_1_grouping_benefit_stronger_at_FULL', evidence=str(folder)))
    select = table(parsed, 'GROUP_GUARD_SELECT')
    if len(select) != 1 or str(select[0].get('uses_timing')) != '0':
        raise ValueError('Missing/nonstructural guard selection')
    selected = select[0]['selected']
    for p in table(parsed, 'GROUP_GUARD_PROBE'):
        probes.append(dict(meta, **p, evidence=str(folder)))
    for kind in KINDS:
        for scope in SCOPES:
            q = summary['s64_m%s_qguard_fast' % scope, kind]
            d = summary['s64_m%s_degree_fast' % scope, kind]
            policies = [summary['s64_m%s_%s_fast' % (scope, s), kind] for s in SCHEDULES]
            oracle = min(policies, key=lambda r: (r['median_ms'], r['method']))
            delta = d['median_ms'] - q['median_ms']
            item = dict(meta, graph=folder.name, kind=kind, scope=scope,
                selected_structural_path=selected, predicted_mean_miss_rate_ratio=select[0]['predicted_mean_miss_rate_ratio'],
                low_q_fallback=select[0].get('low_q_fallback', ''), uses_timing=0,
                guard_ms=q['median_ms'], degree_ms=d['median_ms'], speedup_vs_degree=q['speedup_vs_degree'],
                measured_direction='faster' if delta > 0 else ('slower' if delta < 0 else 'equal'),
                selected_nontrivial_path=int(selected != 'degree'),
                model_wrong_sign_observed=int(selected != 'degree' and delta < 0),
                wrong_sign_interpretation='descriptive_not_statistical_significance',
                guard_cv=q['cv'], degree_cv=d['cv'],
                oracle_same_scope_measured_policy=oracle['method'], oracle_ms=oracle['median_ms'],
                oracle_regret_guard_over_oracle=q['median_ms'] / oracle['median_ms'],
                oracle_interpretation='hindsight_only_all_measured_policies_not_guarded_candidate_oracle',
                probe_ms=select[0].get('probe_ms', ''), assembly_ms=select[0].get('assembly_ms', ''),
                all_schedules_setup_ms=setup if setup is not None else '',
                full_pipeline_one_forward_ms=setup + q['median_ms'] if setup is not None else '',
                full_pipeline_break_even_reuses=math.ceil(setup / delta) if setup is not None and delta > 0 and selected != 'degree' else '',
                break_even_status='no_runtime_change_degree_fallback' if selected == 'degree' else
                    ('no_positive_median_gain' if delta <= 0 else 'descriptive_median_delta_not_significance'),
                setup_interpretation='actual_all_five_schedule_pipeline_not_isolated_candidate_cost', evidence=str(folder))
            for prefix in ('paired_median', 'forward_paired_median', 'reverse_paired_median'):
                item[prefix + '_vs_degree'] = q.get(prefix + '_vs_degree', '')
            selections.append(item)
    return per_graph, interactions, selections, probes


def fixed_gmeans(per_graph):
    buckets = collections.defaultdict(list)
    for r in per_graph:
        key = tuple(r[k] for k in ('cohort', 'partition', 'threads', 'binary_set', 'method', 'kind'))
        buckets[key].append(r)
    out = []
    sets = [('all17', REAL_GRAPHS), ('diagnostic4', DIAGNOSTIC),
            ('remaining13', REAL_GRAPHS - DIAGNOSTIC)]
    for key, rows in sorted(buckets.items()):
        group = dict(zip(('cohort', 'partition', 'threads', 'binary_set', 'method', 'kind'), key))
        extras = {r['graph'] for r in rows} - REAL_GRAPHS
        subsets = sets + ([('other_synthetic_or_fixture', extras)] if extras else [])
        for label, targets in subsets:
            subset = [r for r in rows if r['graph'] in targets]
            if not subset:
                continue
            graph_counts = collections.Counter(r['graph'] for r in subset)
            duplicates = sorted(g for g, n in graph_counts.items() if n > 1)
            for metric in ('speedup_vs_degree', 'speedup_vs_b256', 'speedup_vs_tfs', 'speedup_vs_mkl'):
                eligible = [r for r in subset if metric in r]
                if not eligible:
                    continue
                values = [r[metric] for r in eligible]
                out.append(dict(group, subset=label, metric=metric,
                    graphs_observed=len(graph_counts), graphs_expected=len(targets),
                    coverage_complete=int(set(graph_counts) == targets and not duplicates),
                    graph_list=';'.join(sorted(graph_counts)), missing_graphs=';'.join(sorted(targets - set(graph_counts))),
                    duplicate_graphs=';'.join(duplicates), observations=len(eligible),
                    gmean='' if duplicates else gm(values),
                    min_speedup='' if duplicates else min(values), max_speedup='' if duplicates else max(values),
                    wins='' if duplicates else sum(v > 1 for v in values),
                    losses='' if duplicates else sum(v < 1 for v in values),
                    jobs=';'.join(sorted({r['job'] for r in eligible})),
                    nodes=';'.join(sorted({r['node'] for r in eligible})),
                    aggregation='geomean_of_within_job_graph_ratios_one_vote_per_graph',
                    status='DUPLICATE_GRAPH_ASSIGN_DISTINCT_COHORTS' if duplicates else
                           ('COMPLETE' if set(graph_counts) == targets else 'PARTIAL_COVERAGE')))
    return out


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('runs', nargs='+')
    ap.add_argument('--out', required=True)
    ap.add_argument('--cohort', action='append', default=[], metavar='JOB=LABEL')
    args = ap.parse_args()
    overrides = {}
    for mapping in args.cohort:
        job, sep, label = mapping.partition('=')
        if not sep or not job or not label:
            ap.error('--cohort requires JOB=LABEL')
        overrides[job] = label
    destination = pathlib.Path(args.out).resolve()
    if destination.exists():
        ap.error('--out must be a new, nonexistent directory')
    in_results = ROOT / 'results' in destination.parents
    in_study_analysis = any(
        destination.parent == find_study(pathlib.Path(arg).resolve())
        and destination.name.startswith('analysis_')
        for arg in args.runs)
    if not (in_results or in_study_analysis):
        ap.error('--out must be below ROOT/results or a new analysis_* directory in an input study')
    folders = discover(args.runs)
    if not folders:
        ap.error('No per-graph status.json records found')
    per_graph, interactions, selections, probes, gate_rows, records = [], [], [], [], [], []
    launcher_cache = {}
    for folder in folders:
        state = read_json(folder / 'status.json')
        parsed = read_json(folder / 'parsed.json', {})
        meta = metadata(folder, overrides, launcher_cache)
        record = dict(meta, graph=folder.name, source_status=state.get('status', 'MISSING'),
                      source_reason=state.get('reason', ''), evidence=str(folder), included=0)
        local_gates = []
        for label, expected in EXPECTED_GATES.items():
            rows = table(parsed, label)
            local_gates.append(dict(meta, graph=folder.name, gate=label, expected=expected,
                observed=len(rows), passing=sum(str(r.get('pass')) == '1' for r in rows),
                failing=sum(str(r.get('pass')) != '1' for r in rows),
                bitwise_passing=sum(str(r.get('bitwise')) == '1' for r in rows),
                source_status=state.get('status', ''), included=0, evidence=str(folder)))
        try:
            if state.get('status') != 'PASS':
                raise ValueError('Graph status is not PASS')
            times = read_csv(folder / 'method_time.csv')
            groups = check_record(folder, parsed, times)
            outputs = analyze_record(meta, folder, parsed, groups)
            for destination_rows, additions in zip((per_graph, interactions, selections, probes), outputs):
                destination_rows.extend(additions)
            record.update(included=1, analysis_status='PASS')
            for row in local_gates:
                row['included'] = 1
        except (ValueError, KeyError, TypeError, OSError) as exc:
            record.update(analysis_status='EXCLUDED', exclusion_reason=str(exc))
        records.append(record)
        gate_rows.extend(local_gates)
    aggregates = fixed_gmeans(per_graph)
    totals = {}
    for name in EXPECTED_GATES:
        selected = [r for r in gate_rows if r['gate'] == name]
        totals[name] = dict(observed_all=sum(r['observed'] for r in selected),
            passing_all=sum(r['passing'] for r in selected), failing_all=sum(r['failing'] for r in selected),
            included_observed=sum(r['observed'] for r in selected if r['included']),
            included_bitwise=sum(r['bitwise_passing'] for r in selected if r['included']))
    warnings = []
    if any(r['analysis_status'] != 'PASS' for r in records):
        warnings.append('Excluded failed/incomplete records are retained in record_status.csv and gate_summary.csv.')
    if any(r['duplicate_graphs'] for r in aggregates):
        warnings.append('Duplicate graph/cohort cells have no geometric mean; assign distinct --cohort JOB=LABEL values.')
    if any(r['cohort'].startswith('unclassified') for r in records):
        warnings.append('Unclassified jobs are isolated; use explicit --cohort assignments if appropriate.')
    manifest = dict(created_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
        script_sha256=hashlib.sha256(pathlib.Path(__file__).read_bytes()).hexdigest(),
        inputs=[str(pathlib.Path(p).resolve()) for p in args.runs], cohort_overrides=overrides,
        real_graphs=sorted(REAL_GRAPHS), diagnostic_graphs=sorted(DIAGNOSTIC),
        remaining_graphs=sorted(REAL_GRAPHS - DIAGNOSTIC), records=len(records),
        included=sum(r['included'] for r in records), excluded=sum(not r['included'] for r in records),
        primary='consecutive_e2e', order_control='alternating_e2e_reported_separately',
        speedup_definition='same_graph_same_job_reference_median / candidate_median; above 1 faster',
        interaction_definition='(degree_FULL/group_FULL)/(degree_M64/group_M64)',
        aggregation='cohort/partition/threads/binary_set; geomean graph-local within-job ratios; no repeated graph weighting',
        oracle='hindsight only; no deployed selector claim', gate_totals=totals, warnings=warnings,
        input_sha256=HASHES)
    lines = ['# Grouping x projection-scope analysis', '',
        'Primary: consecutive E2E. Alternating E2E is an independent order check. '
        'All speedup denominators come from the same graph, job and process. '
        'Cohort geometric means combine graph-local ratios; shared/exclusive and replications remain separate.', '',
        'Included %d / %d graph records. Full records and exclusions: record_status.csv.' % (manifest['included'], len(records)), '',
        '| Gate | Included rows | All observed | All failed |',
        '|---|---:|---:|---:|']
    for name, values in totals.items():
        lines.append('| %s | %d | %d | %d |' % (name, values['included_observed'], values['observed_all'], values['failing_all']))
    lines += ['', 'Selected fixed-policy primary results (GM > 1 means faster; partial coverage explicitly labelled):', '',
        '| Cohort | Subset | Method | Reference | Graphs | GM | Status |',
        '|---|---|---|---|---:|---:|---|']
    for r in aggregates:
        if r['kind'] == KINDS[0] and r['method'].endswith('_qguard_fast') and r['metric'] in ('speedup_vs_degree', 'speedup_vs_b256', 'speedup_vs_tfs'):
            value = 'NA' if r['gmean'] == '' else '%.5f' % r['gmean']
            lines.append('| %s | %s | %s | %s | %d/%d | %s | %s |' %
                (r['cohort'], r['subset'], r['method'], r['metric'], r['graphs_observed'], r['graphs_expected'], value, r['status']))
    wrong = [r for r in selections if r['kind'] == KINDS[0] and r['model_wrong_sign_observed']]
    lines += ['', 'Qguard nontrivial selections with a slower primary median: %d scope/graph/job cases. '
              'These are retained in qguard_selection.csv; the sign is descriptive, not a significance test.' % len(wrong),
              '', 'group_x_scope_interactions.csv reports (grouping benefit at FULL)/(benefit at M64); >1 means a stronger grouping benefit at FULL. '
              'Oracle fields are explicitly hindsight over measured policies, not a guarded-candidate oracle or implementable policy.',
              '', 'Setup is the measured complete five-schedule pipeline. Full-pipeline single-forward and break-even columns do not estimate an isolated candidate setup cost. '
              'Degree fallback has no changed permutation, so its small timing differences are not used to claim amortization. '
              'Sampled thread phases are not additive wall fractions; logical counters are not DRAM traffic.']
    if warnings:
        lines += ['', 'Warnings:'] + ['- ' + w for w in warnings]
    destination.mkdir(parents=True, exist_ok=False)
    for name, rows in [('per_graph.csv', per_graph), ('fixed_gmeans.csv', aggregates),
                       ('group_x_scope_interactions.csv', interactions), ('qguard_selection.csv', selections),
                       ('qguard_probes.csv', probes), ('gate_summary.csv', gate_rows), ('record_status.csv', records)]:
        write_csv(destination / name, rows, META)
    (destination / 'summary.json').write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
    (destination / 'README.md').write_text('\n'.join(lines) + '\n', encoding='utf-8')
    artifacts = {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in destination.iterdir() if p.is_file()}
    (destination / 'artifact_hashes.json').write_text(json.dumps(artifacts, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(dict(out=str(destination), included=manifest['included'], excluded=manifest['excluded'],
                          per_graph_rows=len(per_graph), warnings=warnings)))
    return 0 if per_graph else 2


if __name__ == '__main__':
    sys.exit(main())
