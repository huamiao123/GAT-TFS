# Grouping x projection-scope analysis

Primary: consecutive E2E. Alternating E2E is an independent order check. All speedup denominators come from the same graph, job and process. Cohort geometric means combine graph-local ratios; shared/exclusive and replications remain separate.

Included 20 / 20 graph records. Full records and exclusions: record_status.csv.

| Gate | Included rows | All observed | All failed |
|---|---:|---:|---:|
| METHOD_CHECK | 1520 | 1520 | 0 |
| METHOD_EQUIV | 900 | 900 | 0 |
| METHOD_PROFILE_GATE | 600 | 600 | 0 |
| GROUP_WORK_GATE | 600 | 600 | 0 |
| GROUP_AUDIT | 100 | 100 | 0 |

Selected fixed-policy primary results (GM > 1 means faster; partial coverage explicitly labelled):

| Cohort | Subset | Method | Reference | Graphs | GM | Status |
|---|---|---|---|---:|---:|---|
| shared_primary | all17 | s64_m256_qguard_fast | speedup_vs_degree | 1/17 | 1.20958 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m256_qguard_fast | speedup_vs_b256 | 1/17 | 1.15636 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m256_qguard_fast | speedup_vs_tfs | 1/17 | 2.39380 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_m256_qguard_fast | speedup_vs_degree | 1/4 | 1.20958 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_m256_qguard_fast | speedup_vs_b256 | 1/4 | 1.15636 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_m256_qguard_fast | speedup_vs_tfs | 1/4 | 2.39380 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m64_qguard_fast | speedup_vs_degree | 1/17 | 1.20867 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m64_qguard_fast | speedup_vs_b256 | 1/17 | 1.12431 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m64_qguard_fast | speedup_vs_tfs | 1/17 | 2.32745 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_m64_qguard_fast | speedup_vs_degree | 1/4 | 1.20867 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_m64_qguard_fast | speedup_vs_b256 | 1/4 | 1.12431 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_m64_qguard_fast | speedup_vs_tfs | 1/4 | 2.32745 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_mfull_qguard_fast | speedup_vs_degree | 1/17 | 1.20890 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_mfull_qguard_fast | speedup_vs_b256 | 1/17 | 1.16964 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_mfull_qguard_fast | speedup_vs_tfs | 1/17 | 2.42129 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_mfull_qguard_fast | speedup_vs_degree | 1/4 | 1.20890 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_mfull_qguard_fast | speedup_vs_b256 | 1/4 | 1.16964 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_mfull_qguard_fast | speedup_vs_tfs | 1/4 | 2.42129 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m256_qguard_fast | speedup_vs_degree | 3/17 | 1.08400 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m256_qguard_fast | speedup_vs_b256 | 3/17 | 1.06060 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m256_qguard_fast | speedup_vs_tfs | 3/17 | 1.47876 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_m256_qguard_fast | speedup_vs_degree | 3/4 | 1.08400 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_m256_qguard_fast | speedup_vs_b256 | 3/4 | 1.06060 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_m256_qguard_fast | speedup_vs_tfs | 3/4 | 1.47876 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m64_qguard_fast | speedup_vs_degree | 3/17 | 1.06571 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m64_qguard_fast | speedup_vs_b256 | 3/17 | 1.03981 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m64_qguard_fast | speedup_vs_tfs | 3/17 | 1.44977 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_m64_qguard_fast | speedup_vs_degree | 3/4 | 1.06571 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_m64_qguard_fast | speedup_vs_b256 | 3/4 | 1.03981 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_m64_qguard_fast | speedup_vs_tfs | 3/4 | 1.44977 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_mfull_qguard_fast | speedup_vs_degree | 3/17 | 1.06664 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_mfull_qguard_fast | speedup_vs_b256 | 3/17 | 1.05292 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_mfull_qguard_fast | speedup_vs_tfs | 3/17 | 1.46805 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_mfull_qguard_fast | speedup_vs_degree | 3/4 | 1.06664 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_mfull_qguard_fast | speedup_vs_b256 | 3/4 | 1.05292 | PARTIAL_COVERAGE |
| shared_primary | diagnostic4 | s64_mfull_qguard_fast | speedup_vs_tfs | 3/4 | 1.46805 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m256_qguard_fast | speedup_vs_degree | 4/17 | 1.02078 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m256_qguard_fast | speedup_vs_b256 | 4/17 | 1.01639 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m256_qguard_fast | speedup_vs_tfs | 4/17 | 1.35580 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m256_qguard_fast | speedup_vs_degree | 4/13 | 1.02078 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m256_qguard_fast | speedup_vs_b256 | 4/13 | 1.01639 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m256_qguard_fast | speedup_vs_tfs | 4/13 | 1.35580 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m64_qguard_fast | speedup_vs_degree | 4/17 | 1.02539 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m64_qguard_fast | speedup_vs_b256 | 4/17 | 1.00271 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m64_qguard_fast | speedup_vs_tfs | 4/17 | 1.33755 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m64_qguard_fast | speedup_vs_degree | 4/13 | 1.02539 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m64_qguard_fast | speedup_vs_b256 | 4/13 | 1.00271 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m64_qguard_fast | speedup_vs_tfs | 4/13 | 1.33755 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_mfull_qguard_fast | speedup_vs_degree | 4/17 | 1.02169 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_mfull_qguard_fast | speedup_vs_b256 | 4/17 | 1.01551 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_mfull_qguard_fast | speedup_vs_tfs | 4/17 | 1.35463 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_mfull_qguard_fast | speedup_vs_degree | 4/13 | 1.02169 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_mfull_qguard_fast | speedup_vs_b256 | 4/13 | 1.01551 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_mfull_qguard_fast | speedup_vs_tfs | 4/13 | 1.35463 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m256_qguard_fast | speedup_vs_degree | 4/17 | 1.00510 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m256_qguard_fast | speedup_vs_b256 | 4/17 | 0.99144 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m256_qguard_fast | speedup_vs_tfs | 4/17 | 1.03237 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m256_qguard_fast | speedup_vs_degree | 4/13 | 1.00510 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m256_qguard_fast | speedup_vs_b256 | 4/13 | 0.99144 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m256_qguard_fast | speedup_vs_tfs | 4/13 | 1.03237 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m64_qguard_fast | speedup_vs_degree | 4/17 | 0.99719 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m64_qguard_fast | speedup_vs_b256 | 4/17 | 0.98417 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m64_qguard_fast | speedup_vs_tfs | 4/17 | 1.02480 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m64_qguard_fast | speedup_vs_degree | 4/13 | 0.99719 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m64_qguard_fast | speedup_vs_b256 | 4/13 | 0.98417 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m64_qguard_fast | speedup_vs_tfs | 4/13 | 1.02480 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_mfull_qguard_fast | speedup_vs_degree | 4/17 | 1.00718 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_mfull_qguard_fast | speedup_vs_b256 | 4/17 | 0.99602 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_mfull_qguard_fast | speedup_vs_tfs | 4/17 | 1.03714 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_mfull_qguard_fast | speedup_vs_degree | 4/13 | 1.00718 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_mfull_qguard_fast | speedup_vs_b256 | 4/13 | 0.99602 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_mfull_qguard_fast | speedup_vs_tfs | 4/13 | 1.03714 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m256_qguard_fast | speedup_vs_degree | 5/17 | 1.00753 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m256_qguard_fast | speedup_vs_b256 | 5/17 | 0.99858 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m256_qguard_fast | speedup_vs_tfs | 5/17 | 1.38391 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m256_qguard_fast | speedup_vs_degree | 5/13 | 1.00753 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m256_qguard_fast | speedup_vs_b256 | 5/13 | 0.99858 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m256_qguard_fast | speedup_vs_tfs | 5/13 | 1.38391 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m64_qguard_fast | speedup_vs_degree | 5/17 | 1.00063 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m64_qguard_fast | speedup_vs_b256 | 5/17 | 0.98409 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_m64_qguard_fast | speedup_vs_tfs | 5/17 | 1.36382 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m64_qguard_fast | speedup_vs_degree | 5/13 | 1.00063 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m64_qguard_fast | speedup_vs_b256 | 5/13 | 0.98409 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_m64_qguard_fast | speedup_vs_tfs | 5/13 | 1.36382 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_mfull_qguard_fast | speedup_vs_degree | 5/17 | 1.00048 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_mfull_qguard_fast | speedup_vs_b256 | 5/17 | 1.00370 | PARTIAL_COVERAGE |
| shared_primary | all17 | s64_mfull_qguard_fast | speedup_vs_tfs | 5/17 | 1.39100 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_mfull_qguard_fast | speedup_vs_degree | 5/13 | 1.00048 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_mfull_qguard_fast | speedup_vs_b256 | 5/13 | 1.00370 | PARTIAL_COVERAGE |
| shared_primary | remaining13 | s64_mfull_qguard_fast | speedup_vs_tfs | 5/13 | 1.39100 | PARTIAL_COVERAGE |
| synthetic_intel | other_synthetic_or_fixture | s64_m256_qguard_fast | speedup_vs_degree | 3/3 | 0.99852 | COMPLETE |
| synthetic_intel | other_synthetic_or_fixture | s64_m256_qguard_fast | speedup_vs_b256 | 3/3 | 0.97370 | COMPLETE |
| synthetic_intel | other_synthetic_or_fixture | s64_m256_qguard_fast | speedup_vs_tfs | 3/3 | 1.53637 | COMPLETE |
| synthetic_intel | other_synthetic_or_fixture | s64_m64_qguard_fast | speedup_vs_degree | 3/3 | 1.00112 | COMPLETE |
| synthetic_intel | other_synthetic_or_fixture | s64_m64_qguard_fast | speedup_vs_b256 | 3/3 | 0.94109 | COMPLETE |
| synthetic_intel | other_synthetic_or_fixture | s64_m64_qguard_fast | speedup_vs_tfs | 3/3 | 1.48491 | COMPLETE |
| synthetic_intel | other_synthetic_or_fixture | s64_mfull_qguard_fast | speedup_vs_degree | 3/3 | 0.99963 | COMPLETE |
| synthetic_intel | other_synthetic_or_fixture | s64_mfull_qguard_fast | speedup_vs_b256 | 3/3 | 0.97379 | COMPLETE |
| synthetic_intel | other_synthetic_or_fixture | s64_mfull_qguard_fast | speedup_vs_tfs | 3/3 | 1.53652 | COMPLETE |

Qguard nontrivial selections with a slower primary median: 7 scope/graph/job cases. These are retained in qguard_selection.csv; the sign is descriptive, not a significance test.

group_x_scope_interactions.csv reports (grouping benefit at FULL)/(benefit at M64); >1 means a stronger grouping benefit at FULL. Oracle fields are explicitly hindsight over measured policies, not a guarded-candidate oracle or implementable policy.

Setup is the measured complete five-schedule pipeline. Full-pipeline single-forward and break-even columns do not estimate an isolated candidate setup cost. Degree fallback has no changed permutation, so its small timing differences are not used to claim amortization. Sampled thread phases are not additive wall fractions; logical counters are not DRAM traffic.
