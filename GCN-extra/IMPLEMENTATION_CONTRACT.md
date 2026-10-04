# Current controlled implementation contract

Authoritative controls: docs/PAPER_METHODS_PROTOCOL_20261004.md.
Keep original gcn_e2e_v3.cpp and its source MKL helpers byte-for-byte unchanged.
Use the same graph, source-seeded H/W, 128->128->128, DegreeSort, TR=16, R=64,
packed weights, activation, output precision and timing boundaries.
32 OpenMP threads on formal exclusive nodes; source compilation flags/modules;
default NUMA policy; no MKL thread/dynamic override; at most five nodes.
Write only within wzh; yx is read-only. Read applicable AGENTS.md/SKILL.md.
Modified TFS parameters are neighbor block and FAST/ACCURATE representation.
Use the canonical validated paper_methods_kernels.hpp. No numerical gate
relaxation, per-graph oracle treated as deployed policy, or cross-run timing
denominator. Preserve all current negative cases and raw repeated measurements.
No training, dynamic graph or checkpoint classification is claimed.
Previous result sets were deleted by explicit user request on 2026-10-04.
Current frozen build/launcher snapshots remain immutable evidence of actual runs.
