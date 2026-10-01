# Joint GAT experiment results

Run directory: `joint-10849337`. All times are milliseconds.

## Reading these results

- E2E median/min/max count each path/repetition once; only repetitions with all three layers are included. Warmup rows are excluded.
- Speedup = control E2E median / candidate E2E median. A ratio above 1 means the candidate is faster. This is a ratio of medians, not a median of paired ratios.
- `B1_TFS` is the existing B1 TFS BF16 path. `local_online_fp32` and all `joint_g*` paths are FP32 candidates; they are not improved B1 BF16.
- Each phase median is computed independently. Phase medians need not sum to the E2E median; E2E includes orchestration overhead.
- `PROFILE` values ending in `_worker_ms` are sampled worker-time sums, not an additive wall-time decomposition. Rescale is fused into weighted SpMM; its scalar exp also belongs to the score phase. Do not add worker sums to layer wall time.
- Zero stage fields can mean the work is inside another stage: joint/local UW and normalization are inside `kernel_ms`; a zero separate normalization field does not mean zero normalization cost.
- Logical source bytes and source-vector counters are source-code workload estimates, not measured DRAM traffic or cache-miss counts. Sparse arithmetic amplification remains.
- `UNVERIFIED` is preserved: an implementation comparison does not prove the original precision gate passed. CONFIG `untrained=true` means random-weight forward timing, not classification accuracy or end-to-end training performance.
- Degree Sort and parameter preparation costs are recorded separately in CONFIG/STATIC and excluded from the reported steady-state E2E timing.

## Dataset availability

| Dataset | Log | Complete marker | Failure marker | Measured paths |
| --- | --- | --- | --- | --- |
| arxiv | arxiv_joint.log | True | False | 8 |
| products | products_joint.log | True | False | 8 |

## arxiv

### E2E

| Path | Repeats | Median ms | Min ms | Max ms | Rep IDs |
| --- | --- | --- | --- | --- | --- |
| B0_FP32 | 3 | 74.015147 | 73.949877 | 74.065852 | 0,1,2 |
| B0_BF16 | 3 | 67.599089 | 67.488888 | 67.661604 | 0,1,2 |
| B1_TFS | 3 | 432.552320 | 430.579914 | 433.165942 | 0,1,2 |
| local_online_fp32 | 3 | 248.122118 | 244.635264 | 253.680410 | 0,1,2 |
| joint_g1 | 3 | 215.848719 | 214.125569 | 217.872815 | 0,1,2 |
| joint_g2 | 3 | 154.656529 | 150.789431 | 157.838047 | 0,1,2 |
| joint_g4 | 3 | 152.238040 | 151.465006 | 154.506285 | 0,1,2 |
| joint_g8 | 3 | 142.562127 | 141.131791 | 143.794925 | 0,1,2 |

### Speedup matrix

Rows are candidates; columns are controls. Missing controls appear as `-`.

| Candidate \ Control | B0_FP32 | B0_BF16 | B1_TFS | local_online_fp32 | joint_g1 | joint_g2 | joint_g4 | joint_g8 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| B0_FP32 | 1.0000 | 0.9133 | 5.8441 | 3.3523 | 2.9163 | 2.0895 | 2.0568 | 1.9261 |
| B0_BF16 | 1.0949 | 1.0000 | 6.3988 | 3.6705 | 3.1931 | 2.2878 | 2.2521 | 2.1089 |
| B1_TFS | 0.1711 | 0.1563 | 1.0000 | 0.5736 | 0.4990 | 0.3575 | 0.3520 | 0.3296 |
| local_online_fp32 | 0.2983 | 0.2724 | 1.7433 | 1.0000 | 0.8699 | 0.6233 | 0.6136 | 0.5746 |
| joint_g1 | 0.3429 | 0.3132 | 2.0040 | 1.1495 | 1.0000 | 0.7165 | 0.7053 | 0.6605 |
| joint_g2 | 0.4786 | 0.4371 | 2.7969 | 1.6043 | 1.3957 | 1.0000 | 0.9844 | 0.9218 |
| joint_g4 | 0.4862 | 0.4440 | 2.8413 | 1.6298 | 1.4178 | 1.0159 | 1.0000 | 0.9364 |
| joint_g8 | 0.5192 | 0.4742 | 3.0341 | 1.7404 | 1.5141 | 1.0848 | 1.0679 | 1.0000 |

### Layer 1 phase medians

| Path | layer_ms | lr_ms | kernel_ms | projection_ms | conversion_ms | max_prescan_ms | normalization_ms | activation_ms |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| B0_FP32 | 30.826264 | 1.308395 | 20.873249 | 5.309697 | 0.000030 | 0.640124 | 1.304947 | 1.438740 |
| B0_BF16 | 29.855893 | 1.299601 | 20.728900 | 3.868379 | 0.603347 | 0.632057 | 1.294956 | 1.450357 |
| B1_TFS | 131.526811 | 0.583960 | 126.182541 | 0.000000 | 0.609429 | 0.788258 | 1.230769 | 1.439529 |
| local_online_fp32 | 87.938563 | 0.873448 | 85.635070 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.424183 |
| joint_g1 | 73.714381 | 0.851072 | 71.359311 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.499828 |
| joint_g2 | 49.032131 | 0.855920 | 46.680573 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.498208 |
| joint_g4 | 50.643107 | 0.864189 | 48.281053 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.509188 |
| joint_g8 | 49.314639 | 0.855200 | 46.968583 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.512623 |

### Layer 2 (D=256, K=8, d=32 focus) phase medians

| Path | layer_ms | lr_ms | kernel_ms | projection_ms | conversion_ms | max_prescan_ms | normalization_ms | activation_ms |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| B0_FP32 | 35.450118 | 1.215859 | 20.734873 | 10.137101 | 0.000029 | 0.636961 | 1.239121 | 1.455583 |
| B0_BF16 | 30.639045 | 1.202463 | 20.691518 | 4.156520 | 1.256318 | 0.636552 | 1.242795 | 1.441717 |
| B1_TFS | 263.151850 | 0.832075 | 257.402604 | 0.000000 | 1.256321 | 0.765029 | 1.334617 | 1.539561 |
| local_online_fp32 | 135.224379 | 1.266257 | 132.527511 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.431745 |
| joint_g1 | 118.460875 | 1.336192 | 115.654067 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.476561 |
| joint_g2 | 80.171908 | 1.334136 | 77.375485 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.468705 |
| joint_g4 | 75.706624 | 1.325526 | 72.921071 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.467584 |
| joint_g8 | 67.616851 | 1.326701 | 64.804661 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 1.472046 |

### Layer 3 phase medians

| Path | layer_ms | lr_ms | kernel_ms | projection_ms | conversion_ms | max_prescan_ms | normalization_ms | activation_ms |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| B0_FP32 | 7.712767 | 0.239726 | 3.903314 | 2.974791 | 0.000027 | 0.409116 | 0.192733 | 0.000035 |
| B0_BF16 | 7.055100 | 0.226083 | 3.970893 | 0.993151 | 1.256072 | 0.409208 | 0.191407 | 0.000045 |
| B1_TFS | 38.483273 | 0.657818 | 35.891143 | 0.000000 | 1.334564 | 0.395857 | 0.199260 | 0.000035 |
| local_online_fp32 | 25.013403 | 1.253296 | 23.762203 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000160 |
| joint_g1 | 25.280568 | 1.260171 | 24.004310 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000041 |
| joint_g2 | 25.839768 | 1.252258 | 24.587086 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000081 |
| joint_g4 | 25.694882 | 1.259803 | 24.442797 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000040 |
| joint_g8 | 25.521008 | 1.258148 | 24.270281 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000031 |

### Diagnostics: original log text

The following lines are reproduced without reinterpretation. Oracle selection may exclude costly high-degree rows; sampled correctness is not a full-graph accuracy guarantee.

#### Configuration and static preparation

```text
CONFIG N=169343 E=2484941 Din=128 C=40 threads=16 warmups=1 repeats=3 block=32 panel=64 tile_rows=16 seed=11 untrained=true degree_sort_ms=16.782921 global_U=false global_Z_joint=false global_e_alpha_joint=false precision_joint=FP32 task_acceptance=UNVERIFIED
STATIC layer=1 prepare_ms=0.428463 joint_U_scratch_bytes_per_worker=65536
STATIC layer=2 prepare_ms=0.712509 joint_U_scratch_bytes_per_worker=131072
STATIC layer=3 prepare_ms=0.117247 joint_U_scratch_bytes_per_worker=16384
```

#### CHECK, matched attention, oracle, PROFILE and online statistics

```text
ORACLE_SELECTION path=common_input layer=1 graph_nodes=169343 graph_edges=2484941 selected_row_count=23 selected_edges=2444 projection_products=80084992 max_projection_products=100000000 effective_max_row_edges=488 oversized_graph_rows=171 global_max_degree_node=1353 global_max_degree=13162 quantiles=admissible_rows full_neighborhood=true coverage=sampled_only row_ids=0,107257,5,4,175,37,9604,2,274,184,544,300,634,2948,1055,1739,8512,508,33272,42015,43337,137967,169342
ORACLE_SKIPPED path=common_input layer=1 node=1353 degree=13162 reason=row_cost_limit
ATTENTION_DRIFT path=contracted_vs_B0 layer=1 row_heads=184 edge_heads=19552 left_max_abs=2.38418579102e-07 left_mean_abs=5.97640436077e-08 left_relative_L2=3.46345438438e-07 left_finite=true right_edge_weighted_max_abs=5.96046447754e-07 right_edge_weighted_mean_abs=5.91890826818e-08 right_edge_weighted_relative_L2=3.40540141089e-07 right_edge_weighted_finite=true score_max_abs=8.34465026855e-07 score_mean_abs=6.75827666403e-08 score_relative_L2=3.09521543388e-07 score_finite=true probability_max_abs=7.23102170364e-08 probability_mean_abs=4.53299254799e-10 probability_relative_L2=5.63614495595e-08 probability_finite=true max_score_node=42015 max_score_edge=619087 max_score_head=3 max_probability_node=169342 max_probability_edge=2484938 max_probability_head=3 same_input_required=true softmax_eval=FP64_stable coverage=sampled_only
ORACLE_CHECK path=FP64_fixed_attention_associativity layer=1 compared_values=5888 output_max_abs=1.16573417586e-15 output_mean_abs=8.1491555246e-17 output_relative_L2=5.47148325992e-16 output_finite=true max_error_node=107257 head=2 dim=7 reference_at_max=-0.339224562615 actual_at_max=-0.339224562615 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
ORACLE_CHECK path=B0_vs_fixed_attention_FP64 layer=1 compared_values=5888 output_max_abs=5.92451491643e-07 output_mean_abs=2.93662172672e-08 output_relative_L2=2.40923253487e-07 output_finite=true max_error_node=107257 head=2 dim=4 reference_at_max=0.615437504034 actual_at_max=0.615436911583 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_joint_g1 layer=1 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=1 group=1 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g1 layer=1 compared_values=5888 output_max_abs=3.22071150416e-07 output_mean_abs=4.02803776974e-08 output_relative_L2=2.53608487238e-07 output_finite=true max_error_node=4 head=3 dim=7 reference_at_max=-0.616731369701 actual_at_max=-0.61673104763 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g1 layer=1 reference=B0_same_input_same_LR max_abs_error=3.1590461731e-06 mean_abs_error=3.66445585974e-08 relative_L2_error=2.41498039705e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g2 layer=1 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=1 group=2 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g2 layer=1 compared_values=5888 output_max_abs=3.22071150416e-07 output_mean_abs=4.02803776974e-08 output_relative_L2=2.53608487238e-07 output_finite=true max_error_node=4 head=3 dim=7 reference_at_max=-0.616731369701 actual_at_max=-0.61673104763 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g2 layer=1 reference=B0_same_input_same_LR max_abs_error=3.1590461731e-06 mean_abs_error=3.66445585974e-08 relative_L2_error=2.41498039705e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g4 layer=1 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=1 group=4 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g4 layer=1 compared_values=5888 output_max_abs=3.22071150416e-07 output_mean_abs=4.02803776974e-08 output_relative_L2=2.53608487238e-07 output_finite=true max_error_node=4 head=3 dim=7 reference_at_max=-0.616731369701 actual_at_max=-0.61673104763 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g4 layer=1 reference=B0_same_input_same_LR max_abs_error=3.1590461731e-06 mean_abs_error=3.66445585974e-08 relative_L2_error=2.41498039705e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g8 layer=1 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=1 group=8 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g8 layer=1 compared_values=5888 output_max_abs=3.22071150416e-07 output_mean_abs=4.02803776974e-08 output_relative_L2=2.53608487238e-07 output_finite=true max_error_node=4 head=3 dim=7 reference_at_max=-0.616731369701 actual_at_max=-0.61673104763 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g8 layer=1 reference=B0_same_input_same_LR max_abs_error=3.1590461731e-06 mean_abs_error=3.66445585974e-08 relative_L2_error=2.41498039705e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=TF_attention_only_shift layer=1 reference=same_Z_original_B0_LR max_abs_error=4.17232513428e-07 mean_abs_error=9.50725719281e-09 relative_L2_error=8.47865475714e-08 finite=1 task_acceptance=UNVERIFIED
CHECK path=contracted_fixed_LR_TF_vs_AF layer=1 reference=TF_same_contracted_LR max_abs_error=3.09944152832e-06 mean_abs_error=3.6641821926e-08 relative_L2_error=2.41462526142e-07 finite=1 task_acceptance=UNVERIFIED
ORACLE_SELECTION path=common_input layer=2 graph_nodes=169343 graph_edges=2484941 selected_row_count=19 selected_edges=1135 projection_products=74383360 max_projection_products=100000000 effective_max_row_edges=244 oversized_graph_rows=473 global_max_degree_node=1353 global_max_degree=13162 quantiles=admissible_rows full_neighborhood=true coverage=sampled_only row_ids=26931,5,4,175,29,21692,2,274,184,544,300,634,2948,1055,1739,8512,508,33272,169342
ORACLE_SKIPPED path=common_input layer=2 node=0 degree=292 reason=row_cost_limit
ORACLE_SKIPPED path=common_input layer=2 node=1353 degree=13162 reason=row_cost_limit
ATTENTION_DRIFT path=contracted_vs_B0 layer=2 row_heads=152 edge_heads=9080 left_max_abs=3.57627868652e-07 left_mean_abs=7.68953836278e-08 left_relative_L2=4.715146189e-07 left_finite=true right_edge_weighted_max_abs=7.15255737305e-07 right_edge_weighted_mean_abs=8.63994260438e-08 right_edge_weighted_relative_L2=3.71224016792e-07 right_edge_weighted_finite=true score_max_abs=9.53674316406e-07 score_mean_abs=7.27571768988e-08 score_relative_L2=3.24272718598e-07 score_finite=true probability_max_abs=1.18838354379e-07 probability_mean_abs=9.09859867228e-10 probability_relative_L2=8.79787454642e-08 probability_finite=true max_score_node=33272 max_score_edge=488495 max_score_head=2 max_probability_node=169342 max_probability_edge=2484939 max_probability_head=2 same_input_required=true softmax_eval=FP64_stable coverage=sampled_only
ORACLE_CHECK path=FP64_fixed_attention_associativity layer=2 compared_values=4864 output_max_abs=7.77156117238e-16 output_mean_abs=9.35255009097e-17 output_relative_L2=6.25558153407e-16 output_finite=true max_error_node=508 head=1 dim=17 reference_at_max=0.552497252399 actual_at_max=0.552497252399 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
ORACLE_CHECK path=B0_vs_fixed_attention_FP64 layer=2 compared_values=4864 output_max_abs=2.85716986625e-07 output_mean_abs=2.46984426777e-08 output_relative_L2=1.80269986436e-07 output_finite=true max_error_node=8512 head=2 dim=30 reference_at_max=-0.528942811104 actual_at_max=-0.528942525387 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_joint_g1 layer=2 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=2 group=1 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g1 layer=2 compared_values=4864 output_max_abs=4.13263459065e-07 output_mean_abs=4.72079929975e-08 output_relative_L2=3.13289134381e-07 output_finite=true max_error_node=26931 head=0 dim=9 reference_at_max=-0.341080669557 actual_at_max=-0.341081082821 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g1 layer=2 reference=B0_same_input_same_LR max_abs_error=3.33786010742e-06 mean_abs_error=4.92604533766e-08 relative_L2_error=3.36275341348e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g2 layer=2 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=2 group=2 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g2 layer=2 compared_values=4864 output_max_abs=4.13263459065e-07 output_mean_abs=4.72079929975e-08 output_relative_L2=3.13289134381e-07 output_finite=true max_error_node=26931 head=0 dim=9 reference_at_max=-0.341080669557 actual_at_max=-0.341081082821 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g2 layer=2 reference=B0_same_input_same_LR max_abs_error=3.33786010742e-06 mean_abs_error=4.92604533766e-08 relative_L2_error=3.36275341348e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g4 layer=2 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=2 group=4 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g4 layer=2 compared_values=4864 output_max_abs=4.13263459065e-07 output_mean_abs=4.72079929975e-08 output_relative_L2=3.13289134381e-07 output_finite=true max_error_node=26931 head=0 dim=9 reference_at_max=-0.341080669557 actual_at_max=-0.341081082821 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g4 layer=2 reference=B0_same_input_same_LR max_abs_error=3.33786010742e-06 mean_abs_error=4.92604533766e-08 relative_L2_error=3.36275341348e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g8 layer=2 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=2 group=8 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g8 layer=2 compared_values=4864 output_max_abs=4.13263459065e-07 output_mean_abs=4.72079929975e-08 output_relative_L2=3.13289134381e-07 output_finite=true max_error_node=26931 head=0 dim=9 reference_at_max=-0.341080669557 actual_at_max=-0.341081082821 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g8 layer=2 reference=B0_same_input_same_LR max_abs_error=3.33786010742e-06 mean_abs_error=4.92604533766e-08 relative_L2_error=3.36275341348e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=TF_attention_only_shift layer=2 reference=same_Z_original_B0_LR max_abs_error=3.57627868652e-07 mean_abs_error=8.19503327191e-09 relative_L2_error=8.0061700749e-08 finite=1 task_acceptance=UNVERIFIED
CHECK path=contracted_fixed_LR_TF_vs_AF layer=2 reference=TF_same_contracted_LR max_abs_error=3.3974647522e-06 mean_abs_error=4.92492079805e-08 relative_L2_error=3.3622711625e-07 finite=1 task_acceptance=UNVERIFIED
ORACLE_SELECTION path=common_input layer=3 graph_nodes=169343 graph_edges=2484941 selected_row_count=24 selected_edges=4412 projection_products=45178880 max_projection_products=100000000 effective_max_row_edges=1562 oversized_graph_rows=29 global_max_degree_node=1353 global_max_degree=13162 quantiles=admissible_rows full_neighborhood=true coverage=sampled_only row_ids=0,87868,5,4,175,37,11234,2,274,184,544,300,634,2948,1055,1739,8512,508,33272,42015,43337,137967,143001,169342
ORACLE_SKIPPED path=common_input layer=3 node=1353 degree=13162 reason=row_cost_limit
ATTENTION_DRIFT path=contracted_vs_B0 layer=3 row_heads=24 edge_heads=4412 left_max_abs=2.38418579102e-07 left_mean_abs=7.40983523428e-08 left_relative_L2=2.80473506079e-06 left_finite=true right_edge_weighted_max_abs=3.57627868652e-07 right_edge_weighted_mean_abs=7.32308312751e-08 right_edge_weighted_relative_L2=3.42880557973e-07 right_edge_weighted_finite=true score_max_abs=1.00582838058e-07 score_mean_abs=1.72573313158e-08 score_relative_L2=4.36938369285e-07 score_finite=true probability_max_abs=8.26462670522e-09 probability_mean_abs=7.3078101518e-11 probability_relative_L2=1.57968089444e-08 probability_finite=true max_score_node=42015 max_score_edge=619082 max_score_head=0 max_probability_node=169342 max_probability_edge=2484939 max_probability_head=0 same_input_required=true softmax_eval=FP64_stable coverage=sampled_only
ORACLE_CHECK path=FP64_fixed_attention_associativity layer=3 compared_values=960 output_max_abs=8.32667268469e-16 output_mean_abs=9.43466165992e-17 output_relative_L2=6.66965615016e-16 output_finite=true max_error_node=43337 head=0 dim=11 reference_at_max=-0.443244893402 actual_at_max=-0.443244893402 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
ORACLE_CHECK path=B0_vs_fixed_attention_FP64 layer=3 compared_values=960 output_max_abs=8.66873651639e-07 output_mean_abs=3.05157262791e-08 output_relative_L2=3.1801605066e-07 output_finite=true max_error_node=87868 head=0 dim=32 reference_at_max=0.563755365832 actual_at_max=0.563754498959 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_joint_g1 layer=3 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=3 group=1 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g1 layer=3 compared_values=960 output_max_abs=3.27858463767e-07 output_mean_abs=4.92737659555e-08 output_relative_L2=3.31351387663e-07 output_finite=true max_error_node=143001 head=0 dim=11 reference_at_max=-0.471555799279 actual_at_max=-0.47155547142 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g1 layer=3 reference=B0_same_input_same_LR max_abs_error=1.84774398804e-06 mean_abs_error=4.40197497741e-08 relative_L2_error=3.04297609086e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g2 layer=3 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=3 group=2 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g2 layer=3 compared_values=960 output_max_abs=3.27858463767e-07 output_mean_abs=4.92737659555e-08 output_relative_L2=3.31351387663e-07 output_finite=true max_error_node=143001 head=0 dim=11 reference_at_max=-0.471555799279 actual_at_max=-0.47155547142 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g2 layer=3 reference=B0_same_input_same_LR max_abs_error=1.84774398804e-06 mean_abs_error=4.40197497741e-08 relative_L2_error=3.04297609086e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g4 layer=3 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=3 group=4 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g4 layer=3 compared_values=960 output_max_abs=3.27858463767e-07 output_mean_abs=4.92737659555e-08 output_relative_L2=3.31351387663e-07 output_finite=true max_error_node=143001 head=0 dim=11 reference_at_max=-0.471555799279 actual_at_max=-0.47155547142 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g4 layer=3 reference=B0_same_input_same_LR max_abs_error=1.84774398804e-06 mean_abs_error=4.40197497741e-08 relative_L2_error=3.04297609086e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g8 layer=3 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=3 group=8 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g8 layer=3 compared_values=960 output_max_abs=3.27858463767e-07 output_mean_abs=4.92737659555e-08 output_relative_L2=3.31351387663e-07 output_finite=true max_error_node=143001 head=0 dim=11 reference_at_max=-0.471555799279 actual_at_max=-0.47155547142 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g8 layer=3 reference=B0_same_input_same_LR max_abs_error=1.84774398804e-06 mean_abs_error=4.40197497741e-08 relative_L2_error=3.04297609086e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=TF_attention_only_shift layer=3 reference=same_Z_original_B0_LR max_abs_error=1.78813934326e-07 mean_abs_error=3.65816058638e-09 relative_L2_error=5.04697926307e-08 finite=1 task_acceptance=UNVERIFIED
CHECK path=contracted_fixed_LR_TF_vs_AF layer=3 reference=TF_same_contracted_LR max_abs_error=1.78813934326e-06 mean_abs_error=4.40349236072e-08 relative_L2_error=3.04365375074e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=B0_FP32 layer=1 reference=B0_FP32_full_model max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
CHECK path=B0_FP32 layer=2 reference=B0_FP32_full_model max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
CHECK path=B0_FP32 layer=3 reference=B0_FP32_full_model max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
CHECK path=B0_BF16 layer=1 reference=B0_FP32_full_model max_abs_error=0.00294017791748 mean_abs_error=0.000310601022267 relative_L2_error=0.00190785210806 finite=1 task_acceptance=UNVERIFIED
CHECK path=B0_BF16 layer=2 reference=B0_FP32_full_model max_abs_error=0.00309580564499 mean_abs_error=0.00039252464584 relative_L2_error=0.00253741964854 finite=1 task_acceptance=UNVERIFIED
CHECK path=B0_BF16 layer=3 reference=B0_FP32_full_model max_abs_error=0.00333481281996 mean_abs_error=0.000497330746597 relative_L2_error=0.00310539759334 finite=1 task_acceptance=UNVERIFIED
CHECK path=B1_TFS layer=1 reference=B0_FP32_full_model max_abs_error=0.00286803173367 mean_abs_error=0.000324894712624 relative_L2_error=0.00200438990919 finite=1 task_acceptance=UNVERIFIED
CHECK path=B1_TFS layer=2 reference=B0_FP32_full_model max_abs_error=0.00310099124908 mean_abs_error=0.000403808328807 relative_L2_error=0.00261475128793 finite=1 task_acceptance=UNVERIFIED
CHECK path=B1_TFS layer=3 reference=B0_FP32_full_model max_abs_error=0.00376975163817 mean_abs_error=0.000507920908992 relative_L2_error=0.00317701341922 finite=1 task_acceptance=UNVERIFIED
CHECK path=local_online_fp32 layer=1 reference=B0_FP32_full_model max_abs_error=2.98023223877e-06 mean_abs_error=3.37278126141e-08 relative_L2_error=2.47892636985e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=local_online_fp32 layer=2 reference=B0_FP32_full_model max_abs_error=2.32458114624e-06 mean_abs_error=5.11332031516e-08 relative_L2_error=3.73431298945e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=local_online_fp32 layer=3 reference=B0_FP32_full_model max_abs_error=1.81794166565e-06 mean_abs_error=5.53293051788e-08 relative_L2_error=3.67525850721e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g1 layer=1 reference=B0_FP32_full_model max_abs_error=2.98023223877e-06 mean_abs_error=3.37278126141e-08 relative_L2_error=2.47892636985e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g1 layer=2 reference=B0_FP32_full_model max_abs_error=2.32458114624e-06 mean_abs_error=5.11332031516e-08 relative_L2_error=3.73431298945e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g1 layer=3 reference=B0_FP32_full_model max_abs_error=1.81794166565e-06 mean_abs_error=5.53293051788e-08 relative_L2_error=3.67525850721e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g2 layer=1 reference=B0_FP32_full_model max_abs_error=2.98023223877e-06 mean_abs_error=3.37278126141e-08 relative_L2_error=2.47892636985e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g2 layer=2 reference=B0_FP32_full_model max_abs_error=2.32458114624e-06 mean_abs_error=5.11332031516e-08 relative_L2_error=3.73431298945e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g2 layer=3 reference=B0_FP32_full_model max_abs_error=1.81794166565e-06 mean_abs_error=5.53293051788e-08 relative_L2_error=3.67525850721e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g4 layer=1 reference=B0_FP32_full_model max_abs_error=2.98023223877e-06 mean_abs_error=3.37278126141e-08 relative_L2_error=2.47892636985e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g4 layer=2 reference=B0_FP32_full_model max_abs_error=2.32458114624e-06 mean_abs_error=5.11332031516e-08 relative_L2_error=3.73431298945e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g4 layer=3 reference=B0_FP32_full_model max_abs_error=1.81794166565e-06 mean_abs_error=5.53293051788e-08 relative_L2_error=3.67525850721e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g8 layer=1 reference=B0_FP32_full_model max_abs_error=2.98023223877e-06 mean_abs_error=3.37278126141e-08 relative_L2_error=2.47892636985e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g8 layer=2 reference=B0_FP32_full_model max_abs_error=2.32458114624e-06 mean_abs_error=5.11332031516e-08 relative_L2_error=3.73431298945e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g8 layer=3 reference=B0_FP32_full_model max_abs_error=1.81794166565e-06 mean_abs_error=5.53293051788e-08 relative_L2_error=3.67525850721e-07 finite=1 task_acceptance=UNVERIFIED
PROFILE path=joint_g8 layer=1 layer_wall_ms=49.957955 kernel_wall_ms=47.582832 sampled_tiles=96 sampled_neighbor_group_blocks=1728 sampled_row_head_blocks=13824 sampled_source_vector_loads=158360 sampled_fma_vectors=1266880 init_worker_ms=0.103822 score_exp_den_worker_ms=1.453632 weighted_spmm_rescale_worker_ms=2.506904 UW_worker_ms=0.918729 normalization_scatter_worker_ms=0.143177 U_global_bytes=0 rescale_timing=fused_with_weighted_spmm
JOINT_ONLINE_STATS layer=1 blocks=1580688 running_max_updates_including_initial=1400775 rescales=46031 rescaled_feature_elements=5891968 rescale_per_block=0.02912086383 row_head_ratio_P50=0 P90=0 P95=0 P99=0.5 rescale_timing=fused_with_weighted_spmm
PROFILE path=joint_g8 layer=2 layer_wall_ms=67.743162 kernel_wall_ms=64.952409 sampled_tiles=96 sampled_neighbor_group_blocks=1728 sampled_row_head_blocks=13824 sampled_source_vector_loads=316720 sampled_fma_vectors=2533760 init_worker_ms=0.281438 score_exp_den_worker_ms=1.497586 weighted_spmm_rescale_worker_ms=3.310469 UW_worker_ms=1.544737 normalization_scatter_worker_ms=0.147025 U_global_bytes=0 rescale_timing=fused_with_weighted_spmm
JOINT_ONLINE_STATS layer=2 blocks=1580688 running_max_updates_including_initial=1403398 rescales=48654 rescaled_feature_elements=12455424 rescale_per_block=0.03078026783 row_head_ratio_P50=0 P90=0 P95=0 P99=0.5 rescale_timing=fused_with_weighted_spmm
PROFILE path=joint_g8 layer=3 layer_wall_ms=26.922288 kernel_wall_ms=25.64513 sampled_tiles=96 sampled_neighbor_group_blocks=1728 sampled_row_head_blocks=1728 sampled_source_vector_loads=316720 sampled_fma_vectors=316720 init_worker_ms=0.04517 score_exp_den_worker_ms=0.482116 weighted_spmm_rescale_worker_ms=1.613931 UW_worker_ms=0.330107 normalization_scatter_worker_ms=0.03789 U_global_bytes=0 rescale_timing=fused_with_weighted_spmm
JOINT_ONLINE_STATS layer=3 blocks=197586 running_max_updates_including_initial=175516 rescales=6173 rescaled_feature_elements=1580288 rescale_per_block=0.03124209205 row_head_ratio_P50=0 P90=0 P95=0 P99=0.5 rescale_timing=fused_with_weighted_spmm
JOINT_COMPLETE implementation_gate=PASS master_reference_gate=UNVERIFIED task_accuracy=UNVERIFIED performance=EXPLORATORY
```

## products

### E2E

| Path | Repeats | Median ms | Min ms | Max ms | Rep IDs |
| --- | --- | --- | --- | --- | --- |
| B0_FP32 | 3 | 3820.758131 | 3819.119030 | 3823.576802 | 0,1,2 |
| B0_BF16 | 3 | 3723.108644 | 3717.187988 | 3740.246371 | 0,1,2 |
| B1_TFS | 3 | 15698.649803 | 15656.267157 | 15698.654560 | 0,1,2 |
| local_online_fp32 | 3 | 7982.355176 | 7974.643166 | 8165.258323 | 0,1,2 |
| joint_g1 | 3 | 6380.318251 | 6332.123771 | 6396.393801 | 0,1,2 |
| joint_g2 | 3 | 4499.233311 | 4465.397412 | 4554.580679 | 0,1,2 |
| joint_g4 | 3 | 4940.558192 | 4914.221103 | 4974.951980 | 0,1,2 |
| joint_g8 | 3 | 4661.920985 | 4653.891479 | 4713.941385 | 0,1,2 |

### Speedup matrix

Rows are candidates; columns are controls. Missing controls appear as `-`.

| Candidate \ Control | B0_FP32 | B0_BF16 | B1_TFS | local_online_fp32 | joint_g1 | joint_g2 | joint_g4 | joint_g8 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| B0_FP32 | 1.0000 | 0.9744 | 4.1088 | 2.0892 | 1.6699 | 1.1776 | 1.2931 | 1.2202 |
| B0_BF16 | 1.0262 | 1.0000 | 4.2165 | 2.1440 | 1.7137 | 1.2085 | 1.3270 | 1.2522 |
| B1_TFS | 0.2434 | 0.2372 | 1.0000 | 0.5085 | 0.4064 | 0.2866 | 0.3147 | 0.2970 |
| local_online_fp32 | 0.4787 | 0.4664 | 1.9667 | 1.0000 | 0.7993 | 0.5636 | 0.6189 | 0.5840 |
| joint_g1 | 0.5988 | 0.5835 | 2.4605 | 1.2511 | 1.0000 | 0.7052 | 0.7743 | 0.7307 |
| joint_g2 | 0.8492 | 0.8275 | 3.4892 | 1.7742 | 1.4181 | 1.0000 | 1.0981 | 1.0362 |
| joint_g4 | 0.7733 | 0.7536 | 3.1775 | 1.6157 | 1.2914 | 0.9107 | 1.0000 | 0.9436 |
| joint_g8 | 0.8196 | 0.7986 | 3.3674 | 1.7122 | 1.3686 | 0.9651 | 1.0598 | 1.0000 |

### Layer 1 phase medians

| Path | layer_ms | lr_ms | kernel_ms | projection_ms | conversion_ms | max_prescan_ms | normalization_ms | activation_ms |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| B0_FP32 | 1609.160555 | 17.724303 | 1447.643776 | 62.293572 | 0.000038 | 41.062341 | 19.262966 | 21.273689 |
| B0_BF16 | 1612.240999 | 17.822227 | 1449.545540 | 53.052443 | 10.061859 | 41.044915 | 19.262573 | 21.299446 |
| B1_TFS | 5304.312110 | 6.551147 | 5205.311177 | 0.000000 | 10.194856 | 41.083853 | 18.689508 | 21.250576 |
| local_online_fp32 | 2341.148571 | 12.384598 | 2307.204601 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 21.534257 |
| joint_g1 | 1752.396454 | 11.461047 | 1719.863477 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 21.626459 |
| joint_g2 | 1230.076830 | 10.865477 | 1197.567341 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 21.650235 |
| joint_g4 | 1544.148732 | 10.900977 | 1511.485210 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 21.685959 |
| joint_g8 | 1563.220659 | 10.903591 | 1530.698154 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 21.613708 |

### Layer 2 (D=256, K=8, d=32 focus) phase medians

| Path | layer_ms | lr_ms | kernel_ms | projection_ms | conversion_ms | max_prescan_ms | normalization_ms | activation_ms |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| B0_FP32 | 1688.217488 | 17.579321 | 1441.149689 | 145.173059 | 0.000036 | 41.138154 | 20.164980 | 22.498827 |
| B0_BF16 | 1626.604403 | 17.554293 | 1445.424513 | 61.099869 | 20.272793 | 41.152306 | 20.150325 | 22.470541 |
| B1_TFS | 9116.377329 | 11.601199 | 9004.051798 | 0.000000 | 19.253190 | 41.155900 | 19.138748 | 21.379032 |
| local_online_fp32 | 4652.281602 | 18.715074 | 4611.733741 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 21.861632 |
| joint_g1 | 3591.152019 | 18.603654 | 3550.415957 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 22.131919 |
| joint_g2 | 2243.382134 | 18.471624 | 2202.815713 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 22.114583 |
| joint_g4 | 2373.065140 | 18.467256 | 2332.456822 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 22.095079 |
| joint_g8 | 2083.908382 | 18.524599 | 2043.368946 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 22.085742 |

### Layer 3 phase medians

| Path | layer_ms | lr_ms | kernel_ms | projection_ms | conversion_ms | max_prescan_ms | normalization_ms | activation_ms |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| B0_FP32 | 522.549231 | 3.703544 | 420.950018 | 68.577405 | 0.000019 | 25.989413 | 3.334587 | 0.000026 |
| B0_BF16 | 489.720452 | 3.541060 | 421.023938 | 14.712070 | 21.065363 | 26.078471 | 3.328347 | 0.000026 |
| B1_TFS | 1269.038699 | 8.452239 | 1210.429521 | 0.000000 | 19.748866 | 25.958021 | 3.495716 | 0.000026 |
| local_online_fp32 | 987.091493 | 18.188496 | 968.775390 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000150 |
| joint_g1 | 1004.685533 | 18.529438 | 986.152916 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000158 |
| joint_g2 | 1020.746257 | 18.451946 | 1002.164819 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000163 |
| joint_g4 | 1008.626649 | 18.560034 | 990.049900 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000146 |
| joint_g8 | 1021.174131 | 18.629390 | 1002.568004 | 0.000000 | 0.000000 | 0.000000 | 0.000000 | 0.000151 |

### Diagnostics: original log text

The following lines are reproduced without reinterpretation. Oracle selection may exclude costly high-degree rows; sampled correctness is not a full-graph accuracy guarantee.

#### Configuration and static preparation

```text
CONFIG N=2449029 E=126167053 Din=100 C=47 threads=16 warmups=1 repeats=3 block=32 panel=64 tile_rows=16 seed=11 untrained=true degree_sort_ms=329.211701 global_U=false global_Z_joint=false global_e_alpha_joint=false precision_joint=FP32 task_acceptance=UNVERIFIED
STATIC layer=1 prepare_ms=7.802357 joint_U_scratch_bytes_per_worker=65536
STATIC layer=2 prepare_ms=0.74288 joint_U_scratch_bytes_per_worker=131072
STATIC layer=3 prepare_ms=0.168582 joint_U_scratch_bytes_per_worker=16384
```

#### CHECK, matched attention, oracle, PROFILE and online statistics

```text
ORACLE_SELECTION path=common_input layer=1 graph_nodes=2449029 graph_edges=126167053 selected_row_count=24 selected_edges=2955 projection_products=75648000 max_projection_products=100000000 effective_max_row_edges=625 oversized_graph_rows=8500 global_max_degree_node=86036 global_max_degree=17482 quantiles=admissible_rows full_neighborhood=true coverage=sampled_only row_ids=0,7939,4677,253,304,1761,2125,188,68,262,78,28,177,56,234,233,17,278,174,265,136,123,1446,2449028
ORACLE_SKIPPED path=common_input layer=1 node=86036 degree=17482 reason=row_cost_limit
ATTENTION_DRIFT path=contracted_vs_B0 layer=1 row_heads=192 edge_heads=23640 left_max_abs=8.34465026855e-07 left_mean_abs=1.11190956886e-07 left_relative_L2=3.13632429068e-07 left_finite=true right_edge_weighted_max_abs=7.62939453125e-06 right_edge_weighted_mean_abs=1.30908337263e-07 right_edge_weighted_relative_L2=2.10451088941e-07 right_edge_weighted_finite=true score_max_abs=5.72204589844e-06 score_mean_abs=1.06845091191e-07 score_relative_L2=2.58537832068e-07 score_finite=true probability_max_abs=4.42338058182e-07 probability_mean_abs=1.05125988066e-09 probability_relative_L2=2.09874912265e-07 probability_finite=true max_score_node=234 max_score_edge=36621 max_score_head=2 max_probability_node=1446 max_probability_edge=241816 max_probability_head=2 same_input_required=true softmax_eval=FP64_stable coverage=sampled_only
ORACLE_CHECK path=FP64_fixed_attention_associativity layer=1 compared_values=6144 output_max_abs=2.84217094304e-14 output_mean_abs=4.84491999837e-16 output_relative_L2=3.85868095072e-16 output_finite=true max_error_node=234 head=1 dim=27 reference_at_max=70.379656345 actual_at_max=70.379656345 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
ORACLE_CHECK path=B0_vs_fixed_attention_FP64 layer=1 compared_values=6144 output_max_abs=2.30784184581e-05 output_mean_abs=2.47974634094e-07 output_relative_L2=2.38314363205e-07 output_finite=true max_error_node=234 head=1 dim=12 reference_at_max=46.7784193044 actual_at_max=46.7784423828 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_joint_g1 layer=1 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=1 group=1 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g1 layer=1 compared_values=6144 output_max_abs=9.59039572024e-06 output_mean_abs=1.96148770673e-07 output_relative_L2=1.30897030019e-07 output_finite=true max_error_node=234 head=2 dim=8 reference_at_max=79.7104854047 actual_at_max=79.7104949951 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g1 layer=1 reference=B0_same_input_same_LR max_abs_error=0.00605010986328 mean_abs_error=1.72347290289e-07 relative_L2_error=7.12726936108e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g2 layer=1 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=1 group=2 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g2 layer=1 compared_values=6144 output_max_abs=9.59039572024e-06 output_mean_abs=1.96148770673e-07 output_relative_L2=1.30897030019e-07 output_finite=true max_error_node=234 head=2 dim=8 reference_at_max=79.7104854047 actual_at_max=79.7104949951 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g2 layer=1 reference=B0_same_input_same_LR max_abs_error=0.00605010986328 mean_abs_error=1.72347290289e-07 relative_L2_error=7.12726936108e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g4 layer=1 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=1 group=4 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g4 layer=1 compared_values=6144 output_max_abs=9.59039572024e-06 output_mean_abs=1.96148770673e-07 output_relative_L2=1.30897030019e-07 output_finite=true max_error_node=234 head=2 dim=8 reference_at_max=79.7104854047 actual_at_max=79.7104949951 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g4 layer=1 reference=B0_same_input_same_LR max_abs_error=0.00605010986328 mean_abs_error=1.72347290289e-07 relative_L2_error=7.12726936108e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g8 layer=1 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=1 group=8 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g8 layer=1 compared_values=6144 output_max_abs=9.59039572024e-06 output_mean_abs=1.96148770673e-07 output_relative_L2=1.30897030019e-07 output_finite=true max_error_node=234 head=2 dim=8 reference_at_max=79.7104854047 actual_at_max=79.7104949951 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g8 layer=1 reference=B0_same_input_same_LR max_abs_error=0.00605010986328 mean_abs_error=1.72347290289e-07 relative_L2_error=7.12726936108e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=TF_attention_only_shift layer=1 reference=same_Z_original_B0_LR max_abs_error=0.00163269042969 mean_abs_error=9.61353499838e-08 relative_L2_error=2.25861410762e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=contracted_fixed_LR_TF_vs_AF layer=1 reference=TF_same_contracted_LR max_abs_error=0.00606536865234 mean_abs_error=1.72336394855e-07 relative_L2_error=7.12734460133e-07 finite=1 task_acceptance=UNVERIFIED
ORACLE_SELECTION path=common_input layer=2 graph_nodes=2449029 graph_edges=126167053 selected_row_count=20 selected_edges=1519 projection_products=99549184 max_projection_products=100000000 effective_max_row_edges=244 oversized_graph_rows=57920 global_max_degree_node=86036 global_max_degree=17482 quantiles=admissible_rows full_neighborhood=true coverage=sampled_only row_ids=0,245,4677,75,455,574,545,188,68,262,78,28,177,56,234,233,17,278,174,2449028
ORACLE_SKIPPED path=common_input layer=2 node=86036 degree=17482 reason=row_cost_limit
ORACLE_SKIPPED path=common_input layer=2 node=265 degree=129 reason=total_cost_limit
ATTENTION_DRIFT path=contracted_vs_B0 layer=2 row_heads=160 edge_heads=12152 left_max_abs=1.52587890625e-05 left_mean_abs=5.11860707775e-07 left_relative_L2=3.83990473507e-07 left_finite=true right_edge_weighted_max_abs=3.81469726562e-05 right_edge_weighted_mean_abs=6.15928546793e-07 right_edge_weighted_relative_L2=4.39735342638e-07 right_edge_weighted_finite=true score_max_abs=4.19616699219e-05 score_mean_abs=6.05809970973e-07 score_relative_L2=6.28067043832e-07 score_finite=true probability_max_abs=1.5228414898e-06 probability_mean_abs=7.76885559058e-09 probability_relative_L2=7.42601994027e-07 probability_finite=true max_score_node=234 max_score_edge=36632 max_score_head=5 max_probability_node=233 max_probability_edge=36538 max_probability_head=7 same_input_required=true softmax_eval=FP64_stable coverage=sampled_only
ORACLE_CHECK path=FP64_fixed_attention_associativity layer=2 compared_values=5120 output_max_abs=5.68434188608e-14 output_mean_abs=1.6394619561e-15 output_relative_L2=6.87512078475e-16 output_finite=true max_error_node=233 head=3 dim=29 reference_at_max=32.0990298121 actual_at_max=32.0990298121 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
ORACLE_CHECK path=B0_vs_fixed_attention_FP64 layer=2 compared_values=5120 output_max_abs=2.53076531465e-05 output_mean_abs=7.3602846156e-07 output_relative_L2=3.24418155731e-07 output_finite=true max_error_node=0 head=7 dim=19 reference_at_max=18.79722544 actual_at_max=18.7972507477 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_joint_g1 layer=2 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=2 group=1 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g1 layer=2 compared_values=5120 output_max_abs=1.89238159365e-05 output_mean_abs=7.53520699487e-07 output_relative_L2=3.10192857689e-07 output_finite=true max_error_node=234 head=7 dim=29 reference_at_max=57.9737931795 actual_at_max=57.9738121033 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g1 layer=2 reference=B0_same_input_same_LR max_abs_error=0.0223770141602 mean_abs_error=8.28826236932e-07 relative_L2_error=9.18400470477e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g2 layer=2 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=2 group=2 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g2 layer=2 compared_values=5120 output_max_abs=1.89238159365e-05 output_mean_abs=7.53520699487e-07 output_relative_L2=3.10192857689e-07 output_finite=true max_error_node=234 head=7 dim=29 reference_at_max=57.9737931795 actual_at_max=57.9738121033 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g2 layer=2 reference=B0_same_input_same_LR max_abs_error=0.0223770141602 mean_abs_error=8.28826236932e-07 relative_L2_error=9.18400470477e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g4 layer=2 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=2 group=4 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g4 layer=2 compared_values=5120 output_max_abs=1.89238159365e-05 output_mean_abs=7.53520699487e-07 output_relative_L2=3.10192857689e-07 output_finite=true max_error_node=234 head=7 dim=29 reference_at_max=57.9737931795 actual_at_max=57.9738121033 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g4 layer=2 reference=B0_same_input_same_LR max_abs_error=0.0223770141602 mean_abs_error=8.28826236932e-07 relative_L2_error=9.18400470477e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g8 layer=2 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=2 group=8 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g8 layer=2 compared_values=5120 output_max_abs=1.89238159365e-05 output_mean_abs=7.53520699487e-07 output_relative_L2=3.10192857689e-07 output_finite=true max_error_node=234 head=7 dim=29 reference_at_max=57.9737931795 actual_at_max=57.9738121033 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g8 layer=2 reference=B0_same_input_same_LR max_abs_error=0.0223770141602 mean_abs_error=8.28826236932e-07 relative_L2_error=9.18400470477e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=TF_attention_only_shift layer=2 reference=same_Z_original_B0_LR max_abs_error=0.00836181640625 mean_abs_error=6.50602351533e-07 relative_L2_error=8.93894539167e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=contracted_fixed_LR_TF_vs_AF layer=2 reference=TF_same_contracted_LR max_abs_error=0.0223770141602 mean_abs_error=8.29010190135e-07 relative_L2_error=9.18361591172e-07 finite=1 task_acceptance=UNVERIFIED
ORACLE_SELECTION path=common_input layer=3 graph_nodes=2449029 graph_edges=126167053 selected_row_count=27 selected_edges=6778 projection_products=81552896 max_projection_products=100000000 effective_max_row_edges=1329 oversized_graph_rows=1480 global_max_degree_node=86036 global_max_degree=17482 quantiles=admissible_rows full_neighborhood=true coverage=sampled_only row_ids=0,31634,4677,253,247,873,5252,188,68,262,78,28,177,56,234,233,17,278,174,265,136,123,1446,16083,7869,45256,2449028
ORACLE_SKIPPED path=common_input layer=3 node=86036 degree=17482 reason=row_cost_limit
ATTENTION_DRIFT path=contracted_vs_B0 layer=3 row_heads=27 edge_heads=6778 left_max_abs=9.53674316406e-06 left_mean_abs=1.30426552561e-06 left_relative_L2=4.07352273533e-07 left_finite=true right_edge_weighted_max_abs=3.0517578125e-05 right_edge_weighted_mean_abs=1.25884377159e-06 right_edge_weighted_relative_L2=2.54595338512e-07 right_edge_weighted_finite=true score_max_abs=9.53674316406e-06 score_mean_abs=8.48489019725e-07 score_relative_L2=1.76769519318e-07 score_finite=true probability_max_abs=8.7140019922e-07 probability_mean_abs=1.43548475561e-09 probability_relative_L2=6.52683928497e-07 probability_finite=true max_score_node=233 max_score_edge=36516 max_score_head=0 max_probability_node=45256 max_probability_edge=7790776 max_probability_head=0 same_input_required=true softmax_eval=FP64_stable coverage=sampled_only
ORACLE_CHECK path=FP64_fixed_attention_associativity layer=3 compared_values=1269 output_max_abs=4.26325641456e-14 output_mean_abs=1.49151219453e-15 output_relative_L2=9.44328592628e-16 output_finite=true max_error_node=45256 head=0 dim=20 reference_at_max=29.5138853155 actual_at_max=29.5138853155 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
ORACLE_CHECK path=B0_vs_fixed_attention_FP64 layer=3 compared_values=1269 output_max_abs=3.03585874804e-05 output_mean_abs=6.53706766346e-07 output_relative_L2=5.88688913654e-07 output_finite=true max_error_node=45256 head=0 dim=25 reference_at_max=-15.6557682355 actual_at_max=-15.6557378769 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_joint_g1 layer=3 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=3 group=1 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g1 layer=3 compared_values=1269 output_max_abs=2.97121554609e-05 output_mean_abs=9.3119552966e-07 output_relative_L2=7.16005425279e-07 output_finite=true max_error_node=45256 head=0 dim=24 reference_at_max=0.399447054431 actual_at_max=0.399476766586 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g1 layer=3 reference=B0_same_input_same_LR max_abs_error=0.00084114074707 mean_abs_error=5.71405685952e-07 relative_L2_error=4.66209920535e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g2 layer=3 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=3 group=2 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g2 layer=3 compared_values=1269 output_max_abs=2.97121554609e-05 output_mean_abs=9.3119552966e-07 output_relative_L2=7.16005425279e-07 output_finite=true max_error_node=45256 head=0 dim=24 reference_at_max=0.399447054431 actual_at_max=0.399476766586 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g2 layer=3 reference=B0_same_input_same_LR max_abs_error=0.00084114074707 mean_abs_error=5.71405685952e-07 relative_L2_error=4.66209920535e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g4 layer=3 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=3 group=4 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g4 layer=3 compared_values=1269 output_max_abs=2.97121554609e-05 output_mean_abs=9.3119552966e-07 output_relative_L2=7.16005425279e-07 output_finite=true max_error_node=45256 head=0 dim=24 reference_at_max=0.399447054431 actual_at_max=0.399476766586 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g4 layer=3 reference=B0_same_input_same_LR max_abs_error=0.00084114074707 mean_abs_error=5.71405685952e-07 relative_L2_error=4.66209920535e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=fixed_LR_joint_g8 layer=3 reference=local_same_input_same_LR max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
MATCHED_IDENTITY layer=3 group=8 bit_equal=1
ORACLE_CHECK path=joint_fixed_LR_g8 layer=3 compared_values=1269 output_max_abs=2.97121554609e-05 output_mean_abs=9.3119552966e-07 output_relative_L2=7.16005425279e-07 output_finite=true max_error_node=45256 head=0 dim=24 reference_at_max=0.399447054431 actual_at_max=0.399476766586 score_eval=FP32_add_LeakyReLU softmax_eval=FP64_stable coverage=sampled_only
CHECK path=fixed_LR_TF_vs_AF_g8 layer=3 reference=B0_same_input_same_LR max_abs_error=0.00084114074707 mean_abs_error=5.71405685952e-07 relative_L2_error=4.66209920535e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=TF_attention_only_shift layer=3 reference=same_Z_original_B0_LR max_abs_error=0.000245571136475 mean_abs_error=3.80693058686e-07 relative_L2_error=4.07792627223e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=contracted_fixed_LR_TF_vs_AF layer=3 reference=TF_same_contracted_LR max_abs_error=0.000846862792969 mean_abs_error=5.71306587532e-07 relative_L2_error=4.65919886515e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=B0_FP32 layer=1 reference=B0_FP32_full_model max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
CHECK path=B0_FP32 layer=2 reference=B0_FP32_full_model max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
CHECK path=B0_FP32 layer=3 reference=B0_FP32_full_model max_abs_error=0 mean_abs_error=0 relative_L2_error=0 finite=1 task_acceptance=UNVERIFIED
CHECK path=B0_BF16 layer=1 reference=B0_FP32_full_model max_abs_error=14.9706115723 mean_abs_error=0.00119692686635 relative_L2_error=0.00353063576973 finite=1 task_acceptance=UNVERIFIED
CHECK path=B0_BF16 layer=2 reference=B0_FP32_full_model max_abs_error=35.8837661743 mean_abs_error=0.00723928537782 relative_L2_error=0.011932822175 finite=1 task_acceptance=UNVERIFIED
CHECK path=B0_BF16 layer=3 reference=B0_FP32_full_model max_abs_error=25.1209583282 mean_abs_error=0.016643214169 relative_L2_error=0.0253783411542 finite=1 task_acceptance=UNVERIFIED
CHECK path=B1_TFS layer=1 reference=B0_FP32_full_model max_abs_error=9.70662689209 mean_abs_error=0.0012258057698 relative_L2_error=0.00325191592453 finite=1 task_acceptance=UNVERIFIED
CHECK path=B1_TFS layer=2 reference=B0_FP32_full_model max_abs_error=35.2581710815 mean_abs_error=0.00740328380953 relative_L2_error=0.00977656889393 finite=1 task_acceptance=UNVERIFIED
CHECK path=B1_TFS layer=3 reference=B0_FP32_full_model max_abs_error=11.5481319427 mean_abs_error=0.017482932183 relative_L2_error=0.0200899465651 finite=1 task_acceptance=UNVERIFIED
CHECK path=local_online_fp32 layer=1 reference=B0_FP32_full_model max_abs_error=0.00606536865234 mean_abs_error=1.35905505803e-07 relative_L2_error=7.74767803114e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=local_online_fp32 layer=2 reference=B0_FP32_full_model max_abs_error=0.0416793823242 mean_abs_error=5.00723951823e-06 relative_L2_error=1.38032569677e-05 finite=1 task_acceptance=UNVERIFIED
CHECK path=local_online_fp32 layer=3 reference=B0_FP32_full_model max_abs_error=0.00691890716553 mean_abs_error=9.83264858265e-06 relative_L2_error=1.49426901194e-05 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g1 layer=1 reference=B0_FP32_full_model max_abs_error=0.00606536865234 mean_abs_error=1.35905505803e-07 relative_L2_error=7.74767803114e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g1 layer=2 reference=B0_FP32_full_model max_abs_error=0.0416793823242 mean_abs_error=5.00723951823e-06 relative_L2_error=1.38032569677e-05 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g1 layer=3 reference=B0_FP32_full_model max_abs_error=0.00691890716553 mean_abs_error=9.83264858265e-06 relative_L2_error=1.49426901194e-05 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g2 layer=1 reference=B0_FP32_full_model max_abs_error=0.00606536865234 mean_abs_error=1.35905505803e-07 relative_L2_error=7.74767803114e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g2 layer=2 reference=B0_FP32_full_model max_abs_error=0.0416793823242 mean_abs_error=5.00723951823e-06 relative_L2_error=1.38032569677e-05 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g2 layer=3 reference=B0_FP32_full_model max_abs_error=0.00691890716553 mean_abs_error=9.83264858265e-06 relative_L2_error=1.49426901194e-05 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g4 layer=1 reference=B0_FP32_full_model max_abs_error=0.00606536865234 mean_abs_error=1.35905505803e-07 relative_L2_error=7.74767803114e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g4 layer=2 reference=B0_FP32_full_model max_abs_error=0.0416793823242 mean_abs_error=5.00723951823e-06 relative_L2_error=1.38032569677e-05 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g4 layer=3 reference=B0_FP32_full_model max_abs_error=0.00691890716553 mean_abs_error=9.83264858265e-06 relative_L2_error=1.49426901194e-05 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g8 layer=1 reference=B0_FP32_full_model max_abs_error=0.00606536865234 mean_abs_error=1.35905505803e-07 relative_L2_error=7.74767803114e-07 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g8 layer=2 reference=B0_FP32_full_model max_abs_error=0.0416793823242 mean_abs_error=5.00723951823e-06 relative_L2_error=1.38032569677e-05 finite=1 task_acceptance=UNVERIFIED
CHECK path=joint_g8 layer=3 reference=B0_FP32_full_model max_abs_error=0.00691890716553 mean_abs_error=9.83264858265e-06 relative_L2_error=1.49426901194e-05 finite=1 task_acceptance=UNVERIFIED
PROFILE path=joint_g8 layer=1 layer_wall_ms=1613.269993 kernel_wall_ms=1580.467953 sampled_tiles=1306 sampled_neighbor_group_blocks=46688 sampled_row_head_blocks=373504 sampled_source_vector_loads=7711900 sampled_fma_vectors=61695200 init_worker_ms=1.318932 score_exp_den_worker_ms=53.043145 weighted_spmm_rescale_worker_ms=152.892358 UW_worker_ms=11.78361 normalization_scatter_worker_ms=1.860419 U_global_bytes=0 rescale_timing=fused_with_weighted_spmm
JOINT_ONLINE_STATS layer=1 blocks=42882792 running_max_updates_including_initial=25358082 rescales=5765850 rescaled_feature_elements=576585000 rescale_per_block=0.1344560308 row_head_ratio_P50=0 P90=0.3333333433 P95=0.5 P99=0.6666666865 rescale_timing=fused_with_weighted_spmm
PROFILE path=joint_g8 layer=2 layer_wall_ms=2173.196231 kernel_wall_ms=2132.575175 sampled_tiles=1306 sampled_neighbor_group_blocks=46688 sampled_row_head_blocks=373504 sampled_source_vector_loads=17627200 sampled_fma_vectors=141017600 init_worker_ms=4.055895 score_exp_den_worker_ms=56.513079 weighted_spmm_rescale_worker_ms=207.666187 UW_worker_ms=21.816288 normalization_scatter_worker_ms=1.953879 U_global_bytes=0 rescale_timing=fused_with_weighted_spmm
JOINT_ONLINE_STATS layer=2 blocks=42882792 running_max_updates_including_initial=24069181 rescales=4476949 rescaled_feature_elements=1146098944 rescale_per_block=0.1043996622 row_head_ratio_P50=0 P90=0.3333333433 P95=0.5 P99=0.5 rescale_timing=fused_with_weighted_spmm
PROFILE path=joint_g8 layer=3 layer_wall_ms=1037.547972 kernel_wall_ms=1019.108798 sampled_tiles=1306 sampled_neighbor_group_blocks=46688 sampled_row_head_blocks=46688 sampled_source_vector_loads=17627200 sampled_fma_vectors=17627200 init_worker_ms=0.643903 score_exp_den_worker_ms=20.548316 weighted_spmm_rescale_worker_ms=124.712669 UW_worker_ms=5.03653 normalization_scatter_worker_ms=0.582469 U_global_bytes=0 rescale_timing=fused_with_weighted_spmm
JOINT_ONLINE_STATS layer=3 blocks=5360349 running_max_updates_including_initial=3253695 rescales=804666 rescaled_feature_elements=205994496 rescale_per_block=0.1501144795 row_head_ratio_P50=0 P90=0.400000006 P95=0.5 P99=0.6666666865 rescale_timing=fused_with_weighted_spmm
JOINT_COMPLETE implementation_gate=PASS master_reference_gate=UNVERIFIED task_accuracy=UNVERIFIED performance=EXPLORATORY
```
