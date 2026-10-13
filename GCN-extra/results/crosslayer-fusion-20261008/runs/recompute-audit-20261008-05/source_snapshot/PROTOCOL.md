# Selective H1 regeneration cost audit, retry 04

This read-only audit keeps the frozen 128 -> 128 -> 128 path and scans only
CSR structure. It estimates the optimistic policy that discards a BF16 H1 row
after its first second-layer consumer rank and regenerates it once per later
consumer rank. For source j with r distinct consumer ranks, the added first
layer work is (r-1) deg(j) edge visits and the added row projections are r-1.
Projection rows are grouped into 16-row AMX batches only as a work proxy.

The result is logical work and a source-state model, not DRAM traffic, cache
capacity, synchronization cost, elapsed time, or accuracy. It does not replace
the canonical kernel or justify a performance claim. The C++ scanner uses two
passes over each mmap'ed CSR and sorts compact (source, consumer-rank) keys so
the 903M-edge graph does not require Python per-edge loops.

Retry history: 10899639 omitted the prior manifest; 10899667 had an EOF
sentinel typo; 10899674 was cancelled after 11:16 because its Python loop had
not produced output. All three runs remain immutable evidence.

Retry 05: retry04 staging failed because prior input file was absent from the remote experiment directory before sbatch. No compute job was submitted; the partial directory is retained. Retry05 explicitly stages all six files.
