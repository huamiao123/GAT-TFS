#!/usr/bin/env python3
import csv, pathlib, statistics, sys
root=pathlib.Path(sys.argv[1])
for sub in sorted(root.iterdir()):
 if not sub.is_dir() or not (sub/'timings.csv').exists():continue
 print('\nGRAPH',sub.name)
 try:rows=list(csv.DictReader((sub/'timings.csv').open()))
 except Exception as e:print(e);continue
 for kind in ['kernel','e2e']:
  groups={}
  for r in rows:
   if r.get('kind')==kind and r.get('ms'):groups.setdefault(r['method'],[]).append(float(r['ms']))
  meds={m:statistics.median(v) for m,v in groups.items()};orig=meds.get('original');mkl=meds.get('mkl_fp32')
  for m in ['mkl_fp32','mkl_bf16_inputs','original','shared_b8_fast','shared_b32_fast','shared_bfull_fast','shared_bfull_accurate','replay_bfull_fast']:
   if m in meds:
    v=meds[m];print(kind,m,'median_ms',round(v,4),'n',len(groups[m]),'vs_original',round(orig/v,3) if orig else None,'vs_mkl',round(mkl/v,3) if mkl else None)
 if (sub/'correctness.csv').exists():
  checks=list(csv.DictReader((sub/'correctness.csv').open()));bad=[r for r in checks if r.get('pass')=='0'];print('checked_rows',len(checks),'failed',len(bad))
  for r in bad[:3]:print('FAIL',r)
