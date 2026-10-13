# Selective H1 regeneration cost audit, 2026-10-08

This is a read-only graph cost audit for the fixed 128 -> 128 -> 128 path.
It does not change the canonical TFS kernel, feature allocation, NUMA policy,
first-touch, numerical boundaries, or timing protocol. It does not claim a
speedup or a new algorithm. The question is whether discarding a BF16 H1 row
after its first second-layer consumer and regenerating it for later consumer
batches can plausibly pay for its extra AVX and AMX work.

For source node j, let C_j be the set of distinct row-completion producer ranks
of second-layer destination rows that consume j. If r_j=|C_j|, the audit uses
the optimistic one-regeneration-per-rank plan. Relative to computing H1 once,
it adds (r_j-1) * deg(j) first-layer CSR occurrences and (r_j-1) row
projections. Projection count is grouped into 16-row AMX batches at each
regeneration rank, and is only a logical work proxy. No DRAM traffic is inferred.

The candidate source-state trace releases source j after its first consumer rank
and regenerates it for later ranks. It reports modeled source-row state and
transient regenerated rows separately. It does not model cache, partial sums,
synchronization, or input X reuse. Sources with no consumer are excluded.
CSR row completion is max(column)/16+1 under node-ID production.

The audit retains the full input SHA, file identity, authority hashes, compiler
and preflight evidence, raw JSON, and Slurm accounting. A shared intel node
with one CPU and 16 GiB is sufficient. Large extra edge work is a stop signal,
not a tuning invitation.

## Retry history
Runs 01 (10899639) and 02 (10899667) failed before input scanning. Run 01 omitted the frozen prior-input manifest; run 02 had a Python EOF sentinel typo. Both remain immutable. Run 03 only fixes these runner defects.
