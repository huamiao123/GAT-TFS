# Journal P0: A0/A1/A2/A3 results

two-layer 128->128->128 static inference compute with random source H/W; not checkpoint classification

Controls and gates: journal A3 source snapshot PROTOCOL.md. Original source TFS and MKL unchanged; DegreeSort/TR16/R64/AMX/default NUMA preserved. Source TFS ends in BF16, source MKL in FP32. A3 uses the same 64-neighbor boundaries and FP32 partial state traffic as A1 with row-major windows.

17 graphs; 1088 standard gates, 459 same-scope bitwise gates, 510 profiling bitwise gates passed. Raw evidence: `runs/journal-a3-reconciled-20261004-230637`.

## Complete two-layer times

| Graph | Source TFS ms | Source MKL ms | A2 B64 ms | A0 FULL ms | A1 S64/MFULL ms | A3 rowwise ms | S128/MFULL ms |
|---|---:|---:|---:|---:|---:|---:|
| amazon0601 | 12.548 | 25.346 | 11.115 | 11.227 | 11.498 | 11.504 | 11.490 |
| as-Skitter | 101.792 | 162.541 | 79.789 | 79.031 | 76.894 | 78.442 | 77.386 |
| cit-Patents | 76.353 | 271.800 | 81.792 | 82.017 | 83.088 | 82.872 | 82.903 |
| com-LiveJournal | 242.267 | 644.811 | 248.873 | 250.363 | 252.404 | 249.626 | 251.347 |
| com-Youtube | 47.670 | 53.643 | 34.609 | 33.840 | 33.726 | 33.677 | 33.289 |
| email-Enron | 2.317 | 1.455 | 1.223 | 1.153 | 1.155 | 1.164 | 1.157 |
| hollywood-2009 | 307.792 | 386.899 | 195.128 | 195.643 | 192.944 | 197.279 | 191.429 |
| indochina-2004 | 346.798 | 690.350 | 214.116 | 207.528 | 208.833 | 208.341 | 208.379 |
| mycielskian19 | 2262.860 | 4773.165 | 1129.101 | 2479.395 | 1109.771 | 2471.474 | 1093.191 |
| ogbn-products | 342.440 | 1355.604 | 296.748 | 293.622 | 296.819 | 300.111 | 295.675 |
| reddit | 313.175 | 441.101 | 126.648 | 135.830 | 121.478 | 133.348 | 119.460 |
| rgg_n_2_24_s0 | 894.840 | 2025.805 | 925.173 | 924.796 | 928.646 | 925.427 | 925.772 |
| roadNet-CA | 34.847 | 67.630 | 38.108 | 38.359 | 38.580 | 38.783 | 38.656 |
| soc-LiveJournal1 | 253.591 | 683.408 | 268.296 | 268.458 | 271.914 | 270.535 | 271.512 |
| soc-Pokec | 96.225 | 330.761 | 93.522 | 93.821 | 93.478 | 93.710 | 93.682 |
| web-Google | 20.524 | 46.788 | 21.213 | 21.479 | 21.475 | 21.623 | 21.522 |
| wiki-Talk | 104.649 | 90.766 | 48.165 | 45.996 | 46.596 | 46.035 | 46.680 |

## Fixed policy results

| Method | Gmean vs TFS | Gmean vs MKL | Wins vs TFS | Rotation vs TFS |
|---|---:|---:|---:|---:|
| b256_accurate | 1.2817 | 2.4253 | 10/17 | 1.2857 |
| b256_fast | 1.3191 | 2.4962 | 11/17 | 1.3236 |
| b64_accurate | 1.2543 | 2.3734 | 10/17 | 1.2610 |
| b64_fast | 1.3032 | 2.4660 | 11/17 | 1.3057 |
| bfull_accurate | 1.2144 | 2.2980 | 9/17 | 1.2195 |
| bfull_fast | 1.2491 | 2.3636 | 10/17 | 1.2511 |
| paper_tfs | 1.0000 | 1.8923 | 0/17 | 1.0000 |
| s128_mfull_fast | 1.3184 | 2.4949 | 11/17 | 1.3239 |
| s32_m256_fast | 1.3058 | 2.4710 | 11/17 | 1.3106 |
| s32_m64_fast | 1.2856 | 2.4328 | 11/17 | 1.2933 |
| s32_mfull_fast | 1.3108 | 2.4803 | 11/17 | 1.3150 |
| s64_m256_fast | 1.3110 | 2.4809 | 11/17 | 1.3139 |
| s64_m64_fast | 1.2916 | 2.4441 | 11/17 | 1.3018 |
| s64_mfull_accurate | 1.2785 | 2.4193 | 10/17 | 1.2832 |
| s64_mfull_fast | 1.3142 | 2.4869 | 11/17 | 1.3211 |
| s64_mfull_rowwise_fast | 1.2441 | 2.3542 | 10/17 | 1.2450 |
| source_mkl_fp32 | 0.5285 | 1.0000 | 2/17 | 0.5214 |

## Measurement limits

A0/A1/A3 preserve output bitwise and the FULL AMX count. A1/A3 preserve the 64-neighbor boundaries and partial state byte counts; their inner loop nesting changes. Varying M changes projection grouping and quantization points. Speedups are paired within each graph/process/node in this run; prior timing values are not denominators.

24 sampled child intervals and per-thread records overlap with parent timers and include profiling perturbation. They are diagnostic records, not additive wall time fractions. Analytic/requested bytes are not measured DRAM traffic. PMU records retain per-thread coverage/scaling and enable/disable skew; generic cache events are CPU dependent. Performance counters are measured in separate complete-forward regions after primary timing.

No graph-wise oracle is reported as an implemented policy. A useful window or block mechanism alone does not establish a journal contribution; the actual AMX work, sparse dependency traversal, partial-state lifetime and numerical constraints need a validated general model.
