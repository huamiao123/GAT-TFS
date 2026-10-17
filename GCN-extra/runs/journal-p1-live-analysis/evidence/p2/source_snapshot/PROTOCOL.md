# P2: fixed-work same-core AVX/AMX handoff microbenchmark

This is the supplied journal plan's section 6.5 mechanism gate, not an
end-to-end inference speedup claim. Use one physical SPR core on shared intel,
default NUMA, original compilation flags and tile configuration. No source
baseline or validated graph kernel is changed. If fixed-work overlap has no
stable benefit, do not add buffering complexity to the full model on that basis.

Each case processes 1024 tiles of 16 destination rows, D=F=128, a fixed degree,
and the same BF16 H/W/CSR source sequence. Cases: hot1024-source pools with
degree1/8/64; random524288-source pools with degree64/128. H data are serially
generated on the allocated core; no explicit NUMA/first-touch optimization.
The names describe working sets, not experimentally proved cache-hit states.

Prepare tile0. SERIAL projects tile t, then reduces/quantizes tile t+1.
INTERLEAVED performs one AMX K-block (4 TDPs) then prepares two rows of tile
t+1, repeated across 2 panels x 4 K-blocks. Both use two identical local
partial/packed buffers, the same CSR addition and prefetch order, the same
weight loads/AMX operations, and the same final FP32 output stores. Packing
tile t+1 remains after all its rows are prepared. No async queue or separate
producer thread is assumed. Reordering independent instructions is the only
intended variable; resulting contention/cache/compiler effects are part of
what is measured. All current AMX output panels start at zero; there is no
repeated projection or output reload introduced to improve AMX utilization.

Require complete FP32 output memcmp equality, finite results, and the same
analytic edge/add/pack/TDP/tile-store counts. Primary warmup1+repeat7 uses
forward/reverse paired ordering. Report all repeats, median/min/max/CV,
sampled per-tile phase diagnostics (CSR/prefetch, load/decode, add, partial
store, pack, AMX A/B load, compute, zero, C store), profile bitwise equality,
and three full-region PMU repeats. Add generic reference cycles to the isolated
PMU helper, retaining per-TID scaling/coverage and unsupported events. A change
in cycles/ref-cycles is only a frequency-related diagnostic, not MHz.

This microbenchmark has two local handoff buffers and final C output. It
does not materialize full AH. It has no ReLU/interlayer conversion/two-layer
classification. Compare within each case/process/node; do not add sampled
times to reconstruct wall time or extrapolate gains directly to GCN E2E.
