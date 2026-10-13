import hashlib
import json
import shutil
import subprocess
from pathlib import Path

ROOT = Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
EXP = ROOT / 'experiments/recompute_audit_20261008/recompute_audit_04'
STUDY = ROOT / 'runs/recompute-audit-20261008-04'
RESULT = ROOT / 'results/recompute_audit_20261008/retry-04'
NAMES = ['PROTOCOL.md', 'recompute_scan.cpp', 'run.py', 'audit_job.sh', 'stage.py', 'frontier_prior_inputs.json']

def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def save(path, value):
    with path.open('x') as f:
        json.dump(value, f, indent=2)
        f.write('\n')

def record(path, value):
    save(path, value)
    subprocess.run(['python3', str(ROOT / 'scripts/record_event.py'), str(path)], check=True)

assert not STUDY.exists() and not RESULT.exists(), 'refuse overwrite'
subprocess.run(['python3', str(ROOT / 'scripts/record_event.py'), 'check'], check=True)
queue = subprocess.check_output(['squeue', '-h', '-u', 'hdacp1', '-o', '%D'], text=True)
assert sum(map(int, queue.split())) + 1 <= 10
STUDY.mkdir(); RESULT.mkdir(); snapshot = STUDY / 'source_snapshot'; snapshot.mkdir()
for name in NAMES:
    shutil.copyfile(EXP / name, snapshot / name)
rules = {
    str(ROOT.parent / 'AGENTS.md'): '55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7',
    str(ROOT.parent / 'skills/tfs-research-engineering/SKILL.md'): '641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca'
}
for path, digest in rules.items():
    assert sha(path) == digest
spec = {name: sha(ROOT.parent / 'TFS-Train/docs/spec' / name) for name in
        ['MATH_SPEC_V2.md', 'KERNEL_LAYOUT_V2.md', 'PYTORCH_EXTENSION_SPEC.md', 'EXPERIMENT_SPEC_V2.md']}
manifest = dict(source_sha256={name: sha(snapshot / name) for name in NAMES},
                authority_sha256=rules, authority_spec_sha256=spec,
                contract_sha256=sha(ROOT / 'IMPLEMENTATION_CONTRACT.md'),
                requested_nodes_this_study=1, user_cap=10,
                numa_interleave=False, desktop_write=False, scope='LOGICAL_RECOMPUTATION_COST')
save(STUDY / 'snapshot_manifest.json', manifest)
record(EXP / 'prepare_event.json', dict(
    id='recompute-audit-prepared-20261008-04', status='PREPARED', evidence=str(STUDY),
    manifest=manifest, paper_eligible=False,
    issues='Retries 01-03 retained; retry04 replaces Python per-edge loop with C++ two-pass scan.',
    next='Compile and run one shared-node read-only CSR scan for four frozen graphs.'))
cmd = ['sbatch', '--parsable', '--partition=intel', '--nodes=1', '--ntasks=1', '--cpus-per-task=1',
       '--mem=16G', '--time=00:25:00', '--job-name=gcn-recompute', '--chdir=' + str(STUDY),
       '--output=' + str(STUDY / 'slurm-%j.out'), '--error=' + str(STUDY / 'slurm-%j.err'),
       str(snapshot / 'audit_job.sh'), str(STUDY)]
save(STUDY / 'submission_command.json', cmd)
response = subprocess.run(cmd, text=True, capture_output=True)
save(STUDY / 'submission_response.json', dict(exit_code=response.returncode,
                                               stdout=response.stdout, stderr=response.stderr))
if response.returncode:
    record(STUDY / 'submission_failure_event.json', dict(
        id='recompute-audit-submit-failed-20261008-04', status='FAILED', evidence=str(STUDY),
        issues=response.stderr, paper_eligible=False))
    raise SystemExit(response.returncode)
job = response.stdout.strip().split(';')[0]
assert job.isdigit(), response.stdout
save(STUDY / 'jobs.json', dict(job=job, partition='intel', nodes=1, cpus=1, mem='16G', minutes=25))
record(STUDY / 'submission_event.json', dict(
    id='recompute-audit-submitted-' + job, status='SUBMITTED', job=job, evidence=str(STUDY),
    paper_eligible=False, desktop_write=False, issues='No feature or performance kernel.',
    next='Check C++ scan completion and compare extra AVX edge work with source-state release.'))
print(json.dumps(dict(status='SUBMITTED', job=job, run=str(STUDY))))
