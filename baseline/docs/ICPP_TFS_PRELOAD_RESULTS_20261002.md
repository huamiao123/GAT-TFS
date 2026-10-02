# PH coefficient tile preload, 2026-10-02

Exploratory Job **10855585**, shared intel `qhcn080`, 16 physical cores on socket 0, NUMA 0/1 interleave, completed 0:0 in 8:23. Same destination CSR, untrained seed 11, 3-layer 8×32/8×32/1×C Vanilla GAT and own-output timing as the preceding Heads experiment. One correctness run, one warmup, three alternating measured repetitions. Identical graph/model/precision/threads/binary for old AMX and preload. Strong standard GAT B0 BF16 was timed in this same job.

Preload retains DegreeSort/TR16 and independent Online max/den/P for every head. It loads `P_hi` into TMM6 and `P_lo` into TMM2 **once per destination row and neighbor block**, then reuses both across `Dpad/16` feature tiles. TMM7 computes PH; the local U block immediately enters the unchanged AMX UW step. TMM output head state remains worker local across blocks. Same TDP high then low order and unchanged RNE interface. The predecessor loaded high and low P once per feature tile.

| Same-job 3-layer median | arxiv ms | products ms |
|---|---:|---:|
| B0 BF16 standard GAT | 69.157 | 3773.251 |
| Shared-H AVX PH, AMX UW B32 | **119.143** | 4444.704 |
| Old AMX high+low PH, AMX UW B32 | 134.822 | 4071.255 |
| New AMX high+low preload, AMX UW B32 | 133.428 | **4058.823** |

The preload improves old AMX by **1.010× arxiv** and **1.003× products** in 3-layer E2E medians. Its second layer alone improves from 70.861 to 67.065 ms on arxiv and 1891.413 to 1807.526 ms on products; the first layer gets slower from 39.620 to 42.104 ms and 1067.313 to 1135.831 ms. We do not attribute this first-layer slowdown to a specific hardware cause without counters. The entire model remains **1.93× slower than B0 BF16 on arxiv** and **1.076× slower on products**. The arxiv AVX path is still best among TFS candidates. Three repetitions and overlapping product timing ranges limit confidence in the small E2E preload gain.

At layer 2, AMX PH calls and executed work are exactly unchanged: arxiv 6,322,752 PH TDP and 25.898 billion physical FMA; products 171,531,168 TDP and 702.592 billion FMA. Useful wide PH work remains 8× B0's narrow sparse feature work. Derived logical P tile loads fall `rowblocks × (Dpad/16) ×1024` → `rowblocks ×1024`: **3.237 GB → 0.202 GB arxiv** and **87.824 GB → 5.489 GB products**. This is L1/L2 tile input traffic implied by instructions, not measured DRAM traffic. The already packed H traffic, local U scatter, and V context movement are unchanged. Sampled worker PH-load spans drop 1.777→1.089 ms arxiv and 47.484→30.730 ms products; those spans include timing overhead and cannot be treated as wall-time shares. The uninstrumented E2E rows above govern speed claims.

Prior independent 28 optimized operator + 4 fallback + 2 real-LR layer checks ran before the new regression. The preload regression passed **18 operator cases × four profile/counter variants**, **6 layer controls × four variants**, old/new raw numerator/max/den/output bitwise equality and exact work-count identities. Profiles on real graphs preserved uninstrumented output fingerprints. All real outputs finite. Original `.003` absolute master gate still fails: final arxiv max abs 0.00412494 for old/new; final products 6.04773712 for old/new. Trained task accuracy remains unverified. No softmax approximation or shared-head attention was introduced.

Artifacts: `baseline/runs/icpp-preload-10855585/` has raw smoke/arxiv/products logs, timings/profiles/checks/work TSVs, status, manifest, binding and hashes; `baseline/runs/icpp-preload-build-20261002-171807/` has binary/compiler/source snapshot/command/assembly. Binary SHA256 `f75dc00f1e2c42f216e97964a2591574f2914b3a90b24a08bb2fcd5119dd5721`; MaxRSS `59386932K`. Exploratory evidence only, not a paper table.

The remaining concrete opportunity is block occupancy: short rows pay K32 padding and a double P pass. An exact mixed PH dispatcher can use the checked shared-H AVX PH for short blocks and checked preloaded AMX PH for dense blocks, while retaining identical FP32 Online state and AMX UW fusion. This will require independent mixed-path oracle checks and same-job E2E timing; neither backend should be chosen from the first-block microbenchmark alone.
