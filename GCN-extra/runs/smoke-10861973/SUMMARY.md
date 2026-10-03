# GCN-extra first inference experiments

Numerical sum-aggregation experiments with random H/W; no task-accuracy claim.
Uninstrumented medians; original source is unchanged. Speedups are original/new and FP32 MKL/new.
Profiles are sampled summed thread time and do not equal wall time. Physical AMX work is modeled, not a hardware counter.

## smoke

N=37, E=538, threads=8, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_fp32 | 0.010 | 0.009 | 0.009 | 0.010 | 4.300x | 1.000x |
| mkl_bf16_inputs | 0.010 | 0.010 | 0.010 | 0.010 | 4.095x | 0.952x |
| shared_b64_fast_nozero | 0.013 | 0.013 | 0.013 | 0.013 | 3.127x | 0.727x |
| shared_bfull_fast_nozero | 0.014 | 0.013 | 0.013 | 0.014 | 3.018x | 0.702x |
| shared_b64_accurate_nozero | 0.015 | 0.014 | 0.014 | 0.015 | 2.820x | 0.656x |
| shared_b32_fast_nozero | 0.015 | 0.015 | 0.015 | 0.016 | 2.646x | 0.615x |
| shared_b16_fast_nozero | 0.016 | 0.016 | 0.016 | 0.016 | 2.567x | 0.597x |
| shared_b32_accurate_nozero | 0.017 | 0.017 | 0.017 | 0.017 | 2.406x | 0.559x |
| shared_bfull_accurate_nozero | 0.018 | 0.015 | 0.017 | 0.019 | 2.278x | 0.530x |
| replay_bfull_fast_nozero | 0.018 | 0.018 | 0.018 | 0.019 | 2.219x | 0.516x |
| replay_b32_fast_nozero | 0.021 | 0.020 | 0.020 | 0.021 | 2.000x | 0.465x |
| shared_b8_fast_nozero | 0.021 | 0.021 | 0.021 | 0.021 | 1.955x | 0.455x |
| shared_b16_accurate_nozero | 0.022 | 0.021 | 0.022 | 0.023 | 1.830x | 0.426x |
| shared_b8_accurate_nozero | 0.024 | 0.024 | 0.024 | 0.025 | 1.686x | 0.392x |
| replay_b8_fast_nozero | 0.026 | 0.026 | 0.026 | 0.027 | 1.550x | 0.360x |
| shared_b4_fast_nozero | 0.028 | 0.027 | 0.027 | 0.028 | 1.470x | 0.342x |
| original_nozero | 0.032 | 0.032 | 0.032 | 0.032 | 1.284x | 0.299x |
| shared_b4_accurate_nozero | 0.038 | 0.037 | 0.037 | 0.039 | 1.078x | 0.251x |
| original | 0.041 | 0.037 | 0.039 | 0.043 | 1.000x | 0.233x |
| shared_b2_fast_nozero | 0.041 | 0.041 | 0.041 | 0.042 | 0.989x | 0.230x |
| shared_b2_accurate_nozero | 0.064 | 0.063 | 0.063 | 0.064 | 0.645x | 0.150x |
| shared_b1_fast_nozero | 0.068 | 0.062 | 0.065 | 0.072 | 0.599x | 0.139x |
| shared_b1_accurate_nozero | 0.110 | 0.109 | 0.109 | 0.110 | 0.374x | 0.087x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_fp32 | 0.022 | 0.020 | 0.021 | 0.022 | 3.265x | 1.000x |
| shared_b64_fast_nozero | 0.023 | 0.022 | 0.023 | 0.024 | 3.000x | 0.919x |
| mkl_bf16_inputs | 0.023 | 0.023 | 0.023 | 0.024 | 3.000x | 0.919x |
| shared_bfull_fast_nozero | 0.026 | 0.023 | 0.024 | 0.027 | 2.724x | 0.834x |
| shared_bfull_accurate_nozero | 0.028 | 0.027 | 0.028 | 0.029 | 2.504x | 0.767x |
| shared_b64_accurate_nozero | 0.030 | 0.027 | 0.028 | 0.031 | 2.355x | 0.721x |
| shared_b32_accurate_nozero | 0.031 | 0.029 | 0.030 | 0.033 | 2.239x | 0.686x |
| shared_b32_fast_nozero | 0.033 | 0.033 | 0.033 | 0.033 | 2.141x | 0.656x |
| replay_bfull_fast_nozero | 0.035 | 0.032 | 0.034 | 0.037 | 2.003x | 0.614x |
| replay_b32_fast_nozero | 0.037 | 0.033 | 0.035 | 0.039 | 1.906x | 0.584x |
| shared_b16_fast_nozero | 0.037 | 0.037 | 0.037 | 0.037 | 1.900x | 0.582x |
| shared_b16_accurate_nozero | 0.041 | 0.041 | 0.041 | 0.041 | 1.718x | 0.526x |
| shared_b8_fast_nozero | 0.043 | 0.042 | 0.043 | 0.044 | 1.624x | 0.497x |
| replay_b8_fast_nozero | 0.050 | 0.047 | 0.048 | 0.051 | 1.421x | 0.435x |
| shared_b8_accurate_nozero | 0.057 | 0.055 | 0.056 | 0.058 | 1.236x | 0.379x |
| shared_b4_fast_nozero | 0.065 | 0.063 | 0.064 | 0.066 | 1.084x | 0.332x |
| original_nozero | 0.068 | 0.067 | 0.067 | 0.068 | 1.037x | 0.318x |
| original | 0.070 | 0.070 | 0.070 | 0.071 | 1.000x | 0.306x |
| shared_b4_accurate_nozero | 0.080 | 0.078 | 0.079 | 0.080 | 0.886x | 0.271x |
| shared_b2_fast_nozero | 0.087 | 0.085 | 0.086 | 0.088 | 0.810x | 0.248x |
| shared_b2_accurate_nozero | 0.128 | 0.127 | 0.127 | 0.128 | 0.552x | 0.169x |
| shared_b1_fast_nozero | 0.132 | 0.126 | 0.129 | 0.134 | 0.535x | 0.164x |
| shared_b1_accurate_nozero | 0.228 | 0.222 | 0.225 | 0.231 | 0.309x | 0.095x |

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
| original | output_zero | 0.0019 | 2 | 0.161 |
| original | row_schedule | 0.0010 | 2 | 0.161 |
| original | partial_zero | 0.0021 | 2 | 0.161 |
| original | csr_prefetch | 0.0048 | 2 | 0.161 |
| original | feature_gather_decode | 0.0031 | 2 | 0.161 |
| original | fp32_neighbor_reduction | 0.0000 | 2 | 0.161 |
| original | partial_bf16_conversion | 0.0000 | 2 | 0.161 |
| original | tile_load | 0.0222 | 2 | 0.161 |
| original | tile_compute | 0.0286 | 2 | 0.161 |
| original | tile_store | 0.0010 | 2 | 0.161 |
| original | output_scatter | 0.0000 | 2 | 0.161 |
| original | thread_amx_setup | 0.3216 | 2 | 0.161 |

## tail-n137-q130

N=137, E=17810, threads=8, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_bf16_inputs | 0.028 | 0.028 | 0.028 | 0.028 | 8.026x | 1.145x |
| mkl_fp32 | 0.032 | 0.032 | 0.032 | 0.032 | 7.007x | 1.000x |
| shared_bfull_fast_nozero | 0.053 | 0.053 | 0.053 | 0.053 | 4.230x | 0.604x |
| shared_b64_fast_nozero | 0.061 | 0.061 | 0.061 | 0.061 | 3.668x | 0.523x |
| shared_bfull_accurate_nozero | 0.067 | 0.067 | 0.067 | 0.067 | 3.342x | 0.477x |
| shared_b32_fast_nozero | 0.070 | 0.070 | 0.070 | 0.070 | 3.205x | 0.457x |
| shared_b64_accurate_nozero | 0.081 | 0.081 | 0.081 | 0.081 | 2.762x | 0.394x |
| shared_b16_fast_nozero | 0.086 | 0.086 | 0.086 | 0.086 | 2.601x | 0.371x |
| replay_bfull_fast_nozero | 0.102 | 0.102 | 0.102 | 0.102 | 2.194x | 0.313x |
| shared_b8_fast_nozero | 0.119 | 0.119 | 0.119 | 0.119 | 1.882x | 0.269x |
| shared_b32_accurate_nozero | 0.143 | 0.143 | 0.143 | 0.143 | 1.568x | 0.224x |
| shared_b16_accurate_nozero | 0.146 | 0.146 | 0.146 | 0.146 | 1.534x | 0.219x |
| shared_b8_accurate_nozero | 0.148 | 0.148 | 0.148 | 0.148 | 1.515x | 0.216x |
| replay_b32_fast_nozero | 0.158 | 0.158 | 0.158 | 0.158 | 1.418x | 0.202x |
| replay_b8_fast_nozero | 0.174 | 0.174 | 0.174 | 0.174 | 1.288x | 0.184x |
| shared_b4_fast_nozero | 0.184 | 0.184 | 0.184 | 0.184 | 1.216x | 0.174x |
| original_nozero | 0.203 | 0.203 | 0.203 | 0.203 | 1.102x | 0.157x |
| original | 0.224 | 0.224 | 0.224 | 0.224 | 1.000x | 0.143x |
| shared_b4_accurate_nozero | 0.239 | 0.239 | 0.239 | 0.239 | 0.936x | 0.134x |
| shared_b2_fast_nozero | 0.298 | 0.298 | 0.298 | 0.298 | 0.751x | 0.107x |
| shared_b1_fast_nozero | 0.485 | 0.485 | 0.485 | 0.485 | 0.461x | 0.066x |
| shared_b2_accurate_nozero | 0.524 | 0.524 | 0.524 | 0.524 | 0.427x | 0.061x |
| shared_b1_accurate_nozero | 0.738 | 0.738 | 0.738 | 0.738 | 0.303x | 0.043x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_fp32 | 0.066 | 0.066 | 0.066 | 0.066 | 5.570x | 1.000x |
| mkl_bf16_inputs | 0.075 | 0.075 | 0.075 | 0.075 | 4.898x | 0.879x |
| shared_b64_fast_nozero | 0.132 | 0.132 | 0.132 | 0.132 | 2.785x | 0.500x |
| shared_bfull_fast_nozero | 0.135 | 0.135 | 0.135 | 0.135 | 2.726x | 0.489x |
| shared_bfull_accurate_nozero | 0.138 | 0.138 | 0.138 | 0.138 | 2.670x | 0.479x |
| shared_b32_fast_nozero | 0.147 | 0.147 | 0.147 | 0.147 | 2.505x | 0.450x |
| shared_b64_accurate_nozero | 0.152 | 0.152 | 0.152 | 0.152 | 2.418x | 0.434x |
| shared_b32_accurate_nozero | 0.167 | 0.167 | 0.167 | 0.167 | 2.204x | 0.396x |
| shared_b16_fast_nozero | 0.178 | 0.178 | 0.178 | 0.178 | 2.066x | 0.371x |
| shared_b16_accurate_nozero | 0.204 | 0.204 | 0.204 | 0.204 | 1.803x | 0.324x |
| replay_b32_fast_nozero | 0.239 | 0.239 | 0.239 | 0.239 | 1.540x | 0.276x |
| replay_bfull_fast_nozero | 0.242 | 0.242 | 0.242 | 0.242 | 1.520x | 0.273x |
| shared_b8_fast_nozero | 0.246 | 0.246 | 0.246 | 0.246 | 1.495x | 0.268x |
| shared_b8_accurate_nozero | 0.345 | 0.345 | 0.345 | 0.345 | 1.066x | 0.191x |
| original_nozero | 0.357 | 0.357 | 0.357 | 0.357 | 1.030x | 0.185x |
| replay_b8_fast_nozero | 0.362 | 0.362 | 0.362 | 0.362 | 1.016x | 0.182x |
| original | 0.368 | 0.368 | 0.368 | 0.368 | 1.000x | 0.180x |
| shared_b4_fast_nozero | 0.407 | 0.407 | 0.407 | 0.407 | 0.904x | 0.162x |
| shared_b4_accurate_nozero | 0.489 | 0.489 | 0.489 | 0.489 | 0.752x | 0.135x |
| shared_b2_fast_nozero | 0.711 | 0.711 | 0.711 | 0.711 | 0.517x | 0.093x |
| shared_b2_accurate_nozero | 0.891 | 0.891 | 0.891 | 0.891 | 0.413x | 0.074x |
| shared_b1_fast_nozero | 0.916 | 0.916 | 0.916 | 0.916 | 0.402x | 0.072x |
| shared_b1_accurate_nozero | 1.484 | 1.484 | 1.484 | 1.484 | 0.248x | 0.045x |

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
| original | output_zero | 0.0060 | 2 | 0.469 |
| original | row_schedule | 0.0000 | 2 | 0.469 |
| original | partial_zero | 0.0010 | 2 | 0.469 |
| original | csr_prefetch | 0.0157 | 2 | 0.469 |
| original | feature_gather_decode | 0.0210 | 2 | 0.469 |
| original | fp32_neighbor_reduction | 0.0000 | 2 | 0.469 |
| original | partial_bf16_conversion | 0.0000 | 2 | 0.469 |
| original | tile_load | 0.1338 | 2 | 0.469 |
| original | tile_compute | 0.1400 | 2 | 0.469 |
| original | tile_store | 0.0010 | 2 | 0.469 |
| original | output_scatter | 0.0000 | 2 | 0.469 |
| original | thread_amx_setup | 0.2170 | 2 | 0.469 |

## tail-n17-q0

N=17, E=0, threads=8, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b1_accurate_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 7.375x | 3.750x |
| shared_b32_fast_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 7.375x | 3.750x |
| shared_b64_accurate_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 7.375x | 3.750x |
| shared_bfull_accurate_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 7.375x | 3.750x |
| shared_b4_fast_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 6.556x | 3.333x |
| shared_b64_fast_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 6.556x | 3.333x |
| shared_b1_fast_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 4.917x | 2.500x |
| shared_b2_fast_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 4.917x | 2.500x |
| shared_b8_fast_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 4.917x | 2.500x |
| shared_b8_accurate_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 4.917x | 2.500x |
| shared_b16_accurate_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 4.917x | 2.500x |
| shared_bfull_fast_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 4.917x | 2.500x |
| shared_b2_accurate_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 4.538x | 2.308x |
| shared_b4_accurate_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 4.538x | 2.308x |
| shared_b32_accurate_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 4.538x | 2.308x |
| replay_b8_fast_nozero | 0.004 | 0.004 | 0.004 | 0.004 | 3.688x | 1.875x |
| shared_b16_fast_nozero | 0.004 | 0.004 | 0.004 | 0.004 | 3.471x | 1.765x |
| replay_b32_fast_nozero | 0.004 | 0.004 | 0.004 | 0.004 | 3.471x | 1.765x |
| replay_bfull_fast_nozero | 0.004 | 0.004 | 0.004 | 0.004 | 3.471x | 1.765x |
| mkl_bf16_inputs | 0.006 | 0.006 | 0.006 | 0.006 | 2.360x | 1.200x |
| original_nozero | 0.006 | 0.006 | 0.006 | 0.006 | 2.360x | 1.200x |
| mkl_fp32 | 0.007 | 0.007 | 0.007 | 0.007 | 1.967x | 1.000x |
| original | 0.014 | 0.014 | 0.014 | 0.014 | 1.000x | 0.508x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b1_fast_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.310x | 2.034x |
| shared_b2_accurate_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.310x | 2.034x |
| shared_b16_fast_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.310x | 2.034x |
| shared_b32_fast_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.310x | 2.034x |
| shared_b32_accurate_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.310x | 2.034x |
| shared_b1_accurate_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.152x | 1.788x |
| shared_b2_fast_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.152x | 1.788x |
| shared_b4_fast_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.152x | 1.788x |
| shared_b4_accurate_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.152x | 1.788x |
| shared_b8_fast_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.152x | 1.788x |
| shared_b16_accurate_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.152x | 1.788x |
| shared_b8_accurate_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.118x | 1.735x |
| shared_b64_fast_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.118x | 1.735x |
| shared_b64_accurate_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.118x | 1.735x |
| shared_bfull_fast_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.118x | 1.735x |
| shared_bfull_accurate_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.118x | 1.735x |
| replay_b8_fast_nozero | 0.009 | 0.009 | 0.009 | 0.009 | 1.027x | 1.595x |
| original | 0.009 | 0.009 | 0.009 | 0.009 | 1.000x | 1.553x |
| original_nozero | 0.009 | 0.009 | 0.009 | 0.009 | 1.000x | 1.553x |
| replay_b32_fast_nozero | 0.009 | 0.009 | 0.009 | 0.009 | 1.000x | 1.553x |
| replay_bfull_fast_nozero | 0.012 | 0.012 | 0.012 | 0.012 | 0.745x | 1.157x |
| mkl_fp32 | 0.014 | 0.014 | 0.014 | 0.014 | 0.644x | 1.000x |
| mkl_bf16_inputs | 0.016 | 0.016 | 0.016 | 0.016 | 0.567x | 0.881x |

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
| original | output_zero | 0.0012 | 2 | 0.051 |
| original | row_schedule | 0.0000 | 2 | 0.051 |
| original | partial_zero | 0.0000 | 2 | 0.051 |
| original | csr_prefetch | 0.0000 | 2 | 0.051 |
| original | feature_gather_decode | 0.0000 | 2 | 0.051 |
| original | fp32_neighbor_reduction | 0.0000 | 2 | 0.051 |
| original | partial_bf16_conversion | 0.0000 | 2 | 0.051 |
| original | tile_load | 0.0000 | 2 | 0.051 |
| original | tile_compute | 0.0000 | 2 | 0.051 |
| original | tile_store | 0.0000 | 2 | 0.051 |
| original | output_scatter | 0.0000 | 2 | 0.051 |
| original | thread_amx_setup | 0.2301 | 2 | 0.051 |

