# Original ICPP TFS with block Online Softmax

## Verified 2026-10-02 extensions

- `icpp_heads.cpp`, `icpp_heads_preload.cpp`, and `icpp_heads_hybrid.cpp` keep DegreeSort/TR16, independent per-head Online state and AMX `UW` fusion while sharing original source `H` reads across eight heads. Short neighbor blocks use checked AVX FP32 `PH`; longer blocks use AMX BF16 high+low `PH`. The block-local aggregate is consumed immediately by AMX `UW`. The physical work falls, but direct aggregate-first L2 still processes eight times B0's useful sparse feature width. See the [heads](../docs/ICPP_TFS_HEADS_RESULTS_20261002.md), [preload](../docs/ICPP_TFS_PRELOAD_RESULTS_20261002.md), and [hybrid](../docs/ICPP_TFS_HYBRID_RESULTS_20261002.md) reports.
- `adaptive_main.cpp` adds an explicitly mixed model path: TFS hybrid in L1/L2 and standard BF16 transform-first GAT in L3 with contracted FP32 L/R. Same-job products E2E was 3411 ms versus strong B0 at 3845 ms; full TFS was 4033 ms. Arxiv remained slower than B0. See the [adaptive result](../docs/ICPP_TFS_ADAPTIVE_RESULTS_20261002.md). This does not make the entire three-layer model a TFS dataflow.
- Build and run through `../scripts/build_icpp_adaptive.sh` and `../scripts/run_icpp_adaptive.slurm`. `../scripts/summarize_icpp_adaptive.py` regenerates TSV summaries from complete raw logs. Prior independent smoke and real-graph numerical checks passed; the original strict master absolute-error gate and trained task accuracy remain unaccepted.

This extension adapts `../src/baseline_tfs.cpp`. The frozen B0/B1 kernels are unchanged. It is distinct from the FP32 local-U and grouped-head prototypes.

## Preserved execution

- Destination-row CSR, logical Degree Sort and original-node output order.
- Dynamic OpenMP row panels and TR16 synchronized neighbor steps.
- Independent per-head W, attention and softmax state; fixed 8x32 / 8x32 / 1xC model.
- BF16 input, RNE weighted input, per-head VNNI-packed W.
- **Each neighbor step performs AMX `BF16(pH) * W` and adds to resident FP32 output TMM.** There is no separate final SGEMM.
- Full graph Z, edge scores, alpha and aggregate U are not allocated. Scratch is thread-local and bounded.

## Online state and tile lifetime

Attention uses the unchanged Vanilla GAT reassociation `bL = W*aL`, `bR = W*aR`, `L/R = H*bL/bR`. The benchmark freezes B1 and this path to FP32 attention preparation.

For each destination and head, maintain FP32 `m`, `l`, and the output-dimensional accumulator `V = U*W`:

```
m' = max(m, max(scores_in_block))
r  = exp(m-m')
p  = exp(scores-m')
l' = r*l + sum(p)
V' = r*V + sum_j (p_j*H_j)*W
O  = V/l
```

The first block initializes the maximum without rescaling a previous accumulator. On a later maximum increase, all output tiles are stored to thread scratch, only changed destination rows are scaled with AVX-512, and all output tiles are reloaded. Unchanged maxima keep output tiles resident. Empty and finished lanes retain independent legal state.

Layer 2 rescales **32 logical output values per changed row/head**, instead of the 256 input-dimensional values used by local-U. Physical spill/reload still transfers whole TR16 output tiles. The kernel still gathers and processes all 256 input features per head and executes projection for every synchronized neighbor step; it does not solve repeated source projections or eliminate wide sparse work.

This identity holds in real arithmetic. BF16 rounding of `pH`, FP32 reduction and row rescaling introduce precision differences, which are explicitly checked and reported.

## Files

| File | Role |
|---|---|
| `icpp_online.cpp/.hpp` | Actual AMX kernel, dispatch, bounded workspace and stats |
| `checks.cpp/.hpp` | FP64 stable oracle, FP32 V simulator, BF16 instruction emulator, original-B1 isolation and boundary checks |
| `main.cpp` | Same-binary B0 FP32/BF16, original B1 and new Online comparisons, three-layer timing and separate profiling |

Build and run entry points are `../scripts/build_icpp_online.sh` and `../scripts/run_icpp_online.slurm`. Numerical workloads run on Slurm compute nodes.

## Timing and counters

Speed repetitions disable hot-loop clocks and counters. Layer timers include conversion, attention L/R, complete kernel, normalization and activation. Three-layer E2E uses each path's own preceding output. Static weight preparation, Degree Sort, workspace allocation, copies and validation are outside steady-state timing and separately disclosed.

Sampled profiling is a separate pass and logs worker-time sums for score generation, block maximum, exp/denominator, tile spill, row scaling, tile reload, source gather, weighted BF16 conversion, tile load, AMX compute, final store and output scatter. These sums are **not an additive wall-time decomposition**. Profile fingerprints must match the untimed specialization.

Exact counters cover all row/head states: neighbor blocks, maximum updates including first initialization, actual rescales, logical and padded rescaled features, rescale tile events, physical spill/reload bytes, AMX calls and padded executed FMA. Rescale-per-block P50/P90/P95/P99 include empty rows as zero.

## Acceptance

The original master absolute-error gate `.003` is unchanged. Bounded smoke acceptance and agreement with a BF16 emulator establish implementation controls, not trained-model accuracy. Real-graph master errors and high-degree complete-neighborhood oracles are reported independently. Shared-node timings with untrained seed11 weights are exploratory and cannot enter a formal paper table without the remaining precision/task and performance acceptance work.

The fixed oneAPI 2024.1 checks match the SVML entry actually emitted for the AMX kernel (`__svml_expf16_z0`). Assembly showed that the same intrinsic in the independent emulator originally selected `__svml_expf16`, producing small ULP differences. This internal library entry is used only by the diagnostic harness. Standard `std::exp` remains an independent control; the SIMD denominator replay must match the kernel bitwise. A scalar positive-sum rounding budget is an additional diagnostic, not a replacement for this exact replay or the unchanged master gate.

## Independent head-pair source reuse candidate

`icpp_pair.cpp/.hpp`, `pair_checks.cpp/.hpp` and `pair_main.cpp` are a separate local candidate. They preserve the original neighbor-step AMX fusion and keep two heads' W, attention, softmax state and output tiles independent. Even head counts with head dimension at most 32 share raw H loads and expansion; other shapes fall back to the existing Online kernel. Per-head weighted conversion and AMX work are unchanged.

The new harness contains 57 operator cases and two layer wrappers, with exact original-Online comparisons and profile/counter controls. Complete job10853989 passed these controls and real-graph three-layer bitwise comparisons. Same-job arxiv/products medians328.160ms/10.023882s are1.255x/1.472x faster than Online, still slower than B0 BF16. The original master gate remains failed. See [status/results](../docs/ICPP_TFS_PAIR_STATUS_20261002.md); build/run entries are `build_icpp_pair.sh` and `run_icpp_pair.slurm` under `../scripts/`.

## Block-granularity SpMM to AMX GeMM extension

`icpp_block.cpp/.hpp` keeps DegreeSort/TR16, independent block Online states and resident output V. It forms a bounded FP32 U_B from one neighbor block, converts U_B to BF16 and immediately consumes it with real AMX U_BW. Old V alone is rescaled; U_B is cleared for every block. No full Z/e/alpha/U and no final SGEMM. This reduces repeated dense projection while retaining KED input-width sparse work. Moving the quantization point from each pH to U_B requires its own oracle; it is not bitwise identical to original Online.

`block_checks.cpp` provides57 operator and2 actual-LR/ELU layer controls; `block_main.cpp` compares8 paths, full own-output3layer timing and separate sampled profiling. `head_ph_probe.cpp` is an additional sampled first32-neighbor PH diagnostic comparing shared-H AVX to one/two-pass BF16 AMX with independent8head rows. It does not certify end-to-end acceleration or the master gate. `reuse_probe.hpp` reports sampled source unions/support masks, not a full-graph estimate.

Build/run entries: `build_icpp_block.sh`, `run_icpp_block.slurm`. Job10854102 passed operator smoke then failed in driver matched-attention scratch allocation after partial arxiv timing; preserved. Fixed job10854118 completed both graphs and all internal controls: best block E2E289.028ms/arxiv and12.954649s/products, still slower than B0; master gate remains failed. See [complete results](../docs/ICPP_TFS_BLOCK_RESULTS_20261002.md).

`head_ph_compact_probe.cpp` is an additional standalone micro diagnostic:8 configured head rows and direct vector native-BF16 VNNI packing. Entry points `build_icpp_ph.sh`/`run_icpp_ph.slurm` use `ph_main.cpp`. Complete job10854131 passed first-block instruction/den controls; conservative products high+low micro speedup1.237x against the faster duplicated AVX control. Not integrated full-model PH/UW, not master/task acceptance, and does not eliminate effective8x wide arithmetic.
