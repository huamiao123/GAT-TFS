# Fixed-frontier source/partial capacity certificate, 2026-10-08

This is a graph-structure experiment, not an AMX runtime benchmark. The target
inference remains 128 -> 128 -> 128 with existing ReLU and BF16 boundaries.
No feature allocation, first-touch change, NUMA interleave, training change,
desktop output, baseline replacement, or GitHub publication is authorized here.
Use one shared intel node, one CPU, 16 GiB, 25 minutes. Count requested nodes,
including pending jobs, against the user-authorized maximum of ten.

## Question and model

The first experiment found that periodic ordered-prefix consumption often costs
more state and many more partial reloads than consuming a row once all inputs
are available. Can any mixture of whole sources and destination partials escape
the large frontier for Mycielskian19 and Reddit under node-ID producer order?

A producer batch has 16 consecutive node IDs. CSR is sorted, as fully verified
in the SHA-bound parent experiment. At completed producer rank p, each produced
source has size 256 B, and each independent FP32 partial for an unfinished
destination has size 512 B. Contributions may reside in one of these two forms.
No recomputation, spilling to an uncounted state, cross-destination algebraic
sharing, alternative precision, or separate edge-contribution representation
is allowed in this restricted model. Empty destinations are skipped. Ready
rows and runtime scratch are excluded from the lower bound.

Choose actual CSR occurrences joining produced sources to unfinished consumers,
using each source at most once and each consumer at most twice. For M selected
edges, every legal source/partial cover has cost at least 256*M bytes: a source
can cover at most one selected edge for 256 B; a partial can cover at most two
for 512 B. This is a standard weighted-cover dual certificate, not a novelty
claim. Any legal matching is sufficient; failed greedy saturation does not
prove optimization headroom. Native optional-source counts are not independently
certified and must not be reported as an optimality certificate.

## Frozen experiment

Only Mycielskian19 and Reddit are included. Select frontiers before running the
new algorithm from ceil(N/16): quarter, half, three quarters (integer floors),
and the parent node-ID row-complete peak-after-drain rank and its two neighbors.
Clamp to [1,B] and deduplicate. Preserve all sampled results, not only the best.
The maximum verified lower bound at these frontiers bounds the peak of any
allowed representation at the fixed producer order. Compare it with the prior
constructive row-complete post-drain peak. This does not bound all producer
orders, all state representations, actual cache capacity, or elapsed time.

## Gates

Before real inputs: compile the frozen parent analyzer and the new builder;
run five small valid graph fixtures and five malicious certificate mutations
(duplicate source, destination capacity exceeded, unproduced source, completed
destination, invalid offset). The independent Python verifier must accept the
former and reject the latter for the intended invariant.

For each real graph: full file SHA256 must match the parent, with file identity
unchanged during processing. Read-only CSR mapping only. Bind parent structure
JSON to its status hash. Verify every certificate record directly against CSR,
including source rank, destination completion, source uniqueness, destination
capacity, row/offset ownership, and the source column at that occurrence.
Persist commands, exit codes, raw certificates, logs, hashes, verifier reports,
Slurm allocation/accounting, and preflight authority snapshots. Require both
completion PASS and Slurm COMPLETED 0:0 before interpreting results.

All writes stay in the announced experiment/run/result directories under wzh.
Reread rules/specs for this experiment; preserve the first experiment unchanged.
Synchronize all three inference handoffs at preparation, submission, execution,
and analysis. Do not develop a complex runtime merely because a certificate
is weak; first establish that a stronger algorithmic opportunity exists.

## Retry 02
Attempt 01 job 10899017 failed at link time before self-tests or inputs. GCC 8 libstdc++ requires explicit -lstdc++fs. This retry changes only that link option and unique artifact paths. The failed run is preserved.
