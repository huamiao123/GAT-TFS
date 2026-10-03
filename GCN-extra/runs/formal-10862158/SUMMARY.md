# GCN-extra first inference experiments

Numerical sum-aggregation experiments with random H/W; no task-accuracy claim.
Uninstrumented medians; original source is unchanged. Speedups are original/new and FP32 MKL/new.
Profiles are sampled summed thread time and do not equal wall time. Physical AMX work is modeled, not a hardware counter.

## com-Friendster

N=65608366, E=3612134270, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 8856.494 | 8846.474 | 8856.116 | 8870.309 | 2.061x | 1.559x |
| shared_bfull_accurate_nozero | 8993.503 | 8974.347 | 8986.978 | 9000.781 | 2.029x | 1.536x |
| shared_b64_fast_nozero | 9116.514 | 9104.689 | 9115.013 | 9128.388 | 2.002x | 1.515x |
| shared_b64_accurate_nozero | 9336.737 | 9322.501 | 9334.712 | 9348.618 | 1.955x | 1.479x |
| shared_b32_fast_nozero | 9370.779 | 9355.889 | 9367.025 | 9385.272 | 1.948x | 1.474x |
| shared_b32_accurate_nozero | 9680.837 | 9668.077 | 9677.998 | 9692.165 | 1.885x | 1.427x |
| shared_b16_fast_nozero | 9845.624 | 9828.030 | 9829.414 | 9854.373 | 1.854x | 1.403x |
| shared_b16_accurate_nozero | 10330.389 | 10310.254 | 10310.360 | 10335.237 | 1.767x | 1.337x |
| shared_b8_fast_nozero | 10570.556 | 10542.432 | 10545.045 | 10613.692 | 1.727x | 1.307x |
| shared_b8_accurate_nozero | 11654.377 | 11648.839 | 11653.726 | 11654.887 | 1.566x | 1.185x |
| original_nozero | 11721.669 | 11701.777 | 11718.316 | 11724.186 | 1.557x | 1.178x |
| mkl_bf16_inputs | 13668.581 | 13654.107 | 13655.648 | 13698.219 | 1.335x | 1.010x |
| mkl_fp32 | 13811.140 | 13784.649 | 13802.029 | 13816.634 | 1.322x | 1.000x |
| original | 18251.452 | 18223.722 | 18245.874 | 18259.813 | 1.000x | 0.757x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 22832.924 | 22788.738 | 22811.456 | 22861.976 | 2.005x | 1.243x |
| shared_bfull_accurate_nozero | 23063.263 | 23014.488 | 23036.298 | 23080.686 | 1.985x | 1.231x |
| shared_b64_fast_nozero | 23490.578 | 23457.295 | 23476.336 | 23520.936 | 1.949x | 1.209x |
| shared_b64_accurate_nozero | 23862.039 | 23818.314 | 23839.502 | 23886.731 | 1.919x | 1.190x |
| shared_b32_fast_nozero | 24042.997 | 23997.869 | 24019.753 | 24053.067 | 1.904x | 1.181x |
| shared_b32_accurate_nozero | 24563.503 | 24514.345 | 24537.289 | 24567.683 | 1.864x | 1.156x |
| shared_b16_fast_nozero | 25066.766 | 25029.660 | 25054.617 | 25097.878 | 1.826x | 1.133x |
| shared_b16_accurate_nozero | 25869.270 | 25829.965 | 25852.535 | 25895.408 | 1.770x | 1.098x |
| shared_b8_fast_nozero | 26569.558 | 26530.836 | 26548.921 | 26580.792 | 1.723x | 1.069x |
| shared_b8_accurate_nozero | 28325.115 | 28279.781 | 28310.872 | 28347.052 | 1.616x | 1.002x |
| mkl_fp32 | 28392.124 | 28375.261 | 28386.650 | 28395.890 | 1.612x | 1.000x |
| mkl_bf16_inputs | 28904.533 | 28865.589 | 28903.141 | 28919.134 | 1.584x | 0.982x |
| original_nozero | 31889.764 | 31875.951 | 31886.453 | 31895.809 | 1.436x | 0.890x |
| original | 45782.212 | 45318.404 | 45779.613 | 45786.594 | 1.000x | 0.620x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.244e-03 | 5.703e+00 | 5.118e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.220e-03 | 5.703e+00 | 5.092e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 7.531e-07 | 5.127e-03 | 4.601e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.220e-03 | 5.703e+00 | 5.092e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 7.531e-07 | 5.127e-03 | 4.601e-06 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.220e-03 | 5.703e+00 | 5.092e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.144e-03 | 3.413e+00 | 3.063e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.864e-03 | 8.653e+00 | 7.725e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 7.130e-07 | 2.869e-03 | 2.574e-06 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.220e-03 | 5.703e+00 | 5.092e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.245e-03 | 3.648e+00 | 3.274e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 7.973e-03 | 9.350e+00 | 8.348e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.862e-06 | 2.518e-03 | 2.259e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.220e-03 | 5.704e+00 | 5.093e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.260e-03 | 4.108e+00 | 3.686e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 7.992e-03 | 9.810e+00 | 8.759e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.511e-06 | 4.089e-03 | 3.670e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.220e-03 | 5.704e+00 | 5.093e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.282e-03 | 3.798e+00 | 3.408e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 8.015e-03 | 9.500e+00 | 8.482e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 5.290e-06 | 5.981e-03 | 5.368e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.222e-03 | 5.708e+00 | 5.096e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.294e-03 | 3.627e+00 | 3.255e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.020e-03 | 8.962e+00 | 8.002e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 7.449e-06 | 1.123e-02 | 1.008e-05 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.223e-03 | 5.709e+00 | 5.097e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.144e-03 | 3.413e+00 | 3.063e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 7.130e-07 | 2.869e-03 | 2.574e-06 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.245e-03 | 3.648e+00 | 3.274e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.862e-06 | 2.518e-03 | 2.259e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.260e-03 | 4.108e+00 | 3.686e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.511e-06 | 4.089e-03 | 3.670e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.282e-03 | 3.798e+00 | 3.408e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 5.290e-06 | 5.981e-03 | 5.368e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.294e-03 | 3.627e+00 | 3.255e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 7.449e-06 | 1.123e-02 | 1.008e-05 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.001e-02 | 3.846e+04 | 1.030e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 9.908e-03 | 3.846e+04 | 1.019e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 5.308e-06 | 4.250e+01 | 1.138e-05 | 1 |
| original | two_layer_e2e | mkl_fp32 | 9.908e-03 | 3.845e+04 | 1.019e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 5.308e-06 | 4.250e+01 | 1.138e-05 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 9.908e-03 | 3.845e+04 | 1.019e-02 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.273e-03 | 1.969e+04 | 5.271e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.508e-02 | 5.706e+04 | 1.512e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 4.387e-06 | 1.400e+01 | 3.748e-06 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.909e-03 | 3.846e+04 | 1.019e-02 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.534e-03 | 2.101e+04 | 5.624e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.534e-02 | 5.838e+04 | 1.547e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 6.324e-06 | 2.125e+01 | 5.688e-06 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.909e-03 | 3.846e+04 | 1.019e-02 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.594e-03 | 2.074e+04 | 5.553e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.539e-02 | 5.811e+04 | 1.540e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 9.533e-06 | 3.490e+01 | 9.342e-06 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.912e-03 | 3.847e+04 | 1.020e-02 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.638e-03 | 2.099e+04 | 5.619e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.543e-02 | 5.834e+04 | 1.546e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.361e-05 | 4.200e+01 | 1.124e-05 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.916e-03 | 3.849e+04 | 1.020e-02 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.842e-03 | 2.420e+04 | 6.478e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.551e-02 | 6.234e+04 | 1.652e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.041e-05 | 7.525e+01 | 2.014e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.921e-03 | 3.850e+04 | 1.020e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 6322.2010 | 16019 | 18796.542 |
| original | row_schedule | 10.2382 | 16019 | 18796.542 |
| original | partial_zero | 22.4667 | 16019 | 18796.542 |
| original | csr_prefetch | 895.9100 | 16019 | 18796.542 |
| original | feature_gather_decode | 204.0603 | 16019 | 18796.542 |
| original | fp32_neighbor_reduction | 0.0000 | 16019 | 18796.542 |
| original | partial_bf16_conversion | 0.0000 | 16019 | 18796.542 |
| original | tile_load | 826.5843 | 16019 | 18796.542 |
| original | tile_compute | 1043.4296 | 16019 | 18796.542 |
| original | tile_store | 31.7428 | 16019 | 18796.542 |
| original | output_scatter | 15.6319 | 16019 | 18796.542 |
| original | thread_amx_setup | 627.8784 | 16019 | 18796.542 |

