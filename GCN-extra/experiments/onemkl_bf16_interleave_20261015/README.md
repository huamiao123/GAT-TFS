# Interleaved NUMA comparison: BF16 oneMKL versus original TFS

The launcher runs the frozen `onemkl_bf16_twostep_packed_20261015` benchmark under `numactl --interleave=all`. It changes no kernel source or compiler flags. The existing benchmark driver compares original TFS, FULL DegreeSort, FULL Identity, BF16 oneMKL two-step, prepacked-W BF16 oneMKL two-step, and FP32 oneMKL two-step in one process, with a warmup and five rotated measured repetitions per method.

The launcher first records the NUMA policy from a child process; the original benchmark job script records it again. Both must report `policy: interleave`. `record_run.py` checks this before appending the smoke or fixed-17 event to the three GCN-extra handoff documents.

Fixed-17 run: job `10973480`, node `qhcn509`, 32 CPUs, Slurm `intel` shared partition. Run directory: `/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/onemkl-bf16-interleave-20261015-171946/full17`. The binary is frozen at SHA-256 `97109515e895f3f755a0dbfc9b31d7ceca02ffc4bbf28e182df62a24e25c9f5f`.

The results and limits are explained in `GCN-extra/docs/ONEMKL_BF16_INTERLEAVE_TFS_20261015.md`. This is a shared-node diagnostic comparison; it does not isolate the causal effect of turning interleave on or off because previous default-NUMA runs used other nodes.
