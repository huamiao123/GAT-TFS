#!/usr/bin/env python3
"""Export completed evidence, append event, and update own handoff summary."""
import hashlib
import json
import pathlib
import shutil
import subprocess
import sys
import tarfile

root = pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
build, out = map(pathlib.Path, sys.argv[1:3])
gate_job, perf_job = sys.argv[3:5]
audit = json.loads((out / 'audit.json').read_text())
assert audit['status'] == 'PASS'
assert audit['performance_job'] == perf_job


def sha(p):
    h = hashlib.sha256()
    with p.open('rb') as f:
        for b in iter(lambda: f.read(8*1024*1024), b''):
            h.update(b)
    return h.hexdigest()


# Check every frozen source/binary hash before export; preserve failure evidence.
manifest=json.loads((build/'manifest.json').read_text())
for name, content in [('source.sha256',(build/'source.sha256').read_text()),
                      ('binary_manifest',manifest['binary_hashes'])]:
    for line in content.splitlines():
        expected, path = line.split(maxsplit=1)
        assert sha(pathlib.Path(path.strip())) == expected, (name, path)
for job in [gate_job, perf_job]:
    run = build / ('logs/ordered-' + job)
    for line in (run / 'binary.sha256').read_text().splitlines():
        expected, path = line.split(maxsplit=1)
        assert sha(pathlib.Path(path.strip())) == expected, (job, path)

def allowed(directory, names):
    return {n for n in names if n in {'tmp','__pycache__','.handoff.lock'}}

shutil.copytree(build / 'source_snapshot', out / 'source_snapshot', ignore=allowed, dirs_exist_ok=True)
raw = out / 'raw'
raw.mkdir(exist_ok=True)
for job in [gate_job, perf_job]:
    p = build / ('logs/ordered-' + job)
    shutil.copytree(p, raw / p.name, ignore=allowed, dirs_exist_ok=True)
for p in (build / 'logs').iterdir():
    if p.is_file():
        shutil.copy2(p, raw / p.name)
for name in ['manifest.json','source.sha256','binary.sha256','commands.txt','compile.log','generation.log','compiler.txt','ldd.txt']:
    p=build/name
    if p.exists():shutil.copy2(p,out/p.name)
(out/'binary.sha256').write_text(manifest['binary_hashes'])
(out/'commands.txt').write_text(manifest['commands']+'\n')
for p in (build/'build').iterdir():
    if p.is_file() and p.suffix in ['.txt','.log','.json','.sha256']:
        shutil.copy2(p,out/p.name)
for p in build.glob('*.txt'):
    if not (out/p.name).exists():shutil.copy2(p,out/p.name)
exp=root/'experiments/ordered_source_plan_20261014'
for name in ['analyze_ordered.py','augment_detail.py','augment_timer.py','finalize_ordered.py','generate_detail.py','build_detail.sh','timer_calibration.cpp','timer_job.sh','local_provenance.json']:
    shutil.copy2(exp/name,out/name)

sacct = subprocess.run(['sacct','-j',gate_job+','+perf_job,'--format=JobID,State,ExitCode,Elapsed,NodeList','-P'],check=True,capture_output=True,text=True).stdout
(out/'sacct.txt').write_text(sacct)
for job in [gate_job,perf_job]:
    line=next(x for x in sacct.splitlines() if x.startswith(job+'|'))
    assert '|COMPLETED|0:0|' in line,(job,line)

doc='GCN_EXTRA_ORDERED_SOURCE_PLAN_RESULTS.md'
readme='''# Ordered partial-source plan evidence

Read GCN_EXTRA_ORDERED_SOURCE_PLAN_RESULTS.md first. Static unit-weight two-layer
128->128->128 inference, random H/W seed12345. Fixed DegreeSort adjacent pairs,
S64, >=8 matches and <=8 event runs. FULL FAST main performance; M64/M256 and
ACCURATE are correctness controls, not measured main performance methods.
Shared intel: exploratory timing, paper_eligible=false.

source_snapshot/ is the exact compiled source; raw/ contains all commands,
graphs/fixtures, checks, 675 timing samples, thread diagnostic records and PMU.
Real graph data and executables are not exported. Source/binary hashes were
checked at finalization; remote absolute paths in manifests are intentional.
local_provenance.json describes the local base commit and uncommitted changes;
the remote source tree is not a Git checkout. Do not treat a base commit as a
commit containing this experiment. All thresholds and negative outcomes retained.
'''
(out/'README.md').write_text(readme)
event=dict(id='ordered-source-plan-archived-20261014',kind='analysis_export',status='PASS',
           build=str(build),evidence=str(out),report=str(out/doc),gate_job=gate_job,
           performance_job=perf_job,paper_eligible=False,raw_timings=675,
           conclusions='Partial sharing correct; fixed DegreeSort/current-CSR-order coverage limited. Must separate same-plan decode, dynamic matching and canonical baseline. No general performance/novelty claim.',
           issues='Shared-node evidence only; logical requests are not DRAM bytes; exact source hashes checked; no thresholds retuned',
           next='Read final five-graph report; do not expand17 solely from tiny/no coverage gains')
(out/'event.json').write_text(json.dumps(event,indent=2)+'\n')
subprocess.run(['python3',str(root/'scripts/record_event.py'),str(out/'event.json')],check=True)
subprocess.run(['python3',str(root/'scripts/record_event.py'),'check'],check=True)

handoff=root/'docs/handoff/CURRENT_HANDOFF.md'
text=handoff.read_text(encoding='utf-8')
anchor='# GCN-extra 当前交接摘要'
assert anchor in text
historical=text[text.index(anchor):]
header=f'''# 最新实验补充（2026-10-14）

partial bundle 与 ordered partial-source plan 两轮独立实验均已完成，历史正文如下。

- partial bundle：五图，Job10944718/10944719。固定G4 M64 FAST五图GM约1.0203，ACCURATE约1.0254；FULL控制也存在变化，不能全归因于C traffic。
- ordered source plan：build `{build.name}`，小图/结构Job{gate_job}，32核共享五图Job{perf_job}，全部检查PASS。固定相邻DegreeSort配对、S64、>=8 matches/<=8 runs，主性能FULL FAST。六张小图+五图结构先行检查，五图675条主时间；未扩大17图或回调阈值。
- 正式报告：`results/{out.name}/{doc}`。源码/原始输出/环境/图哈希/计时/PMU/工作量均已导出，原TFS/MKL和父冻结源码未覆盖。共享数据不能宣称独占论文结果。
- 主性能固定StaticShared五图正反顺序GM约1.0069，连续GM约1.0046；Myciel约0.998倍。Products近零覆盖、RoadNet零覆盖，不能把小幅时间差全归于省decode。这轮不是强通用加速或已成立的期刊贡献。
- load/decode细分补充：独立诊断build/Job见 `separated_detail_audit.json`。原load/decode细分零值为未测，已在独立快照拆开并通过逐位/工作量检查；主675条时间保留原快照，不混入诊断数据。
- 空区间校准：见 `timer_calibration_summary.json`，同节点32核；纳秒级load/decode/add接近计时底噪，不能据此计算production decode占比或机械扣除空区间。计时校准和原始数据完整保留。
- 本轮证明部分共享可以保持逐行加法顺序和四边界逐位一致；真实图选区覆盖很低，必须核对同计划独立/共享、同选区动态和PACK_EPI，不能把低覆盖图的时间差全部解释为decode收益。
- 当前授权节点上限5，本轮最多2个单节点作业；服务器写入仅wzh，yx只读。下面历史“10节点”不代表当前授权。
- GitHub尚未发布本轮新实验；本地base commit和dirty状态单独记录，不能把base commit误认为已包含新增代码。

---

'''
handoff.write_text(header+historical,encoding='utf-8')
handoffs=out/'handoff'
handoffs.mkdir(exist_ok=True)
for p in [handoff,*[root/'docs/handoff'/n for n in ['ALL_EXPERIMENTS.md','EXPERIMENT_ISSUES.md','PROJECT_PROGRESS.md','events.jsonl']]]:
    shutil.copy2(p,handoffs/p.name)
hashes={p.relative_to(out).as_posix():sha(p) for p in sorted(out.rglob('*')) if p.is_file()}
(out/'artifact_hashes.json').write_text(json.dumps(hashes,indent=2)+'\n')
archive=out/'source_and_raw_evidence.tar.gz'
with tarfile.open(archive,'w:gz') as tf:
    for p in sorted(out.iterdir()):
        if p != archive:tf.add(p,arcname=p.name)
(out/'archive.sha256').write_text(sha(archive)+'  '+archive.name+'\n')
print(json.dumps(dict(status='PASS',archive=str(archive),sha256=sha(archive),bytes=archive.stat().st_size,report=str(out/doc)),indent=2))
