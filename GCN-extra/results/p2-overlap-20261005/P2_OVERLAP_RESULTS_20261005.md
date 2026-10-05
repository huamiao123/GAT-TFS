# P2 fixed-work AVX/AMX interleaving microbenchmark

One physical core, shared SPR node, same case/process, default NUMA. This is a local producer/projection handoff test, not two-layer GCN E2E. All five complete FP32 output memcmp gates and ten profile gates passed. Both paths use the same two buffers, real source sequence, FP32 reduction order, BF16 packing, AMX instructions, and output stores. SERIAL projects a tile then prepares the next; INTERLEAVED prepares two rows of the next tile after each current AMX K-block. The names hot/random indicate source working sets, not proved cache-hit states.

| Case | Serial ms | Interleaved ms | Serial/Interleaved | Paired ratio min/median/max | Serial CV | Interleaved CV |
|---|---:|---:|---:|---|---:|---:|
| hot1024_d1 | 1.460 | 1.342 | 1.0879 | 0.993/1.077/1.117 | 0.017 | 0.046 |
| hot1024_d64 | 6.138 | 7.384 | 0.8313 | 0.815/0.828/0.854 | 0.015 | 0.005 |
| hot1024_d8 | 1.941 | 1.920 | 1.0109 | 0.935/1.022/1.059 | 0.017 | 0.028 |
| random524288_d128 | 72.210 | 73.077 | 0.9881 | 0.975/0.992/1.014 | 0.011 | 0.009 |
| random524288_d64 | 36.573 | 36.970 | 0.9893 | 0.981/0.988/0.999 | 0.008 | 0.012 |

A ratio above one favors interleaving. Seven alternating-order paired repetitions are retained. Sampled phase times measure thread instruction issue/stalls and include instrumentation; they are not hardware completion times or additive wall fractions. PMU retains generic cycles/instructions/ref-cycles/cache events and speculative SPR AMX busy, per-TID scaling and enable/disable skew. Neither simultaneously nonzero counters nor a faster interleaved loop alone proves hardware overlap as a unique cause. No extrapolation to model inference or DRAM bandwidth is made.

Each case has 1024 x 16 rows, D=F=128, 32768 TDPBF16PS instructions and 8MiB final FP32 output writes. Producer visits are 16384*degree. Pool BF16 H sizes are 256KiB and 128MiB. All analytic work counters are in overlap_config.csv; no projection work is added to inflate utilization. The microbenchmark does not cover S64/MFULL row-window execution or multithread throughput, so deployment requires a separate same-work full-model gate if this stage justifies it.
