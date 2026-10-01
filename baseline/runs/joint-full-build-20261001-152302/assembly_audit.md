# Full-group assembly evidence

Compiler/flags and source hashes are in this build directory. This is static instruction evidence, not measured DRAM traffic or a timing result.

`joint_full.s` lines 51254–57480 contain `run<8, true, false>` (G=8, Full=true, Counters=false). It contains runtime-selected sampled and unsampled tile code; this function name alone does not distinguish those tile branches.

An emitted feature/edge inner loop is at lines 57147–57169 (`LBB15_65`):

- 57155–57157 load a source index and form the H address.
- 57158: one masked H vector load into `zmm16`.
- 57159–57166: eight consecutive `vfmadd231ps` using that same `zmm16`, with independent weights and accumulator registers `zmm8` through `zmm15`.
- 57167–57169: only the edge loop increment/condition remains. There are no active-head `cmp/jcc`, calls, or accumulator stack stores/reloads within this loop.

The stack memory operands on these FMAs are `weights[head][edge]`, not accumulator spills. U is still stored in worker-local memory between neighbor blocks and consumed by the unchanged per-head SGEMM.

The v1 counterpart retained active-head guards after the first three FMAs; see `../joint-build-20261001-150558/assembly_audit.md`. Compare v1/v2 in the same Slurm job to measure benefit. Both still execute independent head FMAs; neither this evidence nor the 8:1 logical source-load model proves an 8:1 DRAM reduction.
