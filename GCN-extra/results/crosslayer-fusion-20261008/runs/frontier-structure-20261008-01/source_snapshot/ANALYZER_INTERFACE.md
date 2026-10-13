# Structure-only analyzer contract

CLI: `frontier_structure --csr PATH --out JSON --graph LABEL --orders all --checkpoints 1,4,16,whole [--batch-out CSV] [--emit-plan]`.
Only `nodeid`, `degree`, `qguard` are supported; `all` expands in that order. Checkpoints are fixed at 1,4,16,ALL, plus `row_complete`.

Input is read-only mmap. Header is exactly 36 little-endian bytes: uint32 0,0,2; uint64 n,n,E; followed by (n+1) uint32 row offsets, E uint32 column indices, E float32 values. File length, all row offsets and every column are validated. Feature values and edge values are never read. Original CSR order and duplicate occurrences remain unchanged. `unsorted_rows` counts rows containing a descending adjacent pair; `adjacent_duplicate_occurrences` counts adjacent equal IDs only, not all duplicates.

Rank is 1..B, B=ceil(n/16). First-layer sources in one producer batch arrive together. Empty second-layer rows complete at rank0 under an explicit zero/bias-free fastpath and never enter ready grouping. `rows_skipped_zero` records them. Nonempty ready rows can be grouped arbitrarily in batches of16; a final nonempty tail is padded to16. This is abstract accounting, not original-kernel projection counting. Sources with no CSR consumers are discarded immediately and are excluded from retained-source counts, even at the after-produce boundary.

`checkpoint_k`: after every k producer batches, drain each row's entire newly legal CSR prefix; last batch always drains. ALL waits until B. `row_complete`: each row consumes all its CSR occurrences once, at its final producer's rank.

Each source-last-use is the maximum actual edge consumption rank. An unused source has last_use_rank0. Source postdrain lifetime is [rank,last_use). Partial lifetime is [first_consume,last_consume). Rows with one segment have no persistent partial. Final-ready tail is `(cumulative_nonempty_completed % 16)` until final tail flush.

Three boundaries per producer batch: `before_batch` is previous postdrain; `after_produce` adds new sources that have consumers, with old partial/ready; `after_drain` applies all consumption, releases and ready grouping. Their maxima are NOT exact intradrain peaks. Per-row feature temporary, first-layer aggregation buffers, AMX operands/TMM state, metadata, allocator overhead, cache associativity and multithread ownership are excluded. Retained information is not necessarily cache-resident. No DRAM traffic, runtime, overlap, speedup or innovation claim follows.

State bytes = source_rows*256 + partial_rows*512 + ready_rows*256. Logical partial loads and stores each equal segments minus nonempty rows, assuming each pause spills its FP32 partial and each resume reloads it. They are logical cache/interface bytes, not DRAM bytes. Logical source operand bytes = E*256, with no claim about actual load sharing or physical traffic.

Batch-area is the discrete sum over postdrain state persisting from rank p to p+1, p=1..B-1. At B, sources/partials are already zero and the final ready tail is immediately flushed, so it adds no interval area. The after_drain boundary at B still reports that tail for peak auditing. Top-level full_hidden_reference_bytes=256*n is a reference, not a measured baseline. Peak ranks use the earliest strictly positive peak, or0 if all boundaries are zero.

Output schema is `frontier-structure-v1`, independent JSON via --out; frozen grouping helper diagnostic lines remain on stdout. `analysis_pass=true` is written only after successful complete analysis. A failed process can leave incomplete output which must never be accepted. Existing output paths are refused.

Top level contains n,e,file_bytes,batch_rows,rank_base,rows_skipped_zero,no_consumer_sources,unsorted_rows,adjacent_duplicate_occurrences,model,orders,analysis_wall_ms,analysis_pass.

Each orders[] entry has name, permutation_fnv64 (frozen helper's per-element FNV recurrence, not byte-wise FNV), producer_batches,pure_forced,methods. pure_forced is independent of delayed consumption: peak_source_rows,peak_bytes,source_row_batch_area,byte_batch_area,eager_segments. Its intervals end at the maximum prefix-max production rank over all CSR occurrences of each source.

pure_forced also has peak_batch. first_layer_work_proxy contains sum_tile_max_degree,sum_tile_max_degree_x16,sum_tile_s64_windows,sum_tile_degree_spread,max_tile_degree,nonempty_producer_tiles,actual_kernel_work_claim=false. These are O(N) structural proxies; they are not predictions of FULL's actual AMX arithmetic or latency. Methods also include peak_before_batch_rank,peak_after_produce_rank,peak_after_drain_rank.

Each methods[] entry has name,checkpoint_batches (0 for ALL),row_complete,logical_edges_consumed,segments,resumes,nonempty_rows_completed,partial_stores,partial_loads,logical_partial_store_bytes,logical_partial_load_bytes,logical_source_operand_bytes,full_output_groups,final_tail_rows,final_tail_groups,projection_accounted_rows,peak_boundary_source_rows,peak_boundary_partial_rows,peak_boundary_ready_rows,peak_before_batch_bytes,peak_after_produce_bytes,peak_after_drain_bytes,peak_all_boundaries_bytes,source_row_batch_area,partial_row_batch_area,ready_row_batch_area,postdrain_byte_batch_area,post_final_flush_state.

--emit-plan (n<=4096 only) adds per-order permutation,rank,forced_release_rank,row_completion_rank,forced_postdrain_source_rows (index0=0); each method adds source_last_use_rank,first_consume_rank,last_consume_rank,segments_per_row,batches. batches[] uses p plus before_batch,after_produce,after_drain, cumulative_edges_consumed,cumulative_segments,cumulative_resumes,cumulative_completed_rows,full_output_groups,ready_tail_rows. Every state object uses source_rows,partial_rows,ready_rows,bytes. Empty rows have first/last0 and segments0.

--batch-out emits 3 boundary rows per producer batch per method. Header: order,method,p,phase,source_rows,partial_rows,ready_rows,bytes.

Analysis complexity per order is O(E+N) with a fixed factor for the four checkpoint policies and one additional row-complete pass. Storage is O(N+B); no per-edge trace is materialized and no batch scans the whole graph. Frozen degree sorting is O(N log N), and frozen qguard preprocessing is separate from this complexity claim. Degree sort intentionally has no tie-break; build with the same compiler/standard-library configuration and independently compare hashes to the frozen native run.
