# Static source audit: feature representation and projection granularity

Audit date: 2026-10-04. This document records source inspection and byte comparisons, before interpretation of experimental results. It contains no measured speedup, hardware-counter result, or claim that either feature representation wins. The numerical thresholds are preserved as requested.

## 1. Source identity and original controls

The fixed source is commit `8aeef1613fdeaeb929379874c4152eccde620f74`, tree `GCN-extra/runs/paper-method-build-20261004-123445/source_snapshot`. All five files in `frozen_source/` were independently compared with their Git blobs using `git show` and were byte identical:

| File | SHA256 |
|---|---|
| gcn_e2e_v3.cpp | 85c51bde92792f98223e80107e20b042f8f8d0e40068cdf6ce168143f0c7743f |
| paper_methods.cpp | 3671ac4736fa70abf54353460e68b1223b710f5f0f4bacba13a5e1f82a2212ee |
| paper_methods_kernels.hpp | 35d291b886ee0f00e79e0e5d17c6226b2fa1ad64eb715ecbe7bcb1c3f02e7b85 |
| paper_methods_runtime.hpp | 9d93d80ae1d811d3477aba39b563cf600c2fbd726c8f9b8e7a74e52446359fcc |
| source_protocol_kernels.hpp | 59b556373fcfa206bde29f3ab134e8e8955f5c854de328ef8b6ffec0cedab96b |

The BF16 B64/FULL controls call `gcn_extra_paper::block_kernel` from the frozen header. The original TFS and MKL anchors call the original functions in `gcn_e2e_v3.cpp`. `generate_driver.py` inserts an include and a supplementary call after the original comparisons and before freeing their buffers. Removing those two insertions must recover the original driver byte for byte; the generator asserts this. The build also compares the frozen original driver with the read-only yx copy. Source freezing does not imply that all untracked files in the current worktree belong to that historical commit; the build must retain its actual source hashes and generation manifest.

The FP32 header was independently compared with the frozen kernel. After removing its seven introductory comment lines, exactly five substitutions reproduce it byte for byte: namespace, reduction input pointer type, block-kernel input pointer type, source-row pointer type, and eight direct FP32 loads in place of BF16 expansion/shift. Each substitution occurred once. This establishes source-level identity of the remaining scheduling, partial packing, AMX projection, output staging and scatter. Compiler output must still be recorded separately.

DegreeSort, destination TR16, R64 OpenMP dynamic(1), D=F=128, KB4, NP2, FAST high-word BF16 partial packing, original weight packing, AMX tile configuration and C staging remain present. This experiment does not introduce the separate S/M scheduling proposal.

## 2. Precision semantics and correctness

FP32 feature storage does not make the dense projection FP32: both controls truncate each FP32 neighbor partial to BF16 and use the unchanged BF16 AMX projection with BF16 weights. The FP32 native two-layer path reads its actual FP32 first-layer activation directly at layer 2; the BF16 path converts that activation to BF16 first. This is an explicit difference in the native inference dataflow.

The independent reference preserves CSR addition order, each TR16 tile's maximum degree, B64/FULL partial boundaries, FP32 neighbor additions and high-16-bit partial truncation. It independently quantizes original row-major W, expands it to FP32 and applies dense FP32 FMA without calling AMX projection code. Each input path uses its own actual input representation, including the FP32 layer-2 bypass. Its FP32 FMA accumulation may differ from AMX accumulation in rounding order, so that reference requires a tolerance rather than byte identity.

Established gates remain unchanged: FAST FP32 relative L2 and normalized maximum below 0.01; BF16 final below 0.02 against original TFS/its own oracle; BF16 final below 0.03 against source MKL. These are acceptance thresholds, not a statement that observed errors equal the thresholds or that independent-oracle checks are bitwise exact. Preserve and report actual maximum, mean, relative L2 and normalized maximum errors.

Representation-only controls losslessly expand BF16 features to FP32 and require `memcmp` equality at matched layer-1 FP32, layer-2 FP32 and layer-2 BF16 boundaries, separately for B64 and FULL. Instrumented diagnostic outputs also require `memcmp` equality against the corresponding uninstrumented implementation. Finite checks and full indexed checksums are retained. Native FP32/BF16 differences are report-only; they are not failed equivalence checks and do not establish classification accuracy.

## 3. Logical work-count invariants

Let P be `projection_scopes`, T+ the number of destination TR16 tiles with positive maximum degree, and E the stored CSR edge count. For B64, P is the sum of ceil(tile_max_degree/64) across nonempty tiles. For FULL, P=T+. Empty tiles write output zeros without a projection scope. Padding of the final tile and shorter rows is included in executed AMX work.

| Count, per layer | Expected invariant |
|---|---:|
| source visits | E |
| FP32 feature additions | 128E |
| requested source bytes, BF16 / FP32 | 256E / 512E |
| decoded feature elements, BF16 / FP32 | 128E / 0 |
| neighbor blocks | P |
| projection panels | 2P |
| AMX A / B / combined input loads | 8P / 32P / 40P |
| TDPBF16PS instructions in source | 32P |
| executed padded AMX FLOPs | 524288P |
| C tile stores / local stored bytes | 8P / 8192P |
| C tile reloads / local reloaded bytes | 8(P-T+) / 8192(P-T+) |
| partial-zero bytes | 8192P |
| partial-packed BF16 bytes | 4096P |
| output bytes, FP32 / BF16 | 512N / 256N |
| prefetch instructions | 4(E-nonempty_row_blocks) |
| prefetch requested bytes | 256(E-nonempty_row_blocks) |

Each TDP has 16 destination rows, 16 output columns and a 32-element BF16 reduction dimension, giving 2*16*16*32=16384 FLOPs. Four K blocks, four output tiles and two panels give 32 TDP per projection scope. FULL therefore has zero C reloads in this implementation. C stores include final local staging, not only intermediate stores between scopes.

These are complete logical/code counts, not PMU-retired instruction counts or measured DRAM traffic. Compilers may fuse memory operands. Prefetch requests may hit caches, duplicate demand loads, or be dropped. Fixed four-line prefetching covers all of a BF16 source row but only half of a FP32 source row; the experiment keeps this instruction budget unchanged.

## 4. Measurement boundaries and diagnostics

Uninstrumented kernel timing covers one complete layer. Prepared E2E covers two layers, ReLU and the path-specific interlayer conversion, while excluding initial H preparation, graph loading, DegreeSort and W packing. Preparation-inclusive timings add the BF16 initial conversion in a separate boundary. Stage intervals telescope to that stage run's own total; they are not substitutes for the independently measured prepared E2E.

Each primary sample follows its own immediate warmup. Four factorial methods use five forward and five reverse orders. Original TFS and MKL anchors currently remain at positions 4 and 5 respectively; small changes against those anchors are more exposed to fixed-order effects than the internally balanced four-method comparisons. Keep per-sample order and shared-node dispersion visible.

Fine profile intervals overlap their coarse parent intervals and contain timer overhead. Sampled BF16 loads/decoding and BF16 conversion/stores deliberately use diagnostic staging to distinguish sections. These clocks can change register pressure and scheduling. Their summed thread milliseconds are not wall-clock percentages and must not be added across parent/child levels. Only uninstrumented timings supply speedup numerators and denominators. Per-thread completion times can diagnose a scheduling tail, but instrumented imbalance is not proof of the uninstrumented critical path.

All current PMU regions cover the native complete two-layer path, including ReLU and any interlayer conversion. They are separate passes and contain helper enable/disable overhead; they are not isolated feature-reduction measurements. Per-TID scaling, enabled/running coverage and enable/disable skew must accompany counts. Unsupported counters remain unavailable. AMX_BUSY measures speculative matrix arithmetic busy cycles, not retired TDP count, FLOPs or E2E wall fraction. Load-stall groups are overlapping indicators, not a complete TopDown decomposition.

## 5. NUMA and causal limits

Original H0 FP32 is serially initialized, whereas H0 BF16 is created by parallel static conversion. Original H1 FP32 is first produced by DegreeSort/dynamic scatter; H1 BF16 is produced by parallel static conversion. Keeping the default NUMA policy does not make their physical page placements identical. The supplementary qFloat buffer is first touched by the same flattened logical static partition as BF16 conversion; it is not substituted for the native source allocations.

`/proc/self/numa_maps` gives VMA-level placement totals. Buffer addresses help correlate ranges, but adjacent anonymous allocations can share a VMA. Such a map must not be presented as exact per-buffer page placement without range-resolved evidence. The study makes no NUMA policy change, page migration or HBM configuration change.

Within a fixed B, matched-kernel bitwise equality supports comparison of source representation, footprint and decoding under the fixed prefetch policy. It does not isolate decoding from doubled requested bytes, changed cache coverage or physical placement. Native E2E additionally changes H0 quantization, interlayer rounding and conversion work. B64 versus FULL also changes partial quantization boundaries along with projection count and C staging, so it is not a pure cache ablation.

Report native and matched ratios separately. A useful descriptive interaction is log(T_BF16_FULL/T_FP32_FULL)-log(T_BF16_B64/T_FP32_B64), computed separately within each measurement boundary. Retired cache misses, instructions and load-stall changes can corroborate a mechanism, but they do not alone identify a unique bottleneck or prove process-attributed DRAM bandwidth. Shared uncore traffic is not attributed to this process; requested bytes must not be relabeled as measured bandwidth.

Use only current same-graph, same-process, same-node/protocol denominators. Earlier exclusive-node timings are background and must not enter shared-node speedup ratios. Preserve failures and slowdowns. The experiment remains source-defined random H/W two-layer inference computation, not a trained GCN classification model or an accuracy evaluation. No conclusion about speedups or journal novelty is made by this static audit.
