# P3/P4 follow-up measured results

Shared SPR, one socket/32 physical cores for diagnostic graphs; four for smoke. Default NUMA/no interleave; original compiler flags. Random source-style discrete H/W, unit adjacency. P3 retains original source harness; P4 independent shape harness does not replace paper-compatible TFS/MKL. Cache rebuild enters each P4 layer/forward. Numerical gates are frozen; rejected candidates are never timed. No task accuracy is claimed.

P3 FP32 checks use original TFS with ACCURATE 1e-3; BF16-final original 1e-2 and source MKL 3e-2. P4 uses quantized-H/W mathematical MKL references with native BF16 interlayer boundary (1e-3 FP32, 1e-2 BF16-final). The timed original-style shape MKL uses FP32 master inputs/weights and no BF16 interlayer boundary; it is a separate anchor, not a precision-matched fusion attribution.

## P3 diagnostic two-layer medians (ms)

| Graph | Original TFS | B64 accurate | S64/FULL accurate | D2 | D3 period4 | FULL/D2 |
|---|---:|---:|---:|---:|---:|---:|
| mycielskian19 | 2221.749 | 1134.082 | 1087.586 | 1091.427 | 1120.026 | 0.9965 |
| ogbn-products | 345.697 | 307.698 | 296.056 | 319.269 | 317.833 | 0.9273 |
| reddit | 312.182 | 124.776 | 121.994 | 126.050 | 127.501 | 0.9678 |
| roadNet-CA | 35.406 | 40.813 | 41.125 | 48.102 | 47.725 | 0.8550 |
| wiki-Talk | 107.933 | 49.295 | 46.507 | 50.644 | 50.661 | 0.9183 |

## P4 all fixed paths, diagnostic and controlled graphs

Ratio >1 means the candidate is faster than fused FULL ACCURATE. Stage tables separate cache rebuild from sparse reduction/cold AMX projection and merge. Source-consumer preprocessing is outside prepared forward; setup estimates are reported separately, not hidden. Both project_all and project_used and the predefined project_used-L1/FULL-L2 control are retained.

| Graph | Shape | Method | Median ms | FULL/Method | CV |
|---|---|---|---:|---:|---:|
| bipartite_hot | 128_32_32 | fused_full_accurate | 1.076 | 1.0000 | 0.2561 |
| bipartite_hot | 128_32_32 | hybrid_none | 1.635 | 0.6582 | 0.0355 |
| bipartite_hot | 128_32_32 | top_1_64 | 0.930 | 1.1569 | 0.0153 |
| bipartite_hot | 128_32_32 | top_1_16 | 0.933 | 1.1533 | 0.0163 |
| bipartite_hot | 128_32_32 | top_1_4 | 0.938 | 1.1472 | 0.0169 |
| bipartite_hot | 128_32_32 | hybrid_all | 1.134 | 0.9489 | 0.0175 |
| bipartite_hot | 128_32_32 | project_all | 0.895 | 1.2022 | 0.0412 |
| bipartite_hot | 128_32_32 | project_used | 0.866 | 1.2422 | 0.1105 |
| bipartite_hot | 128_32_32 | project_used_L1_full_L2 | 0.820 | 1.3123 | 0.2993 |
| bipartite_hot | 128_32_32 | shape_mkl_fp32_original_style | 1.435 | 0.7498 | 0.0432 |
| bipartite_hot | 128_128_128 | fused_full_accurate | 2.899 | 1.0000 | 0.0663 |
| bipartite_hot | 128_128_128 | hybrid_none | 3.487 | 0.8314 | 0.0269 |
| bipartite_hot | 128_128_128 | top_1_64 | 3.193 | 0.9079 | 0.0535 |
| bipartite_hot | 128_128_128 | top_1_16 | 3.051 | 0.9502 | 0.0205 |
| bipartite_hot | 128_128_128 | top_1_4 | 3.069 | 0.9446 | 0.0272 |
| bipartite_hot | 128_128_128 | hybrid_all | 3.629 | 0.7988 | 0.0129 |
| bipartite_hot | 128_128_128 | project_all | 3.141 | 0.9229 | 0.0136 |
| bipartite_hot | 128_128_128 | project_used | 3.202 | 0.9054 | 0.0362 |
| bipartite_hot | 128_128_128 | project_used_L1_full_L2 | 3.392 | 0.8546 | 0.0695 |
| bipartite_hot | 128_128_128 | shape_mkl_fp32_original_style | 3.538 | 0.8193 | 0.0464 |
| bipartite_spread | 128_32_32 | fused_full_accurate | 2.463 | 1.0000 | 0.1287 |
| bipartite_spread | 128_32_32 | hybrid_none | 2.842 | 0.8665 | 0.0128 |
| bipartite_spread | 128_32_32 | top_1_64 | 2.927 | 0.8415 | 0.0133 |
| bipartite_spread | 128_32_32 | top_1_16 | 3.071 | 0.8020 | 0.0189 |
| bipartite_spread | 128_32_32 | top_1_4 | 3.524 | 0.6989 | 0.0549 |
| bipartite_spread | 128_32_32 | hybrid_all | 2.887 | 0.8531 | 0.0546 |
| bipartite_spread | 128_32_32 | project_all | 2.359 | 1.0441 | 0.0428 |
| bipartite_spread | 128_32_32 | project_used | 2.700 | 0.9121 | 0.0271 |
| bipartite_spread | 128_32_32 | project_used_L1_full_L2 | 2.080 | 1.1841 | 0.1123 |
| bipartite_spread | 128_32_32 | shape_mkl_fp32_original_style | 15.739 | 0.1565 | 0.0047 |
| bipartite_spread | 128_128_128 | fused_full_accurate | 5.911 | 1.0000 | 0.0375 |
| bipartite_spread | 128_128_128 | hybrid_none | 6.139 | 0.9628 | 0.0078 |
| bipartite_spread | 128_128_128 | top_1_64 | 6.283 | 0.9408 | 0.0028 |
| bipartite_spread | 128_128_128 | top_1_16 | 6.691 | 0.8834 | 0.0131 |
| bipartite_spread | 128_128_128 | top_1_4 | 8.890 | 0.6649 | 0.0363 |
| bipartite_spread | 128_128_128 | hybrid_all | 12.046 | 0.4907 | 0.0191 |
| bipartite_spread | 128_128_128 | project_all | 12.294 | 0.4808 | 0.0234 |
| bipartite_spread | 128_128_128 | project_used | 11.990 | 0.4930 | 0.0236 |
| bipartite_spread | 128_128_128 | project_used_L1_full_L2 | 9.036 | 0.6541 | 0.0265 |
| bipartite_spread | 128_128_128 | shape_mkl_fp32_original_style | 27.588 | 0.2143 | 0.0138 |
| ogbn-products | 128_32_32 | fused_full_accurate | 208.367 | 1.0000 | 0.0076 |
| ogbn-products | 128_32_32 | hybrid_none | 247.553 | 0.8417 | 0.0068 |
| ogbn-products | 128_32_32 | top_1_64 | 249.288 | 0.8358 | 0.0098 |
| ogbn-products | 128_32_32 | top_1_16 | 235.282 | 0.8856 | 0.0134 |
| ogbn-products | 128_32_32 | top_1_4 | 219.973 | 0.9472 | 0.0095 |
| ogbn-products | 128_32_32 | hybrid_all | 179.202 | 1.1627 | 0.0052 |
| ogbn-products | 128_32_32 | project_all | 143.393 | 1.4531 | 0.0089 |
| ogbn-products | 128_32_32 | project_used | 192.754 | 1.0810 | 0.0112 |
| ogbn-products | 128_32_32 | project_used_L1_full_L2 | 157.216 | 1.3254 | 0.0125 |
| ogbn-products | 128_32_32 | shape_mkl_fp32_original_style | 1279.404 | 0.1629 | 0.0015 |
| ogbn-products | 128_128_128 | fused_full_accurate | 385.757 | 1.0000 | 0.0156 |
| ogbn-products | 128_128_128 | hybrid_none | 442.639 | 0.8715 | 0.0035 |
| ogbn-products | 128_128_128 | top_1_64 | 460.140 | 0.8383 | 0.0117 |
| ogbn-products | 128_128_128 | top_1_16 | 490.918 | 0.7858 | 0.0030 |
| ogbn-products | 128_128_128 | top_1_4 | 574.392 | 0.6716 | 0.0099 |
| ogbn-products | 128_128_128 | hybrid_all | 641.074 | 0.6017 | 0.0044 |
| ogbn-products | 128_128_128 | project_all | 578.738 | 0.6665 | 0.0054 |
| ogbn-products | 128_128_128 | project_used | 677.518 | 0.5694 | 0.0231 |
| ogbn-products | 128_128_128 | project_used_L1_full_L2 | 535.395 | 0.7205 | 0.0248 |
| ogbn-products | 128_128_128 | shape_mkl_fp32_original_style | 2303.613 | 0.1675 | 0.0011 |
| reddit | 128_32_32 | fused_full_accurate | 75.163 | 1.0000 | 0.0366 |
| reddit | 128_32_32 | hybrid_none | 84.937 | 0.8849 | 0.0263 |
| reddit | 128_32_32 | top_1_64 | 92.580 | 0.8119 | 0.0263 |
| reddit | 128_32_32 | top_1_16 | 101.009 | 0.7441 | 0.0219 |
| reddit | 128_32_32 | top_1_4 | 83.177 | 0.9037 | 0.0254 |
| reddit | 128_32_32 | hybrid_all | 65.263 | 1.1517 | 0.0401 |
| reddit | 128_32_32 | project_all | 51.509 | 1.4592 | 0.0422 |
| reddit | 128_32_32 | project_used | 74.594 | 1.0076 | 0.0397 |
| reddit | 128_32_32 | project_used_L1_full_L2 | 53.368 | 1.4084 | 0.0127 |
| reddit | 128_32_32 | shape_mkl_fp32_original_style | 384.203 | 0.1956 | 0.0104 |
| reddit | 128_128_128 | fused_full_accurate | 128.466 | 1.0000 | 0.0295 |
| reddit | 128_128_128 | hybrid_none | 139.410 | 0.9215 | 0.0311 |
| reddit | 128_128_128 | top_1_64 | 172.063 | 0.7466 | 0.0184 |
| reddit | 128_128_128 | top_1_16 | 297.012 | 0.4325 | 0.0072 |
| reddit | 128_128_128 | top_1_4 | 362.641 | 0.3543 | 0.0104 |
| reddit | 128_128_128 | hybrid_all | 439.690 | 0.2922 | 0.0056 |
| reddit | 128_128_128 | project_all | 439.671 | 0.2922 | 0.0109 |
| reddit | 128_128_128 | project_used | 357.973 | 0.3589 | 0.0348 |
| reddit | 128_128_128 | project_used_L1_full_L2 | 241.755 | 0.5314 | 0.0250 |
| reddit | 128_128_128 | shape_mkl_fp32_original_style | 676.726 | 0.1898 | 0.0009 |
| mycielskian19 | 128_32_32 | fused_full_accurate | 662.591 | 1.0000 | 0.0267 |
| mycielskian19 | 128_32_32 | hybrid_none | 736.471 | 0.8997 | 0.0142 |
| mycielskian19 | 128_32_32 | top_1_64 | 681.537 | 0.9722 | 0.0142 |
| mycielskian19 | 128_32_32 | top_1_16 | 605.717 | 1.0939 | 0.0180 |
| mycielskian19 | 128_32_32 | top_1_4 | 491.363 | 1.3485 | 0.0309 |
| mycielskian19 | 128_32_32 | hybrid_all | 501.884 | 1.3202 | 0.0171 |
| mycielskian19 | 128_32_32 | project_all | 446.336 | 1.4845 | 0.0159 |
| mycielskian19 | 128_32_32 | project_used | 435.488 | 1.5215 | 0.0390 |
| mycielskian19 | 128_32_32 | project_used_L1_full_L2 | 340.251 | 1.9474 | 0.0269 |
| mycielskian19 | 128_32_32 | shape_mkl_fp32_original_style | 3436.304 | 0.1928 | 0.0010 |
| mycielskian19 | 128_128_128 | fused_full_accurate | 1103.174 | 1.0000 | 0.0125 |
| mycielskian19 | 128_128_128 | hybrid_none | 1190.898 | 0.9263 | 0.0121 |
| mycielskian19 | 128_128_128 | top_1_64 | 1346.664 | 0.8192 | 0.0145 |
| mycielskian19 | 128_128_128 | top_1_16 | 1833.824 | 0.6016 | 0.0115 |
| mycielskian19 | 128_128_128 | top_1_4 | 1736.587 | 0.6353 | 0.0124 |
| mycielskian19 | 128_128_128 | hybrid_all | 2014.106 | 0.5477 | 0.0259 |
| mycielskian19 | 128_128_128 | project_all | 1938.316 | 0.5691 | 0.0151 |
| mycielskian19 | 128_128_128 | project_used | 2128.473 | 0.5183 | 0.0089 |
| mycielskian19 | 128_128_128 | project_used_L1_full_L2 | 1606.962 | 0.6865 | 0.0350 |
| mycielskian19 | 128_128_128 | shape_mkl_fp32_original_style | 6163.777 | 0.1790 | 0.0003 |
| roadNet-CA | 128_32_32 | fused_full_accurate | 30.774 | 1.0000 | 0.0032 |
| roadNet-CA | 128_32_32 | hybrid_none | 35.031 | 0.8785 | 0.0057 |
| roadNet-CA | 128_32_32 | top_1_64 | 35.476 | 0.8675 | 0.0082 |
| roadNet-CA | 128_32_32 | top_1_16 | 36.279 | 0.8483 | 0.0061 |
| roadNet-CA | 128_32_32 | top_1_4 | 36.877 | 0.8345 | 0.0049 |
| roadNet-CA | 128_32_32 | hybrid_all | 32.263 | 0.9539 | 0.0093 |
| roadNet-CA | 128_32_32 | project_all | 34.327 | 0.8965 | 0.0073 |
| roadNet-CA | 128_32_32 | project_used | 36.858 | 0.8349 | 0.0076 |
| roadNet-CA | 128_32_32 | project_used_L1_full_L2 | 32.886 | 0.9358 | 0.0068 |
| roadNet-CA | 128_32_32 | shape_mkl_fp32_original_style | 64.693 | 0.4757 | 0.0051 |
| roadNet-CA | 128_128_128 | fused_full_accurate | 99.068 | 1.0000 | 0.0035 |
| roadNet-CA | 128_128_128 | hybrid_none | 104.018 | 0.9524 | 0.0028 |
| roadNet-CA | 128_128_128 | top_1_64 | 102.581 | 0.9658 | 0.0035 |
| roadNet-CA | 128_128_128 | top_1_16 | 97.652 | 1.0145 | 0.0021 |
| roadNet-CA | 128_128_128 | top_1_4 | 98.694 | 1.0038 | 0.0054 |
| roadNet-CA | 128_128_128 | hybrid_all | 98.058 | 1.0103 | 0.0024 |
| roadNet-CA | 128_128_128 | project_all | 95.166 | 1.0410 | 0.0065 |
| roadNet-CA | 128_128_128 | project_used | 98.642 | 1.0043 | 0.0092 |
| roadNet-CA | 128_128_128 | project_used_L1_full_L2 | 94.357 | 1.0499 | 0.0063 |
| roadNet-CA | 128_128_128 | shape_mkl_fp32_original_style | 126.360 | 0.7840 | 0.0042 |
| wiki-Talk | 128_32_32 | fused_full_accurate | 48.517 | 1.0000 | 0.0146 |
| wiki-Talk | 128_32_32 | hybrid_none | 54.450 | 0.8910 | 0.0175 |
| wiki-Talk | 128_32_32 | top_1_64 | 52.335 | 0.9270 | 0.0115 |
| wiki-Talk | 128_32_32 | top_1_16 | 53.459 | 0.9076 | 0.0133 |
| wiki-Talk | 128_32_32 | top_1_4 | 53.430 | 0.9081 | 0.0081 |
| wiki-Talk | 128_32_32 | hybrid_all | 52.883 | 0.9174 | 0.0132 |
| wiki-Talk | 128_32_32 | project_all | 48.615 | 0.9980 | 0.0161 |
| wiki-Talk | 128_32_32 | project_used | 53.840 | 0.9011 | 0.0075 |
| wiki-Talk | 128_32_32 | project_used_L1_full_L2 | 47.107 | 1.0299 | 0.0200 |
| wiki-Talk | 128_32_32 | shape_mkl_fp32_original_style | 66.208 | 0.7328 | 0.0045 |
| wiki-Talk | 128_128_128 | fused_full_accurate | 152.142 | 1.0000 | 0.0163 |
| wiki-Talk | 128_128_128 | hybrid_none | 159.833 | 0.9519 | 0.0111 |
| wiki-Talk | 128_128_128 | top_1_64 | 160.442 | 0.9483 | 0.0157 |
| wiki-Talk | 128_128_128 | top_1_16 | 160.658 | 0.9470 | 0.0076 |
| wiki-Talk | 128_128_128 | top_1_4 | 168.322 | 0.9039 | 0.0147 |
| wiki-Talk | 128_128_128 | hybrid_all | 186.865 | 0.8142 | 0.0150 |
| wiki-Talk | 128_128_128 | project_all | 179.309 | 0.8485 | 0.0142 |
| wiki-Talk | 128_128_128 | project_used | 188.105 | 0.8088 | 0.0129 |
| wiki-Talk | 128_128_128 | project_used_L1_full_L2 | 167.845 | 0.9064 | 0.0147 |
| wiki-Talk | 128_128_128 | shape_mkl_fp32_original_style | 119.969 | 1.2682 | 0.0056 |

## Integrity, rejection and interpretation boundaries

P3 160 numerical records, 0 rejections; P4 540 numerical records, 0 rejections. These include aligned smoke, not the superseded first P4 smoke.
Frozen source hashes verified. P4 none/all degeneracy and width128 P0 kernel checks passed; profiled outputs are bitwise checked. Every cold/cache tile count also matches source-plan topology. All raw samples and stdout are archived. See p3/ and p4/ for checks, acceptance, stages, sampled child phases, PMU/per-TID, plans, theoretical feature-request bytes, thread diagnostics, min/max/CV. Sampled child/thread times are not additive wall time; logical bytes are not DRAM traffic. PMU generic misses do not identify a cache level. No global AH is materialized; Q cache sizes and mixed-plan overhead are explicit.
Controlled two-direction bipartite graphs share N=65536, degree64 and E=4194304, with source pools512/32768 per side. They are unit, directed relation operators without self-loops, not necessarily symmetric normalized GCN. They establish an applicable condition, not real-graph generality. No oracle winner is presented as an implemented adaptive policy.
