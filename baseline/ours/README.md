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
