import hashlib,json,shutil,subprocess
from pathlib import Path
ROOT=Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
EXP=ROOT/'experiments/recompute_audit_20261008/recompute_audit_02'; STUDY=ROOT/'runs/recompute-audit-20261008-02'; RESULT=ROOT/'results/recompute_audit_20261008/recompute_audit_02'
NAMES=['PROTOCOL.md','recompute_audit.py','audit_job.sh','stage.py','frontier_prior_inputs.json']
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def save(p,o):
    with p.open('x') as f:json.dump(o,f,indent=2);f.write('\n')
def rec(p,o):save(p,o);subprocess.run(['python3',str(ROOT/'scripts/record_event.py'),str(p)],check=True)
assert not STUDY.exists() and not RESULT.exists(),'refuse overwrite'
subprocess.run(['python3',str(ROOT/'scripts/record_event.py'),'check'],check=True)
q=subprocess.check_output(['squeue','-h','-u','hdacp1','-o','%D'],text=True);assert sum(map(int,q.split()))+1<=10
STUDY.mkdir();RESULT.mkdir();snap=STUDY/'source_snapshot';snap.mkdir()
for n in NAMES:shutil.copyfile(EXP/n,snap/n)
rules={str(ROOT.parent/'AGENTS.md'):'55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7',str(ROOT.parent/'skills/tfs-research-engineering/SKILL.md'):'641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca'}
for p,d in rules.items():assert sha(p)==d
spec={n:sha(ROOT.parent/'TFS-Train/docs/spec'/n) for n in ['MATH_SPEC_V2.md','KERNEL_LAYOUT_V2.md','PYTORCH_EXTENSION_SPEC.md','EXPERIMENT_SPEC_V2.md']}
manifest=dict(source_sha256={n:sha(snap/n) for n in NAMES},authority_sha256=rules,authority_spec_sha256=spec,contract_sha256=sha(ROOT/'IMPLEMENTATION_CONTRACT.md'),requested_nodes_this_study=1,user_cap=10,numa_interleave=False,desktop_write=False,scope='LOGICAL_RECOMPUTATION_COST')
save(STUDY/'snapshot_manifest.json',manifest)
rec(EXP/'prepare_event.json',dict(id='recompute-audit-prepared-20261008',status='PREPARED',evidence=str(STUDY),manifest=manifest,paper_eligible=False,issues='No new major issue; logical cost audit only.',next='One shared-node read-only CSR scan for four frozen graphs.'))
cmd=['sbatch','--parsable','--partition=intel','--nodes=1','--ntasks=1','--cpus-per-task=1','--mem=16G','--time=00:25:00','--job-name=gcn-recompute','--chdir='+str(STUDY),'--output='+str(STUDY/'slurm-%j.out'),'--error='+str(STUDY/'slurm-%j.err'),str(snap/'audit_job.sh'),str(STUDY)]
save(STUDY/'submission_command.json',cmd);ret=subprocess.run(cmd,text=True,capture_output=True);save(STUDY/'submission_response.json',dict(exit_code=ret.returncode,stdout=ret.stdout,stderr=ret.stderr))
if ret.returncode:
    rec(STUDY/'submission_failure_event.json',dict(id='recompute-audit-submit-failed-20261008',status='FAILED',evidence=str(STUDY),issues=ret.stderr,paper_eligible=False));raise SystemExit(ret.returncode)
job=ret.stdout.strip().split(';')[0];assert job.isdigit()
save(STUDY/'jobs.json',dict(job=job,partition='intel',nodes=1,cpus=1,mem='16G',minutes=25))
rec(STUDY/'submission_event.json',dict(id='recompute-audit-submitted-'+job,status='SUBMITTED',job=job,evidence=str(STUDY),paper_eligible=False,desktop_write=False,issues='No feature or performance kernel.',next='Check completion and compare logical extra edge work with source-state release.'))
print(json.dumps(dict(status='SUBMITTED',job=job,run=str(STUDY))))
