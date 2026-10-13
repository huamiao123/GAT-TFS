# Cross-graph source-overlap partition audit, 2026-10-08

This follow-up runs the exact source-set partition analyzer on `amazon0601` and `reddit` with 32 equal destination partitions. It compares DegreeSort, identity, degree-descending, minimum-source, average-source and maximum-source orders. The analyzer is structural only and does not change graph data, NUMA policy, or feature layout.

The purpose is to determine whether the RoadNet-CA source-overlap signal generalizes enough to justify another parallel fusion kernel run.
