# Pair plans and exact-window sharing: preregistered protocol 2026-10-07

## Objective and authorization
User says continue actual planned experiments. Study fixed-tile pairing and whether static matching can amortize dynamic head-comparison costs. This is GCN-extra unit-weight inference, not TFS-Train normalized training; four v2 specs read for scope. Original TFS/MKL and previous runs immutable. 128->128->128 only. No NUMA interleave or first-touch change. At most10 requested/allocated nodes combined (including pending). Shared intel and exclusive intel_expr are explicitly authorized; report separately.

## Hypotheses
H1: Changing 8 pairs inside an unchanged TR16 tile can improve realizable reuse without changing AMX row slots, projection work, individual CSR addition order, or output order.
H2: A CSR window that has exactly the same positive-length source sequence for both rows can decode once per source and update both accumulators without per-edge head comparison.
H3: A static exact-sharing fast path plus scalar fallback can outperform unrestricted dynamic matching, but coverage and metadata/setup overhead may make it ineffective. These are hypotheses, not accepted contributions.

## Fixed controls
Same unit-weight CSR and original source seed12345 H/W; TR16, R64, S64, dynamic(1), 32 physical cores on one socket, OMP close/cores, original packed weights, BF16 integer truncation, default MKL environment. Main performance FAST/MFULL only; correctness also M64/M256 and ACCURATE. Use previous run05 PACK_EPI as strong baseline; paper TFS and source FP32 MKL are anchors. Never attribute all cumulative TFS speedup to this round.
Existing degree and qguard schedules retained. Only row-pair order inside each tile changes. Tile members, original permutation-to-output mapping, and AMX row slots do not change. No graph rewrite, set-wise reassociation, or common partial sums.

## Pair plans
adj: existing consecutive logical rows.
opt: at most first/middle/last S64 windows per row; deterministic minhash signatures choose at most3 candidate partners plus original adjacent partner per row; only representative windows (union<=6) use true current-head merge scoring. Deterministic greedy pairs and original-order unmatched completion. If sampled score is below original adjacent score, retain original tile pairs; all-zero score tiles retain adjacency. No performance-based choice. Sampling/assignment/full verification costs and plan bytes reported.
rand: deterministic seeded shuffle of rows inside the same tile; control only, seed20261007.
A plan owns a 16-byte bijective row order per tile, offsets/window count, and one8-bit exact-sharing mask per S64 tile window. A mask bit is set only when both lengths are equal and positive and the actual CSR sequences pass memcmp. Hash collision never authorizes sharing. Empty and odd-tail rows explicit. Global S64 window indices align for M64/M256/FULL.
Full plan audit counts realized dynamic shared heads and exact shared edges without timings; exact masks and permutations independently checked. This is source-level structure, not cache traffic. Additional structural audit is reported outside primary timing, not silently excluded from deployment setup estimate.

## Execution variants
scalar: use plan row order, each row independently.
dyn_i/dyn_s: same head-merge traversal and pair order; decode separately vs shared on equal current heads.
exact_i/exact_s: exact flagged windows run two-row lockstep, decode separately vs once; unflagged windows run scalar rows. No per-edge equality test in flagged path.
All keep edge additions, AMX TDP, scope/window count, partial state budget and logical per-edge prefetch policy. exact_s may remove second-row current CSR reads; distinguish this from constant logical edge incidence. Regular intrinsics and compiler optimizations allowed for all controls; never weaken controls with volatile asm. Audit actual generated instructions and hot-loop spills.

## Frozen candidate matrix
Diagnostic4 (mycielskian19,reddit,roadNet-CA,amazon0601), synthetic3, smoke:
paper_tfs; pack_epi_degree; legacy_i_degree; legacy_s_degree;
adj_degree and opt_degree each scalar/dyn_i/dyn_s/exact_i/exact_s;
rand_degree scalar/dyn_s/exact_s;
pack_epi_qguard; opt_qguard each scalar/dyn_i/dyn_s/exact_i/exact_s.
Total23 methods plus FP32 MKL anchor. No per-graph best is presented as deployed policy.
Remaining13 fixed validation:
paper_tfs,pack_epi_degree,legacy_s_degree,adj_exact_s_degree,opt_dyn_i_degree,opt_dyn_s_degree,opt_exact_i_degree,opt_exact_s_degree,pack_epi_qguard,opt_dyn_i_qguard,opt_dyn_s_qguard,opt_exact_i_qguard,opt_exact_s_qguard.
Total13 methods plus FP32 MKL anchor. Diagnostic-only random and scalar methods never reported as full17 GM.

## Correctness before timing
Same four numeric gates: FAST kernel/finalFP32 <.01 relativeL2 and normalizedmax vs original TFS; finalBF16 <.02 vs original TFS; <.03 vs sourceFP32MKL. Same-M/precision paths must match canonical coupled output bitwise at kernelFP32, finalFP32, H1BF16 and finalBF16. New planned paths are validated against original degree schedule, not against one another only. NaN prefilling checks complete writes.
Preserve19-value ReLU/truncation semantics test, including NaN payload, signed zero and subnormal behavior. Smoke covers M64/M256/FULL x FAST/ACCURATE x plans/modes; signed cancellation, unsorted/duplicate CSR, empty/odd/tail rows and window/scope boundaries. Include explicit same-window and almost-same-window fixtures; assert exact shared path coverage>0 and mismatches remain unshared. Do not time after any failure; preserve failed builds/jobs.
Instrumented vs uninstrumented outputs bitwise; analytic and actual edge/add/TDP/decode/CSR/prefetch/state work gates for both layers. Distinguish FP32 diagnostic first layer from production fused-EPI when reporting bytes. Static plan build/validation logs retained.

## Measurements
Prepared two-layer E2E includes first layer fused original-semantics ReLU/truncation, second layer and final conversion. Graph load, all plan/schedule construction, initial allocation/packing excluded and separately measured. Same graph/process/job/node/input/affinity for each comparison. 1warmup+5consecutive, independently10forward/reverse alternating samples. Ratio of medians primary, followed by graph-equal GM; min/max/CV and raw samples retained. No cross-job denominator and no pooling shared/exclusive or synthetic/real.
Optional existing user-level PMU on diagnostic4 and synthetic3: PACK_EPIdegree, legacySdegree,optDynSdegree,optExactSdegree,3repeats, complete32thread coverage or explicit missing. PMU time is not primary timing. No permission changes or new event claims. Cache misses are not DRAM bytes; AMX_BUSY is not proof of AVX/AMX overlap.
Report plan setup and retained bytes, per-inference saving, amortization only where positive and larger than noise; construction of multiple experimental plans is distinguished from incremental single-plan deployment cost.

## Execution and reporting
1. Unique immutable build after all source files are complete, fresh AGENTS/SKILL/spec/queue/handoff preflight. Explicit snapshot list; analysis tools frozen separately only after completion.
2. Shared 1node smoke and structure/work gates.
3. Diagnostic4 on up to2exclusive and2shared jobs, synthetic3 shared; combined node cap10.
4. Once diagnostic correctness passes, run remaining13 on <=4shared jobs if within cap; retain negative cases.
5. Reconcile all exits, source/binary/input hashes, gate counts, complete17 coverage, all3 GCN-extra handoffs. Report successes and failures, derive next question from evidence. TXT copied to desktop under prior document authorization.

## Pre-build clarification
Synthetic3 also receive the same fixed4-method,3repeat PMU pass as diagnostic4, because equal shared-head fractions had different benefits last round. This is separate from primary timing and introduces no tuning or new permissions.
