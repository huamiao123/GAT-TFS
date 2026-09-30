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

## history-build-20260930-165236
- Build PASS: compare_history links preserved historical FP32 functions and current kernels without modifying either. Added same-graph, bit-identical parameter/permutation checks and allocation-inclusive forward boundary. Source, binary, rules hashes saved server-side. Next: one shared node, arxiv/products, 16 physical cores and explicit NUMA interleave; three measured repeats. No new issue.


## history-10809340 — 2026-09-30
- COMPLETED 0:0, qhcn059 intel shared, 1 node, 16 physical cores 44-59, explicit NUMA 5/6/7 interleave, elapsed 00:10:44, max RSS 18956172 KB. icpx 2024.1.0, oneMKL 2023.0 Update2; artifact/source/rules hashes in runs/history-10809340. Official arxiv and products graphs/features, seed 11/22/33; W/a and degree permutations checked bit identical. Boundaries and finite checks PASS; each path 1 warmup, 3 measured repetitions with alternating order.
- Allocation-inclusive bridge medians ms: arxiv old Reference 1346.797, old parallel transform-first 334.957, old TFS local-U 440.355, new FP32 B0 177.017, BF16 B0 204.093, B1 544.954. Products corresponding medians 70227.515 / 8358.110 / 17110.845 / 5363.311 / 5643.935 / 18024.315. Old TFS speedup against old Reference still 3.058x / 4.104x, but slower than old parallel control. New B1 vs old TFS regression 23.8% / 5.3%. Main speed-ratio reversal is baseline strength plus execution-order and timing-boundary changes.
- Separate preallocated production ms: arxiv FP32 B0 74.065 / BF16 B0 68.349 / B1 444.112; products 3910.139 / 3812.717 / 16080.420. Do not mix the two timing boundaries. No kernel changes.
- New material issue: products BF16 logits error vs FP32 Reference: B0 max_abs 25.1209, rel_L2 0.0253783; B1 max_abs 11.5483, rel_L2 0.0200900. New FP32 B0 max_abs 0.000532, rel_L2 9.429e-7. Old FP32 TFS remains above old max_abs 0.003 gate. BF16 task acceptance remains unconfigured; these BF16 speed measurements are diagnostic only. Cause of large BF16 numeric deviation not isolated in this experiment; next precision task is quantization/attention sensitivity control with an actual checkpoint when available.
- Report docs/HISTORY_COMPARISON_20260930.md. Historical speed claims reconciled; diagnosis complete, no paper/task acceptance and no new Ours implementation. All three handoff records synchronized.

## sample-build-20260930-183053
- Build PASS: independent tile-granularity diagnostic copy, original B1 unchanged. Separate sampled/unsampled compile-time specializations; no per-vector clocks. Coarse phases combine tile loads and AMX compute. Added empty-clock calibration, original/no-clocks controls, bit-identical full-model check, periods 128/512 with varying seeds. Compiler icpx 2024.1.0, MKL 2023.0 Update2. Source/binary/rules hashes saved in run. Next: same-node real arxiv/products test; no new issue yet.

## sample-10809713 - 2026-09-30
- COMPLETED 0:0, qhcn059 shared intel, 1 node, 16 physical cores44-59, NUMA5/6/7 explicit interleave, elapsed00:05:45, MaxRSS20552972KB. Real official arxiv/products, seed11 untrained model, 1warm3reps alternating order. Boundary smoke and all full-model outputs bit-identical to original B1. Production kernels unchanged.
- Original B1 E2E median arxiv433.195ms/products15959.170ms. No-clocks diagnostic copy changes -1.13%/+1.02%; sampling1/128 +1.46%/+1.27%, sampling1/512 +0.38%/+0.98%. Low global perturbation; sampled stage shares remain worker-time estimates, not layer wall-time decomposition.
- Layer2 raw worker shares1/512: arxiv prefetch/setup19.63%,score/exp/den12.41%,gather/weight/RNE/store27.72%,tileload+compute36.62%,output3.67%; products23.03/12.84/34.20/28.97/0.87%. 1/128 broadly agrees. Repeated per-edge projection and dynamic feature staging are primary research targets. No individual AMX compute/load or gather/conversion percentages claimed.
- Independent perf cycles:u199Hz on unmodified products B1:210K samples,0 lost; tfs_aggregate93.82%,OpenMPwait2.30%. Initial perf annotate unsupported --percent-limit recorded; reprocessed same data with compatible flags, launcher fixed. Old objdump misdecodes BF16/AMX, so instruction attribution remains unavailable. Post-run launcher edit differs from original source snapshot; diagnostic C++ unchanged.
- BF16 products precision problem remains open (previous B1 max_abs11.5483/relL2~0.02009). Bit identity here only validates diagnostic copy. Current B1 stable-max prescan has no Online accumulator rescale, which this run does not measure. No new B0 speedup measurement, no new Ours, no formal acceptance. Report TFS_SAMPLED_DIAGNOSIS_20260930.md; raw evidence runs/sample-10809713 and sample-build-20260930-183053. Next independent method: local-U aggregation then once-per-destination UW, preserving original B1; must validate real-graph precision and benchmark fully.
