# PROJECT_PROGRESS — current controlled GCN-extra experiments

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
