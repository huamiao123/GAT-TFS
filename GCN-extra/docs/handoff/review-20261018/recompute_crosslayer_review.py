"""Read-only reconstruction of published timing records; no benchmark execution."""
from pathlib import Path
import csv,json,re,statistics as st,math,collections
ROOT=Path(__file__).resolve().parent.parent/'tfs18'
OUT=Path(__file__).resolve().parent
B=ROOT/'results/crosslayer-fusion-20261008/runs'
checks={}
for family,rawglob in [('partial-bundle-20261013','logs/bundle-10944719/*time.csv'),('ordered-source-plan-20261014','raw/ordered-10955178/*time.csv')]:
    b=ROOT/'results'/family; d=collections.defaultdict(list)
    for p in b.glob(rawglob):
        for r in csv.DictReader(p.open()):d[r['graph'],r['method'],r.get('kind',r.get('protocol'))].append(float(r['ms']))
    errors=[];n=0
    for r in csv.DictReader((b/'timing_summary.csv').open()):
        k=(r['graph'],r['method'],r['protocol'])
        if k in d:
            n+=1
            if abs(st.median(d[k])-float(r['median_ms']))>1e-8:errors.append((k,st.median(d[k]),r['median_ms']))
    checks[family]={'raw_groups':len(d),'matched':n,'errors':errors}
interleaved=[]
for run in ['recompute-window-interleave-20261008-230519','recompute-window-interleave-20261008-230957','recompute-window-interleave-20261008-231443','recompute-window-adaptive-20261008-234255']:
    chunks={};cur=''
    for line in (B/run/'program.log').read_text().splitlines():
        if line.startswith('PAR_RUN'):
            d=dict(re.findall(r'(\w+)=([^\s]+)',line));cur=d['order'];chunks.setdefault(cur,[]).append((float(d['baseline_ms']),float(d['candidate_ms'])))
        if line.startswith('PLAN'):
            d=dict(re.findall(r'(\w+)=([^\s]+)',line));cur=d['graph'];chunks.setdefault(cur,[])
        if line.startswith('ADAPT_RUN'):
            d=dict(re.findall(r'(\w+)=([^\s]+)',line));chunks[cur].append((float(d['baseline_ms']),float(d['candidate_ms'])))
    for k,v in chunks.items():
        a,c=map(list,zip(*v));interleaved.append(dict(run=run,case=k,n=len(a),baseline_median=st.median(a),candidate_median=st.median(c),ratio=st.median(a)/st.median(c),median_paired_ratios=st.median(x/y for x,y in v),baseline_cv=st.pstdev(a)/st.mean(a),candidate_cv=st.pstdev(c)/st.mean(c)))
earlier=[]
contrasts=[('grouping-balance-20261007-01','s64_mfull_degree_fast',['s64_mfull_qguard_fast','s64_mfull_qsource_fast']),('state-reuse-20261007-05','p0_degree',['pack_epi_degree']),('state-reuse-20261007-05','pack_epi_degree',['pair_i_degree','pair_s_degree']),('pair-plan-20261007-01','pack_epi_degree',['opt_exact_s_degree']),('order-share-factorial-20261007-01','pack_epi_degree',['slot_pairs_s1_exact_s']),('event-trace-20261008-01','pack_epi_degree',['trace_s_rep','trace_s_cached'])]
for family,base,variants in contrasts:
    out=collections.defaultdict(list)
    for p in (B/family).glob('logs/*/*time.csv'):
        envp=p.parent/'environment.json';env=json.loads(envp.read_text()) if envp.exists() else {}
        part=env.get('SLURM_JOB_PARTITION','?');job=p.parent.name
        if job=='grouping-10887942':part='intel_wholenode_exclusive'
        grp=collections.defaultdict(list)
        for r in csv.DictReader(p.open()):
            if r['graph'].startswith(('regular','window','state','plan')):continue
            grp[r['graph'],r['kind'],r['method']].append(float(r['ms']))
        for (g,k,m),vs in grp.items():
            if m not in variants or (g,k,base) not in grp:continue
            out[part,k,m].append(dict(graph=g,job=job,ratio=st.median(grp[g,k,base])/st.median(vs)))
    for (part,k,m),v in out.items():
        earlier.append(dict(family=family,baseline=base,method=m,allocation=part,protocol=k,n=len(v),gm=math.exp(st.mean(math.log(x['ratio']) for x in v)),wins=sum(x['ratio']>1 for x in v),rows=v))
result=dict(method='Within-run ratio of conventional medians; population CV; no cross-node pooling of samples; no benchmark execution',summary_consistency=checks,interleaved=interleaved,early_families=earlier)
(OUT/'audit_crosslayer_calculations.json').write_text(json.dumps(result,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
print(json.dumps({'summary_consistency':checks,'interleaved_cases':len(interleaved),'early_aggregate_rows':len(earlier)}))
