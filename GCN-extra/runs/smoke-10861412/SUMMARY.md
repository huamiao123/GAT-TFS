# GCN-extra first inference experiments

Numerical sum-aggregation experiments with random H/W; no task-accuracy claim.
Uninstrumented medians; original source is unchanged. Speedups are original/new and FP32 MKL/new.
Profiles are sampled summed thread time and do not equal wall time. Physical AMX work is modeled, not a hardware counter.

## smoke

N=37, E=538, threads=8, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_bf16_inputs | 0.008 | 0.008 | 0.008 | 0.009 | 4.314x | 1.086x |
| mkl_fp32 | 0.009 | 0.009 | 0.009 | 0.009 | 3.974x | 1.000x |
| shared_bfull_fast | 0.015 | 0.013 | 0.014 | 0.015 | 2.455x | 0.618x |
| shared_b64_accurate | 0.016 | 0.015 | 0.015 | 0.016 | 2.254x | 0.567x |
| shared_b64_fast | 0.017 | 0.016 | 0.017 | 0.018 | 2.112x | 0.531x |
| shared_bfull_accurate | 0.017 | 0.017 | 0.017 | 0.017 | 2.112x | 0.531x |
| shared_b32_fast | 0.019 | 0.016 | 0.017 | 0.020 | 1.899x | 0.478x |
| replay_bfull_fast | 0.021 | 0.020 | 0.020 | 0.021 | 1.756x | 0.442x |
| shared_b16_fast | 0.021 | 0.017 | 0.019 | 0.022 | 1.746x | 0.439x |
| shared_b32_accurate | 0.021 | 0.018 | 0.019 | 0.022 | 1.746x | 0.439x |
| shared_b8_fast | 0.021 | 0.021 | 0.021 | 0.021 | 1.706x | 0.429x |
| replay_b32_fast | 0.022 | 0.019 | 0.020 | 0.023 | 1.669x | 0.420x |
| shared_b16_accurate | 0.023 | 0.023 | 0.023 | 0.023 | 1.565x | 0.394x |
| replay_b8_fast | 0.025 | 0.024 | 0.025 | 0.026 | 1.438x | 0.362x |
| shared_b8_accurate | 0.026 | 0.023 | 0.024 | 0.028 | 1.385x | 0.349x |
| shared_b4_fast | 0.031 | 0.030 | 0.030 | 0.031 | 1.180x | 0.297x |
| original | 0.036 | 0.035 | 0.036 | 0.036 | 1.000x | 0.252x |
| shared_b2_fast | 0.044 | 0.043 | 0.043 | 0.044 | 0.825x | 0.208x |
| shared_b4_accurate | 0.054 | 0.040 | 0.047 | 0.060 | 0.673x | 0.169x |
| shared_b1_fast | 0.070 | 0.065 | 0.068 | 0.073 | 0.511x | 0.129x |
| shared_b2_accurate | 0.075 | 0.073 | 0.074 | 0.077 | 0.477x | 0.120x |
| shared_b1_accurate | 0.111 | 0.110 | 0.110 | 0.111 | 0.325x | 0.082x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_fp32 | 0.020 | 0.020 | 0.020 | 0.020 | 3.946x | 1.000x |
| mkl_bf16_inputs | 0.022 | 0.021 | 0.021 | 0.022 | 3.663x | 0.928x |
| shared_bfull_fast | 0.033 | 0.032 | 0.032 | 0.033 | 2.429x | 0.615x |
| shared_b64_fast | 0.033 | 0.032 | 0.032 | 0.033 | 2.402x | 0.609x |
| shared_b32_fast | 0.033 | 0.031 | 0.032 | 0.035 | 2.359x | 0.598x |
| shared_bfull_accurate | 0.034 | 0.033 | 0.034 | 0.035 | 2.318x | 0.587x |
| shared_b64_accurate | 0.035 | 0.034 | 0.035 | 0.036 | 2.232x | 0.566x |
| shared_b16_fast | 0.038 | 0.037 | 0.037 | 0.039 | 2.078x | 0.527x |
| shared_b32_accurate | 0.039 | 0.034 | 0.036 | 0.041 | 2.053x | 0.520x |
| replay_bfull_fast | 0.041 | 0.040 | 0.040 | 0.041 | 1.950x | 0.494x |
| replay_b32_fast | 0.046 | 0.046 | 0.046 | 0.046 | 1.718x | 0.435x |
| shared_b16_accurate | 0.047 | 0.045 | 0.046 | 0.048 | 1.678x | 0.425x |
| shared_b8_fast | 0.047 | 0.047 | 0.047 | 0.048 | 1.666x | 0.422x |
| replay_b8_fast | 0.056 | 0.054 | 0.055 | 0.056 | 1.423x | 0.361x |
| shared_b8_accurate | 0.057 | 0.055 | 0.056 | 0.057 | 1.399x | 0.354x |
| shared_b4_fast | 0.062 | 0.059 | 0.061 | 0.064 | 1.265x | 0.321x |
| original | 0.079 | 0.078 | 0.078 | 0.080 | 1.000x | 0.253x |
| shared_b4_accurate | 0.087 | 0.085 | 0.086 | 0.089 | 0.905x | 0.229x |
| shared_b2_fast | 0.095 | 0.093 | 0.094 | 0.095 | 0.836x | 0.212x |
| shared_b1_fast | 0.131 | 0.127 | 0.129 | 0.134 | 0.601x | 0.152x |
| shared_b2_accurate | 0.137 | 0.134 | 0.135 | 0.138 | 0.577x | 0.146x |
| shared_b1_accurate | 0.226 | 0.225 | 0.226 | 0.226 | 0.350x | 0.089x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.672e-03 | 3.386e-01 | 5.527e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.497e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 2.297e-07 | 2.289e-05 | 3.736e-07 | 1 |
| original | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.496e-03 | 1 |
| shared_b1_fast | kernel_full | mkl_bf16_inputs | 2.297e-07 | 2.289e-05 | 3.736e-07 | 1 |
| shared_b1_fast | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.496e-03 | 1 |
| shared_b1_fast | b1_identity | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_accurate | kernel_full | mkl_bf16_inputs | 2.297e-07 | 2.289e-05 | 3.736e-07 | 1 |
| shared_b1_accurate | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.496e-03 | 1 |
| shared_b2_fast | kernel_full | mkl_bf16_inputs | 2.152e-03 | 1.292e-01 | 2.109e-03 | 1 |
| shared_b2_fast | kernel_full | mkl_fp32 | 7.156e-03 | 4.542e-01 | 7.373e-03 | 1 |
| shared_b2_accurate | kernel_full | mkl_bf16_inputs | 2.311e-07 | 2.670e-05 | 4.359e-07 | 1 |
| shared_b2_accurate | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.497e-03 | 1 |
| shared_b4_fast | kernel_full | mkl_bf16_inputs | 2.933e-03 | 2.015e-01 | 3.289e-03 | 1 |
| shared_b4_fast | kernel_full | mkl_fp32 | 8.031e-03 | 5.265e-01 | 8.547e-03 | 1 |
| shared_b4_accurate | kernel_full | mkl_bf16_inputs | 2.025e-07 | 2.289e-05 | 3.736e-07 | 1 |
| shared_b4_accurate | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.497e-03 | 1 |
| shared_b8_fast | kernel_full | mkl_bf16_inputs | 3.096e-03 | 1.797e-01 | 2.933e-03 | 1 |
| shared_b8_fast | kernel_full | mkl_fp32 | 8.226e-03 | 5.182e-01 | 8.413e-03 | 1 |
| shared_b8_accurate | kernel_full | mkl_bf16_inputs | 1.905e-07 | 2.289e-05 | 3.736e-07 | 1 |
| shared_b8_accurate | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.497e-03 | 1 |
| replay_b8_fast | kernel_full | mkl_bf16_inputs | 3.096e-03 | 1.797e-01 | 2.933e-03 | 1 |
| replay_b8_fast | kernel_full | mkl_fp32 | 8.226e-03 | 5.182e-01 | 8.413e-03 | 1 |
| shared_b16_fast | kernel_full | mkl_bf16_inputs | 3.222e-03 | 1.745e-01 | 2.848e-03 | 1 |
| shared_b16_fast | kernel_full | mkl_fp32 | 8.348e-03 | 5.071e-01 | 8.232e-03 | 1 |
| shared_b16_accurate | kernel_full | mkl_bf16_inputs | 1.620e-06 | 1.183e-04 | 1.930e-06 | 1 |
| shared_b16_accurate | kernel_full | mkl_fp32 | 5.642e-03 | 3.386e-01 | 5.497e-03 | 1 |
| shared_b32_fast | kernel_full | mkl_bf16_inputs | 3.313e-03 | 2.092e-01 | 3.415e-03 | 1 |
| shared_b32_fast | kernel_full | mkl_fp32 | 8.417e-03 | 5.348e-01 | 8.682e-03 | 1 |
| shared_b32_accurate | kernel_full | mkl_bf16_inputs | 3.102e-06 | 2.365e-04 | 3.861e-06 | 1 |
| shared_b32_accurate | kernel_full | mkl_fp32 | 5.643e-03 | 3.386e-01 | 5.497e-03 | 1 |
| replay_b32_fast | kernel_full | mkl_bf16_inputs | 3.313e-03 | 2.092e-01 | 3.415e-03 | 1 |
| replay_b32_fast | kernel_full | mkl_fp32 | 8.417e-03 | 5.348e-01 | 8.682e-03 | 1 |
| shared_b64_fast | kernel_full | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| shared_b64_fast | kernel_full | mkl_fp32 | 8.453e-03 | 5.256e-01 | 8.534e-03 | 1 |
| shared_b64_accurate | kernel_full | mkl_bf16_inputs | 4.007e-06 | 2.871e-04 | 4.686e-06 | 1 |
| shared_b64_accurate | kernel_full | mkl_fp32 | 5.643e-03 | 3.386e-01 | 5.497e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_fp32 | 8.453e-03 | 5.256e-01 | 8.534e-03 | 1 |
| shared_bfull_accurate | kernel_full | mkl_bf16_inputs | 4.007e-06 | 2.871e-04 | 4.686e-06 | 1 |
| shared_bfull_accurate | kernel_full | mkl_fp32 | 5.643e-03 | 3.386e-01 | 5.497e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_fp32 | 8.453e-03 | 5.256e-01 | 8.534e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_fast | instrumented_kernel | mkl_bf16_inputs | 2.297e-07 | 2.289e-05 | 3.736e-07 | 1 |
| shared_b1_accurate | instrumented_kernel | mkl_bf16_inputs | 2.297e-07 | 2.289e-05 | 3.736e-07 | 1 |
| shared_b2_fast | instrumented_kernel | mkl_bf16_inputs | 2.152e-03 | 1.292e-01 | 2.109e-03 | 1 |
| shared_b2_accurate | instrumented_kernel | mkl_bf16_inputs | 2.311e-07 | 2.670e-05 | 4.359e-07 | 1 |
| shared_b4_fast | instrumented_kernel | mkl_bf16_inputs | 2.933e-03 | 2.015e-01 | 3.289e-03 | 1 |
| shared_b4_accurate | instrumented_kernel | mkl_bf16_inputs | 2.025e-07 | 2.289e-05 | 3.736e-07 | 1 |
| shared_b8_fast | instrumented_kernel | mkl_bf16_inputs | 3.096e-03 | 1.797e-01 | 2.933e-03 | 1 |
| shared_b8_accurate | instrumented_kernel | mkl_bf16_inputs | 1.905e-07 | 2.289e-05 | 3.736e-07 | 1 |
| replay_b8_fast | instrumented_kernel | mkl_bf16_inputs | 3.096e-03 | 1.797e-01 | 2.933e-03 | 1 |
| shared_b16_fast | instrumented_kernel | mkl_bf16_inputs | 3.222e-03 | 1.745e-01 | 2.848e-03 | 1 |
| shared_b16_accurate | instrumented_kernel | mkl_bf16_inputs | 1.620e-06 | 1.183e-04 | 1.930e-06 | 1 |
| shared_b32_fast | instrumented_kernel | mkl_bf16_inputs | 3.313e-03 | 2.092e-01 | 3.415e-03 | 1 |
| shared_b32_accurate | instrumented_kernel | mkl_bf16_inputs | 3.102e-06 | 2.365e-04 | 3.861e-06 | 1 |
| replay_b32_fast | instrumented_kernel | mkl_bf16_inputs | 3.313e-03 | 2.092e-01 | 3.415e-03 | 1 |
| shared_b64_fast | instrumented_kernel | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| shared_b64_accurate | instrumented_kernel | mkl_bf16_inputs | 4.007e-06 | 2.871e-04 | 4.686e-06 | 1 |
| shared_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| shared_bfull_accurate | instrumented_kernel | mkl_bf16_inputs | 4.007e-06 | 2.871e-04 | 4.686e-06 | 1 |
| replay_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.331e-03 | 2.000e-01 | 3.266e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.095e-02 | 3.557e+01 | 1.028e-02 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 2.519e-07 | 1.709e-03 | 4.939e-07 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| shared_b1_fast | two_layer_e2e | mkl_bf16_inputs | 2.519e-07 | 1.709e-03 | 4.939e-07 | 1 |
| shared_b1_fast | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| shared_b1_accurate | two_layer_e2e | mkl_bf16_inputs | 2.519e-07 | 1.709e-03 | 4.939e-07 | 1 |
| shared_b1_accurate | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| shared_b2_fast | two_layer_e2e | mkl_bf16_inputs | 2.807e-03 | 1.079e+01 | 3.119e-03 | 1 |
| shared_b2_fast | two_layer_e2e | mkl_fp32 | 1.341e-02 | 4.503e+01 | 1.289e-02 | 1 |
| shared_b2_accurate | two_layer_e2e | mkl_bf16_inputs | 6.174e-05 | 1.245e-01 | 3.598e-05 | 1 |
| shared_b2_accurate | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.565e+01 | 1.020e-02 | 1 |
| shared_b4_fast | two_layer_e2e | mkl_bf16_inputs | 4.584e-03 | 1.668e+01 | 4.820e-03 | 1 |
| shared_b4_fast | two_layer_e2e | mkl_fp32 | 1.520e-02 | 5.158e+01 | 1.476e-02 | 1 |
| shared_b4_accurate | two_layer_e2e | mkl_bf16_inputs | 3.501e-07 | 2.197e-03 | 6.350e-07 | 1 |
| shared_b4_accurate | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| shared_b8_fast | two_layer_e2e | mkl_bf16_inputs | 5.076e-03 | 1.909e+01 | 5.516e-03 | 1 |
| shared_b8_fast | two_layer_e2e | mkl_fp32 | 1.566e-02 | 5.399e+01 | 1.545e-02 | 1 |
| shared_b8_accurate | two_layer_e2e | mkl_bf16_inputs | 7.603e-07 | 3.174e-03 | 9.172e-07 | 1 |
| shared_b8_accurate | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| replay_b8_fast | two_layer_e2e | mkl_bf16_inputs | 5.076e-03 | 1.909e+01 | 5.516e-03 | 1 |
| replay_b8_fast | two_layer_e2e | mkl_fp32 | 1.566e-02 | 5.399e+01 | 1.545e-02 | 1 |
| shared_b16_fast | two_layer_e2e | mkl_bf16_inputs | 5.250e-03 | 2.091e+01 | 6.043e-03 | 1 |
| shared_b16_fast | two_layer_e2e | mkl_fp32 | 1.579e-02 | 5.519e+01 | 1.579e-02 | 1 |
| shared_b16_accurate | two_layer_e2e | mkl_bf16_inputs | 2.137e-06 | 1.196e-02 | 3.457e-06 | 1 |
| shared_b16_accurate | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.557e+01 | 1.018e-02 | 1 |
| shared_b32_fast | two_layer_e2e | mkl_bf16_inputs | 5.423e-03 | 2.186e+01 | 6.316e-03 | 1 |
| shared_b32_fast | two_layer_e2e | mkl_fp32 | 1.594e-02 | 5.530e+01 | 1.582e-02 | 1 |
| shared_b32_accurate | two_layer_e2e | mkl_bf16_inputs | 6.183e-05 | 1.436e-01 | 4.149e-05 | 1 |
| shared_b32_accurate | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.567e+01 | 1.021e-02 | 1 |
| replay_b32_fast | two_layer_e2e | mkl_bf16_inputs | 5.423e-03 | 2.186e+01 | 6.316e-03 | 1 |
| replay_b32_fast | two_layer_e2e | mkl_fp32 | 1.594e-02 | 5.530e+01 | 1.582e-02 | 1 |
| shared_b64_fast | two_layer_e2e | mkl_bf16_inputs | 5.281e-03 | 1.925e+01 | 5.564e-03 | 1 |
| shared_b64_fast | two_layer_e2e | mkl_fp32 | 1.578e-02 | 5.186e+01 | 1.484e-02 | 1 |
| shared_b64_accurate | two_layer_e2e | mkl_bf16_inputs | 6.184e-05 | 1.443e-01 | 4.170e-05 | 1 |
| shared_b64_accurate | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.567e+01 | 1.021e-02 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.281e-03 | 1.925e+01 | 5.564e-03 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_fp32 | 1.578e-02 | 5.186e+01 | 1.484e-02 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_bf16_inputs | 6.184e-05 | 1.443e-01 | 4.170e-05 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_fp32 | 1.083e-02 | 3.567e+01 | 1.021e-02 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.281e-03 | 1.925e+01 | 5.564e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_fp32 | 1.578e-02 | 5.186e+01 | 1.484e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 0.0010 | 2 | 0.134 |
| original | row_schedule | 0.0012 | 2 | 0.134 |
| original | partial_zero | 0.0000 | 2 | 0.134 |
| original | csr_prefetch | 0.0038 | 2 | 0.134 |
| original | feature_gather_decode | 0.0031 | 2 | 0.134 |
| original | fp32_neighbor_reduction | 0.0000 | 2 | 0.134 |
| original | partial_bf16_conversion | 0.0000 | 2 | 0.134 |
| original | tile_load | 0.0229 | 2 | 0.134 |
| original | tile_compute | 0.0267 | 2 | 0.134 |
| original | tile_store | 0.0000 | 2 | 0.134 |
| original | output_scatter | 0.0012 | 2 | 0.134 |
| original | thread_amx_setup | 0.4773 | 2 | 0.134 |
| shared_b8_fast | output_zero | 0.0012 | 2 | 0.079 |
| shared_b8_fast | row_schedule | 0.0012 | 2 | 0.079 |
| shared_b8_fast | partial_zero | 0.0000 | 2 | 0.079 |
| shared_b8_fast | csr_prefetch | 0.0081 | 2 | 0.079 |
| shared_b8_fast | feature_gather_decode | 0.0079 | 2 | 0.079 |
| shared_b8_fast | fp32_neighbor_reduction | 0.0062 | 2 | 0.079 |
| shared_b8_fast | partial_bf16_conversion | 0.0010 | 2 | 0.079 |
| shared_b8_fast | tile_load | 0.0060 | 2 | 0.079 |
| shared_b8_fast | tile_compute | 0.0064 | 2 | 0.079 |
| shared_b8_fast | tile_store | 0.0010 | 2 | 0.079 |
| shared_b8_fast | output_scatter | 0.0000 | 2 | 0.079 |
| shared_b8_fast | thread_amx_setup | 0.0029 | 2 | 0.079 |
| shared_bfull_fast | output_zero | 0.0010 | 2 | 0.059 |
| shared_bfull_fast | row_schedule | 0.0010 | 2 | 0.059 |
| shared_bfull_fast | partial_zero | 0.0010 | 2 | 0.059 |
| shared_bfull_fast | csr_prefetch | 0.0060 | 2 | 0.059 |
| shared_bfull_fast | feature_gather_decode | 0.0041 | 2 | 0.059 |
| shared_bfull_fast | fp32_neighbor_reduction | 0.0081 | 2 | 0.059 |
| shared_bfull_fast | partial_bf16_conversion | 0.0000 | 2 | 0.059 |
| shared_bfull_fast | tile_load | 0.0021 | 2 | 0.059 |
| shared_bfull_fast | tile_compute | 0.0041 | 2 | 0.059 |
| shared_bfull_fast | tile_store | 0.0000 | 2 | 0.059 |
| shared_bfull_fast | output_scatter | 0.0000 | 2 | 0.059 |
| shared_bfull_fast | thread_amx_setup | 0.0000 | 2 | 0.059 |
| shared_bfull_accurate | output_zero | 0.0012 | 2 | 0.061 |
| shared_bfull_accurate | row_schedule | 0.0000 | 2 | 0.061 |
| shared_bfull_accurate | partial_zero | 0.0010 | 2 | 0.061 |
| shared_bfull_accurate | csr_prefetch | 0.0074 | 2 | 0.061 |
| shared_bfull_accurate | feature_gather_decode | 0.0050 | 2 | 0.061 |
| shared_bfull_accurate | fp32_neighbor_reduction | 0.0038 | 2 | 0.061 |
| shared_bfull_accurate | partial_bf16_conversion | 0.0021 | 2 | 0.061 |
| shared_bfull_accurate | tile_load | 0.0010 | 2 | 0.061 |
| shared_bfull_accurate | tile_compute | 0.0038 | 2 | 0.061 |
| shared_bfull_accurate | tile_store | 0.0000 | 2 | 0.061 |
| shared_bfull_accurate | output_scatter | 0.0010 | 2 | 0.061 |
| shared_bfull_accurate | thread_amx_setup | 0.0000 | 2 | 0.061 |
| replay_bfull_fast | output_zero | 0.0010 | 2 | 0.112 |
| replay_bfull_fast | row_schedule | 0.0010 | 2 | 0.112 |
| replay_bfull_fast | partial_zero | 0.0010 | 2 | 0.112 |
| replay_bfull_fast | csr_prefetch | 0.0136 | 2 | 0.112 |
| replay_bfull_fast | feature_gather_decode | 0.0210 | 2 | 0.112 |
| replay_bfull_fast | fp32_neighbor_reduction | 0.0079 | 2 | 0.112 |
| replay_bfull_fast | partial_bf16_conversion | 0.0010 | 2 | 0.112 |
| replay_bfull_fast | tile_load | 0.0000 | 2 | 0.112 |
| replay_bfull_fast | tile_compute | 0.0103 | 2 | 0.112 |
| replay_bfull_fast | tile_store | 0.0000 | 2 | 0.112 |
| replay_bfull_fast | output_scatter | 0.0010 | 2 | 0.112 |
| replay_bfull_fast | thread_amx_setup | 0.0019 | 2 | 0.112 |

