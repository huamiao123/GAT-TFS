# RoadNet-CA source-aware reuse-window kernel prototype, 2026-10-08

This is an isolated shadow experiment. It keeps the paper-compatible 128 -> 128 -> 128 computation, DegreeSort logical order, TR=16, row-major BF16 H0/H1 and VNNI-packed weights. It does not modify the canonical paper-method kernel or the original TFS/MKL sources.

The candidate computes H1 BF16 source rows on demand with the existing AVX reduction plus AMX projection semantics, caches them by source node ID, and processes the second layer with an AVX full-neighbor reduction followed by one AMX projection per destination tile. A source is retained across consumer 16-row ranks only when the next-use gap is within W; W=0 is the all-retain control. All source generation, cache operations, reductions, conversions, and projections are inside the candidate timing boundary.

The first implementation is single-thread only. This isolates numerical/dataflow and work/state effects from parallel cache ownership and synchronization. No NUMA interleave, first-touch modification, graph reorder, or feature reorder is used. The primary candidate is RoadNet-CA W=1024; W=0 is the all-retain control.

The structural window audit was a logical proxy and is not reused as a performance claim. This run must report exact BF16 output equality against the materialized FULL FAST path, source row/projection counts, cache peak, and elapsed time. It is exploratory and paper-ineligible until a multi-thread implementation and formal repeated comparison exist.
