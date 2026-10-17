#!/usr/bin/env python3
"""Record empty timestamp resolution/floor without correcting main results."""
import pathlib,csv,json,shutil,statistics,subprocess,sys,hashlib
build,out=map(pathlib.Path,sys.argv[1:3]);job=sys.argv[3]
run=build/('logs/cal-'+job)
assert (run/'exit_status.txt').read_text().strip()=='0'
rr=list(csv.DictReader((run/'timer.csv').open()))
assert len(rr)==352
formal=[r for r in rr if r['warmup']=='0'];assert len(formal)==320
assert set(int(r['tid']) for r in formal)==set(range(32))
assert all(int(r['negative_count'])==0 for r in rr)
s=dict(status='PASS',job=job,node=(run/'node.txt').read_text().strip(),formal_records=len(formal),
       mean_empty_ns_median=statistics.median(float(r['mean_empty_ns']) for r in formal),
       mean_empty_ns_min=min(float(r['mean_empty_ns']) for r in formal),
       mean_empty_ns_max=max(float(r['mean_empty_ns']) for r in formal),
       zero_fraction_median=statistics.median(float(r['zero_fraction']) for r in formal),
       median_empty_ns_median=statistics.median(float(r['median_empty_ns']) for r in formal),
       min_positive_ns=min(float(r['min_positive_ns']) for r in formal if float(r['min_positive_ns'])>0),
       timestamp_ulp_ns_median=statistics.median(float(r['timestamp_ulp_ns']) for r in formal),
       precision_note='empirical empty interval/returned-double spacing, not exact hardware timer resolution',
       consequence='tiny phases are near instrumentation floor; no mechanical subtraction, no production decoder cost percentage')
for line in (build/'hashes.sha256').read_text().splitlines():
    expected,p=line.split(maxsplit=1);assert hashlib.sha256(pathlib.Path(p.strip()).read_bytes()).hexdigest()==expected
dest=out/'timer_calibration_evidence'
shutil.copytree(build,dest,ignore=lambda d,ns:{n for n in ns if n in ['build','tmp','__pycache__']})
sacct=subprocess.check_output(['sacct','-j',job,'--format=JobID,State,ExitCode,Elapsed,NodeList','-P'],text=True)
assert '|COMPLETED|0:0|' in next(x for x in sacct.splitlines() if x.startswith(job+'|'))
(dest/'sacct.txt').write_text(sacct)
(out/'timer_calibration_summary.json').write_text(json.dumps(s,indent=2)+'\n')
p=out/'GCN_EXTRA_ORDERED_SOURCE_PLAN_RESULTS.md'
text=p.read_text(encoding='utf-8')
text+='\n## 补充：空区间计时校准\n\n'
text+=f'同节点{s["node"]}、单socket32物理核 Job{job}，复制原tick/detail_tock的omp_get_wtime表达式进行空区间校准，1批warmup+10批×32线程×20000区间，320条正式统计。源代码、flags、环境及全部原始结果保存于 timer_calibration_evidence。\n\n'
text+=f'空区间均值的中位数 **{s["mean_empty_ns_median"]:.3f} ns**；min/max {s["mean_empty_ns_min"]:.3f}/{s["mean_empty_ns_max"]:.3f} ns；区间中位数 {s["median_empty_ns_median"]:.3f} ns；零值比例中位数 **{100*s["zero_fraction_median"]:.3f}%**；最小观测正差 {s["min_positive_ns"]:.3f} ns；返回double的ULP中位数 {s["timestamp_ulp_ns_median"]:.3f} ns。没有负区间。\n\n'
text+='这意味着单次微段往往小于返回时间戳的可表示间距，聚合均值同时包含计时开销和量化误差。补充的load/decode/add数据仅作诊断线索，不能据此宣称decode占production某个百分比；也不能机械减去一个空区间均值。细分计时还会改变寄存器生存期及指令调度。ms级E2E中位数仍来自原主快照，没有修正或替换。后续若要定量区分这些纳秒微段，应测足够长的成组区域，并校准更细粒度的时间基准/硬件事件；这属于测量设计，不是当前已实现的加速方法。\n'
p.write_text(text,encoding='utf-8')
root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
e=dict(id='ordered-source-timer-cal-analysis-20261014',kind='measurement_calibration_analysis',status='PASS',evidence=str(out),summary=s,issues='Do not treat microphase empty-slot0 as zero cost or sampled sums as wall percent; main timing unchanged',next='Finalize evidence archive and handoffs')
f=out/'timer_analysis_event.json';f.write_text(json.dumps(e,indent=2)+'\n');subprocess.run(['python3',str(root/'scripts/record_event.py'),str(f)],check=True)
print(json.dumps(s,indent=2))
