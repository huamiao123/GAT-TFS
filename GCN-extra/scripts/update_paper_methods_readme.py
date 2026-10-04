#!/usr/bin/env python3
import json,pathlib
root=pathlib.Path('/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra')
main=json.loads((root/'runs/paper-method-reconciled-20261004/audit_summary.json').read_text())
cache=json.loads((root/'runs/paper-cache-reconciled-20261004/audit_summary.json').read_text())
assert main['graphs']==cache['graphs']==17
old='The reduction extension has not yet been compared with the paper v3 baseline.\nIt must preserve that baseline\'s output precision in the next comparison.'
p=root/'README.md';text=p.read_text();assert text.count(old)==1
fixed={x['method']:x for x in cache['fixed']}
new=['## Controlled method remeasurement (2026-10-04)','',
'The neighbor-reduction extension now has a controlled comparison against the',
'paper v3 baseline: 17 eligible real graphs, all 14 B/precision candidates,',
'1020/1020 numerical checks, and 2720 measured repetitions. A second all-17',
'execution-order control keeps the exact frozen kernels and gates, with an',
'immediate warmup plus five consecutive E2E repetitions per method; it adds',
'612/612 passing checks and 850 measurements. Both protocols are retained.',
'Same graph/process/source buffers, random H/W, DegreeSort, 128->128->128,',
'32 physical threads, original MKL helpers/hint, default NUMA policy and BF16',
'final TFS output are used. A speedup never mixes times from different nodes.',
'Original MKL remains FP32 exactly as in the paper; no NUMA optimization was added.','',
'All-17 geometric means below use median-of-five consecutive measurements and',
'their same-process original baselines. Interleaved comparisons remain in the',
'reports; do not substitute one protocol\'s denominator into the other.','',
'| Fixed method | vs paper TFS | vs source MKL | Faster than TFS |',
'|---|---:|---:|---:|']
for m in ['b8_fast','b16_fast','b64_fast','b64_accurate','bfull_fast','bfull_accurate']:
 x=fixed[m];new.append(f"| {m} | {x['consecutive_gmean_vs_tfs']:.3f}x | {x['consecutive_gmean_vs_mkl']:.3f}x | {x['wins_vs_tfs']}/17 |")
new+=['','Reports:',
'- `docs/PAPER_METHODS_RESULTS_20261004.md`: full seven-block-size sweep, raw timings and gates.',
'- `docs/PAPER_CACHE_CONTROL_RESULTS_20261004.md`: consecutive/interleaved order control.',
'- `docs/PAPER_METHODS_INTERPRETATION_20261004.md`: actual source dataflow, stage timings, modeled AMX counts and limits.',
'- `docs/figures/paper_methods_20261004/`: standalone PNG/PDF figures and repeat variability CSV.','',
'Latest controlled sources: `src/paper_methods_runtime.hpp`,',
'`scripts/generate_paper_methods.py` and the frozen complete generated source',
'in `runs/paper-method-build-20261004-123445/source_snapshot/`.',
'Reconciled evidence: `runs/paper-method-reconciled-20261004` and',
'`runs/paper-cache-reconciled-20261004`. The source/binary/log archive is',
'`evidence-paper-methods-controlled-20261004.tar.gz` with SHA256 manifest.','',
'No full global AH is materialized in the fused method. DegreeSort and AMX',
'tile-aware fusion remain. FULL is not universally fastest: Mycielskian19',
'favors bounded blocks; low-degree losses remain visible. There is no deployed',
'adaptive selector. Sampled phase timing is diagnostic, not additive wall time',
'or PMU proof. This is prepared two-layer computational inference on the',
'original random H/W, not checkpoint classification accuracy.',
'Six nonunit graphs and two original-LP64-incompatible shapes retain their',
'explicit exclusions; datasets and operator semantics were not altered.']
p.write_text(text.replace(old,'\n'.join(new)))
print('README_UPDATED_WITH_BOTH_PROTOCOLS')
