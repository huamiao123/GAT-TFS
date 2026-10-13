#!/usr/bin/env python3
"""Independent frozen pair-plan evidence audit. Python 3.8 standard library.
CLI: analyze_plan.py RUN NEW_OUTPUT_DIRECTORY
Reads raw methods.log and a single read-only sacct snapshot; writes only a new
analysis directory. No experimental code, binary or raw evidence is executed or
modified. Source/binary hashes are recomputed, never trusted by pathname alone.
"""
import argparse
import collections
import csv
import hashlib
import json
import math
import os
import pathlib
import re
import shlex
import statistics
import subprocess
import sys
WORKSPACE=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh')
ALL17=('ogbn-products','reddit','mycielskian19','hollywood-2009','indochina-2004',
       'soc-Pokec','cit-Patents','soc-LiveJournal1','com-LiveJournal','as-Skitter',
       'rgg_n_2_24_s0','web-Google','roadNet-CA','wiki-Talk','email-Enron','amazon0601','com-Youtube')
DIAG4=('mycielskian19','reddit','roadNet-CA','amazon0601')
REST13=tuple(g for g in ALL17 if g not in DIAG4)
SYN3=('regular128_reuse1','regular128_reuse4','regular128_reuse16')
GRAPHSETS={'all17':ALL17,'diag4':DIAG4,'rest13':REST13}
ANCHOR='source_mkl_fp32'
KINDS={'consecutive':5,'alternating':10}
MODES=('scalar','dyn_i','dyn_s','exact_i','exact_s')
EQ_BOUNDARIES={'kernel_FP32','e2e_FP32','H1_BF16','e2e_BF16'}
CHECK_PAIRS={('kernel_FP32','paper_tfs'),('e2e_FP32_final','paper_tfs'),
             ('e2e_BF16_final','paper_tfs'),('e2e_BF16_final','source_MKL_FP32')}
INPUT_HASHES={}
HASH_CACHE={}
PMU_METHODS=('pack_epi_degree','legacy_s_degree','opt_dyn_s_degree','opt_exact_s_degree')

def digest(path):
    h = hashlib.sha256()
    with path.open("rb") as f:
        for block in iter(lambda: f.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()

def read_json(path, default=None):
    if not path.is_file():
        return {} if default is None else default
    raw = path.read_bytes()
    INPUT_HASHES[str(path.resolve())] = hashlib.sha256(raw).hexdigest()
    return json.loads(raw.decode("utf-8-sig"))

def read_text(path):
    if not path.is_file():
        return ""
    raw = path.read_bytes()
    INPUT_HASHES[str(path.resolve())] = hashlib.sha256(raw).hexdigest()
    return raw.decode("utf-8", errors="replace")

def as_int(value):
    try:
        return int(value)
    except (TypeError, ValueError):
        return None

def as_float(value):
    try:
        f = float(value)
        return f if math.isfinite(f) else None
    except (TypeError, ValueError):
        return None

def truth(value):
    if value in (True, 1, "1", "true", "True"):
        return True
    if value in (False, 0, "0", "false", "False"):
        return False
    return None

def gm(values):
    values = list(values)
    if not values:
        return None
    if any(v <= 0 or not math.isfinite(v) for v in values):
        raise ValueError("GM requires finite positive values")
    return math.exp(statistics.mean(math.log(v) for v in values))

def write_csv(path, rows, initial_fields):
    fields = list(initial_fields)
    for row in rows:
        for k in row:
            if k not in fields:
                fields.append(k)
    with path.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        w.writerows(rows)

def freeze_check(rows, boundaries, expected_method=None):
    return (len(rows) == len(boundaries)
            and {r.get("boundary") for r in rows} == boundaries
            and all(r.get("bitwise") == "1" and r.get("pass") == "1"
                    and (expected_method is None or r.get("method") == expected_method)
                    for r in rows))

def numerical_ok(rows, accurate, method):
    if len(rows) != 4 or {(r.get("boundary"), r.get("reference")) for r in rows} != CHECK_PAIRS:
        return False
    for r in rows:
        if r.get("reference") == "source_MKL_FP32":
            gate = .03
        elif r.get("boundary") == "e2e_BF16_final":
            gate = .01 if accurate else .02
        else:
            gate = .001 if accurate else .01
        actual_gate = as_float(r.get("gate"))
        l2, nmax = as_float(r.get("relative_L2")), as_float(r.get("normalized_max"))
        ma, mean = as_float(r.get("max_abs")), as_float(r.get("mean_abs"))
        if (r.get("pass") != "1" or actual_gate is None
                or abs(actual_gate - gate) > 1e-12
                or any(v is None or v < 0 for v in (l2, nmax, ma, mean))
                or l2 >= gate or nmax >= gate):
            return False
        if method == "paper_tfs" and r.get("reference") == "paper_tfs" and ma != 0:
            return False
    return True


def hashed(path):
    path=pathlib.Path(path).resolve()
    if str(path) not in HASH_CACHE:
        HASH_CACHE[str(path)]=digest(path)
    INPUT_HASHES[str(path)]=HASH_CACHE[str(path)]
    return HASH_CACHE[str(path)]


def method_matrix(full):
    out=[dict(method='paper_tfs',kind='0',scope='1',schedule='degree',pair='0',plan_index='-1',plan='none',mode='0',accurate='0',epilogue='0'),
         dict(method='pack_epi_degree',kind='3',scope='0',schedule='degree',pair='0',plan_index='-1',plan='none',mode='0',accurate='0',epilogue='1')]
    def legacy(pair):
        out.append(dict(method='legacy_'+('i' if pair==1 else 's')+'_degree',kind='3',scope='0',schedule='degree',pair=str(pair),plan_index='-1',plan='none',mode='0',accurate='0',epilogue='1'))
    def add(prefix,pi,schedule,modes):
        for mode in modes:
            out.append(dict(method=prefix+'_'+MODES[mode]+'_'+schedule,kind='4',scope='0',schedule=schedule,pair='0',plan_index=str(pi),plan=prefix+'_'+schedule,mode=str(mode),accurate='0',epilogue='1'))
    if full: legacy(1)
    legacy(2)
    if full:
        add('adj',0,'degree',range(5));add('opt',1,'degree',range(5));add('rand',2,'degree',(0,2,4))
    else:
        add('adj',0,'degree',(4,));add('opt',1,'degree',(1,2,3,4))
    out.append(dict(method='pack_epi_qguard',kind='3',scope='0',schedule='qguard',pair='0',plan_index='-1',plan='none',mode='0',accurate='0',epilogue='1'))
    add('opt',3,'qguard',range(5) if full else (1,2,3,4))
    return out

FULL_NAMES={m['method'] for m in method_matrix(True)}
LITE_NAMES={m['method'] for m in method_matrix(False)}
DIAG_ONLY=FULL_NAMES-LITE_NAMES


def parse_records(path):
    tables=collections.defaultdict(list)
    errors=[]
    for line_no,line in enumerate(read_text(path).splitlines(),1):
        label,_,body=line.partition(' ')
        if not label.startswith(('METHOD_','PAIRPLAN_','GROUP_','PLAN_GATE')):
            continue
        try:
            record={}
            for token in shlex.split(body):
                if '=' not in token:continue
                key,val=token.split('=',1)
                if key in record:raise ValueError('duplicate key '+key)
                record[key]=val
            tables[label].append(record)
        except Exception as exc:
            errors.append('raw_parse_line_%d:%r'%(line_no,exc))
    return dict(tables),errors


def hash_list(text,root):
    entries={};errors=[]
    for line in text.splitlines():
        if not line.strip():continue
        match=re.match(r'^([0-9a-f]{64})\s+\*?(.+)$',line)
        if not match:errors.append('invalid_hash_line:'+line[:120]);continue
        expected,path=match.groups();f=pathlib.Path(path)
        if not f.is_absolute():f=root/f
        f=f.resolve()
        if str(f) in entries:errors.append('duplicate_hash_path:'+str(f))
        try:
            actual=hashed(f)
            entries[str(f)]=dict(expected=expected,actual=actual,pass_hash=actual==expected)
            if actual!=expected:errors.append('hash_mismatch:'+str(f))
        except Exception as exc:errors.append('unreadable_hash_target:'+str(f)+':'+repr(exc))
    return entries,errors


def audit_build(root):
    manifest=read_json(root/'manifest.json');errors=[]
    def need(ok,msg):
        if not ok:errors.append(msg)
    source_text=read_text(root/'source.sha256')
    need(bool(source_text),'missing_source_hash_list')
    need(manifest.get('source_hashes')==source_text,'manifest_source_hash_list_mismatch')
    sources,ee=hash_list(source_text,root);errors+=ee
    actual_files={str(p.resolve()) for p in (root/'source_snapshot').rglob('*') if p.is_file()}
    need(set(sources)==actual_files and bool(actual_files),'source_snapshot_hash_coverage')
    group_hash=sources.get(str((root/'source_snapshot/grouping_schedule.hpp').resolve()),{}).get('actual')
    need(group_hash=='2dc2a0a978c87959b5523a1ae11e29e74b36e1f447d4f2dbd3dcb62b8d8dab45','frozen_q64_preserving_schedule_source_mismatch')
    binary_text=read_text(root/'build/binary.sha256')
    need(bool(binary_text),'missing_binary_hash_list')
    need(manifest.get('binary_hashes')==binary_text,'manifest_binary_hash_list_mismatch')
    binaries,ee=hash_list(binary_text,root);errors+=ee
    need(set(binaries)=={str((root/'build'/n).resolve()) for n in ('plan_methods','paper_original')},'binary_hash_coverage')
    need(manifest.get('dimensions')==[128,128,128] and manifest.get('numa_interleave') is False and manifest.get('node_cap')==10,'frozen_build_contract')
    evt=read_json(root/'build/event.json')
    need(evt.get('status')=='PASS' and evt.get('exit_status')==0,'build_not_completed_successfully')
    return dict(valid=not errors,errors=errors,manifest=manifest,sources=sources,binaries=binaries,
                source_text=source_text,build_fingerprint=hashlib.sha256(source_text.encode()).hexdigest())


def slurm_snapshot(ids):
    if not ids:return dict(records={},errors=['no_job_ids'],raw='',command=[])
    command=['sacct','--parsable2','--noheader','-j',','.join(sorted(ids)),
             '--format=JobIDRaw,State,ExitCode,NodeList,Partition']
    try:
        p=subprocess.run(command,stdout=subprocess.PIPE,stderr=subprocess.PIPE,
                         universal_newlines=True,timeout=30,check=False)
        rows={}
        for line in p.stdout.splitlines():
            fields=line.rstrip('|').split('|')
            if len(fields)!=5:continue
            jid,state,code,node,partition=fields
            if jid not in ids:continue
            rows[jid]=dict(job=jid,state=state,exit_code=code,node=node,partition=partition)
        return dict(records=rows,errors=[] if p.returncode==0 else ['sacct_exit_'+str(p.returncode)],
                    raw=p.stdout,stderr=p.stderr,command=command,returncode=p.returncode)
    except Exception as exc:
        return dict(records={},errors=['sacct_unavailable:'+repr(exc)],raw='',command=command)


def load_packets(root,build):
    launches={}
    for ep in sorted((root/'launcher').glob('*/event.json')):
        event=read_json(ep);jid=str(event.get('job','')).split(';')[0]
        if jid:launches[jid]=dict(event=event,command=read_json(ep.parent/'command.json',[]),path=str(ep.parent))
    jobdirs={}
    for d in (root/'logs').iterdir():
        if not d.is_dir():continue
        match=re.search(r'(\d+)$',d.name)
        if match:jobdirs[match.group(1)]=d
    ids=set(launches)|set(jobdirs);slurm=slurm_snapshot(ids)
    inventory=read_json(root.parent.parent/'docs/CURRENT_GRAPH_INVENTORY_20261004.json',[])
    expected={v['graph']:v.get('input_sha256') for v in inventory}
    jobs=[];packets=[]
    for jid in sorted(ids,key=lambda j:int(j)):
        launch=launches.get(jid,{})
        d=jobdirs.get(jid,root/'logs'/('plan-'+jid));ev=read_json(d/'event.json')
        env=read_json(d/'environment.json');aff=read_json(d/'affinity.json')
        comp=read_json(d/'completion.json');graphs=read_json(d/'graph_list.json',[])
        se=slurm['records'].get(jid,{})
        le=launch.get('event',{});mode=str(ev.get('mode') or le.get('mode') or comp.get('mode',''))
        partition=str(ev.get('partition') or le.get('partition') or env.get('SLURM_JOB_PARTITION','unknown'))
        exclusive=truth(le.get('exclusive'))
        command=launch.get('command',[])
        if exclusive is None and command:exclusive='--exclusive' in command
        ext='unknown' if exclusive is None else str(exclusive).lower()
        node=str(ev.get('node') or read_text(d/'node.txt').strip() or se.get('node',''))
        terminal=se.get('state','').split()[0].split('+')[0]
        pending=terminal in ('PENDING','RUNNING','CONFIGURING','COMPLETING','SUSPENDED','RESIZING') or not se
        errors=[]
        def req(ok,msg):
            if not ok:errors.append(msg)
        if not pending:
            req(terminal=='COMPLETED' and se.get('exit_code')=='0:0','slurm_not_COMPLETED_0:0')
            req(ev.get('status')=='PASS' and ev.get('exit_status')==0,'job_event_exit_failed_or_missing')
            req(read_text(d/'exit_status.txt').strip()=='0','job_exit_status_not_zero')
            req(comp.get('failed')==0 and comp.get('passed')==comp.get('graphs') and comp.get('graphs',0)>0,'job_completion_missing_or_failed')
        req(bool(launch),'launcher_evidence_missing')
        req(ext!='unknown','allocation_exclusivity_unknown')
        if env:
            topo=aff.get('topology',[])
            req(aff.get('threads')==32 and len(aff.get('cpus',[]))==32 and len(topo)==32 and
                len({r.get('socket') for r in topo})==1 and len({(r.get('socket'),r.get('core')) for r in topo})==32,'32_physical_cores_one_socket')
            req({r.get('cpu') for r in topo}==set(aff.get('cpus',[])),'affinity_cpu_topology_mismatch')
            req(all(env.get(k)==v for k,v in {'OMP_NUM_THREADS':'32','OMP_PROC_BIND':'close','OMP_PLACES':'cores'}.items()),'OpenMP_binding_mismatch')
            req(all(env.get(k) in (None,'') for k in ('MKL_NUM_THREADS','MKL_DYNAMIC','MKL_DOMAIN_NUM_THREADS')),'MKL_override')
            req(bool(re.search(r'^policy:\s*default\s*$',read_text(d/'numa_policy.txt'),re.M)),'NUMA_policy_not_default')
            req(read_text(d/'source.sha256')==build['source_text'],'job_source_hash_list_mismatch')
            entries,ee=hash_list(read_text(d/'build_files.sha256'),root);errors+=ee
            for binary in build['binaries']:
                req(binary in entries and entries[binary]['expected']==build['binaries'][binary]['actual'],'job_binary_stamp_missing_or_changed:'+binary)
            req(str(env.get('SLURM_JOB_ID'))==jid,'job_environment_ID_mismatch')
            if se:
                req(se.get('node')==node and se.get('partition')==partition,'Slurm_node_or_partition_mismatch')
        elif not pending:errors.append('job_environment_missing')
        job=dict(job=jid,jobdir=str(d),node=node,partition=partition,exclusive=ext,
                 mode=mode,pending=pending,valid=not pending and not errors,errors=errors,
                 completion=comp,event=ev,slurm=se,launcher=launch,environment=env,affinity=aff)
        if graphs and comp and not pending:req(comp.get('graphs')==len(graphs),'job_graph_completion_count_mismatch')
        job['valid']=not pending and not errors
        jobs.append(job)
        folders={x['graph']:dict(x) for x in graphs}
        for folder in d.iterdir() if d.is_dir() else []:
            if folder.is_dir() and any((folder/n).exists() for n in ('methods.log','status.json','parsed.json')):
                folders.setdefault(folder.name,dict(graph=folder.name))
        if not folders and mode not in ('smoke','synthetic',''):
            folders={g:dict(graph=g) for g in mode.split(',')}
        for graph,item in sorted(folders.items()):
            folder=d/graph;parsed,parse_errors=parse_records(folder/'methods.log')
            status=read_json(folder/'status.json')
            data_class='smoke' if mode=='smoke' else ('real' if graph in ALL17 else ('synthetic' if graph in SYN3 else 'unknown'))
            packets.append(dict(root=str(root),folder=str(folder),jobdir=str(d),job=jid,node=node,
                partition=partition,exclusive=ext,mode=mode,data_class=data_class,graph=graph,
                graphset='diag4' if graph in DIAG4 else ('rest13' if graph in REST13 else data_class),
                build_fingerprint=build['build_fingerprint'],job_status='PENDING' if pending else ('PASS' if job['valid'] else 'FAILED'),
                job_context=job,parsed=parsed,parse_errors=parse_errors,status=status,graph_item=item,
                expected_input_sha256=expected.get(graph),input_sha256=status.get('actual_input_sha256',''),
                build=build))
    groups=collections.defaultdict(list)
    for p in packets:groups[(p['partition'],p['exclusive'],p['data_class'],p['graph'],p['build_fingerprint'])].append(p)
    for group in groups.values():
        group.sort(key=lambda p:(int(p['job']),p['folder']))
        for i,p in enumerate(group):
            declared_retest='retest-' in pathlib.Path(p['job_context']['launcher'].get('path','')).name
            p['attempt_role']='retest' if declared_retest or i else 'primary'
    return packets,jobs,slurm


def context(p):
    return {k:p[k] for k in ('root','folder','job','node','partition','exclusive','attempt_role','data_class','graphset','graph','build_fingerprint','input_sha256','job_status')}


def extra_methods(prefix):
    return {'%s_m%d_a%d_p%d_v%d'%(prefix,sc,ac,pi,md):dict(method='%s_m%d_a%d_p%d_v%d'%(prefix,sc,ac,pi,md),
            kind='4',scope=str(sc),accurate=str(ac),plan_index=str(pi),plan=('adj','opt','rand')[pi]+'_degree',mode=str(md),schedule='degree',pair='0',epilogue='1')
            for sc in (64,256,0) for ac in (0,1) for pi in range(3) for md in range(5)}


DEGREE_CACHE={}
def degree_work(path):
    """Read only CSR rowptr, then analytically count original ascending-degree tiles."""
    import array,struct
    path=pathlib.Path(path).resolve()
    if str(path) in DEGREE_CACHE:return DEGREE_CACHE[str(path)]
    h=hashlib.sha256();hist=collections.Counter();previous=None;read_count=0
    with path.open('rb') as f:
        header=f.read(36);h.update(header)
        if len(header)!=36:raise ValueError('short CSR header')
        _,_,_,N,columns,E=struct.unpack('<IIIQQQ',header)
        if N!=columns:raise ValueError('non-square graph')
        remaining=N+1
        while remaining:
            raw=f.read(min(remaining,262144)*4)
            if not raw or len(raw)%4:raise ValueError('short CSR rowptr')
            h.update(raw);rows=array.array('I');rows.frombytes(raw)
            if sys.byteorder!='little':rows.byteswap()
            for v in rows:
                if previous is None:
                    if v!=0:raise ValueError('CSR rowptr does not start at zero')
                else:
                    if v<previous:raise ValueError('decreasing CSR rowptr')
                    hist[v-previous]+=1
                previous=v
            count=len(rows);read_count+=count;remaining-=count
        if previous!=E:raise ValueError('CSR rowptr edge count mismatch')
    results={}
    for scope in (0,64,256):
        row_scopes=sum(count*((d+scope-1)//scope if scope else int(d>0)) for d,count in hist.items())
        row_windows=sum(count*((d+63)//64) for d,count in hist.items())
        scopes=0;direct=0;pos=0;tile_windows=0
        for d,count in sorted(hist.items()):
            end=pos+count;full_ends=end//16-pos//16
            q=(d+scope-1)//scope if scope else int(d>0)
            scopes+=full_ends*q;direct+=full_ends*q*16;tile_windows+=full_ends*((d+63)//64)
            pos=end
        if N%16:
            d=max(hist) if hist else 0;q=(d+scope-1)//scope if scope else int(d>0)
            scopes+=q;direct+=q*(N%16);tile_windows+=(d+63)//64
        results[str(scope)]=dict(edges=E,scopes=scopes,windows=tile_windows,
            loads=(row_windows-row_scopes)*512,stores=(row_windows-row_scopes)*512,
            direct=direct,packed=scopes*4096,prefetch=(E-row_scopes)*4,
            row_scopes=row_scopes,nonempty_rows=N-hist.get(0,0))
    result=dict(N=N,E=E,rowptr_sha256=h.hexdigest(),rowptr_bytes=36+(N+1)*4,
                distinct_degrees=len(hist),work=results,source=str(path))
    DEGREE_CACHE[str(path)]=result
    return result


def audit_packet(p):
    t=p['parsed'];errors=list(p['parse_errors']);warnings=[];j=p['job_context'];folder=pathlib.Path(p['folder'])
    def req(ok,msg):
        if not ok:errors.append(msg)
    # A running job remains pending even when one graph has complete timings.
    # No provisional timing enters performance or a complete-cohort GM.
    if j['pending']:
        failures=[(k,r) for k,rr in t.items() for r in rr if r.get('pass')=='0']
        return dict(**context(p),valid=False,pending=True,errors=errors+(['observed_failed_gate'] if failures else []),
                    warnings=['job_final_completion_pending'],raw_records=t,graph_status=p['status'],methods={})
    errors+=j['errors'];errors+=p['build']['errors']
    req(p['status'].get('status')=='PASS','graph_status_not_PASS')
    configs=t.get('PAIRPLAN_CONFIG',[]);cfg=configs[0] if len(configs)==1 else {}
    req(len(configs)==1,'config_cardinality')
    req(all(cfg.get(k)==v for k,v in {'D':'128','H':'128','F':'128','S':'64','R':'64','TR':'16',
        'warmups':'1','consecutive':'5','alternating':'10','numa':'default','final_output':'BF16'}.items()),'frozen_config_mismatch')
    smoke=p['data_class']=='smoke';full=smoke or p['data_class']=='synthetic' or p['graph'] in DIAG4
    expected=method_matrix(full);specs={m['method']:m for m in expected};names=set(specs)
    declared=t.get('PAIRPLAN_PLAN',[])
    req(len(declared)==len(expected) and [m.get('method') for m in declared]==[m['method'] for m in expected],'candidate_matrix_or_order_mismatch')
    req(cfg.get('methods')==str(len(expected)) and cfg.get('smoke')==str(int(smoke)) and cfg.get('ablations')==str(int(full)),'matrix_config_mismatch')
    for row in declared:
        target=specs.get(row.get('method'),{})
        req(bool(target) and all(row.get(k)==v for k,v in target.items()),'method_contract:'+row.get('method','MISSING'))
    for label in ('PAIRPLAN_COMPLETE','PAIRPLAN_GATE_COMPLETE','PAIRPLAN_SEMANTIC'):
        rr=t.get(label,[]);req(len(rr)==1 and rr[0].get('pass')=='1',label+'_missing_duplicate_or_failed')
    done=t.get('PAIRPLAN_COMPLETE',[])
    req(len(done)==1 and done[0].get('methods')==cfg.get('methods') and done[0].get('smoke')==cfg.get('smoke'),'completion_config_mismatch')
    sem=t.get('PAIRPLAN_SEMANTIC',[])
    req(len(sem)==1 and sem[0].get('bitwise')=='1' and sem[0].get('values')=='19','19_value_semantic_coverage')
    for label,rr in t.items():
        req(all(r.get('graph',p['graph'])==p['graph'] for r in rr),'cross_graph_record:'+label)
        req(all(r.get('pass','1')!='0' for r in rr),'failed_gate:'+label)
    es=read_json(folder/'experiment_env.json')
    req(es=={'PAIRPLAN_SMOKE':str(int(smoke)),'PAIRPLAN_ABLATIONS':str(int(full)),
             'PAIRPLAN_PMU':str(int(full and not smoke))},'experiment_environment_mismatch')
    stamps=read_json(folder/'binary_input_hashes.json');cmd=read_json(folder/'methods.command.json')
    proc=read_json(folder/'methods.status.json');binary=str((pathlib.Path(p['root'])/'build/plan_methods').resolve())
    req(proc.get('returncode')==0 and not proc.get('timed_out',False),'methods_process_not_successful')
    req(cmd.get('argv',[None])[0] is not None and str(pathlib.Path(cmd['argv'][0]).resolve())==binary and cmd.get('numa_policy') is None,'method_invocation_binary_or_NUMA_mismatch')
    req(stamps.get('binary_sha256')==p['build']['binaries'].get(binary,{}).get('actual') and
        str(pathlib.Path(stamps.get('binary','.')).resolve())==binary,'per_graph_binary_hash_mismatch')
    req(bool(p['input_sha256']) and stamps.get('input_sha256')==p['input_sha256'],'per_graph_input_hash_missing_or_mismatch')
    if p['expected_input_sha256']:req(p['input_sha256']==p['expected_input_sha256'],'inventory_input_hash_mismatch')
    graph_path=p['graph_item'].get('path') or stamps.get('input')
    req(bool(graph_path) and str(pathlib.Path(graph_path).resolve())==str(pathlib.Path(stamps.get('input','.')).resolve()),'graph_path_mismatch')
    independent=degree_work(graph_path) if graph_path else {'N':None,'E':None,'work':{}}
    N,E=as_int(cfg.get('N')),as_int(cfg.get('E'))
    req(N==independent['N'] and E==independent['E'],'CSR_header_config_mismatch')
    if N is None or E is None:raise ValueError('missing integer N/E')
    if not smoke:
        ar=read_json(folder/'original_raw.status.json');ac=read_json(folder/'original_raw.command.json')
        req(ar.get('returncode')==0 and ac.get('argv',[None])[0] is not None and pathlib.Path(ac['argv'][0]).resolve()==pathlib.Path(p['root'])/'build/paper_original','original_anchor_process_failed')
        anchor=p['status'].get('anchor',{})
        req(set(anchor)=={'paper_tfs','source_mkl_fp32'} and all(len(v.get('raw_ms',[]))==5 and all(as_float(x) is not None and x>0 for x in v['raw_ms']) for v in anchor.values()),'raw_original_anchor_missing')
    groups=t.get('GROUP_AUDIT',[]);gmap={r.get('schedule'):r for r in groups}
    req(len(groups)==len(gmap)==5 and set(gmap)=={'degree','qsource','qpage','qsamplepage','qguard'},'frozen_schedule_audit_matrix')
    for schedule,r in gmap.items():
        req(r.get('pass')=='1' and as_int(r.get('rows'))==N,'group_schedule_gate:'+schedule)
        req(as_int(r.get('full_projections'))==independent['work']['0']['scopes'] and
            as_int(r.get('b64_projections'))==independent['work']['64']['scopes'] and
            as_int(r.get('b256_projections'))==independent['work']['256']['scopes'] and
            as_int(r.get('windows'))==independent['work']['0']['windows'],'group_projection_budget:'+schedule)
    plans_expected={'adj_degree':0,'opt_degree':1,'opt_qguard':1}
    if full:plans_expected['rand_degree']=2
    structs=t.get('PAIRPLAN_STRUCTURE',[]);smap={r.get('plan'):r for r in structs}
    setups=t.get('PAIRPLAN_SETUP',[]);setupmap={r.get('plan'):r for r in setups}
    gates=t.get('PLAN_GATE',[]);gatemap={r.get('plan'):r for r in gates}
    for label,rr,lookup in [('structure',structs,smap),('setup',setups,setupmap),('gate',gates,gatemap)]:
        req(len(rr)==len(lookup)==len(plans_expected) and set(lookup)==set(plans_expected),'plan_'+label+'_matrix')
    for name,pk in plans_expected.items():
        s,u,g=smap.get(name,{}),setupmap.get(name,{}),gatemap.get(name,{})
        hits,exact,steps,comp=[as_int(s.get(k)) for k in ('dynamic_shared_heads','exact_shared_edges','merge_steps','comparisons')]
        req(s.get('kind')==str(pk) and as_int(s.get('source_edges'))==E and as_int(s.get('tiles'))==(N+15)//16,'plan_structure_shape:'+name)
        req(None not in (hits,exact,steps,comp) and 0<=exact<=hits<=E//2 and steps==E-hits and 0<=comp<=steps,'plan_structure_hits:'+name)
        req(as_int(s.get('neighbor_windows'))==independent['work']['0']['windows'] and
            as_int(s.get('full_projections'))==independent['work']['0']['scopes'] and
            as_int(s.get('b64_projections'))==independent['work']['64']['scopes'] and
            as_int(s.get('b256_projections'))==independent['work']['256']['scopes'],'plan_projection_budget:'+name)
        req(g.get('pass')=='1' and g.get('invalid')=='0' and all(g.get(k)=='1' for k in ('bijection','masks_independent','projection_budget_unchanged')) and g.get('feature_first_touch_changed')=='0' and as_int(g.get('source_edges'))==E and as_int(g.get('expected_edges'))==E,'plan_gate:'+name)
        req(all(as_float(u.get(k)) is not None and as_float(u.get(k))>=0 for k in ('layout_ms','pairing_ms','verify_ms','audit_ms','total_ms')),'plan_setup_time:'+name)
        req(u.get('uses_timing')=='0' and u.get('construction_includes_full_audit')=='1' and u.get('seed')=='20261007','plan_setup_policy:'+name)
        req(as_int(u.get('merge_index_reads'))==E and as_int(u.get('signature_probes')) is not None and as_int(u.get('signature_probes'))<=192*N and as_int(u.get('score_pairs')) is not None and as_int(u.get('score_pairs'))<=64*((N+15)//16),'plan_setup_probe_budget:'+name)
        req(as_int(u.get('sample_edge_probes'))==(as_int(u.get('signature_probes')) or 0)+(as_int(u.get('score_probes')) or 0) and
            as_int(u.get('full_verification_reads'))==sum(as_int(u.get(k)) or 0 for k in ('merge_index_reads','exact_compare_reads','memcmp_requested_reads')),'plan_setup_probe_accounting:'+name)
        req(as_int(u.get('plan_bytes')) is not None and as_int(u.get('plan_bytes'))>=16*((N+15)//16)+(as_int(s.get('exact_masks_bytes')) or 0) and
            as_int(u.get('allocated_bytes')) is not None and as_int(u.get('allocated_bytes'))>=as_int(u.get('plan_bytes')),'plan_retained_bytes:'+name)
        if pk==1:req(as_int(s.get('selected_sample_score')) is not None and as_int(s.get('adjacent_sample_score')) is not None and as_int(s.get('selected_sample_score'))>=as_int(s.get('adjacent_sample_score')),'greedy_sample_proxy_regression:'+name)
    if smoke:
        coverage=t.get('PAIRPLAN_COVERAGE',[]);req(len(coverage)==1,'exact_fixture_coverage_cardinality')
        cov=coverage[0] if len(coverage)==1 else {}
        need='positive' if p['graph'] in ('plan_exact_tail','plan_exact_unsorteddup') else ('zero' if p['graph']=='plan_exact_mismatch' else 'unconstrained')
        c=as_int(cov.get('adj_exact_shared_edges'))
        req(cov.get('pass')=='1' and cov.get('required')==need and c==as_int(smap.get('adj_degree',{}).get('exact_shared_edges')) and (need!='positive' or c is not None and c>0) and (need!='zero' or c==0),'exact_fixture_coverage')
    extras=extra_methods('smoke') if smoke else {};stress=extra_methods('stress') if smoke else {}
    all_numeric=dict(specs,**extras);all_eq={n:m for n,m in specs.items() if n!='paper_tfs'};all_eq.update(extras);all_eq.update(stress)
    profiles={n:m for n,m in specs.items() if n!='paper_tfs'};profiles.update(extras)
    req({r.get('method') for r in t.get('METHOD_CHECK',[])}==set(all_numeric),'numerical_method_coverage')
    req({r.get('method') for r in t.get('PAIRPLAN_EQUIV',[])}==set(all_eq),'equivalence_method_coverage')
    for label in ('PAIRPLAN_COUNTERS','PAIRPLAN_WORK_GATE','PAIRPLAN_PROFILE_GATE'):
        req({r.get('method') for r in t.get(label,[])}==set(profiles),'profile_method_coverage:'+label)
    for name,m in all_numeric.items():
        req(numerical_ok([r for r in t.get('METHOD_CHECK',[]) if r.get('method')==name],m['accurate']=='1',name),'four_numerical_gates:'+name)
    for name,m in all_eq.items():
        rr=[r for r in t.get('PAIRPLAN_EQUIV',[]) if r.get('method')==name]
        req(freeze_check(rr,EQ_BOUNDARIES,name) and all(r.get('scope')==m['scope'] and r.get('accurate')==m['accurate'] for r in rr),'four_bitwise_boundaries:'+name)
    for name,m in profiles.items():
        wr=independent['work'][m['scope']];accurate=m['accurate']=='1';md=int(m['mode']);pk=int(m['kind']);pair=int(m['pair'])
        plan=smap.get(m['plan'],{}) if pk==4 else smap.get('adj_degree',{})
        hits=(as_int(plan.get('dynamic_shared_heads')) or 0) if (pk==4 and md in (1,2) or pk==3 and pair) else ((as_int(plan.get('exact_shared_edges')) or 0) if pk==4 and md in (3,4) else 0)
        decode=E-hits if pk==4 and md in (2,4) or pk==3 and pair==2 else E
        csr=E-hits if pk==4 and md==4 else E
        for label in ('PAIRPLAN_COUNTERS','PAIRPLAN_WORK_GATE','PAIRPLAN_PROFILE_GATE'):
            rr=[r for r in t.get(label,[]) if r.get('method')==name]
            req(len(rr)==2 and {r.get('layer') for r in rr}=={'1','2'},'two_layers:'+label+':'+name)
            if label!='PAIRPLAN_COUNTERS':req(all(r.get('pass')=='1' and (label!='PAIRPLAN_PROFILE_GATE' or r.get('bitwise')=='1') for r in rr),'work_or_profile_gate:'+name)
        for c in [r for r in t.get('PAIRPLAN_COUNTERS',[]) if r.get('method')==name]:
            desired=dict(source_visits=E,feature_adds=E*128,decode_rows=decode,shared_heads=hits,
                projection_scopes=wr['scopes'],windows=wr['windows'],amx_tdp=wr['scopes']*32*(2 if accurate else 1),
                partial_zero_bytes=0,partial_load_bytes=wr['loads'],partial_store_bytes=wr['stores'],
                packing_load_bytes=0,direct_rows=wr['direct'],packed_bytes=wr['packed']*(2 if accurate else 1),
                prefetch_requests=wr['prefetch'],prefetch_index_reads=wr['prefetch']//4,csr_reads=csr,
                relu_elements=N*128 if c.get('layer')=='1' else 0)
            req(all(as_int(c.get(k))==v for k,v in desired.items()),'independent_work_counter:'+name+':L'+c.get('layer','?'))
            req(c.get('scope')==m['scope'] and c.get('accurate')==m['accurate'] and c.get('profile_path')=='production_BF16_epilogue','counter_context:'+name)
            if pk==4 and md>=3:
                req(as_int(c.get('exact_pair_edges'))==hits and as_int(c.get('exact_shared_edges'))==(hits if md==4 else 0) and as_int(c.get('runtime_head_comparisons'))==0 and as_int(c.get('comparisons'))==0,'exact_path_removed_head_comparisons:'+name)
            if pk==4 and md in (1,2):req(as_int(c.get('comparisons'))==as_int(plan.get('comparisons')),'dynamic_comparison_count:'+name)
    pmu_audit=[]
    if full and not smoke:
        pmu=t.get('METHOD_PMU',[]);walls=t.get('PAIRPLAN_PMU_WALL',[]);threads=t.get('METHOD_PMU_THREAD',[])
        expected_keys={(n,str(r)) for n in PMU_METHODS for r in range(3)}
        req(len(pmu)==12 and {(r.get('method'),r.get('repeat')) for r in pmu}==expected_keys,'PMU_summary_matrix_missing')
        req(len(walls)==12 and {(r.get('method'),r.get('repeat')) for r in walls}==expected_keys,'PMU_wall_matrix_missing')
        for rec in pmu:
            rr=[r for r in threads if (r.get('method'),r.get('repeat'))==(rec.get('method'),rec.get('repeat'))]
            available=rec.get('available')=='1'
            usable=available and rec.get('coverage_complete')=='1' and rec.get('scaled_complete')=='1' and all(as_int(rec.get(k))==32 for k in ('initial_threads','final_threads','counted_threads')) and all(as_int(rec.get(k))==0 for k in ('failed_threads','zero_running_threads','new_threads','disappeared_threads')) and len(rr)==32 and len({r.get('tid') for r in rr})==32 and all(r.get('available')=='1' and r.get('scaled_valid')=='1' and as_int(r.get('time_running_ns')) is not None and as_int(r.get('time_running_ns'))>0 for r in rr)
            pmu_audit.append(dict(method=rec.get('method'),repeat=rec.get('repeat'),usable=usable,available=available,thread_records=len(rr),reason='' if usable else 'missing_or_incomplete_counter_coverage_not_performance_failure'))
        if not all(r['usable'] for r in pmu_audit):warnings.append('PMU_not_fully_usable_retained_explicitly')
    elif t.get('METHOD_PMU') or t.get('PAIRPLAN_PMU_WALL'):errors.append('unexpected_PMU_outside_declared_scope')
    return dict(**context(p),valid=not errors,pending=False,errors=errors,warnings=warnings,
        methods=specs,raw_records=t,independent_CSR_work=independent,pmu_audit=pmu_audit,
        graph_status=p['status'],process_status=proc,job_completion=j['completion'],binary_input_stamps=stamps,
        counts=dict(numerical=len(t.get('METHOD_CHECK',[])),bitwise=len(t.get('PAIRPLAN_EQUIV',[])),
                    work=len(t.get('PAIRPLAN_WORK_GATE',[])),profile=len(t.get('PAIRPLAN_PROFILE_GATE',[]))))



def comparison_pairs(names):
    pairs=set()
    def put(candidate,reference,mechanism):
        if candidate in names and reference in names and candidate!=reference:pairs.add((candidate,reference,mechanism))
    for schedule in ('degree','qguard'):
        for mode in MODES:put('opt_'+mode+'_'+schedule,'pack_epi_'+schedule,'new_vs_strong_baseline')
    for prefix in ('adj','opt','rand'):
        for schedule in ('degree','qguard'):
            put(prefix+'_dyn_s_'+schedule,prefix+'_dyn_i_'+schedule,'dynamic_shared_decode')
            put(prefix+'_exact_s_'+schedule,prefix+'_exact_i_'+schedule,'static_shared_decode')
    for mode in MODES:put('opt_'+mode+'_degree','adj_'+mode+'_degree','pair_assignment_same_kernel')
    for mode in ('scalar','dyn_s','exact_s'):put('opt_'+mode+'_degree','rand_'+mode+'_degree','pair_assignment_vs_random')
    put('adj_dyn_i_degree','legacy_i_degree','plan_table_overhead_dynamic_interleave')
    put('adj_dyn_s_degree','legacy_s_degree','plan_table_overhead_dynamic_share')
    return sorted(pairs)


def summarize(packets,audits):
    summaries=[];ratios=[];paired=[];lookup={};amap={a['folder']:a for a in audits}
    for p in packets:
        audit=amap[p['folder']];t=p['parsed'];full=p['data_class'] in ('smoke','synthetic') or p['graph'] in DIAG4
        spec=method_matrix(full);plans={m['method']:m for m in spec};positions={m['method']:i for i,m in enumerate(spec)};positions[ANCHOR]=len(spec)
        by=collections.defaultdict(list)
        for r in t.get('PAIRPLAN_TIME',[]):by[(r.get('method',''),r.get('kind',''))].append(r)
        if p['data_class']=='smoke':
            if by:audit['errors'].append('smoke_unexpected_timing');audit['valid']=False
            continue
        keys={(n,k) for n in set(plans)|{ANCHOR} for k in KINDS}
        if not audit['pending'] and set(by)!=keys:audit['errors'].append('timing_matrix_missing_or_extra');audit['valid']=False
        for method,kind in sorted(keys|set(by)):
            rr=by.get((method,kind),[]);vals=[as_float(r.get('ms')) for r in rr];repeats=[as_int(r.get('repeat')) for r in rr]
            count=KINDS.get(kind,-1)
            valid=len(rr)==count and set(repeats)==set(range(max(0,count))) and all(v is not None and v>0 for v in vals)
            position=positions.get(method)
            valid=valid and position is not None and all(as_int(r.get('order'))==
                    (len(spec)-position if kind=='alternating' and int(r['repeat'])%2 else position) for r in rr)
            if not valid and not audit['pending']:audit['errors'].append('timing_invalid:'+method+':'+kind);audit['valid']=False
            finite=[v for v in vals if v is not None and v>0];m=plans.get(method,{})
            row=dict(**context(p),method=method,timing_kind=kind,schedule=m.get('schedule','anchor'),scope=m.get('scope',''),mode=m.get('mode',''),
                plan=m.get('plan','none'),precision='FP32_master_input_interlayer' if method==ANCHOR else 'BF16_input_output_FAST',
                samples=len(rr),timing_valid=int(valid),eligible_for_performance=0,
                median_ms=statistics.median(finite) if finite else None,min_ms=min(finite) if finite else None,
                max_ms=max(finite) if finite else None,cv=statistics.pstdev(finite)/statistics.mean(finite) if finite else None,
                graph_audit_pass=0,reason='',pending=int(audit['pending']))
            summaries.append(row);lookup[(p['folder'],method,kind)]=(row,rr)
    for row in summaries:
        a=amap[row['folder']];row['graph_audit_pass']=int(a['valid']);row['eligible_for_performance']=int(a['valid'] and row['timing_valid'])
        row['reason']=';'.join(a['errors']) if not a['pending'] else 'pending_job_completion'
    for p in packets:
        if p['data_class']=='smoke':continue
        full=p['data_class']=='synthetic' or p['graph'] in DIAG4
        names={m['method'] for m in method_matrix(full)}
        for candidate,reference,mechanism in comparison_pairs(names):
            for kind in KINDS:
                ca,cr=lookup[(p['folder'],candidate,kind)];re,rr=lookup[(p['folder'],reference,kind)]
                eligible=bool(ca['eligible_for_performance'] and re['eligible_for_performance'])
                ratio=re['median_ms']/ca['median_ms'] if eligible else None
                row=dict(**context(p),method=candidate,reference=reference,mechanism=mechanism,timing_kind=kind,
                    eligible=int(eligible),precision_matched=1,same_scope=int(ca['scope']==re['scope']),
                    candidate_schedule=ca['schedule'],reference_schedule=re['schedule'],candidate_median_ms=ca['median_ms'],
                    reference_median_ms=re['median_ms'],candidate_min_ms=ca['min_ms'],candidate_max_ms=ca['max_ms'],
                    reference_min_ms=re['min_ms'],reference_max_ms=re['max_ms'],speedup_ratio_of_medians=ratio,
                    alternating_repeat_gm_diagnostic=None,denominator_same_graph_job_protocol=1,losses_retained=1)
                if eligible and kind=='alternating':
                    c={int(r['repeat']):float(r['ms']) for r in cr};r={int(v['repeat']):float(v['ms']) for v in rr};values=[]
                    for rep in range(10):
                        value=r[rep]/c[rep];values.append(value)
                        paired.append(dict(**context(p),method=candidate,reference=reference,repeat=rep,
                            pair=rep//2,orientation='reverse' if rep%2 else 'forward',candidate_ms=c[rep],reference_ms=r[rep],speedup=value,diagnostic_only=1))
                    row['alternating_repeat_gm_diagnostic']=gm(values)
                ratios.append(row)
    return summaries,ratios,paired


def cohort_summaries(ratios,audits):
    grouped=collections.defaultdict(list)
    for r in ratios:
        if r['eligible'] and r['data_class'] in ('real','synthetic'):
            key=tuple(r[k] for k in ('partition','exclusive','attempt_role','data_class','build_fingerprint','timing_kind','method','reference','mechanism'))+(r['job'] if r['data_class']=='synthetic' else 'graph_equal',)
            grouped[(key,r['graph'])].append(r)
    collapsed={}
    for (key,graph),rr in grouped.items():
        collapsed[(key,graph)]=dict(speedup=gm(r['speedup_ratio_of_medians'] for r in rr),
            jobs=sorted({r['job'] for r in rr}),observations=len(rr))
    comparisons=[]
    for key in sorted({k for k,_ in grouped}):
        partition,exclusive,role,dc,build,kind,method,reference,mechanism,job_cohort=key
        if dc=='synthetic':sets={'synthetic3':SYN3}
        elif method in DIAG_ONLY or reference in DIAG_ONLY:sets={'diag4':DIAG4}
        else:sets=GRAPHSETS
        for setname,target in sets.items():
            present=sorted(g for g in target if (key,g) in collapsed);missing=sorted(set(target)-set(present))
            values=[collapsed[(key,g)]['speedup'] for g in present]
            comparisons.append(dict(partition=partition,exclusive=exclusive,attempt_role=role,data_class=dc,
                build_fingerprint=build,timing_kind=kind,method=method,reference=reference,mechanism=mechanism,
                graphset=setname,job_cohort=job_cohort,expected_graph_count=len(target),graph_count=len(present),graphs=present,missing_graphs=missing,
                complete_graphset=not missing,gm_complete_graphset=gm(values) if not missing else None,
                gm_available_graphs=gm(values),available_GM_is_partial=bool(missing),
                comparison_has_diagnostic_only_method=method in DIAG_ONLY or reference in DIAG_ONLY,
                wins=sum(v>1 for v in values),losses=sum(v<1 for v in values),ties=sum(v==1 for v in values),
                per_graph={g:collapsed[(key,g)] for g in present}))
    coverage=[]
    for partition,exclusive,role,dc,build,job_cohort in sorted({tuple(a[k] for k in ('partition','exclusive','attempt_role','data_class','build_fingerprint'))+(a['job'] if a['data_class']=='synthetic' else 'graph_equal',) for a in audits}):
        relevant=[a for a in audits if tuple(a[k] for k in ('partition','exclusive','attempt_role','data_class','build_fingerprint'))==(partition,exclusive,role,dc,build) and (dc!='synthetic' or a['job']==job_cohort)]
        sets=GRAPHSETS if dc=='real' else ({'synthetic3':SYN3} if dc=='synthetic' else {})
        for setname,target in sets.items():
            passed=sorted({a['graph'] for a in relevant if a['valid'] and a['graph'] in target})
            pending=sorted({a['graph'] for a in relevant if a['pending'] and a['graph'] in target})
            failed=sorted({a['graph'] for a in relevant if not a['valid'] and not a['pending'] and a['graph'] in target})
            coverage.append(dict(partition=partition,exclusive=exclusive,attempt_role=role,data_class=dc,build_fingerprint=build,
                graphset=setname,job_cohort=job_cohort,expected_graph_count=len(target),eligible_graphs=passed,pending_graphs=pending,failed_graphs=failed,
                missing_graphs=sorted(set(target)-set(passed)),complete=len(passed)==len(target)))
    return dict(comparisons=comparisons,coverage=coverage,
        estimator='GM of same-job ratios of medians; repeated job observations first collapsed within graph; equal weight per graph.',
        diagnostic_only_methods=sorted(DIAG_ONLY),no_best_candidate_selection=True,
        protocols_kept_separate=True,real_synthetic_kept_separate=True,shared_exclusive_kept_separate=True)


def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('runPath',type=pathlib.Path);ap.add_argument('outputDir',type=pathlib.Path)
    args=ap.parse_args();root=args.runPath.resolve();out=args.outputDir.resolve();ws=WORKSPACE.resolve()
    if not root.is_dir():ap.error('RUN must exist')
    if os.path.commonpath([str(ws),str(out)])!=str(ws) or out==ws:ap.error('NEW_OUTPUT must be a new directory under wzh')
    if out.exists():ap.error('NEW_OUTPUT exists; analyses never overwrite previous evidence')
    build=audit_build(root);packets,jobs,slurm=load_packets(root,build)
    audits=[]
    for p in packets:
        try:audits.append(audit_packet(p))
        except Exception as exc:
            audits.append(dict(**context(p),valid=False,pending=p['job_context']['pending'],errors=['audit_exception:'+repr(exc)],warnings=[],
                               methods={},raw_records=p['parsed'],graph_status=p['status']))
    rows,ratios,paired=summarize(packets,audits);cohorts=cohort_summaries(ratios,audits)
    packet_map={a['folder']:a for a in audits}
    for j in jobs:
        if j['pending']:continue
        jj=[a for a in audits if a['job']==j['job']]
        if len(jj)!=j['completion'].get('graphs'):j['errors'].append('graph_packets_completion_count_mismatch')
        if j['mode']=='smoke':
            expected={'window_tail','window_high_tail','window_scope_tail'}|{'state_tail_'+str(n) for n in (1,15,16,17,31,33,65)}|{'plan_exact_tail','plan_exact_unsorteddup','plan_exact_mismatch'}
            if {a['graph'] for a in jj}!=expected:j['errors'].append('smoke_13_fixture_coverage_missing')
        if j['mode']=='synthetic' and {a['graph'] for a in jj}!=set(SYN3):j['errors'].append('synthetic3_coverage_missing')
        if j['completion']:
            for field,label in [('checks','METHOD_CHECK'),('equiv','PAIRPLAN_EQUIV'),('profile','PAIRPLAN_PROFILE_GATE'),('work','PAIRPLAN_WORK_GATE')]:
                actual=sum(len(a.get('raw_records',{}).get(label,[])) for a in jj)
                if j['completion'].get(field)!=actual:j['errors'].append('completion_'+field+'_count_mismatch')
        j['valid']=not j['errors']
    # Final job-level errors invalidate all its observations, including a missing
    # fixture elsewhere in the same suite. Do this before final cohorts.
    badjobs={j['job'] for j in jobs if not j['valid'] and not j['pending']}
    if badjobs:
        for a in audits:
            if a['job'] in badjobs:a['valid']=False;a['errors'].append('job_level_audit_failed')
        for row in rows:
            if row['job'] in badjobs:row['eligible_for_performance']=0;row['graph_audit_pass']=0;row['reason']=';'.join(packet_map[row['folder']]['errors'])
        for row in ratios:
            if row['job'] in badjobs:row['eligible']=0;row['speedup_ratio_of_medians']=None;row['alternating_repeat_gm_diagnostic']=None
        cohorts=cohort_summaries(ratios,audits)
    hashed(pathlib.Path(__file__))
    analysis_pass=bool(packets) and build['valid'] and all(j['valid'] for j in jobs) and all(a['valid'] for a in audits)
    audit=dict(run=str(root),analysis_source=str(pathlib.Path(__file__).resolve()),analysis_pass=analysis_pass,
        build_audit=build,slurm_snapshot=slurm,job_audits=jobs,graph_audits=audits,
        graph_job_packets=len(audits),passed_graph_jobs=sum(a['valid'] for a in audits),
        failed_graph_jobs=sum(not a['valid'] and not a['pending'] for a in audits),pending_graph_jobs=sum(a['pending'] for a in audits),
        pending_jobs=[j['job'] for j in jobs if j['pending']],performance_records=sum(r['eligible_for_performance'] for r in rows),
        alternating_repeat_diagnostics=paired,input_sha256=INPUT_HASHES,
        interpretation=[
            'All input data derives from immutable raw logs; suite parsed.json is not an audit authority.',
            'A single read-only sacct snapshot verifies COMPLETED 0:0, node and partition. Pending jobs never enter performance.',
            'Source and binary SHA-256 values are recomputed; per-job and per-graph hash stamps must agree.',
            'CSR header/rowptr are read independently. Degree histograms reconstruct ascending DegreeSort tile budgets.',
            'Frozen grouping_schedule.hpp plus every GROUP_AUDIT pass establishes preserved per-slot q64 before reusing work budgets.',
            'Profile counters use the actual BF16 production epilogue for both layers, including first-layer ReLU.',
            'Source-level decode/CSR/prefetch/state counters are logical work, not retired instructions or DRAM bytes.',
            'PMU records require all 32 threads and valid scaling to be usable. Missing PMU coverage is explicitly retained.',
            'The FP32 MKL anchor is separately timed and excluded from precision-matched speedup ratios and GM.',
            'Ratios use only predeclared mechanism comparisons within one graph/job/protocol; negative values are retained.',
            'Consecutive and alternating protocols, real and synthetic graphs, shared and exclusive nodes, and primary/retests remain separate.',
            'Diagnostic-only methods receive only diag4 or synthetic3 cohorts, never an all17 headline.',
            'Static-plan construction, its independent full audit and retained bytes are preserved outside primary inference timing.',
            'No per-graph oracle winner is selected; complete GM requires all named graphs and equal weight per graph.',
            'Caller must run analysis preflight and synchronize the three project handoff documents.'])
    out.mkdir(parents=True,exist_ok=False)
    write_csv(out/'summary.csv',rows,['graph','job','partition','exclusive','attempt_role','method','timing_kind','eligible_for_performance','median_ms','min_ms','max_ms'])
    write_csv(out/'ratios.csv',ratios,['graph','job','partition','exclusive','attempt_role','method','reference','mechanism','timing_kind','eligible','speedup_ratio_of_medians'])
    (out/'audit.json').write_text(json.dumps(audit,indent=2,ensure_ascii=False,allow_nan=False)+'\n',encoding='utf-8')
    (out/'cohort_summary.json').write_text(json.dumps(cohorts,indent=2,ensure_ascii=False,allow_nan=False)+'\n',encoding='utf-8')
    print(json.dumps(dict(output=str(out),analysis_pass=analysis_pass,passed=audit['passed_graph_jobs'],failed=audit['failed_graph_jobs'],
                         pending=audit['pending_graph_jobs'],pending_jobs=audit['pending_jobs'],performance_records=audit['performance_records']),ensure_ascii=False))
    return 0 if analysis_pass else (3 if audit['pending_jobs'] and not audit['failed_graph_jobs'] and build['valid'] else 2)

if __name__=='__main__':sys.exit(main())
