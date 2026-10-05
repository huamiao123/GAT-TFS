"""Keep the initial P4 implementation smoke, outside all main result tables."""
import hashlib
import json
import pathlib
import shutil
import subprocess
import sys
import tarfile

study, out = map(pathlib.Path, sys.argv[1:3])
out.mkdir(parents=True, exist_ok=False)
shutil.copytree(study/'source_snapshot', out/'source_snapshot')
for file in ['manifest.json', 'source.sha256']:
    shutil.copyfile(study/file, out/file)
(out/'build').mkdir()
for file in (study/'build').iterdir():
    if file.is_file() and file.suffix in ['.json', '.txt', '.log', '.sha256']:
        shutil.copyfile(file, out/'build'/file.name)
run = study/'logs/smoke-10869522'
assert json.loads((run/'completion.json').read_text())['failed'] == 0
assert (run/'exit_status.txt').read_text().strip() == '0'
with tarfile.open(out/'RAW_LOGS.tar.gz', 'w:gz') as archive:
    archive.add(run, arcname='logs/smoke-10869522')
for file in ['event.json', 'completion.json', 'suite_status.json', 'affinity.json', 'binary.sha256']:
    shutil.copyfile(run/file, out/file)
(out/'slurm_accounting.txt').write_text(subprocess.check_output(
    ['sacct','-j','10869522','--format=JobID,State,ExitCode,Elapsed,NodeList','-P'],text=True))
(out/'README.md').write_text(
    '# Superseded P4 implementation smoke\n\n'
    'Job 10869522 passed 144 numerical checks on three small fixtures. '
    'It used continuous small random H/W and eight paths. Before any real graph '
    'measurement, the generator was aligned to the original discrete rand()%200 '
    'distribution and the mixed-layer strong control was added, then rebuilt '
    'to a different immutable snapshot. This smoke is retained for provenance '
    'only; it is excluded from all main result counts, speedup tables and '
    'performance conclusions. See p4_control_alignment.json and the handoffs.\n')
for line in (out/'source.sha256').read_text().splitlines():
    digest, path = line.split(maxsplit=1)
    actual=out/'source_snapshot'/pathlib.PurePosixPath(path).name
    assert hashlib.sha256(actual.read_bytes()).hexdigest() == digest
(out/'artifact_hashes.json').write_text(json.dumps({
    p.relative_to(out).as_posix():hashlib.sha256(p.read_bytes()).hexdigest()
    for p in out.rglob('*') if p.is_file()},indent=2)+'\n')
print('PRESERVED superseded smoke 10869522')
