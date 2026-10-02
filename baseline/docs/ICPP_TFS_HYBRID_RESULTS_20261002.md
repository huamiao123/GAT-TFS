# Exact short-block AVX / dense-block AMX PH dispatch (2026-10-02)

Job 10855618 completed 0:0 on shared `intel` node `qhcn080` in 10:05; MaxRSS of the compute step was 59527356K. The run used 16 pinned physical cores (0–15), NUMA 0/1 interleave, the same destination CSR and untrained seed-11 three-layer Vanilla GAT for each path, one correctness run, one warmup, and three alternating measured repetitions. The timing includes three layers with each path consuming its own previous-layer output. It excludes graph degree sorting and static weight preparation, both recorded separately. Results are exploratory shared-node measurements, not a paper table.

The new path retains DegreeSort, TR16 scheduling, independent head attention and FP32 online max/denominator, local `PH` followed immediately by AMX `UW`, and no global `Z`, `U`, score or alpha matrix. For each destination neighbor block, fewer than 16 actual neighbors use the checked shared-source AVX FP32 `PH`; 16 or more use checked AMX BF16 high+low `PH` with preloaded coefficient tiles. Both feed the same bounded local RNE interface and AMX `UW`. Layer 3 (one head, 47 classes on products) falls back to the original block TFS kernel.

| Three-layer same-job median | ogbn-arxiv ms | ogbn-products ms |
|---|---:|---:|
| Strong B0 BF16 standard transform-first GAT | **68.923** | **3754.817** |
| Original B1 TFS premax | 422.791 | 15227.614 |
| Prior block B32 TFS | 306.066 | 13993.627 |
| Shared-source AVX PH + AMX UW B32 | **118.023** | 4427.571 |
| AMX preload PH + AMX UW B32 | 133.124 | 4070.692 |
| New hybrid threshold 16 B32 | 120.608 | **3942.222** |
| New hybrid threshold 24 B32 | 120.487 | 3964.330 |
| New hybrid threshold 32 B32 | 121.247 | 3999.665 |

Hybrid T16 is 3.86× faster than original B1 and 3.55× faster than old block B32 on products, but remains 4.99% slower than strong B0 BF16. On arxiv the best TFS candidate is the full AVX PH path at 118.023 ms, still 1.71× slower than B0. The hybrid is 1.75× slower than B0 there. The three products repetitions for T16 are in the raw log; these exploratory medians do not establish formal statistical significance.

Products layer medians show where the remaining gap lies: hybrid T16 L1/L2/L3 = 1058.876/1765.228/1106.190 ms versus B0 = 1623.428/1640.795/494.285 ms. Hybrid gains about 565 ms in L1, loses 124 ms in L2, and loses 612 ms in L3. The L3 fallback alone costs about 612 ms against B0 on this input. Arxiv hybrid L1/L2/L3 = 36.020/60.291/24.482 ms versus B0 = 30.421/31.393/7.118 ms, so B0 wins each layer.

For products L2, hybrid T16 chooses 1,505,008 AVX and 3,855,341 AMX destination neighbor blocks. Its physical `PH` work is 527.796 billion FMA versus 258.390 billion useful wide-aggregation FMA (2.043×), below pure AMX high+low's 702.592 billion (2.719×). Raising the threshold to 24/32 reduces physical-to-useful work to 1.866×/1.750× but is slightly slower end to end. The source feature processing width is still 256 versus B0's 32 per head: the exact direct aggregate-first sparse work remains 8×. Raw source `H` logical bytes are 64.598 GB versus the per-head block predecessor's 516.780 GB, an 8× cross-head reuse. All byte counters are logical instruction traffic, not measured DRAM bandwidth.

The independent hybrid smoke passed 46 optimized oracle cases, 6 fallback cases, 3 actual-LR/ELU layer cases, 2 fallback layer controls, and 4 profile/counter variants. Same-input, contracted-attention L2 checks and exact work identities passed. Profiled runs preserved output fingerprints. All real outputs were finite. The original absolute-error 0.003 master gate is **not passed** (also failed by B0 BF16): products final max absolute error versus FP32 B0 is 25.121 for B0 BF16 and 6.048 for hybrid T16; relative L2 is 0.02538 and 0.01513 respectively. No trained-checkpoint task accuracy is available.

Evidence: `baseline/runs/icpp-hybrid-10855618/` contains `arxiv.log`, `products.log`, `smoke.log`, all summary TSVs, status, Slurm configuration, affinity, pinning, hashes, preflight and source snapshot. Corrected build: `baseline/runs/icpp-hybrid-build-20261002-175141/`, binary SHA256 `b3812f9046f695f9ac6fd6987bb60f1aaaaf1dccd0c9225f9920f4bf1808505c`. The earlier compiler build at 17:49:54 was superseded before execution because of a misleading log field name and remains in local/server evidence, excluded from this performance result.

The curated GitHub export contains verified source, scripts, logs and summary tables; generated assembly and duplicate source snapshots stay in local/server run evidence. The historical source hash glob included three incomplete `icpp_head_block` drafts that were **never in the explicit compiler source list**. Those drafts are removed from the current source tree and excluded from GitHub. The retained source snapshot and exact compiler command identify the executed binary.

Next bounded experiment: retain TFS hybrid in layers 1 and 2 while dispatching layer 3 to standard transform-first BF16 GAT with contracted FP32 L/R. Label this as a mixed model path, alongside the intact all-layer TFS and strong B0 controls. Its purpose is to measure the cost of the one-head layer-3 fallback without claiming all-layer TFS speedup.
