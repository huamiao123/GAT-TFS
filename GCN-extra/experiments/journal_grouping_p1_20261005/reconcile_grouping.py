#!/usr/bin/env python3
"""Reconcile immutable P1 records, retaining all negative and noisy cases."""
import pathlib,sys,json,csv,math,statistics,subprocess,hashlib
ROOT=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
def write_csv(p,rows):
    if not rows:return
    keys=list(dict.fromkeys(k for r in rows for k in r))
    with p.open('w',newline='') as f:
        w=csv.DictWriter(f,keys);w.writeheader();w.writerows(rows)
def gmean(x):return math.exp(statistics.mean(math.log(v) for v in x))
def main():
    study=pathlib.Path(sys.argv[1]);out=pathlib.Path(sys.argv[2]);out.mkdir(parents=True,exist_ok=False)
    runs=[]
    for mode in ['smoke','focus','validation','synthetic']:
        candidates=[p for p in (study/'logs').glob(mode+'-*') if (p/'completion.json').exists()]
        assert len(candidates)==1,(mode,'missing/ambiguous complete run')
        p=candidates[0];c=json.loads((p/'completion.json').read_text());assert c['failed']==0
        assert (p/'exit_status.txt').read_text().strip()=='0'
        assert (p/'binary.sha256').read_text().splitlines()[:2]==(study/'build/binary.sha256').read_text().splitlines()[:2]
        runs.append(dict(mode=mode,path=str(p),completion=c,node=(p/'node.txt').read_text().strip(),affinity=json.loads((p/'affinity.json').read_text())))
    all_tables={};states=[];time_rows=[]
    for run in runs:
        if run['mode']=='smoke':continue
        p=pathlib.Path(run['path'])
        for state in json.loads((p/'suite_status.json').read_text()):
            assert state['status']=='PASS';state=dict(state,mode=run['mode'],node=run['node'],job=p.name.rsplit('-',1)[1]);states.append(state)
            graph=p/state['graph'];tables=json.loads((graph/'parsed.json').read_text())
            for key,rows in tables.items():all_tables.setdefault(key,[]).extend(rows)
            time_rows.extend(csv.DictReader((graph/'summary.csv').open()))
    assert len(states)==20 and len({r['graph'] for r in states})==20
    for key,rows in all_tables.items():write_csv(out/(key.lower()+'.csv'),rows)
    write_csv(out/'states.csv',states);write_csv(out/'timing_summary.csv',time_rows)
    primary={(r['graph'],r['method']):r for r in time_rows if r['kind']=='consecutive_e2e'}
    alternate={(r['graph'],r['method']):r for r in time_rows if r['kind']=='alternating_e2e'}
    real=[r['graph'] for r in states if r['mode']!='synthetic'];synth=[r['graph'] for r in states if r['mode']=='synthetic']
    assert len(real)==17 and len(synth)==3
    audit={(r['graph'],r['schedule']):r for r in all_tables['GROUP_AUDIT']}
    fixed=[];pair=[]
    for scope in ['mfull','m64']:
        for schedule in ['qshuffle','qsource','qpage']:
            method='s64_'+scope+'_'+schedule+'_fast';base='s64_'+scope+'_degree_fast';ratios=[];ar=[];tfs=[]
            for graph in real+synth:
                t=float(primary[graph,method]['median_ms']);b=float(primary[graph,base]['median_ms']);ratio=b/t
                at=float(alternate[graph,method]['median_ms']);ab=float(alternate[graph,base]['median_ms']);cost=float(audit[graph,schedule]['candidate_setup_upper_ms'])
                pair.append(dict(graph=graph,scope=scope,schedule=schedule,degree_ms=b,group_ms=t,speedup_vs_degree=ratio,alternating_speedup_vs_degree=ab/at,setup_upper_ms=cost,one_forward_plus_setup_upper_ms=t+cost,break_even_upper_forwards=math.ceil(cost/(b-t)) if b>t else '',group_cv=primary[graph,method]['cv'],degree_cv=primary[graph,base]['cv'],synthetic=int(graph in synth)))
                if graph in real:ratios.append(ratio);ar.append(ab/at);tfs.append(float(primary[graph,'paper_tfs']['median_ms'])/t)
            fixed.append(dict(scope=scope,schedule=schedule,graphs=17,gmean_vs_degree=gmean(ratios),alternating_gmean_vs_degree=gmean(ar),wins_vs_degree=sum(v>1 for v in ratios),wins_over_5pct=sum(v>1.05 for v in ratios),gmean_vs_source_tfs=gmean(tfs)))
    write_csv(out/'paired_grouping.csv',pair);write_csv(out/'fixed.csv',fixed)
    anomalies=[r for r in time_rows if float(r['cv'])>.05];write_csv(out/'timing_anomalies.csv',anomalies)
    meta=dict(runs=runs,real_graphs=real,synthetic_graphs=synth,numerical_checks=len(all_tables['CHECK']),same_scope_bitwise_checks=len(all_tables['EQUIV']),profile_bitwise_checks=len(all_tables['PROFILE_GATE']),same_work_checks=len(all_tables['GROUP_WORK_GATE']),schedule_audits=len(all_tables['GROUP_AUDIT']),fixed=fixed,timing_anomalies=len(anomalies),manifest=json.loads((study/'manifest.json').read_text()))
    (out/'summary.json').write_text(json.dumps(meta,indent=2)+'\n')
    lines=['# Journal P1: projection-budget-preserving destination grouping','',
        'Two-layer 128->128->128 prepared inference compute; random source H/W and unit edges. Original TFS/MKL and P0 kernels are unchanged. All comparisons are same graph/process/node, 32 physical cores, one socket, shared intel, default NUMA. The inherited 24 child timings are sampled thread diagnostics, not additive wall decomposition. Generic cache events and logical feature bytes are not measured DRAM bandwidth.','',
        f"Coverage: 17 real + 3 controlled graphs; {meta['numerical_checks']} numerical, {meta['same_scope_bitwise_checks']} same-scope bitwise, {meta['profile_bitwise_checks']} profile bitwise, {meta['same_work_checks']} work gates passed. All q64 slots and projection budgets preserved.",'',
        '## Fixed policies, real graphs only','',
        '| Scope | Schedule | Gmean vs DegreeSort | Alternating order | Wins | >5% wins | Gmean vs source TFS |',
        '|---|---|---:|---:|---:|---:|---:|']
    for r in fixed:lines.append(f"| {r['scope']} | {r['schedule']} | {r['gmean_vs_degree']:.4f} | {r['alternating_gmean_vs_degree']:.4f} | {r['wins_vs_degree']}/17 | {r['wins_over_5pct']}/17 | {r['gmean_vs_source_tfs']:.4f} |")
    for title,graphs in [('Real graph prepared E2E',real),('Controlled graphs (separate from real-graph gmean)',synth)]:
        lines+=['','## '+title,'','Times are five-repeat uninstrumented medians, ms.','', '| Graph | Source TFS | Source MKL | Coupled B64 | Coupled FULL | FULL Degree | FULL Shuffle | FULL Source | FULL Page | B64 Degree | B64 Shuffle | B64 Source | B64 Page |','|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|']
        names=['paper_tfs','source_mkl_fp32','b64_fast','bfull_fast']+['s64_'+s+'_'+p+'_fast' for s in ['mfull','m64'] for p in ['degree','qshuffle','qsource','qpage']]
        for graph in graphs:lines.append('| '+graph+' | '+' | '.join(f"{float(primary[graph,m]['median_ms']):.3f}" for m in names)+' |')
    lines+=['','## Preprocessing and amortization','', 'See group_setup.csv and group_audit.csv for allocation, parallel key pass, bounded segment creation, each sort, and verification. paired_grouping.csv charges a conservative candidate upper setup (the shared all-key pass and allocation are charged to each candidate). The derived break-even count uses the measured median difference and this upper cost; it is descriptive, not a production decision rule. Only prepared E2E enters the table above. One-forward plus setup is explicitly separate.','', '| Graph | Scope | Schedule | Prepared speedup | Setup upper ms | One forward + setup upper ms | Upper break-even forwards |','|---|---|---|---:|---:|---:|---:|']
    for r in pair:lines.append(f"| {r['graph']} | {r['scope']} | {r['schedule']} | {r['speedup_vs_degree']:.3f} | {r['setup_upper_ms']:.3f} | {r['one_forward_plus_setup_upper_ms']:.3f} | {r['break_even_upper_forwards'] or 'no measured gain'} |")
    lines+=['','## Evidence and limitations','', 'Full raw repetitions: time.csv; per-layer stages: stage.csv; 24 sampled child phases: detail.csv; per-thread work/completion: thread.csv and thread_detail.csv; work: counters.csv; PMU: pmu.csv, pmu_thread.csv and pmu_wall.csv; sampled source reuse/page occupancy: group_structure.csv. Preserve slowdowns and order discrepancies. Source/page signatures use only the first 64 CSR entries and relative feature-page IDs; neither is a complete neighborhood overlap estimator. The 4096 segment and signature constants were fixed before all performance runs.','', 'Synthetic graphs share N=524288, E=67108864, degree128, source in-degree128 and the same shuffled source ID mapping; native DegreeSort tile-local reuse is 1/4/16. They do not establish real-graph generality. Source TFS is BF16 and source MKL FP32, as in the original paper protocol. No classifier checkpoint accuracy or measured DRAM bandwidth is claimed.']
    (out/'JOURNAL_P1_RESULTS_20261005.md').write_text('\n'.join(lines)+'\n')
    (out/'reconcile_grouping.py').write_bytes(pathlib.Path(__file__).read_bytes())
    hashes={p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in out.iterdir() if p.is_file()};(out/'artifact_hashes.json').write_text(json.dumps(hashes,indent=2)+'\n')
    event=dict(id=out.name,kind='journal_p1_reconciliation',status='PASS',evidence=str(out),study=str(study),paper_eligible=False,summary={k:meta[k] for k in ['real_graphs','synthetic_graphs','numerical_checks','same_scope_bitwise_checks','profile_bitwise_checks','same_work_checks','schedule_audits','fixed']},issues='Shared-node paired complete forwards; all slowdowns retained; grouping preprocessing charged separately; generic PMU not DRAM bytes',next='Assess grouping necessity, amortization and held-out validity before P2/P3')
    (out/'event.json').write_text(json.dumps(event,indent=2)+'\n');subprocess.run([sys.executable,str(ROOT/'scripts/record_event.py'),str(out/'event.json')],check=True);print(json.dumps(event))
if __name__=='__main__':main()
