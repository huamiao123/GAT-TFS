# Multi-Head GAT / TFS-Online 实验归档

本仓库保存三层 Vanilla GAT 的 Online Softmax 与 TFS 前向计算实验，以及原 TFS 论文、源码和技术交接文档。归档起始时间：2026-09-29。

2026-10-02 新结果：[保留 DegreeSort/TR16 的跨 head PH 融合与混合调度](baseline/docs/ICPP_TFS_HYBRID_RESULTS_20261002.md)、[前两层 TFS + 第三层标准 GAT 的同场实验](baseline/docs/ICPP_TFS_ADAPTIVE_RESULTS_20261002.md)。products 上混合路径三层 3411 ms，强 B0 BF16 3845 ms，探索性加速 1.127×；完整三层 TFS 4033 ms。arxiv 上混合路径仍慢于 B0（105 ms 对 68 ms）。这是明确标记的混合数据流；L2 的 8× 稀疏宽度放大未消除。随机权重的原始精度门槛和训练任务精度尚未验收。

2026-10-02 最新本地/服务器实验：[保留ICPP TFS的块投影与AMX PH报告](baseline/docs/ICPP_TFS_BLOCK_RESULTS_20261002.md)、[两项瓶颈及原始文献调研](baseline/docs/ICPP_TFS_TWO_BOTTLENECKS_RESEARCH_20261002.md)。保留DegreeSort/TR16/真实AMX融合，products L2重复投影指令少13.8–35.6倍，但最佳完整三层仍慢于纯GAT B0。双head读取复用和块投影分别有完整真实图证据；8行/向量打包的PH只有局部micro收益，尚无整模型集成或master精度验收。原始日志、源码快照和失败记录完整保留。

2026-10-01 最新：新增保留原 ICPP TFS **Degree Sort、TR16逐邻居AMX融合和输出TMM驻留**的 [`baseline/tfs_online/`](baseline/tfs_online/README.md)。Online Softmax 在输出维 `V[32]` 上 rescale；完整三层 products 相对原B1为1.143×探索性加速，arxiv略慢，均未超过强B0。真实图master精度门仍未通过。详见[本轮实现与结果](baseline/docs/ICPP_TFS_ONLINE_RESULTS_20261001.md)。此前FP32 local-U/grouped-head候选分别归档，不能当作原ICPP TFS的改进结果。

2026-09-30 新增独立 [`baseline/`](baseline/README.md)，按新实现合同搭建 R0 FP32 correctness oracle、B0 强标准 GAT（不使用 TFS）与 B1 原 TFS 风格加权 AMX 路径。正式性能主基线改为 B0；旧 `reference` 仅作正确性参考，以下旧速度表保留为历史诊断记录。新 suite 采用精确 max 预扫描和第二遍融合聚合，包含 fixed-p、micro、full-layer、full-model 以及独立 FP64 oracle 验证。

## 目录

- [`GAT/`](GAT/)：完整复制自 `D:\Apaper\GAT`，包含原 TFS 源码、源码压缩包、中文论文和实现型技术交接文档。
- [`implementation/`](implementation/)：复制自服务器 `wzh/GAT` 的 GAT 实现、构建与 Slurm 脚本、实验原始日志、结果总结和小型 smoke 数据。
- [`implementation/runs/TFS_ONLINE_EXPERIMENT_SUMMARY.md`](implementation/runs/TFS_ONLINE_EXPERIMENT_SUMMARY.md)：当前 TFS-Online 正确性、分段计时与性能分析。
- [`implementation/runs/EXPERIMENT_SUMMARY.md`](implementation/runs/EXPERIMENT_SUMMARY.md)：前期 Reference / FP32 Online / AVX-512 Online 实验。
- [`implementation/TFS_SOURCE_AUDIT.md`](implementation/TFS_SOURCE_AUDIT.md)：原 TFS 源码与当前适配范围的核对。
- [`docs/GAT_TFS_AMX_问题审计与方案设计全记录.md`](docs/GAT_TFS_AMX_问题审计与方案设计全记录.md)：2026-09-29 问题审计与实施路线。
- [`implementation/IMPLEMENTATION_STATUS_20260929.md`](implementation/IMPLEMENTATION_STATUS_20260929.md)：依据审计实施的最新代码范围、轻量验证和待验收事项。

## 2026-09-29 历史实现

模型为 `D_in → 8×32 → 8×32 → 1×C`。每个 attention head 的权重和 softmax 独立，输入为含 self-loop 的 destination-row CSR。当前代码只实现前向，没有反向传播或训练循环。

原有六条 FP32/AVX 路径：`reference`、`online_fp32`、`online_avx512`、`tfs_online_reference`、`tfs_online_fused`，以及使用相同 panel/线程调度的 `panel_transform_first_fp32` 对照。TFS 路径用 `H(Wa_L)` / `H(Wa_R)` 计算与标准 GAT 等价的 attention，Online Softmax 聚合原始 `H`，再执行 GeMM。`tfs_online_reference` 落地完整 `U=AαH`；`tfs_online_fused` 使用线程局部 tile 立即消费 `U`，不落地完整的全局 `U`。

**上述历史 fused 路径是 FP32 TFS-style tile-local fusion，尚未执行 BF16/AMX tile 指令。** 它复用了原 TFS 源码中的 degree sort、R-panel、16 行 tile 和 OpenMP panel 调度思想；准确范围见源码审计，不应称为 AMX 加速结果。

2026-09-29 另增实验性 `gat_tfs_amx`：真实 AMX-BF16 `head-as-row` 加权聚合、BF16 高低两部分补偿与局部 FP32 GeMM。1024 节点小图的逐层和三层输出通过现有 FP32 容差；ogbn-arxiv 与真实 checkpoint 尚未验收，不能据此声称 AMX 加速。

## 初步结果

ogbn-arxiv 图：169,343 个节点、2,484,941 条含 self-loop 的 CSR 边，128 输入维、40 类。16 线程共享节点实验 `10782776` 的两次计时中位数如下（秒）；原三条路径的稀疏循环仍为单线程，因此跨实现的直接速度比包含线程调度差异。

| 路径 | Layer 2 | 三层前向 E2E |
| --- | ---: | ---: |
| Reference | 0.625 | 1.384 |
| FP32 Online | 0.791 | 1.744 |
| AVX-512 Online | 0.696 | 1.543 |
| TFS Online Reference | 2.296 | 4.095 |
| TFS Online Fused | 0.297 | 0.558 |
| 匹配调度的 transform-first 对照 | 0.170 | 0.410 |

这是修正计时门槛之前的历史记录。审计发现旧三层 E2E 计时包含结果比较等额外操作；新版本已修正计时边界，旧数值不能直接当作正式加速比。

完整三层 TFS 路径对 Reference 的最大绝对误差为 `1.84774399e-6`，相对 L2 为 `3.58968159e-7`。Layer 2 的 TFS aggregate-first 稀疏特征处理量是 transform-first 的 **8 倍**。同为 16 worker 和相同 panel 调度时，TFS fused 的 Layer 2 为 0.297 s，对照路径为 0.170 s，即 fused 约慢 1.74 倍。单线程复核 `10782790` 也未显示净加速。共享节点实验只支持初步趋势；详细条件、分段时间和原始日志见上述实验总结。

## 复现与数据边界

构建及运行入口见 [`implementation/README.md`](implementation/README.md) 和 [`implementation/DATASETS_AND_TOOLCHAIN.md`](implementation/DATASETS_AND_TOOLCHAIN.md)。仓库包含 `implementation/data/smoke1024.gatbin`。约 94 MB 的 `arxiv.gatbin` 是由官方 ogbn-arxiv 数据转换得到的派生文件，未纳入 Git；使用 `implementation/tools/prepare_arxiv.py` 生成。编译产物 `implementation/build/` 也未纳入 Git。除这两类可再生成的内容外，服务器 `wzh/GAT` 的源码、脚本、文档和 `runs/` 原始实验记录均已归档。
