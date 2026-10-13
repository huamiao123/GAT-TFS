# Event trace kernel API, frozen implementation interface

Namespace `gcn_extra_event_trace`; headers `trace_plan.hpp`, `trace_reducer.hpp`, `trace_kernel.hpp`.

TracePlan public fields:
- `vector<uint64_t> tile_offsets` (tiles+1), `vector<uint8_t> data`;
- `array<uint64_t,129> event_histogram` (record event counts 0..128);
- uint64_t `source_edges,shared_heads,merge_steps,comparisons,nonempty_tiles,windows,headers,payload_bytes,data_bytes,offset_bytes,plan_bytes,allocated_bytes,fnv64,take1,take2,take3`;
- double `build_ms,validate_ms,total_ms`.

`build_trace(g,perm,base_plan,graph,bool emit_log=true)` returns TracePlan. Normally emits one TRACE_PLAN_SETUP and one TRACE_PLAN_GATE. The optional false argument is for quiet synthetic malformed-plan tests. `trace_validate(g,perm,base_plan,trace,std::string* reason=nullptr)` independently validates and returns bool without logging/mutating the plan. `trace_hash(trace)` hashes offset bytes (explicit low-byte-first) and data bytes. Runtime may call it before/after to check immutability.

Each tile contains records in the existing window/pair order. Record header is one byte count0..128; payload is ceil(count/4) bytes, two-bit codes from low bits, unused bits zero. Empty pair-windows retain count0 headers; all-empty tiles have no records. No source IDs are stored. Offsets and data are public for smoke-only dump. Runtime owns 129 TRACE_PLAN_HIST output lines comparing builder histogram to its own CSR replay; builder does not output that label.

TracePlan build uses parallel size and fill passes. Builder uses old pair_take; a separate scalar validator independently reads CSR and rederives all decisions, counts, byte bounds, exhausted/odd rows, trailing zero bits, histograms and old Plan structural totals. plan_bytes is the persistent vector payload data_bytes+offset_bytes, not full object/RSS. allocated_bytes is vector capacity bytes. TRACE_PLAN_SETUP also records object_bytes, which includes scalar statistics and histogram. Build cost includes shape checking, size/fill and content hashing; validate_ms is a separate full scalar validation/hash pass. Total includes both; no preprocessing is part of kernel timing.

`Profile : gcn_extra_pair_plan::Profile` adds `TraceLogical trace_logical`; fields are uint64_t `headers,payload_bytes,events,take1,take2,take3,tile_offset_reads`. Participating kernel workers use this derived type and main result merges all fields. tile_offset_reads counts one production start-offset load per nonempty tile; validated end offsets are not read in the hot path. All counts are source-level logical requests, not retired loads or DRAM traffic.

`kernel<P,B16,MODE>(g,hb,w,out,perm,state_method,base_plan,trace_plan,Profile*)` rejects scope!=FULL or window!=64. MODE0=cached independent expressions; MODE1=cached shared decode; MODE2=representative index/shared decode. Cached modes express E current-index reads using the original current-head initialization/update; MODE2 expresses E-Sdyn. MODE0 expresses E full source decodes; MODE1/2 express E-Sdyn. No volatile, compiler barriers or forced redundant loads. Compiler CSE/load elimination is allowed and must be checked in assembly. All trace modes preserve old per-row prefetch calls, source additions, row order and partial/AMX orchestration; pair_comparisons/runtime_head_comparisons are zero while pair_shared_sources=take3=Sdyn. Trace event count is E-Sdyn in every mode.

Preserved counts: headers=sum pair-window records; payload_bytes=sum ceil(record_count/4), NOT ceil(total_events/4); events=take1+take2+take3; source_edges=take1+take2+2*take3. Counts and format histograms need independent runtime and external smoke validation. Production trusts an already validated immutable plan; it does not rescan trace validity inside timed regions.

Static checker invocation: `python3 check_trace_kernel.py SOURCE_SNAPSHOT`. It prints one JSON object or exits nonzero. It compares orchestration source exactly after undoing declared trace plumbing and FULL guard, and runs small Python sequence/encoding/malformed models. It does not compile C++, scan real graphs, run AMX or establish numerical/performance acceptance.
