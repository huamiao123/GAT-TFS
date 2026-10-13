"""Run once on login node after the declared experiment directory is uploaded."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys

ROOT=Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
EXP=ROOT/'experiments/frontier_structure_20261008'
STUDY=ROOT/'runs/frontier-structure-20261008-01'
RESULT=ROOT/'results/frontier_structure_20261008'
names=['PROTOCOL.md','ANALYZER_INTERFACE.md','frontier_structure.cpp','grouping_schedule.hpp',
       'audit_frontier.py','frontier_prior_inputs.json','run_structure.py',
       'validate_real.py','structure_job.sh','stage_and_submit.py']

def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def save(path,obj):
    with path.open('x',encoding='utf-8') as stream:
        json.dump(obj,stream,ensure_ascii=False,indent=2);stream.write('\n')

def record(path,obj):
    save(path,obj)
    subprocess.run(['python3',str(ROOT/'scripts/record_event.py'),str(path)],check=True)

assert Path(__file__).resolve()==EXP.resolve()/'stage_and_submit.py'
assert not STUDY.exists() and not RESULT.exists(), 'Run/result already exists; no overwrite'
for name in names:
    assert (EXP/name).is_file(),name
assert sha(EXP/'grouping_schedule.hpp')==sha(ROOT/'runs/event-trace-20261008-01/source_snapshot/grouping_schedule.hpp')
rules={str(ROOT.parent/'AGENTS.md'):'55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7',
       str(ROOT.parent/'skills/tfs-research-engineering/SKILL.md'):'641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca'}
for path,digest in rules.items():
    assert sha(Path(path))==digest,('Changed authority',path)
subprocess.run(['python3',str(ROOT/'scripts/record_event.py'),'check'],check=True)
queue=subprocess.check_output(['squeue','-h','-u','hdacp1','-o','%D'],text=True)
requested=sum(int(n) for n in queue.split())
assert requested+1<=10,('User requested node cap',requested)
STUDY.mkdir();RESULT.mkdir()
snapshot=STUDY/'source_snapshot';snapshot.mkdir()
for name in names:
    shutil.copyfile(EXP/name,snapshot/name)
manifest=dict(experiment=str(EXP),run=str(STUDY),result=str(RESULT),scope='STRUCTURE_ONLY',
              source_sha256={name:sha(snapshot/name) for name in names},authority_sha256=rules,
              contract_sha256=sha(ROOT/'IMPLEMENTATION_CONTRACT.md'),
              authority_spec_sha256={name:sha(ROOT.parent/'TFS-Train/docs/spec'/name) for name in
               ('MATH_SPEC_V2.md','KERNEL_LAYOUT_V2.md','PYTORCH_EXTENSION_SPEC.md','EXPERIMENT_SPEC_V2.md')},
              previous_terminal_evidence=str(ROOT/'results/event_trace_20261008/final/compact_audit.json'),
              requested_nodes_before=requested,requested_nodes_this_study=1,user_cap=10,
              desktop_write=False,numa_interleave=False)
save(STUDY/'snapshot_manifest.json',manifest)
record(EXP/'prepare_event.json',dict(id='frontier-structure-prepared-20261008',status='PREPARED',
       evidence=str(STUDY),manifest=manifest,paper_eligible=False,
       issues='No new major issue; fixed inference shape and preserved sources. Structure-only serial producer abstraction, not runtime proof.',
       next='One shared-node compile/oracle/diagnostic4 job; stop on any gate failure.'))
cmd=['sbatch','--parsable','--partition=intel','--nodes=1','--ntasks=1','--cpus-per-task=1',
     '--mem=16G','--time=00:25:00','--job-name=gcn-frontier','--chdir='+str(STUDY),
     '--output='+str(STUDY/'slurm-%j.out'),'--error='+str(STUDY/'slurm-%j.err'),
     str(snapshot/'structure_job.sh'),str(STUDY)]
save(STUDY/'submission_command.json',cmd)
ret=subprocess.run(cmd,text=True,capture_output=True)
save(STUDY/'submission_response.json',dict(exit_code=ret.returncode,stdout=ret.stdout,stderr=ret.stderr))
if ret.returncode:
    record(STUDY/'submission_failure_event.json',dict(id='frontier-structure-submission-failed-20261008',
           status='FAILED',evidence=str(STUDY),issues=ret.stderr,paper_eligible=False))
    raise SystemExit(ret.returncode)
job=ret.stdout.strip().split(';')[0]
assert job.isdigit(),ret.stdout
save(STUDY/'jobs.json',{'job':job,'partition':'intel','nodes':1,'cpus':1,'mem':'16G','minutes':25})
record(STUDY/'submission_event.json',dict(id='frontier-structure-submitted-'+job,status='SUBMITTED',
       job=job,evidence=str(STUDY),paper_eligible=False,desktop_write=False,
       issues='No new major issue; graph-only counts, no performance kernel. User-authorized one shared node.',
       next='Verify oracle success, frozen input/permutation hashes, real-graph output and Slurm status.'))
print(json.dumps({'status':'SUBMITTED','job':job,'run':str(STUDY)}))
