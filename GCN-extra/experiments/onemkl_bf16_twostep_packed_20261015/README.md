# Prepacked-W oneMKL BF16 two-step control

This control extends the sibling `onemkl_bf16_twostep_20261015` experiment with one method: `bf16_mkl_packed_twostep`. It still computes full `Z=AH` through the FP32 oneMKL Sparse BLAS API, converts `Z` to BF16, and performs a separate BF16-input/FP32-output oneMKL GEMM. The only new factor is preparing `W` through `cblas_gemm_bf16bf16f32_pack` before the timed interval and using `cblas_gemm_bf16bf16f32_compute` in the timed GEMM. This matches the original TFS treatment of W packing outside the timed interval.

The program keeps the original un-packed BF16 path intact as a within-process control. For every graph, it checks each BF16 path against original TFS and checks FULL DegreeSort vs Identity bitwise. Six methods each receive one warmup and five rotated measured repetitions. Stage logs distinguish the packed and un-packed paths.

The 32-core fixed-17 shared-node run is `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/onemkl-bf16-twostep-packed-20261015-163448/full17` (job `10973268`, node `qhcn522`). Its summary and raw per-graph logs are mirrored in `GCN-extra/results/onemkl-bf16-twostep-packed-20261015/`. This is a diagnostic, not an exclusive-node formal benchmark. Compare methods **within the same run**; absolute times from the separate `qhcn071` run are not directly controlled because of shared-node conditions and differing CPU affinity.

An additional 4-core fixed-17 run used job `10973286` on node `qhcn015`; its summary is mirrored as `full17_4core.csv`. The 32-core and 4-core results must remain separate.
