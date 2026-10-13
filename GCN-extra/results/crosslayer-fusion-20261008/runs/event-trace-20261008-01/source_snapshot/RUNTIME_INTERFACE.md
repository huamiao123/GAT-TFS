# Event trace runtime and suite interface

Inference-only experiment, 2026-10-08. This document describes the new runtime/suite only; original input generation, first touch, TFS and MKL functions remain frozen. Root owns compilation, jobs, source freeze and final audit. This agent has not compiled or run the kernels.

## Entry points and ownership

`#include "trace_runtime.hpp"`; `gcn_extra_event_trace_runtime::run_methods` has exactly the prior pf_runtime driver signature. Binary: `trace_methods`. Suite CLI: `python3 trace_suite.py RUN BUILD MODE`. Environment set by suite: `TRACE_SMOKE=1/0`, `TRACE_PMU=0/1`. MODE is smoke, synthetic, or a comma-separated subset of the four frozen real diagnostics. Existing evidence_utils supplies process invocation and immutable result writing.

Read-only dependencies: plan_runtime.hpp, trace_kernel.hpp, and root-owned trace_opportunity.hpp. The latter exposes `gcn_extra_event_trace::report_opportunity(g,perm,base,trace,graph)`, called once before numeric gates/timing. Its diagnostic cost is not candidate-required setup cost.

Only these files are owned here: trace_runtime.hpp, trace_suite.py, RUNTIME_INTERFACE.md.

## Fixed methods and dispatch

Order is fixed:

| Method | kind | mode | schedule | plan_index |
|---|---:|---:|---|---:|
| pack_epi_degree | 3 | 0 | degree | -1 |
| pack_epi_qguard | 3 | 0 | qguard | -1 |
| dyn_i | 4 | 1 | degree | 0 |
| dyn_s | 4 | 2 | degree | 0 |
| exact_s | 4 | 4 | degree | 0 |
| trace_i | 5 | 0 | degree | 0 |
| trace_s_cached | 5 | 1 | degree | 0 |
| trace_s_rep | 5 | 2 | degree | 0 |

Every method has FULL scope, S64, PF1, FAST, BF16 output, 128->128->128, TR16 and fused first-layer ReLU/truncation. Methods 3--8 share one fixed actual Method object named `trace_exec`; kind/mode change only in select(), outside timing. The eight label objects are immutable after vector construction. The two PACK paths use their own original objects. Four bitwise boundaries compare the actual timed object with each label path before timing.

A single adjacent degree Plan and a single TracePlan are constructed once, remain const and resident during every method including both PACK baselines. Qguard uses the existing frozen schedule selector; no new tuning or graph-specific choice occurs. There is no scratch plan copy. Allocation identities and vector sizes/capacities are checked per select; independent full content hashes are checked once before and once after the whole measurement section. Hash scans are not repeated before each timing.

## Gate counts per graph

| Record family | smoke | performance |
|---|---:|---:|
| METHOD_CHECK | 32 | 32 |
| PAIRPLAN_EQUIV | 32 | 32 |
| TRACE_EXEC_EQUIV | 32 | 32 |
| TRACE_DYNS_EQUIV | 8 | 8 |
| PAIRPLAN_COUNTERS / WORK_GATE / PROFILE_GATE | 16 each | 16 each |
| TRACE_LOGICAL | 16 | 16 |
| TRACE_PLAN_HIST | 129 | 129 |
| TRACE_SLOT | 8 | 120 |
| TRACE_TIME | 0 | 120 |
| TRACE_PMU_WALL / METHOD_PMU | 0 | 24 each |
| METHOD_PMU_THREAD | 0 | 768 |
| TRACE_SMOKE_DUMP | 1 | 0 |
| TRACE_REJECT_CASE / TRACE_REJECT_AUDIT | 2 / 1 | 0 / 0 |
| TRACE_IMMUTABLE | 2 | 2 |

Unique main records: TRACE_CONFIG, TRACE_COMPLETE, TRACE_GATE_COMPLETE, TRACE_PLAN, TRACE_PLAN_SETUP, TRACE_PLAN_GATE, TRACE_SCHEDULE_SETUP, PAIRPLAN_SETUP, PAIRPLAN_STRUCTURE, PLAN_GATE, PAIRPLAN_SEMANTIC; root's TRACE_OPPORTUNITY is also one per graph. TRACE_METHOD has 8 rows. Frozen grouping helper emits its original GROUP_* records.

For all 27 planned packets (13 smoke + 14 performance), expected numeric/canonical/actual = 864 each, dyn_s equivalence = 216, work/profile/logical = 432 each, histogram = 3483, slots = 1784, time samples = 1680, PMU regions = 336, PMU thread rows = 10752, malformed rejection cases = 26. These are expected counts, not completed results.

Numeric gates remain kernel FP32 vs TFS .01, e2e FP32 vs TFS .01, e2e BF16 vs TFS .02, e2e BF16 vs source MKL FP32 .03; both relative L2 and normalized max must pass. Canonical and actual bitwise boundaries are kernel_FP32, e2e_FP32, H1_BF16, e2e_BF16. Both shared trace methods additionally compare all four boundaries with old dyn_s. Instrumented production BF16 output must equal non-instrumented output independently for each layer. The inherited 19-value ReLU/truncation semantic fixture remains mandatory.

## Independent trace and work validation

Runtime first independently replays original CSR greedy decisions for each tile/window/adjacent pair including odd tails and empty records. It does not invoke the production event iterator or original pair_take. It checks exact row exhaustion, each code, event header, tile offset, and unused padding bits. Separately counted expected events form validator_records. Builder's event_histogram forms builder_records; a third histogram obtained from encoded headers must agree. TRACE_PLAN_HIST contains all event_count=0..128, including zero-count bins. Required payload bytes are sum over records of ceil(event_count/4), never ceil(total_events/4).

For smoke only, TRACE_SMOKE_DUMP logs degree_perm and tile_offsets as comma-separated integers and data_hex as lowercase hex (`none` for empty data). The Python suite rereads the real immutable CSR header/rowptr/colidx and independently reconstructs every greedy event and byte boundary. Two fixed tiny-fixture rejection cases, used_code_zero and tile_offset_oob, use a valid two-row trace and a silent builder call, so no extra main graph records occur. trace_validate must reject both and produce a reason.

Let E be edges, S dynamic shared heads, X exact shared edges. All paths add 128E feature values. dyn_i reads E current CSR indexes and expresses E decodes; dyn_s reads E and decodes E-S; exact_s reads/decodes E-X; trace_i reads/expresses E; trace_s_cached reads E and decodes E-S; trace_s_rep reads/decodes E-S. Trace modes have zero runtime head comparisons; old dyn_i/dyn_s retain Plan.comparisons. trace_i allows natural compiler common-subexpression elimination; logical counters do not claim physical instruction counts.

PAIRPLAN_COUNTERS keeps current `csr_reads` separate from `prefetch_index_reads`; fixed PF1 next-neighbor index requests and its four hints per eligible edge are unchanged in trace_s_rep. Independent expected_work checks projection scopes/windows, direct packing, partial load/store bytes, packed bytes, AMX work and prefetch requests. Qguard work is checked against its own actual schedule. The frozen grouping helper enforces identical q64(degree) at each position and unchanged projection/state budgets; therefore common FULL/S64 work is also required to match across all eight methods. All five GROUP_AUDIT records are checked; padded_steps may differ and is not a FULL work invariant.

TRACE_LOGICAL is emitted for every method/layer, with headers, payload_bytes, events, take1/2/3, tile_offset_reads and matching expected_* fields. Nontrace methods have zeros. Trace offset reads equal nonempty_tiles, because only each nonempty tile start is read in the validated hot path. Metadata counts are source-level requests, not hardware-retired traffic.

## Timing, addresses and PMU

TRACE_TIME.kind is `consecutive` or `alternating`. Consecutive: one warmup then five samples per method in fixed order. Alternating: ten forward/reverse rounds, one pre-switch warmup before each measured sample. Every sample is kept. TRACE_SLOT.phase is correctness, consecutive, alternating or pmu; repeats are 0 for correctness/consecutive, 0..9 alternating, 0..2 pmu. Serial is contiguous from zero. Slot count is 8+8+80+24=120 for performance. No MKL/TFS path enters the new timing matrix.

All *_ptr slot fields are globally fixed except execution_method_ptr (three objects total) and schedule_ptr (degree/qguard). Each method's execution/schedule pointers are fixed and actual-equivalence pointers must match. Size/capacity fields are globally fixed. Slot hash_checked=0 explicitly distinguishes address checks from TRACE_IMMUTABLE before/after content checks with hash_checked=1.

Three PMU repeats for every method use the inherited full32-worker Region. The suite requires all 32 workers, five events, nonzero enabled/running time, valid scaling, complete start/stop membership, and exact summed raw counters plus numerically consistent scaled totals. AMX raw event remains 0x02b7. Successful counting does not establish overlap or a cache cause.

## Setup, bytes, status and expansion

TRACE_PLAN_SETUP reports build_ms, validate_ms, total_ms and persistent logical/allocated vector bytes; object_bytes separately includes object-resident histogram/state. TRACE_PLAN adds independent_audit_ms and actual vector size/capacity/offset element bytes. TRACE_COMPLETE reports process max_rss_kib. Existing schedule/base-plan setup remains separately visible. Common resident trace memory is recorded even for PACK; that does not make it a deployment requirement of PACK. Opportunity diagnostic_ms is reported separately and must not be added to required candidate setup.

Suite output includes per-graph raw methods.log, parsed JSON/CSV, status, binary/input hashes, environment, all combined CSVs, PASS-only raw-derived summary.csv, suite_status.json and completion.json. A failed gate or process preserves raw evidence and produces FAILED, never partial PASS. No result is silently removed as an outlier. TRACE_START/PASS/FAILED/SUITE_COMPLETE are suite lifecycle messages; TRACE_COMPLETE is the binary's per-graph gate/timing completion record.

Expansion candidate is fixed trace_s_rep. Four independent cohorts are shared/consecutive, shared/alternating, exclusive/consecutive and exclusive/alternating. Within every cohort it must beat BOTH pack_epi_degree and pack_epi_qguard with geometric mean speedup >=1.03 and >=3 wins among the four diagnostics, after all gates pass. No replacement by another variant, per-graph oracle, cross-run denominator or best-of-protocol selection is allowed. Final expansion decision belongs to the root's independent analyzer, not this suite.

## Runtime emitted scalar schemas

Below lists literal runtime printf schemas; inherited helper/PMU fields retain their frozen definitions. Equivalence records additionally use a common dynamic-tag writer with fields graph, method, boundary, reference, kind, mode, execution_kind, execution_mode, execution_name, execution_method_ptr, scope, accurate, bitwise, pass.

`TRACE_PLAN_HIST graph=%s event_count=%u builder_records=%llu validator_records=%llu pass=%d`

`TRACE_PLAN graph=%s tiles=%zu nonempty_tiles=%llu windows=%llu source_edges=%llu events=%llu shared_heads=%llu comparisons=%llu take1=%llu take2=%llu take3=%llu headers=%llu payload_bytes=%llu data_bytes=%zu offset_bytes=%llu plan_bytes=%llu allocated_bytes=%llu data_size=%zu data_capacity=%zu offsets_size=%zu offsets_capacity=%zu offset_element_bytes=%zu object_bytes=%zu plan_fnv64=%llu payload_hash=%llu independent_audit_ms=%.12g order_preserved=1 exact_exhaustion=1 padding_zero=1 header_hist_checked=1 pass=%d`

`TRACE_SMOKE_DUMP graph=%s degree_perm=`

`TRACE_REJECT_CASE graph=%s case=%s valid_fixture=1 rejected=%d reason_present=%d pass=%d`

`TRACE_REJECT_AUDIT graph=%s cases=2 rejected=%u valid_fixture=1 pass=%d`

`TRACE_LOGICAL graph=%s method=%s layer=%d headers=%llu payload_bytes=%llu events=%llu take1=%llu take2=%llu take3=%llu tile_offset_reads=%llu expected_headers=%llu expected_payload_bytes=%llu expected_events=%llu expected_take1=%llu expected_take2=%llu expected_take3=%llu expected_tile_offset_reads=%llu pass=%d unit=source_level_not_retired`

`PAIRPLAN_COUNTERS graph=%s method=%s layer=%d scope=%d accurate=%d profile_path=production_BF16_epilogue unit=source_level_not_DRAM source_visits=%llu feature_adds=%llu decode_rows=%llu shared_heads=%llu projection_scopes=%llu windows=%llu amx_tdp=%llu partial_zero_bytes=%llu partial_load_bytes=%llu partial_store_bytes=%llu packing_load_bytes=%llu direct_rows=%llu packed_bytes=%llu prefetch_requests=%llu prefetch_index_reads=%llu csr_reads=%llu comparisons=%llu order_index_reads=%llu exact_mask_bytes_read=%llu exact_pair_flags_tested=%llu exact_pair_windows=%llu exact_pair_edges=%llu exact_shared_edges=%llu runtime_head_comparisons=%llu relu_elements=%llu expected_hits=%llu expected_decode=%llu expected_csr=%llu expected_scopes=%llu expected_windows=%llu expected_loads=%llu expected_stores=%llu expected_packed=%llu expected_direct=%llu expected_prefetch=%llu`

`PAIRPLAN_WORK_GATE graph=%s method=%s layer=%d pass=%d`

`PAIRPLAN_PROFILE_GATE graph=%s method=%s layer=%d bitwise=%d pass=%d`

`TRACE_SCHEDULE_SETUP graph=%s phase=all_five_frozen_schedules ms=%.12g included_in_primary=0`

`TRACE_CONFIG graph=%s N=%d E=%u D=128 H=128 F=128 S=64 M=FULL R=64 TR=16 methods=8 pf=1 warmup_consecutive=1 warmup_each_alternating_switch=1 consecutive=5 alternating=10 numa=default smoke=%d final_output=BF16 copy_in_compute=0 purpose=mechanism_not_deployment fixed_expansion_candidate=trace_s_rep trace_resident_for_all_methods=1`

`TRACE_METHOD graph=%s method=%s kind=%d mode=%d scope=0 schedule=%s schedule_index=%d plan_index=%d epilogue=1 accurate=0 pf=1`

`TRACE_IMMUTABLE graph=%s phase=%s base_hash=%llu expected_base_hash=%llu trace_hash=%llu expected_trace_hash=%llu hash_checked=1 pass=%d`

`TRACE_SLOT graph=%s method=%s phase=%s repeat=%d serial=%llu kind=%d mode=%d execution_kind=%d execution_mode=%d execution_name=%s execution_method_ptr=%p schedule=%s schedule_ptr=%p base_plan_ptr=%p tiles_ptr=%p masks_ptr=%p trace_plan_ptr=%p offsets_ptr=%p data_ptr=%p tiles_size=%zu tiles_capacity=%zu masks_size=%zu masks_capacity=%zu offsets_size=%zu offsets_capacity=%zu data_size=%zu data_capacity=%zu h0_ptr=%p w1_ptr=%p w2_ptr=%p wv1_ptr=%p wv2_ptr=%p h0b_ptr=%p h1_ptr=%p h1b_ptr=%p h2b_ptr=%p h2f_ptr=%p perm_ptr=%p row_ptr=%p col_ptr=%p hash_checked=0 stable=%d pass=%d copy_in_compute=0`

`TRACE_GATE_COMPLETE graph=%s pass=1`

`TRACE_TIME graph=%s method=%s kind=consecutive repeat=%d order=%zu warmups_before_series=1 ms=%.12g`

`TRACE_TIME graph=%s method=%s kind=alternating repeat=%d order=%zu warmups_before_measure=1 ms=%.12g`

`TRACE_PMU_WALL graph=%s method=%s repeat=%d ms=%.12g`

`TRACE_COMPLETE graph=%s pass=1 methods=8 smoke=%d slot_checks=%llu max_rss_kib=%ld`

