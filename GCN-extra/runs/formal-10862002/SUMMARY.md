# GCN-extra first inference experiments

Numerical sum-aggregation experiments with random H/W; no task-accuracy claim.
Uninstrumented medians; original source is unchanged. Speedups are original/new and FP32 MKL/new.
Profiles are sampled summed thread time and do not equal wall time. Physical AMX work is modeled, not a hardware counter.

## amazon0601

N=403394, E=3387388, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b16_fast_nozero | 3.700 | 3.656 | 3.685 | 3.711 | 6.479x | 2.125x |
| shared_b32_fast_nozero | 3.709 | 3.665 | 3.698 | 3.719 | 6.463x | 2.120x |
| shared_bfull_fast_nozero | 3.714 | 3.691 | 3.704 | 3.740 | 6.454x | 2.118x |
| shared_b64_fast_nozero | 3.715 | 3.700 | 3.709 | 3.725 | 6.452x | 2.117x |
| shared_bfull_accurate_nozero | 3.987 | 3.976 | 3.978 | 4.019 | 6.012x | 1.972x |
| shared_b64_accurate_nozero | 3.991 | 3.976 | 3.985 | 3.999 | 6.006x | 1.971x |
| shared_b32_accurate_nozero | 4.003 | 3.971 | 3.991 | 4.013 | 5.988x | 1.965x |
| shared_b16_accurate_nozero | 4.004 | 3.967 | 3.987 | 4.012 | 5.987x | 1.964x |
| shared_b8_fast_nozero | 4.256 | 4.233 | 4.243 | 4.266 | 5.632x | 1.848x |
| original_nozero | 4.604 | 4.594 | 4.601 | 4.614 | 5.206x | 1.708x |
| shared_b8_accurate_nozero | 4.676 | 4.667 | 4.675 | 4.699 | 5.126x | 1.682x |
| shared_b4_fast_nozero | 4.863 | 4.833 | 4.859 | 4.873 | 4.929x | 1.617x |
| shared_b4_accurate_nozero | 5.592 | 5.573 | 5.581 | 5.619 | 4.287x | 1.406x |
| shared_b2_fast_nozero | 6.091 | 6.074 | 6.086 | 6.103 | 3.935x | 1.291x |
| shared_b2_accurate_nozero | 7.371 | 7.338 | 7.349 | 7.395 | 3.252x | 1.067x |
| mkl_bf16_inputs | 7.806 | 7.638 | 7.786 | 7.822 | 3.071x | 1.008x |
| mkl_fp32 | 7.864 | 7.789 | 7.831 | 7.891 | 3.048x | 1.000x |
| original | 23.971 | 23.882 | 23.938 | 23.994 | 1.000x | 0.328x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b16_fast_nozero | 10.176 | 9.947 | 10.052 | 10.281 | 4.529x | 1.580x |
| shared_b32_fast_nozero | 10.177 | 10.036 | 10.077 | 10.255 | 4.528x | 1.580x |
| shared_b64_fast_nozero | 10.234 | 10.127 | 10.174 | 10.408 | 4.503x | 1.571x |
| shared_bfull_fast_nozero | 10.327 | 10.178 | 10.283 | 10.409 | 4.463x | 1.557x |
| shared_b16_accurate_nozero | 10.682 | 10.559 | 10.664 | 10.730 | 4.314x | 1.505x |
| shared_b32_accurate_nozero | 10.735 | 10.550 | 10.665 | 10.804 | 4.293x | 1.497x |
| shared_b64_accurate_nozero | 10.807 | 10.554 | 10.689 | 10.864 | 4.265x | 1.488x |
| shared_bfull_accurate_nozero | 10.844 | 10.788 | 10.833 | 11.000 | 4.250x | 1.482x |
| shared_b8_fast_nozero | 11.048 | 10.907 | 10.980 | 11.104 | 4.172x | 1.455x |
| original_nozero | 11.391 | 11.243 | 11.294 | 11.406 | 4.046x | 1.411x |
| shared_b8_accurate_nozero | 11.969 | 11.776 | 11.840 | 12.064 | 3.850x | 1.343x |
| shared_b4_fast_nozero | 12.397 | 12.314 | 12.352 | 12.437 | 3.718x | 1.297x |
| shared_b4_accurate_nozero | 13.930 | 13.821 | 13.891 | 14.001 | 3.308x | 1.154x |
| shared_b2_fast_nozero | 14.828 | 14.712 | 14.796 | 14.840 | 3.108x | 1.084x |
| mkl_fp32 | 16.075 | 15.919 | 15.995 | 16.164 | 2.867x | 1.000x |
| shared_b2_accurate_nozero | 17.408 | 17.323 | 17.374 | 17.492 | 2.647x | 0.923x |
| mkl_bf16_inputs | 18.046 | 17.737 | 17.972 | 18.103 | 2.554x | 0.891x |
| original | 46.086 | 45.265 | 45.324 | 47.184 | 1.000x | 0.349x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.331e-03 | 3.697e-01 | 5.297e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.305e-03 | 3.697e-01 | 5.275e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 1.919e-07 | 3.815e-05 | 5.466e-07 | 1 |
| original | kernel_full | mkl_fp32 | 5.305e-03 | 3.697e-01 | 5.275e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 1.919e-07 | 3.815e-05 | 5.466e-07 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.305e-03 | 3.697e-01 | 5.275e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.363e-03 | 1.648e-01 | 2.362e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 7.080e-03 | 4.694e-01 | 6.698e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.925e-07 | 4.578e-05 | 6.559e-07 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.305e-03 | 3.697e-01 | 5.275e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.869e-03 | 1.925e-01 | 2.759e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.618e-03 | 5.556e-01 | 7.927e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.823e-07 | 4.196e-05 | 6.013e-07 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.305e-03 | 3.697e-01 | 5.275e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.026e-03 | 2.178e-01 | 3.121e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.804e-03 | 5.260e-01 | 7.506e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 5.410e-07 | 1.678e-04 | 2.405e-06 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.305e-03 | 3.697e-01 | 5.275e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.187e-03 | 2.332e-01 | 3.342e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 7.991e-03 | 5.878e-01 | 8.387e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 9.360e-07 | 1.831e-04 | 2.624e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.305e-03 | 3.697e-01 | 5.276e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.187e-03 | 2.332e-01 | 3.342e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 7.991e-03 | 5.878e-01 | 8.387e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 9.360e-07 | 1.831e-04 | 2.624e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.305e-03 | 3.697e-01 | 5.276e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.187e-03 | 2.332e-01 | 3.342e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 7.991e-03 | 5.878e-01 | 8.387e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 9.360e-07 | 1.831e-04 | 2.624e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.305e-03 | 3.697e-01 | 5.276e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.187e-03 | 2.332e-01 | 3.342e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.991e-03 | 5.878e-01 | 8.387e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 9.360e-07 | 1.831e-04 | 2.624e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.305e-03 | 3.697e-01 | 5.276e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.363e-03 | 1.648e-01 | 2.362e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.925e-07 | 4.578e-05 | 6.559e-07 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.869e-03 | 1.925e-01 | 2.759e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.823e-07 | 4.196e-05 | 6.013e-07 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.026e-03 | 2.178e-01 | 3.121e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 5.410e-07 | 1.678e-04 | 2.405e-06 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.187e-03 | 2.332e-01 | 3.342e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 9.360e-07 | 1.831e-04 | 2.624e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.187e-03 | 2.332e-01 | 3.342e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 9.360e-07 | 1.831e-04 | 2.624e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.187e-03 | 2.332e-01 | 3.342e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 9.360e-07 | 1.831e-04 | 2.624e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.187e-03 | 2.332e-01 | 3.342e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 9.360e-07 | 1.831e-04 | 2.624e-06 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.041e-02 | 2.724e+01 | 1.051e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.031e-02 | 2.724e+01 | 1.040e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 1.269e-05 | 2.500e-01 | 9.644e-05 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.031e-02 | 2.724e+01 | 1.040e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 1.269e-05 | 2.500e-01 | 9.644e-05 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.031e-02 | 2.724e+01 | 1.040e-02 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.606e-03 | 9.175e+00 | 3.539e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.344e-02 | 3.513e+01 | 1.341e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.260e-05 | 2.501e-01 | 9.646e-05 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.031e-02 | 2.724e+01 | 1.040e-02 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.792e-03 | 1.281e+01 | 4.940e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.466e-02 | 3.801e+01 | 1.451e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.202e-05 | 2.501e-01 | 9.646e-05 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.031e-02 | 2.724e+01 | 1.040e-02 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.334e-03 | 1.424e+01 | 5.491e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.517e-02 | 4.097e+01 | 1.564e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.406e-05 | 2.879e-01 | 1.110e-04 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.031e-02 | 2.724e+01 | 1.040e-02 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.819e-03 | 1.616e+01 | 6.232e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.563e-02 | 4.257e+01 | 1.625e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.945e-05 | 2.577e-01 | 9.941e-05 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.031e-02 | 2.724e+01 | 1.040e-02 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.819e-03 | 1.616e+01 | 6.232e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.563e-02 | 4.257e+01 | 1.625e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.945e-05 | 2.577e-01 | 9.941e-05 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.031e-02 | 2.724e+01 | 1.040e-02 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.819e-03 | 1.616e+01 | 6.232e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.563e-02 | 4.257e+01 | 1.625e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.945e-05 | 2.577e-01 | 9.941e-05 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.031e-02 | 2.724e+01 | 1.040e-02 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.819e-03 | 1.616e+01 | 6.232e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.563e-02 | 4.257e+01 | 1.625e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.945e-05 | 2.577e-01 | 9.941e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.031e-02 | 2.724e+01 | 1.040e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 16.8650 | 100 | 21.675 |
| original | row_schedule | 0.0248 | 100 | 21.675 |
| original | partial_zero | 0.0389 | 100 | 21.675 |
| original | csr_prefetch | 0.1640 | 100 | 21.675 |
| original | feature_gather_decode | 0.1101 | 100 | 21.675 |
| original | fp32_neighbor_reduction | 0.0000 | 100 | 21.675 |
| original | partial_bf16_conversion | 0.0000 | 100 | 21.675 |
| original | tile_load | 0.5047 | 100 | 21.675 |
| original | tile_compute | 0.5808 | 100 | 21.675 |
| original | tile_store | 0.0281 | 100 | 21.675 |
| original | output_scatter | 0.0439 | 100 | 21.675 |
| original | thread_amx_setup | 4.2524 | 100 | 21.675 |

## as-Skitter

N=1696415, E=22190596, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 30.981 | 29.535 | 30.517 | 31.429 | 3.738x | 1.387x |
| shared_b64_fast_nozero | 31.472 | 29.873 | 30.671 | 31.579 | 3.680x | 1.365x |
| shared_b32_fast_nozero | 31.903 | 30.538 | 31.764 | 32.350 | 3.630x | 1.347x |
| shared_bfull_accurate_nozero | 32.636 | 31.518 | 32.289 | 32.950 | 3.549x | 1.316x |
| shared_b64_accurate_nozero | 33.472 | 32.112 | 32.787 | 33.738 | 3.460x | 1.283x |
| shared_b32_accurate_nozero | 34.336 | 33.659 | 33.900 | 35.089 | 3.373x | 1.251x |
| shared_b16_fast_nozero | 34.958 | 32.616 | 34.255 | 35.586 | 3.313x | 1.229x |
| shared_b16_accurate_nozero | 38.050 | 36.294 | 36.954 | 38.608 | 3.044x | 1.129x |
| shared_b8_fast_nozero | 40.858 | 38.818 | 40.249 | 41.367 | 2.835x | 1.051x |
| mkl_fp32 | 42.958 | 42.787 | 42.875 | 43.007 | 2.696x | 1.000x |
| mkl_bf16_inputs | 43.197 | 42.844 | 43.033 | 43.522 | 2.681x | 0.994x |
| shared_b8_accurate_nozero | 45.375 | 44.360 | 44.722 | 46.954 | 2.553x | 0.947x |
| original_nozero | 46.349 | 43.458 | 45.851 | 46.976 | 2.499x | 0.927x |
| shared_b4_fast_nozero | 51.231 | 47.465 | 50.178 | 51.931 | 2.261x | 0.839x |
| shared_b4_accurate_nozero | 61.167 | 57.029 | 60.394 | 61.813 | 1.894x | 0.702x |
| shared_b2_fast_nozero | 70.396 | 67.593 | 69.180 | 71.250 | 1.645x | 0.610x |
| shared_b2_accurate_nozero | 89.589 | 85.078 | 87.651 | 92.653 | 1.293x | 0.479x |
| original | 115.821 | 111.440 | 113.982 | 116.805 | 1.000x | 0.371x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 69.295 | 67.867 | 68.607 | 70.032 | 3.447x | 1.290x |
| shared_b64_fast_nozero | 70.025 | 68.932 | 69.441 | 70.920 | 3.411x | 1.277x |
| shared_bfull_accurate_nozero | 70.948 | 69.739 | 70.416 | 71.171 | 3.367x | 1.260x |
| shared_b32_fast_nozero | 72.049 | 69.977 | 71.725 | 72.367 | 3.315x | 1.241x |
| shared_b64_accurate_nozero | 73.263 | 72.643 | 72.892 | 75.251 | 3.260x | 1.220x |
| shared_b32_accurate_nozero | 76.949 | 74.783 | 76.425 | 78.803 | 3.104x | 1.162x |
| shared_b16_fast_nozero | 78.169 | 76.437 | 77.548 | 79.471 | 3.056x | 1.144x |
| shared_b16_accurate_nozero | 84.147 | 82.288 | 83.376 | 86.092 | 2.839x | 1.062x |
| shared_b8_fast_nozero | 88.027 | 85.192 | 86.978 | 88.572 | 2.714x | 1.016x |
| mkl_fp32 | 89.399 | 89.238 | 89.350 | 89.481 | 2.672x | 1.000x |
| mkl_bf16_inputs | 96.294 | 95.552 | 96.057 | 96.766 | 2.481x | 0.928x |
| original_nozero | 97.995 | 94.712 | 97.087 | 99.121 | 2.438x | 0.912x |
| shared_b8_accurate_nozero | 99.630 | 94.210 | 97.249 | 100.629 | 2.398x | 0.897x |
| shared_b4_fast_nozero | 108.839 | 106.704 | 107.970 | 111.565 | 2.195x | 0.821x |
| shared_b4_accurate_nozero | 128.142 | 126.305 | 127.336 | 130.165 | 1.864x | 0.698x |
| shared_b2_fast_nozero | 146.586 | 142.393 | 144.647 | 148.829 | 1.630x | 0.610x |
| shared_b2_accurate_nozero | 187.534 | 183.501 | 186.356 | 189.461 | 1.274x | 0.477x |
| original | 238.865 | 235.939 | 237.316 | 240.416 | 1.000x | 0.374x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.081e-03 | 1.745e+01 | 3.720e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.058e-03 | 1.745e+01 | 3.708e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 1.906e-06 | 4.102e-02 | 8.742e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.058e-03 | 1.745e+01 | 3.707e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 1.906e-06 | 4.102e-02 | 8.742e-06 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.058e-03 | 1.745e+01 | 3.707e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.267e-03 | 8.567e+00 | 1.826e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 6.764e-03 | 2.474e+01 | 5.256e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.914e-06 | 4.395e-02 | 9.367e-06 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.058e-03 | 1.745e+01 | 3.707e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.870e-03 | 1.110e+01 | 2.366e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.399e-03 | 2.618e+01 | 5.561e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.339e-06 | 3.296e-02 | 7.025e-06 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.058e-03 | 1.746e+01 | 3.709e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.057e-03 | 1.223e+01 | 2.606e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.616e-03 | 2.683e+01 | 5.699e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.090e-06 | 2.197e-02 | 4.683e-06 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.058e-03 | 1.747e+01 | 3.711e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.147e-03 | 1.318e+01 | 2.809e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 7.712e-03 | 3.063e+01 | 6.508e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.637e-06 | 1.392e-02 | 2.966e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.058e-03 | 1.745e+01 | 3.707e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.165e-03 | 1.431e+01 | 3.050e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 7.734e-03 | 2.941e+01 | 6.248e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.759e-06 | 1.587e-02 | 3.382e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.059e-03 | 1.746e+01 | 3.710e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.173e-03 | 1.334e+01 | 2.843e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 7.743e-03 | 2.906e+01 | 6.174e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.960e-06 | 1.855e-02 | 3.955e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.059e-03 | 1.746e+01 | 3.710e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.222e-03 | 1.435e+01 | 3.059e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.763e-03 | 2.851e+01 | 6.058e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 6.147e-06 | 4.688e-02 | 9.991e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.060e-03 | 1.748e+01 | 3.714e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.267e-03 | 8.567e+00 | 1.826e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.914e-06 | 4.395e-02 | 9.367e-06 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.870e-03 | 1.110e+01 | 2.366e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.339e-06 | 3.296e-02 | 7.025e-06 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.057e-03 | 1.223e+01 | 2.606e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.090e-06 | 2.197e-02 | 4.683e-06 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.147e-03 | 1.318e+01 | 2.809e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.637e-06 | 1.392e-02 | 2.966e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.165e-03 | 1.431e+01 | 3.050e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.759e-06 | 1.587e-02 | 3.382e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.173e-03 | 1.334e+01 | 2.843e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.960e-06 | 1.855e-02 | 3.955e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.222e-03 | 1.435e+01 | 3.059e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 6.147e-06 | 4.688e-02 | 9.991e-06 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 9.534e-03 | 6.155e+04 | 9.830e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 9.447e-03 | 6.155e+04 | 9.740e-03 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 4.416e-05 | 1.020e+02 | 1.629e-05 | 1 |
| original | two_layer_e2e | mkl_fp32 | 9.446e-03 | 6.151e+04 | 9.734e-03 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 4.416e-05 | 1.020e+02 | 1.629e-05 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 9.446e-03 | 6.151e+04 | 9.734e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.327e-03 | 2.145e+04 | 3.426e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.253e-02 | 8.018e+04 | 1.269e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 4.312e-05 | 1.290e+03 | 2.060e-04 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.448e-03 | 6.284e+04 | 9.944e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.813e-03 | 3.071e+04 | 4.905e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.401e-02 | 8.868e+04 | 1.403e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.673e-05 | 1.305e+02 | 2.084e-05 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.447e-03 | 6.160e+04 | 9.749e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.443e-03 | 3.345e+04 | 5.342e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.462e-02 | 9.311e+04 | 1.473e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.487e-05 | 4.900e+01 | 7.826e-06 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.446e-03 | 6.153e+04 | 9.737e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.681e-03 | 3.461e+04 | 5.528e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.483e-02 | 9.448e+04 | 1.495e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.062e-05 | 2.550e+01 | 4.073e-06 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.448e-03 | 6.155e+04 | 9.740e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.855e-03 | 3.493e+04 | 5.579e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.496e-02 | 9.470e+04 | 1.499e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.925e-05 | 3.400e+01 | 5.430e-06 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.451e-03 | 6.158e+04 | 9.746e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.898e-03 | 3.507e+04 | 5.601e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.499e-02 | 9.456e+04 | 1.496e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.534e-05 | 4.000e+01 | 6.389e-06 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.453e-03 | 6.158e+04 | 9.745e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.974e-03 | 3.855e+04 | 6.156e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.500e-02 | 9.938e+04 | 1.573e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 5.554e-05 | 8.400e+01 | 1.342e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.454e-03 | 6.161e+04 | 9.749e-03 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 67.8630 | 416 | 178.896 |
| original | row_schedule | 0.1068 | 416 | 178.896 |
| original | partial_zero | 0.1502 | 416 | 178.896 |
| original | csr_prefetch | 6.0399 | 416 | 178.896 |
| original | feature_gather_decode | 4.0114 | 416 | 178.896 |
| original | fp32_neighbor_reduction | 0.0000 | 416 | 178.896 |
| original | partial_bf16_conversion | 0.0000 | 416 | 178.896 |
| original | tile_load | 20.6711 | 416 | 178.896 |
| original | tile_compute | 21.7276 | 416 | 178.896 |
| original | tile_store | 0.1662 | 416 | 178.896 |
| original | output_scatter | 0.1218 | 416 | 178.896 |
| original | thread_amx_setup | 4.2052 | 416 | 178.896 |

## cit-Patents

N=3774768, E=16518948, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b64_fast_nozero | 32.335 | 32.242 | 32.263 | 32.424 | 6.172x | 2.018x |
| shared_b32_fast_nozero | 32.375 | 32.272 | 32.322 | 32.413 | 6.165x | 2.016x |
| shared_bfull_fast_nozero | 32.871 | 32.169 | 32.304 | 32.960 | 6.071x | 1.985x |
| shared_b16_fast_nozero | 32.896 | 32.786 | 32.864 | 32.977 | 6.067x | 1.984x |
| original_nozero | 33.448 | 33.380 | 33.415 | 33.729 | 5.967x | 1.951x |
| shared_b32_accurate_nozero | 34.781 | 34.668 | 34.702 | 34.828 | 5.738x | 1.876x |
| shared_b64_accurate_nozero | 34.865 | 34.446 | 34.561 | 35.397 | 5.724x | 1.872x |
| shared_b8_fast_nozero | 35.159 | 34.987 | 35.035 | 35.258 | 5.676x | 1.856x |
| shared_b16_accurate_nozero | 35.284 | 35.135 | 35.245 | 35.352 | 5.656x | 1.849x |
| shared_bfull_accurate_nozero | 35.331 | 34.576 | 35.144 | 35.385 | 5.649x | 1.847x |
| shared_b8_accurate_nozero | 37.420 | 37.233 | 37.315 | 37.482 | 5.333x | 1.744x |
| shared_b4_fast_nozero | 38.948 | 38.653 | 38.822 | 39.071 | 5.124x | 1.675x |
| shared_b4_accurate_nozero | 42.475 | 42.246 | 42.356 | 42.708 | 4.699x | 1.536x |
| shared_b2_fast_nozero | 46.680 | 46.312 | 46.513 | 46.923 | 4.275x | 1.398x |
| shared_b2_accurate_nozero | 53.028 | 52.729 | 52.838 | 53.696 | 3.764x | 1.231x |
| mkl_fp32 | 65.255 | 65.019 | 65.213 | 65.397 | 3.058x | 1.000x |
| mkl_bf16_inputs | 67.091 | 64.684 | 66.818 | 67.175 | 2.975x | 0.973x |
| original | 199.577 | 187.457 | 194.447 | 205.844 | 1.000x | 0.327x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b32_fast_nozero | 79.987 | 79.546 | 79.910 | 80.206 | 5.023x | 1.749x |
| shared_b64_fast_nozero | 80.577 | 80.003 | 80.473 | 80.733 | 4.986x | 1.736x |
| shared_bfull_fast_nozero | 80.667 | 80.457 | 80.591 | 80.721 | 4.981x | 1.734x |
| shared_b16_fast_nozero | 81.096 | 80.887 | 80.988 | 81.311 | 4.954x | 1.725x |
| original_nozero | 81.797 | 81.439 | 81.610 | 82.087 | 4.912x | 1.710x |
| shared_b32_accurate_nozero | 85.498 | 84.814 | 85.125 | 85.954 | 4.699x | 1.636x |
| shared_b8_fast_nozero | 85.812 | 85.521 | 85.761 | 86.138 | 4.682x | 1.630x |
| shared_b64_accurate_nozero | 85.890 | 85.242 | 85.840 | 86.221 | 4.678x | 1.629x |
| shared_bfull_accurate_nozero | 85.968 | 85.683 | 85.871 | 86.038 | 4.674x | 1.627x |
| shared_b16_accurate_nozero | 86.355 | 85.967 | 86.143 | 86.482 | 4.653x | 1.620x |
| shared_b8_accurate_nozero | 91.256 | 90.690 | 90.901 | 91.357 | 4.403x | 1.533x |
| shared_b4_fast_nozero | 94.097 | 93.664 | 93.840 | 94.167 | 4.270x | 1.487x |
| shared_b4_accurate_nozero | 101.289 | 100.938 | 101.012 | 101.621 | 3.967x | 1.381x |
| shared_b2_fast_nozero | 108.670 | 107.686 | 108.067 | 108.861 | 3.697x | 1.287x |
| shared_b2_accurate_nozero | 121.945 | 121.202 | 121.592 | 122.425 | 3.295x | 1.147x |
| mkl_fp32 | 139.908 | 138.839 | 139.811 | 140.403 | 2.872x | 1.000x |
| mkl_bf16_inputs | 158.553 | 154.427 | 158.339 | 158.971 | 2.534x | 0.882x |
| original | 401.770 | 389.910 | 400.324 | 402.883 | 1.000x | 0.348x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.303e-03 | 1.916e+00 | 5.901e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.277e-03 | 1.916e+00 | 5.880e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 2.248e-07 | 4.578e-04 | 1.410e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.277e-03 | 1.916e+00 | 5.879e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 2.248e-07 | 4.578e-04 | 1.410e-06 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.277e-03 | 1.916e+00 | 5.879e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.310e-03 | 7.239e-01 | 2.230e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 6.982e-03 | 2.150e+00 | 6.599e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.259e-07 | 6.104e-04 | 1.880e-06 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.277e-03 | 1.916e+00 | 5.880e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.889e-03 | 1.007e+00 | 3.101e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.603e-03 | 2.443e+00 | 7.498e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.009e-07 | 4.120e-04 | 1.269e-06 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.277e-03 | 1.916e+00 | 5.880e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.070e-03 | 9.622e-01 | 2.964e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.823e-03 | 2.575e+00 | 7.903e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 5.011e-07 | 3.052e-04 | 9.400e-07 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.278e-03 | 1.916e+00 | 5.880e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.149e-03 | 1.137e+00 | 3.503e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 7.914e-03 | 3.053e+00 | 9.369e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.243e-06 | 6.475e-04 | 1.994e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.278e-03 | 1.916e+00 | 5.880e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.161e-03 | 1.056e+00 | 3.253e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 7.928e-03 | 2.635e+00 | 8.086e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.815e-06 | 1.617e-03 | 4.982e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.278e-03 | 1.916e+00 | 5.880e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.165e-03 | 1.039e+00 | 3.199e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 7.932e-03 | 2.793e+00 | 8.571e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.159e-06 | 1.740e-03 | 5.358e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.278e-03 | 1.917e+00 | 5.882e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.164e-03 | 1.148e+00 | 3.535e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.932e-03 | 2.785e+00 | 8.546e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.308e-06 | 3.105e-03 | 9.564e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.278e-03 | 1.918e+00 | 5.885e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.310e-03 | 7.239e-01 | 2.230e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.259e-07 | 6.104e-04 | 1.880e-06 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.889e-03 | 1.007e+00 | 3.101e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.009e-07 | 4.120e-04 | 1.269e-06 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.070e-03 | 9.622e-01 | 2.964e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 5.011e-07 | 3.052e-04 | 9.400e-07 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.149e-03 | 1.137e+00 | 3.503e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.243e-06 | 6.475e-04 | 1.994e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.161e-03 | 1.056e+00 | 3.253e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.815e-06 | 1.617e-03 | 4.982e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.165e-03 | 1.039e+00 | 3.199e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.159e-06 | 1.740e-03 | 5.358e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.164e-03 | 1.148e+00 | 3.535e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.308e-06 | 3.105e-03 | 9.564e-06 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.028e-02 | 4.409e+02 | 9.407e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.018e-02 | 4.409e+02 | 9.322e-03 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 1.273e-05 | 4.943e-01 | 1.055e-05 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.018e-02 | 4.410e+02 | 9.323e-03 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 1.273e-05 | 4.943e-01 | 1.055e-05 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.018e-02 | 4.410e+02 | 9.323e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.076e-03 | 1.214e+02 | 2.589e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.283e-02 | 5.521e+02 | 1.167e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.371e-05 | 5.003e-01 | 1.067e-05 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.018e-02 | 4.409e+02 | 9.321e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.533e-03 | 1.946e+02 | 4.151e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.433e-02 | 6.253e+02 | 1.322e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.221e-05 | 5.010e-01 | 1.069e-05 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.018e-02 | 4.410e+02 | 9.322e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.244e-03 | 2.297e+02 | 4.900e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.504e-02 | 6.604e+02 | 1.396e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.355e-05 | 1.007e+00 | 2.149e-05 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.018e-02 | 4.410e+02 | 9.323e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.552e-03 | 2.391e+02 | 5.101e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.532e-02 | 6.699e+02 | 1.416e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.257e-05 | 1.001e+00 | 2.136e-05 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.018e-02 | 4.408e+02 | 9.319e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.664e-03 | 2.390e+02 | 5.098e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.541e-02 | 6.669e+02 | 1.410e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.774e-05 | 1.010e+00 | 2.154e-05 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.018e-02 | 4.408e+02 | 9.319e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.720e-03 | 2.442e+02 | 5.210e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.544e-02 | 6.750e+02 | 1.427e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.982e-05 | 1.106e+00 | 2.359e-05 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.018e-02 | 4.409e+02 | 9.321e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.765e-03 | 2.553e+02 | 5.446e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.546e-02 | 6.862e+02 | 1.451e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.051e-05 | 1.089e+00 | 2.324e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.018e-02 | 4.410e+02 | 9.323e-03 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 151.1421 | 923 | 186.544 |
| original | row_schedule | 0.2387 | 923 | 186.544 |
| original | partial_zero | 0.2968 | 923 | 186.544 |
| original | csr_prefetch | 1.4355 | 923 | 186.544 |
| original | feature_gather_decode | 0.8290 | 923 | 186.544 |
| original | fp32_neighbor_reduction | 0.0000 | 923 | 186.544 |
| original | partial_bf16_conversion | 0.0000 | 923 | 186.544 |
| original | tile_load | 2.6891 | 923 | 186.544 |
| original | tile_compute | 4.6470 | 923 | 186.544 |
| original | tile_store | 0.3533 | 923 | 186.544 |
| original | output_scatter | 0.3078 | 923 | 186.544 |
| original | thread_amx_setup | 3.9856 | 923 | 186.544 |

## com-Friendster

N=None, E=None, threads=None, status=INCOMPLETE

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|

## com-LiveJournal

N=3997962, E=69362378, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 90.294 | 89.008 | 89.524 | 91.059 | 3.030x | 1.410x |
| shared_b64_fast_nozero | 91.931 | 89.318 | 90.238 | 92.766 | 2.976x | 1.385x |
| shared_b32_fast_nozero | 93.899 | 91.215 | 92.177 | 94.525 | 2.913x | 1.356x |
| shared_bfull_accurate_nozero | 95.924 | 93.692 | 94.493 | 96.609 | 2.852x | 1.327x |
| original_nozero | 96.145 | 94.453 | 94.867 | 99.384 | 2.845x | 1.324x |
| shared_b64_accurate_nozero | 97.285 | 95.359 | 95.583 | 98.525 | 2.812x | 1.309x |
| shared_b16_fast_nozero | 98.482 | 96.764 | 97.251 | 99.522 | 2.778x | 1.293x |
| shared_b32_accurate_nozero | 99.249 | 98.082 | 98.873 | 101.288 | 2.756x | 1.283x |
| shared_b16_accurate_nozero | 104.625 | 103.327 | 103.759 | 106.292 | 2.615x | 1.217x |
| shared_b8_fast_nozero | 109.637 | 105.912 | 106.647 | 110.351 | 2.495x | 1.161x |
| shared_b8_accurate_nozero | 117.212 | 113.066 | 114.419 | 118.591 | 2.334x | 1.086x |
| shared_b4_fast_nozero | 122.177 | 121.450 | 121.849 | 124.031 | 2.239x | 1.042x |
| mkl_fp32 | 127.330 | 126.956 | 127.202 | 127.475 | 2.148x | 1.000x |
| mkl_bf16_inputs | 129.650 | 126.343 | 129.445 | 129.734 | 2.110x | 0.982x |
| shared_b4_accurate_nozero | 139.502 | 135.976 | 137.277 | 141.010 | 1.961x | 0.913x |
| shared_b2_fast_nozero | 153.259 | 151.363 | 151.881 | 158.677 | 1.785x | 0.831x |
| shared_b2_accurate_nozero | 185.280 | 179.869 | 183.335 | 190.224 | 1.476x | 0.687x |
| original | 273.561 | 268.086 | 269.309 | 274.940 | 1.000x | 0.465x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 197.783 | 195.689 | 197.642 | 198.040 | 2.769x | 1.340x |
| shared_b64_fast_nozero | 198.379 | 196.250 | 196.882 | 198.976 | 2.760x | 1.336x |
| shared_b32_fast_nozero | 203.123 | 200.431 | 202.762 | 203.332 | 2.696x | 1.305x |
| shared_bfull_accurate_nozero | 207.981 | 205.387 | 206.392 | 209.515 | 2.633x | 1.275x |
| shared_b64_accurate_nozero | 212.034 | 207.561 | 210.019 | 212.699 | 2.583x | 1.250x |
| original_nozero | 212.151 | 208.022 | 211.104 | 213.603 | 2.581x | 1.250x |
| shared_b16_fast_nozero | 215.876 | 211.937 | 212.995 | 218.104 | 2.537x | 1.228x |
| shared_b32_accurate_nozero | 217.195 | 213.436 | 216.320 | 219.246 | 2.521x | 1.221x |
| shared_b16_accurate_nozero | 229.775 | 224.968 | 226.723 | 230.247 | 2.383x | 1.154x |
| shared_b8_fast_nozero | 234.797 | 229.814 | 233.749 | 236.534 | 2.332x | 1.129x |
| shared_b8_accurate_nozero | 251.362 | 245.753 | 246.507 | 253.072 | 2.179x | 1.055x |
| mkl_fp32 | 265.090 | 264.551 | 264.698 | 265.338 | 2.066x | 1.000x |
| shared_b4_fast_nozero | 269.322 | 264.394 | 268.327 | 270.280 | 2.033x | 0.984x |
| mkl_bf16_inputs | 278.528 | 278.118 | 278.377 | 278.673 | 1.966x | 0.952x |
| shared_b4_accurate_nozero | 300.470 | 291.376 | 296.103 | 302.561 | 1.822x | 0.882x |
| shared_b2_fast_nozero | 328.617 | 321.321 | 328.127 | 334.012 | 1.666x | 0.807x |
| shared_b2_accurate_nozero | 387.969 | 382.421 | 383.173 | 389.691 | 1.411x | 0.683x |
| original | 547.594 | 538.298 | 546.494 | 548.599 | 1.000x | 0.484x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.297e-03 | 7.181e+00 | 4.499e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.272e-03 | 7.181e+00 | 4.485e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 4.543e-07 | 1.270e-02 | 7.953e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.272e-03 | 7.194e+00 | 4.493e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 4.543e-07 | 1.270e-02 | 7.953e-06 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.272e-03 | 7.194e+00 | 4.493e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.344e-03 | 3.079e+00 | 1.929e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 7.024e-03 | 9.375e+00 | 5.855e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.570e-07 | 1.117e-02 | 6.998e-06 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.272e-03 | 7.179e+00 | 4.484e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.935e-03 | 4.806e+00 | 3.011e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.656e-03 | 1.081e+01 | 6.749e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.535e-07 | 7.202e-03 | 4.512e-06 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.272e-03 | 7.180e+00 | 4.484e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.114e-03 | 4.695e+00 | 2.941e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.872e-03 | 1.130e+01 | 7.060e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 6.173e-07 | 5.249e-03 | 3.288e-06 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.272e-03 | 7.184e+00 | 4.487e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.210e-03 | 4.808e+00 | 3.012e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 7.977e-03 | 1.199e+01 | 7.488e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.710e-06 | 3.784e-03 | 2.371e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.272e-03 | 7.180e+00 | 4.484e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.227e-03 | 5.263e+00 | 3.297e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 7.998e-03 | 1.117e+01 | 6.978e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.083e-06 | 5.981e-03 | 3.747e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.272e-03 | 7.182e+00 | 4.485e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.244e-03 | 5.597e+00 | 3.506e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 8.015e-03 | 1.145e+01 | 7.153e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.351e-06 | 8.301e-03 | 5.200e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.273e-03 | 7.186e+00 | 4.488e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.248e-03 | 5.438e+00 | 3.407e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.018e-03 | 1.120e+01 | 6.993e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 5.351e-06 | 1.672e-02 | 1.048e-05 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.274e-03 | 7.186e+00 | 4.488e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.344e-03 | 3.079e+00 | 1.929e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.570e-07 | 1.117e-02 | 6.998e-06 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.935e-03 | 4.806e+00 | 3.011e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.535e-07 | 7.202e-03 | 4.512e-06 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.114e-03 | 4.695e+00 | 2.941e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 6.173e-07 | 5.249e-03 | 3.288e-06 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.210e-03 | 4.808e+00 | 3.012e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.710e-06 | 3.784e-03 | 2.371e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.227e-03 | 5.263e+00 | 3.297e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.083e-06 | 5.981e-03 | 3.747e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.244e-03 | 5.597e+00 | 3.506e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.351e-06 | 8.301e-03 | 5.200e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.248e-03 | 5.438e+00 | 3.407e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 5.351e-06 | 1.672e-02 | 1.048e-05 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.008e-02 | 3.122e+04 | 1.014e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 9.985e-03 | 3.122e+04 | 1.004e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 5.928e-06 | 2.414e+01 | 7.845e-06 | 1 |
| original | two_layer_e2e | mkl_fp32 | 9.985e-03 | 3.121e+04 | 1.004e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 5.928e-06 | 2.414e+01 | 7.845e-06 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 9.985e-03 | 3.121e+04 | 1.004e-02 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.137e-03 | 8.849e+03 | 2.876e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.299e-02 | 4.006e+04 | 1.289e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 9.750e-06 | 5.750e+01 | 1.869e-05 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.985e-03 | 3.127e+04 | 1.006e-02 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.652e-03 | 1.388e+04 | 4.511e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.451e-02 | 4.510e+04 | 1.451e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 5.740e-06 | 1.650e+01 | 5.362e-06 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.985e-03 | 3.122e+04 | 1.004e-02 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.274e-03 | 1.619e+04 | 5.260e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.513e-02 | 4.740e+04 | 1.525e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 7.988e-06 | 1.075e+01 | 3.493e-06 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.985e-03 | 3.121e+04 | 1.004e-02 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.530e-03 | 1.695e+04 | 5.507e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.538e-02 | 4.816e+04 | 1.549e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 8.483e-06 | 7.250e+00 | 2.356e-06 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.986e-03 | 3.122e+04 | 1.004e-02 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.607e-03 | 1.700e+04 | 5.523e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.544e-02 | 4.821e+04 | 1.551e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.191e-05 | 1.475e+01 | 4.793e-06 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.987e-03 | 3.123e+04 | 1.005e-02 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.683e-03 | 1.725e+04 | 5.607e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.550e-02 | 4.847e+04 | 1.559e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.787e-05 | 2.050e+01 | 6.662e-06 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.991e-03 | 3.123e+04 | 1.005e-02 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.874e-03 | 1.692e+04 | 5.498e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.559e-02 | 4.772e+04 | 1.535e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.329e-05 | 3.725e+01 | 1.210e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.996e-03 | 3.123e+04 | 1.005e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 160.1589 | 978 | 285.895 |
| original | row_schedule | 0.3059 | 978 | 285.895 |
| original | partial_zero | 0.3383 | 978 | 285.895 |
| original | csr_prefetch | 8.0192 | 978 | 285.895 |
| original | feature_gather_decode | 3.5906 | 978 | 285.895 |
| original | fp32_neighbor_reduction | 0.0000 | 978 | 285.895 |
| original | partial_bf16_conversion | 0.0000 | 978 | 285.895 |
| original | tile_load | 17.6420 | 978 | 285.895 |
| original | tile_compute | 23.3400 | 978 | 285.895 |
| original | tile_store | 0.3963 | 978 | 285.895 |
| original | output_scatter | 0.3462 | 978 | 285.895 |
| original | thread_amx_setup | 4.1945 | 978 | 285.895 |

## com-Youtube

N=1134890, E=5975248, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 12.226 | 11.725 | 12.063 | 12.311 | 5.653x | 1.587x |
| shared_bfull_accurate_nozero | 12.713 | 12.464 | 12.557 | 12.935 | 5.437x | 1.526x |
| shared_b64_fast_nozero | 12.806 | 12.277 | 12.434 | 12.899 | 5.397x | 1.515x |
| shared_b32_fast_nozero | 13.403 | 12.896 | 13.262 | 13.497 | 5.157x | 1.447x |
| shared_b64_accurate_nozero | 13.788 | 12.981 | 13.662 | 13.825 | 5.013x | 1.407x |
| shared_b16_fast_nozero | 14.629 | 13.998 | 14.442 | 15.053 | 4.725x | 1.326x |
| shared_b32_accurate_nozero | 14.774 | 13.773 | 14.552 | 14.984 | 4.678x | 1.313x |
| shared_b16_accurate_nozero | 16.536 | 16.108 | 16.186 | 16.715 | 4.180x | 1.173x |
| shared_b8_fast_nozero | 17.270 | 15.737 | 16.458 | 17.587 | 4.002x | 1.123x |
| mkl_bf16_inputs | 19.370 | 19.329 | 19.355 | 19.403 | 3.568x | 1.001x |
| mkl_fp32 | 19.399 | 19.379 | 19.384 | 19.414 | 3.563x | 1.000x |
| shared_b8_accurate_nozero | 20.372 | 19.334 | 20.067 | 20.503 | 3.393x | 0.952x |
| original_nozero | 21.087 | 20.789 | 20.927 | 21.333 | 3.278x | 0.920x |
| shared_b4_fast_nozero | 21.399 | 20.182 | 21.073 | 21.836 | 3.230x | 0.906x |
| shared_b4_accurate_nozero | 26.864 | 25.848 | 26.213 | 27.255 | 2.573x | 0.722x |
| shared_b2_fast_nozero | 30.272 | 29.005 | 29.330 | 30.577 | 2.283x | 0.641x |
| shared_b2_accurate_nozero | 41.448 | 39.410 | 40.169 | 41.858 | 1.668x | 0.468x |
| original | 69.116 | 68.078 | 69.014 | 69.264 | 1.000x | 0.281x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 30.099 | 29.377 | 29.868 | 30.162 | 4.617x | 1.361x |
| shared_bfull_accurate_nozero | 31.755 | 30.533 | 31.087 | 32.071 | 4.376x | 1.290x |
| shared_b64_fast_nozero | 31.779 | 31.149 | 31.577 | 31.855 | 4.373x | 1.289x |
| shared_b32_fast_nozero | 32.717 | 31.710 | 32.251 | 33.046 | 4.247x | 1.252x |
| shared_b64_accurate_nozero | 33.284 | 32.607 | 33.099 | 33.976 | 4.175x | 1.231x |
| shared_b16_fast_nozero | 34.940 | 34.490 | 34.831 | 35.464 | 3.977x | 1.172x |
| shared_b32_accurate_nozero | 35.434 | 34.256 | 34.808 | 35.870 | 3.922x | 1.156x |
| shared_b16_accurate_nozero | 39.232 | 38.656 | 38.945 | 39.726 | 3.542x | 1.044x |
| shared_b8_fast_nozero | 40.225 | 39.404 | 40.037 | 40.562 | 3.455x | 1.018x |
| mkl_fp32 | 40.965 | 40.721 | 40.884 | 41.116 | 3.392x | 1.000x |
| mkl_bf16_inputs | 45.253 | 44.865 | 45.133 | 45.405 | 3.071x | 0.905x |
| shared_b8_accurate_nozero | 45.975 | 45.321 | 45.563 | 46.563 | 3.023x | 0.891x |
| original_nozero | 46.348 | 45.412 | 45.794 | 47.309 | 2.998x | 0.884x |
| shared_b4_fast_nozero | 49.409 | 48.048 | 49.036 | 49.706 | 2.813x | 0.829x |
| shared_b4_accurate_nozero | 59.319 | 58.289 | 58.849 | 59.947 | 2.343x | 0.691x |
| shared_b2_fast_nozero | 67.019 | 65.312 | 66.511 | 67.184 | 2.073x | 0.611x |
| shared_b2_accurate_nozero | 88.750 | 85.863 | 87.823 | 89.415 | 1.566x | 0.462x |
| original | 138.963 | 137.964 | 138.518 | 139.625 | 1.000x | 0.295x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.246e-03 | 1.246e+01 | 3.887e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.221e-03 | 1.246e+01 | 3.876e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 9.121e-07 | 2.490e-02 | 7.768e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.221e-03 | 1.245e+01 | 3.873e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 9.121e-07 | 2.490e-02 | 7.768e-06 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.221e-03 | 1.245e+01 | 3.873e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.202e-03 | 5.148e+00 | 1.606e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 6.803e-03 | 1.625e+01 | 5.054e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 9.506e-07 | 2.051e-02 | 6.397e-06 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.222e-03 | 1.248e+01 | 3.882e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.750e-03 | 9.881e+00 | 3.082e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.372e-03 | 1.879e+01 | 5.846e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 6.897e-07 | 1.501e-02 | 4.683e-06 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.221e-03 | 1.245e+01 | 3.874e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 2.905e-03 | 7.636e+00 | 2.382e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.553e-03 | 1.936e+01 | 6.021e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 6.871e-07 | 1.147e-02 | 3.579e-06 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.222e-03 | 1.246e+01 | 3.876e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 2.988e-03 | 8.835e+00 | 2.756e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 7.640e-03 | 2.083e+01 | 6.481e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.481e-06 | 8.179e-03 | 2.551e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.222e-03 | 1.246e+01 | 3.875e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.001e-03 | 9.880e+00 | 3.082e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 7.656e-03 | 2.085e+01 | 6.486e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.603e-06 | 8.301e-03 | 2.589e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.222e-03 | 1.246e+01 | 3.877e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.008e-03 | 8.850e+00 | 2.760e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 7.663e-03 | 1.990e+01 | 6.191e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.765e-06 | 9.766e-03 | 3.046e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.223e-03 | 1.247e+01 | 3.878e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.027e-03 | 1.049e+01 | 3.273e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.669e-03 | 1.857e+01 | 5.775e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 5.254e-06 | 2.515e-02 | 7.844e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.223e-03 | 1.248e+01 | 3.883e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.202e-03 | 5.148e+00 | 1.606e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 9.506e-07 | 2.051e-02 | 6.397e-06 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.750e-03 | 9.881e+00 | 3.082e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 6.897e-07 | 1.501e-02 | 4.683e-06 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.905e-03 | 7.636e+00 | 2.382e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 6.871e-07 | 1.147e-02 | 3.579e-06 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.988e-03 | 8.835e+00 | 2.756e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.481e-06 | 8.179e-03 | 2.551e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.001e-03 | 9.880e+00 | 3.082e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.603e-06 | 8.301e-03 | 2.589e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.008e-03 | 8.850e+00 | 2.760e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.765e-06 | 9.766e-03 | 3.046e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.027e-03 | 1.049e+01 | 3.273e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 5.254e-06 | 2.515e-02 | 7.844e-06 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 9.908e-03 | 3.187e+04 | 9.675e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 9.814e-03 | 3.187e+04 | 9.587e-03 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 2.468e-05 | 1.790e+02 | 5.435e-05 | 1 |
| original | two_layer_e2e | mkl_fp32 | 9.815e-03 | 3.205e+04 | 9.641e-03 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 2.468e-05 | 1.790e+02 | 5.435e-05 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 9.815e-03 | 3.205e+04 | 9.641e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.276e-03 | 9.621e+03 | 2.921e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.276e-02 | 4.038e+04 | 1.215e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.574e-05 | 3.120e+02 | 9.473e-05 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.817e-03 | 3.218e+04 | 9.681e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.881e-03 | 1.470e+04 | 4.464e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.437e-02 | 4.489e+04 | 1.351e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 8.217e-06 | 5.847e+01 | 1.775e-05 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.814e-03 | 3.189e+04 | 9.593e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.308e-03 | 1.690e+04 | 5.133e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.483e-02 | 4.709e+04 | 1.417e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 8.676e-06 | 5.744e+01 | 1.744e-05 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.814e-03 | 3.187e+04 | 9.589e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.626e-03 | 1.771e+04 | 5.376e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.513e-02 | 4.789e+04 | 1.441e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.101e-05 | 5.820e+01 | 1.767e-05 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.815e-03 | 3.189e+04 | 9.596e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.707e-03 | 1.793e+04 | 5.444e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.519e-02 | 4.812e+04 | 1.448e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.572e-05 | 5.953e+01 | 1.807e-05 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.817e-03 | 3.190e+04 | 9.596e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.769e-03 | 1.804e+04 | 5.478e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.525e-02 | 4.823e+04 | 1.451e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.096e-05 | 6.169e+01 | 1.873e-05 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.819e-03 | 3.191e+04 | 9.600e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.899e-03 | 1.778e+04 | 5.399e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.526e-02 | 4.953e+04 | 1.490e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.227e-05 | 3.350e+01 | 1.017e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.821e-03 | 3.189e+04 | 9.594e-03 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 45.4512 | 279 | 120.397 |
| original | row_schedule | 0.0496 | 279 | 120.397 |
| original | partial_zero | 0.0765 | 279 | 120.397 |
| original | csr_prefetch | 3.2022 | 279 | 120.397 |
| original | feature_gather_decode | 2.2099 | 279 | 120.397 |
| original | fp32_neighbor_reduction | 0.0000 | 279 | 120.397 |
| original | partial_bf16_conversion | 0.0000 | 279 | 120.397 |
| original | tile_load | 15.1587 | 279 | 120.397 |
| original | tile_compute | 15.0063 | 279 | 120.397 |
| original | tile_store | 0.0942 | 279 | 120.397 |
| original | output_scatter | 0.0916 | 279 | 120.397 |
| original | thread_amx_setup | 3.3090 | 279 | 120.397 |

## email-Enron

N=36692, E=367662, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 0.411 | 0.380 | 0.392 | 0.418 | 7.223x | 1.433x |
| shared_b64_fast_nozero | 0.433 | 0.405 | 0.427 | 0.443 | 6.857x | 1.361x |
| shared_bfull_accurate_nozero | 0.437 | 0.419 | 0.435 | 0.446 | 6.786x | 1.347x |
| shared_b64_accurate_nozero | 0.465 | 0.443 | 0.455 | 0.482 | 6.385x | 1.267x |
| shared_b32_fast_nozero | 0.488 | 0.458 | 0.468 | 0.524 | 6.085x | 1.207x |
| shared_b32_accurate_nozero | 0.517 | 0.493 | 0.509 | 0.539 | 5.748x | 1.141x |
| shared_b16_fast_nozero | 0.536 | 0.506 | 0.514 | 0.574 | 5.543x | 1.100x |
| mkl_fp32 | 0.589 | 0.582 | 0.584 | 0.622 | 5.040x | 1.000x |
| shared_b16_accurate_nozero | 0.612 | 0.581 | 0.607 | 0.669 | 4.848x | 0.962x |
| mkl_bf16_inputs | 0.643 | 0.594 | 0.603 | 0.644 | 4.621x | 0.917x |
| shared_b8_fast_nozero | 0.684 | 0.656 | 0.669 | 0.686 | 4.344x | 0.862x |
| shared_b8_accurate_nozero | 0.855 | 0.846 | 0.851 | 0.858 | 3.473x | 0.689x |
| shared_b4_fast_nozero | 0.992 | 0.939 | 0.977 | 1.016 | 2.992x | 0.594x |
| original_nozero | 1.000 | 0.974 | 0.986 | 1.006 | 2.969x | 0.589x |
| shared_b4_accurate_nozero | 1.328 | 1.269 | 1.302 | 1.357 | 2.236x | 0.444x |
| shared_b2_fast_nozero | 1.441 | 1.411 | 1.438 | 1.449 | 2.060x | 0.409x |
| shared_b2_accurate_nozero | 2.257 | 2.224 | 2.241 | 2.276 | 1.316x | 0.261x |
| original | 2.969 | 2.920 | 2.961 | 2.977 | 1.000x | 0.198x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 1.069 | 1.031 | 1.064 | 1.077 | 4.079x | 1.232x |
| shared_b64_fast_nozero | 1.098 | 1.070 | 1.087 | 1.124 | 3.970x | 1.199x |
| shared_bfull_accurate_nozero | 1.116 | 1.087 | 1.099 | 1.127 | 3.906x | 1.179x |
| shared_b64_accurate_nozero | 1.191 | 1.154 | 1.182 | 1.207 | 3.658x | 1.105x |
| shared_b32_fast_nozero | 1.197 | 1.168 | 1.184 | 1.230 | 3.643x | 1.100x |
| shared_b32_accurate_nozero | 1.264 | 1.245 | 1.257 | 1.273 | 3.447x | 1.041x |
| mkl_fp32 | 1.316 | 1.280 | 1.308 | 1.329 | 3.312x | 1.000x |
| shared_b16_fast_nozero | 1.388 | 1.297 | 1.340 | 1.553 | 3.141x | 0.948x |
| shared_b16_accurate_nozero | 1.468 | 1.443 | 1.453 | 1.477 | 2.969x | 0.896x |
| mkl_bf16_inputs | 1.606 | 1.568 | 1.602 | 1.611 | 2.715x | 0.820x |
| shared_b8_fast_nozero | 1.615 | 1.595 | 1.608 | 1.650 | 2.699x | 0.815x |
| shared_b8_accurate_nozero | 1.945 | 1.909 | 1.930 | 1.958 | 2.242x | 0.677x |
| shared_b4_fast_nozero | 2.151 | 2.117 | 2.140 | 2.195 | 2.026x | 0.612x |
| original_nozero | 2.241 | 2.218 | 2.237 | 2.257 | 1.945x | 0.587x |
| shared_b4_accurate_nozero | 2.905 | 2.881 | 2.894 | 2.937 | 1.501x | 0.453x |
| shared_b2_fast_nozero | 3.157 | 3.070 | 3.125 | 3.179 | 1.381x | 0.417x |
| original | 4.359 | 4.317 | 4.343 | 4.408 | 1.000x | 0.302x |
| shared_b2_accurate_nozero | 4.813 | 4.768 | 4.785 | 4.826 | 0.906x | 0.273x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.315e-03 | 2.567e+00 | 4.513e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.290e-03 | 2.567e+00 | 4.495e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 4.677e-07 | 1.343e-03 | 2.361e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.290e-03 | 2.567e+00 | 4.495e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 4.677e-07 | 1.343e-03 | 2.361e-06 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.290e-03 | 2.567e+00 | 4.495e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.309e-03 | 1.229e+00 | 2.160e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 7.005e-03 | 3.600e+00 | 6.304e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.700e-07 | 1.068e-03 | 1.878e-06 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.290e-03 | 2.567e+00 | 4.496e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.899e-03 | 1.532e+00 | 2.693e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.631e-03 | 3.799e+00 | 6.653e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.586e-07 | 6.409e-04 | 1.127e-06 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.290e-03 | 2.567e+00 | 4.495e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.054e-03 | 1.781e+00 | 3.131e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.822e-03 | 4.048e+00 | 7.089e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 5.811e-07 | 4.883e-04 | 8.584e-07 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.290e-03 | 2.567e+00 | 4.495e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.149e-03 | 1.555e+00 | 2.735e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 7.924e-03 | 3.823e+00 | 6.694e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.595e-06 | 1.221e-03 | 2.146e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.290e-03 | 2.567e+00 | 4.495e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.159e-03 | 1.658e+00 | 2.914e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 7.936e-03 | 3.925e+00 | 6.873e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.870e-06 | 1.724e-03 | 3.031e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.290e-03 | 2.568e+00 | 4.497e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.171e-03 | 1.754e+00 | 3.084e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 7.946e-03 | 4.022e+00 | 7.042e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.173e-06 | 2.655e-03 | 4.668e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.291e-03 | 2.567e+00 | 4.495e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.179e-03 | 1.930e+00 | 3.393e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.952e-03 | 3.743e+00 | 6.554e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 5.418e-06 | 5.066e-03 | 8.906e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.292e-03 | 2.572e+00 | 4.503e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.309e-03 | 1.229e+00 | 2.160e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.700e-07 | 1.068e-03 | 1.878e-06 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.899e-03 | 1.532e+00 | 2.693e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.586e-07 | 6.409e-04 | 1.127e-06 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.054e-03 | 1.781e+00 | 3.131e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 5.811e-07 | 4.883e-04 | 8.584e-07 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.149e-03 | 1.555e+00 | 2.735e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.595e-06 | 1.221e-03 | 2.146e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.159e-03 | 1.658e+00 | 2.914e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.870e-06 | 1.724e-03 | 3.031e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.171e-03 | 1.754e+00 | 3.084e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.173e-06 | 2.655e-03 | 4.668e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.179e-03 | 1.930e+00 | 3.393e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 5.418e-06 | 5.066e-03 | 8.906e-06 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.041e-02 | 2.157e+03 | 9.600e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.030e-02 | 2.157e+03 | 9.509e-03 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 9.154e-06 | 3.047e+00 | 1.356e-05 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.030e-02 | 2.157e+03 | 9.513e-03 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 9.154e-06 | 3.047e+00 | 1.356e-05 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.030e-02 | 2.157e+03 | 9.513e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.164e-03 | 6.494e+02 | 2.891e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.329e-02 | 2.780e+03 | 1.226e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 8.065e-06 | 8.477e-01 | 3.774e-06 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.030e-02 | 2.157e+03 | 9.511e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.716e-03 | 1.032e+03 | 4.596e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.484e-02 | 3.189e+03 | 1.406e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 8.742e-06 | 9.492e-01 | 4.226e-06 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.030e-02 | 2.157e+03 | 9.510e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.303e-03 | 1.187e+03 | 5.283e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.542e-02 | 3.343e+03 | 1.474e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.270e-05 | 1.438e+00 | 6.400e-06 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.030e-02 | 2.157e+03 | 9.511e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.552e-03 | 1.251e+03 | 5.570e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.566e-02 | 3.408e+03 | 1.503e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.897e-05 | 1.857e+00 | 8.269e-06 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.031e-02 | 2.158e+03 | 9.514e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.664e-03 | 1.291e+03 | 5.747e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.575e-02 | 3.448e+03 | 1.520e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.183e-05 | 2.625e+00 | 1.169e-05 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.031e-02 | 2.159e+03 | 9.518e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.726e-03 | 1.308e+03 | 5.821e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.578e-02 | 3.464e+03 | 1.528e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.366e-05 | 2.797e+00 | 1.245e-05 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.031e-02 | 2.159e+03 | 9.519e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.893e-03 | 1.357e+03 | 6.041e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.585e-02 | 3.496e+03 | 1.541e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 4.038e-05 | 4.148e+00 | 1.847e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.032e-02 | 2.159e+03 | 9.518e-03 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 1.3721 | 10 | 5.421 |
| original | row_schedule | 0.0019 | 10 | 5.421 |
| original | partial_zero | 0.0010 | 10 | 5.421 |
| original | csr_prefetch | 0.1152 | 10 | 5.421 |
| original | feature_gather_decode | 0.1051 | 10 | 5.421 |
| original | fp32_neighbor_reduction | 0.0000 | 10 | 5.421 |
| original | partial_bf16_conversion | 0.0000 | 10 | 5.421 |
| original | tile_load | 0.8013 | 10 | 5.421 |
| original | tile_compute | 0.6819 | 10 | 5.421 |
| original | tile_store | 0.0050 | 10 | 5.421 |
| original | output_scatter | 0.0019 | 10 | 5.421 |
| original | thread_amx_setup | 3.7177 | 10 | 5.421 |

## hollywood-2009

N=1139905, E=113891327, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b32_fast_nozero | 88.354 | 87.880 | 88.059 | 88.897 | 1.970x | 1.261x |
| shared_b64_accurate_nozero | 88.536 | 86.473 | 87.543 | 89.759 | 1.966x | 1.258x |
| shared_bfull_accurate_nozero | 88.700 | 86.294 | 87.592 | 89.082 | 1.962x | 1.256x |
| shared_bfull_fast_nozero | 89.254 | 86.113 | 87.325 | 90.229 | 1.950x | 1.248x |
| shared_b32_accurate_nozero | 89.334 | 88.296 | 88.778 | 89.709 | 1.948x | 1.247x |
| shared_b64_fast_nozero | 89.761 | 87.321 | 89.477 | 90.206 | 1.939x | 1.241x |
| shared_b16_fast_nozero | 91.126 | 89.617 | 90.551 | 91.672 | 1.910x | 1.222x |
| shared_b16_accurate_nozero | 96.895 | 95.349 | 96.235 | 97.843 | 1.796x | 1.150x |
| shared_b8_fast_nozero | 101.523 | 99.823 | 101.069 | 101.992 | 1.714x | 1.097x |
| mkl_bf16_inputs | 105.999 | 103.585 | 105.708 | 106.356 | 1.642x | 1.051x |
| mkl_fp32 | 111.397 | 111.058 | 111.336 | 111.611 | 1.562x | 1.000x |
| shared_b8_accurate_nozero | 113.078 | 111.000 | 111.750 | 114.206 | 1.539x | 0.985x |
| shared_b4_fast_nozero | 122.982 | 121.047 | 122.177 | 124.985 | 1.415x | 0.906x |
| original_nozero | 129.495 | 126.975 | 128.357 | 130.493 | 1.344x | 0.860x |
| shared_b4_accurate_nozero | 148.619 | 144.318 | 145.798 | 150.576 | 1.171x | 0.750x |
| original | 174.050 | 171.810 | 173.115 | 175.076 | 1.000x | 0.640x |
| shared_b2_fast_nozero | 178.478 | 172.254 | 175.944 | 181.298 | 0.975x | 0.624x |
| shared_b2_accurate_nozero | 218.361 | 214.460 | 217.137 | 221.325 | 0.797x | 0.510x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 180.951 | 177.586 | 180.109 | 182.853 | 1.931x | 1.217x |
| shared_bfull_accurate_nozero | 181.034 | 178.338 | 179.832 | 181.984 | 1.930x | 1.217x |
| shared_b64_accurate_nozero | 181.521 | 178.834 | 180.737 | 181.792 | 1.925x | 1.213x |
| shared_b32_fast_nozero | 183.297 | 180.863 | 182.667 | 184.173 | 1.907x | 1.202x |
| shared_b64_fast_nozero | 184.053 | 180.867 | 182.641 | 185.485 | 1.899x | 1.197x |
| shared_b32_accurate_nozero | 186.594 | 182.532 | 184.038 | 188.040 | 1.873x | 1.180x |
| shared_b16_fast_nozero | 189.544 | 187.194 | 188.102 | 190.350 | 1.844x | 1.162x |
| shared_b16_accurate_nozero | 200.028 | 197.949 | 199.364 | 201.011 | 1.747x | 1.101x |
| shared_b8_fast_nozero | 209.330 | 207.284 | 208.037 | 209.901 | 1.669x | 1.052x |
| mkl_bf16_inputs | 220.121 | 219.634 | 219.786 | 220.376 | 1.588x | 1.001x |
| mkl_fp32 | 220.238 | 219.700 | 220.087 | 220.408 | 1.587x | 1.000x |
| shared_b8_accurate_nozero | 231.510 | 227.788 | 229.061 | 232.397 | 1.510x | 0.951x |
| shared_b4_fast_nozero | 254.876 | 251.467 | 253.064 | 259.361 | 1.371x | 0.864x |
| original_nozero | 269.143 | 267.949 | 268.575 | 273.907 | 1.298x | 0.818x |
| shared_b4_accurate_nozero | 300.474 | 295.227 | 298.184 | 304.899 | 1.163x | 0.733x |
| original | 349.468 | 345.495 | 349.245 | 350.714 | 1.000x | 0.630x |
| shared_b2_fast_nozero | 357.392 | 350.883 | 353.875 | 358.522 | 0.978x | 0.616x |
| shared_b2_accurate_nozero | 447.426 | 439.359 | 445.698 | 454.114 | 0.781x | 0.492x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.257e-03 | 7.612e+00 | 5.248e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.232e-03 | 7.612e+00 | 5.221e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 1.062e-06 | 9.766e-03 | 6.733e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.232e-03 | 7.613e+00 | 5.221e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 1.062e-06 | 9.766e-03 | 6.733e-06 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.232e-03 | 7.613e+00 | 5.221e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.356e-03 | 3.276e+00 | 2.259e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 7.018e-03 | 1.038e+01 | 7.117e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.055e-06 | 9.399e-03 | 6.480e-06 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.232e-03 | 7.615e+00 | 5.223e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.965e-03 | 4.608e+00 | 3.177e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.665e-03 | 1.222e+01 | 8.381e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 7.685e-07 | 4.944e-03 | 3.408e-06 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.232e-03 | 7.612e+00 | 5.221e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.146e-03 | 4.420e+00 | 3.047e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.882e-03 | 1.067e+01 | 7.318e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 8.058e-07 | 3.601e-03 | 2.483e-06 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.232e-03 | 7.611e+00 | 5.220e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.247e-03 | 4.916e+00 | 3.389e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 7.993e-03 | 1.136e+01 | 7.794e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.883e-06 | 3.418e-03 | 2.356e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.232e-03 | 7.613e+00 | 5.221e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.267e-03 | 4.511e+00 | 3.110e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 8.016e-03 | 1.103e+01 | 7.567e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.532e-06 | 4.791e-03 | 3.303e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.233e-03 | 7.613e+00 | 5.221e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.286e-03 | 5.777e+00 | 3.983e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 8.037e-03 | 1.103e+01 | 7.564e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 5.328e-06 | 7.812e-03 | 5.386e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.234e-03 | 7.615e+00 | 5.223e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.312e-03 | 4.577e+00 | 3.156e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.048e-03 | 1.067e+01 | 7.319e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 7.907e-06 | 1.575e-02 | 1.086e-05 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.236e-03 | 7.616e+00 | 5.223e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.356e-03 | 3.276e+00 | 2.259e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.055e-06 | 9.399e-03 | 6.480e-06 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.965e-03 | 4.608e+00 | 3.177e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 7.685e-07 | 4.944e-03 | 3.408e-06 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.146e-03 | 4.420e+00 | 3.047e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 8.058e-07 | 3.601e-03 | 2.483e-06 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.247e-03 | 4.916e+00 | 3.389e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.883e-06 | 3.418e-03 | 2.356e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.267e-03 | 4.511e+00 | 3.110e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.532e-06 | 4.791e-03 | 3.303e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.286e-03 | 5.777e+00 | 3.983e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 5.328e-06 | 7.812e-03 | 5.386e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.312e-03 | 4.577e+00 | 3.156e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 7.907e-06 | 1.575e-02 | 1.086e-05 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 9.995e-03 | 1.057e+05 | 1.051e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 9.900e-03 | 1.057e+05 | 1.040e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 4.273e-06 | 6.725e+01 | 6.689e-06 | 1 |
| original | two_layer_e2e | mkl_fp32 | 9.899e-03 | 1.058e+05 | 1.041e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 4.273e-06 | 6.725e+01 | 6.689e-06 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 9.899e-03 | 1.058e+05 | 1.041e-02 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.450e-03 | 3.131e+04 | 3.114e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.312e-02 | 1.370e+05 | 1.349e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.731e-06 | 1.020e+02 | 1.015e-05 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.900e-03 | 1.058e+05 | 1.041e-02 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.814e-03 | 4.665e+04 | 4.640e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.450e-02 | 1.524e+05 | 1.500e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.814e-06 | 3.100e+01 | 3.083e-06 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.900e-03 | 1.057e+05 | 1.041e-02 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.297e-03 | 5.370e+04 | 5.341e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.500e-02 | 1.594e+05 | 1.569e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.716e-06 | 3.700e+01 | 3.680e-06 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.900e-03 | 1.057e+05 | 1.040e-02 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.619e-03 | 5.697e+04 | 5.666e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.531e-02 | 1.627e+05 | 1.601e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 4.230e-06 | 3.180e+01 | 3.163e-06 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.900e-03 | 1.057e+05 | 1.041e-02 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.670e-03 | 5.808e+04 | 5.777e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.536e-02 | 1.638e+05 | 1.612e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 6.017e-06 | 4.900e+01 | 4.874e-06 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.902e-03 | 1.058e+05 | 1.041e-02 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.714e-03 | 5.597e+04 | 5.567e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.540e-02 | 1.617e+05 | 1.591e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 9.268e-06 | 7.300e+01 | 7.261e-06 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.904e-03 | 1.058e+05 | 1.041e-02 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.976e-03 | 5.540e+04 | 5.510e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.549e-02 | 1.610e+05 | 1.585e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.733e-05 | 1.685e+02 | 1.676e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.912e-03 | 1.058e+05 | 1.042e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 46.2611 | 280 | 173.202 |
| original | row_schedule | 0.0880 | 280 | 173.202 |
| original | partial_zero | 0.1001 | 280 | 173.202 |
| original | csr_prefetch | 9.2835 | 280 | 173.202 |
| original | feature_gather_decode | 3.9749 | 280 | 173.202 |
| original | fp32_neighbor_reduction | 0.0000 | 280 | 173.202 |
| original | partial_bf16_conversion | 0.0000 | 280 | 173.202 |
| original | tile_load | 21.0021 | 280 | 173.202 |
| original | tile_compute | 26.3672 | 280 | 173.202 |
| original | tile_store | 0.1130 | 280 | 173.202 |
| original | output_scatter | 0.0908 | 280 | 173.202 |
| original | thread_amx_setup | 1.4491 | 280 | 173.202 |

## indochina-2004

N=7414866, E=194109311, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 87.745 | 86.634 | 87.013 | 88.888 | 5.274x | 4.214x |
| shared_b64_fast_nozero | 91.178 | 90.790 | 90.976 | 91.419 | 5.075x | 4.056x |
| shared_bfull_accurate_nozero | 93.667 | 91.863 | 92.950 | 94.101 | 4.940x | 3.948x |
| shared_b32_fast_nozero | 97.636 | 96.709 | 97.418 | 98.049 | 4.739x | 3.788x |
| shared_b64_accurate_nozero | 98.988 | 97.691 | 98.230 | 99.588 | 4.675x | 3.736x |
| shared_b32_accurate_nozero | 104.846 | 104.400 | 104.485 | 105.045 | 4.414x | 3.527x |
| shared_b16_fast_nozero | 107.190 | 105.715 | 106.904 | 107.274 | 4.317x | 3.450x |
| shared_b16_accurate_nozero | 120.638 | 119.948 | 120.231 | 121.138 | 3.836x | 3.065x |
| shared_b8_fast_nozero | 129.364 | 128.787 | 129.020 | 129.730 | 3.577x | 2.859x |
| shared_b8_accurate_nozero | 151.286 | 150.312 | 150.596 | 151.967 | 3.059x | 2.444x |
| original_nozero | 163.811 | 161.790 | 162.914 | 164.553 | 2.825x | 2.257x |
| shared_b4_fast_nozero | 171.170 | 167.946 | 170.112 | 171.782 | 2.703x | 2.160x |
| shared_b4_accurate_nozero | 210.674 | 208.744 | 210.393 | 211.138 | 2.196x | 1.755x |
| mkl_bf16_inputs | 242.117 | 238.914 | 241.227 | 242.632 | 1.911x | 1.527x |
| shared_b2_fast_nozero | 253.057 | 247.451 | 251.682 | 254.070 | 1.829x | 1.461x |
| shared_b2_accurate_nozero | 330.062 | 324.001 | 328.220 | 333.094 | 1.402x | 1.120x |
| mkl_fp32 | 369.801 | 364.214 | 368.306 | 370.251 | 1.251x | 1.000x |
| original | 462.742 | 462.298 | 462.555 | 463.474 | 1.000x | 0.799x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 205.885 | 203.642 | 204.880 | 206.430 | 4.633x | 2.972x |
| shared_b64_fast_nozero | 212.694 | 211.423 | 212.183 | 212.850 | 4.485x | 2.877x |
| shared_bfull_accurate_nozero | 217.286 | 216.627 | 217.146 | 217.659 | 4.390x | 2.816x |
| shared_b32_fast_nozero | 224.864 | 222.769 | 224.251 | 225.161 | 4.242x | 2.722x |
| shared_b64_accurate_nozero | 226.282 | 224.379 | 225.616 | 227.455 | 4.215x | 2.704x |
| shared_b32_accurate_nozero | 239.309 | 237.833 | 238.539 | 240.191 | 3.986x | 2.557x |
| shared_b16_fast_nozero | 245.667 | 244.612 | 244.949 | 246.039 | 3.883x | 2.491x |
| shared_b16_accurate_nozero | 269.102 | 267.541 | 268.544 | 269.871 | 3.545x | 2.274x |
| shared_b8_fast_nozero | 290.040 | 288.577 | 288.992 | 291.306 | 3.289x | 2.110x |
| shared_b8_accurate_nozero | 333.513 | 331.137 | 332.578 | 334.100 | 2.860x | 1.835x |
| original_nozero | 354.777 | 352.906 | 354.305 | 355.350 | 2.689x | 1.725x |
| shared_b4_fast_nozero | 370.807 | 366.498 | 369.728 | 371.554 | 2.572x | 1.650x |
| shared_b4_accurate_nozero | 451.856 | 448.619 | 451.416 | 453.763 | 2.111x | 1.354x |
| shared_b2_fast_nozero | 534.600 | 529.768 | 533.260 | 543.773 | 1.784x | 1.145x |
| mkl_fp32 | 611.980 | 606.947 | 608.578 | 614.960 | 1.559x | 1.000x |
| mkl_bf16_inputs | 652.294 | 646.950 | 649.432 | 653.304 | 1.462x | 0.938x |
| shared_b2_accurate_nozero | 692.741 | 686.550 | 690.873 | 693.963 | 1.377x | 0.883x |
| original | 953.836 | 951.478 | 952.142 | 954.248 | 1.000x | 0.642x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.180e-03 | 7.208e+00 | 5.281e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.156e-03 | 7.208e+00 | 5.253e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 1.701e-06 | 8.057e-03 | 5.903e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.156e-03 | 7.215e+00 | 5.259e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 1.701e-06 | 8.057e-03 | 5.903e-06 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.156e-03 | 7.215e+00 | 5.259e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.317e-03 | 3.340e+00 | 2.447e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 6.936e-03 | 1.050e+01 | 7.653e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.523e-06 | 6.958e-03 | 5.098e-06 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.156e-03 | 7.215e+00 | 5.258e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.928e-03 | 3.692e+00 | 2.705e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.597e-03 | 1.087e+01 | 7.925e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.144e-06 | 4.272e-03 | 3.130e-06 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.156e-03 | 7.210e+00 | 5.255e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.134e-03 | 4.876e+00 | 3.573e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.844e-03 | 1.207e+01 | 8.797e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 9.760e-07 | 4.395e-03 | 3.220e-06 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.156e-03 | 7.207e+00 | 5.253e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.176e-03 | 4.981e+00 | 3.649e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 7.864e-03 | 1.080e+01 | 7.872e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.791e-06 | 3.540e-03 | 2.594e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.156e-03 | 7.209e+00 | 5.255e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.126e-03 | 4.346e+00 | 3.184e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 7.832e-03 | 1.151e+01 | 8.385e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.226e-06 | 5.371e-03 | 3.935e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.157e-03 | 7.207e+00 | 5.253e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.211e-03 | 4.592e+00 | 3.365e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 7.892e-03 | 1.176e+01 | 8.572e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.425e-06 | 7.202e-03 | 5.277e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.157e-03 | 7.209e+00 | 5.254e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.304e-03 | 5.827e+00 | 4.269e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.981e-03 | 1.282e+01 | 9.347e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 6.951e-06 | 1.648e-02 | 1.207e-05 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.159e-03 | 7.216e+00 | 5.259e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.317e-03 | 3.340e+00 | 2.447e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.523e-06 | 6.958e-03 | 5.098e-06 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.928e-03 | 3.692e+00 | 2.705e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.144e-06 | 4.272e-03 | 3.130e-06 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.134e-03 | 4.876e+00 | 3.573e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 9.760e-07 | 4.395e-03 | 3.220e-06 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.176e-03 | 4.981e+00 | 3.649e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.791e-06 | 3.540e-03 | 2.594e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.126e-03 | 4.346e+00 | 3.184e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.226e-06 | 5.371e-03 | 3.935e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.211e-03 | 4.592e+00 | 3.365e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.425e-06 | 7.202e-03 | 5.277e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.304e-03 | 5.827e+00 | 4.269e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 6.951e-06 | 1.648e-02 | 1.207e-05 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.023e-02 | 4.508e+05 | 1.000e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.013e-02 | 4.508e+05 | 9.904e-03 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 2.386e-05 | 2.204e+03 | 4.890e-05 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.013e-02 | 4.489e+05 | 9.861e-03 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 2.386e-05 | 2.204e+03 | 4.890e-05 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.013e-02 | 4.489e+05 | 9.861e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 2.412e-03 | 1.215e+05 | 2.697e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.223e-02 | 5.723e+05 | 1.257e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.558e-05 | 7.080e+02 | 1.571e-05 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.013e-02 | 4.504e+05 | 9.895e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.678e-03 | 1.618e+05 | 3.589e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.355e-02 | 6.125e+05 | 1.346e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 5.287e-06 | 2.760e+02 | 6.124e-06 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.013e-02 | 4.509e+05 | 9.905e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.664e-03 | 2.080e+05 | 4.614e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.458e-02 | 6.587e+05 | 1.447e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.429e-05 | 7.460e+02 | 1.655e-05 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.013e-02 | 4.504e+05 | 9.894e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.299e-03 | 2.331e+05 | 5.173e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.519e-02 | 6.839e+05 | 1.502e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.410e-05 | 7.590e+02 | 1.684e-05 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.013e-02 | 4.504e+05 | 9.895e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.173e-03 | 2.315e+05 | 5.137e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.508e-02 | 6.824e+05 | 1.499e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.087e-05 | 7.244e+02 | 1.607e-05 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.013e-02 | 4.505e+05 | 9.896e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.496e-03 | 2.418e+05 | 5.365e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.535e-02 | 6.920e+05 | 1.520e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.871e-05 | 5.666e+02 | 1.257e-05 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.014e-02 | 4.506e+05 | 9.900e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 6.375e-03 | 3.152e+05 | 6.994e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.607e-02 | 7.601e+05 | 1.670e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.736e-05 | 1.032e+03 | 2.290e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.014e-02 | 4.516e+05 | 9.922e-03 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 296.6180 | 1812 | 475.324 |
| original | row_schedule | 0.3667 | 1812 | 475.324 |
| original | partial_zero | 0.5722 | 1812 | 475.324 |
| original | csr_prefetch | 6.3396 | 1812 | 475.324 |
| original | feature_gather_decode | 6.0768 | 1812 | 475.324 |
| original | fp32_neighbor_reduction | 0.0000 | 1812 | 475.324 |
| original | partial_bf16_conversion | 0.0000 | 1812 | 475.324 |
| original | tile_load | 30.1857 | 1812 | 475.324 |
| original | tile_compute | 29.7670 | 1812 | 475.324 |
| original | tile_store | 0.6876 | 1812 | 475.324 |
| original | output_scatter | 0.6256 | 1812 | 475.324 |
| original | thread_amx_setup | 3.5577 | 1812 | 475.324 |

## mycielskian19

N=393215, E=903194710, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b64_fast_nozero | 491.288 | 465.688 | 483.817 | 506.748 | 1.990x | 2.893x |
| shared_b64_accurate_nozero | 517.368 | 500.221 | 509.967 | 523.477 | 1.890x | 2.747x |
| shared_b32_fast_nozero | 522.757 | 496.337 | 512.687 | 529.324 | 1.870x | 2.719x |
| shared_b32_accurate_nozero | 531.649 | 499.626 | 520.283 | 547.365 | 1.839x | 2.673x |
| shared_b16_fast_nozero | 568.954 | 529.268 | 549.239 | 575.551 | 1.718x | 2.498x |
| shared_b16_accurate_nozero | 584.182 | 531.983 | 564.082 | 593.915 | 1.674x | 2.433x |
| shared_b8_fast_nozero | 637.921 | 609.318 | 619.786 | 650.576 | 1.533x | 2.228x |
| shared_b8_accurate_nozero | 739.526 | 719.387 | 723.439 | 761.140 | 1.322x | 1.922x |
| shared_b4_fast_nozero | 813.400 | 739.181 | 795.404 | 831.092 | 1.202x | 1.747x |
| original_nozero | 946.674 | 909.519 | 928.892 | 955.311 | 1.033x | 1.501x |
| original | 977.645 | 931.883 | 963.008 | 998.210 | 1.000x | 1.454x |
| shared_bfull_fast_nozero | 1020.424 | 1008.245 | 1014.534 | 1040.809 | 0.958x | 1.393x |
| shared_bfull_accurate_nozero | 1030.288 | 1015.019 | 1022.253 | 1035.611 | 0.949x | 1.379x |
| shared_b4_accurate_nozero | 1066.288 | 991.703 | 1023.578 | 1087.851 | 0.917x | 1.333x |
| shared_b2_fast_nozero | 1284.378 | 1258.412 | 1268.701 | 1324.624 | 0.761x | 1.107x |
| mkl_bf16_inputs | 1368.145 | 1367.186 | 1367.774 | 1369.243 | 0.715x | 1.039x |
| mkl_fp32 | 1421.219 | 1418.403 | 1420.169 | 1422.720 | 0.688x | 1.000x |
| shared_b2_accurate_nozero | 1698.951 | 1655.007 | 1667.966 | 1753.529 | 0.575x | 0.837x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b64_fast_nozero | 981.045 | 939.968 | 971.852 | 997.081 | 1.939x | 2.862x |
| shared_b64_accurate_nozero | 1001.693 | 965.235 | 995.151 | 1022.632 | 1.900x | 2.803x |
| shared_b32_fast_nozero | 1049.720 | 983.340 | 1047.586 | 1052.536 | 1.813x | 2.675x |
| shared_b32_accurate_nozero | 1066.141 | 1015.564 | 1051.095 | 1070.729 | 1.785x | 2.634x |
| shared_b16_fast_nozero | 1107.407 | 1050.423 | 1074.400 | 1138.657 | 1.718x | 2.536x |
| shared_b16_accurate_nozero | 1174.809 | 1118.395 | 1148.081 | 1189.307 | 1.620x | 2.390x |
| shared_b8_fast_nozero | 1277.338 | 1185.455 | 1256.221 | 1310.528 | 1.490x | 2.198x |
| shared_b8_accurate_nozero | 1443.330 | 1406.518 | 1433.154 | 1469.024 | 1.318x | 1.946x |
| shared_b4_fast_nozero | 1665.927 | 1578.987 | 1644.070 | 1693.766 | 1.142x | 1.686x |
| original_nozero | 1870.293 | 1809.775 | 1848.647 | 1891.886 | 1.017x | 1.501x |
| original | 1902.735 | 1828.510 | 1858.256 | 1942.933 | 1.000x | 1.476x |
| shared_bfull_accurate_nozero | 2036.970 | 2011.875 | 2022.902 | 2049.428 | 0.934x | 1.379x |
| shared_bfull_fast_nozero | 2042.140 | 2004.890 | 2034.315 | 2050.363 | 0.932x | 1.375x |
| shared_b4_accurate_nozero | 2049.371 | 1894.932 | 2006.964 | 2116.555 | 0.928x | 1.370x |
| shared_b2_fast_nozero | 2542.807 | 2401.492 | 2478.173 | 2585.006 | 0.748x | 1.104x |
| mkl_bf16_inputs | 2733.598 | 2731.411 | 2733.016 | 2734.782 | 0.696x | 1.027x |
| mkl_fp32 | 2808.225 | 2804.692 | 2805.455 | 2809.560 | 0.678x | 1.000x |
| shared_b2_accurate_nozero | 3371.667 | 3217.353 | 3287.969 | 3463.809 | 0.564x | 0.833x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 4.349e-03 | 6.006e+01 | 2.763e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 4.333e-03 | 6.006e+01 | 2.756e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 5.037e-06 | 5.684e-01 | 2.615e-05 | 1 |
| original | kernel_full | mkl_fp32 | 4.333e-03 | 5.994e+01 | 2.750e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 5.037e-06 | 5.684e-01 | 2.615e-05 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 4.333e-03 | 5.994e+01 | 2.750e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.072e-03 | 3.955e+01 | 1.820e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 5.970e-03 | 9.903e+01 | 4.544e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 5.006e-06 | 5.801e-01 | 2.669e-05 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 4.333e-03 | 5.977e+01 | 2.742e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.746e-03 | 5.148e+01 | 2.369e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 6.640e-03 | 1.107e+02 | 5.080e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.613e-06 | 4.688e-01 | 2.157e-05 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 4.333e-03 | 6.007e+01 | 2.756e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 2.947e-03 | 5.687e+01 | 2.617e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 6.853e-03 | 1.169e+02 | 5.365e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.570e-06 | 2.754e-01 | 1.267e-05 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 4.333e-03 | 6.023e+01 | 2.764e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.051e-03 | 6.756e+01 | 3.109e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 6.960e-03 | 1.251e+02 | 5.740e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.302e-06 | 2.539e-01 | 1.168e-05 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 4.333e-03 | 6.000e+01 | 2.753e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.066e-03 | 6.584e+01 | 3.029e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 6.977e-03 | 1.234e+02 | 5.661e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.121e-06 | 1.348e-01 | 6.201e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 4.334e-03 | 6.015e+01 | 2.760e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.094e-03 | 6.396e+01 | 2.943e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 7.004e-03 | 1.228e+02 | 5.636e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.698e-06 | 1.895e-01 | 8.717e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 4.335e-03 | 6.017e+01 | 2.761e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.297e-03 | 7.993e+01 | 3.678e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.096e-03 | 1.375e+02 | 6.307e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 9.380e-06 | 2.480e-01 | 1.141e-05 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 4.339e-03 | 6.017e+01 | 2.761e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.072e-03 | 3.955e+01 | 1.820e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 5.006e-06 | 5.801e-01 | 2.669e-05 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.746e-03 | 5.148e+01 | 2.369e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.613e-06 | 4.688e-01 | 2.157e-05 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.947e-03 | 5.687e+01 | 2.617e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.570e-06 | 2.754e-01 | 1.267e-05 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.051e-03 | 6.756e+01 | 3.109e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.302e-06 | 2.539e-01 | 1.168e-05 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.066e-03 | 6.584e+01 | 3.029e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.121e-06 | 1.348e-01 | 6.201e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.094e-03 | 6.396e+01 | 2.943e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.698e-06 | 1.895e-01 | 8.717e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.297e-03 | 7.993e+01 | 3.678e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 9.380e-06 | 2.480e-01 | 1.141e-05 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 8.802e-03 | 2.114e+06 | 9.737e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 8.727e-03 | 2.114e+06 | 9.648e-03 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 7.360e-06 | 7.296e+03 | 3.360e-05 | 1 |
| original | two_layer_e2e | mkl_fp32 | 8.727e-03 | 2.114e+06 | 9.650e-03 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 7.360e-06 | 7.296e+03 | 3.360e-05 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 8.727e-03 | 2.114e+06 | 9.650e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.314e-03 | 7.220e+05 | 3.325e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.194e-02 | 2.818e+06 | 1.286e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 7.699e-05 | 1.799e+05 | 8.286e-04 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 8.750e-03 | 2.282e+06 | 1.041e-02 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.788e-03 | 1.037e+06 | 4.774e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.338e-02 | 3.136e+06 | 1.431e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.964e-05 | 1.470e+05 | 6.770e-04 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 8.735e-03 | 2.242e+06 | 1.023e-02 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.295e-03 | 1.147e+06 | 5.284e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.388e-02 | 3.252e+06 | 1.484e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.226e-05 | 6.475e+04 | 2.982e-04 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 8.729e-03 | 2.160e+06 | 9.859e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.514e-03 | 1.196e+06 | 5.506e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.410e-02 | 3.304e+06 | 1.508e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 4.062e-06 | 1.498e+04 | 6.897e-05 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 8.728e-03 | 2.122e+06 | 9.685e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.571e-03 | 1.207e+06 | 5.561e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.416e-02 | 3.321e+06 | 1.516e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 4.724e-06 | 2.752e+03 | 1.267e-05 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 8.730e-03 | 2.114e+06 | 9.649e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.618e-03 | 1.214e+06 | 5.589e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.420e-02 | 3.328e+06 | 1.519e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 8.378e-06 | 2.624e+03 | 1.208e-05 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 8.734e-03 | 2.115e+06 | 9.654e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.868e-03 | 1.385e+06 | 6.379e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.430e-02 | 3.386e+06 | 1.545e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.635e-05 | 3.744e+03 | 1.724e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 8.741e-03 | 2.117e+06 | 9.662e-03 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 15.7590 | 97 | 1302.418 |
| original | row_schedule | 0.0453 | 97 | 1302.418 |
| original | partial_zero | 0.0384 | 97 | 1302.418 |
| original | csr_prefetch | 57.5838 | 97 | 1302.418 |
| original | feature_gather_decode | 40.7269 | 97 | 1302.418 |
| original | fp32_neighbor_reduction | 0.0000 | 97 | 1302.418 |
| original | partial_bf16_conversion | 0.0000 | 97 | 1302.418 |
| original | tile_load | 202.4250 | 97 | 1302.418 |
| original | tile_compute | 202.1947 | 97 | 1302.418 |
| original | tile_store | 0.0527 | 97 | 1302.418 |
| original | output_scatter | 0.0536 | 97 | 1302.418 |
| original | thread_amx_setup | 4.0438 | 97 | 1302.418 |

## ogbn-products

N=2449029, E=123718152, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 133.636 | 131.625 | 132.187 | 135.544 | 1.918x | 1.717x |
| shared_b64_fast_nozero | 135.752 | 133.102 | 134.166 | 137.000 | 1.888x | 1.690x |
| shared_bfull_accurate_nozero | 136.631 | 134.878 | 135.547 | 138.186 | 1.876x | 1.679x |
| shared_b32_fast_nozero | 139.883 | 136.871 | 137.231 | 141.540 | 1.832x | 1.640x |
| shared_b64_accurate_nozero | 141.603 | 138.387 | 139.776 | 142.419 | 1.810x | 1.620x |
| original_nozero | 150.782 | 146.763 | 148.556 | 153.178 | 1.700x | 1.521x |
| shared_b32_accurate_nozero | 151.365 | 147.615 | 148.538 | 152.242 | 1.693x | 1.515x |
| shared_b16_fast_nozero | 153.595 | 152.420 | 152.968 | 157.916 | 1.669x | 1.493x |
| shared_b16_accurate_nozero | 161.896 | 158.910 | 159.757 | 163.507 | 1.583x | 1.417x |
| shared_b8_fast_nozero | 171.139 | 168.903 | 170.400 | 174.018 | 1.498x | 1.340x |
| shared_b8_accurate_nozero | 183.846 | 179.020 | 181.669 | 186.560 | 1.394x | 1.248x |
| shared_b4_fast_nozero | 201.490 | 199.624 | 200.400 | 203.351 | 1.272x | 1.138x |
| mkl_bf16_inputs | 227.757 | 227.582 | 227.663 | 228.000 | 1.125x | 1.007x |
| mkl_fp32 | 229.389 | 228.755 | 229.166 | 229.488 | 1.117x | 1.000x |
| shared_b4_accurate_nozero | 229.837 | 224.905 | 225.644 | 230.760 | 1.115x | 0.998x |
| original | 256.318 | 251.854 | 254.558 | 259.718 | 1.000x | 0.895x |
| shared_b2_fast_nozero | 262.557 | 256.492 | 257.910 | 265.617 | 0.976x | 0.874x |
| shared_b2_accurate_nozero | 314.101 | 305.037 | 307.019 | 316.671 | 0.816x | 0.730x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 279.020 | 273.481 | 274.874 | 280.836 | 1.853x | 1.658x |
| shared_b64_fast_nozero | 283.754 | 277.978 | 280.952 | 285.247 | 1.822x | 1.630x |
| shared_bfull_accurate_nozero | 284.833 | 280.778 | 283.576 | 286.963 | 1.815x | 1.624x |
| shared_b32_fast_nozero | 288.387 | 285.774 | 287.497 | 290.219 | 1.793x | 1.604x |
| shared_b64_accurate_nozero | 293.618 | 291.653 | 291.877 | 294.823 | 1.761x | 1.576x |
| shared_b32_accurate_nozero | 310.144 | 308.144 | 309.572 | 311.349 | 1.667x | 1.492x |
| original_nozero | 320.320 | 313.559 | 317.083 | 321.631 | 1.614x | 1.444x |
| shared_b16_fast_nozero | 323.627 | 318.623 | 319.845 | 324.996 | 1.598x | 1.429x |
| shared_b16_accurate_nozero | 338.318 | 329.833 | 331.134 | 340.410 | 1.528x | 1.367x |
| shared_b8_fast_nozero | 358.379 | 353.362 | 354.652 | 360.523 | 1.443x | 1.291x |
| shared_b8_accurate_nozero | 383.285 | 377.340 | 378.713 | 384.628 | 1.349x | 1.207x |
| shared_b4_fast_nozero | 416.754 | 409.917 | 412.983 | 421.064 | 1.241x | 1.110x |
| mkl_fp32 | 462.621 | 462.157 | 462.366 | 462.861 | 1.118x | 1.000x |
| mkl_bf16_inputs | 468.778 | 468.266 | 468.682 | 468.962 | 1.103x | 0.987x |
| shared_b4_accurate_nozero | 470.272 | 465.038 | 467.016 | 476.269 | 1.100x | 0.984x |
| original | 517.067 | 506.355 | 511.994 | 521.086 | 1.000x | 0.895x |
| shared_b2_fast_nozero | 534.603 | 524.350 | 527.194 | 540.931 | 0.967x | 0.865x |
| shared_b2_accurate_nozero | 636.635 | 625.527 | 632.510 | 641.590 | 0.812x | 0.727x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.282e-03 | 6.956e+00 | 3.748e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.257e-03 | 6.956e+00 | 3.738e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 5.965e-07 | 1.221e-02 | 6.578e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.257e-03 | 6.957e+00 | 3.738e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 5.965e-07 | 1.221e-02 | 6.578e-06 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.257e-03 | 6.957e+00 | 3.738e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.367e-03 | 3.832e+00 | 2.065e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 7.043e-03 | 9.787e+00 | 5.259e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 5.982e-07 | 9.033e-03 | 4.868e-06 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.257e-03 | 6.954e+00 | 3.736e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.969e-03 | 4.894e+00 | 2.637e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.688e-03 | 1.134e+01 | 6.092e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.492e-07 | 1.416e-02 | 7.631e-06 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.257e-03 | 6.953e+00 | 3.736e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.150e-03 | 6.191e+00 | 3.336e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.906e-03 | 1.256e+01 | 6.749e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 6.736e-07 | 5.981e-03 | 3.223e-06 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.257e-03 | 6.956e+00 | 3.738e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.251e-03 | 4.783e+00 | 2.577e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 8.016e-03 | 1.115e+01 | 5.993e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.841e-06 | 3.662e-03 | 1.973e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.257e-03 | 6.955e+00 | 3.737e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.268e-03 | 5.155e+00 | 2.778e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 8.037e-03 | 1.150e+01 | 6.182e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.441e-06 | 6.104e-03 | 3.289e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.258e-03 | 6.955e+00 | 3.737e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.290e-03 | 5.850e+00 | 3.152e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 8.059e-03 | 1.113e+01 | 5.979e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 5.093e-06 | 8.423e-03 | 4.539e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.259e-03 | 6.959e+00 | 3.739e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.296e-03 | 7.792e+00 | 4.199e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.062e-03 | 1.414e+01 | 7.598e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 6.624e-06 | 2.209e-02 | 1.191e-05 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.260e-03 | 6.969e+00 | 3.745e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.367e-03 | 3.832e+00 | 2.065e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 5.982e-07 | 9.033e-03 | 4.868e-06 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.969e-03 | 4.894e+00 | 2.637e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.492e-07 | 1.416e-02 | 7.631e-06 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.150e-03 | 6.191e+00 | 3.336e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 6.736e-07 | 5.981e-03 | 3.223e-06 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.251e-03 | 4.783e+00 | 2.577e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.841e-06 | 3.662e-03 | 1.973e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.268e-03 | 5.155e+00 | 2.778e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.441e-06 | 6.104e-03 | 3.289e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.290e-03 | 5.850e+00 | 3.152e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 5.093e-06 | 8.423e-03 | 4.539e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.296e-03 | 7.792e+00 | 4.199e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 6.624e-06 | 2.209e-02 | 1.191e-05 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 9.999e-03 | 5.824e+04 | 1.101e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 9.902e-03 | 5.824e+04 | 1.089e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 9.524e-06 | 3.075e+01 | 5.815e-06 | 1 |
| original | two_layer_e2e | mkl_fp32 | 9.902e-03 | 5.823e+04 | 1.089e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 9.524e-06 | 3.075e+01 | 5.815e-06 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 9.902e-03 | 5.823e+04 | 1.089e-02 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.083e-03 | 1.470e+04 | 2.780e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.287e-02 | 7.294e+04 | 1.364e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 8.596e-06 | 6.600e+01 | 1.248e-05 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.902e-03 | 5.827e+04 | 1.090e-02 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.657e-03 | 2.336e+04 | 4.417e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.443e-02 | 8.160e+04 | 1.526e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 6.004e-06 | 2.750e+01 | 5.200e-06 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.902e-03 | 5.824e+04 | 1.089e-02 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.306e-03 | 2.745e+04 | 5.191e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.507e-02 | 8.569e+04 | 1.603e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 7.572e-06 | 1.875e+01 | 3.545e-06 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.903e-03 | 5.824e+04 | 1.089e-02 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.557e-03 | 2.891e+04 | 5.467e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.531e-02 | 8.715e+04 | 1.630e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.115e-05 | 2.300e+01 | 4.349e-06 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.903e-03 | 5.824e+04 | 1.089e-02 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.630e-03 | 2.950e+04 | 5.579e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.537e-02 | 8.774e+04 | 1.641e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.551e-05 | 2.400e+01 | 4.538e-06 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.906e-03 | 5.825e+04 | 1.090e-02 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.704e-03 | 2.979e+04 | 5.632e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.542e-02 | 8.802e+04 | 1.646e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.128e-05 | 4.600e+01 | 8.698e-06 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.909e-03 | 5.828e+04 | 1.090e-02 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.859e-03 | 3.086e+04 | 5.836e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.548e-02 | 8.297e+04 | 1.552e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.832e-05 | 6.925e+01 | 1.309e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.913e-03 | 5.830e+04 | 1.090e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 100.0400 | 599 | 280.468 |
| original | row_schedule | 0.2246 | 599 | 280.468 |
| original | partial_zero | 0.2067 | 599 | 280.468 |
| original | csr_prefetch | 12.0721 | 599 | 280.468 |
| original | feature_gather_decode | 5.0542 | 599 | 280.468 |
| original | fp32_neighbor_reduction | 0.0000 | 599 | 280.468 |
| original | partial_bf16_conversion | 0.0000 | 599 | 280.468 |
| original | tile_load | 24.9147 | 599 | 280.468 |
| original | tile_compute | 33.6778 | 599 | 280.468 |
| original | tile_store | 0.2403 | 599 | 280.468 |
| original | output_scatter | 0.2246 | 599 | 280.468 |
| original | thread_amx_setup | 4.1156 | 599 | 280.468 |

## reddit

N=232965, E=114615892, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b64_fast_nozero | 59.635 | 57.314 | 58.263 | 61.212 | 2.590x | 1.755x |
| shared_b32_fast_nozero | 61.064 | 59.506 | 60.624 | 63.034 | 2.529x | 1.714x |
| shared_b64_accurate_nozero | 61.738 | 59.034 | 60.666 | 62.295 | 2.501x | 1.695x |
| shared_bfull_fast_nozero | 66.015 | 63.845 | 65.343 | 67.147 | 2.339x | 1.586x |
| shared_bfull_accurate_nozero | 66.461 | 62.949 | 64.484 | 67.264 | 2.324x | 1.575x |
| shared_b32_accurate_nozero | 66.722 | 64.215 | 65.011 | 67.638 | 2.315x | 1.569x |
| shared_b16_fast_nozero | 70.163 | 67.985 | 69.579 | 70.616 | 2.201x | 1.492x |
| shared_b16_accurate_nozero | 75.412 | 72.415 | 74.803 | 77.667 | 2.048x | 1.388x |
| shared_b8_fast_nozero | 80.186 | 75.055 | 79.424 | 82.699 | 1.926x | 1.305x |
| shared_b8_accurate_nozero | 96.775 | 87.399 | 94.158 | 98.216 | 1.596x | 1.082x |
| mkl_fp32 | 104.674 | 103.857 | 104.101 | 105.226 | 1.475x | 1.000x |
| mkl_bf16_inputs | 105.040 | 102.704 | 104.358 | 105.048 | 1.470x | 0.997x |
| shared_b4_fast_nozero | 109.042 | 101.139 | 104.209 | 112.615 | 1.416x | 0.960x |
| shared_b4_accurate_nozero | 132.390 | 124.978 | 127.788 | 140.031 | 1.167x | 0.791x |
| original_nozero | 138.627 | 131.090 | 135.706 | 143.485 | 1.114x | 0.755x |
| original | 154.435 | 143.889 | 151.485 | 156.887 | 1.000x | 0.678x |
| shared_b2_fast_nozero | 167.217 | 156.763 | 164.105 | 174.439 | 0.924x | 0.626x |
| shared_b2_accurate_nozero | 225.226 | 203.356 | 217.715 | 228.652 | 0.686x | 0.465x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b64_fast_nozero | 123.061 | 120.453 | 121.805 | 124.297 | 2.449x | 1.722x |
| shared_b64_accurate_nozero | 125.306 | 121.940 | 124.945 | 125.981 | 2.405x | 1.691x |
| shared_b32_fast_nozero | 128.986 | 125.433 | 126.820 | 131.854 | 2.336x | 1.643x |
| shared_bfull_accurate_nozero | 131.039 | 128.511 | 129.409 | 133.792 | 2.300x | 1.617x |
| shared_b32_accurate_nozero | 134.226 | 129.882 | 133.243 | 135.810 | 2.245x | 1.579x |
| shared_bfull_fast_nozero | 134.826 | 132.927 | 133.217 | 135.531 | 2.235x | 1.572x |
| shared_b16_fast_nozero | 143.677 | 140.595 | 142.720 | 144.969 | 2.097x | 1.475x |
| shared_b16_accurate_nozero | 154.274 | 145.183 | 153.017 | 155.130 | 1.953x | 1.374x |
| shared_b8_fast_nozero | 168.910 | 164.278 | 166.906 | 169.388 | 1.784x | 1.255x |
| shared_b8_accurate_nozero | 192.308 | 182.764 | 189.410 | 194.948 | 1.567x | 1.102x |
| mkl_bf16_inputs | 208.295 | 207.941 | 208.063 | 208.531 | 1.447x | 1.017x |
| mkl_fp32 | 211.924 | 211.080 | 211.597 | 212.143 | 1.422x | 1.000x |
| shared_b4_fast_nozero | 221.873 | 213.106 | 219.143 | 223.071 | 1.358x | 0.955x |
| shared_b4_accurate_nozero | 271.641 | 260.194 | 270.565 | 275.276 | 1.109x | 0.780x |
| original_nozero | 285.627 | 273.919 | 278.694 | 286.272 | 1.055x | 0.742x |
| original | 301.331 | 288.321 | 297.081 | 307.267 | 1.000x | 0.703x |
| shared_b2_fast_nozero | 343.506 | 336.189 | 340.943 | 349.483 | 0.877x | 0.617x |
| shared_b2_accurate_nozero | 446.841 | 432.966 | 435.776 | 454.026 | 0.674x | 0.474x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.076e-03 | 1.111e+01 | 3.980e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.053e-03 | 1.111e+01 | 3.972e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 1.602e-06 | 2.124e-02 | 7.606e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.053e-03 | 1.111e+01 | 3.972e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 1.602e-06 | 2.124e-02 | 7.606e-06 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.053e-03 | 1.111e+01 | 3.972e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.308e-03 | 6.220e+00 | 2.227e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 6.811e-03 | 1.458e+01 | 5.211e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.585e-06 | 2.417e-02 | 8.655e-06 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.053e-03 | 1.111e+01 | 3.970e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.927e-03 | 6.762e+00 | 2.421e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.460e-03 | 1.606e+01 | 5.741e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.135e-06 | 1.538e-02 | 5.507e-06 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.053e-03 | 1.111e+01 | 3.971e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.115e-03 | 7.335e+00 | 2.626e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.678e-03 | 1.687e+01 | 6.030e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 9.923e-07 | 1.123e-02 | 4.021e-06 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.053e-03 | 1.111e+01 | 3.972e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.219e-03 | 7.950e+00 | 2.847e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 7.790e-03 | 1.755e+01 | 6.271e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.896e-06 | 7.568e-03 | 2.710e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.053e-03 | 1.112e+01 | 3.974e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.231e-03 | 8.240e+00 | 2.951e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 7.805e-03 | 1.690e+01 | 6.040e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.528e-06 | 1.062e-02 | 3.803e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.054e-03 | 1.112e+01 | 3.973e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.257e-03 | 7.935e+00 | 2.841e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 7.831e-03 | 1.905e+01 | 6.808e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 5.430e-06 | 1.208e-02 | 4.327e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.055e-03 | 1.112e+01 | 3.975e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.856e-03 | 1.662e+01 | 5.941e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 8.823e-06 | 2.722e-02 | 9.747e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.058e-03 | 1.113e+01 | 3.978e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.308e-03 | 6.220e+00 | 2.227e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.585e-06 | 2.417e-02 | 8.655e-06 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.927e-03 | 6.762e+00 | 2.421e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.135e-06 | 1.538e-02 | 5.507e-06 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.115e-03 | 7.335e+00 | 2.626e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 9.923e-07 | 1.123e-02 | 4.021e-06 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.219e-03 | 7.950e+00 | 2.847e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.896e-06 | 7.568e-03 | 2.710e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.231e-03 | 8.240e+00 | 2.951e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.528e-06 | 1.062e-02 | 3.803e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.257e-03 | 7.935e+00 | 2.841e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 5.430e-06 | 1.208e-02 | 4.327e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 8.823e-06 | 2.722e-02 | 9.747e-06 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 9.375e-03 | 2.940e+05 | 9.142e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 9.290e-03 | 2.940e+05 | 9.059e-03 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 3.425e-06 | 2.860e+02 | 8.894e-06 | 1 |
| original | two_layer_e2e | mkl_fp32 | 9.290e-03 | 2.940e+05 | 9.060e-03 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 3.425e-06 | 2.860e+02 | 8.894e-06 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 9.290e-03 | 2.940e+05 | 9.060e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.182e-03 | 1.012e+05 | 3.149e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.237e-02 | 3.952e+05 | 1.218e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.723e-06 | 7.580e+02 | 2.357e-05 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.291e-03 | 2.947e+05 | 9.082e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.763e-03 | 1.522e+05 | 4.732e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.392e-02 | 4.461e+05 | 1.375e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.240e-06 | 2.220e+02 | 6.904e-06 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.290e-03 | 2.939e+05 | 9.058e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.325e-03 | 1.714e+05 | 5.331e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.448e-02 | 4.654e+05 | 1.434e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.449e-06 | 1.220e+02 | 3.794e-06 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.290e-03 | 2.940e+05 | 9.059e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.546e-03 | 1.782e+05 | 5.543e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.470e-02 | 4.722e+05 | 1.455e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.039e-06 | 1.800e+02 | 5.598e-06 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.292e-03 | 2.940e+05 | 9.059e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.590e-03 | 1.789e+05 | 5.563e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.474e-02 | 4.728e+05 | 1.457e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 5.302e-06 | 1.600e+02 | 4.976e-06 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.294e-03 | 2.941e+05 | 9.064e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.621e-03 | 1.809e+05 | 5.624e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.477e-02 | 4.748e+05 | 1.463e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 9.578e-06 | 3.080e+02 | 9.578e-06 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.298e-03 | 2.943e+05 | 9.068e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 1.943e+05 | 6.043e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.486e-02 | 4.803e+05 | 1.480e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.625e-05 | 5.360e+02 | 1.667e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.304e-03 | 2.944e+05 | 9.072e-03 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 9.8259 | 58 | 169.231 |
| original | row_schedule | 0.0215 | 58 | 169.231 |
| original | partial_zero | 0.0284 | 58 | 169.231 |
| original | csr_prefetch | 11.7409 | 58 | 169.231 |
| original | feature_gather_decode | 4.1492 | 58 | 169.231 |
| original | fp32_neighbor_reduction | 0.0000 | 58 | 169.231 |
| original | partial_bf16_conversion | 0.0000 | 58 | 169.231 |
| original | tile_load | 23.4940 | 58 | 169.231 |
| original | tile_compute | 31.7478 | 58 | 169.231 |
| original | tile_store | 0.0229 | 58 | 169.231 |
| original | output_scatter | 0.0298 | 58 | 169.231 |
| original | thread_amx_setup | 4.6203 | 58 | 169.231 |

## rgg_n_2_24_s0

N=16777216, E=265114400, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| original_nozero | 395.863 | 395.320 | 395.665 | 396.056 | 2.736x | 1.182x |
| shared_b64_fast_nozero | 405.167 | 404.696 | 404.927 | 405.502 | 2.673x | 1.154x |
| shared_bfull_fast_nozero | 405.401 | 404.956 | 405.101 | 405.634 | 2.672x | 1.154x |
| shared_b32_fast_nozero | 405.418 | 404.717 | 405.103 | 405.577 | 2.672x | 1.154x |
| shared_b16_fast_nozero | 427.269 | 426.519 | 426.960 | 427.437 | 2.535x | 1.095x |
| shared_b32_accurate_nozero | 437.414 | 437.061 | 437.251 | 437.646 | 2.476x | 1.069x |
| shared_b64_accurate_nozero | 437.627 | 436.910 | 437.492 | 437.800 | 2.475x | 1.069x |
| shared_bfull_accurate_nozero | 437.672 | 437.304 | 437.554 | 437.805 | 2.475x | 1.069x |
| shared_b16_accurate_nozero | 457.905 | 457.170 | 457.603 | 458.037 | 2.366x | 1.021x |
| mkl_bf16_inputs | 460.578 | 455.015 | 458.678 | 461.514 | 2.352x | 1.016x |
| mkl_fp32 | 467.736 | 466.146 | 466.296 | 469.275 | 2.316x | 1.000x |
| shared_b8_fast_nozero | 480.817 | 480.408 | 480.591 | 481.312 | 2.253x | 0.973x |
| shared_b8_accurate_nozero | 508.113 | 505.613 | 507.279 | 508.778 | 2.132x | 0.921x |
| shared_b4_fast_nozero | 547.616 | 544.554 | 547.202 | 547.824 | 1.978x | 0.854x |
| shared_b4_accurate_nozero | 601.343 | 599.844 | 601.149 | 601.639 | 1.801x | 0.778x |
| shared_b2_fast_nozero | 655.177 | 653.986 | 654.779 | 656.087 | 1.653x | 0.714x |
| shared_b2_accurate_nozero | 766.279 | 763.645 | 765.661 | 766.503 | 1.414x | 0.610x |
| original | 1083.214 | 1074.671 | 1077.003 | 1092.008 | 1.000x | 0.432x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| original_nozero | 866.849 | 865.820 | 866.345 | 867.149 | 2.571x | 1.122x |
| shared_b64_fast_nozero | 888.570 | 887.570 | 888.180 | 889.212 | 2.508x | 1.094x |
| shared_b32_fast_nozero | 888.673 | 887.229 | 887.899 | 889.222 | 2.508x | 1.094x |
| shared_bfull_fast_nozero | 889.097 | 888.385 | 888.461 | 889.825 | 2.507x | 1.094x |
| shared_b16_fast_nozero | 928.358 | 926.592 | 927.818 | 928.747 | 2.401x | 1.047x |
| shared_b64_accurate_nozero | 952.957 | 952.265 | 952.636 | 953.437 | 2.339x | 1.020x |
| shared_bfull_accurate_nozero | 953.099 | 952.138 | 952.826 | 953.784 | 2.338x | 1.020x |
| shared_b32_accurate_nozero | 953.208 | 951.902 | 952.660 | 953.687 | 2.338x | 1.020x |
| mkl_fp32 | 972.404 | 967.720 | 970.673 | 973.499 | 2.292x | 1.000x |
| shared_b16_accurate_nozero | 995.646 | 994.932 | 995.286 | 996.089 | 2.238x | 0.977x |
| shared_b8_fast_nozero | 1038.552 | 1037.100 | 1037.804 | 1039.085 | 2.146x | 0.936x |
| mkl_bf16_inputs | 1044.853 | 1039.364 | 1043.872 | 1047.041 | 2.133x | 0.931x |
| shared_b8_accurate_nozero | 1102.495 | 1100.995 | 1102.238 | 1104.864 | 2.022x | 0.882x |
| shared_b4_fast_nozero | 1185.317 | 1184.558 | 1184.865 | 1186.019 | 1.880x | 0.820x |
| shared_b4_accurate_nozero | 1287.339 | 1284.467 | 1286.874 | 1287.675 | 1.731x | 0.755x |
| shared_b2_fast_nozero | 1401.033 | 1392.829 | 1400.612 | 1401.522 | 1.591x | 0.694x |
| shared_b2_accurate_nozero | 1613.783 | 1611.554 | 1613.493 | 1615.316 | 1.381x | 0.603x |
| original | 2228.749 | 2221.759 | 2225.155 | 2230.737 | 1.000x | 0.436x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.304e-03 | 6.379e-01 | 5.692e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.278e-03 | 6.379e-01 | 5.667e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 2.342e-07 | 7.629e-05 | 6.808e-07 | 1 |
| original | kernel_full | mkl_fp32 | 5.278e-03 | 6.378e-01 | 5.667e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 2.342e-07 | 7.629e-05 | 6.808e-07 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.278e-03 | 6.378e-01 | 5.667e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.351e-03 | 2.750e-01 | 2.454e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 7.037e-03 | 8.580e-01 | 7.623e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.342e-07 | 8.392e-05 | 7.488e-07 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.278e-03 | 6.378e-01 | 5.667e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.942e-03 | 3.432e-01 | 3.062e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.672e-03 | 8.792e-01 | 7.811e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.129e-07 | 7.629e-05 | 6.808e-07 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.278e-03 | 6.378e-01 | 5.667e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.127e-03 | 3.541e-01 | 3.159e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.895e-03 | 9.028e-01 | 8.021e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 5.731e-07 | 2.069e-04 | 1.847e-06 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.278e-03 | 6.378e-01 | 5.667e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.232e-03 | 3.901e-01 | 3.481e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 8.012e-03 | 9.304e-01 | 8.266e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.685e-06 | 4.463e-04 | 3.982e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.279e-03 | 6.379e-01 | 5.668e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.268e-03 | 4.046e-01 | 3.610e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 8.057e-03 | 9.401e-01 | 8.352e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.069e-06 | 6.371e-04 | 5.684e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.279e-03 | 6.378e-01 | 5.667e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.268e-03 | 4.046e-01 | 3.610e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 8.057e-03 | 9.401e-01 | 8.352e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.069e-06 | 6.847e-04 | 6.110e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.279e-03 | 6.378e-01 | 5.667e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.268e-03 | 4.046e-01 | 3.610e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.057e-03 | 9.401e-01 | 8.352e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.069e-06 | 6.847e-04 | 6.110e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.279e-03 | 6.378e-01 | 5.667e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.351e-03 | 2.750e-01 | 2.454e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.342e-07 | 8.392e-05 | 7.488e-07 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.942e-03 | 3.432e-01 | 3.062e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.129e-07 | 7.629e-05 | 6.808e-07 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.127e-03 | 3.541e-01 | 3.159e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 5.731e-07 | 2.069e-04 | 1.847e-06 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.232e-03 | 3.901e-01 | 3.481e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.685e-06 | 4.463e-04 | 3.982e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.268e-03 | 4.046e-01 | 3.610e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.069e-06 | 6.371e-04 | 5.684e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.268e-03 | 4.046e-01 | 3.610e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.069e-06 | 6.847e-04 | 6.110e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.268e-03 | 4.046e-01 | 3.610e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.069e-06 | 6.847e-04 | 6.110e-06 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.035e-02 | 1.168e+02 | 1.104e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.024e-02 | 1.168e+02 | 1.092e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 9.622e-06 | 5.015e-01 | 4.737e-05 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.024e-02 | 1.169e+02 | 1.092e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 9.622e-06 | 5.015e-01 | 4.737e-05 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.024e-02 | 1.169e+02 | 1.092e-02 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.468e-03 | 3.992e+01 | 3.771e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.341e-02 | 1.550e+02 | 1.449e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 9.590e-06 | 5.792e-01 | 5.472e-05 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.024e-02 | 1.168e+02 | 1.092e-02 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.813e-03 | 5.405e+01 | 5.106e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.478e-02 | 1.700e+02 | 1.589e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 9.143e-06 | 4.817e-01 | 4.551e-05 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.024e-02 | 1.168e+02 | 1.092e-02 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.394e-03 | 6.161e+01 | 5.821e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.533e-02 | 1.715e+02 | 1.603e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.109e-05 | 5.797e-01 | 5.477e-05 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.024e-02 | 1.168e+02 | 1.092e-02 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.705e-03 | 6.194e+01 | 5.852e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.559e-02 | 1.775e+02 | 1.660e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.090e-05 | 7.219e-01 | 6.820e-05 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.024e-02 | 1.169e+02 | 1.093e-02 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.869e-03 | 6.943e+01 | 6.559e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.573e-02 | 1.818e+02 | 1.700e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.454e-05 | 7.803e-01 | 7.372e-05 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.024e-02 | 1.168e+02 | 1.092e-02 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.869e-03 | 6.943e+01 | 6.559e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.573e-02 | 1.818e+02 | 1.700e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.454e-05 | 7.803e-01 | 7.372e-05 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.024e-02 | 1.168e+02 | 1.092e-02 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.869e-03 | 6.943e+01 | 6.559e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.573e-02 | 1.818e+02 | 1.700e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.454e-05 | 7.803e-01 | 7.372e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.024e-02 | 1.168e+02 | 1.092e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 671.9508 | 4097 | 1075.635 |
| original | row_schedule | 1.3504 | 4097 | 1075.635 |
| original | partial_zero | 1.5230 | 4097 | 1075.635 |
| original | csr_prefetch | 24.8001 | 4097 | 1075.635 |
| original | feature_gather_decode | 9.4426 | 4097 | 1075.635 |
| original | fp32_neighbor_reduction | 0.0000 | 4097 | 1075.635 |
| original | partial_bf16_conversion | 0.0000 | 4097 | 1075.635 |
| original | tile_load | 37.6935 | 4097 | 1075.635 |
| original | tile_compute | 67.3313 | 4097 | 1075.635 |
| original | tile_store | 1.6708 | 4097 | 1075.635 |
| original | output_scatter | 1.4522 | 4097 | 1075.635 |
| original | thread_amx_setup | 0.0827 | 4097 | 1075.635 |

## roadNet-CA

N=1971281, E=5533214, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b32_fast_nozero | 13.299 | 13.208 | 13.281 | 13.315 | 7.396x | 1.872x |
| shared_b16_fast_nozero | 13.309 | 13.242 | 13.277 | 13.381 | 7.390x | 1.870x |
| shared_b4_fast_nozero | 13.318 | 13.287 | 13.294 | 13.360 | 7.385x | 1.869x |
| shared_b8_fast_nozero | 13.333 | 13.269 | 13.305 | 13.373 | 7.376x | 1.867x |
| shared_b64_fast_nozero | 13.526 | 13.264 | 13.447 | 13.604 | 7.271x | 1.840x |
| original_nozero | 13.695 | 13.680 | 13.687 | 13.706 | 7.181x | 1.817x |
| shared_bfull_fast_nozero | 13.804 | 13.321 | 13.396 | 13.883 | 7.125x | 1.803x |
| shared_b4_accurate_nozero | 14.662 | 14.591 | 14.621 | 14.746 | 6.708x | 1.698x |
| shared_b8_accurate_nozero | 14.699 | 14.604 | 14.668 | 14.736 | 6.691x | 1.693x |
| shared_b16_accurate_nozero | 14.742 | 14.670 | 14.689 | 14.751 | 6.672x | 1.688x |
| shared_b32_accurate_nozero | 14.820 | 14.675 | 14.745 | 14.900 | 6.636x | 1.679x |
| shared_b64_accurate_nozero | 14.961 | 14.709 | 14.759 | 15.007 | 6.574x | 1.664x |
| shared_bfull_accurate_nozero | 15.135 | 14.668 | 14.750 | 15.301 | 6.498x | 1.645x |
| shared_b2_fast_nozero | 16.010 | 15.915 | 15.957 | 16.041 | 6.143x | 1.555x |
| shared_b2_accurate_nozero | 18.523 | 18.483 | 18.506 | 18.599 | 5.309x | 1.344x |
| mkl_bf16_inputs | 24.767 | 24.553 | 24.726 | 24.887 | 3.971x | 1.005x |
| mkl_fp32 | 24.890 | 24.605 | 24.789 | 24.940 | 3.951x | 1.000x |
| original | 98.349 | 94.748 | 95.546 | 102.281 | 1.000x | 0.253x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b32_fast_nozero | 34.918 | 34.751 | 34.800 | 35.345 | 5.938x | 1.525x |
| shared_b4_fast_nozero | 34.930 | 34.690 | 34.736 | 35.200 | 5.936x | 1.524x |
| shared_b8_fast_nozero | 35.001 | 34.599 | 34.825 | 35.250 | 5.924x | 1.521x |
| shared_b16_fast_nozero | 35.092 | 34.731 | 35.037 | 35.609 | 5.909x | 1.517x |
| original_nozero | 35.123 | 35.099 | 35.109 | 35.307 | 5.904x | 1.516x |
| shared_b64_fast_nozero | 35.257 | 34.869 | 35.006 | 35.342 | 5.881x | 1.510x |
| shared_bfull_fast_nozero | 35.567 | 35.024 | 35.116 | 35.699 | 5.830x | 1.497x |
| shared_b4_accurate_nozero | 37.695 | 37.386 | 37.644 | 38.038 | 5.501x | 1.413x |
| shared_b8_accurate_nozero | 37.766 | 37.482 | 37.543 | 38.289 | 5.491x | 1.410x |
| shared_b16_accurate_nozero | 37.769 | 37.317 | 37.552 | 37.871 | 5.490x | 1.410x |
| shared_b32_accurate_nozero | 37.812 | 37.496 | 37.549 | 37.934 | 5.484x | 1.408x |
| shared_bfull_accurate_nozero | 37.894 | 37.621 | 37.832 | 38.840 | 5.472x | 1.405x |
| shared_b64_accurate_nozero | 37.989 | 37.618 | 37.786 | 38.058 | 5.458x | 1.402x |
| shared_b2_fast_nozero | 40.030 | 39.720 | 39.796 | 40.285 | 5.180x | 1.330x |
| shared_b2_accurate_nozero | 45.277 | 44.841 | 45.003 | 45.474 | 4.580x | 1.176x |
| mkl_fp32 | 53.246 | 53.093 | 53.212 | 53.325 | 3.894x | 1.000x |
| mkl_bf16_inputs | 60.743 | 60.175 | 60.614 | 61.035 | 3.414x | 0.877x |
| original | 207.359 | 194.216 | 199.131 | 209.777 | 1.000x | 0.257x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.318e-03 | 2.578e-01 | 5.048e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.292e-03 | 2.578e-01 | 5.022e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 1.260e-07 | 1.907e-05 | 3.734e-07 | 1 |
| original | kernel_full | mkl_fp32 | 5.292e-03 | 2.578e-01 | 5.022e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 1.260e-07 | 1.907e-05 | 3.734e-07 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.292e-03 | 2.578e-01 | 5.022e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.089e-03 | 1.074e-01 | 2.103e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 6.718e-03 | 3.252e-01 | 6.335e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.278e-07 | 2.098e-05 | 4.108e-07 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.292e-03 | 2.578e-01 | 5.022e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.755e-03 | 1.362e-01 | 2.666e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.458e-03 | 3.435e-01 | 6.691e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.223e-07 | 2.289e-05 | 4.481e-07 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.292e-03 | 2.578e-01 | 5.022e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 2.760e-03 | 1.509e-01 | 2.955e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.464e-03 | 4.087e-01 | 7.962e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.225e-07 | 6.485e-05 | 1.270e-06 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.292e-03 | 2.578e-01 | 5.022e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 2.760e-03 | 1.509e-01 | 2.955e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 7.464e-03 | 4.087e-01 | 7.962e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.225e-07 | 6.485e-05 | 1.270e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.292e-03 | 2.578e-01 | 5.022e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 2.760e-03 | 1.509e-01 | 2.955e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 7.464e-03 | 4.087e-01 | 7.962e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.225e-07 | 6.485e-05 | 1.270e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.292e-03 | 2.578e-01 | 5.022e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 2.760e-03 | 1.509e-01 | 2.955e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 7.464e-03 | 4.087e-01 | 7.962e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.225e-07 | 6.485e-05 | 1.270e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.292e-03 | 2.578e-01 | 5.022e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 2.760e-03 | 1.509e-01 | 2.955e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.464e-03 | 4.087e-01 | 7.962e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.225e-07 | 6.485e-05 | 1.270e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.292e-03 | 2.578e-01 | 5.022e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.089e-03 | 1.074e-01 | 2.103e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.278e-07 | 2.098e-05 | 4.108e-07 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.755e-03 | 1.362e-01 | 2.666e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.223e-07 | 2.289e-05 | 4.481e-07 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.760e-03 | 1.509e-01 | 2.955e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.225e-07 | 6.485e-05 | 1.270e-06 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.760e-03 | 1.509e-01 | 2.955e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.225e-07 | 6.485e-05 | 1.270e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.760e-03 | 1.509e-01 | 2.955e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.225e-07 | 6.485e-05 | 1.270e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.760e-03 | 1.509e-01 | 2.955e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.225e-07 | 6.485e-05 | 1.270e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.760e-03 | 1.509e-01 | 2.955e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.225e-07 | 6.485e-05 | 1.270e-06 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.054e-02 | 7.927e+00 | 1.052e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.043e-02 | 7.927e+00 | 1.041e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 1.502e-05 | 1.250e-01 | 1.659e-04 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.043e-02 | 7.927e+00 | 1.041e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 1.502e-05 | 1.250e-01 | 1.659e-04 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.043e-02 | 7.927e+00 | 1.041e-02 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.549e-03 | 2.980e+00 | 3.955e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.308e-02 | 1.091e+01 | 1.432e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.511e-05 | 2.319e-01 | 3.078e-04 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.043e-02 | 7.927e+00 | 1.041e-02 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.091e-03 | 3.620e+00 | 4.803e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.472e-02 | 1.097e+01 | 1.441e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.519e-05 | 1.251e-01 | 1.660e-04 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.043e-02 | 7.927e+00 | 1.041e-02 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.112e-03 | 4.183e+00 | 5.552e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.474e-02 | 1.096e+01 | 1.439e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.514e-05 | 1.251e-01 | 1.660e-04 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.043e-02 | 7.927e+00 | 1.041e-02 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.112e-03 | 4.183e+00 | 5.552e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.474e-02 | 1.124e+01 | 1.477e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.514e-05 | 1.251e-01 | 1.660e-04 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.043e-02 | 7.927e+00 | 1.041e-02 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.112e-03 | 4.183e+00 | 5.552e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.474e-02 | 1.124e+01 | 1.477e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.514e-05 | 1.251e-01 | 1.660e-04 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.043e-02 | 7.927e+00 | 1.041e-02 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.112e-03 | 4.183e+00 | 5.552e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.474e-02 | 1.124e+01 | 1.477e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.514e-05 | 1.251e-01 | 1.660e-04 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.043e-02 | 7.927e+00 | 1.041e-02 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.112e-03 | 4.183e+00 | 5.552e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.474e-02 | 1.124e+01 | 1.477e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.514e-05 | 1.251e-01 | 1.660e-04 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.043e-02 | 7.927e+00 | 1.041e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 79.4091 | 483 | 93.466 |
| original | row_schedule | 0.0823 | 483 | 93.466 |
| original | partial_zero | 0.1726 | 483 | 93.466 |
| original | csr_prefetch | 0.2890 | 483 | 93.466 |
| original | feature_gather_decode | 0.3281 | 483 | 93.466 |
| original | fp32_neighbor_reduction | 0.0000 | 483 | 93.466 |
| original | partial_bf16_conversion | 0.0000 | 483 | 93.466 |
| original | tile_load | 0.8402 | 483 | 93.466 |
| original | tile_compute | 1.2331 | 483 | 93.466 |
| original | tile_store | 0.1652 | 483 | 93.466 |
| original | output_scatter | 0.1416 | 483 | 93.466 |
| original | thread_amx_setup | 4.5824 | 483 | 93.466 |

## road_usa

N=None, E=None, threads=None, status=INCOMPLETE

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|

## soc-LiveJournal1

N=4847571, E=68993773, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 92.422 | 91.110 | 91.848 | 92.577 | 3.369x | 1.476x |
| shared_b64_fast_nozero | 92.895 | 91.637 | 91.939 | 93.863 | 3.351x | 1.469x |
| shared_b32_fast_nozero | 94.313 | 93.097 | 93.377 | 95.651 | 3.301x | 1.447x |
| shared_bfull_accurate_nozero | 96.064 | 95.678 | 95.949 | 96.285 | 3.241x | 1.420x |
| shared_b64_accurate_nozero | 98.410 | 96.741 | 97.855 | 99.433 | 3.164x | 1.386x |
| shared_b16_fast_nozero | 101.182 | 98.790 | 99.391 | 101.976 | 3.077x | 1.348x |
| original_nozero | 101.498 | 96.838 | 97.532 | 103.266 | 3.067x | 1.344x |
| shared_b32_accurate_nozero | 101.944 | 100.456 | 100.723 | 103.207 | 3.054x | 1.338x |
| shared_b16_accurate_nozero | 106.915 | 105.372 | 105.684 | 108.052 | 2.912x | 1.276x |
| shared_b8_fast_nozero | 110.270 | 106.910 | 107.938 | 111.043 | 2.823x | 1.237x |
| shared_b8_accurate_nozero | 120.798 | 116.594 | 118.132 | 121.606 | 2.577x | 1.129x |
| shared_b4_fast_nozero | 127.990 | 124.847 | 126.918 | 129.068 | 2.432x | 1.066x |
| mkl_fp32 | 136.433 | 136.064 | 136.392 | 136.678 | 2.282x | 1.000x |
| mkl_bf16_inputs | 138.872 | 135.254 | 138.553 | 138.933 | 2.242x | 0.982x |
| shared_b4_accurate_nozero | 144.921 | 137.724 | 143.454 | 146.368 | 2.148x | 0.941x |
| shared_b2_fast_nozero | 162.414 | 154.477 | 160.897 | 165.350 | 1.917x | 0.840x |
| shared_b2_accurate_nozero | 190.300 | 181.270 | 182.188 | 196.135 | 1.636x | 0.717x |
| original | 311.330 | 304.974 | 309.888 | 315.114 | 1.000x | 0.438x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 203.316 | 202.211 | 202.594 | 203.496 | 3.072x | 1.405x |
| shared_b64_fast_nozero | 204.889 | 203.460 | 204.134 | 205.481 | 3.048x | 1.394x |
| shared_b32_fast_nozero | 207.243 | 206.784 | 206.941 | 208.969 | 3.013x | 1.378x |
| shared_bfull_accurate_nozero | 213.394 | 212.564 | 212.838 | 214.285 | 2.927x | 1.339x |
| shared_b64_accurate_nozero | 217.414 | 215.377 | 216.417 | 218.094 | 2.873x | 1.314x |
| shared_b16_fast_nozero | 221.843 | 218.517 | 219.906 | 223.054 | 2.815x | 1.288x |
| original_nozero | 223.347 | 215.782 | 221.589 | 226.051 | 2.796x | 1.279x |
| shared_b32_accurate_nozero | 223.383 | 221.187 | 223.130 | 223.926 | 2.796x | 1.279x |
| shared_b16_accurate_nozero | 233.178 | 231.845 | 232.510 | 235.369 | 2.678x | 1.225x |
| shared_b8_fast_nozero | 241.372 | 237.117 | 240.140 | 242.632 | 2.587x | 1.183x |
| shared_b8_accurate_nozero | 257.253 | 255.192 | 256.262 | 258.666 | 2.428x | 1.110x |
| shared_b4_fast_nozero | 278.665 | 275.325 | 276.132 | 280.900 | 2.241x | 1.025x |
| mkl_fp32 | 285.660 | 284.976 | 285.454 | 286.016 | 2.186x | 1.000x |
| mkl_bf16_inputs | 302.218 | 301.797 | 302.024 | 302.269 | 2.066x | 0.945x |
| shared_b4_accurate_nozero | 310.385 | 302.284 | 304.793 | 315.409 | 2.012x | 0.920x |
| shared_b2_fast_nozero | 340.536 | 331.295 | 338.976 | 342.494 | 1.834x | 0.839x |
| shared_b2_accurate_nozero | 406.526 | 395.327 | 401.193 | 409.784 | 1.536x | 0.703x |
| original | 624.526 | 611.349 | 622.346 | 627.825 | 1.000x | 0.457x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.308e-03 | 8.901e+00 | 4.186e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.283e-03 | 8.901e+00 | 4.171e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 4.353e-07 | 1.550e-02 | 7.291e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.283e-03 | 8.902e+00 | 4.172e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 4.353e-07 | 1.550e-02 | 7.291e-06 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.283e-03 | 8.902e+00 | 4.172e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.340e-03 | 4.525e+00 | 2.128e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 7.031e-03 | 1.185e+01 | 5.554e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.356e-07 | 1.514e-02 | 7.119e-06 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.283e-03 | 8.902e+00 | 4.171e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.928e-03 | 5.171e+00 | 2.432e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.659e-03 | 1.279e+01 | 5.993e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.402e-07 | 1.318e-02 | 6.201e-06 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.283e-03 | 8.901e+00 | 4.171e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.107e-03 | 6.149e+00 | 2.892e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.875e-03 | 1.363e+01 | 6.388e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 6.116e-07 | 6.958e-03 | 3.273e-06 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.283e-03 | 8.902e+00 | 4.172e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.203e-03 | 6.637e+00 | 3.122e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 7.980e-03 | 1.444e+01 | 6.765e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.696e-06 | 5.981e-03 | 2.813e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.283e-03 | 8.900e+00 | 4.171e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.222e-03 | 6.348e+00 | 2.986e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 8.002e-03 | 1.412e+01 | 6.618e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.046e-06 | 7.080e-03 | 3.330e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.283e-03 | 8.901e+00 | 4.171e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.236e-03 | 6.039e+00 | 2.840e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 8.018e-03 | 1.376e+01 | 6.447e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.280e-06 | 8.972e-03 | 4.220e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.284e-03 | 8.899e+00 | 4.170e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.239e-03 | 6.959e+00 | 3.273e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.020e-03 | 1.459e+01 | 6.837e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 5.203e-06 | 1.929e-02 | 9.071e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.284e-03 | 8.908e+00 | 4.174e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.340e-03 | 4.525e+00 | 2.128e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.356e-07 | 1.514e-02 | 7.119e-06 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.928e-03 | 5.171e+00 | 2.432e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.402e-07 | 1.318e-02 | 6.201e-06 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.107e-03 | 6.149e+00 | 2.892e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 6.116e-07 | 6.958e-03 | 3.273e-06 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.203e-03 | 6.637e+00 | 3.122e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.696e-06 | 5.981e-03 | 2.813e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.222e-03 | 6.348e+00 | 2.986e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.046e-06 | 7.080e-03 | 3.330e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.236e-03 | 6.039e+00 | 2.840e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.280e-06 | 8.972e-03 | 4.220e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.239e-03 | 6.959e+00 | 3.273e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 5.203e-06 | 1.929e-02 | 9.071e-06 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.015e-02 | 3.935e+04 | 9.963e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.005e-02 | 3.935e+04 | 9.869e-03 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 1.031e-05 | 2.825e+01 | 7.152e-06 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.005e-02 | 3.936e+04 | 9.872e-03 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 1.031e-05 | 2.825e+01 | 7.152e-06 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.005e-02 | 3.936e+04 | 9.872e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.042e-03 | 1.123e+04 | 2.843e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.294e-02 | 5.028e+04 | 1.261e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.129e-05 | 1.512e+02 | 3.829e-05 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.005e-02 | 3.936e+04 | 9.871e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.555e-03 | 1.759e+04 | 4.453e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.445e-02 | 5.680e+04 | 1.424e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 9.872e-06 | 2.675e+01 | 6.772e-06 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.005e-02 | 3.937e+04 | 9.873e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.246e-03 | 2.069e+04 | 5.238e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.515e-02 | 5.977e+04 | 1.499e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 5.932e-06 | 1.500e+01 | 3.798e-06 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.005e-02 | 3.934e+04 | 9.867e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.548e-03 | 2.170e+04 | 5.495e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.544e-02 | 6.066e+04 | 1.521e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.058e-05 | 1.300e+01 | 3.291e-06 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.005e-02 | 3.935e+04 | 9.870e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.612e-03 | 2.214e+04 | 5.606e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.549e-02 | 6.126e+04 | 1.536e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.428e-05 | 1.650e+01 | 4.177e-06 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.005e-02 | 3.936e+04 | 9.871e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.687e-03 | 2.212e+04 | 5.601e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.555e-02 | 6.143e+04 | 1.541e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.109e-05 | 3.650e+01 | 9.241e-06 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.006e-02 | 3.938e+04 | 9.877e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.856e-03 | 2.694e+04 | 6.821e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.562e-02 | 6.629e+04 | 1.663e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.423e-05 | 5.650e+01 | 1.430e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.006e-02 | 3.938e+04 | 9.876e-03 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 193.9671 | 1185 | 336.985 |
| original | row_schedule | 0.3901 | 1185 | 336.985 |
| original | partial_zero | 0.4361 | 1185 | 336.985 |
| original | csr_prefetch | 7.2501 | 1185 | 336.985 |
| original | feature_gather_decode | 3.7463 | 1185 | 336.985 |
| original | fp32_neighbor_reduction | 0.0000 | 1185 | 336.985 |
| original | partial_bf16_conversion | 0.0000 | 1185 | 336.985 |
| original | tile_load | 19.5370 | 1185 | 336.985 |
| original | tile_compute | 24.4043 | 1185 | 336.985 |
| original | tile_store | 0.4847 | 1185 | 336.985 |
| original | output_scatter | 0.4227 | 1185 | 336.985 |
| original | thread_amx_setup | 0.7036 | 1185 | 336.985 |

## soc-Pokec

N=1632803, E=30622564, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 36.800 | 36.306 | 36.551 | 37.644 | 3.287x | 1.747x |
| shared_b64_fast_nozero | 37.460 | 36.340 | 36.547 | 37.736 | 3.229x | 1.716x |
| shared_b32_fast_nozero | 37.932 | 36.959 | 37.732 | 38.958 | 3.189x | 1.695x |
| shared_bfull_accurate_nozero | 38.427 | 38.087 | 38.310 | 38.957 | 3.148x | 1.673x |
| shared_b64_accurate_nozero | 40.012 | 38.956 | 39.892 | 40.136 | 3.023x | 1.607x |
| shared_b16_fast_nozero | 40.996 | 39.674 | 40.469 | 41.476 | 2.951x | 1.568x |
| original_nozero | 41.499 | 39.540 | 40.862 | 43.000 | 2.915x | 1.549x |
| shared_b32_accurate_nozero | 41.855 | 40.363 | 40.871 | 42.014 | 2.890x | 1.536x |
| shared_b16_accurate_nozero | 43.582 | 41.997 | 42.604 | 44.159 | 2.776x | 1.475x |
| shared_b8_fast_nozero | 45.010 | 43.844 | 44.145 | 45.859 | 2.688x | 1.428x |
| shared_b8_accurate_nozero | 49.421 | 47.131 | 48.562 | 49.819 | 2.448x | 1.301x |
| shared_b4_fast_nozero | 53.115 | 51.029 | 51.973 | 54.543 | 2.278x | 1.210x |
| shared_b4_accurate_nozero | 60.207 | 58.602 | 59.474 | 62.176 | 2.009x | 1.068x |
| mkl_fp32 | 64.282 | 64.109 | 64.226 | 64.334 | 1.882x | 1.000x |
| mkl_bf16_inputs | 64.987 | 63.190 | 64.885 | 65.018 | 1.861x | 0.989x |
| shared_b2_fast_nozero | 70.061 | 67.960 | 69.441 | 70.352 | 1.727x | 0.918x |
| shared_b2_accurate_nozero | 84.684 | 82.577 | 84.030 | 86.058 | 1.428x | 0.759x |
| original | 120.971 | 111.763 | 117.305 | 121.373 | 1.000x | 0.531x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 81.348 | 80.225 | 80.835 | 81.772 | 2.831x | 1.625x |
| shared_b64_fast_nozero | 81.562 | 80.699 | 81.335 | 82.092 | 2.824x | 1.620x |
| shared_b32_fast_nozero | 83.322 | 81.970 | 83.114 | 83.527 | 2.764x | 1.586x |
| shared_bfull_accurate_nozero | 84.915 | 83.991 | 84.635 | 85.187 | 2.712x | 1.556x |
| shared_b64_accurate_nozero | 86.758 | 85.315 | 86.246 | 86.883 | 2.654x | 1.523x |
| shared_b16_fast_nozero | 90.176 | 88.201 | 89.047 | 90.647 | 2.554x | 1.465x |
| shared_b32_accurate_nozero | 90.269 | 88.611 | 89.042 | 90.785 | 2.551x | 1.464x |
| original_nozero | 90.886 | 87.694 | 89.896 | 92.325 | 2.534x | 1.454x |
| shared_b16_accurate_nozero | 95.783 | 92.683 | 95.184 | 96.581 | 2.404x | 1.380x |
| shared_b8_fast_nozero | 98.966 | 96.115 | 97.558 | 99.865 | 2.327x | 1.335x |
| shared_b8_accurate_nozero | 106.517 | 104.709 | 105.657 | 107.538 | 2.162x | 1.241x |
| shared_b4_fast_nozero | 114.319 | 112.471 | 113.372 | 115.328 | 2.015x | 1.156x |
| shared_b4_accurate_nozero | 129.717 | 126.856 | 128.562 | 131.891 | 1.775x | 1.019x |
| mkl_fp32 | 132.150 | 131.977 | 132.100 | 132.403 | 1.743x | 1.000x |
| mkl_bf16_inputs | 139.646 | 137.150 | 139.540 | 139.736 | 1.649x | 0.946x |
| shared_b2_fast_nozero | 145.947 | 140.353 | 143.823 | 148.166 | 1.578x | 0.905x |
| shared_b2_accurate_nozero | 178.550 | 172.323 | 176.607 | 181.633 | 1.290x | 0.740x |
| original | 230.298 | 225.682 | 227.578 | 232.570 | 1.000x | 0.574x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.319e-03 | 5.965e+00 | 4.261e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.293e-03 | 5.965e+00 | 4.244e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 3.741e-07 | 1.160e-02 | 8.283e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.293e-03 | 5.968e+00 | 4.246e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 3.741e-07 | 1.160e-02 | 8.283e-06 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.293e-03 | 5.968e+00 | 4.246e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.355e-03 | 2.712e+00 | 1.937e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 7.059e-03 | 8.610e+00 | 6.125e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.740e-07 | 6.104e-03 | 4.360e-06 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.293e-03 | 5.960e+00 | 4.240e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.950e-03 | 4.213e+00 | 3.009e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.696e-03 | 9.557e+00 | 6.799e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.019e-07 | 5.432e-03 | 3.880e-06 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.293e-03 | 5.967e+00 | 4.245e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.130e-03 | 3.773e+00 | 2.695e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.914e-03 | 9.197e+00 | 6.543e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 6.111e-07 | 2.686e-03 | 1.918e-06 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.293e-03 | 5.965e+00 | 4.244e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.229e-03 | 5.043e+00 | 3.602e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 8.022e-03 | 1.063e+01 | 7.561e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.735e-06 | 3.113e-03 | 2.223e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.293e-03 | 5.964e+00 | 4.243e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.247e-03 | 4.145e+00 | 2.961e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 8.046e-03 | 9.731e+00 | 6.923e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.113e-06 | 5.493e-03 | 3.924e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.294e-03 | 5.967e+00 | 4.245e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.264e-03 | 4.400e+00 | 3.143e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 8.063e-03 | 9.986e+00 | 7.104e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.327e-06 | 6.104e-03 | 4.360e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.294e-03 | 5.966e+00 | 4.245e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.064e-03 | 9.351e+00 | 6.653e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.976e-06 | 1.331e-02 | 9.504e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.295e-03 | 5.975e+00 | 4.251e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.355e-03 | 2.712e+00 | 1.937e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.740e-07 | 6.104e-03 | 4.360e-06 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.950e-03 | 4.213e+00 | 3.009e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.019e-07 | 5.432e-03 | 3.880e-06 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.130e-03 | 3.773e+00 | 2.695e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 6.111e-07 | 2.686e-03 | 1.918e-06 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.229e-03 | 5.043e+00 | 3.602e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.735e-06 | 3.113e-03 | 2.223e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.247e-03 | 4.145e+00 | 2.961e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.113e-06 | 5.493e-03 | 3.924e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.264e-03 | 4.400e+00 | 3.143e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.327e-06 | 6.104e-03 | 4.360e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.976e-06 | 1.331e-02 | 9.504e-06 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.015e-02 | 1.593e+04 | 9.942e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.005e-02 | 1.593e+04 | 9.844e-03 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 7.647e-06 | 7.250e+00 | 4.525e-06 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.005e-02 | 1.593e+04 | 9.847e-03 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 7.647e-06 | 7.250e+00 | 4.525e-06 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.005e-02 | 1.593e+04 | 9.847e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 2.912e-03 | 4.523e+03 | 2.823e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.282e-02 | 2.045e+04 | 1.264e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 7.558e-06 | 9.375e+00 | 5.852e-06 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.005e-02 | 1.593e+04 | 9.847e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.520e-03 | 7.109e+03 | 4.437e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.442e-02 | 2.304e+04 | 1.424e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 7.496e-06 | 5.750e+00 | 3.589e-06 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.005e-02 | 1.592e+04 | 9.841e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.266e-03 | 8.265e+03 | 5.159e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.516e-02 | 2.419e+04 | 1.495e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 8.338e-06 | 3.250e+00 | 2.029e-06 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.005e-02 | 1.593e+04 | 9.845e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.562e-03 | 8.758e+03 | 5.466e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.545e-02 | 2.469e+04 | 1.526e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.518e-05 | 4.125e+00 | 2.575e-06 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.005e-02 | 1.593e+04 | 9.845e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.669e-03 | 8.903e+03 | 5.557e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.553e-02 | 2.483e+04 | 1.535e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.173e-05 | 6.250e+00 | 3.901e-06 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.005e-02 | 1.593e+04 | 9.848e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.740e-03 | 8.929e+03 | 5.574e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.557e-02 | 2.486e+04 | 1.536e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.751e-05 | 1.300e+01 | 8.114e-06 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.006e-02 | 1.594e+04 | 9.852e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 9.892e+03 | 6.175e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.561e-02 | 2.582e+04 | 1.596e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.211e-05 | 1.731e+01 | 1.081e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.006e-02 | 1.594e+04 | 9.854e-03 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 65.3231 | 400 | 122.912 |
| original | row_schedule | 0.1359 | 400 | 122.912 |
| original | partial_zero | 0.1264 | 400 | 122.912 |
| original | csr_prefetch | 2.8536 | 400 | 122.912 |
| original | feature_gather_decode | 1.6470 | 400 | 122.912 |
| original | fp32_neighbor_reduction | 0.0000 | 400 | 122.912 |
| original | partial_bf16_conversion | 0.0000 | 400 | 122.912 |
| original | tile_load | 8.5390 | 400 | 122.912 |
| original | tile_compute | 10.2594 | 400 | 122.912 |
| original | tile_store | 0.1733 | 400 | 122.912 |
| original | output_scatter | 0.1419 | 400 | 122.912 |
| original | thread_amx_setup | 3.0088 | 400 | 122.912 |

## web-Google

N=916428, E=5105039, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 7.261 | 7.208 | 7.243 | 7.320 | 6.774x | 2.125x |
| shared_b64_fast_nozero | 7.269 | 7.147 | 7.227 | 7.323 | 6.768x | 2.123x |
| shared_b32_fast_nozero | 7.271 | 7.150 | 7.216 | 7.312 | 6.765x | 2.122x |
| shared_b16_fast_nozero | 7.447 | 7.365 | 7.428 | 7.499 | 6.605x | 2.072x |
| shared_b32_accurate_nozero | 7.882 | 7.759 | 7.840 | 7.896 | 6.241x | 1.958x |
| shared_bfull_accurate_nozero | 7.893 | 7.817 | 7.868 | 7.970 | 6.232x | 1.955x |
| shared_b64_accurate_nozero | 7.959 | 7.790 | 7.902 | 7.988 | 6.181x | 1.939x |
| shared_b16_accurate_nozero | 7.982 | 7.902 | 7.959 | 8.005 | 6.163x | 1.934x |
| shared_b8_fast_nozero | 8.172 | 8.045 | 8.128 | 8.193 | 6.019x | 1.889x |
| original_nozero | 8.629 | 8.519 | 8.544 | 8.713 | 5.701x | 1.789x |
| shared_b8_accurate_nozero | 8.794 | 8.723 | 8.756 | 8.901 | 5.594x | 1.755x |
| shared_b4_fast_nozero | 9.406 | 9.235 | 9.289 | 9.502 | 5.230x | 1.641x |
| shared_b4_accurate_nozero | 10.544 | 10.318 | 10.449 | 10.624 | 4.665x | 1.464x |
| shared_b2_fast_nozero | 11.725 | 11.496 | 11.624 | 11.926 | 4.195x | 1.316x |
| shared_b2_accurate_nozero | 13.862 | 13.429 | 13.658 | 13.939 | 3.549x | 1.113x |
| mkl_bf16_inputs | 15.131 | 14.857 | 15.058 | 15.181 | 3.251x | 1.020x |
| mkl_fp32 | 15.433 | 15.010 | 15.316 | 15.479 | 3.187x | 1.000x |
| original | 49.190 | 48.227 | 49.033 | 49.279 | 1.000x | 0.314x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b64_fast_nozero | 19.300 | 18.908 | 19.212 | 20.140 | 5.114x | 1.640x |
| shared_bfull_fast_nozero | 19.317 | 18.998 | 19.136 | 20.198 | 5.109x | 1.639x |
| shared_b32_fast_nozero | 19.465 | 19.109 | 19.294 | 20.397 | 5.070x | 1.626x |
| shared_b16_fast_nozero | 19.544 | 19.342 | 19.448 | 19.730 | 5.050x | 1.619x |
| shared_b64_accurate_nozero | 20.522 | 20.327 | 20.376 | 21.537 | 4.809x | 1.542x |
| shared_b32_accurate_nozero | 20.533 | 20.249 | 20.443 | 21.600 | 4.807x | 1.541x |
| shared_bfull_accurate_nozero | 20.590 | 20.262 | 20.377 | 21.901 | 4.793x | 1.537x |
| shared_b16_accurate_nozero | 20.922 | 20.716 | 20.813 | 21.055 | 4.717x | 1.513x |
| shared_b8_fast_nozero | 21.142 | 20.667 | 20.917 | 21.243 | 4.668x | 1.497x |
| original_nozero | 21.277 | 21.099 | 21.203 | 21.384 | 4.639x | 1.488x |
| shared_b8_accurate_nozero | 22.512 | 22.368 | 22.459 | 22.766 | 4.384x | 1.406x |
| shared_b4_fast_nozero | 23.377 | 23.151 | 23.242 | 23.480 | 4.222x | 1.354x |
| shared_b4_accurate_nozero | 25.724 | 25.286 | 25.586 | 25.800 | 3.837x | 1.230x |
| shared_b2_fast_nozero | 27.747 | 27.309 | 27.576 | 28.082 | 3.557x | 1.141x |
| mkl_fp32 | 31.651 | 31.509 | 31.563 | 31.872 | 3.118x | 1.000x |
| shared_b2_accurate_nozero | 32.118 | 31.615 | 32.031 | 32.223 | 3.073x | 0.985x |
| mkl_bf16_inputs | 35.591 | 35.066 | 35.556 | 35.773 | 2.773x | 0.889x |
| original | 98.696 | 97.139 | 98.488 | 98.825 | 1.000x | 0.321x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.333e-03 | 1.204e+00 | 4.419e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.307e-03 | 1.204e+00 | 4.399e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 2.083e-07 | 2.747e-04 | 1.008e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.307e-03 | 1.204e+00 | 4.400e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 2.083e-07 | 2.747e-04 | 1.008e-06 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.307e-03 | 1.204e+00 | 4.400e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.292e-03 | 6.431e-01 | 2.361e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 6.990e-03 | 1.717e+00 | 6.276e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.092e-07 | 2.594e-04 | 9.523e-07 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.307e-03 | 1.203e+00 | 4.399e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.857e-03 | 8.133e-01 | 2.986e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.595e-03 | 2.017e+00 | 7.372e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.906e-07 | 3.052e-04 | 1.120e-06 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.307e-03 | 1.204e+00 | 4.400e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.031e-03 | 1.045e+00 | 3.836e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.804e-03 | 2.248e+00 | 8.218e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 5.015e-07 | 1.984e-04 | 7.283e-07 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.307e-03 | 1.204e+00 | 4.400e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.118e-03 | 8.726e-01 | 3.204e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 7.901e-03 | 2.076e+00 | 7.589e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.281e-06 | 4.807e-04 | 1.765e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.307e-03 | 1.204e+00 | 4.401e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.134e-03 | 8.734e-01 | 3.207e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 7.920e-03 | 2.077e+00 | 7.592e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.609e-06 | 1.030e-03 | 3.781e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.308e-03 | 1.204e+00 | 4.403e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.135e-03 | 7.666e-01 | 2.815e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 7.922e-03 | 1.953e+00 | 7.137e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.725e-06 | 1.370e-03 | 5.031e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.308e-03 | 1.204e+00 | 4.402e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.135e-03 | 1.049e+00 | 3.851e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.922e-03 | 2.197e+00 | 8.031e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.774e-06 | 2.014e-03 | 7.395e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.308e-03 | 1.204e+00 | 4.402e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.292e-03 | 6.431e-01 | 2.361e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.092e-07 | 2.594e-04 | 9.523e-07 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.857e-03 | 8.133e-01 | 2.986e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.906e-07 | 3.052e-04 | 1.120e-06 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.031e-03 | 1.045e+00 | 3.836e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 5.015e-07 | 1.984e-04 | 7.283e-07 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.118e-03 | 8.726e-01 | 3.204e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.281e-06 | 4.807e-04 | 1.765e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.134e-03 | 8.734e-01 | 3.207e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.609e-06 | 1.030e-03 | 3.781e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.135e-03 | 7.666e-01 | 2.815e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.725e-06 | 1.370e-03 | 5.031e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.135e-03 | 1.049e+00 | 3.851e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.774e-06 | 2.014e-03 | 7.395e-06 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.037e-02 | 6.874e+02 | 1.075e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.026e-02 | 6.874e+02 | 1.064e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 8.365e-06 | 6.245e-01 | 9.768e-06 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.026e-02 | 6.874e+02 | 1.064e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 8.365e-06 | 6.245e-01 | 9.768e-06 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.026e-02 | 6.874e+02 | 1.064e-02 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.408e-03 | 2.118e+02 | 3.313e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.328e-02 | 8.992e+02 | 1.392e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 8.739e-06 | 3.744e-01 | 5.856e-06 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.026e-02 | 6.874e+02 | 1.064e-02 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.783e-03 | 2.753e+02 | 4.306e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.469e-02 | 9.627e+02 | 1.490e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 8.623e-06 | 2.502e-01 | 3.914e-06 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.026e-02 | 6.874e+02 | 1.064e-02 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.375e-03 | 3.240e+02 | 5.068e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.527e-02 | 1.011e+03 | 1.565e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.121e-05 | 3.756e-01 | 5.875e-06 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.026e-02 | 6.873e+02 | 1.064e-02 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.680e-03 | 3.218e+02 | 5.033e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.553e-02 | 1.009e+03 | 1.562e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.243e-05 | 9.689e-01 | 1.516e-05 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.026e-02 | 6.872e+02 | 1.063e-02 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.806e-03 | 3.220e+02 | 5.037e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.564e-02 | 1.001e+03 | 1.549e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.714e-05 | 9.764e-01 | 1.527e-05 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.026e-02 | 6.872e+02 | 1.064e-02 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.824e-03 | 3.345e+02 | 5.232e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.565e-02 | 1.022e+03 | 1.581e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.891e-05 | 9.764e-01 | 1.527e-05 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.026e-02 | 6.874e+02 | 1.064e-02 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.832e-03 | 3.841e+02 | 6.008e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.566e-02 | 1.071e+03 | 1.658e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.882e-05 | 9.764e-01 | 1.527e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.026e-02 | 6.875e+02 | 1.064e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 36.8922 | 225 | 46.470 |
| original | row_schedule | 0.0448 | 225 | 46.470 |
| original | partial_zero | 0.0713 | 225 | 46.470 |
| original | csr_prefetch | 0.3719 | 225 | 46.470 |
| original | feature_gather_decode | 0.2267 | 225 | 46.470 |
| original | fp32_neighbor_reduction | 0.0000 | 225 | 46.470 |
| original | partial_bf16_conversion | 0.0000 | 225 | 46.470 |
| original | tile_load | 0.9537 | 225 | 46.470 |
| original | tile_compute | 1.3449 | 225 | 46.470 |
| original | tile_store | 0.0851 | 225 | 46.470 |
| original | output_scatter | 0.0715 | 225 | 46.470 |
| original | thread_amx_setup | 4.0522 | 225 | 46.470 |

## wiki-Talk

N=2394385, E=5021410, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 18.340 | 17.385 | 17.959 | 18.741 | 8.158x | 1.973x |
| shared_bfull_accurate_nozero | 19.198 | 17.804 | 18.863 | 19.509 | 7.793x | 1.885x |
| shared_b64_fast_nozero | 19.702 | 18.487 | 19.186 | 20.233 | 7.594x | 1.837x |
| shared_b64_accurate_nozero | 20.587 | 19.223 | 20.123 | 20.795 | 7.267x | 1.758x |
| shared_b32_fast_nozero | 20.991 | 19.343 | 20.808 | 21.290 | 7.128x | 1.724x |
| shared_b32_accurate_nozero | 21.906 | 21.633 | 21.787 | 22.730 | 6.830x | 1.652x |
| shared_b16_fast_nozero | 24.625 | 23.601 | 24.266 | 25.178 | 6.076x | 1.470x |
| shared_b16_accurate_nozero | 28.426 | 27.140 | 27.471 | 29.146 | 5.263x | 1.273x |
| shared_b8_fast_nozero | 32.089 | 30.786 | 31.183 | 32.257 | 4.662x | 1.128x |
| mkl_fp32 | 36.188 | 36.063 | 36.179 | 36.226 | 4.134x | 1.000x |
| mkl_bf16_inputs | 36.452 | 35.883 | 36.361 | 36.468 | 4.104x | 0.993x |
| shared_b8_accurate_nozero | 39.145 | 37.848 | 38.508 | 40.133 | 3.822x | 0.924x |
| shared_b4_fast_nozero | 47.897 | 46.273 | 46.732 | 48.222 | 3.124x | 0.756x |
| original_nozero | 52.561 | 51.321 | 52.047 | 53.184 | 2.846x | 0.689x |
| shared_b4_accurate_nozero | 61.164 | 59.247 | 60.616 | 62.259 | 2.446x | 0.592x |
| shared_b2_fast_nozero | 74.650 | 72.822 | 73.876 | 76.014 | 2.004x | 0.485x |
| shared_b2_accurate_nozero | 106.547 | 103.606 | 103.939 | 106.793 | 1.404x | 0.340x |
| original | 149.613 | 148.730 | 149.218 | 150.182 | 1.000x | 0.242x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 48.515 | 47.258 | 47.644 | 49.199 | 6.290x | 1.567x |
| shared_bfull_accurate_nozero | 48.753 | 47.902 | 48.123 | 49.245 | 6.259x | 1.559x |
| shared_b64_fast_nozero | 49.879 | 47.867 | 48.663 | 51.123 | 6.118x | 1.524x |
| shared_b64_accurate_nozero | 51.302 | 49.837 | 50.147 | 51.847 | 5.948x | 1.482x |
| shared_b32_fast_nozero | 52.516 | 51.309 | 51.841 | 53.053 | 5.810x | 1.447x |
| shared_b32_accurate_nozero | 55.861 | 53.369 | 54.584 | 56.822 | 5.462x | 1.361x |
| shared_b16_fast_nozero | 58.696 | 57.703 | 58.351 | 59.589 | 5.199x | 1.295x |
| shared_b16_accurate_nozero | 67.344 | 64.876 | 66.463 | 68.259 | 4.531x | 1.129x |
| shared_b8_fast_nozero | 74.534 | 72.209 | 74.323 | 74.975 | 4.094x | 1.020x |
| mkl_fp32 | 76.007 | 75.940 | 75.984 | 76.050 | 4.015x | 1.000x |
| mkl_bf16_inputs | 86.028 | 85.511 | 85.914 | 86.147 | 3.547x | 0.884x |
| shared_b8_accurate_nozero | 87.895 | 85.694 | 87.173 | 88.354 | 3.472x | 0.865x |
| shared_b4_fast_nozero | 104.987 | 102.533 | 104.148 | 105.278 | 2.906x | 0.724x |
| original_nozero | 112.792 | 111.059 | 112.324 | 113.262 | 2.705x | 0.674x |
| shared_b4_accurate_nozero | 133.349 | 130.404 | 132.028 | 135.340 | 2.288x | 0.570x |
| shared_b2_fast_nozero | 160.534 | 156.157 | 158.147 | 163.015 | 1.901x | 0.473x |
| shared_b2_accurate_nozero | 219.154 | 216.134 | 217.740 | 222.069 | 1.392x | 0.347x |
| original | 305.135 | 302.500 | 304.119 | 305.584 | 1.000x | 0.249x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 4.732e-03 | 2.785e+01 | 2.877e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 4.712e-03 | 2.785e+01 | 2.868e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 3.923e-06 | 1.382e-01 | 1.427e-05 | 1 |
| original | kernel_full | mkl_fp32 | 4.712e-03 | 2.784e+01 | 2.868e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 3.923e-06 | 1.382e-01 | 1.427e-05 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 4.712e-03 | 2.784e+01 | 2.868e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.164e-03 | 1.669e+01 | 1.724e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 6.396e-03 | 4.196e+01 | 4.321e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.477e-06 | 1.348e-01 | 1.392e-05 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 4.713e-03 | 2.785e+01 | 2.869e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.819e-03 | 2.431e+01 | 2.511e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.063e-03 | 5.159e+01 | 5.314e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.658e-06 | 9.668e-02 | 9.986e-06 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 4.712e-03 | 2.784e+01 | 2.868e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.000e-03 | 2.576e+01 | 2.661e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.267e-03 | 5.304e+01 | 5.463e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.323e-06 | 9.961e-02 | 1.029e-05 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 4.713e-03 | 2.787e+01 | 2.870e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.109e-03 | 2.995e+01 | 3.094e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 7.377e-03 | 5.723e+01 | 5.894e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.138e-06 | 4.688e-02 | 4.842e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 4.713e-03 | 2.787e+01 | 2.870e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.125e-03 | 2.943e+01 | 3.040e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 7.398e-03 | 5.671e+01 | 5.841e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.162e-06 | 2.734e-02 | 2.824e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 4.713e-03 | 2.785e+01 | 2.868e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.118e-03 | 2.600e+01 | 2.686e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 7.386e-03 | 5.328e+01 | 5.488e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.815e-06 | 4.980e-02 | 5.144e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 4.715e-03 | 2.785e+01 | 2.869e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.261e-03 | 2.680e+01 | 2.768e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.469e-03 | 5.144e+01 | 5.299e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 8.763e-06 | 9.277e-02 | 9.583e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 4.718e-03 | 2.791e+01 | 2.875e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.164e-03 | 1.669e+01 | 1.724e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.477e-06 | 1.348e-01 | 1.392e-05 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.819e-03 | 2.431e+01 | 2.511e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.658e-06 | 9.668e-02 | 9.986e-06 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.000e-03 | 2.576e+01 | 2.661e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.323e-06 | 9.961e-02 | 1.029e-05 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.109e-03 | 2.995e+01 | 3.094e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.138e-06 | 4.688e-02 | 4.842e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.125e-03 | 2.943e+01 | 3.040e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.162e-06 | 2.734e-02 | 2.824e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.118e-03 | 2.600e+01 | 2.686e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.815e-06 | 4.980e-02 | 5.144e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.261e-03 | 2.680e+01 | 2.768e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 8.763e-06 | 9.277e-02 | 9.583e-06 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 9.510e-03 | 4.444e+04 | 9.851e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 9.422e-03 | 4.444e+04 | 9.755e-03 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 9.824e-06 | 5.162e+01 | 1.144e-05 | 1 |
| original | two_layer_e2e | mkl_fp32 | 9.422e-03 | 4.445e+04 | 9.755e-03 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 9.824e-06 | 5.162e+01 | 1.144e-05 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 9.422e-03 | 4.445e+04 | 9.755e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.091e-03 | 1.337e+04 | 2.963e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.239e-02 | 5.781e+04 | 1.269e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.201e-05 | 1.175e+02 | 2.604e-05 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.423e-03 | 4.456e+04 | 9.781e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.734e-03 | 2.099e+04 | 4.652e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.402e-02 | 6.543e+04 | 1.436e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 7.863e-06 | 3.650e+01 | 8.090e-06 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.422e-03 | 4.448e+04 | 9.762e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.319e-03 | 2.354e+04 | 5.217e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.460e-02 | 6.798e+04 | 1.492e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 9.965e-06 | 3.000e+01 | 6.649e-06 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.423e-03 | 4.448e+04 | 9.762e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.567e-03 | 2.470e+04 | 5.476e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.485e-02 | 6.915e+04 | 1.518e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 9.949e-06 | 2.300e+01 | 5.098e-06 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.424e-03 | 4.447e+04 | 9.760e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.593e-03 | 2.488e+04 | 5.514e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.486e-02 | 6.932e+04 | 1.522e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.723e-05 | 3.000e+01 | 6.649e-06 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.427e-03 | 4.448e+04 | 9.762e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.657e-03 | 2.522e+04 | 5.589e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.491e-02 | 6.966e+04 | 1.529e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.707e-05 | 4.950e+01 | 1.097e-05 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.430e-03 | 4.449e+04 | 9.766e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.818e-03 | 2.723e+04 | 6.036e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.498e-02 | 7.168e+04 | 1.573e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.081e-05 | 5.675e+01 | 1.258e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.436e-03 | 4.450e+04 | 9.767e-03 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 95.7179 | 586 | 329.035 |
| original | row_schedule | 0.0820 | 586 | 329.035 |
| original | partial_zero | 0.2110 | 586 | 329.035 |
| original | csr_prefetch | 5.9967 | 586 | 329.035 |
| original | feature_gather_decode | 6.1452 | 586 | 329.035 |
| original | fp32_neighbor_reduction | 0.0000 | 586 | 329.035 |
| original | partial_bf16_conversion | 0.0000 | 586 | 329.035 |
| original | tile_load | 49.2606 | 586 | 329.035 |
| original | tile_compute | 44.7500 | 586 | 329.035 |
| original | tile_store | 0.1862 | 586 | 329.035 |
| original | output_scatter | 0.1733 | 586 | 329.035 |
| original | thread_amx_setup | 3.7422 | 586 | 329.035 |

