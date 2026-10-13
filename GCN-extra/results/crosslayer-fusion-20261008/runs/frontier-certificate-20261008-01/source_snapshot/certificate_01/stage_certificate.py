import hashlib
import json
from pathlib import Path
import shutil
import subprocess

ROOT=Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
EXP=ROOT/'experiments/frontier_structure_20261008/certificate_01'
STUDY=ROOT/'runs/frontier-certificate-20261008-01'
RESULT=ROOT/'results/frontier_structure_20261008/certificate-01'
PARENT=ROOT/'runs/frontier-structure-20261008-01'
child_names=['PROTOCOL.md','frontier_certificate.cpp','verify_certificate.py',
             'run_certificate.py','certificate_job.sh','stage_certificate.py']
root_names=['frontier_structure.cpp','grouping_schedule.hpp','frontier_prior_inputs.json']

def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def save(path,obj):
    with path.open('x',encoding='utf-8') as f:json.dump(obj,f,indent=2);f.write('\n')
def record(path,obj):
    save(path,obj);subprocess.run(['python3',str(ROOT/'scripts/record_event.py'),str(path)],check=True)

assert Path(__file__).resolve()==EXP.resolve()/'stage_certificate.py'
assert not STUDY.exists() and not RESULT.exists(),'refuse overwrite'
oldmanifest=json.loads((PARENT/'snapshot_manifest.json').read_text())
for n in root_names:assert sha(EXP.parent/n)==oldmanifest['source_sha256'][n],n
rules={str(ROOT.parent/'AGENTS.md'):'55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7',
       str(ROOT.parent/'skills/tfs-research-engineering/SKILL.md'):'641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca'}
for path,digest in rules.items():assert sha(Path(path))==digest,path
assert sha(ROOT/'IMPLEMENTATION_CONTRACT.md')=='b6419e5106278106da6727c90f4cfa185cc70c587d70b4087c0bc286fd0dc244'
subprocess.run(['python3',str(ROOT/'scripts/record_event.py'),'check'],check=True)
queue=subprocess.check_output(['squeue','-h','-u','hdacp1','-o','%D'],text=True)
requested=sum(map(int,queue.split()));assert requested+1<=10,requested
STUDY.mkdir();RESULT.mkdir();snapshot=STUDY/'source_snapshot';snapshot.mkdir();(snapshot/'certificate_01').mkdir()
for n in root_names:shutil.copyfile(EXP.parent/n,snapshot/n)
for n in child_names:shutil.copyfile(EXP/n,snapshot/'certificate_01'/n)
names=root_names+['certificate_01/'+n for n in child_names]
parent_paths=[PARENT/'snapshot_manifest.json',PARENT/'compute-10898893/completion.json']
for g in ['mycielskian19','reddit']:
    parent_paths.extend([PARENT/'compute-10898893'/g/n for n in ['structure.json','status.json']])
manifest=dict(source_sha256={n:sha(snapshot/n) for n in names},authority_sha256=rules,
    contract_sha256=sha(ROOT/'IMPLEMENTATION_CONTRACT.md'),
    authority_spec_sha256={n:sha(ROOT.parent/'TFS-Train/docs/spec'/n) for n in
    ['MATH_SPEC_V2.md','KERNEL_LAYOUT_V2.md','PYTORCH_EXTENSION_SPEC.md','EXPERIMENT_SPEC_V2.md']},
    parent_evidence_sha256={str(p):sha(p) for p in parent_paths},
    requested_nodes_before=requested,requested_nodes_this_study=1,user_cap=10,
    desktop_write=False,numa_interleave=False,scope='RESTRICTED_STATE_LOWER_BOUND')
save(STUDY/'snapshot_manifest.json',manifest)
record(EXP/'prepare_event.json',dict(id='frontier-certificate-prepared-20261008',status='PREPARED',
    evidence=str(STUDY),manifest=manifest,paper_eligible=False,
    issues='No new major issue. Capacity-two matching certifies fixed-order restricted state only.',
    next='One CPU small self-test plus two real graph certificate construction and independent verification.'))
cmd=['sbatch','--parsable','--partition=intel','--nodes=1','--ntasks=1','--cpus-per-task=1',
    '--mem=16G','--time=00:25:00','--job-name=gcn-cert','--chdir='+str(STUDY),
    '--output='+str(STUDY/'slurm-%j.out'),'--error='+str(STUDY/'slurm-%j.err'),
    str(snapshot/'certificate_01/certificate_job.sh'),str(STUDY)]
save(STUDY/'submission_command.json',cmd)
ret=subprocess.run(cmd,text=True,capture_output=True)
save(STUDY/'submission_response.json',dict(exit_code=ret.returncode,stdout=ret.stdout,stderr=ret.stderr))
if ret.returncode:
    record(STUDY/'submission_failure_event.json',dict(id='frontier-certificate-submit-failed-20261008',
           status='FAILED',evidence=str(STUDY),issues=ret.stderr,paper_eligible=False))
    raise SystemExit(ret.returncode)
job=ret.stdout.strip().split(';')[0];assert job.isdigit(),ret.stdout
save(STUDY/'jobs.json',dict(job=job,partition='intel',nodes=1,cpus=1,mem='16G',minutes=25))
record(STUDY/'submission_event.json',dict(id='frontier-certificate-submitted-'+job,status='SUBMITTED',
    evidence=str(STUDY),job=job,paper_eligible=False,desktop_write=False,
    issues='No new major issue. No feature computation, performance conclusion, or baseline modification.',
    next='Check oracle mutations, CSR-bound matching certificates and Slurm completion.'))
print(json.dumps(dict(status='SUBMITTED',job=job,run=str(STUDY))))
