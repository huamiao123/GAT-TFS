# TFS-Online 问题记录

2026-09-28：原 TFS v3 内核使用静态无权邻接、固定 128×128、BF16/VNNI 和 AMX tile_dpbf16ps，直接累加输出 C。Vanilla GAT 的 attention 是动态、per-head、per-row online rescale，且当前严格对 FP32 Reference 验证；原内核无法不改语义地直接调用。处理：保留原 degree-sort/R-panel/16 行局部 tile 调度，先做 FP32 正确性与性能适配；AMX 指令版本仍待实现。来源及源码哈希在 GAT/TFS_SOURCE_AUDIT.md。

2026-09-28：初次 smoke 命令未加载运行时 MKL 模块，出现 libmkl_intel_lp64.so.2 找不到。加载 compiler 与 MKL 模块后复跑通过。证据 GAT/runs/tfs_smoke1024_profile.log。

2026-09-28：共享节点上的 16 线程绝对时间波动显著。10782767 在 qhcn006 的 fused Layer 2 约 1.87–1.92 s；10782776 在提交时空闲的 qhcn100 为 0.288–0.306 s。处理：仅把 qhcn100 结果作为初步趋势，并增加同节点、同线程、同 panel 的 transform-first 控制路径。正式论文速度需独享节点、更多重复和绑定/NUMA 审计。

2026-09-28：直接比较 16 worker TFS fused 与原有单线程稀疏 AVX 路径会夸大可归因于 fusion 的收益。处理：新增 panel_transform_first_fp32 控制路径；同 16 worker 下 Layer 2 TFS fused 慢约 1.74×，1 worker 下慢约 2.30×。原始日志保留。
