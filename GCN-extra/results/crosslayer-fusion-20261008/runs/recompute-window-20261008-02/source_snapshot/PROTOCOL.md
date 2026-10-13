# Consumer reuse-window structural audit, 2026-10-08

This follows the selective H1 regeneration cost audit. It keeps 128 -> 128 ->
128, node-ID producer batches of 16, sorted CSR, and the existing BF16/FP32
boundaries. A source H1 row is retained across nearby consumer completion ranks
within a window W; when the next consumer rank is farther than W batches, the
source is released and regenerated at the next rank. The scanner uses W=-1 for
regeneration at every distinct rank, W=1 for reuse across adjacent ranks, and
W=0 for retaining one source through all consumer ranks.

For each W the scanner reports extra first-layer edge visits, extra AMX row
projection groups, and the modeled source plus transient regenerated-row peak.
These are logical proxies. It does not model DRAM, cache, synchronization,
partial rows, ready rows, numerical execution, elapsed time, or legal full
kernel scheduling. It does not claim the window policy is novel. The goal is
to see whether a source-aware reuse window creates a nontrivial three-way
tradeoff worth implementing.

One shared intel node, one CPU, 16 GiB, 25 minutes. No NUMA interleave and no
feature or first-touch change. All writes stay under wzh; old runs remain
immutable.

Retry 02 fixes the explicit window endpoint labels; retry01 remains immutable.
