"""Import and verify published evidence; derive statistics without rerunning kernels."""
import collections
import csv
import hashlib
import json
import pathlib
import shlex
import statistics
import sys
import tarfile


def sha(p):
    h=hashlib.sha256()
    with p.open('rb') as f:
        for b in iter(lambda:f.read(1<<20),b''):h.update(b)
    return h.hexdigest()


def write_csv(p,rows):
    if not rows:return
    with p.open('w',newline='',encoding='utf-8') as f:
        w=csv.DictWriter(f,list(dict.fromkeys(k for r in rows for k in r)));w.writeheader();w.writerows(rows)


archive,root=map(pathlib.Path,sys.argv[1:3])
assert sha(archive)=='6e4d6157984522212a45bae0f7f896477fb345c114adf7faa4eb26123c972606'
root.mkdir(exist_ok=False)
with tarfile.open(archive) as a:
    for m in a.getmembers():
        assert not pathlib.PurePosixPath(m.name).is_absolute() and '..' not in pathlib.PurePosixPath(m.name).parts
        assert m.isfile() or m.isdir()
    a.extractall(root,filter='data')
inventory=json.loads((root/'SERVER_ARTIFACT_HASHES.json').read_text(encoding='utf-8'))
for path,digest in inventory.items():assert sha(root/path)==digest,path

latest=root/'runs/recompute-window-adaptive-20261008-234255/program.log'
raw=[];results=[];plans={};graph=None
for line in latest.read_text(encoding='utf-8').splitlines():
    label,_,body=line.partition(' ')
    if label not in ['PLAN','ADAPT_RUN','ADAPT_RESULT']:continue
    row=dict(token.split('=',1) for token in shlex.split(body) if '=' in token)
    if label=='PLAN':graph=row['graph'];plans[graph]=row
    elif label=='ADAPT_RUN':raw.append(dict(graph=graph,**row))
    else:results.append(row)
assert len(raw)==18 and len(results)==3
stats=[]
for r in results:
    samples=[s for s in raw if s['graph']==r['graph']];assert len(samples)==6 and r['exact']=='1'
    bv=[float(s['baseline_ms']) for s in samples];cv=[float(s['candidate_ms']) for s in samples]
    fallback=plans[r['graph']]['mode']=='materialized_full'
    if fallback:assert bv==cv
    stats.append(dict(graph=r['graph'],rho_w=r['rho'],mode=r['mode'],repeats=6,
        baseline_standard_median_ms=statistics.median(bv),candidate_standard_median_ms=statistics.median(cv),
        standard_median_speedup=statistics.median(bv)/statistics.median(cv),
        baseline_original_upper_median_ms=r['baseline_median_ms'],candidate_original_upper_median_ms=r['candidate_median_ms'],
        original_upper_median_speedup=float(r['baseline_median_ms'])/float(r['candidate_median_ms']),
        baseline_min_ms=min(bv),baseline_max_ms=max(bv),candidate_min_ms=min(cv),candidate_max_ms=max(cv),
        baseline_cv=statistics.pstdev(bv)/statistics.mean(bv),candidate_cv=statistics.pstdev(cv)/statistics.mean(cv),
        independently_timed_candidate=int(not fallback),preprocessing_in_timed_region=0,
        exact_final_bf16=r['exact'],source_rows=r['source_rows'],source_projection_groups=r['source_projection_groups'],
        paper_eligible=0,raw_run='recompute-window-adaptive-20261008-234255'))
write_csv(root/'adaptive_raw_samples.csv',raw);write_csv(root/'adaptive_recomputed_statistics.csv',stats)

with (root/'excluded_files.csv').open(encoding='utf-8',newline='') as f:excluded=list(csv.DictReader(f))
with (root/'original_manifest_verification.csv').open(encoding='utf-8',newline='') as f:checks=list(csv.DictReader(f))
publication=dict(archive_sha256=sha(archive),server_inventory_files_verified=len(inventory),
    original_hash_status=dict(collections.Counter(r['status'] for r in checks)),
    original_hash_changes=[r for r in checks if r['status']!='MATCH'],
    excluded_suffix_counts=dict(collections.Counter(pathlib.Path(r['path']).suffix for r in excluded)),
    correction_scope='post-hoc publication explanation/statistics only; original code and measurements unchanged')
(root/'REVIEW_AUDIT.json').write_text(json.dumps(publication,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
(root/'README.md').write_text('''# October 7/8 GCN cross-layer fusion experiments

Start with [the review and corrections](../../docs/CROSSLAYER_FUSION_REVIEW_20261013.md)
and [the original latest server handoff](handoff_snapshot/CURRENT_HANDOFF.md).

## Main implementation and result

- [Latest frozen adaptive C++](runs/recompute-window-adaptive-build-20261008-234221/source_snapshot/recompute_window_adaptive.cpp)
- [Source cache and H1 generation](runs/recompute-window-adaptive-build-20261008-234221/source_snapshot/recompute_window_kernel.cpp)
- [Adaptive launch/protocol](experiments/recompute_window_adaptive_20261008/)
- [Raw latest adaptive log](runs/recompute-window-adaptive-20261008-234255/program.log)
- [Recomputed ordinary medians](adaptive_recomputed_statistics.csv) and [all 18 samples](adaptive_raw_samples.csv)

## Complete history

- [All 19 experiment source/protocol directories](experiments/)
- [All 65 run/build directories](runs/), including failure and superseded records
- [Run inventory](run_inventory.json), [Slurm accounting](slurm_accounting.txt)
- [Published file origins and SHA256](exported_files.csv), [original manifest checks](original_manifest_verification.csv)
- [Dependency snapshot](reference_dependencies/), [handoff history](handoff_snapshot/), [manual update scripts](handoff_updaters/)
- [Excluded datasets/executables and other non-text artifacts](excluded_files.csv)

This is a publication of existing exploratory evidence, with no new compile
or benchmark. Unit-weight graphs, seeded random H/W and two layers128->128->128.
The latest adaptive denominator is same-order materialized FULL FAST, not
original TFS/MKL/FULL ACCURATE. All paper claims remain unverified.

The original source/log/status bytes are preserved. The reported six-sample
upper median differs from the conventional median; both are retained. Fallback
candidate time aliases baseline time, and selector/order preprocessing is outside
the timed region. These limitations are explicit in the review and derived CSV.
Two early source manifests referenced subsequently modified live prototype
files; their old expected and present hashes remain visible. Latest adaptive
source and executable hashes match. Original binaries/datasets stay on server.

`SERVER_ARTIFACT_HASHES.json` covers the exact server export before derived
publication tables. `PUBLICATION_HASHES.json` additionally covers review aids.
Frozen include/compile paths retain the original server layout; dependency
snapshots must be placed at their original relative positions for replay.
''',encoding='utf-8')
print(json.dumps(publication,ensure_ascii=False,indent=2))
