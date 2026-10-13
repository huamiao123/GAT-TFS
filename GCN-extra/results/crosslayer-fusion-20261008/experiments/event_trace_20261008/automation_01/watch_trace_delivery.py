"""One-shot delivery watcher for the seven already authorized trace jobs.

Does not submit jobs, alter protocols, or retry the final analysis after an
ambiguous transport failure. All data/report creation is guarded remotely.
"""
from pathlib import Path
import argparse
import base64
import datetime as dt
import hashlib
import json
import subprocess
import time

ROOT = '/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra'
EXP = ROOT + '/experiments/event_trace_20261008'
RUN = ROOT + '/runs/event-trace-20261008-01'
RESULT = ROOT + '/results/event_trace_20261008'
JOBS = ['10896223', '10896227', '10896229', '10896230',
        '10896231', '10896233', '10896234']
HOST = 'hdacp1@176.0.250.88'
SSH = ['ssh', '-o', 'BatchMode=yes', '-o', 'ConnectTimeout=12',
       '-o', 'StrictHostKeyChecking=yes', HOST]
HERE = Path(__file__).resolve().parent
STATUS = HERE / 'delivery_watcher_status.json'
DESKTOP_REPORT = Path.home() / 'Desktop' / 'TFS_消费事件计划与分组边界_完整审计_20261008.txt'
FINALIZER_SHA256 = '727949fdc362f5caade82b4dd8dc471dd8fe66d5b61bf002ac2eeaaddfa106e3'


def save_status(state, **fields):
    data = {'state': state, 'updated_at': dt.datetime.now().astimezone().isoformat(),
            'jobs': JOBS, 'run': RUN, 'desktop_report': str(DESKTOP_REPORT), **fields}
    tmp = STATUS.with_suffix('.json.tmp')
    tmp.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    tmp.replace(STATUS)
    print(json.dumps(data, ensure_ascii=False), flush=True)


def remote_py(source, timeout=60):
    data = base64.b64encode(source.encode()).decode('ascii')
    return subprocess.run(SSH + ['echo ' + data + ' | base64 -d | python3'],
                          capture_output=True, text=True, encoding='utf-8',
                          errors='replace', timeout=timeout)


def query():
    cp = remote_py("import subprocess,json\n"
                   "s=subprocess.check_output(['sacct','-n','-X','-P','-j',"
                   + repr(','.join(JOBS)) + ", '--format=JobID,State,ExitCode,Elapsed,NodeList'],text=True)\n"
                   "rows={}\n"
                   "for line in s.splitlines():\n"
                   " p=line.split('|')\n"
                   " if len(p)>=5 and p[0] in " + repr(JOBS) + ": rows[p[0]]=dict(state=p[1],exit_code=p[2],elapsed=p[3],node=p[4])\n"
                   "print(json.dumps(rows))\n")
    if cp.returncode:
        raise RuntimeError('SSH/accounting query failed: ' + cp.stderr[-1200:])
    return json.loads(cp.stdout.strip().splitlines()[-1])


def fetch(remote, local):
    local = Path(local)
    if local.exists():
        raise FileExistsError('Delivery preserves existing file: ' + str(local))
    cp = subprocess.run(['scp', '-o', 'BatchMode=yes', '-o', 'ConnectTimeout=12',
                         '-o', 'StrictHostKeyChecking=yes', HOST + ':' + remote,
                         str(local)], capture_output=True, text=True,
                        encoding='utf-8', errors='replace', timeout=120)
    if cp.returncode:
        raise RuntimeError('SCP failed: ' + cp.stderr[-1200:])
    return hashlib.sha256(local.read_bytes()).hexdigest()


def deliver():
    cp = remote_py("from pathlib import Path\nimport hashlib\n"
                   "p=Path(" + repr(EXP + '/automation_01/finalize_trace.py') + ")\n"
                   "assert hashlib.sha256(p.read_bytes()).hexdigest()==" + repr(FINALIZER_SHA256) + "\n"
                   "print('REVIEWED_FINALIZER_HASH_PASS')\n")
    if cp.returncode:
        raise RuntimeError('Reviewed finalizer hash changed or could not be verified: ' + cp.stderr[-1200:])
    command = ['python3', EXP + '/automation_01/finalize_trace.py', RUN,
               RESULT + '/analysis-01', RESULT + '/final']
    cp = subprocess.run(SSH + [' '.join(command)], capture_output=True,
                        text=True, encoding='utf-8', errors='replace', timeout=1800)
    (HERE / 'finalizer.stdout.txt').write_text(cp.stdout, encoding='utf-8')
    (HERE / 'finalizer.stderr.txt').write_text(cp.stderr, encoding='utf-8')
    if cp.returncode:
        raise RuntimeError('Remote finalizer failed; preserve evidence; no automatic retry: '
                           + str(cp.returncode) + ' ' + cp.stdout[-1600:] + cp.stderr[-800:])
    # Verify the independent audit itself, not only the launcher return code.
    cp = remote_py("from pathlib import Path\nimport json,hashlib\n"
                   "r=Path(" + repr(RESULT) + ")\n"
                   "a=json.loads((r/'analysis-01/audit.json').read_text())\n"
                   "assert a['analysis_pass'] is True and len(a['jobs'])==7\n"
                   "assert {str(j['job']) for j in a['jobs']}==set(" + repr(JOBS) + ")\n"
                   "run=Path(" + repr(RUN) + ").resolve()\n"
                   "assert Path(a['build']['manifest']['root']).resolve()==run\n"
                   "assert all(Path(j['jobdir']).resolve()==run/'logs'/('trace-'+str(j['job'])) and j['valid'] is True for j in a['jobs'])\n"
                   "p=r/'final/TECHNICAL_REPORT_20261008.txt'\n"
                   "assert p.is_file() and p.stat().st_size>1000\n"
                   "manifest=json.loads((r/'final/delivery_manifest.json').read_text())\n"
                   "assert manifest['status']=='SUCCESS' and manifest['analysis_pass'] is True\n"
                   "assert manifest['script_sha256']==" + repr(FINALIZER_SHA256) + "\n"
                   "assert Path(manifest['run']).resolve()==run\n"
                   "assert manifest['report_sha256']==hashlib.sha256(p.read_bytes()).hexdigest()\n"
                   "assert manifest['report_bytes']==p.stat().st_size\n"
                   "print(json.dumps(dict(analysis_pass=True,sha256=hashlib.sha256(p.read_bytes()).hexdigest(),size=p.stat().st_size,expand_full17=a['expansion_gate']['eligible_to_expand_full17'])))\n")
    if cp.returncode:
        raise RuntimeError('Final independent audit/report verification failed: ' + cp.stderr[-1400:])
    checked = json.loads(cp.stdout.strip().splitlines()[-1])
    report_hash = fetch(RESULT + '/final/TECHNICAL_REPORT_20261008.txt', DESKTOP_REPORT)
    if report_hash != checked['sha256']:
        raise RuntimeError('Desktop report hash mismatch; do not claim delivered')
    for name in ['delivery_manifest.json', 'compact_audit.json']:
        fetch(RESULT + '/final/' + name, HERE / ('final_' + name))
    for name in ['expansion_gate.json', 'summary.csv', 'ratios.csv', 'pmu_summary.csv', 'setup.csv', 'structure.csv']:
        fetch(RESULT + '/analysis-01/' + name, HERE / ('final_' + name))
    event = dict(id='event-trace-desktop-delivery-20261008', status='DELIVERED',
                 evidence=RESULT + '/final', desktop_path=str(DESKTOP_REPORT),
                 desktop_sha256=report_hash, paper_eligible=False,
                 issues='Independent seven-job audit passed; desktop bytes match remote report. Automatically aggregated report, with interpretation limits retained.',
                 next='Review full report; follow fixed expansion decision; no new jobs automatically submitted')
    cp = remote_py("from pathlib import Path\nimport json,subprocess\n"
                   "r=Path(" + repr(ROOT) + ")\n"
                   "p=r/'results/event_trace_20261008/final/desktop_delivery_event.json'\n"
                   "assert not p.exists()\n"
                   "p.write_text(json.dumps(" + repr(event) + ",ensure_ascii=False,indent=2)+'\\n')\n"
                   "subprocess.run(['python3',str(r/'scripts/record_event.py'),str(p)],check=True)\n")
    if cp.returncode:
        raise RuntimeError('Desktop copied but handoff synchronization failed: ' + cp.stderr[-1200:])
    save_status('DELIVERED', **checked, report_sha256=report_hash)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--check-once', action='store_true', help='Read-only accounting probe; no status/report writes')
    args = ap.parse_args()
    if args.check_once:
        print(json.dumps(query(), ensure_ascii=False, indent=2))
        return 0
    if STATUS.exists() or DESKTOP_REPORT.exists():
        raise FileExistsError('Watcher status or final report already exists; inspect before restarting')
    deadline = time.monotonic() + 4 * 3600
    try:
        while time.monotonic() < deadline:
            try:
                rows = query()
            except Exception as exc:
                save_status('QUERY_RETRY', error=repr(exc))
                time.sleep(45)
                continue
            bad = {j:r for j,r in rows.items() if r['state'].split()[0].rstrip('+')
                   in {'FAILED','CANCELLED','TIMEOUT','NODE_FAIL','OUT_OF_MEMORY','PREEMPTED','BOOT_FAIL','DEADLINE','REVOKED'}
                   or (r['state']=='COMPLETED' and r['exit_code']!='0:0')}
            if bad:
                save_status('JOB_FAILED', slurm=rows, failed=bad)
                return 2
            ready = len(rows) == len(JOBS) and all(r['state']=='COMPLETED' and r['exit_code']=='0:0' for r in rows.values())
            if ready:
                save_status('FINALIZING', slurm=rows)
                deliver()
                return 0
            save_status('WAITING_FOR_EXISTING_JOBS', slurm=rows)
            time.sleep(45)
        save_status('WAIT_DEADLINE_REACHED', error='No job was cancelled; inspect scheduler and resume delivery explicitly')
        return 3
    except Exception as exc:
        save_status('DELIVERY_FAILED_REQUIRES_REVIEW', error=repr(exc))
        return 4


if __name__ == '__main__':
    raise SystemExit(main())
