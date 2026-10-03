# GCN-extra first inference experiments

Numerical sum-aggregation experiments with random H/W; no task-accuracy claim.
Uninstrumented medians; original source is unchanged. Speedups are original/new and FP32 MKL/new.
Profiles are sampled summed thread time and do not equal wall time. Physical AMX work is modeled, not a hardware counter.

## smoke

N=37, E=538, threads=8, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_fp32 | 0.009 | 0.009 | 0.009 | 0.010 | 5.203x | 1.000x |
| mkl_bf16_inputs | 0.010 | 0.009 | 0.009 | 0.010 | 5.137x | 0.988x |
| shared_bfull_fast_nozero | 0.015 | 0.014 | 0.015 | 0.015 | 3.262x | 0.627x |
| shared_bfull_fast | 0.017 | 0.014 | 0.015 | 0.018 | 2.957x | 0.568x |
| shared_bfull_accurate_nozero | 0.017 | 0.015 | 0.016 | 0.017 | 2.957x | 0.568x |
| shared_b32_fast_nozero | 0.018 | 0.018 | 0.018 | 0.018 | 2.722x | 0.523x |
| replay_bfull_fast_nozero | 0.018 | 0.018 | 0.018 | 0.018 | 2.722x | 0.523x |
| shared_bfull_accurate | 0.019 | 0.017 | 0.018 | 0.020 | 2.553x | 0.491x |
| replay_bfull_fast | 0.026 | 0.022 | 0.024 | 0.027 | 1.921x | 0.369x |
| original_nozero | 0.037 | 0.037 | 0.037 | 0.038 | 1.309x | 0.252x |
| original | 0.049 | 0.046 | 0.048 | 0.050 | 1.000x | 0.192x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_fp32 | 0.023 | 0.023 | 0.023 | 0.023 | 2.870x | 1.000x |
| mkl_bf16_inputs | 0.023 | 0.022 | 0.023 | 0.024 | 2.812x | 0.980x |
| shared_bfull_fast_nozero | 0.030 | 0.029 | 0.030 | 0.031 | 2.198x | 0.766x |
| shared_bfull_accurate_nozero | 0.032 | 0.030 | 0.031 | 0.033 | 2.059x | 0.717x |
| shared_bfull_fast | 0.035 | 0.034 | 0.034 | 0.035 | 1.891x | 0.659x |
| shared_b32_fast_nozero | 0.037 | 0.034 | 0.035 | 0.039 | 1.787x | 0.623x |
| shared_bfull_accurate | 0.040 | 0.040 | 0.040 | 0.040 | 1.649x | 0.574x |
| replay_bfull_fast_nozero | 0.041 | 0.041 | 0.041 | 0.041 | 1.610x | 0.561x |
| replay_bfull_fast | 0.046 | 0.046 | 0.046 | 0.047 | 1.424x | 0.496x |
| original | 0.066 | 0.063 | 0.064 | 0.068 | 1.000x | 0.348x |
| original_nozero | 0.073 | 0.068 | 0.070 | 0.075 | 0.905x | 0.315x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.672e-03 | 3.386e-01 | 5.527e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.497e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 2.297e-07 | 2.289e-05 | 3.736e-07 | 1 |
| original | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.496e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_fp32 | 8.453e-03 | 5.256e-01 | 8.534e-03 | 1 |
| shared_bfull_accurate | kernel_full | mkl_bf16_inputs | 4.007e-06 | 2.871e-04 | 4.686e-06 | 1 |
| shared_bfull_accurate | kernel_full | mkl_fp32 | 5.643e-03 | 3.386e-01 | 5.497e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_fp32 | 8.453e-03 | 5.256e-01 | 8.534e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 2.297e-07 | 2.289e-05 | 3.736e-07 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.496e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.453e-03 | 5.256e-01 | 8.534e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.007e-06 | 2.871e-04 | 4.686e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.643e-03 | 3.386e-01 | 5.497e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.313e-03 | 2.092e-01 | 3.415e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 8.417e-03 | 5.348e-01 | 8.682e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.453e-03 | 5.256e-01 | 8.534e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| shared_bfull_accurate | instrumented_kernel | mkl_bf16_inputs | 4.007e-06 | 2.871e-04 | 4.686e-06 | 1 |
| replay_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.007e-06 | 2.871e-04 | 4.686e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.313e-03 | 2.092e-01 | 3.415e-03 | 1 |
| replay_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.095e-02 | 3.557e+01 | 1.028e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 2.519e-07 | 1.709e-03 | 4.939e-07 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.281e-03 | 1.925e+01 | 5.564e-03 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_fp32 | 1.578e-02 | 5.186e+01 | 1.484e-02 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_bf16_inputs | 6.184e-05 | 1.443e-01 | 4.170e-05 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.567e+01 | 1.021e-02 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.281e-03 | 1.925e+01 | 5.564e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_fp32 | 1.578e-02 | 5.186e+01 | 1.484e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 2.519e-07 | 1.709e-03 | 4.939e-07 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.281e-03 | 1.925e+01 | 5.564e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.578e-02 | 5.186e+01 | 1.484e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 6.184e-05 | 1.443e-01 | 4.170e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.567e+01 | 1.021e-02 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.423e-03 | 2.186e+01 | 6.316e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.594e-02 | 5.530e+01 | 1.582e-02 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.281e-03 | 1.925e+01 | 5.564e-03 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.578e-02 | 5.186e+01 | 1.484e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 0.0019 | 2 | 0.124 |
| original | row_schedule | 0.0000 | 2 | 0.124 |
| original | partial_zero | 0.0000 | 2 | 0.124 |
| original | csr_prefetch | 0.0031 | 2 | 0.124 |
| original | feature_gather_decode | 0.0019 | 2 | 0.124 |
| original | fp32_neighbor_reduction | 0.0000 | 2 | 0.124 |
| original | partial_bf16_conversion | 0.0000 | 2 | 0.124 |
| original | tile_load | 0.0262 | 2 | 0.124 |
| original | tile_compute | 0.0253 | 2 | 0.124 |
| original | tile_store | 0.0000 | 2 | 0.124 |
| original | output_scatter | 0.0000 | 2 | 0.124 |
| original | thread_amx_setup | 0.0010 | 2 | 0.124 |
| shared_bfull_fast | output_zero | 0.0010 | 2 | 0.070 |
| shared_bfull_fast | row_schedule | 0.0000 | 2 | 0.070 |
| shared_bfull_fast | partial_zero | 0.0010 | 2 | 0.070 |
| shared_bfull_fast | csr_prefetch | 0.0072 | 2 | 0.070 |
| shared_bfull_fast | feature_gather_decode | 0.0057 | 2 | 0.070 |
| shared_bfull_fast | fp32_neighbor_reduction | 0.0100 | 2 | 0.070 |
| shared_bfull_fast | partial_bf16_conversion | 0.0031 | 2 | 0.070 |
| shared_bfull_fast | tile_load | 0.0031 | 2 | 0.070 |
| shared_bfull_fast | tile_compute | 0.0069 | 2 | 0.070 |
| shared_bfull_fast | tile_store | 0.0000 | 2 | 0.070 |
| shared_bfull_fast | output_scatter | 0.0010 | 2 | 0.070 |
| shared_bfull_fast | thread_amx_setup | 0.0067 | 2 | 0.070 |
| shared_bfull_accurate | output_zero | 0.0019 | 2 | 0.062 |
| shared_bfull_accurate | row_schedule | 0.0000 | 2 | 0.062 |
| shared_bfull_accurate | partial_zero | 0.0010 | 2 | 0.062 |
| shared_bfull_accurate | csr_prefetch | 0.0060 | 2 | 0.062 |
| shared_bfull_accurate | feature_gather_decode | 0.0110 | 2 | 0.062 |
| shared_bfull_accurate | fp32_neighbor_reduction | 0.0050 | 2 | 0.062 |
| shared_bfull_accurate | partial_bf16_conversion | 0.0000 | 2 | 0.062 |
| shared_bfull_accurate | tile_load | 0.0012 | 2 | 0.062 |
| shared_bfull_accurate | tile_compute | 0.0060 | 2 | 0.062 |
| shared_bfull_accurate | tile_store | 0.0000 | 2 | 0.062 |
| shared_bfull_accurate | output_scatter | 0.0010 | 2 | 0.062 |
| shared_bfull_accurate | thread_amx_setup | 0.0057 | 2 | 0.062 |
| replay_bfull_fast | output_zero | 0.0021 | 2 | 0.116 |
| replay_bfull_fast | row_schedule | 0.0010 | 2 | 0.116 |
| replay_bfull_fast | partial_zero | 0.0010 | 2 | 0.116 |
| replay_bfull_fast | csr_prefetch | 0.0148 | 2 | 0.116 |
| replay_bfull_fast | feature_gather_decode | 0.0150 | 2 | 0.116 |
| replay_bfull_fast | fp32_neighbor_reduction | 0.0143 | 2 | 0.116 |
| replay_bfull_fast | partial_bf16_conversion | 0.0029 | 2 | 0.116 |
| replay_bfull_fast | tile_load | 0.0012 | 2 | 0.116 |
| replay_bfull_fast | tile_compute | 0.0122 | 2 | 0.116 |
| replay_bfull_fast | tile_store | 0.0000 | 2 | 0.116 |
| replay_bfull_fast | output_scatter | 0.0000 | 2 | 0.116 |
| replay_bfull_fast | thread_amx_setup | 0.0000 | 2 | 0.116 |

## tail-n137-q130

N=137, E=17810, threads=8, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_bf16_inputs | 0.027 | 0.027 | 0.027 | 0.027 | 6.938x | 1.035x |
| mkl_fp32 | 0.028 | 0.028 | 0.028 | 0.028 | 6.701x | 1.000x |
| shared_bfull_fast_nozero | 0.064 | 0.064 | 0.064 | 0.064 | 2.925x | 0.437x |
| shared_b32_fast_nozero | 0.068 | 0.068 | 0.068 | 0.068 | 2.741x | 0.409x |
| shared_bfull_fast | 0.069 | 0.069 | 0.069 | 0.069 | 2.713x | 0.405x |
| shared_bfull_accurate_nozero | 0.069 | 0.069 | 0.069 | 0.069 | 2.703x | 0.403x |
| shared_bfull_accurate | 0.071 | 0.071 | 0.071 | 0.071 | 2.631x | 0.393x |
| replay_bfull_fast_nozero | 0.136 | 0.136 | 0.136 | 0.136 | 1.375x | 0.205x |
| replay_bfull_fast | 0.139 | 0.139 | 0.139 | 0.139 | 1.345x | 0.201x |
| original_nozero | 0.171 | 0.171 | 0.171 | 0.171 | 1.093x | 0.163x |
| original | 0.187 | 0.187 | 0.187 | 0.187 | 1.000x | 0.149x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_fp32 | 0.055 | 0.055 | 0.055 | 0.055 | 8.372x | 1.000x |
| mkl_bf16_inputs | 0.066 | 0.066 | 0.066 | 0.066 | 6.982x | 0.834x |
| shared_bfull_accurate_nozero | 0.123 | 0.123 | 0.123 | 0.123 | 3.755x | 0.449x |
| shared_bfull_fast_nozero | 0.125 | 0.125 | 0.125 | 0.125 | 3.691x | 0.441x |
| shared_bfull_fast | 0.134 | 0.134 | 0.134 | 0.134 | 3.441x | 0.411x |
| shared_bfull_accurate | 0.134 | 0.134 | 0.134 | 0.134 | 3.441x | 0.411x |
| shared_b32_fast_nozero | 0.137 | 0.137 | 0.137 | 0.137 | 3.363x | 0.402x |
| replay_bfull_fast_nozero | 0.197 | 0.197 | 0.197 | 0.197 | 2.339x | 0.279x |
| replay_bfull_fast | 0.215 | 0.215 | 0.215 | 0.215 | 2.144x | 0.256x |
| original_nozero | 0.348 | 0.348 | 0.348 | 0.348 | 1.326x | 0.158x |
| original | 0.461 | 0.461 | 0.461 | 0.461 | 1.000x | 0.119x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.141e-03 | 7.387e-01 | 5.237e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.117e-03 | 7.387e-01 | 5.220e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 4.635e-07 | 1.335e-04 | 9.465e-07 | 1 |
| original | kernel_full | mkl_fp32 | 5.117e-03 | 7.388e-01 | 5.220e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_bf16_inputs | 3.463e-03 | 5.628e-01 | 3.990e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_fp32 | 7.984e-03 | 1.092e+00 | 7.715e-03 | 1 |
| shared_bfull_accurate | kernel_full | mkl_bf16_inputs | 8.387e-06 | 1.442e-03 | 1.022e-05 | 1 |
| shared_bfull_accurate | kernel_full | mkl_fp32 | 5.121e-03 | 7.392e-01 | 5.223e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_bf16_inputs | 3.463e-03 | 5.628e-01 | 3.990e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_fp32 | 7.984e-03 | 1.092e+00 | 7.715e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 4.635e-07 | 1.335e-04 | 9.465e-07 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.117e-03 | 7.388e-01 | 5.220e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.463e-03 | 5.628e-01 | 3.990e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.984e-03 | 1.092e+00 | 7.715e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 8.387e-06 | 1.442e-03 | 1.022e-05 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.121e-03 | 7.392e-01 | 5.223e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.297e-03 | 5.337e-01 | 3.784e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 7.860e-03 | 1.102e+00 | 7.787e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.463e-03 | 5.628e-01 | 3.990e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.984e-03 | 1.092e+00 | 7.715e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.463e-03 | 5.628e-01 | 3.990e-03 | 1 |
| shared_bfull_accurate | instrumented_kernel | mkl_bf16_inputs | 8.387e-06 | 1.442e-03 | 1.022e-05 | 1 |
| replay_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.463e-03 | 5.628e-01 | 3.990e-03 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.463e-03 | 5.628e-01 | 3.990e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 8.387e-06 | 1.442e-03 | 1.022e-05 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.297e-03 | 5.337e-01 | 3.784e-03 | 1 |
| replay_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.463e-03 | 5.628e-01 | 3.990e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.052e-02 | 6.153e+02 | 9.670e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.153e+02 | 9.585e-03 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 6.108e-06 | 3.008e-01 | 4.727e-06 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.156e+02 | 9.589e-03 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.927e-03 | 4.922e+02 | 7.735e-03 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_fp32 | 1.595e-02 | 1.055e+03 | 1.643e-02 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_bf16_inputs | 1.292e-05 | 9.004e-01 | 1.415e-05 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.154e+02 | 9.586e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.927e-03 | 4.922e+02 | 7.735e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_fp32 | 1.595e-02 | 1.055e+03 | 1.643e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 6.108e-06 | 3.008e-01 | 4.727e-06 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.156e+02 | 9.589e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.927e-03 | 4.922e+02 | 7.735e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.595e-02 | 1.055e+03 | 1.643e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.292e-05 | 9.004e-01 | 1.415e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.154e+02 | 9.586e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.617e-03 | 3.900e+02 | 6.129e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.579e-02 | 9.830e+02 | 1.531e-02 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.927e-03 | 4.922e+02 | 7.735e-03 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.595e-02 | 1.055e+03 | 1.643e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 0.0060 | 2 | 0.443 |
| original | row_schedule | 0.0000 | 2 | 0.443 |
| original | partial_zero | 0.0010 | 2 | 0.443 |
| original | csr_prefetch | 0.0150 | 2 | 0.443 |
| original | feature_gather_decode | 0.0291 | 2 | 0.443 |
| original | fp32_neighbor_reduction | 0.0000 | 2 | 0.443 |
| original | partial_bf16_conversion | 0.0000 | 2 | 0.443 |
| original | tile_load | 0.1402 | 2 | 0.443 |
| original | tile_compute | 0.1266 | 2 | 0.443 |
| original | tile_store | 0.0000 | 2 | 0.443 |
| original | output_scatter | 0.0010 | 2 | 0.443 |
| original | thread_amx_setup | 0.0057 | 2 | 0.443 |
| shared_bfull_fast | output_zero | 0.0050 | 2 | 0.439 |
| shared_bfull_fast | row_schedule | 0.0010 | 2 | 0.439 |
| shared_bfull_fast | partial_zero | 0.0000 | 2 | 0.439 |
| shared_bfull_fast | csr_prefetch | 0.0992 | 2 | 0.439 |
| shared_bfull_fast | feature_gather_decode | 0.1180 | 2 | 0.439 |
| shared_bfull_fast | fp32_neighbor_reduction | 0.0896 | 2 | 0.439 |
| shared_bfull_fast | partial_bf16_conversion | 0.0000 | 2 | 0.439 |
| shared_bfull_fast | tile_load | 0.0033 | 2 | 0.439 |
| shared_bfull_fast | tile_compute | 0.0067 | 2 | 0.439 |
| shared_bfull_fast | tile_store | 0.0010 | 2 | 0.439 |
| shared_bfull_fast | output_scatter | 0.0010 | 2 | 0.439 |
| shared_bfull_fast | thread_amx_setup | 0.0076 | 2 | 0.439 |
| shared_bfull_accurate | output_zero | 0.0050 | 2 | 0.457 |
| shared_bfull_accurate | row_schedule | 0.0000 | 2 | 0.457 |
| shared_bfull_accurate | partial_zero | 0.0000 | 2 | 0.457 |
| shared_bfull_accurate | csr_prefetch | 0.0970 | 2 | 0.457 |
| shared_bfull_accurate | feature_gather_decode | 0.1121 | 2 | 0.457 |
| shared_bfull_accurate | fp32_neighbor_reduction | 0.0942 | 2 | 0.457 |
| shared_bfull_accurate | partial_bf16_conversion | 0.0012 | 2 | 0.457 |
| shared_bfull_accurate | tile_load | 0.0000 | 2 | 0.457 |
| shared_bfull_accurate | tile_compute | 0.0079 | 2 | 0.457 |
| shared_bfull_accurate | tile_store | 0.0000 | 2 | 0.457 |
| shared_bfull_accurate | output_scatter | 0.0010 | 2 | 0.457 |
| shared_bfull_accurate | thread_amx_setup | 0.0010 | 2 | 0.457 |
| replay_bfull_fast | output_zero | 0.0050 | 2 | 0.833 |
| replay_bfull_fast | row_schedule | 0.0000 | 2 | 0.833 |
| replay_bfull_fast | partial_zero | 0.0010 | 2 | 0.833 |
| replay_bfull_fast | csr_prefetch | 0.1853 | 2 | 0.833 |
| replay_bfull_fast | feature_gather_decode | 0.2215 | 2 | 0.833 |
| replay_bfull_fast | fp32_neighbor_reduction | 0.1903 | 2 | 0.833 |
| replay_bfull_fast | partial_bf16_conversion | 0.0000 | 2 | 0.833 |
| replay_bfull_fast | tile_load | 0.0019 | 2 | 0.833 |
| replay_bfull_fast | tile_compute | 0.0079 | 2 | 0.833 |
| replay_bfull_fast | tile_store | 0.0019 | 2 | 0.833 |
| replay_bfull_fast | output_scatter | 0.0000 | 2 | 0.833 |
| replay_bfull_fast | thread_amx_setup | 0.0010 | 2 | 0.833 |

## tail-n17-q0

N=17, E=0, threads=8, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 3.222x | 4.222x |
| shared_bfull_accurate_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 2.231x | 2.923x |
| shared_b32_fast_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 2.231x | 2.923x |
| shared_bfull_accurate | 0.004 | 0.004 | 0.004 | 0.004 | 1.813x | 2.375x |
| replay_bfull_fast_nozero | 0.004 | 0.004 | 0.004 | 0.004 | 1.706x | 2.235x |
| shared_bfull_fast | 0.005 | 0.005 | 0.005 | 0.005 | 1.381x | 1.810x |
| original_nozero | 0.005 | 0.005 | 0.005 | 0.005 | 1.381x | 1.810x |
| original | 0.007 | 0.007 | 0.007 | 0.007 | 1.000x | 1.310x |
| replay_bfull_fast | 0.007 | 0.007 | 0.007 | 0.007 | 1.000x | 1.310x |
| mkl_bf16_inputs | 0.008 | 0.008 | 0.008 | 0.008 | 0.853x | 1.118x |
| mkl_fp32 | 0.009 | 0.009 | 0.009 | 0.009 | 0.763x | 1.000x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.121x | 2.030x |
| shared_bfull_fast | 0.008 | 0.008 | 0.008 | 0.008 | 1.088x | 1.971x |
| original | 0.009 | 0.009 | 0.009 | 0.009 | 1.000x | 1.811x |
| shared_bfull_accurate | 0.009 | 0.009 | 0.009 | 0.009 | 1.000x | 1.811x |
| original_nozero | 0.009 | 0.009 | 0.009 | 0.009 | 0.974x | 1.763x |
| shared_bfull_accurate_nozero | 0.009 | 0.009 | 0.009 | 0.009 | 0.974x | 1.763x |
| shared_b32_fast_nozero | 0.009 | 0.009 | 0.009 | 0.009 | 0.974x | 1.763x |
| replay_bfull_fast | 0.010 | 0.010 | 0.010 | 0.010 | 0.881x | 1.595x |
| mkl_fp32 | 0.016 | 0.016 | 0.016 | 0.016 | 0.552x | 1.000x |
| mkl_bf16_inputs | 0.017 | 0.017 | 0.017 | 0.017 | 0.521x | 0.944x |
| replay_bfull_fast_nozero | 0.017 | 0.017 | 0.017 | 0.017 | 0.514x | 0.931x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 0.0012 | 2 | 0.009 |
| original | row_schedule | 0.0000 | 2 | 0.009 |
| original | partial_zero | 0.0000 | 2 | 0.009 |
| original | csr_prefetch | 0.0000 | 2 | 0.009 |
| original | feature_gather_decode | 0.0000 | 2 | 0.009 |
| original | fp32_neighbor_reduction | 0.0000 | 2 | 0.009 |
| original | partial_bf16_conversion | 0.0000 | 2 | 0.009 |
| original | tile_load | 0.0000 | 2 | 0.009 |
| original | tile_compute | 0.0000 | 2 | 0.009 |
| original | tile_store | 0.0000 | 2 | 0.009 |
| original | output_scatter | 0.0012 | 2 | 0.009 |
| original | thread_amx_setup | 0.0029 | 2 | 0.009 |
| shared_bfull_fast | output_zero | 0.0010 | 2 | 0.006 |
| shared_bfull_fast | row_schedule | 0.0010 | 2 | 0.006 |
| shared_bfull_fast | partial_zero | 0.0000 | 2 | 0.006 |
| shared_bfull_fast | csr_prefetch | 0.0000 | 2 | 0.006 |
| shared_bfull_fast | feature_gather_decode | 0.0000 | 2 | 0.006 |
| shared_bfull_fast | fp32_neighbor_reduction | 0.0000 | 2 | 0.006 |
| shared_bfull_fast | partial_bf16_conversion | 0.0000 | 2 | 0.006 |
| shared_bfull_fast | tile_load | 0.0000 | 2 | 0.006 |
| shared_bfull_fast | tile_compute | 0.0000 | 2 | 0.006 |
| shared_bfull_fast | tile_store | 0.0000 | 2 | 0.006 |
| shared_bfull_fast | output_scatter | 0.0000 | 2 | 0.006 |
| shared_bfull_fast | thread_amx_setup | 0.0012 | 2 | 0.006 |
| shared_bfull_accurate | output_zero | 0.0012 | 2 | 0.004 |
| shared_bfull_accurate | row_schedule | 0.0010 | 2 | 0.004 |
| shared_bfull_accurate | partial_zero | 0.0000 | 2 | 0.004 |
| shared_bfull_accurate | csr_prefetch | 0.0000 | 2 | 0.004 |
| shared_bfull_accurate | feature_gather_decode | 0.0000 | 2 | 0.004 |
| shared_bfull_accurate | fp32_neighbor_reduction | 0.0000 | 2 | 0.004 |
| shared_bfull_accurate | partial_bf16_conversion | 0.0000 | 2 | 0.004 |
| shared_bfull_accurate | tile_load | 0.0000 | 2 | 0.004 |
| shared_bfull_accurate | tile_compute | 0.0000 | 2 | 0.004 |
| shared_bfull_accurate | tile_store | 0.0000 | 2 | 0.004 |
| shared_bfull_accurate | output_scatter | 0.0000 | 2 | 0.004 |
| shared_bfull_accurate | thread_amx_setup | 0.0000 | 2 | 0.004 |
| replay_bfull_fast | output_zero | 0.0010 | 2 | 0.006 |
| replay_bfull_fast | row_schedule | 0.0000 | 2 | 0.006 |
| replay_bfull_fast | partial_zero | 0.0000 | 2 | 0.006 |
| replay_bfull_fast | csr_prefetch | 0.0000 | 2 | 0.006 |
| replay_bfull_fast | feature_gather_decode | 0.0000 | 2 | 0.006 |
| replay_bfull_fast | fp32_neighbor_reduction | 0.0000 | 2 | 0.006 |
| replay_bfull_fast | partial_bf16_conversion | 0.0000 | 2 | 0.006 |
| replay_bfull_fast | tile_load | 0.0000 | 2 | 0.006 |
| replay_bfull_fast | tile_compute | 0.0000 | 2 | 0.006 |
| replay_bfull_fast | tile_store | 0.0021 | 2 | 0.006 |
| replay_bfull_fast | output_scatter | 0.0000 | 2 | 0.006 |
| replay_bfull_fast | thread_amx_setup | 0.0000 | 2 | 0.006 |

