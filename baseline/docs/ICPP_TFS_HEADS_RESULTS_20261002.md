# Cross-head TFS PH→UW, 2026-10-02

This is exploratory evidence from Job **10855493**, not a paper result. Fixed 3-layer Vanilla GAT, destination-row CSR with self loops, seed 11 **untrained** parameters, independent heads, contracted FP32 L/R attention, exact FP32 online max/denominator/normalization, BF16 RNE H/W and local U to AMX UW. Same own-output 3-layer timing interval, one correctness run, one warmup, three alternated repetitions; medians below. All nine paths ran in the same process and allocation on shared intel `qhcn080`, 16 physical cores 0–15 of socket 0, NUMA 0/1 interleave. Degree sort and static preparation were outside steady-state time and separately logged. The other account job was untouched.

| Path | arxiv 3-layer ms | products 3-layer ms |
|---|---:|---:|
| B0 FP32 standard GAT | 75.677 | 3855.633 |
| B0 BF16 standard GAT | **69.356** | **3762.105** |
| B1 original TFS | 425.860 | 15198.386 |
| Pair B32 original TFS | 355.015 | 10631.640 |
| Block B32 TFS local U→AMX UW | 307.916 | 14073.959 |
| Shared-H AVX PH→AMX UW B16 | 136.330 | 4775.773 |
| Shared-H AVX PH→AMX UW B32 | **120.496** | 4405.851 |
| Shared-H AMX high+low PH→AMX UW B16 | 164.075 | 4849.547 |
| Shared-H AMX high+low PH→AMX UW B32 | 136.677 | **4097.741** |

Relative to old Block B32, best new path gains **2.56× arxiv** (AVX B32) and **3.43× products** (AMX B32). Relative to original B1, gains **3.53×** and **3.71×**. The best new path remains **1.74× slower than B0 BF16 on arxiv** and **1.09× slower on products**. Best method changes by dataset; the AMX PH microbench gain on full first blocks did not predict arxiv full graph.

Second layer medians: arxiv B0 BF16 31.793 ms, old Block 185.545, AVX B32 60.101, AMX B32 72.180; products 1643.898, 8837.169, 2058.045, 1911.535 ms respectively. Full model includes L1 and L3; L3 is a one-head C-wide fallback to the old Block path for new wrappers. No method silently alters model shape or shares attention weights.

The shared-H branch logically reads `E×D×2` source BF16 bytes rather than `E×K×D×2` in the head-separate Block path. In products L2 it accounts for **64.598 GB logical H reads**, versus **516.780 GB** for head-separate Block. This is source-load reuse, not a claim about measured DRAM traffic. It still executes **258.390 billion useful wide PH FMA**, 8× the narrow GAT sparse feature work. AMX high+low B32 executes **702.592 billion physical PH FMA** (2.719× useful, accounting for K32/feature padding and two coefficient passes), packs **87.824 GB logical H**, scatters **43.912 GB local U**, and stores/reloads **5.498/2.990 GB** local V contexts across blocks. These memory accesses are counted software traffic, not hardware counter measurements. The V buffer exists because 8 heads×2 output tiles do not fit in 8 AMX tile registers. The rescale happens in that local V buffer; additional rescale-triggered tile spill/reload is zero, but row scaling and normal head-context traffic are included separately.

On arxiv L2, AMX B32 performs **25.898 billion physical PH FMA / 5.089 billion useful**, 5.089×. Its average effective row-block length is 12.576, so K32 padding plus two coefficient passes hurts more than in products. The earlier compact PH microbench sampled first full blocks of degree≥32 and measured a different operating point. The new full-path AMX high+low does real `TDPBF16PS` PH and AMX UW with one palette; source and weight packing, PH store/scatter, UB BF16 conversion, V context movement, score/exp and normalization are inside the full layer/model timers. Fine stage timings are sampled worker sums with clock overhead and cannot be read as percentages of wall time; the speed conclusions use uninstrumented timing repetitions.

The correctness gate before speed passed all prior Online/Pair/Block controls and new **28 optimized operator, 4 fallback, 2 real-LR layer controls**. New oracle separately simulates FP64 stable attention, exact SVML `z0` max/den, AVX FP32 PH, two-pass high+low BF16 AMX PH, RNE U, AMX UW even/odd accumulation and operation-derived error bounds. All real-graph outputs finite, measured profile/counter variants bitwise equal to speed output, and per-row/global work identities pass. Production assembly includes `TDPBF16PS` and `__svml_expf16_z0`.

The original **0.003 absolute master gate still fails** on both real graphs for BF16 B0 and all TFS candidates. Final products max absolute error vs own-output FP32 B0: B0 BF16 25.121, AVX B32 6.043, AMX B32 6.048; relative L2 0.02538, 0.01513, 0.01513. These use an untrained seed, so no trained task accuracy is established. Keeping the gate visible is required; differences also include BF16 X/W, attention contraction and local U BF16 rounding. This result is a performance research lead, not an accuracy-certified model.

Raw evidence: `baseline/runs/icpp-heads-10855493/` contains `smoke.log`, arxiv/products logs, machine TSVs, manifest, pinning, hashes, source snapshot, Slurm config, status. Build `baseline/runs/icpp-heads-build-20261002-165027/`, binary SHA256 `ded2462a348db4b04530622660ffcd9afe134e6b808e324f83115ac97aa964e4`; Job completed `0:0` in 7:12, MaxRSS `59369432K`, source/data hashes in run. Old incomplete `icpp_head_block` leftovers excluded from binary. Build attempt `icpp-heads-build-20261002-164747` ended before numerical work due to absent `rg`, retained as failure; successful build uses `grep -E` for opcode audit.

Next, test one bounded tile-register change: preload both PH coefficient components once per destination block into TMM6/TMM2, keep H in TMM5 and output in TMM7, preserving high→low TDP order. This removes repeated P loads across feature blocks. Test bitwise equality and measure full-model B0/old-AMX/preload in the same job. Future algorithmic work must explicitly address remaining 8× useful wide PH arithmetic; this change affects only avoidable tile load traffic.
