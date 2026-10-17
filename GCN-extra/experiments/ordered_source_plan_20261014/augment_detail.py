#!/usr/bin/env python3
"""Attach separately compiled diagnostic evidence, never mix it into main times."""
import csv
import hashlib
import json
import pathlib
import shutil
import subprocess
import sys

build,out=map(pathlib.Path,sys.argv[1:3]);gate_job,job=sys.argv[3:5]
gate=build/('logs/ordered-'+gate_job);run=build/('logs/ordered-'+job)
root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')

def rows(p):
    with p.open(newline='') as f:return list(csv.DictReader(f))
def sha(p):
    h=hashlib.sha256()
    with p.open('rb') as f:
        for b in iter(lambda:f.read(8*1024*1024),b''):h.update(b)
    return h.hexdigest()
def write(p,rr):
    with p.open('w',newline='') as f:
        w=csv.DictWriter(f,list(dict.fromkeys(k for r in rr for k in r)));w.writeheader();w.writerows(rr)

completions=[]
for p,n in [(gate,11),(run,5)]:
    c=json.loads((p/'completion.json').read_text());assert c['status']=='PASS' and len(c['graphs'])==n
    assert (p/'exit_status.txt').read_text().strip()=='0'
    completions.append(c)
    for name in ['method_check','state_equiv','state_work_gate','state_profile_gate','state_semantic']:
        rr=rows(p/(name+'.csv'));assert rr and all(r['pass']=='1' for r in rr),(p,name)
assert 'STATE_TIME' not in completions[1]['counts']
for name in ['source.sha256','binary.sha256']:
    for line in (build/name).read_text().splitlines():
        digest,p=line.split(maxsplit=1);assert sha(pathlib.Path(p.strip()))==digest,(name,p)
main=json.loads((out/'audit.json').read_text())
main_counts={ (r['graph'],r['method'],r['layer']):r for r in rows(pathlib.Path(main['build'])/('logs/ordered-'+main['performance_job'])/'event_counters.csv') }
for r in rows(run/'event_counters.csv'):
    ref=main_counts[r['graph'],r['method'],r['layer']]
    for k in ['edges','decoded_rows','saved_decode_rows','adds','tdp','csr_reads','prefetch','partial_load_bytes','partial_store_bytes','packed_bytes']:
        assert r[k]==ref[k],(r['graph'],r['method'],k)
details=rows(run/'event_detail.csv');coarse=rows(run/'event_profile.csv')
summary=[]
for g in ['ogbn-products','reddit','mycielskian19','roadNet-CA','wiki-Talk']:
    for m in ['pack_epi_degree','event_independent','event_share','dynamic_selected_share']:
        for layer in ['1','2']:
            r=dict(graph=g,method=m,layer=layer)
            event_counts=[]
            for phase in ['source_bf16_load','source_bf16_decode','feature_fp32_add']:
                d=next(x for x in details if x['graph']==g and x['method']==m and x['layer']==layer and x['phase']==phase)
                assert int(d['events'])>0 and float(d['sampled_thread_ms'])>0,(g,m,phase)
                r[phase+'_sampled_thread_ms']=float(d['sampled_thread_ms']);r[phase+'_events']=int(d['events'])
                event_counts.append(int(d['events']))
            assert len(set(event_counts))==1
            d=next(x for x in coarse if x['graph']==g and x['method']==m and x['layer']==layer and x['phase']=='feature_gather_decode')
            r['coarse_gather_decode_sampled_thread_ms']=float(d['sampled_thread_ms'])
            r['units']='sampled/thread-local instrumented time; not production wall percentages'
            summary.append(r)
write(out/'separated_detail_summary.csv',summary)
dest=out/'separate_detail_evidence';dest.mkdir()
shutil.copytree(build/'source_snapshot',dest/'source_snapshot')
raw=dest/'raw';raw.mkdir()
def ignore(d,ns):return {n for n in ns if n in ['tmp','__pycache__']}
for p in [gate,run]:shutil.copytree(p,raw/p.name,ignore=ignore)
for p in (build/'logs').iterdir():
    if p.is_file():shutil.copy2(p,raw/p.name)
for p in build.iterdir():
    if p.is_file():shutil.copy2(p,dest/p.name)
sacct=subprocess.check_output(['sacct','-j',gate_job+','+job,'--format=JobID,State,ExitCode,Elapsed,NodeList','-P'],text=True)
(dest/'sacct.txt').write_text(sacct)
for j in [gate_job,job]:assert '|COMPLETED|0:0|' in next(x for x in sacct.splitlines() if x.startswith(j+'|'))
a=dict(status='PASS',build=str(build),gate_job=gate_job,diagnostic_job=job,
       node=(run/'node.txt').read_text().strip(),completion_counts=[c['counts'] for c in completions],
       source_manifest=json.loads((build/'manifest.json').read_text()),main_timing_samples_added=0,
       caveat='sampled timing split changes register lifetimes and timer overhead; diagnosis only, not production phase proportions',
       hashes_verified=True,logical_work_matches_main=True)
(out/'separated_detail_audit.json').write_text(json.dumps(a,indent=2)+'\n')
report=out/'GCN_EXTRA_ORDERED_SOURCE_PLAN_RESULTS.md'
text=report.read_text(encoding='utf-8')
text+='\n## 补充：独立诊断快照的 load/decode/add 拆分\n\n'
text+=f'主性能快照不变。新增诊断 build `{build.name}`；小图Job{gate_job}、五图32核Job{job}/{a["node"]}。全部数值、四边界、工作量及插桩输出检查PASS；逻辑工作量与主快照逐字段一致。未从这份build采集或混入任何STATE_TIME主性能样本。继承STATE_CONFIG的warmup/repeat标签在诊断模式不执行，以实际0条STATE_TIME为准。\n\n'
text+='原helper的load/decode合测、add另测；独立load/decode字段原为未测零。补充仅修改sampled的P=true分支，先加载8个256-bit BF16向量，再做扩展/位移decode，随后原FP32 add；非sampled及production分支仍用原表达式。独立计时改变向量生存期、寄存器压力、流水及timer开销；值不能直接解释production decode成本、不能求和冒充E2E wall百分比。\n\n'
text+='下表为Layer2，单位 sampled thread ms；全部Layer1/2、6方法、24细分段及12外层段保存在 separate_detail_evidence/raw 和 separated_detail_summary.csv。\n\n'
text+='| 图 | 方法 | BF16 load | BF16 decode | FP32 add | 采样source事件 |\n|---|---|---:|---:|---:|---:|\n'
for r in summary:
    if r['layer']=='2':text+='| '+r['graph']+' | '+r['method']+' | '+f'{r["source_bf16_load_sampled_thread_ms"]:.4f} | {r["source_bf16_decode_sampled_thread_ms"]:.4f} | {r["feature_fp32_add_sampled_thread_ms"]:.4f} | {r["source_bf16_load_events"]}'+' |\n'
text+='\n本轮细分提供定位线索，尚不能证明load/decode哪一项是production主瓶颈。最明确的结构证据仍是：固定原DegreeSort配对和CSR顺序后，可压缩部分共享覆盖率低；新静态计划五图prepared GM仅约1.0069×，最高覆盖Myciel没有改善。不要扩展17图后以逐图最好阈值拼成论文加速。\n'
report.write_text(text,encoding='utf-8')
e=dict(id='ordered-separated-detail-analysis-20261014',kind='diagnostic_analysis',status='PASS',evidence=str(out),audit=a,issues='Parent independent load/decode slots were not measured; separate diagnostic split now populated. Never interpret sampled sums as E2E wall percentages.',next='Archive both snapshots/raw evidence and finalize handoffs')
p=out/'detail_analysis_event.json';p.write_text(json.dumps(e,indent=2)+'\n');subprocess.run(['python3',str(root/'scripts/record_event.py'),str(p)],check=True)
print(json.dumps(dict(status='PASS',separated_records=len(summary),diagnostic_build=str(build)),indent=2))
