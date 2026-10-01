# Grouped-head FP32 kernel assembly audit

Date: 2026-10-01. Scope: read-only inspection of the downloaded `joint_online.s` in this directory. No compilation, execution, SSH command, or hardware counter measurement was performed for this audit.

## Specialization and control flow

The function beginning at `joint_online.s:33088` is the OpenMP extracted body of `run<8, false>`: group size 8, counters disabled. This function contains both measured and unmeasured tile branches; locating a spill somewhere inside the function is insufficient to identify a feature-loop spill.

At lines 33599–33604, a zero sampling period branches to `.LBB16_258` at line 33628. This is the unmeasured tile path. Its feature loop is `.LBB16_306` at line 36506; the edge loop used when all eight heads are active is `.LBB16_611` at line 36761. The edge-loop path inspected below has no clock calls.

## Source reuse is present in the generated instructions

Lines 36769–36771 obtain `sources[e]` and calculate the source feature address. Line 36772 executes one masked source-vector load:

```asm
vmovups (%rcx,%r8,4), %zmm17 {%k1} {z}
```

The same `zmm17` then feeds eight independent FP32 accumulator updates:

| Head | Assembly line | Weight operand | Accumulator |
| --- | ---: | --- | --- |
| 0 | 36773 | `5696(%rsp,%rdi,4){1to16}` | `zmm11` |
| 1 | 36774 | `5952(%rsp,%rdi,4){1to16}` | `zmm9` |
| 2 | 36775 | `6208(%rsp,%rdi,4){1to16}` | `zmm13` |
| 3 | 36793 | `6464(%rsp,%rdi,4){1to16}` | `zmm10` |
| 4 | 36797 | `6720(%rsp,%rdi,4){1to16}` | `zmm15` |
| 5 | 36801 | `6976(%rsp,%rdi,4){1to16}` | `zmm12` |
| 6 | 36805 | `7232(%rsp,%rdi,4){1to16}` | `zmm16` |
| 7 | 36809 | `7488(%rsp,%rdi,4){1to16}` | `zmm14` |

Each instruction is `vfmadd231ps`, with `zmm17` as the shared source operand and a different destination accumulator. The eight heads use separate scalar weights; sharing the source vector does not share their attention values or online states.

The loop advances the edge index and checks its bound at lines 36757–36760, reached by the jump at line 36810. The accumulator registers remain live across these edge iterations. There is no ZMM accumulator store to the stack, accumulator reload, or function call in this edge-loop path.

## Stack weights are not accumulator spills

The `%rsp` operands in the eight FMA instructions are the intentional `weights[head][e]` block buffer. Each head occupies 64 FP32 weights, hence the 256-byte separation between weight offsets. For example, line 34396 stores computed exponential weights into `5696(%rsp,%rdi,4)`.

The edge source indices at `1856(%rsp,%rdi,4)` are also intentional block data: lines 34122–34123 copy source indices into that array. Neither access should be described as a spilled feature accumulator.

The U feature accumulators are loaded and rescaled before processing the current neighbor block at lines 36537–36635. After all edges in that block have been consumed, lines 36873, 36903, 36911, 36918, 36926, 36933, 36940, and 36948 store the eight accumulators back to worker-local U. This is one accumulator load/store cycle per neighbor block and feature vector, rather than one per edge.

Spills around score generation and exponential calls exist elsewhere. They are outside the inspected feature-edge loop and do not establish per-edge accumulator spilling.

## Remaining instruction overhead

The generated G8 body retains runtime active-head guards even when all eight heads are active. For example, line 36776 checks whether head 3 is active, followed by analogous `cmp`/conditional branches before later head updates. A full-eight-head specialization could remove these guards while preserving a separate partial-group path. Its performance benefit has not been measured by this audit.

The compiler also selects a scale vector of 1 when a head does not need rescaling, then executes an unconditional vector multiplication when loading U. Examples are `vmulps` at lines 36538, 36577, 36587, 36596, 36606, 36615, 36626, and 36635. Thus a zero logical rescale count does not imply that the generated feature loop executed no scaling instructions. Avoiding multiplication by 1 is a possible optimization, subject to numerical consistency and performance measurements.

## What this audit establishes

The generated unmeasured G8 feature-edge loop performs one source feature-vector load followed by eight independent FMAs, without per-edge accumulator stack spill/reload. The intended software source reuse is therefore present in this assembly artifact.

Actual DRAM bytes, cache misses, memory bandwidth, and speedup remain unknown from this inspection. Logical load counts are not hardware traffic measurements, and an eightfold reduction in logical source loads does not establish an eightfold reduction in DRAM traffic or runtime. The aggregate-first arithmetic amplification and per-head accumulator updates remain.
