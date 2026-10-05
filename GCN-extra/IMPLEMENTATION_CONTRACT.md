# Current controlled implementation contract

Authoritative controls: docs/PAPER_METHODS_PROTOCOL_20261004.md.
Keep original gcn_e2e_v3.cpp and its source MKL helpers byte-for-byte unchanged.
Use the same graph, source-seeded H/W, 128->128->128, DegreeSort, TR=16, R=64,
packed weights, activation, output precision and timing boundaries.
32 OpenMP threads on formal exclusive nodes; source compilation flags/modules;
default NUMA policy; no MKL thread/dynamic override; at most five nodes.
Write only within wzh; yx is read-only. Read applicable AGENTS.md/SKILL.md.
Modified TFS parameters are neighbor block and FAST/ACCURATE representation.
Independent neighbor-window/projection-scope experiments are additionally
authorized by docs/PROJECTION_WINDOW_PROTOCOL_20261004.md. Their new kernels
remain isolated; same-scope output must be bitwise identical to the canonical
validated kernel. No prefetch/NUMA/permutation changes in that controlled round.
The user additionally authorizes the frozen 8aeef16 feature-input precision
2x2 ablation under experiments/feature_precision_8aeef16/PROTOCOL.md. BF16
control stays byte-identical; FP32 sparse input bypasses decode and native
interlayer conversion, with separate matched-input/reference diagnostics.
The user explicitly allows shared intel nodes for this experiment; verify
32 physical cores on one socket and label shared measurements accordingly.
The follow-on journal P1 study is authorized under
experiments/journal_grouping_p1_20261005/PROTOCOL.md. It changes only the
logical destination schedule within bounded identical-q64 buckets and
requires bitwise output and identical projection/window work. P0 sources
and existing frozen run snapshots remain unchanged.
The user-supplied journal study also authorizes isolated P2 same-work
AVX/AMX handoff microbenchmarks and P3 delayed-residual numerical gates,
under their own experiments/ protocols. These do not replace the paper
baseline or constitute complete-model performance/accuracy evidence.
Use the canonical validated paper_methods_kernels.hpp. No numerical gate
relaxation, per-graph oracle treated as deployed policy, or cross-run timing
denominator. Preserve all current negative cases and raw repeated measurements.
No training, dynamic graph or checkpoint classification is claimed.
Previous result sets were deleted by explicit user request on 2026-10-04.
Current frozen build/launcher snapshots remain immutable evidence of actual runs.
