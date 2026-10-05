# Independent window / projection scope results

two-layer 128->128->128 static inference compute with random source H/W; not checkpoint classification

Controls and gates: [preregistered protocol](PROJECTION_WINDOW_PROTOCOL_20261004.md). Original source TFS and MKL unchanged; DegreeSort/TR16/R64/AMX/default NUMA preserved. Source TFS ends in BF16, source MKL in FP32.

17 graphs; 1020 standard gates, 408 same-scope bitwise gates, 476 profiling bitwise gates passed. Raw evidence: `runs/projection-window-reconciled-20261004-222427`.

## Complete two-layer times

| Graph | Source TFS ms | Source MKL ms | B64 FAST ms | FULL FAST ms | S64/MFULL ms | S128/MFULL ms | S64/M256 ms |
|---|---:|---:|---:|---:|---:|---:|---:|
| amazon0601 | 12.633 | 25.036 | 11.218 | 11.424 | 11.392 | 11.467 | 11.407 |
| as-Skitter | 102.949 | 162.485 | 78.128 | 79.057 | 76.558 | 76.406 | 77.610 |
| cit-Patents | 76.516 | 270.686 | 81.872 | 82.912 | 83.450 | 83.438 | 83.307 |
| com-LiveJournal | 245.924 | 643.357 | 250.159 | 252.251 | 253.658 | 253.060 | 254.921 |
| com-Youtube | 46.943 | 53.431 | 34.558 | 33.341 | 33.512 | 33.027 | 33.977 |
| email-Enron | 2.291 | 1.416 | 1.209 | 1.152 | 1.179 | 1.166 | 1.173 |
| hollywood-2009 | 308.760 | 385.942 | 196.747 | 196.125 | 195.057 | 194.761 | 195.586 |
| indochina-2004 | 348.773 | 691.313 | 214.759 | 208.562 | 209.677 | 209.374 | 210.720 |
| mycielskian19 | 2209.944 | 4764.692 | 1120.540 | 2450.815 | 1084.969 | 1079.979 | 1077.809 |
| ogbn-products | 344.676 | 1357.572 | 294.572 | 296.765 | 299.484 | 296.279 | 293.276 |
| reddit | 317.022 | 442.625 | 129.968 | 135.601 | 122.053 | 119.507 | 124.451 |
| rgg_n_2_24_s0 | 904.574 | 2014.816 | 930.732 | 933.871 | 935.808 | 934.233 | 934.831 |
| roadNet-CA | 36.249 | 68.302 | 39.444 | 39.681 | 39.699 | 39.822 | 39.678 |
| soc-LiveJournal1 | 260.758 | 678.946 | 274.703 | 272.756 | 276.646 | 277.009 | 277.796 |
| soc-Pokec | 97.759 | 330.531 | 95.181 | 94.805 | 94.171 | 94.987 | 94.419 |
| web-Google | 20.749 | 46.672 | 21.342 | 21.348 | 21.526 | 21.472 | 21.389 |
| wiki-Talk | 107.755 | 90.727 | 48.199 | 45.755 | 46.601 | 46.488 | 46.991 |

## Fixed policy results

| Method | Gmean vs TFS | Gmean vs MKL | Wins vs TFS | Rotation vs TFS |
|---|---:|---:|---:|---:|
| b256_accurate | 1.2875 | 2.4078 | 11/17 | 1.2902 |
| b256_fast | 1.3263 | 2.4803 | 11/17 | 1.3228 |
| b64_accurate | 1.2596 | 2.3556 | 10/17 | 1.2646 |
| b64_fast | 1.3074 | 2.4450 | 11/17 | 1.3010 |
| bfull_accurate | 1.2190 | 2.2797 | 10/17 | 1.2204 |
| bfull_fast | 1.2537 | 2.3445 | 10/17 | 1.2526 |
| paper_tfs | 1.0000 | 1.8701 | 0/17 | 1.0000 |
| s128_mfull_fast | 1.3239 | 2.4758 | 11/17 | 1.3243 |
| s32_m256_fast | 1.3095 | 2.4489 | 11/17 | 1.3120 |
| s32_m64_fast | 1.2971 | 2.4257 | 11/17 | 1.2991 |
| s32_mfull_fast | 1.3148 | 2.4588 | 11/17 | 1.3128 |
| s64_m256_fast | 1.3168 | 2.4627 | 11/17 | 1.3160 |
| s64_m64_fast | 1.3005 | 2.4322 | 11/17 | 1.3011 |
| s64_mfull_accurate | 1.2823 | 2.3981 | 10/17 | 1.2832 |
| s64_mfull_fast | 1.3195 | 2.4675 | 11/17 | 1.3221 |
| source_mkl_fp32 | 0.5347 | 1.0000 | 2/17 | 0.5261 |

## Measurement limits

Varying S at fixed M preserves bitwise output and executed AMX count. Varying M changes projection grouping and quantization points. Speedups are paired within this run; prior timing values are not denominators.

24 sampled child intervals and per-thread records overlap with parent timers and include profiling perturbation. They are diagnostic records, not additive wall time fractions. Analytic/requested bytes are not measured DRAM traffic. PMU records retain per-thread coverage/scaling and enable/disable skew; generic cache events are CPU dependent. Performance counters are measured in separate complete-forward regions after primary timing.

No graph-wise oracle is reported as an implemented policy. A useful window or block mechanism alone does not establish a journal contribution; the actual AMX work, sparse dependency traversal, partial-state lifetime and numerical constraints need a validated general model.
