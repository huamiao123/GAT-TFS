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

## local-build-20260930-193506
- Build PASS: independent ours/local_online candidate, production B0/B1 unchanged. Degree-sorted destination TR16 tiles and dynamic panels64; neighbor block32 FP32 Online Softmax, worker-local U[D], normalize after once-per-tile UW. FP32 UW SGEMM single-thread per worker; optional AMX UW uses all four high/low BF16 products for U/W residual retention. FP32 L/R via contracted master W/a and FP32 H; no full Z/U/score/alpha. Builds separate build/local_online binary. Current user explicitly requested continuation into candidate implementation; no formal baseline/task acceptance assumed.
- Compiler icpx2024.1.0, MKL2023.0 Update2; sources, binary, rules hashes and AMX assembly in run. Next: independent FP32-first smoke then AMX, real arxiv/products matched B0/B1 comparisons, separate sampled stages/rescale counters. No runtime correctness or speed result yet.

## local-10809920 - FAILED correctness gate
- Shared qhcn059,1node16cores,64GB; baseline boundary tests passed, new local smoke failed against R0 before fixture/full real-graph benchmarks. No arxiv/products speed results. Initial driver omitted failing shape; root cause pending. Added mismatch dimensions/backend/block/error logging. Production baselines unchanged. Must rebuild and pass FP32/AMX smoke before benchmarking.

## local-build-20260930-193823
- Build PASS, diagnostic-only driver edit prints exact failing smoke backend/shape/block/errors. Kernel identical to initial local build. Prior local-10809920 failed before real graphs. Next rerun correctness gate then real graphs only if passed.

## local-10809928 - FAILED strict relative smoke
- Failed before real graphs: approximate AMX backend D256,K8,d7,block16, max_abs2.50991e-6,relL2=1.03686e-4 on trig weights with near-cancelling outputs. All FP32 shapes/blocks passed. No kernel edit. Root issue: relative-only AMX smoke gate on near-zero outputs. New gate retains FP32 abs/relative1e-4 limits; AMX additionally accepts max_abs<=1e-5 and directly checks AMX versus same FP32 local backend max_abs<=1e-5. All real-graph errors must still be reported; this is not a checkpoint/task precision threshold. Debugging failure retained.

## local-build-20260930-194217
- Build PASS: same candidate kernel; smoke gate now compares AMX directly to the FP32 local backend and declares near-zero absolute-error fallback. Binary/source hashes and AMX assembly recorded. Execution interrupted by local command-tool failure before submitting next job; no new runtime result. SSH restored on 2026-09-30 21:00 CST. Next: same-node smoke and matched real-graph benchmark.

## local-10810186 - 2026-09-30
- COMPLETED0:0, qhcn059 shared intel,1node16physicalcores44-59,NUMA5/6/7 interleave,elapsed00:04:09,MaxRSS42318076KB. Compiler icpx2024.1.0/MKL2023.0Update2. Build local-build-20260930-194217. Official arxiv/products,seed11 untrained,preallocated forward boundary; initial correctness comparison plus1warm3reps alternating order. Baseline boundary regression and local FP32-first/AMX smoke finished; near-zero AMX absolute fallback explicitly logged. Production B0/B1 unchanged.
- Median full3layer ms: arxiv B0FP3273.996/B0BF1668.437/B1425.709/localFP32250.887/localAMX270.005; products3908.668/3817.678/15807.121/8221.782/8534.037. LocalFP32 improves B1 by1.697x/1.923x, still3.39x/2.10x slower than pureGAT FP32. Mixed precision/control differences disclosed. No positive speedup against B0. AMX backend slower than local FP32.
- Products local FP32 final max_abs0.006919,mean_abs9.833e-6,relL2=1.494e-5; AMX0.009336/2.029e-5/2.002e-5. Layer2 max_abs0.041679/0.048580. Above old0.003 absolute gate; real-graph correctness/task acceptance remains UNVERIFIED, errors not hidden by finite checks. Arxiv FP32 final rel3.675e-7,AMX5.226e-6.
- Products layer2 local FP32 kernel4799.471ms/layer4840.405ms. Sample worker shares roughlySpMM79.87%,score/exp13.97%,UW3.90%,rescale-check/scaling1.17%. Sampled worker time not wall decomposition; clocks include no-op packing/rescale checks, floor not removed. Profile+counters separate from medians; FP32layer2 profile change+1.57% products/+7.79% arxiv, cannot claim zero perturbation.
- Exact FP32layer2 Online counters: arxiv blocks1580688,rescales48654(3.078%);products42882792/4476949(10.440%),rescaled_features1146098944. U is FP32 local memory; no live AMX tile during sparse scan, so rescale spill/reload0. AMX accelerates UW only, not sparse PH. FP32 sampled/counter output bit-identical; AMX64bit fingerprints match.
- Report docs/LOCAL_ONLINE_RESULTS_20260930.md, raw runs/local-10810186; failed runs10809920/10809928 retained. No paper/task acceptance. Next: shared-reference-input/matched-attention precision controls, then independent-head sourceH read reuse optimization. Checkpoint/accuracy gate still unavailable. Original execution failure during sandbox/tool refresh is resolved; SSH restored.


## 2026-10-01 current cycle: independent-head H reuse

Completed SSH recovery and joint FP32 source build (joint-build-20261001-150558, status 0). Implemented head groups 1/2/4/8, shared source-vector load with independent attention/state, fused rescale, degree-sorted TR16 and immediate per-head local UW. Added sampled fixed-attention FP64 associativity oracle and attention-only shift controls. No AMX PH claimed; old B0/B1 sources unchanged. Next: one shared intel qhcn059 job with 16 physical cores, NUMA interleave, smoke before real arxiv/products timing, 1 warmup/3 reps; full model own outputs, profile separately. Correctness/task/paper acceptance remains UNVERIFIED.


## 2026-10-01 joint cycle measured

Completed job10849337, reconciled successful Slurm accounting and full logs. Current best FP32 grouped-head E2E: arxiv G8 142.56ms (1.740x vs oldlocal), products G2 4499.23ms (1.774x vs oldlocal); compared with B0_FP32 still 1.926x/1.178x slower. Real fixed-LR output bit identical to oldlocal; master absolute gate remains failed. Next: independent full-group specialization to remove active-head branches without changing attention/state/math, compare v1/v2 in same shared-node job with B0 controls. No AMX PH yet; future PH precision/packing/tails remain research work. Original B0/B1 kernels unchanged.


## 2026-10-01 Full-group controlled variant built

Completed joint-full-build-20261001-152302 (status0). Ready for same-binary v1/v2 benchmark with B0FP32/BF16 controls on qhcn059. Smoke will cover 1/2/3/4/5/7/8 heads, blocks16/32/64 and groups1/2/4/8, including partial head groups. Then arxiv/products 3-layer own-output timing, 1 warmup/3 alternating reps, per-path separate sampled profiles and output fingerprints. Gate/task/paper remain UNVERIFIED.


## 2026-10-01 cycle reconciliation

Both real-graph jobs10849337 and10849393 completed with0:0;576+1008 smoke configurations pass and real fixed-input/fixed-LR implementation comparisons bit equal. Source H reuse is confirmed in assembly; Full guard removal adds only about1-6%,not a solution to the whole bottleneck. Latest fastest exploratory candidate:arxiv FullG8135.469ms vs B0FP3274.242ms;products FullG24407.019ms vs B0FP323838.167ms. Against old local job10849337 these are approximately1.83x/1.81x faster,but cross-job ratios are descriptive,not the clean within-job guard ablation. No candidate beats strong B0; original master absolute gate and trained-task acceptance remain open. Completed broad primary-source audit including full Illinois CPU GAT thesis (TF z aggregation,no post-UW); don't claim first head-loop/attention fusion. Next research should target source-access/wide PH and precision at high-degree rows before AMX PH; packing/tails/component costs need matched precision controls. Current cycle code/results/research and all handoffs will be synchronized to GitHub with no baseline kernel changes.
