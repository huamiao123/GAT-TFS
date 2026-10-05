# Execution-order control (2026-10-04)

17 real graphs, 612/612 numerical checks passed, 850 measured two-layer E2E repetitions. This supplement was planned after observing small-graph cache sensitivity in interleaved execution. Every method receives an immediate warmup and five consecutive measurements, matching the original source timing order. The previous interleaved results remain intact.

## Frozen controls

Same full CSR hashes, source-seeded H/W, shapes 128→128→128, DegreeSort, original node order, AMX tiles, 32 physical threads, exclusive allocation, default NUMA policy, compiler flags, original MKL helper/hint, FP32 intermediate and direct BF16 final output. No kernel change, parameter selection, numerical gate relaxation, or input substitution. `cache_generation.json` records the frozen kernel hash and the timing-only insertion. Original source MKL stays FP32 as in the paper. The shared smoke had 72/72 checks.

Measured protocols ran at different times/nodes; within each graph all methods share one node/process. Ratios use the TFS/MKL measured in the same protocol and process. Between-protocol differences include run variability and cannot all be causally assigned to cache.

## Fixed policies: all-17 geometric means

| Method | Consecutive / TFS | Consecutive / MKL | Interleaved / TFS | Interleaved / MKL | TFS wins | MKL wins |
|---|---:|---:|---:|---:|---:|---:|
| b8_fast | 1.106x | 2.057x | 1.110x | 2.105x | 9/17 | 16/17 |
| b8_accurate | 1.005x | 1.870x | 1.014x | 1.921x | 6/17 | 16/17 |
| b16_fast | 1.216x | 2.262x | 1.223x | 2.318x | 10/17 | 16/17 |
| b16_accurate | 1.145x | 2.130x | 1.148x | 2.176x | 9/17 | 16/17 |
| b64_fast | 1.309x | 2.435x | 1.301x | 2.466x | 11/17 | 17/17 |
| b64_accurate | 1.262x | 2.348x | 1.261x | 2.391x | 11/17 | 17/17 |
| bfull_fast | 1.258x | 2.341x | 1.251x | 2.372x | 11/17 | 17/17 |
| bfull_accurate | 1.221x | 2.272x | 1.219x | 2.311x | 10/17 | 17/17 |

## Consecutive E2E milliseconds

| Graph | MKL | Original TFS | B8 FAST | B16 FAST | B64 FAST | FULL FAST | FULL ACCURATE |
|---|---:|---:|---:|---:|---:|---:|---:|
| ogbn-products | 1347.40 | 340.52 | 370.62 | 334.29 | 291.99 | 288.54 | 297.94 |
| reddit | 439.82 | 310.50 | 173.05 | 144.60 | 125.29 | 133.85 | 134.02 |
| mycielskian19 | 4768.73 | 2164.31 | 1369.02 | 1211.23 | 1132.98 | 2437.43 | 2432.83 |
| hollywood-2009 | 386.33 | 311.08 | 218.68 | 203.08 | 198.76 | 198.66 | 201.30 |
| indochina-2004 | 696.21 | 350.43 | 291.75 | 248.12 | 216.02 | 209.75 | 217.62 |
| soc-Pokec | 332.10 | 98.89 | 109.92 | 102.14 | 95.04 | 94.92 | 97.92 |
| cit-Patents | 270.21 | 77.24 | 87.88 | 84.18 | 82.78 | 82.80 | 85.89 |
| soc-LiveJournal1 | 680.40 | 265.36 | 283.32 | 285.59 | 276.92 | 275.29 | 280.33 |
| com-LiveJournal | 646.53 | 251.96 | 269.33 | 261.76 | 253.03 | 251.87 | 255.99 |
| as-Skitter | 162.81 | 105.13 | 98.27 | 89.53 | 80.03 | 79.50 | 81.49 |
| rgg_n_2_24_s0 | 2017.73 | 894.11 | 1057.25 | 951.15 | 917.16 | 916.72 | 971.47 |
| web-Google | 47.56 | 21.34 | 23.07 | 21.68 | 21.44 | 21.59 | 22.42 |
| roadNet-CA | 66.12 | 35.55 | 39.15 | 39.19 | 39.14 | 39.11 | 42.28 |
| wiki-Talk | 90.86 | 107.76 | 73.82 | 59.19 | 48.59 | 47.60 | 46.78 |
| email-Enron | 1.40 | 2.34 | 1.72 | 1.43 | 1.25 | 1.17 | 1.24 |
| amazon0601 | 25.60 | 12.88 | 11.91 | 11.31 | 11.31 | 11.44 | 11.93 |
| com-Youtube | 53.71 | 47.70 | 43.47 | 38.89 | 34.90 | 33.84 | 35.38 |

## Boundaries and retained evidence

No losing graph is removed. FULL is not universally faster; compare Mycielskian19 with bounded blocks. Average degree alone does not select the best policy. There is no implemented adaptive selector. FAST/ACCURATE are numerical error controls, not measured classification accuracy. Random source-defined H/W remain in use.

Prepared two-layer computational inference includes layer1, ReLU, intermediate BF16 conversion, layer2 and final output. It excludes initial graph loading, DegreeSort, feature conversion and weight packing exactly as the original source. Detailed stage and sampled hot-loop profiles remain in the main experiment, with their instrumentation limits. Weighted graphs and original LP64-incompatible shapes retain the prior explicit exclusions.

Build: `runs/paper-cache-build-20261004-131154`. Runs: paper-cache-high-10864906, paper-cache-medium-10864907, paper-cache-low-10864908. Reconciled CSV, checks, raw repetitions, source/binary hashes and Slurm accounting are retained under `runs/paper-cache-reconciled-20261004`. Scheduler timeout alone was reduced from two hours to 30 minutes while jobs were pending, based on the larger completed main sweep taking at most 16m43s; before/after requests are recorded in `runs/paper-cache-scheduling-adjustment-20261004-133536`. No measured interval, kernel, thread, memory, affinity or exclusivity setting changed. The parser retains the historical JSON key `rotating` for the supplemental anchor object; in these cache runs it contains consecutive measurements.

Reproduce: restore the hash-verified evidence archive for a source-only Git checkout (compiled binaries are archived, not tracked as separate Git files). The cache build intentionally pins the frozen `paper-method-build-20261004-123445`, rather than silently picking a newly generated source. Then `bash scripts/build_paper_cache_control.sh`, shared `bash scripts/submit_paper_cache_control.sh smoke`, and authorized exclusive `high`, `medium`, `low` using the same script.
