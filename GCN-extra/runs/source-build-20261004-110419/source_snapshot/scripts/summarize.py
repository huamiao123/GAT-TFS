#!/usr/bin/env python3
import csv, json, pathlib, statistics, sys
root=pathlib.Path(sys.argv[1]);lines=['# GCN-extra first inference experiments','',
 'Numerical sum-aggregation experiments with random H/W; no task-accuracy claim.',
 'Uninstrumented medians; original source is unchanged. Speedups are original/new and FP32 MKL/new.',
 'Profiles are sampled summed thread time and do not equal wall time. Physical AMX work is modeled, not a hardware counter.','']
summary=[]
for sub in sorted(root.iterdir()):
 if not sub.is_dir() or not (sub/'timings.csv').exists():continue
 rows=list(csv.DictReader((sub/'timings.csv').open()));checks=list(csv.DictReader((sub/'correctness.csv').open()))
 work={r['method']:r for r in csv.DictReader((sub/'work.csv').open())};info=json.loads((sub/'info.json').read_text()) if (sub/'info.json').exists() else {}
 lines += ['## '+sub.name,'',f"N={info.get('N')}, E={info.get('E')}, threads={info.get('threads')}, status={info.get('status','INCOMPLETE')}",'']
 for kind in ['kernel','e2e']:
  groups={}
  for r in rows:
   if r['kind']==kind:groups.setdefault(r['method'],[]).append(float(r['ms']))
  if not groups:continue
  med={m:statistics.median(v) for m,v in groups.items()};base=med.get('original');mkl=med.get('mkl_fp32')
  lines += [f'### {kind}','','| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |','|---|---:|---:|---:|---:|---:|---:|']
  for m,ms in sorted(med.items(),key=lambda p:p[1]):
   v=groups[m];quart=statistics.quantiles(v,n=4,method='inclusive') if len(v)>1 else [v[0]]*3
   lines.append(f'| {m} | {ms:.3f} | {min(v):.3f} | {quart[0]:.3f} | {quart[2]:.3f} | {(base/ms if base else float("nan")):.3f}x | {(mkl/ms if mkl else float("nan")):.3f}x |')
   summary.append({'graph':info.get('graph',sub.name),'kind':kind,'method':m,'median_ms':ms,'min_ms':min(v),'speedup_original':base/ms if base else None,'speedup_mkl_fp32':mkl/ms if mkl else None,'work':work.get(m)})
  lines.append('')
 lines += ['### Numerical errors','','| method | boundary | reference | rel L2 | max abs | normalized max | pass |','|---|---|---|---:|---:|---:|---|']
 for r in checks:
  if r['boundary'] in ['kernel_full','two_layer_e2e','instrumented_kernel','b1_identity']:
   lines.append('| '+ ' | '.join([r['method'],r['boundary'],r['reference'],f"{float(r['relative_l2']):.3e}",f"{float(r['max_abs']):.3e}",f"{float(r['normalized_max']):.3e}",r['pass']])+' |')
 lines.append('')
 profile=list(csv.DictReader((sub/'profiles.csv').open()));lines += ['### Sampled phase profile','','| method | phase | sampled thread ms | sampled tiles | profile wall ms |','|---|---|---:|---:|---:|']
 for r in profile:
  if r['method'] in ['original','shared_b8_fast','shared_bfull_fast','shared_bfull_accurate','replay_bfull_fast']:
   lines.append(f"| {r['method']} | {r['phase']} | {float(r['sampled_thread_ms']):.4f} | {r['sampled_tiles']} | {float(r['profile_wall_ms']):.3f} |")
 lines.append('')
(root/'SUMMARY.md').write_text('\n'.join(lines)+'\n');(root/'summary.json').write_text(json.dumps(summary,indent=2))
print(root/'SUMMARY.md')
