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
