# Original-source MKL/TFS protocol comparison

All paths use default NUMA policy; no explicit interleave/bind/membind. Original source MKL is FP32; TFS/candidates use BF16 inputs and FP32 outputs.

Primary statistic: minimum of five after one warmup. Secondary median and every raw repetition are retained. Prepared two-layer E2E includes ReLU and intermediate conversion, excludes loading/sorting/initial packing.

| Graph | Method | E2E min ms | E2E median ms | vs source MKL | vs source TFS |
|---|---|---:|---:|---:|---:|
| source_tail | source_original_tfs | 0.080 | 0.080 | 0.250x | 1.000x |
| source_tail | source_mkl_fp32 | 0.020 | 0.030 | 1.000x | 4.000x |
| source_tail | original_nozero | 0.057 | 0.057 | 0.351x | 1.404x |
| source_tail | shared_b1_fast_nozero | 0.115 | 0.127 | 0.174x | 0.696x |
| source_tail | shared_b1_accurate_nozero | 0.215 | 0.219 | 0.093x | 0.372x |
| source_tail | shared_b2_fast_nozero | 0.079 | 0.081 | 0.253x | 1.014x |
| source_tail | shared_b2_accurate_nozero | 0.120 | 0.123 | 0.167x | 0.667x |
| source_tail | shared_b4_fast_nozero | 0.050 | 0.054 | 0.401x | 1.605x |
| source_tail | shared_b4_accurate_nozero | 0.071 | 0.073 | 0.281x | 1.126x |
| source_tail | shared_b8_fast_nozero | 0.035 | 0.038 | 0.571x | 2.283x |
| source_tail | shared_b8_accurate_nozero | 0.044 | 0.046 | 0.456x | 1.824x |
| source_tail | shared_b16_fast_nozero | 0.030 | 0.036 | 0.666x | 2.663x |
| source_tail | shared_b16_accurate_nozero | 0.036 | 0.040 | 0.556x | 2.222x |
| source_tail | shared_b32_fast_nozero | 0.025 | 0.027 | 0.799x | 3.196x |
| source_tail | shared_b32_accurate_nozero | 0.028 | 0.032 | 0.717x | 2.868x |
| source_tail | shared_b64_fast_nozero | 0.021 | 0.024 | 0.953x | 3.813x |
| source_tail | shared_b64_accurate_nozero | 0.023 | 0.029 | 0.865x | 3.459x |
| source_tail | shared_bfull_fast_nozero | 0.021 | 0.024 | 0.953x | 3.813x |
| source_tail | shared_bfull_accurate_nozero | 0.025 | 0.028 | 0.799x | 3.196x |
| source_high_tail | source_original_tfs | 0.310 | 0.310 | 0.226x | 1.000x |
| source_high_tail | source_mkl_fp32 | 0.070 | 0.070 | 1.000x | 4.429x |
| source_high_tail | original_nozero | 0.301 | 0.304 | 0.233x | 1.030x |
| source_high_tail | shared_b1_fast_nozero | 0.750 | 0.764 | 0.093x | 0.413x |
| source_high_tail | shared_b1_accurate_nozero | 1.275 | 1.283 | 0.055x | 0.243x |
| source_high_tail | shared_b2_fast_nozero | 0.511 | 0.514 | 0.137x | 0.607x |
| source_high_tail | shared_b2_accurate_nozero | 0.703 | 0.704 | 0.100x | 0.441x |
| source_high_tail | shared_b4_fast_nozero | 0.318 | 0.320 | 0.220x | 0.975x |
| source_high_tail | shared_b4_accurate_nozero | 0.412 | 0.417 | 0.170x | 0.752x |
| source_high_tail | shared_b8_fast_nozero | 0.204 | 0.205 | 0.343x | 1.519x |
| source_high_tail | shared_b8_accurate_nozero | 0.250 | 0.252 | 0.280x | 1.241x |
| source_high_tail | shared_b16_fast_nozero | 0.139 | 0.143 | 0.504x | 2.230x |
| source_high_tail | shared_b16_accurate_nozero | 0.168 | 0.171 | 0.417x | 1.847x |
| source_high_tail | shared_b32_fast_nozero | 0.113 | 0.116 | 0.619x | 2.743x |
| source_high_tail | shared_b32_accurate_nozero | 0.127 | 0.129 | 0.551x | 2.439x |
| source_high_tail | shared_b64_fast_nozero | 0.100 | 0.101 | 0.699x | 3.096x |
| source_high_tail | shared_b64_accurate_nozero | 0.105 | 0.109 | 0.666x | 2.948x |
| source_high_tail | shared_bfull_fast_nozero | 0.086 | 0.087 | 0.813x | 3.602x |
| source_high_tail | shared_bfull_accurate_nozero | 0.093 | 0.097 | 0.751x | 3.325x |

## Status and exclusions

- source_tail: PASS
- source_high_tail: PASS

Historical results with explicit NUMA interleave are a different protocol and must not be combined with this table.
Baseline E2E values retain the original 0.01 ms stdout rounding. Stage/profile experiments are separate from primary timing; sampled thread time is not wall-time percentage.
Random H/W exercise the source inference operator; trained-model accuracy and full application end-to-end are not claimed.
