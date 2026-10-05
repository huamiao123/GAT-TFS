# Journal P1: projection-budget-preserving destination grouping

Two-layer 128->128->128 prepared inference compute; random source H/W and unit edges. Original TFS/MKL and P0 kernels are unchanged. All comparisons are same graph/process/node, 32 physical cores, one socket, shared intel, default NUMA. The inherited 24 child timings are sampled thread diagnostics, not additive wall decomposition. Generic cache events and logical feature bytes are not measured DRAM bandwidth.

Coverage: 17 real + 3 controlled graphs; 880 numerical, 480 same-scope bitwise, 320 profile bitwise, 320 work gates passed. All q64 slots and projection budgets preserved.

## Fixed policies, real graphs only

| Scope | Schedule | Gmean vs DegreeSort | Alternating order | Wins | >5% wins | Gmean vs source TFS |
|---|---|---:|---:|---:|---:|---:|
| mfull | qshuffle | 0.9291 | 0.9297 | 0/17 | 0/17 | 1.2288 |
| mfull | qsource | 0.9995 | 1.0028 | 4/17 | 2/17 | 1.3219 |
| mfull | qpage | 1.0126 | 1.0185 | 8/17 | 3/17 | 1.3392 |
| m64 | qshuffle | 0.9270 | 0.9338 | 0/17 | 0/17 | 1.2035 |
| m64 | qsource | 0.9994 | 1.0035 | 5/17 | 2/17 | 1.2975 |
| m64 | qpage | 1.0123 | 1.0183 | 7/17 | 3/17 | 1.3142 |

## Real graph prepared E2E

Times are five-repeat uninstrumented medians, ms.

| Graph | Source TFS | Source MKL | Coupled B64 | Coupled FULL | FULL Degree | FULL Shuffle | FULL Source | FULL Page | B64 Degree | B64 Shuffle | B64 Source | B64 Page |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| ogbn-products | 344.196 | 1359.369 | 298.290 | 295.536 | 294.426 | 294.879 | 289.464 | 289.743 | 298.234 | 302.883 | 297.085 | 295.855 |
| reddit | 311.829 | 441.116 | 127.545 | 136.240 | 121.422 | 122.146 | 95.376 | 104.517 | 126.919 | 129.424 | 103.018 | 111.371 |
| mycielskian19 | 2274.864 | 4807.972 | 1146.049 | 2437.959 | 1102.346 | 1846.722 | 1142.801 | 904.912 | 1111.277 | 1856.692 | 1152.941 | 943.295 |
| roadNet-CA | 35.744 | 67.786 | 39.530 | 39.361 | 39.675 | 43.778 | 43.122 | 42.152 | 40.065 | 43.923 | 43.529 | 42.212 |
| wiki-Talk | 108.137 | 90.953 | 48.445 | 45.883 | 46.296 | 47.919 | 47.661 | 47.080 | 49.546 | 50.327 | 49.350 | 50.355 |
| hollywood-2009 | 318.690 | 389.022 | 200.549 | 201.628 | 199.344 | 216.185 | 188.807 | 182.587 | 201.748 | 221.560 | 187.054 | 181.295 |
| indochina-2004 | 352.704 | 692.548 | 217.339 | 210.599 | 211.439 | 253.983 | 213.748 | 211.434 | 217.882 | 261.190 | 219.393 | 217.369 |
| soc-Pokec | 98.053 | 333.633 | 95.024 | 95.063 | 95.594 | 96.721 | 95.721 | 95.820 | 96.206 | 97.001 | 97.326 | 97.667 |
| cit-Patents | 77.440 | 273.006 | 82.566 | 83.826 | 84.261 | 87.113 | 84.925 | 84.978 | 84.109 | 87.093 | 85.111 | 85.094 |
| soc-LiveJournal1 | 261.287 | 685.088 | 278.974 | 276.541 | 281.292 | 283.849 | 289.704 | 289.946 | 280.879 | 284.785 | 290.337 | 291.599 |
| com-LiveJournal | 255.048 | 648.981 | 253.981 | 253.603 | 257.536 | 259.393 | 262.612 | 263.641 | 256.801 | 259.911 | 263.398 | 265.105 |
| as-Skitter | 105.559 | 163.746 | 79.761 | 79.730 | 78.132 | 80.246 | 78.282 | 77.411 | 80.795 | 82.830 | 80.248 | 78.881 |
| rgg_n_2_24_s0 | 911.690 | 2038.607 | 938.844 | 937.184 | 939.994 | 983.217 | 948.670 | 948.999 | 940.744 | 982.768 | 950.522 | 949.531 |
| web-Google | 20.998 | 47.000 | 21.584 | 21.653 | 21.773 | 22.043 | 21.778 | 21.771 | 21.688 | 22.116 | 21.782 | 21.751 |
| email-Enron | 2.298 | 1.452 | 1.236 | 1.162 | 1.189 | 1.191 | 1.177 | 1.178 | 1.229 | 1.301 | 1.248 | 1.200 |
| amazon0601 | 12.707 | 25.615 | 11.230 | 11.399 | 11.348 | 13.061 | 12.338 | 12.340 | 11.690 | 13.001 | 12.462 | 12.343 |
| com-Youtube | 47.517 | 54.438 | 35.035 | 34.080 | 33.801 | 35.241 | 34.455 | 34.567 | 35.091 | 36.413 | 35.322 | 35.857 |

## Controlled graphs (separate from real-graph gmean)

Times are five-repeat uninstrumented medians, ms.

| Graph | Source TFS | Source MKL | Coupled B64 | Coupled FULL | FULL Degree | FULL Shuffle | FULL Source | FULL Page | B64 Degree | B64 Shuffle | B64 Source | B64 Page |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| regular128_reuse1 | 183.414 | 405.039 | 198.950 | 196.654 | 193.867 | 124.095 | 184.497 | 183.281 | 197.945 | 129.062 | 188.395 | 188.467 |
| regular128_reuse4 | 110.815 | 139.184 | 72.985 | 67.689 | 71.158 | 75.059 | 72.253 | 72.163 | 74.090 | 77.579 | 74.966 | 74.811 |
| regular128_reuse16 | 92.065 | 77.989 | 37.119 | 34.382 | 35.852 | 67.746 | 35.638 | 35.408 | 37.745 | 69.258 | 38.166 | 37.558 |

## Preprocessing and amortization

See group_setup.csv and group_audit.csv for allocation, parallel key pass, bounded segment creation, each sort, and verification. paired_grouping.csv charges a conservative candidate upper setup (the shared all-key pass and allocation are charged to each candidate). The derived break-even count uses the measured median difference and this upper cost; it is descriptive, not a production decision rule. Only prepared E2E enters the table above. One-forward plus setup is explicitly separate.

| Graph | Scope | Schedule | Prepared speedup | Setup upper ms | One forward + setup upper ms | Upper break-even forwards |
|---|---|---|---:|---:|---:|---:|
| ogbn-products | mfull | qshuffle | 0.998 | 133.615 | 428.494 | no measured gain |
| reddit | mfull | qshuffle | 0.994 | 16.071 | 138.217 | no measured gain |
| mycielskian19 | mfull | qshuffle | 0.597 | 27.930 | 1874.652 | no measured gain |
| roadNet-CA | mfull | qshuffle | 0.906 | 42.709 | 86.487 | no measured gain |
| wiki-Talk | mfull | qshuffle | 0.966 | 50.840 | 98.759 | no measured gain |
| hollywood-2009 | mfull | qshuffle | 0.922 | 50.640 | 266.825 | no measured gain |
| indochina-2004 | mfull | qshuffle | 0.832 | 264.756 | 518.739 | no measured gain |
| soc-Pokec | mfull | qshuffle | 0.988 | 62.059 | 158.780 | no measured gain |
| cit-Patents | mfull | qshuffle | 0.967 | 109.651 | 196.764 | no measured gain |
| soc-LiveJournal1 | mfull | qshuffle | 0.991 | 226.773 | 510.622 | no measured gain |
| com-LiveJournal | mfull | qshuffle | 0.993 | 176.291 | 435.684 | no measured gain |
| as-Skitter | mfull | qshuffle | 0.974 | 58.291 | 138.537 | no measured gain |
| rgg_n_2_24_s0 | mfull | qshuffle | 0.956 | 965.386 | 1948.603 | no measured gain |
| web-Google | mfull | qshuffle | 0.988 | 27.092 | 49.135 | no measured gain |
| email-Enron | mfull | qshuffle | 0.998 | 1.609 | 2.800 | no measured gain |
| amazon0601 | mfull | qshuffle | 0.869 | 10.839 | 23.900 | no measured gain |
| com-Youtube | mfull | qshuffle | 0.959 | 32.406 | 67.647 | no measured gain |
| regular128_reuse1 | mfull | qshuffle | 1.562 | 21.744 | 145.839 | 1 |
| regular128_reuse4 | mfull | qshuffle | 0.948 | 21.843 | 96.902 | no measured gain |
| regular128_reuse16 | mfull | qshuffle | 0.529 | 21.728 | 89.474 | no measured gain |
| ogbn-products | mfull | qsource | 1.017 | 134.723 | 424.187 | 28 |
| reddit | mfull | qsource | 1.273 | 16.503 | 111.879 | 1 |
| mycielskian19 | mfull | qsource | 0.965 | 28.990 | 1171.791 | no measured gain |
| roadNet-CA | mfull | qsource | 0.920 | 42.958 | 86.080 | no measured gain |
| wiki-Talk | mfull | qsource | 0.971 | 51.174 | 98.835 | no measured gain |
| hollywood-2009 | mfull | qsource | 1.056 | 51.726 | 240.533 | 5 |
| indochina-2004 | mfull | qsource | 0.989 | 261.677 | 475.425 | no measured gain |
| soc-Pokec | mfull | qsource | 0.999 | 62.517 | 158.238 | no measured gain |
| cit-Patents | mfull | qsource | 0.992 | 108.713 | 193.638 | no measured gain |
| soc-LiveJournal1 | mfull | qsource | 0.971 | 227.454 | 517.158 | no measured gain |
| com-LiveJournal | mfull | qsource | 0.981 | 175.594 | 438.206 | no measured gain |
| as-Skitter | mfull | qsource | 0.998 | 58.945 | 137.227 | no measured gain |
| rgg_n_2_24_s0 | mfull | qsource | 0.991 | 992.140 | 1940.810 | no measured gain |
| web-Google | mfull | qsource | 1.000 | 27.944 | 49.722 | no measured gain |
| email-Enron | mfull | qsource | 1.010 | 1.634 | 2.811 | 138 |
| amazon0601 | mfull | qsource | 0.920 | 10.748 | 23.086 | no measured gain |
| com-Youtube | mfull | qsource | 0.981 | 33.079 | 67.534 | no measured gain |
| regular128_reuse1 | mfull | qsource | 1.051 | 21.604 | 206.101 | 3 |
| regular128_reuse4 | mfull | qsource | 0.985 | 21.467 | 93.720 | no measured gain |
| regular128_reuse16 | mfull | qsource | 1.006 | 21.079 | 56.717 | 99 |
| ogbn-products | mfull | qpage | 1.016 | 135.349 | 425.092 | 29 |
| reddit | mfull | qpage | 1.162 | 16.672 | 121.189 | 1 |
| mycielskian19 | mfull | qpage | 1.218 | 27.972 | 932.884 | 1 |
| roadNet-CA | mfull | qpage | 0.941 | 42.481 | 84.633 | no measured gain |
| wiki-Talk | mfull | qpage | 0.983 | 50.602 | 97.682 | no measured gain |
| hollywood-2009 | mfull | qpage | 1.092 | 51.503 | 234.090 | 4 |
| indochina-2004 | mfull | qpage | 1.000 | 260.273 | 471.707 | 54584 |
| soc-Pokec | mfull | qpage | 0.998 | 62.873 | 158.693 | no measured gain |
| cit-Patents | mfull | qpage | 0.992 | 108.474 | 193.452 | no measured gain |
| soc-LiveJournal1 | mfull | qpage | 0.970 | 227.851 | 517.797 | no measured gain |
| com-LiveJournal | mfull | qpage | 0.977 | 176.313 | 439.954 | no measured gain |
| as-Skitter | mfull | qpage | 1.009 | 58.402 | 135.813 | 81 |
| rgg_n_2_24_s0 | mfull | qpage | 0.991 | 988.441 | 1937.440 | no measured gain |
| web-Google | mfull | qpage | 1.000 | 27.368 | 49.139 | 16399 |
| email-Enron | mfull | qpage | 1.010 | 1.643 | 2.821 | 147 |
| amazon0601 | mfull | qpage | 0.920 | 10.654 | 22.994 | no measured gain |
| com-Youtube | mfull | qpage | 0.978 | 32.823 | 67.390 | no measured gain |
| regular128_reuse1 | mfull | qpage | 1.058 | 21.585 | 204.866 | 3 |
| regular128_reuse4 | mfull | qpage | 0.986 | 21.350 | 93.513 | no measured gain |
| regular128_reuse16 | mfull | qpage | 1.013 | 20.885 | 56.293 | 48 |
| ogbn-products | m64 | qshuffle | 0.985 | 133.615 | 436.498 | no measured gain |
| reddit | m64 | qshuffle | 0.981 | 16.071 | 145.495 | no measured gain |
| mycielskian19 | m64 | qshuffle | 0.599 | 27.930 | 1884.622 | no measured gain |
| roadNet-CA | m64 | qshuffle | 0.912 | 42.709 | 86.632 | no measured gain |
| wiki-Talk | m64 | qshuffle | 0.984 | 50.840 | 101.167 | no measured gain |
| hollywood-2009 | m64 | qshuffle | 0.911 | 50.640 | 272.200 | no measured gain |
| indochina-2004 | m64 | qshuffle | 0.834 | 264.756 | 525.946 | no measured gain |
| soc-Pokec | m64 | qshuffle | 0.992 | 62.059 | 159.060 | no measured gain |
| cit-Patents | m64 | qshuffle | 0.966 | 109.651 | 196.744 | no measured gain |
| soc-LiveJournal1 | m64 | qshuffle | 0.986 | 226.773 | 511.558 | no measured gain |
| com-LiveJournal | m64 | qshuffle | 0.988 | 176.291 | 436.202 | no measured gain |
| as-Skitter | m64 | qshuffle | 0.975 | 58.291 | 141.121 | no measured gain |
| rgg_n_2_24_s0 | m64 | qshuffle | 0.957 | 965.386 | 1948.153 | no measured gain |
| web-Google | m64 | qshuffle | 0.981 | 27.092 | 49.208 | no measured gain |
| email-Enron | m64 | qshuffle | 0.945 | 1.609 | 2.910 | no measured gain |
| amazon0601 | m64 | qshuffle | 0.899 | 10.839 | 23.840 | no measured gain |
| com-Youtube | m64 | qshuffle | 0.964 | 32.406 | 68.819 | no measured gain |
| regular128_reuse1 | m64 | qshuffle | 1.534 | 21.744 | 150.806 | 1 |
| regular128_reuse4 | m64 | qshuffle | 0.955 | 21.843 | 99.422 | no measured gain |
| regular128_reuse16 | m64 | qshuffle | 0.545 | 21.728 | 90.986 | no measured gain |
| ogbn-products | m64 | qsource | 1.004 | 134.723 | 431.808 | 118 |
| reddit | m64 | qsource | 1.232 | 16.503 | 119.521 | 1 |
| mycielskian19 | m64 | qsource | 0.964 | 28.990 | 1181.931 | no measured gain |
| roadNet-CA | m64 | qsource | 0.920 | 42.958 | 86.487 | no measured gain |
| wiki-Talk | m64 | qsource | 1.004 | 51.174 | 100.524 | 262 |
| hollywood-2009 | m64 | qsource | 1.079 | 51.726 | 238.780 | 4 |
| indochina-2004 | m64 | qsource | 0.993 | 261.677 | 481.070 | no measured gain |
| soc-Pokec | m64 | qsource | 0.988 | 62.517 | 159.843 | no measured gain |
| cit-Patents | m64 | qsource | 0.988 | 108.713 | 193.824 | no measured gain |
| soc-LiveJournal1 | m64 | qsource | 0.967 | 227.454 | 517.791 | no measured gain |
| com-LiveJournal | m64 | qsource | 0.975 | 175.594 | 438.992 | no measured gain |
| as-Skitter | m64 | qsource | 1.007 | 58.945 | 139.193 | 108 |
| rgg_n_2_24_s0 | m64 | qsource | 0.990 | 992.140 | 1942.662 | no measured gain |
| web-Google | m64 | qsource | 0.996 | 27.944 | 49.726 | no measured gain |
| email-Enron | m64 | qsource | 0.985 | 1.634 | 2.882 | no measured gain |
| amazon0601 | m64 | qsource | 0.938 | 10.748 | 23.210 | no measured gain |
| com-Youtube | m64 | qsource | 0.993 | 33.079 | 68.401 | no measured gain |
| regular128_reuse1 | m64 | qsource | 1.051 | 21.604 | 209.999 | 3 |
| regular128_reuse4 | m64 | qsource | 0.988 | 21.467 | 96.433 | no measured gain |
| regular128_reuse16 | m64 | qsource | 0.989 | 21.079 | 59.245 | no measured gain |
| ogbn-products | m64 | qpage | 1.008 | 135.349 | 431.204 | 57 |
| reddit | m64 | qpage | 1.140 | 16.672 | 128.043 | 2 |
| mycielskian19 | m64 | qpage | 1.178 | 27.972 | 971.267 | 1 |
| roadNet-CA | m64 | qpage | 0.949 | 42.481 | 84.693 | no measured gain |
| wiki-Talk | m64 | qpage | 0.984 | 50.602 | 100.957 | no measured gain |
| hollywood-2009 | m64 | qpage | 1.113 | 51.503 | 232.798 | 3 |
| indochina-2004 | m64 | qpage | 1.002 | 260.273 | 477.642 | 508 |
| soc-Pokec | m64 | qpage | 0.985 | 62.873 | 160.540 | no measured gain |
| cit-Patents | m64 | qpage | 0.988 | 108.474 | 193.568 | no measured gain |
| soc-LiveJournal1 | m64 | qpage | 0.963 | 227.851 | 519.450 | no measured gain |
| com-LiveJournal | m64 | qpage | 0.969 | 176.313 | 441.418 | no measured gain |
| as-Skitter | m64 | qpage | 1.024 | 58.402 | 137.283 | 31 |
| rgg_n_2_24_s0 | m64 | qpage | 0.991 | 988.441 | 1937.972 | no measured gain |
| web-Google | m64 | qpage | 0.997 | 27.368 | 49.119 | no measured gain |
| email-Enron | m64 | qpage | 1.024 | 1.643 | 2.843 | 57 |
| amazon0601 | m64 | qpage | 0.947 | 10.654 | 22.997 | no measured gain |
| com-Youtube | m64 | qpage | 0.979 | 32.823 | 68.680 | no measured gain |
| regular128_reuse1 | m64 | qpage | 1.050 | 21.585 | 210.052 | 3 |
| regular128_reuse4 | m64 | qpage | 0.990 | 21.350 | 96.161 | no measured gain |
| regular128_reuse16 | m64 | qpage | 1.005 | 20.885 | 58.443 | 112 |

## Evidence and limitations

Full raw repetitions: time.csv; per-layer stages: stage.csv; 24 sampled child phases: detail.csv; per-thread work/completion: thread.csv and thread_detail.csv; work: counters.csv; PMU: pmu.csv, pmu_thread.csv and pmu_wall.csv; sampled source reuse/page occupancy: group_structure.csv. Preserve slowdowns and order discrepancies. Source/page signatures use only the first 64 CSR entries and relative feature-page IDs; neither is a complete neighborhood overlap estimator. The 4096 segment and signature constants were fixed before all performance runs.

Synthetic graphs share N=524288, E=67108864, degree128, source in-degree128 and the same shuffled source ID mapping; native DegreeSort tile-local reuse is 1/4/16. They do not establish real-graph generality. Source TFS is BF16 and source MKL FP32, as in the original paper protocol. No classifier checkpoint accuracy or measured DRAM bandwidth is claimed.
