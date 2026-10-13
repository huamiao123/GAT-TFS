# Exclusive source-aware fusion validation, 2026-10-08

This is the first stability validation of the source-overlap-aware consumer-driven fusion candidate. It runs RoadNet-CA and Amazon on one exclusive `intel_expr` node with 32 physical CPUs. Each graph uses DegreeSort, identity, and minimum-source orders; each order compares the same-order materialized FULL FAST path and the per-thread W=0 fused path with one warmup and three measured repetitions.

The exact 128 -> 128 -> 128 computation, BF16 boundaries, random seed, original output scatter, and default NUMA policy remain fixed. No NUMA interleave or feature reorder is used. The goal is to separate the RoadNet positive signal and Amazon shared-node signal from scheduling noise. Results remain exploratory until repeated across another node or graph family.
