# Source-overlap-aware destination partition audit, 2026-10-08

This structural audit measures the number of unique H1 source rows required by 32 destination partitions. It compares the current DegreeSort order, original row order, degree-descending order, and simple source-neighbor locality orders based on minimum, average, and maximum source IDs. The graph and CSR are read-only; no NUMA interleave or feature/graph mutation is used.

The result is a feasibility bound for the parallel consumer-driven fusion path. It does not claim a runtime improvement. A partition is counted by its exact CSR source set, with equal destination-row counts and output scatter to original node IDs implied. The key decision is whether a legal schedule can reduce the 1.85x source duplication measured in the first 32-thread prototype.
