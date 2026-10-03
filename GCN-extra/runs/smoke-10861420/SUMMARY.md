# GCN-extra first inference experiments

Numerical sum-aggregation experiments with random H/W; no task-accuracy claim.
Uninstrumented medians; original source is unchanged. Speedups are original/new and FP32 MKL/new.
Profiles are sampled summed thread time and do not equal wall time. Physical AMX work is modeled, not a hardware counter.

## smoke

N=37, E=538, threads=8, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_fp32 | 0.008 | 0.008 | 0.008 | 0.008 | 4.894x | 1.000x |
| mkl_bf16_inputs | 0.008 | 0.008 | 0.008 | 0.009 | 4.549x | 0.930x |
| shared_b64_accurate | 0.014 | 0.012 | 0.013 | 0.016 | 2.669x | 0.545x |
| shared_bfull_fast | 0.015 | 0.015 | 0.015 | 0.016 | 2.485x | 0.508x |
| shared_b64_fast | 0.016 | 0.016 | 0.016 | 0.016 | 2.410x | 0.493x |
| shared_bfull_accurate | 0.017 | 0.017 | 0.017 | 0.018 | 2.212x | 0.452x |
| shared_b32_fast | 0.018 | 0.018 | 0.018 | 0.018 | 2.139x | 0.437x |
| shared_b32_accurate | 0.018 | 0.017 | 0.018 | 0.019 | 2.125x | 0.434x |
| shared_b16_fast | 0.019 | 0.019 | 0.019 | 0.019 | 2.019x | 0.413x |
| replay_bfull_fast | 0.021 | 0.021 | 0.021 | 0.021 | 1.835x | 0.375x |
| replay_b32_fast | 0.021 | 0.021 | 0.021 | 0.022 | 1.794x | 0.367x |
| shared_b16_accurate | 0.022 | 0.019 | 0.021 | 0.024 | 1.746x | 0.357x |
| shared_b8_fast | 0.023 | 0.023 | 0.023 | 0.023 | 1.682x | 0.344x |
| shared_b8_accurate | 0.028 | 0.027 | 0.027 | 0.029 | 1.374x | 0.281x |
| replay_b8_fast | 0.029 | 0.028 | 0.029 | 0.030 | 1.324x | 0.270x |
| shared_b4_fast | 0.031 | 0.031 | 0.031 | 0.032 | 1.223x | 0.250x |
| original | 0.039 | 0.038 | 0.038 | 0.039 | 1.000x | 0.204x |
| shared_b4_accurate | 0.039 | 0.037 | 0.038 | 0.040 | 0.988x | 0.202x |
| shared_b2_fast | 0.044 | 0.043 | 0.043 | 0.044 | 0.885x | 0.181x |
| shared_b1_fast | 0.061 | 0.061 | 0.061 | 0.061 | 0.632x | 0.129x |
| shared_b2_accurate | 0.066 | 0.065 | 0.065 | 0.066 | 0.587x | 0.120x |
| shared_b1_accurate | 0.124 | 0.122 | 0.123 | 0.125 | 0.310x | 0.063x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_fp32 | 0.020 | 0.020 | 0.020 | 0.021 | 3.386x | 1.000x |
| mkl_bf16_inputs | 0.022 | 0.021 | 0.021 | 0.022 | 3.199x | 0.945x |
| shared_b64_fast | 0.026 | 0.025 | 0.026 | 0.027 | 2.620x | 0.774x |
| shared_bfull_fast | 0.029 | 0.028 | 0.028 | 0.029 | 2.412x | 0.712x |
| shared_b64_accurate | 0.031 | 0.029 | 0.030 | 0.033 | 2.193x | 0.648x |
| shared_bfull_accurate | 0.034 | 0.033 | 0.034 | 0.034 | 2.032x | 0.600x |
| shared_b32_fast | 0.035 | 0.034 | 0.035 | 0.036 | 1.969x | 0.582x |
| shared_b16_fast | 0.037 | 0.033 | 0.035 | 0.040 | 1.844x | 0.545x |
| shared_b32_accurate | 0.038 | 0.035 | 0.036 | 0.039 | 1.838x | 0.543x |
| replay_bfull_fast | 0.039 | 0.036 | 0.037 | 0.040 | 1.793x | 0.529x |
| shared_b16_accurate | 0.040 | 0.039 | 0.039 | 0.040 | 1.728x | 0.510x |
| replay_b32_fast | 0.041 | 0.039 | 0.040 | 0.043 | 1.664x | 0.491x |
| shared_b8_fast | 0.045 | 0.041 | 0.043 | 0.047 | 1.532x | 0.452x |
| replay_b8_fast | 0.055 | 0.054 | 0.055 | 0.056 | 1.245x | 0.368x |
| shared_b8_accurate | 0.057 | 0.056 | 0.056 | 0.057 | 1.222x | 0.361x |
| shared_b4_fast | 0.061 | 0.060 | 0.061 | 0.062 | 1.131x | 0.334x |
| original | 0.069 | 0.062 | 0.066 | 0.073 | 1.000x | 0.295x |
| shared_b4_accurate | 0.085 | 0.081 | 0.083 | 0.086 | 0.815x | 0.241x |
| shared_b2_fast | 0.087 | 0.084 | 0.085 | 0.089 | 0.793x | 0.234x |
| shared_b1_fast | 0.124 | 0.117 | 0.121 | 0.127 | 0.557x | 0.164x |
| shared_b2_accurate | 0.139 | 0.130 | 0.135 | 0.144 | 0.495x | 0.146x |
| shared_b1_accurate | 0.238 | 0.223 | 0.230 | 0.245 | 0.290x | 0.086x |

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
| original | output_zero | 0.0021 | 2 | 0.151 |
| original | row_schedule | 0.0000 | 2 | 0.151 |
| original | partial_zero | 0.0010 | 2 | 0.151 |
| original | csr_prefetch | 0.0050 | 2 | 0.151 |
| original | feature_gather_decode | 0.0052 | 2 | 0.151 |
| original | fp32_neighbor_reduction | 0.0000 | 2 | 0.151 |
| original | partial_bf16_conversion | 0.0000 | 2 | 0.151 |
| original | tile_load | 0.0288 | 2 | 0.151 |
| original | tile_compute | 0.0308 | 2 | 0.151 |
| original | tile_store | 0.0010 | 2 | 0.151 |
| original | output_scatter | 0.0000 | 2 | 0.151 |
| original | thread_amx_setup | 0.0029 | 2 | 0.151 |
| shared_b8_fast | output_zero | 0.0010 | 2 | 0.076 |
| shared_b8_fast | row_schedule | 0.0010 | 2 | 0.076 |
| shared_b8_fast | partial_zero | 0.0010 | 2 | 0.076 |
| shared_b8_fast | csr_prefetch | 0.0033 | 2 | 0.076 |
| shared_b8_fast | feature_gather_decode | 0.0052 | 2 | 0.076 |
| shared_b8_fast | fp32_neighbor_reduction | 0.0098 | 2 | 0.076 |
| shared_b8_fast | partial_bf16_conversion | 0.0012 | 2 | 0.076 |
| shared_b8_fast | tile_load | 0.0086 | 2 | 0.076 |
| shared_b8_fast | tile_compute | 0.0038 | 2 | 0.076 |
| shared_b8_fast | tile_store | 0.0000 | 2 | 0.076 |
| shared_b8_fast | output_scatter | 0.0000 | 2 | 0.076 |
| shared_b8_fast | thread_amx_setup | 0.0000 | 2 | 0.076 |
| shared_bfull_fast | output_zero | 0.0019 | 2 | 0.060 |
| shared_bfull_fast | row_schedule | 0.0000 | 2 | 0.060 |
| shared_bfull_fast | partial_zero | 0.0000 | 2 | 0.060 |
| shared_bfull_fast | csr_prefetch | 0.0069 | 2 | 0.060 |
| shared_bfull_fast | feature_gather_decode | 0.0052 | 2 | 0.060 |
| shared_bfull_fast | fp32_neighbor_reduction | 0.0062 | 2 | 0.060 |
| shared_bfull_fast | partial_bf16_conversion | 0.0000 | 2 | 0.060 |
| shared_bfull_fast | tile_load | 0.0010 | 2 | 0.060 |
| shared_bfull_fast | tile_compute | 0.0043 | 2 | 0.060 |
| shared_bfull_fast | tile_store | 0.0010 | 2 | 0.060 |
| shared_bfull_fast | output_scatter | 0.0000 | 2 | 0.060 |
| shared_bfull_fast | thread_amx_setup | 0.0060 | 2 | 0.060 |
| shared_bfull_accurate | output_zero | 0.0019 | 2 | 0.064 |
| shared_bfull_accurate | row_schedule | 0.0000 | 2 | 0.064 |
| shared_bfull_accurate | partial_zero | 0.0000 | 2 | 0.064 |
| shared_bfull_accurate | csr_prefetch | 0.0069 | 2 | 0.064 |
| shared_bfull_accurate | feature_gather_decode | 0.0072 | 2 | 0.064 |
| shared_bfull_accurate | fp32_neighbor_reduction | 0.0050 | 2 | 0.064 |
| shared_bfull_accurate | partial_bf16_conversion | 0.0000 | 2 | 0.064 |
| shared_bfull_accurate | tile_load | 0.0060 | 2 | 0.064 |
| shared_bfull_accurate | tile_compute | 0.0052 | 2 | 0.064 |
| shared_bfull_accurate | tile_store | 0.0000 | 2 | 0.064 |
| shared_bfull_accurate | output_scatter | 0.0000 | 2 | 0.064 |
| shared_bfull_accurate | thread_amx_setup | 0.0072 | 2 | 0.064 |
| replay_bfull_fast | output_zero | 0.0010 | 2 | 0.107 |
| replay_bfull_fast | row_schedule | 0.0000 | 2 | 0.107 |
| replay_bfull_fast | partial_zero | 0.0010 | 2 | 0.107 |
| replay_bfull_fast | csr_prefetch | 0.0107 | 2 | 0.107 |
| replay_bfull_fast | feature_gather_decode | 0.0179 | 2 | 0.107 |
| replay_bfull_fast | fp32_neighbor_reduction | 0.0126 | 2 | 0.107 |
| replay_bfull_fast | partial_bf16_conversion | 0.0012 | 2 | 0.107 |
| replay_bfull_fast | tile_load | 0.0010 | 2 | 0.107 |
| replay_bfull_fast | tile_compute | 0.0062 | 2 | 0.107 |
| replay_bfull_fast | tile_store | 0.0010 | 2 | 0.107 |
| replay_bfull_fast | output_scatter | 0.0000 | 2 | 0.107 |
| replay_bfull_fast | thread_amx_setup | 0.0000 | 2 | 0.107 |

## tail-n137-q130

N=137, E=17810, threads=8, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_fp32 | 0.027 | 0.027 | 0.027 | 0.027 | 7.611x | 1.000x |
| mkl_bf16_inputs | 0.029 | 0.029 | 0.029 | 0.029 | 7.049x | 0.926x |
| shared_bfull_fast | 0.058 | 0.058 | 0.058 | 0.058 | 3.525x | 0.463x |
| shared_bfull_accurate | 0.068 | 0.068 | 0.068 | 0.068 | 3.018x | 0.396x |
| shared_b32_fast | 0.074 | 0.074 | 0.074 | 0.074 | 2.765x | 0.363x |
| shared_b64_fast | 0.083 | 0.083 | 0.083 | 0.083 | 2.471x | 0.325x |
| shared_b32_accurate | 0.086 | 0.086 | 0.086 | 0.086 | 2.382x | 0.313x |
| shared_b16_fast | 0.091 | 0.091 | 0.091 | 0.091 | 2.251x | 0.296x |
| shared_b64_accurate | 0.095 | 0.095 | 0.095 | 0.095 | 2.161x | 0.284x |
| shared_b16_accurate | 0.105 | 0.105 | 0.105 | 0.105 | 1.950x | 0.256x |
| replay_b32_fast | 0.121 | 0.121 | 0.121 | 0.121 | 1.693x | 0.222x |
| shared_b8_fast | 0.123 | 0.123 | 0.123 | 0.123 | 1.667x | 0.219x |
| replay_bfull_fast | 0.128 | 0.128 | 0.128 | 0.128 | 1.601x | 0.210x |
| shared_b8_accurate | 0.151 | 0.151 | 0.151 | 0.151 | 1.359x | 0.179x |
| replay_b8_fast | 0.176 | 0.176 | 0.176 | 0.176 | 1.165x | 0.153x |
| shared_b4_fast | 0.186 | 0.186 | 0.186 | 0.186 | 1.103x | 0.145x |
| original | 0.205 | 0.205 | 0.205 | 0.205 | 1.000x | 0.131x |
| shared_b4_accurate | 0.246 | 0.246 | 0.246 | 0.246 | 0.833x | 0.109x |
| shared_b2_fast | 0.349 | 0.349 | 0.349 | 0.349 | 0.587x | 0.077x |
| shared_b2_accurate | 0.402 | 0.402 | 0.402 | 0.402 | 0.510x | 0.067x |
| shared_b1_fast | 0.457 | 0.457 | 0.457 | 0.457 | 0.449x | 0.059x |
| shared_b1_accurate | 0.828 | 0.828 | 0.828 | 0.828 | 0.248x | 0.033x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| mkl_fp32 | 0.056 | 0.056 | 0.056 | 0.056 | 6.363x | 1.000x |
| mkl_bf16_inputs | 0.072 | 0.072 | 0.072 | 0.072 | 4.930x | 0.775x |
| shared_bfull_fast | 0.123 | 0.123 | 0.123 | 0.123 | 2.886x | 0.453x |
| shared_bfull_accurate | 0.134 | 0.134 | 0.134 | 0.134 | 2.649x | 0.416x |
| shared_b32_fast | 0.155 | 0.155 | 0.155 | 0.155 | 2.291x | 0.360x |
| replay_bfull_fast | 0.209 | 0.209 | 0.209 | 0.209 | 1.698x | 0.267x |
| shared_b8_fast | 0.252 | 0.252 | 0.252 | 0.252 | 1.409x | 0.221x |
| original | 0.355 | 0.355 | 0.355 | 0.355 | 1.000x | 0.157x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 5.141e-03 | 7.387e-01 | 5.237e-03 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 5.117e-03 | 7.387e-01 | 5.220e-03 | 1 |
| original | kernel_full | mkl_bf16_inputs | 4.635e-07 | 1.335e-04 | 9.465e-07 | 1 |
| original | kernel_full | mkl_fp32 | 5.117e-03 | 7.388e-01 | 5.220e-03 | 1 |
| shared_b1_fast | kernel_full | mkl_bf16_inputs | 4.635e-07 | 1.335e-04 | 9.465e-07 | 1 |
| shared_b1_fast | kernel_full | mkl_fp32 | 5.117e-03 | 7.388e-01 | 5.220e-03 | 1 |
| shared_b1_fast | b1_identity | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_accurate | kernel_full | mkl_bf16_inputs | 4.635e-07 | 1.335e-04 | 9.465e-07 | 1 |
| shared_b1_accurate | kernel_full | mkl_fp32 | 5.117e-03 | 7.388e-01 | 5.220e-03 | 1 |
| shared_b2_fast | kernel_full | mkl_bf16_inputs | 2.361e-03 | 4.068e-01 | 2.884e-03 | 1 |
| shared_b2_fast | kernel_full | mkl_fp32 | 6.821e-03 | 1.005e+00 | 7.101e-03 | 1 |
| shared_b2_accurate | kernel_full | mkl_bf16_inputs | 4.767e-07 | 1.450e-04 | 1.028e-06 | 1 |
| shared_b2_accurate | kernel_full | mkl_fp32 | 5.117e-03 | 7.388e-01 | 5.220e-03 | 1 |
| shared_b4_fast | kernel_full | mkl_bf16_inputs | 2.985e-03 | 4.910e-01 | 3.481e-03 | 1 |
| shared_b4_fast | kernel_full | mkl_fp32 | 7.522e-03 | 1.079e+00 | 7.624e-03 | 1 |
| shared_b4_accurate | kernel_full | mkl_bf16_inputs | 3.738e-07 | 1.297e-04 | 9.195e-07 | 1 |
| shared_b4_accurate | kernel_full | mkl_fp32 | 5.117e-03 | 7.388e-01 | 5.220e-03 | 1 |
| shared_b8_fast | kernel_full | mkl_bf16_inputs | 3.164e-03 | 4.889e-01 | 3.466e-03 | 1 |
| shared_b8_fast | kernel_full | mkl_fp32 | 7.735e-03 | 1.049e+00 | 7.415e-03 | 1 |
| shared_b8_accurate | kernel_full | mkl_bf16_inputs | 6.609e-07 | 1.717e-04 | 1.217e-06 | 1 |
| shared_b8_accurate | kernel_full | mkl_fp32 | 5.117e-03 | 7.387e-01 | 5.220e-03 | 1 |
| replay_b8_fast | kernel_full | mkl_bf16_inputs | 3.164e-03 | 4.889e-01 | 3.466e-03 | 1 |
| replay_b8_fast | kernel_full | mkl_fp32 | 7.735e-03 | 1.049e+00 | 7.415e-03 | 1 |
| shared_b16_fast | kernel_full | mkl_bf16_inputs | 3.305e-03 | 5.967e-01 | 4.231e-03 | 1 |
| shared_b16_fast | kernel_full | mkl_fp32 | 7.888e-03 | 1.178e+00 | 8.322e-03 | 1 |
| shared_b16_accurate | kernel_full | mkl_bf16_inputs | 1.990e-06 | 3.719e-04 | 2.637e-06 | 1 |
| shared_b16_accurate | kernel_full | mkl_fp32 | 5.118e-03 | 7.388e-01 | 5.220e-03 | 1 |
| shared_b32_fast | kernel_full | mkl_bf16_inputs | 3.297e-03 | 5.337e-01 | 3.784e-03 | 1 |
| shared_b32_fast | kernel_full | mkl_fp32 | 7.860e-03 | 1.102e+00 | 7.787e-03 | 1 |
| shared_b32_accurate | kernel_full | mkl_bf16_inputs | 4.165e-06 | 7.896e-04 | 5.598e-06 | 1 |
| shared_b32_accurate | kernel_full | mkl_fp32 | 5.119e-03 | 7.385e-01 | 5.218e-03 | 1 |
| replay_b32_fast | kernel_full | mkl_bf16_inputs | 3.297e-03 | 5.337e-01 | 3.784e-03 | 1 |
| replay_b32_fast | kernel_full | mkl_fp32 | 7.860e-03 | 1.102e+00 | 7.787e-03 | 1 |
| shared_b64_fast | kernel_full | mkl_bf16_inputs | 3.405e-03 | 6.370e-01 | 4.516e-03 | 1 |
| shared_b64_fast | kernel_full | mkl_fp32 | 7.959e-03 | 1.139e+00 | 8.050e-03 | 1 |
| shared_b64_accurate | kernel_full | mkl_bf16_inputs | 6.456e-06 | 1.282e-03 | 9.087e-06 | 1 |
| shared_b64_accurate | kernel_full | mkl_fp32 | 5.120e-03 | 7.392e-01 | 5.223e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_bf16_inputs | 3.463e-03 | 5.628e-01 | 3.990e-03 | 1 |
| shared_bfull_fast | kernel_full | mkl_fp32 | 7.984e-03 | 1.092e+00 | 7.715e-03 | 1 |
| shared_bfull_accurate | kernel_full | mkl_bf16_inputs | 8.387e-06 | 1.442e-03 | 1.022e-05 | 1 |
| shared_bfull_accurate | kernel_full | mkl_fp32 | 5.121e-03 | 7.392e-01 | 5.223e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_bf16_inputs | 3.463e-03 | 5.628e-01 | 3.990e-03 | 1 |
| replay_bfull_fast | kernel_full | mkl_fp32 | 7.984e-03 | 1.092e+00 | 7.715e-03 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_fast | instrumented_kernel | mkl_bf16_inputs | 4.635e-07 | 1.335e-04 | 9.465e-07 | 1 |
| shared_b1_accurate | instrumented_kernel | mkl_bf16_inputs | 4.635e-07 | 1.335e-04 | 9.465e-07 | 1 |
| shared_b2_fast | instrumented_kernel | mkl_bf16_inputs | 2.361e-03 | 4.068e-01 | 2.884e-03 | 1 |
| shared_b2_accurate | instrumented_kernel | mkl_bf16_inputs | 4.767e-07 | 1.450e-04 | 1.028e-06 | 1 |
| shared_b4_fast | instrumented_kernel | mkl_bf16_inputs | 2.985e-03 | 4.910e-01 | 3.481e-03 | 1 |
| shared_b4_accurate | instrumented_kernel | mkl_bf16_inputs | 3.738e-07 | 1.297e-04 | 9.195e-07 | 1 |
| shared_b8_fast | instrumented_kernel | mkl_bf16_inputs | 3.164e-03 | 4.889e-01 | 3.466e-03 | 1 |
| shared_b8_accurate | instrumented_kernel | mkl_bf16_inputs | 6.609e-07 | 1.717e-04 | 1.217e-06 | 1 |
| replay_b8_fast | instrumented_kernel | mkl_bf16_inputs | 3.164e-03 | 4.889e-01 | 3.466e-03 | 1 |
| shared_b16_fast | instrumented_kernel | mkl_bf16_inputs | 3.305e-03 | 5.967e-01 | 4.231e-03 | 1 |
| shared_b16_accurate | instrumented_kernel | mkl_bf16_inputs | 1.990e-06 | 3.719e-04 | 2.637e-06 | 1 |
| shared_b32_fast | instrumented_kernel | mkl_bf16_inputs | 3.297e-03 | 5.337e-01 | 3.784e-03 | 1 |
| shared_b32_accurate | instrumented_kernel | mkl_bf16_inputs | 4.165e-06 | 7.896e-04 | 5.598e-06 | 1 |
| replay_b32_fast | instrumented_kernel | mkl_bf16_inputs | 3.297e-03 | 5.337e-01 | 3.784e-03 | 1 |
| shared_b64_fast | instrumented_kernel | mkl_bf16_inputs | 3.405e-03 | 6.370e-01 | 4.516e-03 | 1 |
| shared_b64_accurate | instrumented_kernel | mkl_bf16_inputs | 6.456e-06 | 1.282e-03 | 9.087e-06 | 1 |
| shared_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.463e-03 | 5.628e-01 | 3.990e-03 | 1 |
| shared_bfull_accurate | instrumented_kernel | mkl_bf16_inputs | 8.387e-06 | 1.442e-03 | 1.022e-05 | 1 |
| replay_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 3.463e-03 | 5.628e-01 | 3.990e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 1.052e-02 | 6.153e+02 | 9.670e-03 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.153e+02 | 9.585e-03 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 6.108e-06 | 3.008e-01 | 4.727e-06 | 1 |
| original | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.156e+02 | 9.589e-03 | 1 |
| shared_b8_fast | two_layer_e2e | mkl_bf16_inputs | 5.162e-03 | 3.436e+02 | 5.399e-03 | 1 |
| shared_b8_fast | two_layer_e2e | mkl_fp32 | 1.536e-02 | 9.311e+02 | 1.450e-02 | 1 |
| shared_b32_fast | two_layer_e2e | mkl_bf16_inputs | 5.617e-03 | 3.900e+02 | 6.129e-03 | 1 |
| shared_b32_fast | two_layer_e2e | mkl_fp32 | 1.579e-02 | 9.830e+02 | 1.531e-02 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.927e-03 | 4.922e+02 | 7.735e-03 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_fp32 | 1.595e-02 | 1.055e+03 | 1.643e-02 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_bf16_inputs | 1.292e-05 | 9.004e-01 | 1.415e-05 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_fp32 | 1.042e-02 | 6.154e+02 | 9.586e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 5.927e-03 | 4.922e+02 | 7.735e-03 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_fp32 | 1.595e-02 | 1.055e+03 | 1.643e-02 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 0.0050 | 2 | 0.491 |
| original | row_schedule | 0.0010 | 2 | 0.491 |
| original | partial_zero | 0.0000 | 2 | 0.491 |
| original | csr_prefetch | 0.0246 | 2 | 0.491 |
| original | feature_gather_decode | 0.0265 | 2 | 0.491 |
| original | fp32_neighbor_reduction | 0.0000 | 2 | 0.491 |
| original | partial_bf16_conversion | 0.0000 | 2 | 0.491 |
| original | tile_load | 0.1273 | 2 | 0.491 |
| original | tile_compute | 0.1130 | 2 | 0.491 |
| original | tile_store | 0.0000 | 2 | 0.491 |
| original | output_scatter | 0.0029 | 2 | 0.491 |
| original | thread_amx_setup | 0.0076 | 2 | 0.491 |
| shared_b8_fast | output_zero | 0.0041 | 2 | 0.604 |
| shared_b8_fast | row_schedule | 0.0012 | 2 | 0.604 |
| shared_b8_fast | partial_zero | 0.0043 | 2 | 0.604 |
| shared_b8_fast | csr_prefetch | 0.1044 | 2 | 0.604 |
| shared_b8_fast | feature_gather_decode | 0.1235 | 2 | 0.604 |
| shared_b8_fast | fp32_neighbor_reduction | 0.1023 | 2 | 0.604 |
| shared_b8_fast | partial_bf16_conversion | 0.0064 | 2 | 0.604 |
| shared_b8_fast | tile_load | 0.0346 | 2 | 0.604 |
| shared_b8_fast | tile_compute | 0.0997 | 2 | 0.604 |
| shared_b8_fast | tile_store | 0.0038 | 2 | 0.604 |
| shared_b8_fast | output_scatter | 0.0019 | 2 | 0.604 |
| shared_b8_fast | thread_amx_setup | 0.0000 | 2 | 0.604 |
| shared_bfull_fast | output_zero | 0.0038 | 2 | 0.406 |
| shared_bfull_fast | row_schedule | 0.0000 | 2 | 0.406 |
| shared_bfull_fast | partial_zero | 0.0000 | 2 | 0.406 |
| shared_bfull_fast | csr_prefetch | 0.0932 | 2 | 0.406 |
| shared_bfull_fast | feature_gather_decode | 0.0939 | 2 | 0.406 |
| shared_bfull_fast | fp32_neighbor_reduction | 0.0887 | 2 | 0.406 |
| shared_bfull_fast | partial_bf16_conversion | 0.0010 | 2 | 0.406 |
| shared_bfull_fast | tile_load | 0.0041 | 2 | 0.406 |
| shared_bfull_fast | tile_compute | 0.0031 | 2 | 0.406 |
| shared_bfull_fast | tile_store | 0.0000 | 2 | 0.406 |
| shared_bfull_fast | output_scatter | 0.0000 | 2 | 0.406 |
| shared_bfull_fast | thread_amx_setup | 0.0083 | 2 | 0.406 |
| shared_bfull_accurate | output_zero | 0.0041 | 2 | 0.389 |
| shared_bfull_accurate | row_schedule | 0.0012 | 2 | 0.389 |
| shared_bfull_accurate | partial_zero | 0.0021 | 2 | 0.389 |
| shared_bfull_accurate | csr_prefetch | 0.0868 | 2 | 0.389 |
| shared_bfull_accurate | feature_gather_decode | 0.0968 | 2 | 0.389 |
| shared_bfull_accurate | fp32_neighbor_reduction | 0.0834 | 2 | 0.389 |
| shared_bfull_accurate | partial_bf16_conversion | 0.0000 | 2 | 0.389 |
| shared_bfull_accurate | tile_load | 0.0019 | 2 | 0.389 |
| shared_bfull_accurate | tile_compute | 0.0029 | 2 | 0.389 |
| shared_bfull_accurate | tile_store | 0.0029 | 2 | 0.389 |
| shared_bfull_accurate | output_scatter | 0.0000 | 2 | 0.389 |
| shared_bfull_accurate | thread_amx_setup | 0.0000 | 2 | 0.389 |
| replay_bfull_fast | output_zero | 0.0041 | 2 | 0.884 |
| replay_bfull_fast | row_schedule | 0.0000 | 2 | 0.884 |
| replay_bfull_fast | partial_zero | 0.0000 | 2 | 0.884 |
| replay_bfull_fast | csr_prefetch | 0.2027 | 2 | 0.884 |
| replay_bfull_fast | feature_gather_decode | 0.2153 | 2 | 0.884 |
| replay_bfull_fast | fp32_neighbor_reduction | 0.1707 | 2 | 0.884 |
| replay_bfull_fast | partial_bf16_conversion | 0.0000 | 2 | 0.884 |
| replay_bfull_fast | tile_load | 0.0031 | 2 | 0.884 |
| replay_bfull_fast | tile_compute | 0.0103 | 2 | 0.884 |
| replay_bfull_fast | tile_store | 0.0010 | 2 | 0.884 |
| replay_bfull_fast | output_scatter | 0.0000 | 2 | 0.884 |
| replay_bfull_fast | thread_amx_setup | 0.0010 | 2 | 0.884 |

## tail-n17-q0

N=17, E=0, threads=8, status=PASS

### kernel

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_b32_accurate | 0.001 | 0.001 | 0.001 | 0.001 | 25.250x | 7.250x |
| shared_b16_accurate | 0.001 | 0.001 | 0.001 | 0.001 | 20.200x | 5.800x |
| shared_b64_accurate | 0.001 | 0.001 | 0.001 | 0.001 | 20.200x | 5.800x |
| shared_b1_accurate | 0.002 | 0.002 | 0.002 | 0.002 | 12.625x | 3.625x |
| shared_b2_fast | 0.002 | 0.002 | 0.002 | 0.002 | 12.625x | 3.625x |
| shared_b4_fast | 0.002 | 0.002 | 0.002 | 0.002 | 12.625x | 3.625x |
| shared_b4_accurate | 0.002 | 0.002 | 0.002 | 0.002 | 12.625x | 3.625x |
| shared_b8_accurate | 0.002 | 0.002 | 0.002 | 0.002 | 12.625x | 3.625x |
| shared_b32_fast | 0.002 | 0.002 | 0.002 | 0.002 | 12.625x | 3.625x |
| shared_bfull_fast | 0.002 | 0.002 | 0.002 | 0.002 | 12.625x | 3.625x |
| shared_bfull_accurate | 0.002 | 0.002 | 0.002 | 0.002 | 12.625x | 3.625x |
| shared_b2_accurate | 0.002 | 0.002 | 0.002 | 0.002 | 11.222x | 3.222x |
| shared_b8_fast | 0.002 | 0.002 | 0.002 | 0.002 | 11.222x | 3.222x |
| shared_b16_fast | 0.003 | 0.003 | 0.003 | 0.003 | 8.417x | 2.417x |
| shared_b1_fast | 0.003 | 0.003 | 0.003 | 0.003 | 7.769x | 2.231x |
| shared_b64_fast | 0.003 | 0.003 | 0.003 | 0.003 | 7.769x | 2.231x |
| replay_b8_fast | 0.004 | 0.004 | 0.004 | 0.004 | 6.313x | 1.813x |
| replay_b32_fast | 0.004 | 0.004 | 0.004 | 0.004 | 5.941x | 1.706x |
| replay_bfull_fast | 0.004 | 0.004 | 0.004 | 0.004 | 5.941x | 1.706x |
| mkl_bf16_inputs | 0.006 | 0.006 | 0.006 | 0.006 | 4.040x | 1.160x |
| mkl_fp32 | 0.007 | 0.007 | 0.007 | 0.007 | 3.483x | 1.000x |
| original | 0.024 | 0.024 | 0.024 | 0.024 | 1.000x | 0.287x |

### e2e

| method | median ms | min ms | P25 ms | P75 ms | speedup vs Original | speedup vs MKL FP32 |
|---|---:|---:|---:|---:|---:|---:|
| shared_bfull_fast | 0.007 | 0.007 | 0.007 | 0.007 | 1.897x | 1.759x |
| shared_bfull_accurate | 0.007 | 0.007 | 0.007 | 0.007 | 1.897x | 1.759x |
| shared_b32_fast | 0.007 | 0.007 | 0.007 | 0.007 | 1.833x | 1.700x |
| shared_b8_fast | 0.008 | 0.008 | 0.008 | 0.008 | 1.618x | 1.500x |
| replay_bfull_fast | 0.011 | 0.011 | 0.011 | 0.011 | 1.196x | 1.109x |
| mkl_fp32 | 0.012 | 0.012 | 0.012 | 0.012 | 1.078x | 1.000x |
| original | 0.013 | 0.013 | 0.013 | 0.013 | 1.000x | 0.927x |
| mkl_bf16_inputs | 0.014 | 0.014 | 0.014 | 0.014 | 0.932x | 0.864x |

### Numerical errors

| method | boundary | reference | rel L2 | max abs | normalized max | pass |
|---|---|---|---:|---:|---:|---|
| mkl_fp32 | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_fp32 | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_fast | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_fast | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_fast | b1_identity | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_accurate | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_accurate | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_accurate | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_accurate | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b4_fast | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b4_fast | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b4_accurate | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b4_accurate | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_fast | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_fast | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_accurate | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_accurate | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_b8_fast | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_b8_fast | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b16_fast | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b16_fast | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b16_accurate | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b16_accurate | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_fast | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_fast | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_accurate | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_accurate | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_b32_fast | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_b32_fast | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b64_fast | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b64_fast | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b64_accurate | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b64_accurate | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast | kernel_full | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast | kernel_full | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original | instrumented_kernel | unchanged_original | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_fast | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b1_accurate | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_fast | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b2_accurate | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b4_fast | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b4_accurate | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_fast | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_accurate | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_b8_fast | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b16_fast | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b16_accurate | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_fast | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_accurate | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_b32_fast | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b64_fast | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b64_accurate | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast | instrumented_kernel | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_fp32 | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| mkl_bf16_inputs | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| original | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_fast | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b8_fast | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_fast | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_b32_fast | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_fast | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| shared_bfull_accurate | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_bf16_inputs | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |
| replay_bfull_fast | two_layer_e2e | mkl_fp32 | 0.000e+00 | 0.000e+00 | 0.000e+00 | 1 |

### Sampled phase profile

| method | phase | sampled thread ms | sampled tiles | profile wall ms |
|---|---|---:|---:|---:|
| original | output_zero | 0.0010 | 2 | 0.020 |
| original | row_schedule | 0.0000 | 2 | 0.020 |
| original | partial_zero | 0.0150 | 2 | 0.020 |
| original | csr_prefetch | 0.0000 | 2 | 0.020 |
| original | feature_gather_decode | 0.0000 | 2 | 0.020 |
| original | fp32_neighbor_reduction | 0.0000 | 2 | 0.020 |
| original | partial_bf16_conversion | 0.0000 | 2 | 0.020 |
| original | tile_load | 0.0000 | 2 | 0.020 |
| original | tile_compute | 0.0000 | 2 | 0.020 |
| original | tile_store | 0.0000 | 2 | 0.020 |
| original | output_scatter | 0.0000 | 2 | 0.020 |
| original | thread_amx_setup | 0.0029 | 2 | 0.020 |
| shared_b8_fast | output_zero | 0.0010 | 2 | 0.003 |
| shared_b8_fast | row_schedule | 0.0000 | 2 | 0.003 |
| shared_b8_fast | partial_zero | 0.0000 | 2 | 0.003 |
| shared_b8_fast | csr_prefetch | 0.0000 | 2 | 0.003 |
| shared_b8_fast | feature_gather_decode | 0.0000 | 2 | 0.003 |
| shared_b8_fast | fp32_neighbor_reduction | 0.0000 | 2 | 0.003 |
| shared_b8_fast | partial_bf16_conversion | 0.0000 | 2 | 0.003 |
| shared_b8_fast | tile_load | 0.0000 | 2 | 0.003 |
| shared_b8_fast | tile_compute | 0.0000 | 2 | 0.003 |
| shared_b8_fast | tile_store | 0.0000 | 2 | 0.003 |
| shared_b8_fast | output_scatter | 0.0000 | 2 | 0.003 |
| shared_b8_fast | thread_amx_setup | 0.0000 | 2 | 0.003 |
| shared_bfull_fast | output_zero | 0.0010 | 2 | 0.003 |
| shared_bfull_fast | row_schedule | 0.0000 | 2 | 0.003 |
| shared_bfull_fast | partial_zero | 0.0000 | 2 | 0.003 |
| shared_bfull_fast | csr_prefetch | 0.0000 | 2 | 0.003 |
| shared_bfull_fast | feature_gather_decode | 0.0000 | 2 | 0.003 |
| shared_bfull_fast | fp32_neighbor_reduction | 0.0000 | 2 | 0.003 |
| shared_bfull_fast | partial_bf16_conversion | 0.0000 | 2 | 0.003 |
| shared_bfull_fast | tile_load | 0.0000 | 2 | 0.003 |
| shared_bfull_fast | tile_compute | 0.0000 | 2 | 0.003 |
| shared_bfull_fast | tile_store | 0.0000 | 2 | 0.003 |
| shared_bfull_fast | output_scatter | 0.0000 | 2 | 0.003 |
| shared_bfull_fast | thread_amx_setup | 0.0000 | 2 | 0.003 |
| shared_bfull_accurate | output_zero | 0.0010 | 2 | 0.003 |
| shared_bfull_accurate | row_schedule | 0.0000 | 2 | 0.003 |
| shared_bfull_accurate | partial_zero | 0.0000 | 2 | 0.003 |
| shared_bfull_accurate | csr_prefetch | 0.0000 | 2 | 0.003 |
| shared_bfull_accurate | feature_gather_decode | 0.0000 | 2 | 0.003 |
| shared_bfull_accurate | fp32_neighbor_reduction | 0.0000 | 2 | 0.003 |
| shared_bfull_accurate | partial_bf16_conversion | 0.0000 | 2 | 0.003 |
| shared_bfull_accurate | tile_load | 0.0000 | 2 | 0.003 |
| shared_bfull_accurate | tile_compute | 0.0000 | 2 | 0.003 |
| shared_bfull_accurate | tile_store | 0.0000 | 2 | 0.003 |
| shared_bfull_accurate | output_scatter | 0.0000 | 2 | 0.003 |
| shared_bfull_accurate | thread_amx_setup | 0.0000 | 2 | 0.003 |
| replay_bfull_fast | output_zero | 0.0010 | 2 | 0.010 |
| replay_bfull_fast | row_schedule | 0.0000 | 2 | 0.010 |
| replay_bfull_fast | partial_zero | 0.0010 | 2 | 0.010 |
| replay_bfull_fast | csr_prefetch | 0.0000 | 2 | 0.010 |
| replay_bfull_fast | feature_gather_decode | 0.0000 | 2 | 0.010 |
| replay_bfull_fast | fp32_neighbor_reduction | 0.0000 | 2 | 0.010 |
| replay_bfull_fast | partial_bf16_conversion | 0.0000 | 2 | 0.010 |
| replay_bfull_fast | tile_load | 0.0000 | 2 | 0.010 |
| replay_bfull_fast | tile_compute | 0.0000 | 2 | 0.010 |
| replay_bfull_fast | tile_store | 0.0038 | 2 | 0.010 |
| replay_bfull_fast | output_scatter | 0.0012 | 2 | 0.010 |
| replay_bfull_fast | thread_amx_setup | 0.0000 | 2 | 0.010 |

