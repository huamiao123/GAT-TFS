# Original-source MKL/TFS protocol comparison

All paths use default NUMA policy; no explicit interleave/bind/membind. Original source MKL is FP32; TFS/candidates use BF16 inputs and FP32 outputs.

Primary statistic: minimum of five after one warmup. Secondary median and every raw repetition are retained. Prepared two-layer E2E includes ReLU and intermediate conversion, excludes loading/sorting/initial packing.

| Graph | Method | E2E min ms | E2E median ms | vs source MKL | vs source TFS |
|---|---|---:|---:|---:|---:|
| rgg_n_2_24_s0 | source_original_tfs | 1819.500 | 1824.240 | 1.105x | 1.000x |
| rgg_n_2_24_s0 | source_mkl_fp32 | 2009.950 | 2012.090 | 1.000x | 0.905x |
| rgg_n_2_24_s0 | original_nozero | 1290.833 | 1291.353 | 1.557x | 1.410x |
| rgg_n_2_24_s0 | shared_b2_fast_nozero | 1829.932 | 1832.706 | 1.098x | 0.994x |
| rgg_n_2_24_s0 | shared_b2_accurate_nozero | 2045.089 | 2045.985 | 0.983x | 0.890x |
| rgg_n_2_24_s0 | shared_b4_fast_nozero | 1620.802 | 1622.084 | 1.240x | 1.123x |
| rgg_n_2_24_s0 | shared_b4_accurate_nozero | 1733.104 | 1735.248 | 1.160x | 1.050x |
| rgg_n_2_24_s0 | shared_b8_fast_nozero | 1465.054 | 1467.874 | 1.372x | 1.242x |
| rgg_n_2_24_s0 | shared_b8_accurate_nozero | 1540.724 | 1542.327 | 1.305x | 1.181x |
| rgg_n_2_24_s0 | shared_b16_fast_nozero | 1357.836 | 1358.771 | 1.480x | 1.340x |
| rgg_n_2_24_s0 | shared_b16_accurate_nozero | 1429.200 | 1430.012 | 1.406x | 1.273x |
| rgg_n_2_24_s0 | shared_b32_fast_nozero | 1319.887 | 1323.488 | 1.523x | 1.379x |
| rgg_n_2_24_s0 | shared_b32_accurate_nozero | 1384.284 | 1386.930 | 1.452x | 1.314x |
| rgg_n_2_24_s0 | shared_b64_fast_nozero | 1320.287 | 1322.937 | 1.522x | 1.378x |
| rgg_n_2_24_s0 | shared_b64_accurate_nozero | 1386.525 | 1388.358 | 1.450x | 1.312x |
| rgg_n_2_24_s0 | shared_bfull_fast_nozero | 1321.020 | 1322.638 | 1.522x | 1.377x |
| rgg_n_2_24_s0 | shared_bfull_accurate_nozero | 1388.005 | 1389.348 | 1.448x | 1.311x |

## Status and exclusions

- mycielskian19: SKIPPED — Outside the explicitly requested supplemental graph subset
- reddit: SKIPPED — Outside the explicitly requested supplemental graph subset
- hollywood-2009: SKIPPED — Outside the explicitly requested supplemental graph subset
- kron_g500-logn21: SKIPPED — Outside the explicitly requested supplemental graph subset
- com-Friendster: SKIPPED — Outside the explicitly requested supplemental graph subset
- ogbn-products: SKIPPED — Outside the explicitly requested supplemental graph subset
- indochina-2004: SKIPPED — Outside the explicitly requested supplemental graph subset
- cage15: SKIPPED — Outside the explicitly requested supplemental graph subset
- soc-Pokec: SKIPPED — Outside the explicitly requested supplemental graph subset
- com-LiveJournal: SKIPPED — Outside the explicitly requested supplemental graph subset
- rgg_n_2_24_s0: PASS
- soc-LiveJournal1: SKIPPED — Outside the explicitly requested supplemental graph subset
- as-Skitter: SKIPPED — Outside the explicitly requested supplemental graph subset
- email-Enron: SKIPPED — Outside the explicitly requested supplemental graph subset
- FullChip: SKIPPED — Outside the explicitly requested supplemental graph subset
- amazon0601: SKIPPED — Outside the explicitly requested supplemental graph subset
- scircuit: SKIPPED — Outside the explicitly requested supplemental graph subset
- web-Google: SKIPPED — Outside the explicitly requested supplemental graph subset
- com-Youtube: SKIPPED — Outside the explicitly requested supplemental graph subset
- cit-Patents: SKIPPED — Outside the explicitly requested supplemental graph subset
- sx-stackoverflow: SKIPPED — Outside the explicitly requested supplemental graph subset
- rajat31: SKIPPED — Outside the explicitly requested supplemental graph subset
- roadNet-CA: SKIPPED — Outside the explicitly requested supplemental graph subset
- road_usa: SKIPPED — Outside the explicitly requested supplemental graph subset
- wiki-Talk: SKIPPED — Outside the explicitly requested supplemental graph subset

Historical results with explicit NUMA interleave are a different protocol and must not be combined with this table.
Baseline E2E values retain the original 0.01 ms stdout rounding. Stage/profile experiments are separate from primary timing; sampled thread time is not wall-time percentage.
Random H/W exercise the source inference operator; trained-model accuracy and full application end-to-end are not claimed.
