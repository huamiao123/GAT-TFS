# Controlled neighbor-reduction results against paper TFS v3 (2026-10-04)

## Controls and scope

17 real, unit-valued graphs; same CSR and source-seeded H/W, DegreeSort, original node order, TR=16, R=64, 128->128->128. Every graph compares all methods on one node and process. The three graph batches use disjoint exclusive nodes; speedups never mix nodes. All TFS methods have FP32 layer1, FP32 ReLU, original BF16 interlayer conversion and direct BF16 layer2 output. No NUMA/interleave policy, MKL helper/hint/allocation change, or replacement data. Original MKL remains FP32 as in the paper. Fixed variables and gates are in `PAPER_METHODS_PROTOCOL_20261004.md`.

Existing neighbor reduction was retained; only output store typing and the paper-v3 harness were adapted. FAST uses one truncated BF16 partial; ACCURATE uses hi plus truncated residual lo. Every partial is projected immediately by AMX in a destination tile, with no global AH. Original paper v3 source is byte-recoverable after removing its include/hook, and an untouched independently compiled binary is retained as an anchor.

## Complete evidence

- Build: `runs/paper-method-build-20261004-123445`.
- Smoke 10864873: two empty-row/tail/group-boundary fixtures, 120 checks, PASS.
- Formal batches: paper-method-high-10864874 on qhcn818, paper-method-medium-10864875 on qhcn817, paper-method-low-10864876 on qhcn818.
- Formal checks: 1020/1020 PASS; 2720 measured repetitions.
- Reconciled evidence: `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-reconciled-20261004`.
- Source and binary hashes, input full SHA256, exact argv, environment, graph exit statuses, Slurm accounting, raw times and profiles are retained.

## Primary fixed-method results

All-17 geometric means, median of five with one warmup. Faster than original TFS means speedup >1. No slow graphs are removed.

| Method | vs paper TFS | vs source MKL | TFS wins /17 | MKL wins /17 |
|---|---:|---:|---:|---:|
| b2_fast | 0.729x | 1.381x | 0 | 14 |
| b2_accurate | 0.585x | 1.108x | 0 | 10 |
| b4_fast | 0.938x | 1.778x | 4 | 15 |
| b4_accurate | 0.813x | 1.541x | 2 | 14 |
| b8_fast | 1.110x | 2.105x | 9 | 17 |
| b8_accurate | 1.014x | 1.921x | 7 | 16 |
| b16_fast | 1.223x | 2.318x | 10 | 17 |
| b16_accurate | 1.148x | 2.176x | 9 | 17 |
| b32_fast | 1.280x | 2.426x | 11 | 17 |
| b32_accurate | 1.226x | 2.325x | 10 | 17 |
| b64_fast | 1.301x | 2.466x | 11 | 17 |
| b64_accurate | 1.261x | 2.391x | 11 | 17 |
| bfull_fast | 1.251x | 2.372x | 10 | 17 |
| bfull_accurate | 1.219x | 2.311x | 10 | 17 |

## Per-graph two-layer E2E

Median milliseconds. FULL FAST/ACCURATE are fixed policies. Best FAST is selected after measurement and must be labelled an oracle.

| Graph | Average degree | MKL | Paper TFS | B8 FAST | B16 FAST | B64 FAST | FULL FAST | FULL ACCURATE | Best FAST | Best/TFS speedup |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---|---:|
| ogbn-products | 50.5 | 1351.19 | 332.40 | 364.24 | 331.72 | 294.01 | 291.21 | 293.78 | bfull_fast | 1.141x |
| reddit | 492.0 | 441.73 | 302.20 | 170.76 | 146.15 | 123.33 | 136.22 | 134.62 | b64_fast | 2.450x |
| mycielskian19 | 2296.9 | 4793.14 | 2132.54 | 1343.65 | 1188.25 | 1136.28 | 2459.94 | 2426.16 | b64_fast | 1.877x |
| hollywood-2009 | 99.9 | 387.36 | 302.72 | 219.25 | 201.51 | 205.19 | 209.87 | 200.55 | b16_fast | 1.502x |
| indochina-2004 | 26.2 | 697.52 | 355.14 | 291.85 | 246.99 | 215.00 | 207.97 | 217.89 | bfull_fast | 1.708x |
| soc-Pokec | 18.8 | 335.66 | 100.08 | 108.44 | 103.70 | 96.33 | 95.88 | 99.05 | b32_fast | 1.051x |
| cit-Patents | 4.4 | 274.57 | 79.28 | 88.13 | 84.62 | 83.33 | 83.31 | 86.81 | bfull_fast | 0.952x |
| soc-LiveJournal1 | 14.2 | 685.25 | 265.93 | 289.86 | 290.14 | 282.56 | 281.83 | 285.41 | bfull_fast | 0.944x |
| com-LiveJournal | 17.3 | 649.59 | 254.87 | 271.05 | 265.32 | 258.97 | 257.13 | 262.77 | bfull_fast | 0.991x |
| as-Skitter | 13.1 | 164.58 | 109.84 | 97.21 | 89.38 | 80.65 | 80.63 | 81.50 | bfull_fast | 1.362x |
| rgg_n_2_24_s0 | 15.8 | 2042.42 | 922.23 | 1063.96 | 958.73 | 924.37 | 923.62 | 975.62 | bfull_fast | 0.999x |
| web-Google | 5.6 | 46.53 | 21.05 | 22.86 | 21.39 | 21.61 | 21.20 | 22.31 | bfull_fast | 0.993x |
| roadNet-CA | 2.8 | 66.36 | 36.24 | 39.01 | 38.94 | 39.33 | 39.02 | 41.64 | b16_fast | 0.930x |
| wiki-Talk | 2.1 | 90.30 | 109.21 | 74.68 | 57.44 | 48.45 | 47.11 | 48.19 | bfull_fast | 2.318x |
| email-Enron | 10.0 | 1.93 | 2.34 | 1.73 | 1.42 | 1.26 | 1.19 | 1.26 | bfull_fast | 1.968x |
| amazon0601 | 8.4 | 25.04 | 12.64 | 11.92 | 11.06 | 11.42 | 11.46 | 11.83 | b16_fast | 1.142x |
| com-Youtube | 5.3 | 54.17 | 47.89 | 43.71 | 38.69 | 35.57 | 33.55 | 35.35 | bfull_fast | 1.427x |

After-the-fact best FAST oracle geometric mean vs paper TFS: 1.324x, wins 11/17. This is not an adaptive implementation and must not replace the fixed-method averages.

## Numerical controls

Full-output strict FP32 checks retain FAST 1e-2 and ACCURATE 1e-3 thresholds. Actual BF16 final output vs paper BF16 output uses FAST 2e-2 and ACCURATE 1e-2 to allow one final BF16 rounding bin; FP32 MKL reference gate stays 3e-2. All thresholds were recorded before build or measurement, and none were relaxed. Original repeated outputs must be bitwise identical. See `numerical_maxima.csv` and every `check.csv`; these are numerical gates, not classification accuracy.

## Timing, anchor audit, and limits

Primary rotating measurements use one warmup plus five repetitions for every method, source MKL and paper TFS in the same process and original source buffers. Both minimum and median are retained. Raw unmodified anchor measurements and embedded source timings are separate from this primary comparison. `anchor_audit.csv` gives per-graph correspondence, rather than substituting a historical baseline.

Embedded original versus untouched original minimum-ratio range across TFS and MKL: 0.9733 to 1.0354. All raw values remain visible for variability review.

Stage timing is separate: layer1, ReLU, BF16 interlayer conversion, layer2, total; three repeats each. Sampled hot-loop profiles cover row scheduling, partial zeroing, prefetch, feature gather/decode, FP32 reduction, partial BF16 conversion, tile load, tile compute, tile store, final scatter, and AMX setup. BF16 output scatter includes final BF16 conversion. Profile sums are sampled thread time, not additive wall latency, and do not determine speedups.

E2E is prepared two-layer computational inference. Initial loading, DegreeSort, input conversion, and W packing are outside the interval. Random source-defined H/W are used; no trained classification checkpoint is claimed. Six nonunit graphs remain incomparable because original TFS ignores values while MKL uses them; Friendster and road_usa remain outside original LP64 safe bounds. No dataset was changed.

## Reproduction

`bash scripts/build_paper_methods.sh`; then shared `bash scripts/submit_paper_methods.sh smoke`; after the gate, exclusive `high`, `medium`, `low` submissions using the same script. Launcher scripts and pinned build paths are immutable per submission.
