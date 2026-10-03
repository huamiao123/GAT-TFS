# GCN-extra first inference experiments

Numerical sum-aggregation experiments with random H/W; no task-accuracy claim.
Uninstrumented medians; original source is unchanged. Speedups are original/new and FP32 MKL/new.
Profiles are sampled summed thread time and do not equal wall time. Physical AMX work is modeled, not a hardware counter.

## smoke

N=37, E=538, threads=8, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_bf16_inputs | 0.009 | 0.009 | 0.009 | 0.009 | 3.697x | 1.211x |
| mkl_fp32 | 0.011 | 0.011 | 0.011 | 0.011 | 3.054x | 1.000x |
| shared_b64_fast_nozero | 0.013 | 0.012 | 0.012 | 0.014 | 2.578x | 0.844x |
| shared_bfull_fast_nozero | 0.013 | 0.013 | 0.013 | 0.014 | 2.487x | 0.814x |
| shared_b64_accurate_nozero | 0.014 | 0.014 | 0.014 | 0.015 | 2.342x | 0.767x |
| shared_bfull_accurate_nozero | 0.014 | 0.014 | 0.014 | 0.015 | 2.322x | 0.760x |
| shared_b32_fast_nozero | 0.015 | 0.015 | 0.015 | 0.016 | 2.162x | 0.708x |
| shared_b16_fast_nozero | 0.016 | 0.016 | 0.016 | 0.017 | 2.036x | 0.667x |
| shared_b32_accurate_nozero | 0.018 | 0.018 | 0.018 | 0.018 | 1.861x | 0.609x |
| shared_b16_accurate_nozero | 0.020 | 0.020 | 0.020 | 0.021 | 1.643x | 0.538x |
| shared_b8_fast_nozero | 0.024 | 0.021 | 0.023 | 0.026 | 1.371x | 0.449x |
| shared_b8_accurate_nozero | 0.025 | 0.024 | 0.024 | 0.025 | 1.344x | 0.440x |
| replay_bfull_fast_nozero | 0.025 | 0.019 | 0.022 | 0.028 | 1.332x | 0.436x |
| replay_b32_fast_nozero | 0.026 | 0.020 | 0.023 | 0.029 | 1.289x | 0.422x |
| replay_b8_fast_nozero | 0.026 | 0.026 | 0.026 | 0.026 | 1.289x | 0.422x |
| shared_b4_fast_nozero | 0.028 | 0.028 | 0.028 | 0.028 | 1.196x | 0.391x |
| original | 0.033 | 0.031 | 0.032 | 0.035 | 1.000x | 0.327x |
| original_nozero | 0.037 | 0.034 | 0.035 | 0.038 | 0.915x | 0.300x |
| shared_b4_accurate_nozero | 0.042 | 0.039 | 0.041 | 0.043 | 0.798x | 0.261x |
| shared_b2_fast_nozero | 0.045 | 0.041 | 0.043 | 0.047 | 0.743x | 0.243x |
| shared_b2_accurate_nozero | 0.062 | 0.062 | 0.062 | 0.063 | 0.536x | 0.176x |
| shared_b1_fast_nozero | 0.076 | 0.061 | 0.069 | 0.084 | 0.440x | 0.144x |
| shared_b1_accurate_nozero | 0.110 | 0.110 | 0.110 | 0.110 | 0.304x | 0.100x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_fp32 | 0.023 | 0.023 | 0.023 | 0.023 | 3.326x | 1.000x |
| shared_bfull_accurate_nozero | 0.025 | 0.025 | 0.025 | 0.025 | 3.057x | 0.919x |
| mkl_bf16_inputs | 0.026 | 0.025 | 0.025 | 0.026 | 2.959x | 0.889x |
| shared_b64_fast_nozero | 0.027 | 0.027 | 0.027 | 0.027 | 2.841x | 0.854x |
| shared_b64_accurate_nozero | 0.029 | 0.026 | 0.027 | 0.030 | 2.642x | 0.794x |
| shared_bfull_fast_nozero | 0.029 | 0.027 | 0.028 | 0.031 | 2.599x | 0.781x |
| shared_b32_fast_nozero | 0.033 | 0.031 | 0.032 | 0.033 | 2.352x | 0.707x |
| replay_bfull_fast_nozero | 0.035 | 0.032 | 0.034 | 0.036 | 2.184x | 0.656x |
| shared_b16_fast_nozero | 0.038 | 0.034 | 0.036 | 0.039 | 2.038x | 0.613x |
| shared_b16_accurate_nozero | 0.043 | 0.039 | 0.041 | 0.044 | 1.798x | 0.541x |
| shared_b32_accurate_nozero | 0.043 | 0.039 | 0.041 | 0.044 | 1.798x | 0.541x |
| replay_b32_fast_nozero | 0.043 | 0.040 | 0.041 | 0.044 | 1.783x | 0.536x |
| shared_b8_fast_nozero | 0.049 | 0.045 | 0.047 | 0.050 | 1.577x | 0.474x |
| shared_b8_accurate_nozero | 0.050 | 0.048 | 0.049 | 0.051 | 1.532x | 0.461x |
| replay_b8_fast_nozero | 0.051 | 0.051 | 0.051 | 0.052 | 1.486x | 0.447x |
| shared_b4_fast_nozero | 0.056 | 0.055 | 0.056 | 0.057 | 1.366x | 0.411x |
| original_nozero | 0.068 | 0.066 | 0.067 | 0.069 | 1.126x | 0.339x |
| original | 0.077 | 0.068 | 0.072 | 0.081 | 1.000x | 0.301x |
| shared_b4_accurate_nozero | 0.079 | 0.078 | 0.078 | 0.080 | 0.968x | 0.291x |
| shared_b2_fast_nozero | 0.088 | 0.085 | 0.086 | 0.090 | 0.870x | 0.262x |
| shared_b1_fast_nozero | 0.128 | 0.123 | 0.125 | 0.130 | 0.600x | 0.180x |
| shared_b2_accurate_nozero | 0.130 | 0.130 | 0.130 | 0.130 | 0.588x | 0.177x |
| shared_b1_accurate_nozero | 0.220 | 0.219 | 0.220 | 0.221 | 0.347x | 0.104x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.672e-03 | 3.386e-01 | 5.527e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.497e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 2.297e-07 | 2.289e-05 | 3.736e-07 | 1 |
| original | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.496e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 2.297e-07 | 2.289e-05 | 3.736e-07 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.496e-03 | 1 |
| shared_b1_fast_nozero | kernel_full | mkl_bf16_inputs | 2.297e-07 | 2.289e-05 | 3.736e-07 | 1 |
| shared_b1_fast_nozero | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.496e-03 | 1 |
| shared_b1_fast_nozero | b1_identity | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.297e-07 | 2.289e-05 | 3.736e-07 | 1 |
| shared_b1_accurate_nozero | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.496e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.152e-03 | 1.292e-01 | 2.109e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 7.156e-03 | 4.542e-01 | 7.373e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.311e-07 | 2.670e-05 | 4.359e-07 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.497e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.933e-03 | 2.015e-01 | 3.289e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 8.031e-03 | 5.265e-01 | 8.547e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 2.025e-07 | 2.289e-05 | 3.736e-07 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.497e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.096e-03 | 1.797e-01 | 2.933e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 8.226e-03 | 5.182e-01 | 8.413e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.905e-07 | 2.289e-05 | 3.736e-07 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.497e-03 | 1 |
| replay_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.096e-03 | 1.797e-01 | 2.933e-03 | 1 |
| replay_b8_fast_nozero | kernel_full | mkl_fp32 | 8.226e-03 | 5.182e-01 | 8.413e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.222e-03 | 1.745e-01 | 2.848e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 8.348e-03 | 5.071e-01 | 8.232e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.620e-06 | 1.183e-04 | 1.930e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.497e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.313e-03 | 2.092e-01 | 3.415e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 8.417e-03 | 5.348e-01 | 8.682e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.102e-06 | 2.365e-04 | 3.861e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.643e-03 | 3.386e-01 | 5.497e-03 | 1 |
| replay_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.313e-03 | 2.092e-01 | 3.415e-03 | 1 |
| replay_b32_fast_nozero | kernel_full | mkl_fp32 | 8.417e-03 | 5.348e-01 | 8.682e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 8.453e-03 | 5.256e-01 | 8.534e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.007e-06 | 2.871e-04 | 4.686e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.643e-03 | 3.386e-01 | 5.497e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.453e-03 | 5.256e-01 | 8.534e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.007e-06 | 2.871e-04 | 4.686e-06 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.643e-03 | 3.386e-01 | 5.497e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_fp32 | 8.453e-03 | 5.256e-01 | 8.534e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.297e-07 | 2.289e-05 | 3.736e-07 | 1 |
| shared_b1_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.297e-07 | 2.289e-05 | 3.736e-07 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.152e-03 | 1.292e-01 | 2.109e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.311e-07 | 2.670e-05 | 4.359e-07 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.933e-03 | 2.015e-01 | 3.289e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 2.025e-07 | 2.289e-05 | 3.736e-07 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.096e-03 | 1.797e-01 | 2.933e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.905e-07 | 2.289e-05 | 3.736e-07 | 1 |
| replay_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.096e-03 | 1.797e-01 | 2.933e-03 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.222e-03 | 1.745e-01 | 2.848e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.620e-06 | 1.183e-04 | 1.930e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.313e-03 | 2.092e-01 | 3.415e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.102e-06 | 2.365e-04 | 3.861e-06 | 1 |
| replay_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.313e-03 | 2.092e-01 | 3.415e-03 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.007e-06 | 2.871e-04 | 4.686e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.007e-06 | 2.871e-04 | 4.686e-06 | 1 |
| replay_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.095e-02 | 3.557e+01 | 1.028e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 2.519e-07 | 1.709e-03 | 4.939e-07 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 2.519e-07 | 1.709e-03 | 4.939e-07 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| shared_b1_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 2.519e-07 | 1.709e-03 | 4.939e-07 | 1 |
| shared_b1_fast_nozero | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| shared_b1_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.519e-07 | 1.709e-03 | 4.939e-07 | 1 |
| shared_b1_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 2.807e-03 | 1.079e+01 | 3.119e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.341e-02 | 4.503e+01 | 1.289e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 6.174e-05 | 1.245e-01 | 3.598e-05 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.565e+01 | 1.020e-02 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.584e-03 | 1.668e+01 | 4.820e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.520e-02 | 5.158e+01 | 1.476e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.501e-07 | 2.197e-03 | 6.350e-07 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.076e-03 | 1.909e+01 | 5.516e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.566e-02 | 5.399e+01 | 1.545e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 7.603e-07 | 3.174e-03 | 9.172e-07 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| replay_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.076e-03 | 1.909e+01 | 5.516e-03 | 1 |
| replay_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.566e-02 | 5.399e+01 | 1.545e-02 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.250e-03 | 2.091e+01 | 6.043e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.579e-02 | 5.519e+01 | 1.579e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 2.137e-06 | 1.196e-02 | 3.457e-06 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.423e-03 | 2.186e+01 | 6.316e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.594e-02 | 5.530e+01 | 1.582e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 6.183e-05 | 1.436e-01 | 4.149e-05 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.567e+01 | 1.021e-02 | 1 |
| replay_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.423e-03 | 2.186e+01 | 6.316e-03 | 1 |
| replay_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.594e-02 | 5.530e+01 | 1.582e-02 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.281e-03 | 1.925e+01 | 5.564e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.578e-02 | 5.186e+01 | 1.484e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 6.184e-05 | 1.443e-01 | 4.170e-05 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.567e+01 | 1.021e-02 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.281e-03 | 1.925e+01 | 5.564e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.578e-02 | 5.186e+01 | 1.484e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 6.184e-05 | 1.443e-01 | 4.170e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.567e+01 | 1.021e-02 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.281e-03 | 1.925e+01 | 5.564e-03 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.578e-02 | 5.186e+01 | 1.484e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 0.0021 | 2 | 0.149 |
| original | row_schedule | 0.0010 | 2 | 0.149 |
| original | partial_zero | 0.0021 | 2 | 0.149 |
| original | csr_prefetch | 0.0029 | 2 | 0.149 |
| original | feature_gather_decode | 0.0048 | 2 | 0.149 |
| original | fp32_neighbor_reduction | 0.0000 | 2 | 0.149 |
| original | partial_bf16_conversion | 0.0000 | 2 | 0.149 |
| original | tile_load | 0.0288 | 2 | 0.149 |
| original | tile_compute | 0.0224 | 2 | 0.149 |
| original | tile_store | 0.0000 | 2 | 0.149 |
| original | output_scatter | 0.0010 | 2 | 0.149 |
| original | thread_amx_setup | 0.2823 | 2 | 0.149 |

## tail-n137-q130

N=137, E=17810, threads=8, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_fp32 | 0.027 | 0.027 | 0.027 | 0.027 | 6.623x | 1.000x |
| mkl_bf16_inputs | 0.032 | 0.032 | 0.032 | 0.032 | 5.634x | 0.851x |
| shared_bfull_fast_nozero | 0.053 | 0.053 | 0.053 | 0.053 | 3.401x | 0.514x |
| shared_b32_fast_nozero | 0.070 | 0.070 | 0.070 | 0.070 | 2.568x | 0.388x |
| shared_bfull_accurate_nozero | 0.070 | 0.070 | 0.070 | 0.070 | 2.568x | 0.388x |
| shared_b32_accurate_nozero | 0.076 | 0.076 | 0.076 | 0.076 | 2.374x | 0.358x |
| shared_b64_fast_nozero | 0.076 | 0.076 | 0.076 | 0.076 | 2.367x | 0.357x |
| shared_b64_accurate_nozero | 0.082 | 0.082 | 0.082 | 0.082 | 2.195x | 0.331x |
| shared_b16_fast_nozero | 0.098 | 0.098 | 0.098 | 0.098 | 1.837x | 0.277x |
| replay_bfull_fast_nozero | 0.100 | 0.100 | 0.100 | 0.100 | 1.798x | 0.271x |
| replay_b32_fast_nozero | 0.113 | 0.113 | 0.113 | 0.113 | 1.593x | 0.241x |
| shared_b16_accurate_nozero | 0.117 | 0.117 | 0.117 | 0.117 | 1.538x | 0.232x |
| shared_b8_fast_nozero | 0.144 | 0.144 | 0.144 | 0.144 | 1.250x | 0.189x |
| shared_b8_accurate_nozero | 0.144 | 0.144 | 0.144 | 0.144 | 1.250x | 0.189x |
| original | 0.180 | 0.180 | 0.180 | 0.180 | 1.000x | 0.151x |
| replay_b8_fast_nozero | 0.195 | 0.195 | 0.195 | 0.195 | 0.923x | 0.139x |
| shared_b4_fast_nozero | 0.216 | 0.216 | 0.216 | 0.216 | 0.833x | 0.126x |
| original_nozero | 0.231 | 0.231 | 0.231 | 0.231 | 0.779x | 0.118x |
| shared_b4_accurate_nozero | 0.239 | 0.239 | 0.239 | 0.239 | 0.753x | 0.114x |
| shared_b2_fast_nozero | 0.292 | 0.292 | 0.292 | 0.292 | 0.616x | 0.093x |
| shared_b1_fast_nozero | 0.435 | 0.435 | 0.435 | 0.435 | 0.414x | 0.062x |
| shared_b2_accurate_nozero | 0.500 | 0.500 | 0.500 | 0.500 | 0.360x | 0.054x |
| shared_b1_accurate_nozero | 0.750 | 0.750 | 0.750 | 0.750 | 0.240x | 0.036x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_fp32 | 0.065 | 0.065 | 0.065 | 0.065 | 6.267x | 1.000x |
| mkl_bf16_inputs | 0.066 | 0.066 | 0.066 | 0.066 | 6.177x | 0.986x |
| shared_bfull_accurate_nozero | 0.131 | 0.131 | 0.131 | 0.131 | 3.117x | 0.497x |
| shared_bfull_fast_nozero | 0.136 | 0.136 | 0.136 | 0.136 | 3.002x | 0.479x |
| shared_b64_accurate_nozero | 0.145 | 0.145 | 0.145 | 0.145 | 2.814x | 0.449x |
| shared_b64_fast_nozero | 0.157 | 0.157 | 0.157 | 0.157 | 2.596x | 0.414x |
| shared_b32_accurate_nozero | 0.168 | 0.168 | 0.168 | 0.168 | 2.427x | 0.387x |
| shared_b32_fast_nozero | 0.169 | 0.169 | 0.169 | 0.169 | 2.413x | 0.385x |
| shared_b16_fast_nozero | 0.185 | 0.185 | 0.185 | 0.185 | 2.205x | 0.352x |
| shared_b16_accurate_nozero | 0.221 | 0.221 | 0.221 | 0.221 | 1.846x | 0.294x |
| replay_b32_fast_nozero | 0.236 | 0.236 | 0.236 | 0.236 | 1.728x | 0.276x |
| shared_b8_fast_nozero | 0.242 | 0.242 | 0.242 | 0.242 | 1.684x | 0.269x |
| replay_bfull_fast_nozero | 0.246 | 0.246 | 0.246 | 0.246 | 1.658x | 0.265x |
| replay_b8_fast_nozero | 0.345 | 0.345 | 0.345 | 0.345 | 1.182x | 0.189x |
| shared_b8_accurate_nozero | 0.346 | 0.346 | 0.346 | 0.346 | 1.178x | 0.188x |
| original_nozero | 0.369 | 0.369 | 0.369 | 0.369 | 1.106x | 0.176x |
| original | 0.408 | 0.408 | 0.408 | 0.408 | 1.000x | 0.160x |
| shared_b4_fast_nozero | 0.408 | 0.408 | 0.408 | 0.408 | 1.000x | 0.160x |
| shared_b4_accurate_nozero | 0.492 | 0.492 | 0.492 | 0.492 | 0.829x | 0.132x |
| shared_b2_fast_nozero | 0.691 | 0.691 | 0.691 | 0.691 | 0.590x | 0.094x |
| shared_b2_accurate_nozero | 0.947 | 0.947 | 0.947 | 0.947 | 0.431x | 0.069x |
| shared_b1_fast_nozero | 1.076 | 1.076 | 1.076 | 1.076 | 0.379x | 0.060x |
| shared_b1_accurate_nozero | 1.581 | 1.581 | 1.581 | 1.581 | 0.258x | 0.041x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.141e-03 | 7.387e-01 | 5.237e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.117e-03 | 7.387e-01 | 5.220e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 4.635e-07 | 1.335e-04 | 9.465e-07 | 1 |
| original | kernel_full | mkl_fp32 | 5.117e-03 | 7.388e-01 | 5.220e-03 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 4.635e-07 | 1.335e-04 | 9.465e-07 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 5.117e-03 | 7.388e-01 | 5.220e-03 | 1 |
| shared_b1_fast_nozero | kernel_full | mkl_bf16_inputs | 4.635e-07 | 1.335e-04 | 9.465e-07 | 1 |
| shared_b1_fast_nozero | kernel_full | mkl_fp32 | 5.117e-03 | 7.388e-01 | 5.220e-03 | 1 |
| shared_b1_fast_nozero | b1_identity | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.635e-07 | 1.335e-04 | 9.465e-07 | 1 |
| shared_b1_accurate_nozero | kernel_full | mkl_fp32 | 5.117e-03 | 7.388e-01 | 5.220e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 2.361e-03 | 4.068e-01 | 2.884e-03 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 6.821e-03 | 1.005e+00 | 7.101e-03 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.767e-07 | 1.450e-04 | 1.028e-06 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 5.117e-03 | 7.388e-01 | 5.220e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 2.985e-03 | 4.910e-01 | 3.481e-03 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 7.522e-03 | 1.079e+00 | 7.624e-03 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 3.738e-07 | 1.297e-04 | 9.195e-07 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 5.117e-03 | 7.388e-01 | 5.220e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.164e-03 | 4.889e-01 | 3.466e-03 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 7.735e-03 | 1.049e+00 | 7.415e-03 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 6.609e-07 | 1.717e-04 | 1.217e-06 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 5.117e-03 | 7.387e-01 | 5.220e-03 | 1 |
| replay_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 3.164e-03 | 4.889e-01 | 3.466e-03 | 1 |
| replay_b8_fast_nozero | kernel_full | mkl_fp32 | 7.735e-03 | 1.049e+00 | 7.415e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 3.305e-03 | 5.967e-01 | 4.231e-03 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 7.888e-03 | 1.178e+00 | 8.322e-03 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 1.990e-06 | 3.719e-04 | 2.637e-06 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 5.118e-03 | 7.388e-01 | 5.220e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.297e-03 | 5.337e-01 | 3.784e-03 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 7.860e-03 | 1.102e+00 | 7.787e-03 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 4.165e-06 | 7.896e-04 | 5.598e-06 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 5.119e-03 | 7.385e-01 | 5.218e-03 | 1 |
| replay_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 3.297e-03 | 5.337e-01 | 3.784e-03 | 1 |
| replay_b32_fast_nozero | kernel_full | mkl_fp32 | 7.860e-03 | 1.102e+00 | 7.787e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 3.405e-03 | 6.370e-01 | 4.516e-03 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 7.959e-03 | 1.139e+00 | 8.050e-03 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 6.456e-06 | 1.282e-03 | 9.087e-06 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 5.120e-03 | 7.392e-01 | 5.223e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.463e-03 | 5.628e-01 | 3.990e-03 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.984e-03 | 1.092e+00 | 7.715e-03 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 8.387e-06 | 1.442e-03 | 1.022e-05 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 5.121e-03 | 7.392e-01 | 5.223e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 3.463e-03 | 5.628e-01 | 3.990e-03 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_fp32 | 7.984e-03 | 1.092e+00 | 7.715e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 4.635e-07 | 1.335e-04 | 9.465e-07 | 1 |
| shared_b1_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.635e-07 | 1.335e-04 | 9.465e-07 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.361e-03 | 4.068e-01 | 2.884e-03 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.767e-07 | 1.450e-04 | 1.028e-06 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 2.985e-03 | 4.910e-01 | 3.481e-03 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 3.738e-07 | 1.297e-04 | 9.195e-07 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.164e-03 | 4.889e-01 | 3.466e-03 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 6.609e-07 | 1.717e-04 | 1.217e-06 | 1 |
| replay_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.164e-03 | 4.889e-01 | 3.466e-03 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.305e-03 | 5.967e-01 | 4.231e-03 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 1.990e-06 | 3.719e-04 | 2.637e-06 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.297e-03 | 5.337e-01 | 3.784e-03 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 4.165e-06 | 7.896e-04 | 5.598e-06 | 1 |
| replay_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.297e-03 | 5.337e-01 | 3.784e-03 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.405e-03 | 6.370e-01 | 4.516e-03 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 6.456e-06 | 1.282e-03 | 9.087e-06 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.463e-03 | 5.628e-01 | 3.990e-03 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 8.387e-06 | 1.442e-03 | 1.022e-05 | 1 |
| replay_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 3.463e-03 | 5.628e-01 | 3.990e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.052e-02 | 6.153e+02 | 9.670e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.153e+02 | 9.585e-03 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 6.108e-06 | 3.008e-01 | 4.727e-06 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.156e+02 | 9.589e-03 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 6.108e-06 | 3.008e-01 | 4.727e-06 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.156e+02 | 9.589e-03 | 1 |
| shared_b1_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 6.108e-06 | 3.008e-01 | 4.727e-06 | 1 |
| shared_b1_fast_nozero | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.156e+02 | 9.589e-03 | 1 |
| shared_b1_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 6.108e-06 | 3.008e-01 | 4.727e-06 | 1 |
| shared_b1_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.156e+02 | 9.589e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 3.311e-03 | 2.390e+02 | 3.756e-03 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 1.354e-02 | 8.413e+02 | 1.311e-02 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 6.084e-06 | 2.891e-01 | 4.543e-06 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.156e+02 | 9.589e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 4.721e-03 | 3.074e+02 | 4.831e-03 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 1.491e-02 | 9.003e+02 | 1.402e-02 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 3.168e-07 | 6.250e-02 | 9.822e-07 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.154e+02 | 9.585e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.162e-03 | 3.436e+02 | 5.399e-03 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.536e-02 | 9.311e+02 | 1.450e-02 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 6.081e-06 | 2.812e-01 | 4.420e-06 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.156e+02 | 9.589e-03 | 1 |
| replay_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.162e-03 | 3.436e+02 | 5.399e-03 | 1 |
| replay_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 1.536e-02 | 9.311e+02 | 1.450e-02 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.565e-03 | 3.848e+02 | 6.047e-03 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 1.575e-02 | 9.717e+02 | 1.514e-02 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 5.130e-07 | 4.297e-02 | 6.752e-07 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.153e+02 | 9.585e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.617e-03 | 3.900e+02 | 6.129e-03 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.579e-02 | 9.830e+02 | 1.531e-02 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 4.337e-06 | 2.690e-01 | 4.228e-06 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.154e+02 | 9.586e-03 | 1 |
| replay_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.617e-03 | 3.900e+02 | 6.129e-03 | 1 |
| replay_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 1.579e-02 | 9.830e+02 | 1.531e-02 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.736e-03 | 4.058e+02 | 6.377e-03 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 1.586e-02 | 1.008e+03 | 1.569e-02 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 9.746e-06 | 7.461e-01 | 1.172e-05 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.157e+02 | 9.591e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.927e-03 | 4.922e+02 | 7.735e-03 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.595e-02 | 1.055e+03 | 1.643e-02 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 1.292e-05 | 9.004e-01 | 1.415e-05 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.154e+02 | 9.586e-03 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 5.927e-03 | 4.922e+02 | 7.735e-03 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 1.595e-02 | 1.055e+03 | 1.643e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 0.0060 | 2 | 0.478 |
| original | row_schedule | 0.0000 | 2 | 0.478 |
| original | partial_zero | 0.0000 | 2 | 0.478 |
| original | csr_prefetch | 0.0250 | 2 | 0.478 |
| original | feature_gather_decode | 0.0229 | 2 | 0.478 |
| original | fp32_neighbor_reduction | 0.0000 | 2 | 0.478 |
| original | partial_bf16_conversion | 0.0000 | 2 | 0.478 |
| original | tile_load | 0.1395 | 2 | 0.478 |
| original | tile_compute | 0.1202 | 2 | 0.478 |
| original | tile_store | 0.0000 | 2 | 0.478 |
| original | output_scatter | 0.0010 | 2 | 0.478 |
| original | thread_amx_setup | 0.1907 | 2 | 0.478 |

## tail-n17-q0

N=17, E=0, threads=8, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b2_fast_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 2.125x | 3.125x |
| shared_b2_accurate_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 2.125x | 3.125x |
| shared_b4_fast_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 2.125x | 3.125x |
| shared_b32_accurate_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 2.125x | 3.125x |
| replay_b32_fast_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 2.125x | 3.125x |
| shared_b64_accurate_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 2.125x | 3.125x |
| replay_bfull_fast_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 2.125x | 3.125x |
| original_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 1.889x | 2.778x |
| shared_b1_accurate_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 1.889x | 2.778x |
| shared_b4_accurate_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 1.889x | 2.778x |
| replay_b8_fast_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 1.889x | 2.778x |
| shared_b16_fast_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 1.889x | 2.778x |
| shared_b64_fast_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 1.889x | 2.778x |
| shared_bfull_accurate_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 1.889x | 2.778x |
| shared_b8_accurate_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 1.417x | 2.083x |
| shared_bfull_fast_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 1.417x | 2.083x |
| shared_b1_fast_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 1.308x | 1.923x |
| shared_b8_fast_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 1.308x | 1.923x |
| shared_b16_accurate_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 1.308x | 1.923x |
| original | 0.004 | 0.004 | 0.004 | 0.004 | 1.000x | 1.471x |
| shared_b32_fast_nozero | 0.004 | 0.004 | 0.004 | 0.004 | 1.000x | 1.471x |
| mkl_fp32 | 0.006 | 0.006 | 0.006 | 0.006 | 0.680x | 1.000x |
| mkl_bf16_inputs | 0.008 | 0.008 | 0.008 | 0.008 | 0.515x | 0.758x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b1_fast_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.448x | 2.034x |
| shared_b2_fast_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.448x | 2.034x |
| shared_b8_accurate_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.448x | 2.034x |
| shared_b64_fast_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.448x | 2.034x |
| shared_b64_accurate_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.448x | 2.034x |
| shared_b4_fast_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.400x | 1.967x |
| shared_b16_fast_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.400x | 1.967x |
| shared_b16_accurate_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.400x | 1.967x |
| shared_bfull_accurate_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.273x | 1.788x |
| shared_b1_accurate_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.235x | 1.735x |
| shared_b2_accurate_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.235x | 1.735x |
| shared_b4_accurate_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.235x | 1.735x |
| shared_b8_fast_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.235x | 1.735x |
| shared_b32_fast_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.235x | 1.735x |
| shared_b32_accurate_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.235x | 1.735x |
| shared_bfull_fast_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.235x | 1.735x |
| original_nozero | 0.009 | 0.009 | 0.009 | 0.009 | 1.135x | 1.595x |
| replay_b32_fast_nozero | 0.009 | 0.009 | 0.009 | 0.009 | 1.105x | 1.553x |
| original | 0.010 | 0.010 | 0.010 | 0.010 | 1.000x | 1.405x |
| replay_b8_fast_nozero | 0.010 | 0.010 | 0.010 | 0.010 | 1.000x | 1.405x |
| replay_bfull_fast_nozero | 0.010 | 0.010 | 0.010 | 0.010 | 1.000x | 1.405x |
| mkl_fp32 | 0.014 | 0.014 | 0.014 | 0.014 | 0.712x | 1.000x |
| mkl_bf16_inputs | 0.015 | 0.015 | 0.015 | 0.015 | 0.667x | 0.937x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_fast_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_fast_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_fast_nozero | b1_identity | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_accurate_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_accurate_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_accurate_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b4_fast_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b4_accurate_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_fast_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_accurate_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_b8_fast_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_b8_fast_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b16_fast_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b16_accurate_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_fast_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_accurate_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_b32_fast_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_b32_fast_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b64_fast_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b64_accurate_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast_nozero | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b4_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b4_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_b8_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b16_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b16_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_b32_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b64_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b64_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast_nozero | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_fast_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_accurate_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_accurate_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b4_fast_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b4_accurate_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_accurate_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_b8_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_b8_fast_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b16_fast_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b16_accurate_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_accurate_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_b32_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_b32_fast_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b64_fast_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b64_accurate_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast_nozero | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 0.0012 | 2 | 0.012 |
| original | row_schedule | 0.0000 | 2 | 0.012 |
| original | partial_zero | 0.0012 | 2 | 0.012 |
| original | csr_prefetch | 0.0000 | 2 | 0.012 |
| original | feature_gather_decode | 0.0000 | 2 | 0.012 |
| original | fp32_neighbor_reduction | 0.0000 | 2 | 0.012 |
| original | partial_bf16_conversion | 0.0000 | 2 | 0.012 |
| original | tile_load | 0.0000 | 2 | 0.012 |
| original | tile_compute | 0.0000 | 2 | 0.012 |
| original | tile_store | 0.0000 | 2 | 0.012 |
| original | output_scatter | 0.0010 | 2 | 0.012 |
| original | thread_amx_setup | 0.0241 | 2 | 0.012 |

