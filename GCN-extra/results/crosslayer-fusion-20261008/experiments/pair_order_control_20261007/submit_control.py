#!/usr/bin/env python3
import pathlib,subprocess,json,datetime,sys,hashlib,os
root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
study_path,mode,tag,partition=sys.argv[1:5];study=pathlib.Path(study_path);kind='control'
assert partition in ('intel','intel_expr')
assert study.resolve().is_relative_to(root.resolve()) if hasattr(pathlib.Path,'is_relative_to') else str(study.resolve()).startswith(str(root.resolve())+'/')
assert json.loads((study/'build/event.json').read_text())['status']=='PASS'
if mode!='smoke':
    gates=[json.loads(p.read_text()) for p in (study/'logs').glob('*/completion.json')]
    assert any(g.get('mode')=='smoke' and g.get('failed')==0 and g.get('passed',0)>0 for g in gates),'Successful smoke required'
queue=subprocess.check_output(['squeue','--noheader','--user=hdacp1','--format=%i|%D|%T'],text=True)
requested=sum(int(line.split('|')[1]) for line in queue.splitlines() if line.strip())
assert requested<10,('User node cap10 reached',queue)
subprocess.run(['python3',str(root/'scripts/record_event.py'),'check'],check=True)
stamp=datetime.datetime.now().strftime('%Y%m%d-%H%M%S-%f');folder=study/'launcher'/(tag+'-'+stamp);folder.mkdir(parents=True)
(folder/'queue.txt').write_text(queue)
(folder/'partitions.txt').write_text(subprocess.check_output(['sinfo','-p','intel,intel_expr'],text=True))
for src,name in [(root.parent/'AGENTS.md','AGENTS.read.txt'),(root.parent/'skills/tfs-research-engineering/SKILL.md','SKILL.read.txt'),(root/'IMPLEMENTATION_CONTRACT.md','contract.read.txt')]:
    (folder/name).write_bytes(src.read_bytes())
(folder/'preflight.sha256.json').write_text(json.dumps({n:hashlib.sha256((folder/n).read_bytes()).hexdigest() for n in ['AGENTS.read.txt','SKILL.read.txt','contract.read.txt']},indent=2))
cpus=32
script=folder/'job.slurm';script.write_text('#!/usr/bin/env bash\nset -euo pipefail\nexec srun --ntasks=1 --cpus-per-task='+str(cpus)+' --cpu-bind=cores --distribution=block:block --hint=nomultithread bash '+str(study/'source_snapshot/control_job.sh')+' '+str(study)+' '+mode+'\n')
cmd=['sbatch','--parsable','--partition='+partition,'--nodes=1','--ntasks=1','--cpus-per-task='+str(cpus),'--sockets-per-node=1','--cores-per-socket='+str(cpus),'--threads-per-core=1','--distribution=block:block','--time='+('00:20:00' if mode=='smoke' else '01:30:00'),'--job-name=bal-'+kind[:1]+'-'+tag,'--chdir='+str(root),'--output='+str(folder/'slurm-%j.out'),'--error='+str(folder/'slurm-%j.err')]
if partition=='intel_expr':cmd.append('--exclusive')
cmd.append(str(script));(folder/'command.json').write_text(json.dumps(cmd,indent=2))
job=subprocess.check_output(cmd,text=True).strip();(folder/'job_id.txt').write_text(job+'\n')
e=dict(id=study.name+'-submit-'+job,kind='order_control_submission',status='SUBMITTED',job=job,mode=mode,study=kind,partition=partition,exclusive=partition=='intel_expr',requested_nodes_before=requested,node_cap=10,evidence=str(folder),paper_eligible=False,issues='Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave',next='Validate completion and retain raw paired controls')
(folder/'event.json').write_text(json.dumps(e,indent=2)+'\n');subprocess.run(['python3',str(root/'scripts/record_event.py'),str(folder/'event.json')],check=True)
print(json.dumps(e))
