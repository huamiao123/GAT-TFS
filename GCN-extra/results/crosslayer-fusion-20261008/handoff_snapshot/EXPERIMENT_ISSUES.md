# EXPERIMENT_ISSUES — current controlled GCN-extra experiments

Prior experiment entries removed by explicit user request. The retained run snapshots are immutable; older package hashes in current publication events describe superseded current-run bundles. See the cleaned archive manifest for active artifacts.

## paper-method-build-20261004-123445

```json
{
  "id": "paper-method-build-20261004-123445",
  "kind": "paper_method_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-build-20261004-123445",
  "paper_eligible": false,
  "issues": "Controls fixed in PAPER_METHODS_PROTOCOL_20261004.md; no numerical gate changes allowed",
  "next": "Shared smoke before exclusive controlled comparisons",
  "date": "2026-10-04T04:34:55.032222+00:00"
}
```

## paper-method-submit-smoke-20261004-123733

```json
{
  "id": "paper-method-submit-smoke-20261004-123733",
  "kind": "controlled_paper_method_submission",
  "status": "SUBMITTED",
  "job_id": "10864873",
  "mode": "smoke",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-submit-smoke-20261004-123733",
  "paper_eligible": false,
  "next": "Correctness before formal",
  "date": "2026-10-04T04:37:34.705347+00:00"
}
```

## paper-method-smoke-10864873

```json
{
  "id": "paper-method-smoke-10864873",
  "kind": "controlled_paper_method_comparison",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10864873",
  "node": "qhcn049",
  "build": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-build-20261004-123445",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-smoke-10864873",
  "completion": {
    "passed": 2,
    "failed": 0,
    "checks": 120,
    "mode": "smoke"
  },
  "paper_eligible": false,
  "issues": "Same graph/tensors/thread binding/NUMA policy/output precision/timing; original source anchor and rotated method order; frozen numerical gates",
  "next": "Reconcile all three disjoint graph batches against raw evidence",
  "date": "2026-10-04T04:37:42.137819+00:00"
}
```

## paper-method-submit-high-20261004-123850

```json
{
  "id": "paper-method-submit-high-20261004-123850",
  "kind": "controlled_paper_method_submission",
  "status": "SUBMITTED",
  "job_id": "10864874",
  "mode": "high",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-submit-high-20261004-123850",
  "paper_eligible": false,
  "next": "Within-graph same-process controlled comparison; never mix nodes in speedup",
  "date": "2026-10-04T04:38:51.660421+00:00"
}
```

## paper-method-submit-medium-20261004-123855

```json
{
  "id": "paper-method-submit-medium-20261004-123855",
  "kind": "controlled_paper_method_submission",
  "status": "SUBMITTED",
  "job_id": "10864875",
  "mode": "medium",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-submit-medium-20261004-123855",
  "paper_eligible": false,
  "next": "Within-graph same-process controlled comparison; never mix nodes in speedup",
  "date": "2026-10-04T04:38:57.167762+00:00"
}
```

## paper-method-submit-low-20261004-123901

```json
{
  "id": "paper-method-submit-low-20261004-123901",
  "kind": "controlled_paper_method_submission",
  "status": "SUBMITTED",
  "job_id": "10864876",
  "mode": "low",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-submit-low-20261004-123901",
  "paper_eligible": false,
  "next": "Within-graph same-process controlled comparison; never mix nodes in speedup",
  "date": "2026-10-04T04:39:02.443074+00:00"
}
```

## paper-method-high-10864874

```json
{
  "id": "paper-method-high-10864874",
  "kind": "controlled_paper_method_comparison",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10864874",
  "node": "qhcn818",
  "build": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-build-20261004-123445",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-high-10864874",
  "completion": {
    "passed": 5,
    "failed": 0,
    "checks": 300,
    "mode": "high"
  },
  "paper_eligible": true,
  "issues": "Same graph/tensors/thread binding/NUMA policy/output precision/timing; original source anchor and rotated method order; frozen numerical gates",
  "next": "Reconcile all three disjoint graph batches against raw evidence",
  "date": "2026-10-04T04:55:35.275083+00:00"
}
```

## paper-method-medium-10864875

```json
{
  "id": "paper-method-medium-10864875",
  "kind": "controlled_paper_method_comparison",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10864875",
  "node": "qhcn817",
  "build": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-build-20261004-123445",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-medium-10864875",
  "completion": {
    "passed": 5,
    "failed": 0,
    "checks": 300,
    "mode": "medium"
  },
  "paper_eligible": true,
  "issues": "Same graph/tensors/thread binding/NUMA policy/output precision/timing; original source anchor and rotated method order; frozen numerical gates",
  "next": "Reconcile all three disjoint graph batches against raw evidence",
  "date": "2026-10-04T04:55:50.627902+00:00"
}
```

## paper-method-low-10864876

```json
{
  "id": "paper-method-low-10864876",
  "kind": "controlled_paper_method_comparison",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10864876",
  "node": "qhcn818",
  "build": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-build-20261004-123445",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-low-10864876",
  "completion": {
    "passed": 7,
    "failed": 0,
    "checks": 420,
    "mode": "low"
  },
  "paper_eligible": true,
  "issues": "Same graph/tensors/thread binding/NUMA policy/output precision/timing; original source anchor and rotated method order; frozen numerical gates",
  "next": "Reconcile all three disjoint graph batches against raw evidence",
  "date": "2026-10-04T05:03:34.569899+00:00"
}
```

## paper-method-reconciliation-20261004

```json
{
  "id": "paper-method-reconciliation-20261004",
  "kind": "controlled_paper_method_reconciliation",
  "status": "PASS",
  "report": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/docs/PAPER_METHODS_RESULTS_20261004.md",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-reconciled-20261004",
  "summary": {
    "graphs": 17,
    "methods": 16,
    "candidate_methods": 14,
    "numerical_checks": 1020,
    "numerical_failures": 0,
    "measured_repetitions": 2720,
    "nodes": {
      "paper-method-high-10864874": "qhcn818",
      "paper-method-medium-10864875": "qhcn817",
      "paper-method-low-10864876": "qhcn818"
    },
    "same_binary_hashes": true,
    "primary_statistic": "median of five, same graph/process/buffers; corresponding original TFS/MKL medians",
    "fixed": [
      {
        "method": "b2_fast",
        "graphs": 17,
        "median_gmean_vs_tfs": 0.7285863197833305,
        "median_gmean_vs_mkl": 1.3810643114762338,
        "min_gmean_vs_tfs": 0.729950949469959,
        "wins_vs_tfs": 0,
        "wins_vs_mkl": 14
      },
      {
        "method": "b2_accurate",
        "graphs": 17,
        "median_gmean_vs_tfs": 0.5846698085894831,
        "median_gmean_vs_mkl": 1.1082648475759236,
        "min_gmean_vs_tfs": 0.5870799658335182,
        "wins_vs_tfs": 0,
        "wins_vs_mkl": 10
      },
      {
        "method": "b4_fast",
        "graphs": 17,
        "median_gmean_vs_tfs": 0.9381041940957733,
        "median_gmean_vs_mkl": 1.778213765113145,
        "min_gmean_vs_tfs": 0.9402015721230681,
        "wins_vs_tfs": 4,
        "wins_vs_mkl": 15
      },
      {
        "method": "b4_accurate",
        "graphs": 17,
        "median_gmean_vs_tfs": 0.8127004176159869,
        "median_gmean_vs_mkl": 1.5405059252622955,
        "min_gmean_vs_tfs": 0.8123747097562057,
        "wins_vs_tfs": 2,
        "wins_vs_mkl": 14
      },
      {
        "method": "b8_fast",
        "graphs": 17,
        "median_gmean_vs_tfs": 1.1103264515904865,
        "median_gmean_vs_mkl": 2.104667895542813,
        "min_gmean_vs_tfs": 1.1070459684564808,
        "wins_vs_tfs": 9,
        "wins_vs_mkl": 17
      },
      {
        "method": "b8_accurate",
        "graphs": 17,
        "median_gmean_vs_tfs": 1.0136172733180502,
        "median_gmean_vs_mkl": 1.921351806456795,
        "min_gmean_vs_tfs": 1.0129931339428484,
        "wins_vs_tfs": 7,
        "wins_vs_mkl": 16
      },
      {
        "method": "b16_fast",
        "graphs": 17,
        "median_gmean_vs_tfs": 1.2226753543235622,
        "median_gmean_vs_mkl": 2.317629703615613,
        "min_gmean_vs_tfs": 1.2200868880591302,
        "wins_vs_tfs": 10,
        "wins_vs_mkl": 17
      },
      {
        "method": "b16_accurate",
        "graphs": 17,
        "median_gmean_vs_tfs": 1.1481607022295304,
        "median_gmean_vs_mkl": 2.176384220555021,
        "min_gmean_vs_tfs": 1.1465153723509696,
        "wins_vs_tfs": 9,
        "wins_vs_mkl": 17
      },
      {
        "method": "b32_fast",
        "graphs": 17,
        "median_gmean_vs_tfs": 1.2797964001422986,
        "median_gmean_vs_mkl": 2.4259049150386263,
        "min_gmean_vs_tfs": 1.2781556736618351,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "b32_accurate",
        "graphs": 17,
        "median_gmean_vs_tfs": 1.2263766070725375,
        "median_gmean_vs_mkl": 2.324645575229677,
        "min_gmean_vs_tfs": 1.225162452806029,
        "wins_vs_tfs": 10,
        "wins_vs_mkl": 17
      },
      {
        "method": "b64_fast",
        "graphs": 17,
        "median_gmean_vs_tfs": 1.3011897199830935,
        "median_gmean_vs_mkl": 2.4664568026240326,
        "min_gmean_vs_tfs": 1.301418260991466,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "b64_accurate",
        "graphs": 17,
        "median_gmean_vs_tfs": 1.261220829948195,
        "median_gmean_vs_mkl": 2.3906941838406714,
        "min_gmean_vs_tfs": 1.260304386756196,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "bfull_fast",
        "graphs": 17,
        "median_gmean_vs_tfs": 1.2513960779370457,
        "median_gmean_vs_mkl": 2.3720709761255754,
        "min_gmean_vs_tfs": 1.2525722184789048,
        "wins_vs_tfs": 10,
        "wins_vs_mkl": 17
      },
      {
        "method": "bfull_accurate",
        "graphs": 17,
        "median_gmean_vs_tfs": 1.2194150264866357,
        "median_gmean_vs_mkl": 2.311449622687617,
        "min_gmean_vs_tfs": 1.2157535252217064,
        "wins_vs_tfs": 10,
        "wins_vs_mkl": 17
      }
    ],
    "oracle_best_fast_gmean_vs_tfs": 1.3238165012335805,
    "oracle_best_fast_wins": 11,
    "oracle_is_not_implemented_adaptive_method": true,
    "anchor_min_ratio_range": [
      0.9733333333333333,
      1.0354236951776146
    ]
  },
  "paper_eligible": true,
  "issues": "No gate relaxation or historical/cross-node denominator substitution; fixed-method losses and oracle selection retained",
  "next": "Interpret degree-dependent crossover before any further optimization",
  "date": "2026-10-04T05:08:14.235748+00:00"
}
```

## paper-cache-build-20261004-131154

```json
{
  "id": "paper-cache-build-20261004-131154",
  "kind": "paper_cache_control_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-cache-build-20261004-131154",
  "paper_eligible": false,
  "issues": "Interleaved method execution changes small-graph cache warmth, especially FP32 MKL; supplemental source-equivalent warmup plus five consecutive repetitions. Frozen kernels and gates unchanged.",
  "next": "Shared smoke then all-17 paired cache control",
  "date": "2026-10-04T05:11:58.725583+00:00"
}
```

## paper-cache-submit-smoke-20261004-131545

```json
{
  "id": "paper-cache-submit-smoke-20261004-131545",
  "kind": "source_warm_cache_control_submission",
  "status": "SUBMITTED",
  "job_id": "10864904",
  "mode": "smoke",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-cache-submit-smoke-20261004-131545",
  "paper_eligible": false,
  "issues": "Same frozen kernels/gates/inputs/output precision; only measured execution order differs",
  "next": "Check consecutive vs interleaved sensitivity, retaining both",
  "date": "2026-10-04T05:15:46.910980+00:00"
}
```

## paper-cache-smoke-10864904

```json
{
  "id": "paper-cache-smoke-10864904",
  "kind": "source_warm_cache_control",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10864904",
  "node": "qhcn049",
  "build": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-cache-build-20261004-131154",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-cache-smoke-10864904",
  "completion": {
    "passed": 2,
    "failed": 0,
    "checks": 72,
    "mode": "smoke"
  },
  "paper_eligible": false,
  "issues": "Same graph/tensors/thread binding/NUMA policy/output precision/timing; original source anchor and immediate warmup plus consecutive method repeats; frozen numerical gates",
  "next": "Reconcile cache control with retained interleaved measurements",
  "date": "2026-10-04T05:15:49.421960+00:00"
}
```

## paper-cache-submit-high-20261004-131618

```json
{
  "id": "paper-cache-submit-high-20261004-131618",
  "kind": "source_warm_cache_control_submission",
  "status": "SUBMITTED",
  "job_id": "10864906",
  "mode": "high",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-cache-submit-high-20261004-131618",
  "paper_eligible": false,
  "issues": "Same frozen kernels/gates/inputs/output precision; only measured execution order differs",
  "next": "Check consecutive vs interleaved sensitivity, retaining both",
  "date": "2026-10-04T05:16:19.648158+00:00"
}
```

## paper-cache-submit-medium-20261004-131623

```json
{
  "id": "paper-cache-submit-medium-20261004-131623",
  "kind": "source_warm_cache_control_submission",
  "status": "SUBMITTED",
  "job_id": "10864907",
  "mode": "medium",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-cache-submit-medium-20261004-131623",
  "paper_eligible": false,
  "issues": "Same frozen kernels/gates/inputs/output precision; only measured execution order differs",
  "next": "Check consecutive vs interleaved sensitivity, retaining both",
  "date": "2026-10-04T05:16:24.631864+00:00"
}
```

## paper-cache-submit-low-20261004-131628

```json
{
  "id": "paper-cache-submit-low-20261004-131628",
  "kind": "source_warm_cache_control_submission",
  "status": "SUBMITTED",
  "job_id": "10864908",
  "mode": "low",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-cache-submit-low-20261004-131628",
  "paper_eligible": false,
  "issues": "Same frozen kernels/gates/inputs/output precision; only measured execution order differs",
  "next": "Check consecutive vs interleaved sensitivity, retaining both",
  "date": "2026-10-04T05:16:29.817880+00:00"
}
```

## paper-cache-scheduling-adjustment-20261004-133536

```json
{
  "id": "paper-cache-scheduling-adjustment-20261004-133536",
  "kind": "cache_control_scheduler_limit_only",
  "status": "PASS",
  "job_ids": [
    "10864906",
    "10864907",
    "10864908"
  ],
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-cache-scheduling-adjustment-20261004-133536",
  "paper_eligible": false,
  "issues": "Original two-hour timeout overstates scheduling duration. Complete larger main sweep took at most 16m43s. Supplemental scope is smaller. Only scheduler timeout changes to 30 minutes; frozen kernels/gates/data/threads/affinity/memory/partition/exclusivity/timing boundaries unchanged.",
  "next": "Retain timeout outcomes if any; reconcile all cache-control outputs",
  "date": "2026-10-04T05:35:38.111582+00:00"
}
```

## paper-cache-high-10864906

```json
{
  "id": "paper-cache-high-10864906",
  "kind": "source_warm_cache_control",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10864906",
  "node": "qhcn819",
  "build": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-cache-build-20261004-131154",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-cache-high-10864906",
  "completion": {
    "passed": 5,
    "failed": 0,
    "checks": 180,
    "mode": "high"
  },
  "paper_eligible": true,
  "issues": "Same graph/tensors/thread binding/NUMA policy/output precision/timing; original source anchor and immediate warmup plus consecutive method repeats; frozen numerical gates",
  "next": "Reconcile cache control with retained interleaved measurements",
  "date": "2026-10-04T05:48:12.490661+00:00"
}
```

## paper-cache-medium-10864907

```json
{
  "id": "paper-cache-medium-10864907",
  "kind": "source_warm_cache_control",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10864907",
  "node": "qhcn819",
  "build": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-cache-build-20261004-131154",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-cache-medium-10864907",
  "completion": {
    "passed": 5,
    "failed": 0,
    "checks": 180,
    "mode": "medium"
  },
  "paper_eligible": true,
  "issues": "Same graph/tensors/thread binding/NUMA policy/output precision/timing; original source anchor and immediate warmup plus consecutive method repeats; frozen numerical gates",
  "next": "Reconcile cache control with retained interleaved measurements",
  "date": "2026-10-04T05:51:08.563447+00:00"
}
```

## paper-cache-low-10864908

```json
{
  "id": "paper-cache-low-10864908",
  "kind": "source_warm_cache_control",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10864908",
  "node": "qhcn819",
  "build": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-cache-build-20261004-131154",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-cache-low-10864908",
  "completion": {
    "passed": 7,
    "failed": 0,
    "checks": 252,
    "mode": "low"
  },
  "paper_eligible": true,
  "issues": "Same graph/tensors/thread binding/NUMA policy/output precision/timing; original source anchor and immediate warmup plus consecutive method repeats; frozen numerical gates",
  "next": "Reconcile cache control with retained interleaved measurements",
  "date": "2026-10-04T05:55:18.550789+00:00"
}
```

## paper-cache-reconciliation-20261004

```json
{
  "id": "paper-cache-reconciliation-20261004",
  "kind": "execution_order_control_reconciliation",
  "status": "PASS",
  "report": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/docs/PAPER_CACHE_CONTROL_RESULTS_20261004.md",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-cache-reconciled-20261004",
  "summary": {
    "graphs": 17,
    "numerical_checks": 612,
    "numerical_failures": 0,
    "measured_repetitions": 850,
    "source_input_hashes_match_main": true,
    "same_binary_hashes": true,
    "fixed": [
      {
        "method": "b8_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.105749263059566,
        "consecutive_gmean_vs_mkl": 2.0573426850763235,
        "interleaved_gmean_vs_tfs": 1.1103264515904865,
        "interleaved_gmean_vs_mkl": 2.104667895542813,
        "wins_vs_tfs": 9,
        "wins_vs_mkl": 16
      },
      {
        "method": "b8_accurate",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.005135913059786,
        "consecutive_gmean_vs_mkl": 1.8701427957720151,
        "interleaved_gmean_vs_tfs": 1.0136172733180502,
        "interleaved_gmean_vs_mkl": 1.921351806456795,
        "wins_vs_tfs": 6,
        "wins_vs_mkl": 16
      },
      {
        "method": "b16_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.2156395617942832,
        "consecutive_gmean_vs_mkl": 2.2618031444368496,
        "interleaved_gmean_vs_tfs": 1.2226753543235622,
        "interleaved_gmean_vs_mkl": 2.317629703615613,
        "wins_vs_tfs": 10,
        "wins_vs_mkl": 16
      },
      {
        "method": "b16_accurate",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.1445930939871658,
        "consecutive_gmean_vs_mkl": 2.1296150112618433,
        "interleaved_gmean_vs_tfs": 1.1481607022295304,
        "interleaved_gmean_vs_mkl": 2.176384220555021,
        "wins_vs_tfs": 9,
        "wins_vs_mkl": 16
      },
      {
        "method": "b64_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.308859185605137,
        "consecutive_gmean_vs_mkl": 2.435246363039741,
        "interleaved_gmean_vs_tfs": 1.3011897199830935,
        "interleaved_gmean_vs_mkl": 2.4664568026240326,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "b64_accurate",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.2617924426767824,
        "consecutive_gmean_vs_mkl": 2.3476745938250043,
        "interleaved_gmean_vs_tfs": 1.261220829948195,
        "interleaved_gmean_vs_mkl": 2.3906941838406714,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "bfull_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.2581041315920165,
        "consecutive_gmean_vs_mkl": 2.3408121702322155,
        "interleaved_gmean_vs_tfs": 1.2513960779370457,
        "interleaved_gmean_vs_mkl": 2.3720709761255754,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "bfull_accurate",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.2212485249671163,
        "consecutive_gmean_vs_mkl": 2.2722391083031614,
        "interleaved_gmean_vs_tfs": 1.2194150264866357,
        "interleaved_gmean_vs_mkl": 2.311449622687617,
        "wins_vs_tfs": 10,
        "wins_vs_mkl": 17
      }
    ],
    "primary_statistic": "median of five consecutive repetitions after immediate warmup; same-process TFS/MKL denominator",
    "nodes": {
      "paper-cache-high-10864906": "qhcn819",
      "paper-cache-medium-10864907": "qhcn819",
      "paper-cache-low-10864908": "qhcn819"
    }
  },
  "paper_eligible": true,
  "issues": "Both protocols retained; same-process denominator, frozen kernel/gates and input hashes; cache effects not isolated from cross-run variability",
  "next": "Archive and publish all main and supplemental evidence",
  "date": "2026-10-04T05:56:20.386382+00:00"
}
```

## paper-method-package-20261004

```json
{
  "id": "paper-method-package-20261004",
  "kind": "controlled_method_archive",
  "status": "RECONCILED_READY_FOR_ARCHIVE",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-reconciled-20261004",
  "archive": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/evidence-paper-methods-controlled-20261004.tar.gz",
  "paper_eligible": true,
  "issues": "All numerical gates and slow cases retained; no cross-node or historical denominator",
  "next": "Verify local mirrors and publish existing isolated experiment branch",
  "date": "2026-10-04T05:56:55.729881+00:00"
}
```

## paper-method-report-qa-20261004

```json
{
  "id": "paper-method-report-qa-20261004",
  "kind": "post_archive_report_visual_qa",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-report-qa-20261004",
  "paper_eligible": false,
  "changed_report_assets": {
    "scripts/plot_paper_methods.py": "fc8154362c53a846a7f8a888d9aa64d13b4012ad1c03f21f2a58b7548406cee7",
    "docs/figures/paper_methods_20261004/execution_order_control.png": "4e8887b185c847e4d3e0c1d5c30d3448fa06c2f25940097db5bea0f41f40d21f",
    "docs/figures/paper_methods_20261004/execution_order_control.pdf": "41e5a05894d827d78fc345448a4a1ed9b8f0609038d575eb159d5bae128a2308"
  },
  "benchmark_and_numerical_evidence_unchanged": true,
  "mirror_verified_files": 1176,
  "issues": "Initial local mirror hash check caught a concurrent plot layout edit; re-extraction succeeded and both mirrors verified all 1176 manifest files. Only then the legend was moved outside bars to expose final row numbers; PNG visually reviewed. A stalled upload connection left only the renderer updated, which the reporting-only hash gate rejected before recording success. Retry with BatchMode and ConnectTimeout=15 transferred both assets. Immutable archive retains its original reporting snapshot.",
  "next": "Publish verified benchmark evidence plus clearly recorded reporting-only revision",
  "date": "2026-10-04T06:04:15.944564+00:00"
}
```

## paper-method-publication-20261004

```json
{
  "id": "paper-method-publication-20261004",
  "kind": "controlled_methods_github_publication",
  "status": "PUBLISHED_VERIFIED",
  "source_evidence_commit": "0b20cc0ae1cc4749c7a24a256dae02dc8f78b54f",
  "branch": "gcn-extra-experiments-20261003",
  "repository": "https://github.com/huamiao123/GAT-TFS",
  "archive_sha256": "af7b17e5403f7f22e7baef1c4fc306525b68c7214f55565cee9e392d195556f4",
  "paper_eligible": true,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/paper-method-publication-20261004",
  "issues": "17 graphs, original paper v3/source MKL, both measurement orders, fixed gates and negative results published; archive captures pre-publication handoff snapshot and publication is a subsequent history event",
  "next": "Controlled remeasurement complete; future mechanism/PMU/shape/checkpoint experiments remain separate scope",
  "date": "2026-10-04T06:07:27.811518+00:00"
}
```

## cleanup-current-20261004

```json
{
  "id": "cleanup-current-20261004",
  "date": "2026-10-04T06:44:20.920473+00:00",
  "kind": "user_authorized_historical_cleanup",
  "status": "CLEANED",
  "scope": "GCN-extra only; server wzh and local mirrors; experiment Git branch latest tree",
  "kept": "Original TFS, exact source MKL and validated modified TFS; both current measurement orders, 17 graphs including negative cases",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/cleanup-current-20261004",
  "issues": "Previous bundles and numerical claims removed from active files. Existing Git commits remain; no history rewrite. Current frozen source snapshots stay byte-for-byte intact.",
  "next": "Use current cleaned archive and CURRENT_EVIDENCE_20261004.md"
}
```

## projection-window-build-20261004-161106

```json
{
  "id": "projection-window-build-20261004-161106",
  "kind": "projection_window_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-build-20261004-161106",
  "paper_eligible": false,
  "issues": "Frozen original source/control kernels; independent S/M; unchanged numerical gates plus bitwise equivalence",
  "next": "Shared correctness before exclusive formal",
  "date": "2026-10-04T08:11:19.431347+00:00"
}
```

## projection-window-submit-smoke-20261004-161133

```json
{
  "id": "projection-window-submit-smoke-20261004-161133",
  "kind": "projection_window_submission",
  "status": "SUBMITTED",
  "job_id": "10865904",
  "mode": "smoke",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-submit-smoke-20261004-161133",
  "paper_eligible": false,
  "next": "Correctness before formal",
  "date": "2026-10-04T08:11:34.617624+00:00"
}
```

## projection-window-smoke-10865904

```json
{
  "id": "projection-window-smoke-10865904",
  "kind": "independent_projection_window",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10865904",
  "node": "qhcn047",
  "build": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-build-20261004-161106",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-smoke-10865904",
  "completion": {
    "passed": 3,
    "failed": 0,
    "checks": 180,
    "equiv_checks": 72,
    "mode": "smoke",
    "primary_protocol": "consecutive_e2e",
    "secondary_protocol": "rotating_e2e"
  },
  "paper_eligible": false,
  "issues": "Same source/matrix/degree scheduling/NUMA/precision; same-M bitwise gates; both timing orders retained",
  "next": "Reconcile all raw results and retain negative outcomes",
  "date": "2026-10-04T08:11:42.016710+00:00"
}
```

## projection-window-build-20261004-162142

```json
{
  "id": "projection-window-build-20261004-162142",
  "kind": "projection_window_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-build-20261004-162142",
  "paper_eligible": false,
  "issues": "Frozen original source/control kernels; independent S/M; unchanged numerical gates plus bitwise equivalence",
  "next": "Shared correctness before exclusive formal",
  "date": "2026-10-04T08:21:50.380655+00:00"
}
```

## projection-window-submit-smoke-20261004-162223

```json
{
  "id": "projection-window-submit-smoke-20261004-162223",
  "kind": "projection_window_submission",
  "status": "SUBMITTED",
  "job_id": "10865927",
  "mode": "smoke",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-submit-smoke-20261004-162223",
  "paper_eligible": false,
  "next": "Correctness before formal",
  "date": "2026-10-04T08:22:25.472970+00:00"
}
```

## projection-window-smoke-10865927

```json
{
  "id": "projection-window-smoke-10865927",
  "kind": "independent_projection_window",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10865927",
  "node": "qhcn016",
  "build": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-build-20261004-162142",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-smoke-10865927",
  "completion": {
    "passed": 3,
    "failed": 0,
    "checks": 180,
    "equiv_checks": 72,
    "mode": "smoke",
    "primary_protocol": "consecutive_e2e",
    "secondary_protocol": "rotating_e2e"
  },
  "paper_eligible": false,
  "issues": "Same source/matrix/degree scheduling/NUMA/precision; same-M bitwise gates; both timing orders retained",
  "next": "Reconcile all raw results and retain negative outcomes",
  "date": "2026-10-04T08:22:38.780794+00:00"
}
```

## projection-window-submit-focus-20261004-162303

```json
{
  "id": "projection-window-submit-focus-20261004-162303",
  "kind": "projection_window_submission",
  "status": "SUBMITTED",
  "job_id": "10865930",
  "mode": "focus",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-submit-focus-20261004-162303",
  "paper_eligible": false,
  "next": "Same-process formal comparison; max two nodes",
  "date": "2026-10-04T08:23:07.395798+00:00"
}
```

## projection-window-submit-validation-20261004-162310

```json
{
  "id": "projection-window-submit-validation-20261004-162310",
  "kind": "projection_window_submission",
  "status": "SUBMITTED",
  "job_id": "10865931",
  "mode": "validation",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-submit-validation-20261004-162310",
  "paper_eligible": false,
  "next": "Same-process formal comparison; max two nodes",
  "date": "2026-10-04T08:23:12.302498+00:00"
}
```

## feature-precision-switch-20261004-164044

```json
{
  "id": "feature-precision-switch-20261004-164044",
  "kind": "user_steering_task_replacement",
  "status": "PASS",
  "cancelled_job_ids": [
    "10865930",
    "10865931"
  ],
  "reason": "User replaces S/M experiment with frozen 8aeef16 feature-input BF16/FP32 B64/FULL 2x2 and explicitly allows shared nodes",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-switch-20261004-164044",
  "paper_eligible": false,
  "next": "Frozen source review, minimal FP32-input kernel, smoke, five diagnostic real graphs before 17 expansion",
  "date": "2026-10-04T08:40:44.201176+00:00"
}
```

## feature-precision-ablation-20261004-170251-build

```json
{
  "id": "feature-precision-ablation-20261004-170251-build",
  "kind": "frozen_feature_precision_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-170251",
  "paper_eligible": false,
  "issues": "Frozen 8aeef16 BF16 control; minimal FP32 source loads; user authorizes shared nodes",
  "next": "Small-fixture correctness before five diagnostic graphs",
  "date": "2026-10-04T09:03:01.133367+00:00"
}
```

## feature-precision-ablation-20261004-212957-build

```json
{
  "id": "feature-precision-ablation-20261004-212957-build",
  "kind": "frozen_feature_precision_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-212957",
  "paper_eligible": false,
  "issues": "Frozen 8aeef16 BF16 control; minimal FP32 source loads; user authorizes shared nodes",
  "next": "Small-fixture correctness before five diagnostic graphs",
  "date": "2026-10-04T13:30:09.870521+00:00"
}
```

## feature-precision-ablation-20261004-212957-submit-smoke-20261004-213021

```json
{
  "id": "feature-precision-ablation-20261004-212957-submit-smoke-20261004-213021",
  "kind": "feature_precision_submission",
  "status": "SUBMITTED",
  "job_id": "10867467",
  "mode": "smoke",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-212957/logs/submit-smoke-20261004-213021",
  "paper_eligible": false,
  "issues": "User explicitly authorizes shared intel node; 32 physical cores on one socket verified in job",
  "next": "Correctness before real-graph timing",
  "date": "2026-10-04T13:30:23.219024+00:00"
}
```

## feature-precision-ablation-20261004-212957-smoke-10867467

```json
{
  "id": "feature-precision-ablation-20261004-212957-smoke-10867467",
  "kind": "feature_precision_smoke",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10867467",
  "node": "qhcn001",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-212957",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-212957/logs/smoke-10867467",
  "completion": {
    "status": "PASS",
    "mode": "smoke",
    "study": "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/runs/feature-precision-ablation-20261004-212957",
    "node": "qhcn001",
    "job_id": "10867467",
    "passed": 3,
    "failed": 0,
    "graphs": 3,
    "graph_names": [
      "precision_tail",
      "precision_high_tail",
      "precision_scope_tail"
    ],
    "driver_sha256": "1ac9764c6a6d1955196e6610b7b9896be502f8023bb780ed26e4444a5fa97c70",
    "source_manifest_sha256": "53abbacb5281b40df0d8d2835379dc59c4487fbc559f023a2320dcb633dfd475",
    "counts": {
      "CHECK": 72,
      "EQ": 18,
      "DIFF": 18,
      "PROFILE_GATE": 24,
      "TIME": 540,
      "STAGE": 60
    }
  },
  "paper_eligible": false,
  "issues": "Shared node explicitly authorized; source controls frozen; no NUMA/MKL policy change",
  "next": "Five diagnostic graph gates before remaining-twelve extension",
  "date": "2026-10-04T13:30:28.880283+00:00"
}
```

## feature-precision-ablation-20261004-212957-submit-diagnostic-20261004-213212

```json
{
  "id": "feature-precision-ablation-20261004-212957-submit-diagnostic-20261004-213212",
  "kind": "feature_precision_submission",
  "status": "SUBMITTED",
  "job_id": "10867476",
  "mode": "diagnostic",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-212957/logs/submit-diagnostic-20261004-213212",
  "paper_eligible": false,
  "issues": "User explicitly authorizes shared intel node; 32 physical cores on one socket verified in job",
  "next": "Correctness before real-graph timing",
  "date": "2026-10-04T13:32:13.724283+00:00"
}
```

## projection-window-submit-smoke-20261004-213400

```json
{
  "id": "projection-window-submit-smoke-20261004-213400",
  "kind": "projection_window_submission",
  "status": "SUBMITTED",
  "job_id": "10867477",
  "mode": "smoke",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-submit-smoke-20261004-213400",
  "paper_eligible": false,
  "issues": "Shared intel node explicitly permitted by current user; one socket physical-core assertion; default NUMA no interleave",
  "next": "Correctness before formal",
  "date": "2026-10-04T13:34:02.347812+00:00"
}
```

## projection-window-smoke-10867477

```json
{
  "id": "projection-window-smoke-10867477",
  "kind": "independent_projection_window",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10867477",
  "node": "qhcn001",
  "build": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-build-20261004-162142",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-smoke-10867477",
  "completion": {
    "passed": 3,
    "failed": 0,
    "checks": 180,
    "equiv_checks": 72,
    "mode": "smoke",
    "primary_protocol": "consecutive_e2e",
    "secondary_protocol": "rotating_e2e"
  },
  "paper_eligible": false,
  "issues": "Same source/matrix/degree scheduling/NUMA/precision; same-M bitwise gates; both timing orders retained",
  "next": "Reconcile all raw results and retain negative outcomes",
  "date": "2026-10-04T13:34:08.912878+00:00"
}
```

## projection-window-submit-focus-20261004-213446

```json
{
  "id": "projection-window-submit-focus-20261004-213446",
  "kind": "projection_window_submission",
  "status": "SUBMITTED",
  "job_id": "10867479",
  "mode": "focus",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-submit-focus-20261004-213446",
  "paper_eligible": false,
  "issues": "Shared intel node explicitly permitted by current user; one socket physical-core assertion; default NUMA no interleave",
  "next": "Same-process formal comparison; max two nodes",
  "date": "2026-10-04T13:34:47.520141+00:00"
}
```

## feature-precision-ablation-20261004-214015-build

```json
{
  "id": "feature-precision-ablation-20261004-214015-build",
  "kind": "frozen_feature_precision_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-214015",
  "paper_eligible": false,
  "issues": "Frozen 8aeef16 BF16 control; minimal FP32 source loads; user authorizes shared nodes",
  "next": "Small-fixture correctness before five diagnostic graphs",
  "date": "2026-10-04T13:40:23.766952+00:00"
}
```

## feature-precision-ablation-20261004-212957-diagnostic-10867476

```json
{
  "id": "feature-precision-ablation-20261004-212957-diagnostic-10867476",
  "kind": "feature_precision_diagnostic",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10867476",
  "node": "qhcn052",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-212957",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-212957/logs/diagnostic-10867476",
  "completion": {},
  "paper_eligible": false,
  "issues": "Shared node explicitly authorized; source controls frozen; no NUMA/MKL policy change",
  "next": "Five diagnostic graph gates before remaining-twelve extension",
  "date": "2026-10-04T13:43:17.502675+00:00"
}
```

## feature-precision-ablation-20261004-212957-serial-first-touch

```json
{
  "id": "feature-precision-ablation-20261004-212957-serial-first-touch",
  "kind": "feature_precision_protocol_correction",
  "status": "CANCELLED",
  "job_id": "10867476",
  "source_study": "runs/feature-precision-ablation-20261004-212957",
  "evidence": "runs/feature-precision-ablation-20261004-212957/logs/diagnostic-10867476",
  "paper_eligible": false,
  "reason": "Raw H0 FP32 serial first touch differs from BF16 parallel-static initial preparation; Products native FP32 kernel ~1100 ms while matched lossless-quantized FP32 with parallel-static first touch ~240 ms. Native contrast confounded by placement. Products and Reddit passed raw records retained; Myciel partial raw evidence retained. A corrected independent study copies raw FP32 H0 exactly with parallel-static first touch before prepared timing, no NUMA policy change.",
  "next": "Corrected small-fixture gate and five diagnostic graphs; old timings never serve as speedup denominators",
  "created_at": "2026-10-04T21:43:48.045085+08:00",
  "date": "2026-10-04T13:43:56.506390+00:00"
}
```

## feature-precision-ablation-20261004-214015-submit-smoke-20261004-214408

```json
{
  "id": "feature-precision-ablation-20261004-214015-submit-smoke-20261004-214408",
  "kind": "feature_precision_submission",
  "status": "SUBMITTED",
  "job_id": "10867490",
  "mode": "smoke",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-214015/logs/submit-smoke-20261004-214408",
  "paper_eligible": false,
  "issues": "User explicitly authorizes shared intel node; 32 physical cores on one socket verified in job",
  "next": "Correctness before real-graph timing",
  "date": "2026-10-04T13:44:10.215303+00:00"
}
```

## feature-precision-ablation-20261004-214015-smoke-10867490

```json
{
  "id": "feature-precision-ablation-20261004-214015-smoke-10867490",
  "kind": "feature_precision_smoke",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10867490",
  "node": "qhcn001",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-214015",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-214015/logs/smoke-10867490",
  "completion": {
    "status": "PASS",
    "mode": "smoke",
    "study": "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/runs/feature-precision-ablation-20261004-214015",
    "node": "qhcn001",
    "job_id": "10867490",
    "passed": 3,
    "failed": 0,
    "graphs": 3,
    "graph_names": [
      "precision_tail",
      "precision_high_tail",
      "precision_scope_tail"
    ],
    "driver_sha256": "48815149955a6f38339a9b37dd64076f16fe365acd4c2d07ed026e129988ec88",
    "source_manifest_sha256": "88d9225dd5fa9b1a61a7d256f3c50f13211a253cc5d1da19eb9d63b36b901120",
    "counts": {
      "CHECK": 72,
      "EQ": 18,
      "DIFF": 18,
      "PROFILE_GATE": 24,
      "TIME": 540,
      "STAGE": 60
    }
  },
  "paper_eligible": false,
  "issues": "Shared node explicitly authorized; source controls frozen; no NUMA/MKL policy change",
  "next": "Five diagnostic graph gates before remaining-twelve extension",
  "date": "2026-10-04T13:44:13.574115+00:00"
}
```

## feature-precision-ablation-20261004-214015-submit-diagnostic-20261004-214552

```json
{
  "id": "feature-precision-ablation-20261004-214015-submit-diagnostic-20261004-214552",
  "kind": "feature_precision_submission",
  "status": "SUBMITTED",
  "job_id": "10867492",
  "mode": "diagnostic",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-214015/logs/submit-diagnostic-20261004-214552",
  "paper_eligible": false,
  "issues": "User explicitly authorizes shared intel node; 32 physical cores on one socket verified in job",
  "next": "Correctness before real-graph timing",
  "date": "2026-10-04T13:45:53.457424+00:00"
}
```

## projection-window-focus-10867479

```json
{
  "id": "projection-window-focus-10867479",
  "kind": "independent_projection_window",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10867479",
  "node": "qhcn068",
  "build": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-build-20261004-162142",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-focus-10867479",
  "completion": {
    "passed": 4,
    "failed": 0,
    "checks": 240,
    "equiv_checks": 96,
    "mode": "focus",
    "primary_protocol": "consecutive_e2e",
    "secondary_protocol": "rotating_e2e"
  },
  "paper_eligible": true,
  "issues": "Same source/matrix/degree scheduling/NUMA/precision; same-M bitwise gates; both timing orders retained",
  "next": "Reconcile all raw results and retain negative outcomes",
  "date": "2026-10-04T13:52:50.032777+00:00"
}
```

## projection-window-submit-validation-20261004-215536

```json
{
  "id": "projection-window-submit-validation-20261004-215536",
  "kind": "projection_window_submission",
  "status": "SUBMITTED",
  "job_id": "10867699",
  "mode": "validation",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-submit-validation-20261004-215536",
  "paper_eligible": false,
  "issues": "Shared intel node explicitly permitted by current user; one socket physical-core assertion; default NUMA no interleave",
  "next": "Same-process formal comparison; max two nodes",
  "date": "2026-10-04T13:55:38.751818+00:00"
}
```

## journal-a3-build-20261004-215613-build

```json
{
  "id": "journal-a3-build-20261004-215613-build",
  "kind": "journal_a3_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-a3-build-20261004-215613",
  "paper_eligible": false,
  "issues": "A3 row-order control; original frozen TFS/MKL/B64/FULL/A1 source remains unchanged; default NUMA and fixed numerics",
  "next": "Small partial/output bitwise gates before four diagnostic graphs",
  "date": "2026-10-04T13:56:22.907479+00:00"
}
```

## journal-a3-build-20261004-220554-build

```json
{
  "id": "journal-a3-build-20261004-220554-build",
  "kind": "journal_a3_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-a3-build-20261004-220554",
  "paper_eligible": false,
  "issues": "A3 row-order control; original frozen TFS/MKL/B64/FULL/A1 source remains unchanged; default NUMA and fixed numerics",
  "next": "Small partial/output bitwise gates before four diagnostic graphs",
  "date": "2026-10-04T14:06:03.913799+00:00"
}
```

## journal-a3-build-20261004-220554-submit-smoke-20261004-220636

```json
{
  "id": "journal-a3-build-20261004-220554-submit-smoke-20261004-220636",
  "kind": "journal_a3_submission",
  "status": "SUBMITTED",
  "job_id": "10868124",
  "mode": "smoke",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-a3-build-20261004-220554/logs/submit-smoke-20261004-220636",
  "paper_eligible": false,
  "issues": "Shared intel one socket 4/32 physical cores, no interleave",
  "next": "Bitwise partial/output gate then focus",
  "date": "2026-10-04T14:06:37.614943+00:00"
}
```

## feature-precision-ablation-20261004-214015-diagnostic-10867492

```json
{
  "id": "feature-precision-ablation-20261004-214015-diagnostic-10867492",
  "kind": "feature_precision_diagnostic",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10867492",
  "node": "qhcn064",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-214015",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-214015/logs/diagnostic-10867492",
  "completion": {
    "status": "PASS",
    "mode": "diagnostic",
    "study": "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/runs/feature-precision-ablation-20261004-214015",
    "node": "qhcn064",
    "job_id": "10867492",
    "passed": 5,
    "failed": 0,
    "graphs": 5,
    "graph_names": [
      "ogbn-products",
      "reddit",
      "mycielskian19",
      "roadNet-CA",
      "wiki-Talk"
    ],
    "driver_sha256": "48815149955a6f38339a9b37dd64076f16fe365acd4c2d07ed026e129988ec88",
    "source_manifest_sha256": "88d9225dd5fa9b1a61a7d256f3c50f13211a253cc5d1da19eb9d63b36b901120",
    "counts": {
      "CHECK": 120,
      "EQ": 30,
      "DIFF": 30,
      "PROFILE_GATE": 40,
      "TIME": 900,
      "STAGE": 100
    }
  },
  "paper_eligible": false,
  "issues": "Shared node explicitly authorized; source controls frozen; no NUMA/MKL policy change",
  "next": "Five diagnostic graph gates before remaining-twelve extension",
  "date": "2026-10-04T14:13:02.698730+00:00"
}
```

## journal-a3-build-20261004-220554-smoke-10868124

```json
{
  "id": "journal-a3-build-20261004-220554-smoke-10868124",
  "kind": "journal_a3_smoke",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10868124",
  "node": "qhcn001",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-a3-build-20261004-220554",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-a3-build-20261004-220554/logs/smoke-10868124",
  "completion": {
    "passed": 3,
    "failed": 0,
    "checks": 192,
    "equiv_checks": 81,
    "mode": "smoke",
    "primary_protocol": "consecutive_e2e",
    "secondary_protocol": "rotating_e2e"
  },
  "paper_eligible": false,
  "issues": "Shared-node same-process method comparisons; no interleave; A3 control partial trace only in checks",
  "next": "Compare A0/A1/A2/A3 then extend after successful four-graph diagnostic",
  "date": "2026-10-04T14:13:07.488361+00:00"
}
```

## journal-a3-build-20261004-221445-build

```json
{
  "id": "journal-a3-build-20261004-221445-build",
  "kind": "journal_a3_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-a3-build-20261004-221445",
  "paper_eligible": false,
  "issues": "A3 row-order control; original frozen TFS/MKL/B64/FULL/A1 source remains unchanged; default NUMA and fixed numerics",
  "next": "Small partial/output bitwise gates before four diagnostic graphs",
  "date": "2026-10-04T14:14:57.683054+00:00"
}
```

## journal-a3-build-20261004-221445-submit-smoke-20261004-221506

```json
{
  "id": "journal-a3-build-20261004-221445-submit-smoke-20261004-221506",
  "kind": "journal_a3_submission",
  "status": "SUBMITTED",
  "job_id": "10868161",
  "mode": "smoke",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-a3-build-20261004-221445/logs/submit-smoke-20261004-221506",
  "paper_eligible": false,
  "issues": "Shared intel one socket 4/32 physical cores, no interleave",
  "next": "Bitwise partial/output gate then focus",
  "date": "2026-10-04T14:15:07.491806+00:00"
}
```

## journal-a3-build-20261004-221445-smoke-10868161

```json
{
  "id": "journal-a3-build-20261004-221445-smoke-10868161",
  "kind": "journal_a3_smoke",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10868161",
  "node": "qhcn001",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-a3-build-20261004-221445",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-a3-build-20261004-221445/logs/smoke-10868161",
  "completion": {
    "passed": 3,
    "failed": 0,
    "checks": 192,
    "equiv_checks": 81,
    "mode": "smoke",
    "primary_protocol": "consecutive_e2e",
    "secondary_protocol": "rotating_e2e"
  },
  "paper_eligible": false,
  "issues": "Shared-node same-process method comparisons; no interleave; A3 control partial trace only in checks",
  "next": "Compare A0/A1/A2/A3 then extend after successful four-graph diagnostic",
  "date": "2026-10-04T14:15:11.944526+00:00"
}
```

## journal-a3-build-20261004-221445-submit-focus-20261004-221554

```json
{
  "id": "journal-a3-build-20261004-221445-submit-focus-20261004-221554",
  "kind": "journal_a3_submission",
  "status": "SUBMITTED",
  "job_id": "10868162",
  "mode": "focus",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-a3-build-20261004-221445/logs/submit-focus-20261004-221554",
  "paper_eligible": false,
  "issues": "Shared intel one socket 4/32 physical cores, no interleave",
  "next": "Bitwise partial/output gate then focus",
  "date": "2026-10-04T14:15:56.032266+00:00"
}
```

## projection-window-validation-10867699

```json
{
  "id": "projection-window-validation-10867699",
  "kind": "independent_projection_window",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10867699",
  "node": "qhcn052",
  "build": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-build-20261004-162142",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/projection-window-validation-10867699",
  "completion": {
    "passed": 13,
    "failed": 0,
    "checks": 780,
    "equiv_checks": 312,
    "mode": "validation",
    "primary_protocol": "consecutive_e2e",
    "secondary_protocol": "rotating_e2e"
  },
  "paper_eligible": true,
  "issues": "Same source/matrix/degree scheduling/NUMA/precision; same-M bitwise gates; both timing orders retained",
  "next": "Reconcile all raw results and retain negative outcomes",
  "date": "2026-10-04T14:23:21.770375+00:00"
}
```

## projection-window-reconciled-20261004-222427

```json
{
  "id": "projection-window-reconciled-20261004-222427",
  "kind": "projection_window_reconciliation",
  "status": "PASS",
  "evidence": "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/runs/projection-window-reconciled-20261004-222427",
  "report": "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/docs/PROJECTION_WINDOW_RESULTS_20261004.md",
  "paper_eligible": true,
  "summary": {
    "graphs": 17,
    "numerical_checks": 1020,
    "bitwise_checks": 408,
    "profile_bitwise_checks": 476,
    "measured_repetitions": 2720,
    "fixed": [
      {
        "method": "b256_accurate",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.2875289401518484,
        "consecutive_gmean_vs_mkl": 2.407840103414133,
        "rotating_gmean_vs_tfs": 1.2901778623906341,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "b256_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.3262670292594745,
        "consecutive_gmean_vs_mkl": 2.4802851736367653,
        "rotating_gmean_vs_tfs": 1.3227614266857368,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "b64_accurate",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.2596072542899965,
        "consecutive_gmean_vs_mkl": 2.35562305968293,
        "rotating_gmean_vs_tfs": 1.264568811028768,
        "wins_vs_tfs": 10,
        "wins_vs_mkl": 17
      },
      {
        "method": "b64_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.3074146321937725,
        "consecutive_gmean_vs_mkl": 2.445028833926894,
        "rotating_gmean_vs_tfs": 1.300968457960267,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "bfull_accurate",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.2190001765584901,
        "consecutive_gmean_vs_mkl": 2.279682747045884,
        "rotating_gmean_vs_tfs": 1.2204429856783343,
        "wins_vs_tfs": 10,
        "wins_vs_mkl": 17
      },
      {
        "method": "bfull_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.253684756440289,
        "consecutive_gmean_vs_mkl": 2.344547248188372,
        "rotating_gmean_vs_tfs": 1.2525780771123407,
        "wins_vs_tfs": 10,
        "wins_vs_mkl": 17
      },
      {
        "method": "paper_tfs",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.0,
        "consecutive_gmean_vs_mkl": 1.8701250343391562,
        "rotating_gmean_vs_tfs": 1.0,
        "wins_vs_tfs": 0,
        "wins_vs_mkl": 15
      },
      {
        "method": "s128_mfull_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.3238602881863386,
        "consecutive_gmean_vs_mkl": 2.475784266904722,
        "rotating_gmean_vs_tfs": 1.3243461045820613,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "s32_m256_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.3095007298576296,
        "consecutive_gmean_vs_mkl": 2.4489300973921497,
        "rotating_gmean_vs_tfs": 1.3119716829836694,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "s32_m64_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.2970858077262817,
        "consecutive_gmean_vs_mkl": 2.4257126407149445,
        "rotating_gmean_vs_tfs": 1.2991063829877882,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "s32_mfull_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.314789714045438,
        "consecutive_gmean_vs_mkl": 2.4588211591279943,
        "rotating_gmean_vs_tfs": 1.3127997526652513,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "s64_m256_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.316843632737476,
        "consecutive_gmean_vs_mkl": 2.4626622438924715,
        "rotating_gmean_vs_tfs": 1.315981341952931,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "s64_m64_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.300538679597426,
        "consecutive_gmean_vs_mkl": 2.432169942841537,
        "rotating_gmean_vs_tfs": 1.3011266458663404,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "s64_mfull_accurate",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.2823198993748035,
        "consecutive_gmean_vs_mkl": 2.3980985458520876,
        "rotating_gmean_vs_tfs": 1.2831922260171589,
        "wins_vs_tfs": 10,
        "wins_vs_mkl": 17
      },
      {
        "method": "s64_mfull_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.3194523971458192,
        "consecutive_gmean_vs_mkl": 2.467540959521207,
        "rotating_gmean_vs_tfs": 1.3221276679326475,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "source_mkl_fp32",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 0.5347236049130634,
        "consecutive_gmean_vs_mkl": 1.0,
        "rotating_gmean_vs_tfs": 0.5261171969466055,
        "wins_vs_tfs": 2,
        "wins_vs_mkl": 0
      }
    ],
    "contrasts": [
      {
        "graph": "amazon0601",
        "s64_full_vs_coupled_full": 1.0028253908388123,
        "s128_full_vs_coupled_full": 0.9962574850245681,
        "s64_full_vs_b64": 0.98472196060712,
        "s64_m256_vs_coupled_b256": 1.0001881113608824,
        "new_coupled64_vs_old": 0.982726930950948
      },
      {
        "graph": "as-Skitter",
        "s64_full_vs_coupled_full": 1.032640108624386,
        "s128_full_vs_coupled_full": 1.0346959153738877,
        "s64_full_vs_b64": 1.0205040048830432,
        "s64_m256_vs_coupled_b256": 0.9922708282135679,
        "new_coupled64_vs_old": 0.977097142584256
      },
      {
        "graph": "cit-Patents",
        "s64_full_vs_coupled_full": 0.9935517049273154,
        "s128_full_vs_coupled_full": 0.9936936550795215,
        "s64_full_vs_b64": 0.9810922389039691,
        "s64_m256_vs_coupled_b256": 0.9966257888179835,
        "new_coupled64_vs_old": 0.9811735326555853
      },
      {
        "graph": "com-LiveJournal",
        "s64_full_vs_coupled_full": 0.9944525852056391,
        "s128_full_vs_coupled_full": 0.9968023696737485,
        "s64_full_vs_b64": 0.9862057167881658,
        "s64_m256_vs_coupled_b256": 0.9823908358995666,
        "new_coupled64_vs_old": 0.985354579758002
      },
      {
        "graph": "com-Youtube",
        "s64_full_vs_coupled_full": 0.9948989392359476,
        "s128_full_vs_coupled_full": 1.0095073091506892,
        "s64_full_vs_b64": 1.0312110928531533,
        "s64_m256_vs_coupled_b256": 0.99246368675751,
        "new_coupled64_vs_old": 1.0034823425160755
      },
      {
        "graph": "email-Enron",
        "s64_full_vs_coupled_full": 0.9771486349835947,
        "s128_full_vs_coupled_full": 0.9881390593029283,
        "s64_full_vs_b64": 1.025480283108432,
        "s64_m256_vs_coupled_b256": 0.9786585365816752,
        "new_coupled64_vs_old": 1.006750049633199
      },
      {
        "graph": "hollywood-2009",
        "s64_full_vs_coupled_full": 1.0054759157523347,
        "s128_full_vs_coupled_full": 1.0070034166290056,
        "s64_full_vs_b64": 1.008664903290139,
        "s64_m256_vs_coupled_b256": 0.9811067985756015,
        "new_coupled64_vs_old": 0.9957994347752452
      },
      {
        "graph": "indochina-2004",
        "s64_full_vs_coupled_full": 0.9946818974174905,
        "s128_full_vs_coupled_full": 0.9961203809246164,
        "s64_full_vs_b64": 1.0242379035049092,
        "s64_m256_vs_coupled_b256": 0.9999241930510603,
        "new_coupled64_vs_old": 0.9979890979196158
      },
      {
        "graph": "mycielskian19",
        "s64_full_vs_coupled_full": 2.2588805783127994,
        "s128_full_vs_coupled_full": 2.269317358982073,
        "s64_full_vs_b64": 1.0327853650316114,
        "s64_m256_vs_coupled_b256": 0.9722049312301099,
        "new_coupled64_vs_old": 0.9918177256070377
      },
      {
        "graph": "ogbn-products",
        "s64_full_vs_coupled_full": 0.990921300155404,
        "s128_full_vs_coupled_full": 1.001640803294233,
        "s64_full_vs_b64": 0.9835979960632604,
        "s64_m256_vs_coupled_b256": 1.0076693699954573,
        "new_coupled64_vs_old": 0.9847158805179771
      },
      {
        "graph": "reddit",
        "s64_full_vs_coupled_full": 1.1110021546001685,
        "s128_full_vs_coupled_full": 1.1346695953501265,
        "s64_full_vs_b64": 1.0648490898113754,
        "s64_m256_vs_coupled_b256": 0.9896893786401338,
        "new_coupled64_vs_old": 1.0356583756987248
      },
      {
        "graph": "rgg_n_2_24_s0",
        "s64_full_vs_coupled_full": 0.9979302243042748,
        "s128_full_vs_coupled_full": 0.9996126026127442,
        "s64_full_vs_b64": 0.9945758832406767,
        "s64_m256_vs_coupled_b256": 0.9982063089059072,
        "new_coupled64_vs_old": 0.9961193994050752
      },
      {
        "graph": "roadNet-CA",
        "s64_full_vs_coupled_full": 0.9995435709569944,
        "s128_full_vs_coupled_full": 0.9964616075450085,
        "s64_full_vs_b64": 0.9935739595229246,
        "s64_m256_vs_coupled_b256": 1.012294047663447,
        "new_coupled64_vs_old": 0.9927511881344443
      },
      {
        "graph": "soc-LiveJournal1",
        "s64_full_vs_coupled_full": 0.9859394296658451,
        "s128_full_vs_coupled_full": 0.9846470314840523,
        "s64_full_vs_b64": 0.9929770402912382,
        "s64_m256_vs_coupled_b256": 0.9855144482145834,
        "new_coupled64_vs_old": 0.9956223957738479
      },
      {
        "graph": "soc-Pokec",
        "s64_full_vs_coupled_full": 1.006731952341802,
        "s128_full_vs_coupled_full": 0.9980848585852785,
        "s64_full_vs_b64": 1.0107245393463697,
        "s64_m256_vs_coupled_b256": 1.012880597542586,
        "new_coupled64_vs_old": 0.9928449744471184
      },
      {
        "graph": "web-Google",
        "s64_full_vs_coupled_full": 0.9917263836473829,
        "s128_full_vs_coupled_full": 0.9942260715094153,
        "s64_full_vs_b64": 0.9914494888552837,
        "s64_m256_vs_coupled_b256": 0.994995095412554,
        "new_coupled64_vs_old": 0.9946884757708325
      },
      {
        "graph": "wiki-Talk",
        "s64_full_vs_coupled_full": 0.9818427393987024,
        "s128_full_vs_coupled_full": 0.9842295561204978,
        "s64_full_vs_b64": 1.0342885208675936,
        "s64_m256_vs_coupled_b256": 0.9832263629216336,
        "new_coupled64_vs_old": 0.9973704204379816
      }
    ],
    "runs": [
      "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/runs/projection-window-focus-10867479",
      "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/runs/projection-window-validation-10867699"
    ],
    "nodes": {
      "projection-window-focus-10867479": "qhcn068",
      "projection-window-validation-10867699": "qhcn052"
    },
    "same_binary_hashes": true,
    "pmu_regions": 459,
    "pmu_available": 459,
    "primary_statistic": "median of five consecutive complete forwards, same graph/process/protocol baseline",
    "model": "two-layer 128->128->128 static inference compute with random source H/W; not checkpoint classification"
  },
  "issues": "All negative cases and both timing orders retained; same-process denominators; no novelty claim from caching alone",
  "next": "Analyze fixed-M contrasts, detailed timing, PMU and remaining limits",
  "date": "2026-10-04T14:24:52.265728+00:00"
}
```

## feature-precision-ablation-20261004-214015-submit-extension-20261004-222500

```json
{
  "id": "feature-precision-ablation-20261004-214015-submit-extension-20261004-222500",
  "kind": "feature_precision_submission",
  "status": "SUBMITTED",
  "job_id": "10868182",
  "mode": "extension",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-214015/logs/submit-extension-20261004-222500",
  "paper_eligible": false,
  "issues": "User explicitly authorizes shared intel node; 32 physical cores on one socket verified in job",
  "next": "Correctness before real-graph timing",
  "date": "2026-10-04T14:25:02.052413+00:00"
}
```

## journal-a3-build-20261004-221445-focus-10868162

```json
{
  "id": "journal-a3-build-20261004-221445-focus-10868162",
  "kind": "journal_a3_focus",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10868162",
  "node": "qhcn064",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-a3-build-20261004-221445",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-a3-build-20261004-221445/logs/focus-10868162",
  "completion": {
    "passed": 4,
    "failed": 0,
    "checks": 256,
    "equiv_checks": 108,
    "mode": "focus",
    "primary_protocol": "consecutive_e2e",
    "secondary_protocol": "rotating_e2e"
  },
  "paper_eligible": false,
  "issues": "Shared-node same-process method comparisons; no interleave; A3 control partial trace only in checks",
  "next": "Compare A0/A1/A2/A3 then extend after successful four-graph diagnostic",
  "date": "2026-10-04T14:35:38.053571+00:00"
}
```

## journal-a3-build-20261004-221445-submit-validation-20261004-223606

```json
{
  "id": "journal-a3-build-20261004-221445-submit-validation-20261004-223606",
  "kind": "journal_a3_submission",
  "status": "SUBMITTED",
  "job_id": "10868206",
  "mode": "validation",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-a3-build-20261004-221445/logs/submit-validation-20261004-223606",
  "paper_eligible": false,
  "issues": "Shared intel one socket 4/32 physical cores, no interleave",
  "next": "Bitwise partial/output gate then focus",
  "date": "2026-10-04T14:36:07.794350+00:00"
}
```

## feature-precision-ablation-20261004-214015-extension-10868182

```json
{
  "id": "feature-precision-ablation-20261004-214015-extension-10868182",
  "kind": "feature_precision_extension",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10868182",
  "node": "qhcn052",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-214015",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/feature-precision-ablation-20261004-214015/logs/extension-10868182",
  "completion": {
    "status": "PASS",
    "mode": "extension",
    "study": "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/runs/feature-precision-ablation-20261004-214015",
    "node": "qhcn052",
    "job_id": "10868182",
    "passed": 12,
    "failed": 0,
    "graphs": 12,
    "graph_names": [
      "hollywood-2009",
      "indochina-2004",
      "soc-Pokec",
      "cit-Patents",
      "soc-LiveJournal1",
      "com-LiveJournal",
      "as-Skitter",
      "rgg_n_2_24_s0",
      "web-Google",
      "email-Enron",
      "amazon0601",
      "com-Youtube"
    ],
    "driver_sha256": "48815149955a6f38339a9b37dd64076f16fe365acd4c2d07ed026e129988ec88",
    "source_manifest_sha256": "88d9225dd5fa9b1a61a7d256f3c50f13211a253cc5d1da19eb9d63b36b901120",
    "counts": {
      "CHECK": 288,
      "EQ": 72,
      "DIFF": 72,
      "PROFILE_GATE": 96,
      "TIME": 2160,
      "STAGE": 240
    }
  },
  "paper_eligible": false,
  "issues": "Shared node explicitly authorized; source controls frozen; no NUMA/MKL policy change",
  "next": "Five diagnostic graph gates before remaining-twelve extension",
  "date": "2026-10-04T14:51:34.220629+00:00"
}
```

## journal-a3-build-20261004-221445-validation-10868206

```json
{
  "id": "journal-a3-build-20261004-221445-validation-10868206",
  "kind": "journal_a3_validation",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10868206",
  "node": "qhcn165",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-a3-build-20261004-221445",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-a3-build-20261004-221445/logs/validation-10868206",
  "completion": {
    "passed": 13,
    "failed": 0,
    "checks": 832,
    "equiv_checks": 351,
    "mode": "validation",
    "primary_protocol": "consecutive_e2e",
    "secondary_protocol": "rotating_e2e"
  },
  "paper_eligible": false,
  "issues": "Shared-node same-process method comparisons; no interleave; A3 control partial trace only in checks",
  "next": "Compare A0/A1/A2/A3 then extend after successful four-graph diagnostic",
  "date": "2026-10-04T15:05:41.575170+00:00"
}
```

## journal-a3-reconciled-20261004-230637

```json
{
  "id": "journal-a3-reconciled-20261004-230637",
  "kind": "journal_a3_reconciliation",
  "status": "PASS",
  "evidence": "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/runs/journal-a3-reconciled-20261004-230637",
  "report": "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/docs/JOURNAL_A3_RESULTS_20261004.md",
  "paper_eligible": true,
  "summary": {
    "graphs": 17,
    "numerical_checks": 1088,
    "bitwise_checks": 459,
    "profile_bitwise_checks": 510,
    "measured_repetitions": 2890,
    "fixed": [
      {
        "method": "b256_accurate",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.281665817889672,
        "consecutive_gmean_vs_mkl": 2.425270478134848,
        "rotating_gmean_vs_tfs": 1.2856995927238688,
        "wins_vs_tfs": 10,
        "wins_vs_mkl": 17
      },
      {
        "method": "b256_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.3191397611232631,
        "consecutive_gmean_vs_mkl": 2.4961816680528064,
        "rotating_gmean_vs_tfs": 1.3236452198446997,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "b64_accurate",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.2542675415609839,
        "consecutive_gmean_vs_mkl": 2.373425270277812,
        "rotating_gmean_vs_tfs": 1.2610267984051047,
        "wins_vs_tfs": 10,
        "wins_vs_mkl": 17
      },
      {
        "method": "b64_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.303216245484174,
        "consecutive_gmean_vs_mkl": 2.466049919317251,
        "rotating_gmean_vs_tfs": 1.3057303290798898,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "bfull_accurate",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.2144240983270467,
        "consecutive_gmean_vs_mkl": 2.2980303231131773,
        "rotating_gmean_vs_tfs": 1.2194687533539812,
        "wins_vs_tfs": 9,
        "wins_vs_mkl": 17
      },
      {
        "method": "bfull_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.2490701904048966,
        "consecutive_gmean_vs_mkl": 2.3635904271015202,
        "rotating_gmean_vs_tfs": 1.2510668092224049,
        "wins_vs_tfs": 10,
        "wins_vs_mkl": 17
      },
      {
        "method": "paper_tfs",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.0,
        "consecutive_gmean_vs_mkl": 1.8922799096945404,
        "rotating_gmean_vs_tfs": 1.0,
        "wins_vs_tfs": 0,
        "wins_vs_mkl": 15
      },
      {
        "method": "s128_mfull_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.3184363399330747,
        "consecutive_gmean_vs_mkl": 2.4948505982665594,
        "rotating_gmean_vs_tfs": 1.3239344035768585,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "s32_m256_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.305842087768284,
        "consecutive_gmean_vs_mkl": 2.471018747917498,
        "rotating_gmean_vs_tfs": 1.3106382891570718,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "s32_m64_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.285618809781826,
        "consecutive_gmean_vs_mkl": 2.4327506452755565,
        "rotating_gmean_vs_tfs": 1.2932763138025698,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "s32_mfull_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.310767551202732,
        "consecutive_gmean_vs_mkl": 2.4803391034204396,
        "rotating_gmean_vs_tfs": 1.3150391498724763,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "s64_m256_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.3110389619399403,
        "consecutive_gmean_vs_mkl": 2.4808526885057343,
        "rotating_gmean_vs_tfs": 1.3139436480750208,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "s64_m64_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.2916013138471463,
        "consecutive_gmean_vs_mkl": 2.4440712175280277,
        "rotating_gmean_vs_tfs": 1.3017928906045362,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "s64_mfull_accurate",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.2785134852086866,
        "consecutive_gmean_vs_mkl": 2.4193053823339454,
        "rotating_gmean_vs_tfs": 1.2832268932078303,
        "wins_vs_tfs": 10,
        "wins_vs_mkl": 17
      },
      {
        "method": "s64_mfull_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.3142315778502682,
        "consecutive_gmean_vs_mkl": 2.4868940114522196,
        "rotating_gmean_vs_tfs": 1.321084300375867,
        "wins_vs_tfs": 11,
        "wins_vs_mkl": 17
      },
      {
        "method": "s64_mfull_rowwise_fast",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 1.2441322569238396,
        "consecutive_gmean_vs_mkl": 2.354246474779908,
        "rotating_gmean_vs_tfs": 1.245002600148868,
        "wins_vs_tfs": 10,
        "wins_vs_mkl": 17
      },
      {
        "method": "source_mkl_fp32",
        "graphs": 17,
        "consecutive_gmean_vs_tfs": 0.5284630433778816,
        "consecutive_gmean_vs_mkl": 1.0,
        "rotating_gmean_vs_tfs": 0.5213572936782243,
        "wins_vs_tfs": 2,
        "wins_vs_mkl": 0
      }
    ],
    "contrasts": [
      {
        "graph": "amazon0601",
        "s64_full_vs_coupled_full": 0.9764235059873657,
        "rowwise_a3_vs_coupled_full": 0.9759175975640753,
        "interleaved_a1_vs_rowwise_a3": 1.0005183925615782,
        "s128_full_vs_coupled_full": 0.977092108814848,
        "s64_full_vs_b64": 0.9666984614055267,
        "s64_m256_vs_coupled_b256": 0.9827931005782756,
        "new_coupled64_vs_old": 0.9812051438499446
      },
      {
        "graph": "as-Skitter",
        "s64_full_vs_coupled_full": 1.0277907831216746,
        "rowwise_a3_vs_coupled_full": 1.007510432845815,
        "interleaved_a1_vs_rowwise_a3": 1.0201291714851781,
        "s128_full_vs_coupled_full": 1.0212550950299184,
        "s64_full_vs_b64": 1.037647627876314,
        "s64_m256_vs_coupled_b256": 0.9988718942697479,
        "new_coupled64_vs_old": 0.9979989860731338
      },
      {
        "graph": "cit-Patents",
        "s64_full_vs_coupled_full": 0.9871103255133569,
        "rowwise_a3_vs_coupled_full": 0.9896804002409639,
        "interleaved_a1_vs_rowwise_a3": 0.9974031265780535,
        "s128_full_vs_coupled_full": 0.9893132405388643,
        "s64_full_vs_b64": 0.9844015426288538,
        "s64_m256_vs_coupled_b256": 0.9997438716242812,
        "new_coupled64_vs_old": 0.9851846108338113
      },
      {
        "graph": "com-LiveJournal",
        "s64_full_vs_coupled_full": 0.9919133545350208,
        "rowwise_a3_vs_coupled_full": 1.002952224770219,
        "interleaved_a1_vs_rowwise_a3": 0.9889936230634244,
        "s128_full_vs_coupled_full": 0.996084331061874,
        "s64_full_vs_b64": 0.9860106039830274,
        "s64_m256_vs_coupled_b256": 0.9755581320000497,
        "new_coupled64_vs_old": 0.9901135570326403
      },
      {
        "graph": "com-Youtube",
        "s64_full_vs_coupled_full": 1.003372025620949,
        "rowwise_a3_vs_coupled_full": 1.0048424435931536,
        "interleaved_a1_vs_rowwise_a3": 0.9985366681298349,
        "s128_full_vs_coupled_full": 1.0165515957140805,
        "s64_full_vs_b64": 1.0261773812747947,
        "s64_m256_vs_coupled_b256": 1.014498149098757,
        "new_coupled64_vs_old": 0.9888284139761442
      },
      {
        "graph": "email-Enron",
        "s64_full_vs_coupled_full": 0.9981424148623428,
        "rowwise_a3_vs_coupled_full": 0.9903747696154998,
        "interleaved_a1_vs_rowwise_a3": 1.0078431372498098,
        "s128_full_vs_coupled_full": 0.9964970121615137,
        "s64_full_vs_b64": 1.0588235294082,
        "s64_m256_vs_coupled_b256": 1.011184755591111,
        "new_coupled64_vs_old": 1.0219123505966274
      },
      {
        "graph": "hollywood-2009",
        "s64_full_vs_coupled_full": 1.0139879841762836,
        "rowwise_a3_vs_coupled_full": 0.9917070317449788,
        "interleaved_a1_vs_rowwise_a3": 1.0224672728115074,
        "s128_full_vs_coupled_full": 1.0220136478409123,
        "s64_full_vs_b64": 1.0113188988562625,
        "s64_m256_vs_coupled_b256": 0.9906174401765474,
        "new_coupled64_vs_old": 0.9986285156483741
      },
      {
        "graph": "indochina-2004",
        "s64_full_vs_coupled_full": 0.9937516340187925,
        "rowwise_a3_vs_coupled_full": 0.9960977105793251,
        "interleaved_a1_vs_rowwise_a3": 0.9976447325040351,
        "s128_full_vs_coupled_full": 0.9959164993326394,
        "s64_full_vs_b64": 1.0252971484493836,
        "s64_m256_vs_coupled_b256": 1.0022734728099927,
        "new_coupled64_vs_old": 0.9941814832557015
      },
      {
        "graph": "mycielskian19",
        "s64_full_vs_coupled_full": 2.2341504830828365,
        "rowwise_a3_vs_coupled_full": 1.0032049621671406,
        "interleaved_a1_vs_rowwise_a3": 2.227012990701906,
        "s128_full_vs_coupled_full": 2.2680346987890836,
        "s64_full_vs_b64": 1.0174182485052257,
        "s64_m256_vs_coupled_b256": 0.9521446294101099,
        "new_coupled64_vs_old": 1.0079367849553154
      },
      {
        "graph": "ogbn-products",
        "s64_full_vs_coupled_full": 0.9892292776657751,
        "rowwise_a3_vs_coupled_full": 0.9783778759535755,
        "interleaved_a1_vs_rowwise_a3": 1.0110912173904416,
        "s128_full_vs_coupled_full": 0.993056490703703,
        "s64_full_vs_b64": 0.9997606327634355,
        "s64_m256_vs_coupled_b256": 1.016832924822684,
        "new_coupled64_vs_old": 0.9856966592545361
      },
      {
        "graph": "reddit",
        "s64_full_vs_coupled_full": 1.1181456875641056,
        "rowwise_a3_vs_coupled_full": 1.0186124848437834,
        "interleaved_a1_vs_rowwise_a3": 1.097714493196681,
        "s128_full_vs_coupled_full": 1.1370339546304267,
        "s64_full_vs_b64": 1.0425600816477512,
        "s64_m256_vs_coupled_b256": 0.9907610274279249,
        "new_coupled64_vs_old": 0.9796544652523012
      },
      {
        "graph": "rgg_n_2_24_s0",
        "s64_full_vs_coupled_full": 0.9958539408239367,
        "rowwise_a3_vs_coupled_full": 0.999318051012901,
        "interleaved_a1_vs_rowwise_a3": 0.9965335258525021,
        "s128_full_vs_coupled_full": 0.9989456521552269,
        "s64_full_vs_b64": 0.9962601000669655,
        "s64_m256_vs_coupled_b256": 0.9966782741023621,
        "new_coupled64_vs_old": 0.9988468281471675
      },
      {
        "graph": "roadNet-CA",
        "s64_full_vs_coupled_full": 0.9942774509331683,
        "rowwise_a3_vs_coupled_full": 0.9890758420593336,
        "interleaved_a1_vs_rowwise_a3": 1.0052590596723145,
        "s128_full_vs_coupled_full": 0.9923212138054127,
        "s64_full_vs_b64": 0.9877700598221203,
        "s64_m256_vs_coupled_b256": 0.9808852253593144,
        "new_coupled64_vs_old": 0.985571320224156
      },
      {
        "graph": "soc-LiveJournal1",
        "s64_full_vs_coupled_full": 0.9872905505494601,
        "rowwise_a3_vs_coupled_full": 0.9923231215822668,
        "interleaved_a1_vs_rowwise_a3": 0.994928495644864,
        "s128_full_vs_coupled_full": 0.9887522347996525,
        "s64_full_vs_b64": 0.9866943156007193,
        "s64_m256_vs_coupled_b256": 0.9903288999051175,
        "new_coupled64_vs_old": 0.9896637668114519
      },
      {
        "graph": "soc-Pokec",
        "s64_full_vs_coupled_full": 1.0036676656257568,
        "rowwise_a3_vs_coupled_full": 1.0011830616109856,
        "interleaved_a1_vs_rowwise_a3": 1.002481668048572,
        "s128_full_vs_coupled_full": 1.0014837248363113,
        "s64_full_vs_b64": 1.0004692979664354,
        "s64_m256_vs_coupled_b256": 1.0060288755417108,
        "new_coupled64_vs_old": 0.9891592150450229
      },
      {
        "graph": "web-Google",
        "s64_full_vs_coupled_full": 1.0001887358057557,
        "rowwise_a3_vs_coupled_full": 0.9933402430185388,
        "interleaved_a1_vs_rowwise_a3": 1.0068944078681499,
        "s128_full_vs_coupled_full": 0.9980059820556173,
        "s64_full_vs_b64": 0.9877987854311063,
        "s64_m256_vs_coupled_b256": 1.0013608022016953,
        "new_coupled64_vs_old": 0.9847267414862606
      },
      {
        "graph": "wiki-Talk",
        "s64_full_vs_coupled_full": 0.9871212353789791,
        "rowwise_a3_vs_coupled_full": 0.9991506331424226,
        "interleaved_a1_vs_rowwise_a3": 0.9879603761791054,
        "s128_full_vs_coupled_full": 0.9853465447682623,
        "s64_full_vs_b64": 1.0336730830222909,
        "s64_m256_vs_coupled_b256": 0.9870939835647526,
        "new_coupled64_vs_old": 0.9702888979615683
      }
    ],
    "runs": [
      "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/runs/journal-a3-build-20261004-221445/logs/focus-10868162",
      "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/runs/journal-a3-build-20261004-221445/logs/validation-10868206"
    ],
    "nodes": {
      "focus-10868162": "qhcn064",
      "validation-10868206": "qhcn165"
    },
    "same_binary_hashes": true,
    "pmu_regions": 510,
    "pmu_available": 510,
    "primary_statistic": "median of five consecutive complete forwards, same graph/process/protocol baseline",
    "model": "two-layer 128->128->128 static inference compute with random source H/W; not checkpoint classification"
  },
  "issues": "All negative cases and both timing orders retained; same-process denominators; no single-cause cache claim",
  "next": "Analyze fixed-M contrasts, detailed timing, PMU and remaining limits",
  "date": "2026-10-04T15:07:04.638175+00:00"
}
```

## journal-p1-build-20261005-111037-build

```json
{
  "id": "journal-p1-build-20261005-111037-build",
  "kind": "journal_p1_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p1-build-20261005-111037",
  "paper_eligible": false,
  "issues": "P1 budget-preserving grouping, unchanged P0 kernels/TFS/MKL, no interleave",
  "next": "Smoke correctness/work gates then five diagnostic graphs and controlled sharing graphs",
  "date": "2026-10-05T03:10:47.609238+00:00"
}
```

## journal-p1-build-20261005-111037-submit-smoke-20261005-111058

```json
{
  "id": "journal-p1-build-20261005-111037-submit-smoke-20261005-111058",
  "kind": "journal_p1_submission",
  "status": "SUBMITTED",
  "job_id": "10869325",
  "mode": "smoke",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p1-build-20261005-111037/logs/submit-smoke-20261005-111058",
  "paper_eligible": false,
  "issues": "Shared intel one socket 4/32 physical cores, no interleave",
  "next": "P1 bitwise/output/work gate then focus",
  "date": "2026-10-05T03:11:00.186912+00:00"
}
```

## journal-p1-build-20261005-111037-smoke-10869325

```json
{
  "id": "journal-p1-build-20261005-111037-smoke-10869325",
  "kind": "journal_p1_smoke",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10869325",
  "node": "qhcn001",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p1-build-20261005-111037",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p1-build-20261005-111037/logs/smoke-10869325",
  "completion": {
    "mode": "smoke",
    "passed": 3,
    "failed": 0,
    "checks": 132,
    "equiv_checks": 72
  },
  "paper_eligible": false,
  "issues": "Shared-node same-process method comparisons; no interleave; P1 same-q64 schedule control, fixed kernels",
  "next": "Reconcile same-work grouping and preprocessing, extend after successful five-graph diagnostic",
  "date": "2026-10-05T03:11:04.747449+00:00"
}
```

## journal-p1-build-20261005-111037-submit-focus-20261005-111201

```json
{
  "id": "journal-p1-build-20261005-111037-submit-focus-20261005-111201",
  "kind": "journal_p1_submission",
  "status": "SUBMITTED",
  "job_id": "10869326",
  "mode": "focus",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p1-build-20261005-111037/logs/submit-focus-20261005-111201",
  "paper_eligible": false,
  "issues": "Shared intel one socket 4/32 physical cores, no interleave",
  "next": "P1 bitwise/output/work gate then focus",
  "date": "2026-10-05T03:12:03.152021+00:00"
}
```

## journal-p1-build-20261005-111037-submit-synthetic-20261005-111206

```json
{
  "id": "journal-p1-build-20261005-111037-submit-synthetic-20261005-111206",
  "kind": "journal_p1_submission",
  "status": "SUBMITTED",
  "job_id": "10869328",
  "mode": "synthetic",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p1-build-20261005-111037/logs/submit-synthetic-20261005-111206",
  "paper_eligible": false,
  "issues": "Shared intel one socket 4/32 physical cores, no interleave",
  "next": "P1 bitwise/output/work gate then focus",
  "date": "2026-10-05T03:12:08.354378+00:00"
}
```

## journal-p1-build-20261005-111037-synthetic-10869328

```json
{
  "id": "journal-p1-build-20261005-111037-synthetic-10869328",
  "kind": "journal_p1_synthetic",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10869328",
  "node": "qhcn177",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p1-build-20261005-111037",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p1-build-20261005-111037/logs/synthetic-10869328",
  "completion": {
    "mode": "synthetic",
    "passed": 3,
    "failed": 0,
    "checks": 132,
    "equiv_checks": 72
  },
  "paper_eligible": false,
  "issues": "Shared-node same-process method comparisons; no interleave; P1 same-q64 schedule control, fixed kernels",
  "next": "Reconcile same-work grouping and preprocessing, extend after successful five-graph diagnostic",
  "date": "2026-10-05T03:14:40.369502+00:00"
}
```

## delayed-residual-gate-20261005-112010-submit

```json
{
  "id": "delayed-residual-gate-20261005-112010-submit",
  "kind": "delayed_residual_gate_submission",
  "status": "SUBMITTED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/delayed-residual-gate-20261005-112010",
  "job_id": "10869361",
  "paper_eligible": false,
  "issues": "Small hardware numerical counterexample under frozen accurate gate",
  "next": "Inspect both FP32 and BF16 boundaries",
  "date": "2026-10-05T03:20:13.985149+00:00"
}
```

## delayed-residual-gate-20261005-112010-build

```json
{
  "id": "delayed-residual-gate-20261005-112010-build",
  "kind": "delayed_residual_gate_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/delayed-residual-gate-20261005-112010",
  "paper_eligible": false,
  "issues": "Original TFS/ACCURATE source unchanged; exact hardware cancellation fixture",
  "next": "Hardware FP32 and final BF16 gate",
  "date": "2026-10-05T03:20:14.063134+00:00"
}
```

## logs

```json
{
  "id": "logs",
  "kind": "delayed_residual_amx_counterexample",
  "status": "FAILED",
  "job_id": "10869361",
  "node": "qhcn001",
  "exit_status": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/delayed-residual-gate-20261005-112010/logs",
  "paper_eligible": false,
  "issues": "D1 expected to fail frozen FP32 accurate numerical gate; BF16 final may hide it. D2 only checked on this counterexample, not general validation.",
  "next": "Do not time D1 as accepted ACCURATE; compare any D2 design to decoupled FULL ACCURATE",
  "date": "2026-10-05T03:20:14.865058+00:00"
}
```

## delayed-residual-gate-20261005-112212-submit

```json
{
  "id": "delayed-residual-gate-20261005-112212-submit",
  "kind": "delayed_residual_gate_submission",
  "status": "SUBMITTED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/delayed-residual-gate-20261005-112212",
  "job_id": "10869364",
  "paper_eligible": false,
  "issues": "Small hardware numerical counterexample under frozen accurate gate",
  "next": "Inspect both FP32 and BF16 boundaries",
  "date": "2026-10-05T03:22:15.762432+00:00"
}
```

## delayed-residual-gate-20261005-112212-build

```json
{
  "id": "delayed-residual-gate-20261005-112212-build",
  "kind": "delayed_residual_gate_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/delayed-residual-gate-20261005-112212",
  "paper_eligible": false,
  "issues": "Original TFS/ACCURATE source unchanged; exact hardware cancellation fixture",
  "next": "Hardware FP32 and final BF16 gate",
  "date": "2026-10-05T03:22:15.829301+00:00"
}
```

## delayed-residual-gate-20261005-112212

```json
{
  "id": "delayed-residual-gate-20261005-112212",
  "kind": "delayed_residual_amx_counterexample",
  "status": "PASS",
  "job_id": "10869364",
  "node": "qhcn001",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/delayed-residual-gate-20261005-112212",
  "paper_eligible": false,
  "issues": "D1 expected to fail frozen FP32 accurate numerical gate; BF16 final may hide it. D2 only checked on this counterexample, not general validation.",
  "next": "Do not time D1 as accepted ACCURATE; compare any D2 design to decoupled FULL ACCURATE",
  "date": "2026-10-05T03:22:17.443695+00:00"
}
```

## avx-amx-overlap-20261005-112746-submit

```json
{
  "id": "avx-amx-overlap-20261005-112746-submit",
  "kind": "avx_amx_overlap_submission",
  "status": "SUBMITTED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/avx-amx-overlap-20261005-112746",
  "job_id": "10869371",
  "paper_eligible": false,
  "issues": "One-core fixed-work AMX handoff microbenchmark",
  "next": "Compare paired serial/interleaved timing and PMU",
  "date": "2026-10-05T03:27:50.693393+00:00"
}
```

## avx-amx-overlap-20261005-112746-build

```json
{
  "id": "avx-amx-overlap-20261005-112746-build",
  "kind": "avx_amx_overlap_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/avx-amx-overlap-20261005-112746",
  "paper_eligible": false,
  "issues": "Original source unchanged; fixed-work two-buffer handoff microbenchmark",
  "next": "FP32 output bitwise and sampled profile gate, paired timings",
  "date": "2026-10-05T03:27:50.806685+00:00"
}
```

## avx-amx-overlap-20261005-112746

```json
{
  "id": "avx-amx-overlap-20261005-112746",
  "kind": "avx_amx_overlap_microbenchmark",
  "status": "PASS",
  "job_id": "10869371",
  "node": "qhcn014",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/avx-amx-overlap-20261005-112746",
  "paper_eligible": false,
  "issues": "P2 microbenchmark only, not two-layer E2E; serial/interleaved fixed work, one physical core",
  "next": "Assess whether fixed-work interleaving warrants a full GCN kernel",
  "date": "2026-10-05T03:27:58.067774+00:00"
}
```

## journal-p1-build-20261005-111037-focus-10869326

```json
{
  "id": "journal-p1-build-20261005-111037-focus-10869326",
  "kind": "journal_p1_focus",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10869326",
  "node": "qhcn068",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p1-build-20261005-111037",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p1-build-20261005-111037/logs/focus-10869326",
  "completion": {
    "mode": "focus",
    "passed": 5,
    "failed": 0,
    "checks": 220,
    "equiv_checks": 120
  },
  "paper_eligible": false,
  "issues": "Shared-node same-process method comparisons; no interleave; P1 same-q64 schedule control, fixed kernels",
  "next": "Reconcile same-work grouping and preprocessing, extend after successful five-graph diagnostic",
  "date": "2026-10-05T03:28:51.460967+00:00"
}
```

## delayed-residual-launch-10869361-resolved

```json
{
  "id": "delayed-residual-launch-10869361-resolved",
  "kind": "launcher_issue_resolution",
  "status": "RESOLVED",
  "job_id": "10869361",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/delayed-residual-gate-20261005-112010/logs",
  "issues": "Preflight shell source assigns RUN=$1. Passing RUN/logs overwrote caller RUN and created logs/logs output targets. Hardware program never ran. The old completion event has id logs because of the same variable overwrite. Failed evidence is retained.",
  "resolution": "Preserve GATE_RUN and restore RUN after sourced preflight. Rebuild in unique delayed-residual-gate-20261005-112212; job10869364 completed with the expected D1 FP32-gate rejection and other counterexample outputs exact.",
  "prevention": "Preserve experiment-specific run variables when sourcing shared shell preflight; do not pass subdirectories through an unprotected RUN global.",
  "paper_eligible": false,
  "next": "Retain this failure and use the successful unique hardware gate as numerical evidence only",
  "date": "2026-10-05T03:30:01.666554+00:00"
}
```

## journal-p1-build-20261005-111037-submit-validation-20261005-113027

```json
{
  "id": "journal-p1-build-20261005-111037-submit-validation-20261005-113027",
  "kind": "journal_p1_submission",
  "status": "SUBMITTED",
  "job_id": "10869374",
  "mode": "validation",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p1-build-20261005-111037/logs/submit-validation-20261005-113027",
  "paper_eligible": false,
  "issues": "Shared intel one socket 4/32 physical cores, no interleave",
  "next": "P1 bitwise/output/work gate then focus",
  "date": "2026-10-05T03:30:28.512186+00:00"
}
```

## p2-overlap-20261005-v2

```json
{
  "id": "p2-overlap-20261005-v2",
  "kind": "p2_overlap_analysis",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/p2-overlap-20261005-v2",
  "run": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/avx-amx-overlap-20261005-112746",
  "paper_eligible": false,
  "summary": [
    {
      "case": "hot1024_d1",
      "serial_median_ms": 1.46007537842,
      "interleaved_median_ms": 1.34205818176,
      "median_speedup": 1.087937466694052,
      "paired_ratio_median": 1.0774560312680912,
      "paired_ratio_min": 0.993275696447254,
      "paired_ratio_max": 1.1165013525700167,
      "serial_min_ms": 1.42097473145,
      "serial_max_ms": 1.49297714233,
      "serial_cv": 0.01713050459742254,
      "interleaved_min_ms": 1.30200386047,
      "interleaved_max_ms": 1.48916244507,
      "interleaved_cv": 0.04564154126221865,
      "serial_pmu_regions": 3,
      "serial_cycles_scaled": 3823712.0,
      "serial_instructions_scaled": 1580582.0,
      "serial_ref_cycles_scaled": 4066524.0,
      "serial_cache_misses_scaled": 799.0,
      "serial_amx_busy_scaled": 524368.0,
      "interleaved_pmu_regions": 3,
      "interleaved_cycles_scaled": 3440872.0,
      "interleaved_instructions_scaled": 1710515.0,
      "interleaved_ref_cycles_scaled": 3571344.0,
      "interleaved_cache_misses_scaled": 108.0,
      "interleaved_amx_busy_scaled": 524336.0
    },
    {
      "case": "hot1024_d64",
      "serial_median_ms": 6.13808631897,
      "interleaved_median_ms": 7.38406181335,
      "median_speedup": 0.8312615026966133,
      "paired_ratio_median": 0.8275326189017032,
      "paired_ratio_min": 0.8149202187189233,
      "paired_ratio_max": 0.8536774443620023,
      "serial_min_ms": 6.00695610046,
      "serial_max_ms": 6.30116462708,
      "serial_cv": 0.01547382560816542,
      "interleaved_min_ms": 7.36379623413,
      "interleaved_max_ms": 7.45892524719,
      "interleaved_cv": 0.005187336530140684,
      "serial_pmu_regions": 3,
      "serial_cycles_scaled": 17784610.0,
      "serial_instructions_scaled": 37723690.0,
      "serial_ref_cycles_scaled": 17147916.0,
      "serial_cache_misses_scaled": 66067.0,
      "serial_amx_busy_scaled": 524648.0,
      "interleaved_pmu_regions": 3,
      "interleaved_cycles_scaled": 20739288.0,
      "interleaved_instructions_scaled": 37886363.0,
      "interleaved_ref_cycles_scaled": 19993500.0,
      "interleaved_cache_misses_scaled": 63087.0,
      "interleaved_amx_busy_scaled": 525516.0
    },
    {
      "case": "hot1024_d8",
      "serial_median_ms": 1.94096565247,
      "interleaved_median_ms": 1.9199848175,
      "median_speedup": 1.0109276046241444,
      "paired_ratio_median": 1.0223287498417717,
      "paired_ratio_min": 0.9349838037968804,
      "paired_ratio_max": 1.0591641490445873,
      "serial_min_ms": 1.9268989563,
      "serial_max_ms": 2.00605392456,
      "serial_cv": 0.016527372847495614,
      "interleaved_min_ms": 1.88994407654,
      "interleaved_max_ms": 2.06089019775,
      "interleaved_cv": 0.028175883651291884,
      "serial_pmu_regions": 3,
      "serial_cycles_scaled": 4855516.0,
      "serial_instructions_scaled": 5611046.0,
      "serial_ref_cycles_scaled": 5037660.0,
      "serial_cache_misses_scaled": 267.0,
      "serial_amx_busy_scaled": 524288.0,
      "interleaved_pmu_regions": 3,
      "interleaved_cycles_scaled": 5092474.0,
      "interleaved_instructions_scaled": 5773716.0,
      "interleaved_ref_cycles_scaled": 5280444.0,
      "interleaved_cache_misses_scaled": 6847.0,
      "interleaved_amx_busy_scaled": 525176.0
    },
    {
      "case": "random524288_d128",
      "serial_median_ms": 72.2100734711,
      "interleaved_median_ms": 73.0769634247,
      "median_speedup": 0.9881373019215111,
      "paired_ratio_median": 0.9920114310112034,
      "paired_ratio_min": 0.9752941939239579,
      "paired_ratio_max": 1.0139311663346415,
      "serial_min_ms": 71.7279911041,
      "serial_max_ms": 74.0950107574,
      "serial_cv": 0.010813649517378274,
      "interleaved_min_ms": 71.6190338135,
      "interleaved_max_ms": 73.5459327698,
      "interleaved_cv": 0.008660556594221649,
      "serial_pmu_regions": 3,
      "serial_cycles_scaled": 212589489.0,
      "serial_instructions_scaled": 74423920.0,
      "serial_ref_cycles_scaled": 196504056.0,
      "serial_cache_misses_scaled": 9101885.0,
      "serial_amx_busy_scaled": 526584.0,
      "interleaved_pmu_regions": 3,
      "interleaved_cycles_scaled": 208706433.0,
      "interleaved_instructions_scaled": 74586589.0,
      "interleaved_ref_cycles_scaled": 195873660.0,
      "interleaved_cache_misses_scaled": 9047640.0,
      "interleaved_amx_busy_scaled": 526212.0
    },
    {
      "case": "random524288_d64",
      "serial_median_ms": 36.572933197,
      "interleaved_median_ms": 36.9701385498,
      "median_speedup": 0.9892560491147483,
      "paired_ratio_median": 0.9875277304849999,
      "paired_ratio_min": 0.9813467163257031,
      "paired_ratio_max": 0.9985483755459676,
      "serial_min_ms": 36.1478328705,
      "serial_max_ms": 37.0290279388,
      "serial_cv": 0.008216458433057211,
      "interleaved_min_ms": 36.5388393402,
      "interleaved_max_ms": 37.7049446106,
      "interleaved_cv": 0.012323636549727863,
      "serial_pmu_regions": 3,
      "serial_cycles_scaled": 107766366.0,
      "serial_instructions_scaled": 37723722.0,
      "serial_ref_cycles_scaled": 100105848.0,
      "serial_cache_misses_scaled": 4517135.0,
      "serial_amx_busy_scaled": 525756.0,
      "interleaved_pmu_regions": 3,
      "interleaved_cycles_scaled": 107690642.0,
      "interleaved_instructions_scaled": 37886393.0,
      "interleaved_ref_cycles_scaled": 101355408.0,
      "interleaved_cache_misses_scaled": 4567121.0,
      "interleaved_amx_busy_scaled": 527236.0
    }
  ],
  "issues": "One-core microbenchmark, not full two-layer inference; fixed work/FP32 outputs verified; retains slowdowns and noisy small cases",
  "next": "No full-kernel buffering expansion from this implementation without stronger evidence",
  "date": "2026-10-05T03:32:47.575390+00:00"
}
```

## p2-amx-decoder-audit-20261005

```json
{
  "id": "p2-amx-decoder-audit-20261005",
  "kind": "decoder_issue_and_resolution",
  "status": "RESOLVED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/p2-overlap-20261005-v2",
  "paper_eligible": false,
  "issues": "GNU objdump 2.30 does not decode AMX; its (bad) entries are not an instruction audit. The old immutable disassembly remains preserved.",
  "resolution": "Decode actual ELF function bytes with iced-x86 1.21.0. ELF symbol sizes/PT_LOAD select bytes; the old demangled output supplies addresses only. Both functions have zero invalid instructions. AVX neighbor reduction appears between current-tile AMX K blocks in the interleaved function.",
  "binary_sha256": "1aff557978952bf8c069c57dd30a0bce281ba0e0be69506664a6ca00fa2a3daa",
  "decoder": "iced-x86 1.21.0",
  "decoded_hashes": {
    "serial": "24406aa69283f013b833c59eff7ea631a41ee0dc446ac75e1ae5c403c1d49c00",
    "interleaved": "666b5d5e15d86736405d258477d84263596b6edad9c196c7956f75a086f07f87"
  },
  "next": "Instruction ordering is validated; effective hardware overlap and full-model speedup are not thereby proved.",
  "date": "2026-10-05T03:51:55.652375+00:00"
}
```

## journal-p1-build-20261005-111037-validation-10869374

```json
{
  "id": "journal-p1-build-20261005-111037-validation-10869374",
  "kind": "journal_p1_validation",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10869374",
  "node": "qhcn068",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p1-build-20261005-111037",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p1-build-20261005-111037/logs/validation-10869374",
  "completion": {
    "mode": "validation",
    "passed": 12,
    "failed": 0,
    "checks": 528,
    "equiv_checks": 288
  },
  "paper_eligible": false,
  "issues": "Shared-node same-process method comparisons; no interleave; P1 same-q64 schedule control, fixed kernels",
  "next": "Reconcile same-work grouping and preprocessing, extend after successful five-graph diagnostic",
  "date": "2026-10-05T03:53:42.293608+00:00"
}
```

## journal-p1-reconciled-20261005-115500

```json
{
  "id": "journal-p1-reconciled-20261005-115500",
  "kind": "journal_p1_reconciliation",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p1-reconciled-20261005-115500",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p1-build-20261005-111037",
  "paper_eligible": false,
  "summary": {
    "real_graphs": [
      "ogbn-products",
      "reddit",
      "mycielskian19",
      "roadNet-CA",
      "wiki-Talk",
      "hollywood-2009",
      "indochina-2004",
      "soc-Pokec",
      "cit-Patents",
      "soc-LiveJournal1",
      "com-LiveJournal",
      "as-Skitter",
      "rgg_n_2_24_s0",
      "web-Google",
      "email-Enron",
      "amazon0601",
      "com-Youtube"
    ],
    "synthetic_graphs": [
      "regular128_reuse1",
      "regular128_reuse4",
      "regular128_reuse16"
    ],
    "numerical_checks": 880,
    "same_scope_bitwise_checks": 480,
    "profile_bitwise_checks": 320,
    "same_work_checks": 320,
    "schedule_audits": 60,
    "fixed": [
      {
        "scope": "mfull",
        "schedule": "qshuffle",
        "graphs": 17,
        "gmean_vs_degree": 0.9291035085365146,
        "alternating_gmean_vs_degree": 0.9297490416641357,
        "wins_vs_degree": 0,
        "wins_over_5pct": 0,
        "gmean_vs_source_tfs": 1.2288167818871714
      },
      {
        "scope": "mfull",
        "schedule": "qsource",
        "graphs": 17,
        "gmean_vs_degree": 0.9994843472007671,
        "alternating_gmean_vs_degree": 1.0027982652396021,
        "wins_vs_degree": 4,
        "wins_over_5pct": 2,
        "gmean_vs_source_tfs": 1.3219013035570495
      },
      {
        "scope": "mfull",
        "schedule": "qpage",
        "graphs": 17,
        "gmean_vs_degree": 1.0125667474113085,
        "alternating_gmean_vs_degree": 1.018495549686195,
        "wins_vs_degree": 8,
        "wins_over_5pct": 3,
        "gmean_vs_source_tfs": 1.3392038675645834
      },
      {
        "scope": "m64",
        "schedule": "qshuffle",
        "graphs": 17,
        "gmean_vs_degree": 0.9270381198910522,
        "alternating_gmean_vs_degree": 0.9338472849143825,
        "wins_vs_degree": 0,
        "wins_over_5pct": 0,
        "gmean_vs_source_tfs": 1.203522596830953
      },
      {
        "scope": "m64",
        "schedule": "qsource",
        "graphs": 17,
        "gmean_vs_degree": 0.9994088151295487,
        "alternating_gmean_vs_degree": 1.0035482130226085,
        "wins_vs_degree": 5,
        "wins_over_5pct": 2,
        "gmean_vs_source_tfs": 1.2974774895144743
      },
      {
        "scope": "m64",
        "schedule": "qpage",
        "graphs": 17,
        "gmean_vs_degree": 1.0122629278359105,
        "alternating_gmean_vs_degree": 1.018310586933332,
        "wins_vs_degree": 7,
        "wins_over_5pct": 3,
        "gmean_vs_source_tfs": 1.3141652769661234
      }
    ]
  },
  "issues": "Shared-node paired complete forwards; all slowdowns retained; grouping preprocessing charged separately; generic PMU not DRAM bytes",
  "next": "Assess grouping necessity, amortization and held-out validity before P2/P3",
  "date": "2026-10-05T03:54:28.439374+00:00"
}
```

## journal-followup-interpretation-20261005

```json
{
  "id": "journal-followup-interpretation-20261005",
  "kind": "journal_p1_p2_p3_interpretation",
  "status": "COMPLETE_CURRENT_ROUND",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/docs/JOURNAL_FOLLOWUP_VALIDATION_20261005.md",
  "paper_eligible": false,
  "summary": {
    "p1_graphs": "17 real + 3 synthetic",
    "p1_numeric": 880,
    "p1_same_scope_bitwise": 480,
    "p1_profile_bitwise": 320,
    "p1_same_work": 320,
    "p1_full_page_gmean_vs_degree": 1.0125667474113085,
    "p1_full_source_gmean_vs_degree": 0.9994843472007671,
    "p2": "5 same-output fixed-work one-core cases; current interleaving slower in sparse-heavy cases",
    "p3": "D1 fails unchanged FP32 1e-3 gate on exact-BF16 hardware fixture; D2 only passes this fixture"
  },
  "issues": "Shared-node evidence; P1 setup charged separately, limited gains and all negative/noisy cases retained. P2 micro is not E2E. P3 experiment PASS denotes expected D1 rejection. Journal plan not fully validated.",
  "next": "Prioritize P0/P1 applicable-region and held-out selection evidence. P4, cross-shape/full-model tests and broad D2 validation remain incomplete.",
  "date": "2026-10-05T04:00:26.738461+00:00"
}
```

## journal-evidence-gitignore-resolved-20261005

```json
{
  "id": "journal-evidence-gitignore-resolved-20261005",
  "kind": "artifact_curation_issue",
  "status": "RESOLVED",
  "paper_eligible": false,
  "evidence": "GCN-extra/results/journal-p1-20261005/CURATED_ARTIFACT_HASHES.json",
  "issues": "The staged-byte integrity check detected that GCN-extra/.gitignore build/ excluded copied provenance/build metadata from an initial unpublished local commit. No executable or numerical measurement was affected.",
  "resolution": "Explicitly include only the copied command, compiler log, binary-hash, build-event and library metadata directories. Keep executables and scratch data excluded. Verify all curated file hashes against staged Git blobs before publishing.",
  "next": "Future evidence packaging must check tracked-file coverage, not only filesystem hashes. Raw NUMA command output retains original trailing spaces.",
  "date": "2026-10-05T04:04:44.964758+00:00"
}
```

## residual-broad-build-20261005-121707

```json
{
  "id": "residual-broad-build-20261005-121707",
  "kind": "residual_broad_build",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/residual-broad-build-20261005-121707",
  "paper_eligible": false,
  "issues": "Frozen ACCURATE controls unchanged; isolated D2/D3 code",
  "next": "Smoke gates before diagnostic graphs",
  "date": "2026-10-05T04:17:14.181063+00:00"
}
```

## residual-broad-build-20261005-121707-smoke-submit-10869475

```json
{
  "id": "residual-broad-build-20261005-121707-smoke-submit-10869475",
  "kind": "residual_broad_submission",
  "status": "SUBMITTED",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/residual-broad-build-20261005-121707",
  "mode": "smoke",
  "job": "10869475",
  "paper_eligible": false,
  "issues": "Shared single-socket physical cores, default NUMA",
  "next": "Numerical gates before accepted timing",
  "date": "2026-10-05T04:19:35.979520+00:00"
}
```

## smoke-10869475

```json
{
  "id": "smoke-10869475",
  "kind": "residual_broad_run",
  "status": "COMPLETE",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/residual-broad-build-20261005-121707/logs/smoke-10869475",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/residual-broad-build-20261005-121707",
  "mode": "smoke",
  "job": "10869475",
  "node": "qhcn001",
  "paper_eligible": false,
  "issues": "Candidate numeric rejection remains evidence; no relaxed gates or cross-node ratios",
  "next": "Inspect accepted candidates against decoupled FULL ACCURATE",
  "completion": {
    "mode": "smoke",
    "graphs": 3,
    "failed": 0,
    "checks": 60,
    "rejections": [
      {
        "graph": "window_tail",
        "methods": []
      },
      {
        "graph": "window_high_tail",
        "methods": []
      },
      {
        "graph": "window_scope_tail",
        "methods": []
      }
    ]
  },
  "date": "2026-10-05T04:19:39.848796+00:00"
}
```

## residual-broad-build-20261005-121707-focus-submit-10869493

```json
{
  "id": "residual-broad-build-20261005-121707-focus-submit-10869493",
  "kind": "residual_broad_submission",
  "status": "SUBMITTED",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/residual-broad-build-20261005-121707",
  "mode": "focus",
  "job": "10869493",
  "paper_eligible": false,
  "issues": "Shared single-socket physical cores, default NUMA",
  "next": "Numerical gates before accepted timing",
  "date": "2026-10-05T04:22:56.387763+00:00"
}
```

## focus-10869493

```json
{
  "id": "focus-10869493",
  "kind": "residual_broad_run",
  "status": "COMPLETE",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/residual-broad-build-20261005-121707/logs/focus-10869493",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/residual-broad-build-20261005-121707",
  "mode": "focus",
  "job": "10869493",
  "node": "qhcn274",
  "paper_eligible": false,
  "issues": "Candidate numeric rejection remains evidence; no relaxed gates or cross-node ratios",
  "next": "Inspect accepted candidates against decoupled FULL ACCURATE",
  "completion": {
    "mode": "focus",
    "graphs": 5,
    "failed": 0,
    "checks": 100,
    "rejections": [
      {
        "graph": "ogbn-products",
        "methods": []
      },
      {
        "graph": "reddit",
        "methods": []
      },
      {
        "graph": "mycielskian19",
        "methods": []
      },
      {
        "graph": "roadNet-CA",
        "methods": []
      },
      {
        "graph": "wiki-Talk",
        "methods": []
      }
    ]
  },
  "date": "2026-10-05T04:27:08.676104+00:00"
}
```

## selective-shape-build-20261005-122940

```json
{
  "id": "selective-shape-build-20261005-122940",
  "kind": "selective_shape_build",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/selective-shape-build-20261005-122940",
  "paper_eligible": false,
  "issues": "P4 isolated ACCURATE shape controls; source TFS/MKL unchanged",
  "next": "Smoke gates before diagnostic graphs",
  "date": "2026-10-05T04:29:57.903012+00:00"
}
```

## selective-shape-build-20261005-122940-smoke-submit-10869522

```json
{
  "id": "selective-shape-build-20261005-122940-smoke-submit-10869522",
  "kind": "selective_shape_submission",
  "status": "SUBMITTED",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/selective-shape-build-20261005-122940",
  "mode": "smoke",
  "job": "10869522",
  "paper_eligible": false,
  "issues": "Shared single-socket physical cores, default NUMA",
  "next": "Numerical gates before accepted timing",
  "date": "2026-10-05T04:30:44.592527+00:00"
}
```

## smoke-10869522

```json
{
  "id": "smoke-10869522",
  "kind": "selective_shape_run",
  "status": "COMPLETE",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/selective-shape-build-20261005-122940/logs/smoke-10869522",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/selective-shape-build-20261005-122940",
  "mode": "smoke",
  "job": "10869522",
  "node": "qhcn001",
  "paper_eligible": false,
  "issues": "Candidate numeric rejection remains evidence; no relaxed gates or cross-node ratios",
  "next": "Check conditional benefits against project_all and project_used, including cache rebuild",
  "completion": {
    "mode": "smoke",
    "graphs": 3,
    "failed": 0,
    "checks": 144
  },
  "date": "2026-10-05T04:30:54.381274+00:00"
}
```

## p4-smoke-control-alignment-20261005

```json
{
  "id": "p4-smoke-control-alignment-20261005",
  "kind": "pre-formal_control_alignment",
  "status": "RESOLVED",
  "paper_eligible": false,
  "evidence": "runs/selective-shape-build-20261005-122940/logs/smoke-10869522",
  "issues": "First P4 smoke passed 144 numerical and degenerate/frozen controls using a continuous small-magnitude random generator. This is a valid independent implementation gate, but the journal shape study should inherit the original discrete H/W distribution.",
  "resolution": "Before any real-graph timing, align to original 0.01*(rand()%200-100), preserving original interleaved W1/W2 order for width128. Add predefined project-used L1/full L2 strong control and sampled state-load/per-thread records. Build/run a new snapshot; preserve the earlier smoke as superseded. No previous measurement is overwritten.",
  "next": "Repeat smoke, then controlled source-sharing graphs and five real graphs with the frozen aligned generator.",
  "date": "2026-10-05T04:34:18.881892+00:00"
}
```

## selective-shape-build-20261005-123418

```json
{
  "id": "selective-shape-build-20261005-123418",
  "kind": "selective_shape_build",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/selective-shape-build-20261005-123418",
  "paper_eligible": false,
  "issues": "P4 isolated ACCURATE shape controls; source TFS/MKL unchanged",
  "next": "Smoke gates before diagnostic graphs",
  "date": "2026-10-05T04:34:31.068672+00:00"
}
```

## selective-shape-build-20261005-123418-smoke-submit-10869541

```json
{
  "id": "selective-shape-build-20261005-123418-smoke-submit-10869541",
  "kind": "selective_shape_submission",
  "status": "SUBMITTED",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/selective-shape-build-20261005-123418",
  "mode": "smoke",
  "job": "10869541",
  "paper_eligible": false,
  "issues": "Shared single-socket physical cores, default NUMA",
  "next": "Numerical gates before accepted timing",
  "date": "2026-10-05T04:36:11.116920+00:00"
}
```

## smoke-10869541

```json
{
  "id": "smoke-10869541",
  "kind": "selective_shape_run",
  "status": "COMPLETE",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/selective-shape-build-20261005-123418/logs/smoke-10869541",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/selective-shape-build-20261005-123418",
  "mode": "smoke",
  "job": "10869541",
  "node": "qhcn001",
  "paper_eligible": false,
  "issues": "Candidate numeric rejection remains evidence; no relaxed gates or cross-node ratios",
  "next": "Check conditional benefits against project_all and project_used, including cache rebuild",
  "completion": {
    "mode": "smoke",
    "graphs": 3,
    "failed": 0,
    "checks": 162
  },
  "date": "2026-10-05T04:36:15.626800+00:00"
}
```

## selective-shape-build-20261005-123418-controlled-submit-10869544

```json
{
  "id": "selective-shape-build-20261005-123418-controlled-submit-10869544",
  "kind": "selective_shape_submission",
  "status": "SUBMITTED",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/selective-shape-build-20261005-123418",
  "mode": "controlled",
  "job": "10869544",
  "paper_eligible": false,
  "issues": "Shared single-socket physical cores, default NUMA",
  "next": "Numerical gates before accepted timing",
  "date": "2026-10-05T04:37:18.175284+00:00"
}
```

## controlled-10869544

```json
{
  "id": "controlled-10869544",
  "kind": "selective_shape_run",
  "status": "COMPLETE",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/selective-shape-build-20261005-123418/logs/controlled-10869544",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/selective-shape-build-20261005-123418",
  "mode": "controlled",
  "job": "10869544",
  "node": "qhcn177",
  "paper_eligible": false,
  "issues": "Candidate numeric rejection remains evidence; no relaxed gates or cross-node ratios",
  "next": "Check conditional benefits against project_all and project_used, including cache rebuild",
  "completion": {
    "mode": "controlled",
    "graphs": 2,
    "failed": 0,
    "checks": 108
  },
  "date": "2026-10-05T04:37:30.235962+00:00"
}
```

## selective-shape-build-20261005-123418-focus-submit-10869550

```json
{
  "id": "selective-shape-build-20261005-123418-focus-submit-10869550",
  "kind": "selective_shape_submission",
  "status": "SUBMITTED",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/selective-shape-build-20261005-123418",
  "mode": "focus",
  "job": "10869550",
  "paper_eligible": false,
  "issues": "Shared single-socket physical cores, default NUMA",
  "next": "Numerical gates before accepted timing",
  "date": "2026-10-05T04:40:35.744552+00:00"
}
```

## focus-10869550

```json
{
  "id": "focus-10869550",
  "kind": "selective_shape_run",
  "status": "COMPLETE",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/selective-shape-build-20261005-123418/logs/focus-10869550",
  "study": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/selective-shape-build-20261005-123418",
  "mode": "focus",
  "job": "10869550",
  "node": "qhcn177",
  "paper_eligible": false,
  "issues": "Candidate numeric rejection remains evidence; no relaxed gates or cross-node ratios",
  "next": "Check conditional benefits against project_all and project_used, including cache rebuild",
  "completion": {
    "mode": "focus",
    "graphs": 5,
    "failed": 0,
    "checks": 270
  },
  "date": "2026-10-05T04:56:20.584725+00:00"
}
```

## journal-p3-p4-reconciled-20261005-1301

```json
{
  "id": "journal-p3-p4-reconciled-20261005-1301",
  "kind": "journal_p3_p4_reconciliation",
  "status": "COMPLETE",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/journal-p3-p4-reconciled-20261005-1301",
  "paper_eligible": false,
  "summary": {
    "p3": {
      "checks": 160,
      "rejections": [],
      "runs": [
        {
          "mode": "smoke",
          "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/residual-broad-build-20261005-121707/logs/smoke-10869475",
          "node": "qhcn001",
          "job": "10869475",
          "completion": {
            "mode": "smoke",
            "graphs": 3,
            "failed": 0,
            "checks": 60,
            "rejections": [
              {
                "graph": "window_tail",
                "methods": []
              },
              {
                "graph": "window_high_tail",
                "methods": []
              },
              {
                "graph": "window_scope_tail",
                "methods": []
              }
            ]
          },
          "affinity": {
            "cpus": [
              32,
              33,
              34,
              35
            ],
            "topology": [
              {
                "cpu": 32,
                "socket": 1,
                "core": 0
              },
              {
                "cpu": 33,
                "socket": 1,
                "core": 1
              },
              {
                "cpu": 34,
                "socket": 1,
                "core": 2
              },
              {
                "cpu": 35,
                "socket": 1,
                "core": 3
              }
            ]
          }
        },
        {
          "mode": "focus",
          "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/residual-broad-build-20261005-121707/logs/focus-10869493",
          "node": "qhcn274",
          "job": "10869493",
          "completion": {
            "mode": "focus",
            "graphs": 5,
            "failed": 0,
            "checks": 100,
            "rejections": [
              {
                "graph": "ogbn-products",
                "methods": []
              },
              {
                "graph": "reddit",
                "methods": []
              },
              {
                "graph": "mycielskian19",
                "methods": []
              },
              {
                "graph": "roadNet-CA",
                "methods": []
              },
              {
                "graph": "wiki-Talk",
                "methods": []
              }
            ]
          },
          "affinity": {
            "cpus": [
              32,
              33,
              34,
              35,
              36,
              37,
              38,
              39,
              40,
              41,
              42,
              43,
              44,
              45,
              46,
              47,
              48,
              49,
              50,
              51,
              52,
              53,
              54,
              55,
              56,
              57,
              58,
              59,
              60,
              61,
              62,
              63
            ],
            "topology": [
              {
                "cpu": 32,
                "socket": 1,
                "core": 0
              },
              {
                "cpu": 33,
                "socket": 1,
                "core": 1
              },
              {
                "cpu": 34,
                "socket": 1,
                "core": 2
              },
              {
                "cpu": 35,
                "socket": 1,
                "core": 3
              },
              {
                "cpu": 36,
                "socket": 1,
                "core": 4
              },
              {
                "cpu": 37,
                "socket": 1,
                "core": 5
              },
              {
                "cpu": 38,
                "socket": 1,
                "core": 6
              },
              {
                "cpu": 39,
                "socket": 1,
                "core": 7
              },
              {
                "cpu": 40,
                "socket": 1,
                "core": 8
              },
              {
                "cpu": 41,
                "socket": 1,
                "core": 9
              },
              {
                "cpu": 42,
                "socket": 1,
                "core": 10
              },
              {
                "cpu": 43,
                "socket": 1,
                "core": 11
              },
              {
                "cpu": 44,
                "socket": 1,
                "core": 12
              },
              {
                "cpu": 45,
                "socket": 1,
                "core": 13
              },
              {
                "cpu": 46,
                "socket": 1,
                "core": 14
              },
              {
                "cpu": 47,
                "socket": 1,
                "core": 15
              },
              {
                "cpu": 48,
                "socket": 1,
                "core": 16
              },
              {
                "cpu": 49,
                "socket": 1,
                "core": 17
              },
              {
                "cpu": 50,
                "socket": 1,
                "core": 18
              },
              {
                "cpu": 51,
                "socket": 1,
                "core": 19
              },
              {
                "cpu": 52,
                "socket": 1,
                "core": 20
              },
              {
                "cpu": 53,
                "socket": 1,
                "core": 21
              },
              {
                "cpu": 54,
                "socket": 1,
                "core": 22
              },
              {
                "cpu": 55,
                "socket": 1,
                "core": 23
              },
              {
                "cpu": 56,
                "socket": 1,
                "core": 24
              },
              {
                "cpu": 57,
                "socket": 1,
                "core": 25
              },
              {
                "cpu": 58,
                "socket": 1,
                "core": 26
              },
              {
                "cpu": 59,
                "socket": 1,
                "core": 27
              },
              {
                "cpu": 60,
                "socket": 1,
                "core": 28
              },
              {
                "cpu": 61,
                "socket": 1,
                "core": 29
              },
              {
                "cpu": 62,
                "socket": 1,
                "core": 30
              },
              {
                "cpu": 63,
                "socket": 1,
                "core": 31
              }
            ]
          }
        }
      ]
    },
    "p4": {
      "checks": 540,
      "rejections": [],
      "runs": [
        {
          "mode": "smoke",
          "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/selective-shape-build-20261005-123418/logs/smoke-10869541",
          "node": "qhcn001",
          "job": "10869541",
          "completion": {
            "mode": "smoke",
            "graphs": 3,
            "failed": 0,
            "checks": 162
          },
          "affinity": {
            "cpus": [
              32,
              33,
              34,
              35
            ],
            "topology": [
              {
                "cpu": 32,
                "socket": 1,
                "core": 0
              },
              {
                "cpu": 33,
                "socket": 1,
                "core": 1
              },
              {
                "cpu": 34,
                "socket": 1,
                "core": 2
              },
              {
                "cpu": 35,
                "socket": 1,
                "core": 3
              }
            ]
          }
        },
        {
          "mode": "controlled",
          "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/selective-shape-build-20261005-123418/logs/controlled-10869544",
          "node": "qhcn177",
          "job": "10869544",
          "completion": {
            "mode": "controlled",
            "graphs": 2,
            "failed": 0,
            "checks": 108
          },
          "affinity": {
            "cpus": [
              0,
              1,
              2,
              3,
              4,
              5,
              6,
              7,
              8,
              9,
              10,
              11,
              12,
              13,
              14,
              15,
              16,
              17,
              18,
              19,
              20,
              21,
              22,
              23,
              24,
              25,
              26,
              27,
              28,
              29,
              30,
              31
            ],
            "topology": [
              {
                "cpu": 0,
                "socket": 0,
                "core": 0
              },
              {
                "cpu": 1,
                "socket": 0,
                "core": 1
              },
              {
                "cpu": 2,
                "socket": 0,
                "core": 2
              },
              {
                "cpu": 3,
                "socket": 0,
                "core": 3
              },
              {
                "cpu": 4,
                "socket": 0,
                "core": 4
              },
              {
                "cpu": 5,
                "socket": 0,
                "core": 5
              },
              {
                "cpu": 6,
                "socket": 0,
                "core": 6
              },
              {
                "cpu": 7,
                "socket": 0,
                "core": 7
              },
              {
                "cpu": 8,
                "socket": 0,
                "core": 8
              },
              {
                "cpu": 9,
                "socket": 0,
                "core": 9
              },
              {
                "cpu": 10,
                "socket": 0,
                "core": 10
              },
              {
                "cpu": 11,
                "socket": 0,
                "core": 11
              },
              {
                "cpu": 12,
                "socket": 0,
                "core": 12
              },
              {
                "cpu": 13,
                "socket": 0,
                "core": 13
              },
              {
                "cpu": 14,
                "socket": 0,
                "core": 14
              },
              {
                "cpu": 15,
                "socket": 0,
                "core": 15
              },
              {
                "cpu": 16,
                "socket": 0,
                "core": 16
              },
              {
                "cpu": 17,
                "socket": 0,
                "core": 17
              },
              {
                "cpu": 18,
                "socket": 0,
                "core": 18
              },
              {
                "cpu": 19,
                "socket": 0,
                "core": 19
              },
              {
                "cpu": 20,
                "socket": 0,
                "core": 20
              },
              {
                "cpu": 21,
                "socket": 0,
                "core": 21
              },
              {
                "cpu": 22,
                "socket": 0,
                "core": 22
              },
              {
                "cpu": 23,
                "socket": 0,
                "core": 23
              },
              {
                "cpu": 24,
                "socket": 0,
                "core": 24
              },
              {
                "cpu": 25,
                "socket": 0,
                "core": 25
              },
              {
                "cpu": 26,
                "socket": 0,
                "core": 26
              },
              {
                "cpu": 27,
                "socket": 0,
                "core": 27
              },
              {
                "cpu": 28,
                "socket": 0,
                "core": 28
              },
              {
                "cpu": 29,
                "socket": 0,
                "core": 29
              },
              {
                "cpu": 30,
                "socket": 0,
                "core": 30
              },
              {
                "cpu": 31,
                "socket": 0,
                "core": 31
              }
            ]
          }
        },
        {
          "mode": "focus",
          "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/selective-shape-build-20261005-123418/logs/focus-10869550",
          "node": "qhcn177",
          "job": "10869550",
          "completion": {
            "mode": "focus",
            "graphs": 5,
            "failed": 0,
            "checks": 270
          },
          "affinity": {
            "cpus": [
              0,
              1,
              2,
              3,
              4,
              5,
              6,
              7,
              8,
              9,
              10,
              11,
              12,
              13,
              14,
              15,
              16,
              17,
              18,
              19,
              20,
              21,
              22,
              23,
              24,
              25,
              26,
              27,
              28,
              29,
              30,
              31
            ],
            "topology": [
              {
                "cpu": 0,
                "socket": 0,
                "core": 0
              },
              {
                "cpu": 1,
                "socket": 0,
                "core": 1
              },
              {
                "cpu": 2,
                "socket": 0,
                "core": 2
              },
              {
                "cpu": 3,
                "socket": 0,
                "core": 3
              },
              {
                "cpu": 4,
                "socket": 0,
                "core": 4
              },
              {
                "cpu": 5,
                "socket": 0,
                "core": 5
              },
              {
                "cpu": 6,
                "socket": 0,
                "core": 6
              },
              {
                "cpu": 7,
                "socket": 0,
                "core": 7
              },
              {
                "cpu": 8,
                "socket": 0,
                "core": 8
              },
              {
                "cpu": 9,
                "socket": 0,
                "core": 9
              },
              {
                "cpu": 10,
                "socket": 0,
                "core": 10
              },
              {
                "cpu": 11,
                "socket": 0,
                "core": 11
              },
              {
                "cpu": 12,
                "socket": 0,
                "core": 12
              },
              {
                "cpu": 13,
                "socket": 0,
                "core": 13
              },
              {
                "cpu": 14,
                "socket": 0,
                "core": 14
              },
              {
                "cpu": 15,
                "socket": 0,
                "core": 15
              },
              {
                "cpu": 16,
                "socket": 0,
                "core": 16
              },
              {
                "cpu": 17,
                "socket": 0,
                "core": 17
              },
              {
                "cpu": 18,
                "socket": 0,
                "core": 18
              },
              {
                "cpu": 19,
                "socket": 0,
                "core": 19
              },
              {
                "cpu": 20,
                "socket": 0,
                "core": 20
              },
              {
                "cpu": 21,
                "socket": 0,
                "core": 21
              },
              {
                "cpu": 22,
                "socket": 0,
                "core": 22
              },
              {
                "cpu": 23,
                "socket": 0,
                "core": 23
              },
              {
                "cpu": 24,
                "socket": 0,
                "core": 24
              },
              {
                "cpu": 25,
                "socket": 0,
                "core": 25
              },
              {
                "cpu": 26,
                "socket": 0,
                "core": 26
              },
              {
                "cpu": 27,
                "socket": 0,
                "core": 27
              },
              {
                "cpu": 28,
                "socket": 0,
                "core": 28
              },
              {
                "cpu": 29,
                "socket": 0,
                "core": 29
              },
              {
                "cpu": 30,
                "socket": 0,
                "core": 30
              },
              {
                "cpu": 31,
                "socket": 0,
                "core": 31
              }
            ]
          }
        }
      ]
    }
  },
  "issues": "Shared-node conditional shape experiments; no universal accuracy/novelty or additive profile claims",
  "next": "Interpret fixed selective fractions against strong project-used and mixed-layer controls",
  "date": "2026-10-05T04:59:38.357108+00:00"
}
```

## journal-p3-p4-interpretation-20261005

```json
{
  "id": "journal-p3-p4-interpretation-20261005",
  "kind": "journal_followup_final_audit",
  "status": "COMPLETE",
  "evidence": "GCN-extra/results/journal-p3-p4-20261005",
  "source": "GCN-extra/experiments/journal_followup_p3_p4_20261005",
  "report": "GCN-extra/docs/JOURNAL_P3_P4_INTERPRETATION_20261005.md",
  "paper_eligible": false,
  "results": "P3:160/P4:540 numerical checks passed; zero candidate rejection; 660 PMU regions. D2/D3 0/5 wins vs FULL. Partial P4 fractions do not beat all global/mixed controls.",
  "issues": "Analysis initially stopped on missing P3 work pass-column; schema corrected and complete audit rerun. Controlled small graphs have high CV. P3 fine profile is layer1 only. Source-cache layout/map dependence is not isolated. No universal accuracy or task/novelty claims.",
  "resolution": "All raw measurements retained; source/input hashes, Slurm exit, physical binding, NUMA policy, gates and PMU scaling reconciled. Superseded first P4 smoke remains outside main tables.",
  "next": "Prioritize P0/P1 predictive conditions and held-out selection evidence; isolate same-set Q layout before claiming a selective materialization mechanism.",
  "date": "2026-10-05T05:16:35.872807+00:00"
}
```

## grouping-order-balance-plan-20261007

```json
{
  "id": "grouping-order-balance-plan-20261007",
  "kind": "new_authorized_study_setup",
  "status": "PLANNED",
  "paper_eligible": false,
  "evidence": "experiments/grouping_balance_20261007 and experiments/order_balance_20261007",
  "issues": "User explicitly authorizes new careful experiments on grouping, aggregation/projection placement and AVX-AMX balance; shared/exclusive combined active nodes <=10 supersedes old <=5; no NUMA interleave; existing frozen sources untouched.",
  "next": "Freeze preregistered matrices, build unique snapshots, smoke gates, then paired performance and analysis.",
  "date": "2026-10-07T07:16:01.305137+00:00"
}
```

## grouping-balance-20261007-01-build

```json
{
  "id": "grouping-balance-20261007-01-build",
  "kind": "balance_build",
  "study": "grouping",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01",
  "paper_eligible": false,
  "issues": "New isolated source snapshot; previous sources and frozen evidence unchanged; user node cap10; no NUMA interleave",
  "next": "Smoke numerical/equivalence/work gates before performance",
  "date": "2026-10-07T07:23:15.004897+00:00"
}
```

## grouping-balance-20261007-01-submit-10887774

```json
{
  "id": "grouping-balance-20261007-01-submit-10887774",
  "kind": "balance_submission",
  "status": "SUBMITTED",
  "job": "10887774",
  "mode": "smoke",
  "study": "grouping",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 0,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/launcher/smoke-20261007-152450-398148",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T07:24:50.527635+00:00"
}
```

## grouping-balance-20261007-01-grouping-10887774

```json
{
  "id": "grouping-balance-20261007-01-grouping-10887774",
  "kind": "balance_run",
  "study": "grouping",
  "mode": "smoke",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/logs/grouping-10887774",
  "completion": {
    "study": "grouping",
    "mode": "smoke",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 228
  },
  "job": "10887774",
  "node": "qhcn006",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T07:24:57.223182+00:00"
}
```

## order-balance-20261007-01-build

```json
{
  "id": "order-balance-20261007-01-build",
  "kind": "balance_build",
  "study": "order",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-balance-20261007-01",
  "paper_eligible": false,
  "issues": "New isolated source snapshot; previous sources and frozen evidence unchanged; user node cap10; no NUMA interleave",
  "next": "Smoke numerical/equivalence/work gates before performance",
  "date": "2026-10-07T07:26:22.482593+00:00"
}
```

## grouping-balance-20261007-01-submit-10887784

```json
{
  "id": "grouping-balance-20261007-01-submit-10887784",
  "kind": "balance_submission",
  "status": "SUBMITTED",
  "job": "10887784",
  "mode": "mycielskian19",
  "study": "grouping",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 0,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/launcher/gfocus1-20261007-152800-691396",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T07:28:00.882566+00:00"
}
```

## grouping-balance-20261007-01-submit-10887785

```json
{
  "id": "grouping-balance-20261007-01-submit-10887785",
  "kind": "balance_submission",
  "status": "SUBMITTED",
  "job": "10887785",
  "mode": "reddit,roadNet-CA,amazon0601",
  "study": "grouping",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 1,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/launcher/gfocus2-20261007-152800-976734",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T07:28:01.065794+00:00"
}
```

## grouping-balance-20261007-01-submit-10887786

```json
{
  "id": "grouping-balance-20261007-01-submit-10887786",
  "kind": "balance_submission",
  "status": "SUBMITTED",
  "job": "10887786",
  "mode": "ogbn-products,hollywood-2009,indochina-2004,soc-Pokec",
  "study": "grouping",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 2,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/launcher/gval1-20261007-152801-160880",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T07:28:01.288282+00:00"
}
```

## grouping-balance-20261007-01-submit-10887787

```json
{
  "id": "grouping-balance-20261007-01-submit-10887787",
  "kind": "balance_submission",
  "status": "SUBMITTED",
  "job": "10887787",
  "mode": "cit-Patents,soc-LiveJournal1,com-LiveJournal,as-Skitter",
  "study": "grouping",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 3,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/launcher/gval2-20261007-152801-376747",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T07:28:01.459737+00:00"
}
```

## grouping-balance-20261007-01-submit-10887788

```json
{
  "id": "grouping-balance-20261007-01-submit-10887788",
  "kind": "balance_submission",
  "status": "SUBMITTED",
  "job": "10887788",
  "mode": "rgg_n_2_24_s0,web-Google,wiki-Talk,email-Enron,com-Youtube",
  "study": "grouping",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 4,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/launcher/gval3-20261007-152801-551552",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T07:28:01.701200+00:00"
}
```

## grouping-balance-20261007-01-submit-10887789

```json
{
  "id": "grouping-balance-20261007-01-submit-10887789",
  "kind": "balance_submission",
  "status": "SUBMITTED",
  "job": "10887789",
  "mode": "synthetic",
  "study": "grouping",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 5,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/launcher/gsynthetic-20261007-152801-798430",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T07:28:01.893875+00:00"
}
```

## grouping-balance-20261007-01-submit-10887790

```json
{
  "id": "grouping-balance-20261007-01-submit-10887790",
  "kind": "balance_submission",
  "status": "SUBMITTED",
  "job": "10887790",
  "mode": "mycielskian19,reddit,roadNet-CA,amazon0601",
  "study": "grouping",
  "partition": "intel_expr",
  "exclusive": true,
  "requested_nodes_before": 6,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/launcher/gexclusive-20261007-152801-981607",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T07:28:02.104029+00:00"
}
```

## order-balance-20261007-02-revision

```json
{
  "id": "order-balance-20261007-02-revision",
  "kind": "premeasurement_diagnostic_revision",
  "status": "PLANNED",
  "paper_eligible": false,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-balance-20261007-02",
  "issues": "Build01 preserved; before any order smoke/timing, diagnostic source counts revised to distinguish Pused demand/prefetch map reads and FP32 window state bytes. Kernels, timing, references and gates unchanged.",
  "next": "Rebuild immutable revision02 then smoke",
  "date": "2026-10-07T07:28:53.417183+00:00"
}
```

## order-balance-20261007-02-build

```json
{
  "id": "order-balance-20261007-02-build",
  "kind": "balance_build",
  "study": "order",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-balance-20261007-02",
  "paper_eligible": false,
  "issues": "New isolated source snapshot; previous sources and frozen evidence unchanged; user node cap10; no NUMA interleave",
  "next": "Smoke numerical/equivalence/work gates before performance",
  "date": "2026-10-07T07:29:13.874312+00:00"
}
```

## order-balance-20261007-02-submit-10887799

```json
{
  "id": "order-balance-20261007-02-submit-10887799",
  "kind": "balance_submission",
  "status": "SUBMITTED",
  "job": "10887799",
  "mode": "smoke",
  "study": "order",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 7,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-balance-20261007-02/launcher/smoke-20261007-153002-832308",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T07:30:02.966275+00:00"
}
```

## order-balance-20261007-02-order-10887799

```json
{
  "id": "order-balance-20261007-02-order-10887799",
  "kind": "balance_run",
  "study": "order",
  "mode": "smoke",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-balance-20261007-02/logs/order-10887799",
  "completion": {
    "study": "order",
    "mode": "smoke",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 324
  },
  "job": "10887799",
  "node": "qhcn006",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T07:30:08.414521+00:00"
}
```

## grouping-balance-20261007-01-grouping-10887785

```json
{
  "id": "grouping-balance-20261007-01-grouping-10887785",
  "kind": "balance_run",
  "study": "grouping",
  "mode": "reddit,roadNet-CA,amazon0601",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/logs/grouping-10887785",
  "completion": {
    "study": "grouping",
    "mode": "reddit,roadNet-CA,amazon0601",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 228
  },
  "job": "10887785",
  "node": "qhcn111",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T07:31:00.797102+00:00"
}
```

## grouping-balance-20261007-01-grouping-10887789

```json
{
  "id": "grouping-balance-20261007-01-grouping-10887789",
  "kind": "balance_run",
  "study": "grouping",
  "mode": "synthetic",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/logs/grouping-10887789",
  "completion": {
    "study": "grouping",
    "mode": "synthetic",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 228
  },
  "job": "10887789",
  "node": "qhcn196",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T07:32:10.268253+00:00"
}
```

## order-balance-20261007-02-submit-10887809

```json
{
  "id": "order-balance-20261007-02-submit-10887809",
  "kind": "balance_submission",
  "status": "SUBMITTED",
  "job": "10887809",
  "mode": "mycielskian19",
  "study": "order",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 5,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-balance-20261007-02/launcher/ofocus1-20261007-153419-552280",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T07:34:19.640308+00:00"
}
```

## order-balance-20261007-02-submit-10887810

```json
{
  "id": "order-balance-20261007-02-submit-10887810",
  "kind": "balance_submission",
  "status": "SUBMITTED",
  "job": "10887810",
  "mode": "reddit,ogbn-products",
  "study": "order",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 6,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-balance-20261007-02/launcher/ofocus2-20261007-153419-736761",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T07:34:19.839850+00:00"
}
```

## order-balance-20261007-02-submit-10887811

```json
{
  "id": "order-balance-20261007-02-submit-10887811",
  "kind": "balance_submission",
  "status": "SUBMITTED",
  "job": "10887811",
  "mode": "roadNet-CA,wiki-Talk,hollywood-2009",
  "study": "order",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 7,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-balance-20261007-02/launcher/oval-20261007-153419-933454",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T07:34:20.050748+00:00"
}
```

## grouping-balance-exclusive-backfill-adjustment-20261007

```json
{
  "id": "grouping-balance-exclusive-backfill-adjustment-20261007",
  "kind": "scheduler_adjustment",
  "status": "PASS",
  "job": "10887790",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/launcher/exclusive_backfill_adjustment_20261007",
  "issues": "Exclusive ETA 16:59 with115min request; reduced own pending limit to45min to permit backfill; no work/repeats/numerical changes; all raw evidence retained",
  "next": "Check scheduler start and complete fixed four-graph replication",
  "paper_eligible": false,
  "date": "2026-10-07T07:38:27.543390+00:00"
}
```

## grouping-balance-20261007-01-early-agent-analysis-20261007

```json
{
  "id": "grouping-balance-20261007-01-early-agent-analysis-20261007",
  "kind": "balance_postprocessing_verification",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/analysis_early_agent_20261007",
  "jobs": [
    "10887785",
    "10887789"
  ],
  "graphs": 6,
  "included": 6,
  "excluded": 0,
  "independent_time_groups_verified": 240,
  "rule_hashes": {
    "/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md": {
      "sha256": "55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7",
      "bytes": 7298
    },
    "/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md": {
      "sha256": "641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca",
      "bytes": 4436
    },
    "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/IMPLEMENTATION_CONTRACT.md": {
      "sha256": "b6419e5106278106da6727c90f4cfa185cc70c587d70b4087c0bc286fd0dc244",
      "bytes": 2809
    },
    "/home/huangjianqiang_group/hdacp1/data/wzh/TFS-Train/docs/spec/MATH_SPEC_V2.md": {
      "sha256": "1938d301e3fc913766744af166c0a41b1cb767d7d7419af9c098d3de955cf266",
      "bytes": 824
    },
    "/home/huangjianqiang_group/hdacp1/data/wzh/TFS-Train/docs/spec/KERNEL_LAYOUT_V2.md": {
      "sha256": "732e859128baa3cf87813df82cf7562d14358fc6e1cf17c32ec0dde5a743595f",
      "bytes": 1051
    },
    "/home/huangjianqiang_group/hdacp1/data/wzh/TFS-Train/docs/spec/PYTORCH_EXTENSION_SPEC.md": {
      "sha256": "a3784dd66d4986bde33a40d523f761ee8956da9bbbe711f1707d13d8d2d82df2",
      "bytes": 697
    },
    "/home/huangjianqiang_group/hdacp1/data/wzh/TFS-Train/docs/spec/EXPERIMENT_SPEC_V2.md": {
      "sha256": "ecb8fe1c394d9ec47aae97d8f5b87c5b948963c7e8c8b25d70128483c7c0d10d",
      "bytes": 841
    }
  },
  "paper_eligible": false,
  "source_script": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/grouping_balance_20261007/analyze_balance.py",
  "script_sha256": "403c5ffb74a4450787db8113daee097002aa82fece200bce968bd6a0f208200e",
  "issues": "Analysis-only path guard fixed for canonical workspace and authorized study output. Initial degree-fallback break-even numbers are not meaningful; caveat preserved in verification.json and future script corrected. No raw evidence modified.",
  "next": "Wait for remaining grouping cohorts and exclusive validation; analyze full17 using within-job denominators; retain negative results and order sensitivity.",
  "date": "2026-10-07T07:39:42.472754+00:00"
}
```

## grouping-balance-20261007-01-grouping-10887787

```json
{
  "id": "grouping-balance-20261007-01-grouping-10887787",
  "kind": "balance_run",
  "study": "grouping",
  "mode": "cit-Patents,soc-LiveJournal1,com-LiveJournal,as-Skitter",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/logs/grouping-10887787",
  "completion": {
    "study": "grouping",
    "mode": "cit-Patents,soc-LiveJournal1,com-LiveJournal,as-Skitter",
    "graphs": 4,
    "passed": 4,
    "failed": 0,
    "checks": 304
  },
  "job": "10887787",
  "node": "qhcn115",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T07:39:42.975971+00:00"
}
```

## grouping-balance-20261007-01-submit-10887942

```json
{
  "id": "grouping-balance-20261007-01-submit-10887942",
  "kind": "balance_submission",
  "status": "SUBMITTED",
  "job": "10887942",
  "mode": "mycielskian19,reddit,roadNet-CA,amazon0601",
  "study": "grouping",
  "partition": "intel",
  "exclusive": true,
  "requested_nodes_before": 7,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/launcher/gnodeexclusive-20261007-154006-502675",
  "paper_eligible": false,
  "issues": "Dedicated partition ETA16:59; normal intel partition whole-node --exclusive exploratory validation, not intel_expr result; fresh rules/queue; cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T07:40:06.618984+00:00"
}
```

## order-balance-20261007-02-submit-10887943

```json
{
  "id": "order-balance-20261007-02-submit-10887943",
  "kind": "balance_submission",
  "status": "SUBMITTED",
  "job": "10887943",
  "mode": "reddit,ogbn-products,roadNet-CA,wiki-Talk",
  "study": "order",
  "partition": "intel",
  "exclusive": true,
  "requested_nodes_before": 8,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-balance-20261007-02/launcher/onodeexclusive-20261007-154029-304687",
  "paper_eligible": false,
  "issues": "Dedicated partition ETA16:59; normal intel partition whole-node --exclusive exploratory validation, not intel_expr result; fresh rules/queue; cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T07:40:29.411655+00:00"
}
```

## grouping-balance-20261007-01-grouping-10887786

```json
{
  "id": "grouping-balance-20261007-01-grouping-10887786",
  "kind": "balance_run",
  "study": "grouping",
  "mode": "ogbn-products,hollywood-2009,indochina-2004,soc-Pokec",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/logs/grouping-10887786",
  "completion": {
    "study": "grouping",
    "mode": "ogbn-products,hollywood-2009,indochina-2004,soc-Pokec",
    "graphs": 4,
    "passed": 4,
    "failed": 0,
    "checks": 304
  },
  "job": "10887786",
  "node": "qhcn113",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T07:40:55.527140+00:00"
}
```

## grouping-balance-exclusive-pending-cancelled-20261007

```json
{
  "id": "grouping-balance-exclusive-pending-cancelled-20261007",
  "kind": "scheduler_cancellation",
  "job": "10887790",
  "status": "CANCELLED_PENDING",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/launcher/exclusive_pending_cancellation_20261007",
  "paper_eligible": false,
  "issues": "Dedicated intel_expr estimated start16:59; two normal-intel whole-node exclusive replacements RUNNING (10887942,10887943). Cancel only our unstarted redundant replication. No kernel result lost; no dedicated-partition performance claim.",
  "next": "Complete whole-node exclusive exploratory validation and separate cohorts",
  "date": "2026-10-07T07:42:02.379769+00:00"
}
```

## order-balance-20261007-02-order-10887811

```json
{
  "id": "order-balance-20261007-02-order-10887811",
  "kind": "balance_run",
  "study": "order",
  "mode": "roadNet-CA,wiki-Talk,hollywood-2009",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-balance-20261007-02/logs/order-10887811",
  "completion": {
    "study": "order",
    "mode": "roadNet-CA,wiki-Talk,hollywood-2009",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 324
  },
  "job": "10887811",
  "node": "qhcn259",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T07:43:19.763211+00:00"
}
```

## grouping-balance-20261007-01-grouping-10887784

```json
{
  "id": "grouping-balance-20261007-01-grouping-10887784",
  "kind": "balance_run",
  "study": "grouping",
  "mode": "mycielskian19",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/logs/grouping-10887784",
  "completion": {
    "study": "grouping",
    "mode": "mycielskian19",
    "graphs": 1,
    "passed": 1,
    "failed": 0,
    "checks": 76
  },
  "job": "10887784",
  "node": "qhcn047",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T07:43:35.926896+00:00"
}
```

## order-balance-20261007-02-order-10887810

```json
{
  "id": "order-balance-20261007-02-order-10887810",
  "kind": "balance_run",
  "study": "order",
  "mode": "reddit,ogbn-products",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-balance-20261007-02/logs/order-10887810",
  "completion": {
    "study": "order",
    "mode": "reddit,ogbn-products",
    "graphs": 2,
    "passed": 2,
    "failed": 0,
    "checks": 216
  },
  "job": "10887810",
  "node": "qhcn196",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T07:45:53.693435+00:00"
}
```

## grouping-balance-20261007-01-grouping-10887788

```json
{
  "id": "grouping-balance-20261007-01-grouping-10887788",
  "kind": "balance_run",
  "study": "grouping",
  "mode": "rgg_n_2_24_s0,web-Google,wiki-Talk,email-Enron,com-Youtube",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/logs/grouping-10887788",
  "completion": {
    "study": "grouping",
    "mode": "rgg_n_2_24_s0,web-Google,wiki-Talk,email-Enron,com-Youtube",
    "graphs": 5,
    "passed": 5,
    "failed": 0,
    "checks": 380
  },
  "job": "10887788",
  "node": "qhcn192",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T07:49:25.962329+00:00"
}
```

## order-balance-20261007-02-order-10887943

```json
{
  "id": "order-balance-20261007-02-order-10887943",
  "kind": "balance_run",
  "study": "order",
  "mode": "reddit,ogbn-products,roadNet-CA,wiki-Talk",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-balance-20261007-02/logs/order-10887943",
  "completion": {
    "study": "order",
    "mode": "reddit,ogbn-products,roadNet-CA,wiki-Talk",
    "graphs": 4,
    "passed": 4,
    "failed": 0,
    "checks": 432
  },
  "job": "10887943",
  "node": "qhcn303",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T07:56:32.601527+00:00"
}
```

## grouping-balance-20261007-01-grouping-10887942

```json
{
  "id": "grouping-balance-20261007-01-grouping-10887942",
  "kind": "balance_run",
  "study": "grouping",
  "mode": "mycielskian19,reddit,roadNet-CA,amazon0601",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/grouping-balance-20261007-01/logs/grouping-10887942",
  "completion": {
    "study": "grouping",
    "mode": "mycielskian19,reddit,roadNet-CA,amazon0601",
    "graphs": 4,
    "passed": 4,
    "failed": 0,
    "checks": 304
  },
  "job": "10887942",
  "node": "qhcn302",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T07:58:39.077199+00:00"
}
```

## order-balance-20261007-02-order-10887809

```json
{
  "id": "order-balance-20261007-02-order-10887809",
  "kind": "balance_run",
  "study": "order",
  "mode": "mycielskian19",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-balance-20261007-02/logs/order-10887809",
  "completion": {
    "study": "order",
    "mode": "mycielskian19",
    "graphs": 1,
    "passed": 1,
    "failed": 0,
    "checks": 108
  },
  "job": "10887809",
  "node": "qhcn111",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T07:59:13.234429+00:00"
}
```

## grouping-order-balance-final-reconciliation-20261007

```json
{
  "id": "grouping-order-balance-final-reconciliation-20261007",
  "kind": "balance_final_reconciliation",
  "status": "PASS",
  "required_jobs": 13,
  "graph_run_records": 40,
  "numerical_checks": 3456,
  "bitwise_checks": 1930,
  "profile_output_checks": 1746,
  "work_counter_checks": 1746,
  "raw_timing_samples": 15510,
  "node_cap": 10,
  "numa_interleave": false,
  "queue_empty": true,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/grouping_order_balance_20261007",
  "report": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/grouping_order_balance_20261007/TECHNICAL_REPORT_20261007.txt",
  "report_sha256": "0418a0e596bc8138901b3806bfbb228c09d1998bc3cb5b8eece03e102aaeca2e",
  "desktop_report": "C:/Users/花喵/Desktop/TFS_AVX_AMX分组与计算顺序实验报告_20261007.txt",
  "grouping_shared17": {
    "full_guard_vs_tfs_consecutive": 1.345595179622961,
    "full_guard_vs_b256_consecutive": 1.022333640450388,
    "full_guard_vs_b256_alternating": 1.032966560524897,
    "remaining13_vs_b256_consecutive": 1.004942000980985
  },
  "order_shared6": {
    "compressed_best_fixed_posthoc": "Pall_A_qsource",
    "compressed_vs_AA_degree_consecutive": 1.478101369079781,
    "equal_width_best_fixed_posthoc": "A_A_qsource",
    "equal_width_vs_AA_degree_consecutive": 1.0356621034829687
  },
  "exclusive": {
    "partition": "intel",
    "whole_node": true,
    "group_job": "10887942",
    "order_job": "10887943",
    "program_threads": 32,
    "allocated_cpus": 64,
    "dedicated_partition_job_cancelled": "10887790"
  },
  "issues": "All required Slurm COMPLETED/0:0 and source/binary/numerical/work audits PASS. Retain shared/exclusive and FAST/ACCURATE separation; no universal synergy or overlap proof. Initial analysis path/fallback-amortization/missing-job-binary-manifest issues fixed without altering raw results; earlier analyses retained. Original TFS/MKL and previous frozen experiments unchanged. Two whole-node intel exclusive replacements completed; unstarted intel_expr duplicate cancelled. Random unit-weight inputs, only 32/128 widths, one CPU model; precision-mismatched FP32 MKL timing anchor separately labelled.",
  "next": "Research dataflow/representation/locality cost model with graph-family holdout and overhead; retain negative cases and matched-precision baseline requirement. No active jobs remain.",
  "paper_eligible": false,
  "date": "2026-10-07T08:09:07.806418+00:00"
}
```

## state-reuse-plan-20261007

```json
{
  "id": "state-reuse-plan-20261007",
  "kind": "preregistered_plan",
  "status": "PLANNED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/state_reuse_20261007",
  "protocol_sha256": "8d6225e2dfe1737ce9c36fa5c58b0e2a5a277a91d02121371908b78c5dd736a6",
  "dimensions": "128->128->128",
  "node_cap": 10,
  "numa_interleave": false,
  "paper_eligible": false,
  "issues": "No new major issue. New user authorization covers cluster experiments; preserve old protocols and frozen evidence. New trial separates state lifetime, epilogue, interleaving and shared decode.",
  "next": "Implement isolated kernels and harness; build then smoke gates before timing",
  "date": "2026-10-07T08:35:19.178174+00:00"
}
```

## state-reuse-20261007-01-build

```json
{
  "id": "state-reuse-20261007-01-build",
  "kind": "state_build",
  "status": "FAILED",
  "exit_status": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-01",
  "paper_eligible": false,
  "issues": "Isolated immutable build; 128x128x128; default NUMA; no new major issue if PASS",
  "next": "Smoke all gates before performance",
  "date": "2026-10-07T08:45:26.996400+00:00"
}
```

## state-reuse-build01-macro-fix-20261007

```json
{
  "id": "state-reuse-build01-macro-fix-20261007",
  "kind": "state_build_issue",
  "status": "RESOLVED_FOR_REBUILD",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-01",
  "issues": "Original source defines TR macro; namespace qualification expanded to ::16 in new pair bound check. Changed isolated working header to use existing TR macro. Failed frozen snapshot retained. No numerical gate or algorithm changed.",
  "next": "Fresh build run02 then smoke",
  "paper_eligible": false,
  "date": "2026-10-07T08:46:12.199467+00:00"
}
```

## state-reuse-20261007-02-build

```json
{
  "id": "state-reuse-20261007-02-build",
  "kind": "state_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-02",
  "paper_eligible": false,
  "issues": "Isolated immutable build; 128x128x128; default NUMA; no new major issue if PASS",
  "next": "Smoke all gates before performance",
  "date": "2026-10-07T08:46:24.197927+00:00"
}
```

## state-reuse-20261007-02-submit-10888776

```json
{
  "id": "state-reuse-20261007-02-submit-10888776",
  "kind": "state_submission",
  "status": "SUBMITTED",
  "job": "10888776",
  "mode": "smoke",
  "study": "state",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 0,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-02/launcher/smoke-20261007-164655-560783",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T08:46:55.652214+00:00"
}
```

## state-reuse-20261007-02-state-10888776

```json
{
  "id": "state-reuse-20261007-02-state-10888776",
  "kind": "state_run",
  "study": "state",
  "mode": "smoke",
  "status": "FAILED",
  "exit_status": 2,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-02/logs/state-10888776",
  "completion": {
    "study": "state",
    "mode": "smoke",
    "graphs": 10,
    "passed": 0,
    "failed": 10,
    "checks": 0,
    "equiv": 0
  },
  "job": "10888776",
  "node": "qhcn001",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T08:47:01.283926+00:00"
}
```

## state-reuse-smoke02-semantic-diagnostic-20261007

```json
{
  "id": "state-reuse-smoke02-semantic-diagnostic-20261007",
  "kind": "state_semantic_issue",
  "status": "INVESTIGATING",
  "job": "10888776",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-02/logs/state-10888776",
  "issues": "All ten fixtures stopped at additional nonfinite/subnormal ReLU helper gate before candidate numerical/timing. Original assembly uses compare+masked store. Hypothesis: constant-folded helper on literal subnormals differs from runtime MXCSR DAZ. Route semantic fixture through noinline runtime helper with same production scatter, print bit patterns on failure; no kernel or gate changes.",
  "next": "Fresh run03 build and smoke; preserve failures; resolve exact differing bit pattern",
  "paper_eligible": false,
  "date": "2026-10-07T08:49:48.755933+00:00"
}
```

## state-reuse-20261007-03-build

```json
{
  "id": "state-reuse-20261007-03-build",
  "kind": "state_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-03",
  "paper_eligible": false,
  "issues": "Isolated immutable build; 128x128x128; default NUMA; no new major issue if PASS",
  "next": "Smoke all gates before performance",
  "date": "2026-10-07T08:49:59.719955+00:00"
}
```

## state-reuse-20261007-03-submit-10888795

```json
{
  "id": "state-reuse-20261007-03-submit-10888795",
  "kind": "state_submission",
  "status": "SUBMITTED",
  "job": "10888795",
  "mode": "smoke",
  "study": "state",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 0,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-03/launcher/semantics-20261007-165040-288540",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T08:50:40.376289+00:00"
}
```

## state-reuse-20261007-03-state-10888795

```json
{
  "id": "state-reuse-20261007-03-state-10888795",
  "kind": "state_run",
  "study": "state",
  "mode": "smoke",
  "status": "FAILED",
  "exit_status": 2,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-03/logs/state-10888795",
  "completion": {
    "study": "state",
    "mode": "smoke",
    "graphs": 10,
    "passed": 0,
    "failed": 10,
    "checks": 0,
    "equiv": 0
  },
  "job": "10888795",
  "node": "qhcn001",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T08:50:45.634048+00:00"
}
```

## state-reuse-semantic03-rootcause-20261007

```json
{
  "id": "state-reuse-semantic03-rootcause-20261007",
  "kind": "state_semantic_issue",
  "status": "ROOT_CAUSE_CONFIRMED",
  "job": "10888795",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-03/logs/state-10888795",
  "issues": "No-inline semantic fixture still fails only FP32 subnormals 0x00010000/0x80010000: fused BF16 0x0000/0x8000 vs original 0x0001/0x8001, MXCSR0x9fe1. icpx folds floating compare/select to vmaxps in production epilogue; original compare+conditional store preserves original bits when DAZ treats tiny operand as zero. Use compare-derived integer mask and integer bitwise clear; preserve compiler flags/MXCSR/gates.",
  "next": "Build fixed integer-mask epilogue and rerun complete smoke. Preserve failed run02/03.",
  "paper_eligible": false,
  "date": "2026-10-07T08:52:18.672407+00:00"
}
```

## state-reuse-20261007-04-build

```json
{
  "id": "state-reuse-20261007-04-build",
  "kind": "state_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-04",
  "paper_eligible": false,
  "issues": "Isolated immutable build; 128x128x128; default NUMA; no new major issue if PASS",
  "next": "Smoke all gates before performance",
  "date": "2026-10-07T08:53:12.945358+00:00"
}
```

## state-reuse-20261007-04-submit-10888814

```json
{
  "id": "state-reuse-20261007-04-submit-10888814",
  "kind": "state_submission",
  "status": "SUBMITTED",
  "job": "10888814",
  "mode": "smoke",
  "study": "state",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 0,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-04/launcher/smoke-20261007-165348-173089",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T08:53:48.266062+00:00"
}
```

## state-reuse-20261007-04-state-10888814

```json
{
  "id": "state-reuse-20261007-04-state-10888814",
  "kind": "state_run",
  "study": "state",
  "mode": "smoke",
  "status": "FAILED",
  "exit_status": 2,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-04/logs/state-10888814",
  "completion": {
    "study": "state",
    "mode": "smoke",
    "graphs": 10,
    "passed": 0,
    "failed": 10,
    "checks": 0,
    "equiv": 0
  },
  "job": "10888814",
  "node": "qhcn001",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T08:53:53.505621+00:00"
}
```

## state-reuse-semantic04-nan-20261007

```json
{
  "id": "state-reuse-semantic04-nan-20261007",
  "kind": "state_semantic_issue",
  "status": "INVESTIGATING_COMPILER_FP_ASSUMPTIONS",
  "job": "10888814",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-04/logs/state-10888814",
  "issues": "Integer masking fixes subnormal preservation but vector NaN becomes zero under inherited icpx optimization. Tail NaN remains preserved. No numerical/performance claims from these runs. Will constrain only epilogue comparison semantics without changing all-kernel flags/MXCSR or relaxing fixture.",
  "next": "Inspect compiler predicate and enforce ordered comparison locally; rerun full gate",
  "paper_eligible": false,
  "date": "2026-10-07T08:55:58.334701+00:00"
}
```

## state-reuse-20261007-05-build

```json
{
  "id": "state-reuse-20261007-05-build",
  "kind": "state_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05",
  "paper_eligible": false,
  "issues": "Isolated immutable build; 128x128x128; default NUMA; no new major issue if PASS",
  "next": "Smoke all gates before performance",
  "date": "2026-10-07T08:58:13.678192+00:00"
}
```

## state-reuse-20261007-05-submit-10888825

```json
{
  "id": "state-reuse-20261007-05-submit-10888825",
  "kind": "state_submission",
  "status": "SUBMITTED",
  "job": "10888825",
  "mode": "smoke",
  "study": "state",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 0,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/launcher/smoke-20261007-165857-627017",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T08:58:57.715574+00:00"
}
```

## state-reuse-20261007-05-state-10888825

```json
{
  "id": "state-reuse-20261007-05-state-10888825",
  "kind": "state_run",
  "study": "state",
  "mode": "smoke",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/logs/state-10888825",
  "completion": {
    "study": "state",
    "mode": "smoke",
    "graphs": 10,
    "passed": 10,
    "failed": 0,
    "checks": 2400,
    "equiv": 3000
  },
  "job": "10888825",
  "node": "qhcn034",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T08:59:06.082791+00:00"
}
```

## state-reuse-smoke05-accepted-20261007

```json
{
  "id": "state-reuse-smoke05-accepted-20261007",
  "kind": "state_gate_acceptance",
  "status": "PASS",
  "job": "10888825",
  "checks": 2400,
  "equiv": 3000,
  "fixtures": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05",
  "issues": "Special-value compiler rewrite resolved using local precise comparison plus integer masking; all finite and special semantic gates pass. No numerical gate relaxation, no global flag/MXCSR/NUMA changes. Failed01-04 evidence retained.",
  "next": "Fixed diagnostic4 shared and exclusive, synthetic3; then remaining13",
  "paper_eligible": false,
  "date": "2026-10-07T09:01:09.040206+00:00"
}
```

## state-reuse-20261007-05-submit-10888828

```json
{
  "id": "state-reuse-20261007-05-submit-10888828",
  "kind": "state_submission",
  "status": "SUBMITTED",
  "job": "10888828",
  "mode": "mycielskian19",
  "study": "state",
  "partition": "intel_expr",
  "exclusive": true,
  "requested_nodes_before": 0,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/launcher/ex-myc-20261007-170109-128955",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T09:01:09.194940+00:00"
}
```

## state-reuse-20261007-05-submit-10888829

```json
{
  "id": "state-reuse-20261007-05-submit-10888829",
  "kind": "state_submission",
  "status": "SUBMITTED",
  "job": "10888829",
  "mode": "reddit,roadNet-CA,amazon0601",
  "study": "state",
  "partition": "intel_expr",
  "exclusive": true,
  "requested_nodes_before": 1,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/launcher/ex-focus-20261007-170109-286133",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T09:01:09.366707+00:00"
}
```

## state-reuse-20261007-05-submit-10888830

```json
{
  "id": "state-reuse-20261007-05-submit-10888830",
  "kind": "state_submission",
  "status": "SUBMITTED",
  "job": "10888830",
  "mode": "mycielskian19",
  "study": "state",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 2,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/launcher/sh-myc-20261007-170109-459082",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T09:01:09.534871+00:00"
}
```

## state-reuse-20261007-05-submit-10888831

```json
{
  "id": "state-reuse-20261007-05-submit-10888831",
  "kind": "state_submission",
  "status": "SUBMITTED",
  "job": "10888831",
  "mode": "reddit,roadNet-CA,amazon0601",
  "study": "state",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 3,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/launcher/sh-focus-20261007-170109-629234",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T09:01:09.692163+00:00"
}
```

## state-reuse-20261007-05-submit-10888832

```json
{
  "id": "state-reuse-20261007-05-submit-10888832",
  "kind": "state_submission",
  "status": "SUBMITTED",
  "job": "10888832",
  "mode": "synthetic",
  "study": "state",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 4,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/launcher/sh-synth-20261007-170109-779416",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T09:01:09.873604+00:00"
}
```

## state-reuse-20261007-05-state-10888832

```json
{
  "id": "state-reuse-20261007-05-state-10888832",
  "kind": "state_run",
  "study": "state",
  "mode": "synthetic",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/logs/state-10888832",
  "completion": {
    "study": "state",
    "mode": "synthetic",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 132,
    "equiv": 96
  },
  "job": "10888832",
  "node": "qhcn056",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T09:03:29.731052+00:00"
}
```

## state-reuse-20261007-05-state-10888829

```json
{
  "id": "state-reuse-20261007-05-state-10888829",
  "kind": "state_run",
  "study": "state",
  "mode": "reddit,roadNet-CA,amazon0601",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/logs/state-10888829",
  "completion": {
    "study": "state",
    "mode": "reddit,roadNet-CA,amazon0601",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 216,
    "equiv": 180
  },
  "job": "10888829",
  "node": "qhcn818",
  "partition": "intel_expr",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T09:03:47.480273+00:00"
}
```

## state-reuse-20261007-05-state-10888831

```json
{
  "id": "state-reuse-20261007-05-state-10888831",
  "kind": "state_run",
  "study": "state",
  "mode": "reddit,roadNet-CA,amazon0601",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/logs/state-10888831",
  "completion": {
    "study": "state",
    "mode": "reddit,roadNet-CA,amazon0601",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 216,
    "equiv": 180
  },
  "job": "10888831",
  "node": "qhcn044",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T09:03:50.039212+00:00"
}
```

## state-reuse-analysis01-20261007

```json
{
  "id": "state-reuse-analysis01-20261007",
  "kind": "state_analysis",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/state_reuse_20261007/analysis-01",
  "preflight": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/state_reuse_20261007/analysis01_preflight.json",
  "paper_eligible": false,
  "issues": "Partial cohort only, Mycielskian runs still in progress; no incomplete graph substituted; source-level counters not DRAM.",
  "next": "Complete diagnostic4 then fixed remaining13; final whole-cohort audit",
  "date": "2026-10-07T09:05:19.657799+00:00"
}
```

## state-reuse-20261007-05-submit-10888847

```json
{
  "id": "state-reuse-20261007-05-submit-10888847",
  "kind": "state_submission",
  "status": "SUBMITTED",
  "job": "10888847",
  "mode": "ogbn-products,hollywood-2009,indochina-2004,soc-Pokec",
  "study": "state",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 2,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/launcher/val-a-20261007-171030-439764",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T09:10:30.548038+00:00"
}
```

## state-reuse-20261007-05-submit-10888848

```json
{
  "id": "state-reuse-20261007-05-submit-10888848",
  "kind": "state_submission",
  "status": "SUBMITTED",
  "job": "10888848",
  "mode": "cit-Patents,soc-LiveJournal1,com-LiveJournal,as-Skitter",
  "study": "state",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 3,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/launcher/val-b-20261007-171030-645819",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T09:10:30.732125+00:00"
}
```

## state-reuse-20261007-05-submit-10888849

```json
{
  "id": "state-reuse-20261007-05-submit-10888849",
  "kind": "state_submission",
  "status": "SUBMITTED",
  "job": "10888849",
  "mode": "rgg_n_2_24_s0",
  "study": "state",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 4,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/launcher/val-c-20261007-171030-827325",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T09:10:30.893208+00:00"
}
```

## state-reuse-20261007-05-submit-10888850

```json
{
  "id": "state-reuse-20261007-05-submit-10888850",
  "kind": "state_submission",
  "status": "SUBMITTED",
  "job": "10888850",
  "mode": "web-Google,wiki-Talk,email-Enron,com-Youtube",
  "study": "state",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 5,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/launcher/val-d-20261007-171030-981480",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T09:10:31.048210+00:00"
}
```

## state-reuse-20261007-05-state-10888850

```json
{
  "id": "state-reuse-20261007-05-state-10888850",
  "kind": "state_run",
  "study": "state",
  "mode": "web-Google,wiki-Talk,email-Enron,com-Youtube",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/logs/state-10888850",
  "completion": {
    "study": "state",
    "mode": "web-Google,wiki-Talk,email-Enron,com-Youtube",
    "graphs": 4,
    "passed": 4,
    "failed": 0,
    "checks": 176,
    "equiv": 128
  },
  "job": "10888850",
  "node": "qhcn113",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T09:11:57.450392+00:00"
}
```

## state-reuse-20261007-05-state-10888830

```json
{
  "id": "state-reuse-20261007-05-state-10888830",
  "kind": "state_run",
  "study": "state",
  "mode": "mycielskian19",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/logs/state-10888830",
  "completion": {
    "study": "state",
    "mode": "mycielskian19",
    "graphs": 1,
    "passed": 1,
    "failed": 0,
    "checks": 72,
    "equiv": 60
  },
  "job": "10888830",
  "node": "qhcn034",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T09:15:15.832660+00:00"
}
```

## state-reuse-20261007-05-state-10888828

```json
{
  "id": "state-reuse-20261007-05-state-10888828",
  "kind": "state_run",
  "study": "state",
  "mode": "mycielskian19",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/logs/state-10888828",
  "completion": {
    "study": "state",
    "mode": "mycielskian19",
    "graphs": 1,
    "passed": 1,
    "failed": 0,
    "checks": 72,
    "equiv": 60
  },
  "job": "10888828",
  "node": "qhcn817",
  "partition": "intel_expr",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T09:15:28.981508+00:00"
}
```

## state-reuse-20261007-05-state-10888848

```json
{
  "id": "state-reuse-20261007-05-state-10888848",
  "kind": "state_run",
  "study": "state",
  "mode": "cit-Patents,soc-LiveJournal1,com-LiveJournal,as-Skitter",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/logs/state-10888848",
  "completion": {
    "study": "state",
    "mode": "cit-Patents,soc-LiveJournal1,com-LiveJournal,as-Skitter",
    "graphs": 4,
    "passed": 4,
    "failed": 0,
    "checks": 176,
    "equiv": 128
  },
  "job": "10888848",
  "node": "qhcn044",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T09:17:16.042737+00:00"
}
```

## state-reuse-20261007-05-state-10888847

```json
{
  "id": "state-reuse-20261007-05-state-10888847",
  "kind": "state_run",
  "study": "state",
  "mode": "ogbn-products,hollywood-2009,indochina-2004,soc-Pokec",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/logs/state-10888847",
  "completion": {
    "study": "state",
    "mode": "ogbn-products,hollywood-2009,indochina-2004,soc-Pokec",
    "graphs": 4,
    "passed": 4,
    "failed": 0,
    "checks": 176,
    "equiv": 128
  },
  "job": "10888847",
  "node": "qhcn012",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T09:18:23.253512+00:00"
}
```

## state-reuse-20261007-05-state-10888849

```json
{
  "id": "state-reuse-20261007-05-state-10888849",
  "kind": "state_run",
  "study": "state",
  "mode": "rgg_n_2_24_s0",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/state-reuse-20261007-05/logs/state-10888849",
  "completion": {
    "study": "state",
    "mode": "rgg_n_2_24_s0",
    "graphs": 1,
    "passed": 1,
    "failed": 0,
    "checks": 44,
    "equiv": 32
  },
  "job": "10888849",
  "node": "qhcn056",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T09:19:55.426797+00:00"
}
```

## state-reuse-final-20261007

```json
{
  "id": "state-reuse-final-20261007",
  "kind": "state_final",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/state_reuse_20261007/final",
  "report": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/state_reuse_20261007/TECHNICAL_REPORT_20261007.txt",
  "paper_eligible": false,
  "issues": "All planned jobs complete; 128-only, default NUMA; synthetic and shared/exclusive separate; frozen intermediate analyzer preserved and not used",
  "next": "Use EPI strong baseline and measured decode/cost boundary to design grouping followup",
  "date": "2026-10-07T09:23:58.160921+00:00"
}
```

## state-reuse-desktop-delivery-20261007

```json
{
  "id": "state-reuse-desktop-delivery-20261007",
  "kind": "report_delivery",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/state_reuse_20261007/TECHNICAL_REPORT_20261007.txt",
  "sha256": "25a555b835a757bf2b3fa67525f548404b9d683faed046aa3f4df264f1e8250f",
  "desktop_path": "C:/Users/花喵/Desktop/TFS_128维状态管理与共享解码实验报告_20261007.txt",
  "queue_at_delivery": "",
  "issues": "UTF-8 TXT read back; desktop and remote SHA256 match; all planned jobs completed; no open jobs",
  "paper_eligible": false,
  "next": "Review technical report; future pairing/compile-time reuse work remains a proposal",
  "date": "2026-10-07T09:24:59.895890+00:00"
}
```

## pair-plan-protocol-20261007

```json
{
  "id": "pair-plan-protocol-20261007",
  "kind": "plan_freeze",
  "status": "PREPARED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/pair_plan_20261007",
  "paper_eligible": false,
  "issues": "Continuation authorized; fixed128; fixed TR16 membership; separate pairing and exact-window sharing; no NUMA interleave; old training/nodecap5 contract scope superseded by user",
  "next": "Complete source implementations, build immutable snapshot, smoke before timing",
  "date": "2026-10-07T10:06:14.546330+00:00"
}
```

## pair-plan-prebuild-pmu-20261007

```json
{
  "id": "pair-plan-prebuild-pmu-20261007",
  "kind": "protocol_clarification",
  "status": "PREPARED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/pair_plan_20261007/PROTOCOL.md",
  "paper_eligible": false,
  "issues": "Synthetic PMU coverage added before any new performance data, same four methods/events and32threads as diagnostic4; independent primary timings unchanged",
  "next": "Freeze complete source files then compile",
  "date": "2026-10-07T10:14:26.078160+00:00"
}
```

## pair-plan-20261007-01-build

```json
{
  "id": "pair-plan-20261007-01-build",
  "kind": "plan_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01",
  "paper_eligible": false,
  "issues": "Isolated immutable build; 128x128x128; default NUMA; no new major issue if PASS",
  "next": "Smoke all gates before performance",
  "date": "2026-10-07T10:14:51.360327+00:00"
}
```

## pair-plan-20261007-01-submit-10889177

```json
{
  "id": "pair-plan-20261007-01-submit-10889177",
  "kind": "plan_submission",
  "status": "SUBMITTED",
  "job": "10889177",
  "mode": "smoke",
  "study": "plan",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 0,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/launcher/smoke01-20261007-181513-971868",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T10:15:14.055382+00:00"
}
```

## pair-plan-20261007-01-plan-10889177

```json
{
  "id": "pair-plan-20261007-01-plan-10889177",
  "kind": "plan_run",
  "study": "plan",
  "mode": "smoke",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/logs/plan-10889177",
  "completion": {
    "study": "pair_plan",
    "mode": "smoke",
    "graphs": 13,
    "passed": 13,
    "failed": 0,
    "checks": 5876,
    "equiv": 10504,
    "profile": 2912,
    "work": 2912
  },
  "job": "10889177",
  "node": "qhcn001",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T10:15:27.067298+00:00"
}
```

## pair-plan-20261007-01-submit-10889181

```json
{
  "id": "pair-plan-20261007-01-submit-10889181",
  "kind": "plan_submission",
  "status": "SUBMITTED",
  "job": "10889181",
  "mode": "mycielskian19",
  "study": "plan",
  "partition": "intel_expr",
  "exclusive": true,
  "requested_nodes_before": 0,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/launcher/ex-myc-20261007-181617-750983",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T10:16:17.835464+00:00"
}
```

## pair-plan-20261007-01-submit-10889182

```json
{
  "id": "pair-plan-20261007-01-submit-10889182",
  "kind": "plan_submission",
  "status": "SUBMITTED",
  "job": "10889182",
  "mode": "reddit,roadNet-CA,amazon0601",
  "study": "plan",
  "partition": "intel_expr",
  "exclusive": true,
  "requested_nodes_before": 1,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/launcher/ex-rest-20261007-181617-925709",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T10:16:17.989353+00:00"
}
```

## pair-plan-20261007-01-submit-10889183

```json
{
  "id": "pair-plan-20261007-01-submit-10889183",
  "kind": "plan_submission",
  "status": "SUBMITTED",
  "job": "10889183",
  "mode": "mycielskian19",
  "study": "plan",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 2,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/launcher/sh-myc-20261007-181618-084189",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T10:16:18.148657+00:00"
}
```

## pair-plan-20261007-01-submit-10889184

```json
{
  "id": "pair-plan-20261007-01-submit-10889184",
  "kind": "plan_submission",
  "status": "SUBMITTED",
  "job": "10889184",
  "mode": "reddit,roadNet-CA,amazon0601",
  "study": "plan",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 3,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/launcher/sh-rest-20261007-181618-243133",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T10:16:18.307661+00:00"
}
```

## pair-plan-20261007-01-submit-10889185

```json
{
  "id": "pair-plan-20261007-01-submit-10889185",
  "kind": "plan_submission",
  "status": "SUBMITTED",
  "job": "10889185",
  "mode": "synthetic",
  "study": "plan",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 4,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/launcher/synthetic-20261007-181618-397831",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T10:16:18.460886+00:00"
}
```

## pair-plan-20261007-01-plan-10889182

```json
{
  "id": "pair-plan-20261007-01-plan-10889182",
  "kind": "plan_run",
  "study": "plan",
  "mode": "reddit,roadNet-CA,amazon0601",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/logs/plan-10889182",
  "completion": {
    "study": "pair_plan",
    "mode": "reddit,roadNet-CA,amazon0601",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 276,
    "equiv": 264,
    "profile": 132,
    "work": 132
  },
  "job": "10889182",
  "node": "qhcn819",
  "partition": "intel_expr",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T10:19:23.212750+00:00"
}
```

## pair-plan-20261007-01-plan-10889184

```json
{
  "id": "pair-plan-20261007-01-plan-10889184",
  "kind": "plan_run",
  "study": "plan",
  "mode": "reddit,roadNet-CA,amazon0601",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/logs/plan-10889184",
  "completion": {
    "study": "pair_plan",
    "mode": "reddit,roadNet-CA,amazon0601",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 276,
    "equiv": 264,
    "profile": 132,
    "work": 132
  },
  "job": "10889184",
  "node": "qhcn041",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T10:19:26.659230+00:00"
}
```

## pair-plan-20261007-01-plan-10889185

```json
{
  "id": "pair-plan-20261007-01-plan-10889185",
  "kind": "plan_run",
  "study": "plan",
  "mode": "synthetic",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/logs/plan-10889185",
  "completion": {
    "study": "pair_plan",
    "mode": "synthetic",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 276,
    "equiv": 264,
    "profile": 132,
    "work": 132
  },
  "job": "10889185",
  "node": "qhcn044",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T10:20:28.365138+00:00"
}
```

## pair-plan-synthetic-anomaly-20261007

```json
{
  "id": "pair-plan-synthetic-anomaly-20261007",
  "kind": "retest_preregistration",
  "status": "PREPARED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/logs/plan-10889185/regular128_reuse1/methods.log",
  "paper_eligible": false,
  "issues": "Zero-sharing synthetic reuse1 shows ~2.38x rand_scalar vs PACK in both protocols. Repeat identical frozen full synthetic suite on one exclusive and one separate shared node. No implementation, fixture, selector or timing changes; label retest separately and do not pool with primary",
  "next": "Run frozen synthetic suite twice; verify structure/work and cross-node repeatability before interpretation",
  "date": "2026-10-07T10:22:00.755575+00:00"
}
```

## pair-plan-20261007-01-submit-10889202

```json
{
  "id": "pair-plan-20261007-01-submit-10889202",
  "kind": "plan_submission",
  "status": "SUBMITTED",
  "job": "10889202",
  "mode": "synthetic",
  "study": "plan",
  "partition": "intel_expr",
  "exclusive": true,
  "requested_nodes_before": 2,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/launcher/retest-synth-ex-20261007-182200-848444",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T10:22:00.934560+00:00"
}
```

## pair-plan-20261007-01-submit-10889203

```json
{
  "id": "pair-plan-20261007-01-submit-10889203",
  "kind": "plan_submission",
  "status": "SUBMITTED",
  "job": "10889203",
  "mode": "synthetic",
  "study": "plan",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 3,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/launcher/retest-synth-sh-20261007-182201-032994",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T10:22:01.154344+00:00"
}
```

## pair-plan-analysis01-20261007

```json
{
  "id": "pair-plan-analysis01-20261007",
  "kind": "analysis",
  "status": "FAILED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/pair_plan_20261007/analysis-01",
  "paper_eligible": false,
  "issues": "Independent audit of available records; pending jobs retained and excluded. 2",
  "next": "Complete all planned jobs and final audit",
  "date": "2026-10-07T10:25:01.029652+00:00"
}
```

## pair-plan-20261007-01-plan-10889202

```json
{
  "id": "pair-plan-20261007-01-plan-10889202",
  "kind": "plan_run",
  "study": "plan",
  "mode": "synthetic",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/logs/plan-10889202",
  "completion": {
    "study": "pair_plan",
    "mode": "synthetic",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 276,
    "equiv": 264,
    "profile": 132,
    "work": 132
  },
  "job": "10889202",
  "node": "qhcn819",
  "partition": "intel_expr",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T10:26:10.288436+00:00"
}
```

## pair-plan-20261007-01-plan-10889203

```json
{
  "id": "pair-plan-20261007-01-plan-10889203",
  "kind": "plan_run",
  "study": "plan",
  "mode": "synthetic",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/logs/plan-10889203",
  "completion": {
    "study": "pair_plan",
    "mode": "synthetic",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 276,
    "equiv": 264,
    "profile": 132,
    "work": 132
  },
  "job": "10889203",
  "node": "qhcn041",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T10:26:11.701147+00:00"
}
```

## pair-plan-analysis02-relu-fix-20261007

```json
{
  "id": "pair-plan-analysis02-relu-fix-20261007",
  "kind": "analysis_correction",
  "status": "PARTIAL",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/pair_plan_20261007/analysis-02",
  "paper_eligible": false,
  "issues": "Previous independent analysis incorrectly assumed N*128 ReLU operations. Zero-degree-only tiles directly zero outputs and skip ReLU. Correct CSR-derived active tile rows formula with11 math regressions; all624 old counter mismatches differed only ReLU. Frozen kernels, binaries, numeric gates and raw results unchanged; failed analysis01 retained.",
  "next": "If no completed packet errors, finish remaining13 fixed validation graphs",
  "date": "2026-10-07T10:29:52.858357+00:00"
}
```

## pair-plan-20261007-01-submit-10889209

```json
{
  "id": "pair-plan-20261007-01-submit-10889209",
  "kind": "plan_submission",
  "status": "SUBMITTED",
  "job": "10889209",
  "mode": "ogbn-products,hollywood-2009,indochina-2004,soc-Pokec",
  "study": "plan",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 2,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/launcher/val-a-20261007-182952-960754",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T10:29:53.091961+00:00"
}
```

## pair-plan-20261007-01-submit-10889210

```json
{
  "id": "pair-plan-20261007-01-submit-10889210",
  "kind": "plan_submission",
  "status": "SUBMITTED",
  "job": "10889210",
  "mode": "cit-Patents,soc-LiveJournal1,com-LiveJournal,as-Skitter",
  "study": "plan",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 3,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/launcher/val-b-20261007-182953-194406",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T10:29:53.279134+00:00"
}
```

## pair-plan-20261007-01-submit-10889211

```json
{
  "id": "pair-plan-20261007-01-submit-10889211",
  "kind": "plan_submission",
  "status": "SUBMITTED",
  "job": "10889211",
  "mode": "rgg_n_2_24_s0",
  "study": "plan",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 4,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/launcher/val-c-20261007-182953-376599",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T10:29:53.480058+00:00"
}
```

## pair-plan-20261007-01-submit-10889212

```json
{
  "id": "pair-plan-20261007-01-submit-10889212",
  "kind": "plan_submission",
  "status": "SUBMITTED",
  "job": "10889212",
  "mode": "web-Google,wiki-Talk,email-Enron,com-Youtube",
  "study": "plan",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 5,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/launcher/val-d-20261007-182953-578364",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T10:29:53.671125+00:00"
}
```

## pair-plan-20261007-01-plan-10889212

```json
{
  "id": "pair-plan-20261007-01-plan-10889212",
  "kind": "plan_run",
  "study": "plan",
  "mode": "web-Google,wiki-Talk,email-Enron,com-Youtube",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/logs/plan-10889212",
  "completion": {
    "study": "pair_plan",
    "mode": "web-Google,wiki-Talk,email-Enron,com-Youtube",
    "graphs": 4,
    "passed": 4,
    "failed": 0,
    "checks": 208,
    "equiv": 192,
    "profile": 96,
    "work": 96
  },
  "job": "10889212",
  "node": "qhcn044",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T10:31:33.667804+00:00"
}
```

## pair-plan-20261007-01-plan-10889181

```json
{
  "id": "pair-plan-20261007-01-plan-10889181",
  "kind": "plan_run",
  "study": "plan",
  "mode": "mycielskian19",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/logs/plan-10889181",
  "completion": {
    "study": "pair_plan",
    "mode": "mycielskian19",
    "graphs": 1,
    "passed": 1,
    "failed": 0,
    "checks": 92,
    "equiv": 88,
    "profile": 44,
    "work": 44
  },
  "job": "10889181",
  "node": "qhcn818",
  "partition": "intel_expr",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T10:32:58.354103+00:00"
}
```

## pair-plan-20261007-01-plan-10889183

```json
{
  "id": "pair-plan-20261007-01-plan-10889183",
  "kind": "plan_run",
  "study": "plan",
  "mode": "mycielskian19",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/logs/plan-10889183",
  "completion": {
    "study": "pair_plan",
    "mode": "mycielskian19",
    "graphs": 1,
    "passed": 1,
    "failed": 0,
    "checks": 92,
    "equiv": 88,
    "profile": 44,
    "work": 44
  },
  "job": "10889183",
  "node": "qhcn001",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T10:33:24.126936+00:00"
}
```

## pair-order-control-protocol-20261007

```json
{
  "id": "pair-order-control-protocol-20261007",
  "kind": "exploratory_preregistration",
  "status": "PREPARED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/pair_order_control_20261007",
  "paper_eligible": false,
  "issues": "Same-storage and fixed-pair order controls isolate visit-order anomaly; fixed128/defaultNUMA/cap10; all existing primary results remain unchanged",
  "next": "Complete wrapper and harness, immutable build, smoke, then synthetic3 exclusive/shared",
  "date": "2026-10-07T10:36:09.906752+00:00"
}
```

## pair-plan-20261007-01-plan-10889210

```json
{
  "id": "pair-plan-20261007-01-plan-10889210",
  "kind": "plan_run",
  "study": "plan",
  "mode": "cit-Patents,soc-LiveJournal1,com-LiveJournal,as-Skitter",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/logs/plan-10889210",
  "completion": {
    "study": "pair_plan",
    "mode": "cit-Patents,soc-LiveJournal1,com-LiveJournal,as-Skitter",
    "graphs": 4,
    "passed": 4,
    "failed": 0,
    "checks": 208,
    "equiv": 192,
    "profile": 96,
    "work": 96
  },
  "job": "10889210",
  "node": "qhcn034",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T10:37:50.811947+00:00"
}
```

## pair-plan-20261007-01-plan-10889209

```json
{
  "id": "pair-plan-20261007-01-plan-10889209",
  "kind": "plan_run",
  "study": "plan",
  "mode": "ogbn-products,hollywood-2009,indochina-2004,soc-Pokec",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/logs/plan-10889209",
  "completion": {
    "study": "pair_plan",
    "mode": "ogbn-products,hollywood-2009,indochina-2004,soc-Pokec",
    "graphs": 4,
    "passed": 4,
    "failed": 0,
    "checks": 208,
    "equiv": 192,
    "profile": 96,
    "work": 96
  },
  "job": "10889209",
  "node": "qhcn027",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T10:38:44.836958+00:00"
}
```

## pair-plan-20261007-01-plan-10889211

```json
{
  "id": "pair-plan-20261007-01-plan-10889211",
  "kind": "plan_run",
  "study": "plan",
  "mode": "rgg_n_2_24_s0",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-plan-20261007-01/logs/plan-10889211",
  "completion": {
    "study": "pair_plan",
    "mode": "rgg_n_2_24_s0",
    "graphs": 1,
    "passed": 1,
    "failed": 0,
    "checks": 52,
    "equiv": 48,
    "profile": 24,
    "work": 24
  },
  "job": "10889211",
  "node": "qhcn041",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T10:41:31.590089+00:00"
}
```

## pair-order-control-20261007-01-build

```json
{
  "id": "pair-order-control-20261007-01-build",
  "kind": "order_control_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-order-control-20261007-01",
  "paper_eligible": false,
  "issues": "Isolated immutable build; 128x128x128; default NUMA; no new major issue if PASS",
  "next": "Smoke all gates before performance",
  "date": "2026-10-07T10:44:12.739382+00:00"
}
```

## pair-order-control-20261007-01-submit-10889221

```json
{
  "id": "pair-order-control-20261007-01-submit-10889221",
  "kind": "order_control_submission",
  "status": "SUBMITTED",
  "job": "10889221",
  "mode": "smoke",
  "study": "control",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 0,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-order-control-20261007-01/launcher/smoke-20261007-184437-794988",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T10:44:37.863165+00:00"
}
```

## pair-order-control-20261007-01-control-10889221

```json
{
  "id": "pair-order-control-20261007-01-control-10889221",
  "kind": "order_control_run",
  "study": "control",
  "mode": "smoke",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-order-control-20261007-01/logs/control-10889221",
  "completion": {
    "study": "pair_order_control",
    "mode": "smoke",
    "graphs": 13,
    "passed": 13,
    "failed": 0,
    "checks": 260,
    "equiv": 260,
    "profile": 130,
    "work": 130,
    "slot_checks": 65
  },
  "job": "10889221",
  "node": "qhcn001",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T10:44:42.563824+00:00"
}
```

## pair-order-control-20261007-01-submit-10889222

```json
{
  "id": "pair-order-control-20261007-01-submit-10889222",
  "kind": "order_control_submission",
  "status": "SUBMITTED",
  "job": "10889222",
  "mode": "synthetic",
  "study": "control",
  "partition": "intel_expr",
  "exclusive": true,
  "requested_nodes_before": 0,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-order-control-20261007-01/launcher/synthetic-exclusive-20261007-184506-039236",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T10:45:06.124802+00:00"
}
```

## pair-order-control-20261007-01-submit-10889223

```json
{
  "id": "pair-order-control-20261007-01-submit-10889223",
  "kind": "order_control_submission",
  "status": "SUBMITTED",
  "job": "10889223",
  "mode": "synthetic",
  "study": "control",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 1,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-order-control-20261007-01/launcher/synthetic-shared-20261007-184506-221168",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T10:45:06.319116+00:00"
}
```

## pair-plan-analysis03-final-20261007

```json
{
  "id": "pair-plan-analysis03-final-20261007",
  "kind": "pair_plan_independent_final_analysis",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/pair_plan_20261007/analysis-03",
  "preflight": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/pair_plan_20261007/analysis-03-preflight",
  "source_sha256": "bcac6ddc97042cbf4d3643ae7fa846a2f716ee1fc4c05d6b238269874cf90fd9",
  "statistics_source_sha256": "d6d9438c12e90abe34a6060f9ad644398ad1aa5b7bc4251f657e841df3623d13",
  "jobs": 12,
  "graph_job_packets": 43,
  "performance_summary_records": 1180,
  "raw_timing_records": 8850,
  "dimensions": [
    128,
    128,
    128
  ],
  "numa_interleave": false,
  "correctness": {
    "METHOD_CHECK": 8116,
    "PAIRPLAN_EQUIV": 12624,
    "PAIRPLAN_WORK_GATE": 3972,
    "PAIRPLAN_PROFILE_GATE": 3972,
    "PAIRPLAN_SEMANTIC": 43,
    "PLAN_GATE": 159,
    "PAIRPLAN_TIME": 8850,
    "METHOD_PMU": 204,
    "METHOD_PMU_THREAD": 6528
  },
  "pmu": {
    "records": 204,
    "usable": 204,
    "unusable": 0
  },
  "coverage": {
    "shared_real_primary17": 17,
    "exclusive_real_primary_diag4": 4,
    "smoke": 13,
    "synthetic_primary": 3,
    "synthetic_retests_separate": 6
  },
  "statistics": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/pair_plan_20261007/final_statistics.json",
  "notes": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/pair_plan_20261007/final_notes_zh.txt",
  "paper_eligible": false,
  "issues": "No new major issue. analysis-01 failed remains preserved: only analyzer ReLU work formula was incorrect for fully empty tiles. Corrected formula independently validated by 11 mathematical regressions and all field-level work gates. No kernel, binary or raw data changed.",
  "next": "Report main round separately from order_control supplement; retain negative gains and per-job synthetic retests. User-level PMU does not establish AVX-AMX overlap.",
  "date": "2026-10-07T10:45:49.515981+00:00"
}
```

## pair-order-control-20261007-01-control-10889222

```json
{
  "id": "pair-order-control-20261007-01-control-10889222",
  "kind": "order_control_run",
  "study": "control",
  "mode": "synthetic",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-order-control-20261007-01/logs/control-10889222",
  "completion": {
    "study": "pair_order_control",
    "mode": "synthetic",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 60,
    "equiv": 60,
    "profile": 30,
    "work": 30,
    "slot_checks": 225
  },
  "job": "10889222",
  "node": "qhcn817",
  "partition": "intel_expr",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T10:46:20.657495+00:00"
}
```

## pair-order-control-20261007-01-control-10889223

```json
{
  "id": "pair-order-control-20261007-01-control-10889223",
  "kind": "order_control_run",
  "study": "control",
  "mode": "synthetic",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/pair-order-control-20261007-01/logs/control-10889223",
  "completion": {
    "study": "pair_order_control",
    "mode": "synthetic",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 60,
    "equiv": 60,
    "profile": 30,
    "work": 30,
    "slot_checks": 225
  },
  "job": "10889223",
  "node": "qhcn001",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T10:46:22.135679+00:00"
}
```

## pair-order-control-independent-analysis01-20261007

```json
{
  "id": "pair-order-control-independent-analysis01-20261007",
  "kind": "independent_control_analysis",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/pair_order_control_20261007/analysis-01",
  "counts": {
    "jobs": 3,
    "graph_packets": 19,
    "numerical": 380,
    "bitwise": 380,
    "work": 190,
    "profile": 190,
    "slot_checks": 515,
    "timing_samples": 450,
    "time_summary_rows": 60,
    "PMU_regions": 90,
    "PMU_threads": 2880,
    "PMU_regions_usable": 90
  },
  "jobs": [
    "10889221",
    "10889222",
    "10889223"
  ],
  "paper_eligible": false,
  "issues": "19/19 packets; same storage and execution object; same MODE0 work, bitwise gates and PMU passed; synthetic-only order effect, no generalized hardware mechanism claim",
  "next": "Archive technical report and deliver desktop TXT; all jobs complete",
  "date": "2026-10-07T10:54:15.062874+00:00"
}
```

## pair-plan-and-order-control-final-report-20261007

```json
{
  "id": "pair-plan-and-order-control-final-report-20261007",
  "kind": "final_research_delivery",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/pair_plan_20261007/final",
  "report": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/pair_plan_20261007/final/TECHNICAL_REPORT_20261007.txt",
  "report_sha256": "df4ba16e8388007d89eaf0502bffb77b98bc00c3d5ee9bf67ea29d7360c9950f",
  "desktop_report": "C:/Users/花喵/Desktop/TFS_分组配对与访问顺序实验报告_20261007.txt",
  "dimensions": [
    128,
    128,
    128
  ],
  "node_cap": 10,
  "observed_peak_nodes": 6,
  "paper_eligible": false,
  "issues": "All15jobs/62packets passed and nodes released. Exact sharing17graph alternating1.0132x but consecutive0.9996x; no robust generalized increment. Same-slot pair-order synthetic controls2.324x/2.483x, not a true-graph or hardware-cause claim. No interleave/first-touch changes; old logs preserved; no GitHub push.",
  "next": "Delivered desktop technical TXT; next candidate is fixed pairing order-by-sharing factorial and independent hardware-cause controls, not yet executed",
  "date": "2026-10-07T10:57:29.508636+00:00"
}
```

## order-share-factorial-protocol-20261007

```json
{
  "id": "order-share-factorial-protocol-20261007",
  "kind": "preregistered_followup",
  "status": "PLANNED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/order_share_factorial_20261007",
  "dimensions": [
    128,
    128,
    128
  ],
  "node_cap": 10,
  "seeds": [
    20261007,
    20261017,
    20261027
  ],
  "paper_eligible": false,
  "issues": "Authorized continued inference study; same pair relations x order x exact sharing, 2new fixed holdoutseeds; no interleave/firsttouch/prefetch change",
  "next": "Freeze complete sources then smoke,3synthetic+4realdiagnostic shared/exclusive; no full17 unless registered gates met",
  "date": "2026-10-07T11:29:47.269887+00:00"
}
```

## order-share-factorial-20261007-01-build

```json
{
  "id": "order-share-factorial-20261007-01-build",
  "kind": "factorial_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-share-factorial-20261007-01",
  "paper_eligible": false,
  "issues": "Isolated immutable build; 128x128x128; default NUMA; no new major issue if PASS",
  "next": "Smoke all gates before performance",
  "date": "2026-10-07T11:35:53.301036+00:00"
}
```

## order-share-factorial-20261007-01-submit-10889444

```json
{
  "id": "order-share-factorial-20261007-01-submit-10889444",
  "kind": "factorial_submission",
  "status": "SUBMITTED",
  "job": "10889444",
  "mode": "smoke",
  "study": "factorial",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 0,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-share-factorial-20261007-01/launcher/smoke-20261007-193617-844124",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T11:36:17.913251+00:00"
}
```

## order-share-factorial-20261007-01-factorial-10889444

```json
{
  "id": "order-share-factorial-20261007-01-factorial-10889444",
  "kind": "factorial_run",
  "study": "factorial",
  "mode": "smoke",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-share-factorial-20261007-01/logs/factorial-10889444",
  "completion": {
    "study": "order_share_factorial",
    "mode": "smoke",
    "graphs": 13,
    "passed": 13,
    "failed": 0,
    "checks": 676,
    "equiv": 676,
    "exec_equiv": 676,
    "profile": 338,
    "work": 338,
    "slot_checks": 169,
    "pmu_regions": 0
  },
  "job": "10889444",
  "node": "qhcn041",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T11:36:24.341170+00:00"
}
```

## order-share-factorial-20261007-01-submit-10889452

```json
{
  "id": "order-share-factorial-20261007-01-submit-10889452",
  "kind": "factorial_submission",
  "status": "SUBMITTED",
  "job": "10889452",
  "mode": "synthetic",
  "study": "factorial",
  "partition": "intel_expr",
  "exclusive": true,
  "requested_nodes_before": 0,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-share-factorial-20261007-01/launcher/syn-ex-20261007-193705-236295",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T11:37:05.317394+00:00"
}
```

## order-share-factorial-20261007-01-submit-10889453

```json
{
  "id": "order-share-factorial-20261007-01-submit-10889453",
  "kind": "factorial_submission",
  "status": "SUBMITTED",
  "job": "10889453",
  "mode": "mycielskian19",
  "study": "factorial",
  "partition": "intel_expr",
  "exclusive": true,
  "requested_nodes_before": 1,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-share-factorial-20261007-01/launcher/myc-ex-20261007-193705-416145",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T11:37:05.517538+00:00"
}
```

## order-share-factorial-20261007-01-submit-10889454

```json
{
  "id": "order-share-factorial-20261007-01-submit-10889454",
  "kind": "factorial_submission",
  "status": "SUBMITTED",
  "job": "10889454",
  "mode": "reddit,roadNet-CA,amazon0601",
  "study": "factorial",
  "partition": "intel_expr",
  "exclusive": true,
  "requested_nodes_before": 2,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-share-factorial-20261007-01/launcher/diag-ex-20261007-193705-604505",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T11:37:05.670340+00:00"
}
```

## order-share-factorial-20261007-01-submit-10889455

```json
{
  "id": "order-share-factorial-20261007-01-submit-10889455",
  "kind": "factorial_submission",
  "status": "SUBMITTED",
  "job": "10889455",
  "mode": "synthetic",
  "study": "factorial",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 3,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-share-factorial-20261007-01/launcher/syn-sh-20261007-193705-764405",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T11:37:05.881121+00:00"
}
```

## order-share-factorial-20261007-01-submit-10889456

```json
{
  "id": "order-share-factorial-20261007-01-submit-10889456",
  "kind": "factorial_submission",
  "status": "SUBMITTED",
  "job": "10889456",
  "mode": "mycielskian19",
  "study": "factorial",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 4,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-share-factorial-20261007-01/launcher/myc-sh-20261007-193705-975730",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T11:37:06.037703+00:00"
}
```

## order-share-factorial-20261007-01-submit-10889457

```json
{
  "id": "order-share-factorial-20261007-01-submit-10889457",
  "kind": "factorial_submission",
  "status": "SUBMITTED",
  "job": "10889457",
  "mode": "reddit,roadNet-CA,amazon0601",
  "study": "factorial",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 5,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-share-factorial-20261007-01/launcher/diag-sh-20261007-193706-135024",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-07T11:37:06.223679+00:00"
}
```

## order-share-factorial-20261007-01-factorial-10889457

```json
{
  "id": "order-share-factorial-20261007-01-factorial-10889457",
  "kind": "factorial_run",
  "study": "factorial",
  "mode": "reddit,roadNet-CA,amazon0601",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-share-factorial-20261007-01/logs/factorial-10889457",
  "completion": {
    "study": "order_share_factorial",
    "mode": "reddit,roadNet-CA,amazon0601",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 156,
    "equiv": 156,
    "exec_equiv": 156,
    "profile": 78,
    "work": 78,
    "slot_checks": 585,
    "pmu_regions": 117
  },
  "job": "10889457",
  "node": "qhcn113",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T11:39:49.499622+00:00"
}
```

## order-share-factorial-20261007-01-factorial-10889452

```json
{
  "id": "order-share-factorial-20261007-01-factorial-10889452",
  "kind": "factorial_run",
  "study": "factorial",
  "mode": "synthetic",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-share-factorial-20261007-01/logs/factorial-10889452",
  "completion": {
    "study": "order_share_factorial",
    "mode": "synthetic",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 156,
    "equiv": 156,
    "exec_equiv": 156,
    "profile": 78,
    "work": 78,
    "slot_checks": 585,
    "pmu_regions": 117
  },
  "job": "10889452",
  "node": "qhcn817",
  "partition": "intel_expr",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T11:40:00.576482+00:00"
}
```

## order-share-factorial-20261007-01-factorial-10889455

```json
{
  "id": "order-share-factorial-20261007-01-factorial-10889455",
  "kind": "factorial_run",
  "study": "factorial",
  "mode": "synthetic",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-share-factorial-20261007-01/logs/factorial-10889455",
  "completion": {
    "study": "order_share_factorial",
    "mode": "synthetic",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 156,
    "equiv": 156,
    "exec_equiv": 156,
    "profile": 78,
    "work": 78,
    "slot_checks": 585,
    "pmu_regions": 117
  },
  "job": "10889455",
  "node": "qhcn047",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T11:40:02.123137+00:00"
}
```

## order-share-factorial-20261007-01-factorial-10889454

```json
{
  "id": "order-share-factorial-20261007-01-factorial-10889454",
  "kind": "factorial_run",
  "study": "factorial",
  "mode": "reddit,roadNet-CA,amazon0601",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-share-factorial-20261007-01/logs/factorial-10889454",
  "completion": {
    "study": "order_share_factorial",
    "mode": "reddit,roadNet-CA,amazon0601",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 156,
    "equiv": 156,
    "exec_equiv": 156,
    "profile": 78,
    "work": 78,
    "slot_checks": 585,
    "pmu_regions": 117
  },
  "job": "10889454",
  "node": "qhcn817",
  "partition": "intel_expr",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T11:42:46.526791+00:00"
}
```

## order-share-factorial-20261007-01-factorial-10889453

```json
{
  "id": "order-share-factorial-20261007-01-factorial-10889453",
  "kind": "factorial_run",
  "study": "factorial",
  "mode": "mycielskian19",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-share-factorial-20261007-01/logs/factorial-10889453",
  "completion": {
    "study": "order_share_factorial",
    "mode": "mycielskian19",
    "graphs": 1,
    "passed": 1,
    "failed": 0,
    "checks": 52,
    "equiv": 52,
    "exec_equiv": 52,
    "profile": 26,
    "work": 26,
    "slot_checks": 195,
    "pmu_regions": 39
  },
  "job": "10889453",
  "node": "qhcn818",
  "partition": "intel_expr",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T11:50:26.501143+00:00"
}
```

## order-share-factorial-20261007-01-factorial-10889456

```json
{
  "id": "order-share-factorial-20261007-01-factorial-10889456",
  "kind": "factorial_run",
  "study": "factorial",
  "mode": "mycielskian19",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-share-factorial-20261007-01/logs/factorial-10889456",
  "completion": {
    "study": "order_share_factorial",
    "mode": "mycielskian19",
    "graphs": 1,
    "passed": 1,
    "failed": 0,
    "checks": 52,
    "equiv": 52,
    "exec_equiv": 52,
    "profile": 26,
    "work": 26,
    "slot_checks": 195,
    "pmu_regions": 39
  },
  "job": "10889456",
  "node": "qhcn056",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-07T11:50:41.385984+00:00"
}
```

## order-share-factorial-20261007-01-analysis-01-independent

```json
{
  "id": "order-share-factorial-20261007-01-analysis-01-independent",
  "kind": "factorial_independent_analysis",
  "status": "PASS",
  "date": "2026-10-07T11:54:55.763308+00:00",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/order_share_factorial_20261007/analysis-01",
  "run": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/order-share-factorial-20261007-01",
  "jobs": [
    "10889444",
    "10889452",
    "10889453",
    "10889454",
    "10889455",
    "10889456",
    "10889457"
  ],
  "packets": 27,
  "counts": {
    "numerical": 1404,
    "canonical_bitwise": 1404,
    "execution_bitwise": 1404,
    "work": 702,
    "profile": 702,
    "slots": 2899,
    "timings": 2730,
    "pmu_regions": 546,
    "pmu_threads": 17472
  },
  "pmu_usable": 546,
  "summary_rows": 364,
  "interaction_rows": 84,
  "findings": {
    "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/order_share_factorial_20261007/analysis-01/factorial_findings.json",
    "sha256": "535726c48dbc2e92036bf300518847093ef2989da66d5ec8eeef37a140955b65"
  },
  "assembly_crosscheck": {
    "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/order_share_factorial_20261007/assembly_crosscheck.json",
    "sha256": "eabacb567259e6b6d7a4eac7624af8110a5b811b624b531f9c43dca08567958d"
  },
  "analyzer_sha256": "c4351fedf2c351fa5546374c4cd0bf144f87430b0d5e63e9abbfc855a2c0dc19",
  "full17_expansion_eligible": false,
  "paper_eligible": false,
  "issues": "All source/numerical/bitwise/work/slot/PMU gates pass. Fixed s1 ExactS expansion candidate fails all four diagnostic4 cohort filters; retain all three seeds and both timing protocols. Shared reuse4 s3 ExactI consecutive timing variability retained, not excluded. No NUMA interleave; no AVX-AMX overlap or DRAM-reduction claim.",
  "next": "Archive diagnostic report; do not expand full17 under this protocol; investigate broader reusable-source opportunities only under a new protocol."
}
```

## order-share-factorial-final-report-20261007

```json
{
  "id": "order-share-factorial-final-report-20261007",
  "kind": "final_research_delivery",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/order_share_factorial_20261007/final",
  "report": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/order_share_factorial_20261007/final/TECHNICAL_REPORT_20261007.txt",
  "report_sha256": "834b42f298c8f99aa8531f9828434021f88874ce89d0b06b7ddd7fc6a88fb311",
  "desktop_report": "C:/Users/花喵/Desktop/TFS_访问顺序与共享交互实验报告_20261007.txt",
  "secondary_review": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/order_share_factorial_20261007/final/secondary_findings.json",
  "paper_eligible": false,
  "issues": "7jobs27packetsPASS; 3fixedseeds. Positive order-sharing interaction on syntheticreuse4; fixeds1 benchmarkdiag4GM0.9982-1.0032x, registered full17 expansion gate failed. All negative/outlier data retained; same128/nointerleave/nofirsttouch orprefetch change. Dataset wording clarified: Myc is constructed; old data_class=real is a cohort label. All nodes released.",
  "next": "Desktop technical TXT delivered. Next research is targeted access-timing mechanism controls and realizable sharing/cost boundaries, not yet executed; no generalized method or overlap claim.",
  "date": "2026-10-07T12:00:08.555744+00:00"
}
```

## access-prefetch-protocol-20261008

```json
{
  "id": "access-prefetch-protocol-20261008",
  "kind": "new_controlled_protocol",
  "status": "READY_FOR_IMPLEMENTATION",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/access_prefetch_20261008",
  "paper_eligible": false,
  "dimensions": [
    128,
    128,
    128
  ],
  "node_cap": 10,
  "numa_interleave": false,
  "issues": "Execution tools recovered; prior outage produced no jobs. Prospective13-method fixed-target prefetch factorial. No new numerical or timing result. Source tree lacks .git; frozen hashes authoritative.",
  "next": "Implement isolated PF0/PF1/PF8MATCH kernels/runtime; freeze build then smoke all gates.",
  "date": "2026-10-08T03:26:11.571553+00:00"
}
```

## access-prefetch-20261008-01-build

```json
{
  "id": "access-prefetch-20261008-01-build",
  "kind": "prefetch_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01",
  "paper_eligible": false,
  "issues": "Isolated immutable build; 128x128x128; default NUMA; no new major issue if PASS",
  "next": "Smoke all gates before performance",
  "date": "2026-10-08T03:31:55.672852+00:00"
}
```

## access-prefetch-20261008-01-submit-10895466

```json
{
  "id": "access-prefetch-20261008-01-submit-10895466",
  "kind": "prefetch_submission",
  "status": "SUBMITTED",
  "job": "10895466",
  "mode": "smoke",
  "study": "prefetch",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 0,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/launcher/smoke-20261008-113244-173729",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-08T03:32:44.324938+00:00"
}
```

## access-prefetch-20261008-01-prefetch-10895466

```json
{
  "id": "access-prefetch-20261008-01-prefetch-10895466",
  "kind": "prefetch_run",
  "study": "prefetch",
  "mode": "smoke",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/logs/prefetch-10895466",
  "completion": {
    "study": "access_prefetch",
    "mode": "smoke",
    "graphs": 13,
    "passed": 13,
    "failed": 0,
    "checks": 676,
    "equiv": 676,
    "exec_equiv": 676,
    "profile": 338,
    "work": 338,
    "slot_checks": 169,
    "pmu_regions": 0,
    "pf_identity": 208,
    "pf_identity_profile": 104,
    "pf_target_cases": 468
  },
  "job": "10895466",
  "node": "qhcn047",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-08T03:32:55.318556+00:00"
}
```

## access-prefetch-20261008-01-submit-10895499

```json
{
  "id": "access-prefetch-20261008-01-submit-10895499",
  "kind": "prefetch_submission",
  "status": "SUBMITTED",
  "job": "10895499",
  "mode": "synthetic",
  "study": "prefetch",
  "partition": "intel_expr",
  "exclusive": true,
  "requested_nodes_before": 0,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/launcher/synth-ex-20261008-113522-685169",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-08T03:35:22.786119+00:00"
}
```

## access-prefetch-20261008-01-submit-10895500

```json
{
  "id": "access-prefetch-20261008-01-submit-10895500",
  "kind": "prefetch_submission",
  "status": "SUBMITTED",
  "job": "10895500",
  "mode": "mycielskian19",
  "study": "prefetch",
  "partition": "intel_expr",
  "exclusive": true,
  "requested_nodes_before": 1,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/launcher/myc-ex-20261008-113522-890878",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-08T03:35:23.007701+00:00"
}
```

## access-prefetch-20261008-01-submit-10895501

```json
{
  "id": "access-prefetch-20261008-01-submit-10895501",
  "kind": "prefetch_submission",
  "status": "SUBMITTED",
  "job": "10895501",
  "mode": "reddit,roadNet-CA,amazon0601",
  "study": "prefetch",
  "partition": "intel_expr",
  "exclusive": true,
  "requested_nodes_before": 2,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/launcher/other3-ex-20261008-113523-102777",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-08T03:35:23.240058+00:00"
}
```

## access-prefetch-20261008-01-submit-10895502

```json
{
  "id": "access-prefetch-20261008-01-submit-10895502",
  "kind": "prefetch_submission",
  "status": "SUBMITTED",
  "job": "10895502",
  "mode": "synthetic",
  "study": "prefetch",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 3,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/launcher/synth-sh-20261008-113523-338769",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-08T03:35:23.419320+00:00"
}
```

## access-prefetch-20261008-01-submit-10895503

```json
{
  "id": "access-prefetch-20261008-01-submit-10895503",
  "kind": "prefetch_submission",
  "status": "SUBMITTED",
  "job": "10895503",
  "mode": "mycielskian19",
  "study": "prefetch",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 4,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/launcher/myc-sh-20261008-113523-515750",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-08T03:35:23.653612+00:00"
}
```

## access-prefetch-20261008-01-submit-10895504

```json
{
  "id": "access-prefetch-20261008-01-submit-10895504",
  "kind": "prefetch_submission",
  "status": "SUBMITTED",
  "job": "10895504",
  "mode": "reddit,roadNet-CA,amazon0601",
  "study": "prefetch",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 5,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/launcher/other3-sh-20261008-113523-746173",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-08T03:35:23.852925+00:00"
}
```

## access-prefetch-20261008-01-prefetch-10895504

```json
{
  "id": "access-prefetch-20261008-01-prefetch-10895504",
  "kind": "prefetch_run",
  "study": "prefetch",
  "mode": "reddit,roadNet-CA,amazon0601",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/logs/prefetch-10895504",
  "completion": {
    "study": "access_prefetch",
    "mode": "reddit,roadNet-CA,amazon0601",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 156,
    "equiv": 156,
    "exec_equiv": 156,
    "profile": 78,
    "work": 78,
    "slot_checks": 585,
    "pmu_regions": 117,
    "pf_identity": 48,
    "pf_identity_profile": 24,
    "pf_target_cases": 108
  },
  "job": "10895504",
  "node": "qhcn021",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-08T03:38:24.073263+00:00"
}
```

## access-prefetch-20261008-01-prefetch-10895502

```json
{
  "id": "access-prefetch-20261008-01-prefetch-10895502",
  "kind": "prefetch_run",
  "study": "prefetch",
  "mode": "synthetic",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/logs/prefetch-10895502",
  "completion": {
    "study": "access_prefetch",
    "mode": "synthetic",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 156,
    "equiv": 156,
    "exec_equiv": 156,
    "profile": 78,
    "work": 78,
    "slot_checks": 585,
    "pmu_regions": 117,
    "pf_identity": 48,
    "pf_identity_profile": 24,
    "pf_target_cases": 108
  },
  "job": "10895502",
  "node": "qhcn047",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-08T03:38:49.100390+00:00"
}
```

## access-prefetch-source-review-20261008

```json
{
  "id": "access-prefetch-source-review-20261008",
  "kind": "implementation_and_paper_review",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/access_prefetch_20261008",
  "assembly_identity": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/access_prefetch_20261008/assembly_identity.json",
  "assembly_sha256": "dbfdec69fc8ff90c343ba94802034e35072085d48cf8f2fd21679a94253df372",
  "paper_alignment": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/access_prefetch_20261008/PAPER_ALIGNMENT_20261008.txt",
  "paper_alignment_sha256": "1ca42466a3de0f0564321440fb003b263caf504e200e3a878d08d6ed0db05580",
  "source_pdf_sha256": "cdbd2396e9cacdfad34b9e811af131d3c5a5c2636d13f55a4855a15dd2522a48",
  "paper_eligible": false,
  "issues": "PF1 full normalized MODE0/4 reducers match frozen implementation;13smoke packets pass. Real kernel numeric/performance scopeFULL only; M64/M256 are target visitor/static audits. Local supplied anonymous TFS PDF now directly inspected. Performance jobs pending/running; no final timing claim.",
  "next": "Complete shared/exclusive27packets; strict raw audit and fixed candidate gate.",
  "date": "2026-10-08T03:41:12.550907+00:00"
}
```

## access-prefetch-interim-20261008

```json
{
  "id": "access-prefetch-interim-20261008",
  "kind": "partial_research_record",
  "status": "PARTIAL",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/access_prefetch_20261008/interim-01",
  "report": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/access_prefetch_20261008/interim-01/TECHNICAL_PROGRESS_20261008.txt",
  "report_sha256": "84ece1f629399fc05d6f9a750535e33f30dbdac5195296f6d7ab2bf8f76c8c10",
  "paper_eligible": false,
  "issues": "Shared6graphs156summary independently checked. Shared Myc and allexclusive not complete in interim snapshot. Synthetic order/PF/share interactions; do not infer cache/MLP/DRAM/AVX-AMX overlap. Preserved reuse16 shared consecutive timing outlier. No full17 expansion. Desktop UTF8 interim TXT saved.",
  "next": "Keep exclusive jobs queued normally; wait full27packets then final strict audit/report; do not call partial record complete.",
  "date": "2026-10-08T03:47:39.505335+00:00"
}
```

## access-prefetch-pending-limits-20261008

```json
{
  "id": "access-prefetch-pending-limits-20261008",
  "kind": "scheduler_reservation_refinement",
  "status": "PASS",
  "changes": [
    {
      "job": "10895499",
      "old_limit": "01:30:00",
      "new_limit": "00:15:00"
    },
    {
      "job": "10895500",
      "old_limit": "01:30:00",
      "new_limit": "00:45:00"
    },
    {
      "job": "10895501",
      "old_limit": "01:30:00",
      "new_limit": "00:15:00"
    }
  ],
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/launcher/timelimit-refinement",
  "paper_eligible": false,
  "issues": "Completed shared synthetic3:26/other3:01 support shorter generous limits. Only own pendingjobs updated; no priority,QOS,partition,nodecount,threads,affinity,kernels or timing repetitions changed. Frozen launcher retains original submission; before/after scontrol evidence saved.",
  "next": "Wait for normal scheduling; preserve any timeout instead of deleting/replacing evidence.",
  "date": "2026-10-08T03:48:56.183612+00:00"
}
```

## access-prefetch-20261008-01-prefetch-10895503

```json
{
  "id": "access-prefetch-20261008-01-prefetch-10895503",
  "kind": "prefetch_run",
  "study": "prefetch",
  "mode": "mycielskian19",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/logs/prefetch-10895503",
  "completion": {
    "study": "access_prefetch",
    "mode": "mycielskian19",
    "graphs": 1,
    "passed": 1,
    "failed": 0,
    "checks": 52,
    "equiv": 52,
    "exec_equiv": 52,
    "profile": 26,
    "work": 26,
    "slot_checks": 195,
    "pmu_regions": 39,
    "pf_identity": 16,
    "pf_identity_profile": 8,
    "pf_target_cases": 36
  },
  "job": "10895503",
  "node": "qhcn021",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-08T03:49:27.605794+00:00"
}
```

## access-prefetch-shared-interim-02-20261008

```json
{
  "id": "access-prefetch-shared-interim-02-20261008",
  "kind": "prefetch_partial_shared_independent_audit",
  "status": "PASS",
  "stage": "PARTIAL_SHARED_ONLY",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/access_prefetch_20261008/interim-02",
  "jobs": [
    "10895466",
    "10895502",
    "10895503",
    "10895504"
  ],
  "dimensions": [
    128,
    128,
    128
  ],
  "numa_interleave": false,
  "first_touch_changed": false,
  "paper_eligible": false,
  "counts": {
    "numerical": 1040,
    "canonical_bitwise": 1040,
    "execution_bitwise": 1040,
    "work": 520,
    "profile": 520,
    "slots": 1534,
    "timings": 1365,
    "pmu_regions": 273,
    "pmu_threads": 8736,
    "pf1_identity": 320,
    "pf1_profile_identity": 160,
    "prefetch_target_cases": 720,
    "prefetch_target_audits": 20
  },
  "shared_candidate_gate": [
    {
      "partition": "intel",
      "exclusive": "false",
      "protocol": "same_slot_consecutive",
      "graphs": 4,
      "geometric_mean": 1.0132383982097102,
      "wins": 2,
      "required_geometric_mean": 1.03,
      "required_wins": 3,
      "pass_filter": false,
      "ratios": [
        {
          "graph": "mycielskian19",
          "job": "10895503",
          "speedup": 1.001644956820648
        },
        {
          "graph": "reddit",
          "job": "10895504",
          "speedup": 0.9870559020026727
        },
        {
          "graph": "roadNet-CA",
          "job": "10895504",
          "speedup": 0.971427448125989
        },
        {
          "graph": "amazon0601",
          "job": "10895504",
          "speedup": 1.0974396065378598
        }
      ]
    },
    {
      "partition": "intel",
      "exclusive": "false",
      "protocol": "same_slot_alternating",
      "graphs": 4,
      "geometric_mean": 1.0153432554622304,
      "wins": 2,
      "required_geometric_mean": 1.03,
      "required_wins": 3,
      "pass_filter": false,
      "ratios": [
        {
          "graph": "mycielskian19",
          "job": "10895503",
          "speedup": 1.0291320783914713
        },
        {
          "graph": "reddit",
          "job": "10895504",
          "speedup": 0.9874249677115483
        },
        {
          "graph": "roadNet-CA",
          "job": "10895504",
          "speedup": 0.9874362501412696
        },
        {
          "graph": "amazon0601",
          "job": "10895504",
          "speedup": 1.0591738905795105
        }
      ]
    }
  ],
  "eligible_to_expand_full17": false,
  "output_sha256": {
    "shared_audit.json": "80fb2a8725cace643440f27ba379fb88c7b60b82527d9bfe7d13e8677fc3dd6f",
    "selected_effects.csv": "74cdb12ac133694a639f47b355813f66fcfa7b0982f8b411d0d97b7ec8a46318",
    "findings.json": "9e5485046e09af9af79457fecc6c067dc153e36581f52e6977b409f34646559c",
    "summary.csv": "8883328362a58ef8b4ea05135d87f77348a781f13e01b8c80537e11e4d5a910e"
  },
  "analyzer_sha256": "865b853106bb6d818b31067b475ce04a06afb1bb427d2672f37745bd0530c4b4",
  "issues": "No new major issue if interim audit PASS. Exclusive evidence pending; this artifact is partial shared-only and cannot be presented as final acceptance.",
  "next": "Wait for the three original exclusive jobs, then run unchanged final 7-job/27-packet analyzer into new analysis-01.",
  "date": "2026-10-08T03:53:21.733374+00:00"
}
```

## access-prefetch-research-model-20261008

```json
{
  "id": "access-prefetch-research-model-20261008",
  "status": "THEORY_AND_SOURCE_REVIEW_ONLY",
  "files": [
    {
      "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/access_prefetch_20261008/WORK_MODEL_20261008.txt",
      "sha256": "206087e395b75b649d2da035af9466e80abd2921ce80de19375ec51b174a2e9f"
    },
    {
      "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/access_prefetch_20261008/NEXT_RESEARCH_NOTE_20261008.txt",
      "sha256": "c5d59d405a7ea049dfc6eec90a421a5dca0d75b185fef13ff17d4c21935f2fb2"
    },
    {
      "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/access_prefetch_20261008/PAPER_ALIGNMENT_20261008.txt",
      "sha256": "1ca42466a3de0f0564321440fb003b263caf504e200e3a878d08d6ed0db05580"
    },
    {
      "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/access_prefetch_20261008/TAIL_COST_MODEL_NOTE.txt",
      "sha256": "63430b6865523a02e464e99a5a0777a87424d93314bc3cc4cdb49666a3b21bde"
    }
  ],
  "conclusions": [
    "Current FP32 aggregation always 128E additions; M changes projections and local partial traffic, ExactS changes decode count.",
    "Restricted no-retention consumption model maps to SCS; per-row same-source runs require a separate event model. Classical tools, not claimed new algorithms.",
    "Intel manual already describes AVX-AMX interleaving/double buffering; resource independence cannot be assumed.",
    "Defer fixed-pair quad given low decode-work bound; compact event trace remains unimplemented mechanism probe."
  ],
  "experiment_state": "shared complete; exclusive 10895499/10895500/10895501 pending; frozen kernels unchanged; no added jobs",
  "date": "2026-10-08T04:20:02.128908+00:00"
}
```

## access-prefetch-20261008-01-prefetch-10895499

```json
{
  "id": "access-prefetch-20261008-01-prefetch-10895499",
  "kind": "prefetch_run",
  "study": "prefetch",
  "mode": "synthetic",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/logs/prefetch-10895499",
  "completion": {
    "study": "access_prefetch",
    "mode": "synthetic",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 156,
    "equiv": 156,
    "exec_equiv": 156,
    "profile": 78,
    "work": 78,
    "slot_checks": 585,
    "pmu_regions": 117,
    "pf_identity": 48,
    "pf_identity_profile": 24,
    "pf_target_cases": 108
  },
  "job": "10895499",
  "node": "qhcn817",
  "partition": "intel_expr",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-08T04:26:35.581684+00:00"
}
```

## access-prefetch-20261008-01-prefetch-10895501

```json
{
  "id": "access-prefetch-20261008-01-prefetch-10895501",
  "kind": "prefetch_run",
  "study": "prefetch",
  "mode": "reddit,roadNet-CA,amazon0601",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/logs/prefetch-10895501",
  "completion": {
    "study": "access_prefetch",
    "mode": "reddit,roadNet-CA,amazon0601",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 156,
    "equiv": 156,
    "exec_equiv": 156,
    "profile": 78,
    "work": 78,
    "slot_checks": 585,
    "pmu_regions": 117,
    "pf_identity": 48,
    "pf_identity_profile": 24,
    "pf_target_cases": 108
  },
  "job": "10895501",
  "node": "qhcn819",
  "partition": "intel_expr",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-08T04:34:08.659607+00:00"
}
```

## access-prefetch-20261008-01-prefetch-10895500

```json
{
  "id": "access-prefetch-20261008-01-prefetch-10895500",
  "kind": "prefetch_run",
  "study": "prefetch",
  "mode": "mycielskian19",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/logs/prefetch-10895500",
  "completion": {
    "study": "access_prefetch",
    "mode": "mycielskian19",
    "graphs": 1,
    "passed": 1,
    "failed": 0,
    "checks": 52,
    "equiv": 52,
    "exec_equiv": 52,
    "profile": 26,
    "work": 26,
    "slot_checks": 195,
    "pmu_regions": 39,
    "pf_identity": 16,
    "pf_identity_profile": 8,
    "pf_target_cases": 36
  },
  "job": "10895500",
  "node": "qhcn817",
  "partition": "intel_expr",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-08T04:40:51.029639+00:00"
}
```

## access-prefetch-final-analysis-20261008

```json
{
  "id": "access-prefetch-final-analysis-20261008",
  "kind": "prefetch_final_independent_audit",
  "status": "PASS",
  "exit_status": 0,
  "created_utc": "2026-10-08T04:42:30.451024+00:00",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/access_prefetch_20261008/analysis-01",
  "preflight": {
    "/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md": {
      "sha256": "55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7",
      "fully_read_before_final_analysis": true
    },
    "/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md": {
      "sha256": "641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca",
      "fully_read_before_final_analysis": true
    },
    "/home/huangjianqiang_group/hdacp1/data/wzh/TFS-Train/docs/spec/MATH_SPEC_V2.md": {
      "sha256": "1938d301e3fc913766744af166c0a41b1cb767d7d7419af9c098d3de955cf266",
      "fully_read_before_final_analysis": true
    },
    "/home/huangjianqiang_group/hdacp1/data/wzh/TFS-Train/docs/spec/KERNEL_LAYOUT_V2.md": {
      "sha256": "732e859128baa3cf87813df82cf7562d14358fc6e1cf17c32ec0dde5a743595f",
      "fully_read_before_final_analysis": true
    },
    "/home/huangjianqiang_group/hdacp1/data/wzh/TFS-Train/docs/spec/PYTORCH_EXTENSION_SPEC.md": {
      "sha256": "a3784dd66d4986bde33a40d523f761ee8956da9bbbe711f1707d13d8d2d82df2",
      "fully_read_before_final_analysis": true
    },
    "/home/huangjianqiang_group/hdacp1/data/wzh/TFS-Train/docs/spec/EXPERIMENT_SPEC_V2.md": {
      "sha256": "ecb8fe1c394d9ec47aae97d8f5b87c5b948963c7e8c8b25d70128483c7c0d10d",
      "fully_read_before_final_analysis": true
    },
    "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/IMPLEMENTATION_CONTRACT.md": {
      "sha256": "b6419e5106278106da6727c90f4cfa185cc70c587d70b4087c0bc286fd0dc244",
      "fully_read_before_final_analysis": true
    }
  },
  "pre_handoff": {
    "argv": [
      "python3",
      "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/scripts/record_event.py",
      "check"
    ],
    "returncode": 0,
    "stdout": "HANDOFF_SYNCHRONIZED\n",
    "stderr": ""
  },
  "analysis_command": {
    "argv": [
      "python3",
      "-B",
      "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/access_prefetch_20261008/analysis_inputs_01/analyze_prefetch.py",
      "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01",
      "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/access_prefetch_20261008/analysis-01"
    ],
    "returncode": 0,
    "stdout": "{\"analysis_pass\": true, \"errors\": [], \"invalid_packets\": [], \"counts\": {\"numerical\": 1404, \"canonical_bitwise\": 1404, \"execution_bitwise\": 1404, \"work\": 702, \"profile\": 702, \"slots\": 2899, \"timings\": 2730, \"pmu_regions\": 546, \"pmu_threads\": 17472, \"pf1_identity\": 432, \"pf1_profile_identity\": 216, \"prefetch_target_cases\": 972, \"prefetch_target_audits\": 27}, \"summary_rows\": 364, \"pmu_usable\": 546, \"expand_full17\": false, \"out\": \"/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/results/access_prefetch_20261008/analysis-01\"}\n",
    "stderr": ""
  },
  "analyzer_sha256": "865b853106bb6d818b31067b475ce04a06afb1bb427d2672f37745bd0530c4b4",
  "jobs": {
    "10895466": {
      "state": "COMPLETED",
      "exit_code": "0:0",
      "node": "qhcn047",
      "partition": "intel"
    },
    "10895499": {
      "state": "COMPLETED",
      "exit_code": "0:0",
      "node": "qhcn817",
      "partition": "intel_expr"
    },
    "10895500": {
      "state": "COMPLETED",
      "exit_code": "0:0",
      "node": "qhcn817",
      "partition": "intel_expr"
    },
    "10895501": {
      "state": "COMPLETED",
      "exit_code": "0:0",
      "node": "qhcn819",
      "partition": "intel_expr"
    },
    "10895502": {
      "state": "COMPLETED",
      "exit_code": "0:0",
      "node": "qhcn047",
      "partition": "intel"
    },
    "10895503": {
      "state": "COMPLETED",
      "exit_code": "0:0",
      "node": "qhcn021",
      "partition": "intel"
    },
    "10895504": {
      "state": "COMPLETED",
      "exit_code": "0:0",
      "node": "qhcn021",
      "partition": "intel"
    }
  },
  "job_completions": {
    "10895500": {
      "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/logs/prefetch-10895500/completion.json",
      "sha256": "fcc817a8d6c8981bd0219a1e7fd2b646443fae3b44a6c9ec89e3375f7bc39941",
      "value": {
        "study": "access_prefetch",
        "mode": "mycielskian19",
        "graphs": 1,
        "passed": 1,
        "failed": 0,
        "checks": 52,
        "equiv": 52,
        "exec_equiv": 52,
        "profile": 26,
        "work": 26,
        "slot_checks": 195,
        "pmu_regions": 39,
        "pf_identity": 16,
        "pf_identity_profile": 8,
        "pf_target_cases": 36
      }
    },
    "10895504": {
      "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/logs/prefetch-10895504/completion.json",
      "sha256": "78c4f89d3e8887ac8e62f2da1f219bbdb3c46d8b6c6b8fa48c4d1fb281f2a6cb",
      "value": {
        "study": "access_prefetch",
        "mode": "reddit,roadNet-CA,amazon0601",
        "graphs": 3,
        "passed": 3,
        "failed": 0,
        "checks": 156,
        "equiv": 156,
        "exec_equiv": 156,
        "profile": 78,
        "work": 78,
        "slot_checks": 585,
        "pmu_regions": 117,
        "pf_identity": 48,
        "pf_identity_profile": 24,
        "pf_target_cases": 108
      }
    },
    "10895503": {
      "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/logs/prefetch-10895503/completion.json",
      "sha256": "fcc817a8d6c8981bd0219a1e7fd2b646443fae3b44a6c9ec89e3375f7bc39941",
      "value": {
        "study": "access_prefetch",
        "mode": "mycielskian19",
        "graphs": 1,
        "passed": 1,
        "failed": 0,
        "checks": 52,
        "equiv": 52,
        "exec_equiv": 52,
        "profile": 26,
        "work": 26,
        "slot_checks": 195,
        "pmu_regions": 39,
        "pf_identity": 16,
        "pf_identity_profile": 8,
        "pf_target_cases": 36
      }
    },
    "10895501": {
      "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/logs/prefetch-10895501/completion.json",
      "sha256": "78c4f89d3e8887ac8e62f2da1f219bbdb3c46d8b6c6b8fa48c4d1fb281f2a6cb",
      "value": {
        "study": "access_prefetch",
        "mode": "reddit,roadNet-CA,amazon0601",
        "graphs": 3,
        "passed": 3,
        "failed": 0,
        "checks": 156,
        "equiv": 156,
        "exec_equiv": 156,
        "profile": 78,
        "work": 78,
        "slot_checks": 585,
        "pmu_regions": 117,
        "pf_identity": 48,
        "pf_identity_profile": 24,
        "pf_target_cases": 108
      }
    },
    "10895499": {
      "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/logs/prefetch-10895499/completion.json",
      "sha256": "5de555539ac98af1977687c00752869aeae66dc9f9e9243734325da68bfa683e",
      "value": {
        "study": "access_prefetch",
        "mode": "synthetic",
        "graphs": 3,
        "passed": 3,
        "failed": 0,
        "checks": 156,
        "equiv": 156,
        "exec_equiv": 156,
        "profile": 78,
        "work": 78,
        "slot_checks": 585,
        "pmu_regions": 117,
        "pf_identity": 48,
        "pf_identity_profile": 24,
        "pf_target_cases": 108
      }
    },
    "10895502": {
      "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/logs/prefetch-10895502/completion.json",
      "sha256": "5de555539ac98af1977687c00752869aeae66dc9f9e9243734325da68bfa683e",
      "value": {
        "study": "access_prefetch",
        "mode": "synthetic",
        "graphs": 3,
        "passed": 3,
        "failed": 0,
        "checks": 156,
        "equiv": 156,
        "exec_equiv": 156,
        "profile": 78,
        "work": 78,
        "slot_checks": 585,
        "pmu_regions": 117,
        "pf_identity": 48,
        "pf_identity_profile": 24,
        "pf_target_cases": 108
      }
    },
    "10895466": {
      "path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/access-prefetch-20261008-01/logs/prefetch-10895466/completion.json",
      "sha256": "4741d8884a42c7132397331da8e5ed09ec635f1d3575c1a70ec72d957ab5d4fb",
      "value": {
        "study": "access_prefetch",
        "mode": "smoke",
        "graphs": 13,
        "passed": 13,
        "failed": 0,
        "checks": 676,
        "equiv": 676,
        "exec_equiv": 676,
        "profile": 338,
        "work": 338,
        "slot_checks": 169,
        "pmu_regions": 0,
        "pf_identity": 208,
        "pf_identity_profile": 104,
        "pf_target_cases": 468
      }
    }
  },
  "dimensions": [
    128,
    128,
    128
  ],
  "numa_interleave": false,
  "first_touch_changed": false,
  "node_cap": 10,
  "counts": {
    "numerical": 1404,
    "canonical_bitwise": 1404,
    "execution_bitwise": 1404,
    "work": 702,
    "profile": 702,
    "slots": 2899,
    "timings": 2730,
    "pmu_regions": 546,
    "pmu_threads": 17472,
    "pf1_identity": 432,
    "pf1_profile_identity": 216,
    "prefetch_target_cases": 972,
    "prefetch_target_audits": 27
  },
  "expansion_gate": {
    "candidate": "s1ExactSP8",
    "seed": 20261007,
    "all_other_gates_pass": true,
    "cohorts": [
      {
        "partition": "intel",
        "exclusive": "false",
        "protocol": "same_slot_consecutive",
        "graphs": 4,
        "geometric_mean": 1.0132383982097102,
        "wins": 2,
        "required_geometric_mean": 1.03,
        "required_wins": 3,
        "pass_filter": false,
        "ratios": [
          {
            "graph": "mycielskian19",
            "job": "10895503",
            "speedup": 1.001644956820648
          },
          {
            "graph": "reddit",
            "job": "10895504",
            "speedup": 0.9870559020026727
          },
          {
            "graph": "roadNet-CA",
            "job": "10895504",
            "speedup": 0.971427448125989
          },
          {
            "graph": "amazon0601",
            "job": "10895504",
            "speedup": 1.0974396065378598
          }
        ]
      },
      {
        "partition": "intel",
        "exclusive": "false",
        "protocol": "same_slot_alternating",
        "graphs": 4,
        "geometric_mean": 1.0153432554622304,
        "wins": 2,
        "required_geometric_mean": 1.03,
        "required_wins": 3,
        "pass_filter": false,
        "ratios": [
          {
            "graph": "mycielskian19",
            "job": "10895503",
            "speedup": 1.0291320783914713
          },
          {
            "graph": "reddit",
            "job": "10895504",
            "speedup": 0.9874249677115483
          },
          {
            "graph": "roadNet-CA",
            "job": "10895504",
            "speedup": 0.9874362501412696
          },
          {
            "graph": "amazon0601",
            "job": "10895504",
            "speedup": 1.0591738905795105
          }
        ]
      },
      {
        "partition": "intel_expr",
        "exclusive": "true",
        "protocol": "same_slot_consecutive",
        "graphs": 4,
        "geometric_mean": 1.0077163100183217,
        "wins": 2,
        "required_geometric_mean": 1.03,
        "required_wins": 3,
        "pass_filter": false,
        "ratios": [
          {
            "graph": "mycielskian19",
            "job": "10895500",
            "speedup": 1.0273257723732077
          },
          {
            "graph": "reddit",
            "job": "10895501",
            "speedup": 0.9664498804817194
          },
          {
            "graph": "roadNet-CA",
            "job": "10895501",
            "speedup": 0.9712973707448082
          },
          {
            "graph": "amazon0601",
            "job": "10895501",
            "speedup": 1.069334103711814
          }
        ]
      },
      {
        "partition": "intel_expr",
        "exclusive": "true",
        "protocol": "same_slot_alternating",
        "graphs": 4,
        "geometric_mean": 1.0085655298835545,
        "wins": 2,
        "required_geometric_mean": 1.03,
        "required_wins": 3,
        "pass_filter": false,
        "ratios": [
          {
            "graph": "mycielskian19",
            "job": "10895500",
            "speedup": 1.0217958292757605
          },
          {
            "graph": "reddit",
            "job": "10895501",
            "speedup": 0.9756405503766076
          },
          {
            "graph": "roadNet-CA",
            "job": "10895501",
            "speedup": 0.9877031486492555
          },
          {
            "graph": "amazon0601",
            "job": "10895501",
            "speedup": 1.0508387465200515
          }
        ]
      }
    ],
    "eligible_to_expand_full17": false,
    "rule": "All four protocol x partition cohorts must each achieve GM>=1.03 and wins>=3/4; no replacement by better seed or candidate.",
    "scope": "Predeclared practical expansion filter, not a statistical significance guarantee."
  },
  "output_sha256": {
    "audit.json": "a6dc782cf43ed5a554770d102ea8ccc090b60c30eafccd799ecde43d51bde65f",
    "interactions.csv": "b61eba2f613241eefa7c6895f52614116b38d5775dae56b92b953d411b6c8fe6",
    "expansion_gate.json": "bcc9c661cd582566b01e17574d3a401e14d838b83cdfe5f56db7268534bcc23e",
    "pmu_summary.csv": "ac06605f9111105856d68aee99521094dff7e3a5c683343503dbcdc093cc56bd",
    "ratios.csv": "c19c97dc5d18d5237ec7c4edeb3c7d5f7ffbdfcabc14f05f522dfa3541d35e2a",
    "summary.csv": "e6282069f095a7978a58774a22f1368dad977f38ec934861e8ff4dbdb414a581",
    "setup.csv": "517abdeb5a4f4cf1ff3e5e47946d5274b988fc396fee743a444555e788d59581"
  },
  "pmu_quality": [
    {
      "graph": "regular128_reuse1",
      "job": "10895499",
      "partition": "intel_expr",
      "regions": 39,
      "thread_records": 1248,
      "enabled_running_unequal": 0,
      "min_running_fraction": 1.0,
      "max_scale": 1.0,
      "max_enable_ms": 0.687294,
      "max_disable_ms": 0.423143
    },
    {
      "graph": "regular128_reuse16",
      "job": "10895499",
      "partition": "intel_expr",
      "regions": 39,
      "thread_records": 1248,
      "enabled_running_unequal": 0,
      "min_running_fraction": 1.0,
      "max_scale": 1.0,
      "max_enable_ms": 0.708734,
      "max_disable_ms": 0.442994
    },
    {
      "graph": "regular128_reuse4",
      "job": "10895499",
      "partition": "intel_expr",
      "regions": 39,
      "thread_records": 1248,
      "enabled_running_unequal": 0,
      "min_running_fraction": 1.0,
      "max_scale": 1.0,
      "max_enable_ms": 0.666173,
      "max_disable_ms": 0.401378
    },
    {
      "graph": "mycielskian19",
      "job": "10895500",
      "partition": "intel_expr",
      "regions": 39,
      "thread_records": 1248,
      "enabled_running_unequal": 0,
      "min_running_fraction": 1.0,
      "max_scale": 1.0,
      "max_enable_ms": 0.715468,
      "max_disable_ms": 0.480495
    },
    {
      "graph": "amazon0601",
      "job": "10895501",
      "partition": "intel_expr",
      "regions": 39,
      "thread_records": 1248,
      "enabled_running_unequal": 0,
      "min_running_fraction": 1.0,
      "max_scale": 1.0,
      "max_enable_ms": 0.679684,
      "max_disable_ms": 0.461603
    },
    {
      "graph": "reddit",
      "job": "10895501",
      "partition": "intel_expr",
      "regions": 39,
      "thread_records": 1248,
      "enabled_running_unequal": 0,
      "min_running_fraction": 1.0,
      "max_scale": 1.0,
      "max_enable_ms": 0.716044,
      "max_disable_ms": 0.418602
    },
    {
      "graph": "roadNet-CA",
      "job": "10895501",
      "partition": "intel_expr",
      "regions": 39,
      "thread_records": 1248,
      "enabled_running_unequal": 0,
      "min_running_fraction": 1.0,
      "max_scale": 1.0,
      "max_enable_ms": 0.777465,
      "max_disable_ms": 0.515733
    },
    {
      "graph": "regular128_reuse1",
      "job": "10895502",
      "partition": "intel",
      "regions": 39,
      "thread_records": 1248,
      "enabled_running_unequal": 0,
      "min_running_fraction": 1.0,
      "max_scale": 1.0,
      "max_enable_ms": 2.412783,
      "max_disable_ms": 0.475904
    },
    {
      "graph": "regular128_reuse16",
      "job": "10895502",
      "partition": "intel",
      "regions": 39,
      "thread_records": 1248,
      "enabled_running_unequal": 0,
      "min_running_fraction": 1.0,
      "max_scale": 1.0,
      "max_enable_ms": 0.758337,
      "max_disable_ms": 0.4617
    },
    {
      "graph": "regular128_reuse4",
      "job": "10895502",
      "partition": "intel",
      "regions": 39,
      "thread_records": 1248,
      "enabled_running_unequal": 0,
      "min_running_fraction": 1.0,
      "max_scale": 1.0,
      "max_enable_ms": 0.755536,
      "max_disable_ms": 0.473659
    },
    {
      "graph": "mycielskian19",
      "job": "10895503",
      "partition": "intel",
      "regions": 39,
      "thread_records": 1248,
      "enabled_running_unequal": 0,
      "min_running_fraction": 1.0,
      "max_scale": 1.0,
      "max_enable_ms": 0.80938,
      "max_disable_ms": 0.582368
    },
    {
      "graph": "amazon0601",
      "job": "10895504",
      "partition": "intel",
      "regions": 39,
      "thread_records": 1248,
      "enabled_running_unequal": 0,
      "min_running_fraction": 1.0,
      "max_scale": 1.0,
      "max_enable_ms": 0.769028,
      "max_disable_ms": 0.434824
    },
    {
      "graph": "reddit",
      "job": "10895504",
      "partition": "intel",
      "regions": 39,
      "thread_records": 1248,
      "enabled_running_unequal": 0,
      "min_running_fraction": 1.0,
      "max_scale": 1.0,
      "max_enable_ms": 0.735506,
      "max_disable_ms": 0.430324
    },
    {
      "graph": "roadNet-CA",
      "job": "10895504",
      "partition": "intel",
      "regions": 39,
      "thread_records": 1248,
      "enabled_running_unequal": 0,
      "min_running_fraction": 1.0,
      "max_scale": 1.0,
      "max_enable_ms": 0.988312,
      "max_disable_ms": 0.639651
    }
  ],
  "descriptive_outlier_flag_rule": "max/min>=1.10; all raw samples retained, no exclusion",
  "descriptive_outliers": [
    {
      "graph": "regular128_reuse4",
      "job": "10895499",
      "partition": "intel_expr",
      "method": "s1ExactSOff",
      "protocol": "same_slot_consecutive",
      "median_ms": 37.9688739777,
      "min_ms": 36.4129543304,
      "max_ms": 40.2400493622,
      "raw_ms": [
        40.2400493622,
        40.1899814606,
        37.9688739777,
        36.416053772,
        36.4129543304
      ]
    },
    {
      "graph": "mycielskian19",
      "job": "10895500",
      "partition": "intel_expr",
      "method": "s1ScalarOff",
      "protocol": "same_slot_consecutive",
      "median_ms": 1047.25694656,
      "min_ms": 1012.46404648,
      "max_ms": 1114.538908,
      "raw_ms": [
        1037.90211678,
        1069.70906258,
        1047.25694656,
        1114.538908,
        1012.46404648
      ]
    },
    {
      "graph": "amazon0601",
      "job": "10895501",
      "partition": "intel_expr",
      "method": "s1ExactSOff",
      "protocol": "same_slot_consecutive",
      "median_ms": 8.75091552734,
      "min_ms": 8.59498977661,
      "max_ms": 10.6530189514,
      "raw_ms": [
        10.6530189514,
        8.75091552734,
        8.59498977661,
        8.74400138855,
        8.89205932617
      ]
    },
    {
      "graph": "roadNet-CA",
      "job": "10895501",
      "partition": "intel_expr",
      "method": "adjExactSP8",
      "protocol": "same_slot_consecutive",
      "median_ms": 23.2918262482,
      "min_ms": 23.2100486755,
      "max_ms": 26.3180732727,
      "raw_ms": [
        23.2918262482,
        23.2100486755,
        26.3180732727,
        23.2989788055,
        23.2310295105
      ]
    },
    {
      "graph": "regular128_reuse1",
      "job": "10895502",
      "partition": "intel",
      "method": "adjScalarOff",
      "protocol": "same_slot_alternating",
      "median_ms": 231.02152347549998,
      "min_ms": 230.61299324,
      "max_ms": 288.640975952,
      "raw_ms": [
        231.14490509,
        231.414079666,
        231.051921844,
        231.17685318,
        230.61299324,
        230.640172958,
        230.741024017,
        230.786085129,
        230.991125107,
        288.640975952
      ]
    },
    {
      "graph": "regular128_reuse16",
      "job": "10895502",
      "partition": "intel",
      "method": "adjExactSP8",
      "protocol": "same_slot_consecutive",
      "median_ms": 66.3180351257,
      "min_ms": 23.589849472,
      "max_ms": 79.9200534821,
      "raw_ms": [
        23.7901210785,
        77.6259899139,
        79.9200534821,
        66.3180351257,
        23.589849472
      ]
    },
    {
      "graph": "regular128_reuse4",
      "job": "10895502",
      "partition": "intel",
      "method": "adjExactSP8",
      "protocol": "same_slot_alternating",
      "median_ms": 47.71852493285,
      "min_ms": 47.5430488586,
      "max_ms": 60.8100891113,
      "raw_ms": [
        47.7249622345,
        47.8210449219,
        47.975063324,
        47.6429462433,
        47.5430488586,
        47.5771427155,
        48.7668514252,
        47.5888252258,
        47.7120876312,
        60.8100891113
      ]
    },
    {
      "graph": "regular128_reuse4",
      "job": "10895502",
      "partition": "intel",
      "method": "s1ScalarOff",
      "protocol": "same_slot_alternating",
      "median_ms": 41.50450229645,
      "min_ms": 40.787935257,
      "max_ms": 45.6359386444,
      "raw_ms": [
        40.787935257,
        42.1040058136,
        40.9181118011,
        41.9931411743,
        40.8408641815,
        41.9809818268,
        41.0280227661,
        42.0081615448,
        40.8940315247,
        45.6359386444
      ]
    },
    {
      "graph": "amazon0601",
      "job": "10895504",
      "partition": "intel",
      "method": "adjExactSOff",
      "protocol": "same_slot_alternating",
      "median_ms": 9.007573127745001,
      "min_ms": 8.81099700928,
      "max_ms": 10.1509094238,
      "raw_ms": [
        9.00483131409,
        8.81099700928,
        8.82411003113,
        9.00912284851,
        9.04083251953,
        8.97002220154,
        9.00602340698,
        10.1509094238,
        9.23919677734,
        9.1290473938
      ]
    },
    {
      "graph": "roadNet-CA",
      "job": "10895504",
      "partition": "intel",
      "method": "adjExactSP1",
      "protocol": "same_slot_consecutive",
      "median_ms": 23.4880447388,
      "min_ms": 23.4260559082,
      "max_ms": 47.4379062653,
      "raw_ms": [
        23.4880447388,
        23.4298706055,
        23.5078334808,
        23.4260559082,
        47.4379062653
      ]
    }
  ],
  "paper_eligible": false,
  "issues": "No new major correctness/evidence issue if PASS. Fixed full17 expansion candidate is accepted only by unchanged four-cohort gate; negative outcomes retained. PMU wall is not used as end-to-end timing.",
  "next": "Root writes final technical report and desktop TXT using audited same-job results; do not expand full17 unless unchanged gate passes.",
  "date": "2026-10-08T04:42:30.494637+00:00"
}
```

## access-prefetch-final-delivery-20261008

```json
{
  "id": "access-prefetch-final-delivery-20261008",
  "status": "ROUND_COMPLETE_AUDITED_NO_FULL17_EXPANSION",
  "desktop_file": "C:\\Users\\花喵\\Desktop\\TFS_预取分组协作与期刊创新边界_20261008.txt",
  "remote_report": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/access_prefetch_20261008/final/TECHNICAL_REPORT_20261008.txt",
  "report_sha256": "35184f811323da48b226429ee98625935e1108691c68887861406fa4dc46790f",
  "report_bytes": 45029,
  "delivery_manifest": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/access_prefetch_20261008/final/DELIVERY_MANIFEST.json",
  "delivery_manifest_sha256": "1fff70ea78c9c26ce096c9e9a392d7c48b49ebce79fea33338236b7f8dd44ed5",
  "analysis_directory": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/access_prefetch_20261008/analysis-01",
  "fixed_candidate": "s1ExactSP8",
  "expansion_gate": {
    "candidate": "s1ExactSP8",
    "seed": 20261007,
    "all_other_gates_pass": true,
    "cohorts": [
      {
        "partition": "intel",
        "exclusive": "false",
        "protocol": "same_slot_consecutive",
        "graphs": 4,
        "geometric_mean": 1.0132383982097102,
        "wins": 2,
        "required_geometric_mean": 1.03,
        "required_wins": 3,
        "pass_filter": false,
        "ratios": [
          {
            "graph": "mycielskian19",
            "job": "10895503",
            "speedup": 1.001644956820648
          },
          {
            "graph": "reddit",
            "job": "10895504",
            "speedup": 0.9870559020026727
          },
          {
            "graph": "roadNet-CA",
            "job": "10895504",
            "speedup": 0.971427448125989
          },
          {
            "graph": "amazon0601",
            "job": "10895504",
            "speedup": 1.0974396065378598
          }
        ]
      },
      {
        "partition": "intel",
        "exclusive": "false",
        "protocol": "same_slot_alternating",
        "graphs": 4,
        "geometric_mean": 1.0153432554622304,
        "wins": 2,
        "required_geometric_mean": 1.03,
        "required_wins": 3,
        "pass_filter": false,
        "ratios": [
          {
            "graph": "mycielskian19",
            "job": "10895503",
            "speedup": 1.0291320783914713
          },
          {
            "graph": "reddit",
            "job": "10895504",
            "speedup": 0.9874249677115483
          },
          {
            "graph": "roadNet-CA",
            "job": "10895504",
            "speedup": 0.9874362501412696
          },
          {
            "graph": "amazon0601",
            "job": "10895504",
            "speedup": 1.0591738905795105
          }
        ]
      },
      {
        "partition": "intel_expr",
        "exclusive": "true",
        "protocol": "same_slot_consecutive",
        "graphs": 4,
        "geometric_mean": 1.0077163100183217,
        "wins": 2,
        "required_geometric_mean": 1.03,
        "required_wins": 3,
        "pass_filter": false,
        "ratios": [
          {
            "graph": "mycielskian19",
            "job": "10895500",
            "speedup": 1.0273257723732077
          },
          {
            "graph": "reddit",
            "job": "10895501",
            "speedup": 0.9664498804817194
          },
          {
            "graph": "roadNet-CA",
            "job": "10895501",
            "speedup": 0.9712973707448082
          },
          {
            "graph": "amazon0601",
            "job": "10895501",
            "speedup": 1.069334103711814
          }
        ]
      },
      {
        "partition": "intel_expr",
        "exclusive": "true",
        "protocol": "same_slot_alternating",
        "graphs": 4,
        "geometric_mean": 1.0085655298835545,
        "wins": 2,
        "required_geometric_mean": 1.03,
        "required_wins": 3,
        "pass_filter": false,
        "ratios": [
          {
            "graph": "mycielskian19",
            "job": "10895500",
            "speedup": 1.0217958292757605
          },
          {
            "graph": "reddit",
            "job": "10895501",
            "speedup": 0.9756405503766076
          },
          {
            "graph": "roadNet-CA",
            "job": "10895501",
            "speedup": 0.9877031486492555
          },
          {
            "graph": "amazon0601",
            "job": "10895501",
            "speedup": 1.0508387465200515
          }
        ]
      }
    ],
    "eligible_to_expand_full17": false,
    "rule": "All four protocol x partition cohorts must each achieve GM>=1.03 and wins>=3/4; no replacement by better seed or candidate.",
    "scope": "Predeclared practical expansion filter, not a statistical significance guarantee."
  },
  "jobs": [
    [
      "10895466",
      "COMPLETED",
      "00:00:11",
      "qhcn047",
      "0:0"
    ],
    [
      "10895499",
      "COMPLETED",
      "00:03:24",
      "qhcn817",
      "0:0"
    ],
    [
      "10895500",
      "COMPLETED",
      "00:14:14",
      "qhcn817",
      "0:0"
    ],
    [
      "10895501",
      "COMPLETED",
      "00:02:54",
      "qhcn819",
      "0:0"
    ],
    [
      "10895502",
      "COMPLETED",
      "00:03:26",
      "qhcn047",
      "0:0"
    ],
    [
      "10895503",
      "COMPLETED",
      "00:14:04",
      "qhcn021",
      "0:0"
    ],
    [
      "10895504",
      "COMPLETED",
      "00:03:01",
      "qhcn021",
      "0:0"
    ]
  ],
  "all_round_jobs_released": true,
  "constraints": {
    "dimensions": [
      128,
      128,
      128
    ],
    "numa_interleave": false,
    "changed_feature_first_touch": false,
    "peak_requested_nodes_including_pending": 6,
    "user_node_cap": 10,
    "github_push": false
  },
  "research_notes": "Current report includes revised work model and restricted SCS correspondence, original local PDF alignment, primary-source novelty boundaries. Trace/quad and actual overlap remain unimplemented/unproved. Historical interim preserved.",
  "date": "2026-10-08T04:44:54.883170+00:00"
}
```

## event-trace-protocol-20261008

```json
{
  "id": "event-trace-protocol-20261008",
  "status": "PROTOCOL_FROZEN_IMPLEMENTATION_IN_PROGRESS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/event_trace_20261008",
  "paper_eligible": false,
  "fixed_candidate": "trace_s_rep",
  "dimensions": [
    128,
    128,
    128
  ],
  "numa_interleave": false,
  "changed_first_touch": false,
  "user_node_cap": 10,
  "planned_peak": 6,
  "protocol_sha256": "e13a2d91d9d00179054c732f2ce135ce2f7c0bb8b12339b9f1ef842325a99116",
  "issues": "New probe isolates offline decision replay; no new result. Fixed PF1, one adjacent plan. Original and previous snapshots retained.",
  "next": "Build trace plan/kernel/runtime, validate small fixtures, then fixed diagnostics.",
  "date": "2026-10-08T04:52:21.570703+00:00"
}
```

## event-trace-structural-diagnostic-20261008

```json
{
  "id": "event-trace-structural-diagnostic-20261008",
  "status": "PRE_BUILD_PROTOCOL_CLARIFICATION",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/event_trace_20261008/trace_opportunity.hpp",
  "old_protocol_sha256": "e13a2d91d9d00179054c732f2ce135ce2f7c0bb8b12339b9f1ef842325a99116",
  "new_protocol_sha256": "c4b432e2953a2388d40b9133f06db282c2fe7924b9719ded62ab23fd48c8a993",
  "purpose": "Distinguish S64 boundary opportunity loss from online discovery cost; sorted/unsorted scope explicit",
  "timing_matrix_changed": false,
  "thresholds_changed": false,
  "candidate_changed": false,
  "diagnostic_excluded_from_required_setup": true,
  "issues": "Unsorted full greedy can share less than S64; report both signs rather than assume monotonicity.",
  "date": "2026-10-08T05:01:48.027640+00:00"
}
```

## event-trace-20261008-01-build

```json
{
  "id": "event-trace-20261008-01-build",
  "kind": "trace_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01",
  "paper_eligible": false,
  "issues": "Isolated immutable build; 128x128x128; default NUMA; no new major issue if PASS",
  "next": "Smoke all gates before performance",
  "date": "2026-10-08T05:09:15.218257+00:00"
}
```

## event-trace-20261008-01-submit-10896223

```json
{
  "id": "event-trace-20261008-01-submit-10896223",
  "kind": "trace_submission",
  "status": "SUBMITTED",
  "job": "10896223",
  "mode": "smoke",
  "study": "trace",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 0,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01/launcher/smoke-20261008-130946-875366",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-08T05:09:47.126242+00:00"
}
```

## event-trace-20261008-01-trace-10896223

```json
{
  "id": "event-trace-20261008-01-trace-10896223",
  "kind": "trace_run",
  "study": "trace",
  "mode": "smoke",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01/logs/trace-10896223",
  "completion": {
    "study": "event_trace",
    "mode": "smoke",
    "graphs": 13,
    "passed": 13,
    "failed": 0,
    "checks": 416,
    "equiv": 416,
    "exec_equiv": 416,
    "dyns_equiv": 104,
    "profile": 208,
    "work": 208,
    "trace_logical": 208,
    "trace_hist": 1677,
    "trace_reject_cases": 26,
    "slot_checks": 104,
    "pmu_regions": 0
  },
  "job": "10896223",
  "node": "qhcn139",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-08T05:10:04.859801+00:00"
}
```

## event-trace-20261008-01-submit-10896227

```json
{
  "id": "event-trace-20261008-01-submit-10896227",
  "kind": "trace_submission",
  "status": "SUBMITTED",
  "job": "10896227",
  "mode": "synthetic",
  "study": "trace",
  "partition": "intel_expr",
  "exclusive": true,
  "requested_nodes_before": 0,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01/launcher/ex-syn-20261008-131052-408838",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-08T05:10:52.545530+00:00"
}
```

## event-trace-20261008-01-submit-10896229

```json
{
  "id": "event-trace-20261008-01-submit-10896229",
  "kind": "trace_submission",
  "status": "SUBMITTED",
  "job": "10896229",
  "mode": "mycielskian19",
  "study": "trace",
  "partition": "intel_expr",
  "exclusive": true,
  "requested_nodes_before": 1,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01/launcher/ex-myc-20261008-131056-397886",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-08T05:10:56.621714+00:00"
}
```

## event-trace-20261008-01-submit-10896230

```json
{
  "id": "event-trace-20261008-01-submit-10896230",
  "kind": "trace_submission",
  "status": "SUBMITTED",
  "job": "10896230",
  "mode": "reddit,roadNet-CA,amazon0601",
  "study": "trace",
  "partition": "intel_expr",
  "exclusive": true,
  "requested_nodes_before": 2,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01/launcher/ex-other-20261008-131100-288014",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-08T05:11:00.444774+00:00"
}
```

## event-trace-20261008-01-submit-10896231

```json
{
  "id": "event-trace-20261008-01-submit-10896231",
  "kind": "trace_submission",
  "status": "SUBMITTED",
  "job": "10896231",
  "mode": "synthetic",
  "study": "trace",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 3,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01/launcher/sh-syn-20261008-131104-045320",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-08T05:11:04.188930+00:00"
}
```

## event-trace-20261008-01-submit-10896233

```json
{
  "id": "event-trace-20261008-01-submit-10896233",
  "kind": "trace_submission",
  "status": "SUBMITTED",
  "job": "10896233",
  "mode": "mycielskian19",
  "study": "trace",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 4,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01/launcher/sh-myc-20261008-131108-005874",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-08T05:11:08.172902+00:00"
}
```

## event-trace-20261008-01-submit-10896234

```json
{
  "id": "event-trace-20261008-01-submit-10896234",
  "kind": "trace_submission",
  "status": "SUBMITTED",
  "job": "10896234",
  "mode": "reddit,roadNet-CA,amazon0601",
  "study": "trace",
  "partition": "intel",
  "exclusive": false,
  "requested_nodes_before": 5,
  "node_cap": 10,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01/launcher/sh-other-20261008-131111-977859",
  "paper_eligible": false,
  "issues": "Fresh rule read and queue checked; shared/exclusive cap10 combined; no NUMA interleave",
  "next": "Validate completion and retain raw paired controls",
  "date": "2026-10-08T05:11:12.138075+00:00"
}
```

## event-trace-20261008-01-trace-10896234

```json
{
  "id": "event-trace-20261008-01-trace-10896234",
  "kind": "trace_run",
  "study": "trace",
  "mode": "reddit,roadNet-CA,amazon0601",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01/logs/trace-10896234",
  "completion": {
    "study": "event_trace",
    "mode": "reddit,roadNet-CA,amazon0601",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 96,
    "equiv": 96,
    "exec_equiv": 96,
    "dyns_equiv": 24,
    "profile": 48,
    "work": 48,
    "trace_logical": 48,
    "trace_hist": 387,
    "trace_reject_cases": 0,
    "slot_checks": 360,
    "pmu_regions": 72
  },
  "job": "10896234",
  "node": "qhcn152",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-08T05:13:10.364484+00:00"
}
```

## event-trace-20261008-01-trace-10896231

```json
{
  "id": "event-trace-20261008-01-trace-10896231",
  "kind": "trace_run",
  "study": "trace",
  "mode": "synthetic",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01/logs/trace-10896231",
  "completion": {
    "study": "event_trace",
    "mode": "synthetic",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 96,
    "equiv": 96,
    "exec_equiv": 96,
    "dyns_equiv": 24,
    "profile": 48,
    "work": 48,
    "trace_logical": 48,
    "trace_hist": 387,
    "trace_reject_cases": 0,
    "slot_checks": 360,
    "pmu_regions": 72
  },
  "job": "10896231",
  "node": "qhcn139",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-08T05:13:44.512747+00:00"
}
```

## event-trace-pending-timelimit-adjustment-20261008-01

```json
{
  "id": "event-trace-pending-timelimit-adjustment-20261008-01",
  "status": "APPLIED",
  "kind": "scheduling_only",
  "jobs": [
    "10896227",
    "10896230"
  ],
  "previous_minutes": 12,
  "new_minutes": 5,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/event_trace_20261008/scheduling_adjustment_01",
  "shared_actual_elapsed": {
    "10896231": "00:02:40",
    "10896234": "00:01:58"
  },
  "paper_eligible": false,
  "issues": "Only own pending wall limits shortened for backfill; independent/exclusive method, binding, NUMA, samples and node cap unchanged",
  "next": "Wait for exclusive execution and audit all raw results",
  "date": "2026-10-08T05:16:18.409421+00:00"
}
```

## event-trace-20261008-01-trace-10896233

```json
{
  "id": "event-trace-20261008-01-trace-10896233",
  "kind": "trace_run",
  "study": "trace",
  "mode": "mycielskian19",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01/logs/trace-10896233",
  "completion": {
    "study": "event_trace",
    "mode": "mycielskian19",
    "graphs": 1,
    "passed": 1,
    "failed": 0,
    "checks": 32,
    "equiv": 32,
    "exec_equiv": 32,
    "dyns_equiv": 8,
    "profile": 16,
    "work": 16,
    "trace_logical": 16,
    "trace_hist": 129,
    "trace_reject_cases": 0,
    "slot_checks": 120,
    "pmu_regions": 24
  },
  "job": "10896233",
  "node": "qhcn140",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-08T05:19:45.275833+00:00"
}
```

## event-trace-shared-stage-delivery-20261008

```json
{
  "id": "event-trace-shared-stage-delivery-20261008",
  "status": "SHARED_AUDITED_EXCLUSIVE_PENDING",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/event_trace_20261008/shared-stage",
  "report_sha256": "aa0c160893f102d7a65bbf851995b35d6df9c473edf7c1d17af84e3239e1257c",
  "report_bytes": 15376,
  "paper_eligible": false,
  "shared_analysis": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/event_trace_20261008/analysis_inputs_01/shared_review_02.json",
  "shared_graphs": 7,
  "shared_timings": 840,
  "shared_pmu_regions": 168,
  "smoke_fixtures": 13,
  "expansion_ruled_out": true,
  "exclusive_pending": [
    "10896227",
    "10896229",
    "10896230"
  ],
  "issues": "No correctness failure. Fixed trace_s_rep fails all completed shared practical expansion cohorts; exclusive phase and full final audit still pending. Negative result retained.",
  "next": "Complete existing exclusive jobs; one-shot independent final audit and separate desktop full report; no full17 expansion",
  "date": "2026-10-08T05:29:51.964267+00:00"
}
```

## event-trace-shared-stage-delivery-20261008-v2

```json
{
  "id": "event-trace-shared-stage-delivery-20261008-v2",
  "status": "SHARED_AUDITED_EXCLUSIVE_PENDING",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/event_trace_20261008/shared-stage",
  "report_sha256": "7e819475723d3a26f97a31ef87440dc045f0bd82c62dab46b98a0d94838ec174",
  "report_bytes": 15385,
  "paper_eligible": false,
  "shared_analysis": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/event_trace_20261008/analysis_inputs_01/shared_review_02.json",
  "shared_graphs": 7,
  "shared_timings": 840,
  "shared_pmu_regions": 168,
  "smoke_fixtures": 13,
  "expansion_ruled_out": true,
  "exclusive_pending": [
    "10896227",
    "10896229",
    "10896230"
  ],
  "issues": "Two table labels corrected: baseline/candidate ratios, and current S64 persistent trace capacity. Numbers and decisions unchanged; v1 archive retained. Exclusive final audit remains pending.",
  "next": "Complete existing exclusive jobs; one-shot independent final audit and separate desktop full report; no full17 expansion",
  "report_path": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/event_trace_20261008/shared-stage/TECHNICAL_REPORT_SHARED_20261008_v2.txt",
  "date": "2026-10-08T05:33:17.396743+00:00"
}
```

## event-trace-delivery-watcher-started-20261008

```json
{
  "id": "event-trace-delivery-watcher-started-20261008",
  "kind": "existing_experiment_automation",
  "status": "WAITING_FOR_EXISTING_JOBS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/event_trace_20261008/automation_01",
  "launch": {
    "pid": 23200,
    "script": "C:\\Users\\花喵\\tmp\\tfs_event_trace_20261008\\watch_trace_delivery.py",
    "script_sha256": "58c9c7af9d2d55f848493e17f821719881b64ba5b1248d5e4e69aa1603717309",
    "finalizer_sha256": "727949fdc362f5caade82b4dd8dc471dd8fe66d5b61bf002ac2eeaaddfa106e3",
    "started_at": "2026-10-08T13:38:40.5159543+08:00",
    "existing_jobs_only": true,
    "no_new_jobs": true,
    "max_wait_hours": 4
  },
  "run": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01",
  "pending_jobs": [
    "10896227",
    "10896229",
    "10896230"
  ],
  "shared_report": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/event_trace_20261008/shared-stage/TECHNICAL_REPORT_SHARED_20261008_v2.txt",
  "shared_report_sha256": "7e819475723d3a26f97a31ef87440dc045f0bd82c62dab46b98a0d94838ec174",
  "final_report_expected": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/event_trace_20261008/final/TECHNICAL_REPORT_20261008.txt",
  "paper_eligible": false,
  "issues": "Shared 7 graphs plus 13 smoke audited. Exclusive not yet run; final analysis incomplete. Local hidden watcher PID23200 verified alive and accounting read succeeded. One-shot pinned finalizer, fail closed, no new jobs. No scientific claims from pending data.",
  "next": "Inspect local delivery_watcher_status.json or remote automation attempts. Do not double launch. After 7 completed jobs independent all-gate audit creates final report and copies desktop; deadline4h or any failure requires diagnosis.",
  "date": "2026-10-08T05:39:44.574127+00:00"
}
```

## event-trace-20261008-01-trace-10896227

```json
{
  "id": "event-trace-20261008-01-trace-10896227",
  "kind": "trace_run",
  "study": "trace",
  "mode": "synthetic",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01/logs/trace-10896227",
  "completion": {
    "study": "event_trace",
    "mode": "synthetic",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 96,
    "equiv": 96,
    "exec_equiv": 96,
    "dyns_equiv": 24,
    "profile": 48,
    "work": 48,
    "trace_logical": 48,
    "trace_hist": 387,
    "trace_reject_cases": 0,
    "slot_checks": 360,
    "pmu_regions": 72
  },
  "job": "10896227",
  "node": "qhcn818",
  "partition": "intel_expr",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-08T06:21:00.097780+00:00"
}
```

## event-trace-20261008-01-trace-10896230

```json
{
  "id": "event-trace-20261008-01-trace-10896230",
  "kind": "trace_run",
  "study": "trace",
  "mode": "reddit,roadNet-CA,amazon0601",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01/logs/trace-10896230",
  "completion": {
    "study": "event_trace",
    "mode": "reddit,roadNet-CA,amazon0601",
    "graphs": 3,
    "passed": 3,
    "failed": 0,
    "checks": 96,
    "equiv": 96,
    "exec_equiv": 96,
    "dyns_equiv": 24,
    "profile": 48,
    "work": 48,
    "trace_logical": 48,
    "trace_hist": 387,
    "trace_reject_cases": 0,
    "slot_checks": 360,
    "pmu_regions": 72
  },
  "job": "10896230",
  "node": "qhcn818",
  "partition": "intel_expr",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-08T06:22:55.595941+00:00"
}
```

## event-trace-20261008-01-trace-10896229

```json
{
  "id": "event-trace-20261008-01-trace-10896229",
  "kind": "trace_run",
  "study": "trace",
  "mode": "mycielskian19",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/event-trace-20261008-01/logs/trace-10896229",
  "completion": {
    "study": "event_trace",
    "mode": "mycielskian19",
    "graphs": 1,
    "passed": 1,
    "failed": 0,
    "checks": 32,
    "equiv": 32,
    "exec_equiv": 32,
    "dyns_equiv": 8,
    "profile": 16,
    "work": 16,
    "trace_logical": 16,
    "trace_hist": 129,
    "trace_reject_cases": 0,
    "slot_checks": 120,
    "pmu_regions": 24
  },
  "job": "10896229",
  "node": "qhcn817",
  "partition": "intel_expr",
  "paper_eligible": false,
  "issues": "Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched",
  "next": "Numerical/work gate before interpretation, reconcile all three handoffs",
  "date": "2026-10-08T06:29:59.053252+00:00"
}
```

## event-trace-20261008-01-auto-final-attempt_20261008T063020216522Z_511945

```json
{
  "analysis": "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/results/event_trace_20261008/analysis-01",
  "analysis_pass": true,
  "automatic_summary_not_new_human_review": true,
  "compact_audit_sha256": "3621ba1a23b0d2788b4b7feae984f58bd07e0c7e69e77aaeb006343cacc2d8c5",
  "counts": {
    "canonical_bitwise": 864,
    "dyns_bitwise": 216,
    "execution_bitwise": 864,
    "immutable": 54,
    "numerical": 864,
    "opportunity": 27,
    "pmu_regions": 336,
    "pmu_threads": 10752,
    "profile": 432,
    "reject_cases": 26,
    "slots": 1784,
    "smoke_dumps": 13,
    "timings": 1680,
    "trace_hist": 3483,
    "trace_logical": 432,
    "work": 432
  },
  "date": "2026-10-08T06:30:54.109959+00:00",
  "evidence": "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/results/event_trace_20261008/final",
  "expand_full17": false,
  "fixed_candidate": "trace_s_rep",
  "id": "event-trace-20261008-01-auto-final-attempt_20261008T063020216522Z_511945",
  "issues": "No new experimental issue when all gates pass. Preserve negative results and limits; no DRAM/overlap claim.",
  "jobs": [
    "10896223",
    "10896227",
    "10896229",
    "10896230",
    "10896231",
    "10896233",
    "10896234"
  ],
  "kind": "automatic_final_analysis",
  "new_jobs_submitted": false,
  "next": "Deliver automatic TXT and compact manifests; no additional research/jobs.",
  "paper_eligible": false,
  "run": "/online1/huangjianqiang_group/hdacp1/wzh/GCN-extra/runs/event-trace-20261008-01",
  "status": "PASS",
  "technical_report_sha256": "48319947bbe58789c4d27a6785756f98d4041ccb982b167256a970c8aff720d9"
}
```

## event-trace-desktop-delivery-20261008

```json
{
  "id": "event-trace-desktop-delivery-20261008",
  "status": "DELIVERED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/event_trace_20261008/final",
  "desktop_path": "C:\\Users\\花喵\\Desktop\\TFS_消费事件计划与分组边界_完整审计_20261008.txt",
  "desktop_sha256": "48319947bbe58789c4d27a6785756f98d4041ccb982b167256a970c8aff720d9",
  "paper_eligible": false,
  "issues": "Independent seven-job audit passed; desktop bytes match remote report. Automatically aggregated report, with interpretation limits retained.",
  "next": "Review full report; follow fixed expansion decision; no new jobs automatically submitted",
  "date": "2026-10-08T06:31:06.879939+00:00"
}
```

## frontier-structure-prepared-20261008

```json
{
  "id": "frontier-structure-prepared-20261008",
  "status": "PREPARED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01",
  "manifest": {
    "experiment": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/experiments/frontier_structure_20261008",
    "run": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01",
    "result": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/frontier_structure_20261008",
    "scope": "STRUCTURE_ONLY",
    "source_sha256": {
      "PROTOCOL.md": "ccc367cc34b91ffc94618bc29a5fd3e94efa5299ae2472e0fbdba0a4daa84377",
      "ANALYZER_INTERFACE.md": "cbe21b1a33b898cefde223c22ef986235413c3ef331912289e8fb7bcc5dd95ce",
      "frontier_structure.cpp": "2d7a8ae89203a08f40e24abaf65d21a0c3dfd418a82632bcdc5bb01d23f0f511",
      "grouping_schedule.hpp": "2dc2a0a978c87959b5523a1ae11e29e74b36e1f447d4f2dbd3dcb62b8d8dab45",
      "audit_frontier.py": "ea75cff52b94b7c9f655c0c765434670ef981d55be2f667efcab962b73ba51c4",
      "frontier_prior_inputs.json": "53c981305c7cc7db4ea368a54288309517053c75df9f3eec7ecd41659028d5cf",
      "run_structure.py": "3fa19d7ae8343a45709c19fd91f5ff0fc077400e453b2e73a143684982e20840",
      "validate_real.py": "1d767f15000d0e861f33360f9188b42a034cf2ca3ac203c8047e72db30e5eec6",
      "structure_job.sh": "e3ba3b583ef5dbac8238cfe3537753b7d344549cae9f57b994cfe8631eb445c4",
      "stage_and_submit.py": "72059d1b78209687393b6fde72797f2779a21c1dc0c85b4f782923a85b57e016"
    },
    "authority_sha256": {
      "/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md": "55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7",
      "/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md": "641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca"
    },
    "contract_sha256": "b6419e5106278106da6727c90f4cfa185cc70c587d70b4087c0bc286fd0dc244",
    "authority_spec_sha256": {
      "MATH_SPEC_V2.md": "1938d301e3fc913766744af166c0a41b1cb767d7d7419af9c098d3de955cf266",
      "KERNEL_LAYOUT_V2.md": "732e859128baa3cf87813df82cf7562d14358fc6e1cf17c32ec0dde5a743595f",
      "PYTORCH_EXTENSION_SPEC.md": "a3784dd66d4986bde33a40d523f761ee8956da9bbbe711f1707d13d8d2d82df2",
      "EXPERIMENT_SPEC_V2.md": "ecb8fe1c394d9ec47aae97d8f5b87c5b948963c7e8c8b25d70128483c7c0d10d"
    },
    "previous_terminal_evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/event_trace_20261008/final/compact_audit.json",
    "requested_nodes_before": 0,
    "requested_nodes_this_study": 1,
    "user_cap": 10,
    "desktop_write": false,
    "numa_interleave": false
  },
  "paper_eligible": false,
  "issues": "No new major issue; fixed inference shape and preserved sources. Structure-only serial producer abstraction, not runtime proof.",
  "next": "One shared-node compile/oracle/diagnostic4 job; stop on any gate failure.",
  "date": "2026-10-08T08:16:39.565520+00:00"
}
```

## frontier-structure-submitted-10898893

```json
{
  "id": "frontier-structure-submitted-10898893",
  "status": "SUBMITTED",
  "job": "10898893",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01",
  "paper_eligible": false,
  "desktop_write": false,
  "issues": "No new major issue; graph-only counts, no performance kernel. User-authorized one shared node.",
  "next": "Verify oracle success, frozen input/permutation hashes, real-graph output and Slurm status.",
  "date": "2026-10-08T08:16:39.713992+00:00"
}
```

## frontier-structure-20261008-10898893

```json
{
  "id": "frontier-structure-20261008-10898893",
  "status": "PASS",
  "exit_code": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01/compute-10898893",
  "completion": {
    "status": "PASS",
    "scope": "STRUCTURE_ONLY",
    "graphs": [
      {
        "graph": "amazon0601",
        "status": "PASS",
        "input_sha256": "b8e8abf5adc129bfc3c65248e8acca03fa364481f37a9efc78b9a5c9bff1d1fb",
        "elapsed_diagnostic_seconds": 0.8449976611882448,
        "structure_sha256": "dc8072b139ba20ff24a960cffd6efebbe5ed80f982e7e75a6a37d8be0196d72c"
      },
      {
        "graph": "roadNet-CA",
        "status": "PASS",
        "input_sha256": "861d74683ef034b3fe719cf00770fcb2b600a862aa02ade084217d21f96da86c",
        "elapsed_diagnostic_seconds": 2.3514850325882435,
        "structure_sha256": "33869c1729a7a17382fcd58d8832ec107e9197594c08eb3cb139902459438ee6"
      },
      {
        "graph": "reddit",
        "status": "PASS",
        "input_sha256": "26ac2ddc04e52796ba175721fee4e376fcde46bff6ca6a8a4922cce6a296617a",
        "elapsed_diagnostic_seconds": 9.38690217770636,
        "structure_sha256": "8865defe056ebe8abcf59809ec4213cfb37b93b64368f920f57803c2f33833ee"
      },
      {
        "graph": "mycielskian19",
        "status": "PASS",
        "input_sha256": "e685700b64a07b5d871461ff9438126d4b4782f7bc57f25ed7045e250610cefe",
        "elapsed_diagnostic_seconds": 39.49518750421703,
        "structure_sha256": "a58c7146e1fd732a78343d89eb8beffca1bfcf02834db3395ead4a4dc3dbdf62"
      }
    ],
    "oracle_status": {
      "schema": "independent-frontier-audit-v1",
      "status": "PASS",
      "oracle": "produced bool + per-row cursor + occurrence reference counts",
      "empty_semantics": "rank0 zero-complete; no ready/projection; no bias",
      "comparisons": [
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "single_empty",
          "nodes": 1,
          "edges": 0,
          "input_sha256": "25fb090da0cc6d2cb58dfddcc8a47d6b09e925bc41569639029e60da57a4fe7a",
          "native_json": "fixtures/single_empty.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "single_self",
          "nodes": 1,
          "edges": 1,
          "input_sha256": "511115b01f59ac87109b43e737f93388ad445adb3bdd3e5d38a8b4f4da9359a8",
          "native_json": "fixtures/single_self.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 17781897524818458121
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 12142191458066772073
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 12142191458066772073
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "all_empty_17",
          "nodes": 17,
          "edges": 0,
          "input_sha256": "79fc352179977792662580c9614f65be7dc8cb6ba515ab023f29a9457e656a55",
          "native_json": "fixtures/all_empty_17.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 17781897524818458121
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 12142191458066772073
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 12142191458066772073
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "all_self_17",
          "nodes": 17,
          "edges": 17,
          "input_sha256": "4265e056f2b523773b8a1c9d84e2c27e7ff0966e542db2164b31d47698f16c8c",
          "native_json": "fixtures/all_self_17.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 11906479051428177006
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 14074308581311334692
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 14074308581311334692
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "all_self_31",
          "nodes": 31,
          "edges": 31,
          "input_sha256": "bce9dcf7d374f2dc70e5976c501ec070fdbdaf98e4e837f3fe9995c08640a314",
          "native_json": "fixtures/all_self_31.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 2689684201023061881
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 4342150641277792007
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 4342150641277792007
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "all_self_33",
          "nodes": 33,
          "edges": 33,
          "input_sha256": "1b3024ddefe5fe771ac984840a999de0266c61804126077de8760f681b9c6ba6",
          "native_json": "fixtures/all_self_33.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 12195995521320448702
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11570871483800784260
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11570871483800784260
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "source_repeated_after_hole",
          "nodes": 3,
          "edges": 3,
          "input_sha256": "cf3b2e5172c924538976e6866a1b4ed131d775fa800b33a0a0ebcae82b771068",
          "native_json": "fixtures/source_repeated_after_hole.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 12195995521320448702
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 10947662795537084930
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 10947662795537084930
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "unsorted_repeated",
          "nodes": 3,
          "edges": 135,
          "input_sha256": "dc1c87f29ffae64db08e5d326a220965bae7747574ef3b200ac42fa7b136a2c4",
          "native_json": "fixtures/unsorted_repeated.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11126444148914698056
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11125488673309971922
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11125488673309971922
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "prefix_partial_releases_source",
          "nodes": 2,
          "edges": 3,
          "input_sha256": "6a28fd036787bffae21dfb7b09eb965c2384eddad752ff192f2c39d11c5f5ce3",
          "native_json": "fixtures/prefix_partial_releases_source.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 14813328752031797358
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 10724748386326730340
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 10724748386326730340
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "one_source_many_consumers",
          "nodes": 19,
          "edges": 19,
          "input_sha256": "8cbb98bb92546c5eb8251abe44c8203759a76f8a7db5329acb2763ab005251e5",
          "native_json": "fixtures/one_source_many_consumers.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 12725725452840658654
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 15904748385504867674
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 15904748385504867674
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "many_sources_one_consumer",
          "nodes": 35,
          "edges": 35,
          "input_sha256": "24487dde38bf6ad555fa19f6a53332386d5278a8d9072f3da5e8b55b2c9ed42f",
          "native_json": "fixtures/many_sources_one_consumer.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 12725725452840658654
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 15904748385504867674
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 15904748385504867674
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "reverse_long_row",
          "nodes": 35,
          "edges": 35,
          "input_sha256": "8dbcaeb8df70ee67d7784d0f368c802b19e4430d595d704367480ea2e70e5671",
          "native_json": "fixtures/reverse_long_row.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 7128975744487924360
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 1736370304185003888
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 1736370304185003888
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "mixed_empty_nonempty_34",
          "nodes": 34,
          "edges": 17,
          "input_sha256": "35cc875c21446baa161957546331950b88f84b19063f5ff6bea971be8f07c192",
          "native_json": "fixtures/mixed_empty_nonempty_34.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 7128975744487924360
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 8151927772102078144
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 8151927772102078144
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "holes_between_batches",
          "nodes": 34,
          "edges": 8,
          "input_sha256": "7bed9d4dcc16283283d4f81ebf79ff235ac82cf62b87192c985df71499018f74",
          "native_json": "fixtures/holes_between_batches.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 17781897524818458121
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 12142191458066772073
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 12142191458066772073
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "complete_17",
          "nodes": 17,
          "edges": 289,
          "input_sha256": "7403c6b9cadbcb953daf4b42c98bb64d2d21cf7e02a2047dd0f8e2770a7179f3",
          "native_json": "fixtures/complete_17.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 14948070509728317059
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 14337615746030281347
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 14337615746030281347
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "interleaved_cliques_64",
          "nodes": 64,
          "edges": 1024,
          "input_sha256": "dd7dba909ea361eb32f07c6e6344a46fd9fc2a77d37e8bb799936a0d1e8a6cdd",
          "native_json": "fixtures/interleaved_cliques_64.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 18,
              "status": "PASS",
              "permutation_fnv64": 12018768605665587465
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 18,
              "status": "PASS",
              "permutation_fnv64": 6100711701888931385
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 18,
              "status": "PASS",
              "permutation_fnv64": 6100711701888931385
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "checkpoint16_tail_273",
          "nodes": 273,
          "edges": 275,
          "input_sha256": "e7c4d02446a8a299d72341dbe0b6e0cf4d7265e20b7d873184a0c9fa5407a268",
          "native_json": "fixtures/checkpoint16_tail_273.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_00",
          "nodes": 1,
          "edges": 0,
          "input_sha256": "25fb090da0cc6d2cb58dfddcc8a47d6b09e925bc41569639029e60da57a4fe7a",
          "native_json": "fixtures/random_00.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11126444148914698056
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11125488673309971922
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11125488673309971922
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_01",
          "nodes": 2,
          "edges": 1,
          "input_sha256": "1ffe5073de307d3b2cb05541da813dc49741260199f190ca441e2fc4e231c7a4",
          "native_json": "fixtures/random_01.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 12195995521320448702
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 12198861948134627104
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 12198861948134627104
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_02",
          "nodes": 3,
          "edges": 9,
          "input_sha256": "234f8c0e994a6175af64e26e0a4010cdd2e784c0f0b8312588d39c904e8bc0a7",
          "native_json": "fixtures/random_02.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 5247242255190488182
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 516319112600493668
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 516319112600493668
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_03",
          "nodes": 7,
          "edges": 40,
          "input_sha256": "c192ed81915b9500aba18967fb924f2a1e45b34fe4c291654e861cab0978bedd",
          "native_json": "fixtures/random_03.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 8637538703190335550
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 14300789680413993582
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 14300789680413993582
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_04",
          "nodes": 15,
          "edges": 178,
          "input_sha256": "c861c81c5774fbab4a7d9016ad044ba5790ff83957c7f731633420d8bf1dd4e7",
          "native_json": "fixtures/random_04.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 615731795747517251
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 15685578572634478765
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 15685578572634478765
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_05",
          "nodes": 16,
          "edges": 270,
          "input_sha256": "2ddc26cc66d96cdda1deb7cd9976fbc15b7af2052a4511d2620aeddc5549fc4f",
          "native_json": "fixtures/random_05.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 17781897524818458121
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 3538226403078292701
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 3538226403078292701
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_06",
          "nodes": 17,
          "edges": 251,
          "input_sha256": "8e3de0059cfb42310a22be498804258d96ec1965ea8b69b25dbf409f0e483859",
          "native_json": "fixtures/random_06.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 11906479051428177006
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 16665321237429780548
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 16665321237429780548
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_07",
          "nodes": 31,
          "edges": 518,
          "input_sha256": "3e5eafb7de4e7755e091e47b27bdbcbd094daaf59fe0d7ce5e219982a7aa07f4",
          "native_json": "fixtures/random_07.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 14348390467614960643
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 8087546031708206725
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 8087546031708206725
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_08",
          "nodes": 32,
          "edges": 503,
          "input_sha256": "e991981f871e698a413574075406af5e44bee00c3d38b5a4e66b4664171c2437",
          "native_json": "fixtures/random_08.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 2689684201023061881
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 7403495974024892825
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 7403495974024892825
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_09",
          "nodes": 33,
          "edges": 390,
          "input_sha256": "ba68a89d1f8a9f7f330a911e9a104dce22bc34151f30cb2370aeab335cc89ef5",
          "native_json": "fixtures/random_09.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 15140976715926322910
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 15466639323401867020
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 15466639323401867020
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_10",
          "nodes": 47,
          "edges": 988,
          "input_sha256": "016dd2881026d5ddab18521cc43303fe4463d07d06dc7ea018ae17cda9d91713",
          "native_json": "fixtures/random_10.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 5430610765907325827
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 4595665417053337179
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 4595665417053337179
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_11",
          "nodes": 48,
          "edges": 829,
          "input_sha256": "be58e715f0e3335c107c93b1952fea7a1539e8eef69255ffaa67a9dc338cfe2a",
          "native_json": "fixtures/random_11.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 16460632226177304526
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 2857820231334612792
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 2857820231334612792
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_12",
          "nodes": 63,
          "edges": 1389,
          "input_sha256": "c448b02f719989a5b2788c6787cd1dd9eee0204934dcbc38173668402bc1d345",
          "native_json": "fixtures/random_12.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 14948070509728317059
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 13549907680927591083
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 13549907680927591083
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_13",
          "nodes": 64,
          "edges": 1263,
          "input_sha256": "292d9fb56b708815a50e352b8cdea6e38b65f98446919997c9a303de1db761c3",
          "native_json": "fixtures/random_13.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_14",
          "nodes": 1,
          "edges": 1,
          "input_sha256": "511115b01f59ac87109b43e737f93388ad445adb3bdd3e5d38a8b4f4da9359a8",
          "native_json": "fixtures/random_14.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11126444148914698056
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11126444148914698056
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11126444148914698056
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_15",
          "nodes": 2,
          "edges": 0,
          "input_sha256": "834a4ef2c30215bd51ca4d342e2356b3313114fdb52207a14dd835ef0190d603",
          "native_json": "fixtures/random_15.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 12195995521320448702
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11570871483800784260
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11570871483800784260
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_16",
          "nodes": 3,
          "edges": 7,
          "input_sha256": "affe892b0f837a9e1c75775f3feb3fa72c004e377610802370b82126b6a8ecfc",
          "native_json": "fixtures/random_16.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 5247242255190488182
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 13698135938802784830
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 13698135938802784830
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_17",
          "nodes": 7,
          "edges": 42,
          "input_sha256": "0b50dfafdc33aba65aa3d25198ed07d1d1074dd84ddee8698384beb2eb978ab7",
          "native_json": "fixtures/random_17.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 8637538703190335550
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 17034893968432645716
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 17034893968432645716
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_18",
          "nodes": 15,
          "edges": 109,
          "input_sha256": "20a99723748b8b7dcc29c10b9e4b73ad4c34379bb6ff7612d88ede4ef7c546e9",
          "native_json": "fixtures/random_18.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 615731795747517251
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 2360895407242716599
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 2360895407242716599
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_19",
          "nodes": 16,
          "edges": 218,
          "input_sha256": "7e67ab3d7573488e1aa854899be869374b55ef769ea8f3ea8ab6d7d9276eecd4",
          "native_json": "fixtures/random_19.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 17781897524818458121
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 7856528663018885825
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 7856528663018885825
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_20",
          "nodes": 17,
          "edges": 296,
          "input_sha256": "37727e1d5630510ca4b5f08c415c5252f03bc08a9674d98c5350e5edc750bdb3",
          "native_json": "fixtures/random_20.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 11906479051428177006
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 4773549144062678196
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 4773549144062678196
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_21",
          "nodes": 31,
          "edges": 440,
          "input_sha256": "df23f70b94268bdab6e5c2aeae95ce7d14a097b732e7c2476588c78f1bd1a258",
          "native_json": "fixtures/random_21.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 14348390467614960643
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 5641635762634943435
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 5641635762634943435
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_22",
          "nodes": 32,
          "edges": 480,
          "input_sha256": "c94392018238bed8a1d2afbfa5b3ad80e725e754199a81df33444cb19226cbf2",
          "native_json": "fixtures/random_22.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 2689684201023061881
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 15132883425172879143
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 15132883425172879143
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_23",
          "nodes": 33,
          "edges": 557,
          "input_sha256": "b2c08e4c955d7951878034db01646f53351603ea0f717000e872c546fe3602f2",
          "native_json": "fixtures/random_23.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 15140976715926322910
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 6424489400688739170
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 6424489400688739170
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_24",
          "nodes": 47,
          "edges": 947,
          "input_sha256": "bf87de29b66e55c11cefd25c4bb6db6cd932950f6ef1799cae340d55f6d49609",
          "native_json": "fixtures/random_24.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 5430610765907325827
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 11032589607080220571
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 11032589607080220571
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_25",
          "nodes": 48,
          "edges": 804,
          "input_sha256": "a3d3e3c2215ef6583b86d0b0bd99ee69520ccd90c84cdf0a1706b4773cdb92ce",
          "native_json": "fixtures/random_25.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 16460632226177304526
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 10989407360562571028
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 10989407360562571028
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_26",
          "nodes": 63,
          "edges": 976,
          "input_sha256": "77499c318742afb39d69f7311562c6e67852e0a834f94657a11fa66c61470319",
          "native_json": "fixtures/random_26.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 14948070509728317059
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 13366242123403084297
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 13366242123403084297
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_27",
          "nodes": 64,
          "edges": 869,
          "input_sha256": "bd7b92c93303e45f6f9e386914c42d5dad788845d31185f33ee7ca370ee0ca80",
          "native_json": "fixtures/random_27.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_28",
          "nodes": 1,
          "edges": 0,
          "input_sha256": "25fb090da0cc6d2cb58dfddcc8a47d6b09e925bc41569639029e60da57a4fe7a",
          "native_json": "fixtures/random_28.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11126444148914698056
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11125488673309971922
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11125488673309971922
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_29",
          "nodes": 2,
          "edges": 5,
          "input_sha256": "90f9b05a4eb753b4156d6ec0d7872444af751677da1be9d7214ce474dfde9c7c",
          "native_json": "fixtures/random_29.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 12195995521320448702
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11570871483800784260
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11570871483800784260
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_30",
          "nodes": 3,
          "edges": 2,
          "input_sha256": "3afed7e40c49191ae315532de4f3c48e9ec466c1d8bf24b589412fb11d67f334",
          "native_json": "fixtures/random_30.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 5247242255190488182
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 17436917186296829782
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 17436917186296829782
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_31",
          "nodes": 7,
          "edges": 19,
          "input_sha256": "5f9de875a0b3a25fece2d9774cf641e8d52fd8a7cc005259e899541f334a4596",
          "native_json": "fixtures/random_31.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 8637538703190335550
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 5452759782115167050
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 5452759782115167050
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_32",
          "nodes": 15,
          "edges": 214,
          "input_sha256": "61da20d8e51e33f1b150fe1c67b1b1aa72ddc2ed532792e8d415085aefee9726",
          "native_json": "fixtures/random_32.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 615731795747517251
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 16360244860434357109
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 16360244860434357109
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_33",
          "nodes": 16,
          "edges": 221,
          "input_sha256": "48965d7bccba934bc75f7f28f729be29c85b3398cebcd85f745466f2084a21f2",
          "native_json": "fixtures/random_33.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 17781897524818458121
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 305296219257019215
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 305296219257019215
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_34",
          "nodes": 17,
          "edges": 229,
          "input_sha256": "f26ef1297e3b60e0d7c8191ba360f1067e6ea310d1f841906159960c3983beb7",
          "native_json": "fixtures/random_34.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 11906479051428177006
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 133427470624457144
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 133427470624457144
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_35",
          "nodes": 31,
          "edges": 566,
          "input_sha256": "8bd1df4a3ea80bad85e7f8b90906876c5f5f8360e89c28b5d34c78feb98c9ea7",
          "native_json": "fixtures/random_35.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 14348390467614960643
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 12723768410895455633
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 12723768410895455633
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_36",
          "nodes": 32,
          "edges": 418,
          "input_sha256": "c758a753a24d85eedd6d939324f84d1384d575d5dc1a0bca78869933352d5858",
          "native_json": "fixtures/random_36.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 2689684201023061881
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 8599051761961969309
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 8599051761961969309
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_37",
          "nodes": 33,
          "edges": 551,
          "input_sha256": "06fef7e8870b846f490f2bb9d7d5156590d78e067e31e8fa8d4844fdaf3a639a",
          "native_json": "fixtures/random_37.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 15140976715926322910
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 6668111659645825402
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 6668111659645825402
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_38",
          "nodes": 47,
          "edges": 752,
          "input_sha256": "1336fadb13867763a2f33c992a74fdfe4c1561756c9ab8dc8f58e6baa5524da6",
          "native_json": "fixtures/random_38.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 5430610765907325827
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 14668010938316431039
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 14668010938316431039
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_39",
          "nodes": 48,
          "edges": 893,
          "input_sha256": "313e497bf284782fa10bf153acc7c01578a4a674c3ba19dc208e7f28cc833b21",
          "native_json": "fixtures/random_39.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 16460632226177304526
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 664518870143142018
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 664518870143142018
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_40",
          "nodes": 63,
          "edges": 1041,
          "input_sha256": "97b733ee5e217d5e09f56dfba7ab9c4b97e82f5de6028087b2cf47bfcdc641b7",
          "native_json": "fixtures/random_40.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 14948070509728317059
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 17559906074278205651
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 17559906074278205651
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_41",
          "nodes": 64,
          "edges": 1109,
          "input_sha256": "6d70fd0eb189f7914dc58a219c5d52d8ba268e1e00787c55b0e6474c17377289",
          "native_json": "fixtures/random_41.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_42",
          "nodes": 1,
          "edges": 0,
          "input_sha256": "25fb090da0cc6d2cb58dfddcc8a47d6b09e925bc41569639029e60da57a4fe7a",
          "native_json": "fixtures/random_42.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11126444148914698056
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11125488673309971922
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11125488673309971922
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_43",
          "nodes": 2,
          "edges": 3,
          "input_sha256": "dc247d0b447d9cfd90186e37959ac3602ecc88fd2b78ede494699508c58d8152",
          "native_json": "fixtures/random_43.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 12195995521320448702
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 12195995521320448702
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 12195995521320448702
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_44",
          "nodes": 3,
          "edges": 14,
          "input_sha256": "5e770081b093e4b45f84a4ee1dfa21516e21b3ed91b3c438143923650cd01de1",
          "native_json": "fixtures/random_44.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 5247242255190488182
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 7398977492957258824
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 7398977492957258824
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_45",
          "nodes": 7,
          "edges": 16,
          "input_sha256": "ffec4cc466086ad6d680d79255e218ef5afdf2f7c8b6dc71bf20730b0ce69a81",
          "native_json": "fixtures/random_45.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 8637538703190335550
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 12129602771379186202
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 12129602771379186202
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_46",
          "nodes": 15,
          "edges": 244,
          "input_sha256": "301de26e6c1c900686d1edc8987a6a92c031ad6d637eda85489755914e12b4cf",
          "native_json": "fixtures/random_46.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 615731795747517251
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4939069285380648989
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4939069285380648989
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_47",
          "nodes": 16,
          "edges": 210,
          "input_sha256": "d16ff12463ccbc653efd6cc3262be4656e3961de74123e9c876f78a22db45b6c",
          "native_json": "fixtures/random_47.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 17781897524818458121
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 6396286268166596013
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 6396286268166596013
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_48",
          "nodes": 17,
          "edges": 297,
          "input_sha256": "96bffd0a97d7ac3638fb35331ace035afbe0e8703b5fc9a7e7b896d435191217",
          "native_json": "fixtures/random_48.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 11906479051428177006
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 4555195176172640932
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 4555195176172640932
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_49",
          "nodes": 31,
          "edges": 481,
          "input_sha256": "c868bf8244918c1e2bf8977ed0778186d9614868e751f87b07939d535a86bdaa",
          "native_json": "fixtures/random_49.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 14348390467614960643
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 4643679163369252319
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 4643679163369252319
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_50",
          "nodes": 32,
          "edges": 587,
          "input_sha256": "683b0b02c1a4b7d09ce088d70c0bcfb8d20fc1a7086ba3cc21e2fec46074d660",
          "native_json": "fixtures/random_50.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 2689684201023061881
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 4827703841831473869
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 4827703841831473869
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_51",
          "nodes": 33,
          "edges": 607,
          "input_sha256": "fe564afadcd0bc7d3a10365ea046e7c40b7e3487b1a6366220ba733e055e8bbd",
          "native_json": "fixtures/random_51.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 15140976715926322910
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 4689203768053804456
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 4689203768053804456
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_52",
          "nodes": 47,
          "edges": 934,
          "input_sha256": "028bcda46497d05d60dfd37794562cdaa8b9eb4221218777fed49f81dd3cb48c",
          "native_json": "fixtures/random_52.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 5430610765907325827
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 7464031208699510743
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 3,
              "status": "PASS",
              "permutation_fnv64": 7464031208699510743
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_53",
          "nodes": 48,
          "edges": 780,
          "input_sha256": "11f973c847eda9c2c6f1af811132904820b07473a693ddde5a225888043e5f9f",
          "native_json": "fixtures/random_53.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 16460632226177304526
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 2626834293184039104
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 2626834293184039104
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_54",
          "nodes": 63,
          "edges": 1181,
          "input_sha256": "5b4f67c8e3c8d6389bf25e092472395d010fa4f0715f1b2dc1abe0bd56c8c6e2",
          "native_json": "fixtures/random_54.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 14948070509728317059
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 15729775078000165601
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 4,
              "status": "PASS",
              "permutation_fnv64": 15729775078000165601
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_55",
          "nodes": 64,
          "edges": 1076,
          "input_sha256": "2887476d1411eaaa46e196e14b3c27685c2c47c58a2d4b4038130f0005f47571",
          "native_json": "fixtures/random_55.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 4953163356653287321
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_56",
          "nodes": 1,
          "edges": 0,
          "input_sha256": "25fb090da0cc6d2cb58dfddcc8a47d6b09e925bc41569639029e60da57a4fe7a",
          "native_json": "fixtures/random_56.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11126444148914698056
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11126444148914698056
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11126444148914698056
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_57",
          "nodes": 2,
          "edges": 5,
          "input_sha256": "28dc525626824ee707df95617a2128ffaffd7d6f6f2d6faa1a690416fd4cd9b8",
          "native_json": "fixtures/random_57.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 12195995521320448702
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11570871483800784260
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 11570871483800784260
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_58",
          "nodes": 3,
          "edges": 3,
          "input_sha256": "72d7cd6744ec9c6a7df654252285b4aef474750d0ad15543dbe813a988112f7c",
          "native_json": "fixtures/random_58.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 5247242255190488182
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 7880229776504200106
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 7880229776504200106
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_59",
          "nodes": 7,
          "edges": 61,
          "input_sha256": "cddcbb000259ab45c494af328f8c02a6ff97f520d22f13744ad137abece6bf9f",
          "native_json": "fixtures/random_59.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 8637538703190335550
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 7158283867870172678
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 7158283867870172678
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_60",
          "nodes": 15,
          "edges": 223,
          "input_sha256": "3b248eb60006be413d0d2fce051a324b72516d0fd1abddfca0d72889d2a54ce4",
          "native_json": "fixtures/random_60.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 615731795747517251
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 14540625679389467517
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 1,
              "status": "PASS",
              "permutation_fnv64": 14540625679389467517
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_61",
          "nodes": 16,
          "edges": 206,
          "input_sha256": "dfa40309735e1bf8a8741aeec6dcf7281e3db02c783d42b5d4b569bdf9ffb1ea",
          "native_json": "fixtures/random_61.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 17781897524818458121
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 17094357414990548821
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 17094357414990548821
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_62",
          "nodes": 17,
          "edges": 272,
          "input_sha256": "13f14611ed45e2638fb90a3cb21a9f159d40dc52662e394a366a7b3879f21c27",
          "native_json": "fixtures/random_62.native.json"
        },
        {
          "status": "PASS",
          "orders": [
            {
              "name": "nodeid",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 11906479051428177006
            },
            {
              "name": "degree",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 16112002986427918960
            },
            {
              "name": "qguard",
              "methods": 5,
              "batches": 2,
              "status": "PASS",
              "permutation_fnv64": 16112002986427918960
            }
          ],
          "scope": "All emitted row/source plans, boundary traces and aggregate fields compared against actual cursor execution. No intradrain peak/cache/DRAM/performance inference.",
          "fixture": "random_63",
          "nodes": 31,
          "edges": 569,
          "input_sha256": "8972bd832ab1822c3166ced2ae4e6086d5ddcd6123840c4e09f3b7f37d40c60b",
          "native_json": "fixtures/random_63.native.json"
        }
      ],
      "malformed": [
        {
          "case": "truncated_header",
          "status": "REJECTED",
          "returncode": 2
        },
        {
          "case": "truncated_payload",
          "status": "REJECTED",
          "returncode": 2
        },
        {
          "case": "wrong_version",
          "status": "REJECTED",
          "returncode": 2
        },
        {
          "case": "non_square",
          "status": "REJECTED",
          "returncode": 2
        },
        {
          "case": "rowptr_out_of_bounds",
          "status": "REJECTED",
          "returncode": 2
        },
        {
          "case": "column_out_of_bounds",
          "status": "REJECTED",
          "returncode": 2
        }
      ],
      "mathematical_structure_only": true,
      "fixture_count": 81,
      "directed_fixtures": 17,
      "random_fixtures": 64,
      "malformed_count": 6,
      "script_sha256": "ea75cff52b94b7c9f655c0c765434670ef981d55be2f667efcab962b73ba51c4"
    },
    "source_manifest_sha256": "049cd4a8dfe6cca25b01a4aa6ad97f9f071da415ae0351a51d70c5e0cf278001",
    "node": "qhcn006",
    "job": "10898893",
    "cpu_affinity": [
      24
    ],
    "amx_performance_claim": false,
    "desktop_write": false
  },
  "node": "qhcn006",
  "job": "10898893",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Graph-only diagnostic; no timing/performance interpretation. All malformed fixtures retained as expected rejects, any unexpected failure stops interpretation.",
  "next": "Independently analyze real graph state/resumption tradeoffs; compare ID and simple delayed consumers before developing a new kernel.",
  "date": "2026-10-08T08:17:53.702378+00:00"
}
```

## frontier-structure-analysis-20261008-01

```json
{
  "id": "frontier-structure-analysis-20261008-01",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/frontier_structure_20261008/analysis-01",
  "audit": {
    "status": "PASS",
    "job": "10898893",
    "node": "qhcn006",
    "slurm_state": "COMPLETED",
    "exit_code": "0:0",
    "oracle_fixtures": 81,
    "expected_malformed_rejects": 6,
    "graphs": 4,
    "plans": 60,
    "manifest_sha256": "049cd4a8dfe6cca25b01a4aa6ad97f9f071da415ae0351a51d70c5e0cf278001",
    "completion_sha256": "091a2d336e6410287ab4e7dad3218e4a0ab24c05e663c2de7700d1fbbbbe9d25",
    "rows_complete_policy_is_baseline": true,
    "scope": "STRUCTURE_ONLY",
    "paper_performance_eligible": false,
    "desktop_write": false
  },
  "findings": [
    {
      "graph": "mycielskian19",
      "n": 393215,
      "e": 903194710,
      "full_h1_bytes": 100663040,
      "source": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01/compute-10898893/mycielskian19/structure.json",
      "source_sha256": "a58c7146e1fd732a78343d89eb8beffca1bfcf02834db3395ead4a4dc3dbdf62",
      "orders": {
        "nodeid": {
          "forced_peak_bytes": 0,
          "forced_peak_fraction": 0.0,
          "row_complete_peak_bytes": 100666112,
          "row_complete_fraction": 1.0000305176557354,
          "row_complete_dominance": [
            {
              "other": "checkpoint_1",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_4",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_16",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_ALL",
              "no_worse_in_three_metrics": false,
              "strictly_dominates": false
            }
          ],
          "s64_window_proxy_ratio_vs_degree": 2.5262022607183465
        },
        "degree": {
          "forced_peak_bytes": 100655104,
          "forced_peak_fraction": 0.9999211627226835,
          "row_complete_peak_bytes": 100662784,
          "row_complete_fraction": 0.999997456862022,
          "row_complete_dominance": [
            {
              "other": "checkpoint_1",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_4",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_16",
              "no_worse_in_three_metrics": false,
              "strictly_dominates": false
            },
            {
              "other": "checkpoint_ALL",
              "no_worse_in_three_metrics": false,
              "strictly_dominates": false
            }
          ],
          "s64_window_proxy_ratio_vs_degree": 1.0
        },
        "qguard": {
          "forced_peak_bytes": 100655104,
          "forced_peak_fraction": 0.9999211627226835,
          "row_complete_peak_bytes": 100662784,
          "row_complete_fraction": 0.999997456862022,
          "row_complete_dominance": [
            {
              "other": "checkpoint_1",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_4",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_16",
              "no_worse_in_three_metrics": false,
              "strictly_dominates": false
            },
            {
              "other": "checkpoint_ALL",
              "no_worse_in_three_metrics": false,
              "strictly_dominates": false
            }
          ],
          "s64_window_proxy_ratio_vs_degree": 1.0
        }
      }
    },
    {
      "graph": "reddit",
      "n": 232965,
      "e": 114615892,
      "full_h1_bytes": 59639040,
      "source": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01/compute-10898893/reddit/structure.json",
      "source_sha256": "8865defe056ebe8abcf59809ec4213cfb37b93b64368f920f57803c2f33833ee",
      "orders": {
        "nodeid": {
          "forced_peak_bytes": 0,
          "forced_peak_fraction": 0.0,
          "row_complete_peak_bytes": 59166464,
          "row_complete_fraction": 0.9920760629279076,
          "row_complete_dominance": [
            {
              "other": "checkpoint_1",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_4",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_16",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_ALL",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            }
          ],
          "s64_window_proxy_ratio_vs_degree": 4.703208600716726
        },
        "degree": {
          "forced_peak_bytes": 58945024,
          "forced_peak_fraction": 0.9883630588285794,
          "row_complete_peak_bytes": 59295232,
          "row_complete_fraction": 0.9942351855428927,
          "row_complete_dominance": [
            {
              "other": "checkpoint_1",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_4",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_16",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_ALL",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            }
          ],
          "s64_window_proxy_ratio_vs_degree": 1.0
        },
        "qguard": {
          "forced_peak_bytes": 58943488,
          "forced_peak_fraction": 0.9883373038868499,
          "row_complete_peak_bytes": 59297280,
          "row_complete_fraction": 0.9942695254651986,
          "row_complete_dominance": [
            {
              "other": "checkpoint_1",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_4",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_16",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_ALL",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            }
          ],
          "s64_window_proxy_ratio_vs_degree": 1.0
        }
      }
    },
    {
      "graph": "roadNet-CA",
      "n": 1971281,
      "e": 5533214,
      "full_h1_bytes": 504647936,
      "source": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01/compute-10898893/roadNet-CA/structure.json",
      "source_sha256": "33869c1729a7a17382fcd58d8832ec107e9197594c08eb3cb139902459438ee6",
      "orders": {
        "nodeid": {
          "forced_peak_bytes": 0,
          "forced_peak_fraction": 0.0,
          "row_complete_peak_bytes": 5935104,
          "row_complete_fraction": 0.011760880361551701,
          "row_complete_dominance": [
            {
              "other": "checkpoint_1",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_4",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_16",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_ALL",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            }
          ],
          "s64_window_proxy_ratio_vs_degree": 1.0030612161821097
        },
        "degree": {
          "forced_peak_bytes": 197739776,
          "forced_peak_fraction": 0.39183708461655137,
          "row_complete_peak_bytes": 258183680,
          "row_complete_fraction": 0.5116114851205891,
          "row_complete_dominance": [
            {
              "other": "checkpoint_1",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_4",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_16",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_ALL",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            }
          ],
          "s64_window_proxy_ratio_vs_degree": 1.0
        },
        "qguard": {
          "forced_peak_bytes": 197739776,
          "forced_peak_fraction": 0.39183708461655137,
          "row_complete_peak_bytes": 258183680,
          "row_complete_fraction": 0.5116114851205891,
          "row_complete_dominance": [
            {
              "other": "checkpoint_1",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_4",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_16",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_ALL",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            }
          ],
          "s64_window_proxy_ratio_vs_degree": 1.0
        }
      }
    },
    {
      "graph": "amazon0601",
      "n": 403394,
      "e": 3387388,
      "full_h1_bytes": 103268864,
      "source": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01/compute-10898893/amazon0601/structure.json",
      "source_sha256": "dc8072b139ba20ff24a960cffd6efebbe5ed80f982e7e75a6a37d8be0196d72c",
      "orders": {
        "nodeid": {
          "forced_peak_bytes": 0,
          "forced_peak_fraction": 0.0,
          "row_complete_peak_bytes": 40876800,
          "row_complete_fraction": 0.3958288918526304,
          "row_complete_dominance": [
            {
              "other": "checkpoint_1",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_4",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_16",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_ALL",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            }
          ],
          "s64_window_proxy_ratio_vs_degree": 1.0023455514033552
        },
        "degree": {
          "forced_peak_bytes": 65847808,
          "forced_peak_fraction": 0.637634669826522,
          "row_complete_peak_bytes": 67238912,
          "row_complete_fraction": 0.6511053709276786,
          "row_complete_dominance": [
            {
              "other": "checkpoint_1",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_4",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_16",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_ALL",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            }
          ],
          "s64_window_proxy_ratio_vs_degree": 1.0
        },
        "qguard": {
          "forced_peak_bytes": 65847808,
          "forced_peak_fraction": 0.637634669826522,
          "row_complete_peak_bytes": 67238912,
          "row_complete_fraction": 0.6511053709276786,
          "row_complete_dominance": [
            {
              "other": "checkpoint_1",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_4",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_16",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            },
            {
              "other": "checkpoint_ALL",
              "no_worse_in_three_metrics": true,
              "strictly_dominates": true
            }
          ],
          "s64_window_proxy_ratio_vs_degree": 1.0
        }
      }
    }
  ],
  "paper_eligible": false,
  "issues": "Negative opportunity evidence: frequent partial consumption can increase both boundary state and logical resume payload. Byte-batch area is not elapsed time, ready count is abstract, and source E*256 is not measured traffic.",
  "decision": "Do not develop complex cross-layer runtime from these counts alone. Compare simple row-complete policy and seek valid mixed-state lower-bound certificates before further implementation.",
  "next": "Evaluate source/partial capacity-two matching certificates at fixed production frontiers. No claim about arbitrary reorderings or a performance speedup.",
  "date": "2026-10-08T08:26:21.582522+00:00"
}
```

## frontier-certificate-prepared-20261008

```json
{
  "id": "frontier-certificate-prepared-20261008",
  "status": "PREPARED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-certificate-20261008-01",
  "manifest": {
    "source_sha256": {
      "frontier_structure.cpp": "2d7a8ae89203a08f40e24abaf65d21a0c3dfd418a82632bcdc5bb01d23f0f511",
      "grouping_schedule.hpp": "2dc2a0a978c87959b5523a1ae11e29e74b36e1f447d4f2dbd3dcb62b8d8dab45",
      "frontier_prior_inputs.json": "53c981305c7cc7db4ea368a54288309517053c75df9f3eec7ecd41659028d5cf",
      "certificate_01/PROTOCOL.md": "1d7eabd042e576c9bc8540610394a0952fafee7e7db02fb49835d2ed2f7ebfef",
      "certificate_01/frontier_certificate.cpp": "c235e7356f1e8de0ba52d20bcd7c96096b056f045bbc9de483c4db84cce6cf16",
      "certificate_01/verify_certificate.py": "88dfa847e5a836f8232be8d388e370ea59439cc7780074ef5f968edb7d149155",
      "certificate_01/run_certificate.py": "0022b77346a166e7ed55216c953c3f0f39aa5f0c0f241b6aed56a9a02a150eb3",
      "certificate_01/certificate_job.sh": "2e4ad7675c801d647d98a4c242c2de1f4cc99db281cc5eac57a2e16b8cc06435",
      "certificate_01/stage_certificate.py": "bef585cc97902314edb4ae735f4a9cd60f0cb23b5a65a3a35625264038154704"
    },
    "authority_sha256": {
      "/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md": "55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7",
      "/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md": "641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca"
    },
    "contract_sha256": "b6419e5106278106da6727c90f4cfa185cc70c587d70b4087c0bc286fd0dc244",
    "authority_spec_sha256": {
      "MATH_SPEC_V2.md": "1938d301e3fc913766744af166c0a41b1cb767d7d7419af9c098d3de955cf266",
      "KERNEL_LAYOUT_V2.md": "732e859128baa3cf87813df82cf7562d14358fc6e1cf17c32ec0dde5a743595f",
      "PYTORCH_EXTENSION_SPEC.md": "a3784dd66d4986bde33a40d523f761ee8956da9bbbe711f1707d13d8d2d82df2",
      "EXPERIMENT_SPEC_V2.md": "ecb8fe1c394d9ec47aae97d8f5b87c5b948963c7e8c8b25d70128483c7c0d10d"
    },
    "parent_evidence_sha256": {
      "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01/snapshot_manifest.json": "049cd4a8dfe6cca25b01a4aa6ad97f9f071da415ae0351a51d70c5e0cf278001",
      "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01/compute-10898893/completion.json": "091a2d336e6410287ab4e7dad3218e4a0ab24c05e663c2de7700d1fbbbbe9d25",
      "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01/compute-10898893/mycielskian19/structure.json": "a58c7146e1fd732a78343d89eb8beffca1bfcf02834db3395ead4a4dc3dbdf62",
      "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01/compute-10898893/mycielskian19/status.json": "0690a765106e9aa9e0620e264117a3dc1d530ab73cd689d6603b40f871321df9",
      "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01/compute-10898893/reddit/structure.json": "8865defe056ebe8abcf59809ec4213cfb37b93b64368f920f57803c2f33833ee",
      "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01/compute-10898893/reddit/status.json": "6c57fde3e3bf38f8e7690536c03da4e43a704043a3a911e64569107ba19e7766"
    },
    "requested_nodes_before": 0,
    "requested_nodes_this_study": 1,
    "user_cap": 10,
    "desktop_write": false,
    "numa_interleave": false,
    "scope": "RESTRICTED_STATE_LOWER_BOUND"
  },
  "paper_eligible": false,
  "issues": "No new major issue. Capacity-two matching certifies fixed-order restricted state only.",
  "next": "One CPU small self-test plus two real graph certificate construction and independent verification.",
  "date": "2026-10-08T08:39:25.244487+00:00"
}
```

## frontier-certificate-submitted-10899017

```json
{
  "id": "frontier-certificate-submitted-10899017",
  "status": "SUBMITTED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-certificate-20261008-01",
  "job": "10899017",
  "paper_eligible": false,
  "desktop_write": false,
  "issues": "No new major issue. No feature computation, performance conclusion, or baseline modification.",
  "next": "Check oracle mutations, CSR-bound matching certificates and Slurm completion.",
  "date": "2026-10-08T08:39:25.313958+00:00"
}
```

## frontier-certificate-20261008-10899017

```json
{
  "id": "frontier-certificate-20261008-10899017",
  "status": "FAILED",
  "exit_code": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-certificate-20261008-01/compute-10899017",
  "completion": {},
  "node": "qhcn006",
  "job": "10899017",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Restricted state lower bound only; fixed node-ID order. No hardware speedup or unrestricted optimality claim.",
  "next": "Check Slurm and certificate PASS; compare verified lower bound to frozen constructive state peak, then decide whether complex state selection warrants a runtime.",
  "date": "2026-10-08T08:39:33.268891+00:00"
}
```

## frontier-certificate-link-failure-reviewed-10899017

```json
{
  "id": "frontier-certificate-link-failure-reviewed-10899017",
  "status": "FAILED_BUILD_RETAINED",
  "job": "10899017",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-certificate-20261008-01/compute-10899017",
  "paper_eligible": false,
  "issues": "Build failed before numerical or graph runs: icpx with GCC 8 libstdc++ requires explicit -lstdc++fs for std::filesystem. compile_certificate.stderr.log contains unresolved filesystem symbols. No source/kernel logic changed.",
  "slurm_accounting": "10899017|FAILED|1:0|00:00:08|qhcn006\n10899017.batch|FAILED|1:0|00:00:08|qhcn006\n10899017.extern|COMPLETED|0:0|00:00:08|qhcn006\n10899017.0|FAILED|1:0|00:00:07|qhcn006\n",
  "next": "Retry unique certificate_02/frontier-certificate-20261008-02 with explicit -lstdc++fs; retain all old files; rerun both binaries and complete self-tests.",
  "date": "2026-10-08T08:41:43.268874+00:00"
}
```

## frontier-certificate-prepared-20261008-02

```json
{
  "id": "frontier-certificate-prepared-20261008-02",
  "status": "PREPARED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-certificate-20261008-02",
  "manifest": {
    "source_sha256": {
      "frontier_structure.cpp": "2d7a8ae89203a08f40e24abaf65d21a0c3dfd418a82632bcdc5bb01d23f0f511",
      "grouping_schedule.hpp": "2dc2a0a978c87959b5523a1ae11e29e74b36e1f447d4f2dbd3dcb62b8d8dab45",
      "frontier_prior_inputs.json": "53c981305c7cc7db4ea368a54288309517053c75df9f3eec7ecd41659028d5cf",
      "certificate_02/PROTOCOL.md": "fbf67d2296dccf2556d68277481f31a98ecc1a0d11301cb1a02911c7b4ceb2ed",
      "certificate_02/frontier_certificate.cpp": "c235e7356f1e8de0ba52d20bcd7c96096b056f045bbc9de483c4db84cce6cf16",
      "certificate_02/verify_certificate.py": "88dfa847e5a836f8232be8d388e370ea59439cc7780074ef5f968edb7d149155",
      "certificate_02/run_certificate.py": "8b5273f907b705fb99c170b1a5cf56fd9fec6643717cae3ed6e952be22ff7a00",
      "certificate_02/certificate_job.sh": "fdb7063773eb5e2ba2a62b6cd4923a198cd3bc8c8625998ec044a6caecb49aa6",
      "certificate_02/stage_certificate.py": "d2f7e23fc117c245aa775af8fa72374e417707e2b02ba933f3afd67cfafdbef0"
    },
    "authority_sha256": {
      "/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md": "55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7",
      "/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md": "641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca"
    },
    "contract_sha256": "b6419e5106278106da6727c90f4cfa185cc70c587d70b4087c0bc286fd0dc244",
    "authority_spec_sha256": {
      "MATH_SPEC_V2.md": "1938d301e3fc913766744af166c0a41b1cb767d7d7419af9c098d3de955cf266",
      "KERNEL_LAYOUT_V2.md": "732e859128baa3cf87813df82cf7562d14358fc6e1cf17c32ec0dde5a743595f",
      "PYTORCH_EXTENSION_SPEC.md": "a3784dd66d4986bde33a40d523f761ee8956da9bbbe711f1707d13d8d2d82df2",
      "EXPERIMENT_SPEC_V2.md": "ecb8fe1c394d9ec47aae97d8f5b87c5b948963c7e8c8b25d70128483c7c0d10d"
    },
    "parent_evidence_sha256": {
      "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01/snapshot_manifest.json": "049cd4a8dfe6cca25b01a4aa6ad97f9f071da415ae0351a51d70c5e0cf278001",
      "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01/compute-10898893/completion.json": "091a2d336e6410287ab4e7dad3218e4a0ab24c05e663c2de7700d1fbbbbe9d25",
      "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01/compute-10898893/mycielskian19/structure.json": "a58c7146e1fd732a78343d89eb8beffca1bfcf02834db3395ead4a4dc3dbdf62",
      "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01/compute-10898893/mycielskian19/status.json": "0690a765106e9aa9e0620e264117a3dc1d530ab73cd689d6603b40f871321df9",
      "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01/compute-10898893/reddit/structure.json": "8865defe056ebe8abcf59809ec4213cfb37b93b64368f920f57803c2f33833ee",
      "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-structure-20261008-01/compute-10898893/reddit/status.json": "6c57fde3e3bf38f8e7690536c03da4e43a704043a3a911e64569107ba19e7766"
    },
    "requested_nodes_before": 0,
    "requested_nodes_this_study": 1,
    "user_cap": 10,
    "desktop_write": false,
    "numa_interleave": false,
    "scope": "RESTRICTED_STATE_LOWER_BOUND"
  },
  "paper_eligible": false,
  "issues": "No new major issue. Capacity-two matching certifies fixed-order restricted state only.",
  "next": "One CPU small self-test plus two real graph certificate construction and independent verification.",
  "date": "2026-10-08T08:42:06.538527+00:00"
}
```

## frontier-certificate-submitted-10899028

```json
{
  "id": "frontier-certificate-submitted-10899028",
  "status": "SUBMITTED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-certificate-20261008-02",
  "job": "10899028",
  "paper_eligible": false,
  "desktop_write": false,
  "issues": "No new major issue. No feature computation, performance conclusion, or baseline modification.",
  "next": "Check oracle mutations, CSR-bound matching certificates and Slurm completion.",
  "date": "2026-10-08T08:42:06.620690+00:00"
}
```

## frontier-certificate-20261008-10899028

```json
{
  "id": "frontier-certificate-20261008-10899028",
  "status": "PASS",
  "exit_code": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/frontier-certificate-20261008-02/compute-10899028",
  "completion": {
    "status": "PASS",
    "scope": "RESTRICTED_STATE_LOWER_BOUND",
    "graphs": [
      {
        "graph": "reddit",
        "status": "PASS",
        "frontiers": [
          3640,
          7280,
          10920,
          14510,
          14511,
          14512
        ],
        "input_sha256": "26ac2ddc04e52796ba175721fee4e376fcde46bff6ca6a8a4922cce6a296617a",
        "verified_sha256": "565209e75ff90e9c29d981f8f4289fddd5e3dbded4ffa09915b4c45c3b58a6ce",
        "summary_sha256": "655fe28b2e7f418fc572c7082b6291574ffe7235c838af717a5a1f4086586afb"
      },
      {
        "graph": "mycielskian19",
        "status": "PASS",
        "frontiers": [
          6144,
          12288,
          18432,
          24574,
          24575,
          24576
        ],
        "input_sha256": "e685700b64a07b5d871461ff9438126d4b4782f7bc57f25ed7045e250610cefe",
        "verified_sha256": "eb130415623062c860d850e5f191652ad9ffde974c356effd872fe33967ae569",
        "summary_sha256": "93d3b1f726f05cadd88a85020d2c81f85c282dc17334a6f4aeea1aead0bc2411"
      }
    ],
    "self_test": {
      "schema_version": "certificate-self-test-v1",
      "status": "PASS",
      "valid": [
        {
          "fixture": "capacity_two",
          "status": "PASS",
          "frontiers": 3
        },
        {
          "fixture": "repeated",
          "status": "PASS",
          "frontiers": 3
        },
        {
          "fixture": "empty",
          "status": "PASS",
          "frontiers": 2
        },
        {
          "fixture": "self17",
          "status": "PASS",
          "frontiers": 3
        },
        {
          "fixture": "cycle33",
          "status": "PASS",
          "frontiers": 3
        }
      ],
      "mutations": [
        {
          "case": "duplicate_source",
          "status": "REJECTED",
          "reason": "source capacity exceeded"
        },
        {
          "case": "destination_overcapacity",
          "status": "REJECTED",
          "reason": "destination capacity exceeded"
        },
        {
          "case": "unproduced_source",
          "status": "REJECTED",
          "reason": "source not produced"
        },
        {
          "case": "completed_destination",
          "status": "REJECTED",
          "reason": "destination already complete"
        },
        {
          "case": "wrong_offset",
          "status": "REJECTED",
          "reason": "offset does not belong to destination"
        }
      ]
    },
    "source_manifest_sha256": "57793a2fe1462175ff97da929de7ab473f54819c3b252b7e6c8616cba160072d",
    "node": "qhcn006",
    "job": "10899028",
    "cpu_affinity": [
      24
    ],
    "desktop_write": false,
    "numa_interleave": false,
    "performance_claim": false
  },
  "node": "qhcn006",
  "job": "10899028",
  "partition": "intel",
  "paper_eligible": false,
  "issues": "Restricted state lower bound only; fixed node-ID order. No hardware speedup or unrestricted optimality claim.",
  "next": "Check Slurm and certificate PASS; compare verified lower bound to frozen constructive state peak, then decide whether complex state selection warrants a runtime.",
  "date": "2026-10-08T08:42:44.371419+00:00"
}
```

## frontier-certificate-analysis-20261008-02

```json
{
  "id": "frontier-certificate-analysis-20261008-02",
  "status": "ANALYZED_PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/frontier_structure_20261008/certificate-02",
  "audit_sha256": "d3907ffba0a62cd32c14cb47c211cecf7fc2e9d23b96a06df109a22019d59272",
  "report_sha256": "857c66a0397616fbfba3750b0aab26a92f6e7fe792c34bffe300d7d3594fb93b",
  "scope": "GRAPH_STRUCTURE_AND_RESTRICTED_STATE_CERTIFICATE",
  "paper_eligible": false,
  "bounds": [
    {
      "graph": "reddit",
      "frontier": 14510,
      "certified_peak_lower_bound_bytes": 56528640,
      "constructive_postdrain_peak_upper_bytes": 59162368,
      "lower_over_upper": 0.9554830530109951,
      "max_possible_postdrain_peak_saving_fraction": 0.04451694698900488,
      "saving_is_upper_bound_not_achieved": true,
      "globally_optimal_order_claim": false,
      "restricted_representation": true,
      "certified_frontiers": 6
    },
    {
      "graph": "mycielskian19",
      "frontier": 24574,
      "certified_peak_lower_bound_bytes": 68715264,
      "constructive_postdrain_peak_upper_bytes": 100662272,
      "lower_over_upper": 0.6826317609838968,
      "max_possible_postdrain_peak_saving_fraction": 0.3173682390161032,
      "saving_is_upper_bound_not_achieved": true,
      "globally_optimal_order_claim": false,
      "restricted_representation": true,
      "certified_frontiers": 6
    }
  ],
  "issues": "Retained link failure10899017 resolved by explicit -lstdc++fs in immutable retry10899028. New negative research result: Reddit fixed-order source/partial peak has at most 4.4517% saving relative to simple postdrain upper bound; Myc lower bound is not tight. No hardware speedup proven.",
  "next": "Do not develop periodic prefix-drain runtime or scan checkpoint sizes. Only investigate reachable state transformations where a certified gap remains, or explicit recomputation that counts additional AVX/AMX work; compare against row-complete baseline and direct priors.",
  "desktop_write": false,
  "numa_interleave": false,
  "date": "2026-10-08T08:48:32.068515+00:00"
}
```

## recompute-audit-prepared-20261008

```json
{
  "id": "recompute-audit-prepared-20261008",
  "status": "PREPARED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-audit-20261008-01",
  "manifest": {
    "source_sha256": {
      "PROTOCOL.md": "6a20ac2f695a4d74cc7cfc285c174feda43738b765c0c79d2b63abc939f5bf22",
      "recompute_audit.py": "518c403c04df4921f3b70d14e51dc58a8805ea6b564efb91cc9df5ebde043f72",
      "audit_job.sh": "c8bcdfad5e71d68dcb5f8fd95194eef858f0073e922d3a0f65ca58ab322793a0",
      "stage.py": "6272ae736771ae840f5686cf55dfa26b9c31987883dac0cb1d2ddd67f616a6b7"
    },
    "authority_sha256": {
      "/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md": "55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7",
      "/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md": "641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca"
    },
    "authority_spec_sha256": {
      "MATH_SPEC_V2.md": "1938d301e3fc913766744af166c0a41b1cb767d7d7419af9c098d3de955cf266",
      "KERNEL_LAYOUT_V2.md": "732e859128baa3cf87813df82cf7562d14358fc6e1cf17c32ec0dde5a743595f",
      "PYTORCH_EXTENSION_SPEC.md": "a3784dd66d4986bde33a40d523f761ee8956da9bbbe711f1707d13d8d2d82df2",
      "EXPERIMENT_SPEC_V2.md": "ecb8fe1c394d9ec47aae97d8f5b87c5b948963c7e8c8b25d70128483c7c0d10d"
    },
    "contract_sha256": "b6419e5106278106da6727c90f4cfa185cc70c587d70b4087c0bc286fd0dc244",
    "requested_nodes_this_study": 1,
    "user_cap": 10,
    "numa_interleave": false,
    "desktop_write": false,
    "scope": "LOGICAL_RECOMPUTATION_COST"
  },
  "paper_eligible": false,
  "issues": "No new major issue; logical cost audit only.",
  "next": "One shared-node read-only CSR scan for four frozen graphs.",
  "date": "2026-10-08T09:21:45.036571+00:00"
}
```

## recompute-audit-submitted-10899639

```json
{
  "id": "recompute-audit-submitted-10899639",
  "status": "SUBMITTED",
  "job": "10899639",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-audit-20261008-01",
  "paper_eligible": false,
  "desktop_write": false,
  "issues": "No feature or performance kernel.",
  "next": "Check completion and compare logical extra edge work with source-state release.",
  "date": "2026-10-08T09:21:45.222375+00:00"
}
```

## recompute-audit-20261008-10899639

```json
{
  "id": "recompute-audit-20261008-10899639",
  "status": "FAILED",
  "exit_code": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-audit-20261008-01/compute-10899639",
  "completion": {},
  "node": "qhcn006",
  "job": "10899639",
  "paper_eligible": false,
  "issues": "Logical recomputation-cost audit only; no speedup or DRAM claim.",
  "next": "Compare extra AVX edge work and source-state release with row-complete baseline before any kernel work.",
  "date": "2026-10-08T09:21:49.830350+00:00"
}
```

## recompute-audit-prepared-20261008

```json
{
  "id": "recompute-audit-prepared-20261008",
  "status": "PREPARED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-audit-20261008-02",
  "manifest": {
    "source_sha256": {
      "PROTOCOL.md": "6a20ac2f695a4d74cc7cfc285c174feda43738b765c0c79d2b63abc939f5bf22",
      "recompute_audit.py": "518c403c04df4921f3b70d14e51dc58a8805ea6b564efb91cc9df5ebde043f72",
      "audit_job.sh": "c8bcdfad5e71d68dcb5f8fd95194eef858f0073e922d3a0f65ca58ab322793a0",
      "stage.py": "6a374bb539f88799d98704427875ac6e899f1cf3c3af960ea25e49d8e4fa2edd",
      "frontier_prior_inputs.json": "53c981305c7cc7db4ea368a54288309517053c75df9f3eec7ecd41659028d5cf"
    },
    "authority_sha256": {
      "/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md": "55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7",
      "/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md": "641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca"
    },
    "authority_spec_sha256": {
      "MATH_SPEC_V2.md": "1938d301e3fc913766744af166c0a41b1cb767d7d7419af9c098d3de955cf266",
      "KERNEL_LAYOUT_V2.md": "732e859128baa3cf87813df82cf7562d14358fc6e1cf17c32ec0dde5a743595f",
      "PYTORCH_EXTENSION_SPEC.md": "a3784dd66d4986bde33a40d523f761ee8956da9bbbe711f1707d13d8d2d82df2",
      "EXPERIMENT_SPEC_V2.md": "ecb8fe1c394d9ec47aae97d8f5b87c5b948963c7e8c8b25d70128483c7c0d10d"
    },
    "contract_sha256": "b6419e5106278106da6727c90f4cfa185cc70c587d70b4087c0bc286fd0dc244",
    "requested_nodes_this_study": 1,
    "user_cap": 10,
    "numa_interleave": false,
    "desktop_write": false,
    "scope": "LOGICAL_RECOMPUTATION_COST"
  },
  "paper_eligible": false,
  "issues": "No new major issue; logical cost audit only.",
  "next": "One shared-node read-only CSR scan for four frozen graphs.",
  "date": "2026-10-08T09:24:08.873694+00:00"
}
```

## recompute-audit-submitted-10899667

```json
{
  "id": "recompute-audit-submitted-10899667",
  "status": "SUBMITTED",
  "job": "10899667",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-audit-20261008-02",
  "paper_eligible": false,
  "desktop_write": false,
  "issues": "No feature or performance kernel.",
  "next": "Check completion and compare logical extra edge work with source-state release.",
  "date": "2026-10-08T09:24:08.967097+00:00"
}
```

## recompute-audit-20261008-10899667

```json
{
  "id": "recompute-audit-20261008-10899667",
  "status": "FAILED",
  "exit_code": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-audit-20261008-02/compute-10899667",
  "completion": {},
  "node": "qhcn006",
  "job": "10899667",
  "paper_eligible": false,
  "issues": "Logical recomputation-cost audit only; no speedup or DRAM claim.",
  "next": "Compare extra AVX edge work and source-state release with row-complete baseline before any kernel work.",
  "date": "2026-10-08T09:24:12.372862+00:00"
}
```

## recompute-audit-prepared-20261008-03

```json
{
  "id": "recompute-audit-prepared-20261008-03",
  "status": "PREPARED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-audit-20261008-03",
  "manifest": {
    "source_sha256": {
      "PROTOCOL.md": "f30175ba18e05082400b6ed9e781b69b8361ccdf6a70fe13ac490e5ae5f2e723",
      "recompute_audit.py": "54b07362e73e42b7e0c32b5347bc43991e05f35e984ccc82bbd9cd78cbfc90be",
      "audit_job.sh": "c8bcdfad5e71d68dcb5f8fd95194eef858f0073e922d3a0f65ca58ab322793a0",
      "stage.py": "823355de8aa1f168e4e46a196e292d2e19b54a5f769bcae22a2ebb3f0ef690e9",
      "frontier_prior_inputs.json": "53c981305c7cc7db4ea368a54288309517053c75df9f3eec7ecd41659028d5cf"
    },
    "authority_sha256": {
      "/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md": "55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7",
      "/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md": "641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca"
    },
    "authority_spec_sha256": {
      "MATH_SPEC_V2.md": "1938d301e3fc913766744af166c0a41b1cb767d7d7419af9c098d3de955cf266",
      "KERNEL_LAYOUT_V2.md": "732e859128baa3cf87813df82cf7562d14358fc6e1cf17c32ec0dde5a743595f",
      "PYTORCH_EXTENSION_SPEC.md": "a3784dd66d4986bde33a40d523f761ee8956da9bbbe711f1707d13d8d2d82df2",
      "EXPERIMENT_SPEC_V2.md": "ecb8fe1c394d9ec47aae97d8f5b87c5b948963c7e8c8b25d70128483c7c0d10d"
    },
    "contract_sha256": "b6419e5106278106da6727c90f4cfa185cc70c587d70b4087c0bc286fd0dc244",
    "requested_nodes_this_study": 1,
    "user_cap": 10,
    "numa_interleave": false,
    "desktop_write": false,
    "scope": "LOGICAL_RECOMPUTATION_COST"
  },
  "paper_eligible": false,
  "issues": "No new major issue; logical cost audit only.",
  "next": "One shared-node read-only CSR scan for four frozen graphs.",
  "date": "2026-10-08T09:26:27.564782+00:00"
}
```

## recompute-audit-submitted-10899674

```json
{
  "id": "recompute-audit-submitted-10899674",
  "status": "SUBMITTED",
  "job": "10899674",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-audit-20261008-03",
  "paper_eligible": false,
  "desktop_write": false,
  "issues": "No feature or performance kernel.",
  "next": "Check completion and compare logical extra edge work with source-state release.",
  "date": "2026-10-08T09:26:27.683494+00:00"
}
```

## recompute-audit-20261008-10899674

```json
{
  "id": "recompute-audit-20261008-10899674",
  "status": "FAILED",
  "exit_code": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-audit-20261008-03/compute-10899674",
  "completion": {},
  "node": "qhcn006",
  "job": "10899674",
  "paper_eligible": false,
  "issues": "Logical recomputation-cost audit only; no speedup or DRAM claim.",
  "next": "Compare extra AVX edge work and source-state release with row-complete baseline before any kernel work.",
  "date": "2026-10-08T09:37:43.356940+00:00"
}
```

## recompute-audit-cancelled-10899674

```json
{
  "id": "recompute-audit-cancelled-10899674",
  "status": "CANCELLED_IMPLEMENTATION",
  "job": "10899674",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-audit-20261008-03/compute-10899674",
  "paper_eligible": false,
  "issues": "Python per-edge loop was too slow for the 903M-edge graph; no audit JSON was produced. Cancelled after 9 minutes before numerical output, preserving logs.",
  "next": "Use a C++ two-pass read-only CSR scanner in a unique retry directory.",
  "date": "2026-10-08T09:38:19.932202+00:00"
}
```

## recompute-audit-prepared-20261008-05

```json
{
  "id": "recompute-audit-prepared-20261008-05",
  "status": "PREPARED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-audit-20261008-05",
  "manifest": {
    "source_sha256": {
      "PROTOCOL.md": "2a0f64227056eacb986149427fb8d07385a9d840d27d979d6bf72c7a49aa89c3",
      "recompute_scan.cpp": "b152653c06773178368b73b3997b54e5f49ad15932195a8b8a57b0acb81abbb3",
      "run.py": "0cfb340b93831bbf81150bb7c19f6c00a12f2b89f34e54820f185dadc0f614db",
      "audit_job.sh": "abe0a2637f1dfc827832720fd4e1d36390c668af9429a90238940074cee51e09",
      "stage.py": "c38cda81fc0ddd75743af5832d8ea4d349a319e7711ae549752650f351df5750",
      "frontier_prior_inputs.json": "53c981305c7cc7db4ea368a54288309517053c75df9f3eec7ecd41659028d5cf"
    },
    "authority_sha256": {
      "/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md": "55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7",
      "/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md": "641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca"
    },
    "authority_spec_sha256": {
      "MATH_SPEC_V2.md": "1938d301e3fc913766744af166c0a41b1cb767d7d7419af9c098d3de955cf266",
      "KERNEL_LAYOUT_V2.md": "732e859128baa3cf87813df82cf7562d14358fc6e1cf17c32ec0dde5a743595f",
      "PYTORCH_EXTENSION_SPEC.md": "a3784dd66d4986bde33a40d523f761ee8956da9bbbe711f1707d13d8d2d82df2",
      "EXPERIMENT_SPEC_V2.md": "ecb8fe1c394d9ec47aae97d8f5b87c5b948963c7e8c8b25d70128483c7c0d10d"
    },
    "contract_sha256": "b6419e5106278106da6727c90f4cfa185cc70c587d70b4087c0bc286fd0dc244",
    "requested_nodes_this_study": 1,
    "user_cap": 10,
    "numa_interleave": false,
    "desktop_write": false,
    "scope": "LOGICAL_RECOMPUTATION_COST"
  },
  "paper_eligible": false,
  "issues": "Retries 01-03 retained; retry05 replaces Python per-edge loop with C++ two-pass scan.",
  "next": "Compile and run one shared-node read-only CSR scan for four frozen graphs.",
  "date": "2026-10-08T09:58:43.908498+00:00"
}
```

## recompute-audit-submitted-10899784

```json
{
  "id": "recompute-audit-submitted-10899784",
  "status": "SUBMITTED",
  "job": "10899784",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-audit-20261008-05",
  "paper_eligible": false,
  "desktop_write": false,
  "issues": "No feature or performance kernel.",
  "next": "Check C++ scan completion and compare extra AVX edge work with source-state release.",
  "date": "2026-10-08T09:58:43.963127+00:00"
}
```

## recompute-audit-20261008-10899784

```json
{
  "id": "recompute-audit-20261008-10899784",
  "status": "PASS",
  "exit_code": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-audit-20261008-05/compute-10899784",
  "completion": {
    "schema": "recompute-audit-v2",
    "status": "PASS",
    "model": "128->128->128",
    "results": [
      {
        "schema": "recompute-audit-cpp-v1",
        "status": "PASS",
        "graph": "amazon0601",
        "csr_path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/amazon0601/amazon0601.csrbin",
        "n": 403394,
        "e": 3387388,
        "batch_rows": 16,
        "batches": 25213,
        "source_rows_with_consumer": 403312,
        "full_h1_bytes": 103247872,
        "extra_regeneration_rows": 1952884,
        "extra_first_layer_edge_visits": 92139244,
        "extra_edge_visit_ratio": 27.2006761552,
        "extra_projection_rows": 1952884,
        "extra_projection_groups_by_rank": 133887,
        "baseline_peak_source_rows": 159654,
        "candidate_peak_source_plus_transient_rows": 20,
        "candidate_peak_rank": 21305,
        "reuse_histogram": {
          "1": 98685,
          "2": 64042,
          "3": 49976,
          "4": 37931,
          "5": 29445,
          "6": 22522,
          "7": 17354,
          "8": 13684,
          "9": 10721,
          "10": 8580,
          "11": 6850,
          "12": 5543,
          "13": 4702,
          "14": 3885,
          "15": 3280,
          "16": 2731,
          "17": 2442,
          "18": 1988,
          "19": 1718,
          "20": 1529,
          "21": 1299,
          "22": 1152,
          "23": 1047,
          "24": 953,
          "25": 857,
          "26": 736,
          "27": 687,
          "28": 629,
          "29": 507,
          "30": 460,
          "31": 476,
          "32": 384,
          "33": 347,
          "34": 335,
          "35": 315,
          "36": 280,
          "37": 277,
          "38": 269,
          "39": 246,
          "40": 192,
          "41": 196,
          "42": 183,
          "43": 154,
          "44": 165,
          "45": 163,
          "46": 166,
          "47": 129,
          "48": 144,
          "49": 109,
          "50": 119,
          "51": 87,
          "52": 103,
          "53": 97,
          "54": 89,
          "55": 106,
          "56": 70,
          "57": 83,
          "58": 68,
          "59": 73,
          "60": 75,
          "61": 61,
          "62": 65,
          "63": 57,
          "64": 44,
          "65": 55,
          "66": 47,
          "67": 51,
          "68": 54,
          "69": 50,
          "70": 32,
          "71": 44,
          "72": 35,
          "73": 36,
          "74": 25,
          "75": 36,
          "76": 33,
          "77": 26,
          "78": 25,
          "79": 21,
          "80": 26,
          "81": 15,
          "82": 21,
          "83": 23,
          "84": 23,
          "85": 17,
          "86": 20,
          "87": 17,
          "88": 13,
          "89": 22,
          "90": 18,
          "91": 26,
          "92": 18,
          "93": 21,
          "94": 15,
          "95": 21,
          "96": 21,
          "97": 15,
          "98": 15,
          "99": 14,
          "100": 20,
          "101": 14,
          "102": 20,
          "103": 11,
          "104": 15,
          "105": 6,
          "106": 13,
          "107": 6,
          "108": 7,
          "109": 16,
          "110": 9,
          "111": 12,
          "112": 8,
          "113": 15,
          "114": 13,
          "115": 9,
          "116": 7,
          "117": 9,
          "118": 13,
          "119": 8,
          "120": 9,
          "121": 12,
          "122": 7,
          "123": 5,
          "124": 7,
          "125": 9,
          "126": 8,
          "127": 10,
          "128": 10,
          "129": 6,
          "130": 7,
          "131": 6,
          "132": 7,
          "133": 7,
          "134": 4,
          "135": 5,
          "137": 5,
          "138": 5,
          "139": 7,
          "140": 8,
          "141": 4,
          "142": 6,
          "143": 5,
          "144": 5,
          "145": 4,
          "146": 7,
          "147": 4,
          "148": 8,
          "149": 7,
          "150": 8,
          "151": 4,
          "152": 2,
          "153": 4,
          "154": 6,
          "155": 3,
          "156": 4,
          "157": 3,
          "158": 4,
          "159": 4,
          "160": 5,
          "161": 3,
          "165": 1,
          "166": 2,
          "167": 5,
          "168": 1,
          "169": 6,
          "170": 1,
          "173": 3,
          "174": 3,
          "175": 3,
          "176": 3,
          "177": 2,
          "178": 2,
          "179": 5,
          "180": 4,
          "181": 1,
          "182": 3,
          "183": 2,
          "185": 1,
          "186": 3,
          "187": 5,
          "188": 1,
          "190": 3,
          "191": 2,
          "194": 4,
          "195": 1,
          "196": 3,
          "197": 3,
          "198": 3,
          "200": 1,
          "201": 1,
          "202": 2,
          "204": 1,
          "206": 4,
          "208": 2,
          "209": 2,
          "210": 2,
          "211": 1,
          "212": 2,
          "213": 2,
          "214": 1,
          "216": 3,
          "217": 1,
          "218": 1,
          "219": 2,
          "221": 2,
          "222": 3,
          "223": 1,
          "224": 1,
          "225": 2,
          "226": 2,
          "227": 4,
          "228": 3,
          "229": 1,
          "230": 1,
          "232": 2,
          "233": 1,
          "234": 1,
          "235": 2,
          "236": 2,
          "238": 1,
          "240": 1,
          "241": 1,
          "242": 2,
          "243": 1,
          "244": 1,
          "246": 1,
          "247": 1,
          "249": 1,
          "250": 1,
          "252": 1,
          "253": 3,
          "254": 1,
          "256": 1,
          "257": 1,
          "258": 1,
          "259": 1,
          "260": 1,
          "262": 1,
          "263": 1,
          "266": 1,
          "267": 1,
          "269": 1,
          "270": 1,
          "271": 1,
          "273": 1,
          "274": 1,
          "275": 2,
          "278": 1,
          "279": 3,
          "283": 1,
          "284": 1,
          "286": 1,
          "291": 1,
          "292": 1,
          "297": 1,
          "300": 1,
          "301": 1,
          "307": 1,
          "308": 2,
          "317": 1,
          "320": 2,
          "328": 1,
          "329": 1,
          "335": 1,
          "336": 1,
          "339": 1,
          "343": 2,
          "348": 1,
          "354": 1,
          "356": 2,
          "358": 1,
          "368": 1,
          "371": 1,
          "376": 1,
          "380": 1,
          "384": 1,
          "393": 1,
          "394": 1,
          "424": 1,
          "425": 1,
          "429": 2,
          "440": 1,
          "471": 1,
          "481": 1,
          "485": 2,
          "497": 1,
          "501": 1,
          "512": 1,
          "539": 1,
          "592": 1,
          "602": 1,
          "674": 1,
          "679": 1,
          "752": 1,
          "946": 1,
          "1420": 1,
          "2063": 1,
          "2227": 1,
          "2472": 1
        },
        "limitations": [
          "logical work only",
          "no DRAM/cache/synchronization",
          "optimistic one regeneration per consumer rank",
          "no legal AMX schedule proof"
        ]
      },
      {
        "schema": "recompute-audit-cpp-v1",
        "status": "PASS",
        "graph": "roadNet-CA",
        "csr_path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/roadNet-CA/roadNet-CA.csrbin",
        "n": 1971281,
        "e": 5533214,
        "batch_rows": 16,
        "batches": 123206,
        "source_rows_with_consumer": 1965206,
        "full_h1_bytes": 503092736,
        "extra_regeneration_rows": 1860117,
        "extra_first_layer_edge_visits": 6280436,
        "extra_edge_visit_ratio": 1.13504303286,
        "extra_projection_rows": 1860117,
        "extra_projection_groups_by_rank": 175379,
        "baseline_peak_source_rows": 23156,
        "candidate_peak_source_plus_transient_rows": 2531,
        "candidate_peak_rank": 69647,
        "reuse_histogram": {
          "1": 718366,
          "2": 714339,
          "3": 453563,
          "4": 77226,
          "5": 1592,
          "6": 114,
          "7": 6
        },
        "limitations": [
          "logical work only",
          "no DRAM/cache/synchronization",
          "optimistic one regeneration per consumer rank",
          "no legal AMX schedule proof"
        ]
      },
      {
        "schema": "recompute-audit-cpp-v1",
        "status": "PASS",
        "graph": "reddit",
        "csr_path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/reddit/reddit.csrbin",
        "n": 232965,
        "e": 114615892,
        "batch_rows": 16,
        "batches": 14561,
        "source_rows_with_consumer": 232965,
        "full_h1_bytes": 59639040,
        "extra_regeneration_rows": 9153623,
        "extra_first_layer_edge_visits": 8853220177,
        "extra_edge_visit_ratio": 77.242518664,
        "extra_projection_rows": 9153623,
        "extra_projection_groups_by_rank": 574733,
        "baseline_peak_source_rows": 231089,
        "candidate_peak_source_plus_transient_rows": 171711,
        "candidate_peak_rank": 12256,
        "reuse_histogram": {
          "1": 2321,
          "2": 2602,
          "3": 2585,
          "4": 2549,
          "5": 2584,
          "6": 2857,
          "7": 2698,
          "8": 2933,
          "9": 2909,
          "10": 3119,
          "11": 3130,
          "12": 3243,
          "13": 3235,
          "14": 3224,
          "15": 3454,
          "16": 3859,
          "17": 3513,
          "18": 3799,
          "19": 3606,
          "20": 3785,
          "21": 4061,
          "22": 3963,
          "23": 3827,
          "24": 3675,
          "25": 3880,
          "26": 3908,
          "27": 3991,
          "28": 4054,
          "29": 3944,
          "30": 3956,
          "31": 3822,
          "32": 3641,
          "33": 3776,
          "34": 3741,
          "35": 3704,
          "36": 3658,
          "37": 3784,
          "38": 3589,
          "39": 3488,
          "40": 3327,
          "41": 3309,
          "42": 3332,
          "43": 3319,
          "44": 2993,
          "45": 2947,
          "46": 2807,
          "47": 2962,
          "48": 2741,
          "49": 2769,
          "50": 2633,
          "51": 2450,
          "52": 2465,
          "53": 2342,
          "54": 2361,
          "55": 2134,
          "56": 2168,
          "57": 2057,
          "58": 1903,
          "59": 1939,
          "60": 1783,
          "61": 1739,
          "62": 1611,
          "63": 1578,
          "64": 1608,
          "65": 1567,
          "66": 1418,
          "67": 1345,
          "68": 1358,
          "69": 1228,
          "70": 1270,
          "71": 1215,
          "72": 1077,
          "73": 1070,
          "74": 1048,
          "75": 979,
          "76": 901,
          "77": 871,
          "78": 871,
          "79": 854,
          "80": 782,
          "81": 720,
          "82": 675,
          "83": 635,
          "84": 641,
          "85": 610,
          "86": 632,
          "87": 563,
          "88": 547,
          "89": 528,
          "90": 446,
          "91": 465,
          "92": 450,
          "93": 447,
          "94": 416,
          "95": 392,
          "96": 384,
          "97": 399,
          "98": 337,
          "99": 360,
          "100": 325,
          "101": 278,
          "102": 292,
          "103": 288,
          "104": 278,
          "105": 253,
          "106": 253,
          "107": 256,
          "108": 303,
          "109": 204,
          "110": 223,
          "111": 187,
          "112": 215,
          "113": 235,
          "114": 203,
          "115": 164,
          "116": 168,
          "117": 191,
          "118": 164,
          "119": 149,
          "120": 137,
          "121": 145,
          "122": 163,
          "123": 111,
          "124": 110,
          "125": 129,
          "126": 101,
          "127": 125,
          "128": 103,
          "129": 118,
          "130": 137,
          "131": 116,
          "132": 102,
          "133": 102,
          "134": 98,
          "135": 107,
          "136": 82,
          "137": 70,
          "138": 97,
          "139": 63,
          "140": 82,
          "141": 74,
          "142": 73,
          "143": 79,
          "144": 60,
          "145": 69,
          "146": 73,
          "147": 64,
          "148": 55,
          "149": 63,
          "150": 60,
          "151": 49,
          "152": 59,
          "153": 55,
          "154": 50,
          "155": 55,
          "156": 40,
          "157": 53,
          "158": 41,
          "159": 46,
          "160": 59,
          "161": 50,
          "162": 37,
          "163": 35,
          "164": 39,
          "165": 26,
          "166": 38,
          "167": 31,
          "168": 38,
          "169": 32,
          "170": 31,
          "171": 30,
          "172": 30,
          "173": 20,
          "174": 23,
          "175": 29,
          "176": 22,
          "177": 24,
          "178": 22,
          "179": 25,
          "180": 23,
          "181": 24,
          "182": 29,
          "183": 22,
          "184": 22,
          "185": 25,
          "186": 21,
          "187": 20,
          "188": 18,
          "189": 16,
          "190": 14,
          "191": 23,
          "192": 13,
          "193": 24,
          "194": 16,
          "195": 27,
          "196": 28,
          "197": 19,
          "198": 12,
          "199": 12,
          "200": 19,
          "201": 12,
          "202": 23,
          "203": 14,
          "204": 9,
          "205": 11,
          "206": 14,
          "207": 16,
          "208": 11,
          "209": 9,
          "210": 5,
          "211": 11,
          "212": 10,
          "213": 7,
          "214": 7,
          "215": 6,
          "216": 8,
          "217": 14,
          "218": 8,
          "219": 6,
          "220": 13,
          "221": 9,
          "222": 9,
          "223": 5,
          "224": 7,
          "225": 14,
          "226": 9,
          "227": 9,
          "228": 4,
          "229": 7,
          "230": 5,
          "231": 9,
          "232": 7,
          "233": 4,
          "234": 8,
          "235": 7,
          "236": 3,
          "237": 7,
          "238": 3,
          "239": 5,
          "240": 5,
          "241": 2,
          "242": 9,
          "243": 4,
          "244": 8,
          "245": 10,
          "246": 5,
          "247": 3,
          "248": 7,
          "249": 7,
          "250": 7,
          "251": 4,
          "252": 3,
          "253": 1,
          "254": 3,
          "255": 2,
          "256": 2,
          "257": 1,
          "258": 4,
          "259": 4,
          "260": 4,
          "261": 2,
          "262": 1,
          "264": 3,
          "265": 1,
          "266": 4,
          "267": 2,
          "268": 5,
          "269": 3,
          "270": 3,
          "271": 1,
          "272": 3,
          "273": 1,
          "274": 2,
          "275": 4,
          "276": 3,
          "277": 1,
          "278": 4,
          "279": 5,
          "280": 1,
          "281": 2,
          "282": 2,
          "284": 6,
          "285": 2,
          "287": 2,
          "288": 1,
          "289": 1,
          "290": 5,
          "293": 1,
          "294": 2,
          "295": 4,
          "296": 3,
          "299": 1,
          "301": 3,
          "302": 2,
          "303": 1,
          "304": 1,
          "305": 2,
          "306": 2,
          "307": 2,
          "308": 1,
          "309": 1,
          "311": 1,
          "312": 1,
          "313": 2,
          "314": 1,
          "315": 1,
          "317": 2,
          "318": 2,
          "319": 3,
          "320": 2,
          "321": 1,
          "322": 1,
          "324": 1,
          "325": 1,
          "326": 2,
          "327": 2,
          "328": 1,
          "330": 1,
          "332": 1,
          "334": 2,
          "335": 1,
          "336": 1,
          "337": 3,
          "338": 2,
          "342": 1,
          "353": 2,
          "355": 2,
          "356": 1,
          "358": 1,
          "361": 1,
          "365": 1,
          "367": 1,
          "371": 1,
          "372": 1,
          "375": 1,
          "378": 2,
          "379": 1,
          "380": 1,
          "381": 1,
          "385": 1,
          "388": 1,
          "392": 1,
          "394": 1,
          "396": 1,
          "403": 1,
          "409": 1,
          "427": 1,
          "435": 1,
          "437": 1,
          "440": 1,
          "442": 1,
          "444": 1,
          "448": 1,
          "473": 1,
          "488": 1,
          "493": 1,
          "669": 1
        },
        "limitations": [
          "logical work only",
          "no DRAM/cache/synchronization",
          "optimistic one regeneration per consumer rank",
          "no legal AMX schedule proof"
        ]
      },
      {
        "schema": "recompute-audit-cpp-v1",
        "status": "PASS",
        "graph": "mycielskian19",
        "csr_path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/mycielskian19/mycielskian19.csrbin",
        "n": 393215,
        "e": 903194710,
        "batch_rows": 16,
        "batches": 24576,
        "source_rows_with_consumer": 393215,
        "full_h1_bytes": 100663040,
        "extra_regeneration_rows": 360448,
        "extra_first_layer_edge_visits": 731433809,
        "extra_edge_visit_ratio": 0.809829598094,
        "extra_projection_rows": 360448,
        "extra_projection_groups_by_rank": 22528,
        "baseline_peak_source_rows": 393200,
        "candidate_peak_source_plus_transient_rows": 393184,
        "candidate_peak_rank": 24574,
        "reuse_histogram": {
          "1": 32767,
          "2": 360448
        },
        "limitations": [
          "logical work only",
          "no DRAM/cache/synchronization",
          "optimistic one regeneration per consumer rank",
          "no legal AMX schedule proof"
        ]
      }
    ],
    "source_manifest_sha256": "98a601f3f7a157989692c18d7de5d17e726ccaa8599f59ec2b9b769733a3ca76",
    "performance_claim": false,
    "no_numa_interleave": true,
    "limitations": [
      "logical edge/projection work only",
      "no DRAM/cache/synchronization",
      "optimistic one regeneration per consumer rank",
      "no legal AMX schedule proof"
    ]
  },
  "node": "qhcn006",
  "job": "10899784",
  "paper_eligible": false,
  "issues": "Logical recomputation-cost audit only; no speedup or DRAM claim.",
  "next": "Compare extra AVX edge work and source-state release with row-complete baseline before any kernel work.",
  "date": "2026-10-08T09:59:54.349540+00:00"
}
```

## recompute-audit-analysis-20261008-05

```json
{
  "id": "recompute-audit-analysis-20261008-05",
  "status": "ANALYZED_PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/recompute_audit_20261008/retry-05-analysis",
  "job": "10899784",
  "paper_eligible": false,
  "issues": "C++ logical cost audit passed. Selective H1 regeneration is promising only on RoadNet-CA under the optimistic model; rejected for Reddit/Amazon and unresolved for Mycielskian19. No speedup claim.",
  "next": "Prototype only a bounded RoadNet-CA recomputation path with exact numerical checks; retain row-complete baseline and stop if added AVX/AMX/synchronization erases the modeled gap.",
  "desktop_write": false,
  "numa_interleave": false,
  "date": "2026-10-08T10:05:24.359134+00:00"
}
```

## recompute-numerical-gate-20261008

```json
{
  "id": "recompute-numerical-gate-20261008",
  "status": "PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/recompute_audit_20261008/retry-05-analysis/numerical_gate.json",
  "paper_eligible": false,
  "issues": "Toy semantic gate only; reference and batch-regenerated two-layer outputs match bitwise for n=64, dim=128.",
  "next": "If Road prototype is implemented, preserve the same quantization boundaries and compare full output bytes.",
  "date": "2026-10-08T10:12:53.248563+00:00"
}
```

## recompute-audit-analysis-revision-20261008

```json
{
  "id": "recompute-audit-analysis-revision-20261008",
  "status": "ANALYZED_PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/recompute_audit_20261008/retry-05-analysis",
  "paper_eligible": false,
  "issues": "Added toy 128-dimension semantic gate: batch regeneration matches the one-time H1 path bytewise. No performance claim.",
  "next": "Implement only a bounded RoadNet-CA prototype if exact kernel integration preserves this gate; reject broad recomputation for Reddit/Amazon and keep Myc unresolved.",
  "date": "2026-10-08T10:14:24.816331+00:00"
}
```

## recompute-audit-analysis-revision-02-20261008

```json
{
  "id": "recompute-audit-analysis-revision-02-20261008",
  "status": "ANALYZED_PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/recompute_audit_20261008/retry-05-analysis",
  "paper_eligible": false,
  "issues": "Added AMX projection group accounting. RoadNet-CA state reduction is capacity-oriented; extra first-layer AMX groups are 1.423x baseline. No speedup claim.",
  "next": "Use a bounded Road capacity prototype only if memory pressure is the target; otherwise stop selective recomputation and pursue a different AVX-AMX dataflow.",
  "date": "2026-10-08T10:16:22.707830+00:00"
}
```

## state-search-analysis-20261008

```json
{
  "id": "state-search-analysis-20261008",
  "status": "ANALYZED_PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/frontier_structure_20261008/state-search-01",
  "paper_eligible": false,
  "issues": "Pointwise matching lower bound is insufficient for a global legal schedule; 30/30 random fixtures show temporal peak gap.",
  "next": "Use this reachability constraint in any Road prototype; do not claim pointwise certificate as an executable policy.",
  "date": "2026-10-08T10:24:56.653668+00:00"
}
```

## recompute-window-prepared-20261008

```json
{
  "id": "recompute-window-prepared-20261008",
  "status": "PREPARED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-20261008-01",
  "manifest": {
    "source_sha256": {
      "PROTOCOL.md": "c6fe7a91689c38935fd904672a93a68b05f55941fbac5e046545e52e1541102e",
      "window_scan.cpp": "d83f4c5ad605342f83a793e42dfe10ae2c74bd5a6a93439de6e3ff3e2860bb39",
      "recompute_scan.cpp": "b152653c06773178368b73b3997b54e5f49ad15932195a8b8a57b0acb81abbb3",
      "run.py": "e7badf3eac580c6f0224587f1e546f4e90e8b2e23491845fce6cae944fbe2c6d",
      "window_job.sh": "f863a76fcf7a9ecdd80c25072c19183ed2173f4ab9ee818b684243476d78a8d6",
      "stage.py": "d90c75e7f9fda65f5f30258b136f8d5cfed44cf39cf297ee714b91e1b8855c91",
      "frontier_prior_inputs.json": "53c981305c7cc7db4ea368a54288309517053c75df9f3eec7ecd41659028d5cf"
    },
    "authority_sha256": {
      "/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md": "55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7",
      "/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md": "641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca"
    },
    "authority_spec_sha256": {
      "MATH_SPEC_V2.md": "1938d301e3fc913766744af166c0a41b1cb767d7d7419af9c098d3de955cf266",
      "KERNEL_LAYOUT_V2.md": "732e859128baa3cf87813df82cf7562d14358fc6e1cf17c32ec0dde5a743595f",
      "PYTORCH_EXTENSION_SPEC.md": "a3784dd66d4986bde33a40d523f761ee8956da9bbbe711f1707d13d8d2d82df2",
      "EXPERIMENT_SPEC_V2.md": "ecb8fe1c394d9ec47aae97d8f5b87c5b948963c7e8c8b25d70128483c7c0d10d"
    },
    "contract_sha256": "b6419e5106278106da6727c90f4cfa185cc70c587d70b4087c0bc286fd0dc244",
    "requested_nodes_this_study": 1,
    "user_cap": 10,
    "numa_interleave": false,
    "desktop_write": false,
    "scope": "REUSE_WINDOW_STRUCTURE"
  },
  "paper_eligible": false,
  "issues": "No new major issue; source-aware window tradeoff only.",
  "next": "Compile and scan four frozen graphs on one shared node.",
  "date": "2026-10-08T10:37:07.620717+00:00"
}
```

## recompute-window-submitted-10899912

```json
{
  "id": "recompute-window-submitted-10899912",
  "status": "SUBMITTED",
  "job": "10899912",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-20261008-01",
  "paper_eligible": false,
  "desktop_write": false,
  "issues": "Logical structural scan only; no feature or performance kernel.",
  "next": "Check all W metrics and compare AMX groups with row-complete baseline.",
  "date": "2026-10-08T10:37:07.670670+00:00"
}
```

## recompute-window-20261008-10899912

```json
{
  "id": "recompute-window-20261008-10899912",
  "status": "PASS",
  "exit_code": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-20261008-01/compute-10899912",
  "completion": {
    "schema": "recompute-window-audit-v1",
    "status": "PASS",
    "results": [
      {
        "schema": "recompute-window-v1",
        "status": "PASS",
        "graph": "amazon0601",
        "csr_path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/amazon0601/amazon0601.csrbin",
        "n": 403394,
        "e": 3387388,
        "batch_rows": 16,
        "batches": 25213,
        "windows": [
          {
            "window_batches": 1,
            "extra_first_layer_edge_visits": 88769005,
            "extra_edge_visit_ratio": 26.205738758,
            "extra_regeneration_rows": 1918946,
            "extra_projection_groups": 131768,
            "candidate_peak_source_plus_transient_rows": 844187,
            "candidate_peak_rank": 9262
          },
          {
            "window_batches": 2,
            "extra_first_layer_edge_visits": 86459025,
            "extra_edge_visit_ratio": 25.5238032962,
            "extra_regeneration_rows": 1913580,
            "extra_projection_groups": 131449,
            "candidate_peak_source_plus_transient_rows": 841833,
            "candidate_peak_rank": 9262
          },
          {
            "window_batches": 4,
            "extra_first_layer_edge_visits": 82374225,
            "extra_edge_visit_ratio": 24.3179184079,
            "extra_regeneration_rows": 1903350,
            "extra_projection_groups": 130801,
            "candidate_peak_source_plus_transient_rows": 837412,
            "candidate_peak_rank": 9262
          },
          {
            "window_batches": 8,
            "extra_first_layer_edge_visits": 76333399,
            "extra_edge_visit_ratio": 22.5345897783,
            "extra_regeneration_rows": 1885182,
            "extra_projection_groups": 129659,
            "candidate_peak_source_plus_transient_rows": 829562,
            "candidate_peak_rank": 9262
          },
          {
            "window_batches": 16,
            "extra_first_layer_edge_visits": 68567577,
            "extra_edge_visit_ratio": 20.2420204004,
            "extra_regeneration_rows": 1854296,
            "extra_projection_groups": 127737,
            "candidate_peak_source_plus_transient_rows": 815989,
            "candidate_peak_rank": 9262
          },
          {
            "window_batches": 32,
            "extra_first_layer_edge_visits": 60226349,
            "extra_edge_visit_ratio": 17.7795838564,
            "extra_regeneration_rows": 1801242,
            "extra_projection_groups": 124489,
            "candidate_peak_source_plus_transient_rows": 792535,
            "candidate_peak_rank": 9262
          },
          {
            "window_batches": 64,
            "extra_first_layer_edge_visits": 51216184,
            "extra_edge_visit_ratio": 15.119668606,
            "extra_regeneration_rows": 1712165,
            "extra_projection_groups": 118823,
            "candidate_peak_source_plus_transient_rows": 755171,
            "candidate_peak_rank": 9262
          },
          {
            "window_batches": 128,
            "extra_first_layer_edge_visits": 40960892,
            "extra_edge_visit_ratio": 12.0921760365,
            "extra_regeneration_rows": 1567890,
            "extra_projection_groups": 109781,
            "candidate_peak_source_plus_transient_rows": 696065,
            "candidate_peak_rank": 9262
          },
          {
            "window_batches": 256,
            "extra_first_layer_edge_visits": 30008348,
            "extra_edge_visit_ratio": 8.85884581276,
            "extra_regeneration_rows": 1355445,
            "extra_projection_groups": 96501,
            "candidate_peak_source_plus_transient_rows": 610091,
            "candidate_peak_rank": 9341
          },
          {
            "window_batches": 512,
            "extra_first_layer_edge_visits": 19518087,
            "extra_edge_visit_ratio": 5.76198740741,
            "extra_regeneration_rows": 1068196,
            "extra_projection_groups": 78464,
            "candidate_peak_source_plus_transient_rows": 499042,
            "candidate_peak_rank": 10101
          },
          {
            "window_batches": 1024,
            "extra_first_layer_edge_visits": 10673489,
            "extra_edge_visit_ratio": 3.15094964025,
            "extra_regeneration_rows": 722739,
            "extra_projection_groups": 56644,
            "candidate_peak_source_plus_transient_rows": 372478,
            "candidate_peak_rank": 10438
          },
          {
            "window_batches": 0,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 159654,
            "candidate_peak_rank": 15253
          }
        ]
      },
      {
        "schema": "recompute-window-v1",
        "status": "PASS",
        "graph": "roadNet-CA",
        "csr_path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/roadNet-CA/roadNet-CA.csrbin",
        "n": 1971281,
        "e": 5533214,
        "batch_rows": 16,
        "batches": 123206,
        "windows": [
          {
            "window_batches": 1,
            "extra_first_layer_edge_visits": 4568671,
            "extra_edge_visit_ratio": 0.825681240595,
            "extra_regeneration_rows": 1358215,
            "extra_projection_groups": 148449,
            "candidate_peak_source_plus_transient_rows": 30034,
            "candidate_peak_rank": 70775
          },
          {
            "window_batches": 2,
            "extra_first_layer_edge_visits": 4036990,
            "extra_edge_visit_ratio": 0.729592240604,
            "extra_regeneration_rows": 1203006,
            "extra_projection_groups": 138339,
            "candidate_peak_source_plus_transient_rows": 29410,
            "candidate_peak_rank": 70775
          },
          {
            "window_batches": 4,
            "extra_first_layer_edge_visits": 3523536,
            "extra_edge_visit_ratio": 0.636797347798,
            "extra_regeneration_rows": 1053109,
            "extra_projection_groups": 125706,
            "candidate_peak_source_plus_transient_rows": 28821,
            "candidate_peak_rank": 70775
          },
          {
            "window_batches": 8,
            "extra_first_layer_edge_visits": 3036922,
            "extra_edge_visit_ratio": 0.548853162014,
            "extra_regeneration_rows": 910785,
            "extra_projection_groups": 112003,
            "candidate_peak_source_plus_transient_rows": 28246,
            "candidate_peak_rank": 70778
          },
          {
            "window_batches": 16,
            "extra_first_layer_edge_visits": 2588799,
            "extra_edge_visit_ratio": 0.467865331072,
            "extra_regeneration_rows": 779110,
            "extra_projection_groups": 97939,
            "candidate_peak_source_plus_transient_rows": 27773,
            "candidate_peak_rank": 70778
          },
          {
            "window_batches": 32,
            "extra_first_layer_edge_visits": 2226521,
            "extra_edge_visit_ratio": 0.402391991345,
            "extra_regeneration_rows": 671844,
            "extra_projection_groups": 85286,
            "candidate_peak_source_plus_transient_rows": 27320,
            "candidate_peak_rank": 70778
          },
          {
            "window_batches": 64,
            "extra_first_layer_edge_visits": 1923260,
            "extra_edge_visit_ratio": 0.347584604535,
            "extra_regeneration_rows": 581472,
            "extra_projection_groups": 73886,
            "candidate_peak_source_plus_transient_rows": 26853,
            "candidate_peak_rank": 70778
          },
          {
            "window_batches": 128,
            "extra_first_layer_edge_visits": 1652295,
            "extra_edge_visit_ratio": 0.298613970109,
            "extra_regeneration_rows": 500167,
            "extra_projection_groups": 64093,
            "candidate_peak_source_plus_transient_rows": 26341,
            "candidate_peak_rank": 70778
          },
          {
            "window_batches": 256,
            "extra_first_layer_edge_visits": 1351528,
            "extra_edge_visit_ratio": 0.244257315911,
            "extra_regeneration_rows": 409549,
            "extra_projection_groups": 53020,
            "candidate_peak_source_plus_transient_rows": 25599,
            "candidate_peak_rank": 70778
          },
          {
            "window_batches": 512,
            "extra_first_layer_edge_visits": 996383,
            "extra_edge_visit_ratio": 0.180073100372,
            "extra_regeneration_rows": 301780,
            "extra_projection_groups": 40333,
            "candidate_peak_source_plus_transient_rows": 24706,
            "candidate_peak_rank": 70778
          },
          {
            "window_batches": 1024,
            "extra_first_layer_edge_visits": 574248,
            "extra_edge_visit_ratio": 0.103781997226,
            "extra_regeneration_rows": 173153,
            "extra_projection_groups": 25256,
            "candidate_peak_source_plus_transient_rows": 23517,
            "candidate_peak_rank": 70778
          },
          {
            "window_batches": 0,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 23156,
            "candidate_peak_rank": 70778
          }
        ]
      },
      {
        "schema": "recompute-window-v1",
        "status": "PASS",
        "graph": "reddit",
        "csr_path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/reddit/reddit.csrbin",
        "n": 232965,
        "e": 114615892,
        "batch_rows": 16,
        "batches": 14561,
        "windows": [
          {
            "window_batches": 1,
            "extra_first_layer_edge_visits": 4287358428,
            "extra_edge_visit_ratio": 37.4063173369,
            "extra_regeneration_rows": 5247419,
            "extra_projection_groups": 330603,
            "candidate_peak_source_plus_transient_rows": 5037539,
            "candidate_peak_rank": 13787
          },
          {
            "window_batches": 2,
            "extra_first_layer_edge_visits": 3099184499,
            "extra_edge_visit_ratio": 27.0397450556,
            "extra_regeneration_rows": 3872522,
            "extra_projection_groups": 244689,
            "candidate_peak_source_plus_transient_rows": 3734070,
            "candidate_peak_rank": 13661
          },
          {
            "window_batches": 4,
            "extra_first_layer_edge_visits": 2144115143,
            "extra_edge_visit_ratio": 18.706962059,
            "extra_regeneration_rows": 2713633,
            "extra_projection_groups": 172233,
            "candidate_peak_source_plus_transient_rows": 2644402,
            "candidate_peak_rank": 13507
          },
          {
            "window_batches": 8,
            "extra_first_layer_edge_visits": 1423413967,
            "extra_edge_visit_ratio": 12.4189930573,
            "extra_regeneration_rows": 1794358,
            "extra_projection_groups": 114798,
            "candidate_peak_source_plus_transient_rows": 1790923,
            "candidate_peak_rank": 13377
          },
          {
            "window_batches": 16,
            "extra_first_layer_edge_visits": 922649218,
            "extra_edge_visit_ratio": 8.04992398436,
            "extra_regeneration_rows": 1159443,
            "extra_projection_groups": 75126,
            "candidate_peak_source_plus_transient_rows": 1206079,
            "candidate_peak_rank": 13205
          },
          {
            "window_batches": 32,
            "extra_first_layer_edge_visits": 585355154,
            "extra_edge_visit_ratio": 5.10710289634,
            "extra_regeneration_rows": 736147,
            "extra_projection_groups": 48662,
            "candidate_peak_source_plus_transient_rows": 821815,
            "candidate_peak_rank": 12960
          },
          {
            "window_batches": 64,
            "extra_first_layer_edge_visits": 360611014,
            "extra_edge_visit_ratio": 3.14625666395,
            "extra_regeneration_rows": 455737,
            "extra_projection_groups": 31133,
            "candidate_peak_source_plus_transient_rows": 570208,
            "candidate_peak_rank": 12789
          },
          {
            "window_batches": 128,
            "extra_first_layer_edge_visits": 214113112,
            "extra_edge_visit_ratio": 1.86809270742,
            "extra_regeneration_rows": 274570,
            "extra_projection_groups": 19772,
            "candidate_peak_source_plus_transient_rows": 409800,
            "candidate_peak_rank": 12569
          },
          {
            "window_batches": 256,
            "extra_first_layer_edge_visits": 121341588,
            "extra_edge_visit_ratio": 1.05868030936,
            "extra_regeneration_rows": 157435,
            "extra_projection_groups": 12439,
            "candidate_peak_source_plus_transient_rows": 307757,
            "candidate_peak_rank": 12486
          },
          {
            "window_batches": 512,
            "extra_first_layer_edge_visits": 64038354,
            "extra_edge_visit_ratio": 0.558721420586,
            "extra_regeneration_rows": 82427,
            "extra_projection_groups": 7713,
            "candidate_peak_source_plus_transient_rows": 243897,
            "candidate_peak_rank": 12486
          },
          {
            "window_batches": 1024,
            "extra_first_layer_edge_visits": 31329901,
            "extra_edge_visit_ratio": 0.273346919466,
            "extra_regeneration_rows": 38553,
            "extra_projection_groups": 4917,
            "candidate_peak_source_plus_transient_rows": 231090,
            "candidate_peak_rank": 14511
          },
          {
            "window_batches": 0,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 231089,
            "candidate_peak_rank": 14511
          }
        ]
      },
      {
        "schema": "recompute-window-v1",
        "status": "PASS",
        "graph": "mycielskian19",
        "csr_path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/mycielskian19/mycielskian19.csrbin",
        "n": 393215,
        "e": 903194710,
        "batch_rows": 16,
        "batches": 24576,
        "windows": [
          {
            "window_batches": 1,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 2,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 4,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 8,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 16,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 32,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 64,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 128,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 256,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 512,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 1024,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 0,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          }
        ]
      }
    ],
    "performance_claim": false,
    "no_numa_interleave": true,
    "limitations": [
      "logical proxies only",
      "no partial/ready state",
      "no DRAM/cache/synchronization",
      "no legal full-kernel schedule"
    ]
  },
  "node": "qhcn022",
  "job": "10899912",
  "paper_eligible": false,
  "issues": "Logical source reuse-window audit only; no speedup or DRAM claim.",
  "next": "Compare W tradeoffs with row-complete baseline; develop no kernel if AMX or AVX work erases capacity benefit.",
  "date": "2026-10-08T10:38:20.923935+00:00"
}
```

## recompute-window-prepared-20261008-02

```json
{
  "id": "recompute-window-prepared-20261008-02",
  "status": "PREPARED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-20261008-02",
  "manifest": {
    "source_sha256": {
      "PROTOCOL.md": "582fe77405fee9fa12bfbf023a4df48b3f2ac92f124b459ac8141f0c184c612c",
      "window_scan.cpp": "3032fa8d8148f2f60ebf9b4ecd9e722535aab8987aea1f18ac3ceed0a3d991d6",
      "recompute_scan.cpp": "b152653c06773178368b73b3997b54e5f49ad15932195a8b8a57b0acb81abbb3",
      "run.py": "e7badf3eac580c6f0224587f1e546f4e90e8b2e23491845fce6cae944fbe2c6d",
      "window_job.sh": "f863a76fcf7a9ecdd80c25072c19183ed2173f4ab9ee818b684243476d78a8d6",
      "stage.py": "1735e703a0d2a984fa2e149d9a4704ff5a0c18c615987af9ebc3846347dc3f23",
      "frontier_prior_inputs.json": "53c981305c7cc7db4ea368a54288309517053c75df9f3eec7ecd41659028d5cf"
    },
    "authority_sha256": {
      "/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md": "55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7",
      "/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md": "641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca"
    },
    "authority_spec_sha256": {
      "MATH_SPEC_V2.md": "1938d301e3fc913766744af166c0a41b1cb767d7d7419af9c098d3de955cf266",
      "KERNEL_LAYOUT_V2.md": "732e859128baa3cf87813df82cf7562d14358fc6e1cf17c32ec0dde5a743595f",
      "PYTORCH_EXTENSION_SPEC.md": "a3784dd66d4986bde33a40d523f761ee8956da9bbbe711f1707d13d8d2d82df2",
      "EXPERIMENT_SPEC_V2.md": "ecb8fe1c394d9ec47aae97d8f5b87c5b948963c7e8c8b25d70128483c7c0d10d"
    },
    "contract_sha256": "b6419e5106278106da6727c90f4cfa185cc70c587d70b4087c0bc286fd0dc244",
    "requested_nodes_this_study": 1,
    "user_cap": 10,
    "numa_interleave": false,
    "desktop_write": false,
    "scope": "REUSE_WINDOW_STRUCTURE"
  },
  "paper_eligible": false,
  "issues": "No new major issue; source-aware window tradeoff only.",
  "next": "Compile and scan four frozen graphs on one shared node.",
  "date": "2026-10-08T10:43:13.678539+00:00"
}
```

## recompute-window-submitted-10899982

```json
{
  "id": "recompute-window-submitted-10899982",
  "status": "SUBMITTED",
  "job": "10899982",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-20261008-02",
  "paper_eligible": false,
  "desktop_write": false,
  "issues": "Logical structural scan only; no feature or performance kernel.",
  "next": "Check all W metrics and compare AMX groups with row-complete baseline.",
  "date": "2026-10-08T10:43:13.732698+00:00"
}
```

## recompute-window-20261008-10899982

```json
{
  "id": "recompute-window-20261008-10899982",
  "status": "FAILED",
  "exit_code": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-20261008-02/compute-10899982",
  "completion": {},
  "node": "qhcn282",
  "job": "10899982",
  "paper_eligible": false,
  "issues": "Logical source reuse-window audit only; no speedup or DRAM claim.",
  "next": "Compare W tradeoffs with row-complete baseline; develop no kernel if AMX or AVX work erases capacity benefit.",
  "date": "2026-10-08T10:43:22.848372+00:00"
}
```

## recompute-window-prepared-20261008-03

```json
{
  "id": "recompute-window-prepared-20261008-03",
  "status": "PREPARED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-20261008-03",
  "manifest": {
    "source_sha256": {
      "PROTOCOL.md": "649ae9c170a4809a1318037458495dae7c14f487d5e03b4fbe93a054b6350768",
      "window_scan.cpp": "fdd424023e5f2d87b5418b0249cbfbcd726994ac3ccbe439e68fc7741889d9f7",
      "recompute_scan.cpp": "b152653c06773178368b73b3997b54e5f49ad15932195a8b8a57b0acb81abbb3",
      "run.py": "e7badf3eac580c6f0224587f1e546f4e90e8b2e23491845fce6cae944fbe2c6d",
      "window_job.sh": "f863a76fcf7a9ecdd80c25072c19183ed2173f4ab9ee818b684243476d78a8d6",
      "stage.py": "6142ac96200944e38df31578c9c92f50e1576c601ea08e308f42614754188b73",
      "frontier_prior_inputs.json": "53c981305c7cc7db4ea368a54288309517053c75df9f3eec7ecd41659028d5cf"
    },
    "authority_sha256": {
      "/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md": "55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7",
      "/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md": "641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca"
    },
    "authority_spec_sha256": {
      "MATH_SPEC_V2.md": "1938d301e3fc913766744af166c0a41b1cb767d7d7419af9c098d3de955cf266",
      "KERNEL_LAYOUT_V2.md": "732e859128baa3cf87813df82cf7562d14358fc6e1cf17c32ec0dde5a743595f",
      "PYTORCH_EXTENSION_SPEC.md": "a3784dd66d4986bde33a40d523f761ee8956da9bbbe711f1707d13d8d2d82df2",
      "EXPERIMENT_SPEC_V2.md": "ecb8fe1c394d9ec47aae97d8f5b87c5b948963c7e8c8b25d70128483c7c0d10d"
    },
    "contract_sha256": "b6419e5106278106da6727c90f4cfa185cc70c587d70b4087c0bc286fd0dc244",
    "requested_nodes_this_study": 1,
    "user_cap": 10,
    "numa_interleave": false,
    "desktop_write": false,
    "scope": "REUSE_WINDOW_STRUCTURE"
  },
  "paper_eligible": false,
  "issues": "No new major issue; source-aware window tradeoff only.",
  "next": "Compile and scan four frozen graphs on one shared node.",
  "date": "2026-10-08T10:46:03.188376+00:00"
}
```

## recompute-window-submitted-10899992

```json
{
  "id": "recompute-window-submitted-10899992",
  "status": "SUBMITTED",
  "job": "10899992",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-20261008-03",
  "paper_eligible": false,
  "desktop_write": false,
  "issues": "Logical structural scan only; no feature or performance kernel.",
  "next": "Check all W metrics and compare AMX groups with row-complete baseline.",
  "date": "2026-10-08T10:46:03.256081+00:00"
}
```

## recompute-window-20261008-10899992

```json
{
  "id": "recompute-window-20261008-10899992",
  "status": "PASS",
  "exit_code": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-20261008-03/compute-10899992",
  "completion": {
    "schema": "recompute-window-audit-v1",
    "status": "PASS",
    "results": [
      {
        "schema": "recompute-window-v1",
        "status": "PASS",
        "graph": "amazon0601",
        "csr_path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/amazon0601/amazon0601.csrbin",
        "n": 403394,
        "e": 3387388,
        "batch_rows": 16,
        "batches": 25213,
        "windows": [
          {
            "window_batches": -1,
            "extra_first_layer_edge_visits": 92139244,
            "extra_edge_visit_ratio": 27.2006761552,
            "extra_regeneration_rows": 1952884,
            "extra_projection_groups": 133887,
            "candidate_peak_source_plus_transient_rows": 851601,
            "candidate_peak_rank": 9262
          },
          {
            "window_batches": 1,
            "extra_first_layer_edge_visits": 88769005,
            "extra_edge_visit_ratio": 26.205738758,
            "extra_regeneration_rows": 1918946,
            "extra_projection_groups": 131768,
            "candidate_peak_source_plus_transient_rows": 844187,
            "candidate_peak_rank": 9262
          },
          {
            "window_batches": 2,
            "extra_first_layer_edge_visits": 86459025,
            "extra_edge_visit_ratio": 25.5238032962,
            "extra_regeneration_rows": 1913580,
            "extra_projection_groups": 131449,
            "candidate_peak_source_plus_transient_rows": 841833,
            "candidate_peak_rank": 9262
          },
          {
            "window_batches": 4,
            "extra_first_layer_edge_visits": 82374225,
            "extra_edge_visit_ratio": 24.3179184079,
            "extra_regeneration_rows": 1903350,
            "extra_projection_groups": 130801,
            "candidate_peak_source_plus_transient_rows": 837412,
            "candidate_peak_rank": 9262
          },
          {
            "window_batches": 8,
            "extra_first_layer_edge_visits": 76333399,
            "extra_edge_visit_ratio": 22.5345897783,
            "extra_regeneration_rows": 1885182,
            "extra_projection_groups": 129659,
            "candidate_peak_source_plus_transient_rows": 829562,
            "candidate_peak_rank": 9262
          },
          {
            "window_batches": 16,
            "extra_first_layer_edge_visits": 68567577,
            "extra_edge_visit_ratio": 20.2420204004,
            "extra_regeneration_rows": 1854296,
            "extra_projection_groups": 127737,
            "candidate_peak_source_plus_transient_rows": 815989,
            "candidate_peak_rank": 9262
          },
          {
            "window_batches": 32,
            "extra_first_layer_edge_visits": 60226349,
            "extra_edge_visit_ratio": 17.7795838564,
            "extra_regeneration_rows": 1801242,
            "extra_projection_groups": 124489,
            "candidate_peak_source_plus_transient_rows": 792535,
            "candidate_peak_rank": 9262
          },
          {
            "window_batches": 64,
            "extra_first_layer_edge_visits": 51216184,
            "extra_edge_visit_ratio": 15.119668606,
            "extra_regeneration_rows": 1712165,
            "extra_projection_groups": 118823,
            "candidate_peak_source_plus_transient_rows": 755171,
            "candidate_peak_rank": 9262
          },
          {
            "window_batches": 128,
            "extra_first_layer_edge_visits": 40960892,
            "extra_edge_visit_ratio": 12.0921760365,
            "extra_regeneration_rows": 1567890,
            "extra_projection_groups": 109781,
            "candidate_peak_source_plus_transient_rows": 696065,
            "candidate_peak_rank": 9262
          },
          {
            "window_batches": 256,
            "extra_first_layer_edge_visits": 30008348,
            "extra_edge_visit_ratio": 8.85884581276,
            "extra_regeneration_rows": 1355445,
            "extra_projection_groups": 96501,
            "candidate_peak_source_plus_transient_rows": 610091,
            "candidate_peak_rank": 9341
          },
          {
            "window_batches": 512,
            "extra_first_layer_edge_visits": 19518087,
            "extra_edge_visit_ratio": 5.76198740741,
            "extra_regeneration_rows": 1068196,
            "extra_projection_groups": 78464,
            "candidate_peak_source_plus_transient_rows": 499042,
            "candidate_peak_rank": 10101
          },
          {
            "window_batches": 1024,
            "extra_first_layer_edge_visits": 10673489,
            "extra_edge_visit_ratio": 3.15094964025,
            "extra_regeneration_rows": 722739,
            "extra_projection_groups": 56644,
            "candidate_peak_source_plus_transient_rows": 372478,
            "candidate_peak_rank": 10438
          },
          {
            "window_batches": 0,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 159654,
            "candidate_peak_rank": 15253
          }
        ]
      },
      {
        "schema": "recompute-window-v1",
        "status": "PASS",
        "graph": "roadNet-CA",
        "csr_path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/roadNet-CA/roadNet-CA.csrbin",
        "n": 1971281,
        "e": 5533214,
        "batch_rows": 16,
        "batches": 123206,
        "windows": [
          {
            "window_batches": -1,
            "extra_first_layer_edge_visits": 6280436,
            "extra_edge_visit_ratio": 1.13504303286,
            "extra_regeneration_rows": 1860117,
            "extra_projection_groups": 175379,
            "candidate_peak_source_plus_transient_rows": 31954,
            "candidate_peak_rank": 70775
          },
          {
            "window_batches": 1,
            "extra_first_layer_edge_visits": 4568671,
            "extra_edge_visit_ratio": 0.825681240595,
            "extra_regeneration_rows": 1358215,
            "extra_projection_groups": 148449,
            "candidate_peak_source_plus_transient_rows": 30034,
            "candidate_peak_rank": 70775
          },
          {
            "window_batches": 2,
            "extra_first_layer_edge_visits": 4036990,
            "extra_edge_visit_ratio": 0.729592240604,
            "extra_regeneration_rows": 1203006,
            "extra_projection_groups": 138339,
            "candidate_peak_source_plus_transient_rows": 29410,
            "candidate_peak_rank": 70775
          },
          {
            "window_batches": 4,
            "extra_first_layer_edge_visits": 3523536,
            "extra_edge_visit_ratio": 0.636797347798,
            "extra_regeneration_rows": 1053109,
            "extra_projection_groups": 125706,
            "candidate_peak_source_plus_transient_rows": 28821,
            "candidate_peak_rank": 70775
          },
          {
            "window_batches": 8,
            "extra_first_layer_edge_visits": 3036922,
            "extra_edge_visit_ratio": 0.548853162014,
            "extra_regeneration_rows": 910785,
            "extra_projection_groups": 112003,
            "candidate_peak_source_plus_transient_rows": 28246,
            "candidate_peak_rank": 70778
          },
          {
            "window_batches": 16,
            "extra_first_layer_edge_visits": 2588799,
            "extra_edge_visit_ratio": 0.467865331072,
            "extra_regeneration_rows": 779110,
            "extra_projection_groups": 97939,
            "candidate_peak_source_plus_transient_rows": 27773,
            "candidate_peak_rank": 70778
          },
          {
            "window_batches": 32,
            "extra_first_layer_edge_visits": 2226521,
            "extra_edge_visit_ratio": 0.402391991345,
            "extra_regeneration_rows": 671844,
            "extra_projection_groups": 85286,
            "candidate_peak_source_plus_transient_rows": 27320,
            "candidate_peak_rank": 70778
          },
          {
            "window_batches": 64,
            "extra_first_layer_edge_visits": 1923260,
            "extra_edge_visit_ratio": 0.347584604535,
            "extra_regeneration_rows": 581472,
            "extra_projection_groups": 73886,
            "candidate_peak_source_plus_transient_rows": 26853,
            "candidate_peak_rank": 70778
          },
          {
            "window_batches": 128,
            "extra_first_layer_edge_visits": 1652295,
            "extra_edge_visit_ratio": 0.298613970109,
            "extra_regeneration_rows": 500167,
            "extra_projection_groups": 64093,
            "candidate_peak_source_plus_transient_rows": 26341,
            "candidate_peak_rank": 70778
          },
          {
            "window_batches": 256,
            "extra_first_layer_edge_visits": 1351528,
            "extra_edge_visit_ratio": 0.244257315911,
            "extra_regeneration_rows": 409549,
            "extra_projection_groups": 53020,
            "candidate_peak_source_plus_transient_rows": 25599,
            "candidate_peak_rank": 70778
          },
          {
            "window_batches": 512,
            "extra_first_layer_edge_visits": 996383,
            "extra_edge_visit_ratio": 0.180073100372,
            "extra_regeneration_rows": 301780,
            "extra_projection_groups": 40333,
            "candidate_peak_source_plus_transient_rows": 24706,
            "candidate_peak_rank": 70778
          },
          {
            "window_batches": 1024,
            "extra_first_layer_edge_visits": 574248,
            "extra_edge_visit_ratio": 0.103781997226,
            "extra_regeneration_rows": 173153,
            "extra_projection_groups": 25256,
            "candidate_peak_source_plus_transient_rows": 23517,
            "candidate_peak_rank": 70778
          },
          {
            "window_batches": 0,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 23156,
            "candidate_peak_rank": 70778
          }
        ]
      },
      {
        "schema": "recompute-window-v1",
        "status": "PASS",
        "graph": "reddit",
        "csr_path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/reddit/reddit.csrbin",
        "n": 232965,
        "e": 114615892,
        "batch_rows": 16,
        "batches": 14561,
        "windows": [
          {
            "window_batches": -1,
            "extra_first_layer_edge_visits": 8853220177,
            "extra_edge_visit_ratio": 77.242518664,
            "extra_regeneration_rows": 9153623,
            "extra_projection_groups": 574733,
            "candidate_peak_source_plus_transient_rows": 8773919,
            "candidate_peak_rank": 13944
          },
          {
            "window_batches": 1,
            "extra_first_layer_edge_visits": 4287358428,
            "extra_edge_visit_ratio": 37.4063173369,
            "extra_regeneration_rows": 5247419,
            "extra_projection_groups": 330603,
            "candidate_peak_source_plus_transient_rows": 5037539,
            "candidate_peak_rank": 13787
          },
          {
            "window_batches": 2,
            "extra_first_layer_edge_visits": 3099184499,
            "extra_edge_visit_ratio": 27.0397450556,
            "extra_regeneration_rows": 3872522,
            "extra_projection_groups": 244689,
            "candidate_peak_source_plus_transient_rows": 3734070,
            "candidate_peak_rank": 13661
          },
          {
            "window_batches": 4,
            "extra_first_layer_edge_visits": 2144115143,
            "extra_edge_visit_ratio": 18.706962059,
            "extra_regeneration_rows": 2713633,
            "extra_projection_groups": 172233,
            "candidate_peak_source_plus_transient_rows": 2644402,
            "candidate_peak_rank": 13507
          },
          {
            "window_batches": 8,
            "extra_first_layer_edge_visits": 1423413967,
            "extra_edge_visit_ratio": 12.4189930573,
            "extra_regeneration_rows": 1794358,
            "extra_projection_groups": 114798,
            "candidate_peak_source_plus_transient_rows": 1790923,
            "candidate_peak_rank": 13377
          },
          {
            "window_batches": 16,
            "extra_first_layer_edge_visits": 922649218,
            "extra_edge_visit_ratio": 8.04992398436,
            "extra_regeneration_rows": 1159443,
            "extra_projection_groups": 75126,
            "candidate_peak_source_plus_transient_rows": 1206079,
            "candidate_peak_rank": 13205
          },
          {
            "window_batches": 32,
            "extra_first_layer_edge_visits": 585355154,
            "extra_edge_visit_ratio": 5.10710289634,
            "extra_regeneration_rows": 736147,
            "extra_projection_groups": 48662,
            "candidate_peak_source_plus_transient_rows": 821815,
            "candidate_peak_rank": 12960
          },
          {
            "window_batches": 64,
            "extra_first_layer_edge_visits": 360611014,
            "extra_edge_visit_ratio": 3.14625666395,
            "extra_regeneration_rows": 455737,
            "extra_projection_groups": 31133,
            "candidate_peak_source_plus_transient_rows": 570208,
            "candidate_peak_rank": 12789
          },
          {
            "window_batches": 128,
            "extra_first_layer_edge_visits": 214113112,
            "extra_edge_visit_ratio": 1.86809270742,
            "extra_regeneration_rows": 274570,
            "extra_projection_groups": 19772,
            "candidate_peak_source_plus_transient_rows": 409800,
            "candidate_peak_rank": 12569
          },
          {
            "window_batches": 256,
            "extra_first_layer_edge_visits": 121341588,
            "extra_edge_visit_ratio": 1.05868030936,
            "extra_regeneration_rows": 157435,
            "extra_projection_groups": 12439,
            "candidate_peak_source_plus_transient_rows": 307757,
            "candidate_peak_rank": 12486
          },
          {
            "window_batches": 512,
            "extra_first_layer_edge_visits": 64038354,
            "extra_edge_visit_ratio": 0.558721420586,
            "extra_regeneration_rows": 82427,
            "extra_projection_groups": 7713,
            "candidate_peak_source_plus_transient_rows": 243897,
            "candidate_peak_rank": 12486
          },
          {
            "window_batches": 1024,
            "extra_first_layer_edge_visits": 31329901,
            "extra_edge_visit_ratio": 0.273346919466,
            "extra_regeneration_rows": 38553,
            "extra_projection_groups": 4917,
            "candidate_peak_source_plus_transient_rows": 231090,
            "candidate_peak_rank": 14511
          },
          {
            "window_batches": 0,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 231089,
            "candidate_peak_rank": 14511
          }
        ]
      },
      {
        "schema": "recompute-window-v1",
        "status": "PASS",
        "graph": "mycielskian19",
        "csr_path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/mycielskian19/mycielskian19.csrbin",
        "n": 393215,
        "e": 903194710,
        "batch_rows": 16,
        "batches": 24576,
        "windows": [
          {
            "window_batches": -1,
            "extra_first_layer_edge_visits": 731433809,
            "extra_edge_visit_ratio": 0.809829598094,
            "extra_regeneration_rows": 360448,
            "extra_projection_groups": 22528,
            "candidate_peak_source_plus_transient_rows": 753616,
            "candidate_peak_rank": 24574
          },
          {
            "window_batches": 1,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 2,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 4,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 8,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 16,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 32,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 64,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 128,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 256,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 512,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 1024,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 0,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          }
        ]
      }
    ],
    "performance_claim": false,
    "no_numa_interleave": true,
    "limitations": [
      "logical proxies only",
      "no partial/ready state",
      "no DRAM/cache/synchronization",
      "no legal full-kernel schedule"
    ]
  },
  "node": "qhcn006",
  "job": "10899992",
  "paper_eligible": false,
  "issues": "Logical source reuse-window audit only; no speedup or DRAM claim.",
  "next": "Compare W tradeoffs with row-complete baseline; develop no kernel if AMX or AVX work erases capacity benefit.",
  "date": "2026-10-08T10:47:03.308035+00:00"
}
```

## recompute-window-prepared-20261008-04

```json
{
  "id": "recompute-window-prepared-20261008-04",
  "status": "PREPARED",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-20261008-04",
  "manifest": {
    "source_sha256": {
      "PROTOCOL.md": "f9cfb29cf69891886153c7a67566ac0a276c3d1963ee3e6fe8b47780e262cd2b",
      "window_scan.cpp": "fd2185cb82cf669dd568897a953d115e35d6f39a2640cb45f48588092c3aba2c",
      "recompute_scan.cpp": "b152653c06773178368b73b3997b54e5f49ad15932195a8b8a57b0acb81abbb3",
      "run.py": "e7badf3eac580c6f0224587f1e546f4e90e8b2e23491845fce6cae944fbe2c6d",
      "window_job.sh": "f863a76fcf7a9ecdd80c25072c19183ed2173f4ab9ee818b684243476d78a8d6",
      "stage.py": "11f117e14e05390904f43867c9a58e99eed635de3edc90819824cbc9276172ac",
      "frontier_prior_inputs.json": "53c981305c7cc7db4ea368a54288309517053c75df9f3eec7ecd41659028d5cf"
    },
    "authority_sha256": {
      "/home/huangjianqiang_group/hdacp1/data/wzh/AGENTS.md": "55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7",
      "/home/huangjianqiang_group/hdacp1/data/wzh/skills/tfs-research-engineering/SKILL.md": "641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca"
    },
    "authority_spec_sha256": {
      "MATH_SPEC_V2.md": "1938d301e3fc913766744af166c0a41b1cb767d7d7419af9c098d3de955cf266",
      "KERNEL_LAYOUT_V2.md": "732e859128baa3cf87813df82cf7562d14358fc6e1cf17c32ec0dde5a743595f",
      "PYTORCH_EXTENSION_SPEC.md": "a3784dd66d4986bde33a40d523f761ee8956da9bbbe711f1707d13d8d2d82df2",
      "EXPERIMENT_SPEC_V2.md": "ecb8fe1c394d9ec47aae97d8f5b87c5b948963c7e8c8b25d70128483c7c0d10d"
    },
    "contract_sha256": "b6419e5106278106da6727c90f4cfa185cc70c587d70b4087c0bc286fd0dc244",
    "requested_nodes_this_study": 1,
    "user_cap": 10,
    "numa_interleave": false,
    "desktop_write": false,
    "scope": "REUSE_WINDOW_STRUCTURE"
  },
  "paper_eligible": false,
  "issues": "No new major issue; source-aware window tradeoff only.",
  "next": "Compile and scan four frozen graphs on one shared node.",
  "date": "2026-10-08T10:50:07.071655+00:00"
}
```

## recompute-window-submitted-10900001

```json
{
  "id": "recompute-window-submitted-10900001",
  "status": "SUBMITTED",
  "job": "10900001",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-20261008-04",
  "paper_eligible": false,
  "desktop_write": false,
  "issues": "Logical structural scan only; no feature or performance kernel.",
  "next": "Check all W metrics and compare AMX groups with row-complete baseline.",
  "date": "2026-10-08T10:50:07.154558+00:00"
}
```

## recompute-window-20261008-10900001

```json
{
  "id": "recompute-window-20261008-10900001",
  "status": "PASS",
  "exit_code": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-20261008-04/compute-10900001",
  "completion": {
    "schema": "recompute-window-audit-v1",
    "status": "PASS",
    "results": [
      {
        "schema": "recompute-window-v1",
        "status": "PASS",
        "graph": "amazon0601",
        "csr_path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/amazon0601/amazon0601.csrbin",
        "n": 403394,
        "e": 3387388,
        "batch_rows": 16,
        "batches": 25213,
        "windows": [
          {
            "window_batches": -1,
            "extra_first_layer_edge_visits": 92139244,
            "extra_edge_visit_ratio": 27.2006761552,
            "extra_regeneration_rows": 1952884,
            "extra_projection_groups": 133887,
            "candidate_peak_source_plus_transient_rows": 20,
            "candidate_peak_rank": 21305
          },
          {
            "window_batches": 1,
            "extra_first_layer_edge_visits": 88769005,
            "extra_edge_visit_ratio": 26.205738758,
            "extra_regeneration_rows": 1918946,
            "extra_projection_groups": 131768,
            "candidate_peak_source_plus_transient_rows": 46,
            "candidate_peak_rank": 25162
          },
          {
            "window_batches": 2,
            "extra_first_layer_edge_visits": 86459025,
            "extra_edge_visit_ratio": 25.5238032962,
            "extra_regeneration_rows": 1913580,
            "extra_projection_groups": 131449,
            "candidate_peak_source_plus_transient_rows": 72,
            "candidate_peak_rank": 25163
          },
          {
            "window_batches": 4,
            "extra_first_layer_edge_visits": 82374225,
            "extra_edge_visit_ratio": 24.3179184079,
            "extra_regeneration_rows": 1903350,
            "extra_projection_groups": 130801,
            "candidate_peak_source_plus_transient_rows": 89,
            "candidate_peak_rank": 25163
          },
          {
            "window_batches": 8,
            "extra_first_layer_edge_visits": 76333399,
            "extra_edge_visit_ratio": 22.5345897783,
            "extra_regeneration_rows": 1885182,
            "extra_projection_groups": 129659,
            "candidate_peak_source_plus_transient_rows": 168,
            "candidate_peak_rank": 25149
          },
          {
            "window_batches": 16,
            "extra_first_layer_edge_visits": 68567577,
            "extra_edge_visit_ratio": 20.2420204004,
            "extra_regeneration_rows": 1854296,
            "extra_projection_groups": 127737,
            "candidate_peak_source_plus_transient_rows": 292,
            "candidate_peak_rank": 25134
          },
          {
            "window_batches": 32,
            "extra_first_layer_edge_visits": 60226349,
            "extra_edge_visit_ratio": 17.7795838564,
            "extra_regeneration_rows": 1801242,
            "extra_projection_groups": 124489,
            "candidate_peak_source_plus_transient_rows": 530,
            "candidate_peak_rank": 25111
          },
          {
            "window_batches": 64,
            "extra_first_layer_edge_visits": 51216184,
            "extra_edge_visit_ratio": 15.119668606,
            "extra_regeneration_rows": 1712165,
            "extra_projection_groups": 118823,
            "candidate_peak_source_plus_transient_rows": 1011,
            "candidate_peak_rank": 25043
          },
          {
            "window_batches": 128,
            "extra_first_layer_edge_visits": 40960892,
            "extra_edge_visit_ratio": 12.0921760365,
            "extra_regeneration_rows": 1567890,
            "extra_projection_groups": 109781,
            "candidate_peak_source_plus_transient_rows": 2164,
            "candidate_peak_rank": 25020
          },
          {
            "window_batches": 256,
            "extra_first_layer_edge_visits": 30008348,
            "extra_edge_visit_ratio": 8.85884581276,
            "extra_regeneration_rows": 1355445,
            "extra_projection_groups": 96501,
            "candidate_peak_source_plus_transient_rows": 3788,
            "candidate_peak_rank": 24858
          },
          {
            "window_batches": 512,
            "extra_first_layer_edge_visits": 19518087,
            "extra_edge_visit_ratio": 5.76198740741,
            "extra_regeneration_rows": 1068196,
            "extra_projection_groups": 78464,
            "candidate_peak_source_plus_transient_rows": 7966,
            "candidate_peak_rank": 15588
          },
          {
            "window_batches": 1024,
            "extra_first_layer_edge_visits": 10673489,
            "extra_edge_visit_ratio": 3.15094964025,
            "extra_regeneration_rows": 722739,
            "extra_projection_groups": 56644,
            "candidate_peak_source_plus_transient_rows": 20793,
            "candidate_peak_rank": 15725
          },
          {
            "window_batches": 0,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 159654,
            "candidate_peak_rank": 15253
          }
        ]
      },
      {
        "schema": "recompute-window-v1",
        "status": "PASS",
        "graph": "roadNet-CA",
        "csr_path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/roadNet-CA/roadNet-CA.csrbin",
        "n": 1971281,
        "e": 5533214,
        "batch_rows": 16,
        "batches": 123206,
        "windows": [
          {
            "window_batches": -1,
            "extra_first_layer_edge_visits": 6280436,
            "extra_edge_visit_ratio": 1.13504303286,
            "extra_regeneration_rows": 1860117,
            "extra_projection_groups": 175379,
            "candidate_peak_source_plus_transient_rows": 2531,
            "candidate_peak_rank": 69647
          },
          {
            "window_batches": 1,
            "extra_first_layer_edge_visits": 4568671,
            "extra_edge_visit_ratio": 0.825681240595,
            "extra_regeneration_rows": 1358215,
            "extra_projection_groups": 148449,
            "candidate_peak_source_plus_transient_rows": 2539,
            "candidate_peak_rank": 69648
          },
          {
            "window_batches": 2,
            "extra_first_layer_edge_visits": 4036990,
            "extra_edge_visit_ratio": 0.729592240604,
            "extra_regeneration_rows": 1203006,
            "extra_projection_groups": 138339,
            "candidate_peak_source_plus_transient_rows": 2544,
            "candidate_peak_rank": 70325
          },
          {
            "window_batches": 4,
            "extra_first_layer_edge_visits": 3523536,
            "extra_edge_visit_ratio": 0.636797347798,
            "extra_regeneration_rows": 1053109,
            "extra_projection_groups": 125706,
            "candidate_peak_source_plus_transient_rows": 2551,
            "candidate_peak_rank": 70325
          },
          {
            "window_batches": 8,
            "extra_first_layer_edge_visits": 3036922,
            "extra_edge_visit_ratio": 0.548853162014,
            "extra_regeneration_rows": 910785,
            "extra_projection_groups": 112003,
            "candidate_peak_source_plus_transient_rows": 2562,
            "candidate_peak_rank": 70360
          },
          {
            "window_batches": 16,
            "extra_first_layer_edge_visits": 2588799,
            "extra_edge_visit_ratio": 0.467865331072,
            "extra_regeneration_rows": 779110,
            "extra_projection_groups": 97939,
            "candidate_peak_source_plus_transient_rows": 2572,
            "candidate_peak_rank": 69648
          },
          {
            "window_batches": 32,
            "extra_first_layer_edge_visits": 2226521,
            "extra_edge_visit_ratio": 0.402391991345,
            "extra_regeneration_rows": 671844,
            "extra_projection_groups": 85286,
            "candidate_peak_source_plus_transient_rows": 2595,
            "candidate_peak_rank": 69859
          },
          {
            "window_batches": 64,
            "extra_first_layer_edge_visits": 1923260,
            "extra_edge_visit_ratio": 0.347584604535,
            "extra_regeneration_rows": 581472,
            "extra_projection_groups": 73886,
            "candidate_peak_source_plus_transient_rows": 2634,
            "candidate_peak_rank": 69770
          },
          {
            "window_batches": 128,
            "extra_first_layer_edge_visits": 1652295,
            "extra_edge_visit_ratio": 0.298613970109,
            "extra_regeneration_rows": 500167,
            "extra_projection_groups": 64093,
            "candidate_peak_source_plus_transient_rows": 2873,
            "candidate_peak_rank": 70753
          },
          {
            "window_batches": 256,
            "extra_first_layer_edge_visits": 1351528,
            "extra_edge_visit_ratio": 0.244257315911,
            "extra_regeneration_rows": 409549,
            "extra_projection_groups": 53020,
            "candidate_peak_source_plus_transient_rows": 3139,
            "candidate_peak_rank": 70777
          },
          {
            "window_batches": 512,
            "extra_first_layer_edge_visits": 996383,
            "extra_edge_visit_ratio": 0.180073100372,
            "extra_regeneration_rows": 301780,
            "extra_projection_groups": 40333,
            "candidate_peak_source_plus_transient_rows": 4442,
            "candidate_peak_rank": 43363
          },
          {
            "window_batches": 1024,
            "extra_first_layer_edge_visits": 574248,
            "extra_edge_visit_ratio": 0.103781997226,
            "extra_regeneration_rows": 173153,
            "extra_projection_groups": 25256,
            "candidate_peak_source_plus_transient_rows": 7516,
            "candidate_peak_rank": 43152
          },
          {
            "window_batches": 0,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 23156,
            "candidate_peak_rank": 70778
          }
        ]
      },
      {
        "schema": "recompute-window-v1",
        "status": "PASS",
        "graph": "reddit",
        "csr_path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/reddit/reddit.csrbin",
        "n": 232965,
        "e": 114615892,
        "batch_rows": 16,
        "batches": 14561,
        "windows": [
          {
            "window_batches": -1,
            "extra_first_layer_edge_visits": 8853220177,
            "extra_edge_visit_ratio": 77.242518664,
            "extra_regeneration_rows": 9153623,
            "extra_projection_groups": 574733,
            "candidate_peak_source_plus_transient_rows": 171711,
            "candidate_peak_rank": 12256
          },
          {
            "window_batches": 1,
            "extra_first_layer_edge_visits": 4287358428,
            "extra_edge_visit_ratio": 37.4063173369,
            "extra_regeneration_rows": 5247419,
            "extra_projection_groups": 330603,
            "candidate_peak_source_plus_transient_rows": 171711,
            "candidate_peak_rank": 12256
          },
          {
            "window_batches": 2,
            "extra_first_layer_edge_visits": 3099184499,
            "extra_edge_visit_ratio": 27.0397450556,
            "extra_regeneration_rows": 3872522,
            "extra_projection_groups": 244689,
            "candidate_peak_source_plus_transient_rows": 186072,
            "candidate_peak_rank": 14556
          },
          {
            "window_batches": 4,
            "extra_first_layer_edge_visits": 2144115143,
            "extra_edge_visit_ratio": 18.706962059,
            "extra_regeneration_rows": 2713633,
            "extra_projection_groups": 172233,
            "candidate_peak_source_plus_transient_rows": 208810,
            "candidate_peak_rank": 14557
          },
          {
            "window_batches": 8,
            "extra_first_layer_edge_visits": 1423413967,
            "extra_edge_visit_ratio": 12.4189930573,
            "extra_regeneration_rows": 1794358,
            "extra_projection_groups": 114798,
            "candidate_peak_source_plus_transient_rows": 216421,
            "candidate_peak_rank": 14555
          },
          {
            "window_batches": 16,
            "extra_first_layer_edge_visits": 922649218,
            "extra_edge_visit_ratio": 8.04992398436,
            "extra_regeneration_rows": 1159443,
            "extra_projection_groups": 75126,
            "candidate_peak_source_plus_transient_rows": 221858,
            "candidate_peak_rank": 14554
          },
          {
            "window_batches": 32,
            "extra_first_layer_edge_visits": 585355154,
            "extra_edge_visit_ratio": 5.10710289634,
            "extra_regeneration_rows": 736147,
            "extra_projection_groups": 48662,
            "candidate_peak_source_plus_transient_rows": 225281,
            "candidate_peak_rank": 14551
          },
          {
            "window_batches": 64,
            "extra_first_layer_edge_visits": 360611014,
            "extra_edge_visit_ratio": 3.14625666395,
            "extra_regeneration_rows": 455737,
            "extra_projection_groups": 31133,
            "candidate_peak_source_plus_transient_rows": 226717,
            "candidate_peak_rank": 14548
          },
          {
            "window_batches": 128,
            "extra_first_layer_edge_visits": 214113112,
            "extra_edge_visit_ratio": 1.86809270742,
            "extra_regeneration_rows": 274570,
            "extra_projection_groups": 19772,
            "candidate_peak_source_plus_transient_rows": 228132,
            "candidate_peak_rank": 14542
          },
          {
            "window_batches": 256,
            "extra_first_layer_edge_visits": 121341588,
            "extra_edge_visit_ratio": 1.05868030936,
            "extra_regeneration_rows": 157435,
            "extra_projection_groups": 12439,
            "candidate_peak_source_plus_transient_rows": 228990,
            "candidate_peak_rank": 14539
          },
          {
            "window_batches": 512,
            "extra_first_layer_edge_visits": 64038354,
            "extra_edge_visit_ratio": 0.558721420586,
            "extra_regeneration_rows": 82427,
            "extra_projection_groups": 7713,
            "candidate_peak_source_plus_transient_rows": 229597,
            "candidate_peak_rank": 14534
          },
          {
            "window_batches": 1024,
            "extra_first_layer_edge_visits": 31329901,
            "extra_edge_visit_ratio": 0.273346919466,
            "extra_regeneration_rows": 38553,
            "extra_projection_groups": 4917,
            "candidate_peak_source_plus_transient_rows": 230153,
            "candidate_peak_rank": 14531
          },
          {
            "window_batches": 0,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 231089,
            "candidate_peak_rank": 14511
          }
        ]
      },
      {
        "schema": "recompute-window-v1",
        "status": "PASS",
        "graph": "mycielskian19",
        "csr_path": "/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/mycielskian19/mycielskian19.csrbin",
        "n": 393215,
        "e": 903194710,
        "batch_rows": 16,
        "batches": 24576,
        "windows": [
          {
            "window_batches": -1,
            "extra_first_layer_edge_visits": 731433809,
            "extra_edge_visit_ratio": 0.809829598094,
            "extra_regeneration_rows": 360448,
            "extra_projection_groups": 22528,
            "candidate_peak_source_plus_transient_rows": 393184,
            "candidate_peak_rank": 24574
          },
          {
            "window_batches": 1,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 2,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 4,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 8,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 16,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 32,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 64,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 128,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 256,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 512,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 1024,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          },
          {
            "window_batches": 0,
            "extra_first_layer_edge_visits": 0,
            "extra_edge_visit_ratio": 0,
            "extra_regeneration_rows": 0,
            "extra_projection_groups": 0,
            "candidate_peak_source_plus_transient_rows": 393200,
            "candidate_peak_rank": 24575
          }
        ]
      }
    ],
    "performance_claim": false,
    "no_numa_interleave": true,
    "limitations": [
      "logical proxies only",
      "no partial/ready state",
      "no DRAM/cache/synchronization",
      "no legal full-kernel schedule"
    ]
  },
  "node": "qhcn282",
  "job": "10900001",
  "paper_eligible": false,
  "issues": "Logical source reuse-window audit only; no speedup or DRAM claim.",
  "next": "Compare W tradeoffs with row-complete baseline; develop no kernel if AMX or AVX work erases capacity benefit.",
  "date": "2026-10-08T10:51:13.660008+00:00"
}
```

## recompute-window-analysis-20261008-04

```json
{
  "id": "recompute-window-analysis-20261008-04",
  "status": "ANALYZED_PASS",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/results/recompute_window_20261008/retry-04-analysis",
  "job": "10900001",
  "paper_eligible": false,
  "issues": "Source-aware reuse-window scan passed. Only RoadNet-CA W=1024 has a plausible capacity/work tradeoff; all conclusions are logical proxies without performance claims.",
  "next": "If pursuing a prototype, implement only RoadNet-CA W=1024 with exact BF16/FP32 numerical gates and compare against row-complete baseline; stop on AMX/synchronization loss.",
  "desktop_write": false,
  "numa_interleave": false,
  "date": "2026-10-08T10:54:06.238323+00:00"
}
```

## recompute-window-kernel-build-20261008-193715

```json
{
  "id": "recompute-window-kernel-build-20261008-193715",
  "kind": "recompute_window_kernel_build",
  "status": "FAILED",
  "exit_status": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-kernel-build-20261008-193715",
  "paper_eligible": false,
  "issues": "Isolated single-thread shadow prototype; no paper performance claim",
  "next": "Run correctness/timing on RoadNet-CA if build passes",
  "date": "2026-10-08T11:37:19.354903+00:00"
}
```

## recompute-window-kernel-build-20261008-193923

```json
{
  "id": "recompute-window-kernel-build-20261008-193923",
  "kind": "recompute_window_kernel_build",
  "status": "FAILED",
  "exit_status": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-kernel-build-20261008-193923",
  "paper_eligible": false,
  "issues": "Isolated single-thread shadow prototype; no paper performance claim",
  "next": "Run correctness/timing on RoadNet-CA if build passes",
  "date": "2026-10-08T11:39:25.129674+00:00"
}
```

## recompute-window-kernel-build-20261008-194137

```json
{
  "id": "recompute-window-kernel-build-20261008-194137",
  "kind": "recompute_window_kernel_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-kernel-build-20261008-194137",
  "paper_eligible": false,
  "issues": "Isolated single-thread shadow prototype; no paper performance claim",
  "next": "Run correctness/timing on RoadNet-CA if build passes",
  "date": "2026-10-08T11:41:44.789189+00:00"
}
```

## recompute-window-kernel-10900289

```json
{
  "id": "recompute-window-kernel-10900289",
  "kind": "recompute_window_kernel",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10900289",
  "node": "qhcn022",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-kernel-10900289",
  "paper_eligible": false,
  "output_gate": "WINDOW_GATE pass=1 baseline_ms=1237.267971 window_ms=1267.936945",
  "issues": "Single-thread isolated shadow prototype; no multi-thread or paper performance claim",
  "next": "If exact gate passes, analyze timing/work tradeoff and decide whether a parallel cache design is justified",
  "date": "2026-10-08T11:45:17.243658+00:00"
}
```

## recompute-window-sweep-10900343

```json
{
  "id": "recompute-window-sweep-10900343",
  "kind": "recompute_window_sweep",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10900343",
  "node": "qhcn022",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-sweep-10900343",
  "paper_eligible": false,
  "summary": {
    "schema": "recompute-window-sweep-v1",
    "status": "PASS",
    "job_id": "10900343",
    "rows": [
      {
        "file": "window-1.log",
        "baseline_ms": 1227.916956,
        "w0_ms": 1061.644793,
        "w0_exact": 1,
        "window": 1,
        "window_ms": 1374.003887,
        "extra_source_rows": 2706254,
        "peak_cache_rows": 133,
        "window_exact": 1
      }
    ],
    "gates": [
      "WINDOW_GATE pass=1 baseline_ms=1227.916956 window_ms=1374.003887"
    ]
  },
  "next": "Use legal work/state frontier to choose a parallel design or close the window path",
  "date": "2026-10-08T11:57:16.851987+00:00"
}
```

## recompute-window-sweep-10900360

```json
{
  "id": "recompute-window-sweep-10900360",
  "kind": "recompute_window_sweep",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10900360",
  "node": "qhcn022",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-sweep-10900360",
  "paper_eligible": false,
  "summary": {
    "schema": "recompute-window-sweep-v1",
    "status": "PASS",
    "job_id": "10900360",
    "expected_windows": [
      1,
      4,
      16,
      64,
      256,
      512,
      1024,
      2048,
      4096
    ],
    "rows": [
      {
        "file": "window-1.log",
        "baseline_ms": 1239.371061,
        "w0_ms": 1067.980051,
        "w0_exact": 1,
        "window": 1,
        "window_ms": 1371.631145,
        "extra_source_rows": 2706254,
        "peak_cache_rows": 133,
        "window_exact": 1
      },
      {
        "file": "window-1024.log",
        "baseline_ms": 1213.727951,
        "w0_ms": 1058.331013,
        "w0_exact": 1,
        "window": 1024,
        "window_ms": 1266.909838,
        "extra_source_rows": 1687277,
        "peak_cache_rows": 2902,
        "window_exact": 1
      },
      {
        "file": "window-16.log",
        "baseline_ms": 1231.703997,
        "w0_ms": 1055.78804,
        "w0_exact": 1,
        "window": 16,
        "window_ms": 1331.199169,
        "extra_source_rows": 2171634,
        "peak_cache_rows": 186,
        "window_exact": 1
      },
      {
        "file": "window-2048.log",
        "baseline_ms": 1218.735933,
        "w0_ms": 1073.573112,
        "w0_exact": 1,
        "window": 2048,
        "window_ms": 1253.195047,
        "extra_source_rows": 1627021,
        "peak_cache_rows": 5694,
        "window_exact": 1
      },
      {
        "file": "window-256.log",
        "baseline_ms": 1224.417925,
        "w0_ms": 1055.065155,
        "w0_exact": 1,
        "window": 256,
        "window_ms": 1287.703991,
        "extra_source_rows": 1818005,
        "peak_cache_rows": 1281,
        "window_exact": 1
      },
      {
        "file": "window-4.log",
        "baseline_ms": 1239.20989,
        "w0_ms": 1073.611021,
        "w0_exact": 1,
        "window": 4,
        "window_ms": 1364.579916,
        "extra_source_rows": 2426988,
        "peak_cache_rows": 133,
        "window_exact": 1
      },
      {
        "file": "window-4096.log",
        "baseline_ms": 1215.965033,
        "w0_ms": 1057.734013,
        "w0_exact": 1,
        "window": 4096,
        "window_ms": 1257.085085,
        "extra_source_rows": 1563643,
        "peak_cache_rows": 9404,
        "window_exact": 1
      },
      {
        "file": "window-512.log",
        "baseline_ms": 1232.631922,
        "w0_ms": 1053.532124,
        "w0_exact": 1,
        "window": 512,
        "window_ms": 1269.396067,
        "extra_source_rows": 1749820,
        "peak_cache_rows": 1891,
        "window_exact": 1
      },
      {
        "file": "window-64.log",
        "baseline_ms": 1219.882011,
        "w0_ms": 1055.017948,
        "w0_exact": 1,
        "window": 64,
        "window_ms": 1300.692081,
        "extra_source_rows": 1971596,
        "peak_cache_rows": 495,
        "window_exact": 1
      }
    ],
    "gates": [
      "WINDOW_GATE pass=1 baseline_ms=1239.371061 window_ms=1371.631145",
      "WINDOW_GATE pass=1 baseline_ms=1213.727951 window_ms=1266.909838",
      "WINDOW_GATE pass=1 baseline_ms=1231.703997 window_ms=1331.199169",
      "WINDOW_GATE pass=1 baseline_ms=1218.735933 window_ms=1253.195047",
      "WINDOW_GATE pass=1 baseline_ms=1224.417925 window_ms=1287.703991",
      "WINDOW_GATE pass=1 baseline_ms=1239.209890 window_ms=1364.579916",
      "WINDOW_GATE pass=1 baseline_ms=1215.965033 window_ms=1257.085085",
      "WINDOW_GATE pass=1 baseline_ms=1232.631922 window_ms=1269.396067",
      "WINDOW_GATE pass=1 baseline_ms=1219.882011 window_ms=1300.692081"
    ]
  },
  "next": "Use legal work/state frontier to choose a parallel design or close the window path",
  "date": "2026-10-08T12:04:08.965176+00:00"
}
```

## recompute-window-parallel-build-20261008-202152

```json
{
  "id": "recompute-window-parallel-build-20261008-202152",
  "kind": "recompute_window_parallel_build",
  "status": "FAILED",
  "exit_status": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-parallel-build-20261008-202152",
  "paper_eligible": false,
  "issues": "Isolated 32-thread shadow prototype; no paper performance claim",
  "next": "Run the matching RoadNet-CA correctness/timing gate if build passes",
  "date": "2026-10-08T12:21:54.894820+00:00"
}
```

## recompute-window-parallel-build-20261008-202420

```json
{
  "id": "recompute-window-parallel-build-20261008-202420",
  "kind": "recompute_window_parallel_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-parallel-build-20261008-202420",
  "paper_eligible": false,
  "issues": "Isolated 32-thread shadow prototype; no paper performance claim",
  "next": "Run the matching RoadNet-CA correctness/timing gate if build passes",
  "date": "2026-10-08T12:24:23.669777+00:00"
}
```

## recompute-window-parallel-10900662

```json
{
  "id": "recompute-window-parallel-10900662",
  "kind": "recompute_window_parallel",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10900662",
  "node": "qhcn003",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-parallel-10900662",
  "paper_eligible": false,
  "par_result": "PAR_RESULT graph=roadNet-CA threads=32 baseline_median_ms=96.892118454 candidate_median_ms=113.595008850 exact=1 source_rows=3627178 source_projection_groups=273630 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
  "issues": "Isolated 32-thread shadow prototype; no paper performance claim",
  "next": "Use exact result and source-duplication cost to decide whether to continue parallel fusion",
  "date": "2026-10-08T12:41:17.898180+00:00"
}
```

## source-partition-build-20261008-210228

```json
{
  "id": "source-partition-build-20261008-210228",
  "kind": "source_partition_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-partition-build-20261008-210228",
  "paper_eligible": false,
  "issues": "Structural source-overlap audit; no runtime claim",
  "next": "Run analyzer on RoadNet-CA if build passes",
  "date": "2026-10-08T13:02:30.864907+00:00"
}
```

## source-partition-10901309

```json
{
  "id": "source-partition-10901309",
  "kind": "source_partition_audit",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10901309",
  "node": "qhcn022",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-partition-10901309",
  "paper_eligible": false,
  "partition_lines": [
    "PARTITION order=identity parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=2088066 duplication=1.062517619 min_unique=62228 max_unique=68053 min_edges=162112 max_edges=191370 edge_check=5533214",
    "PARTITION order=degree parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=3524721 duplication=1.793563118 min_unique=49438 max_unique=185279 min_edges=55527 max_edges=262667 edge_check=5533214",
    "PARTITION order=degree_desc parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=3528312 duplication=1.795390407 min_unique=49447 max_unique=183261 min_edges=55528 max_edges=262663 edge_check=5533214",
    "PARTITION order=min_source parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=2043374 duplication=1.039775983 min_unique=56224 max_unique=65824 min_edges=157783 max_edges=192723 edge_check=5533214",
    "PARTITION order=avg_source parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=2085989 duplication=1.061460732 min_unique=55938 max_unique=69003 min_edges=156848 max_edges=191156 edge_check=5533214",
    "PARTITION order=max_source parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=2083393 duplication=1.060139751 min_unique=55934 max_unique=69715 min_edges=156693 max_edges=190653 edge_check=5533214"
  ],
  "issues": "Structural source-overlap audit; no runtime claim",
  "next": "Choose a partition only if it reduces source duplication and remains compatible with the kernel",
  "date": "2026-10-08T13:15:06.197886+00:00"
}
```

## recompute-window-parallel-build-20261008-211935

```json
{
  "id": "recompute-window-parallel-build-20261008-211935",
  "kind": "recompute_window_parallel_build",
  "status": "FAILED",
  "exit_status": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-parallel-build-20261008-211935",
  "paper_eligible": false,
  "issues": "Isolated 32-thread shadow prototype; no paper performance claim",
  "next": "Run the matching RoadNet-CA correctness/timing gate if build passes",
  "date": "2026-10-08T13:19:37.312679+00:00"
}
```

## recompute-window-parallel-build-20261008-212208

```json
{
  "id": "recompute-window-parallel-build-20261008-212208",
  "kind": "recompute_window_parallel_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-parallel-build-20261008-212208",
  "paper_eligible": false,
  "issues": "Isolated 32-thread shadow prototype; no paper performance claim",
  "next": "Run the matching RoadNet-CA correctness/timing gate if build passes",
  "date": "2026-10-08T13:22:12.610126+00:00"
}
```

## recompute-window-parallel-10901373

```json
{
  "id": "recompute-window-parallel-10901373",
  "kind": "recompute_window_parallel",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10901373",
  "node": "qhcn002",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-parallel-10901373",
  "paper_eligible": false,
  "par_result": "PAR_RESULT order=min_source graph=roadNet-CA threads=32 baseline_median_ms=32.449007034 candidate_median_ms=29.790878296 exact=1 source_rows=2043040 source_projection_groups=184732 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
  "issues": "Isolated 32-thread shadow prototype; no paper performance claim",
  "next": "Use exact result and source-duplication cost to decide whether to continue parallel fusion",
  "date": "2026-10-08T13:24:22.596201+00:00"
}
```

## source-partition-cross-10901392

```json
{
  "id": "source-partition-cross-10901392",
  "kind": "source_partition_cross_graph",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10901392",
  "node": "qhcn006",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-partition-cross-10901392",
  "paper_eligible": false,
  "partition_lines": [
    "PARTITION order=identity parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1717716 duplication=4.259025271 min_unique=31432 max_unique=60187 min_edges=86912 max_edges=122552 edge_check=3387388",
    "PARTITION order=degree parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1795368 duplication=4.451561074 min_unique=11054 max_unique=71968 min_edges=11651 max_edges=126070 edge_check=3387388",
    "PARTITION order=degree_desc parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1793904 duplication=4.447931130 min_unique=11045 max_unique=72509 min_edges=11652 max_edges=126070 edge_check=3387388",
    "PARTITION order=min_source parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1360221 duplication=3.372627147 min_unique=19397 max_unique=49234 min_edges=78821 max_edges=115637 edge_check=3387388",
    "PARTITION order=avg_source parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1757633 duplication=4.357998274 min_unique=19477 max_unique=67433 min_edges=89446 max_edges=114751 edge_check=3387388",
    "PARTITION order=max_source parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1643112 duplication=4.074046892 min_unique=23470 max_unique=58850 min_edges=99138 max_edges=110237 edge_check=3387388",
    "PARTITION order=identity parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=6533128 duplication=28.043388492 min_unique=202714 max_unique=205390 min_edges=3328028 max_edges=3792207 edge_check=114615892",
    "PARTITION order=degree parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=5035589 duplication=21.615216878 min_unique=13624 max_unique=207222 min_edges=17916 max_edges=27322262 edge_check=114615892",
    "PARTITION order=degree_desc parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=5035972 duplication=21.616860902 min_unique=13655 max_unique=207219 min_edges=17921 max_edges=27320001 edge_check=114615892",
    "PARTITION order=min_source parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=4722199 duplication=20.269993347 min_unique=22492 max_unique=212060 min_edges=34245 max_edges=13245397 edge_check=114615892",
    "PARTITION order=avg_source parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=5363035 duplication=23.020775653 min_unique=47126 max_unique=205478 min_edges=105781 max_edges=7929093 edge_check=114615892",
    "PARTITION order=max_source parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=4768128 duplication=20.467143133 min_unique=22359 max_unique=200015 min_edges=34065 max_edges=11918279 edge_check=114615892"
  ],
  "issues": "Structural source-overlap audit; no runtime claim",
  "next": "Run the parallel fusion kernel only on graphs with low source duplication",
  "date": "2026-10-08T13:30:18.267012+00:00"
}
```

## recompute-window-parallel-cross-10901444

```json
{
  "id": "recompute-window-parallel-cross-10901444",
  "kind": "recompute_window_parallel_cross",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10901444",
  "node": "qhcn140",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-parallel-cross-10901444",
  "paper_eligible": false,
  "results": [
    "PAR_RESULT order=degree graph=amazon0601 threads=32 baseline_median_ms=447.346925735 candidate_median_ms=399.296998978 exact=1 source_rows=1840149 source_projection_groups=126085 second_projection_groups=25213 peak_cache_rows=75480 checksum=3449434827757982282",
    "PAR_RESULT order=identity graph=amazon0601 threads=32 baseline_median_ms=427.399873734 candidate_median_ms=339.286088943 exact=1 source_rows=1717770 source_projection_groups=119141 second_projection_groups=25213 peak_cache_rows=60192 checksum=3449434827757982282",
    "PAR_RESULT order=min_source graph=amazon0601 threads=32 baseline_median_ms=560.470104218 candidate_median_ms=384.896039963 exact=1 source_rows=1357690 source_projection_groups=96659 second_projection_groups=25213 peak_cache_rows=49722 checksum=3449434827757982282",
    "PAR_RESULT order=degree graph=reddit threads=32 baseline_median_ms=1498.723983765 candidate_median_ms=15128.699064255 exact=1 source_rows=5035725 source_projection_groups=321514 second_projection_groups=14561 peak_cache_rows=207222 checksum=7310392754903689554",
    "PAR_RESULT order=identity graph=reddit threads=32 baseline_median_ms=1554.455995560 candidate_median_ms=15640.945911407 exact=1 source_rows=6533110 source_projection_groups=415126 second_projection_groups=14561 peak_cache_rows=205390 checksum=7310392754903689554",
    "PAR_RESULT order=min_source graph=reddit threads=32 baseline_median_ms=1170.876026154 candidate_median_ms=13622.236013412 exact=1 source_rows=4721996 source_projection_groups=301994 second_projection_groups=14561 peak_cache_rows=212072 checksum=7310392754903689554"
  ],
  "gates": [
    "PAR_GATE order=degree pass=1 speedup=1.120336308",
    "PAR_GATE order=identity pass=1 speedup=1.259703500",
    "PAR_GATE order=min_source pass=1 speedup=1.456159706",
    "PAR_GATE order=degree pass=1 speedup=0.099064961",
    "PAR_GATE order=identity pass=1 speedup=0.099383759",
    "PAR_GATE order=min_source pass=1 speedup=0.085953292"
  ],
  "issues": "Exploratory cross-graph source-duplication boundary; no paper performance claim",
  "next": "Use the measured boundary to define an offline policy or close the candidate",
  "date": "2026-10-08T13:39:29.272264+00:00"
}
```

## recompute-window-parallel-exclusive-10901902

```json
{
  "id": "recompute-window-parallel-exclusive-10901902",
  "kind": "recompute_window_parallel_exclusive",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10901902",
  "node": "qhcn819",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-parallel-exclusive-10901902",
  "paper_eligible": false,
  "results": [
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 baseline_median_ms=39.532184601 candidate_median_ms=70.284128189 exact=1 source_rows=3627178 source_projection_groups=273630 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 baseline_median_ms=31.248092651 candidate_median_ms=30.419111252 exact=1 source_rows=2088038 source_projection_groups=189693 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 baseline_median_ms=33.013105392 candidate_median_ms=29.920101166 exact=1 source_rows=2043040 source_projection_groups=184732 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=amazon0601 threads=32 baseline_median_ms=11.162996292 candidate_median_ms=34.625053406 exact=1 source_rows=1840149 source_projection_groups=126085 second_projection_groups=25213 peak_cache_rows=75480 checksum=3449434827757982282",
    "PAR_RESULT order=identity graph=amazon0601 threads=32 baseline_median_ms=9.612083435 candidate_median_ms=28.056859970 exact=1 source_rows=1717770 source_projection_groups=119141 second_projection_groups=25213 peak_cache_rows=60192 checksum=3449434827757982282",
    "PAR_RESULT order=min_source graph=amazon0601 threads=32 baseline_median_ms=10.689973831 candidate_median_ms=23.940086365 exact=1 source_rows=1357690 source_projection_groups=96659 second_projection_groups=25213 peak_cache_rows=49722 checksum=3449434827757982282"
  ],
  "gates": [
    "PAR_GATE order=degree pass=1 speedup=0.562462474",
    "PAR_GATE order=identity pass=1 speedup=1.027251993",
    "PAR_GATE order=min_source pass=1 speedup=1.103375460",
    "PAR_GATE order=degree pass=1 speedup=0.322396508",
    "PAR_GATE order=identity pass=1 speedup=0.342592986",
    "PAR_GATE order=min_source pass=1 speedup=0.446530295"
  ],
  "issues": "Exclusive exploratory validation; not a final paper table",
  "next": "Repeat positive orders on another graph or build a source-aware cost model",
  "date": "2026-10-08T13:50:18.115836+00:00"
}
```

## source-partition-cross-10902064

```json
{
  "id": "source-partition-cross-10902064",
  "kind": "source_partition_screen_graphs",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10902064",
  "node": "qhcn044",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-partition-cross-10902064",
  "paper_eligible": false,
  "partition_lines": [
    "PARTITION order=identity parts=32 n=916428 e=5105039 global_sources=714545 total_unique_sources=3215181 duplication=4.499620038 min_unique=98873 max_unique=102281 min_edges=156294 max_edges=162422 edge_check=5105039",
    "PARTITION order=degree parts=32 n=916428 e=5105039 global_sources=714545 total_unique_sources=1878156 duplication=2.628464267 min_unique=0 max_unique=209681 min_edges=0 max_edges=725597 edge_check=5105039",
    "PARTITION order=degree_desc parts=32 n=916428 e=5105039 global_sources=714545 total_unique_sources=1881636 duplication=2.633334500 min_unique=0 max_unique=210337 min_edges=0 max_edges=725578 edge_check=5105039",
    "PARTITION order=min_source parts=32 n=916428 e=5105039 global_sources=714545 total_unique_sources=1432606 duplication=2.004920614 min_unique=0 max_unique=93197 min_edges=0 max_edges=345332 edge_check=5105039",
    "PARTITION order=avg_source parts=32 n=916428 e=5105039 global_sources=714545 total_unique_sources=2123340 duplication=2.971597310 min_unique=0 max_unique=131567 min_edges=0 max_edges=291663 edge_check=5105039",
    "PARTITION order=max_source parts=32 n=916428 e=5105039 global_sources=714545 total_unique_sources=1440754 duplication=2.016323675 min_unique=0 max_unique=75489 min_edges=0 max_edges=290132 edge_check=5105039",
    "PARTITION order=identity parts=32 n=2394385 e=5021410 global_sources=2369181 total_unique_sources=2495159 duplication=1.053173649 min_unique=445 max_unique=2290053 min_edges=551 max_edges=4706235 edge_check=5021410",
    "PARTITION order=degree parts=32 n=2394385 e=5021410 global_sources=2369181 total_unique_sources=2389488 duplication=1.008571316 min_unique=0 max_unique=2365266 min_edges=0 max_edges=4948633 edge_check=5021410",
    "PARTITION order=degree_desc parts=32 n=2394385 e=5021410 global_sources=2369181 total_unique_sources=2390095 duplication=1.008827523 min_unique=0 max_unique=2364115 min_edges=0 max_edges=4948632 edge_check=5021410",
    "PARTITION order=min_source parts=32 n=2394385 e=5021410 global_sources=2369181 total_unique_sources=2440932 duplication=1.030285149 min_unique=0 max_unique=2313793 min_edges=0 max_edges=4828734 edge_check=5021410",
    "PARTITION order=avg_source parts=32 n=2394385 e=5021410 global_sources=2369181 total_unique_sources=2386555 duplication=1.007333336 min_unique=0 max_unique=2366847 min_edges=0 max_edges=4873671 edge_check=5021410",
    "PARTITION order=max_source parts=32 n=2394385 e=5021410 global_sources=2369181 total_unique_sources=2380611 duplication=1.004824452 min_unique=0 max_unique=2368996 min_edges=0 max_edges=4915834 edge_check=5021410",
    "PARTITION order=identity parts=32 n=36692 e=367662 global_sources=36692 total_unique_sources=107840 duplication=2.939060286 min_unique=604 max_unique=20061 min_edges=1803 max_edges=108867 edge_check=367662",
    "PARTITION order=degree parts=32 n=36692 e=367662 global_sources=36692 total_unique_sources=110169 duplication=3.002534612 min_unique=59 max_unique=27774 min_edges=1146 max_edges=164718 edge_check=367662",
    "PARTITION order=degree_desc parts=32 n=36692 e=367662 global_sources=36692 total_unique_sources=109963 duplication=2.996920310 min_unique=60 max_unique=27769 min_edges=1146 max_edges=164661 edge_check=367662",
    "PARTITION order=min_source parts=32 n=36692 e=367662 global_sources=36692 total_unique_sources=112469 duplication=3.065218576 min_unique=192 max_unique=24824 min_edges=1534 max_edges=100262 edge_check=367662",
    "PARTITION order=avg_source parts=32 n=36692 e=367662 global_sources=36692 total_unique_sources=137361 duplication=3.743622588 min_unique=180 max_unique=8947 min_edges=1416 max_edges=30537 edge_check=367662",
    "PARTITION order=max_source parts=32 n=36692 e=367662 global_sources=36692 total_unique_sources=110316 duplication=3.006540935 min_unique=32 max_unique=6713 min_edges=1185 max_edges=26352 edge_check=367662",
    "PARTITION order=identity parts=32 n=1134890 e=5975248 global_sources=1134890 total_unique_sources=2600143 duplication=2.291096935 min_unique=22352 max_unique=409818 min_edges=51445 max_edges=1473367 edge_check=5975248",
    "PARTITION order=degree parts=32 n=1134890 e=5975248 global_sources=1134890 total_unique_sources=2249681 duplication=1.982289914 min_unique=2300 max_unique=721207 min_edges=35465 max_edges=3008086 edge_check=5975248",
    "PARTITION order=degree_desc parts=32 n=1134890 e=5975248 global_sources=1134890 total_unique_sources=2249447 duplication=1.982083726 min_unique=2889 max_unique=720164 min_edges=35465 max_edges=3008064 edge_check=5975248",
    "PARTITION order=min_source parts=32 n=1134890 e=5975248 global_sources=1134890 total_unique_sources=2464683 duplication=2.171737349 min_unique=14834 max_unique=460420 min_edges=44604 max_edges=1606024 edge_check=5975248",
    "PARTITION order=avg_source parts=32 n=1134890 e=5975248 global_sources=1134890 total_unique_sources=2963958 duplication=2.611669853 min_unique=1465 max_unique=155027 min_edges=45347 max_edges=415418 edge_check=5975248",
    "PARTITION order=max_source parts=32 n=1134890 e=5975248 global_sources=1134890 total_unique_sources=2704082 duplication=2.382682022 min_unique=1133 max_unique=430590 min_edges=41915 max_edges=1045595 edge_check=5975248",
    "PARTITION order=identity parts=32 n=1632803 e=30622564 global_sources=1519452 total_unique_sources=12440258 duplication=8.187332012 min_unique=52283 max_unique=655999 min_edges=62639 max_edges=2422780 edge_check=30622564",
    "PARTITION order=degree parts=32 n=1632803 e=30622564 global_sources=1519452 total_unique_sources=11501744 duplication=7.569665906 min_unique=0 max_unique=986412 min_edges=0 max_edges=6725355 edge_check=30622564",
    "PARTITION order=degree_desc parts=32 n=1632803 e=30622564 global_sources=1519452 total_unique_sources=11494353 duplication=7.564801652 min_unique=0 max_unique=985703 min_edges=0 max_edges=6725269 edge_check=30622564",
    "PARTITION order=min_source parts=32 n=1632803 e=30622564 global_sources=1519452 total_unique_sources=11790098 duplication=7.759440904 min_unique=0 max_unique=688660 min_edges=0 max_edges=2701676 edge_check=30622564",
    "PARTITION order=avg_source parts=32 n=1632803 e=30622564 global_sources=1519452 total_unique_sources=11794045 duplication=7.762038551 min_unique=0 max_unique=563863 min_edges=0 max_edges=1654609 edge_check=30622564",
    "PARTITION order=max_source parts=32 n=1632803 e=30622564 global_sources=1519452 total_unique_sources=12754851 duplication=8.394375735 min_unique=0 max_unique=578172 min_edges=0 max_edges=1415090 edge_check=30622564",
    "PARTITION order=identity parts=32 n=3774768 e=16518948 global_sources=3258983 total_unique_sources=10485830 duplication=3.217516016 min_unique=0 max_unique=754121 min_edges=0 max_edges=1301847 edge_check=16518948",
    "PARTITION order=degree parts=32 n=3774768 e=16518948 global_sources=3258983 total_unique_sources=10546598 duplication=3.236162324 min_unique=0 max_unique=1271581 min_edges=0 max_edges=3832995 edge_check=16518948",
    "PARTITION order=degree_desc parts=32 n=3774768 e=16518948 global_sources=3258983 total_unique_sources=10559604 duplication=3.240153140 min_unique=0 max_unique=1275145 min_edges=0 max_edges=3832976 edge_check=16518948",
    "PARTITION order=min_source parts=32 n=3774768 e=16518948 global_sources=3258983 total_unique_sources=9275730 duplication=2.846203862 min_unique=0 max_unique=777144 min_edges=0 max_edges=1291740 edge_check=16518948",
    "PARTITION order=avg_source parts=32 n=3774768 e=16518948 global_sources=3258983 total_unique_sources=9805760 duplication=3.008840488 min_unique=0 max_unique=609926 min_edges=0 max_edges=1087265 edge_check=16518948",
    "PARTITION order=max_source parts=32 n=3774768 e=16518948 global_sources=3258983 total_unique_sources=9609474 duplication=2.948611269 min_unique=0 max_unique=707514 min_edges=0 max_edges=1476500 edge_check=16518948",
    "PARTITION order=identity parts=32 n=1696415 e=22190596 global_sources=1696415 total_unique_sources=6323683 duplication=3.727674537 min_unique=57069 max_unique=691147 min_edges=93115 max_edges=4388387 edge_check=22190596",
    "PARTITION order=degree parts=32 n=1696415 e=22190596 global_sources=1696415 total_unique_sources=5640293 duplication=3.324830893 min_unique=16379 max_unique=1226115 min_edges=53012 max_edges=9819715 edge_check=22190596",
    "PARTITION order=degree_desc parts=32 n=1696415 e=22190596 global_sources=1696415 total_unique_sources=5665672 duplication=3.339791266 min_unique=17734 max_unique=1223830 min_edges=53013 max_edges=9819667 edge_check=22190596",
    "PARTITION order=min_source parts=32 n=1696415 e=22190596 global_sources=1696415 total_unique_sources=6333066 duplication=3.733205613 min_unique=57071 max_unique=678395 min_edges=93118 max_edges=4262801 edge_check=22190596",
    "PARTITION order=avg_source parts=32 n=1696415 e=22190596 global_sources=1696415 total_unique_sources=7688828 duplication=4.532398028 min_unique=76960 max_unique=309844 min_edges=171038 max_edges=1608709 edge_check=22190596",
    "PARTITION order=max_source parts=32 n=1696415 e=22190596 global_sources=1696415 total_unique_sources=6241066 duplication=3.678973600 min_unique=104138 max_unique=286959 min_edges=204048 max_edges=1586445 edge_check=22190596"
  ],
  "issues": "Structural source-overlap audit; no runtime claim",
  "next": "Use low-duplication graphs for a controlled parallel fusion run",
  "date": "2026-10-08T14:06:21.819318+00:00"
}
```

## recompute-window-batch-build-20261008-221327

```json
{
  "id": "recompute-window-batch-build-20261008-221327",
  "kind": "recompute_window_batch_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-batch-build-20261008-221327",
  "paper_eligible": false,
  "issues": "Isolated source-generation packing prototype; no paper performance claim",
  "next": "Run batch-size timing/correctness gate if build passes",
  "date": "2026-10-08T14:13:40.869469+00:00"
}
```

## source-partition-cross-10902071

```json
{
  "id": "source-partition-cross-10902071",
  "kind": "source_partition_screen_graphs",
  "status": "PASS",
  "exit_status": 0,
  "job_id": "10902071",
  "node": "qhcn260",
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-partition-cross-10902071",
  "paper_eligible": false,
  "partition_lines": [
    "PARTITION order=identity parts=32 n=1139905 e=113891327 global_sources=1139905 total_unique_sources=6999788 duplication=6.140676635 min_unique=35624 max_unique=454947 min_edges=61074 max_edges=14710466 edge_check=113891327",
    "PARTITION order=degree parts=32 n=1139905 e=113891327 global_sources=1139905 total_unique_sources=10604075 duplication=9.302595392 min_unique=38471 max_unique=804085 min_edges=38582 max_edges=47234441 edge_check=113891327",
    "PARTITION order=degree_desc parts=32 n=1139905 e=113891327 global_sources=1139905 total_unique_sources=10604942 duplication=9.303355981 min_unique=35632 max_unique=804257 min_edges=38584 max_edges=47233808 edge_check=113891327",
    "PARTITION order=min_source parts=32 n=1139905 e=113891327 global_sources=1139905 total_unique_sources=10675175 duplication=9.364969011 min_unique=35624 max_unique=728304 min_edges=61074 max_edges=15110896 edge_check=113891327",
    "PARTITION order=avg_source parts=32 n=1139905 e=113891327 global_sources=1139905 total_unique_sources=9451582 duplication=8.291552366 min_unique=35624 max_unique=463848 min_edges=61074 max_edges=8981808 edge_check=113891327",
    "PARTITION order=max_source parts=32 n=1139905 e=113891327 global_sources=1139905 total_unique_sources=10214639 duplication=8.960956395 min_unique=35624 max_unique=709169 min_edges=61074 max_edges=15073309 edge_check=113891327",
    "PARTITION order=identity parts=32 n=7414866 e=194109311 global_sources=7414579 total_unique_sources=7730298 duplication=1.042580840 min_unique=236234 max_unique=249794 min_edges=2669665 max_edges=52002609 edge_check=194109311",
    "PARTITION order=degree parts=32 n=7414866 e=194109311 global_sources=7414579 total_unique_sources=13438021 duplication=1.812378154 min_unique=0 max_unique=1967462 min_edges=0 max_edges=82753251 edge_check=194109311",
    "PARTITION order=degree_desc parts=32 n=7414866 e=194109311 global_sources=7414579 total_unique_sources=13459314 duplication=1.815249929 min_unique=0 max_unique=1976526 min_edges=0 max_edges=82753174 edge_check=194109311",
    "PARTITION order=min_source parts=32 n=7414866 e=194109311 global_sources=7414579 total_unique_sources=8058705 duplication=1.086872903 min_unique=0 max_unique=398273 min_edges=0 max_edges=52997032 edge_check=194109311",
    "PARTITION order=avg_source parts=32 n=7414866 e=194109311 global_sources=7414579 total_unique_sources=8244635 duplication=1.111949175 min_unique=0 max_unique=352623 min_edges=0 max_edges=53685808 edge_check=194109311",
    "PARTITION order=max_source parts=32 n=7414866 e=194109311 global_sources=7414579 total_unique_sources=7936402 duplication=1.070377968 min_unique=0 max_unique=401733 min_edges=0 max_edges=53141531 edge_check=194109311",
    "PARTITION order=identity parts=32 n=16777216 e=265114400 global_sources=16777215 total_unique_sources=17015865 duplication=1.014224649 min_unique=528408 max_unique=532403 min_edges=8247708 max_edges=8324200 edge_check=265114400",
    "PARTITION order=degree parts=32 n=16777216 e=265114400 global_sources=16777215 total_unique_sources=127842339 duplication=7.619997658 min_unique=2507869 max_unique=5621944 min_edges=3992500 max_edges=13332674 edge_check=265114400",
    "PARTITION order=degree_desc parts=32 n=16777216 e=265114400 global_sources=16777215 total_unique_sources=127832897 duplication=7.619434870 min_unique=2506553 max_unique=5619287 min_edges=3992500 max_edges=13332674 edge_check=265114400",
    "PARTITION order=min_source parts=32 n=16777216 e=265114400 global_sources=16777215 total_unique_sources=17010481 duplication=1.013903738 min_unique=528052 max_unique=531964 min_edges=8241062 max_edges=8326196 edge_check=265114400",
    "PARTITION order=avg_source parts=32 n=16777216 e=265114400 global_sources=16777215 total_unique_sources=17127708 duplication=1.020891012 min_unique=528120 max_unique=539186 min_edges=8248241 max_edges=8324005 edge_check=265114400",
    "PARTITION order=max_source parts=32 n=16777216 e=265114400 global_sources=16777215 total_unique_sources=17011100 duplication=1.013940633 min_unique=528013 max_unique=531983 min_edges=8257775 max_edges=8324038 edge_check=265114400"
  ],
  "issues": "Structural source-overlap audit; no runtime claim",
  "next": "Use low-duplication graphs for a controlled parallel fusion run",
  "date": "2026-10-08T14:14:14.990528+00:00"
}
```

## recompute-window-batch-20261008-221757

```json
{
  "id": "recompute-window-batch-20261008-221757",
  "kind": "recompute_window_batch",
  "status": "FAILED",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-batch-20261008-221757",
  "paper_eligible": false,
  "results": [],
  "gates": [],
  "issues": "Isolated source-generation packing prototype; exploratory only",
  "next": "Compare batch sizes and decide whether AMX group packing merits a cost model",
  "date": "2026-10-08T14:18:00.710844+00:00"
}
```

## recompute-window-batch-20261008-222234

```json
{
  "id": "recompute-window-batch-20261008-222234",
  "kind": "recompute_window_batch",
  "status": "FAILED",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-batch-20261008-222234",
  "paper_eligible": false,
  "results": [
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=38.305044174 candidate_median_ms=69.370985031 exact=1 source_rows=3627178 source_projection_groups=273630 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=30.560970306 candidate_median_ms=31.352043152 exact=1 source_rows=2088038 source_projection_groups=189693 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=32.653093338 candidate_median_ms=30.661106110 exact=1 source_rows=2043040 source_projection_groups=184732 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=43.007135391 candidate_median_ms=70.934057236 exact=1 source_rows=3627178 source_projection_groups=273630 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=32.376766205 candidate_median_ms=30.194997787 exact=1 source_rows=2088038 source_projection_groups=189693 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=34.333944321 candidate_median_ms=30.143022537 exact=1 source_rows=2043040 source_projection_groups=184732 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=37.656068802 candidate_median_ms=68.984985352 exact=1 source_rows=3627178 source_projection_groups=273630 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=30.108928680 candidate_median_ms=30.247211456 exact=1 source_rows=2088038 source_projection_groups=189693 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=31.991958618 candidate_median_ms=29.467105865 exact=1 source_rows=2043040 source_projection_groups=184732 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=41.707992554 candidate_median_ms=67.502021790 exact=1 source_rows=3627178 source_projection_groups=240914 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=31.423091888 candidate_median_ms=28.633117676 exact=1 source_rows=2088038 source_projection_groups=144908 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=33.799886703 candidate_median_ms=27.815818787 exact=1 source_rows=2043040 source_projection_groups=142010 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=36.729097366 candidate_median_ms=69.965839386 exact=1 source_rows=3627178 source_projection_groups=240914 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=29.099941254 candidate_median_ms=29.235839844 exact=1 source_rows=2088038 source_projection_groups=144908 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=31.042098999 candidate_median_ms=28.213977814 exact=1 source_rows=2043040 source_projection_groups=142010 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=39.999008179 candidate_median_ms=68.391084671 exact=1 source_rows=3627178 source_projection_groups=240914 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=33.334970474 candidate_median_ms=29.088973999 exact=1 source_rows=2088038 source_projection_groups=144908 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=34.246921539 candidate_median_ms=28.262138367 exact=1 source_rows=2043040 source_projection_groups=142010 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=37.286996841 candidate_median_ms=65.308094025 exact=1 source_rows=3627178 source_projection_groups=230301 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=30.498027802 candidate_median_ms=28.760910034 exact=1 source_rows=2088038 source_projection_groups=134173 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=31.739950180 candidate_median_ms=27.458906174 exact=1 source_rows=2043040 source_projection_groups=131318 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=46.102046967 candidate_median_ms=66.855192184 exact=1 source_rows=3627178 source_projection_groups=230301 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=38.788080215 candidate_median_ms=28.275012970 exact=1 source_rows=2088038 source_projection_groups=134173 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=40.332794189 candidate_median_ms=27.987003326 exact=1 source_rows=2043040 source_projection_groups=131318 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=59.500932693 candidate_median_ms=67.405939102 exact=1 source_rows=3627178 source_projection_groups=230301 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=59.936046600 candidate_median_ms=28.178930283 exact=1 source_rows=2088038 source_projection_groups=134173 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=58.680057526 candidate_median_ms=27.934074402 exact=1 source_rows=2043040 source_projection_groups=131318 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=53.544998169 candidate_median_ms=69.644927979 exact=1 source_rows=3627178 source_projection_groups=227645 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=46.072006226 candidate_median_ms=27.600049973 exact=1 source_rows=2088038 source_projection_groups=131424 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=64.112186432 candidate_median_ms=51.671981812 exact=1 source_rows=2043040 source_projection_groups=128585 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=40.313005447 candidate_median_ms=69.262027740 exact=1 source_rows=3627178 source_projection_groups=227645 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=33.120870590 candidate_median_ms=27.323961258 exact=1 source_rows=2088038 source_projection_groups=131424 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=34.301042557 candidate_median_ms=27.261972427 exact=1 source_rows=2043040 source_projection_groups=128585 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=44.750213623 candidate_median_ms=70.407152176 exact=1 source_rows=3627178 source_projection_groups=227645 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=36.393880844 candidate_median_ms=27.804136276 exact=1 source_rows=2088038 source_projection_groups=131424 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=39.275884628 candidate_median_ms=27.341127396 exact=1 source_rows=2043040 source_projection_groups=128585 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773"
  ],
  "gates": [
    "PAR_GATE order=degree pass=1 speedup=0.552176737",
    "PAR_GATE order=identity pass=1 speedup=0.974768061",
    "PAR_GATE order=min_source pass=1 speedup=1.064967885",
    "PAR_GATE order=degree pass=1 speedup=0.606297413",
    "PAR_GATE order=identity pass=1 speedup=1.072255956",
    "PAR_GATE order=min_source pass=1 speedup=1.139034557",
    "PAR_GATE order=degree pass=1 speedup=0.545858908",
    "PAR_GATE order=identity pass=1 speedup=0.995428247",
    "PAR_GATE order=min_source pass=1 speedup=1.085683771",
    "PAR_GATE order=degree pass=1 speedup=0.617877679",
    "PAR_GATE order=identity pass=1 speedup=1.097438716",
    "PAR_GATE order=min_source pass=1 speedup=1.215131827",
    "PAR_GATE order=degree pass=1 speedup=0.524957575",
    "PAR_GATE order=identity pass=1 speedup=0.995351644",
    "PAR_GATE order=min_source pass=1 speedup=1.100238300",
    "PAR_GATE order=degree pass=1 speedup=0.584857052",
    "PAR_GATE order=identity pass=1 speedup=1.145965838",
    "PAR_GATE order=min_source pass=1 speedup=1.211759744",
    "PAR_GATE order=degree pass=1 speedup=0.570939903",
    "PAR_GATE order=identity pass=1 speedup=1.060398568",
    "PAR_GATE order=min_source pass=1 speedup=1.155907303",
    "PAR_GATE order=degree pass=1 speedup=0.689580651",
    "PAR_GATE order=identity pass=1 speedup=1.371814763",
    "PAR_GATE order=min_source pass=1 speedup=1.441125858",
    "PAR_GATE order=degree pass=1 speedup=0.882725372",
    "PAR_GATE order=identity pass=1 speedup=2.126980904",
    "PAR_GATE order=min_source pass=1 speedup=2.100662319",
    "PAR_GATE order=degree pass=1 speedup=0.768828395",
    "PAR_GATE order=identity pass=1 speedup=1.669272566",
    "PAR_GATE order=min_source pass=1 speedup=1.240753387",
    "PAR_GATE order=degree pass=1 speedup=0.582036171",
    "PAR_GATE order=identity pass=1 speedup=1.212154793",
    "PAR_GATE order=min_source pass=1 speedup=1.258201058",
    "PAR_GATE order=degree pass=1 speedup=0.635591872",
    "PAR_GATE order=identity pass=1 speedup=1.308937652",
    "PAR_GATE order=min_source pass=1 speedup=1.436512989"
  ],
  "issues": "Isolated source-generation packing prototype; exploratory only",
  "next": "Compare batch sizes and decide whether AMX group packing merits a cost model",
  "date": "2026-10-08T14:23:28.320983+00:00"
}
```

## recompute-window-batch-20261008-222602

```json
{
  "id": "recompute-window-batch-20261008-222602",
  "kind": "recompute_window_batch",
  "status": "FAILED",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-batch-20261008-222602",
  "paper_eligible": false,
  "results": [],
  "gates": [],
  "issues": "Isolated source-generation packing prototype; exploratory only",
  "next": "Compare batch sizes and decide whether AMX group packing merits a cost model",
  "date": "2026-10-08T14:26:05.190303+00:00"
}
```

## recompute-window-batch-correction-20261008-222234

```json
{
  "id": "recompute-window-batch-correction-20261008-222234",
  "kind": "recompute_window_batch_correction",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-batch-20261008-222234",
  "paper_eligible": false,
  "results": "The runtime completed 36 PAR_RESULT lines (four source_batch_blocks values times three orders times three repetitions collapsed to one PAR_RESULT per order). All 36 exact=1 and all 36 pass=1. The enclosing event incorrectly expected 12 results and was marked FAILED by bookkeeping only.",
  "issues": "Bookkeeping correction; shared-node exploratory timing remains non-paper-eligible",
  "next": "Use exclusive-node confirmation before any performance claim",
  "date": "2026-10-08T14:26:11.027867+00:00"
}
```

## recompute-window-batch-20261008-222837

```json
{
  "id": "recompute-window-batch-20261008-222837",
  "kind": "recompute_window_batch",
  "status": "FAILED",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-batch-20261008-222837",
  "paper_eligible": false,
  "results": [
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=42.804002762 candidate_median_ms=70.670843124 exact=1 source_rows=3627178 source_projection_groups=273630 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=34.677028656 candidate_median_ms=30.467987061 exact=1 source_rows=2088038 source_projection_groups=189693 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=36.623001099 candidate_median_ms=30.080080032 exact=1 source_rows=2043040 source_projection_groups=184732 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=46.190977097 candidate_median_ms=67.998170853 exact=1 source_rows=3627178 source_projection_groups=273630 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=37.979841232 candidate_median_ms=30.654907227 exact=1 source_rows=2088038 source_projection_groups=189693 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=39.371013641 candidate_median_ms=30.274868011 exact=1 source_rows=2043040 source_projection_groups=184732 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=45.305967331 candidate_median_ms=68.189144135 exact=1 source_rows=3627178 source_projection_groups=273630 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=37.784099579 candidate_median_ms=30.256986618 exact=1 source_rows=2088038 source_projection_groups=189693 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=1 baseline_median_ms=39.685010910 candidate_median_ms=29.263973236 exact=1 source_rows=2043040 source_projection_groups=184732 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=40.076017380 candidate_median_ms=67.970991135 exact=1 source_rows=3627178 source_projection_groups=240914 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=32.543897629 candidate_median_ms=28.278112411 exact=1 source_rows=2088038 source_projection_groups=144908 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=34.070968628 candidate_median_ms=28.475999832 exact=1 source_rows=2043040 source_projection_groups=142010 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=40.447950363 candidate_median_ms=67.053079605 exact=1 source_rows=3627178 source_projection_groups=240914 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=32.701015472 candidate_median_ms=28.118848801 exact=1 source_rows=2088038 source_projection_groups=144908 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=33.834934235 candidate_median_ms=27.745008469 exact=1 source_rows=2043040 source_projection_groups=142010 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=39.556980133 candidate_median_ms=67.805051804 exact=1 source_rows=3627178 source_projection_groups=240914 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=30.821084976 candidate_median_ms=28.370141983 exact=1 source_rows=2088038 source_projection_groups=144908 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=4 baseline_median_ms=32.814979553 candidate_median_ms=27.868986130 exact=1 source_rows=2043040 source_projection_groups=142010 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=41.207075119 candidate_median_ms=65.860986710 exact=1 source_rows=3627178 source_projection_groups=230301 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=33.062934875 candidate_median_ms=28.073072433 exact=1 source_rows=2088038 source_projection_groups=134173 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=35.139799118 candidate_median_ms=27.420043945 exact=1 source_rows=2043040 source_projection_groups=131318 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=37.128925323 candidate_median_ms=66.501140594 exact=1 source_rows=3627178 source_projection_groups=230301 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=29.616117477 candidate_median_ms=27.943134308 exact=1 source_rows=2088038 source_projection_groups=134173 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=31.115055084 candidate_median_ms=27.523040771 exact=1 source_rows=2043040 source_projection_groups=131318 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=43.680906296 candidate_median_ms=67.608118057 exact=1 source_rows=3627178 source_projection_groups=230301 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=35.776853561 candidate_median_ms=27.539968491 exact=1 source_rows=2088038 source_projection_groups=134173 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=37.285804749 candidate_median_ms=27.183055878 exact=1 source_rows=2043040 source_projection_groups=131318 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=40.532827377 candidate_median_ms=70.918083191 exact=1 source_rows=3627178 source_projection_groups=227645 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=32.777070999 candidate_median_ms=27.518987656 exact=1 source_rows=2088038 source_projection_groups=131424 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=34.216880798 candidate_median_ms=27.471065521 exact=1 source_rows=2043040 source_projection_groups=128585 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=46.011924744 candidate_median_ms=69.565057755 exact=1 source_rows=3627178 source_projection_groups=227645 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=38.398981094 candidate_median_ms=27.868986130 exact=1 source_rows=2088038 source_projection_groups=131424 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=39.861917496 candidate_median_ms=26.993036270 exact=1 source_rows=2043040 source_projection_groups=128585 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773",
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=38.093090057 candidate_median_ms=69.720983505 exact=1 source_rows=3627178 source_projection_groups=227645 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=30.615091324 candidate_median_ms=27.656078339 exact=1 source_rows=2088038 source_projection_groups=131424 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=32.150983810 candidate_median_ms=27.286052704 exact=1 source_rows=2043040 source_projection_groups=128585 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773"
  ],
  "gates": [
    "PAR_GATE order=degree pass=1 speedup=0.605681224",
    "PAR_GATE order=identity pass=1 speedup=1.138146363",
    "PAR_GATE order=min_source pass=1 speedup=1.217516744",
    "PAR_GATE order=degree pass=1 speedup=0.679297348",
    "PAR_GATE order=identity pass=1 speedup=1.238948171",
    "PAR_GATE order=min_source pass=1 speedup=1.300452033",
    "PAR_GATE order=degree pass=1 speedup=0.664416131",
    "PAR_GATE order=identity pass=1 speedup=1.248772723",
    "PAR_GATE order=min_source pass=1 speedup=1.356104675",
    "PAR_GATE order=degree pass=1 speedup=0.589604723",
    "PAR_GATE order=identity pass=1 speedup=1.150851130",
    "PAR_GATE order=min_source pass=1 speedup=1.196480153",
    "PAR_GATE order=degree pass=1 speedup=0.603222859",
    "PAR_GATE order=identity pass=1 speedup=1.162957122",
    "PAR_GATE order=min_source pass=1 speedup=1.219496266",
    "PAR_GATE order=degree pass=1 speedup=0.583392816",
    "PAR_GATE order=identity pass=1 speedup=1.086391636",
    "PAR_GATE order=min_source pass=1 speedup=1.177473030",
    "PAR_GATE order=degree pass=1 speedup=0.625667443",
    "PAR_GATE order=identity pass=1 speedup=1.177745505",
    "PAR_GATE order=min_source pass=1 speedup=1.281536937",
    "PAR_GATE order=degree pass=1 speedup=0.558320128",
    "PAR_GATE order=identity pass=1 speedup=1.059870992",
    "PAR_GATE order=min_source pass=1 speedup=1.130509356",
    "PAR_GATE order=degree pass=1 speedup=0.646089664",
    "PAR_GATE order=identity pass=1 speedup=1.299088399",
    "PAR_GATE order=min_source pass=1 speedup=1.371656112",
    "PAR_GATE order=degree pass=1 speedup=0.571544316",
    "PAR_GATE order=identity pass=1 speedup=1.191071104",
    "PAR_GATE order=min_source pass=1 speedup=1.245560744",
    "PAR_GATE order=degree pass=1 speedup=0.661422936",
    "PAR_GATE order=identity pass=1 speedup=1.377839184",
    "PAR_GATE order=min_source pass=1 speedup=1.476748192",
    "PAR_GATE order=degree pass=1 speedup=0.546364783",
    "PAR_GATE order=identity pass=1 speedup=1.106993224",
    "PAR_GATE order=min_source pass=1 speedup=1.178293693"
  ],
  "issues": "Isolated source-generation packing prototype; exploratory only",
  "next": "Compare batch sizes and decide whether AMX group packing merits a cost model",
  "date": "2026-10-08T14:29:30.154630+00:00"
}
```

## recompute-window-batch-exclusive-correction-20261008-222837

```json
{
  "id": "recompute-window-batch-exclusive-correction-20261008-222837",
  "kind": "recompute_window_batch_exclusive_correction",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-batch-20261008-222837",
  "paper_eligible": false,
  "results": "Exclusive node qhcn819 completed 36 PAR_RESULT lines (four source_batch_blocks values times three orders). Every result exact=1 and every gate pass=1. The enclosing event incorrectly expected 12 results and was marked FAILED by bookkeeping only.",
  "key_observation": "On min_source, source_batch_blocks=16 reduced source_projection_groups from 184732 to 131318 and achieved 1.13x to 1.37x in the sequential repetitions; source_batch_blocks=64 reduced groups to 128585 and achieved 1.18x to 1.48x. Identity was also positive; DegreeSort remained negative.",
  "issues": "Exploratory exclusive-node timing; repeated invocations show frequency/thermal drift and are not final paper tables",
  "next": "Run an interleaved single-batch protocol and test an adaptive source-overlap/packing policy",
  "date": "2026-10-08T14:32:38.665439+00:00"
}
```

## recompute-window-interleave-build-20261008-230314

```json
{
  "id": "recompute-window-interleave-build-20261008-230314",
  "kind": "recompute_window_interleave_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-interleave-build-20261008-230314",
  "paper_eligible": false,
  "issues": "Interleaved timing shadow prototype; no paper performance claim",
  "next": "Run exclusive interleaved timing gate if build passes",
  "date": "2026-10-08T15:03:19.496532+00:00"
}
```

## recompute-window-interleave-20261008-230519

```json
{
  "id": "recompute-window-interleave-20261008-230519",
  "kind": "recompute_window_interleave",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-interleave-20261008-230519",
  "paper_eligible": false,
  "results": [
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=119.688987732 candidate_median_ms=141.752958298 exact=1 source_rows=3627178 source_projection_groups=230301 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=70.607900620 candidate_median_ms=54.904937744 exact=1 source_rows=2088038 source_projection_groups=134173 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=78.174829483 candidate_median_ms=58.802843094 exact=1 source_rows=2043040 source_projection_groups=131318 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773"
  ],
  "gates": [
    "PAR_GATE order=degree pass=1 speedup=0.843837793",
    "PAR_GATE order=identity pass=1 speedup=1.308577173",
    "PAR_GATE order=min_source pass=1 speedup=1.343427023"
  ],
  "issues": "Interleaved exclusive-node exploratory timing; no paper performance claim",
  "next": "Compare interleaved ratios and finalize source-overlap/packing policy",
  "date": "2026-10-08T15:05:33.719704+00:00"
}
```

## recompute-window-interleave-20261008-230957

```json
{
  "id": "recompute-window-interleave-20261008-230957",
  "kind": "recompute_window_interleave",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-interleave-20261008-230957",
  "paper_eligible": false,
  "results": [
    "PAR_RESULT order=degree graph=amazon0601 threads=32 source_batch_blocks=16 baseline_median_ms=61.820030212 candidate_median_ms=345.166921616 exact=1 source_rows=1840149 source_projection_groups=115754 second_projection_groups=25213 peak_cache_rows=75480 checksum=3449434827757982282",
    "PAR_RESULT order=identity graph=amazon0601 threads=32 source_batch_blocks=16 baseline_median_ms=53.194999695 candidate_median_ms=277.334928513 exact=1 source_rows=1717770 source_projection_groups=108124 second_projection_groups=25213 peak_cache_rows=60192 checksum=3449434827757982282",
    "PAR_RESULT order=min_source graph=amazon0601 threads=32 source_batch_blocks=16 baseline_median_ms=51.776885986 candidate_median_ms=303.220987320 exact=1 source_rows=1357690 source_projection_groups=85600 second_projection_groups=25213 peak_cache_rows=49722 checksum=3449434827757982282"
  ],
  "gates": [
    "PAR_GATE order=degree pass=1 speedup=0.170802305",
    "PAR_GATE order=identity pass=1 speedup=0.203948512",
    "PAR_GATE order=min_source pass=1 speedup=0.169434974"
  ],
  "issues": "Interleaved exclusive-node exploratory timing; no paper performance claim",
  "next": "Compare interleaved ratios and finalize source-overlap/packing policy",
  "date": "2026-10-08T15:10:06.967944+00:00"
}
```

## recompute-window-interleave-amazon-correction-20261008-230957

```json
{
  "id": "recompute-window-interleave-amazon-correction-20261008-230957",
  "kind": "recompute_window_interleave_amazon_correction",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-interleave-20261008-230957",
  "paper_eligible": false,
  "results": "Shared node qhcn028, not exclusive. All three orders were exact. With source_batch_blocks=16, candidate/baseline ratios were 4.16x (degree), 5.21x (identity), and 5.86x (min_source), so packing does not rescue high source-duplication Amazon.",
  "issues": "Timing is exploratory and shared-node; the run record's generic issue text said exclusive and is superseded by this correction",
  "next": "Use source-row duplication as a fallback condition; continue exclusive RoadNet confirmation",
  "date": "2026-10-08T15:11:34.782137+00:00"
}
```

## recompute-window-interleave-20261008-231443

```json
{
  "id": "recompute-window-interleave-20261008-231443",
  "kind": "recompute_window_interleave",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-interleave-20261008-231443",
  "paper_eligible": false,
  "results": [
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=69.319963455 candidate_median_ms=119.016885757 exact=1 source_rows=3627178 source_projection_groups=227645 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=57.044982910 candidate_median_ms=40.901899338 exact=1 source_rows=2088038 source_projection_groups=131424 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=64 baseline_median_ms=59.428930283 candidate_median_ms=43.226003647 exact=1 source_rows=2043040 source_projection_groups=128585 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773"
  ],
  "gates": [
    "PAR_GATE order=degree pass=1 speedup=0.589719818",
    "PAR_GATE order=identity pass=1 speedup=1.396668355",
    "PAR_GATE order=min_source pass=1 speedup=1.359052157"
  ],
  "issues": "Interleaved exclusive-node exploratory timing; no paper performance claim",
  "next": "Compare interleaved ratios and finalize source-overlap/packing policy",
  "date": "2026-10-08T15:14:51.494157+00:00"
}
```

## recompute-window-interleave-batch64-correction-20261008-231443

```json
{
  "id": "recompute-window-interleave-batch64-correction-20261008-231443",
  "kind": "recompute_window_interleave_batch64_correction",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-interleave-20261008-231443",
  "paper_eligible": false,
  "results": "Shared node qhcn019. All orders exact. With source_batch_blocks=64, identity was 1.397x and min_source 1.359x; DegreeSort was 0.590x. This is consistent with batch16 and does not establish a final paper table.",
  "issues": "Shared-node exploratory timing; generic run record text said exclusive and is superseded by this correction",
  "next": "Use 16 or 64 based on a packing/working-set cost model and validate an adaptive dispatcher",
  "date": "2026-10-08T15:16:13.155730+00:00"
}
```

## source-partition-weighted-build-20261008-232004

```json
{
  "id": "source-partition-weighted-build-20261008-232004",
  "kind": "source_partition_weighted_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-partition-weighted-build-20261008-232004",
  "paper_eligible": false,
  "issues": "Weighted source duplication structural analyzer; no runtime claim",
  "next": "Run weighted graph screening if build passes",
  "date": "2026-10-08T15:20:06.817172+00:00"
}
```

## source-partition-weighted-20261008-232054

```json
{
  "id": "source-partition-weighted-20261008-232054",
  "kind": "source_partition_weighted",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-partition-weighted-20261008-232054",
  "paper_eligible": false,
  "partition_lines": [
    "PARTITION order=identity parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=2088066 duplication=1.062517619 weighted_total=5936271 weighted_duplication=1.072843197 min_unique=62228 max_unique=68053 min_edges=162112 max_edges=191370 edge_check=5533214",
    "PARTITION order=degree parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=3524721 duplication=1.793563118 weighted_total=10647896 weighted_duplication=1.924360055 min_unique=49438 max_unique=185279 min_edges=55527 max_edges=262667 edge_check=5533214",
    "PARTITION order=degree_desc parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=3528312 duplication=1.795390407 weighted_total=10659662 weighted_duplication=1.926486487 min_unique=49447 max_unique=183261 min_edges=55528 max_edges=262663 edge_check=5533214",
    "PARTITION order=min_source parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=2043374 duplication=1.039775983 weighted_total=5783489 weighted_duplication=1.045231397 min_unique=56224 max_unique=65824 min_edges=157783 max_edges=192723 edge_check=5533214",
    "PARTITION order=avg_source parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=2085989 duplication=1.061460732 weighted_total=5931299 weighted_duplication=1.071944624 min_unique=55938 max_unique=69003 min_edges=156848 max_edges=191156 edge_check=5533214",
    "PARTITION order=max_source parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=2083393 duplication=1.060139751 weighted_total=5925655 weighted_duplication=1.070924602 min_unique=55934 max_unique=69715 min_edges=156693 max_edges=190653 edge_check=5533214",
    "PARTITION order=identity parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1717716 duplication=4.259025271 weighted_total=16041581 weighted_duplication=4.736209953 min_unique=31432 max_unique=60187 min_edges=86912 max_edges=122552 edge_check=3387388",
    "PARTITION order=degree parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1795368 duplication=4.451561074 weighted_total=16697153 weighted_duplication=4.929764854 min_unique=11054 max_unique=71968 min_edges=11651 max_edges=126070 edge_check=3387388",
    "PARTITION order=degree_desc parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1793904 duplication=4.447931130 weighted_total=16683222 weighted_duplication=4.925651785 min_unique=11045 max_unique=72509 min_edges=11652 max_edges=126070 edge_check=3387388",
    "PARTITION order=min_source parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1360221 duplication=3.372627147 weighted_total=12438547 weighted_duplication=3.672429176 min_unique=19397 max_unique=49234 min_edges=78821 max_edges=115637 edge_check=3387388",
    "PARTITION order=avg_source parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1757633 duplication=4.357998274 weighted_total=16345307 weighted_duplication=4.825883789 min_unique=19477 max_unique=67433 min_edges=89446 max_edges=114751 edge_check=3387388",
    "PARTITION order=max_source parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1643112 duplication=4.074046892 weighted_total=15442573 weighted_duplication=4.559355337 min_unique=23470 max_unique=58850 min_edges=99138 max_edges=110237 edge_check=3387388",
    "PARTITION order=identity parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=6533128 duplication=28.043388492 weighted_total=3647185899 weighted_duplication=31.820944158 min_unique=202714 max_unique=205390 min_edges=3328028 max_edges=3792207 edge_check=114615892",
    "PARTITION order=degree parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=5035589 duplication=21.615216878 weighted_total=3018337895 weighted_duplication=26.334375123 min_unique=13624 max_unique=207222 min_edges=17916 max_edges=27322262 edge_check=114615892",
    "PARTITION order=degree_desc parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=5035972 duplication=21.616860902 weighted_total=3018066332 weighted_duplication=26.332005792 min_unique=13655 max_unique=207219 min_edges=17921 max_edges=27320001 edge_check=114615892",
    "PARTITION order=min_source parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=4722199 duplication=20.269993347 weighted_total=2987224736 weighted_duplication=26.062919233 min_unique=22492 max_unique=212060 min_edges=34245 max_edges=13245397 edge_check=114615892",
    "PARTITION order=avg_source parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=5363035 duplication=23.020775653 weighted_total=3208698961 weighted_duplication=27.995236132 min_unique=47126 max_unique=205478 min_edges=105781 max_edges=7929093 edge_check=114615892",
    "PARTITION order=max_source parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=4768128 duplication=20.467143133 weighted_total=3016168673 weighted_duplication=26.315449109 min_unique=22359 max_unique=200015 min_edges=34065 max_edges=11918279 edge_check=114615892"
  ],
  "issues": "Structural weighted source-overlap audit; no runtime claim",
  "next": "Use weighted duplication with AMX group utilization in an adaptive plan",
  "date": "2026-10-08T15:21:19.802281+00:00"
}
```

## source-partition-weighted-build-20261008-232709

```json
{
  "id": "source-partition-weighted-build-20261008-232709",
  "kind": "source_partition_hot_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-partition-weighted-build-20261008-232709",
  "paper_eligible": false,
  "issues": "Weighted source duplication structural analyzer; no runtime claim",
  "next": "Run weighted graph screening if build passes",
  "date": "2026-10-08T15:27:11.778477+00:00"
}
```

## source-partition-weighted-20261008-232839

```json
{
  "id": "source-partition-weighted-20261008-232839",
  "kind": "source_partition_hot",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-partition-weighted-20261008-232839",
  "paper_eligible": false,
  "partition_lines": [
    "PARTITION order=identity parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=2088066 duplication=1.062517619 weighted_total=5936271 weighted_duplication=1.072843197 min_unique=62228 max_unique=68053 min_edges=162112 max_edges=191370 edge_check=5533214",
    "PARTITION order=degree parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=3524721 duplication=1.793563118 weighted_total=10647896 weighted_duplication=1.924360055 min_unique=49438 max_unique=185279 min_edges=55527 max_edges=262667 edge_check=5533214",
    "PARTITION order=degree_desc parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=3528312 duplication=1.795390407 weighted_total=10659662 weighted_duplication=1.926486487 min_unique=49447 max_unique=183261 min_edges=55528 max_edges=262663 edge_check=5533214",
    "PARTITION order=min_source parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=2043374 duplication=1.039775983 weighted_total=5783489 weighted_duplication=1.045231397 min_unique=56224 max_unique=65824 min_edges=157783 max_edges=192723 edge_check=5533214",
    "PARTITION order=avg_source parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=2085989 duplication=1.061460732 weighted_total=5931299 weighted_duplication=1.071944624 min_unique=55938 max_unique=69003 min_edges=156848 max_edges=191156 edge_check=5533214",
    "PARTITION order=max_source parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=2083393 duplication=1.060139751 weighted_total=5925655 weighted_duplication=1.070924602 min_unique=55934 max_unique=69715 min_edges=156693 max_edges=190653 edge_check=5533214",
    "PARTITION order=hot_source parts=32 n=1971281 e=5533214 global_sources=1965206 total_unique_sources=2642494 duplication=1.344639697 weighted_total=7495687 weighted_duplication=1.354671444 min_unique=74661 max_unique=90126 min_edges=130027 max_edges=204695 edge_check=5533214",
    "PARTITION order=identity parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1717716 duplication=4.259025271 weighted_total=16041581 weighted_duplication=4.736209953 min_unique=31432 max_unique=60187 min_edges=86912 max_edges=122552 edge_check=3387388",
    "PARTITION order=degree parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1795368 duplication=4.451561074 weighted_total=16697153 weighted_duplication=4.929764854 min_unique=11054 max_unique=71968 min_edges=11651 max_edges=126070 edge_check=3387388",
    "PARTITION order=degree_desc parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1793904 duplication=4.447931130 weighted_total=16683222 weighted_duplication=4.925651785 min_unique=11045 max_unique=72509 min_edges=11652 max_edges=126070 edge_check=3387388",
    "PARTITION order=min_source parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1360221 duplication=3.372627147 weighted_total=12438547 weighted_duplication=3.672429176 min_unique=19397 max_unique=49234 min_edges=78821 max_edges=115637 edge_check=3387388",
    "PARTITION order=avg_source parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1757633 duplication=4.357998274 weighted_total=16345307 weighted_duplication=4.825883789 min_unique=19477 max_unique=67433 min_edges=89446 max_edges=114751 edge_check=3387388",
    "PARTITION order=max_source parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1643112 duplication=4.074046892 weighted_total=15442573 weighted_duplication=4.559355337 min_unique=23470 max_unique=58850 min_edges=99138 max_edges=110237 edge_check=3387388",
    "PARTITION order=hot_source parts=32 n=403394 e=3387388 global_sources=403312 total_unique_sources=1360235 duplication=3.372661860 weighted_total=12438700 weighted_duplication=3.672474349 min_unique=19397 max_unique=49236 min_edges=78821 max_edges=115633 edge_check=3387388",
    "PARTITION order=identity parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=6533128 duplication=28.043388492 weighted_total=3647185899 weighted_duplication=31.820944158 min_unique=202714 max_unique=205390 min_edges=3328028 max_edges=3792207 edge_check=114615892",
    "PARTITION order=degree parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=5035589 duplication=21.615216878 weighted_total=3018337895 weighted_duplication=26.334375123 min_unique=13624 max_unique=207222 min_edges=17916 max_edges=27322262 edge_check=114615892",
    "PARTITION order=degree_desc parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=5035972 duplication=21.616860902 weighted_total=3018066332 weighted_duplication=26.332005792 min_unique=13655 max_unique=207219 min_edges=17921 max_edges=27320001 edge_check=114615892",
    "PARTITION order=min_source parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=4722199 duplication=20.269993347 weighted_total=2987224736 weighted_duplication=26.062919233 min_unique=22492 max_unique=212060 min_edges=34245 max_edges=13245397 edge_check=114615892",
    "PARTITION order=avg_source parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=5363035 duplication=23.020775653 weighted_total=3208698961 weighted_duplication=27.995236132 min_unique=47126 max_unique=205478 min_edges=105781 max_edges=7929093 edge_check=114615892",
    "PARTITION order=max_source parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=4768128 duplication=20.467143133 weighted_total=3016168673 weighted_duplication=26.315449109 min_unique=22359 max_unique=200015 min_edges=34065 max_edges=11918279 edge_check=114615892",
    "PARTITION order=hot_source parts=32 n=232965 e=114615892 global_sources=232965 total_unique_sources=5069103 duplication=21.759075398 weighted_total=3101617713 weighted_duplication=27.060974346 min_unique=38641 max_unique=199461 min_edges=180188 max_edges=19479649 edge_check=114615892"
  ],
  "issues": "Structural weighted source-overlap audit; no runtime claim",
  "next": "Use weighted duplication with AMX group utilization in an adaptive plan",
  "date": "2026-10-08T15:29:06.274683+00:00"
}
```

## source-partition-hot-analysis-20261008-232839

```json
{
  "id": "source-partition-hot-analysis-20261008-232839",
  "kind": "source_partition_hot_analysis",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-partition-weighted-20261008-232839",
  "paper_eligible": false,
  "results": "The hot-source anchor order is not a better general partitioner: RoadNet weighted duplication 1.355 versus min_source 1.045; Amazon 3.672 versus 3.672 (no gain); Reddit 27.061 versus 26.063. It also increases RoadNet edge imbalance. This negative control rejects a naive heavy-source clustering rule.",
  "issues": "Structural only; no runtime claim",
  "next": "Keep min_source/sliding-window locality and use weighted duplication plus packing utilization for fallback",
  "date": "2026-10-08T15:31:29.478534+00:00"
}
```

## recompute-window-interleave-build-20261008-233639

```json
{
  "id": "recompute-window-interleave-build-20261008-233639",
  "kind": "recompute_window_adaptive_build",
  "status": "FAILED",
  "exit_status": 1,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-interleave-build-20261008-233639",
  "paper_eligible": false,
  "issues": "Interleaved timing shadow prototype; no paper performance claim",
  "next": "Run exclusive interleaved timing gate if build passes",
  "date": "2026-10-08T15:36:41.404928+00:00"
}
```

## recompute-window-interleave-20261008-233649

```json
{
  "id": "recompute-window-interleave-20261008-233649",
  "kind": "recompute_window_interleave",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-interleave-20261008-233649",
  "paper_eligible": false,
  "results": [
    "PAR_RESULT order=degree graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=43.956995010 candidate_median_ms=64.821958542 exact=1 source_rows=3627178 source_projection_groups=230301 second_projection_groups=123206 peak_cache_rows=173160 checksum=12151273104846194773",
    "PAR_RESULT order=identity graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=35.864114761 candidate_median_ms=28.103113174 exact=1 source_rows=2088038 source_projection_groups=134173 second_projection_groups=123206 peak_cache_rows=68068 checksum=12151273104846194773",
    "PAR_RESULT order=min_source graph=roadNet-CA threads=32 source_batch_blocks=16 baseline_median_ms=37.718057632 candidate_median_ms=28.516054153 exact=1 source_rows=2043040 source_projection_groups=131318 second_projection_groups=123206 peak_cache_rows=66277 checksum=12151273104846194773"
  ],
  "gates": [
    "PAR_GATE order=degree pass=1 speedup=0.672549571",
    "PAR_GATE order=identity pass=1 speedup=1.277547178",
    "PAR_GATE order=min_source pass=1 speedup=1.326927775"
  ],
  "issues": "Interleaved exclusive-node exploratory timing; no paper performance claim",
  "next": "Compare interleaved ratios and finalize source-overlap/packing policy",
  "date": "2026-10-08T15:37:01.516794+00:00"
}
```

## recompute-window-adaptive-build-20261008-233758

```json
{
  "id": "recompute-window-adaptive-build-20261008-233758",
  "kind": "recompute_window_adaptive_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-adaptive-build-20261008-233758",
  "paper_eligible": false,
  "issues": "Adaptive dispatcher shadow prototype; no paper performance claim",
  "next": "Run exclusive interleaved timing gate if build passes",
  "date": "2026-10-08T15:38:02.868909+00:00"
}
```

## recompute-window-adaptive-20261008-233857

```json
{
  "id": "recompute-window-adaptive-20261008-233857",
  "kind": "recompute_window_adaptive",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-adaptive-20261008-233857",
  "paper_eligible": false,
  "plans": [
    "PLAN graph=roadNet-CA threads=32 weighted_duplication=1.045113744 threshold=1.25 mode=fused_min_source batch=64",
    "PLAN graph=amazon0601 threads=32 weighted_duplication=3.665222816 threshold=1.25 mode=materialized_full batch=64",
    "PLAN graph=reddit threads=32 weighted_duplication=26.062167191 threshold=1.25 mode=materialized_full batch=64"
  ],
  "results": [
    "ADAPT_RESULT graph=roadNet-CA threads=32 mode=fused_min_source rho=1.045113744 baseline_median_ms=61.645984650 candidate_median_ms=55.500030518 exact=1 source_rows=2043040 source_projection_groups=128585",
    "ADAPT_RESULT graph=amazon0601 threads=32 mode=materialized_full rho=3.665222816 baseline_median_ms=55.896043777 candidate_median_ms=0.000000000 exact=1 source_rows=0 source_projection_groups=0",
    "ADAPT_RESULT graph=reddit threads=32 mode=materialized_full rho=26.062167191 baseline_median_ms=223.967790604 candidate_median_ms=0.000000000 exact=1 source_rows=0 source_projection_groups=0"
  ],
  "issues": "Adaptive weighted-duplication dispatcher exploratory gate; no paper performance claim",
  "next": "Repeat on held-out graphs and calibrate threshold with weighted work model",
  "date": "2026-10-08T15:39:06.267599+00:00"
}
```

## recompute-window-adaptive-build-20261008-234221

```json
{
  "id": "recompute-window-adaptive-build-20261008-234221",
  "kind": "recompute_window_adaptive_build",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-adaptive-build-20261008-234221",
  "paper_eligible": false,
  "issues": "Adaptive dispatcher shadow prototype; no paper performance claim",
  "next": "Run exclusive interleaved timing gate if build passes",
  "date": "2026-10-08T15:42:25.904151+00:00"
}
```

## recompute-window-adaptive-20261008-234255

```json
{
  "id": "recompute-window-adaptive-20261008-234255",
  "kind": "recompute_window_adaptive",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-adaptive-20261008-234255",
  "paper_eligible": false,
  "plans": [
    "PLAN graph=roadNet-CA threads=32 weighted_duplication=1.045113744 threshold=1.25 mode=fused_min_source batch=64",
    "PLAN graph=amazon0601 threads=32 weighted_duplication=3.665222816 threshold=1.25 mode=materialized_full batch=64",
    "PLAN graph=reddit threads=32 weighted_duplication=26.062167191 threshold=1.25 mode=materialized_full batch=64"
  ],
  "results": [
    "ADAPT_RESULT graph=roadNet-CA threads=32 mode=fused_min_source rho=1.045113744 baseline_median_ms=58.727025986 candidate_median_ms=50.426959991 exact=1 source_rows=2043040 source_projection_groups=128585",
    "ADAPT_RESULT graph=amazon0601 threads=32 mode=materialized_full rho=3.665222816 baseline_median_ms=50.272941589 candidate_median_ms=50.272941589 exact=1 source_rows=0 source_projection_groups=0",
    "ADAPT_RESULT graph=reddit threads=32 mode=materialized_full rho=26.062167191 baseline_median_ms=164.298057556 candidate_median_ms=164.298057556 exact=1 source_rows=0 source_projection_groups=0"
  ],
  "issues": "Adaptive weighted-duplication dispatcher exploratory gate; no paper performance claim",
  "next": "Repeat on held-out graphs and calibrate threshold with weighted work model",
  "date": "2026-10-08T15:43:03.639373+00:00"
}
```

## recompute-window-adaptive-analysis-20261008-234255

```json
{
  "id": "recompute-window-adaptive-analysis-20261008-234255",
  "kind": "recompute_window_adaptive_analysis",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/recompute-window-adaptive-20261008-234255",
  "paper_eligible": false,
  "results": "Shared node qhcn019, exact on all graphs. RoadNet rho_w=1.045 selected fused min_source+batch64 and measured 1.165x median improvement after warmup. Amazon rho_w=3.665 and Reddit rho_w=26.062 selected materialized fallback; candidate and baseline paths are identical in fallback and had equal medians by construction.",
  "issues": "Exploratory shared-node gate; threshold 1.25 is a provisional safety margin, not a trained or paper-ready threshold",
  "next": "Calibrate rho_w and packing utilization on held-out graphs, then repeat adaptive plan on exclusive nodes",
  "date": "2026-10-08T15:45:01.621207+00:00"
}
```

## source-partition-weighted-heldout-20261008-234650

```json
{
  "id": "source-partition-weighted-heldout-20261008-234650",
  "kind": "source_partition_weighted_heldout",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-partition-weighted-heldout-20261008-234650",
  "paper_eligible": false,
  "partition_lines": [
    "PARTITION order=identity parts=32 n=5154859 e=99199551 global_sources=5154859 total_unique_sources=21256421 duplication=4.123569820 weighted_total=424271648 weighted_duplication=4.276951294 min_unique=288900 max_unique=884107 min_edges=1623339 max_edges=4324977 edge_check=99199551",
    "PARTITION order=degree parts=32 n=5154859 e=99199551 global_sources=5154859 total_unique_sources=36165438 duplication=7.015795776 weighted_total=708727320 weighted_duplication=7.144460966 min_unique=312164 max_unique=1489742 min_edges=1328806 max_edges=5472460 edge_check=99199551",
    "PARTITION order=degree_desc parts=32 n=5154859 e=99199551 global_sources=5154859 total_unique_sources=36429293 duplication=7.066981464 weighted_total=714007731 weighted_duplication=7.197691157 min_unique=375147 max_unique=1512635 min_edges=1328816 max_edges=5472429 edge_check=99199551",
    "PARTITION order=min_source parts=32 n=5154859 e=99199551 global_sources=5154859 total_unique_sources=17736743 duplication=3.440781406 weighted_total=360799856 weighted_duplication=3.637111785 min_unique=246342 max_unique=859962 min_edges=1618120 max_edges=4159223 edge_check=99199551",
    "PARTITION order=avg_source parts=32 n=5154859 e=99199551 global_sources=5154859 total_unique_sources=23921045 duplication=4.640484832 weighted_total=479259277 weighted_duplication=4.831264579 min_unique=281604 max_unique=897054 min_edges=1623158 max_edges=4120734 edge_check=99199551",
    "PARTITION order=max_source parts=32 n=5154859 e=99199551 global_sources=5154859 total_unique_sources=18275300 duplication=3.545257009 weighted_total=364027308 weighted_duplication=3.669646731 min_unique=270167 max_unique=740026 min_edges=1606316 max_edges=4214718 edge_check=99199551",
    "PARTITION order=identity parts=32 n=393215 e=903194710 global_sources=393215 total_unique_sources=5578689 duplication=14.187375863 weighted_total=16955086963 weighted_duplication=18.772349722 min_unique=24579 max_unique=393215 min_edges=4121476 max_edges=118256384 edge_check=903194710",
    "PARTITION order=degree parts=32 n=393215 e=903194710 global_sources=393215 total_unique_sources=6217812 duplication=15.812753837 weighted_total=23333174684 weighted_duplication=25.834047106 min_unique=15149 max_unique=393215 min_edges=997119 max_edges=250261578 edge_check=903194710",
    "PARTITION order=degree_desc parts=32 n=393215 e=903194710 global_sources=393215 total_unique_sources=6219323 duplication=15.816596518 weighted_total=23336806093 weighted_duplication=25.838067733 min_unique=15119 max_unique=393215 min_edges=997233 max_edges=250250158 edge_check=903194710",
    "PARTITION order=min_source parts=32 n=393215 e=903194710 global_sources=393215 total_unique_sources=4867923 duplication=12.379799855 weighted_total=14925314328 weighted_duplication=16.525024076 min_unique=40884 max_unique=393191 min_edges=7160890 max_edges=88882319 edge_check=903194710",
    "PARTITION order=avg_source parts=32 n=393215 e=903194710 global_sources=393215 total_unique_sources=4919639 duplication=12.511320779 weighted_total=16487672759 weighted_duplication=18.254837607 min_unique=20662 max_unique=393211 min_edges=5684626 max_edges=119593553 edge_check=903194710",
    "PARTITION order=max_source parts=32 n=393215 e=903194710 global_sources=393215 total_unique_sources=5578640 duplication=14.187251249 weighted_total=16950319358 weighted_duplication=18.767071120 min_unique=24579 max_unique=393200 min_edges=3928970 max_edges=118256384 edge_check=903194710",
    "PARTITION order=identity parts=32 n=36692 e=367662 global_sources=36692 total_unique_sources=107840 duplication=2.939060286 weighted_total=3120394 weighted_duplication=8.487126763 min_unique=604 max_unique=20061 min_edges=1803 max_edges=108867 edge_check=367662",
    "PARTITION order=degree parts=32 n=36692 e=367662 global_sources=36692 total_unique_sources=110169 duplication=3.002534612 weighted_total=3363482 weighted_duplication=9.148299253 min_unique=59 max_unique=27774 min_edges=1146 max_edges=164718 edge_check=367662",
    "PARTITION order=degree_desc parts=32 n=36692 e=367662 global_sources=36692 total_unique_sources=109963 duplication=2.996920310 weighted_total=3299704 weighted_duplication=8.974830143 min_unique=60 max_unique=27769 min_edges=1146 max_edges=164661 edge_check=367662",
    "PARTITION order=min_source parts=32 n=36692 e=367662 global_sources=36692 total_unique_sources=112469 duplication=3.065218576 weighted_total=3218864 weighted_duplication=8.754954279 min_unique=192 max_unique=24824 min_edges=1534 max_edges=100262 edge_check=367662",
    "PARTITION order=avg_source parts=32 n=36692 e=367662 global_sources=36692 total_unique_sources=137361 duplication=3.743622588 weighted_total=4906508 weighted_duplication=13.345159413 min_unique=180 max_unique=8947 min_edges=1416 max_edges=30537 edge_check=367662",
    "PARTITION order=max_source parts=32 n=36692 e=367662 global_sources=36692 total_unique_sources=110316 duplication=3.006540935 weighted_total=4661063 weighted_duplication=12.677576143 min_unique=32 max_unique=6713 min_edges=1185 max_edges=26352 edge_check=367662",
    "PARTITION order=identity parts=32 n=916428 e=5105039 global_sources=714545 total_unique_sources=3215181 duplication=4.499620038 weighted_total=31994265 weighted_duplication=7.393150405 min_unique=98873 max_unique=102281 min_edges=156294 max_edges=162422 edge_check=5105039",
    "PARTITION order=degree parts=32 n=916428 e=5105039 global_sources=714545 total_unique_sources=1878156 duplication=2.628464267 weighted_total=14939711 weighted_duplication=3.452229030 min_unique=0 max_unique=209681 min_edges=0 max_edges=725597 edge_check=5105039",
    "PARTITION order=degree_desc parts=32 n=916428 e=5105039 global_sources=714545 total_unique_sources=1881636 duplication=2.633334500 weighted_total=14966916 weighted_duplication=3.458515490 min_unique=0 max_unique=210337 min_edges=0 max_edges=725578 edge_check=5105039",
    "PARTITION order=min_source parts=32 n=916428 e=5105039 global_sources=714545 total_unique_sources=1432606 duplication=2.004920614 weighted_total=11148968 weighted_duplication=2.576274132 min_unique=0 max_unique=93197 min_edges=0 max_edges=345332 edge_check=5105039",
    "PARTITION order=avg_source parts=32 n=916428 e=5105039 global_sources=714545 total_unique_sources=2123340 duplication=2.971597310 weighted_total=18636588 weighted_duplication=4.306493621 min_unique=0 max_unique=131567 min_edges=0 max_edges=291663 edge_check=5105039",
    "PARTITION order=max_source parts=32 n=916428 e=5105039 global_sources=714545 total_unique_sources=1440754 duplication=2.016323675 weighted_total=11388349 weighted_duplication=2.631589662 min_unique=0 max_unique=75489 min_edges=0 max_edges=290132 edge_check=5105039",
    "PARTITION order=identity parts=32 n=1134890 e=5975248 global_sources=1134890 total_unique_sources=2600143 duplication=2.291096935 weighted_total=58038521 weighted_duplication=9.713156843 min_unique=22352 max_unique=409818 min_edges=51445 max_edges=1473367 edge_check=5975248",
    "PARTITION order=degree parts=32 n=1134890 e=5975248 global_sources=1134890 total_unique_sources=2249681 duplication=1.982289914 weighted_total=45401974 weighted_duplication=7.598341358 min_unique=2300 max_unique=721207 min_edges=35465 max_edges=3008086 edge_check=5975248",
    "PARTITION order=degree_desc parts=32 n=1134890 e=5975248 global_sources=1134890 total_unique_sources=2249447 duplication=1.982083726 weighted_total=44945523 weighted_duplication=7.521951055 min_unique=2889 max_unique=720164 min_edges=35465 max_edges=3008064 edge_check=5975248",
    "PARTITION order=min_source parts=32 n=1134890 e=5975248 global_sources=1134890 total_unique_sources=2464683 duplication=2.171737349 weighted_total=42878665 weighted_duplication=7.176047756 min_unique=14834 max_unique=460420 min_edges=44604 max_edges=1606024 edge_check=5975248",
    "PARTITION order=avg_source parts=32 n=1134890 e=5975248 global_sources=1134890 total_unique_sources=2963958 duplication=2.611669853 weighted_total=74224108 weighted_duplication=12.421929266 min_unique=1465 max_unique=155027 min_edges=45347 max_edges=415418 edge_check=5975248",
    "PARTITION order=max_source parts=32 n=1134890 e=5975248 global_sources=1134890 total_unique_sources=2704082 duplication=2.382682022 weighted_total=72812693 weighted_duplication=12.185718986 min_unique=1133 max_unique=430590 min_edges=41915 max_edges=1045595 edge_check=5975248"
  ],
  "issues": "Held-out weighted source-overlap screening; no runtime claim",
  "next": "Select a held-out low-rho graph for adaptive runtime validation",
  "date": "2026-10-08T15:47:59.379571+00:00"
}
```

## source-partition-weighted-roadusa-20261008-235043

```json
{
  "id": "source-partition-weighted-roadusa-20261008-235043",
  "kind": "source_partition_weighted_roadusa",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-partition-weighted-roadusa-20261008-235043",
  "paper_eligible": false,
  "partition_lines": [
    "PARTITION order=identity parts=32 n=23947347 e=57708624 global_sources=23947347 total_unique_sources=41344276 duplication=1.726465817 weighted_total=108808097 weighted_duplication=1.885473773 min_unique=950084 max_unique=1541469 min_edges=1721848 max_edges=1857241 edge_check=57708624",
    "PARTITION order=degree parts=32 n=23947347 e=57708624 global_sources=23947347 total_unique_sources=45361363 duplication=1.894212457 weighted_total=120555967 weighted_duplication=2.089045946 min_unique=672054 max_unique=2207839 min_edges=748354 max_edges=3027301 edge_check=57708624",
    "PARTITION order=degree_desc parts=32 n=23947347 e=57708624 global_sources=23947347 total_unique_sources=44616650 duplication=1.863114524 weighted_total=118436564 weighted_duplication=2.052320014 min_unique=668073 max_unique=2240535 min_edges=748354 max_edges=3027297 edge_check=57708624",
    "PARTITION order=min_source parts=32 n=23947347 e=57708624 global_sources=23947347 total_unique_sources=33303789 duplication=1.390708917 weighted_total=84865640 weighted_duplication=1.470588521 min_unique=844516 max_unique=1158109 min_edges=1520076 max_edges=2065606 edge_check=57708624",
    "PARTITION order=avg_source parts=32 n=23947347 e=57708624 global_sources=23947347 total_unique_sources=38602187 duplication=1.611960899 weighted_total=100544285 weighted_duplication=1.742274863 min_unique=863815 max_unique=1394250 min_edges=1534458 max_edges=1982592 edge_check=57708624",
    "PARTITION order=max_source parts=32 n=23947347 e=57708624 global_sources=23947347 total_unique_sources=33333528 duplication=1.391950766 weighted_total=84964013 weighted_duplication=1.472293171 min_unique=816447 max_unique=1261004 min_edges=1473866 max_edges=2040997 edge_check=57708624"
  ],
  "issues": "Structural weighted source-overlap audit; no runtime claim",
  "next": "If weighted duplication is low, run adaptive held-out validation",
  "date": "2026-10-08T15:54:00.829061+00:00"
}
```

## handoff-current-summary-20261008

```json
{
  "id": "handoff-current-summary-20261008",
  "kind": "handoff_current_summary",
  "status": "PASS",
  "exit_status": 0,
  "evidence": "/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/docs/handoff/CURRENT_HANDOFF.md",
  "paper_eligible": false,
  "summary": "Current handoff summary written on the server. It records the source-overlap-aware fusion hypothesis, tile packing results, weighted duplication values, adaptive fallback evidence, negative controls, superseded bookkeeping records, constraints, and the ordered next steps.",
  "authoritative_results": [
    "Exclusive Job 10902135 on qhcn819: exact source-aware/batched gates; min_source positive and DegreeSort negative on RoadNet-CA.",
    "Shared Job 10902324 adaptive v2: RoadNet rho_w=1.045 fused; Amazon rho_w=3.665 and Reddit rho_w=26.062 fallback; all exact.",
    "No NUMA interleave used; canonical kernel unchanged; all exploratory events remain paper-ineligible."
  ],
  "next": "Repeat adaptive v2 on exclusive nodes, calibrate weighted cost model with held-out graphs, and test non-128 D/F shapes.",
  "date": "2026-10-08T16:02:08.812464+00:00"
}
```
