# GCN-extra first inference experiments

Numerical sum-aggregation experiments with random H/W; no task-accuracy claim.
Uninstrumented medians; original source is unchanged. Speedups are original/new and FP32 MKL/new.
Profiles are sampled summed thread time and do not equal wall time. Physical AMX work is modeled, not a hardware counter.

## reddit

N=232965, E=114615892, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b64_accurate | 70.053 | 67.248 | 69.744 | 70.764 | 2.203x | 1.482x |
| shared_b64_fast | 70.356 | 69.849 | 69.873 | 71.802 | 2.193x | 1.476x |
| shared_b32_fast | 70.675 | 68.163 | 68.920 | 71.853 | 2.184x | 1.469x |
| shared_bfull_accurate | 71.114 | 69.550 | 70.607 | 72.961 | 2.170x | 1.460x |
| shared_bfull_fast | 72.450 | 70.447 | 72.104 | 74.358 | 2.130x | 1.433x |
| shared_b32_accurate | 74.141 | 73.331 | 73.649 | 74.367 | 2.082x | 1.401x |
| shared_b16_fast | 80.360 | 78.884 | 79.594 | 80.982 | 1.920x | 1.292x |
| shared_b16_accurate | 82.304 | 79.882 | 81.982 | 84.179 | 1.875x | 1.262x |
| shared_b8_fast | 88.633 | 83.374 | 88.619 | 90.644 | 1.741x | 1.172x |
| shared_b8_accurate | 102.284 | 99.004 | 100.347 | 102.922 | 1.509x | 1.015x |
| mkl_fp32 | 103.838 | 102.679 | 103.384 | 104.140 | 1.486x | 1.000x |
| mkl_bf16_inputs | 104.257 | 101.275 | 103.680 | 104.448 | 1.480x | 0.996x |
| shared_b4_fast | 119.732 | 116.728 | 117.319 | 123.172 | 1.289x | 0.867x |
| replay_b32_fast | 122.438 | 116.394 | 117.396 | 123.415 | 1.260x | 0.848x |
| replay_bfull_fast | 126.755 | 125.116 | 125.207 | 129.567 | 1.218x | 0.819x |
| shared_b4_accurate | 142.088 | 141.079 | 141.479 | 146.168 | 1.086x | 0.731x |
| replay_b8_fast | 144.415 | 138.531 | 140.957 | 146.066 | 1.069x | 0.719x |
| original | 154.326 | 146.985 | 152.833 | 155.212 | 1.000x | 0.673x |
| shared_b2_fast | 182.706 | 177.857 | 179.918 | 185.364 | 0.845x | 0.568x |
| shared_b2_accurate | 228.084 | 223.597 | 226.845 | 232.765 | 0.677x | 0.455x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b32_fast | 148.124 | 145.479 | 145.961 | 148.496 | 1.977x | 1.415x |
| shared_bfull_accurate | 148.306 | 144.716 | 146.554 | 149.847 | 1.975x | 1.413x |
| shared_bfull_fast | 150.660 | 144.134 | 147.451 | 153.689 | 1.944x | 1.391x |
| shared_b8_fast | 182.708 | 180.609 | 181.249 | 183.752 | 1.603x | 1.147x |
| mkl_bf16_inputs | 207.545 | 204.586 | 207.472 | 208.012 | 1.411x | 1.010x |
| mkl_fp32 | 209.574 | 208.804 | 209.388 | 209.749 | 1.397x | 1.000x |
| replay_bfull_fast | 258.693 | 250.762 | 253.806 | 259.807 | 1.132x | 0.810x |
| original | 292.841 | 287.690 | 291.262 | 303.991 | 1.000x | 0.716x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.076e-03 | 1.111e+01 | 3.980e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.053e-03 | 1.111e+01 | 3.972e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 1.602e-06 | 2.124e-02 | 7.606e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.053e-03 | 1.111e+01 | 3.972e-03 | 1 |
| shared_b2_fast | kernel_full | mkl_bf16_inputs | 2.308e-03 | 6.220e+00 | 2.227e-03 | 1 |
| shared_b2_fast | kernel_full | mkl_fp32 | 6.811e-03 | 1.458e+01 | 5.211e-03 | 1 |
| shared_b2_accurate | kernel_full | mkl_bf16_inputs | 1.585e-06 | 2.417e-02 | 8.655e-06 | 1 |
| shared_b2_accurate | kernel_full | mkl_fp32 | 5.053e-03 | 1.111e+01 | 3.970e-03 | 1 |
| shared_b4_fast | kernel_full | mkl_bf16_inputs | 2.927e-03 | 6.762e+00 | 2.421e-03 | 1 |
| shared_b4_fast | kernel_full | mkl_fp32 | 7.460e-03 | 1.606e+01 | 5.741e-03 | 1 |
| shared_b4_accurate | kernel_full | mkl_bf16_inputs | 1.135e-06 | 1.538e-02 | 5.507e-06 | 1 |
| shared_b4_accurate | kernel_full | mkl_fp32 | 5.053e-03 | 1.111e+01 | 3.971e-03 | 1 |
| shared_b8_fast | kernel_full | mkl_bf16_inputs | 3.115e-03 | 7.335e+00 | 2.626e-03 | 1 |
| shared_b8_fast | kernel_full | mkl_fp32 | 7.678e-03 | 1.687e+01 | 6.030e-03 | 1 |
| shared_b8_accurate | kernel_full | mkl_bf16_inputs | 9.923e-07 | 1.123e-02 | 4.021e-06 | 1 |
| shared_b8_accurate | kernel_full | mkl_fp32 | 5.053e-03 | 1.111e+01 | 3.972e-03 | 1 |
| replay_b8_fast | kernel_full | mkl_bf16_inputs | 3.115e-03 | 7.335e+00 | 2.626e-03 | 1 |
| replay_b8_fast | kernel_full | mkl_fp32 | 7.678e-03 | 1.687e+01 | 6.030e-03 | 1 |
| shared_b16_fast | kernel_full | mkl_bf16_inputs | 3.219e-03 | 7.950e+00 | 2.847e-03 | 1 |
| shared_b16_fast | kernel_full | mkl_fp32 | 7.790e-03 | 1.755e+01 | 6.271e-03 | 1 |
| shared_b16_accurate | kernel_full | mkl_bf16_inputs | 1.896e-06 | 7.568e-03 | 2.710e-06 | 1 |
| shared_b16_accurate | kernel_full | mkl_fp32 | 5.053e-03 | 1.112e+01 | 3.974e-03 | 1 |
| shared_b32_fast | kernel_full | mkl_bf16_inputs | 3.231e-03 | 8.240e+00 | 2.951e-03 | 1 |
| shared_b32_fast | kernel_full | mkl_fp32 | 7.805e-03 | 1.690e+01 | 6.040e-03 | 1 |
| shared_b32_accurate | kernel_full | mkl_bf16_inputs | 3.528e-06 | 1.062e-02 | 3.803e-06 | 1 |
| shared_b32_accurate | kernel_full | mkl_fp32 | 5.054e-03 | 1.112e+01 | 3.973e-03 | 1 |
| replay_b32_fast | kernel_full | mkl_bf16_inputs | 3.231e-03 | 8.240e+00 | 2.951e-03 | 1 |
| replay_b32_fast | kernel_full | mkl_fp32 | 7.805e-03 | 1.690e+01 | 6.040e-03 | 1 |
| shared_b64_fast | kernel_full | mkl_bf16_inputs | 3.257e-03 | 7.935e+00 | 2.841e-03 | 1 |
| shared_b64_fast | kernel_full | mkl_fp32 | 7.831e-03 | 1.905e+01 | 6.808e-03 | 1 |
| shared_b64_accurate | kernel_full | mkl_bf16_inputs | 5.430e-06 | 1.208e-02 | 4.327e-06 | 1 |
| shared_b64_accurate | kernel_full | mkl_fp32 | 5.055e-03 | 1.112e+01 | 3.975e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_fp32 | 7.856e-03 | 1.662e+01 | 5.941e-03 | 1 |
| shared_bfull_accurate | kernel_full | mkl_bf16_inputs | 8.823e-06 | 2.722e-02 | 9.747e-06 | 1 |
| shared_bfull_accurate | kernel_full | mkl_fp32 | 5.058e-03 | 1.113e+01 | 3.978e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_fp32 | 7.856e-03 | 1.662e+01 | 5.941e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast | instrumented_kernel | mkl_bf16_inputs | 2.308e-03 | 6.220e+00 | 2.227e-03 | 1 |
| shared_b2_accurate | instrumented_kernel | mkl_bf16_inputs | 1.585e-06 | 2.417e-02 | 8.655e-06 | 1 |
| shared_b4_fast | instrumented_kernel | mkl_bf16_inputs | 2.927e-03 | 6.762e+00 | 2.421e-03 | 1 |
| shared_b4_accurate | instrumented_kernel | mkl_bf16_inputs | 1.135e-06 | 1.538e-02 | 5.507e-06 | 1 |
| shared_b8_fast | instrumented_kernel | mkl_bf16_inputs | 3.115e-03 | 7.335e+00 | 2.626e-03 | 1 |
| shared_b8_accurate | instrumented_kernel | mkl_bf16_inputs | 9.923e-07 | 1.123e-02 | 4.021e-06 | 1 |
| replay_b8_fast | instrumented_kernel | mkl_bf16_inputs | 3.115e-03 | 7.335e+00 | 2.626e-03 | 1 |
| shared_b16_fast | instrumented_kernel | mkl_bf16_inputs | 3.219e-03 | 7.950e+00 | 2.847e-03 | 1 |
| shared_b16_accurate | instrumented_kernel | mkl_bf16_inputs | 1.896e-06 | 7.568e-03 | 2.710e-06 | 1 |
| shared_b32_fast | instrumented_kernel | mkl_bf16_inputs | 3.231e-03 | 8.240e+00 | 2.951e-03 | 1 |
| shared_b32_accurate | instrumented_kernel | mkl_bf16_inputs | 3.528e-06 | 1.062e-02 | 3.803e-06 | 1 |
| replay_b32_fast | instrumented_kernel | mkl_bf16_inputs | 3.231e-03 | 8.240e+00 | 2.951e-03 | 1 |
| shared_b64_fast | instrumented_kernel | mkl_bf16_inputs | 3.257e-03 | 7.935e+00 | 2.841e-03 | 1 |
| shared_b64_accurate | instrumented_kernel | mkl_bf16_inputs | 5.430e-06 | 1.208e-02 | 4.327e-06 | 1 |
| shared_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
| shared_bfull_accurate | instrumented_kernel | mkl_bf16_inputs | 8.823e-06 | 2.722e-02 | 9.747e-06 | 1 |
| replay_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 9.375e-03 | 2.940e+05 | 9.142e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 9.290e-03 | 2.940e+05 | 9.059e-03 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 3.425e-06 | 2.860e+02 | 8.894e-06 | 1 |
| original | two_layer_e2e | mkl_fp32 | 9.290e-03 | 2.940e+05 | 9.060e-03 | 1 |
| shared_b8_fast | two_layer_e2e | mkl_bf16_inputs | 5.325e-03 | 1.714e+05 | 5.331e-03 | 1 |
| shared_b8_fast | two_layer_e2e | mkl_fp32 | 1.448e-02 | 4.654e+05 | 1.434e-02 | 1 |
| shared_b32_fast | two_layer_e2e | mkl_bf16_inputs | 5.590e-03 | 1.789e+05 | 5.563e-03 | 1 |
| shared_b32_fast | two_layer_e2e | mkl_fp32 | 1.474e-02 | 4.728e+05 | 1.457e-02 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 1.943e+05 | 6.043e-03 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_fp32 | 1.486e-02 | 4.803e+05 | 1.480e-02 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_bf16_inputs | 1.625e-05 | 5.360e+02 | 1.667e-05 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_fp32 | 9.304e-03 | 2.944e+05 | 9.072e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 1.943e+05 | 6.043e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_fp32 | 1.486e-02 | 4.803e+05 | 1.480e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 9.2661 | 58 | 165.738 |
| original | row_schedule | 0.0167 | 58 | 165.738 |
| original | partial_zero | 0.0343 | 58 | 165.738 |
| original | csr_prefetch | 11.2655 | 58 | 165.738 |
| original | feature_gather_decode | 4.1735 | 58 | 165.738 |
| original | fp32_neighbor_reduction | 0.0000 | 58 | 165.738 |
| original | partial_bf16_conversion | 0.0000 | 58 | 165.738 |
| original | tile_load | 24.1330 | 58 | 165.738 |
| original | tile_compute | 29.3002 | 58 | 165.738 |
| original | tile_store | 0.0310 | 58 | 165.738 |
| original | output_scatter | 0.0255 | 58 | 165.738 |
| original | thread_amx_setup | 0.0420 | 58 | 165.738 |
| shared_b8_fast | output_zero | 10.0510 | 58 | 94.407 |
| shared_b8_fast | row_schedule | 0.0238 | 58 | 94.407 |
| shared_b8_fast | partial_zero | 0.9863 | 58 | 94.407 |
| shared_b8_fast | csr_prefetch | 15.5301 | 58 | 94.407 |
| shared_b8_fast | feature_gather_decode | 19.3381 | 58 | 94.407 |
| shared_b8_fast | fp32_neighbor_reduction | 16.6979 | 58 | 94.407 |
| shared_b8_fast | partial_bf16_conversion | 0.9112 | 58 | 94.407 |
| shared_b8_fast | tile_load | 4.5366 | 58 | 94.407 |
| shared_b8_fast | tile_compute | 7.9670 | 58 | 94.407 |
| shared_b8_fast | tile_store | 0.6461 | 58 | 94.407 |
| shared_b8_fast | output_scatter | 0.0122 | 58 | 94.407 |
| shared_b8_fast | thread_amx_setup | 0.0336 | 58 | 94.407 |
| shared_bfull_fast | output_zero | 9.6550 | 58 | 74.058 |
| shared_bfull_fast | row_schedule | 0.0207 | 58 | 74.058 |
| shared_bfull_fast | partial_zero | 0.0279 | 58 | 74.058 |
| shared_bfull_fast | csr_prefetch | 13.8922 | 58 | 74.058 |
| shared_bfull_fast | feature_gather_decode | 15.9955 | 58 | 74.058 |
| shared_bfull_fast | fp32_neighbor_reduction | 12.9797 | 58 | 74.058 |
| shared_bfull_fast | partial_bf16_conversion | 0.0262 | 58 | 74.058 |
| shared_bfull_fast | tile_load | 0.1533 | 58 | 74.058 |
| shared_bfull_fast | tile_compute | 0.2837 | 58 | 74.058 |
| shared_bfull_fast | tile_store | 0.0107 | 58 | 74.058 |
| shared_bfull_fast | output_scatter | 0.0174 | 58 | 74.058 |
| shared_bfull_fast | thread_amx_setup | 0.0372 | 58 | 74.058 |
| shared_bfull_accurate | output_zero | 9.7771 | 58 | 72.075 |
| shared_bfull_accurate | row_schedule | 0.0219 | 58 | 72.075 |
| shared_bfull_accurate | partial_zero | 0.0298 | 58 | 72.075 |
| shared_bfull_accurate | csr_prefetch | 13.9897 | 58 | 72.075 |
| shared_bfull_accurate | feature_gather_decode | 16.0909 | 58 | 72.075 |
| shared_bfull_accurate | fp32_neighbor_reduction | 13.0398 | 58 | 72.075 |
| shared_bfull_accurate | partial_bf16_conversion | 0.0286 | 58 | 72.075 |
| shared_bfull_accurate | tile_load | 0.1984 | 58 | 72.075 |
| shared_bfull_accurate | tile_compute | 0.4082 | 58 | 72.075 |
| shared_bfull_accurate | tile_store | 0.0126 | 58 | 72.075 |
| shared_bfull_accurate | output_scatter | 0.0148 | 58 | 72.075 |
| shared_bfull_accurate | thread_amx_setup | 0.0432 | 58 | 72.075 |
| replay_bfull_fast | output_zero | 10.0820 | 58 | 135.988 |
| replay_bfull_fast | row_schedule | 0.0246 | 58 | 135.988 |
| replay_bfull_fast | partial_zero | 0.0243 | 58 | 135.988 |
| replay_bfull_fast | csr_prefetch | 27.8172 | 58 | 135.988 |
| replay_bfull_fast | feature_gather_decode | 32.3632 | 58 | 135.988 |
| replay_bfull_fast | fp32_neighbor_reduction | 26.3097 | 58 | 135.988 |
| replay_bfull_fast | partial_bf16_conversion | 0.0601 | 58 | 135.988 |
| replay_bfull_fast | tile_load | 0.1171 | 58 | 135.988 |
| replay_bfull_fast | tile_compute | 0.2337 | 58 | 135.988 |
| replay_bfull_fast | tile_store | 0.0408 | 58 | 135.988 |
| replay_bfull_fast | output_scatter | 0.0391 | 58 | 135.988 |
| replay_bfull_fast | thread_amx_setup | 0.0367 | 58 | 135.988 |

## regular-q16

N=262144, E=4194304, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_bf16_inputs | 9.206 | 9.165 | 9.173 | 9.216 | 1.907x | 1.102x |
| mkl_fp32 | 10.149 | 10.061 | 10.143 | 10.933 | 1.730x | 1.000x |
| shared_b64_fast | 13.643 | 13.620 | 13.629 | 13.664 | 1.287x | 0.744x |
| shared_b16_fast | 13.678 | 13.626 | 13.667 | 13.689 | 1.284x | 0.742x |
| shared_bfull_fast | 13.823 | 13.811 | 13.820 | 13.838 | 1.270x | 0.734x |
| shared_b32_fast | 13.828 | 13.812 | 13.813 | 13.848 | 1.270x | 0.734x |
| shared_b16_accurate | 14.102 | 14.088 | 14.091 | 14.106 | 1.245x | 0.720x |
| shared_b32_accurate | 14.106 | 14.063 | 14.074 | 14.106 | 1.245x | 0.719x |
| shared_b64_accurate | 14.109 | 14.071 | 14.074 | 14.124 | 1.245x | 0.719x |
| shared_bfull_accurate | 14.110 | 14.059 | 14.098 | 14.111 | 1.245x | 0.719x |
| shared_b8_fast | 14.314 | 14.305 | 14.306 | 14.339 | 1.227x | 0.709x |
| replay_bfull_fast | 14.728 | 14.684 | 14.708 | 14.743 | 1.192x | 0.689x |
| shared_b8_accurate | 14.741 | 14.733 | 14.737 | 14.755 | 1.191x | 0.688x |
| replay_b32_fast | 14.752 | 14.733 | 14.733 | 14.752 | 1.190x | 0.688x |
| shared_b4_fast | 15.266 | 15.244 | 15.257 | 15.272 | 1.150x | 0.665x |
| replay_b8_fast | 15.359 | 15.340 | 15.358 | 15.396 | 1.143x | 0.661x |
| shared_b4_accurate | 16.021 | 15.992 | 16.018 | 16.024 | 1.096x | 0.633x |
| shared_b2_fast | 17.103 | 16.917 | 16.939 | 17.107 | 1.027x | 0.593x |
| original | 17.560 | 17.411 | 17.413 | 17.618 | 1.000x | 0.578x |
| shared_b2_accurate | 18.758 | 18.625 | 18.742 | 18.771 | 0.936x | 0.541x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_bf16_inputs | 20.273 | 20.132 | 20.266 | 20.409 | 1.664x | 1.020x |
| mkl_fp32 | 20.672 | 19.868 | 20.632 | 20.779 | 1.632x | 1.000x |
| shared_bfull_fast | 32.014 | 31.432 | 31.876 | 32.347 | 1.054x | 0.646x |
| shared_b32_fast | 32.418 | 31.602 | 31.707 | 32.487 | 1.041x | 0.638x |
| shared_b8_fast | 32.824 | 32.537 | 32.606 | 32.876 | 1.028x | 0.630x |
| shared_bfull_accurate | 32.831 | 32.007 | 32.183 | 32.908 | 1.027x | 0.630x |
| replay_bfull_fast | 33.601 | 33.238 | 33.304 | 33.636 | 1.004x | 0.615x |
| original | 33.733 | 33.514 | 33.730 | 33.748 | 1.000x | 0.613x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.310e-03 | 4.422e-01 | 4.846e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 2.315e-07 | 4.959e-05 | 5.435e-07 | 1 |
| original | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| shared_b2_fast | kernel_full | mkl_bf16_inputs | 2.389e-03 | 2.060e-01 | 2.258e-03 | 1 |
| shared_b2_fast | kernel_full | mkl_fp32 | 7.094e-03 | 6.256e-01 | 6.823e-03 | 1 |
| shared_b2_accurate | kernel_full | mkl_bf16_inputs | 2.317e-07 | 6.104e-05 | 6.690e-07 | 1 |
| shared_b2_accurate | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| shared_b4_fast | kernel_full | mkl_bf16_inputs | 2.992e-03 | 2.668e-01 | 2.925e-03 | 1 |
| shared_b4_fast | kernel_full | mkl_fp32 | 7.742e-03 | 7.090e-01 | 7.733e-03 | 1 |
| shared_b4_accurate | kernel_full | mkl_bf16_inputs | 2.084e-07 | 4.578e-05 | 5.017e-07 | 1 |
| shared_b4_accurate | kernel_full | mkl_fp32 | 5.285e-03 | 4.421e-01 | 4.823e-03 | 1 |
| shared_b8_fast | kernel_full | mkl_bf16_inputs | 3.173e-03 | 2.797e-01 | 3.066e-03 | 1 |
| shared_b8_fast | kernel_full | mkl_fp32 | 7.960e-03 | 7.048e-01 | 7.687e-03 | 1 |
| shared_b8_accurate | kernel_full | mkl_bf16_inputs | 6.282e-07 | 1.669e-04 | 1.829e-06 | 1 |
| shared_b8_accurate | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| replay_b8_fast | kernel_full | mkl_bf16_inputs | 3.173e-03 | 2.797e-01 | 3.066e-03 | 1 |
| replay_b8_fast | kernel_full | mkl_fp32 | 7.960e-03 | 7.048e-01 | 7.687e-03 | 1 |
| shared_b16_fast | kernel_full | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_b16_fast | kernel_full | mkl_fp32 | 8.069e-03 | 6.918e-01 | 7.546e-03 | 1 |
| shared_b16_accurate | kernel_full | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| shared_b16_accurate | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| shared_b32_fast | kernel_full | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_b32_fast | kernel_full | mkl_fp32 | 8.069e-03 | 6.918e-01 | 7.546e-03 | 1 |
| shared_b32_accurate | kernel_full | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| shared_b32_accurate | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| replay_b32_fast | kernel_full | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| replay_b32_fast | kernel_full | mkl_fp32 | 8.069e-03 | 6.918e-01 | 7.546e-03 | 1 |
| shared_b64_fast | kernel_full | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_b64_fast | kernel_full | mkl_fp32 | 8.069e-03 | 6.918e-01 | 7.546e-03 | 1 |
| shared_b64_accurate | kernel_full | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| shared_b64_accurate | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_fp32 | 8.069e-03 | 6.918e-01 | 7.546e-03 | 1 |
| shared_bfull_accurate | kernel_full | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| shared_bfull_accurate | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_fp32 | 8.069e-03 | 6.918e-01 | 7.546e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast | instrumented_kernel | mkl_bf16_inputs | 2.389e-03 | 2.060e-01 | 2.258e-03 | 1 |
| shared_b2_accurate | instrumented_kernel | mkl_bf16_inputs | 2.317e-07 | 6.104e-05 | 6.690e-07 | 1 |
| shared_b4_fast | instrumented_kernel | mkl_bf16_inputs | 2.992e-03 | 2.668e-01 | 2.925e-03 | 1 |
| shared_b4_accurate | instrumented_kernel | mkl_bf16_inputs | 2.084e-07 | 4.578e-05 | 5.017e-07 | 1 |
| shared_b8_fast | instrumented_kernel | mkl_bf16_inputs | 3.173e-03 | 2.797e-01 | 3.066e-03 | 1 |
| shared_b8_accurate | instrumented_kernel | mkl_bf16_inputs | 6.282e-07 | 1.669e-04 | 1.829e-06 | 1 |
| replay_b8_fast | instrumented_kernel | mkl_bf16_inputs | 3.173e-03 | 2.797e-01 | 3.066e-03 | 1 |
| shared_b16_fast | instrumented_kernel | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_b16_accurate | instrumented_kernel | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| shared_b32_fast | instrumented_kernel | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_b32_accurate | instrumented_kernel | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| replay_b32_fast | instrumented_kernel | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_b64_fast | instrumented_kernel | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_b64_accurate | instrumented_kernel | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| shared_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_bfull_accurate | instrumented_kernel | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| replay_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.058e-02 | 3.081e+01 | 1.085e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 1.291e-05 | 3.492e-01 | 1.230e-04 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| shared_b8_fast | two_layer_e2e | mkl_bf16_inputs | 5.523e-03 | 1.823e+01 | 6.420e-03 | 1 |
| shared_b8_fast | two_layer_e2e | mkl_fp32 | 1.564e-02 | 4.586e+01 | 1.598e-02 | 1 |
| shared_b32_fast | two_layer_e2e | mkl_bf16_inputs | 5.915e-03 | 1.857e+01 | 6.539e-03 | 1 |
| shared_b32_fast | two_layer_e2e | mkl_fp32 | 1.597e-02 | 4.830e+01 | 1.683e-02 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.915e-03 | 1.857e+01 | 6.539e-03 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_fp32 | 1.597e-02 | 4.830e+01 | 1.683e-02 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_bf16_inputs | 3.003e-05 | 4.364e-01 | 1.537e-04 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.915e-03 | 1.857e+01 | 6.539e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_fp32 | 1.597e-02 | 4.830e+01 | 1.683e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 10.7639 | 65 | 15.494 |
| original | row_schedule | 0.0200 | 65 | 15.494 |
| original | partial_zero | 0.0200 | 65 | 15.494 |
| original | csr_prefetch | 0.2301 | 65 | 15.494 |
| original | feature_gather_decode | 0.1314 | 65 | 15.494 |
| original | fp32_neighbor_reduction | 0.0000 | 65 | 15.494 |
| original | partial_bf16_conversion | 0.0000 | 65 | 15.494 |
| original | tile_load | 0.5734 | 65 | 15.494 |
| original | tile_compute | 0.6955 | 65 | 15.494 |
| original | tile_store | 0.0272 | 65 | 15.494 |
| original | output_scatter | 0.0279 | 65 | 15.494 |
| original | thread_amx_setup | 0.0393 | 65 | 15.494 |
| shared_b8_fast | output_zero | 10.1659 | 65 | 14.430 |
| shared_b8_fast | row_schedule | 0.0184 | 65 | 14.430 |
| shared_b8_fast | partial_zero | 0.0777 | 65 | 14.430 |
| shared_b8_fast | csr_prefetch | 0.4866 | 65 | 14.430 |
| shared_b8_fast | feature_gather_decode | 0.7811 | 65 | 14.430 |
| shared_b8_fast | fp32_neighbor_reduction | 0.5355 | 65 | 14.430 |
| shared_b8_fast | partial_bf16_conversion | 0.0658 | 65 | 14.430 |
| shared_b8_fast | tile_load | 0.2301 | 65 | 14.430 |
| shared_b8_fast | tile_compute | 0.5140 | 65 | 14.430 |
| shared_b8_fast | tile_store | 0.0231 | 65 | 14.430 |
| shared_b8_fast | output_scatter | 0.0076 | 65 | 14.430 |
| shared_b8_fast | thread_amx_setup | 0.0255 | 65 | 14.430 |
| shared_bfull_fast | output_zero | 10.1931 | 65 | 14.045 |
| shared_bfull_fast | row_schedule | 0.0262 | 65 | 14.045 |
| shared_bfull_fast | partial_zero | 0.0348 | 65 | 14.045 |
| shared_bfull_fast | csr_prefetch | 0.5000 | 65 | 14.045 |
| shared_bfull_fast | feature_gather_decode | 0.6433 | 65 | 14.045 |
| shared_bfull_fast | fp32_neighbor_reduction | 0.4809 | 65 | 14.045 |
| shared_bfull_fast | partial_bf16_conversion | 0.0520 | 65 | 14.045 |
| shared_bfull_fast | tile_load | 0.1340 | 65 | 14.045 |
| shared_bfull_fast | tile_compute | 0.3340 | 65 | 14.045 |
| shared_bfull_fast | tile_store | 0.0086 | 65 | 14.045 |
| shared_bfull_fast | output_scatter | 0.0160 | 65 | 14.045 |
| shared_bfull_fast | thread_amx_setup | 0.0281 | 65 | 14.045 |
| shared_bfull_accurate | output_zero | 10.1569 | 65 | 14.188 |
| shared_bfull_accurate | row_schedule | 0.0246 | 65 | 14.188 |
| shared_bfull_accurate | partial_zero | 0.0427 | 65 | 14.188 |
| shared_bfull_accurate | csr_prefetch | 0.4835 | 65 | 14.188 |
| shared_bfull_accurate | feature_gather_decode | 0.7055 | 65 | 14.188 |
| shared_bfull_accurate | fp32_neighbor_reduction | 0.5484 | 65 | 14.188 |
| shared_bfull_accurate | partial_bf16_conversion | 0.0224 | 65 | 14.188 |
| shared_bfull_accurate | tile_load | 0.2265 | 65 | 14.188 |
| shared_bfull_accurate | tile_compute | 0.4287 | 65 | 14.188 |
| shared_bfull_accurate | tile_store | 0.0100 | 65 | 14.188 |
| shared_bfull_accurate | output_scatter | 0.0236 | 65 | 14.188 |
| shared_bfull_accurate | thread_amx_setup | 0.0257 | 65 | 14.188 |
| replay_bfull_fast | output_zero | 10.2050 | 65 | 15.057 |
| replay_bfull_fast | row_schedule | 0.0179 | 65 | 15.057 |
| replay_bfull_fast | partial_zero | 0.0451 | 65 | 15.057 |
| replay_bfull_fast | csr_prefetch | 0.9596 | 65 | 15.057 |
| replay_bfull_fast | feature_gather_decode | 1.2538 | 65 | 15.057 |
| replay_bfull_fast | fp32_neighbor_reduction | 0.9921 | 65 | 15.057 |
| replay_bfull_fast | partial_bf16_conversion | 0.0718 | 65 | 15.057 |
| replay_bfull_fast | tile_load | 0.0851 | 65 | 15.057 |
| replay_bfull_fast | tile_compute | 0.3395 | 65 | 15.057 |
| replay_bfull_fast | tile_store | 0.0358 | 65 | 15.057 |
| replay_bfull_fast | output_scatter | 0.0539 | 65 | 15.057 |
| replay_bfull_fast | thread_amx_setup | 0.0415 | 65 | 15.057 |

## regular-q256

N=262144, E=67108864, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_accurate | 45.396 | 45.290 | 45.361 | 45.440 | 1.620x | 1.991x |
| shared_bfull_fast | 45.452 | 45.112 | 45.297 | 45.590 | 1.618x | 1.989x |
| shared_b64_fast | 45.913 | 45.350 | 45.688 | 45.930 | 1.601x | 1.969x |
| shared_b64_accurate | 46.509 | 46.359 | 46.403 | 46.566 | 1.581x | 1.943x |
| shared_b32_fast | 46.599 | 45.785 | 46.287 | 46.633 | 1.578x | 1.940x |
| shared_b32_accurate | 47.591 | 46.959 | 47.549 | 47.741 | 1.545x | 1.899x |
| shared_b16_fast | 50.066 | 49.640 | 49.985 | 50.127 | 1.468x | 1.805x |
| shared_b16_accurate | 52.275 | 52.115 | 52.263 | 52.281 | 1.406x | 1.729x |
| shared_b8_fast | 56.129 | 55.647 | 55.933 | 56.462 | 1.310x | 1.610x |
| replay_bfull_fast | 60.568 | 60.493 | 60.551 | 60.594 | 1.214x | 1.492x |
| shared_b8_accurate | 62.060 | 61.117 | 61.795 | 62.110 | 1.185x | 1.456x |
| replay_b32_fast | 62.413 | 61.376 | 62.324 | 62.481 | 1.178x | 1.448x |
| shared_b4_fast | 69.981 | 69.733 | 69.933 | 70.056 | 1.051x | 1.292x |
| original | 73.522 | 73.400 | 73.457 | 73.678 | 1.000x | 1.229x |
| replay_b8_fast | 76.141 | 74.206 | 75.872 | 76.166 | 0.966x | 1.187x |
| shared_b4_accurate | 83.875 | 83.330 | 83.466 | 84.003 | 0.877x | 1.078x |
| mkl_bf16_inputs | 89.877 | 89.780 | 89.784 | 89.906 | 0.818x | 1.006x |
| mkl_fp32 | 90.385 | 90.289 | 90.361 | 90.394 | 0.813x | 1.000x |
| shared_b2_fast | 102.228 | 101.056 | 101.228 | 103.242 | 0.719x | 0.884x |
| shared_b2_accurate | 127.240 | 127.203 | 127.206 | 127.302 | 0.578x | 0.710x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast | 93.391 | 92.256 | 93.250 | 93.666 | 1.556x | 1.944x |
| shared_bfull_accurate | 94.143 | 93.892 | 93.932 | 94.235 | 1.544x | 1.928x |
| shared_b32_fast | 96.356 | 95.506 | 96.129 | 96.440 | 1.509x | 1.884x |
| shared_b8_fast | 116.601 | 116.171 | 116.433 | 116.674 | 1.247x | 1.557x |
| replay_bfull_fast | 124.608 | 124.399 | 124.409 | 124.918 | 1.166x | 1.457x |
| original | 145.355 | 145.056 | 145.338 | 145.904 | 1.000x | 1.249x |
| mkl_bf16_inputs | 180.108 | 179.950 | 180.040 | 181.059 | 0.807x | 1.008x |
| mkl_fp32 | 181.547 | 181.342 | 181.507 | 181.549 | 0.801x | 1.000x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.274e-03 | 1.855e+00 | 5.164e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.249e-03 | 1.855e+00 | 5.138e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 6.099e-07 | 5.798e-04 | 1.614e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.249e-03 | 1.855e+00 | 5.137e-03 | 1 |
| shared_b2_fast | kernel_full | mkl_bf16_inputs | 2.376e-03 | 8.840e-01 | 2.461e-03 | 1 |
| shared_b2_fast | kernel_full | mkl_fp32 | 7.051e-03 | 2.488e+00 | 6.890e-03 | 1 |
| shared_b2_accurate | kernel_full | mkl_bf16_inputs | 6.126e-07 | 5.188e-04 | 1.444e-06 | 1 |
| shared_b2_accurate | kernel_full | mkl_fp32 | 5.249e-03 | 1.855e+00 | 5.138e-03 | 1 |
| shared_b4_fast | kernel_full | mkl_bf16_inputs | 2.982e-03 | 1.084e+00 | 3.018e-03 | 1 |
| shared_b4_fast | kernel_full | mkl_fp32 | 7.699e-03 | 2.759e+00 | 7.640e-03 | 1 |
| shared_b4_accurate | kernel_full | mkl_bf16_inputs | 4.575e-07 | 3.510e-04 | 9.768e-07 | 1 |
| shared_b4_accurate | kernel_full | mkl_fp32 | 5.249e-03 | 1.856e+00 | 5.138e-03 | 1 |
| shared_b8_fast | kernel_full | mkl_bf16_inputs | 3.164e-03 | 1.026e+00 | 2.856e-03 | 1 |
| shared_b8_fast | kernel_full | mkl_fp32 | 7.918e-03 | 2.863e+00 | 7.928e-03 | 1 |
| shared_b8_accurate | kernel_full | mkl_bf16_inputs | 6.903e-07 | 3.662e-04 | 1.019e-06 | 1 |
| shared_b8_accurate | kernel_full | mkl_fp32 | 5.249e-03 | 1.855e+00 | 5.138e-03 | 1 |
| replay_b8_fast | kernel_full | mkl_bf16_inputs | 3.164e-03 | 1.026e+00 | 2.856e-03 | 1 |
| replay_b8_fast | kernel_full | mkl_fp32 | 7.918e-03 | 2.863e+00 | 7.928e-03 | 1 |
| shared_b16_fast | kernel_full | mkl_bf16_inputs | 3.266e-03 | 1.211e+00 | 3.370e-03 | 1 |
| shared_b16_fast | kernel_full | mkl_fp32 | 8.028e-03 | 3.066e+00 | 8.491e-03 | 1 |
| shared_b16_accurate | kernel_full | mkl_bf16_inputs | 1.922e-06 | 7.305e-04 | 2.033e-06 | 1 |
| shared_b16_accurate | kernel_full | mkl_fp32 | 5.249e-03 | 1.855e+00 | 5.138e-03 | 1 |
| shared_b32_fast | kernel_full | mkl_bf16_inputs | 3.279e-03 | 1.110e+00 | 3.089e-03 | 1 |
| shared_b32_fast | kernel_full | mkl_fp32 | 8.045e-03 | 2.919e+00 | 8.083e-03 | 1 |
| shared_b32_accurate | kernel_full | mkl_bf16_inputs | 3.707e-06 | 1.358e-03 | 3.780e-06 | 1 |
| shared_b32_accurate | kernel_full | mkl_fp32 | 5.250e-03 | 1.856e+00 | 5.139e-03 | 1 |
| replay_b32_fast | kernel_full | mkl_bf16_inputs | 3.279e-03 | 1.110e+00 | 3.089e-03 | 1 |
| replay_b32_fast | kernel_full | mkl_fp32 | 8.045e-03 | 2.919e+00 | 8.083e-03 | 1 |
| shared_b64_fast | kernel_full | mkl_bf16_inputs | 3.303e-03 | 1.159e+00 | 3.225e-03 | 1 |
| shared_b64_fast | kernel_full | mkl_fp32 | 8.069e-03 | 2.897e+00 | 8.023e-03 | 1 |
| shared_b64_accurate | kernel_full | mkl_bf16_inputs | 5.718e-06 | 2.106e-03 | 5.861e-06 | 1 |
| shared_b64_accurate | kernel_full | mkl_fp32 | 5.252e-03 | 1.856e+00 | 5.140e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_bf16_inputs | 3.314e-03 | 1.190e+00 | 3.312e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_fp32 | 8.076e-03 | 2.869e+00 | 7.946e-03 | 1 |
| shared_bfull_accurate | kernel_full | mkl_bf16_inputs | 7.976e-06 | 3.235e-03 | 9.004e-06 | 1 |
| shared_bfull_accurate | kernel_full | mkl_fp32 | 5.254e-03 | 1.858e+00 | 5.145e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_bf16_inputs | 3.314e-03 | 1.190e+00 | 3.312e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_fp32 | 8.076e-03 | 2.869e+00 | 7.946e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast | instrumented_kernel | mkl_bf16_inputs | 2.376e-03 | 8.840e-01 | 2.461e-03 | 1 |
| shared_b2_accurate | instrumented_kernel | mkl_bf16_inputs | 6.126e-07 | 5.188e-04 | 1.444e-06 | 1 |
| shared_b4_fast | instrumented_kernel | mkl_bf16_inputs | 2.982e-03 | 1.084e+00 | 3.018e-03 | 1 |
| shared_b4_accurate | instrumented_kernel | mkl_bf16_inputs | 4.575e-07 | 3.510e-04 | 9.768e-07 | 1 |
| shared_b8_fast | instrumented_kernel | mkl_bf16_inputs | 3.164e-03 | 1.026e+00 | 2.856e-03 | 1 |
| shared_b8_accurate | instrumented_kernel | mkl_bf16_inputs | 6.903e-07 | 3.662e-04 | 1.019e-06 | 1 |
| replay_b8_fast | instrumented_kernel | mkl_bf16_inputs | 3.164e-03 | 1.026e+00 | 2.856e-03 | 1 |
| shared_b16_fast | instrumented_kernel | mkl_bf16_inputs | 3.266e-03 | 1.211e+00 | 3.370e-03 | 1 |
| shared_b16_accurate | instrumented_kernel | mkl_bf16_inputs | 1.922e-06 | 7.305e-04 | 2.033e-06 | 1 |
| shared_b32_fast | instrumented_kernel | mkl_bf16_inputs | 3.279e-03 | 1.110e+00 | 3.089e-03 | 1 |
| shared_b32_accurate | instrumented_kernel | mkl_bf16_inputs | 3.707e-06 | 1.358e-03 | 3.780e-06 | 1 |
| replay_b32_fast | instrumented_kernel | mkl_bf16_inputs | 3.279e-03 | 1.110e+00 | 3.089e-03 | 1 |
| shared_b64_fast | instrumented_kernel | mkl_bf16_inputs | 3.303e-03 | 1.159e+00 | 3.225e-03 | 1 |
| shared_b64_accurate | instrumented_kernel | mkl_bf16_inputs | 5.718e-06 | 2.106e-03 | 5.861e-06 | 1 |
| shared_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.314e-03 | 1.190e+00 | 3.312e-03 | 1 |
| shared_bfull_accurate | instrumented_kernel | mkl_bf16_inputs | 7.976e-06 | 3.235e-03 | 9.004e-06 | 1 |
| replay_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.314e-03 | 1.190e+00 | 3.312e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.047e-02 | 1.487e+03 | 1.154e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.036e-02 | 1.487e+03 | 1.141e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 5.376e-06 | 2.566e+00 | 1.991e-05 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.036e-02 | 1.487e+03 | 1.141e-02 | 1 |
| shared_b8_fast | two_layer_e2e | mkl_bf16_inputs | 5.235e-03 | 7.062e+02 | 5.479e-03 | 1 |
| shared_b8_fast | two_layer_e2e | mkl_fp32 | 1.547e-02 | 2.172e+03 | 1.666e-02 | 1 |
| shared_b32_fast | two_layer_e2e | mkl_bf16_inputs | 5.607e-03 | 8.013e+02 | 6.217e-03 | 1 |
| shared_b32_fast | two_layer_e2e | mkl_fp32 | 1.583e-02 | 2.225e+03 | 1.707e-02 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.749e-03 | 9.510e+02 | 7.379e-03 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_fp32 | 1.584e-02 | 2.304e+03 | 1.767e-02 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_bf16_inputs | 2.362e-05 | 6.715e+00 | 5.210e-05 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_fp32 | 1.037e-02 | 1.488e+03 | 1.142e-02 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.749e-03 | 9.510e+02 | 7.379e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_fp32 | 1.584e-02 | 2.304e+03 | 1.767e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 10.4570 | 65 | 71.558 |
| original | row_schedule | 0.0162 | 65 | 71.558 |
| original | partial_zero | 0.0260 | 65 | 71.558 |
| original | csr_prefetch | 3.5996 | 65 | 71.558 |
| original | feature_gather_decode | 1.6849 | 65 | 71.558 |
| original | fp32_neighbor_reduction | 0.0000 | 65 | 71.558 |
| original | partial_bf16_conversion | 0.0000 | 65 | 71.558 |
| original | tile_load | 8.5244 | 65 | 71.558 |
| original | tile_compute | 8.8634 | 65 | 71.558 |
| original | tile_store | 0.0286 | 65 | 71.558 |
| original | output_scatter | 0.0312 | 65 | 71.558 |
| original | thread_amx_setup | 0.0451 | 65 | 71.558 |
| shared_b8_fast | output_zero | 10.7460 | 65 | 58.365 |
| shared_b8_fast | row_schedule | 0.0255 | 65 | 58.365 |
| shared_b8_fast | partial_zero | 0.3686 | 65 | 58.365 |
| shared_b8_fast | csr_prefetch | 8.1995 | 65 | 58.365 |
| shared_b8_fast | feature_gather_decode | 12.6488 | 65 | 58.365 |
| shared_b8_fast | fp32_neighbor_reduction | 8.7867 | 65 | 58.365 |
| shared_b8_fast | partial_bf16_conversion | 0.3030 | 65 | 58.365 |
| shared_b8_fast | tile_load | 1.7452 | 65 | 58.365 |
| shared_b8_fast | tile_compute | 4.8342 | 65 | 58.365 |
| shared_b8_fast | tile_store | 0.2186 | 65 | 58.365 |
| shared_b8_fast | output_scatter | 0.0148 | 65 | 58.365 |
| shared_b8_fast | thread_amx_setup | 0.0319 | 65 | 58.365 |
| shared_bfull_fast | output_zero | 10.7520 | 65 | 45.614 |
| shared_bfull_fast | row_schedule | 0.0196 | 65 | 45.614 |
| shared_bfull_fast | partial_zero | 0.0465 | 65 | 45.614 |
| shared_bfull_fast | csr_prefetch | 7.1340 | 65 | 45.614 |
| shared_bfull_fast | feature_gather_decode | 9.2561 | 65 | 45.614 |
| shared_bfull_fast | fp32_neighbor_reduction | 6.7737 | 65 | 45.614 |
| shared_bfull_fast | partial_bf16_conversion | 0.0548 | 65 | 45.614 |
| shared_bfull_fast | tile_load | 0.1624 | 65 | 45.614 |
| shared_bfull_fast | tile_compute | 0.3636 | 65 | 45.614 |
| shared_bfull_fast | tile_store | 0.0138 | 65 | 45.614 |
| shared_bfull_fast | output_scatter | 0.0145 | 65 | 45.614 |
| shared_bfull_fast | thread_amx_setup | 0.0281 | 65 | 45.614 |
| shared_bfull_accurate | output_zero | 10.8001 | 65 | 46.033 |
| shared_bfull_accurate | row_schedule | 0.0198 | 65 | 46.033 |
| shared_bfull_accurate | partial_zero | 0.0489 | 65 | 46.033 |
| shared_bfull_accurate | csr_prefetch | 7.1318 | 65 | 46.033 |
| shared_bfull_accurate | feature_gather_decode | 9.3102 | 65 | 46.033 |
| shared_bfull_accurate | fp32_neighbor_reduction | 6.8777 | 65 | 46.033 |
| shared_bfull_accurate | partial_bf16_conversion | 0.0303 | 65 | 46.033 |
| shared_bfull_accurate | tile_load | 0.2098 | 65 | 46.033 |
| shared_bfull_accurate | tile_compute | 0.3994 | 65 | 46.033 |
| shared_bfull_accurate | tile_store | 0.0143 | 65 | 46.033 |
| shared_bfull_accurate | output_scatter | 0.0126 | 65 | 46.033 |
| shared_bfull_accurate | thread_amx_setup | 0.0360 | 65 | 46.033 |
| replay_bfull_fast | output_zero | 10.8008 | 65 | 62.917 |
| replay_bfull_fast | row_schedule | 0.0217 | 65 | 62.917 |
| replay_bfull_fast | partial_zero | 0.0396 | 65 | 62.917 |
| replay_bfull_fast | csr_prefetch | 14.5011 | 65 | 62.917 |
| replay_bfull_fast | feature_gather_decode | 17.8401 | 65 | 62.917 |
| replay_bfull_fast | fp32_neighbor_reduction | 13.6964 | 65 | 62.917 |
| replay_bfull_fast | partial_bf16_conversion | 0.0739 | 65 | 62.917 |
| replay_bfull_fast | tile_load | 0.2708 | 65 | 62.917 |
| replay_bfull_fast | tile_compute | 0.4275 | 65 | 62.917 |
| replay_bfull_fast | tile_store | 0.0288 | 65 | 62.917 |
| replay_bfull_fast | output_scatter | 0.0515 | 65 | 62.917 |
| replay_bfull_fast | thread_amx_setup | 0.0410 | 65 | 62.917 |

## soc-Pokec

N=1632803, E=30622564, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_bf16_inputs | 63.067 | 62.810 | 63.032 | 63.076 | 1.756x | 1.006x |
| mkl_fp32 | 63.461 | 63.440 | 63.441 | 63.490 | 1.745x | 1.000x |
| shared_bfull_fast | 101.821 | 101.488 | 101.804 | 101.828 | 1.088x | 0.623x |
| shared_b64_fast | 102.821 | 101.586 | 102.363 | 102.898 | 1.077x | 0.617x |
| shared_b32_fast | 103.225 | 102.447 | 103.150 | 103.307 | 1.073x | 0.615x |
| shared_bfull_accurate | 104.208 | 103.138 | 103.637 | 104.321 | 1.063x | 0.609x |
| shared_b64_accurate | 105.035 | 103.980 | 104.113 | 105.076 | 1.054x | 0.604x |
| shared_b32_accurate | 105.600 | 105.259 | 105.469 | 107.022 | 1.049x | 0.601x |
| shared_b16_fast | 106.827 | 105.679 | 106.646 | 106.834 | 1.037x | 0.594x |
| shared_b16_accurate | 108.878 | 108.071 | 108.260 | 109.275 | 1.017x | 0.583x |
| original | 110.746 | 109.214 | 109.683 | 111.242 | 1.000x | 0.573x |
| replay_bfull_fast | 111.440 | 111.277 | 111.395 | 111.648 | 0.994x | 0.569x |
| shared_b8_fast | 111.652 | 109.680 | 111.648 | 112.440 | 0.992x | 0.568x |
| replay_b32_fast | 113.520 | 112.768 | 113.058 | 115.346 | 0.976x | 0.559x |
| shared_b8_accurate | 115.238 | 113.620 | 115.161 | 115.848 | 0.961x | 0.551x |
| shared_b4_fast | 120.727 | 119.809 | 120.667 | 120.874 | 0.917x | 0.526x |
| replay_b8_fast | 122.179 | 120.984 | 121.902 | 122.528 | 0.906x | 0.519x |
| shared_b4_accurate | 127.043 | 126.180 | 126.512 | 128.080 | 0.872x | 0.500x |
| shared_b2_fast | 136.517 | 132.143 | 134.943 | 137.197 | 0.811x | 0.465x |
| shared_b2_accurate | 153.352 | 151.859 | 153.033 | 153.451 | 0.722x | 0.414x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_fp32 | 130.018 | 129.585 | 129.798 | 130.719 | 1.702x | 1.000x |
| mkl_bf16_inputs | 135.043 | 134.900 | 134.930 | 135.115 | 1.638x | 0.963x |
| shared_bfull_fast | 210.465 | 209.599 | 210.094 | 210.482 | 1.051x | 0.618x |
| shared_b32_fast | 213.364 | 212.765 | 212.901 | 213.458 | 1.037x | 0.609x |
| shared_bfull_accurate | 215.679 | 214.544 | 214.880 | 216.310 | 1.026x | 0.603x |
| original | 221.242 | 219.623 | 220.946 | 221.723 | 1.000x | 0.588x |
| shared_b8_fast | 230.400 | 228.937 | 229.530 | 230.428 | 0.960x | 0.564x |
| replay_bfull_fast | 233.370 | 233.026 | 233.234 | 233.688 | 0.948x | 0.557x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.319e-03 | 5.965e+00 | 4.261e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.293e-03 | 5.965e+00 | 4.244e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 3.741e-07 | 1.160e-02 | 8.283e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.293e-03 | 5.968e+00 | 4.246e-03 | 1 |
| shared_b2_fast | kernel_full | mkl_bf16_inputs | 2.355e-03 | 2.712e+00 | 1.937e-03 | 1 |
| shared_b2_fast | kernel_full | mkl_fp32 | 7.059e-03 | 8.610e+00 | 6.125e-03 | 1 |
| shared_b2_accurate | kernel_full | mkl_bf16_inputs | 3.740e-07 | 6.104e-03 | 4.360e-06 | 1 |
| shared_b2_accurate | kernel_full | mkl_fp32 | 5.293e-03 | 5.960e+00 | 4.240e-03 | 1 |
| shared_b4_fast | kernel_full | mkl_bf16_inputs | 2.950e-03 | 4.213e+00 | 3.009e-03 | 1 |
| shared_b4_fast | kernel_full | mkl_fp32 | 7.696e-03 | 9.557e+00 | 6.799e-03 | 1 |
| shared_b4_accurate | kernel_full | mkl_bf16_inputs | 3.019e-07 | 5.432e-03 | 3.880e-06 | 1 |
| shared_b4_accurate | kernel_full | mkl_fp32 | 5.293e-03 | 5.967e+00 | 4.245e-03 | 1 |
| shared_b8_fast | kernel_full | mkl_bf16_inputs | 3.130e-03 | 3.773e+00 | 2.695e-03 | 1 |
| shared_b8_fast | kernel_full | mkl_fp32 | 7.914e-03 | 9.197e+00 | 6.543e-03 | 1 |
| shared_b8_accurate | kernel_full | mkl_bf16_inputs | 6.111e-07 | 2.686e-03 | 1.918e-06 | 1 |
| shared_b8_accurate | kernel_full | mkl_fp32 | 5.293e-03 | 5.965e+00 | 4.244e-03 | 1 |
| replay_b8_fast | kernel_full | mkl_bf16_inputs | 3.130e-03 | 3.773e+00 | 2.695e-03 | 1 |
| replay_b8_fast | kernel_full | mkl_fp32 | 7.914e-03 | 9.197e+00 | 6.543e-03 | 1 |
| shared_b16_fast | kernel_full | mkl_bf16_inputs | 3.229e-03 | 5.043e+00 | 3.602e-03 | 1 |
| shared_b16_fast | kernel_full | mkl_fp32 | 8.022e-03 | 1.063e+01 | 7.561e-03 | 1 |
| shared_b16_accurate | kernel_full | mkl_bf16_inputs | 1.735e-06 | 3.113e-03 | 2.223e-06 | 1 |
| shared_b16_accurate | kernel_full | mkl_fp32 | 5.293e-03 | 5.964e+00 | 4.243e-03 | 1 |
| shared_b32_fast | kernel_full | mkl_bf16_inputs | 3.247e-03 | 4.145e+00 | 2.961e-03 | 1 |
| shared_b32_fast | kernel_full | mkl_fp32 | 8.046e-03 | 9.731e+00 | 6.923e-03 | 1 |
| shared_b32_accurate | kernel_full | mkl_bf16_inputs | 3.113e-06 | 5.493e-03 | 3.924e-06 | 1 |
| shared_b32_accurate | kernel_full | mkl_fp32 | 5.294e-03 | 5.967e+00 | 4.245e-03 | 1 |
| replay_b32_fast | kernel_full | mkl_bf16_inputs | 3.247e-03 | 4.145e+00 | 2.961e-03 | 1 |
| replay_b32_fast | kernel_full | mkl_fp32 | 8.046e-03 | 9.731e+00 | 6.923e-03 | 1 |
| shared_b64_fast | kernel_full | mkl_bf16_inputs | 3.264e-03 | 4.400e+00 | 3.143e-03 | 1 |
| shared_b64_fast | kernel_full | mkl_fp32 | 8.063e-03 | 9.986e+00 | 7.104e-03 | 1 |
| shared_b64_accurate | kernel_full | mkl_bf16_inputs | 4.327e-06 | 6.104e-03 | 4.360e-06 | 1 |
| shared_b64_accurate | kernel_full | mkl_fp32 | 5.294e-03 | 5.966e+00 | 4.245e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_fp32 | 8.064e-03 | 9.351e+00 | 6.653e-03 | 1 |
| shared_bfull_accurate | kernel_full | mkl_bf16_inputs | 4.976e-06 | 1.331e-02 | 9.504e-06 | 1 |
| shared_bfull_accurate | kernel_full | mkl_fp32 | 5.295e-03 | 5.975e+00 | 4.251e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_fp32 | 8.064e-03 | 9.351e+00 | 6.653e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast | instrumented_kernel | mkl_bf16_inputs | 2.355e-03 | 2.712e+00 | 1.937e-03 | 1 |
| shared_b2_accurate | instrumented_kernel | mkl_bf16_inputs | 3.740e-07 | 6.104e-03 | 4.360e-06 | 1 |
| shared_b4_fast | instrumented_kernel | mkl_bf16_inputs | 2.950e-03 | 4.213e+00 | 3.009e-03 | 1 |
| shared_b4_accurate | instrumented_kernel | mkl_bf16_inputs | 3.019e-07 | 5.432e-03 | 3.880e-06 | 1 |
| shared_b8_fast | instrumented_kernel | mkl_bf16_inputs | 3.130e-03 | 3.773e+00 | 2.695e-03 | 1 |
| shared_b8_accurate | instrumented_kernel | mkl_bf16_inputs | 6.111e-07 | 2.686e-03 | 1.918e-06 | 1 |
| replay_b8_fast | instrumented_kernel | mkl_bf16_inputs | 3.130e-03 | 3.773e+00 | 2.695e-03 | 1 |
| shared_b16_fast | instrumented_kernel | mkl_bf16_inputs | 3.229e-03 | 5.043e+00 | 3.602e-03 | 1 |
| shared_b16_accurate | instrumented_kernel | mkl_bf16_inputs | 1.735e-06 | 3.113e-03 | 2.223e-06 | 1 |
| shared_b32_fast | instrumented_kernel | mkl_bf16_inputs | 3.247e-03 | 4.145e+00 | 2.961e-03 | 1 |
| shared_b32_accurate | instrumented_kernel | mkl_bf16_inputs | 3.113e-06 | 5.493e-03 | 3.924e-06 | 1 |
| replay_b32_fast | instrumented_kernel | mkl_bf16_inputs | 3.247e-03 | 4.145e+00 | 2.961e-03 | 1 |
| shared_b64_fast | instrumented_kernel | mkl_bf16_inputs | 3.264e-03 | 4.400e+00 | 3.143e-03 | 1 |
| shared_b64_accurate | instrumented_kernel | mkl_bf16_inputs | 4.327e-06 | 6.104e-03 | 4.360e-06 | 1 |
| shared_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
| shared_bfull_accurate | instrumented_kernel | mkl_bf16_inputs | 4.976e-06 | 1.331e-02 | 9.504e-06 | 1 |
| replay_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.015e-02 | 1.593e+04 | 9.942e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.005e-02 | 1.593e+04 | 9.844e-03 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 7.647e-06 | 7.250e+00 | 4.525e-06 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.005e-02 | 1.593e+04 | 9.847e-03 | 1 |
| shared_b8_fast | two_layer_e2e | mkl_bf16_inputs | 5.266e-03 | 8.265e+03 | 5.159e-03 | 1 |
| shared_b8_fast | two_layer_e2e | mkl_fp32 | 1.516e-02 | 2.419e+04 | 1.495e-02 | 1 |
| shared_b32_fast | two_layer_e2e | mkl_bf16_inputs | 5.669e-03 | 8.903e+03 | 5.557e-03 | 1 |
| shared_b32_fast | two_layer_e2e | mkl_fp32 | 1.553e-02 | 2.483e+04 | 1.535e-02 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 9.892e+03 | 6.175e-03 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_fp32 | 1.561e-02 | 2.582e+04 | 1.596e-02 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_bf16_inputs | 3.211e-05 | 1.731e+01 | 1.081e-05 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_fp32 | 1.006e-02 | 1.594e+04 | 9.854e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 9.892e+03 | 6.175e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_fp32 | 1.561e-02 | 2.582e+04 | 1.596e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 65.5038 | 400 | 125.772 |
| original | row_schedule | 0.1476 | 400 | 125.772 |
| original | partial_zero | 0.1307 | 400 | 125.772 |
| original | csr_prefetch | 2.7266 | 400 | 125.772 |
| original | feature_gather_decode | 1.5948 | 400 | 125.772 |
| original | fp32_neighbor_reduction | 0.0000 | 400 | 125.772 |
| original | partial_bf16_conversion | 0.0000 | 400 | 125.772 |
| original | tile_load | 8.5735 | 400 | 125.772 |
| original | tile_compute | 10.6263 | 400 | 125.772 |
| original | tile_store | 0.1724 | 400 | 125.772 |
| original | output_scatter | 0.1390 | 400 | 125.772 |
| original | thread_amx_setup | 0.1869 | 400 | 125.772 |
| shared_b8_fast | output_zero | 65.7461 | 400 | 118.694 |
| shared_b8_fast | row_schedule | 0.1512 | 400 | 118.694 |
| shared_b8_fast | partial_zero | 0.3967 | 400 | 118.694 |
| shared_b8_fast | csr_prefetch | 5.0328 | 400 | 118.694 |
| shared_b8_fast | feature_gather_decode | 7.2167 | 400 | 118.694 |
| shared_b8_fast | fp32_neighbor_reduction | 4.8208 | 400 | 118.694 |
| shared_b8_fast | partial_bf16_conversion | 0.3166 | 400 | 118.694 |
| shared_b8_fast | tile_load | 1.8423 | 400 | 118.694 |
| shared_b8_fast | tile_compute | 3.8381 | 400 | 118.694 |
| shared_b8_fast | tile_store | 0.2334 | 400 | 118.694 |
| shared_b8_fast | output_scatter | 0.0544 | 400 | 118.694 |
| shared_b8_fast | thread_amx_setup | 0.0386 | 400 | 118.694 |
| shared_bfull_fast | output_zero | 65.7630 | 400 | 106.658 |
| shared_bfull_fast | row_schedule | 0.1657 | 400 | 106.658 |
| shared_bfull_fast | partial_zero | 0.1063 | 400 | 106.658 |
| shared_bfull_fast | csr_prefetch | 4.5686 | 400 | 106.658 |
| shared_bfull_fast | feature_gather_decode | 5.8241 | 400 | 106.658 |
| shared_bfull_fast | fp32_neighbor_reduction | 3.8786 | 400 | 106.658 |
| shared_bfull_fast | partial_bf16_conversion | 0.0792 | 400 | 106.658 |
| shared_bfull_fast | tile_load | 0.5078 | 400 | 106.658 |
| shared_bfull_fast | tile_compute | 1.1919 | 400 | 106.658 |
| shared_bfull_fast | tile_store | 0.0601 | 400 | 106.658 |
| shared_bfull_fast | output_scatter | 0.0670 | 400 | 106.658 |
| shared_bfull_fast | thread_amx_setup | 0.0238 | 400 | 106.658 |
| shared_bfull_accurate | output_zero | 65.7170 | 400 | 105.748 |
| shared_bfull_accurate | row_schedule | 0.1726 | 400 | 105.748 |
| shared_bfull_accurate | partial_zero | 0.1056 | 400 | 105.748 |
| shared_bfull_accurate | csr_prefetch | 4.7007 | 400 | 105.748 |
| shared_bfull_accurate | feature_gather_decode | 5.7745 | 400 | 105.748 |
| shared_bfull_accurate | fp32_neighbor_reduction | 4.0469 | 400 | 105.748 |
| shared_bfull_accurate | partial_bf16_conversion | 0.1009 | 400 | 105.748 |
| shared_bfull_accurate | tile_load | 0.8566 | 400 | 105.748 |
| shared_bfull_accurate | tile_compute | 1.5705 | 400 | 105.748 |
| shared_bfull_accurate | tile_store | 0.0720 | 400 | 105.748 |
| shared_bfull_accurate | output_scatter | 0.0658 | 400 | 105.748 |
| shared_bfull_accurate | thread_amx_setup | 0.0305 | 400 | 105.748 |
| replay_bfull_fast | output_zero | 65.7849 | 400 | 120.155 |
| replay_bfull_fast | row_schedule | 0.1304 | 400 | 120.155 |
| replay_bfull_fast | partial_zero | 0.1574 | 400 | 120.155 |
| replay_bfull_fast | csr_prefetch | 8.6205 | 400 | 120.155 |
| replay_bfull_fast | feature_gather_decode | 10.4976 | 400 | 120.155 |
| replay_bfull_fast | fp32_neighbor_reduction | 7.9813 | 400 | 120.155 |
| replay_bfull_fast | partial_bf16_conversion | 0.1595 | 400 | 120.155 |
| replay_bfull_fast | tile_load | 0.3927 | 400 | 120.155 |
| replay_bfull_fast | tile_compute | 1.3320 | 400 | 120.155 |
| replay_bfull_fast | tile_store | 0.1829 | 400 | 120.155 |
| replay_bfull_fast | output_scatter | 0.1554 | 400 | 120.155 |
| replay_bfull_fast | thread_amx_setup | 0.0343 | 400 | 120.155 |

