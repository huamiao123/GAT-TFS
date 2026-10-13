import pathlib,json,csv,sys,statistics,hashlib,tarfile,subprocess,math
ROOT=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
def write_csv(p,rows):
 if not rows:return
 with p.open('w',newline='') as f:
  w=csv.DictWriter(f,list(dict.fromkeys(k for r in rows for k in r)));w.writeheader();w.writerows(rows)
def times(rows,label):
 groups={}
 for r in rows:
  key=(r['graph'],r.get('shape','128_128_128'),r['method']);groups.setdefault(key,[]).append(float(r['ms']))
 out=[]
 for (graph,shape,method),vs in groups.items():
  assert len(vs)==5
  out.append(dict(graph=graph,shape=shape,method=method,median_ms=statistics.median(vs),min_ms=min(vs),max_ms=max(vs),cv=statistics.pstdev(vs)/statistics.mean(vs)))
 return out
def main():
 d2,sel,out=map(pathlib.Path,sys.argv[1:4]);out.mkdir(parents=True,exist_ok=False);all_runs=[];parts={};raw=[]
 for name,study,modes in [('p3',d2,['smoke','focus']),('p4',sel,['smoke','controlled','focus'])]:
  dest=out/name;dest.mkdir();tables={};runs=[]
  for mode in modes:
   found=[p for p in (study/'logs').glob(mode+'-*') if (p/'completion.json').is_file()];assert len(found)==1,(name,mode)
   r=found[0];comp=json.loads((r/'completion.json').read_text());assert comp['failed']==0;assert (r/'exit_status.txt').read_text().strip()=='0'
   event=json.loads((r/'event.json').read_text());runs.append(dict(mode=mode,path=str(r),node=event['node'],job=event['job'],completion=comp,affinity=json.loads((r/'affinity.json').read_text())))
   for state in json.loads((r/'suite_status.json').read_text()):
    assert state['status']=='COMPLETE';graph=r/state['graph'];parsed=json.loads((graph/'parsed.json').read_text())
    for k,rows in parsed.items():
     enriched=[dict(row,mode=mode) for row in rows];tables.setdefault(k,[]).extend(enriched)
    for file in graph.iterdir():
     if file.is_file() and file.suffix in ['.log','.err','.json','.csv']:raw.append((file,name+'/'+str(file.relative_to(study))))
   for file in ['event.json','completion.json','suite_status.json','affinity.json','node.txt','cpu.txt','numa_policy.txt','preflight.sha256','binary.sha256']:
    if (r/file).exists():raw.append((r/file,name+'/'+str((r/file).relative_to(study))))
  for k,rows in tables.items():write_csv(dest/(k.lower()+'.csv'),rows)
  prefix='D2' if name=='p3' else 'SEL';summary=times(tables[prefix+'_TIME'],name);write_csv(dest/'timing_summary.csv',summary)
  parts[name]=dict(study=str(study),runs=runs,summary=summary,checks=len(tables[prefix+'_CHECK']),rejections=[r for r in tables[prefix+'_ACCEPT'] if r['accepted']=='0'],profile_gates=len(tables.get(prefix+'_PROFILE_GATE',[])))
  all_runs+=runs
  # Actual immutable compilation snapshot and metadata, no binaries or graph data.
  import shutil
  shutil.copytree(study/'source_snapshot',dest/'source_snapshot')
  for file in ['manifest.json','source.sha256']:
   shutil.copyfile(study/file,dest/file)
  (dest/'build').mkdir()
  for file in (study/'build').iterdir():
   if file.is_file() and file.suffix in ['.txt','.json','.log','.sha256']:shutil.copyfile(file,dest/'build'/file.name)
 with tarfile.open(out/'RAW_LOGS.tar.gz','w:gz') as archive:
  for p,arc in raw:archive.add(p,arcname=arc)
 # Validate the enumerated P4 work against plans, including mixed layer2.
 pl={(r['graph'],r['method'],r['mode']):r for r in tables['SEL_PLAN']}
 for w in tables['SEL_WORK']:
  m=w['method'];mode=w['mode'];g=w['graph'];li=w['layer']
  pm='hybrid_none' if m=='fused_full_accurate' else ('hybrid_all' if m=='project_all' else ('project_used' if m=='project_used_L1_full_L2' else m))
  p=pl[g,pm,mode];ns=int(p['selected_sources'])
  is_full=m=='fused_full_accurate' or (m=='project_used_L1_full_L2' and li=='2')
  expected_cold=int(pl[g,'hybrid_none',mode]['cold_projection_tiles']) if is_full else int(p['cold_projection_tiles'])
  expected_cache=0 if is_full else (ns+15)//16
  assert int(w['cold_tiles'])==expected_cold and int(w['cache_tiles'])==expected_cache,w
 # Source hashes are checked against copies, not just self-declared metadata.
 for name in ['p3','p4']:
  p=out/name
  for line in (p/'source.sha256').read_text().splitlines():
   digest,path=line.split(maxsplit=1);actual=p/'source_snapshot'/pathlib.PurePosixPath(path).name
   assert hashlib.sha256(actual.read_bytes()).hexdigest()==digest
 jobs=','.join(str(r['job']) for r in all_runs)
 accounting=subprocess.check_output(['sacct','-j',jobs,'--format=JobID,State,ExitCode,Elapsed,NodeList','-P'],text=True)
 (out/'slurm_accounting.txt').write_text(accounting)
 (out/'summary.json').write_text(json.dumps(parts,indent=2)+'\n')
 lines=['# P3/P4 follow-up measured results','',
 'Shared SPR, one socket/32 physical cores for diagnostic graphs; four for smoke. Default NUMA/no interleave; original compiler flags. Random source-style discrete H/W, unit adjacency. P3 retains original source harness; P4 independent shape harness does not replace paper-compatible TFS/MKL. Cache rebuild enters each P4 layer/forward. Numerical gates are frozen; rejected candidates are never timed. No task accuracy is claimed.','',
 'P3 FP32 checks use original TFS with ACCURATE 1e-3; BF16-final original 1e-2 and source MKL 3e-2. P4 uses quantized-H/W mathematical MKL references with native BF16 interlayer boundary (1e-3 FP32, 1e-2 BF16-final). The timed original-style shape MKL uses FP32 master inputs/weights and no BF16 interlayer boundary; it is a separate anchor, not a precision-matched fusion attribution.','',
 '## P3 diagnostic two-layer medians (ms)','', '| Graph | Original TFS | B64 accurate | S64/FULL accurate | D2 | D3 period4 | FULL/D2 |','|---|---:|---:|---:|---:|---:|---:|']
 t={(r['graph'],r['method']):r for r in parts['p3']['summary']};graphs=sorted({r['graph'] for r in parts['p3']['summary'] if r['graph'] in ['ogbn-products','reddit','mycielskian19','roadNet-CA','wiki-Talk']})
 for g in graphs:
  ms=[float(t[g,m]['median_ms']) if (g,m) in t else None for m in ['paper_tfs','b64_accurate','s64_mfull_accurate','d2_b64_all','d3_b64_period4']]
  lines.append('| '+g+' | '+' | '.join('rejected' if v is None else f'{v:.3f}' for v in ms)+' | '+(f'{ms[2]/ms[3]:.4f}' if ms[3] else 'rejected')+' |')
 lines+=['','## P4 all fixed paths, diagnostic and controlled graphs','', 'Ratio >1 means the candidate is faster than fused FULL ACCURATE. Stage tables separate cache rebuild from sparse reduction/cold AMX projection and merge. Source-consumer preprocessing is outside prepared forward; setup estimates are reported separately, not hidden. Both project_all and project_used and the predefined project_used-L1/FULL-L2 control are retained.','', '| Graph | Shape | Method | Median ms | FULL/Method | CV |','|---|---|---|---:|---:|---:|']
 t4={(r['graph'],r['shape'],r['method']):r for r in parts['p4']['summary']}
 for r in parts['p4']['summary']:
  if r['graph'].startswith('window_'):continue
  base=t4.get((r['graph'],r['shape'],'fused_full_accurate'));ratio=float(base['median_ms'])/float(r['median_ms']) if base else None
  lines.append(f"| {r['graph']} | {r['shape']} | {r['method']} | {r['median_ms']:.3f} | {ratio:.4f} | {r['cv']:.4f} |" if ratio else f"| {r['graph']} | {r['shape']} | {r['method']} | {r['median_ms']:.3f} | unavailable | {r['cv']:.4f} |")
 lines+=['','## Integrity, rejection and interpretation boundaries','',
 f"P3 {parts['p3']['checks']} numerical records, {len(parts['p3']['rejections'])} rejections; P4 {parts['p4']['checks']} numerical records, {len(parts['p4']['rejections'])} rejections. These include aligned smoke, not the superseded first P4 smoke.",
 'Frozen source hashes verified. P4 none/all degeneracy and width128 P0 kernel checks passed; profiled outputs are bitwise checked. Every cold/cache tile count also matches source-plan topology. All raw samples and stdout are archived. See p3/ and p4/ for checks, acceptance, stages, sampled child phases, PMU/per-TID, plans, theoretical feature-request bytes, thread diagnostics, min/max/CV. Sampled child/thread times are not additive wall time; logical bytes are not DRAM traffic. PMU generic misses do not identify a cache level. No global AH is materialized; Q cache sizes and mixed-plan overhead are explicit.',
 'Controlled two-direction bipartite graphs share N=65536, degree64 and E=4194304, with source pools512/32768 per side. They are unit, directed relation operators without self-loops, not necessarily symmetric normalized GCN. They establish an applicable condition, not real-graph generality. No oracle winner is presented as an implemented adaptive policy.']
 (out/'JOURNAL_P3_P4_RESULTS_20261005.md').write_text('\n'.join(lines)+'\n')
 (out/'reconcile_followup.py').write_bytes(pathlib.Path(__file__).read_bytes())
 event=dict(id=out.name,kind='journal_p3_p4_reconciliation',status='COMPLETE',evidence=str(out),paper_eligible=False,summary={k:{'checks':v['checks'],'rejections':v['rejections'],'runs':v['runs']} for k,v in parts.items()},issues='Shared-node conditional shape experiments; no universal accuracy/novelty or additive profile claims',next='Interpret fixed selective fractions against strong project-used and mixed-layer controls')
 (out/'event.json').write_text(json.dumps(event,indent=2)+'\n');subprocess.run([sys.executable,str(ROOT/'scripts/record_event.py'),str(out/'event.json')],check=True)
 (out/'artifact_hashes.json').write_text(json.dumps({p.relative_to(out).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in out.rglob('*') if p.is_file()},indent=2)+'\n')
 print(json.dumps({k:{'checks':v['checks'],'rejections':len(v['rejections'])} for k,v in parts.items()}))
if __name__=='__main__':main()
