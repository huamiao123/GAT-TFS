# Joint GAT experiment results

Run directory: `joint-full-10849393`. All times are milliseconds.

## Reading these results

- E2E median/min/max count each path/repetition once; only repetitions with all three layers are included. Warmup rows are excluded.
- Speedup = control E2E median / candidate E2E median. A ratio above 1 means the candidate is faster. This is a ratio of medians, not a median of paired ratios.
- `B1_TFS` is the existing B1 TFS BF16 path. `local_online_fp32`, `joint_g*`, `joint_v1_g*`, and `joint_full_g*` are FP32 candidates; they are not improved B1 BF16. The full-group variant removes active-head guards while retaining the FP32 dataflow.
- Each phase median is computed independently. Phase medians need not sum to the E2E median; E2E includes orchestration overhead.
- `PROFILE` values ending in `_worker_ms` are sampled worker-time sums, not an additive wall-time decomposition. Rescale is fused into weighted SpMM; its scalar exp also belongs to the score phase. Do not add worker sums to layer wall time.
- Zero stage fields can mean the work is inside another stage: joint/local UW and normalization are inside `kernel_ms`; a zero separate normalization field does not mean zero normalization cost.
- Logical source bytes and source-vector counters are source-code workload estimates, not measured DRAM traffic or cache-miss counts. Sparse arithmetic amplification remains.
- `UNVERIFIED` is preserved: an implementation comparison does not prove the original precision gate passed. CONFIG `untrained=true` means random-weight forward timing, not classification accuracy or end-to-end training performance.
- Degree Sort and parameter preparation costs are recorded separately in CONFIG/STATIC and excluded from the reported steady-state E2E timing.

## Dataset availability

| Dataset | Log | Complete marker | Failure marker | Measured paths |
| --- | --- | --- | --- | --- |
| arxiv | arxiv_joint.log | True | False | 7 |
| products | products_joint.log | True | False | 7 |

## arxiv

### E2E

| Path | Repeats | Median ms | Min ms | Max ms | Rep IDs |
| --- | --- | --- | --- | --- | --- |
| B0_FP32 | 3 | 74.242024 | 74.172471 | 74.271890 | 0,1,2 |
| B0_BF16 | 3 | 67.921739 | 67.912345 | 67.946506 | 0,1,2 |
| joint_v1_g2 | 3 | 157.733656 | 153.776981 | 158.253150 | 0,1,2 |
| joint_v1_g8 | 3 | 139.486002 | 137.931131 | 141.065463 | 0,1,2 |
| joint_full_g2 | 3 | 148.885585 | 148.736838 | 150.643283 | 0,1,2 |
| joint_full_g4 | 3 | 145.184511 | 142.572759 | 148.311381 | 0,1,2 |
| joint_full_g8 | 3 | 135.469152 | 135.209209 | 136.467968 | 0,1,2 |

### Speedup matrix

Rows are candidates; columns are controls. Missing controls appear as `-`.

| Candidate \ Control | B0_FP32 | B0_BF16 | joint_v1_g2 | joint_v1_g8 | joint_full_g2 | joint_full_g4 | joint_full_g8 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| B0_FP32 | 1.0000 | 0.9149 | 2.1246 | 1.8788 | 2.0054 | 1.9556 | 1.8247 |
| B0_BF16 | 1.0931 | 1.0000 | 2.3223 | 2.0536 | 2.1920 | 2.1375 | 1.9945 |
| joint_v1_g2 | 0.4707 | 0.4306 | 1.0000 | 0.8843 | 0.9439 | 0.9204 | 0.8588 |
| joint_v1_g8 | 0.5323 | 0.4869 | 1.1308 | 1.0000 | 1.0674 | 1.0409 | 0.9712 |
| joint_full_g2 | 0.4987 | 0.4562 | 1.0594 | 0.9369 | 1.0000 | 0.9751 | 0.9099 |
| joint_full_g4 | 0.5114 | 0.4678 | 1.0864 | 0.9607 | 1.0255 | 1.0000 | 0.9331 |
| joint_full_g8 | 0.5480 | 0.5014 | 1.1644 | 1.0297 | 1.0990 | 1.0717 | 1.0000 |

### Layer 1 phase medians

| Path | layer_ms | lr_ms | kernel_ms | projection_ms | conversion_ms | max_prescan_ms | normalization_ms | activation_ms |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| B0_FP32 | 30.875289 | 1.329919 | 20.962115 | 5.225011 | 0.000032 | 0.675941 | 1.240627 | 1.436779 |
| B0_BF16 | 29.997257 | 1.296700 | 20.827968 | 3.854533 | 0.630951 | 0.675720 | 1.234841 | 1.439420 |
| joint_v1_g2 | 48.237464 | 0.956076 | 45.858157 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.434249 |
| joint_v1_g8 | 48.009561 | 0.839847 | 45.727542 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.429216 |
| joint_full_g2 | 46.729918 | 0.857887 | 44.444522 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.423394 |
| joint_full_g4 | 48.607320 | 0.847066 | 46.311909 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.416625 |
| joint_full_g8 | 46.943447 | 0.847497 | 44.667289 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.414281 |

### Layer 2 (D=256, K=8, d=32 focus) phase medians

| Path | layer_ms | lr_ms | kernel_ms | projection_ms | conversion_ms | max_prescan_ms | normalization_ms | activation_ms |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| B0_FP32 | 35.629348 | 1.268120 | 20.808212 | 10.180887 | 0.000021 | 0.639244 | 1.246108 | 1.452673 |
| B0_BF16 | 30.870202 | 1.245968 | 20.795634 | 4.208008 | 1.277245 | 0.631833 | 1.250085 | 1.454211 |
| joint_v1_g2 | 81.078417 | 1.259210 | 78.358145 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.470342 |
| joint_v1_g8 | 65.654252 | 1.264501 | 62.912048 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.467699 |
| joint_full_g2 | 79.255562 | 1.280498 | 76.442231 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.511421 |
| joint_full_g4 | 72.565359 | 1.258114 | 69.794058 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.519655 |
| joint_full_g8 | 63.997267 | 1.269457 | 61.200026 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.515900 |

### Layer 3 phase medians

| Path | layer_ms | lr_ms | kernel_ms | projection_ms | conversion_ms | max_prescan_ms | normalization_ms | activation_ms |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| B0_FP32 | 7.754403 | 0.251627 | 3.911591 | 2.992774 | 0.000021 | 0.411299 | 0.192923 | 0.000113 |
| B0_BF16 | 7.061065 | 0.225364 | 3.972723 | 0.996147 | 1.254828 | 0.410107 | 0.191365 | 0.000027 |
| joint_v1_g2 | 25.711245 | 1.248934 | 24.467786 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000146 |
| joint_v1_g8 | 25.341366 | 1.255130 | 24.090032 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000143 |
| joint_full_g2 | 24.654045 | 1.313594 | 23.340260 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000040 |
| joint_full_g4 | 24.473149 | 1.308326 | 23.157160 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000151 |
| joint_full_g8 | 24.526006 | 1.317817 | 23.225952 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000144 |

### Diagnostics: original log text

The following lines are reproduced without reinterpretation. Oracle selection may exclude costly high-degree rows; sampled correctness is not a full-graph accuracy guarantee.

#### Configuration and static preparation

```text
CONFIG N=169343 E=2484941 Din=128 C=40 threads=16 warmups=1 repeats=3 seed=11 untrained=true block=32 panel=64 tile_rows=16 degree_sort_ms=16.794728 full_group_guards=compile_time_removed master_reference_gate=UNVERIFIED
STATIC layer=1 prepare_ms=0.490877 joint_U_scratch_bytes_per_worker=65536
STATIC layer=2 prepare_ms=0.730056 joint_U_scratch_bytes_per_worker=131072
STATIC layer=3 prepare_ms=0.121097 joint_U_scratch_bytes_per_worker=16384
```

#### CHECK, matched attention, oracle, PROFILE and online statistics

```text
CHECK path=full_fixed_input_g1 layer=1 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=1 group=1 bit_equal=1
CHECK path=full_fixed_input_g2 layer=1 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=1 group=2 bit_equal=1
CHECK path=full_fixed_input_g4 layer=1 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=1 group=4 bit_equal=1
CHECK path=full_fixed_input_g8 layer=1 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=1 group=8 bit_equal=1
CHECK path=full_fixed_input_g1 layer=2 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=2 group=1 bit_equal=1
CHECK path=full_fixed_input_g2 layer=2 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=2 group=2 bit_equal=1
CHECK path=full_fixed_input_g4 layer=2 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=2 group=4 bit_equal=1
CHECK path=full_fixed_input_g8 layer=2 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=2 group=8 bit_equal=1
CHECK path=full_fixed_input_g1 layer=3 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=3 group=1 bit_equal=1
CHECK path=full_fixed_input_g2 layer=3 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=3 group=2 bit_equal=1
CHECK path=full_fixed_input_g4 layer=3 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=3 group=4 bit_equal=1
CHECK path=full_fixed_input_g8 layer=3 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=3 group=8 bit_equal=1
CHECK path=B0_FP32 layer=1 reference=B0_FP32_full_model max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
CHECK path=B0_FP32 layer=2 reference=B0_FP32_full_model max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
CHECK path=B0_FP32 layer=3 reference=B0_FP32_full_model max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
CHECK path=B0_BF16 layer=1 reference=B0_FP32_full_model max_abs_error=0.00294017791748 mean_abs_error=0.000310601022267 relative_L2_error=0.00190785210806 finite=1 master_gate=UNVERIFIED
CHECK path=B0_BF16 layer=2 reference=B0_FP32_full_model max_abs_error=0.00309580564499 mean_abs_error=0.00039252464584 relative_L2_error=0.00253741964854 finite=1 master_gate=UNVERIFIED
CHECK path=B0_BF16 layer=3 reference=B0_FP32_full_model max_abs_error=0.00333481281996 mean_abs_error=0.000497330746597 relative_L2_error=0.00310539759334 finite=1 master_gate=UNVERIFIED
CHECK path=joint_v1_g2 layer=1 reference=B0_FP32_full_model max_abs_error=2.98023223877e-06 mean_abs_error=3.37278126141e-08 relative_L2_error=2.47892636985e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_v1_g2 layer=2 reference=B0_FP32_full_model max_abs_error=2.32458114624e-06 mean_abs_error=5.11332031516e-08 relative_L2_error=3.73431298945e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_v1_g2 layer=3 reference=B0_FP32_full_model max_abs_error=1.81794166565e-06 mean_abs_error=5.53293051788e-08 relative_L2_error=3.67525850721e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_v1_g8 layer=1 reference=B0_FP32_full_model max_abs_error=2.98023223877e-06 mean_abs_error=3.37278126141e-08 relative_L2_error=2.47892636985e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_v1_g8 layer=2 reference=B0_FP32_full_model max_abs_error=2.32458114624e-06 mean_abs_error=5.11332031516e-08 relative_L2_error=3.73431298945e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_v1_g8 layer=3 reference=B0_FP32_full_model max_abs_error=1.81794166565e-06 mean_abs_error=5.53293051788e-08 relative_L2_error=3.67525850721e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g2 layer=1 reference=B0_FP32_full_model max_abs_error=2.98023223877e-06 mean_abs_error=3.37278126141e-08 relative_L2_error=2.47892636985e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g2 layer=2 reference=B0_FP32_full_model max_abs_error=2.32458114624e-06 mean_abs_error=5.11332031516e-08 relative_L2_error=3.73431298945e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g2 layer=3 reference=B0_FP32_full_model max_abs_error=1.81794166565e-06 mean_abs_error=5.53293051788e-08 relative_L2_error=3.67525850721e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g4 layer=1 reference=B0_FP32_full_model max_abs_error=2.98023223877e-06 mean_abs_error=3.37278126141e-08 relative_L2_error=2.47892636985e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g4 layer=2 reference=B0_FP32_full_model max_abs_error=2.32458114624e-06 mean_abs_error=5.11332031516e-08 relative_L2_error=3.73431298945e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g4 layer=3 reference=B0_FP32_full_model max_abs_error=1.81794166565e-06 mean_abs_error=5.53293051788e-08 relative_L2_error=3.67525850721e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g8 layer=1 reference=B0_FP32_full_model max_abs_error=2.98023223877e-06 mean_abs_error=3.37278126141e-08 relative_L2_error=2.47892636985e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g8 layer=2 reference=B0_FP32_full_model max_abs_error=2.32458114624e-06 mean_abs_error=5.11332031516e-08 relative_L2_error=3.73431298945e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g8 layer=3 reference=B0_FP32_full_model max_abs_error=1.81794166565e-06 mean_abs_error=5.53293051788e-08 relative_L2_error=3.67525850721e-07 finite=1 master_gate=UNVERIFIED
PROFILE path=joint_v1_g2 layer=1 layer_wall_ms=50.805347 kernel_wall_ms=48.544498 sampled_tiles=96 init_worker_ms=0.101598 score_exp_den_worker_ms=1.633721 weighted_spmm_rescale_worker_ms=2.042324 UW_worker_ms=0.977513 normalization_scatter_worker_ms=0.134197 sampled_source_vector_loads=633440 sampled_fma_vectors=1266880
PROFILE path=joint_v1_g2 layer=2 layer_wall_ms=84.60098 kernel_wall_ms=81.899122 sampled_tiles=96 init_worker_ms=0.305238 score_exp_den_worker_ms=1.723358 weighted_spmm_rescale_worker_ms=3.498641 UW_worker_ms=1.547277 normalization_scatter_worker_ms=0.142115 sampled_source_vector_loads=1266880 sampled_fma_vectors=2533760
PROFILE path=joint_v1_g2 layer=3 layer_wall_ms=26.034404 kernel_wall_ms=24.803649 sampled_tiles=96 init_worker_ms=0.04714 score_exp_den_worker_ms=0.482407 weighted_spmm_rescale_worker_ms=1.630418 UW_worker_ms=0.329991 normalization_scatter_worker_ms=0.036303 sampled_source_vector_loads=316720 sampled_fma_vectors=316720
PROFILE path=joint_v1_g8 layer=1 layer_wall_ms=49.055756 kernel_wall_ms=46.784733 sampled_tiles=96 init_worker_ms=0.104261 score_exp_den_worker_ms=1.373554 weighted_spmm_rescale_worker_ms=2.493212 UW_worker_ms=0.974268 normalization_scatter_worker_ms=0.140558 sampled_source_vector_loads=158360 sampled_fma_vectors=1266880
PROFILE path=joint_v1_g8 layer=2 layer_wall_ms=67.306727 kernel_wall_ms=64.592415 sampled_tiles=96 init_worker_ms=0.293646 score_exp_den_worker_ms=1.502921 weighted_spmm_rescale_worker_ms=3.331885 UW_worker_ms=1.538268 normalization_scatter_worker_ms=0.143947 sampled_source_vector_loads=316720 sampled_fma_vectors=2533760
PROFILE path=joint_v1_g8 layer=3 layer_wall_ms=25.670591 kernel_wall_ms=24.416354 sampled_tiles=96 init_worker_ms=0.047726 score_exp_den_worker_ms=0.482449 weighted_spmm_rescale_worker_ms=1.606855 UW_worker_ms=0.328852 normalization_scatter_worker_ms=0.039228 sampled_source_vector_loads=316720 sampled_fma_vectors=316720
PROFILE path=joint_full_g2 layer=1 layer_wall_ms=47.217403 kernel_wall_ms=44.960873 sampled_tiles=96 init_worker_ms=0.095517 score_exp_den_worker_ms=1.688413 weighted_spmm_rescale_worker_ms=1.919764 UW_worker_ms=0.960678 normalization_scatter_worker_ms=0.134822 sampled_source_vector_loads=633440 sampled_fma_vectors=1266880
PROFILE path=joint_full_g2 layer=2 layer_wall_ms=78.417464 kernel_wall_ms=75.673314 sampled_tiles=96 init_worker_ms=0.292891 score_exp_den_worker_ms=1.754345 weighted_spmm_rescale_worker_ms=3.339509 UW_worker_ms=1.524914 normalization_scatter_worker_ms=0.153285 sampled_source_vector_loads=1266880 sampled_fma_vectors=2533760
PROFILE path=joint_full_g2 layer=3 layer_wall_ms=25.829045 kernel_wall_ms=24.528571 sampled_tiles=96 init_worker_ms=0.057335 score_exp_den_worker_ms=0.442373 weighted_spmm_rescale_worker_ms=1.600664 UW_worker_ms=0.326349 normalization_scatter_worker_ms=0.038835 sampled_source_vector_loads=316720 sampled_fma_vectors=316720
PROFILE path=joint_full_g4 layer=1 layer_wall_ms=48.952591 kernel_wall_ms=46.693404 sampled_tiles=96 init_worker_ms=0.099234 score_exp_den_worker_ms=1.517021 weighted_spmm_rescale_worker_ms=2.200952 UW_worker_ms=0.947964 normalization_scatter_worker_ms=0.14637 sampled_source_vector_loads=316720 sampled_fma_vectors=1266880
PROFILE path=joint_full_g4 layer=2 layer_wall_ms=74.617224 kernel_wall_ms=71.842488 sampled_tiles=96 init_worker_ms=0.278082 score_exp_den_worker_ms=1.634292 weighted_spmm_rescale_worker_ms=3.240203 UW_worker_ms=1.512078 normalization_scatter_worker_ms=0.153064 sampled_source_vector_loads=633440 sampled_fma_vectors=2533760
PROFILE path=joint_full_g4 layer=3 layer_wall_ms=26.651317 kernel_wall_ms=25.342351 sampled_tiles=96 init_worker_ms=0.053472 score_exp_den_worker_ms=0.442392 weighted_spmm_rescale_worker_ms=1.623866 UW_worker_ms=0.328504 normalization_scatter_worker_ms=0.038521 sampled_source_vector_loads=316720 sampled_fma_vectors=316720
PROFILE path=joint_full_g8 layer=1 layer_wall_ms=48.130185 kernel_wall_ms=45.857788 sampled_tiles=96 init_worker_ms=0.096325 score_exp_den_worker_ms=1.367487 weighted_spmm_rescale_worker_ms=2.32793 UW_worker_ms=0.983917 normalization_scatter_worker_ms=0.142259 sampled_source_vector_loads=158360 sampled_fma_vectors=1266880
JOINT_ONLINE_STATS layer=1 blocks=1580688 running_max_updates_including_initial=1400775 rescales=46031 rescaled_feature_elements=5891968 rescale_per_block=0.02912086383 row_head_ratio_P50=0 P90=0 P95=0 P99=0.5 rescale_timing=fused_with_weighted_spmm
PROFILE path=joint_full_g8 layer=2 layer_wall_ms=63.646211 kernel_wall_ms=60.864238 sampled_tiles=96 init_worker_ms=0.284222 score_exp_den_worker_ms=1.513499 weighted_spmm_rescale_worker_ms=3.119185 UW_worker_ms=1.522962 normalization_scatter_worker_ms=0.152793 sampled_source_vector_loads=316720 sampled_fma_vectors=2533760
JOINT_ONLINE_STATS layer=2 blocks=1580688 running_max_updates_including_initial=1403398 rescales=48654 rescaled_feature_elements=12455424 rescale_per_block=0.03078026783 row_head_ratio_P50=0 P90=0 P95=0 P99=0.5 rescale_timing=fused_with_weighted_spmm
PROFILE path=joint_full_g8 layer=3 layer_wall_ms=25.536706 kernel_wall_ms=24.237236 sampled_tiles=96 init_worker_ms=0.057882 score_exp_den_worker_ms=0.442382 weighted_spmm_rescale_worker_ms=1.60369 UW_worker_ms=0.323442 normalization_scatter_worker_ms=0.039166 sampled_source_vector_loads=316720 sampled_fma_vectors=316720
JOINT_ONLINE_STATS layer=3 blocks=197586 running_max_updates_including_initial=175516 rescales=6173 rescaled_feature_elements=1580288 rescale_per_block=0.03124209205 row_head_ratio_P50=0 P90=0 P95=0 P99=0.5 rescale_timing=fused_with_weighted_spmm
JOINT_COMPLETE implementation_gate=PASS master_reference_gate=UNVERIFIED task_accuracy=UNVERIFIED performance=EXPLORATORY
```

## products

### E2E

| Path | Repeats | Median ms | Min ms | Max ms | Rep IDs |
| --- | --- | --- | --- | --- | --- |
| B0_FP32 | 3 | 3838.167375 | 3835.610824 | 3843.718188 | 0,1,2 |
| B0_BF16 | 3 | 3736.940441 | 3725.842859 | 3741.763044 | 0,1,2 |
| joint_v1_g2 | 3 | 4515.482826 | 4497.352281 | 4524.073292 | 0,1,2 |
| joint_v1_g8 | 3 | 4645.452236 | 4645.082798 | 4645.950474 | 0,1,2 |
| joint_full_g2 | 3 | 4407.019438 | 4403.179566 | 4412.148101 | 0,1,2 |
| joint_full_g4 | 3 | 4742.111276 | 4696.747762 | 4782.424644 | 0,1,2 |
| joint_full_g8 | 3 | 4592.145837 | 4575.595867 | 4619.277606 | 0,1,2 |

### Speedup matrix

Rows are candidates; columns are controls. Missing controls appear as `-`.

| Candidate \ Control | B0_FP32 | B0_BF16 | joint_v1_g2 | joint_v1_g8 | joint_full_g2 | joint_full_g4 | joint_full_g8 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| B0_FP32 | 1.0000 | 0.9736 | 1.1765 | 1.2103 | 1.1482 | 1.2355 | 1.1964 |
| B0_BF16 | 1.0271 | 1.0000 | 1.2083 | 1.2431 | 1.1793 | 1.2690 | 1.2289 |
| joint_v1_g2 | 0.8500 | 0.8276 | 1.0000 | 1.0288 | 0.9760 | 1.0502 | 1.0170 |
| joint_v1_g8 | 0.8262 | 0.8044 | 0.9720 | 1.0000 | 0.9487 | 1.0208 | 0.9885 |
| joint_full_g2 | 0.8709 | 0.8480 | 1.0246 | 1.0541 | 1.0000 | 1.0760 | 1.0420 |
| joint_full_g4 | 0.8094 | 0.7880 | 0.9522 | 0.9796 | 0.9293 | 1.0000 | 0.9684 |
| joint_full_g8 | 0.8358 | 0.8138 | 0.9833 | 1.0116 | 0.9597 | 1.0327 | 1.0000 |

### Layer 1 phase medians

| Path | layer_ms | lr_ms | kernel_ms | projection_ms | conversion_ms | max_prescan_ms | normalization_ms | activation_ms |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| B0_FP32 | 1620.705358 | 17.849472 | 1456.856342 | 63.864532 | 0.000036 | 41.091097 | 19.153312 | 21.275176 |
| B0_BF16 | 1614.363236 | 17.730373 | 1451.188350 | 53.530578 | 9.307589 | 41.170467 | 19.146587 | 21.227862 |
| joint_v1_g2 | 1222.778497 | 13.237511 | 1187.798215 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 21.533150 |
| joint_v1_g8 | 1556.644003 | 11.479037 | 1523.562482 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 21.580365 |
| joint_full_g2 | 1194.244205 | 11.561209 | 1161.330968 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 21.401518 |
| joint_full_g4 | 1468.873308 | 11.519423 | 1435.927866 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 21.404577 |
| joint_full_g8 | 1538.199751 | 11.505063 | 1505.318303 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 21.413398 |

### Layer 2 (D=256, K=8, d=32 focus) phase medians

| Path | layer_ms | lr_ms | kernel_ms | projection_ms | conversion_ms | max_prescan_ms | normalization_ms | activation_ms |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| B0_FP32 | 1697.500833 | 17.975182 | 1452.581314 | 144.914609 | 0.000027 | 41.106715 | 19.386607 | 21.638679 |
| B0_BF16 | 1628.212626 | 17.844127 | 1447.136348 | 60.577101 | 19.752509 | 41.198706 | 19.382905 | 21.610545 |
| joint_v1_g2 | 2261.441580 | 18.842015 | 2220.238757 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 22.448346 |
| joint_v1_g8 | 2078.512102 | 18.744556 | 2037.376366 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 22.420008 |
| joint_full_g2 | 2252.299985 | 18.380432 | 2211.481218 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 22.396371 |
| joint_full_g4 | 2286.541619 | 18.317834 | 2245.900207 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 22.405616 |
| joint_full_g8 | 2091.764346 | 18.352606 | 2050.919864 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 22.425701 |

### Layer 3 phase medians

| Path | layer_ms | lr_ms | kernel_ms | projection_ms | conversion_ms | max_prescan_ms | normalization_ms | activation_ms |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| B0_FP32 | 524.201860 | 3.983064 | 422.066001 | 68.504055 | 0.000021 | 26.299478 | 3.349838 | 0.000115 |
| B0_BF16 | 489.574440 | 3.707556 | 422.046107 | 14.709611 | 19.626562 | 26.342656 | 3.349540 | 0.000037 |
| joint_v1_g2 | 1019.338031 | 19.091312 | 1000.303363 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000152 |
| joint_v1_g8 | 1009.711859 | 19.084713 | 990.611687 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000043 |
| joint_full_g2 | 965.993804 | 18.267521 | 947.751038 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000154 |
| joint_full_g4 | 955.457050 | 18.346577 | 936.998909 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000153 |
| joint_full_g8 | 963.874702 | 18.468185 | 945.406194 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000124 |

### Diagnostics: original log text

The following lines are reproduced without reinterpretation. Oracle selection may exclude costly high-degree rows; sampled correctness is not a full-graph accuracy guarantee.

#### Configuration and static preparation

```text
CONFIG N=2449029 E=126167053 Din=100 C=47 threads=16 warmups=1 repeats=3 seed=11 untrained=true block=32 panel=64 tile_rows=16 degree_sort_ms=327.827606 full_group_guards=compile_time_removed master_reference_gate=UNVERIFIED
STATIC layer=1 prepare_ms=0.381995 joint_U_scratch_bytes_per_worker=65536
STATIC layer=2 prepare_ms=0.753473 joint_U_scratch_bytes_per_worker=131072
STATIC layer=3 prepare_ms=0.163615 joint_U_scratch_bytes_per_worker=16384
```

#### CHECK, matched attention, oracle, PROFILE and online statistics

```text
CHECK path=full_fixed_input_g1 layer=1 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=1 group=1 bit_equal=1
CHECK path=full_fixed_input_g2 layer=1 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=1 group=2 bit_equal=1
CHECK path=full_fixed_input_g4 layer=1 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=1 group=4 bit_equal=1
CHECK path=full_fixed_input_g8 layer=1 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=1 group=8 bit_equal=1
CHECK path=full_fixed_input_g1 layer=2 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=2 group=1 bit_equal=1
CHECK path=full_fixed_input_g2 layer=2 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=2 group=2 bit_equal=1
CHECK path=full_fixed_input_g4 layer=2 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=2 group=4 bit_equal=1
CHECK path=full_fixed_input_g8 layer=2 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=2 group=8 bit_equal=1
CHECK path=full_fixed_input_g1 layer=3 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=3 group=1 bit_equal=1
CHECK path=full_fixed_input_g2 layer=3 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=3 group=2 bit_equal=1
CHECK path=full_fixed_input_g4 layer=3 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=3 group=4 bit_equal=1
CHECK path=full_fixed_input_g8 layer=3 reference=joint_v1_g8_same_input max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
MATCHED_IDENTITY layer=3 group=8 bit_equal=1
CHECK path=B0_FP32 layer=1 reference=B0_FP32_full_model max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
CHECK path=B0_FP32 layer=2 reference=B0_FP32_full_model max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
CHECK path=B0_FP32 layer=3 reference=B0_FP32_full_model max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 master_gate=UNVERIFIED
CHECK path=B0_BF16 layer=1 reference=B0_FP32_full_model max_abs_error=14.9706115723 mean_abs_error=0.00119692686635 relative_L2_error=0.00353063576973 finite=1 master_gate=UNVERIFIED
CHECK path=B0_BF16 layer=2 reference=B0_FP32_full_model max_abs_error=35.8837661743 mean_abs_error=0.00723928537782 relative_L2_error=0.011932822175 finite=1 master_gate=UNVERIFIED
CHECK path=B0_BF16 layer=3 reference=B0_FP32_full_model max_abs_error=25.1209583282 mean_abs_error=0.016643214169 relative_L2_error=0.0253783411542 finite=1 master_gate=UNVERIFIED
CHECK path=joint_v1_g2 layer=1 reference=B0_FP32_full_model max_abs_error=0.00606536865234 mean_abs_error=1.35905505803e-07 relative_L2_error=7.74767803114e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_v1_g2 layer=2 reference=B0_FP32_full_model max_abs_error=0.0416793823242 mean_abs_error=5.00723951823e-06 relative_L2_error=1.38032569677e-05 finite=1 master_gate=UNVERIFIED
CHECK path=joint_v1_g2 layer=3 reference=B0_FP32_full_model max_abs_error=0.00691890716553 mean_abs_error=9.83264858265e-06 relative_L2_error=1.49426901194e-05 finite=1 master_gate=UNVERIFIED
CHECK path=joint_v1_g8 layer=1 reference=B0_FP32_full_model max_abs_error=0.00606536865234 mean_abs_error=1.35905505803e-07 relative_L2_error=7.74767803114e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_v1_g8 layer=2 reference=B0_FP32_full_model max_abs_error=0.0416793823242 mean_abs_error=5.00723951823e-06 relative_L2_error=1.38032569677e-05 finite=1 master_gate=UNVERIFIED
CHECK path=joint_v1_g8 layer=3 reference=B0_FP32_full_model max_abs_error=0.00691890716553 mean_abs_error=9.83264858265e-06 relative_L2_error=1.49426901194e-05 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g2 layer=1 reference=B0_FP32_full_model max_abs_error=0.00606536865234 mean_abs_error=1.35905505803e-07 relative_L2_error=7.74767803114e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g2 layer=2 reference=B0_FP32_full_model max_abs_error=0.0416793823242 mean_abs_error=5.00723951823e-06 relative_L2_error=1.38032569677e-05 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g2 layer=3 reference=B0_FP32_full_model max_abs_error=0.00691890716553 mean_abs_error=9.83264858265e-06 relative_L2_error=1.49426901194e-05 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g4 layer=1 reference=B0_FP32_full_model max_abs_error=0.00606536865234 mean_abs_error=1.35905505803e-07 relative_L2_error=7.74767803114e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g4 layer=2 reference=B0_FP32_full_model max_abs_error=0.0416793823242 mean_abs_error=5.00723951823e-06 relative_L2_error=1.38032569677e-05 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g4 layer=3 reference=B0_FP32_full_model max_abs_error=0.00691890716553 mean_abs_error=9.83264858265e-06 relative_L2_error=1.49426901194e-05 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g8 layer=1 reference=B0_FP32_full_model max_abs_error=0.00606536865234 mean_abs_error=1.35905505803e-07 relative_L2_error=7.74767803114e-07 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g8 layer=2 reference=B0_FP32_full_model max_abs_error=0.0416793823242 mean_abs_error=5.00723951823e-06 relative_L2_error=1.38032569677e-05 finite=1 master_gate=UNVERIFIED
CHECK path=joint_full_g8 layer=3 reference=B0_FP32_full_model max_abs_error=0.00691890716553 mean_abs_error=9.83264858265e-06 relative_L2_error=1.49426901194e-05 finite=1 master_gate=UNVERIFIED
PROFILE path=joint_v1_g2 layer=1 layer_wall_ms=1254.746861 kernel_wall_ms=1221.71469 sampled_tiles=1306 init_worker_ms=1.25883 score_exp_den_worker_ms=61.923092 weighted_spmm_rescale_worker_ms=104.460985 UW_worker_ms=11.820364 normalization_scatter_worker_ms=1.847072 sampled_source_vector_loads=30847600 sampled_fma_vectors=61695200
PROFILE path=joint_v1_g2 layer=2 layer_wall_ms=2342.97448 kernel_wall_ms=2301.704024 sampled_tiles=1306 init_worker_ms=4.10885 score_exp_den_worker_ms=66.260749 weighted_spmm_rescale_worker_ms=227.977591 UW_worker_ms=21.959357 normalization_scatter_worker_ms=1.994898 sampled_source_vector_loads=70508800 sampled_fma_vectors=141017600
PROFILE path=joint_v1_g2 layer=3 layer_wall_ms=1041.937754 kernel_wall_ms=1023.071344 sampled_tiles=1306 init_worker_ms=0.662534 score_exp_den_worker_ms=20.5818 weighted_spmm_rescale_worker_ms=124.906057 UW_worker_ms=5.039473 normalization_scatter_worker_ms=0.589147 sampled_source_vector_loads=17627200 sampled_fma_vectors=17627200
PROFILE path=joint_v1_g8 layer=1 layer_wall_ms=1606.207345 kernel_wall_ms=1573.208173 sampled_tiles=1306 init_worker_ms=1.260645 score_exp_den_worker_ms=52.07807 weighted_spmm_rescale_worker_ms=152.683524 UW_worker_ms=12.033451 normalization_scatter_worker_ms=1.788497 sampled_source_vector_loads=7711900 sampled_fma_vectors=61695200
PROFILE path=joint_v1_g8 layer=2 layer_wall_ms=2138.192451 kernel_wall_ms=2096.950203 sampled_tiles=1306 init_worker_ms=4.033355 score_exp_den_worker_ms=56.051225 weighted_spmm_rescale_worker_ms=206.816624 UW_worker_ms=21.917169 normalization_scatter_worker_ms=2.031455 sampled_source_vector_loads=17627200 sampled_fma_vectors=141017600
PROFILE path=joint_v1_g8 layer=3 layer_wall_ms=1049.351728 kernel_wall_ms=1030.518145 sampled_tiles=1306 init_worker_ms=0.666947 score_exp_den_worker_ms=20.585236 weighted_spmm_rescale_worker_ms=125.49681 UW_worker_ms=5.031426 normalization_scatter_worker_ms=0.588145 sampled_source_vector_loads=17627200 sampled_fma_vectors=17627200
PROFILE path=joint_full_g2 layer=1 layer_wall_ms=1204.082126 kernel_wall_ms=1171.188178 sampled_tiles=1306 init_worker_ms=1.266077 score_exp_den_worker_ms=62.475586 weighted_spmm_rescale_worker_ms=102.275555 UW_worker_ms=11.789076 normalization_scatter_worker_ms=1.751415 sampled_source_vector_loads=30847600 sampled_fma_vectors=61695200
PROFILE path=joint_full_g2 layer=2 layer_wall_ms=2195.778374 kernel_wall_ms=2155.032651 sampled_tiles=1306 init_worker_ms=4.064317 score_exp_den_worker_ms=66.819861 weighted_spmm_rescale_worker_ms=225.218626 UW_worker_ms=21.764065 normalization_scatter_worker_ms=2.124505 sampled_source_vector_loads=70508800 sampled_fma_vectors=141017600
PROFILE path=joint_full_g2 layer=3 layer_wall_ms=1005.727899 kernel_wall_ms=987.348636 sampled_tiles=1306 init_worker_ms=0.664568 score_exp_den_worker_ms=20.089304 weighted_spmm_rescale_worker_ms=124.619067 UW_worker_ms=5.016306 normalization_scatter_worker_ms=0.6022 sampled_source_vector_loads=17627200 sampled_fma_vectors=17627200
PROFILE path=joint_full_g4 layer=1 layer_wall_ms=1504.03814 kernel_wall_ms=1471.094713 sampled_tiles=1306 init_worker_ms=1.248254 score_exp_den_worker_ms=55.676717 weighted_spmm_rescale_worker_ms=141.322994 UW_worker_ms=11.77201 normalization_scatter_worker_ms=1.828168 sampled_source_vector_loads=15423800 sampled_fma_vectors=61695200
PROFILE path=joint_full_g4 layer=2 layer_wall_ms=2303.674497 kernel_wall_ms=2262.993777 sampled_tiles=1306 init_worker_ms=4.098972 score_exp_den_worker_ms=60.216257 weighted_spmm_rescale_worker_ms=237.694191 UW_worker_ms=21.685159 normalization_scatter_worker_ms=2.225383 sampled_source_vector_loads=35254400 sampled_fma_vectors=141017600
PROFILE path=joint_full_g4 layer=3 layer_wall_ms=1015.011244 kernel_wall_ms=996.842143 sampled_tiles=1306 init_worker_ms=0.661548 score_exp_den_worker_ms=20.010338 weighted_spmm_rescale_worker_ms=124.039459 UW_worker_ms=5.036703 normalization_scatter_worker_ms=0.594084 sampled_source_vector_loads=17627200 sampled_fma_vectors=17627200
PROFILE path=joint_full_g8 layer=1 layer_wall_ms=1548.091632 kernel_wall_ms=1515.168112 sampled_tiles=1306 init_worker_ms=1.251145 score_exp_den_worker_ms=51.687759 weighted_spmm_rescale_worker_ms=149.32617 UW_worker_ms=11.845944 normalization_scatter_worker_ms=1.804118 sampled_source_vector_loads=7711900 sampled_fma_vectors=61695200
JOINT_ONLINE_STATS layer=1 blocks=42882792 running_max_updates_including_initial=25358082 rescales=5765850 rescaled_feature_elements=576585000 rescale_per_block=0.1344560308 row_head_ratio_P50=0 P90=0.3333333433 P95=0.5 P99=0.6666666865 rescale_timing=fused_with_weighted_spmm
PROFILE path=joint_full_g8 layer=2 layer_wall_ms=2095.944872 kernel_wall_ms=2055.225582 sampled_tiles=1306 init_worker_ms=4.048769 score_exp_den_worker_ms=56.152601 weighted_spmm_rescale_worker_ms=202.377827 UW_worker_ms=21.675846 normalization_scatter_worker_ms=2.184794 sampled_source_vector_loads=17627200 sampled_fma_vectors=141017600
JOINT_ONLINE_STATS layer=2 blocks=42882792 running_max_updates_including_initial=24069181 rescales=4476949 rescaled_feature_elements=1146098944 rescale_per_block=0.1043996622 row_head_ratio_P50=0 P90=0.3333333433 P95=0.5 P99=0.5 rescale_timing=fused_with_weighted_spmm
PROFILE path=joint_full_g8 layer=3 layer_wall_ms=1029.267674 kernel_wall_ms=1010.946551 sampled_tiles=1306 init_worker_ms=0.665654 score_exp_den_worker_ms=20.13028 weighted_spmm_rescale_worker_ms=124.773295 UW_worker_ms=5.051168 normalization_scatter_worker_ms=0.597229 sampled_source_vector_loads=17627200 sampled_fma_vectors=17627200
JOINT_ONLINE_STATS layer=3 blocks=5360349 running_max_updates_including_initial=3253695 rescales=804666 rescaled_feature_elements=205994496 rescale_per_block=0.1501144795 row_head_ratio_P50=0 P90=0.400000006 P95=0.5 P99=0.6666666865 rescale_timing=fused_with_weighted_spmm
JOINT_COMPLETE implementation_gate=PASS master_reference_gate=UNVERIFIED task_accuracy=UNVERIFIED performance=EXPLORATORY
```
