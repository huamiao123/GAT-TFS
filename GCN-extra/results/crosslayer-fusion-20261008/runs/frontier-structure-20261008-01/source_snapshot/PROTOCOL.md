# Ordered cross-layer frontier structure — 2026-10-08

This is a structure-only, inference-only GCN-extra study. It does not execute a new AMX kernel or estimate measured speedup from logical bytes. The user authorizes continued experiments, shared intel / exclusive intel_expr, at most ten requested nodes including pending, and no more desktop TXT delivery. This study requests ONE shared intel node, ONE CPU, 16 GiB, at most 25 minutes. No NUMA interleave, physical graph/feature permutation, changed feature first-touch, environment modification, or writes outside wzh on the server.

Question: with fixed 128→128→128 and 16-row producer batches, can choosing the timing of ordered prefix consumption expose a useful tradeoff between retained BF16 source values and FP32 partial resumptions? Native DegreeSort, frozen qguard, and node-ID production are controls. A periodic delayed consumer and row-complete-only consumer are deliberately simple strong controls, not new contributions.

## Definitions and scope

Read original CSR including duplicates, unsorted entries and whatever self loops the input actually contains. Do not add normalization, self loops, reverse edges or deduplicate. Report observed input structure. First-layer H1 production is abstracted as one complete 16-row batch at a time, ranks 1..B. Completed H1 has already crossed its original ReLU/BF16 boundary; never submit incomplete first-layer values. This is a serial production schedule model, NOT observed 32-thread wall-clock production order.

One source value occupies 256 B; one in-progress destination partial occupies 512 B. A completed aggregate is immediately represented as 256 B BF16-ready input, preserving its conversion boundary. Any ready rows may form a 16-row AMX batch; final tail is separately counted. Empty destination rows are rank-0 zero-complete, do not enter the ready queue, and are counted in rows_skipped_zero. This uniform nonempty-row projection accounting is NOT claimed equal to original mixed empty/nonempty tile execution counts. No arithmetic, BF16 correctness or trained-model accuracy claim follows from this graph-only model.

For row d with CSR occurrences j_l and production ranks r_j, earliest ordered-prefix release is e(d,l)=max_{h<=l}r_jh. A source has forced-retention end t_j=max_{d,l:j_l=j} e(d,l), initialized to r_j. Forced information at a batch boundary is 256*count(r_j<=p<t_j). This bound assumes full source values, one ordered-prefix partial per destination, no recomputation, spill optimization, alternative per-edge buffers or data-value factorization. It is not cache residency or DRAM traffic.

Consumers: periodic global prefix draining every k batches for k=1,4,16,whole, with final forced drain; plus row_complete, consuming each nonempty destination once when all its sources have been produced. Legal prefix order and exactly-once occurrences are invariant. Whole is not a deployed optimal policy. Source values remain until their last actual occurrence is consumed. Empty source rows may still have consumers and must not be dropped based on an unverified value assumption.

Record per-plan source/partial/ready boundary states and combined peaks, before/after production/drain boundaries, source lifetimes, partial segments/resumptions, full ready batches/tail, permutation identity, fixed graph/input identity, and first-layer tile work proxies. Internal allocation/conversion/drain transients, input X, weights, CSR, metadata, registers and producer scratch are excluded from these boundary metrics and must not be silently absorbed into a total memory claim. Partial save/reload payload is a stipulated whole-vector software traffic count (512+512 B), not actual cache traffic or an algorithmic lower bound.

## Gates and evidence

Before compile/run, reread current wzh AGENTS.md, tfs-research-engineering/SKILL.md, all four active training-v2 specs and the GCN-extra contract. The user's inference scope, 128-wide main shape and ten-node maximum supersede historical training-only scope / five-node text, without changing frozen sources. Save current hashes and scheduler state. All writes remain in declared study/run/result directories and three inference handoffs.

Compile a standalone analyzer with the same Intel compiler module as the frozen study. Reuse grouping_schedule.hpp byte-for-byte. Native degree is iota followed by std::sort using degree-only comparator, WITHOUT an invented tie break. Compare degree/qguard FNV permutations with frozen event-trace logs before interpreting any real graph.

Before real graphs: independent Python cursor simulation, directed fixtures and randomized cases check all policy counts and boundary traces. Include duplicate/unsorted adjacency, empty rows, tails and zero-consumer sources. Reject malformed headers, lengths, row bounds and out-of-range indices before indexing. Preserve raw fixture inputs, binary output, failures and exit codes. A failed gate stops real-data interpretation.

Fixed cohort: mycielskian19, reddit, roadNet-CA, amazon0601, inherited graph manifests and SHA-256. Mycielskian is a constructed graph, not mislabeled a real-world dataset. No expansion, per-graph winning policy substitution, new sorting heuristic, performance sweep or tuning based on results in this protocol.

Interpretation gate: only proceed toward a new runtime if this cohort shows a structural opportunity beyond simple ID+delayed/row-complete policies. Negative results remain evidence; no arbitrary source-memory ratio is advertised as speedup. Counterexample graphs explain models, not a universal constant-state guarantee. Any future performance comparison needs a separate numerical/work and timing protocol.

Protocol/source snapshot, exact commands, compiler/binary hashes, input hashes, node/affinity, raw JSON, oracle outputs and Slurm accounting are retained. Update ALL_EXPERIMENTS.md, EXPERIMENT_ISSUES.md and PROJECT_PROGRESS.md after preparation, submission, completion and analysis. No desktop writes or Git push.
