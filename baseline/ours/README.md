# Local-U Online candidate

Independent research implementation; original B0 and B1 kernels are unchanged.

For each head: FP32 `bL=W aL`, `bR=W aR`; FP32 `L/R=H [bL,bR]`; destination-row CSR blocks maintain independent `m,l,U[D]`; consume local `U` immediately with that head's W, then divide the d-dimensional output by l. No max prescan or global Z/U/e/alpha in either local path. Hidden layers use the existing ELU.

Original TFS's ascending degree logical permutation, TR16 destination tiles, dynamic panels and original-row scatter are reused. Sparse aggregation is AVX-512 over FP32 original H. Execution order differs from the original neighbor-step AMX projection B1. This is a new candidate, not a replacement or relabeling of B1.

- `local_online_fp32`: FP32 U and one single-thread SGEMM per local destination tile/head. Outer OpenMP owns parallelism; worker-local MKL thread count is restored.
- `local_online_amx_hilo`: same FP32 attention and online U. Split U and W into RNE BF16 high plus RNE residual low; custom AMX sums all four products into FP32. AMX accelerates the final UW, not sparse PH. Approximate GEMM precision must be measured separately; softmax is unchanged.

Online row rescale occurs in local FP32 memory before UW. There is no live AMX accumulator during neighbor traversal, so no AMX rescale spill/reload. Local U necessarily has memory traffic; `U_global_bytes=0` means no complete N×D intermediate, not zero L1/L2 traffic.

Speed runs preallocate workspaces and exclude graph load, model/weight setup, sorting and correctness comparisons. Separate profile runs sample tile phases and collect exact row/head block/update/rescale counts. Worker sample sums are not additive wall times. Head concatenation writes directly to output; normalization and original-row scatter are fused.

Build: `bash baseline/scripts/build_local.sh` on the authorized cluster workspace. `build/local_online --smoke` validates FP32 before AMX, D/K/d tails, empty rows, blocks16/32/64, and independent stable FP64 adversarial-attention oracle. Near-zero approximate AMX smoke outputs use a declared absolute-error fallback, plus a direct comparison against the same FP32 local backend. This is not task accuracy acceptance.

`scripts/run_local.slurm` runs smoke before matched real arxiv/products comparisons with B0 FP32, B0 BF16 and B1, then separate profile/counters. The real-graph accuracy comparator is the previously validated B0 FP32, with seed11 untrained parameters. An actual checkpoint and frozen task accuracy gate are still needed.
# Grouped-head Online research update (2026-10-01)

New independent candidates: [joint_online.cpp](joint_online.cpp) (v1) and [joint_full.cpp](joint_full.cpp) (full-group specialization). Each head retains its own W/a, L/R, block m/l/U, weights and denominator. Both aggregate original H, consume worker-local U immediately with per-head UW, and normalize after UW. Degree Sort/TR16/dynamic panels remain; no global Z/U/e/alpha is created by these candidates. Sparse aggregation and UW are FP32, not AMX PH or BF16 B1.

The specialization removes only active-head guards for divisible groups. Partial head groups retain guards. [joint_main.cpp](joint_main.cpp) provides the original-baseline/control suite and sampled fixed-attention FP64 oracle; [joint_compare_main.cpp](joint_compare_main.cpp) compares v1/full and B0 controls in the same binary. [joint_checks.cpp](joint_checks.cpp) declares oracle work limits, complete sampled neighborhoods and explicit skipped high-degree rows.

Read current server AGENTS.md and research SKILL.md before building/running. The remote root is `/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline`; write only inside wzh. Reproduction launchers are `scripts/build_joint.sh`, `scripts/run_joint.slurm`, `scripts/build_joint_full.sh`, `scripts/run_joint_full.slurm`. The payloads enforce 16 physical cores on one socket and explicit NUMA interleave before allocations. Tests precede real graph timing, and sampled profiles/counters run separately.

Evidence: [results and interpretation](../docs/JOINT_ONLINE_RESULTS_20261001.md), [method research](../docs/METHOD_RESEARCH_20261001.md), [job10849337](../runs/joint-10849337/RESULTS.md), [job10849393](../runs/joint-full-10849393/RESULTS.md). Small tests pass (576 v1 and 1008 specialization configurations, bit-identical controls); real same-input implementation controls also pass. Master absolute-error and trained-task acceptance remain UNVERIFIED. These are exploratory random-weight three-layer forward measurements on official graph/features; neither candidate beats strong B0 in this cycle.
