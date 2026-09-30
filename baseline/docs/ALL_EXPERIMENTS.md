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
