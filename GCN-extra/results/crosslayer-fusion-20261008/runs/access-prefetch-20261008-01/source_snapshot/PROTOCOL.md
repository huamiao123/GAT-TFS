# Access timing, order and sharing: registered protocol 2026-10-08

Status: prospective. No timing-based candidate selection.
Inference-only GCN-extra; TFS-Train specs are read for workspace authority, not a change to training semantics.
User authorizes experiments on shared intel and exclusive intel_expr, cap10 nodes including pending. This round requests at most six at once. No NUMA interleave or changed feature first-touch. Dimensions128->128->128, unit edges, original seeded H/W, original MKL/TFS immutable.

Question: under fixed arithmetic and fixed per-row prefetch target multisets, does prefetch issuance timing interact with pair visit order and exact shared decode?
Matrix: adj / fixed pairs_s1 seed20261007 salt0x504149524f524452; Scalar MODE0 / ExactS MODE4; OFF / PF1 / PF8-MATCH. Twelve planned methods plus pack_epi_degree. Keep pair membership, direction, odd tail, exact masks, AMX row slots, CSR addition order, S64, M=FULL, fused PACK/EPI, precision, threads32 and same-slot addresses/capacities.
OFF disables only existing software feature prefetch; hardware prefetch configuration unchanged.
PF1 uses old k+1<row_scope_end. PF8-MATCH prelude targets local1..min(7,L-1), ONCE per row per projection scope, then steady k+8<L. Both target multisets1..L-1; counts sum max(L-1,0) and four T0 hints per target. ExactS retains separate prefetch for both rows. PF8 is a preregistered probe, not an optimal distance claim. No clipped8, hint sweep or seed search in this round.
Same target count does not imply same instruction scheduling, burst shape, cache fills or DRAM bytes. Prelude cost is part of treatment.

Gates: preserve all old numerical, canonical/actual execution bitwise, work, semantic and same-slot gates. PF1 identity is checked against old planned path; arithmetic counts equal within MODE; PREFETCH0 counts0, PREFETCH1/8 equal old formula. Explicit target-multiset audit handles short rows/scope/window boundaries; never repeat prelude every S64 window. Preserve empty-tile special paths.
Smoke reuses13 immutable prior fixtures, plus any narrowly required prefetch boundary audit. Correctness precedes performance. Three frozen synthetic graphs and benchmark diagnostic4 {mycielskian19,reddit,roadNet-CA,amazon0601}; Myc is constructed, legacy data_class=real is a cohort label only.
Use existing five consecutive and ten forward/reverse alternating samples, existing pre-switch copy/warmup, full32-thread PMU three repeats. Copy/setup outside compute timing measured separately. Retain every sample/outlier/failure and all old snapshots. PMU unchanged this round.

Fixed expansion candidate: s1 ExactS PF8-MATCH versus PACK, same job. Full17 allowed only if all correctness/identity/work gates pass and, separately in BOTH timing protocols and BOTH allocation classes, diagnostic4 geometric mean>=1.03 and wins>=3/4. Otherwise no full17 expansion under this protocol.
Main effects use matched same-mode/same-order/same-prefetch ratios. Report interaction as descriptive ratio of medians, not statistical significance. No claims of AVX-AMX overlap, DDR/HBM bytes, or cache-level cause from existing counters. A faster synthetic case alone is not a benchmark-general improvement or journal contribution.

Record source/binary/input/rule hashes, original recovery, compiler, bindings/NUMA, allocations, raw output and costs. Three inference handoffs updated through record_event.py after build, submission, smoke, performance and analysis. Never touch others' jobs.
Build/job scripts freshly read and hash current mandatory rules/specs. Previous tool startup failures happened before SSH; no new jobs or remote writes occurred during that outage. Environment restored2026-10-08. GCN-extra root is a source tree without .git; use immutable snapshots/hashes as provenance, do not initialize git.
