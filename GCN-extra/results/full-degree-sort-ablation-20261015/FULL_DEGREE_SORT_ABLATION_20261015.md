# FULL FAST: DegreeSort versus original destination order

Date: 2026-10-15. Status: complete 17-graph shared-node ablation, four-graph reverse-order replication and targeted web-Google repeat; **not yet formal exclusive-node paper-table evidence**.

## Question and control

The original TFS groups similar-degree destination rows to reduce inactive AMX row work during per-neighbor projection. In FULL FAST, one nonempty 16-row destination tile is projected once per output panel. This experiment changes only the destination permutation passed to the same frozen FULL kernel:

- **DegreeSort:** original ascending `make_degree_perm` from `original/gcn_e2e_v3.cpp`.
- **Identity:** `perm[i]=i`, preserving original CSR destination order.

Both use the paper-compatible `bfull_fast` kernel (`block=0`, FAST single BF16 partial, `nozero=true`, TR=16, R=64), the same CSR, source features and weights from `srand(12345)`, BF16 input and VNNI weight packing, dynamic OpenMP schedule, two-layer `128→128→128` with FP32 layer 1, ReLU, FP32→BF16 interlayer conversion and BF16 final output. Kernel source SHA-256 `35d291b886ee0f00e79e0e5d17c6226b2fa1ad64eb715ecbe7bcb1c3f02e7b85`; it matches the frozen `paper-method-build-20261004-123445` snapshot. Original driver SHA-256 `85c51bde92792f98223e80107e20b042f8f8d0e40068cdf6ce168143f0c7743f`, byte-identical to the read-only yx source.

The generated driver retains the untouched source TFS/MKL sections and injects the paired FULL comparison afterward. Only `ABLATION_TIME` is used for the main table. Prepared E2E excludes graph loading, initial BF16 H conversion, W packing, DegreeSort and identity initialization. DegreeSort and identity setup are measured separately. The graph and output remain in original node order; only destination execution order changes.

Small fixtures `tail_37` and `tail_137` passed before real graphs. The first two smoke submissions failed before the complete smoke gate due to an affinity assertion fixed at 32 cores and unavailable `rg` on the compute node; both failures remain in the raw evidence and handoff. The corrected smoke job `10968058` passed. Real graph jobs `10968059` and `10968073` completed with exit `0:0`. Both ran on shared `qhcn081`, one socket, physical CPUs 32–63, 32 OpenMP threads, `OMP_PROC_BIND=close`, `OMP_PLACES=cores`, default NUMA. The second run reversed graph order. Within each graph, methods alternated order across eight warmup-plus-measured paired repetitions per method.

Both four-graph runs used the same binary SHA-256 `95bc0dd12b183ca00bee3e9c2f98f6e6018392adedb8225b711fe9dc10ef3d31`, identical graph hashes and CPU affinity. The source files, binary, logs and input hashes are retained in the run directory.

The remaining 13 graphs from the fixed paper-compatible 17-graph inventory were run without rebuilding the binary: 12 graphs in job `10968094` on shared `qhcn251`, and the memory-heavy `rgg_n_2_24_s0` in job `10968095` on shared `qhcn259`. Both jobs completed with Slurm exit `0:0`. The 17 graph names match `docs/CURRENT_GRAPH_INVENTORY_20261004.json` exactly. Each graph still compares both schedules inside one process/node; no absolute times are divided across nodes. `rgg_n_2_24_s0` peak job RSS was about 81,085,228 KiB, so it used a 96,000 MiB allocation rather than the 64,000 MiB small-graph allocation. Job `10968442` repeated web-Google on `qhcn251` because its first-run CV was high; this repeat is a robustness check and is not substituted into the fixed 17-graph geometric mean.

## Correctness

All 17 graphs, the four reverse-order repeats and the web-Google repeat: Layer 1 FP32 output and final Layer 2 BF16 output were **bitwise identical** between DegreeSort and identity. This validates execution-order equivalence for this kernel/input; it is not a trained-model classification-accuracy result.

## Main performance result

Median of eight prepared two-layer forward times, milliseconds. `Degree / Identity > 1` means **identity is faster**. Forward and reverse are independent runs on the same node and binary.

| Graph | DegreeSort forward / reverse | Identity forward / reverse | Degree / Identity forward / reverse |
|---|---:|---:|---:|
| roadNet-CA | 37.97 / 39.18 | 30.15 / 31.41 | **1.259× / 1.247×** |
| reddit | 135.45 / 134.80 | 127.75 / 128.51 | **1.060× / 1.049×** |
| ogbn-products | 294.67 / 297.13 | 277.98 / 279.97 | **1.060× / 1.061×** |
| mycielskian19 | 2420.28 / 2434.39 | 2074.57 / 2095.77 | **1.167× / 1.162×** |

Four-graph geometric mean Degree/Identity: **1.1335× forward / 1.1269× reverse**. Per-method CV over eight primary samples ranged from about 0.2% to 2.5% in the first run and was similarly low in reverse; see raw `results.json` for every sample and min/max.

### Complete fixed 17-graph set

The fixed primary result uses the first four-graph run plus the 13 validation graphs. `Degree / Identity > 1` means identity is faster. Each ratio is computed within one graph's job/process/node.

| Graph | DegreeSort ms | Identity ms | Degree / Identity |
|---|---:|---:|---:|
| ogbn-products | 294.67 | 277.98 | 1.060× |
| reddit | 135.45 | 127.75 | 1.060× |
| mycielskian19 | 2420.28 | 2074.57 | 1.167× |
| hollywood-2009 | 199.47 | 99.94 | 1.996× |
| indochina-2004 | 209.79 | 154.17 | 1.361× |
| soc-Pokec | 93.99 | 80.53 | 1.167× |
| cit-Patents | 82.50 | 72.64 | 1.136× |
| soc-LiveJournal1 | 270.50 | 195.29 | 1.385× |
| com-LiveJournal | 250.70 | 172.68 | 1.452× |
| as-Skitter | 78.78 | 51.50 | 1.530× |
| rgg_n_2_24_s0 | 928.28 | 425.79 | 2.180× |
| web-Google | 21.70 | 19.09 | 1.137× |
| roadNet-CA | 37.97 | 30.15 | 1.259× |
| wiki-Talk | 45.53 | 31.17 | 1.461× |
| email-Enron | 1.19 | 0.77 | 1.549× |
| amazon0601 | 11.73 | 9.84 | 1.191× |
| com-Youtube | 33.69 | 23.43 | 1.438× |

**17/17 graphs favor identity; fixed-set geometric mean = 1.3553×.** The weakest ratios, Products and Reddit, each remained above 1.04× in their independent reverse-order run. web-Google's first-run CV was 10.4% for DegreeSort and 12.0% for identity, so a separate same-node repeat was run: 21.55 versus 18.80 ms, **1.146×**, with CV 0.25% and 1.03%. The repeat confirms direction without replacing the noisy primary sample. All measurements are on shared nodes and remain diagnostic rather than exclusive-node paper evidence.

### Comparison with the original source oneMKL baseline

`compare_onemkl.py` parses the five `run 0`–`run 4` times under `=== MKL FP32 Two-Step ===` from **the same graph/process/node logs** as each fixed-set FULL comparison, takes their median, and divides it by the corresponding FULL median. Thus every ratio below is `source oneMKL median / FULL median`; values above 1 mean FULL is faster. The original source oneMKL uses FP32 while FULL uses BF16 input/weights and BF16 final output, matching the original paper's baseline convention but **not an equal-precision comparison**. oneMKL was measured in five consecutive runs before the FULL alternating sweeps, so these are same-run derived diagnostics, not a fully interleaved oneMKL/FULL timing experiment. No historical MKL time or cross-node absolute denominator was used.

Precision clarification from the actual call path: original oneMKL calls `mkl_sparse_s_mm` with `float` CSR values/H/Z, then `cblas_sgemm` with `float` Z/W/C; ReLU and the next layer also remain FP32. Original TFS converts H/W to BF16, executes `_tile_dpbf16ps` with BF16 operands and **FP32 accumulators**, stores layer 1 as FP32, converts its activation to BF16, and stores layer 2 as BF16. This FULL FAST variant first decodes BF16 neighbor H to FP32 and reduces in FP32, truncates the partial sum to BF16 for `_tile_dpbf16ps` with FP32 accumulation, then follows the same FP32 layer-1/BF16 layer-2 interface. Thus "BF16 FULL" describes operand/storage precision, not FP32 accumulator absence. The oneMKL API and arguments establish its FP32 data path; this source inspection alone does not establish which ISA oneMKL internally selected for SGEMM. The speedups below are paper-style mixed-precision comparisons, not equal-precision kernel speedups.

| Graph | Source oneMKL FP32 ms | FULL DegreeSort vs oneMKL | FULL Identity vs oneMKL |
|---|---:|---:|---:|
| ogbn-products | 1347.30 | 4.572× | 4.847× |
| reddit | 442.54 | 3.267× | 3.464× |
| mycielskian19 | 4797.55 | 1.982× | 2.313× |
| hollywood-2009 | 391.09 | 1.961× | 3.913× |
| indochina-2004 | 692.04 | 3.299× | 4.489× |
| soc-Pokec | 333.13 | 3.544× | 4.137× |
| cit-Patents | 272.85 | 3.307× | 3.756× |
| soc-LiveJournal1 | 684.97 | 2.532× | 3.507× |
| com-LiveJournal | 649.76 | 2.592× | 3.763× |
| as-Skitter | 164.57 | 2.089× | 3.196× |
| rgg_n_2_24_s0 | 2017.97 | 2.174× | 4.739× |
| web-Google | 47.40 | 2.184× | 2.483× |
| roadNet-CA | 65.34 | 1.721× | 2.167× |
| wiki-Talk | 91.32 | 2.006× | 2.929× |
| email-Enron | 1.47 | 1.231× | 1.907× |
| amazon0601 | 25.73 | 2.194× | 2.614× |
| com-Youtube | 54.09 | 1.606× | 2.308× |

**17-graph geometric mean: FULL+DegreeSort = 2.3601× oneMKL; FULL+Identity = 3.1987× oneMKL.** Both are faster on 17/17 graphs under this derived comparison. The complete five source-oneMKL samples and exact ratios are in `summary_17_vs_onemkl.csv`.

## Work and setup

`nonempty_tiles × 32` gives logical AMX `TDPBF16PS` calls per layer in FULL FAST (two panels, four K blocks, four dot-products per K/panel). Both schedules process the same real edges and use identical precision. Identity does **not** reduce AMX projection work here; it equals or increases that count. Across all 17 graphs, eight have equal nonempty Tile counts and nine have more under identity.

| Graph | Nonempty tiles: Degree / Identity | AMX calls per layer: Degree / Identity | DegreeSort setup, ms forward / reverse |
|---|---:|---:|---:|
| roadNet-CA | 122,827 / 123,203 | 3,930,464 / 3,942,496 | 46.35 / 47.17 |
| reddit | 14,561 / 14,561 | 465,952 / 465,952 | 15.06 / 15.01 |
| ogbn-products | 150,039 / 153,065 | 4,801,248 / 4,898,080 | 125.98 / 127.32 |
| mycielskian19 | 24,576 / 24,576 | 786,432 / 786,432 | 18.94 / 19.12 |

Identity initialization was 0.02–0.66 ms. Its schedule is conceptually the original row order, so a production identity path could avoid materializing an explicit permutation array; this experiment retained the common `perm` interface to isolate the inner-kernel change. DegreeSort setup was measured but excluded from the primary E2E, matching the paper-compatible timing boundary. For a first inference, adding setup strengthens the identity result on these graphs.

The sampled `padded_row_steps = sum_tiles(tile_max_degree × actual_tile_rows)` is a **structural upper-bound indicator**: identity has more mismatched degrees per tile. The FULL kernel's reduction loop instead stops at each row's actual degree, so these padded steps are **not executed feature loads or AMX operations**. Do not use this indicator as measured memory traffic.

## Layer timing and interpretation

First-run mean of three separately timed stage samples, milliseconds; these values are diagnostic and do not sum to the primary median exactly.

| Graph | Layer 1 Degree / Identity | Layer 2 Degree / Identity |
|---|---:|---:|
| roadNet-CA | 16.09 / 11.74 | 12.98 / 9.61 |
| reddit | 65.84 / 62.91 | 63.19 / 62.11 |
| ogbn-products | 144.83 / 136.65 | 138.40 / 133.80 |
| mycielskian19 | 1203.12 / 1018.74 | 1240.71 / 1052.58 |

Both layers benefit from identity in the four-graph stage sample. Reddit and Myciel have the **same** number of nonempty destination tiles and AMX dot-products under both schedules, yet identity is faster. On RoadNet and Products identity uses slightly **more** AMX dot-products and still wins. The full 17-graph set shows the same direction for every graph. Thus, the original DegreeSort objective of minimizing ineffective AMX work is much weaker for FULL than for original per-neighbor TFS; it does not explain this experiment's performance direction.

Possible remaining causes include source-feature/cache locality, row metadata access, dynamic OpenMP task balance and output scatter locality. This run did **not** collect per-thread balance or PMU cache/bandwidth counters, so none of those mechanisms is proven as the cause of the speedup. The result is specific to frozen FULL FAST, these unit-weight graphs, 128-wide BF16 features, one SPR socket per job, and shared-node runs. A formal claim needs exclusive-node repeats and a separate PMU/load-balance diagnostic; an adaptive DegreeSort policy is not established.

One code-level distinction is certain: the kernel scatters each output to `c[rows[n]]`. With identity, destination writes follow original node order; with DegreeSort, they follow the degree permutation. This changes the address sequence of output stores, but its share of the measured gain was not isolated. The same schedule change also changes CSR row traversal order, so source-feature cache effects remain possible.

## Evidence and reproduction

- Source: `../../experiments/full_degree_sort_ablation_20261015/`.
- Build and immutable generated source: `full-degree-sort-ablation-20261015-113936/source_snapshot/`.
- Raw runs and manifests: `full-degree-sort-ablation-20261015-113936/logs/real-20261015-114328/` and `.../reverse-20261015-114839/`.
- 13-graph validation: `.../logs/validation_small-20261015-115845/` and `.../logs/validation_large-20261015-115853/`.
- web-Google robustness repeat: `.../logs/web_repeat-20261015-120308/`.
- Each run has `summary.csv`, `results.json`, per-graph raw stdout/stderr, graph hashes, preflight hashes, Slurm logs, CPU affinity, NUMA policy and environment.
- `summary_17.csv` combines the fixed primary 17 graph runs and records per-graph node, run, CV, work counts and SHA-256 checked against the fixed inventory.
- `summary_17_vs_onemkl.csv` and `../../experiments/full_degree_sort_ablation_20261015/compare_onemkl.py` reproduce the same-process source-oneMKL median comparison.
- Server master location: `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/full-degree-sort-ablation-20261015-113936`.
