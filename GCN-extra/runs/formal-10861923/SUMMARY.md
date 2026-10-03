# GCN-extra first inference experiments

Numerical sum-aggregation experiments with random H/W; no task-accuracy claim.
Uninstrumented medians; original source is unchanged. Speedups are original/new and FP32 MKL/new.
Profiles are sampled summed thread time and do not equal wall time. Physical AMX work is modeled, not a hardware counter.

## reddit

N=232965, E=114615892, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b32_fast_nozero | 61.189 | 60.608 | 60.946 | 64.027 | 2.483x | 1.697x |
| shared_bfull_accurate_nozero | 61.817 | 60.706 | 60.981 | 64.571 | 2.458x | 1.680x |
| shared_bfull_fast_nozero | 65.203 | 59.595 | 63.615 | 65.537 | 2.330x | 1.593x |
| shared_bfull_accurate | 72.138 | 68.341 | 71.867 | 72.960 | 2.106x | 1.440x |
| shared_bfull_fast | 72.834 | 68.302 | 71.978 | 75.295 | 2.086x | 1.426x |
| mkl_bf16_inputs | 103.621 | 101.445 | 103.100 | 104.020 | 1.466x | 1.002x |
| mkl_fp32 | 103.849 | 103.346 | 103.686 | 103.978 | 1.463x | 1.000x |
| replay_bfull_fast_nozero | 124.041 | 120.354 | 121.657 | 124.433 | 1.225x | 0.837x |
| replay_bfull_fast | 129.343 | 127.265 | 127.479 | 131.408 | 1.175x | 0.803x |
| original_nozero | 146.007 | 130.721 | 133.284 | 148.845 | 1.041x | 0.711x |
| original | 151.927 | 148.046 | 148.483 | 156.382 | 1.000x | 0.684x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b32_fast_nozero | 128.310 | 126.092 | 127.717 | 129.287 | 2.357x | 1.633x |
| shared_bfull_fast_nozero | 131.147 | 127.136 | 130.255 | 131.201 | 2.306x | 1.598x |
| shared_bfull_accurate_nozero | 133.472 | 131.460 | 131.796 | 134.200 | 2.266x | 1.570x |
| shared_bfull_fast | 146.966 | 143.408 | 145.797 | 149.960 | 2.058x | 1.426x |
| shared_bfull_accurate | 149.873 | 144.396 | 149.710 | 150.431 | 2.018x | 1.398x |
| mkl_bf16_inputs | 207.735 | 206.654 | 207.587 | 208.104 | 1.456x | 1.009x |
| mkl_fp32 | 209.554 | 208.267 | 209.015 | 209.776 | 1.443x | 1.000x |
| replay_bfull_fast_nozero | 238.797 | 232.973 | 238.371 | 240.800 | 1.267x | 0.878x |
| replay_bfull_fast | 259.973 | 255.917 | 258.936 | 261.148 | 1.164x | 0.806x |
| original_nozero | 276.087 | 275.750 | 275.854 | 279.663 | 1.096x | 0.759x |
| original | 302.490 | 288.281 | 299.150 | 308.469 | 1.000x | 0.693x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.076e-03 | 1.111e+01 | 3.980e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.053e-03 | 1.111e+01 | 3.972e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 1.602e-06 | 2.124e-02 | 7.606e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.053e-03 | 1.111e+01 | 3.972e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_fp32 | 7.856e-03 | 1.662e+01 | 5.941e-03 | 1 |
| shared_bfull_accurate | kernel_full | mkl_bf16_inputs | 8.823e-06 | 2.722e-02 | 9.747e-06 | 1 |
| shared_bfull_accurate | kernel_full | mkl_fp32 | 5.058e-03 | 1.113e+01 | 3.978e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_fp32 | 7.856e-03 | 1.662e+01 | 5.941e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 1.602e-06 | 2.124e-02 | 7.606e-06 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.053e-03 | 1.111e+01 | 3.972e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.856e-03 | 1.662e+01 | 5.941e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 8.823e-06 | 2.722e-02 | 9.747e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.058e-03 | 1.113e+01 | 3.978e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.231e-03 | 8.240e+00 | 2.951e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 7.805e-03 | 1.690e+01 | 6.040e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.856e-03 | 1.662e+01 | 5.941e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
| shared_bfull_accurate | instrumented_kernel | mkl_bf16_inputs | 8.823e-06 | 2.722e-02 | 9.747e-06 | 1 |
| replay_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 8.823e-06 | 2.722e-02 | 9.747e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.231e-03 | 8.240e+00 | 2.951e-03 | 1 |
| replay_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.313e-03 | 9.608e+00 | 3.440e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 9.375e-03 | 2.940e+05 | 9.142e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 9.290e-03 | 2.940e+05 | 9.059e-03 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 3.425e-06 | 2.860e+02 | 8.894e-06 | 1 |
| original | two_layer_e2e | mkl_fp32 | 9.290e-03 | 2.940e+05 | 9.060e-03 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 1.943e+05 | 6.043e-03 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_fp32 | 1.486e-02 | 4.803e+05 | 1.480e-02 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_bf16_inputs | 1.625e-05 | 5.360e+02 | 1.667e-05 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_fp32 | 9.304e-03 | 2.944e+05 | 9.072e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 1.943e+05 | 6.043e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_fp32 | 1.486e-02 | 4.803e+05 | 1.480e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 3.425e-06 | 2.860e+02 | 8.894e-06 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 9.290e-03 | 2.940e+05 | 9.060e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 1.943e+05 | 6.043e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.486e-02 | 4.803e+05 | 1.480e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.625e-05 | 5.360e+02 | 1.667e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 9.304e-03 | 2.944e+05 | 9.072e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.590e-03 | 1.789e+05 | 5.563e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.474e-02 | 4.728e+05 | 1.457e-02 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 1.943e+05 | 6.043e-03 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.486e-02 | 4.803e+05 | 1.480e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 9.4860 | 58 | 175.820 |
| original | row_schedule | 0.0236 | 58 | 175.820 |
| original | partial_zero | 0.0217 | 58 | 175.820 |
| original | csr_prefetch | 11.9152 | 58 | 175.820 |
| original | feature_gather_decode | 4.1406 | 58 | 175.820 |
| original | fp32_neighbor_reduction | 0.0000 | 58 | 175.820 |
| original | partial_bf16_conversion | 0.0000 | 58 | 175.820 |
| original | tile_load | 23.7777 | 58 | 175.820 |
| original | tile_compute | 30.1249 | 58 | 175.820 |
| original | tile_store | 0.0138 | 58 | 175.820 |
| original | output_scatter | 0.0324 | 58 | 175.820 |
| original | thread_amx_setup | 0.0219 | 58 | 175.820 |
| shared_bfull_fast | output_zero | 9.9518 | 58 | 76.500 |
| shared_bfull_fast | row_schedule | 0.0155 | 58 | 76.500 |
| shared_bfull_fast | partial_zero | 0.0219 | 58 | 76.500 |
| shared_bfull_fast | csr_prefetch | 13.9761 | 58 | 76.500 |
| shared_bfull_fast | feature_gather_decode | 15.9459 | 58 | 76.500 |
| shared_bfull_fast | fp32_neighbor_reduction | 12.9519 | 58 | 76.500 |
| shared_bfull_fast | partial_bf16_conversion | 0.0389 | 58 | 76.500 |
| shared_bfull_fast | tile_load | 0.0987 | 58 | 76.500 |
| shared_bfull_fast | tile_compute | 0.2093 | 58 | 76.500 |
| shared_bfull_fast | tile_store | 0.0057 | 58 | 76.500 |
| shared_bfull_fast | output_scatter | 0.0165 | 58 | 76.500 |
| shared_bfull_fast | thread_amx_setup | 0.0153 | 58 | 76.500 |
| shared_bfull_accurate | output_zero | 9.5448 | 58 | 79.027 |
| shared_bfull_accurate | row_schedule | 0.0162 | 58 | 79.027 |
| shared_bfull_accurate | partial_zero | 0.0248 | 58 | 79.027 |
| shared_bfull_accurate | csr_prefetch | 13.9263 | 58 | 79.027 |
| shared_bfull_accurate | feature_gather_decode | 16.1536 | 58 | 79.027 |
| shared_bfull_accurate | fp32_neighbor_reduction | 13.1600 | 58 | 79.027 |
| shared_bfull_accurate | partial_bf16_conversion | 0.0293 | 58 | 79.027 |
| shared_bfull_accurate | tile_load | 0.1800 | 58 | 79.027 |
| shared_bfull_accurate | tile_compute | 0.2933 | 58 | 79.027 |
| shared_bfull_accurate | tile_store | 0.0076 | 58 | 79.027 |
| shared_bfull_accurate | output_scatter | 0.0179 | 58 | 79.027 |
| shared_bfull_accurate | thread_amx_setup | 0.0172 | 58 | 79.027 |
| replay_bfull_fast | output_zero | 9.6929 | 58 | 137.068 |
| replay_bfull_fast | row_schedule | 0.0167 | 58 | 137.068 |
| replay_bfull_fast | partial_zero | 0.0260 | 58 | 137.068 |
| replay_bfull_fast | csr_prefetch | 27.7140 | 58 | 137.068 |
| replay_bfull_fast | feature_gather_decode | 32.1956 | 58 | 137.068 |
| replay_bfull_fast | fp32_neighbor_reduction | 26.0437 | 58 | 137.068 |
| replay_bfull_fast | partial_bf16_conversion | 0.0720 | 58 | 137.068 |
| replay_bfull_fast | tile_load | 0.0947 | 58 | 137.068 |
| replay_bfull_fast | tile_compute | 0.2170 | 58 | 137.068 |
| replay_bfull_fast | tile_store | 0.0196 | 58 | 137.068 |
| replay_bfull_fast | output_scatter | 0.0463 | 58 | 137.068 |
| replay_bfull_fast | thread_amx_setup | 0.0191 | 58 | 137.068 |

## regular-q16

N=262144, E=4194304, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 2.898 | 2.860 | 2.889 | 2.910 | 6.106x | 3.567x |
| shared_bfull_accurate_nozero | 2.914 | 2.877 | 2.903 | 2.921 | 6.072x | 3.548x |
| shared_b32_fast_nozero | 2.941 | 2.882 | 2.938 | 2.946 | 6.016x | 3.515x |
| replay_bfull_fast_nozero | 3.677 | 3.644 | 3.673 | 3.679 | 4.812x | 2.811x |
| original_nozero | 4.536 | 4.529 | 4.531 | 4.537 | 3.901x | 2.279x |
| mkl_bf16_inputs | 9.282 | 9.233 | 9.277 | 9.331 | 1.906x | 1.114x |
| mkl_fp32 | 10.338 | 10.229 | 10.240 | 10.671 | 1.712x | 1.000x |
| shared_bfull_fast | 13.852 | 13.650 | 13.683 | 13.884 | 1.277x | 0.746x |
| shared_bfull_accurate | 13.985 | 13.858 | 13.962 | 13.998 | 1.265x | 0.739x |
| replay_bfull_fast | 14.639 | 14.601 | 14.629 | 14.651 | 1.209x | 0.706x |
| original | 17.694 | 17.530 | 17.609 | 17.815 | 1.000x | 0.584x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b32_fast_nozero | 10.463 | 9.928 | 10.059 | 10.605 | 3.195x | 1.944x |
| shared_bfull_fast_nozero | 10.470 | 10.303 | 10.366 | 10.641 | 3.193x | 1.943x |
| shared_bfull_accurate_nozero | 10.544 | 10.169 | 10.488 | 10.651 | 3.171x | 1.929x |
| replay_bfull_fast_nozero | 12.566 | 12.138 | 12.507 | 12.573 | 2.660x | 1.619x |
| original_nozero | 12.634 | 12.466 | 12.468 | 12.674 | 2.646x | 1.610x |
| mkl_bf16_inputs | 20.187 | 20.028 | 20.128 | 20.831 | 1.656x | 1.008x |
| mkl_fp32 | 20.344 | 19.905 | 19.972 | 20.432 | 1.643x | 1.000x |
| shared_bfull_fast | 32.516 | 32.000 | 32.046 | 32.752 | 1.028x | 0.626x |
| shared_bfull_accurate | 32.540 | 32.357 | 32.498 | 32.555 | 1.027x | 0.625x |
| replay_bfull_fast | 33.414 | 33.361 | 33.363 | 33.456 | 1.001x | 0.609x |
| original | 33.432 | 33.387 | 33.415 | 33.981 | 1.000x | 0.609x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.310e-03 | 4.422e-01 | 4.846e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 2.315e-07 | 4.959e-05 | 5.435e-07 | 1 |
| original | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_fp32 | 8.069e-03 | 6.918e-01 | 7.546e-03 | 1 |
| shared_bfull_accurate | kernel_full | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| shared_bfull_accurate | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_fp32 | 8.069e-03 | 6.918e-01 | 7.546e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 2.315e-07 | 4.959e-05 | 5.435e-07 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.069e-03 | 6.918e-01 | 7.546e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.285e-03 | 4.422e-01 | 4.823e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 8.069e-03 | 6.918e-01 | 7.546e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.069e-03 | 6.918e-01 | 7.546e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_bfull_accurate | instrumented_kernel | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| replay_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.919e-06 | 3.328e-04 | 3.648e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| replay_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.273e-03 | 2.881e-01 | 3.158e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.058e-02 | 3.081e+01 | 1.085e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 1.291e-05 | 3.492e-01 | 1.230e-04 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.915e-03 | 1.857e+01 | 6.539e-03 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_fp32 | 1.597e-02 | 4.830e+01 | 1.683e-02 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_bf16_inputs | 3.003e-05 | 4.364e-01 | 1.537e-04 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.915e-03 | 1.857e+01 | 6.539e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_fp32 | 1.597e-02 | 4.830e+01 | 1.683e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 1.291e-05 | 3.492e-01 | 1.230e-04 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.915e-03 | 1.857e+01 | 6.539e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.597e-02 | 4.830e+01 | 1.683e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.003e-05 | 4.364e-01 | 1.537e-04 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.047e-02 | 3.081e+01 | 1.074e-02 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.915e-03 | 1.857e+01 | 6.539e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.597e-02 | 4.830e+01 | 1.683e-02 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.915e-03 | 1.857e+01 | 6.539e-03 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.597e-02 | 4.830e+01 | 1.683e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 10.6699 | 65 | 15.370 |
| original | row_schedule | 0.0169 | 65 | 15.370 |
| original | partial_zero | 0.0339 | 65 | 15.370 |
| original | csr_prefetch | 0.2246 | 65 | 15.370 |
| original | feature_gather_decode | 0.1221 | 65 | 15.370 |
| original | fp32_neighbor_reduction | 0.0000 | 65 | 15.370 |
| original | partial_bf16_conversion | 0.0000 | 65 | 15.370 |
| original | tile_load | 0.5789 | 65 | 15.370 |
| original | tile_compute | 0.9637 | 65 | 15.370 |
| original | tile_store | 0.0238 | 65 | 15.370 |
| original | output_scatter | 0.0310 | 65 | 15.370 |
| original | thread_amx_setup | 0.0191 | 65 | 15.370 |
| shared_bfull_fast | output_zero | 10.0400 | 65 | 14.171 |
| shared_bfull_fast | row_schedule | 0.0234 | 65 | 14.171 |
| shared_bfull_fast | partial_zero | 0.0334 | 65 | 14.171 |
| shared_bfull_fast | csr_prefetch | 0.4864 | 65 | 14.171 |
| shared_bfull_fast | feature_gather_decode | 0.6888 | 65 | 14.171 |
| shared_bfull_fast | fp32_neighbor_reduction | 0.4671 | 65 | 14.171 |
| shared_bfull_fast | partial_bf16_conversion | 0.0427 | 65 | 14.171 |
| shared_bfull_fast | tile_load | 0.1144 | 65 | 14.171 |
| shared_bfull_fast | tile_compute | 0.3173 | 65 | 14.171 |
| shared_bfull_fast | tile_store | 0.0179 | 65 | 14.171 |
| shared_bfull_fast | output_scatter | 0.0174 | 65 | 14.171 |
| shared_bfull_fast | thread_amx_setup | 0.0181 | 65 | 14.171 |
| shared_bfull_accurate | output_zero | 10.1190 | 65 | 14.178 |
| shared_bfull_accurate | row_schedule | 0.0193 | 65 | 14.178 |
| shared_bfull_accurate | partial_zero | 0.0546 | 65 | 14.178 |
| shared_bfull_accurate | csr_prefetch | 0.4778 | 65 | 14.178 |
| shared_bfull_accurate | feature_gather_decode | 0.7062 | 65 | 14.178 |
| shared_bfull_accurate | fp32_neighbor_reduction | 0.5131 | 65 | 14.178 |
| shared_bfull_accurate | partial_bf16_conversion | 0.0367 | 65 | 14.178 |
| shared_bfull_accurate | tile_load | 0.2058 | 65 | 14.178 |
| shared_bfull_accurate | tile_compute | 0.3951 | 65 | 14.178 |
| shared_bfull_accurate | tile_store | 0.0145 | 65 | 14.178 |
| shared_bfull_accurate | output_scatter | 0.0315 | 65 | 14.178 |
| shared_bfull_accurate | thread_amx_setup | 0.0181 | 65 | 14.178 |
| replay_bfull_fast | output_zero | 10.1299 | 65 | 15.262 |
| replay_bfull_fast | row_schedule | 0.0219 | 65 | 15.262 |
| replay_bfull_fast | partial_zero | 0.0517 | 65 | 15.262 |
| replay_bfull_fast | csr_prefetch | 0.9422 | 65 | 15.262 |
| replay_bfull_fast | feature_gather_decode | 1.2519 | 65 | 15.262 |
| replay_bfull_fast | fp32_neighbor_reduction | 0.9778 | 65 | 15.262 |
| replay_bfull_fast | partial_bf16_conversion | 9.6984 | 65 | 15.262 |
| replay_bfull_fast | tile_load | 0.1037 | 65 | 15.262 |
| replay_bfull_fast | tile_compute | 0.3207 | 65 | 15.262 |
| replay_bfull_fast | tile_store | 0.0196 | 65 | 15.262 |
| replay_bfull_fast | output_scatter | 0.0391 | 65 | 15.262 |
| replay_bfull_fast | thread_amx_setup | 0.0114 | 65 | 15.262 |

## regular-q256

N=262144, E=67108864, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 36.444 | 36.414 | 36.438 | 36.480 | 2.012x | 2.516x |
| shared_bfull_accurate_nozero | 36.756 | 36.649 | 36.739 | 37.225 | 1.995x | 2.494x |
| shared_b32_fast_nozero | 37.075 | 36.636 | 37.052 | 37.384 | 1.978x | 2.473x |
| shared_bfull_accurate | 44.993 | 44.485 | 44.846 | 45.096 | 1.630x | 2.038x |
| shared_bfull_fast | 46.247 | 45.111 | 45.145 | 46.304 | 1.585x | 1.982x |
| replay_bfull_fast_nozero | 53.372 | 52.101 | 52.608 | 53.396 | 1.374x | 1.718x |
| replay_bfull_fast | 59.598 | 59.527 | 59.580 | 60.155 | 1.230x | 1.538x |
| original_nozero | 60.486 | 60.379 | 60.481 | 60.521 | 1.212x | 1.516x |
| original | 73.323 | 73.270 | 73.290 | 74.049 | 1.000x | 1.250x |
| mkl_bf16_inputs | 88.806 | 88.544 | 88.618 | 89.115 | 0.826x | 1.032x |
| mkl_fp32 | 91.684 | 91.431 | 91.562 | 91.823 | 0.800x | 1.000x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_accurate_nozero | 75.676 | 75.496 | 75.500 | 75.800 | 1.921x | 2.385x |
| shared_b32_fast_nozero | 76.478 | 75.790 | 76.307 | 76.801 | 1.901x | 2.360x |
| shared_bfull_fast_nozero | 77.259 | 76.841 | 77.099 | 77.364 | 1.882x | 2.336x |
| shared_bfull_fast | 92.723 | 92.388 | 92.413 | 92.869 | 1.568x | 1.947x |
| shared_bfull_accurate | 93.922 | 92.699 | 93.666 | 94.048 | 1.548x | 1.922x |
| replay_bfull_fast_nozero | 108.898 | 107.089 | 108.452 | 108.989 | 1.335x | 1.658x |
| original_nozero | 124.547 | 124.480 | 124.537 | 124.654 | 1.167x | 1.449x |
| replay_bfull_fast | 125.276 | 124.939 | 125.231 | 125.366 | 1.161x | 1.441x |
| original | 145.403 | 144.716 | 145.377 | 146.231 | 1.000x | 1.241x |
| mkl_bf16_inputs | 177.951 | 177.648 | 177.760 | 178.119 | 0.817x | 1.014x |
| mkl_fp32 | 180.510 | 180.349 | 180.471 | 180.595 | 0.806x | 1.000x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.274e-03 | 1.855e+00 | 5.164e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.249e-03 | 1.855e+00 | 5.138e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 6.099e-07 | 5.798e-04 | 1.614e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.249e-03 | 1.855e+00 | 5.137e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_bf16_inputs | 3.314e-03 | 1.190e+00 | 3.312e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_fp32 | 8.076e-03 | 2.869e+00 | 7.946e-03 | 1 |
| shared_bfull_accurate | kernel_full | mkl_bf16_inputs | 7.976e-06 | 3.235e-03 | 9.004e-06 | 1 |
| shared_bfull_accurate | kernel_full | mkl_fp32 | 5.254e-03 | 1.858e+00 | 5.145e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_bf16_inputs | 3.314e-03 | 1.190e+00 | 3.312e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_fp32 | 8.076e-03 | 2.869e+00 | 7.946e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 6.099e-07 | 5.798e-04 | 1.614e-06 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.249e-03 | 1.855e+00 | 5.137e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.314e-03 | 1.190e+00 | 3.312e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.076e-03 | 2.869e+00 | 7.946e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 7.976e-06 | 3.235e-03 | 9.004e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.254e-03 | 1.858e+00 | 5.145e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.279e-03 | 1.110e+00 | 3.089e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 8.045e-03 | 2.919e+00 | 8.083e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.314e-03 | 1.190e+00 | 3.312e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.076e-03 | 2.869e+00 | 7.946e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.314e-03 | 1.190e+00 | 3.312e-03 | 1 |
| shared_bfull_accurate | instrumented_kernel | mkl_bf16_inputs | 7.976e-06 | 3.235e-03 | 9.004e-06 | 1 |
| replay_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.314e-03 | 1.190e+00 | 3.312e-03 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.314e-03 | 1.190e+00 | 3.312e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 7.976e-06 | 3.235e-03 | 9.004e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.279e-03 | 1.110e+00 | 3.089e-03 | 1 |
| replay_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.314e-03 | 1.190e+00 | 3.312e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.047e-02 | 1.487e+03 | 1.154e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.036e-02 | 1.487e+03 | 1.141e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 5.376e-06 | 2.566e+00 | 1.991e-05 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.036e-02 | 1.487e+03 | 1.141e-02 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.749e-03 | 9.510e+02 | 7.379e-03 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_fp32 | 1.584e-02 | 2.304e+03 | 1.767e-02 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_bf16_inputs | 2.362e-05 | 6.715e+00 | 5.210e-05 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_fp32 | 1.037e-02 | 1.488e+03 | 1.142e-02 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.749e-03 | 9.510e+02 | 7.379e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_fp32 | 1.584e-02 | 2.304e+03 | 1.767e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 5.376e-06 | 2.566e+00 | 1.991e-05 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.036e-02 | 1.487e+03 | 1.141e-02 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.749e-03 | 9.510e+02 | 7.379e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.584e-02 | 2.304e+03 | 1.767e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.362e-05 | 6.715e+00 | 5.210e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.037e-02 | 1.488e+03 | 1.142e-02 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.607e-03 | 8.013e+02 | 6.217e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.583e-02 | 2.225e+03 | 1.707e-02 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.749e-03 | 9.510e+02 | 7.379e-03 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.584e-02 | 2.304e+03 | 1.767e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 11.1511 | 65 | 72.690 |
| original | row_schedule | 0.0205 | 65 | 72.690 |
| original | partial_zero | 0.0367 | 65 | 72.690 |
| original | csr_prefetch | 3.6502 | 65 | 72.690 |
| original | feature_gather_decode | 1.7605 | 65 | 72.690 |
| original | fp32_neighbor_reduction | 0.0000 | 65 | 72.690 |
| original | partial_bf16_conversion | 0.0000 | 65 | 72.690 |
| original | tile_load | 8.6763 | 65 | 72.690 |
| original | tile_compute | 8.9860 | 65 | 72.690 |
| original | tile_store | 0.0136 | 65 | 72.690 |
| original | output_scatter | 0.0346 | 65 | 72.690 |
| original | thread_amx_setup | 0.0257 | 65 | 72.690 |
| shared_bfull_fast | output_zero | 11.1790 | 65 | 47.244 |
| shared_bfull_fast | row_schedule | 0.0231 | 65 | 47.244 |
| shared_bfull_fast | partial_zero | 0.0393 | 65 | 47.244 |
| shared_bfull_fast | csr_prefetch | 7.2329 | 65 | 47.244 |
| shared_bfull_fast | feature_gather_decode | 9.0590 | 65 | 47.244 |
| shared_bfull_fast | fp32_neighbor_reduction | 6.9308 | 65 | 47.244 |
| shared_bfull_fast | partial_bf16_conversion | 0.0541 | 65 | 47.244 |
| shared_bfull_fast | tile_load | 0.1581 | 65 | 47.244 |
| shared_bfull_fast | tile_compute | 0.4056 | 65 | 47.244 |
| shared_bfull_fast | tile_store | 0.0162 | 65 | 47.244 |
| shared_bfull_fast | output_scatter | 0.0205 | 65 | 47.244 |
| shared_bfull_fast | thread_amx_setup | 0.0200 | 65 | 47.244 |
| shared_bfull_accurate | output_zero | 11.0929 | 65 | 46.506 |
| shared_bfull_accurate | row_schedule | 0.0327 | 65 | 46.506 |
| shared_bfull_accurate | partial_zero | 0.0613 | 65 | 46.506 |
| shared_bfull_accurate | csr_prefetch | 7.2482 | 65 | 46.506 |
| shared_bfull_accurate | feature_gather_decode | 8.9693 | 65 | 46.506 |
| shared_bfull_accurate | fp32_neighbor_reduction | 6.7933 | 65 | 46.506 |
| shared_bfull_accurate | partial_bf16_conversion | 0.0460 | 65 | 46.506 |
| shared_bfull_accurate | tile_load | 0.2809 | 65 | 46.506 |
| shared_bfull_accurate | tile_compute | 0.4761 | 65 | 46.506 |
| shared_bfull_accurate | tile_store | 0.0088 | 65 | 46.506 |
| shared_bfull_accurate | output_scatter | 0.0286 | 65 | 46.506 |
| shared_bfull_accurate | thread_amx_setup | 0.0134 | 65 | 46.506 |
| replay_bfull_fast | output_zero | 10.9820 | 65 | 62.438 |
| replay_bfull_fast | row_schedule | 0.0169 | 65 | 62.438 |
| replay_bfull_fast | partial_zero | 0.0448 | 65 | 62.438 |
| replay_bfull_fast | csr_prefetch | 14.4470 | 65 | 62.438 |
| replay_bfull_fast | feature_gather_decode | 17.2715 | 65 | 62.438 |
| replay_bfull_fast | fp32_neighbor_reduction | 13.6399 | 65 | 62.438 |
| replay_bfull_fast | partial_bf16_conversion | 7.1826 | 65 | 62.438 |
| replay_bfull_fast | tile_load | 0.1190 | 65 | 62.438 |
| replay_bfull_fast | tile_compute | 0.3459 | 65 | 62.438 |
| replay_bfull_fast | tile_store | 0.0293 | 65 | 62.438 |
| replay_bfull_fast | output_scatter | 0.0353 | 65 | 62.438 |
| replay_bfull_fast | thread_amx_setup | 0.0095 | 65 | 62.438 |

## soc-Pokec

N=1632803, E=30622564, threads=32, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 36.631 | 36.528 | 36.566 | 36.692 | 3.067x | 1.740x |
| shared_b32_fast_nozero | 37.314 | 36.899 | 36.982 | 37.594 | 3.011x | 1.708x |
| shared_bfull_accurate_nozero | 38.113 | 37.639 | 37.653 | 38.456 | 2.948x | 1.673x |
| original_nozero | 41.378 | 41.270 | 41.356 | 42.575 | 2.716x | 1.541x |
| replay_bfull_fast_nozero | 46.854 | 46.026 | 46.427 | 46.983 | 2.398x | 1.361x |
| mkl_bf16_inputs | 63.460 | 63.148 | 63.160 | 63.490 | 1.771x | 1.004x |
| mkl_fp32 | 63.745 | 63.654 | 63.657 | 63.769 | 1.763x | 1.000x |
| shared_bfull_fast | 103.180 | 101.737 | 102.567 | 103.193 | 1.089x | 0.618x |
| shared_bfull_accurate | 104.387 | 103.179 | 103.886 | 104.486 | 1.076x | 0.611x |
| replay_bfull_fast | 112.165 | 111.173 | 111.279 | 112.244 | 1.002x | 0.568x |
| original | 112.365 | 107.197 | 109.435 | 115.074 | 1.000x | 0.567x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 80.527 | 79.719 | 80.451 | 80.613 | 2.800x | 1.624x |
| shared_b32_fast_nozero | 83.355 | 81.552 | 81.714 | 84.333 | 2.705x | 1.569x |
| shared_bfull_accurate_nozero | 84.468 | 83.976 | 84.236 | 84.479 | 2.669x | 1.549x |
| original_nozero | 91.639 | 85.407 | 89.668 | 91.813 | 2.460x | 1.427x |
| replay_bfull_fast_nozero | 101.107 | 99.247 | 101.082 | 101.558 | 2.230x | 1.294x |
| mkl_fp32 | 130.801 | 130.508 | 130.605 | 131.373 | 1.724x | 1.000x |
| mkl_bf16_inputs | 140.481 | 135.174 | 140.189 | 140.504 | 1.605x | 0.931x |
| shared_bfull_fast | 212.832 | 211.126 | 212.408 | 213.316 | 1.059x | 0.615x |
| shared_bfull_accurate | 215.330 | 214.651 | 214.772 | 216.664 | 1.047x | 0.607x |
| original | 225.445 | 224.710 | 225.158 | 226.965 | 1.000x | 0.580x |
| replay_bfull_fast | 232.488 | 230.600 | 231.875 | 232.559 | 0.970x | 0.563x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.319e-03 | 5.965e+00 | 4.261e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.293e-03 | 5.965e+00 | 4.244e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 3.741e-07 | 1.160e-02 | 8.283e-06 | 1 |
| original | kernel_full | mkl_fp32 | 5.293e-03 | 5.968e+00 | 4.246e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_fp32 | 8.064e-03 | 9.351e+00 | 6.653e-03 | 1 |
| shared_bfull_accurate | kernel_full | mkl_bf16_inputs | 4.976e-06 | 1.331e-02 | 9.504e-06 | 1 |
| shared_bfull_accurate | kernel_full | mkl_fp32 | 5.295e-03 | 5.975e+00 | 4.251e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_fp32 | 8.064e-03 | 9.351e+00 | 6.653e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 3.741e-07 | 1.160e-02 | 8.283e-06 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.293e-03 | 5.968e+00 | 4.246e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.064e-03 | 9.351e+00 | 6.653e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.976e-06 | 1.331e-02 | 9.504e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.295e-03 | 5.975e+00 | 4.251e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.247e-03 | 4.145e+00 | 2.961e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 8.046e-03 | 9.731e+00 | 6.923e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.064e-03 | 9.351e+00 | 6.653e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
| shared_bfull_accurate | instrumented_kernel | mkl_bf16_inputs | 4.976e-06 | 1.331e-02 | 9.504e-06 | 1 |
| replay_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.976e-06 | 1.331e-02 | 9.504e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.247e-03 | 4.145e+00 | 2.961e-03 | 1 |
| replay_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.265e-03 | 3.930e+00 | 2.807e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.015e-02 | 1.593e+04 | 9.942e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.005e-02 | 1.593e+04 | 9.844e-03 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 7.647e-06 | 7.250e+00 | 4.525e-06 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.005e-02 | 1.593e+04 | 9.847e-03 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 9.892e+03 | 6.175e-03 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_fp32 | 1.561e-02 | 2.582e+04 | 1.596e-02 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_bf16_inputs | 3.211e-05 | 1.731e+01 | 1.081e-05 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_fp32 | 1.006e-02 | 1.594e+04 | 9.854e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 9.892e+03 | 6.175e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_fp32 | 1.561e-02 | 2.582e+04 | 1.596e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 7.647e-06 | 7.250e+00 | 4.525e-06 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.005e-02 | 1.593e+04 | 9.847e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 9.892e+03 | 6.175e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.561e-02 | 2.582e+04 | 1.596e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.211e-05 | 1.731e+01 | 1.081e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.006e-02 | 1.594e+04 | 9.854e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.669e-03 | 8.903e+03 | 5.557e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.553e-02 | 2.483e+04 | 1.535e-02 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.837e-03 | 9.892e+03 | 6.175e-03 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.561e-02 | 2.582e+04 | 1.596e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 65.3729 | 400 | 122.428 |
| original | row_schedule | 0.1261 | 400 | 122.428 |
| original | partial_zero | 0.1259 | 400 | 122.428 |
| original | csr_prefetch | 2.7816 | 400 | 122.428 |
| original | feature_gather_decode | 1.6105 | 400 | 122.428 |
| original | fp32_neighbor_reduction | 0.0000 | 400 | 122.428 |
| original | partial_bf16_conversion | 0.0000 | 400 | 122.428 |
| original | tile_load | 8.5452 | 400 | 122.428 |
| original | tile_compute | 10.2453 | 400 | 122.428 |
| original | tile_store | 0.1588 | 400 | 122.428 |
| original | output_scatter | 0.1454 | 400 | 122.428 |
| original | thread_amx_setup | 0.0172 | 400 | 122.428 |
| shared_bfull_fast | output_zero | 65.6040 | 400 | 107.013 |
| shared_bfull_fast | row_schedule | 0.1342 | 400 | 107.013 |
| shared_bfull_fast | partial_zero | 0.0687 | 400 | 107.013 |
| shared_bfull_fast | csr_prefetch | 4.6189 | 400 | 107.013 |
| shared_bfull_fast | feature_gather_decode | 5.6810 | 400 | 107.013 |
| shared_bfull_fast | fp32_neighbor_reduction | 3.9349 | 400 | 107.013 |
| shared_bfull_fast | partial_bf16_conversion | 0.0815 | 400 | 107.013 |
| shared_bfull_fast | tile_load | 0.4883 | 400 | 107.013 |
| shared_bfull_fast | tile_compute | 1.1175 | 400 | 107.013 |
| shared_bfull_fast | tile_store | 0.0832 | 400 | 107.013 |
| shared_bfull_fast | output_scatter | 0.0730 | 400 | 107.013 |
| shared_bfull_fast | thread_amx_setup | 0.0153 | 400 | 107.013 |
| shared_bfull_accurate | output_zero | 65.6180 | 400 | 105.565 |
| shared_bfull_accurate | row_schedule | 0.1168 | 400 | 105.565 |
| shared_bfull_accurate | partial_zero | 0.1006 | 400 | 105.565 |
| shared_bfull_accurate | csr_prefetch | 4.7340 | 400 | 105.565 |
| shared_bfull_accurate | feature_gather_decode | 5.7199 | 400 | 105.565 |
| shared_bfull_accurate | fp32_neighbor_reduction | 4.0076 | 400 | 105.565 |
| shared_bfull_accurate | partial_bf16_conversion | 0.1106 | 400 | 105.565 |
| shared_bfull_accurate | tile_load | 0.8714 | 400 | 105.565 |
| shared_bfull_accurate | tile_compute | 1.5585 | 400 | 105.565 |
| shared_bfull_accurate | tile_store | 0.0560 | 400 | 105.565 |
| shared_bfull_accurate | output_scatter | 0.0696 | 400 | 105.565 |
| shared_bfull_accurate | thread_amx_setup | 0.0212 | 400 | 105.565 |
| replay_bfull_fast | output_zero | 66.0429 | 400 | 122.055 |
| replay_bfull_fast | row_schedule | 0.1237 | 400 | 122.055 |
| replay_bfull_fast | partial_zero | 0.1357 | 400 | 122.055 |
| replay_bfull_fast | csr_prefetch | 8.8162 | 400 | 122.055 |
| replay_bfull_fast | feature_gather_decode | 10.4527 | 400 | 122.055 |
| replay_bfull_fast | fp32_neighbor_reduction | 8.4729 | 400 | 122.055 |
| replay_bfull_fast | partial_bf16_conversion | 0.2203 | 400 | 122.055 |
| replay_bfull_fast | tile_load | 0.3576 | 400 | 122.055 |
| replay_bfull_fast | tile_compute | 1.3361 | 400 | 122.055 |
| replay_bfull_fast | tile_store | 0.1402 | 400 | 122.055 |
| replay_bfull_fast | output_scatter | 0.1514 | 400 | 122.055 |
| replay_bfull_fast | thread_amx_setup | 0.0184 | 400 | 122.055 |

