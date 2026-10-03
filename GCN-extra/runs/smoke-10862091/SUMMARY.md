# GCN-extra first inference experiments

Numerical sum-aggregation experiments with random H/W; no task-accuracy claim.
Uninstrumented medians; original source is unchanged. Speedups are original/new and FP32 MKL/new.
Profiles are sampled summed thread time and do not equal wall time. Physical AMX work is modeled, not a hardware counter.

## smoke

N=37, E=538, threads=8, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 0.011 | 0.009 | 0.010 | 0.013 | 3.625x | 1.927x |
| shared_b64_fast_nozero | 0.013 | 0.013 | 0.013 | 0.014 | 3.080x | 1.637x |
| shared_bfull_accurate_nozero | 0.014 | 0.014 | 0.014 | 0.014 | 2.974x | 1.581x |
| shared_b64_accurate_nozero | 0.014 | 0.014 | 0.014 | 0.015 | 2.876x | 1.529x |
| shared_b32_fast_nozero | 0.015 | 0.014 | 0.014 | 0.015 | 2.852x | 1.516x |
| mkl_bf16_inputs | 0.015 | 0.009 | 0.012 | 0.018 | 2.762x | 1.468x |
| shared_b16_fast_nozero | 0.015 | 0.015 | 0.015 | 0.016 | 2.677x | 1.423x |
| replay_bfull_fast_nozero | 0.018 | 0.018 | 0.018 | 0.019 | 2.245x | 1.194x |
| shared_b16_accurate_nozero | 0.019 | 0.019 | 0.019 | 0.019 | 2.189x | 1.164x |
| shared_b32_accurate_nozero | 0.020 | 0.016 | 0.018 | 0.021 | 2.122x | 1.128x |
| replay_b32_fast_nozero | 0.020 | 0.020 | 0.020 | 0.020 | 2.084x | 1.108x |
| shared_b8_fast_nozero | 0.021 | 0.020 | 0.021 | 0.021 | 1.977x | 1.051x |
| mkl_fp32 | 0.022 | 0.019 | 0.021 | 0.024 | 1.881x | 1.000x |
| replay_b8_fast_nozero | 0.026 | 0.025 | 0.025 | 0.026 | 1.626x | 0.864x |
| shared_b8_accurate_nozero | 0.027 | 0.025 | 0.026 | 0.027 | 1.561x | 0.830x |
| original_nozero | 0.035 | 0.033 | 0.034 | 0.035 | 1.200x | 0.638x |
| shared_b4_fast_nozero | 0.035 | 0.033 | 0.034 | 0.036 | 1.188x | 0.631x |
| original | 0.041 | 0.041 | 0.041 | 0.042 | 1.000x | 0.532x |
| shared_b4_accurate_nozero | 0.042 | 0.040 | 0.041 | 0.043 | 0.989x | 0.526x |
| shared_b2_fast_nozero | 0.043 | 0.043 | 0.043 | 0.044 | 0.956x | 0.508x |
| shared_b1_fast_nozero | 0.061 | 0.059 | 0.060 | 0.062 | 0.680x | 0.361x |
| shared_b2_accurate_nozero | 0.063 | 0.063 | 0.063 | 0.064 | 0.654x | 0.348x |
| shared_b1_accurate_nozero | 0.118 | 0.109 | 0.114 | 0.123 | 0.350x | 0.186x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast_nozero | 0.023 | 0.021 | 0.022 | 0.023 | 3.058x | 1.354x |
| shared_b64_fast_nozero | 0.024 | 0.023 | 0.024 | 0.025 | 2.820x | 1.249x |
| mkl_bf16_inputs | 0.026 | 0.024 | 0.025 | 0.027 | 2.651x | 1.174x |
| shared_bfull_accurate_nozero | 0.027 | 0.027 | 0.027 | 0.027 | 2.558x | 1.133x |
| shared_b16_fast_nozero | 0.030 | 0.027 | 0.028 | 0.031 | 2.331x | 1.032x |
| shared_b64_accurate_nozero | 0.030 | 0.028 | 0.029 | 0.031 | 2.303x | 1.020x |
| mkl_fp32 | 0.031 | 0.025 | 0.028 | 0.033 | 2.258x | 1.000x |
| shared_b32_fast_nozero | 0.032 | 0.030 | 0.031 | 0.033 | 2.157x | 0.955x |
| shared_b32_accurate_nozero | 0.033 | 0.031 | 0.032 | 0.033 | 2.117x | 0.938x |
| replay_b32_fast_nozero | 0.033 | 0.033 | 0.033 | 0.034 | 2.064x | 0.914x |
| replay_bfull_fast_nozero | 0.034 | 0.034 | 0.034 | 0.034 | 2.028x | 0.898x |
| shared_b8_fast_nozero | 0.039 | 0.035 | 0.037 | 0.040 | 1.784x | 0.790x |
| shared_b16_accurate_nozero | 0.039 | 0.034 | 0.037 | 0.042 | 1.762x | 0.780x |
| replay_b8_fast_nozero | 0.047 | 0.042 | 0.044 | 0.049 | 1.467x | 0.650x |
| original_nozero | 0.055 | 0.054 | 0.055 | 0.056 | 1.251x | 0.554x |
| shared_b4_fast_nozero | 0.059 | 0.058 | 0.058 | 0.059 | 1.177x | 0.521x |
| shared_b8_accurate_nozero | 0.059 | 0.055 | 0.057 | 0.061 | 1.170x | 0.518x |
| original | 0.069 | 0.068 | 0.068 | 0.069 | 1.000x | 0.443x |
| shared_b4_accurate_nozero | 0.076 | 0.075 | 0.076 | 0.077 | 0.906x | 0.401x |
| shared_b2_fast_nozero | 0.088 | 0.086 | 0.087 | 0.089 | 0.782x | 0.346x |
| shared_b1_fast_nozero | 0.126 | 0.122 | 0.124 | 0.129 | 0.545x | 0.242x |
| shared_b2_accurate_nozero | 0.137 | 0.125 | 0.131 | 0.143 | 0.503x | 0.223x |
| shared_b1_accurate_nozero | 0.237 | 0.229 | 0.233 | 0.241 | 0.291x | 0.129x |

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
| original | output_zero | 0.0010 | 2 | 0.151 |
| original | row_schedule | 0.0000 | 2 | 0.151 |
| original | partial_zero | 0.0000 | 2 | 0.151 |
| original | csr_prefetch | 0.0038 | 2 | 0.151 |
| original | feature_gather_decode | 0.0012 | 2 | 0.151 |
| original | fp32_neighbor_reduction | 0.0000 | 2 | 0.151 |
| original | partial_bf16_conversion | 0.0000 | 2 | 0.151 |
| original | tile_load | 0.0222 | 2 | 0.151 |
| original | tile_compute | 0.0305 | 2 | 0.151 |
| original | tile_store | 0.0010 | 2 | 0.151 |
| original | output_scatter | 0.0010 | 2 | 0.151 |
| original | thread_amx_setup | 0.3121 | 2 | 0.151 |

## tail-n137-q130

N=137, E=17810, threads=8, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_fp32 | 0.036 | 0.036 | 0.036 | 0.036 | 6.113x | 1.000x |
| mkl_bf16_inputs | 0.042 | 0.042 | 0.042 | 0.042 | 5.244x | 0.858x |
| shared_bfull_accurate_nozero | 0.068 | 0.068 | 0.068 | 0.068 | 3.239x | 0.530x |
| shared_b32_fast_nozero | 0.069 | 0.069 | 0.069 | 0.069 | 3.194x | 0.522x |
| shared_bfull_fast_nozero | 0.069 | 0.069 | 0.069 | 0.069 | 3.183x | 0.521x |
| shared_b64_fast_nozero | 0.079 | 0.079 | 0.079 | 0.079 | 2.780x | 0.455x |
| shared_b64_accurate_nozero | 0.087 | 0.087 | 0.087 | 0.087 | 2.536x | 0.415x |
| shared_b16_fast_nozero | 0.087 | 0.087 | 0.087 | 0.087 | 2.529x | 0.414x |
| shared_b16_accurate_nozero | 0.100 | 0.100 | 0.100 | 0.100 | 2.203x | 0.360x |
| shared_b8_fast_nozero | 0.119 | 0.119 | 0.119 | 0.119 | 1.850x | 0.303x |
| replay_bfull_fast_nozero | 0.123 | 0.123 | 0.123 | 0.123 | 1.789x | 0.293x |
| shared_b8_accurate_nozero | 0.144 | 0.144 | 0.144 | 0.144 | 1.528x | 0.250x |
| shared_b32_accurate_nozero | 0.161 | 0.161 | 0.161 | 0.161 | 1.367x | 0.224x |
| replay_b8_fast_nozero | 0.170 | 0.170 | 0.170 | 0.170 | 1.295x | 0.212x |
| shared_b4_fast_nozero | 0.185 | 0.185 | 0.185 | 0.185 | 1.191x | 0.195x |
| original_nozero | 0.206 | 0.206 | 0.206 | 0.206 | 1.068x | 0.175x |
| replay_b32_fast_nozero | 0.215 | 0.215 | 0.215 | 0.215 | 1.023x | 0.167x |
| original | 0.220 | 0.220 | 0.220 | 0.220 | 1.000x | 0.164x |
| shared_b4_accurate_nozero | 0.238 | 0.238 | 0.238 | 0.238 | 0.924x | 0.151x |
| shared_b2_fast_nozero | 0.294 | 0.294 | 0.294 | 0.294 | 0.749x | 0.122x |
| shared_b2_accurate_nozero | 0.397 | 0.397 | 0.397 | 0.397 | 0.554x | 0.091x |
| shared_b1_fast_nozero | 0.432 | 0.432 | 0.432 | 0.432 | 0.509x | 0.083x |
| shared_b1_accurate_nozero | 0.805 | 0.805 | 0.805 | 0.805 | 0.273x | 0.045x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_fp32 | 0.070 | 0.070 | 0.070 | 0.070 | 5.105x | 1.000x |
| mkl_bf16_inputs | 0.076 | 0.076 | 0.076 | 0.076 | 4.705x | 0.922x |
| shared_bfull_fast_nozero | 0.135 | 0.135 | 0.135 | 0.135 | 2.652x | 0.519x |
| shared_b64_fast_nozero | 0.138 | 0.138 | 0.138 | 0.138 | 2.592x | 0.508x |
| shared_bfull_accurate_nozero | 0.141 | 0.141 | 0.141 | 0.141 | 2.540x | 0.497x |
| shared_b32_fast_nozero | 0.145 | 0.145 | 0.145 | 0.145 | 2.469x | 0.484x |
| shared_b64_accurate_nozero | 0.153 | 0.153 | 0.153 | 0.153 | 2.338x | 0.458x |
| shared_b16_fast_nozero | 0.192 | 0.192 | 0.192 | 0.192 | 1.862x | 0.365x |
| shared_b16_accurate_nozero | 0.204 | 0.204 | 0.204 | 0.204 | 1.754x | 0.343x |
| shared_b32_accurate_nozero | 0.205 | 0.205 | 0.205 | 0.205 | 1.745x | 0.342x |
| replay_b32_fast_nozero | 0.239 | 0.239 | 0.239 | 0.239 | 1.498x | 0.293x |
| shared_b8_fast_nozero | 0.258 | 0.258 | 0.258 | 0.258 | 1.387x | 0.272x |
| replay_bfull_fast_nozero | 0.262 | 0.262 | 0.262 | 0.262 | 1.366x | 0.268x |
| shared_b8_accurate_nozero | 0.298 | 0.298 | 0.298 | 0.298 | 1.201x | 0.235x |
| replay_b8_fast_nozero | 0.349 | 0.349 | 0.349 | 0.349 | 1.025x | 0.201x |
| original_nozero | 0.357 | 0.357 | 0.357 | 0.357 | 1.002x | 0.196x |
| original | 0.358 | 0.358 | 0.358 | 0.358 | 1.000x | 0.196x |
| shared_b4_fast_nozero | 0.420 | 0.420 | 0.420 | 0.420 | 0.852x | 0.167x |
| shared_b4_accurate_nozero | 0.479 | 0.479 | 0.479 | 0.479 | 0.747x | 0.146x |
| shared_b2_fast_nozero | 0.595 | 0.595 | 0.595 | 0.595 | 0.601x | 0.118x |
| shared_b1_fast_nozero | 0.846 | 0.846 | 0.846 | 0.846 | 0.423x | 0.083x |
| shared_b2_accurate_nozero | 0.929 | 0.929 | 0.929 | 0.929 | 0.385x | 0.075x |
| shared_b1_accurate_nozero | 1.471 | 1.471 | 1.471 | 1.471 | 0.243x | 0.048x |

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
| original | output_zero | 0.0041 | 2 | 0.476 |
| original | row_schedule | 0.0000 | 2 | 0.476 |
| original | partial_zero | 0.0010 | 2 | 0.476 |
| original | csr_prefetch | 0.0315 | 2 | 0.476 |
| original | feature_gather_decode | 0.0298 | 2 | 0.476 |
| original | fp32_neighbor_reduction | 0.0000 | 2 | 0.476 |
| original | partial_bf16_conversion | 0.0000 | 2 | 0.476 |
| original | tile_load | 0.1674 | 2 | 0.476 |
| original | tile_compute | 0.1597 | 2 | 0.476 |
| original | tile_store | 0.0000 | 2 | 0.476 |
| original | output_scatter | 0.0010 | 2 | 0.476 |
| original | thread_amx_setup | 0.3295 | 2 | 0.476 |

## tail-n17-q0

N=17, E=0, threads=8, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b2_accurate_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 3.750x | 4.250x |
| shared_b8_fast_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 3.750x | 4.250x |
| shared_b8_accurate_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 3.750x | 4.250x |
| shared_b16_fast_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 3.750x | 4.250x |
| shared_bfull_accurate_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 3.750x | 4.250x |
| shared_b1_fast_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 3.333x | 3.778x |
| shared_b4_fast_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 3.333x | 3.778x |
| shared_b4_accurate_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 3.333x | 3.778x |
| shared_b16_accurate_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 3.333x | 3.778x |
| shared_b64_accurate_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 3.333x | 3.778x |
| shared_bfull_fast_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 3.333x | 3.778x |
| replay_bfull_fast_nozero | 0.002 | 0.002 | 0.002 | 0.002 | 3.333x | 3.778x |
| replay_b8_fast_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 2.500x | 2.833x |
| shared_b32_accurate_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 2.500x | 2.833x |
| shared_b1_accurate_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 2.308x | 2.615x |
| shared_b32_fast_nozero | 0.003 | 0.003 | 0.003 | 0.003 | 2.308x | 2.615x |
| shared_b2_fast_nozero | 0.004 | 0.004 | 0.004 | 0.004 | 1.875x | 2.125x |
| original_nozero | 0.004 | 0.004 | 0.004 | 0.004 | 1.765x | 2.000x |
| shared_b64_fast_nozero | 0.004 | 0.004 | 0.004 | 0.004 | 1.765x | 2.000x |
| original | 0.007 | 0.007 | 0.007 | 0.007 | 1.000x | 1.133x |
| mkl_fp32 | 0.008 | 0.008 | 0.008 | 0.008 | 0.882x | 1.000x |
| mkl_bf16_inputs | 0.009 | 0.009 | 0.009 | 0.009 | 0.789x | 0.895x |
| replay_b32_fast_nozero | 0.022 | 0.022 | 0.022 | 0.022 | 0.326x | 0.370x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b2_accurate_nozero | 0.005 | 0.005 | 0.005 | 0.005 | 2.619x | 3.429x |
| shared_b4_fast_nozero | 0.006 | 0.006 | 0.006 | 0.006 | 2.200x | 2.880x |
| shared_b8_fast_nozero | 0.006 | 0.006 | 0.006 | 0.006 | 2.200x | 2.880x |
| shared_b8_accurate_nozero | 0.006 | 0.006 | 0.006 | 0.006 | 2.200x | 2.880x |
| shared_b32_accurate_nozero | 0.006 | 0.006 | 0.006 | 0.006 | 2.115x | 2.769x |
| shared_b4_accurate_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.897x | 2.483x |
| shared_b2_fast_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.833x | 2.400x |
| shared_b16_accurate_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.833x | 2.400x |
| shared_b32_fast_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.833x | 2.400x |
| shared_b64_accurate_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.833x | 2.400x |
| shared_bfull_fast_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.833x | 2.400x |
| shared_bfull_accurate_nozero | 0.007 | 0.007 | 0.007 | 0.007 | 1.833x | 2.400x |
| shared_b1_fast_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.618x | 2.118x |
| shared_b1_accurate_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.618x | 2.118x |
| shared_b16_fast_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.618x | 2.118x |
| shared_b64_fast_nozero | 0.008 | 0.008 | 0.008 | 0.008 | 1.618x | 2.118x |
| replay_b8_fast_nozero | 0.009 | 0.009 | 0.009 | 0.009 | 1.447x | 1.895x |
| original_nozero | 0.010 | 0.010 | 0.010 | 0.010 | 1.341x | 1.756x |
| replay_bfull_fast_nozero | 0.010 | 0.010 | 0.010 | 0.010 | 1.310x | 1.714x |
| original | 0.013 | 0.013 | 0.013 | 0.013 | 1.000x | 1.309x |
| mkl_bf16_inputs | 0.014 | 0.014 | 0.014 | 0.014 | 0.932x | 1.220x |
| mkl_fp32 | 0.017 | 0.017 | 0.017 | 0.017 | 0.764x | 1.000x |
| replay_b32_fast_nozero | 0.023 | 0.023 | 0.023 | 0.023 | 0.573x | 0.750x |

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
| original | output_zero | 0.0000 | 2 | 0.009 |
| original | row_schedule | 0.0000 | 2 | 0.009 |
| original | partial_zero | 0.0019 | 2 | 0.009 |
| original | csr_prefetch | 0.0000 | 2 | 0.009 |
| original | feature_gather_decode | 0.0000 | 2 | 0.009 |
| original | fp32_neighbor_reduction | 0.0000 | 2 | 0.009 |
| original | partial_bf16_conversion | 0.0000 | 2 | 0.009 |
| original | tile_load | 0.0000 | 2 | 0.009 |
| original | tile_compute | 0.0000 | 2 | 0.009 |
| original | tile_store | 0.0000 | 2 | 0.009 |
| original | output_scatter | 0.0010 | 2 | 0.009 |
| original | thread_amx_setup | 0.0162 | 2 | 0.009 |

