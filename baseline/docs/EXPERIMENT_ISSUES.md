# GAT baseline issues

2026-09-30: first compile passed; no new compile issue. Production accuracy thresholds and task metrics are unconfigured because no trained GAT checkpoint has been supplied. BF16 oneMKL API compiles; vendor AMX dispatch has not yet been established. Old system objdump may not decode AMX instructions, so absence of disassembly matches is not evidence of absent instructions. Record compiler-generated assembly as well as runtime kernel checks.

2026-09-30 check-10808045: correctness tests passed. The initial reordered AVX candidate gathered strided contraction weights inside the dot and was unnecessarily slow. Replace with static transposed bL/bR and fused two-dot reading X once. Skinny SGEMM micro candidate initially allocated/setup temporary weights in the measured function; move preparation to static workspace for a fair comparison. These changes require rebuild and regression before accepting new timings. No correctness failure was observed. BF16 task accuracy remains unconfigured.

2026-09-30 build-20260930-152447: updated sources compile and generated assembly confirms TDPBF16PS. No new compile issue. AVX and static-SGEMM changes still require runtime regression; vendor GEMM AMX counter event is available in perf list but permission is not yet tested.

2026-09-30 arxiv-10808152: srun inherited a one-core step allocation despite the batch job requesting 16 CPUs. Manifest correctly captured OMP/MKL=1, so performance numbers are only single-core diagnostics. Fix: explicitly pass --cpus-per-task=$SLURM_CPUS_PER_TASK to srun, and fail a multi-core validation if affinity exposes fewer than two physical cores. Retain this run and rerun. Vendor AMX counter succeeded; no new numerical implementation issue. BF16 checkpoint accuracy still unconfigured.

2026-09-30 build-20260930-154006: no new compile issue. Speed build still compiles per-edge/per-vector clocks and stats out; heavy worker profiling is a separate binary. Runtime rerun must verify actual step affinity before accepting multi-core timings.

2026-09-30 arxiv-10808269: srun affinity defect resolved; actual16physical cores on one socket. Numerical regressions passed, no new correctness issue. Fine clocks are expensive: L2 TFS profile wall3749ms vs speed345ms, so worker subfield proportions are instrumented diagnostics, not unbiased bottleneck fractions. Use speed-build stage wall/fixed-p and future sampled/microkernel measurements. B0FP32 outperformed BF16 in this shared-node run; keep both results and avoid unsupported hardware-cause claims. PowerShell ConvertFrom-Json treats D/d as case-insensitive duplicate keys; raw JSON is valid, use Python summarizer. Trained-checkpoint/task gate still missing.

2026-09-30 local evaluator fixture: no new issue; classification/accuracy-drop and all input rejection tests passed. This does not calibrate a real checkpoint gate.
