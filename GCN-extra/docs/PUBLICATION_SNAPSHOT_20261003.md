# GCN-extra 当前实验快照

日期：2026-10-03。独立分支：`gcn-extra-experiments-20261003`。

本目录保存推理阶段邻居归约与投影冗余消减实验。沿用 ICPP TFS 的 DegreeSort、16 个目标行分块、局部 SpMM→GeMM 融合及 AMX；没有全局 AH 中间矩阵。当前实验不是训练实验，也不是训练后分类任务准确率实验。

## 结果入口

- [25 图覆盖完整报告](GRAPH_SUITE_RESULTS_20261003.md)：所有图的状态、高平均度图的全部分组、两层时间、精度、准备时间及边界。
- [机器可读比较结果](GRAPH_SUITE_RESULTS_20261003.csv)：单层/两层中位数、P25/P75、测量次数、精度、分阶段时间及源 run。
- [分组加速比热图](figures/graph_suite/graph_suite_heatmap.png)；同目录提供 PDF 和平均度散点图。
- [最初实现及基线控制实验](FIRST_RESULTS_20261003.md)。
- [七种邻居分组实验](BLOCK_SWEEP_RESULTS_20261003.md)。
- [Slurm 最终状态](GRAPH_SUITE_ACCOUNTING_20261003.txt)。

## 当前状态

尝试25张 canonical 图，19张完成，两阶段合计2028条数值校验、0条失败。六张非单位边权输入不兼容当前原TFS单位邻接算子，保留失败证据、不修改数据：Kron、cage15、FullChip、scircuit、sx-stackoverflow、rajat31。

| 指标 | 当前结果 |
|---|---:|
| 事后最佳 Fast B，几何平均加速比 | 1.350× |
| 固定 Full Fast，几何平均加速比 | 1.290× |
| 事后最佳 Accurate B，几何平均加速比 | 1.297× |
| 最佳 Fast 超过原TFS去清零的成功图 | 17/19 |

加速比基线是 `original_nozero`，完整未改动原TFS仍单独测量。最佳B是事后选择，不是已实现的自适应调度。Road-USA和RGG保留负结果；Mycielskian19的Full慢于B64。

统一D=F=128、32物理核心、独享节点。除Friendster外每路径1次预热、10次测量；Friendster各路径5次测量，B8/16/32/64/Full，B2/4未测。prepared两层E2E包括两次完整kernel、ReLU及层间BF16转换，初始输入转换、DegreeSort、权重packing、数据加载另计。

## 源码、执行过程及原始记录

- 当前实现：[src/bench.cpp](../src/bench.cpp)。
- 未修改的原TFS：[original/amx_tfs_v3.cpp](../original/amx_tfs_v3.cpp)。
- 基线清零控制：[src/original_nozero.hpp](../src/original_nozero.hpp)。
- 数学、精度及计时约束：[IMPLEMENTATION_CONTRACT.md](../IMPLEMENTATION_CONTRACT.md)。
- 构建、提交、测量及汇总：[scripts/](../scripts/)。
- 每次构建/正确性/性能/提交记录：[runs/](../runs/)，包括源快照、命令、环境、哈希、stdout/stderr、CSV及状态。
- 持续记录：[ALL_EXPERIMENTS.md](handoff/ALL_EXPERIMENTS.md)、[EXPERIMENT_ISSUES.md](handoff/EXPERIMENT_ISSUES.md)、[PROJECT_PROGRESS.md](handoff/PROJECT_PROGRESS.md)，以及结构化`events.jsonl`。
- 全图主批次：`runs/formal-10862002`；Friendster补测：`runs/formal-10862158`；Road-USA补测：`runs/formal-10862162`。
- 其他正式结果：`formal-10861421`、`formal-10861923`、`formal-10861974`，均保留。
- LP64大图准备阶段崩溃与ILP64补测分开保存。仅切换MKL索引宽度，AMX/local内核及浮点精度不变。待排队取消的10862093也保留。

## 可复现证据

完整证据压缩包：[evidence-graph-suite-completed-20261003.tar.gz](../evidence-graph-suite-completed-20261003.tar.gz)。包括全部源代码、运行快照、编译产物、原始结果、过程与交接记录；早期证据压缩包也保留。外部数据集不包含在本仓库，图数据路径和哈希保存在运行记录中。

SHA256：`ca075624e50bf2b69e127f00287ff106ecf1fe6f2423310a648742d2ffa057ee`。

可浏览的单独编译产物继续忽略，由完整证据压缩包保存；所有实验源码快照、状态和结果直接纳入Git。GCN-extra设置`-text`，避免Git换行转换破坏SHA256。

服务器工作目录：`/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra`。原TFS源码与数据目录`yx`只读。实验全部完成，当前没有运行中的GCN-extra任务。

## 已发现的真实任务数据与未完成工作

见[真实GCN数据清单](YX_REAL_GCN_ASSETS_20261003.md)。当前随机H/W性能结果尚未使用真实节点特征与训练后权重。真实模型精度、真实模型尺寸、重复节点复验及Full/B64反转原因尚未完成，论文级泛化结论仍为UNVERIFIED。
