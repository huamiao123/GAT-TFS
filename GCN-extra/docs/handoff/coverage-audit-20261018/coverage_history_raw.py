import csv, json, re, statistics, collections, pathlib, math, argparse

parser=argparse.ArgumentParser(description='Read-only historical timing/summary audit; does not run kernels')
parser.add_argument('--root', type=pathlib.Path, default=pathlib.Path(r'C:\Users\花喵\tmp\tfs18pre'))
parser.add_argument('--out', type=pathlib.Path, default=pathlib.Path(__file__).parent)
args=parser.parse_args()
ROOT,OUT=args.root,args.out
def rows(p):
    with p.open(encoding='utf-8-sig', newline='') as f:
        return list(csv.DictReader(f))
def text(p): return p.read_text(encoding='utf8', errors='replace')
report = {'snapshot': '8aeef1613fdeaeb929379874c4152eccde620f74', 'runs': {}, 'issues': []}

for run in sorted((ROOT/'runs').iterdir()):
    if not run.is_dir() or not run.name.startswith(('formal-', 'smoke-')): continue
    item = {'timing_rows': 0, 'measured_rows': 0, 'stdout_matches': 0, 'correctness_rows': 0,
            'failed_correctness': [], 'summary_medians_checked': 0, 'summary_mismatches': [],
            'graphs': {}, 'statuses': {}, 'errors': []}
    for p in run.glob('*/timings.csv'):
        rs=rows(p); logs=text(p.parent/'stdout.log') if (p.parent/'stdout.log').exists() else ''
        raw={(m[0],m[1],int(m[2])):float(m[3]) for m in re.findall(r'TIME graph=\S+ method=(\S+) (kernel|e2e) rep=(\d+) ms=([\d.eE+-]+)',logs)}
        buckets=collections.defaultdict(list)
        for r in rs:
            item['timing_rows']+=1
            if r['kind'] not in ('kernel','e2e'): continue
            item['measured_rows']+=1
            key=(r['method'],r['kind'],int(r['repeat']))
            if key in raw and abs(raw[key]-float(r['ms']))<=1.1e-6: item['stdout_matches']+=1
            else: item['errors'].append({'file':str(p.relative_to(ROOT)), 'key':key, 'raw':raw.get(key),'csv':r['ms']})
            buckets[(r['method'],r['kind'])].append(float(r['ms']))
        graph_name=rs[0]['graph'] if rs else p.parent.name
        item['graphs'][graph_name]={f'{m}:{k}':{'n':len(v),'median_ms':statistics.median(v),'min_ms':min(v)} for (m,k),v in buckets.items()}
    for p in run.glob('*/correctness.csv'):
        rs=rows(p);item['correctness_rows']+=len(rs)
        item['failed_correctness'] += [dict(r, file=str(p.relative_to(ROOT))) for r in rs if r.get('pass') not in ('1','true','True')]
    for p in run.glob('*/stderr.log'):
        s=text(p).strip()
        if s: item['errors'].append({'stderr':str(p.relative_to(ROOT)), 'text':s[:1500]})
    for p in [run/'status.txt',run/'suite_status.json',run/'event.json']:
        if p.exists(): item['statuses'][p.name]=text(p)[:3500]
    summary=run/'summary.json'
    if summary.exists():
        for r in json.loads(text(summary)):
            key=f"{r['method']}:{r['kind']}"
            got=item['graphs'].get(r['graph'],{}).get(key)
            if got:
                item['summary_medians_checked']+=1
                if not math.isclose(got['median_ms'],r['median_ms'],rel_tol=1e-9,abs_tol=1e-7):
                    item['summary_mismatches'].append({'summary':r,'raw':got})
            else: item['summary_mismatches'].append({'no_raw_for_summary':r})
    report['runs'][run.name]=item

for name in ['source-formal-10864754','source-formal-10864792','source-smoke-10864752','source-smoke-10864830']:
    run=ROOT/'runs'/name
    item={'measured_rows':0,'all_timing_log_matches':0,'source_time_log_matches':0,'source_time_rows':0,'min_median_checks':0,'summary_mismatches':[], 'correctness_rows':0,'failed_correctness':[], 'graphs':{}, 'status':{}}
    for p in run.glob('*/time.csv'):
        rs=rows(p); buckets=collections.defaultdict(list)
        log=text(p.parent/'stdout.log')
        baseline_e2e=[float(x) for x in re.findall(r'run\s+\d+:\s*([0-9.]+)\s*ms',log)]
        raw={}
        for line in log.splitlines():
            if line.startswith('SOURCE_TIME '):
                d=dict(re.findall(r'(\w+)=(\S+)',line))
                raw[(d['method'],d['kind'],int(d['repeat']))]=float(d['ms'])
        for r in rs:
            if r['kind'] not in ('e2e','kernel'): continue
            item['measured_rows']+=1
            buckets[(r['method'],r['kind'])].append(float(r['ms']))
            key=(r['method'],r['kind'],int(r['repeat']))
            if r.get('precision')=='source_stdout_0.01ms':
                index=int(r['repeat'])+(5 if r['method']=='source_mkl_fp32' else 0)
                observed=baseline_e2e[index] if index<len(baseline_e2e) else None
            else: observed=raw.get(key)
            if observed is not None and abs(observed-float(r['ms']))<1e-7:
                item['all_timing_log_matches']+=1
            else: item['summary_mismatches'].append({'all_raw_mismatch':str(p),'key':key,'observed':observed})
            if r['method'] not in ('source_original_tfs','source_mkl_fp32'):
                item['source_time_rows']+=1
                key=(r['method'],r['kind'],int(r['repeat']))
                if key in raw and abs(raw[key]-float(r['ms']))<1e-7: item['source_time_log_matches']+=1
                else: item['summary_mismatches'].append({'missing_log':str(p.relative_to(ROOT)),'key':key})
        for s in rows(p.parent/'summary.csv'):
            got=buckets.get((s['method'],s['kind']))
            if got:
                item['min_median_checks']+=1
                if abs(min(got)-float(s['min_ms']))>1e-7 or abs(statistics.median(got)-float(s['median_ms']))>1e-7:
                    item['summary_mismatches'].append({'file':str(p),'summary':s})
        item['graphs'][p.parent.name]={f'{m}:{k}':{'n':len(v),'min_ms':min(v),'median_ms':statistics.median(v)} for (m,k),v in buckets.items()}
    for p in run.glob('*/correct.csv'):
        rs=rows(p);item['correctness_rows']+=len(rs)
        item['failed_correctness'] += [dict(r,file=str(p.relative_to(ROOT))) for r in rs if r.get('pass')!='1']
    for p in [run/'status.txt',run/'suite_status.json',run/'reconciled_suite_status.json']:
        if p.exists(): item['status'][p.name]=text(p)
    report['runs'][name]=item

run=ROOT/'runs/paper-formal-10864848'
item={'raw_repeat_rows':0,'raw_repeat_log_matches':0,'min_median_checks':0,'errors':[],'correctness_rows':0,'failed_correctness':[]}
rs=rows(run/'raw_repeats.csv');buckets=collections.defaultdict(list)
for r in rs:
    item['raw_repeat_rows']+=1
    method=r['method'];graph=r['graph'];rep=int(r['repeat']);val=float(r['ms'])
    buckets[(graph,method)].append(val)
    p=run/graph/('e2e.log' if method.endswith('e2e') else method+'.log')
    pattern=r'run\s+\d+:.*?Total=([0-9.]+)\s*ms' if method=='mkl_kernel' else r'run\s+\d+:\s*([0-9.]+)\s*ms'
    vals=[float(x) for x in re.findall(pattern,text(p))]
    if method=='mkl_e2e': rep+=5
    if len(vals)>rep and abs(vals[rep]-val)<1e-8:item['raw_repeat_log_matches']+=1
    else:item['errors'].append({'row':r,'file':str(p)})
for s in rows(run/'summary.csv'):
    item['correctness_rows']+=1
    if s['pass']!='1':item['failed_correctness'].append(s)
    for m in ['tfs_e2e','mkl_e2e','tfs_kernel','mkl_kernel']:
        vals=buckets[(s['graph'],m)];item['min_median_checks']+=1
        if abs(min(vals)-float(s[m+'_min_ms']))>1e-7 or abs(statistics.median(vals)-float(s[m+'_median_ms']))>1e-7:item['errors'].append({'summary':s,'method':m})
summary=rows(run/'summary.csv')
for kind in ['kernel_min','e2e_min','e2e_median']:
    item[kind+'_speedup_gm']=math.exp(statistics.mean(math.log(float(s[kind+'_speedup'])) for s in summary))
report['runs'][run.name]=item

(OUT/'coverage_history_raw.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf8')
print(json.dumps({k:{x:(len(v) if isinstance(v,list) else v) for x,v in d.items() if x not in ('graphs','status','statuses')} for k,d in report['runs'].items()},ensure_ascii=False,indent=2))
