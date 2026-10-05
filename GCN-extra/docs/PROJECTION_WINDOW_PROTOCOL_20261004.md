# Independent neighbor window and projection scope — preregistered protocol

This experiment extends the controlled original-source TFS/MKL comparison. It is static two-layer inference computation with random source H/W (`srand(12345)`), shape 128 → 128 → 128. It is not trained classification.

## Hypotheses and variables

The existing modified TFS couples neighbor access window B with the partial sum projection boundary. FULL minimizes repeated AMX projection but is slow on Mycielskian19. Test whether visiting 16 destination rows alternately in windows S, while preserving a projection span M, changes the performance frontier.

- S: 32, 64 or 128 neighbors per row, preserving that row's CSR addition order.
- M: 64, 256 or full maximum degree of the destination tile.
- Precision: existing FAST (one truncated BF16 component) and ACCURATE (same hi+lo decomposition). No new approximation.
- Fixed: ascending DegreeSort, TR=16, scheduling R=64 dynamic(1), weights packing, output order, empty row handling, same H/W/CSR, compiler/modules, thread count/binding and default NUMA policy.

Original TFS, exact source MKL, current B64/FULL FAST/ACCURATE are retained. Add coupled B256 FAST/ACCURATE controls. Experimental cases: S64/M64 FAST; S32/M64 FAST; S32/M256 FAST; S64/M256 FAST; S32/MFULL FAST; S64/MFULL FAST; S128/MFULL FAST; S64/MFULL ACCURATE. The matrix is frozen before measurement. Do not change prefetch, permutations, scheduling or NUMA in this round.

Fixing M and varying S preserves projection boundaries, quantization frequency, AMX instruction count and CSR FP32 sum order. It adds explicit per-window partial state loads/stores. Changing M also changes summation grouping and quantization points, so that comparison is not a pure memory locality experiment.

## Gates

The four existing frozen numerical gates apply to all methods before timing, using original TFS FP32, original final BF16 and source MKL FP32 references. FAST FP32 relative L2 and normalized maximum < 0.01; ACCURATE < 0.001; actual BF16 final versus TFS < 0.02/0.01; versus MKL < 0.03. All finite; original TFS output must reproduce its original references.

Additionally, every new S/M method must pass bytewise equality (`memcmp`, including signed zero) to the frozen coupled kernel at the same M and precision, at FP32 single-layer, FP32 final two-layer and actual BF16 final boundaries. NaN prefill verifies output coverage. No gate relaxation after failure.

Shared correctness fixtures test node tails/empty rows and degrees around 8/16/32/64/128/256/512 boundaries. Formal exclusive runs begin only after smoke passes. Focus graphs: products, Reddit and Mycielskian19. Validation uses all remaining 14 eligible graphs with the same frozen matrix; no oracle-selected adaptive method is claimed.

## Measurements

32 OpenMP physical threads, `OMP_PROC_BIND=close`, `OMP_PLACES=cores`. Original MKL environment overrides remain unset. Normal NUMA allocation, no interleave/first-touch changes. Source TFS ends in BF16; source MKL ends in FP32, as in the original comparison. Precision difference must remain explicit in conclusions.

Primary: one immediate warmup and five consecutive uninstrumented complete two-layer repeats per method. Secondary: five rotated method orders in the same process; this is not fully balanced Latin scheduling. Speedups use same graph, process and measurement protocol TFS/MKL denominators, never previous runs or other nodes. Include min/median/max and every raw repeat. Untouched source binary provides a separate source anchor, not a substituted denominator.

Report layer1, ReLU, interlayer conversion, layer2 and total in separate three-repeat runs. Sampled thread timers separate reduction initialization, CSR/prefetch, gather/decode, FP32 reduction, conversion, local state loads/stores, AMX tile loads/compute/stores and scatter. These sampled thread sums are not wall time fractions and are not PMU causal evidence.

The detailed profiler records 24 child intervals: partial initialization; current/next CSR index reads; prefetch issue; source BF16 load/decode; FP32 add; partial state load/store; hi pack; lo residual/pack; AMX A/B loads, TDP, C zero/load/store; output BF16 conversion/store; thread permission/config/release; row metadata. Parent and child intervals overlap, so their values must not be summed. Record clocked-section counts and each thread's active interval, setup/release, completion offsets and completed tiles/edges. Each profiled kernel must reproduce its corresponding uninstrumented output byte for byte.

Setup diagnostics separately record CSR file loading, tensor allocations, seeded weight/feature generation, DegreeSort, VNNI packing, input BF16 conversion, output allocations, MKL index allocation/conversion, sparse handle creation and hint/optimization. These are outside the source compute E2E boundary and must not be silently included in or omitted from a claimed boundary. MKL's diagnostic SpMM/GeMM/ReLU timers execute the exact source operations after all primary timing.

Additional three-repeat PMU diagnostics run selected complete forwards with all existing process TIDs enrolled in hardware-event groups: userspace cycles, retired instructions, generic cache misses and references. Preserve raw counts, enabled/running times, per-thread multiplex scaling, thread coverage and enable/disable skew. These regions follow primary timing and are not used as its denominator. Generic cache counters are CPU dependent; they are not measured DRAM bytes. Unavailable events are retained as missing diagnostics, never grounds for changing permissions or numerical gates.

On CPUID family 6/model 143 only, also request `EXE.AMX_BUSY`, raw 0x02b7, from the archived official Intel SPR event JSON. It counts speculative AMX arithmetic busy cycles, not retired tile instructions, FLOPs, or kernel wall time. Unsupported raw-event groups remain visibly unavailable. Selected PMU forwards: original TFS, source MKL, B64/B256/FULL FAST, S64/M256 FAST, S64/MFULL FAST/ACCURATE and S128/MFULL FAST.

Analytic counters count requested accesses and executed tile work, with units labeled. Do not call local buffer byte counts measured DRAM traffic. Probe available perf counters and actual allocation/affinity read-only; retain denied/unsupported access without changing global permissions. CPU Max capability does not establish active HBM mode.

All writes remain under wzh/GCN-extra; yx is read-only. Keep immutable unique run/source/launcher snapshots, hashes, Slurm/node/affinity manifests and all failures/slowdowns. Append all three project handoffs after each build/submission/completion. At most two one-node jobs active for this experiment.

## Interpretation and academic threshold

A speedup from window interleaving alone is evidence for the performance model, not automatically a journal contribution. Existing fusion/tiling/cache work must be compared explicitly. A defensible extension needs a general TFS projection placement model linking actual AMX work, sparse dependency traversal, local state lifetime and numerical constraints, with validated prediction and applicability boundaries.

## 2026-10-04 journal study revision before formal measurement

The user supplied `TFS期刊扩展技术说明_综合研究方案_20261004.txt` after the
two shared smoke jobs had passed. Its P0/W1-W3 experiment takes precedence for
the next formal measurement: A0 is coupled FULL FAST, A1 is S64/MFULL FAST,
and A2 is coupled B64 FAST. Mycielskian19, Reddit, Products and the low-degree
roadNet-CA are the four diagnostic graphs. Remaining eligible graphs form the
subsequent validation set. This changes graph batching, not the kernel,
numeric gates or model semantics.

The user explicitly permits shared intel nodes. Formal runs use one
32-physical-core socket with close/core binding and unchanged default NUMA
allocation, with no interleave. This revision supersedes the older
`intel_expr` exclusive-job requirement for this phase. Shared-node noise is
reported, and all ratios are formed inside a graph/process/node. The original
three smoke fixtures remain valid numerical evidence because the kernel is
unchanged. The new launcher also repeats the smoke gate to validate affinity.

If A1 gives a reproducible benefit, A3 will preserve the 64-neighbor loop
boundaries and partial state traffic in a row-oriented control before making
any attribution to cross-row access. The current S/M code does not implement
A3, and measured A0/A1 differences alone do not isolate cache effects.
