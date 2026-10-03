# GCN-extra first inference experiments

Numerical sum-aggregation experiments with random H/W; no task-accuracy claim.
Uninstrumented medians; original source is unchanged. Speedups are original/new and FP32 MKL/new.
Profiles are sampled summed thread time and do not equal wall time. Physical AMX work is modeled, not a hardware counter.

## reddit

N=232965, E=114615892, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b64_fast_nozero | 61.037 | 57.355 | 59.101 | 61.989 | 2.485x | 1.707x |
| shared_b64_accurate_nozero | 62.699 | 58.474 | 62.372 | 63.516 | 2.419x | 1.661x |
| shared_b32_fast_nozero | 62.730 | 59.596 | 60.176 | 63.242 | 2.418x | 1.661x |
| shared_b32_accurate_nozero | 66.001 | 62.913 | 65.462 | 67.875 | 2.298x | 1.578x |
| shared_bfull_fast_nozero | 66.303 | 62.481 | 65.011 | 66.935 | 2.288x | 1.571x |
| shared_bfull_accurate_nozero | 66.480 | 62.291 | 64.261 | 67.689 | 2.282x | 1.567x |
| shared_b16_fast_nozero | 70.772 | 68.009 | 70.413 | 71.746 | 2.143x | 1.472x |
| shared_b16_accurate_nozero | 74.911 | 73.376 | 74.023 | 75.795 | 2.025x | 1.391x |
| shared_b8_fast_nozero | 82.216 | 76.932 | 80.145 | 83.725 | 1.845x | 1.267x |
| shared_b8_accurate_nozero | 93.667 | 88.816 | 90.289 | 96.252 | 1.619x | 1.112x |
| mkl_fp32 | 104.168 | 102.751 | 103.525 | 104.522 | 1.456x | 1.000x |
| mkl_bf16_inputs | 104.680 | 98.133 | 104.315 | 104.739 | 1.449x | 0.995x |
| shared_b4_fast_nozero | 109.328 | 104.776 | 107.913 | 110.692 | 1.387x | 0.953x |
| replay_b32_fast_nozero | 113.673 | 110.030 | 111.680 | 115.133 | 1.334x | 0.916x |
| replay_bfull_fast_nozero | 117.540 | 113.008 | 114.686 | 120.986 | 1.290x | 0.886x |
| shared_b4_accurate_nozero | 135.439 | 123.015 | 133.697 | 138.783 | 1.120x | 0.769x |
| replay_b8_fast_nozero | 140.124 | 135.813 | 136.951 | 142.119 | 1.082x | 0.743x |
| original_nozero | 147.905 | 142.245 | 143.801 | 149.805 | 1.026x | 0.704x |
| original | 151.682 | 143.752 | 149.635 | 153.990 | 1.000x | 0.687x |
| shared_b2_fast_nozero | 168.098 | 156.900 | 167.832 | 172.790 | 0.902x | 0.620x |
| shared_b2_accurate_nozero | 221.643 | 210.955 | 218.324 | 230.073 | 0.684x | 0.470x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b64_fast_nozero | 124.061 | 122.822 | 123.306 | 125.494 | 2.418x | 1.700x |
| shared_b64_accurate_nozero | 125.659 | 122.097 | 124.537 | 127.441 | 2.387x | 1.678x |
| shared_b32_fast_nozero | 130.186 | 126.644 | 127.687 | 131.611 | 2.304x | 1.620x |
| shared_bfull_accurate_nozero | 131.305 | 126.634 | 129.584 | 133.561 | 2.284x | 1.606x |
| shared_bfull_fast_nozero | 133.657 | 128.609 | 131.574 | 135.528 | 2.244x | 1.578x |
| shared_b32_accurate_nozero | 135.753 | 130.893 | 133.217 | 137.122 | 2.210x | 1.554x |
| shared_b16_fast_nozero | 143.456 | 137.857 | 142.200 | 146.433 | 2.091x | 1.470x |
| shared_b16_accurate_nozero | 152.542 | 149.282 | 151.932 | 153.997 | 1.966x | 1.383x |
| shared_b8_fast_nozero | 168.438 | 163.238 | 165.658 | 169.485 | 1.781x | 1.252x |
| shared_b8_accurate_nozero | 191.626 | 183.617 | 187.474 | 194.705 | 1.565x | 1.101x |
| mkl_bf16_inputs | 208.901 | 207.749 | 208.810 | 209.163 | 1.436x | 1.010x |
| mkl_fp32 | 210.899 | 209.789 | 210.308 | 211.634 | 1.422x | 1.000x |
| shared_b4_fast_nozero | 225.180 | 216.067 | 223.877 | 229.011 | 1.332x | 0.937x |
| replay_b32_fast_nozero | 231.227 | 225.346 | 229.030 | 233.820 | 1.297x | 0.912x |
| replay_bfull_fast_nozero | 245.967 | 236.183 | 241.994 | 249.758 | 1.219x | 0.857x |
| shared_b4_accurate_nozero | 273.481 | 257.124 | 270.310 | 275.371 | 1.097x | 0.771x |
| replay_b8_fast_nozero | 281.716 | 275.676 | 278.450 | 284.951 | 1.065x | 0.749x |
| original_nozero | 286.792 | 277.665 | 283.856 | 289.558 | 1.046x | 0.735x |
| original | 299.947 | 285.555 | 297.351 | 302.383 | 1.000x | 0.703x |
| shared_b2_fast_nozero | 340.539 | 333.681 | 336.214 | 347.346 | 0.881x | 0.619x |
| shared_b2_accurate_nozero | 450.206 | 432.056 | 436.431 | 452.898 | 0.666x | 0.468x |

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
| replay_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.115e-03 | 7.335e+00 | 2.626e-03 | 1 |
| replay_b8_fast_nozero | kernel_full | mkl_fp32 | 7.678e-03 | 1.687e+01 | 6.030e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.219e-03 | 7.950e+00 | 2.847e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 7.790e-03 | 1.755e+01 | 6.271e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.896e-06 | 7.568e-03 | 2.710e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.053e-03 | 1.112e+01 | 3.974e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.231e-03 | 8.240e+00 | 2.951e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 7.805e-03 | 1.690e+01 | 6.040e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.528e-06 | 1.062e-02 | 3.803e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.054e-03 | 1.112e+01 | 3.973e-03 | 1 |
| replay_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.231e-03 | 8.240e+00 | 2.951e-03 | 1 |
| replay_b32_fast_nozero | kernel_full | mkl_fp32 | 7.805e-03 | 1.690e+01 | 6.040e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.257e-03 | 7.935e+00 | 2.841e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 7.831e-03 | 1.905e+01 | 6.808e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 5.430e-06 | 1.208e-02 | 4.327e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.055e-03 | 1.112e+01 | 3.975e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.856e-03 | 1.662e+01 | 5.941e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 8.823e-06 | 2.722e-02 | 9.747e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.058e-03 | 1.113e+01 | 3.978e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.856e-03 | 1.662e+01 | 5.941e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.308e-03 | 6.220e+00 | 2.227e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.585e-06 | 2.417e-02 | 8.655e-06 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.927e-03 | 6.762e+00 | 2.421e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.135e-06 | 1.538e-02 | 5.507e-06 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.115e-03 | 7.335e+00 | 2.626e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 9.923e-07 | 1.123e-02 | 4.021e-06 | 1 |
| replay_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.115e-03 | 7.335e+00 | 2.626e-03 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.219e-03 | 7.950e+00 | 2.847e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.896e-06 | 7.568e-03 | 2.710e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.231e-03 | 8.240e+00 | 2.951e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.528e-06 | 1.062e-02 | 3.803e-06 | 1 |
| replay_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.231e-03 | 8.240e+00 | 2.951e-03 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.257e-03 | 7.935e+00 | 2.841e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 5.430e-06 | 1.208e-02 | 4.327e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 8.823e-06 | 2.722e-02 | 9.747e-06 | 1 |
| replay_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
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
| replay_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.325e-03 | 1.714e+05 | 5.331e-03 | 1 |
| replay_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.448e-02 | 4.654e+05 | 1.434e-02 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.546e-03 | 1.782e+05 | 5.543e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.470e-02 | 4.722e+05 | 1.455e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.039e-06 | 1.800e+02 | 5.598e-06 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.292e-03 | 2.940e+05 | 9.059e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.590e-03 | 1.789e+05 | 5.563e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.474e-02 | 4.728e+05 | 1.457e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 5.302e-06 | 1.600e+02 | 4.976e-06 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.294e-03 | 2.941e+05 | 9.064e-03 | 1 |
| replay_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.590e-03 | 1.789e+05 | 5.563e-03 | 1 |
| replay_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.474e-02 | 4.728e+05 | 1.457e-02 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.621e-03 | 1.809e+05 | 5.624e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.477e-02 | 4.748e+05 | 1.463e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 9.578e-06 | 3.080e+02 | 9.578e-06 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.298e-03 | 2.943e+05 | 9.068e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 1.943e+05 | 6.043e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.486e-02 | 4.803e+05 | 1.480e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.625e-05 | 5.360e+02 | 1.667e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.304e-03 | 2.944e+05 | 9.072e-03 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 1.943e+05 | 6.043e-03 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.486e-02 | 4.803e+05 | 1.480e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 9.8262 | 58 | 170.660 |
| original | row_schedule | 0.0341 | 58 | 170.660 |
| original | partial_zero | 0.0215 | 58 | 170.660 |
| original | csr_prefetch | 11.8740 | 58 | 170.660 |
| original | feature_gather_decode | 4.2140 | 58 | 170.660 |
| original | fp32_neighbor_reduction | 0.0000 | 58 | 170.660 |
| original | partial_bf16_conversion | 0.0000 | 58 | 170.660 |
| original | tile_load | 23.7789 | 58 | 170.660 |
| original | tile_compute | 30.3462 | 58 | 170.660 |
| original | tile_store | 0.0165 | 58 | 170.660 |
| original | output_scatter | 0.0255 | 58 | 170.660 |
| original | thread_amx_setup | 1.8620 | 58 | 170.660 |

## regular-q16

N=262144, E=4194304, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 2.895 | 2.860 | 2.881 | 2.917 | 6.153x | 3.591x |
| shared_b16_fast_nozero | 2.901 | 2.810 | 2.858 | 2.939 | 6.140x | 3.584x |
| shared_b32_fast_nozero | 2.926 | 2.747 | 2.881 | 2.957 | 6.088x | 3.553x |
| shared_b64_fast_nozero | 2.936 | 2.808 | 2.917 | 2.958 | 6.067x | 3.541x |
| shared_bfull_accurate_nozero | 3.016 | 2.988 | 2.999 | 3.060 | 5.906x | 3.447x |
| shared_b32_accurate_nozero | 3.052 | 2.987 | 3.037 | 3.074 | 5.837x | 3.407x |
| shared_b64_accurate_nozero | 3.069 | 3.043 | 3.063 | 3.077 | 5.805x | 3.388x |
| shared_b16_accurate_nozero | 3.091 | 3.017 | 3.076 | 3.103 | 5.764x | 3.364x |
| shared_b8_fast_nozero | 3.384 | 3.343 | 3.371 | 3.399 | 5.264x | 3.072x |
| shared_b8_accurate_nozero | 3.719 | 3.682 | 3.699 | 3.736 | 4.789x | 2.795x |
| replay_bfull_fast_nozero | 3.875 | 3.823 | 3.859 | 3.882 | 4.597x | 2.683x |
| replay_b32_fast_nozero | 3.930 | 3.746 | 3.891 | 3.938 | 4.532x | 2.645x |
| shared_b4_fast_nozero | 4.269 | 4.244 | 4.263 | 4.278 | 4.173x | 2.435x |
| replay_b8_fast_nozero | 4.446 | 4.415 | 4.430 | 4.449 | 4.007x | 2.339x |
| original_nozero | 4.601 | 4.547 | 4.597 | 4.608 | 3.872x | 2.260x |
| shared_b4_accurate_nozero | 5.046 | 5.029 | 5.032 | 5.058 | 3.531x | 2.061x |
| shared_b2_fast_nozero | 6.200 | 6.146 | 6.164 | 6.204 | 2.873x | 1.677x |
| shared_b2_accurate_nozero | 7.800 | 7.787 | 7.796 | 7.823 | 2.284x | 1.333x |
| mkl_bf16_inputs | 9.393 | 9.254 | 9.338 | 9.408 | 1.897x | 1.107x |
| mkl_fp32 | 10.397 | 10.229 | 10.305 | 10.921 | 1.713x | 1.000x |
| original | 17.814 | 17.372 | 17.644 | 17.882 | 1.000x | 0.584x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b16_fast_nozero | 9.265 | 8.930 | 9.134 | 9.434 | 3.813x | 2.182x |
| shared_b8_fast_nozero | 9.749 | 9.473 | 9.629 | 9.826 | 3.624x | 2.074x |
| shared_b32_fast_nozero | 9.765 | 9.436 | 9.575 | 10.085 | 3.618x | 2.070x |
| shared_bfull_fast_nozero | 10.041 | 9.712 | 9.854 | 10.169 | 3.518x | 2.014x |
| shared_b64_fast_nozero | 10.393 | 10.053 | 10.203 | 10.472 | 3.399x | 1.945x |
| shared_b16_accurate_nozero | 10.430 | 9.844 | 10.186 | 10.527 | 3.387x | 1.938x |
| shared_b64_accurate_nozero | 10.537 | 10.189 | 10.396 | 10.566 | 3.353x | 1.919x |
| shared_bfull_accurate_nozero | 10.656 | 10.514 | 10.551 | 10.766 | 3.315x | 1.897x |
| shared_b32_accurate_nozero | 10.669 | 10.191 | 10.510 | 10.713 | 3.311x | 1.895x |
| shared_b8_accurate_nozero | 10.809 | 10.568 | 10.657 | 11.027 | 3.268x | 1.870x |
| shared_b4_fast_nozero | 11.842 | 11.617 | 11.792 | 11.854 | 2.983x | 1.707x |
| replay_b8_fast_nozero | 11.877 | 11.673 | 11.794 | 12.224 | 2.974x | 1.702x |
| replay_b32_fast_nozero | 11.879 | 11.385 | 11.650 | 11.922 | 2.974x | 1.702x |
| replay_bfull_fast_nozero | 11.893 | 11.778 | 11.859 | 11.999 | 2.970x | 1.700x |
| original_nozero | 12.638 | 11.936 | 12.469 | 13.046 | 2.795x | 1.600x |
| shared_b4_accurate_nozero | 13.342 | 13.278 | 13.319 | 13.444 | 2.648x | 1.515x |
| shared_b2_fast_nozero | 15.760 | 15.688 | 15.696 | 15.839 | 2.241x | 1.283x |
| shared_b2_accurate_nozero | 18.815 | 18.616 | 18.644 | 18.873 | 1.878x | 1.075x |
| mkl_fp32 | 20.218 | 19.761 | 20.025 | 20.341 | 1.747x | 1.000x |
| mkl_bf16_inputs | 20.767 | 20.060 | 20.415 | 20.874 | 1.701x | 0.974x |
| original | 35.326 | 33.052 | 34.389 | 35.477 | 1.000x | 0.572x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.310e-03 | 4.422e-01 | 4.846e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 2.315e-07 | 4.959e-05 | 5.435e-07 | 1 |
| original | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 2.315e-07 | 4.959e-05 | 5.435e-07 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.389e-03 | 2.060e-01 | 2.258e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 7.094e-03 | 6.256e-01 | 6.823e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.317e-07 | 6.104e-05 | 6.690e-07 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.992e-03 | 2.668e-01 | 2.925e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.742e-03 | 7.090e-01 | 7.733e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.084e-07 | 4.578e-05 | 5.017e-07 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.285e-03 | 4.421e-01 | 4.823e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.173e-03 | 2.797e-01 | 3.066e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.960e-03 | 7.048e-01 | 7.687e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 6.282e-07 | 1.669e-04 | 1.829e-06 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| replay_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.173e-03 | 2.797e-01 | 3.066e-03 | 1 |
| replay_b8_fast_nozero | kernel_full | mkl_fp32 | 7.960e-03 | 7.048e-01 | 7.687e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 8.069e-03 | 6.918e-01 | 7.546e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 8.069e-03 | 6.918e-01 | 7.546e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| replay_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| replay_b32_fast_nozero | kernel_full | mkl_fp32 | 8.069e-03 | 6.918e-01 | 7.546e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 8.069e-03 | 6.918e-01 | 7.546e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.069e-03 | 6.918e-01 | 7.546e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.069e-03 | 6.918e-01 | 7.546e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.389e-03 | 2.060e-01 | 2.258e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.317e-07 | 6.104e-05 | 6.690e-07 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.992e-03 | 2.668e-01 | 2.925e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.084e-07 | 4.578e-05 | 5.017e-07 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.173e-03 | 2.797e-01 | 3.066e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 6.282e-07 | 1.669e-04 | 1.829e-06 | 1 |
| replay_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.173e-03 | 2.797e-01 | 3.066e-03 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| replay_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| replay_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.058e-02 | 3.081e+01 | 1.085e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 1.291e-05 | 3.492e-01 | 1.230e-04 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 1.291e-05 | 3.492e-01 | 1.230e-04 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.122e-03 | 1.021e+01 | 3.597e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.322e-02 | 4.001e+01 | 1.394e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.330e-05 | 3.650e-01 | 1.285e-04 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.717e-03 | 1.510e+01 | 5.317e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.485e-02 | 4.476e+01 | 1.560e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.198e-05 | 3.002e-01 | 1.057e-04 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.523e-03 | 1.823e+01 | 6.420e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.564e-02 | 4.586e+01 | 1.598e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.550e-05 | 3.582e-01 | 1.261e-04 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| replay_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.523e-03 | 1.823e+01 | 6.420e-03 | 1 |
| replay_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.564e-02 | 4.586e+01 | 1.598e-02 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.915e-03 | 1.857e+01 | 6.539e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.597e-02 | 4.830e+01 | 1.683e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.003e-05 | 4.364e-01 | 1.537e-04 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.915e-03 | 1.857e+01 | 6.539e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.597e-02 | 4.830e+01 | 1.683e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.003e-05 | 4.364e-01 | 1.537e-04 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| replay_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.915e-03 | 1.857e+01 | 6.539e-03 | 1 |
| replay_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.597e-02 | 4.830e+01 | 1.683e-02 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.915e-03 | 1.857e+01 | 6.539e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.597e-02 | 4.830e+01 | 1.683e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.003e-05 | 4.364e-01 | 1.537e-04 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.915e-03 | 1.857e+01 | 6.539e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.597e-02 | 4.830e+01 | 1.683e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.003e-05 | 4.364e-01 | 1.537e-04 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.915e-03 | 1.857e+01 | 6.539e-03 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.597e-02 | 4.830e+01 | 1.683e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 10.7090 | 65 | 15.533 |
| original | row_schedule | 0.0246 | 65 | 15.533 |
| original | partial_zero | 0.0179 | 65 | 15.533 |
| original | csr_prefetch | 0.1905 | 65 | 15.533 |
| original | feature_gather_decode | 0.1447 | 65 | 15.533 |
| original | fp32_neighbor_reduction | 0.0000 | 65 | 15.533 |
| original | partial_bf16_conversion | 0.0000 | 65 | 15.533 |
| original | tile_load | 0.6049 | 65 | 15.533 |
| original | tile_compute | 0.6380 | 65 | 15.533 |
| original | tile_store | 0.0200 | 65 | 15.533 |
| original | output_scatter | 0.0219 | 65 | 15.533 |
| original | thread_amx_setup | 4.1490 | 65 | 15.533 |

## regular-q256

N=262144, E=67108864, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b64_fast_nozero | 37.220 | 37.051 | 37.188 | 37.318 | 1.986x | 2.452x |
| shared_bfull_fast_nozero | 37.641 | 37.133 | 37.494 | 37.873 | 1.964x | 2.424x |
| shared_bfull_accurate_nozero | 37.961 | 37.937 | 37.947 | 37.991 | 1.948x | 2.404x |
| shared_b64_accurate_nozero | 38.262 | 38.000 | 38.069 | 38.369 | 1.932x | 2.385x |
| shared_b32_fast_nozero | 38.982 | 38.779 | 38.956 | 38.999 | 1.897x | 2.341x |
| shared_b32_accurate_nozero | 41.365 | 41.195 | 41.339 | 41.424 | 1.787x | 2.206x |
| shared_b16_fast_nozero | 42.152 | 41.492 | 41.866 | 42.419 | 1.754x | 2.165x |
| shared_b16_accurate_nozero | 44.876 | 44.610 | 44.816 | 44.927 | 1.648x | 2.034x |
| shared_b8_fast_nozero | 48.328 | 47.052 | 47.618 | 48.723 | 1.530x | 1.888x |
| replay_bfull_fast_nozero | 54.026 | 53.925 | 53.966 | 54.092 | 1.369x | 1.689x |
| replay_b32_fast_nozero | 55.324 | 54.910 | 55.179 | 55.522 | 1.336x | 1.650x |
| shared_b8_accurate_nozero | 55.341 | 53.687 | 55.061 | 55.593 | 1.336x | 1.649x |
| original_nozero | 60.800 | 60.662 | 60.766 | 60.904 | 1.216x | 1.501x |
| shared_b4_fast_nozero | 62.660 | 62.540 | 62.579 | 62.709 | 1.180x | 1.456x |
| replay_b8_fast_nozero | 68.630 | 68.233 | 68.474 | 68.695 | 1.077x | 1.330x |
| original | 73.938 | 73.739 | 73.810 | 74.124 | 1.000x | 1.234x |
| shared_b4_accurate_nozero | 75.814 | 75.368 | 75.476 | 76.517 | 0.975x | 1.204x |
| mkl_bf16_inputs | 89.087 | 88.348 | 88.964 | 89.349 | 0.830x | 1.024x |
| mkl_fp32 | 91.261 | 90.948 | 91.141 | 91.440 | 0.810x | 1.000x |
| shared_b2_fast_nozero | 97.329 | 95.868 | 96.515 | 97.668 | 0.760x | 0.938x |
| shared_b2_accurate_nozero | 120.546 | 119.939 | 120.285 | 120.916 | 0.613x | 0.757x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_accurate_nozero | 78.222 | 77.276 | 78.035 | 78.365 | 1.868x | 2.316x |
| shared_b64_accurate_nozero | 78.465 | 77.665 | 78.045 | 78.679 | 1.862x | 2.308x |
| shared_b64_fast_nozero | 78.486 | 77.976 | 78.324 | 78.538 | 1.862x | 2.308x |
| shared_bfull_fast_nozero | 78.589 | 78.224 | 78.418 | 78.771 | 1.859x | 2.305x |
| shared_b32_fast_nozero | 78.708 | 77.883 | 78.564 | 79.609 | 1.856x | 2.301x |
| shared_b32_accurate_nozero | 84.360 | 82.748 | 83.018 | 84.728 | 1.732x | 2.147x |
| shared_b16_fast_nozero | 87.130 | 86.426 | 86.962 | 87.277 | 1.677x | 2.079x |
| shared_b16_accurate_nozero | 90.370 | 90.148 | 90.251 | 90.434 | 1.617x | 2.004x |
| shared_b8_fast_nozero | 99.150 | 97.500 | 98.827 | 99.312 | 1.474x | 1.827x |
| replay_bfull_fast_nozero | 108.588 | 107.955 | 108.474 | 108.725 | 1.346x | 1.668x |
| shared_b8_accurate_nozero | 112.098 | 109.400 | 111.982 | 112.247 | 1.304x | 1.616x |
| replay_b32_fast_nozero | 112.789 | 112.180 | 112.643 | 113.285 | 1.296x | 1.606x |
| original_nozero | 124.509 | 124.430 | 124.475 | 124.550 | 1.174x | 1.455x |
| shared_b4_fast_nozero | 128.405 | 126.565 | 127.838 | 129.144 | 1.138x | 1.411x |
| replay_b8_fast_nozero | 139.108 | 138.913 | 139.057 | 139.279 | 1.050x | 1.302x |
| original | 146.122 | 145.753 | 146.007 | 146.241 | 1.000x | 1.240x |
| shared_b4_accurate_nozero | 155.069 | 154.688 | 154.963 | 155.307 | 0.942x | 1.168x |
| mkl_bf16_inputs | 178.907 | 178.641 | 178.750 | 179.030 | 0.817x | 1.012x |
| mkl_fp32 | 181.133 | 180.829 | 181.052 | 181.332 | 0.807x | 1.000x |
| shared_b2_fast_nozero | 190.157 | 187.839 | 189.757 | 191.651 | 0.768x | 0.953x |
| shared_b2_accurate_nozero | 240.165 | 239.743 | 239.956 | 240.723 | 0.608x | 0.754x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.274e-03 | 1.855e+00 | 5.164e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.249e-03 | 1.855e+00 | 5.138e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 6.099e-07 | 5.798e-04 | 1.614e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.249e-03 | 1.855e+00 | 5.137e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 6.099e-07 | 5.798e-04 | 1.614e-06 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.249e-03 | 1.855e+00 | 5.137e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.376e-03 | 8.840e-01 | 2.461e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 7.051e-03 | 2.488e+00 | 6.890e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 6.126e-07 | 5.188e-04 | 1.444e-06 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.249e-03 | 1.855e+00 | 5.138e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.982e-03 | 1.084e+00 | 3.018e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.699e-03 | 2.759e+00 | 7.640e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.575e-07 | 3.510e-04 | 9.768e-07 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.249e-03 | 1.856e+00 | 5.138e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.164e-03 | 1.026e+00 | 2.856e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.918e-03 | 2.863e+00 | 7.928e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 6.903e-07 | 3.662e-04 | 1.019e-06 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.249e-03 | 1.855e+00 | 5.138e-03 | 1 |
| replay_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.164e-03 | 1.026e+00 | 2.856e-03 | 1 |
| replay_b8_fast_nozero | kernel_full | mkl_fp32 | 7.918e-03 | 2.863e+00 | 7.928e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.266e-03 | 1.211e+00 | 3.370e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 8.028e-03 | 3.066e+00 | 8.491e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.922e-06 | 7.305e-04 | 2.033e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.249e-03 | 1.855e+00 | 5.138e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.279e-03 | 1.110e+00 | 3.089e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 8.045e-03 | 2.919e+00 | 8.083e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.707e-06 | 1.358e-03 | 3.780e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.250e-03 | 1.856e+00 | 5.139e-03 | 1 |
| replay_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.279e-03 | 1.110e+00 | 3.089e-03 | 1 |
| replay_b32_fast_nozero | kernel_full | mkl_fp32 | 8.045e-03 | 2.919e+00 | 8.083e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.303e-03 | 1.159e+00 | 3.225e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 8.069e-03 | 2.897e+00 | 8.023e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 5.718e-06 | 2.106e-03 | 5.861e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.252e-03 | 1.856e+00 | 5.140e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.314e-03 | 1.190e+00 | 3.312e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.076e-03 | 2.869e+00 | 7.946e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 7.976e-06 | 3.235e-03 | 9.004e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.254e-03 | 1.858e+00 | 5.145e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.314e-03 | 1.190e+00 | 3.312e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.076e-03 | 2.869e+00 | 7.946e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.376e-03 | 8.840e-01 | 2.461e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 6.126e-07 | 5.188e-04 | 1.444e-06 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.982e-03 | 1.084e+00 | 3.018e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.575e-07 | 3.510e-04 | 9.768e-07 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.164e-03 | 1.026e+00 | 2.856e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 6.903e-07 | 3.662e-04 | 1.019e-06 | 1 |
| replay_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.164e-03 | 1.026e+00 | 2.856e-03 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.266e-03 | 1.211e+00 | 3.370e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.922e-06 | 7.305e-04 | 2.033e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.279e-03 | 1.110e+00 | 3.089e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.707e-06 | 1.358e-03 | 3.780e-06 | 1 |
| replay_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.279e-03 | 1.110e+00 | 3.089e-03 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.303e-03 | 1.159e+00 | 3.225e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 5.718e-06 | 2.106e-03 | 5.861e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.314e-03 | 1.190e+00 | 3.312e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 7.976e-06 | 3.235e-03 | 9.004e-06 | 1 |
| replay_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.314e-03 | 1.190e+00 | 3.312e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.047e-02 | 1.487e+03 | 1.154e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.036e-02 | 1.487e+03 | 1.141e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 5.376e-06 | 2.566e+00 | 1.991e-05 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.036e-02 | 1.487e+03 | 1.141e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 5.376e-06 | 2.566e+00 | 1.991e-05 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.036e-02 | 1.487e+03 | 1.141e-02 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 2.797e-03 | 3.954e+02 | 3.068e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.307e-02 | 1.859e+03 | 1.426e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 5.555e-06 | 2.638e+00 | 2.046e-05 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.036e-02 | 1.487e+03 | 1.141e-02 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.442e-03 | 6.003e+02 | 4.658e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.469e-02 | 2.064e+03 | 1.583e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 4.908e-06 | 2.941e+00 | 2.282e-05 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.036e-02 | 1.487e+03 | 1.141e-02 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.235e-03 | 7.062e+02 | 5.479e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.547e-02 | 2.172e+03 | 1.666e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 5.402e-06 | 3.238e+00 | 2.512e-05 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.036e-02 | 1.487e+03 | 1.141e-02 | 1 |
| replay_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.235e-03 | 7.062e+02 | 5.479e-03 | 1 |
| replay_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.547e-02 | 2.172e+03 | 1.666e-02 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.546e-03 | 7.546e+02 | 5.854e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.578e-02 | 2.188e+03 | 1.678e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 8.887e-06 | 3.309e+00 | 2.567e-05 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.036e-02 | 1.487e+03 | 1.141e-02 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.607e-03 | 8.013e+02 | 6.217e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.583e-02 | 2.225e+03 | 1.707e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.284e-05 | 4.354e+00 | 3.378e-05 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.036e-02 | 1.487e+03 | 1.140e-02 | 1 |
| replay_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.607e-03 | 8.013e+02 | 6.217e-03 | 1 |
| replay_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.583e-02 | 2.225e+03 | 1.707e-02 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.632e-03 | 8.031e+02 | 6.231e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.583e-02 | 2.273e+03 | 1.743e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.760e-05 | 5.348e+00 | 4.149e-05 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.037e-02 | 1.488e+03 | 1.141e-02 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.749e-03 | 9.510e+02 | 7.379e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.584e-02 | 2.304e+03 | 1.767e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.362e-05 | 6.715e+00 | 5.210e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.037e-02 | 1.488e+03 | 1.142e-02 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.749e-03 | 9.510e+02 | 7.379e-03 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.584e-02 | 2.304e+03 | 1.767e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 11.3308 | 65 | 73.364 |
| original | row_schedule | 0.0162 | 65 | 73.364 |
| original | partial_zero | 0.0203 | 65 | 73.364 |
| original | csr_prefetch | 3.6874 | 65 | 73.364 |
| original | feature_gather_decode | 1.7292 | 65 | 73.364 |
| original | fp32_neighbor_reduction | 0.0000 | 65 | 73.364 |
| original | partial_bf16_conversion | 0.0000 | 65 | 73.364 |
| original | tile_load | 8.4934 | 65 | 73.364 |
| original | tile_compute | 9.1560 | 65 | 73.364 |
| original | tile_store | 0.0207 | 65 | 73.364 |
| original | output_scatter | 0.0308 | 65 | 73.364 |
| original | thread_amx_setup | 2.2511 | 65 | 73.364 |

## soc-Pokec

N=1632803, E=30622564, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 37.137 | 36.491 | 37.000 | 37.492 | 3.151x | 1.723x |
| shared_b64_fast_nozero | 37.768 | 36.631 | 37.018 | 38.023 | 3.099x | 1.694x |
| shared_b32_fast_nozero | 37.982 | 37.063 | 37.426 | 38.700 | 3.081x | 1.684x |
| shared_bfull_accurate_nozero | 38.963 | 38.193 | 38.614 | 39.156 | 3.004x | 1.642x |
| shared_b64_accurate_nozero | 39.048 | 38.406 | 38.875 | 39.750 | 2.997x | 1.638x |
| shared_b32_accurate_nozero | 41.409 | 40.475 | 40.615 | 41.870 | 2.826x | 1.545x |
| original_nozero | 42.115 | 39.011 | 40.914 | 42.480 | 2.779x | 1.519x |
| shared_b16_fast_nozero | 42.349 | 40.681 | 41.303 | 42.859 | 2.763x | 1.511x |
| shared_b16_accurate_nozero | 43.496 | 42.282 | 43.039 | 44.196 | 2.691x | 1.471x |
| shared_b8_fast_nozero | 44.439 | 43.718 | 44.110 | 45.396 | 2.633x | 1.440x |
| replay_bfull_fast_nozero | 47.123 | 46.539 | 47.076 | 47.268 | 2.483x | 1.358x |
| shared_b8_accurate_nozero | 49.692 | 48.464 | 48.842 | 50.055 | 2.355x | 1.287x |
| replay_b32_fast_nozero | 49.878 | 48.137 | 48.741 | 50.471 | 2.346x | 1.283x |
| shared_b4_fast_nozero | 53.858 | 51.679 | 52.808 | 54.401 | 2.173x | 1.188x |
| replay_b8_fast_nozero | 56.284 | 54.961 | 55.851 | 57.230 | 2.079x | 1.137x |
| shared_b4_accurate_nozero | 61.039 | 57.906 | 60.059 | 61.172 | 1.917x | 1.048x |
| mkl_fp32 | 63.972 | 63.716 | 63.939 | 64.009 | 1.829x | 1.000x |
| mkl_bf16_inputs | 65.150 | 63.637 | 65.049 | 65.273 | 1.796x | 0.982x |
| shared_b2_fast_nozero | 69.121 | 65.293 | 67.638 | 70.682 | 1.693x | 0.926x |
| shared_b2_accurate_nozero | 84.570 | 82.463 | 83.053 | 87.222 | 1.384x | 0.756x |
| original | 117.029 | 113.354 | 115.532 | 118.287 | 1.000x | 0.547x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b64_fast_nozero | 81.063 | 80.161 | 80.538 | 81.405 | 2.867x | 1.617x |
| shared_bfull_fast_nozero | 81.784 | 81.166 | 81.477 | 82.342 | 2.842x | 1.603x |
| shared_b32_fast_nozero | 83.660 | 82.193 | 83.223 | 84.986 | 2.778x | 1.567x |
| shared_bfull_accurate_nozero | 85.441 | 84.350 | 85.152 | 86.218 | 2.720x | 1.534x |
| shared_b64_accurate_nozero | 86.650 | 85.019 | 86.259 | 87.441 | 2.682x | 1.513x |
| original_nozero | 89.237 | 86.831 | 87.770 | 92.093 | 2.604x | 1.469x |
| shared_b32_accurate_nozero | 90.297 | 89.171 | 89.812 | 90.766 | 2.574x | 1.452x |
| shared_b16_fast_nozero | 90.915 | 89.218 | 90.768 | 91.702 | 2.556x | 1.442x |
| shared_b16_accurate_nozero | 95.635 | 93.279 | 94.365 | 96.470 | 2.430x | 1.371x |
| shared_b8_fast_nozero | 99.625 | 95.458 | 98.789 | 100.374 | 2.333x | 1.316x |
| replay_bfull_fast_nozero | 102.901 | 100.832 | 101.752 | 103.940 | 2.259x | 1.274x |
| shared_b8_accurate_nozero | 106.999 | 105.548 | 106.323 | 107.283 | 2.172x | 1.225x |
| replay_b32_fast_nozero | 107.745 | 105.829 | 107.115 | 108.238 | 2.157x | 1.217x |
| shared_b4_fast_nozero | 113.389 | 111.958 | 113.237 | 114.815 | 2.050x | 1.156x |
| replay_b8_fast_nozero | 121.018 | 117.436 | 119.711 | 122.550 | 1.920x | 1.083x |
| mkl_fp32 | 131.091 | 130.497 | 130.966 | 131.352 | 1.773x | 1.000x |
| shared_b4_accurate_nozero | 131.186 | 125.743 | 128.120 | 133.436 | 1.772x | 0.999x |
| mkl_bf16_inputs | 139.783 | 136.623 | 139.614 | 140.162 | 1.663x | 0.938x |
| shared_b2_fast_nozero | 144.711 | 140.947 | 142.300 | 146.885 | 1.606x | 0.906x |
| shared_b2_accurate_nozero | 177.747 | 173.847 | 175.994 | 178.379 | 1.308x | 0.738x |
| original | 232.414 | 225.929 | 232.122 | 233.592 | 1.000x | 0.564x |

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
| replay_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.130e-03 | 3.773e+00 | 2.695e-03 | 1 |
| replay_b8_fast_nozero | kernel_full | mkl_fp32 | 7.914e-03 | 9.197e+00 | 6.543e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.229e-03 | 5.043e+00 | 3.602e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 8.022e-03 | 1.063e+01 | 7.561e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.735e-06 | 3.113e-03 | 2.223e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.293e-03 | 5.964e+00 | 4.243e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.247e-03 | 4.145e+00 | 2.961e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 8.046e-03 | 9.731e+00 | 6.923e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.113e-06 | 5.493e-03 | 3.924e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.294e-03 | 5.967e+00 | 4.245e-03 | 1 |
| replay_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.247e-03 | 4.145e+00 | 2.961e-03 | 1 |
| replay_b32_fast_nozero | kernel_full | mkl_fp32 | 8.046e-03 | 9.731e+00 | 6.923e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.264e-03 | 4.400e+00 | 3.143e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 8.063e-03 | 9.986e+00 | 7.104e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.327e-06 | 6.104e-03 | 4.360e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.294e-03 | 5.966e+00 | 4.245e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.064e-03 | 9.351e+00 | 6.653e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.976e-06 | 1.331e-02 | 9.504e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.295e-03 | 5.975e+00 | 4.251e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.064e-03 | 9.351e+00 | 6.653e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.355e-03 | 2.712e+00 | 1.937e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.740e-07 | 6.104e-03 | 4.360e-06 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.950e-03 | 4.213e+00 | 3.009e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.019e-07 | 5.432e-03 | 3.880e-06 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.130e-03 | 3.773e+00 | 2.695e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 6.111e-07 | 2.686e-03 | 1.918e-06 | 1 |
| replay_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.130e-03 | 3.773e+00 | 2.695e-03 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.229e-03 | 5.043e+00 | 3.602e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.735e-06 | 3.113e-03 | 2.223e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.247e-03 | 4.145e+00 | 2.961e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.113e-06 | 5.493e-03 | 3.924e-06 | 1 |
| replay_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.247e-03 | 4.145e+00 | 2.961e-03 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.264e-03 | 4.400e+00 | 3.143e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.327e-06 | 6.104e-03 | 4.360e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.976e-06 | 1.331e-02 | 9.504e-06 | 1 |
| replay_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
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
| replay_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.266e-03 | 8.265e+03 | 5.159e-03 | 1 |
| replay_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.516e-02 | 2.419e+04 | 1.495e-02 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.562e-03 | 8.758e+03 | 5.466e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.545e-02 | 2.469e+04 | 1.526e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.518e-05 | 4.125e+00 | 2.575e-06 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.005e-02 | 1.593e+04 | 9.845e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.669e-03 | 8.903e+03 | 5.557e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.553e-02 | 2.483e+04 | 1.535e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.173e-05 | 6.250e+00 | 3.901e-06 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.005e-02 | 1.593e+04 | 9.848e-03 | 1 |
| replay_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.669e-03 | 8.903e+03 | 5.557e-03 | 1 |
| replay_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.553e-02 | 2.483e+04 | 1.535e-02 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.740e-03 | 8.929e+03 | 5.574e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.557e-02 | 2.486e+04 | 1.536e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.751e-05 | 1.300e+01 | 8.114e-06 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.006e-02 | 1.594e+04 | 9.852e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 9.892e+03 | 6.175e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.561e-02 | 2.582e+04 | 1.596e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.211e-05 | 1.731e+01 | 1.081e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.006e-02 | 1.594e+04 | 9.854e-03 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 9.892e+03 | 6.175e-03 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.561e-02 | 2.582e+04 | 1.596e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 67.9290 | 400 | 127.161 |
| original | row_schedule | 0.1318 | 400 | 127.161 |
| original | partial_zero | 0.1254 | 400 | 127.161 |
| original | csr_prefetch | 2.7418 | 400 | 127.161 |
| original | feature_gather_decode | 1.5872 | 400 | 127.161 |
| original | fp32_neighbor_reduction | 0.0000 | 400 | 127.161 |
| original | partial_bf16_conversion | 0.0000 | 400 | 127.161 |
| original | tile_load | 8.4786 | 400 | 127.161 |
| original | tile_compute | 10.2224 | 400 | 127.161 |
| original | tile_store | 0.1292 | 400 | 127.161 |
| original | output_scatter | 0.1607 | 400 | 127.161 |
| original | thread_amx_setup | 4.0004 | 400 | 127.161 |

