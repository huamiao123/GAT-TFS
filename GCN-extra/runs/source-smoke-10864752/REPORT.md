# Original-source MKL/TFS protocol comparison

All paths use default NUMA policy; no explicit interleave/bind/membind. Original source MKL is FP32; TFS/candidates use BF16 inputs and FP32 outputs.

Primary statistic: minimum of five after one warmup. Secondary median and every raw repetition are retained. Prepared two-layer E2E includes ReLU and intermediate conversion, excludes loading/sorting/initial packing.

| Graph | Method | E2E min ms | E2E median ms | vs source MKL | vs source TFS |
|---|---|---:|---:|---:|---:|
| source_tail | source_original_tfs | 0.070 | 0.080 | 0.429x | 1.000x |
| source_tail | source_mkl_fp32 | 0.030 | 0.030 | 1.000x | 2.333x |
| source_tail | original_nozero | 0.051 | 0.060 | 0.588x | 1.372x |
| source_tail | shared_b1_fast_nozero | 0.117 | 0.122 | 0.256x | 0.598x |
| source_tail | shared_b1_accurate_nozero | 0.212 | 0.216 | 0.142x | 0.330x |
| source_tail | shared_b2_fast_nozero | 0.077 | 0.086 | 0.390x | 0.909x |
| source_tail | shared_b2_accurate_nozero | 0.120 | 0.122 | 0.250x | 0.584x |
| source_tail | shared_b4_fast_nozero | 0.051 | 0.055 | 0.588x | 1.372x |
| source_tail | shared_b4_accurate_nozero | 0.071 | 0.073 | 0.424x | 0.989x |
| source_tail | shared_b8_fast_nozero | 0.035 | 0.036 | 0.862x | 2.011x |
| source_tail | shared_b8_accurate_nozero | 0.044 | 0.051 | 0.680x | 1.587x |
| source_tail | shared_b16_fast_nozero | 0.027 | 0.028 | 1.114x | 2.598x |
| source_tail | shared_b16_accurate_nozero | 0.034 | 0.034 | 0.880x | 2.053x |
| source_tail | shared_b32_fast_nozero | 0.025 | 0.028 | 1.198x | 2.796x |
| source_tail | shared_b32_accurate_nozero | 0.028 | 0.032 | 1.075x | 2.509x |
| source_tail | shared_b64_fast_nozero | 0.021 | 0.021 | 1.430x | 3.336x |
| source_tail | shared_b64_accurate_nozero | 0.024 | 0.027 | 1.258x | 2.936x |
| source_tail | shared_bfull_fast_nozero | 0.021 | 0.021 | 1.430x | 3.336x |
| source_tail | shared_bfull_accurate_nozero | 0.023 | 0.028 | 1.311x | 3.058x |
| source_high_tail | source_original_tfs | 0.310 | 0.320 | 0.226x | 1.000x |
| source_high_tail | source_mkl_fp32 | 0.070 | 0.080 | 1.000x | 4.429x |
| source_high_tail | original_nozero | 0.304 | 0.306 | 0.230x | 1.020x |
| source_high_tail | shared_b1_fast_nozero | 0.750 | 0.765 | 0.093x | 0.413x |
| source_high_tail | shared_b1_accurate_nozero | 1.269 | 1.286 | 0.055x | 0.244x |
| source_high_tail | shared_b2_fast_nozero | 0.516 | 0.536 | 0.136x | 0.601x |
| source_high_tail | shared_b2_accurate_nozero | 0.701 | 0.709 | 0.100x | 0.442x |
| source_high_tail | shared_b4_fast_nozero | 0.314 | 0.323 | 0.223x | 0.987x |
| source_high_tail | shared_b4_accurate_nozero | 0.412 | 0.418 | 0.170x | 0.752x |
| source_high_tail | shared_b8_fast_nozero | 0.201 | 0.205 | 0.348x | 1.542x |
| source_high_tail | shared_b8_accurate_nozero | 0.247 | 0.251 | 0.283x | 1.255x |
| source_high_tail | shared_b16_fast_nozero | 0.142 | 0.146 | 0.493x | 2.182x |
| source_high_tail | shared_b16_accurate_nozero | 0.169 | 0.170 | 0.414x | 1.834x |
| source_high_tail | shared_b32_fast_nozero | 0.112 | 0.116 | 0.626x | 2.772x |
| source_high_tail | shared_b32_accurate_nozero | 0.126 | 0.131 | 0.555x | 2.458x |
| source_high_tail | shared_b64_fast_nozero | 0.099 | 0.102 | 0.707x | 3.133x |
| source_high_tail | shared_b64_accurate_nozero | 0.105 | 0.109 | 0.666x | 2.948x |
| source_high_tail | shared_bfull_fast_nozero | 0.091 | 0.093 | 0.769x | 3.404x |
| source_high_tail | shared_bfull_accurate_nozero | 0.095 | 0.097 | 0.736x | 3.259x |

## Status and exclusions

- source_tail: PASS
- source_high_tail: PASS

Historical results with explicit NUMA interleave are a different protocol and must not be combined with this table.
Baseline E2E values retain the original 0.01 ms stdout rounding. Stage/profile experiments are separate from primary timing; sampled thread time is not wall-time percentage.
Random H/W exercise the source inference operator; trained-model accuracy and full application end-to-end are not claimed.
