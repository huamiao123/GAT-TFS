# P2 fixed-work overlap evidence

Hardware run: `GCN-extra/runs/avx-amx-overlap-20261005-112746`,
Slurm job10869371, qhcn014, one physical core (CPU51/socket1/core19).
These files copy the immutable postprocessing directory
`GCN-extra/results/p2-overlap-20261005-v2` on the server. The earlier
postprocessing directory remains preserved; it has the same measured data.
Read `P2_OVERLAP_RESULTS_20261005.md` for boundaries and raw repetitions.

The paired test is a local handoff microbenchmark, not complete two-layer
inference. The server's GNU objdump2.30 does not recognize AMX.
`serial_asm.txt` and `interleaved_asm.txt` decode actual ELF function bytes
using `experiments/avx_amx_overlap_p2_20261005/decode_elf.py` and
iced-x861.21.0; old objdump text supplies demangled symbol/address mapping.
ELF symbol sizes and PT_LOAD mappings select bytes; all instructions decode
validly. See `assembly_audit.json` for binary/decoded hashes. Static counts
are not dynamic work counts. Decoder API source:
[iced-x86 project documentation](https://pypi.org/project/iced-x86/).
