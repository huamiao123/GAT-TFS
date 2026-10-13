# Matched-prefetch runtime interface

Entry: `gcn_extra_prefetch_runtime::run_methods`, same signature as frozen factorial runtime. Include `pf_runtime.hpp`. Binary: `prefetch_methods`. Suite args: `pf_suite.py LOGDIR BUILDDIR smoke|synthetic|comma-separated-diagnostic-names`.

Fixed method order: pack_epi_degree, adjScalarOff, adjScalarP1, adjScalarP8, adjExactSOff, adjExactSP1, adjExactSP8, s1ScalarOff, s1ScalarP1, s1ScalarP8, s1ExactSOff, s1ExactSP1, s1ExactSP8. Modes 0 and 4; PF 0, 1, 8. PACK uses unchanged state kernel and its original distance 1. Two immutable plans: adj_degree and pairs_s1. Seed 20261007, salt 0x504149524f524452. Original Method is unchanged; the local derived Method adds PF selection. All 12 planned methods execute through one fixed derived Method object and same scratch plan storage. Dispatch occurs before entering the selected kernel.

All old FACT labels retained; FACT_METHOD/FACT_SLOT/FACT_EXEC_EQUIV add pf (and execution_pf). FACT_CONFIG modes=0,4 pf_policies=0,1,8 prefetch_matching=ordered_targets_count_hint. Original four numeric gates, four canonical equalities, four actual execution equalities, two work/profile gates per method retained. Production arithmetic, partial state, projections, and activation budgets stay fixed. OFF alone has zero prefetch counters. PF1/PF8 prefetch_index_reads and four-T0-cacheline requests match.

PF_IDENTITY: 16 rows/graph, four PF1 methods x four boundaries (kernel_FP32, e2e_FP32, H1_BF16, e2e_BF16). Fields graph/method/boundary/pf=1/mode/bitwise/pass/reference=frozen_planned_kernel. The reference is the frozen original planned kernel, separate from performance method matrix.

PF_IDENTITY_PROFILE: 8 rows/graph, same four methods x layer 1/2. fields=37 means all 20 projection Counters, 10 Logical, 7 PlanLogical fields compared memberwise; timing fields deliberately excluded. Also mismatches, bitwise, pass, prefetch_requests/reference_prefetch_requests, prefetch_index_reads/reference_prefetch_index_reads.

PF_TARGET_CASE: 36 rows/graph; degrees 0,1,7,8,9,63,64,65,127,128,129,257 crossed with scope 0(FULL),64,256, window=64. Small CSR is deliberately unsorted with duplicate source indices. PF8 enumeration invokes the same target visitors as production. PF1 production preserves the frozen helper; its enumeration uses that helper's formula, explicitly recorded as pf1_frozen_formula=1. Checks ordered CSR positions against independently generated [scopeStart+1,scopeEnd), then hashes each source's four cacheline byte addresses in request order. OFF emits none. Per-case fields: degree/scope/window/off_targets/p1_targets/p8_targets/expected_targets/p1_address_hash/p8_address_hash/matched_ordered_positions/matched_ordered_addresses/pass.

PF_TARGET_AUDIT: one row/graph; cases=36, p1_targets/p8_targets/prefetch_index_reads totals; prefetch_requests=4*targets; off_targets=0; cross_window_targets positive; matched_ordered_positions/matched_ordered_addresses/pf8_production_visitor/pf1_frozen_formula/pass all 1.

Timing and PMU retain 1 warmup+5 consecutive, 10 alternating forward/reverse samples with one warmup at every switch, 13x3 PMU regions with 32-thread coverage. Plan copying is outside compute timing. Smoke has no measured timing or PMU. Frozen input manifests and old statuses/hashes are reused verbatim; no graph regeneration or candidate selection occurs in suite.

No compilation or cluster jobs were performed by the runtime author. Root owns preflight, build, smoke and performance submission. Frozen old files were never edited.
