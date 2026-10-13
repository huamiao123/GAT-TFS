# Cross-graph source-aware fusion kernel gate, 2026-10-08

This is an exploratory 32-thread negative-control gate for Amazon and Reddit. It uses the same isolated consumer-driven W=0 kernel and the same-order materialized FULL controls for DegreeSort, identity and minimum-source orders. It preserves 128 -> 128 -> 128, BF16 boundaries, original output order, and default NUMA policy. The source-overlap audit predicts high duplication for both graphs; the purpose is to test that prediction, not to search for a per-graph oracle.
