# H Panel Reuse: controlled exploratory protocol

## Research question

Original TFS V3 processes output panels first and gathers a BF16 source vector
once per panel. This experiment changes only the panel/neighbor loop ordering
and the placement of an FP32 AMX output accumulator spill across windows.

## Methods

- `A0_original`: byte-identical included original two-layer source, original
  `tfs_v3_fp32out` and `tfs_v3_bf16out` kernels.
- `A1_step`: source H gathered once per neighbor step, both panels consume it,
  C accumulator is staged between steps.
- `A2_window8`, `A3_window16`, `A4_window32`, `A5_window64`: same source H is
  gathered once per window and replayed locally for two output panels. C tiles
  are staged only between windows.
- `control_window16_duplicate_gather`: same window buffer and C staging as
  `A3`, but gathers H again before the second panel. This measures net benefit
  of one gather at the same window/staging cost.

No method pre-aggregates H, modifies W packing, changes the four `kb` steps,
or changes the per-panel neighbor summation order. Each per-layer, per-batch
AMX `TDPBF16PS` count is `32 * max_degree_of_batch` for every method.

## Fixed controls

- Static CSR, original DegreeSort permutation, `TR=16`, `R=64`, `D=F=128`.
- Seed 12345; W1 and W2 initialized before H0 as in original two-layer source.
- H0 FP32→BF16 and W VNNI packing occur outside prepared E2E timing.
- L1 BF16→FP32, ReLU FP32, interlayer FP32→BF16, L2 BF16→BF16.
- Original high-16-bit BF16 truncation and final output order.
- Intel 2024.1 compiler/MKL and original flags; default NUMA policy.
- Same graph, process, node, and thread binding. Warmup each method, five
  alternating forward/reverse rounds. Use median and report spread.
- Shared-node runs are diagnostic and never a final paper table.

## Correctness gate

Before timing, each candidate must compare bitwise to A0 for L1 FP32 output,
the interlayer BF16 activation, and L2 BF16 output. The tail fixtures contain
degree 0, 1, 7, 8, 16, 32, 64, 65, 129 and partial 16-row batches.

## Work ledger

With `E` real edges and sum of sorted-batch maximum degrees `T` per layer:

- A0 and duplicate-gather: `2E` source copies; shared-H candidates: `E`.
- Logical source copy bytes: copies × 256. These are not DRAM bytes.
- Every method: `8T` AMX H loads, `32T` W loads, `32T` TDP instructions.
- For window S, `Q_b=ceil(T_b/S)` and extra FP32 C staging is
  `16 KiB * sum_b(max(Q_b-1,0))` of logical load+store requests.
- H buffer capacity is `4 KiB * S` per thread; C state is 8 KiB per thread.

Window filling changes prefetch lead time and local H buffer pressure. The
duplicate-gather control helps separate sharing from window/staging effects;
PMU and timed phase probes are still required before attributing actual memory
hierarchy cost. No CPU counter is inferred from logical bytes.

## Diagnostic graphs and launch order

1. `smoke`: 37/137 node fixtures on shared intel, 4 threads.
2. `diagnostic1`: roadNet-CA, reddit on shared intel, 32 threads.
3. `diagnostic2`: ogbn-products, mycielskian19 on shared intel, 32 threads.

Only schedule a new node if the account's distinct running-node count remains
within the previously agreed maximum of five. Do not interrupt other jobs.
