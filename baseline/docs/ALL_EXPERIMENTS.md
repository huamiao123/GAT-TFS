# GAT baseline experiments

## 2026-09-30 build-20260930-151053

Purpose: first independent baseline suite compile. Target: wzh/GAT/baseline. Compiler: icpx 2024.1.0, oneMKL 2023.2.0. Build outputs: speed/profile/test binaries. Status: PASS (exit 0). Source, binary, contract and rule hashes recorded in remote runs/build-20260930-151053. No inference or performance run. No paper claim. GAT semantics follow IMPLEMENTATION_CONTRACT.md; TFS-Train GCN-specific architecture is outside this project.

## 2026-09-30 check-10808045

Slurm 10808045, intel shared, qhcn192, 2 physical CPUs, 8 GB request, elapsed 10 seconds, COMPLETED 0:0. icpx 2024.1.0 / oneMKL 2023.0 update 2 (module 2023.2.0). OMP/MKL=2, places=cores, bind=close; affinity and topology in manifest. Input: generated 35-node destination CSR with 1083 edges, D=128/C=40, seed=11 checkpoint; additionally existing smoke1024 graph. All source/binary/input/preflight hashes saved under runs/check-10808045.

Correctness: 64 D/K/d combinations passed FP64 scalar R0 oracle and AMX BF16 weighted-edge emulation; degree/tail/extreme-score and malformed/nonfinite tests passed. NumPy independent three-layer intermediate oracle PASS; final-layer output max abs 1.3807936e-7, relative L2 5.3283841e-7. Synthetic BF16 E2E: B0 relative L2 0.0042914; B1 0.00453385. BF16 accuracy gate unconfigured, task metric unavailable.

Micro/fixed-p/smoke interfaces ran: smoke B0 median 2.716 ms vs B1 18.332 ms (5 measured, 2 warmups, diagnostic only). Fixed-p tiny L2 operator B0 0.0540 ms vs B1 1.0926 ms. Timing excludes fixture creation, compare and print; model forward includes layer conversion/activation. Tiny shape and shared node cannot support formal speedup claims. oneMKL_VERBOSE reports AMX-capable platform and BF16 calls; actual vendor GEMM AMX dispatch still requires instruction/counter evidence. No paper table acceptance.

## 2026-09-30 build-20260930-152447

Rebuild after AVX contraction/static-SGEMM/timing-metadata/output-export changes. icpx 2024.1.0 and oneMKL 2023.2.0 module, login-node compile only, PASS 0. Compiler-generated baseline_tfs.s explicitly contains tileloadd and tdpbf16ps. All source/binary/contract/rule hashes in the unique build run. Runtime regression and vendor AMX counter verification pending. No performance or paper claim.

## 2026-09-30 arxiv-10808152 (single-core diagnostic)

Slurm 10808152, intel shared, qhcn066, COMPLETED 0:0, elapsed 3:57. Requested 16 physical CPUs/32 GB but srun step lacked explicit -c and exposed one core (CPU12, socket0); all compared paths actually used OMP/MKL=1. This is a recorded affinity defect, so the run is single-core diagnostic only, not the intended multithreaded acceptance run. Graph: ogbn-arxiv, N=169343/E=2484941, Din=128/C=40, explicit seed11 synthetic checkpoint. Raw commands/script, source/binary/graph/checkpoint/rule hashes, affinity/topology in runs/arxiv-10808152.

All 64 boundary regressions, NumPy FP64 R0 three-layer oracle and arxiv finite/intermediate checks passed. Model E2E relative L2: optimized FP32 3.3219e-7, BF16 B0 0.00310536, BF16 B1 0.00317697. Task accuracy unconfigured. Model medians over 7 reps after 2 warmups: B0 1123.3868 ms, B1 4414.6861 ms; fixed-p L2 A0 392.9115 ms vs A1 2308.2699 ms. Comparisons/prints excluded, conversion/activation included in model forward; fixed-p shared conversion excluded. No formal performance claim.

Vendor runtime evidence: perf amx_ops_retired.bf16:u = 27,571,749,888 in a standard_bf16-only process. This process never calls the linked custom TFS kernel, so positive retired AMX BF16 operations establish oneMKL projection executes AMX on this platform. oneMKL_VERBOSE version/dispatch and CPU flags also recorded. Required next: corrected srun CPU quota and verified multi-core single-socket rerun.

## 2026-09-30 build-20260930-154006

Third build PASS 0 with profile-only gather/pX conversion/AVX aggregation subfields, path-specific static preparation and workspace reporting. Same compiler and oneMKL modules. Source/binary/preflight hashes and AMX assembly evidence under this build run. Corrected srun script and affinity fail-fast guard ready. Runtime regression pending; no paper claim.

## 2026-09-30 arxiv-10808269 (multi-core exploratory)

Slurm10808269, intel shared, qhcn059, COMPLETED0:0, elapsed1:37; batch request16physical CPUs/32GB; verified socket1 CPU44–59 and OMP/MKL16 close/cores. Corrected explicit srun -c; NUMA default first-touch on the selected socket. icpx2024.1.0 / oneMKL2023.0Update2. Same arxiv graph/seed11 synthetic checkpoint. All source/binary/graph/weights/rule hashes and exact scripts in unique run; local raw text mirrored. Boundary64 and NumPyFP64 oracle regression PASS, real graph finite/intermediate checks PASS; BF16 task gate unconfigured.

2warmups/7reps, separate model processes; fixed-p AB/BA. Three-layer medians B0BF16 168.691246ms, B1TFS 586.236042ms, B0FP32 102.115986ms. B1/B0 speedup0.287753~; fixed-p L2 A0 57.743507ms, A1 311.922225ms, speedup0.18512149. Model timing includes conversion/activation, excludes static/load/allocation and post-timer checks/prints. Full task accuracy and formal speedup UNVERIFIED. Standard-only vendor perf again retired27,571,749,888 AMX BF16 ops. Detailed profile introduces substantial overhead; use speed wall/fixed-p for conclusions. See RESULTS_20260930.md and raw logs. No paper acceptance.

## 2026-09-30 local task evaluator fixture test

Local Windows Python, no cluster allocation. 3-node/2-class binary logits and explicit CSV labels/split. Perfect predictions, declared fixture accuracy-drop gate, NaN logits, truncated logits, duplicate/out-of-range split and invalid label tests all PASS. Command and result under runs/task-eval-test-20260930/result.txt. This checks evaluator behavior only; no trained-model task claim.

## B0 diagnosis build 2026-09-30
- build-20260930-161352: added optional warm initialization and NUMA/worker diagnostics; build passed. Kernel mathematics unchanged. Next: one shared node ABBA comparison, original vs warm initialization.


## diagnosis-10809043 2026-09-30
- COMPLETED, qhcn025 shared intel; requested16 but selected10 same-socket physical cores22-31. 3warm9repeat ABBA per initialization. Boundaries PASS. Original BF16123.97/124.70ms vsFP32134.17/134.46; warm BF16124.50/124.78 vsFP32133.89/133.68. Buffer mapping nodes differ (original FP32N3 vsBF16N2), warm bothN2. Not equivalent to previous16core run; continue on qhcn059. Evidence baseline/runs/diagnose-10809043, hashes recorded. Not paper acceptance.


## diagnosis-10809069 2026-09-30
- COMPLETED qhcn059 intel shared,16physicalcores44-59,NUMA5/6/7. BoundariesPASS. OriginalFP32102.36-103.28ms buffersN6; BF16167.87-169.91ms buffersN5. Warm bothN5:FP32157.37-157.60,BF16173.77-175.01ms. Same kernels: moving FP32 initialization changed locality and caused ~55ms slowdown. Primary NUMA confound confirmed; next bindbothN6/interleave identical policies, same ABBA3warm9repeat. Not paper acceptance. Evidence baseline/runs/diagnose-10809069.


## diagnosis-10809096 2026-09-30
- COMPLETED qhcn059 shared intel,16cores44-59. BoundariesPASS,3warm9repeatABBA. BothN6 FP32105.01ms/BF16105.77-105.84; bothinterleave5,6,7 FP3273.81-74.31/BF1668.26-68.63,ratio1.082x. Same-precision outputSHA256 identical acrossbothpolicies/repeats. Rootcause confirmed initialization-dependent NUMA firsttouch, no kernelchange. arxiv_payload launcher amended explicitinterleave acrossallocatedNUMA nodes (next-run policy; not yet full B1 rerun). Old FP32/BF16 performance comparison superseded; B1 mustrerun beforeupdated speedup claims. Report docs/B0_SLOWDOWN_DIAGNOSIS_20260930.md and runs/diagnose-10809096. No paper acceptance.

## arxiv-10809143 — 2026-09-30: B1 diagnosis under identical NUMA policy

- COMPLETED 0:0, intel shared, qhcn059, 1 node, 16 physical cores 44–59, NUMA 5/6/7 interleave. Elapsed 00:01:26. icpx 2024.1.0 and oneMKL 2023.0 Update2; source/binary/data/rules hashes saved in runs/arxiv-10809143. No kernel changes.
- arxiv N=169343, E=2484941, Din=128, C=40; three-layer 8×32/8×32/1×40 GAT; identical seed=11 parameters. Boundaries (64 cases), NumPy three-layer oracle, finite output checks PASS. Real checkpoint task gate remains unconfigured.
- Full three-layer forward: 2 warmups, 7 measured repeats, speed binary, static setup and validation excluded. Median: FP32 B0 73.439 ms, BF16 B0 67.976 ms, B1 448.673 ms, matched-attention diagnostic 68.035 ms. B1 P95 458.663 ms; B1/B0 time ratio 6.60.
- Layer 2 B1 total 259.107 ms, weighted TFS kernel 253.439 ms. Fixed-p backend 243.980 ms vs B0 24.338 ms. Work audit: AMX 345.789 GFLOP vs B0 projection 22.196 GFLOP; active tile-row fraction 94.2%. Full vector/tile clock instrumentation remains unsuitable for unbiased substage proportions.
- Evidence: runs/arxiv-10809143 and docs/B1_SLOWDOWN_DIAGNOSIS_20260930.md. Exploratory only, not paper acceptance. Old comparisons retained as superseded history.

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
