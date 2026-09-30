# GAT baseline issues

2026-09-30: first compile passed; no new compile issue. Production accuracy thresholds and task metrics are unconfigured because no trained GAT checkpoint has been supplied. BF16 oneMKL API compiles; vendor AMX dispatch has not yet been established. Old system objdump may not decode AMX instructions, so absence of disassembly matches is not evidence of absent instructions. Record compiler-generated assembly as well as runtime kernel checks.

2026-09-30 check-10808045: correctness tests passed. The initial reordered AVX candidate gathered strided contraction weights inside the dot and was unnecessarily slow. Replace with static transposed bL/bR and fused two-dot reading X once. Skinny SGEMM micro candidate initially allocated/setup temporary weights in the measured function; move preparation to static workspace for a fair comparison. These changes require rebuild and regression before accepting new timings. No correctness failure was observed. BF16 task accuracy remains unconfigured.

2026-09-30 build-20260930-152447: updated sources compile and generated assembly confirms TDPBF16PS. No new compile issue. AVX and static-SGEMM changes still require runtime regression; vendor GEMM AMX counter event is available in perf list but permission is not yet tested.

2026-09-30 arxiv-10808152: srun inherited a one-core step allocation despite the batch job requesting 16 CPUs. Manifest correctly captured OMP/MKL=1, so performance numbers are only single-core diagnostics. Fix: explicitly pass --cpus-per-task=$SLURM_CPUS_PER_TASK to srun, and fail a multi-core validation if affinity exposes fewer than two physical cores. Retain this run and rerun. Vendor AMX counter succeeded; no new numerical implementation issue. BF16 checkpoint accuracy still unconfigured.

2026-09-30 build-20260930-154006: no new compile issue. Speed build still compiles per-edge/per-vector clocks and stats out; heavy worker profiling is a separate binary. Runtime rerun must verify actual step affinity before accepting multi-core timings.

2026-09-30 arxiv-10808269: srun affinity defect resolved; actual16physical cores on one socket. Numerical regressions passed, no new correctness issue. Fine clocks are expensive: L2 TFS profile wall3749ms vs speed345ms, so worker subfield proportions are instrumented diagnostics, not unbiased bottleneck fractions. Use speed-build stage wall/fixed-p and future sampled/microkernel measurements. B0FP32 outperformed BF16 in this shared-node run; keep both results and avoid unsupported hardware-cause claims. PowerShell ConvertFrom-Json treats D/d as case-insensitive duplicate keys; raw JSON is valid, use Python summarizer. Trained-checkpoint/task gate still missing.

2026-09-30 local evaluator fixture: no new issue; classification/accuracy-drop and all input rejection tests passed. This does not calibrate a real checkpoint gate.

## B0 diagnosis build 2026-09-30
- build-20260930-161352: added optional warm initialization and NUMA/worker diagnostics; build passed. Kernel mathematics unchanged. Next: one shared node ABBA comparison, original vs warm initialization.


## diagnosis-10809043 2026-09-30
- COMPLETED, qhcn025 shared intel; requested16 but selected10 same-socket physical cores22-31. 3warm9repeat ABBA per initialization. Boundaries PASS. Original BF16123.97/124.70ms vsFP32134.17/134.46; warm BF16124.50/124.78 vsFP32133.89/133.68. Buffer mapping nodes differ (original FP32N3 vsBF16N2), warm bothN2. Not equivalent to previous16core run; continue on qhcn059. Evidence baseline/runs/diagnose-10809043, hashes recorded. Not paper acceptance.


## diagnosis-10809069 2026-09-30
- COMPLETED qhcn059 intel shared,16physicalcores44-59,NUMA5/6/7. BoundariesPASS. OriginalFP32102.36-103.28ms buffersN6; BF16167.87-169.91ms buffersN5. Warm bothN5:FP32157.37-157.60,BF16173.77-175.01ms. Same kernels: moving FP32 initialization changed locality and caused ~55ms slowdown. Primary NUMA confound confirmed; next bindbothN6/interleave identical policies, same ABBA3warm9repeat. Not paper acceptance. Evidence baseline/runs/diagnose-10809069.


## diagnosis-10809096 2026-09-30
- COMPLETED qhcn059 shared intel,16cores44-59. BoundariesPASS,3warm9repeatABBA. BothN6 FP32105.01ms/BF16105.77-105.84; bothinterleave5,6,7 FP3273.81-74.31/BF1668.26-68.63,ratio1.082x. Same-precision outputSHA256 identical acrossbothpolicies/repeats. Rootcause confirmed initialization-dependent NUMA firsttouch, no kernelchange. arxiv_payload launcher amended explicitinterleave acrossallocatedNUMA nodes (next-run policy; not yet full B1 rerun). Old FP32/BF16 performance comparison superseded; B1 mustrerun beforeupdated speedup claims. Report docs/B0_SLOWDOWN_DIAGNOSIS_20260930.md and runs/diagnose-10809096. No paper acceptance.

## 2026-09-30 — arxiv-10809143: B1 remains slow after NUMA correction

- Unified interleave reduces B1 from old 586.236 to 448.673 ms, but B0 benefits more and is 67.976 ms. B1 is still 6.60 times slower. Second-layer kernel accounts for 253.439 of 259.107 ms; fixed-p backend remains 10.03 times slower.
- Source audit confirms neighbor-step `(pX)W` repeats projections, as prescribed in the implementation contract and inherited original kernel. Layer 2 executed projection FLOPs are 15.58 times B0; degree-sort row utilization is 94.2%. This is an execution-order cost, not evidence that degree sorting was absent or is itself the main bottleneck.
- Existing fine profile inflates Layer 2 to 3654 ms, so its substage ratios cannot quantify actual gather/conversion/compute contributions. Use sampling or controlled microbenchmarks for further attribution.
- Resolution: accurate diagnosis recorded, baseline preserved; any per-destination UW alternative must be a separate method. No new correctness failure. Evidence: docs/B1_SLOWDOWN_DIAGNOSIS_20260930.md and runs/arxiv-10809143.
