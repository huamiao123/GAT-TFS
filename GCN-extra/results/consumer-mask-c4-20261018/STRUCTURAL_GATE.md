# C4 consumer-mask shared aggregation: 17-graph structure gate

2026-10-18; Job `10996266`, shared `intel` node `qhcn252`, one CPU, 16 GiB request, `COMPLETED 0:0`, elapsed 00:02:20. This report concerns the original unit-weight 128→128→128 cohort and 16-row destination tiles. No NUMA interleave was used. It is a *structural* gate, not an AVX/AMX runtime or numerical result.

The algorithm groups distinct source vertices with the exact same 16-bit consumer mask within a destination tile. For a group of `n` sources delivered to `k` rows, a shared FP32 vector sum can in principle save `(n-1)(k-1)*128` additions relative to adding every source to every row. Repeated edges within one row are conservatively excluded. Existing FULL has one AMX projection per nonempty tile, so this structure gives **no implied AMX instruction reduction**. HAG-like shared subaggregation is prior art; the prospective distinction would be an executable mask plan with controlled metadata, AVX delivery, unchanged AMX utilization, and acceptable quantization error.

| Graph | Identity ideal AVX additions saved | DegreeSort ideal saved |
|---|---:|---:|
| ogbn-products | 0.08% | 0.41% |
| reddit | 4.86% | 14.70% |
| mycielskian19 | 67.11% | 52.28% |
| hollywood-2009 | 43.57% | 19.73% |
| indochina-2004 | 77.15% | 64.83% |
| soc-Pokec | 1.84% | 0.18% |
| cit-Patents | 0.70% | 0.66% |
| soc-LiveJournal1 | 9.56% | 2.11% |
| com-LiveJournal | 8.81% | 2.06% |
| as-Skitter | 18.02% | 10.98% |
| rgg_n_2_24_s0 | 1.19% | 0.39% |
| web-Google | 0.10% | 0.58% |
| roadNet-CA | 0.79% | 0.63% |
| wiki-Talk | 1.69% | 2.95% |
| email-Enron | 7.99% | 6.54% |
| amazon0601 | 9.45% | 8.36% |
| com-Youtube | 1.57% | 2.12% |

Identity has ≥10% potential on only four graphs; two largest values come from Mycielskian19 (constructed) and Indochina. Products, RGG, Web-Google, and RoadNet have little room for this exact mechanism. DegreeSort raises Reddit but substantially reduces the mask opportunity on several high-overlap graphs; data layout and shared aggregation therefore must be treated jointly. The upper bound does not establish whether an Identity or Degree kernel will actually be faster.

The naive all-source plan payload estimate (6 bytes per unique tile source plus 4 per distinct mask group) is 1.77 GB for Identity Mycielskian19, 0.32 GB for Hollywood, 0.19 GB for Indochina, 0.65 GB for Reddit, and 0.75 GB for Products. This is **not** measured allocation, a minimal encoding, or DRAM traffic. Selectively encoding only eligible groups could reduce plan size but requires an efficient way to skip their original edge occurrences; that problem has not been solved in this probe. The scanner's per-order analysis time on one shared CPU was about 16.1 s for Identity Mycielskian19, 2.5 s Hollywood, 1.8 s Indochina, and 4.0 s Reddit, excluding current input SHA check. These times describe this hash-map analyzer, not a tuned production planner or formal benchmark.

Evidence: `runs/consumer_mask_c4_20261018-01/summary.json`, `summary.csv`, 34 per-order records in `raw/`, `source_binary.sha256`, `manifest_hashes.sha256`, `results.sha256`, `preflight.sha256`, `smoke.log`, `slurm-10996266.*`, and `sacct.txt`. Each of the 17 CSR files matched the frozen inventory SHA-256, format/size/bounds and E coverage. Independent Python fixtures checked a duplicated unsorted graph, no overlap, cross-tile boundary, and high overlap; all passed. Amazon and RoadNet Identity/Degree permutation FNV hashes match prior frontier-structure logs. An initial smoke attempt failed because the server Python lacks `int.bit_count()`; the independent oracle was changed to `bin(mask).count('1')` and all fixtures passed before submission.

**Interpretation:** C4 passes a *selective structural opportunity* gate, not a universal speedup gate. The next experiment must hold one plan fixed and compare per-source fanout with shared-mask aggregation, plus the existing rowwise FULL Identity on the same input/precision/timing boundary. It must report plan-build cost, plan bytes, numerical error (including BF16 conversion), actual AVX/AMX work and end-to-end time. A mask group reducing AVX adds cannot be equated with fewer AMX TDP calls. Skip low-opportunity graphs rather than claiming a free graph-specific oracle; any deployed selector needs precomputed, amortized, or cheap observable features.
