# GCN-extra

Inference-only extension of ICPP Original TFS: DegreeSort + 16 destination rows +
neighbor-block FP32 reduction + immediate AMX GeMM. No full global AH in fused
paths. Original source is preserved byte for byte.

Remote root: `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra`.
Read-only dataset root: `/home/huangjianqiang_group/hdacp1/data/yx/TFS/data`.

## Paper reproduction: corrected v3 baseline (2026-10-04)

The paper's E2E Table 2 corresponds to `original/gcn_e2e_v3.cpp`, with FP32
intermediate output and BF16 final output. The earlier source comparison below
used `gcn_e2e_bench.cpp`, an older FP32-final version with redundant global
output zeroing. Its 0.891x aggregate result does not describe the paper version;
that inference is withdrawn. Historical measurements remain preserved.

Use `scripts/build_paper_reproduction.sh`, then
`scripts/submit_paper_reproduction.sh smoke` and `formal`. The raw performance
sources remain byte-identical to yx; no NUMA or MKL overrides are introduced.
Separate full-output correctness checks validate the actual BF16 final result.
Both minimum-of-five and median-of-five are retained: historical paper table
entries match source BEST values despite the manuscript describing medians.

Report: `docs/PAPER_REPRODUCTION_RESULTS_20261004.md`.
Raw formal evidence: `runs/paper-formal-10864848`.
## Controlled method remeasurement (2026-10-04)

The neighbor-reduction extension now has a controlled comparison against the
paper v3 baseline: 17 eligible real graphs, all 14 B/precision candidates,
1020/1020 numerical checks, and 2720 measured repetitions. A second all-17
execution-order control keeps the exact frozen kernels and gates, with an
immediate warmup plus five consecutive E2E repetitions per method; it adds
612/612 passing checks and 850 measurements. Both protocols are retained.
Same graph/process/source buffers, random H/W, DegreeSort, 128->128->128,
32 physical threads, original MKL helpers/hint, default NUMA policy and BF16
final TFS output are used. A speedup never mixes times from different nodes.
Original MKL remains FP32 exactly as in the paper; no NUMA optimization was added.

All-17 geometric means below use median-of-five consecutive measurements and
their same-process original baselines. Interleaved comparisons remain in the
reports; do not substitute one protocol's denominator into the other.

| Fixed method | vs paper TFS | vs source MKL | Faster than TFS |
|---|---:|---:|---:|
| b8_fast | 1.106x | 2.057x | 9/17 |
| b16_fast | 1.216x | 2.262x | 10/17 |
| b64_fast | 1.309x | 2.435x | 11/17 |
| b64_accurate | 1.262x | 2.348x | 11/17 |
| bfull_fast | 1.258x | 2.341x | 11/17 |
| bfull_accurate | 1.221x | 2.272x | 10/17 |

Reports:
- `docs/PAPER_METHODS_RESULTS_20261004.md`: full seven-block-size sweep, raw timings and gates.
- `docs/PAPER_CACHE_CONTROL_RESULTS_20261004.md`: consecutive/interleaved order control.
- `docs/PAPER_METHODS_INTERPRETATION_20261004.md`: actual source dataflow, stage timings, modeled AMX counts and limits.
- `docs/figures/paper_methods_20261004/`: standalone PNG/PDF figures and repeat variability CSV.

Latest controlled sources: `src/paper_methods_runtime.hpp`,
`scripts/generate_paper_methods.py` and the frozen complete generated source
in `runs/paper-method-build-20261004-123445/source_snapshot/`.
Reconciled evidence: `runs/paper-method-reconciled-20261004` and
`runs/paper-cache-reconciled-20261004`. The source/binary/log archive is
`evidence-paper-methods-controlled-20261004.tar.gz` with SHA256 manifest.

No full global AH is materialized in the fused method. DegreeSort and AMX
tile-aware fusion remain. FULL is not universally fastest: Mycielskian19
favors bounded blocks; low-degree losses remain visible. There is no deployed
adaptive selector. Sampled phase timing is diagnostic, not additive wall time
or PMU proof. This is prepared two-layer computational inference on the
original random H/W, not checkpoint classification accuracy.
Six nonunit graphs and two original-LP64-incompatible shapes retain their
explicit exclusions; datasets and operator semantics were not altered.

## Historical comparison with older source MKL/TFS (2026-10-04)

Original TFS and its MKL source/scripts have no explicit NUMA memory policy.
The 2026-10-03 experiments below used explicit interleave and other wrapper
changes; retain them as historical evidence, not original-source reproduction.
This older-source comparison uses `docs/SOURCE_MKL_PROTOCOL_20261004.md`, which overrides
the old contract's runtime/data-generation/statistic clauses for this protocol.

Build: `bash scripts/build_source_protocol.sh`.
Correctness: `bash scripts/submit_source_protocol.sh smoke`.
Authorized formal comparison after correctness: `bash scripts/submit_source_protocol.sh formal`.
The generated runner preserves `original/gcn_e2e_bench.cpp` except an include
and a candidate hook after the original timings/checks. Original MKL uses FP32,
hint=10, original allocation/first writes, one warmup and five-run minimum.
All methods use default NUMA policy and only the original declared OMP settings.
Separate untouched original MKL/TFS/E2E binaries are retained as anchors.
This LP64 protocol excludes graphs requiring ILP64; old ILP64 recovery values
must not be substituted into a source-faithful table.

Completed source-aligned comparison: `docs/SOURCE_MKL_RESULTS_20261004.md`.
Runs `source-formal-10864754` and `source-formal-10864792` give 17 valid real
graphs and 765 numerical checks, with eight excluded inputs. Fixed B64 Fast
gives 1.427x geometric mean versus source MKL, 1.601x versus raw source TFS,
and 1.189x versus source TFS with only redundant zeroing removed. FP32/BF16
precision follows the original source protocol; it is not a matched-precision
competition. Preserve the first batch's FAILED shell-completion status after
all 16 graph processes succeeded, and its initial erroneous RGG skip; the
supplement covers RGG without changing the binary. Immutable launcher fix was
validated by smoke 10864830 (102 checks, exit zero). All raw logs remain visible.

## Historical 2026-10-03 experiment protocol and evidence

See IMPLEMENTATION_CONTRACT.md and docs/experiment_plan_v1.txt for the operator,
precision controls, numerical gates, timing boundaries and modeled work counts.
Build with `bash scripts/build.sh`; correctness uses scripts/smoke.slurm;
first exclusive 32-core experiment uses scripts/formal.slurm. Each build/job
has an immutable run directory with source snapshot, manifests, hashes, raw CSV,
stdout/stderr and all three synchronized project handoffs.

Kernel profiles are separate sampled runs, not speedup timings. FP32 MKL is
reported separately from the BF16-input/weight MKL reference. Experiments use
random H/W and sum adjacency; task accuracy remains unverified.

First completed evidence: `runs/formal-10861421` (full B sweep) and
`runs/formal-10861923` (stronger zero-free Original TFS and paired controls).
Report: `docs/FIRST_RESULTS_20261003.md`; standalone plots: `docs/figures/`.
The final algorithm comparison uses original_nozero versus matching local nozero
variants. Raw unchanged original remains in every comparison.

Complete nozero neighbor-block sweeps use `--block-controls --blocks
2,4,8,16,32,64,full`; this runs both Fast and Accurate, including two-layer E2E.
`--controls` retains the previous limited zeroing controls unchanged.
Submit the full sweep with `GCN_BLOCK_SWEEP=1 GCN_REPEATS=10 bash
scripts/submit.sh formal` after the shared correctness gate. The same environment
flag on `submit.sh smoke` tests all block sizes, empty rows and tail fixtures.
Actual configuration is saved in each run's `measurement_config.txt`.

Canonical 25-graph coverage uses `scripts/graph_inventory.py` (header-only
inspection), then `GCN_BLOCK_SWEEP=1 GCN_REPEATS=10
GCN_GRAPH_LIST=/absolute/path/to/inventory.json bash scripts/submit.sh suite`.
The suite uses one exclusive node, 32 physical cores, 448 GiB and a two-hour
limit. It records per-graph commands, full input SHA256, statuses and failures;
incompatible/nonunit data is retained and rejected, without changing files.
`graph_suite_report.py` gives all-graph and high-average-degree tables. It calls
the best measured B an oracle result, not an implemented adaptive selector.

The memory lifetime adjustment frees three kernel-validation reference arrays
before allocating two-layer references, outside all measured intervals. Large
graphs exceeding one socket's memory capacity interleave across online CPU NUMA
domains, equally for every method; this difference is recorded per graph.

For Friendster (CSR offsets exceed INT32_MAX) and road_usa (dense last-element
index exceeds INT32_MAX), use `GCN_ILP64=1 bash scripts/build.sh`, then the shared
correctness gate before formal runs. ILP64 changes MKL integer/index width only;
floating point remains FP32/BF16. The supervisor refuses these shapes with LP64
to prevent recurrence of the preparation crashes retained in formal-10862002.
The recovery runs preserve the same AMX and local kernels. Friendster uses
B8/16/32/64/Full with five repetitions; other successful graph coverage uses all
seven block sizes with ten repetitions. Individual commands and immutable source
snapshots remain the reproduction authority.

Completed graph coverage: `docs/GRAPH_SUITE_RESULTS_20261003.md` and matching
comparison CSV, with standalone PNG/PDF plots under `docs/figures/graph_suite/`.
Runs `formal-10862002`, `formal-10862158`, and `formal-10862162` attempted 25
canonical graphs: 19 completed and passed 2028 numerical records; six nonunit
CSR-value files were rejected and retained. Original LP64 preparation failures
for Road-USA/Friendster remain visible beside successful ILP64 recovery runs.
Posthoc best Fast B gives 1.350x geometric mean versus Original_nozero; fixed
Full Fast gives 1.290x. This is not an adaptive-dispatch claim. RGG and Road-USA
remain slower even at their best measured B. D=F=128 and random H/W limit task
and shape generalization; the prepared E2E boundary excludes initial setup.
