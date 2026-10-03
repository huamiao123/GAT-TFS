# GCN-extra first inference experiments

Numerical sum-aggregation experiments with random H/W; no task-accuracy claim.
Uninstrumented medians; original source is unchanged. Speedups are original/new and FP32 MKL/new.
Profiles are sampled summed thread time and do not equal wall time. Physical AMX work is modeled, not a hardware counter.

## road_usa

N=23947347, E=57708624, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| original_nozero | 175.344 | 174.953 | 175.301 | 175.430 | 6.620x | 2.173x |
| shared_b4_fast_nozero | 176.830 | 176.275 | 176.706 | 177.139 | 6.564x | 2.155x |
| shared_b8_fast_nozero | 177.822 | 176.728 | 177.337 | 179.363 | 6.528x | 2.143x |
| shared_b64_fast_nozero | 182.353 | 179.619 | 181.834 | 182.388 | 6.366x | 2.090x |
| shared_b32_fast_nozero | 182.414 | 179.962 | 182.334 | 182.591 | 6.363x | 2.089x |
| shared_b16_fast_nozero | 182.495 | 181.753 | 182.134 | 183.243 | 6.361x | 2.088x |
| shared_bfull_fast_nozero | 182.562 | 182.111 | 182.343 | 182.700 | 6.358x | 2.087x |
| shared_b4_accurate_nozero | 192.143 | 191.034 | 191.888 | 192.465 | 6.041x | 1.983x |
| shared_b8_accurate_nozero | 195.248 | 193.807 | 194.265 | 197.617 | 5.945x | 1.952x |
| shared_b32_accurate_nozero | 197.356 | 196.868 | 197.167 | 197.691 | 5.882x | 1.931x |
| shared_b64_accurate_nozero | 197.528 | 196.719 | 197.233 | 197.725 | 5.877x | 1.929x |
| shared_bfull_accurate_nozero | 197.577 | 197.004 | 197.343 | 198.269 | 5.875x | 1.929x |
| shared_b16_accurate_nozero | 197.679 | 196.366 | 197.577 | 197.775 | 5.872x | 1.928x |
| shared_b2_fast_nozero | 198.904 | 198.630 | 198.810 | 199.022 | 5.836x | 1.916x |
| shared_b2_accurate_nozero | 224.886 | 224.643 | 224.697 | 225.238 | 5.162x | 1.694x |
| mkl_bf16_inputs | 368.385 | 364.425 | 367.719 | 370.097 | 3.151x | 1.034x |
| mkl_fp32 | 381.055 | 374.790 | 380.629 | 382.016 | 3.046x | 1.000x |
| original | 1160.774 | 1154.417 | 1156.769 | 1168.619 | 1.000x | 0.328x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| original_nozero | 457.248 | 455.663 | 456.297 | 487.547 | 5.326x | 1.809x |
| shared_b4_fast_nozero | 478.964 | 471.147 | 474.936 | 480.261 | 5.085x | 1.727x |
| shared_b64_fast_nozero | 479.317 | 478.371 | 479.120 | 479.597 | 5.081x | 1.725x |
| shared_b16_fast_nozero | 479.923 | 477.879 | 478.882 | 480.785 | 5.075x | 1.723x |
| shared_b8_fast_nozero | 480.315 | 478.221 | 478.720 | 481.144 | 5.071x | 1.722x |
| shared_bfull_fast_nozero | 480.483 | 478.995 | 479.621 | 482.501 | 5.069x | 1.721x |
| shared_b32_fast_nozero | 481.326 | 478.203 | 479.539 | 481.936 | 5.060x | 1.718x |
| shared_bfull_accurate_nozero | 506.800 | 506.199 | 506.482 | 507.532 | 4.806x | 1.632x |
| shared_b16_accurate_nozero | 507.255 | 506.562 | 507.012 | 507.319 | 4.801x | 1.630x |
| shared_b64_accurate_nozero | 507.415 | 506.101 | 506.574 | 508.609 | 4.800x | 1.630x |
| shared_b8_accurate_nozero | 507.540 | 505.867 | 507.020 | 508.644 | 4.799x | 1.629x |
| shared_b32_accurate_nozero | 507.684 | 506.776 | 507.139 | 508.275 | 4.797x | 1.629x |
| shared_b4_accurate_nozero | 507.892 | 506.290 | 507.392 | 508.679 | 4.795x | 1.628x |
| shared_b2_fast_nozero | 513.846 | 509.953 | 510.296 | 541.770 | 4.740x | 1.609x |
| shared_b2_accurate_nozero | 571.890 | 559.394 | 568.359 | 574.068 | 4.259x | 1.446x |
| mkl_fp32 | 826.962 | 811.340 | 824.092 | 828.850 | 2.945x | 1.000x |
| mkl_bf16_inputs | 928.236 | 924.462 | 926.969 | 932.117 | 2.624x | 0.891x |
| original | 2435.493 | 2401.448 | 2410.868 | 2439.202 | 1.000x | 0.340x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.309e-03 | 2.550e-01 | 5.381e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.283e-03 | 2.550e-01 | 5.357e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 1.186e-07 | 1.907e-05 | 4.025e-07 | 1 |
| original | kernel_full | mkl_fp32 | 5.283e-03 | 2.550e-01 | 5.357e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 1.186e-07 | 1.907e-05 | 4.025e-07 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.283e-03 | 2.550e-01 | 5.357e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.077e-03 | 1.216e-01 | 2.567e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 6.694e-03 | 3.424e-01 | 7.193e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.206e-07 | 2.289e-05 | 4.831e-07 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.283e-03 | 2.550e-01 | 5.357e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.619e-03 | 1.604e-01 | 3.384e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.294e-03 | 3.559e-01 | 7.477e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.167e-07 | 2.289e-05 | 4.831e-07 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.283e-03 | 2.550e-01 | 5.357e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 2.620e-03 | 1.535e-01 | 3.239e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.295e-03 | 3.650e-01 | 7.668e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.167e-07 | 6.199e-05 | 1.308e-06 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.283e-03 | 2.550e-01 | 5.357e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 2.620e-03 | 1.535e-01 | 3.239e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 7.295e-03 | 3.650e-01 | 7.668e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.167e-07 | 6.199e-05 | 1.308e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.283e-03 | 2.550e-01 | 5.357e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 2.620e-03 | 1.535e-01 | 3.239e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 7.295e-03 | 3.650e-01 | 7.668e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.167e-07 | 6.199e-05 | 1.308e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.283e-03 | 2.550e-01 | 5.357e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 2.620e-03 | 1.535e-01 | 3.239e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 7.295e-03 | 3.650e-01 | 7.668e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.167e-07 | 6.199e-05 | 1.308e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.283e-03 | 2.550e-01 | 5.357e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 2.620e-03 | 1.535e-01 | 3.239e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.295e-03 | 3.650e-01 | 7.668e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.167e-07 | 6.199e-05 | 1.308e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.283e-03 | 2.550e-01 | 5.357e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.077e-03 | 1.216e-01 | 2.567e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.206e-07 | 2.289e-05 | 4.831e-07 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.619e-03 | 1.604e-01 | 3.384e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.167e-07 | 2.289e-05 | 4.831e-07 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.620e-03 | 1.535e-01 | 3.239e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.167e-07 | 6.199e-05 | 1.308e-06 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.620e-03 | 1.535e-01 | 3.239e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.167e-07 | 6.199e-05 | 1.308e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.620e-03 | 1.535e-01 | 3.239e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.167e-07 | 6.199e-05 | 1.308e-06 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.620e-03 | 1.535e-01 | 3.239e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.167e-07 | 6.199e-05 | 1.308e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.620e-03 | 1.535e-01 | 3.239e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.167e-07 | 6.199e-05 | 1.308e-06 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.059e-02 | 8.178e+00 | 1.182e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.048e-02 | 8.178e+00 | 1.169e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 1.504e-05 | 1.470e-01 | 2.124e-04 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.048e-02 | 8.178e+00 | 1.169e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 1.504e-05 | 1.470e-01 | 2.124e-04 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.048e-02 | 8.178e+00 | 1.169e-02 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.497e-03 | 2.918e+00 | 4.217e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.309e-02 | 9.997e+00 | 1.429e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.523e-05 | 1.250e-01 | 1.807e-04 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.048e-02 | 8.178e+00 | 1.169e-02 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.821e-03 | 3.929e+00 | 5.678e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.450e-02 | 1.132e+01 | 1.618e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.501e-05 | 1.841e-01 | 2.660e-04 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.048e-02 | 8.178e+00 | 1.169e-02 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.826e-03 | 3.929e+00 | 5.678e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.450e-02 | 1.136e+01 | 1.624e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.502e-05 | 1.841e-01 | 2.660e-04 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.048e-02 | 8.178e+00 | 1.169e-02 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.826e-03 | 3.929e+00 | 5.678e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.450e-02 | 1.205e+01 | 1.723e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.502e-05 | 1.841e-01 | 2.660e-04 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.048e-02 | 8.178e+00 | 1.169e-02 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.826e-03 | 3.929e+00 | 5.678e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.450e-02 | 1.205e+01 | 1.723e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.502e-05 | 1.841e-01 | 2.660e-04 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.048e-02 | 8.178e+00 | 1.169e-02 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.826e-03 | 3.929e+00 | 5.678e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.450e-02 | 1.205e+01 | 1.723e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.502e-05 | 1.841e-01 | 2.660e-04 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.048e-02 | 8.178e+00 | 1.169e-02 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.826e-03 | 3.929e+00 | 5.678e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.450e-02 | 1.205e+01 | 1.723e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.502e-05 | 1.841e-01 | 2.660e-04 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.048e-02 | 8.178e+00 | 1.169e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 968.6558 | 5848 | 1145.726 |
| original | row_schedule | 1.2047 | 5848 | 1145.726 |
| original | partial_zero | 2.0289 | 5848 | 1145.726 |
| original | csr_prefetch | 4.1583 | 5848 | 1145.726 |
| original | feature_gather_decode | 4.6096 | 5848 | 1145.726 |
| original | fp32_neighbor_reduction | 0.0000 | 5848 | 1145.726 |
| original | partial_bf16_conversion | 0.0000 | 5848 | 1145.726 |
| original | tile_load | 8.4214 | 5848 | 1145.726 |
| original | tile_compute | 18.3613 | 5848 | 1145.726 |
| original | tile_store | 2.2113 | 5848 | 1145.726 |
| original | output_scatter | 2.0542 | 5848 | 1145.726 |
| original | thread_amx_setup | 2.7661 | 5848 | 1145.726 |

