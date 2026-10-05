"""Audit reconciled evidence and derive tables; never filter timing outliers."""
import csv
import hashlib
import json
import math
import pathlib
import statistics as st
import sys

REAL = ['ogbn-products', 'reddit', 'mycielskian19', 'roadNet-CA', 'wiki-Talk']


def read(root, name):
    with (root / name).open(newline='', encoding='utf-8') as f:
        return list(csv.DictReader(f))


def write(root, name, rows):
    if not rows:
        return
    with (root / name).open('w', newline='', encoding='utf-8') as f:
        w = csv.DictWriter(f, list(rows[0]))
        w.writeheader()
        w.writerows(rows)


def groups(rows, keys):
    out = {}
    for r in rows:
        out.setdefault(tuple(r[k] for k in keys), []).append(r)
    return out


def main():
    root = pathlib.Path(sys.argv[1])
    # This inventory precedes analysis and excludes the inventory itself.
    for name, expected in json.loads((root / 'artifact_hashes.json').read_text()).items():
        assert hashlib.sha256((root / name).read_bytes()).hexdigest() == expected, name
    audit = {'input_inventory_verified': True, 'p3': {}, 'p4': {}}
    for key, prefix in [('p3', 'd2'), ('p4', 'sel')]:
        d = root / key
        check = read(d, prefix + '_check.csv')
        for r in check:
            assert r['pass'] == r['finite'] == '1', r
            assert float(r['relative_L2']) < float(r['gate']), r
            assert float(r['normalized_max']) < float(r['gate']), r
        audit[key]['numerical_records'] = len(check)
        audit[key]['check_boundary_maxima'] = []
        for (boundary, reference), rows in groups(check, ['boundary', 'reference']).items():
            audit[key]['check_boundary_maxima'].append(dict(
                boundary=boundary, reference=reference, records=len(rows),
                max_rel_L2=max(float(r['relative_L2']) for r in rows),
                max_normalized_max=max(float(r['normalized_max']) for r in rows),
                max_abs=max(float(r['max_abs']) for r in rows)))
        for name in ['accept', 'complete', 'profile_gate', 'work'] + (['equiv'] if key == 'p4' else []):
            rows = read(d, prefix + '_' + name + '.csv')
            field = 'accepted' if name == 'accept' else 'pass'
            # P3 D2_WORK has exact counters, not a self-declared pass column.
            if field in rows[0]:
                assert all(r[field] == '1' for r in rows), name
            else:
                assert key == 'p3' and name == 'work'
            audit[key][name + '_records'] = len(rows)
        stage = read(d, prefix + '_stage.csv')
        fields = [k for k in stage[0] if k.endswith('_ms')]
        keys = ['graph', 'method', 'mode'] if key == 'p3' else ['graph', 'shape', 'method', 'mode']
        staged = []
        for group, rows in groups(stage, keys).items():
            assert len(rows) == 3
            summary = dict(zip(keys, group))
            for field in fields:
                summary[field] = st.median(float(r[field]) for r in rows)
            # Medians of children need not sum to median total; raw rows do.
            for r in rows:
                children = sum(float(r[k]) for k in fields if k != 'total_ms')
                assert abs(children - float(r['total_ms'])) < 1e-5
            if key == 'p4':
                method = summary['method']
                summary['cache1_bypass'] = int(method in ['fused_full_accurate', 'hybrid_none'])
                summary['cache2_bypass'] = int(method in ['fused_full_accurate', 'hybrid_none', 'project_used_L1_full_L2'])
            staged.append(summary)
        write(d, 'stage_summary.csv', staged)
        timings = read(d, 'timing_summary.csv')
        anomalies = [r for r in timings if float(r['cv']) > .05]
        write(d, 'timing_anomalies.csv', anomalies)
        audit[key]['timing_anomalies_cv_gt_5pct'] = anomalies
        # Independent PMU regions, not timing samples. Never pool graph shapes.
        pmu = read(d, 'method_pmu.csv')
        for r in pmu:
            assert r['available'] == r['coverage_complete'] == r['scaled_complete'] == '1', r
            assert int(r['failed_threads']) == int(r['zero_running_threads']) == 0, r
            assert int(r['new_threads']) == int(r['disappeared_threads']) == 0, r
            assert r['amx_busy_requested'] == '1'
        audit[key]['pmu_regions'] = len(pmu)
        audit[key]['pmu_per_tid_records'] = len(read(d, 'method_pmu_thread.csv'))
        pmus = []
        metrics = ['cycles_scaled', 'instructions_scaled', 'cache_misses_scaled',
                   'cache_references_scaled', 'amx_busy_scaled', 'region_wall_ms']
        for group, rows in groups(pmu, ['graph', 'method', 'mode']).items():
            assert len(rows) == 3
            s = dict(zip(['graph', 'method', 'mode'], group))
            for metric in metrics:
                s[metric] = st.median(float(r[metric]) for r in rows)
            s['median_per_region_IPC'] = st.median(float(r['instructions_scaled']) / float(r['cycles_scaled']) for r in rows)
            s['causal_scope'] = 'separate_region_thread_sum_generic_misses_not_DRAM'
            pmus.append(s)
        write(d, 'pmu_summary.csv', pmus)
        # All child phases explicitly flag non-additive thread-local diagnostics.
        details = read(d, prefix + '_detail.csv')
        assert all(r['additive_wall'] == '0' for r in details)
        audit[key]['sampled_phases'] = sorted({r['phase'] for r in details})
        audit[key]['fine_profile_layers'] = sorted({r.get('layer', '1') for r in details})

    p3time = {(r['graph'], r['method']): float(r['median_ms']) for r in read(root/'p3', 'timing_summary.csv')}
    ratios = []
    for method in ['d2_b64_all', 'd3_b64_period4']:
        rr = [p3time[g, 's64_mfull_accurate'] / p3time[g, method] for g in REAL]
        ratios.append(dict(method=method, geometric_mean_FULL_over_method=math.prod(rr)**(1/len(rr)),
                           wins_vs_FULL=sum(r>1 for r in rr)))
    audit['p3']['five_graph_fixed_ratios'] = ratios
    work3 = []
    for r in read(root/'p3', 'd2_work.csv'):
        if r['mode'] != 'focus':
            continue
        u = int(r['blocks']); c = int(r['corrections']); parts = int(r['parts'])
        assert parts == u + 2*c and int(r['tdp']) == 32*parts
        s = dict(r)
        s['B64_accurate_parts'] = 2*u
        if r['method'] == 'd2_b64_all':
            s['nonempty_tiles'] = c
            s['FULL_accurate_parts'] = 2*c
        else:
            s['nonempty_tiles'] = ''
            s['FULL_accurate_parts'] = ''
        work3.append(s)
    write(root/'p3', 'projection_comparison.csv', work3)

    p4time = {(r['graph'], r['shape'], r['method']): float(r['median_ms']) for r in read(root/'p4', 'timing_summary.csv')}
    ratios4 = []
    for shape in ['128_32_32', '128_128_128']:
        for method in ['top_1_64', 'top_1_16', 'top_1_4', 'project_all', 'project_used_L1_full_L2']:
            rr = [p4time[g,shape,'fused_full_accurate']/p4time[g,shape,method] for g in REAL]
            ratios4.append(dict(shape=shape,method=method, geometric_mean_FULL_over_method=math.prod(rr)**.2,
                                wins_vs_FULL=sum(r>1 for r in rr)))
    audit['p4']['five_graph_fixed_ratios'] = ratios4
    plan = {(r['graph'],r['method'],r['mode']):r for r in read(root/'p4', 'sel_plan.csv')}
    combined = []
    for r in read(root/'p4', 'sel_work.csv'):
        if r['mode'] == 'smoke':
            continue
        method = r['method']; layer = int(r['layer'])
        pm = 'hybrid_none' if method == 'fused_full_accurate' else ('hybrid_all' if method == 'project_all' else ('project_used' if method == 'project_used_L1_full_L2' else method))
        p = plan[r['graph'], pm, r['mode']]
        full = method == 'fused_full_accurate' or (method == 'project_used_L1_full_L2' and layer == 2)
        f = int(r['shape'].split('_')[1]); d = 128 if layer == 1 else f
        total_bytes = int(r['cold_H_request_bytes']) + int(r['hot_Q_request_bytes'])
        out = dict(r)
        out.update(input_dim=d, output_dim=f, selected_sources=p['selected_sources'],
                   covered_nonempty_rows=p['covered_nonempty_rows'], nonempty_rows=p['nonempty_rows'],
                   setup_cost_estimate_ms=p['setup_cost_estimate_ms'],
                   active_Q_bytes=0 if full else r['cache_storage_bytes'],
                   original_SEL_cache_storage_is_allocated_plan_bytes=1,
                   logical_sparse_bytes=total_bytes,
                   logical_sparse_bytes_over_FULL=total_bytes/(2*d*int(r['edges'])),
                   logical_bytes_not_DRAM=1)
        # full paths have no Q cache; cold tiling remains active.
        combined.append(out)
    write(root/'p4', 'work_plan_summary.csv', combined)
    # Per-thread diagnostics are from fully instrumented runs, not production
    # wall time or an estimate of the wall critical path.
    imbalance = []
    for group, rows in groups(read(root/'p4', 'sel_thread.csv'), ['graph','shape','method','layer','stage','mode']).items():
        v = [float(r['instrumented_active_ms']) for r in rows]
        s = dict(zip(['graph','shape','method','layer','stage','mode'], group))
        s.update(threads=len(v), min_instrumented_ms=min(v), max_instrumented_ms=max(v),
                 mean_instrumented_ms=st.mean(v), CV=st.pstdev(v)/st.mean(v) if st.mean(v) else 0,
                 max_over_mean=max(v)/st.mean(v) if st.mean(v) else 0,
                 processed_edges=sum(int(float(r['edges'])) for r in rows),
                 diagnostic_not_production_wall=1)
        imbalance.append(s)
    write(root/'p4', 'thread_imbalance_summary.csv', imbalance)
    (root/'ANALYSIS.json').write_text(json.dumps(audit,ensure_ascii=False,indent=2)+'\n', encoding='utf-8')
    print(json.dumps(audit,ensure_ascii=False,indent=2))


if __name__ == '__main__':
    main()
