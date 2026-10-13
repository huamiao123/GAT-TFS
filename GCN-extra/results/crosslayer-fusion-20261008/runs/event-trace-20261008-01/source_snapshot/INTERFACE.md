# Event-trace experiment interface, 2026-10-08

Inference only, D=F=128, TR16, S64, M=FULL, FAST performance, existing truncation/ReLU/packing. No NUMA interleave or changed first-touch. Existing sources immutable. Graphs/weights/features/schedules and node cap follow user authorization. No runs before correctness passes.

## Ownership
Root: protocol, this interface, driver generator, build/job/submit/bootstrap and reports.
Kernel agent: trace_plan.hpp, trace_reducer.hpp, trace_kernel.hpp, check_trace_kernel.py, KERNEL_INTERFACE.md.
Runtime agent: trace_runtime.hpp, trace_suite.py, RUNTIME_INTERFACE.md.
Audit agent: analysis_inputs_01/analyze_trace.py and independent audit notes.
All remote files only experiments/event_trace_20261008 plus root-owned runs/results; no edit to originals or frozen snapshots. Before edits announce exact paths. Read wzh AGENTS, research skill, four v2 specs before implementing and use new-run preflight before any experiment.

## Methods (fixed order)
1 pack_epi_degree: existing state kind3, degree permutation.
2 pack_epi_qguard: existing state kind3, existing frozen qguard permutation (no tuning).
3 dyn_i: existing planned kind4 MODE1, adjacent degree pair plan.
4 dyn_s: existing planned kind4 MODE2, same plan.
5 exact_s: existing planned kind4 MODE4, same plan.
6 trace_i: new kind5, trace MODE0 (cached heads, independent expressed decodes).
7 trace_s_cached: new kind5, trace MODE1 (cached heads, share one decode for take3).
8 trace_s_rep: new kind5, trace MODE2 (read representative current index per event, shared decode).

All use unchanged PF1 next-neighbor helper, including each consumed row's own four hints. This round does not combine prior PF8 or OFF treatments. All planned and trace methods share one immutable adjacent Plan and one immutable TracePlan. Timed Method uses one fixed object/short name, configuration changed outside timing; check actual execution object vs labeled object. No per-method buffer relocation or graph-specific selection. PACK/qguard are strong baselines with their own original execution.

## Trace plan and API
Namespace gcn_extra_event_trace; includes planned_reducer.hpp/planned_kernel dependencies without modifying them.
struct TracePlan has vector<uint64_t> tile_offsets (tiles+1), vector<uint8_t> data, and scalar statistics/hash/cost fields. Build function:
TracePlan build_trace(const SourceGraphView&, const int* perm, const gcn_extra_pair_plan::Plan&, const char* graph);
Provide trace_validate(...) or call independent validation inside build_trace. Public plan fields agreed with runtime via KERNEL_INTERFACE.md.
Layout per tile: old window loop then original adjacent pair order (including odd tail), each record one uint8_t event count 0..128 followed by ceil(count/4) bytes. Low-to-high 2-bit codes 1,2,3; trailing unused bits zero. Code0 invalid for used events. No stored source IDs. Empty/exhausted row pair records still have count0; all-empty tile has no windows/records. Read packed payload byte once for each <=4-event group in source code, not a separate byte load every event. Runtime cursor starts at tile_offsets[tile], carries through windows/pairs; M must be FULL to avoid scope-position ambiguity.
The trace exactly reproduces frozen dynamic_pair_window current-head greedy (including duplicate/unsorted rows); it is NOT a new SCS/LCS optimizer. Two-pass parallel size/fill builder acceptable. Separate scalar validator must check each event against independent old greedy rule, row cursor bounds/order/exact exhaustion, shared head equality, header/padding/offset validity, totals vs old Plan dynamic_shared_heads and merge_steps. No sorting or deduplication.

Profile derives gcn_extra_pair_plan::Profile with TraceLogical extra counters (headers, payload bytes, events, take1/2/3, tile offset reads; exact public field names in KERNEL_INTERFACE.md). Preserve existing state/profile counters and per-thread aggregation; do not sum instrumentation into timing. New kernel:
template<bool P,bool B16,int MODE> void kernel(g,hb,w,out,perm,method,base_plan,trace_plan,Profile*);
Same Method type gcn_extra_state::Method; use original planned_kernel orchestration and new TraceReducer only. Reject non-FULL and non-S64 configurations rather than silently misreading trace. ACCURATE correctness allowed if code naturally supports hi/lo; performance FAST.
Cached modes retain original cached head variables and source-level current CSR reads=E, without volatile/barriers or forcing weak code. Optimizer may eliminate physical loads; label counters source-level. Rep reads E-Sdyn current indexes. Both trace share variants decode E-Sdyn; trace_i expresses E decodes. All add128E, same projection/window/partial/packing/PF counts. All trace modes runtime_head_comparisons=0, pair_shared_sources=Sdyn. Old dynamic modes have Plan.comparisons; trace plan stats must report this separately as inspector work.
For profile current-index/head/read helper and unchanged PF1 keep old counter meanings. Codegen audit checks baseline functions, trace paths, vector spills and whether trace_i decodes remain separate; do not invent volatile to manufacture a baseline.

## Runtime and gates
Namespace gcn_extra_event_trace_runtime::run_methods with original driver signature, matching prior pf_runtime run_methods signature. Old input generation and original TFS/MKL unchanged. Build schedules degree/qguard from frozen grouping code. One adjacent Plan based on degree, one TracePlan. Canonical numerical gates remain old thresholds plus bitwise FP32 single layer, e2e FP32, H1 BF16, e2e BF16. Actual timed object checked for those four boundaries. New trace shared outputs compared to old dyn_s as well; compare instrumentation outputs to production. Existing ReLU boundary checks remain.
Work counters independent of kernel: exact source additions/visits, decode/current-index counts, same projections/window/partial/PF packing by expected_work, trace event totals/data+header bytes/offset reads. Check same underlying plan addresses before and after measurements. Plan build/validation time, persistent bytes/capacity and RSS recorded outside compute timing. Graph hashes protect inputs; plans immutable during timing.
Timing 1 warmup+5 consecutive, ten forward/reverse alternating with one pre-switch warmup, all samples kept. No scratch copies needed because plan/order does not vary. 3 PMU repeats per method, unchanged full32-worker Region. Existing data class labels qualified. Eight methods, no MKL/TFS in new timing matrix; original references only correctness and separately inherited historical baseline.

## Suite scope
Reuse 13 immutable smoke fixtures and 3 frozen synthetic graphs from prior round. Smoke include trace malformed-code/offset rejection and original edge cases (empty, isolated, odd tile, duplicate/unsorted rows, degrees crossing64). Real diagnostics fixed Myc, Reddit, RoadNet-CA, Amazon. Shared intel correctness; shared/exclusive performance 3synthetic+4diagnostic each. Cap10 nodes incl pending, target<=6.
TRACE_START/PASS/COMPLETE and JSON status/completion expected. Runtime and suite must agree record names/counts; document all schemas in RUNTIME_INTERFACE.md for independent analyzer.
Fixed expansion candidate trace_s_rep. Expand17 only if all gates and, separately both timing protocols and both allocations, candidate beats BOTH pack_epi_degree and pack_epi_qguard by GM>=1.03 and wins>=3/4. No replacement with better variant or graph-specific winner. This is a mechanism experiment, trace encoding/inspector-executor not claimed novel by itself.
