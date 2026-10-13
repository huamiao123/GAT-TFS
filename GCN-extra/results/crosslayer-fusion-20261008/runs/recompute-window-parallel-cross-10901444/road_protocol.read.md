# Parallel consumer-driven cross-layer fusion prototype, 2026-10-08

This is an isolated shadow experiment for the W=0 consumer-driven dataflow. It keeps 128 -> 128 -> 128, DegreeSort, TR=16, BF16 boundaries, packed weights and default NUMA policy. Destination 16-row rank ranges are statically partitioned across OpenMP threads. Each thread computes and retains its own H1 source rows, so source duplication across partitions is counted rather than hidden behind a shared cache.

The baseline is the canonical materialized FULL FAST path at the same thread count. The candidate includes source discovery, per-thread cache allocation, H1 AVX reduction and AMX projection, second-layer AVX aggregation, AMX projection, and synchronization in its timing boundary. Three measured repetitions follow one warmup. Exact final BF16 equality is required. This remains exploratory and paper-ineligible.
