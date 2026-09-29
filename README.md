# Multi-Head GAT / TFS-Online 实验归档

本仓库保存三层 Vanilla GAT 的 Online Softmax 与 TFS-style aggregate-first 前向计算实验，以及原 TFS 论文、源码和技术交接文档。归档时间：2026-09-29。

## 目录

- [`GAT/`](GAT/)：完整复制自 `D:\Apaper\GAT`，包含原 TFS 源码、源码压缩包、中文论文和实现型技术交接文档。
- [`implementation/`](implementation/)：复制自服务器 `wzh/GAT` 的 GAT 实现、构建与 Slurm 脚本、实验原始日志、结果总结和小型 smoke 数据。
- [`implementation/runs/TFS_ONLINE_EXPERIMENT_SUMMARY.md`](implementation/runs/TFS_ONLINE_EXPERIMENT_SUMMARY.md)：当前 TFS-Online 正确性、分段计时与性能分析。
- [`implementation/runs/EXPERIMENT_SUMMARY.md`](implementation/runs/EXPERIMENT_SUMMARY.md)：前期 Reference / FP32 Online / AVX-512 Online 实验。
- [`implementation/TFS_SOURCE_AUDIT.md`](implementation/TFS_SOURCE_AUDIT.md)：原 TFS 源码与当前适配范围的核对。

## 当前实现

模型为 `D_in → 8×32 → 8×32 → 1×C`。每个 attention head 的权重和 softmax 独立，输入为含 self-loop 的 destination-row CSR。当前代码是三层 **FP32 前向与性能基线**，没有反向传播或训练循环。

共有六条路径：`reference`、`online_fp32`、`online_avx512`、`tfs_online_reference`、`tfs_online_fused`，以及使用相同 panel/线程调度的 `panel_transform_first_fp32` 对照。TFS 路径用 `H(Wa_L)` / `H(Wa_R)` 计算与标准 GAT 等价的 attention，Online Softmax 聚合原始 `H`，再执行 GeMM。`tfs_online_reference` 落地完整 `U=AαH`；`tfs_online_fused` 使用线程局部 tile 立即消费 `U`，不落地完整的全局 `U`。

**当前 fused 路径是 FP32 TFS-style tile-local fusion，尚未执行 BF16/AMX tile 指令。** 它复用了原 TFS 源码中的 degree sort、R-panel、16 行 tile 和 OpenMP panel 调度思想；准确范围见源码审计，不应称为 AMX 加速结果。

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

完整三层 TFS 路径对 Reference 的最大绝对误差为 `1.84774399e-6`，相对 L2 为 `3.58968159e-7`。Layer 2 的 TFS aggregate-first 稀疏特征处理量是 transform-first 的 **8 倍**。同为 16 worker 和相同 panel 调度时，TFS fused 的 Layer 2 为 0.297 s，对照路径为 0.170 s，即 fused 约慢 1.74 倍。单线程复核 `10782790` 也未显示净加速。共享节点实验只支持初步趋势；详细条件、分段时间和原始日志见上述实验总结。

## 复现与数据边界

构建及运行入口见 [`implementation/README.md`](implementation/README.md) 和 [`implementation/DATASETS_AND_TOOLCHAIN.md`](implementation/DATASETS_AND_TOOLCHAIN.md)。仓库包含 `implementation/data/smoke1024.gatbin`。约 94 MB 的 `arxiv.gatbin` 是由官方 ogbn-arxiv 数据转换得到的派生文件，未纳入 Git；使用 `implementation/tools/prepare_arxiv.py` 生成。编译产物 `implementation/build/` 也未纳入 Git。除这两类可再生成的内容外，服务器 `wzh/GAT` 的源码、脚本、文档和 `runs/` 原始实验记录均已归档。
