# 三层 Vanilla GAT：细分计时与纯 GAT 主基线

日期：2026-09-29。源码和 Slurm 作业见 `src/amx_head_row.hpp`、`src/gat_tfs_amx.cpp`、`src/gat_tfs_online.cpp`、`scripts/run_products_fine.slurm`、`scripts/run_pure_gat_baseline.slurm`。原始日志分别在 `fine-timing-10797770/` 与 `pure-gat-baseline-10803234/`。两次作业都在 `intel` 共享分区单节点、16 CPU、OMP/MKL 16 线程，分别使用 `qhcn017` 与 `qhcn026`。两作业的源码、二进制、数据 SHA-256 清单均在各自 run 目录。

## 比较口径

**主基线是 `reference`：原始 transform-first Vanilla GAT，显式生成 score 与 softmax 权重并聚合投影后的特征。** `online_fp32` 与 `online_avx512` 是未使用 TFS 调度的 Online Softmax 对照。`panel_transform_first_fp32` 使用与 TFS 相同的 Degree Sort、R=64 panel 和 OpenMP 调度，只用于拆解调度贡献；它不再充当主基线。当前 `reference` 是本工程 C++ 实现，未测 DGL GAT 实现；其 sparse/attention 循环主要是单线程，虽然环境给了 MKL/OMP 16 线程。

速度表来自 `10803234` 同一共享节点、关闭块级细分计时的二进制。每条路径单独启动进程；rep0 预热，rep1/rep2 测量。时间是完整三层前向，不含图加载、Degree Sort 预处理、模型初始化和 checksum。两次测量取算术中点；加速比为纯 GAT reference 时间除以该路径时间。

| 图 | 路径 | 三层前向中点 / s | 相对纯 GAT reference |
| --- | --- | ---: | ---: |
| ogbn-arxiv | reference | 1.405917 | 1.000× |
| ogbn-arxiv | FP32 Online | 1.702060 | 0.826× |
| ogbn-arxiv | AVX-512 Online | 1.590009 | 0.884× |
| ogbn-arxiv | 同调度 transform-first 控制 | 0.370607 | 3.794× |
| ogbn-arxiv | FP32 TFS-Online fused | 0.530881 | 2.648× |
| ogbn-arxiv | AMX TFS-Online | 1.230435 | 1.143× |
| ogbn-products | reference | 70.963990 | 1.000× |
| ogbn-products | FP32 Online | 77.838117 | 0.912× |
| ogbn-products | AVX-512 Online | 80.493276 | 0.882× |
| ogbn-products | 同调度 transform-first 控制 | 8.844274 | 8.024× |
| ogbn-products | FP32 TFS-Online fused | 22.696066 | 3.127× |
| ogbn-products | AMX TFS-Online | 29.961600 | 2.369× |

这些是**整体实现对纯 GAT reference 的比值**，不能归因于 TFS 聚合本身：同调度 transform-first 控制在两图上都更快。特别是 products 的 TFS/AMX 精度未过固定的最大绝对误差门槛，products 速度仅作诊断，不能当作有效实现的加速验收。

## 细分计时范围

`10797770` 在同一节点上运行 profile 与正确性。新增字段：FP32 的 block max、softmax 指数与分母、rescale、panel 清零、并行区 wall；AMX 的块结果初始化、block max、权重 exp/BF16、特征缓冲清零、特征 BF16 打包、tile config/load/compute/store、tile 结果提取、块结果与 running accumulator 合并、合并中 max 更新的子集、局部 GeMM、归一化、激活、结果写回及并行区 wall。

以下 `worker_s` 是所有 16 个线程相加的 CPU 时间；`layer_wall_s` 是该层实耗时。子字段与父字段重叠，例如 `feature_bf16_pack_worker_s` 包含在 `packing_worker_s` 内，不能重复求和。逐块计时会显著增加指令与时间开销，因此这些 profile 数值只用于定位，不与上表 speed 二进制的时间直接相减。

### ogbn-products 第二层，D=256、8 heads×32

| 路径和环节 | 线程累计时间 / s | 说明 |
| --- | ---: | --- |
| FP32 TFS weighted SpMM | **166.108** | 同调度 transform-first 为 **30.375**；TFS 稀疏特征工作量为 8× |
| FP32 TFS edge score | 24.935 | 同调度控制为 15.675 |
| FP32 TFS online softmax | 8.877 | 其中 block max 1.984、指数与分母 4.236、rescale 0.241；子项不等于父项，剩余含控制和计时开销 |
| FP32 TFS 局部 GeMM | 4.159 | 远低于 weighted SpMM |
| AMX 邻居特征 gather | 16.242 | 从源节点 gather 到邻居块缓冲 |
| AMX 块内处理总计 | **191.079** | 包括下列 BF16 打包与 tile 操作 |
| AMX 动态打包总计 | **135.338** | 其中**特征 BF16 打包 122.988**、特征缓冲清零 2.214、权重 exp/BF16 9.290、block max 0.845 |
| AMX tile compute | 29.939 | 特征 BF16 打包约为其 4.11 倍 |
| AMX tile config/load/store | 0.515 / 2.265 / 2.266 | 线程累计时间 |
| AMX 在线块合并 | 6.534 | 其中 max 更新块的 accumulator 合并 0.346，包含缩放和新贡献相加，非纯 rescale 时间 |
| AMX 局部 GeMM | 2.915 | FP32 MKL |
| AMX 归一化、激活、结果写回 | 0.096 / 4.396 / 0.409 | 前两层有 ELU |

该层 profile 的 wall time：FP32 TFS `14.101 s`、同调度 transform-first `5.429 s`、AMX `14.726 s`。AMX 并行区 wall 为 `14.349 s`。AMX 路径累计处理 `5,360,349` 个多 head 邻居块；FP32 路径计数为 `42,882,792` 个 head 邻居块。FP32 TFS 的逻辑稀疏特征元素数为 `258,390,124,544`，同调度 transform-first 为 `32,298,765,568`，正好 8×。

**当前最明确的瓶颈是 FP32 TFS 的 8× 加权稀疏聚合，以及 AMX 路径中逐块的特征 BF16 打包。** products 第二层 FP32 TFS 的 rescale 为 `4,475,967 / 42,882,792 ≈ 10.44%`，但已测 rescale 子时间相对聚合很小；AMX 的合并中 max 更新子集也不是主要耗时。下一步应先减少重复 gather/打包及每 head 的 256 维稀疏处理，而不是优先优化 tile 乘法或独立 rescale。

## 精度与解释边界

小图与 ogbn-arxiv 的 FP32/AMX 检查均通过。arxiv AMX 三层相对 FP32 reference 的 `max_abs=2.4586916e-6`、`relative_L2=1.30553722e-6`。products 的 FP32 TFS 三层 `max_abs=0.00329780579`，AMX `max_abs=0.0064125061`，均超过固定的 `0.003` 门槛；products AMX 的 `relative_L2=1.71394679e-5`。门槛没有修改。这些检查使用固定随机参数，不代表训练后的任务准确率。共享节点及只有两次测量，也不构成正式论文级性能数据。
