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


## 2026-10-01 joint-build-20261001-150558 / SSH recovery

SSH to hdacp1@176.0.250.88 succeeded (login03). Added independent FP32 grouped-head Online aggregate-first sources joint_online.*, joint_checks.*, joint_main.cpp; original B0/B1/local kernels unchanged. Build with icpx 2024.1, -O3 -fp-model precise, AVX512/OpenMP/MKL succeeded, status=0. No kernels run on login node. Exact commands, current rules/skill/contract hashes, source/binary hashes, compiler, queue and assembly in runs/joint-build-20261001-150558. Three-layer model unchanged; blocks 16/32/64, head groups 1/2/4/8; independent state, local U, fused row rescale. Current queue empty; qhcn059 intel has 32 free CPUs. No timing or correctness acceptance from this build. Paper: UNVERIFIED.


## 2026-10-01 joint-10849337

Status COMPLETED 0:0, 5m45s, qhcn059 shared intel; one node/16 physical cores 32-47, NUMA4/5 explicit interleave, OMP close/cores, MKL dynamic false. Build joint-build-20261001-150558/icpx2024.1 precise; source/binary/data/rules hashes and frozen source_snapshot in runs/joint-10849337. Official arxiv N169343 E2484941 Din128 C40 and products N2449029 E126167053 Din100 C47, same 8x32/8x32/1xC, seed11 untrained. Steady-state preallocated three-layer own-output forward; 1 warmup/3 alternating repeats, comparison/copies/profile outside timer. Boundaries and 576 joint smoke configurations pass; all 576 bit equal to old local and counts identical. On real graphs all groups fixed-LR output bit equal to old local; profile fingerprints equal. E2E median ms: arxiv B0_FP32 74.015147, B0_BF16 67.599089, oldB1 432.552320, oldlocal 248.122118, G1 215.848719/G2 154.656529/G4 152.238040/G8 142.562127; products B0_FP32 3820.758131, B0_BF16 3723.108644, oldB1 15698.649803, oldlocal 7982.355176, G1 6380.318251/G2 4499.233311/G4 4940.558192/G8 4661.920985. Worst products layer2 fixed-LR TF-vs-AF abs .022377 while attention-only shift .008362, so not solely attention reassociation. Full-model G8 error same oldlocal, layer3 abs .0069189 rel1.49427e-5. Original abs .003 gate NOT passed. Sampled FP64 same-attention associativity ~1e-14; sampled oracle excludes costly high-degree rows, not full coverage. Paper/task/master acceptance UNVERIFIED, performance EXPLORATORY. No AMX PH in joint candidate. Raw logs, configs, hashes, accounting and RESULTS.md/summary.tsv are evidence.


## 2026-10-01 joint-full-build-20261001-152302

Compile-only experiment on login03: status0, icpx2024.1 -O3 -fp-model precise and identical ISA/OpenMP/MKL flags; built build/joint_compare, v1 and Full-specialized v2 linked together, original baseline sources unchanged. Source, binary, rules/skill/contract hashes, command, compiler, queue and assembly in runs/joint-full-build-20261001-152302. Only Full template/dispatch removes active-head guards for divisible groups; partial heads retain old guards. Workspace, math, order, SGEMM, online and output remain identical. No compute run or correctness/timing claim from this build.


## 2026-10-01 joint-full-10849393

Status COMPLETED0:0,4m12s,qhcn059 shared intel,one node/16physical cores32-47 NUMA4/5 interleave,OMP close/cores,MKL dynamic false. Build joint-full-build-20261001-152302/icpx2024.1 precise; v1/full compiled in same binary. Same official graphs/features/model seed11 untrained,3 layers independent 8x32/8x32/1xC,workspace preallocated,static preparation excluded. Smoke1008 shape/group/tail/block configurations pass,1008 bit equal; real same-input all groups bit identical; profile fingerprints match. One warmup,3 alternating measured reps,compare/copies/logging/profile outside speed timer. E2E median ms arxiv:B0FP3274.242024/BF1667.921739,v1G2157.733656/G8139.486002,FullG2148.885585/G4145.184511/G8135.469152; products:B0FP323838.167375/BF163736.940441,v1G24515.482826/G84645.452236,FullG24407.019438/G44742.111276/G84592.145837. G8 Full guard removal speedup1.02965x arxiv/1.01161x products; G2 1.05943x/1.02461x. Not the major bottleneck; all full candidates still slower than B0. Original master/task gate remains UNVERIFIED; output error unchanged. MaxRSS step33,567,768K. Config,rules/source/binary/data hashes,frozen source_snapshot,raw logs,accounting,summary/RESULTS evidence in runs/joint-full-10849393. Formal/paper acceptance UNVERIFIED.


## Local reconciliation: icpp-online-method-correction

Reconciled 2026-10-02T03:03:57.732411+00:00; event time is in preserved run evidence.

Purpose: extend actual original ICPP TFS neighbor-step AMX output accumulation with block Online Softmax. The previous FP32 local-U/joint candidates are separate prototypes, not an optimized original TFS kernel. User instructed continuation of actual ICPP TFS extension.

New files only: baseline/tfs_online/icpp_online.*, checks.*, main.cpp and four baseline/scripts/*icpp_online* files. Frozen src/baseline_tfs.cpp, baseline_standard.cpp, ref_fp32.cpp and common.cpp unchanged. Model: independent heads 8x32/8x32/1xC, same CSR/features/self-loops/ELU and random master seed11. Output state V=UW is rescaled in head output dimension d; pH is BF16 and uses original neighbor-step TDPBF16PS. No global Z, e, alpha or U. Degree Sort, TR16, synchronization and packed W retained.

Operational preflight: current server AGENTS.md and tfs-research-engineering/SKILL.md read in full; SHA256 55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7 and 641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca. GAT IMPLEMENTATION_CONTRACT.md hash c0521a5ca650bba596f9b04c219bb1e6e1796e85c578449cef0f4b67c0fe17c7. Previous joint-full-10849393 appears in all three GAT handoffs. Current contract frozen premax B0/B1 unchanged; new Online path is separate, explicitly requested by user.

SSH login03 reachable, conda/User startup warnings unchanged. Current account has other depfacto jobs; untouched. qhcn059 shared intel has 48 free of 64 physical CPUs and sufficient memory. Plan one additional shared intel node, 16 cores/64G/25min, keeping total <=5 nodes. Correctness precedes timing in job; login compile only. Full master .003 gate unchanged and task accuracy UNVERIFIED. Tests include independent FP32 V simulator, BF64 stable complete-neighborhood oracle, BF16 instruction emulator, exact counters, all-equal original-B1 control and profile-off/on bit identity.

All server writes confined to /home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline: new tfs_online/, build/, runs/icpp-online-* and three docs handoffs. Build/tests/benchmark not yet executed. No timing or correctness acceptance. Static agents reviewed new kernel and test logic; actual hardware checks pending. No paper claim.


## Local reconciliation: icpp-online-build-20261001-215442

Reconciled 2026-10-02T03:03:57.732411+00:00; event time is in preserved run evidence.

Compile-only experiment icpp-online-build-20261001-215442 on login03 completed status0. icpx2024.1.0, -O3 -fp-model precise, C++17, AVX512/BF16/AMX/OpenMP/parallelMKL flags identical to frozen baselines. New build/icpp_online links unchanged B0FP32/BF16/B1 and independent new tfs_online path. No numeric kernels run on login.

Evidence: runs/icpp-online-build-20261001-215442/{build.log,assembly_build.log,icpp_online.s,tile_instructions.txt,command.txt,compiler.txt,source.sha256,binary.sha256,preflight.sha256,queue.txt,partition.txt,status.txt,source_snapshot/}. Source/binary and current rules/skill/contract hashes preserved. New files compile without diagnostics. Local src/include Git diff empty and remote frozen SHA256 recorded.

No new major build issue. Hardware correctness, matched original-B1 controls, master .003 and performance not yet tested. Next: one shared intel qhcn059 job with16physicalcores64G; smoke (FP32/FP64/emulator/all-equal B1/counter/profile identity) before arxiv/products3layer timing. One warmup/3alternating repeats; sampled profiling/counters separate. Untrained seed11; all reported performance exploratory, no task/paper acceptance. Original TFS weighted BF16 neighbor-step AMX retained in source; emitted assembly available for audit.


## Local reconciliation: icpp-online-failed-10851291

Reconciled 2026-10-02T03:03:57.732411+00:00; event time is in preserved run evidence.

Job10851291 FAILED1:0 after4seconds onqhcn059 sharedintel,1node16cores64G. Build icpp-online-build-20261001-215442 binary62c90e12cbf8bb2e1a3ac2ba9ba9949263f99afd8d0ddae9c4875a779611a301. Source/rules/skill/contract/artifact hashes,snapshot,Slurm config,manifest,pinning andsmoke.log inruns/icpp-online-10851291. No arxiv/products timing executed.

All-equal scores originalB1 isolation passed block16/32/64: identicalquantizedH/W,FP32LRzero,unnormalizednumerator,denominatorandnormalizedoutputbitwiseequal;rescales/spill/reloadzero. FP32VOnlinevsFP64stable N17D17K1d7B16 passed maxabs8.81e-8 relL2=1.07e-6. AMXvsmatchedSVMLBF16instructionemulatorpassed maxabs5.59e-9, maxerror/budget.00593; smallmasterabs.0002293 also passed.

Smoke stopped at denominator abs1e-5 ad-hoc simulator comparison: observedmaxabs1.52587890625e-5, meanabs1.963447e-6,relL2=5.27253e-8. This denominator-scale-independent threshold is under diagnosis; actual error is tiny relative to denominator but not to be silently suppressed. Investigate FMA/reassociation and SIMDexp arithmetic before choosing an operand/operation-scale numeric budget. Original master output .003 and FP32simulator1e-5 gates unchanged; no correctness/performance acceptance from this failedjob.

Issue open: emulator/kernel denominator operation-order difference or threshold scaling. New checks-only diagnostic/fix pending, preservefailedrun. No changes frozenB0/B1. Next freshcompile anduniqueSlurmrun onlyafter documenting cause. Current jobfinished,otheraccountjobs untouched. This failure is retained in allthreehandoffs; no paper/table claim.


## Local reconciliation: icpp-online-build-20261001-220654

Reconciled 2026-10-02T03:03:57.732411+00:00; event time is in preserved run evidence.

Compile experiment icpp-online-build-20261001-220654 completed status0 on login03, icpx2024.1 precise, identical flags. Kernel and frozen B0/B1 unchanged; only checks.cpp diagnostic and independent SIMD denominator replay added. No numerical kernels run on login. Build preserves commands, compiler, source/binary/rules/skill/contract hashes, source snapshot, core and checks assembly.

Failed smoke10851291 remains visible. Its absolute denominator difference 2^-16 corresponds to one FP32 ULP near [128,256); the particular scalar/vector rounding cause remains unconfirmed. New checks print each differing row/head and force scalar add/multiply rounding separately. An independent TR16 SIMD denominator replay uses the same masked operations and mixed-lane SVML invocation as the kernel and requires exact bitwise equality. Scalar diagnostic comparison uses a positive FP64 shadow and 2*gamma(degree+rescales) operand/operation-based arithmetic budget plus unchanged relativeL2<=1e-5. This budget is not inferred from observed BF16/master errors. Original FP32 output abs/relative1e-5 and master output abs.003 gates remain unchanged.

Static assembly review confirms the speed specialization has no hot-loop chrono or SGEMM. It retains neighbor-step AMX and only changed-max branches perform output tile staging/rescale/reload. This proves the implementation architecture, not hardware correctness or speed. Runtime acceptance pending. Next one shared intel job, 16 physical cores, smoke then arxiv/products full3layer forward; shared-node/untrained performance exploratory only. No new build issue; numerical diagnosis awaits new evidence.


## Local reconciliation: icpp-online-failed-10851339

Reconciled 2026-10-02T03:03:57.732411+00:00; event time is in preserved run evidence.

Job 10851339 FAILED 1:0 after 3 seconds on qhcn059 shared intel, one node/16 physical cores/64 GiB. Build icpp-online-build-20261001-220654. All-equal original-B1 controls passed. The independent TR16 SIMD denominator replay failed its exact comparison in the first ordinary case with the same 1.52587890625e-5 maximum difference as the scalar simulator. No real-graph benchmark ran. Full raw failure, source snapshot, manifest and hashes remain in runs/icpp-online-10851339.

Root cause evidence from two independent assembly audits: kernel exp calls use __svml_expf16_z0 (core assembly lines 3449/3473 and all specializations). Both scalar simulator and TR16 replay use __svml_expf16 (checks assembly lines 4448/4540 and 13397/13549). Core has no vfmadd/vfmsub/vfnmadd instructions. Thus the purported matched-exp oracle did not actually match the compiled kernel's SVML entry. Source-level identical intrinsic does not guarantee identical exp binary entry or rounding. The replay's exact gate correctly exposed this test harness mismatch. Detailed ULP row evidence is in smoke.log.

Resolution in progress: checks-only wrapper calls the kernel-selected __svml_expf16_z0 entry explicitly, confined to this fixed oneAPI diagnostic harness; standard-exp independent controls remain. Add plain-versus-selected SVML numerical diagnostic. Keep exact SIMD replay gate, FP32 output abs/relative 1e-5, original master .003 and all counter/profile gates. Do not relax output tolerances or modify kernel arithmetic. The scalar arithmetic budget remains an auxiliary check, not a substitute for exact replay. Unknown internal library suffix semantics are not claimed.

Next: fresh compile with same compiler/flags, then unique shared-node smoke and real graphs only after passes. No timing/correctness acceptance, task and paper UNVERIFIED. Frozen B0/B1 unchanged, no other jobs touched. Preserve this failure in all three handoffs.


## Local reconciliation: icpp-online-build-20261001-221450

Reconciled 2026-10-02T03:03:57.732411+00:00; event time is in preserved run evidence.

Compile-only experiment icpp-online-build-20261001-221450 completed status0 on login03 with identical icpx2024.1 precise flags. Kernel/frozen baselines unchanged. Checks explicitly bind matched SIMD exp to the kernel-emitted __svml_expf16_z0 entry, only in the fixed oneAPI harness. Plain __svml_expf16 is retained for a 1024-value [-10,0] ULP diagnostic and std::exp remains an independent oracle. Exact TR16 denominator replay, FP32 1e-5 gates and original master .003 remain in force.

Evidence: unique build directory contains commands, compiler, source/binary/rules/skill/contract hashes, source snapshot, core and checks assembly, logs and status. No numerical work on login. No new build issue. Failed jobs10851291/10851339 preserved and documented. Exp numerical difference and full smoke are still pending actual compute-node validation; no accuracy or speed claim.

Next: unique one-node shared intel/16-core/64 GiB run with smoke before arxiv/products full3layer forward. Original degree-sorted TR16 neighbor-step AMX execution retained; performance sampled separately from speed repetitions. Current account has one unrelated pending depfacto job; untouched. Task/paper acceptance remains UNVERIFIED.


## Local reconciliation: icpp-online-complete-10851370

Reconciled 2026-10-02T03:03:57.732411+00:00; event time is in preserved run evidence.

Experiment 10851370 completed 0:0, 4m52s, qhcn059 shared intel, one node/16 physical cores 16-31, NUMA2/3 explicit interleave, OMP close/cores, MKL dynamic false, 64 GiB requested. MaxRSS40682656KiB. Build icpp-online-build-20261001-221450, icpx2024.1 precise, binary SHA2567412a42ebb5439bd601044dc46659b1b2322d40721de77b59e01c16b898aa708. Exact manifests, hashes, accounting and source snapshot in runs/icpp-online-10851370. Frozen src/include unchanged.

Actual TFS preserved: Degree Sort, dynamic panels, TR16 synchronized neighbors, full H gather, independent BF16(pH)W AMX and resident output TMM. New block Online m/l/V rescale dimension is d32 in L2. No full Z/e/alpha/U or separate SGEMM. Official arxiv/products data and independent 8x32/8x32/1xC model, seed11 untrained. B1/new LR policy fp32. Preallocated own-output3layer timing,1warmup/3alternating reps, profile/counters outside speed timing.

48 operator cases plus equal-score oldB1 isolation at3blocks and2 real-attention wrappers pass. SIMD denominator replay bitwise; clocks/counters do not change arithmetic. Earlier jobs10851291/10851339 failed because checks used a different SVML exp entry; actual node diagnostic found739/1024 exp outputs differ, up to4ULP. Explicit checks-only matching fixes exact replay; production kernel and master .003 gate unchanged. Failed runs retained.

Three-layer medians(ms): arxiv B0FP3275.691170/BF1669.442469/B1425.159327/newB16443.104402/B32441.301835/B64441.918665. Products B0FP323923.192752/BF163829.412280/B115865.410591/newB3213882.425689. NewB32 vs originalB1 .96342x arxiv/1.14284x products, still6.355x/3.625x slower thanB0BF16. Products L2 kernel9164.525->8323.746ms; prescan41.167->0ms. Improvement is not yet decomposed by single-factor ablation.

Products L2 exact counts: blocks42882792, max updates24072096, rescales4479864(10.4468%),rescaled elements143355648,staging events1203201,spill/reload each2464155648bytes,AMXcalls1011640704,FMA8287360647168. Repeated peredge projection remains: useful work42.887x standardTF, actual padded42.985x. Fine sampled worker timing has timer floor and is not wall percentages; measured profile kernel+11.32% relative to speed median.

Full-model master .003 gate FAIL: arxiv newfinal maxabs.003581/relL2.0031768; products newfinal maxabs6.019003/mean.0135794/relL2.0150918, L2maxabs25.611694. Products sameinput/LR vs oldB1 L2maxabs.478546/relL2.00015642. Selected FP64 includes maxdegree row but only8rows. No checkpoint/task accuracy acceptance. Performance EXPLORATORY; no formal paper claim.

Local report docs/ICPP_TFS_ONLINE_RESULTS_20261001.md and summary/checks/profiles/stats/accounting preserved. Independent audit recomputed30 groups270timing fields and12counter rows. Local completion entry prepared2026-10-02; server synchronization PENDING because current session network denied SSH. Server handoffs currently contain through matched-exp build, not this completion entry. Do not claim remote synchronized. Next local independent-head pair source reuse candidate will retain actual TFS and get exact original-online controls. No new benchmark until SSH/access and preflight resume.


## Local reconciliation: icpp-pair-local-20261002

Reconciled 2026-10-02T03:22:45.024743+00:00; event time is in preserved run evidence.

Event date: 2026-10-02. LOCAL SOURCE IMPLEMENTATION ONLY. No build, numerical execution, Slurm submission, measurement, server transfer or GitHub push took place for the pair candidate.

Source: baseline/tfs_online/icpp_pair.hpp/.cpp, pair_checks.hpp/.cpp, pair_main.cpp. New build, payload and Slurm scripts are under baseline/scripts/. The existing Online build now uses explicit source files so that the additional driver does not introduce a duplicate main. Frozen baseline src/include and the five previously measured Online source files remain unchanged against job10851370's source snapshot.

Actual original TFS retained: Degree Sort, destination-row CSR, dynamic panels, TR16 synchronized neighbor steps, independent BF16(pH)W AMX operations and resident output TMM. Even K and d<=32 pair two independent heads; all other shapes fully fall back to the existing Online kernel. L1/L2 use pairs, L3 uses fallback for the fixed 8x32/8x32/1xC model. Heads share CSR source indices and raw BF16 H loads/expansion only. W, L/R, scores, probabilities, m/l/V and rescale are independent; a maximum rise spills only the affected head's tiles. No full Z/e/alpha/U or separate final SGEMM.

Expected logical raw H read units for L2 decrease from 8 head visits to 4 pair visits per edge; products logical BF16 bytes 516.780GB -> 258.390GB. This is not measured DRAM traffic and does not imply 2x speedup. Independent weighted-H conversion, per-edge projection and AMX arithmetic work are unchanged; approximately42.887x useful arithmetic vs transform-first remains.

Two independent static kernel audits found no blocking issue with tile ownership, buffer bounds, inactive/empty/tail lanes, arithmetic order, fallback or statistics. New checks contain57 operator cases and2 layer wrappers, with strict original-Online bitwise numerator/den/max/output checks, row/head counts and logical-source counts, plus all4 profile/counters template combinations. CHECKS ARE WRITTEN, NOT EXECUTED. Compilation, actual SVML selection, AMX execution, numerical gates, counter identities and performance remain unverified. The FP32 master .003 gate is unchanged.

Speed path uses no internal clocks/counters. Separate sampled profiling measures complete feature-staging and AMX-load/compute spans, avoiding per-vector/per-TDP clock reads in the paired path. Worker sums are not wall percentages. Single-head fallback uses existing fine spans and is labeled accordingly. Proposed future experiment is one shared intel node/16 physical cores/80GiB, original smoke then pair controls, arxiv/products block32,1warmup/3alternating repeats with independent three-layer outputs. No resources currently consumed.

Current session restricted-network SSH attempt: ssh: connect to host176.0.250.88 port22: Permission denied. No credential prompt or successful remote shell; no evidence that the remote SSH service itself is down. No permission bypass attempted. Server synchronization of job10851370 completion and this local event is PENDING_NETWORK_ACCESS. Local three handoffs contain the preserved event; server handoffs currently contain through icpp-online-build-20261001-221450.

Local report: baseline/docs/ICPP_TFS_PAIR_STATUS_20261002.md. Local immutable evidence: baseline/runs/icpp-pair-local-20261002/manifest.json and source_snapshot/. Prior measured13.882s products result belongs to the single-head-loop ICPP Online job10851370, not to this candidate. Source work is complete locally, server verification pending. No speedup or full correctness acceptance claimed.


## Local reconciliation: icpp-research-resume-20261002

Reconciled 2026-10-02T05:14:38.973663+00:00; event time is in preserved run evidence.

2026-10-02 research resume and SSH environment probe. Current session network permission restored; BatchMode SSH succeeded to hdacp1@176.0.250.88. Login startup printed existing conda/User errors but returned a functioning shell; no startup files changed. Rule/skill/contract current SHA256 match the previous preflight (55ac34d...,641694f...,c0521a5...). Full AGENTS and skill read; GAT-specific user contract overrides GCN model scope while operational workspace, scheduler, evidence and handoff rules remain applied.

Target project /home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline. Account queue empty at probe; intel partition available. No compute allocation by this probe. Previously measured 10851370 completion and local pair source event are being synchronized to all three remote GAT handoffs before a new build. Historic event text describing denied network is retained as historical, current blocker resolved by environment permission change.

User requests investigating and attempting to solve D/d=8 wide sparse feature work and repeated neighbor projection while retaining original ICPP TFS. Existing repeated benchmark authorization persists; use one shared intel node16 physical cores only, no other jobs touched. Initial hypotheses: independent-head rawH sharing affects logical reads only; block-local U followed immediately by real AMX UW can reduce projection counts while retaining DegreeSort/TR16/tile fusion and output TMM; wide sparse arithmetic remains and requires separate analysis. No new speedup or full-master correctness acceptance yet.

All new server writes and TMPDIR remain under wzh. Prior local snapshots and failed/successful runs preserved. Next: verify frozen sources, transfer pair candidate, compile-only build then exact smoke before real-graph measurement; parallel primary-literature and mathematical audits assess stronger alternatives. Research and shared-node timings exploratory, precision/task gates unchanged.


## Local reconciliation: icpp-pair-build-20261002-124922

Reconciled 2026-10-02T05:14:38.973663+00:00; event time is in preserved run evidence.

Compile-only experiment icpp-pair-build-20261002-124922 completed status0 on login01 using icpx2024.1 and oneMKL2023.2, -O3 -fp-model precise AVX512/BF16/AMX/OpenMP flags. No numerical kernels executed on login. Binary cb15dbe4455fc1c3ca62bc9c92a21a631c406fb452b8dc708f01212e606637ed under wzh/GAT/baseline/build/icpp_pair.

Current AGENTS/skill were read fully immediately before build; hashes unchanged55ac34d.../641694f..., contractc0521a5.... Queue empty and intel available. Previous network-resume/10851370-completion/local-pair events synchronized to allthree remote handoffs. Frozen src/include and original icpp_online/checks matched prior source hashes. Unique run contains commands, compiler, rule/source/binary hashes, build/status, source snapshot and pair assembly.

Candidate retains DegreeSort/TR16/independent-head weighted AMX with resident outputTMM. Shared raw H read/expansion only. New tests57operator+2layer and exact oldOnline state/output/profile/counter controls are compiled, not yet executed. The compiler succeeded, which is not runtime or precision acceptance. No new build issue. Original FP32 master .003 gate unchanged. Next one sharedintel node16physicalcores80GiB: oldsmoke then pair exact controls then arxiv/products block32 three-layer own-output,1warmup3alternating repeats and separate coarse profile. Shared/untrained timings exploratory; no paper claim.


## Local reconciliation: icpp-pair-submitted-10853968

Reconciled 2026-10-02T05:14:38.973663+00:00; event time is in preserved run evidence.

2026-10-02 submitted Job10853968 for independent-head pair source-read reuse. One sharedintel node,16 physicalcores,80GiB,25min limit; oldOnline smoke then57pair operator+2layer controls then arxiv/products three-layer 8x32/8x32/1xC seed11 ownoutput; B0FP32/BF16/originalB1/originalOnline/pair samebinary. Buildicpp-pair-build-20261002-124922 binarycb15dbe4455fc1c3ca62bc9c92a21a631c406fb452b8dc708f01212e606637ed.

Mandatory preflight currentAGENTS/skill fullyread and rulehashes unchanged, GATcontractc0521a5..., priorbuild recorded allthreehandoffs, queue wasempty. All writes underwzh/GAT/baseline, runruns/icpp-pair-10853968 contains source/binary/data/rules snapshots, exactbinding/NUMA/commands/logs/status. Actualnode/state/CPU/repeats/correctness/performance pending; do not claim acceptance. No otherjobs touched. Sharednode timing EXPLORATORY, master.003/task gates unchanged.

Parallel research proposes next strongerblock-fusion extension (newfiles only), retainingDegreeSort/TR16/actualAMX/outputTMM but aggregating temporaryU_B and immediately projectingeachblock. This reduces repeatedprojection but doesnot eliminateKED sparseD-wide work. No newmajorissue at submission; arithmetic/precision/runtime pending.


## Local reconciliation: icpp-pair-failed-10853968

Reconciled 2026-10-02T05:14:38.973663+00:00; event time is in preserved run evidence.

Job10853968 FAILED1:0 after4seconds on qhcn006. No numerical kernel, smoke or benchmark ran. Slurm allocated fewer than16 physical cores within any one socket; payload intentionally refused with FAIL: need16physicalcores on one socket. Current guard remains unchanged, no silent cross-socket comparison. The failure is scheduler/affinity layout, not AMX/math. Raw stdout/stderr,status,preflight/source/data/binary manifest and Slurmconfig preserved in runs/icpp-pair-10853968.

Resolution: explicitly request block:block CPU distribution at both sbatch and srun, retain nomultithread and the same16physicalcore/socket check. Pinning.env now logs per-socket physical-core counts even if guard fails. Official Slurm sbatch distribution documentation states block distribution allocates consecutive CPUs from one socket before the next; actual mask still checked at runtime. Source/kernel and compiled binarycb15dbe... unchanged. No new precision gate or speed result. Retry gets uniqueJobID; no otherjob touched. Allthreehandoffs updated before retry. Next blockTFS kernel implementation in parallel retainsrealAMX andblocklocalUB but requires independent checks.


## Local reconciliation: icpp-pair-failed-10853976

Reconciled 2026-10-02T05:14:38.973663+00:00; event time is in preserved run evidence.

Job10853976 was the block:block retry of10853968; FAILED1:0 after3seconds onqhcn006 before any numericalwork. Preserved pinning.env proves actual allocation socket0:7cores/socket1:9cores. CPU-distribution preference didnot override fragmented free cores in a mixed node; payload kept hard16physicalcores/one-socket gate. The previous assumption that block preference alone guaranteedone-socket allocation is false in this cluster and is now corrected.

Resolution: keepblock:block/nomultithread/16coreguard and target currentlyIDLE sharedintel nodeqhcn023 (64physicalcores,2sockets32cores/socket,CPUAlloc0). Requeststill only16cores/80GiB, notexclusive, notfullnode, nootherjobs changed. Naminganidle node is an allocation workaround, notnewhardware result. Actualsocket/NUMA/CPUmodel retained inmanifest. Bothfailedruns retained. Newuniquejob pending. Frozenbinarycb15dbe... andallkernel/checks unchanged. No precision/performance acceptance. Allthreehandoffs updated before nextpreflight.


## Local reconciliation: icpp-pair-complete-10853989

Reconciled 2026-10-02T05:14:38.973663+00:00; event time is in preserved run evidence.

Job10853989 COMPLETED0:0 on qhcn023 in5m30s; step5m26s MaxRSS49983688KiB. One sharedintel node16physicalcores0-15 on socket0, explicitNUMA0/1interleave,80GiBrequested. Buildicpp-pair-build-20261002-124922 binarycb15dbe4455fc1c3ca62bc9c92a21a631c406fb452b8dc708f01212e606637ed. Namedidle node avoided fragmented mixednode allocations; failedJobs10853968/10853976 retained, guards unchanged.

OriginalOnline48smoke controls passed; pair57operator(36optimized/21fallback)+2layer wrappers passed strictbitwise output/numerator/den/max and rowhead/globalcounts, fourprofile/counters configurations. Realthree-layer ownoutput originalOnlinevsPair bitwise gate passed at everylayer for arxiv/products; separateprofile fingerprint unchanged and logicalread/staging identitiespassed. This establishes implementation reuse correctness relative tooriginalOnline, notFP32master/task acceptance. Realmastergate remainsFAIL (sameoutputs asoriginalOnline); .003 notrelaxed.

Samejob three-layer medianms arxiv: B0FP3273.486,B0BF1667.191,originalB1435.465,OnlineB32411.819,PairB32328.160. Products: B0FP323826.526,B0BF163723.189,originalB115800.130,OnlineB3214751.956,PairB3210023.882. PairvsOnline1.25497x arxiv/1.47168x products, vsoriginalB11.32699x/1.57625x, still4.884x/2.692x slowerthanB0BF16. Precision,threads,binding,data,timingboundary samejob.1warmup/3alternatingreps,seed11untrained; exploratory only, noformalpaperclaim.

Actual architecture preserved DegreeSort/TR16/neighbor-stepAMX/residentV. Pair logicalrawHbytes forL2halve516.780GB->258.390GB; eachheadweightedFMA andoriginaldenseprojection/TDPcountsunchanged. Therefore this measuredimprovement addresses repeatedreads, noteliminationofKEDd orD/d8work. Previous10851370time13.882s belongsdifferentjob; do notcrossmixspeedups. Frozenbaselines unchanged.

Allrawlogs,source/rule/binary/datahashes,manifest,Slurmconfig,pinning,accounting,summary/checks/profiles/stats/access tables preserved in runs/icpp-pair-10853989 anddownloadedlocal. No newnumericissue. Next newblock-granularityTFS compileandsmoke,thencompletearxiv/products withsameeightpaths and detailedcoarseprofile; researchprobe includesgraphreuseandhead-as-rowAMXPH diagnostic. BlockRNE interface changes andrequiresneworacle; noassumedbitwiseoldoutput orperformance. Allthreehandoffssynchronized.


## Local reconciliation: icpp-block-build-20261002-132444

Reconciled 2026-10-02T05:26:24.206913+00:00; event time is in preserved run evidence.

2026-10-02 block TFS build icpp-block-build-20261002-132444 completed status0 on login node; numerical workloads not run here. BinarySHA256 8d29a73119d771820c6867fcfdfe19ccbcb396e36a83b431cc51405a9b6daac0. icpx2024.1/MKL2023.2, precise FP32 flags, source/compiler/command/rules/binary hashes and source snapshot retained in unique build directory.

New candidate retains DegreeSort/TR16/dynamic panels, independent per-head FP32 Online m/l/p, output-dimensional V resident in AMX. It accumulates one bounded FP32 neighbor-block U_B, RNE packs and immediately consumes U_BW via true AMX; U_B never persists across blocks or becomes a full graph matrix. This changes the BF16 rounding interface relative to per-edge pH and has new independent FP64/FP32/BF16 emulator controls; all runtime and master gates remain pending. KED wide sparse arithmetic remains unchanged. Baseline src/include unchanged.

Assembly contains actual TDPBF16PS for block TFS and head-as-row PH microbenchmark; diagnostic probability generator uses the same __svml_expf16_z0 symbol as kernel. Static instruction occurrences are not runtime operation counts. No build warnings/errors. New PH probe includes score/exp/H gather/VNNI/P pack/output staging in full microtimer, 8 useful/16 configured head rows, high or high+low BF16 multiplication interfaces, separate quantized-operand instruction and original-FP32 error reports. Microbenchmark is not full-model acceleration or master acceptance.

Rules AGENTS55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7; skill641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca; GATcontractc0521a5ca650bba596f9b04c219bb1e6e1796e85c578449cef0f4b67c0fe17c7. User explicitly authorized continuing static GAT implementation/experiments; skill GCN mathematics is superseded by GAT contract, operational restrictions apply. Queue empty, qhcn023 idle preflight. Next unique sharedintel job16physicalcores96GiB, old/pair/block smoke first, then official arxiv/products complete eight-path three-layer comparisons and separate sampled profiling/reuse/PH diagnostics. No paper claim or runtime correctness claim from build. All three project handoffs synchronized before submission.


## Local reconciliation: icpp-block-submitted-10854102

Reconciled 2026-10-02T05:29:04.294844+00:00; event time is in preserved run evidence.

2026-10-02 Job10854102 submitted for block-granularity TFS controls/full three-layer exploratory arxiv/products benchmark and structural reuse/head-as-row PH microdiagnostics. One sharedintel node qhcn077 requested,16physicalcores96GiB,30min limit; notexclusive. qhcn023 became allocated by others after initial preflight, so named another idle node with same declared64core/2socket32core geometry. Actual CPU/NUMA/core binding saved by payload; single-socket16core guard remains unchanged. No jobs of other users touched. Current account queue was empty.

Buildicpp-block-build-20261002-132444 binary8d29a73119d771820c6867fcfdfe19ccbcb396e36a83b431cc51405a9b6daac0. Full AGENTS/skill reread, hashes unchanged; GAT implementation contract inspected; prior successful block build recorded in all three handoffs. Target run directory baseline/runs/icpp-block-10854102; smoke logs, source/rules/binary/data hashes, pinning/manifest and raw logs retained.1warmup3alternatingrepeats, same8paths, frozen src/include unchanged. No runtime gate/performance claim yet. Shared-node exploratory results only, FP32 master .003 unchanged. Handoff event is submission, not completion.


## Local reconciliation: icpp-block-failed-10854102

Reconciled 2026-10-02T05:31:10.107353+00:00; event time is in preserved run evidence.

2026-10-02 Job10854102 FAILED11:0 in35s on qhcn077; step31s MaxRSS4223712KiB, payloadexit139. Actualsingle-socket16physicalcores0-15,NUMA0/1interleave,CPU XeonMax9462. Original48/pair57+2/block57+2 smoke controls completed successfully, including independent FP32 math and new quantized-block emulator/control gates. Runtime master precision is not certified.

The arxiv program reached all8paths'3measured repetitions, then segfaulted at the post-timing same-input/contracted-LR FP32 control. Source inspection identifies missing standard[1].lr allocation: standard_bf16 Workspace allocates z/left/right/max/den/out, but only tfs_bf16 or matched_attention allocates lr. Driver then passed standard[1] to standard_layer(... lr_policy=fp32), whose lr_reordered SGEMM writes w.lr.data(). Frozen baseline kernel/API itself unchanged. No graph/model/softmax semantics changed. No core file is available in the project; source allocation/call path and last emitted log provide diagnosis. Partial timing is retained, excluded from complete result summaries. Products/reuse/PH were not run.

Repair only the new block driver: allocate required N*2K LR scratch before all timing and add explicit shape guard before matched-attention call. Also emit PH input provenance (last B64 own-L1 native BF16 H) and instrumentation/startup scope. New rebuild and unique rerun required; old build/logs preserved. No numeric gate relaxed. Allthreehandoffs synchronized before rebuild. No formal paper result.


## Local reconciliation: icpp-block-build-20261002-133145

Reconciled 2026-10-02T05:33:49.151092+00:00; event time is in preserved run evidence.

2026-10-02 icpp-block-build-20261002-133145 successful status0, binary890e5354f1dec9e93ef66425e7940658ef887c2967a49b020f927e71da800bf3, icpx2024.1/MKL2023.2 precise flags. Only block_main changed relative to previous build: matched-attention standard[1].lr scratch preallocated before timing, explicit shape guard, and PH input/instrumentation provenance line. Kernels, softmax mathematics, checks, frozen src/include, precision gates and timing intervals unchanged. Original failed job10854102 preserved; its passed block smoke and partial arxiv timing do not qualify as completed results.

Build/log/assembly/source_snapshot/rule/source/compiler/binary/command hashes retained at baseline/runs/icpp-block-build-20261002-133145. Full AGENTS/skill reread, currentcontracthash unchanged, priorfailedexperiment appearsinallthreehandoffs; queueempty andqhcn077idle atbuildpreflight. Numerical validation pending nextunique Slurm job, same16core96GiB sharedintel configuration. No new build issue; no runtime/performance/task certification. Allthreehandoffs synchronized before rerun.


## Local reconciliation: icpp-block-submitted-10854118

Reconciled 2026-10-02T05:34:54.453876+00:00; event time is in preserved run evidence.

2026-10-02 Job10854118 submitted as unique rerun after matched-attention driver scratch repair. Buildicpp-block-build-20261002-133145 binary890e5354f1dec9e93ef66425e7940658ef887c2967a49b020f927e71da800bf3. qhcn077 wasidle atsubmission; one sharedintel node16physicalcores96GiB,30min limit, nomultithread/block:block/single-socket guard/NUMAinterleave. FullAGENTSskill reread and hashes unchanged; GATcontract inspected; priorbuild andfailed10854102 recordedallthreehandoffs. Runbaseline/runs/icpp-block-10854118 preserves hashes/manifest/pinning/source snapshot/rawlogs/status. Same8paths,arxiv/products,1warmup3alternatingrepeats; controls before timing. No performance/master gate claim until logs and accounting complete. Original failed evidence preserved. Extra PH diagnostic input is last B64 own-L1 nativeBF16H, notFP32master; standalone sampledfirstblock micro with instrumentation/OMPstartup, notfullmodel speedup.


## Local reconciliation: icpp-ph-build-20261002-134035

Reconciled 2026-10-02T05:42:56.478960+00:00; event time is in preserved run evidence.

2026-10-02 icpp-ph-build-20261002-134035 successful status0; binary96cf467c6f96cf80ecbb8fd876d2a5b1b58dee01686d7d43b8037baef028d2f9. icpx2024.1/MKL2023.2, precise flags. Source/command/compiler/rules/binary snapshots retained. Original full-block job10854118 remains inprogress and recorded in allthreehandoffs; this is independent lightweight login compile, no extra numerical workload/node allocated.

New standalone PH layout control only: head_ph_compact_probe.cpp derives from preserved original helper, uses8active/configuredA/C rows and16right-operand rows, directly interleaves nativeBF16 H pairs with AVX512 integer instructions into VNNI, no row-major H staging copy. All8head P and FP32den remain independent. High or high+low BF16 coefficient interfaces unchanged. Physical issuedPHFMA expected1x/2x useful FMA versus original16row2x/4x. Current build provides no runtime precision/performance certificate; original quantized-operand FP64 budget and fullmaster labels unchanged. Assembly actualTDPBF16PS/SVML_z0 confirmed, no buildissues.

ph_main prepares actualB64 ownL1 from official graph outside microtimers, then tests original16 andcompact8 helpers on exactlysameL2nativeBF16H/contractedFP32LR.1warmup/3alternating micro repetitions; score/exp,Hgather/pack,Ppack,AMXinit/loadcompute/output included. NotmodelE2E speed, notwideFMAelimination, nottaskacceptance. Rulehashesunchanged,contractinspected,queue/partition inspected. Next unique16core24GiB sharedintel PH job aftercurrentfull-block evidence synchronized; allwrites wzh only. Allthreehandoffs updated.


## Local reconciliation: icpp-block-complete-10854118

Reconciled 2026-10-02T05:46:48.008034+00:00; event time is in preserved run evidence.

2026-10-02 Job10854118 COMPLETED0:0 onqhcn077 in10m24s;step10m21s MaxRSS59377892KiB,96GiBrequested,one sharedintel node16physicalcores0-15/socket0,NUMA0/1interleave,XeonMax9462. Buildicpp-block-build-20261002-133145 binary890e5354f1dec9e93ef66425e7940658ef887c2967a49b020f927e71da800bf3. Original failed10854102 preserved; matched-attention LR scratch repair validated by complete bothgraph controls and diagnostics. Frozenbaselinekernels unchanged.

Original48,pair57+2,block57+2 smoke controls passed, fourprofile/countercombinations; blockindependent FP32math/quantizedinstruction/max/den gates passed. Bothrealgraph8paths complete3layerownoutput1warmup3alternatingreps, profilefingerprints/counteridentities passed; alloutputsfinite. Originalmasterabs.003 remainsFAIL forBF16paths; no taskcheckpoint acceptance. Newblockquantization interfaces correctly modeled but do not equal FP32master. Sameinput/samecontractedLR/noELU L2 block errors separately reported.

Samejob mediansms arxiv:B0FP3274.599,B0BF1668.182,B116453.047,Online449.376,Pair369.766,Block16/32/64309.158/305.562/289.028. Products:B0FP323836.666,B0BF163751.798,B116312.194,Online15986.196,Pair10904.985,Block16/32/6412954.649/14129.061/14036.358. Bestblock vsoriginalB11.56749x arxiv/1.25918x products, still4.24x/3.45x slowerthanB0BF16. Productsblock loses toPair; do notmixolderjobtimes. Sharednode/randomseed11,exploratoryonly.

L2 actualinputFMA remains258390124544 andlogicalHbytes516780249088 inallproductblocks, exactlyD/d8xwidework. ActualAMXTDPcall reductions13.7996/23.5513/35.5998x forB16/32/64 vsneighbor-step; arithmeticreduction isnotE2E speed. SampledworkerPH spans799.041/948.341/998.252ms vsAMXloadcompute27.852/23.077/11.000ms indicate widegather/FP32SpMM expensive; samples areworker sums, notwallpercentages. Largerblocks are notuniformlyfaster.

TR16/B32sourcesupport systematic samples include high-degree tail, notpopulationestimates. productsedge/uniquesource1.00267, denseunionAMXslotsperedge16.1511;arxiv1.01858/16.1115. DegreeSort alone doesnot create favorable union density inthese samples; noassumed16destdensePH advantage. Originalhead-as-row512firstblockPH instructiongatespassed,FP32denbitwiseunchanged; microAVX/high/highlow products.127891/.109303/.097794ms (1.17x/1.30776x),arxiv.089520/.096171/.092108ms (AMXslower). Fulltimerincludespacking; micro ishot/instrumentedfirst32neighbors,notmodelE2E/master acceptance. InputfromlastB64 ownL1 nativeBF16H. Current8xarithmetic remainsunresolved.

Artifacts baseline/runs/icpp-block-10854118: rawlogs,all8pathsummary/checks/profiles/stats/blockwork/reuse/PHtables,source/rules/data/binaryhashes,manifest,pinning,accounting,status/source snapshot complete anddownloadedlocal. Next independentcompact8/direct-vectorPH layout control (alreadybuilt) onone16core24GiBsharednode; quantifypacking benefit before anyfullfused integration. Allthreehandoffs synchronized.


## Local reconciliation: icpp-ph-submitted-10854131

Reconciled 2026-10-02T05:48:03.523191+00:00; event time is in preserved run evidence.

2026-10-02 Job10854131 submitted for original16/scalarpack vscompact8/directvectorpack head-as-row PH micro controls on officialarxiv/products. One sharedintelnodeqhcn077,16physicalcores24GiB,5minlimit,nomultithread/block:block, single-socketguard andexplicitNUMAinterleave. Buildicpp-ph-build-20261002-134035 binary96cf467c6f96cf80ecbb8fd876d2a5b1b58dee01686d7d43b8037baef028d2f9. FullAGENTSskill readbeforejob,hashesunchanged,contractinspected,queueempty,nodeidle. Previousfullblock10854118 completed andallthreehandoffs synchronized; separatePHbuild also recordedallthree.

PH input produced byB64ownL1,score/p/denFP32 identical acrosslayouts, valueHnativeBF16.512sampledestinations/first32neighbors,1warmup/3alternatingmicroreps. Fullmicro includespacking/score/AMXpermission/config/loadcompute/store/scatter; instructionFP64oracleandoriginalFP32P errorsoutside. Acceptance onlymicroarithmeticscope,notfullmodel master/task gate orE2Eacceleration. Runbaseline/runs/icpp-ph-10854131 saves source/rule/binary/datahashes,manifest,rawlogs,pinning,status. No resultclaim yet. Allwriteswzh.


## Local reconciliation: icpp-ph-complete-10854131

Reconciled 2026-10-02T05:50:25.823202+00:00; event time is in preserved run evidence.

2026-10-02 Job10854131 COMPLETED0:0 in10s (step8s),qhcn077/XeonMax9462,single-socket16cores0-15,NUMA0/1interleave,sharedintel24GiBrequest. sacctreportsMaxRSS0 forshortstep; treatmemorymeasurementUNAVAILABLE,notactualzeromemory. Buildicpp-ph-build-20261002-134035 binary96cf467c6f96cf80ecbb8fd876d2a5b1b58dee01686d7d43b8037baef028d2f9. Officialarxiv/products,identicalB64ownL1 nativeBF16H/contractedFP32LR withinprocess,512DegreeSortsystematicdegree>=32samples/first32neighbors,1warmup3alternatingrepsperlayout. Fullmicroincludesallscore/pack/config/AMX/outputwork; currentshortinstrumentedhotsampletimings areexploratory,notformal/task/fullmodelclaims.

All12microvariantinstructiongatespassed,originalFP32denbitwiseunchangedwithinbothlayouts,finite. IndependentquantizedFP64oraclegatesunchanged; originalFP32P/nativeBF16Hdiagnosticsseparate. Compact8highlow normalizedrelativeL2arxiv3.0328158e-7/products4.9484049e-7;maxabs6.807672e-7/.0005044546. TheseonlyvalidatefirstblockPHagainstnativeBF16H; fullmodelmaster.003/taskaccuracy NOT_EVALUATED. High-only normalizedrelativeL2.0002061484/.0002926317; extraPquantizationexplicit.

Medianms arxivoriginal16:AVX.082640/high.099621/highlow.097386;compact8:AVX.092427/high.082519/highlow.080889. Productsoriginal16:AVX.105505/high.098054/highlow.097848;compact8:AVX.108900/high.084792/highlow.085308. AgainstowncompactAVX,highlow1.14264x/1.27655x; againstfasterAVXmedianacrossbothlayouts,conservative1.021647x/1.236757x. DuplicatedAVXcontrolmediansdiffer~11.8%arxiv/~3.2%products, so do notcherry-pickslowercontrolorclaimstatisticallyestablishedproductionthroughput. Original16vscompact8highlow improves1.20399x/1.146997x inthismicro.

SameusefulPHFMA33554432 per512samples;compact8physicalhigh/highlow33554432/67108864 vsold16rows67108864/134217728. DirectBF16pairVNNIinterleave eliminateslocalrow-majorHcopy,halvesconfiguredA/C/PpaddingwithoutchangingnativeH/Pmathdefinitions. productsrep1highlow sampledworkerHpacking.476199ms->.268862ms;AMX/load/store/scatter.482246->.416264ms. Theseareworker sums,notwallpercentages; theyco-varywithtwochanges(rows+packing),noisolatedcausalclaimforeither. Eightindependentheads remain; KEDeffective8xwideworknotremoved.

Artifacts baseline/runs/icpp-ph-10854131 complete rawlogs,status/accounting/manifest/pinning/source/rule/binary/datahashes,snapshots,derivedsummary/time/error/instruction/den/oracle tables downloadedlocal. Frozenbaselines/blockkernel unchanged. Nextresearchtarget isfullfused PH_B->per-headAMX U_BW extension keepingDegreeSort/TR16: limitedtilecapacityrequires boundedU stagingandactualVspill/reload acrossheads. Notyetimplemented; nofullmodelaccelerationinferredfrommicro. CurrentfullblockbeststilllosesB0andmastergatefails. Allthreehandoffs synchronized.


## Local reconciliation: icpp-research-reconciled-20261002

Reconciled 2026-10-02T05:56:54.420594+00:00; event time is in preserved run evidence.

2026-10-02 本轮完整归档：Pair10853989、Block10854118、PH10854131均COMPLETED0:0；失败Pair10853968/10853976和Block10854102，以及每次build、修复、manifest、rawlogs、source/rule/binary/data hashes和source snapshots保留并下载本地。当前账号队列为空，本轮没有运行中的作业，没有写出wzh，yx/shared均只读。

报告baseline/docs/ICPP_TFS_BLOCK_RESULTS_20261002.md、ICPP_TFS_TWO_BOTTLENECKS_RESEARCH_20261002.md及README已与完整rawtables核对。此前事件body的合并标签“B116453.047”表示arxiv B1=453.047ms；“B116312.194”表示products B1=16312.194ms，不是额外路径。正式结构化summary与本报告数字明确无歧义。失败partiallogs不作完整结果。

结论：保留DegreeSort/TR16/AMX融合的Block路径已实测减少重复projection；productsL2 TDP下降13.80/23.55/35.60倍，最佳完整3layer相对原B1为1.259倍，仍慢于B0和Pair。D/d8倍有效宽聚合不变。Compact8/vector-packing AMX PH双分量只有hot firstblock微控制的约1.022倍(arxiv)/1.237倍(products)保守速度，未集成三层PH→UW。两个重复AVX控制有噪声、sacct短PH任务内存未采到、逻辑字节非DRAM、worker sums非wall shares均公开。未放宽精度门；完整masterabs.003仍FAIL，训练checkpoint/task未验收。

源码与原GCN TFS对齐：原H_jW也执行大量额外矩阵运算；需要按AMX内外成本与HW跨边复用解释性能。原GCN kernel忽略values而MKL使用values、BF16截断vs当前RNE、原32thread128dim数据口径均明确，未对未经检查的历史图values作结论。新的研究推导/结构样本只作为可检验假设，不冒充已实现结果。

下一研究目标：保留ICPP调度与AMX SpMM→GeMM，将跨head共享H/AMX PH与block UW接入完整多头Online，同时显式测head/destination布局转换、boundedU staging、tile容量、跨headV spill/reload和精度。当前未实现这一完整新路径，不能宣称两项问题已同时解决。此轮实现/调研/试验完成；本地和服务器源码/结果已有，新迭代尚无GitHub push。三份handoffs已同步。


## Local reconciliation: icpp-heads-local-20261002

Reconciled 2026-10-02T08:45:14.891661+00:00; event time in retained evidence.

2026-10-02: cross-head PH -> UW complete-kernel implementation, static audit only.
New source: tfs_online/icpp_heads.hpp/.cpp, heads_checks.hpp/.cpp, heads_main.cpp; new explicit build/Slurm/payload/summarizer. Frozen src/include and old kernels are untouched. Aborted icpp_head_block.hpp/head_block_checks.hpp remain incomplete and excluded from build.
Objective: preserve DegreeSort/TR16/independent Vanilla GAT attention/Online Softmax/local aggregate-first U -> actual AMX UW; reduce duplicate H loads with heads-as-PH-rows. Source-width arithmetic amplification remains D/d=8 at L2; PH high+low performs two BF16 TDP passes. AVX shared-source control keeps FP32 P.
Unlike original one-head resident V, eight heads use bounded worker-local V across blocks; head-switch store/reload counted separately. Rescale directly acts on that V, extra rescale-triggered spill/reload bytes are zero, not total traffic zero. U_B<=128KiB per worker, consumed immediately; no global U/Z/e/alpha.
Plan: independent instruction-level oracle/gamma checks before speed; old Online/Pair/Block controls retained. Full three-layer own outputs on arxiv/products, same-run B0 FP32/BF16/B1/Pair/Block comparisons, warmup1/measured3 alternating order. Profile source index/score/max/exp/Hpack/Ppack/PHload/PHcompute/PHstore/UBconvert/UWload/UWcompute/Vstore/Vreload/row-rescale/output/scheduling separately, sampled worker spans not wall percentages.
Rules read fully from current server paths; unchanged AGENTS/SKILL/contract hashes verified. Existing v12-memo job10855476 belongs to separate work; left untouched. Will use one intel shared node, 16 physical cores on one socket with explicit NUMA interleave. Performance exploratory, master absolute .003 unchanged, task accuracy unverified. No numerical result or speedup yet.


## Local reconciliation: icpp-heads-build-failed-20261002-164747

Reconciled 2026-10-02T08:50:16.068734+00:00; event time in retained evidence.

2026-10-02 16:47:47 first heads build, run icpp-heads-build-20261002-164747: icpx compile/link and assembly emission succeeded, post-build rg audit failed because rg is not installed on server (exit127). No smoke or benchmark executed; this is an incomplete build experiment, not a numerical failure.
Retained build.log, command.txt, compiler.txt, binary.sha256, source.sha256, assembly, AGENTS/SKILL current read snapshots, preflight hashes, queue/partition,status. Postcompile source_snapshot command was after missing utility and did not execute; no claim of complete source snapshot for this failed build. Driver output/identity labels received a nonnumerical refinement during this interval; rebuild will use a new unique directory and snapshot before compile.
Resolution: grep -E for opcode audit (rg tried first, unavailable), move source snapshot before compile, report fallback rescale/final_store and AVX UB scatter identity, disclose check_runs1 before warmup1. Frozen kernels/precision gates unchanged. Next rebuild, then Slurm smoke before real-graph timing. All three handoffs synchronized, no new arithmetic issue.


## Local reconciliation: icpp-heads-build-20261002-165027

Reconciled 2026-10-02T08:51:54.271851+00:00; event time in retained evidence.

2026-10-02 16:50:27 successful heads rebuild, unique run icpp-heads-build-20261002-165027, exit0.
icpx2024.1 + oneMKL2023.2, -O3 -fp-model precise AVX512BF16 AMXTILE AMXBF16 OpenMP parallelMKL. Explicit SOURCES preserve frozen src/include and old Online/Pair/Block, new icpp_heads/heads_checks/heads_main; incomplete aborted headers and other main files excluded. All source snapshot saved BEFORE compile; command/compiler/source/binary/rule hashes and assembly retained. Source write target baseline/tfs_online, scripts; binary baseline/build/icpp_heads; TMPDIR baseline/build/tmp.
Postbuild opcode grep replaced missing rg; assembly audit explicitly confirms TDPBF16PS and production SVML exp z0 entry. Numerical correctness not yet claimed. No new arithmetic issue. Original master abs.003 unchanged. Next one shared intel node16physicalcores, independent smoke first, then all-nine-path arxiv/products 3-layer same-job exploratory E2E.


## Local reconciliation: icpp-heads-submitted-10855493

Reconciled 2026-10-02T08:52:35.318701+00:00; event time in retained evidence.

2026-10-02 submitted job10855493, intel shared partition, requested one qhcn080 node16physicalcores one socket,96GiB,40min limit (no exclusive). Other separate v12-memo job untouched; total requested nodes within5.
Build icpp-heads-build-20261002-165027, binary SHA256 ded2462a348db4b04530622660ffcd9afe134e6b808e324f83115ac97aa964e4. Assembly104 static tdpbf16ps sites and20 references to __svml_expf16_z0; these are static generated opcode counts, not executed FLOP counts. All current rules reread, contract unchanged, preceding event in all3 handoffs.
Run path baseline/runs/icpp-heads-10855493; stdout/err baseline/runs/icpp-heads-10855493.out/.err. Source snapshot/rule/source/binary/dataset hashes, config, pinning, manifest retained. Pinning guard rejects fewer than16physicalcores on one socket; selected NUMA nodes explicit interleave.
Payload executes old Online/Pair/Block and new independent Heads smoke first; stops before benchmarks on any fail. If accepted, arxiv/products same-run9paths (B0FP32/BF16,B1,PairB32,BlockB32,HeadSharedAVXB16/B32,HeadAMXHiLoB16/B32), own-output3layers,1correctness-run+1warmup+3alternating measured reps, comparisons outside timers, profiles separate. No result yet; exploratory, not paper table; master abs.003 and task acceptance unchanged.


## Local reconciliation: icpp-heads-complete-10855493

Reconciled 2026-10-02T09:16:03.258336+00:00; event time in retained evidence.

2026-10-02 Job10855493 COMPLETED0:0, qhcn080 shared intel16physicalcores0-15(socket0),NUMA0/1 interleave,7:12, MaxRSS59369432K. Binary ded2462a348db4b04530622660ffcd9afe134e6b808e324f83115ac97aa964e4. Build icpp-heads-build-20261002-165027. Full source/rules/contract/binary/data hashes and snapshots, compiler/command, pinning, manifest, rawlogs/summaries retained both server/local under baseline/runs/icpp-heads-10855493 and build dir.
Correctness: old Online/Pair/Block controls and independent Heads 28optimized+4fallback+2realLR layer checks passed before benchmark. Original 0.003 master abs gate still FAIL; real graph outputs finite, profile/counter fingerprints bitwise match speed, counter identities pass. Training task accuracy UNVERIFIED. See docs/ICPP_TFS_HEADS_RESULTS_20261002.md for error table and limitations.
Same-allocation own-output 3layer median arxiv B0BF1669.356ms, B1425.860, oldBlockB32307.916, sharedAVXB32120.496, AMXHiLoB32136.677. Products B0BF163762.105ms, B115198.386, oldBlockB3214073.959, sharedAVXB324405.851, AMXHiLoB324097.741. Best new path vs oldBlock2.56x/3.43x and vsB13.53x/3.71x. Still slower than strong standardGAT B0BF16 1.74x/1.09x. L2 wide sparse usefulPH8xnarrow persists; products logical rawH load 64.598GB vs oldheadseparate516.780GB. AMXHiLo products physicalPH702.592GFMA vs useful258.390GFMA; logicbytesnotDRAM, fineprofileworkersumsnotwallpercent. No paper table. Next preloadedP TMM candidate in new files, samecorrectness/fullmodel controls.


## Local reconciliation: icpp-preload-build-20261002-171807

Reconciled 2026-10-02T09:19:40.094364+00:00; event time in retained evidence.

2026-10-02 17:18:07 independent preload candidate build COMPLETED0, unique directory icpp-preload-build-20261002-171807. Binary baseline/build/icpp_preload SHA256 f75dc00f1e2c42f216e97964a2591574f2914b3a90b24a08bb2fcd5119dd5721. Explicit icpx2024.1/oneMKL2023.2 compile includes original Online/Pair/Block/Heads and new icpp_heads_preload, preload_checks, preload_main, one main only. Original binaries/source frozen; new source snapshot before compile, flags/commands/source/rule/binary hashes and assembly retained. Production preload assembly contains TDPBF16PS and __svml_expf16_z0, verified static symbols; no numerical result yet.
Purpose: keep DegreeSort/TR16/multihead Online exact; retain high/low BF16 P in TMM6/TMM2 for each row block instead of reloading both for every feature16 block. Same PH high then low arithmetic, bounded local U→AMX UW, worker V head state. Independent regression will compare old/new max/den/raw numerator/layer output and work counts bitwise across tail/equal/late-max/fallback/profile variants before speed. Then full3layer own-output arxiv/products same allocation controls. Original .003 master gate unchanged; performance exploratory; task accuracy UNVERIFIED.


## Local reconciliation: icpp-preload-submitted-10855585

Reconciled 2026-10-02T09:20:10.649467+00:00; event time in retained evidence.

2026-10-02 Job10855585 submitted to one intel shared qhcn080,16physicalcores,96GiB40min; exclusive not requested, other jobs untouched, <=5 nodes. Build icpp-preload-build-20261002-171807 binary f75dc00f1e2c42f216e97964a2591574f2914b3a90b24a08bb2fcd5119dd5721.
Purpose: validate register-resident Phi/Plo preload with old/new bitwise operation/layer checks then compare full3layer E2E. New run dir baseline/runs/icpp-preload-10855585, logs/manifest/pinning/data/source/rule/binary hashes/source snapshot. If smoke fails, payload stops before benchmark and failure remains documented. If smoke passes: same-job B0FP32/BF16,B1,Pair,Block,HeadsAVXB16/B32,HeadsAMXHiLoB16/B32,PreloadB16/B32; ownoutput3layers; one correctness run,onewarmup,three alternated measured repetitions and sampled worker profile. Same precision/model/NUMA/threads/panel/block; master .003 unchanged, performance exploratory, task accuracy unverified. Previous job10855493 complete and recorded all3 handoffs before submission.


## Local reconciliation: icpp-preload-complete-10855585

Reconciled 2026-10-02T09:46:34.757186+00:00; event time in retained evidence.

2026-10-02 Job10855585 COMPLETED0:0 on intel shared qhcn080,16physicalcores0-15 one socket,NUMA0/1 interleave,8:23,MaxRSS59386932K. Build icpp-preload-build-20261002-171807, binary f75dc00f1e2c42f216e97964a2591574f2914b3a90b24a08bb2fcd5119dd5721. Complete rawlogs, smoke, machine TSV, status, source/rule/binary/data hashes, compiler/command/assembly, source snapshot, Slurm config/manifest/pinning downloaded local and retained server under baseline/runs/icpp-preload-10855585/ and build path.
Correctness old independent Heads28optimized+4fallback+2realLR controls plus new18operator and6layer preload bitwise regressions×4 profile/counter variants all pass. Real graph outputs finite and profile fingerprints unchanged. Master .003 abs still FAIL old/new (same precision, output errors identical); untrained model, task accuracy UNVERIFIED.
Same-job 3layer medians arxiv B0BF1669.157ms, AVXB32119.143,oldAMXB32134.822,preloadB32133.428. Products B0BF163773.251ms,AVXB324444.704,oldAMXB324071.255,preloadB324058.823. L2 preload improves oldAMX 70.861->67.065ms arxiv and1891.413->1807.526ms products, L1 slows and E2E improvement only1.0%/.3%; repetitions overlap. Preload still1.93x/1.076x slower than B0. PtilelogicalbytesL2 old->new3.237GB->.202GB arxiv,87.824GB->5.489GB products; physicalPHFMA and useful8xwide unchanged. Fine worker timers notwallpercentage, bytes notDRAM. Full report docs/ICPP_TFS_PRELOAD_RESULTS_20261002.md. Next exact mixedPH shortAVX/fullAMX dispatcher with preserved TFS AMX UW and independentoracle, not yet measured. No new major issue; small optimization insufficient to close B0 gap. No paper claim.


## Local reconciliation: icpp-hybrid-superseded-build-20261002-174954

Reconciled 2026-10-02T09:51:18.856067+00:00; event time in retained evidence.

2026-10-02 17:49:54 first hybrid build icpp-hybrid-build-20261002-174954 COMPLETED0 but superseded before running any smoke/benchmark. Static review found a misleading work-log field name: value was physical PH FMA / useful PH FMA, labelled as AVX_to_AMX_physical_fma_ratio. Arithmetic, kernel source, oracle, work identities unchanged. The build/source snapshot/binary/assembly/logs retained in unique directory; this binary is excluded from reported numerical results.
Resolution: rename field physical_to_useful_PH_FMA in new hybrid_main and derivation script, rebuild in a new unique directory, then smoke before timing. No change to master .003 gate, model semantics, or previous complete experiments. All three handoffs synchronized. This is reporting correction, no numerical failure.


## Local reconciliation: icpp-hybrid-build-20261002-175141

Reconciled 2026-10-02T09:53:18.670955+00:00; event time in retained evidence.

2026-10-02 17:51:41 corrected hybrid build COMPLETED0 unique dir icpp-hybrid-build-20261002-175141. Binary SHA256 b3812f9046f695f9ac6fd6987bb60f1aaaaf1dccd0c9225f9920f4bf1808505c. Explicit icpx2024.1/oneMKL2023.2 old Online/Pair/Block/Heads/Preload and new icpp_heads_hybrid/hybrid_checks/hybrid_main, one main; frozen src/include and old candidates unchanged. Source snapshot saved before compile, compiler/command/source/rule/binary hashes, assembly/opcode audit retained. TDPBF16PS and __svml_expf16_z0 present in production assembly. No numerical result yet.
New exact mixed PH: threshold16/24/32 actual destination row block count picks shared-H AVX FP32 P or preloaded AMX high+low P, then both consume bounded RNE U_B with AMX UW. No global U/Z/e/alpha, DegreeSort/TR16, independent heads and FP32 Online preserved. Smoke independently checks arithmetic, counters, tails, mixedmax/rescale, fallback and threshold endpoints before real graph. Original .003 master gate unchanged. First completed build174954 retained and excluded from numerical work due misleading field name; corrected field physical_to_useful_PH_FMA. Performance exploratory, task accuracy unverified.


## Local reconciliation: icpp-hybrid-submitted-10855618

Reconciled 2026-10-02T09:53:55.609166+00:00; event time in retained evidence.

2026-10-02 Job10855618 submitted, intel shared qhcn080, one node16physicalcores one socket,96GiB40min. Corrected build icpp-hybrid-build-20261002-175141 binary b3812f9046f695f9ac6fd6987bb60f1aaaaf1dccd0c9225f9920f4bf1808505c. Other account job10855617 on qhcn818 untouched; total <=5nodes. Complete current AGENTS/SKILL reread, contract hash unchanged, queue/partition inspected; prior superseded/build events in all3 handoffs.
Write target baseline/runs/icpp-hybrid-10855618/, stdout/err matching runs. Source/rule/data/binary hashes, source snapshot, manifest/Slurm config/pinning/status retained. Smoke first: old Online/Pair/Block, Heads independent PH/oracle, preload bitwise, new hybrid independent mixed quantized oracle (46optimized operator, fallback, true L/R and ELU, all profile/counter variants). Stop before speed on any failure. If accepted, all controls and hybrid thresholds16/24/32 B32 on arxiv/products, same model ownoutput3layers, onecheckrun/onewarmup/threealternatingreps. Sampled worker timers notwallshares; uninstrumented E2E determines speed. Master .003 unchanged, performance exploratory, task accuracy UNVERIFIED. No result yet.


## Local reconciliation: icpp-hybrid-completed-10855618

Reconciled 2026-10-02T14:19:29.989857+00:00; event time in retained evidence.

2026-10-02 event: icpp-hybrid-completed-10855618
Job 10855618 COMPLETED 0:0 in 00:10:05 on qhcn080; compute MaxRSS 59527356K. 16 pinned physical cores 0-15, NUMA 0/1 interleave, shared intel. Corrected binary SHA256 b3812f9046f695f9ac6fd6987bb60f1aaaaf1dccd0c9225f9920f4bf1808505c. Same-job real arxiv/products three-layer own-output benchmark, seed11 untrained, one correctness, one warmup, three alternating repetitions.
Hybrid T16 B32 E2E median arxiv 120.608 ms, products 3942.222 ms. Strong B0 BF16 68.923/3754.817 ms; original B1 422.791/15227.614 ms. Hybrid T16 vs original B1 speedup 3.5055x/3.8627x; vs B0 speedup 0.5715x/0.9525x. Products hybrid L1/L2/L3 1058.876/1765.228/1106.190 ms; B0 1623.428/1640.795/494.285 ms. Product L2 physical/useful PH FMA 527.796/258.390 billion =2.043x, but direct sparse width amplification remains 8x. Logical raw H bytes 64.598 GB versus old per-head 516.780 GB. Hybrid T16, T24, T32 complete and independently checked; no trained accuracy; original .003 master gate FAIL also for B0 BF16. Exploratory shared-node only.
Evidence: baseline/runs/icpp-hybrid-10855618/{arxiv.log,products.log,smoke.log,timing_summary.tsv,checks.tsv,profiles.tsv,work.tsv,status.txt,manifest.txt,affinity.txt,source.sha256,artifact.sha256}; baseline/runs/icpp-hybrid-build-20261002-175141/; baseline/docs/ICPP_TFS_HYBRID_RESULTS_20261002.md. No new kernel correctness issue. Open performance issue: products one-head layer-3 old TFS fallback ~612 ms slower than B0; arxiv hybrid slower in all layers. Next: separate clearly labelled TFS-L1/L2 plus standard-L3 dispatcher and same-job B0/full-TFS comparison.


## Local reconciliation: icpp-adaptive-build-20261002-222445

Reconciled 2026-10-02T14:26:25.489556+00:00; event time in retained evidence.

2026-10-02 event: icpp-adaptive-build-20261002-222445
Purpose: compile an isolated layer-dispatch experiment under /home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline. The new source path is baseline/tfs_online/adaptive_main.cpp; scripts are baseline/scripts/build_icpp_adaptive.sh, icpp_adaptive_payload.sh and run_icpp_adaptive.slurm. Existing B0 and TFS kernels are unchanged. Controls are full 3-layer B0 BF16 and full 3-layer HYBRID_T16_B32; candidate retains hybrid DegreeSort/TR16/Online PH->AMX UW in L1/L2 and uses standard BF16 transform-first in L3 with contracted FP32 LR.
Preflight current server AGENTS SHA256 55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7, SKILL SHA256 641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca, GAT contract SHA256 c0521a5ca650bba596f9b04c219bb1e6e1796e85c578449cef0f4b67c0fe17c7. Both mandatory files read from current server paths; previous hybrid-complete event verified in all three server handoffs. Queue empty for hdacp1; normal intel partition up. Build command in run directory uses icpx oneAPI 2024.1.0, MKL 2023.2.0, precise FP model, OpenMP, AVX512 and AMX BF16.
Build exit_status=0, unique run baseline/runs/icpp-adaptive-build-20261002-222445/. Binary SHA256 1da262fd8b9affa6b127a0d54d6ad22cb1385b88115ea78d1b333e50ea48aff5. Assembly grep count 76 for TDPBF16PS/SVML-exp patterns in retained icpp_adaptive.s. Local and server SHA256 for all four new source/script files match. No runtime conclusion from build alone; correctness and real-graph timing are next. No new major issue.


## Local reconciliation: icpp-adaptive-submitted-10856767

Reconciled 2026-10-02T14:27:44.880330+00:00; event time in retained evidence.

2026-10-02 event: icpp-adaptive-submitted-10856767
Submitted one-node normal intel Slurm JobID 10856767 using baseline/scripts/run_icpp_adaptive.slurm, cpus-per-task 16, mem 96G, limit 00:40:00. User account queue was empty and intel partition up at submit. Current server AGENTS, SKILL and GAT contract were reread; their SHA256 values remain 55ac34d581d9cfda4e408ef5ad1fdb58c1dbf0cdd0c5fafbf725f5e31ff12ec7, 641694f2090f9b1581b32b64b8273ff6acc7fb470d0910ca9390f48766a4bcca, c0521a5ca650bba596f9b04c219bb1e6e1796e85c578449cef0f4b67c0fe17c7. Prior adaptive-build event is present in all three server handoffs.
Target unique run path: /home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline/runs/icpp-adaptive-10856767/. Build binary SHA256 1da262fd8b9affa6b127a0d54d6ad22cb1385b88115ea78d1b333e50ea48aff5. Payload first runs old independent hybrid smoke and new mixed-path correctness controls, then one warmup and three alternating three-layer own-output repetitions on arxiv/products under 16 physical pinned cores and explicit NUMA interleave. Controls B0 BF16 and full HYBRID_T16_B32; candidate TFS_L12_B0_L3_MATCHED. Job status and performance pending; no result claimed. No new major issue at submission.


## Local reconciliation: icpp-adaptive-completed-10856767

Reconciled 2026-10-02T14:33:06.516935+00:00; event time in retained evidence.

2026-10-02 event: icpp-adaptive-completed-10856767
JobID 10856767 COMPLETED 0:0 in 00:02:04, shared intel qhcn008, 16 pinned socket-0 physical cores 10-25, NUMA nodes 1-3 explicit interleave, compute MaxRSS 59411708K. Preflight current AGENTS/SKILL hashes recorded in run. Binary SHA256 1da262fd8b9affa6b127a0d54d6ad22cb1385b88115ea78d1b333e50ea48aff5. Same real arxiv/products CSR, untrained seed11 3-layer 8x32/8x32/1xC model, own-output chaining, one independent smoke, one correctness run, one warmup and 3 alternating E2E repetitions. Raw source/binary/graph hashes, binding, manifest, run status and log completion markers retained.
Same-job medians arxiv: B0 BF16 68.318 ms; full hybrid TFS 118.409 ms; mixed TFS L1/L2 + standard BF16 matched-LR L3 105.084 ms. Products: B0 3845.103 ms; full TFS 4032.908 ms; mixed 3411.046 ms. Mixed speedup products vs strong B0 1.1273x, vs full TFS 1.1823x; on arxiv mixed is 1.538x slower than B0. Products L3 full TFS 1144.145 ms vs mixed standard 518.976 ms, a 625.169 ms layer reduction. Products L1 mixed 1088.837 vs B0 1657.345 ms; L2 mixed 1801.185 vs B0 1686.829 ms. This is a mixed-dataflow result, not all-layer TFS faster than B0.
Independent smoke passed (46 optimized operator, 6 fallback, 3 actual-LR/ELU layer, 2 fallback layer controls, 4 profile/counter variants). All real outputs finite and full-TFS profile bit fingerprints equal. Same-input L3 products relative L2 standard vs TFS 0.001287. Products final output versus FP32 master max abs/relL2: B0 BF16 25.121/0.02538, full TFS 6.048/0.01513, mixed 6.046/0.01506. Original .003 absolute master gate FAIL, including B0; task accuracy UNVERIFIED. No paper-accepted performance/accuracy claim. Exact aggregate-first L2 useful sparse width amplification remains 8x. No new runtime correctness issue. Open acceptance issue is missing trained checkpoint/task metric and formal performance validation.
Evidence: baseline/runs/icpp-adaptive-10856767/ (smoke.log, arxiv.log, products.log, timing_summary.tsv, checks.tsv, status.txt, manifest.txt, pinning.env, affinity.txt, source.sha256, artifact.sha256, source_snapshot/), baseline/runs/icpp-adaptive-build-20261002-222445/, baseline/docs/ICPP_TFS_ADAPTIVE_RESULTS_20261002.md. Graph SHA256 arxiv 3407b49a3b659397aa581ea4c5afcdef37e929a70ed98217a52413ef21201280; products cbb38a8715b8cc7c99e7caf782aa286f835f45e2802d0f4860673fb442c69869. Next: preserve full-TFS control; publish verified exploratory source/results; trained task metric and formal rerun only when available.


## Local reconciliation: icpp-tfs-github-published-f9c25e3

Reconciled 2026-10-02T14:43:16.077660+00:00; event time in retained evidence.

2026-10-02 event: icpp-tfs-github-published-f9c25e3
The verified ICPP TFS GAT source, scripts, complete real-graph run logs/tables, build commands and research reports were committed and pushed to https://github.com/huamiao123/GAT-TFS.git master at f9c25e3233657c44d4a986d45206a7813aa413e1. Local HEAD and remote refs/heads/master were both verified at that exact hash after push. This is a publication of exploratory evidence, not an accuracy-accepted or formal paper result.
Scope includes original Online, Pair, Block, cross-head PH, preload, hybrid, and the mixed TFS-L1/L2 plus standard-L3 experiment. The key latest same-job result is products 3411.046 ms mixed versus B0 BF16 3845.103 ms (1.1273x), full TFS 4032.908 ms; arxiv mixed 105.084 ms versus B0 68.318 ms. DegreeSort/TR16 and true AMX SpMM->GeMM fusion remain in the TFS layers; direct L2 8x useful sparse width remains. The strict .003 master absolute gate fails even for BF16 B0, and trained task accuracy is unverified. No paper claim should cite these as accepted speedups yet.
Curated Git excludes the three incomplete icpp_head_block/head_block_checks source drafts, failed raw run directories, partial logs, generated assembly and duplicate source snapshots. Historical local/server run snapshots and failed-job evidence remain retained for provenance, not silently rewritten. Current local incomplete drafts were removed; they were absent on the server source root at cleanup. The explicit build SOURCES lists exclude them. Next concrete research gate: obtain a real checkpoint/labels/split and calibrate task accuracy plus formal controlled performance runs; retain B0 and full TFS controls. No new major runtime issue from publication.
