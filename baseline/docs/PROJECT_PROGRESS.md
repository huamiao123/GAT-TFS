# GAT baseline progress

2026-09-30: R0/B0/B1 independent source suite written and initially compiled. Fixed-p, micro, full-layer, full-model interfaces and boundary/oracle tests implemented. Pending: run boundary/oracle tests, resolve observed failures, inspect dispatch evidence, freeze frontend policies using representative microbenchmarks, obtain a real checkpoint/labels/split and calibrate task accuracy. No formal speedup acceptance. Next: small-graph compute-node correctness run using at most one shared node and two CPUs.

2026-09-30 after check-10808045: all boundary and independent NumPy R0 checks passed. All five paths and fixed-p/micro/profile interfaces execute. Added corrected AVX contraction layout, static skinny-SGEMM workspace, speed-build executed-padding metadata, output export and task-evaluation helper. Pending regression, representative frontend microbenchmarks, vendor AMX execution evidence and trained-checkpoint task gate. Next: rebuild revised code, rerun required checks and run exploratory arxiv tests; synthetic fixtures cannot establish task accuracy.

2026-09-30 build-20260930-152447 passed; compiler assembly contains the intended AMX instructions. Next: combined small-graph regression and arxiv exploratory validation/micro/fixed-p/model tests with single-socket affinity and perf evidence. No new method or formal performance acceptance.

2026-09-30 arxiv-10808152 completed; regression and real-graph finite checks passed. oneMKL AMX execution now supported by retired BF16 AMX counter in standard-only process. Actual step affinity was one core; correction written for srun and requires rerun. Added per-vector gather vs pX/RNE profile timing, AVX sparse profile breakdown, path-specific static preparation and actual workspace reporting. Next: compile these changes, run corrected multithreaded arxiv tests, synchronize report/evidence. Formal acceptance remains pending trained checkpoint/development accuracy and dedicated-node measurements.

2026-09-30 build-20260930-154006 PASS. Next: corrected multi-core single-socket arxiv rerun; all implementation changes are now compiled. Regression and acceptance caveats remain explicit.

2026-09-30 arxiv-10808269 COMPLETE0:0, actual16cores/socket1. R0 oracle and all boundary tests passed after latest implementation changes. All benchmark modes run; vendor AMX evidence verified; default frontend policies supported by arxiv micros (B0 AVX L/R, B1 BF16 LR on all three shapes). New B1 is slower than strong B0 both fixed-p and full model. RESULTS_20260930.md records model/stage/error/profile evidence and limitations. Implementation milestone done; research acceptance is not complete. Pending trained GAT checkpoint/labels/split, calibrated task gate, dedicated-node formal benchmarks and profiling with controlled perturbation. Next: synchronize source/results to GitHub; receive checkpoint location before real task-accuracy acceptance.

2026-09-30 local task-evaluator fixture tests PASS, including accuracy-drop failure and malformed/nonfinite inputs. Helper is ready for checkpoint-based acceptance; no real task threshold chosen.

## B0 diagnosis build 2026-09-30
- build-20260930-161352: added optional warm initialization and NUMA/worker diagnostics; build passed. Kernel mathematics unchanged. Next: one shared node ABBA comparison, original vs warm initialization.


## diagnosis-10809043 2026-09-30
- COMPLETED, qhcn025 shared intel; requested16 but selected10 same-socket physical cores22-31. 3warm9repeat ABBA per initialization. Boundaries PASS. Original BF16123.97/124.70ms vsFP32134.17/134.46; warm BF16124.50/124.78 vsFP32133.89/133.68. Buffer mapping nodes differ (original FP32N3 vsBF16N2), warm bothN2. Not equivalent to previous16core run; continue on qhcn059. Evidence baseline/runs/diagnose-10809043, hashes recorded. Not paper acceptance.


## diagnosis-10809069 2026-09-30
- COMPLETED qhcn059 intel shared,16physicalcores44-59,NUMA5/6/7. BoundariesPASS. OriginalFP32102.36-103.28ms buffersN6; BF16167.87-169.91ms buffersN5. Warm bothN5:FP32157.37-157.60,BF16173.77-175.01ms. Same kernels: moving FP32 initialization changed locality and caused ~55ms slowdown. Primary NUMA confound confirmed; next bindbothN6/interleave identical policies, same ABBA3warm9repeat. Not paper acceptance. Evidence baseline/runs/diagnose-10809069.


## diagnosis-10809096 2026-09-30
- COMPLETED qhcn059 shared intel,16cores44-59. BoundariesPASS,3warm9repeatABBA. BothN6 FP32105.01ms/BF16105.77-105.84; bothinterleave5,6,7 FP3273.81-74.31/BF1668.26-68.63,ratio1.082x. Same-precision outputSHA256 identical acrossbothpolicies/repeats. Rootcause confirmed initialization-dependent NUMA firsttouch, no kernelchange. arxiv_payload launcher amended explicitinterleave acrossallocatedNUMA nodes (next-run policy; not yet full B1 rerun). Old FP32/BF16 performance comparison superseded; B1 mustrerun beforeupdated speedup claims. Report docs/B0_SLOWDOWN_DIAGNOSIS_20260930.md and runs/diagnose-10809096. No paper acceptance.

## 2026-09-30 — current status after arxiv-10809143

- Full B1 rerun with identical explicit interleave completed. Boundaries and independent NumPy oracle pass; no kernel changes. BF16 B0 67.976 ms, FP32 B0 73.439 ms, B1 448.673 ms. Detailed report: B1_SLOWDOWN_DIAGNOSIS_20260930.md.
- Main B1 cost is the weighted neighbor-step TFS backend; Layer 2 253.439 ms kernel / 259.107 ms total. Repeated edge projection creates 15.58 times the projection FLOPs of B0. Degree sorting is present; row padding is a smaller component.
- This cycle's B0/B1 slowdown diagnosis is complete. No formal task/performance acceptance, no new Ours implementation. Pending work: low-overhead attribution if needed, then independently evaluate an alternative destination/local-tile aggregation→UW method without altering B1 or head semantics; real trained checkpoint and calibrated accuracy gate remain pending.

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
