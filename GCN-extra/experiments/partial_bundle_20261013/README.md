# Fixed-quantization partial bundle

Implemented and measured, 2026-10-13/14. Static unit-weight 128→128→128
GCN-extra inference compute with original random source H/W; no classifier
accuracy claim.

## Review

- [Protocol](PROTOCOL.md)
- [Complete report](../../results/partial-bundle-20261013/GCN_EXTRA_PARTIAL_BUNDLE_RESULTS.md)
- [Actual compiled kernel](../../results/partial-bundle-20261013/source_snapshot/bundle_kernels.hpp)
- [Actual compiled runtime](../../results/partial-bundle-20261013/source_snapshot/bundle_runtime.hpp)
- [Numerical, work, and measurement audit](../../results/partial-bundle-20261013/audit.json)
- [Raw timing and evidence](../../results/partial-bundle-20261013/logs/bundle-10944719/)

`generate_bundle.py` derives the isolated executor from passing state-reuse
run05. Frozen ScalarReducer, direct packing, precise EPI and canonical controls
remain byte-identical. G=1/2/4 only buffers separately quantized partials and
changes output panel/C staging lifetime. Per-output AMX order remains intact.

Correctness: five small fixtures and five diagnostic graphs pass. Performance:
shared node qhcn198, 32 physical cores, one socket, close/cores, default NUMA.
M64 fixedG4 descriptive five-graph GM is 1.0203× FAST and 1.0254× ACCURATE
relative to matched PACK_EPI. FULL Q1 control also shifts, so the whole timing
difference cannot be attributed to C staging. This remains a mechanism control;
it does not replace canonical FULL or constitute a formal paper speedup.

## Server artifacts

Workspace: `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra`.

- Frozen build: `runs/partial-bundle-build-20261013-225110/`
- Smoke: `logs/bundle-10944718/`, one core, timing excluded from performance.
- Diagnostic5: `logs/bundle-10944719/`, all 1725 main samples retained.
- Results: `results/partial-bundle-20261013/`

Read current server AGENTS/Skill/specs before execution. Build source layout and
compiler flags are in the frozen manifest and commands.txt. Original TFS/MKL
sources and previous snapshots are untouched. No graph data or executable is
needed in Git; small fixture data and source/binary/input hashes are archived.
