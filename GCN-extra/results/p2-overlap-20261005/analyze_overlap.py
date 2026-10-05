#!/usr/bin/env python3
import pathlib,sys,json,csv,shlex,statistics,math,hashlib,subprocess
def main():
    run=pathlib.Path(sys.argv[1]);out=pathlib.Path(sys.argv[2]);out.mkdir(parents=True,exist_ok=False)
    assert (run/'exit_status.txt').read_text().strip()=='0';tables={}
    for line in (run/'results/overlap.log').read_text().splitlines():
        label,_,body=line.partition(' ')
        if not (label.startswith('OVERLAP_') or label.startswith('METHOD_PMU')):continue
        tables.setdefault(label,[]).append(dict(item.split('=',1) for item in shlex.split(body) if '=' in item))
    assert len(tables['OVERLAP_GATE'])==5 and all(r['pass']=='1' for r in tables['OVERLAP_GATE'])
    assert len(tables['OVERLAP_PROFILE_GATE'])==10 and all(r['pass']=='1' for r in tables['OVERLAP_PROFILE_GATE'])
    for name,rows in tables.items():
        keys=list(dict.fromkeys(k for r in rows for k in r))
        with (out/(name.lower()+'.csv')).open('w',newline='') as f:w=csv.DictWriter(f,keys);w.writeheader();w.writerows(rows)
    groups={}
    for r in tables['OVERLAP_TIME']:groups.setdefault((r['case'],r['method']),[]).append(r)
    summary=[]
    for case in sorted({c for c,m in groups}):
        rows={};vs={}
        for m in ['serial','interleaved']:
            rows[m]=groups[case,m];assert len(rows[m])==7 and {int(r['repeat']) for r in rows[m]}==set(range(7));vs[m]=[float(r['ms']) for r in rows[m]]
        paired=[]
        for rep in range(7):paired.append(float(next(r['ms'] for r in rows['serial'] if int(r['repeat'])==rep))/float(next(r['ms'] for r in rows['interleaved'] if int(r['repeat'])==rep)))
        r=dict(case=case,serial_median_ms=statistics.median(vs['serial']),interleaved_median_ms=statistics.median(vs['interleaved']),median_speedup=statistics.median(vs['serial'])/statistics.median(vs['interleaved']),paired_ratio_median=statistics.median(paired),paired_ratio_min=min(paired),paired_ratio_max=max(paired))
        for m in ['serial','interleaved']:r.update({m+'_min_ms':min(vs[m]),m+'_max_ms':max(vs[m]),m+'_cv':statistics.pstdev(vs[m])/statistics.mean(vs[m])})
        for m in ['serial','interleaved']:
            events=[x for x in tables['METHOD_PMU'] if x['graph']==case and x['method']==m and x['available']=='1' and x['coverage_complete']=='1' and x['scaled_complete']=='1']
            r[m+'_pmu_regions']=len(events)
            for key in ['cycles_scaled','instructions_scaled','ref_cycles_scaled','cache_misses_scaled','amx_busy_scaled']:
                if events:r[m+'_'+key]=statistics.median(float(x[key]) for x in events)
        summary.append(r)
    with (out/'summary.csv').open('w',newline='') as f:w=csv.DictWriter(f,list(dict.fromkeys(k for r in summary for k in r)));w.writeheader();w.writerows(summary)
    metadata=dict(run=str(run),node=(run/'logs/node.txt').read_text().strip(),affinity=json.loads((run/'logs/affinity.json').read_text()),manifest=json.loads((run/'manifest.json').read_text()),summary=summary)
    (out/'summary.json').write_text(json.dumps(metadata,indent=2)+'\n');(out/'overlap.log').write_bytes((run/'results/overlap.log').read_bytes());(out/'analyze_overlap.py').write_bytes(pathlib.Path(__file__).read_bytes())
    lines=['# P2 fixed-work AVX/AMX interleaving microbenchmark','',
      'One physical core, shared SPR node, same case/process, default NUMA. This is a local producer/projection handoff test, not two-layer GCN E2E. All five complete FP32 output memcmp gates and ten profile gates passed. Both paths use the same two buffers, real source sequence, FP32 reduction order, BF16 packing, AMX instructions, and output stores. SERIAL projects a tile then prepares the next; INTERLEAVED prepares two rows of the next tile after each current AMX K-block. The names hot/random indicate source working sets, not proved cache-hit states.','',
      '| Case | Serial ms | Interleaved ms | Serial/Interleaved | Paired ratio min/median/max | Serial CV | Interleaved CV |','|---|---:|---:|---:|---|---:|---:|']
    for r in summary:lines.append(f"| {r['case']} | {r['serial_median_ms']:.3f} | {r['interleaved_median_ms']:.3f} | {r['median_speedup']:.4f} | {r['paired_ratio_min']:.3f}/{r['paired_ratio_median']:.3f}/{r['paired_ratio_max']:.3f} | {r['serial_cv']:.3f} | {r['interleaved_cv']:.3f} |")
    lines+=['','A ratio above one favors interleaving. Seven alternating-order paired repetitions are retained. Sampled phase times measure thread instruction issue/stalls and include instrumentation; they are not hardware completion times or additive wall fractions. PMU retains generic cycles/instructions/ref-cycles/cache events and speculative SPR AMX busy, per-TID scaling and enable/disable skew. Neither simultaneously nonzero counters nor a faster interleaved loop alone proves hardware overlap as a unique cause. No extrapolation to model inference or DRAM bandwidth is made.','', 'Each case has 1024 x 16 rows, D=F=128, 32768 TDPBF16PS instructions and 8MiB final FP32 output writes. Producer visits are 16384*degree. Pool BF16 H sizes are 256KiB and 128MiB. All analytic work counters are in overlap_config.csv; no projection work is added to inflate utilization. The microbenchmark does not cover S64/MFULL row-window execution or multithread throughput, so deployment requires a separate same-work full-model gate if this stage justifies it.']
    (out/'P2_OVERLAP_RESULTS_20261005.md').write_text('\n'.join(lines)+'\n');(out/'artifact_hashes.json').write_text(json.dumps({p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in out.iterdir() if p.is_file()},indent=2)+'\n')
    event=dict(id=out.name,kind='p2_overlap_analysis',status='PASS',evidence=str(out),run=str(run),paper_eligible=False,summary=summary,issues='One-core microbenchmark, not full two-layer inference; fixed work/FP32 outputs verified; retains slowdowns and noisy small cases',next='No full-kernel buffering expansion from this implementation without stronger evidence')
    (out/'event.json').write_text(json.dumps(event,indent=2)+'\n');subprocess.run([sys.executable,'/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/scripts/record_event.py',str(out/'event.json')],check=True);print(json.dumps(metadata))
if __name__=='__main__':main()
