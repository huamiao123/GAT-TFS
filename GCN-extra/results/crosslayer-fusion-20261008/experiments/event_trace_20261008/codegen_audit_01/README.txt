Event trace build01 assembly audit

Status: PASS

trace_i and dyn_i retain two independently decoded BF16 feature vectors on a shared (mask11) event: 16 vpmovzxwd + 16 vpslld + 16 vaddps. No CSE collapsed the intended independent-decode control.

trace_s_cached, trace_s_rep and dyn_s shared events decode once: 8 vpmovzxwd + 8 vpslld + 16 vaddps. Both destination additions remain; single-destination events use 8/8/8.

Cached trace modes retain separate per-row next-current-head loads when consumed rows remain nonempty. Representative mode chooses one of two current-head load sites at event entry; take11 chooses row0. PF1 next-index loads and four prefetch hints per eligible consumed row remain separate in all treatments.

Trace hot loops read packed event bytes and shift masks. Dynamic head-value comparison instructions are replaced by trace decoding and mask control, not by a free oracle.

Each examined pair reducer has scalar stack traffic, but no direct rsp/rbp-based memory instruction involving an xmm/ymm/zmm register. This narrow screen supports no obvious direct vector-accumulator spill, not absence of all spills.

dyn_i/dyn_s pair-window instruction streams match the prior access-prefetch build after address normalization.

Limits

Let E be row-edge consumptions, S be mask11 events, T=E-S. Source feature-decode requests are E for independent and T for shared modes; additions are 128E. The inspected branch paths support the corresponding vector instructions, but software Profile counters are not retired-instruction PMU measurements.

Source current-index requests are E in cached modes and T in representative mode. PF1 index loads, row bases, degree/plan headers, loop control, compiler stack traffic and speculative execution are additional. Neither 4S fewer source index bytes nor 256S fewer source feature bytes means equal DRAM-byte savings.

Histogram-counted payload bytes sum ceil(record_events/4), with one count header per pair-window and one tile-offset read per nonempty tile. The assembly confirms packed-byte outer loops; it does not measure memory hierarchy traffic.

Static mnemonic totals include mutually exclusive paths and epilogues. In particular 24 static decodes in a shared reducer do not mean 24 decodes per event.

No speedup, bottleneck, AMX/AVX overlap, bandwidth saturation, branch-miss rate, cache hit rate, or absence of indirect stack aliasing is established by this audit. Benchmark and hardware evidence are separate.

Baseline equality here covers the two dynamic pair reducers only; no claim of whole-application byte identity or identical placement is made.
