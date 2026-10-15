# oneMKL BF16 two-step GCN control (2026-10-15)

This experiment compares the original TFS V3, FULL with and without DegreeSort, an FP32 oneMKL two-step control, and a BF16 oneMKL two-step control in one program. The original TFS functions are included unchanged from `GCN-extra/original/gcn_e2e_v3.cpp`.

The BF16 oneMKL dataflow for each layer is:

```
BF16 H -> decode to FP32 -> oneMKL mkl_sparse_s_mm (FP32)
       -> full FP32 Z=AH -> quantize Z to BF16
       -> oneMKL cblas_gemm_bf16bf16f32 (BF16 Z and W, FP32 C)
```

Layer 1 is followed by FP32 ReLU and FP32-to-BF16 conversion; layer 2 follows the same sparse and dense stages. Initial H and W preparation, inspector setup, DegreeSort, and original TFS W packing are outside the timed interval. The two SpMM results are fully materialized, so this is a genuinely separated SpMM-then-GEMM path. It is **not** an all-BF16 sparse kernel: the available oneMKL Sparse BLAS call is FP32. Its intermediate `Z` also incurs BF16 quantization before the dense product, which is an additional numerical difference from original TFS.

Each graph runs five warm-up methods, followed by five rotated measured rounds. The stage timers surround each sparse call, intermediate conversion, BF16 GEMM, ReLU, interlayer conversion/decode, and final BF16 conversion. Stage medians and end-to-end medians are reported separately and need not sum. All methods use the same CSR, random H/W seed, 128 input/output width, 32 threads for the formal diagnostic set, and default NUMA policy. Shared-node runs are diagnostic, not paper-qualified.

`GCN-extra/results/onemkl-bf16-twostep-20261015/fixed17.csv` is the consolidated 17-graph result. Raw logs, environment, source snapshot and hashes are retained on the server under `GCN-extra/runs/onemkl-bf16-twostep-20261015-081637`.

The packed-W follow-up is in the sibling `onemkl_bf16_twostep_packed_20261015` experiment. It isolates oneMKL GEMM weight packing while leaving the sparse and conversion stages unchanged.
