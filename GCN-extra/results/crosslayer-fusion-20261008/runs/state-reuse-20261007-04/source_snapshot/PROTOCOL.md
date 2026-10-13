# State lifetime and shared decode: preregistered experiment, 2026-10-07

## Authorization and scope
User requests planning followed by actual experiments. Fixed 128 -> 128 -> 128 only, current source-seeded random H/W, unit-weight CSR, original node order, TR16/R64 dynamic(1), S64. No NUMA interleave, first-touch change, dataset rewriting, or precision gate relaxation. User node cap <=10 allocated/requested across our jobs supersedes historical smaller caps. Dedicated intel_expr exclusive preferred for performance; shared intel allowed and separately labelled. Original TFS/MKL, src, and all prior runs immutable. TFS-Train specifications have been read for scope; training normalization/dropout/backward do not replace current inference semantics.

## Hypotheses and independent changes
H1 NOZERO: omit redundant partial initialization only when all consumed rows are explicitly initialized or filled; tail and zero-degree outputs remain defined.
H2 DIRECTPACK: each row's last S64 window packs identical truncated BF16 hi/lo directly from its FP32 accumulator, avoiding terminal partial store/reload. Intermediate windows unchanged. Test NOZERO alone, DIRECTPACK alone, both.
H3 EPILOGUE: only layer1 final output does original comparison ReLU (negative -> positive zero; otherwise preserve value), then identical integer BF16 truncation directly into global H1b. Keep completed-layer dependency and synchronization. Reference/correctness path retains FP32 first-layer output separately. Do not substitute RNE conversion.
H4 PAIR_INTERLEAVE and PAIR_SHARE: two destination rows keep their own CSR accumulation order within each S64 window. Both use the same head-merge traversal and per-edge prefetch policy; INTERLEAVE separately loads/decodes a shared source, SHARE decodes once and adds to both accumulators. Unsorted and duplicate CSR entries supported without reordering within a row. Pair tails/empty rows explicit. Runtime branch/index cost and any spill belong in timing. Fixed adjacent row pairing inside each TR16 tile; no new grouping search in this round.

## Frozen controls and candidate matrix
Anchors: original TFS, canonical coupled B256 FAST and coupled FULL FAST, source FP32 MKL (precision-mismatched anchor explicitly labelled).
Degree main candidates: S64/FULL canonical P0; NOZERO; DIRECTPACK; both PACK; EPILOGUE only; PACK+EPILOGUE; PAIR_INTERLEAVE+PACK+EPILOGUE; PAIR_SHARE+PACK+EPILOGUE.
Existing frozen qguard grouping: canonical S64/FULL P0, PACK+EPILOGUE, PAIR_INTERLEAVE+PACK+EPILOGUE, PAIR_SHARE+PACK+EPILOGUE. Reuse exact 20261007 qguard, no new threshold.
M256 control: canonical S64/M256 degree and PACK+EPILOGUE degree.
Correctness additionally covers M64/M256/FULL FAST and ACCURATE for all applicable reducer/epilogue modes, but primary performance comparisons remain FAST only. ACCURATE never mixed with FAST speedup denominators.

## Gates before timing
New same-M same-precision variants must match canonical validated coupled/P0 results bitwise (including signed zero) at single-layer FP32, final two-layer FP32, and actual final BF16. EPILOGUE also requires H1b bitwise and final outputs equal separated ReLU/truncation. NaN prefill checks output coverage. Existing numeric gates retained: FAST kernel/final FP32 relL2 and normalized max <.01 vs original TFS, BF16 final <.02; original FP32 MKL <.03; ACCURATE applicable TFS gates <.001 and finalBF16 <.01. No timing after failing gate. Record every failure without replacing frozen runs.
Smoke fixtures: empty rows, N tails incl 1/15/16/17/31/33/65, degrees around 1/16/32/63/64/65/127/128/255/256/257/512, duplicate and unsorted CSR; finite sign/cancellation data and separate ReLU conversion semantic fixture for +/-0 and NaN payload where supported. Semantic edge fixture is not a finite-output numeric benchmark.

## Measurement
Every method: same graph/process/node/input/thread binding. Uninstrumented two-layer compute includes layer1, activation, interlayer conversion, layer2 and final conversion. Excludes graph/schedule creation and allocation, which are reported separately. 1 warmup +5 consecutive samples; independent 10 alternating forward/reverse samples. Preserve all repeats, min/median/max and ratio-of-medians; no cross-run denominator. New optimized production path need not write global FP32 H1; diagnostic FP32 outputs kept outside primary timing. Runtime correctness must verify that no instrumented work changes output.
Record analytic edges, shared decode hits, loads/decodes, add count, TDP count, intermediate partial loads/stores, logical bytes, schedule setup, and actual output boundary. These are not measured DRAM bytes. Inspect generated assembly for pair spills and compiler removal of redundant operations. Optional existing supported PMU captures cycles/instructions/cache-misses/AMX_BUSY; missing events remain missing, not permission changes. Counters do not prove overlap.

## Execution sequence and resources
1. Freeze plan; isolated implementation and immutable build snapshot; fresh rules/specs/queue/handoff preflight.
2. Shared intel 1-node smoke: all numerical/work/bitwise gates, physical 32-core single-socket affinity.
3. Four fixed diagnostic graphs: mycielskian19, reddit, roadNet-CA, amazon0601. Request up to2 dedicated exclusive performance jobs; shared jobs may run separately, total nodes<=10. No dependence on reduced-width results.
4. After smoke and diagnostic correctness pass, remaining13 original eligible graphs evaluate fixed production candidates PACK_EPI, PAIR_INTERLEAVE_EPI, PAIR_SHARE_EPI under degree/qguard plus matched baseline/anchors. Detailed NOZERO/DIRECTPACK/EPILOGUE-only ablations restricted to fixed diagnostic4. Negative results retained; no graph-specific timing oracle claimed.
5. Reconcile Slurm exit status, source/binary hashes, all gates/work counts, raw timings, setup/amortization, and all3 handoffs. Write Chinese technical report and desktop txt if accessible under existing user document request.

## Interpretation
H1-H3 are strong-baseline implementation improvements, not independent first-ever innovations. H4 positive requires gain versus both canonical/PACK_EPI and identical-interleaving no-share control, associated with realized shared-decode count and no spills. Synthetic/structural statistics separate overlap from degree. If performance does not improve, report this boundary and do not reframe fewer logical bytes as measured memory savings. Common-sum reassociation and AMX aggregation are later candidates, not silently introduced into this controlled round.

## Pre-build fairness clarification
PAIR_INTERLEAVE and PAIR_SHARE use normal intrinsics and identical compiler options. Do not prevent normal compiler CSE with volatile assembly to weaken the baseline. Source-level decode requests are distinguished from emitted instructions; assembly audit records whether the compiler already shares loads. Smoke is correctness only; performance repetitions apply to real/synthetic performance cohorts.

A no-change state_identity wrapper is included in diagnostic ablations to expose code-generation/dispatch effects independently of the canonical P0 kernel.

## Pre-performance synthetic control
Reuse the three immutable regular128_reuse1/4/16 CSR fixtures from grouping-balance-20261007-01. Their N=524288, degree128, E and global source marginal counts are identical; exact local co-consumption differs. Treat separately from real17 and run fixed production matrix only, to test whether shared-decode benefit tracks actual matched heads. No fixture rewrites or timing-based selector.
