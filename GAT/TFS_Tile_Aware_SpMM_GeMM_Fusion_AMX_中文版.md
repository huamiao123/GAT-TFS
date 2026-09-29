# TFS：面向 Intel AMX 的 Tile 感知 SpMM–GeMM 融合，用于加速 GNN 推理

**匿名作者**

> 说明：本文依据原始英文稿翻译。公式、算法编号、数据表和实验数值保持不变；参考文献的作者、论文题名、会议与期刊信息保留原文，以避免 bibliographic 信息失真。

## 摘要

图神经网络（Graph Neural Network，GNN）推理的每一层通常包含两个连续的矩阵运算：首先执行稀疏邻居聚合（SpMM），随后执行稠密线性变换（GeMM）。传统的两阶段执行方式会在主存中物化一个规模很大的中间矩阵 \(Z\)，由此产生大量内存流量，并成为端到端时延的主要来源。

本文提出 TFS，一种 Tile 感知的融合框架。TFS 直接在 Intel 高级矩阵扩展（Advanced Matrix Extensions，AMX）的 Tile 寄存器内部融合 SpMM 与 GeMM，从而消除中间矩阵的物化。其核心洞察是：只有在严格控制每个 Tile 批次内部的度数分布时，基于 Tile 的稀疏计算才会高效。

TFS 引入了度数感知的 Tile 调度机制：按照节点度数升序排列各行，使同一个 16 行 AMX Tile 批次中的行具有相近的邻居数量，从而将平均 Tile 效率 \(\eta\) 提升至 0.96。

我们在来自 8 个以上应用领域的 25 个稀疏矩阵上评估了 TFS，其中最大图包含 6560 万个节点和 36 亿条边。相较于 Intel MKL，TFS 在算子级别最高实现 6.67× 加速，在端到端推理中最高实现 4.09× 加速。进一步的 AMX 隔离实验表明，相较于等价的 AVX-512 BF16 实现，AMX Tile 可带来 2.8–13.7× 的加速。这证明性能提升的来源是 Tile 级计算密度，而不仅仅是 BF16 数据格式。

## CCS 概念

- 计算方法学 → 并行计算方法；
- 大规模并行与高性能仿真。

## 关键词

图神经网络，稀疏矩阵乘法，算子融合，Intel AMX，BF16

# 1 引言

图神经网络已经成为处理图结构数据的主流方法，广泛应用于社交网络分析 [13]、推荐系统 [22]、分子性质预测 [5] 和交通流预测 [15] 等领域。

在推理阶段，每一层 GNN 都会执行一次消息传递过程，其中包含两个紧密耦合的矩阵运算：

\[
Z=A\cdot H
\qquad
\underbrace{\phantom{Z=A\cdot H}}_{\text{SpMM：稀疏邻居聚合}}
\]

随后执行：

\[
C=Z\cdot W
\qquad
\underbrace{\phantom{C=Z\cdot W}}_{\text{GeMM：稠密线性变换}}
\tag{1}
\]

其中：

- \(A\in\mathbb{R}^{N\times N}\) 是以 CSR 格式存储的稀疏邻接矩阵；
- \(H\in\mathbb{R}^{N\times K}\) 是输入特征矩阵；
- \(W\in\mathbb{R}^{K\times K}\) 是可学习的权重矩阵；
- GNN 推理中常见的隐藏维度为 \(K\in[64,128]\)。

传统执行策略将 SpMM 和 GeMM 视为两个独立的库函数调用，例如先调用 `mkl_sparse_s_mm`，再调用 `cblas_sgemm`。在两次调用之间，完整的中间矩阵 \(Z\) 必须被写入主存。

对于一个包含 \(N=100\) 万个节点、特征维度 \(K=128\) 的图，单独的矩阵 \(Z\) 就占用 512 MB。SpMM 完成后需要写出 \(Z\)，GeMM 开始前又需要重新读入 \(Z\)，因此会额外产生大约 1 GB 的内存流量。这部分流量完全是冗余的，因为 \(Z\) 刚被一个算子生成，就立即被下一个算子消费。

这一物化开销尤其严重，因为 SpMM 本身已经受到稀疏邻居不规则 gather 访问的限制，通常属于内存带宽受限操作。额外的 \(Z\) 读写会进一步加重内存子系统压力。

## 硬件机遇

Intel Sapphire Rapids 处理器引入了高级矩阵扩展 AMX。AMX 是一种基于 Tile 的矩阵加速器，包含 8 个大小为 1 KB 的 Tile 寄存器，每个寄存器最多可表示 16 行、每行 64 字节的数据。

AMX 的关键指令 `TDPBF16PS` 可在一条指令中完成：

\[
C_{\text{FP32}}^{16\times16}
\mathrel{+}=
A_{\text{BF16}}^{16\times32}
\cdot
B_{\text{BF16}}^{32\times16}
\]

AMX 最初面向 Transformer 推理等稠密工作负载，此类任务的数据通常规则且连续。这自然引出了一个研究问题：

> 基于 Tile 的稠密加速器，能否高效支持稀疏图计算？

本文的回答是肯定的，但前提是进行谨慎的硬件—算法协同设计。

## 核心洞察

如果将 SpMM 与 GeMM 融合，使中间结果 \(Z\) 的部分累积值始终保留在 Tile 寄存器内，就可以完全消除中间矩阵的内存流量。这与 FlashAttention [1] 将 \(QK^T\) 和 \(\text{softmax}\cdot V\) 融合、从而将 \(O(N^2)\) 规模的注意力矩阵保留在片上 SRAM 中的思想相似。

然而，朴素融合会立即遇到一个根本问题。AMX 每次处理 16 行，也就是一个 Tile 高度的批次。内核必须循环到该批次中最大度数对应的迭代次数。

假设一个批次中的节点度数分别为 1 和 10000。即使其中 15 行早已处理完邻居，整个 Tile 仍必须执行 10000 次迭代。已经没有邻居的行只能使用零填充，从而造成大量无效计算。

因此，本文得到核心结论：

> 只有严格控制每个 Tile 批次内部的度数分布，基于 Tile 的稀疏计算才会高效。

## 方法概述

本文提出 TFS，即面向 Sapphire Rapids 的 Tile 感知 SpMM–GeMM 融合框架，主要包含三项机制：

1. **特征驻留数据流。**  
   在 AMX Tile 寄存器内部融合 SpMM 累积和 GeMM 乘法，完全消除中间矩阵。

2. **度数感知 Tile 调度。**  
   按照节点度数升序预排序，使每个 16 行批次包含度数相近的节点，减少 Tile 空闲周期。

3. **混合精度 BF16/FP32 流水线。**  
   面向多层 GCN 推理，结合软件预取、智能 `memset` 和 VNNI 打包权重矩阵。

## 实验结果

我们在来自 8 个以上应用领域的 25 个稀疏矩阵上评估了 TFS，图规模从 3.7 万个节点扩展至 6560 万个节点，最大边数达到 36 亿。

在算子级实验中，使用 \(K=128\) 和 32 个线程，TFS 在 24 个 MKL 能够完成运行的数据集中的 21 个上取得领先，胜率为 87.5%。其中，在 GNN 基准数据集 `ogbn-products` 上最高实现 6.67× 加速。

在两层 GCN 端到端推理中，TFS 在 17 个 MKL 可完成且 BF16 精度稳定的数据集中的 15 个上取得领先，最高加速为 4.09×。

在最大图 `com-Friendster` 上，MKL 因 32 位索引溢出而崩溃，而 TFS 可以正常完成计算。

AMX 硬件隔离实验将 AMX Tile 与等价的 AVX-512 BF16 实现进行比较。结果表明，AMX 提供 2.8–13.7× 的额外加速，说明性能提升来自 AMX 的 Tile 级高计算密度，而不是单纯来自 BF16。

## 主要贡献

本文贡献如下。

### 1. 寄存器级稀疏—稠密融合

TFS 通过特征驻留数据流和完整 \(K\) 维 gather，在 AMX Tile 寄存器内部融合 SpMM 与 GeMM，使中间矩阵 \(Z\) 从不被物化。对于百万节点图，每层最多可消除约 1 GB 的冗余流量。

### 2. 度数感知 Tile 调度与效率分析

本文定义 Tile 效率 \(\eta\)，用于表示 Tile 乘加操作中真正执行有效计算的比例。

实验表明，度数升序排列可以将 25 个矩阵上的平均 \(\eta\) 从低于 0.5 提升至 0.96。该排序不仅是性能优化，同时还是融合内核内存管理正确性的前提。

### 3. 硬件—算法协同设计与分析

AMX 隔离实验表明，相较于等价的 AVX-512 BF16 实现，AMX 可实现 2.8–13.7× 加速。由此证明，真正掩盖稀疏执行中不规则 gather 延迟的是 Tile 级计算密度，而不是 BF16 数据格式本身。

尽管本文在 Intel AMX 上实现，但该融合模型也可推广到其他基于 Tile 的加速器。

# 2 背景与动机

## 2.1 GNN 推理计算模式

图卷积网络（Graph Convolutional Network，GCN）的一层通过两阶段消息传递过程更新节点表示。

给定图：

\[
G=(V,E)
\]

其中节点数为：

\[
N=|V|
\]

稀疏邻接矩阵：

\[
A\in\mathbb{R}^{N\times N}
\]

采用压缩稀疏行（Compressed Sparse Row，CSR）格式存储，包含三个数组：

- `indptr[N+1]`：存储每行的起止位置；
- `indices[NNZ]`：存储非零元素对应的列索引；
- `values[NNZ]`：存储边权重。

输入特征矩阵：

\[
H\in\mathbb{R}^{N\times K}
\]

其中每个节点包含一个 \(K\) 维特征向量，GNN 推理中通常取 \(K=64\sim128\)。

GCN 第 \(l\) 层计算为：

\[
H^{(l+1)}
=
\sigma\left(
\hat{A}\cdot H^{(l)}\cdot W^{(l)}
\right)
\tag{2}
\]

其中：

- \(\hat{A}\) 是归一化邻接矩阵，例如加入自环并执行对称归一化；
- \(W^{(l)}\in\mathbb{R}^{K\times K}\) 是第 \(l\) 层的可学习权重矩阵；
- \(\sigma\) 是 ReLU 等非线性激活函数。

如公式（1）所示，该计算可以拆分为：

1. SpMM：  
   \[
   Z=A\cdot H
   \]
   用于聚合邻居特征；

2. GeMM：  
   \[
   C=Z\cdot W
   \]
   用于执行稠密线性变换。

在标准库实现中，两个操作通常分别调用：

- `mkl_sparse_s_mm`；
- `cblas_sgemm`。

中间矩阵：

\[
Z\in\mathbb{R}^{N\times K}
\]

必须在 SpMM 结束后写入 DRAM，并在 GeMM 开始前从 DRAM 重新读入。TFS 的目标就是消除这次内存往返。

## 2.2 瓶颈分析

两阶段执行中的内存流量主要由中间矩阵 \(Z\) 引起。

对于包含 \(N\) 个节点、特征维度为 \(K\) 的图，冗余流量为：

\[
T_{\text{two-step}}
=
\underbrace{N\times K\times4}_{\text{写入 }Z}
+
\underbrace{N\times K\times4}_{\text{读取 }Z}
=
2NK\times4\ \text{bytes}
\tag{3}
\]

当 \(N=100\) 万、\(K=128\) 时：

\[
2\times10^6\times128\times4
\approx1\ \text{GB}
\]

这部分数据没有携带任何新增信息，因为 \(Z\) 生成后会被立即消费。

TFS 将 SpMM–GeMM 的部分结果始终保留在 AMX Tile 寄存器中，因此对于中间矩阵 \(Z\)：

\[
T_{\text{TFS}}=0
\]

消除 \(Z\) 流量还会改变算术强度。

单独执行 SpMM 时，每个非零元素只进行一次乘加，却需要搬运 FP32 数值，其算术强度通常低于 1 FLOP/byte，因此明显处于内存带宽受限区域。

融合 SpMM 与 GeMM 并移除 \(Z\) 流量后，TFS 的有效算术强度约为：

\[
AI_{\text{TFS}}
\approx
\frac{\text{NNZ}\times K+N\times K^2}
{\text{NNZ}\times K}
=
1+\frac{N\cdot K}{\text{NNZ}}
\tag{4}
\]

其中：

- 分子表示总计算量，包括 SpMM 聚合和 GeMM 变换；
- 分母表示消除 \(Z\) 后剩余的主要内存流量。

该比值严格大于 1，并会随着图稠密程度变化达到数倍，从而将融合内核从内存受限区域推向计算受限区域。

以 `ogbn-products` 为例：

- \(N=245\) 万；
- \(\text{NNZ}=1.237\) 亿；
- \(K=128\)。

则：

\[
AI_{\text{TFS}}
=
1+
\frac{2.45\times10^6\times128}
{123.7\times10^6}
\approx3.5\ \text{FLOP/byte}
\]

相较于两阶段的内存受限执行方式，算术强度提高约 3.5×。

## 2.3 Intel AMX 架构

Intel 高级矩阵扩展 AMX 首次引入于 Sapphire Rapids 微架构 [11]。它是集成在 CPU 核心中的 Tile 矩阵加速器。

AMX 提供 8 个 Tile 寄存器，即 T0–T7。每个寄存器大小为 1 KB，可表示 16 行、每行 64 字节的数据。

根据数据类型不同，每个 Tile 可以容纳：

- \(16\times16\) 个 FP32 元素，用作累加 Tile；
- \(16\times32\) 个 BF16 元素，用作源数据或权重 Tile。

关键指令 `TDPBF16PS` 执行：

\[
C_{\text{FP32}}^{16\times16}
\mathrel{+}=
A_{\text{BF16}}^{16\times32}
\cdot
B_{\text{BF16}}^{32\times16}
\tag{5}
\]

一条指令可以完成：

\[
16\times32\times16=8192
\]

次 BF16 乘加，或者按照 VNNI 配对方式理解为 4096 次 FP32 融合乘加。

这种极高的稠密计算密度是 TFS 获得加速的硬件基础。

AMX Tile 通过专用指令管理：

- `LDTILECFG`：配置 Tile 形状；
- `TILELOADD`：加载 Tile；
- `TILESTORED`：存储 Tile；
- `TILEZERO`：将 Tile 寄存器清零。

每个线程在使用 Tile 前，还需要通过：

```c
arch_prctl(ARCH_SET_STATE_USE, 18)
```

显式向操作系统申请 AMX 状态权限。

权重矩阵 \(W\) 会预先打包为 VNNI 格式，即交错存储 BF16 数值对，使每条 `TDPBF16PS` 指令可以直接读取一个 \(32\times16\) 的 BF16 权重 Tile。

当 \(K=128\) 时，VNNI 打包后的权重矩阵大小为：

\[
128\times128\times2=32\ \text{KB}
\]

可以完整放入 L1 数据缓存。

## 图 1：TFS 总体概览

### （a）传统两阶段执行

- SpMM 计算得到 \(Z[N\times K]\)；
- \(Z\) 被写入 DRAM；
- GeMM 再次从 DRAM 读取 \(Z\)；
- 对于 \(N=100\) 万、\(K=128\)，产生约 1 GB 冗余流量。

### （b）TFS 融合执行

AMX Tile 寄存器分配为：

- T0–T3：\(C\) 的 FP32 累加 Tile；
- T4–T5：BF16 源特征 Tile；
- T6–T7：BF16 权重 Tile。

中间结果 \(Z\) 始终保存在 Tile 寄存器中，从不进入 DRAM。

### （c）Roofline 模型变化

融合前的两阶段执行算术强度低于 1 FLOP/byte，属于内存受限。

融合后的 TFS 算术强度约为：

\[
1+\frac{N\cdot K}{\text{NNZ}}
\]

计算向更高算术强度、更加计算受限的区域移动。

## 2.4 面临的挑战

### 1. 稀疏性与 Tile 结构不匹配

AMX 期望输入是稠密的 \(16\times32\) 块，但稀疏图中的邻居列表在数量和索引模式上都高度不规则。

一个包含 16 行的 Tile 批次可能引用数百个不同邻居列，而这些列通常不构成连续内存块。

### 2. 间接内存访问

读取邻居特征 \(H[j,:]\) 时，需要先通过 `A.indices` 获取节点索引，再执行间接寻址。

每次 gather 都可能造成缓存缺失。由于访问模式不规则，硬件预取器通常难以发挥作用。

### 3. 度数偏斜

真实图通常具有高度偏斜的度数分布。

例如，`as-Skitter` 的平均度数为 13，但最大度数为 35455，最大度数与平均度数之比达到 2710×。

当高阶节点与低阶节点被放入同一批次时，整个 Tile 必须执行到该批次的最大度数。其他行会大量空闲，形成 Tile 稀疏计算中特有的负载不均衡。

在本文测试的 25 个矩阵中：

- 10 个矩阵的度数比超过 1000×；
- 3 个矩阵的度数比超过 100000×。

即使度数比只有约 300，如果不采取措施，也可能浪费 30%–50% 的 Tile 周期。

# 3 TFS 融合设计

## 3.1 总体设计

TFS 将传统的：

\[
\text{SpMM}\rightarrow\text{GeMM}
\]

两阶段流水线替换为一个融合内核。

该内核以 16 行为一个 Tile 批次，与 AMX Tile 的物理高度一致。

对于每个批次，TFS 遍历 16 行对应的邻居。每个邻居的特征向量都会被 gather 到本地缓冲区，随后通过 `TDPBF16PS` 与权重矩阵 \(W\) 相乘，并直接累加到输出 Tile 中。

因此：

- SpMM 邻居聚合；
- GeMM 线性变换；

在同一条指令流中完成融合，中间矩阵 \(Z\) 不再物化。

对于 \(K=128\)，一个 \(16\times128\) 输出块需要：

\[
128/16=8
\]

个 \(16\times16\) FP32 累加 Tile。

但 T0–T3 只有 4 个累加寄存器，因此需要分成两个输出 pass，每个 pass 处理 64 列。

## 3.2 特征驻留数据流

TFS 的核心是特征驻留数据流。输出累加器 \(C\) 的 Tile 在邻居聚合和线性变换期间始终驻留在 Tile 寄存器中。

### 算法 1：TFS 融合 SpMM–GeMM 内核

**输入：**

- 已按度数排序的 CSR 图 \(A\)；
- BF16 特征矩阵 \(H\)；
- 采用 BF16 VNNI 格式打包的权重矩阵 \(W\)。

**输出：**

\[
C=A\cdot H\cdot W
\]

```text
1: 对每个 Tile 批次 b（16 行，使用 OpenMP 并行）执行
2:     对每个输出 pass p ∈ {0,1} 执行       // 每个 pass 处理 4 个 C Tile
3:         将累加 Tile T0–T3 清零
4:         遍历该批次 CSR 范围中的每个邻居 j
5:             Gather：将 H[j,0:K] 载入 BF16 缓冲区    // 完整 K 维
6:             Prefetch：预取 H[next_j]
7:             对每个 K-block kb=0,...,K/32-1 执行
8:                 将 buffer[kb] 载入 T4 或 T5          // 源 Tile 交替使用
9:                 T0–T3 += T4/T5 · T6–T7              // 4 次 TDPBF16PS
10:        将 T0–T3 写入 C[b,p·64:(p+1)·64]
```

每个 Tile 批次同时处理 16 个节点。

对于每个输出 pass，内核遍历该批次的全部邻居，将邻居特征向量载入源 Tile T4–T5，再与预打包的权重 Tile T6–T7 相乘。

结果始终以 FP32 精度累加在 T0–T3 中，从而保证归约过程中的数值稳定性。

### 完整 \(K\) 维 gather

一个关键设计是完整 \(K\) 维 gather。

对于每个邻居以及每个输出 pass，内核只将完整的 \(K\) 维特征向量载入 BF16 缓冲区一次，然后在该 pass 内的所有 \(K/32\) 个 K-block 之间复用。

早期实现中，不同输出 Tile 列会重复读取同一个邻居的特征。完整 \(K\) 维 gather 消除了这部分重复访问。

## 3.3 度数感知 Tile 调度

基于 Tile 的稀疏计算效率高度依赖每个 16 行批次内部的度数分布。

处理一个批次时，内核必须循环到该批次中的最大度数。度数较小的行处理完邻居后，只能继续参与零填充计算。

### 未排序情况

一个批次中的度数可能为：

```text
1, 3, 8500, 2, 1, 45, 2, 12000,
1, 7, 3, 5600, 2, 1, 4, 9200
```

此时最大度数为 12000。大量低度节点只执行了很少的有效计算，却被迫跟随高阶节点循环 12000 次，约 82% 的 AMX 计算周期处于无效状态。

### 按度数升序排序后

低度节点被放在一起：

```text
2, 2, 2, 2, 3, 3, 3, 3,
3, 3, 3, 3, 3, 3, 3, 3
```

最大度数只有 3。

高度节点也被放在一起：

```text
9.0K, 9.1K, 9.2K, ..., 10.2K
```

由于批次内各行负载相近，空闲比例可以降低到约 5%。

TFS 因此会在计算前按照节点度数对全部 \(N\) 行进行升序排序。

排序后：

- 低度批次快速结束；
- 高度批次运行时间较长；
- 但同一高度批次中的 16 行均具有相近工作量，因此浪费较少。

### Tile 效率

本文定义 Tile 效率：

\[
\eta
=
\frac{\text{NNZ}}
{H\cdot\sum_b\max_{i\in b}d_i}
\]

其中：

- \(H=16\) 是 Tile 高度；
- \(b\) 表示一个 16 行批次；
- \(d_i\) 是第 \(i\) 行的度数；
- \(\max_{i\in b}d_i\) 是该批次中的最大度数。

该指标表示所有 Tile 乘加操作中真正执行有效邻居计算的比例。

通过交换论证可以证明，按照度数升序排列会最小化：

\[
\sum_b\max_{i\in b}d_i
\]

若把一个高度节点移动到低度批次，该批次的最大值一定增加，而其他批次的最大值不一定下降，因此排序后的连续分组是最优的。

在 25 个测试矩阵上，排序后平均：

\[
\eta=0.96
\]

说明度数感知调度基本消除了 Tile 稀疏计算中的填充浪费。

### 排序开销

度数排序复杂度为：

\[
O(N\log N)
\]

该操作只需要在图预处理阶段执行一次，并可被所有 GNN 层和多次推理调用复用。

例如，对于 \(N=91.6\) 万的 `web-Google`：

- 排序耗时小于 0.1 ms；
- 内核耗时约为 12 ms。

排序开销不足总推理时间的 1%。

所有层都保持相同的排序后行顺序，因此多层推理过程中无需恢复原始排列。若下游任务需要原始节点顺序，只需在最后一层后执行一次逆置换。

## 3.4 AMX Tile 映射

8 个 AMX Tile 寄存器被划分为三组：

1. **T0–T3：FP32 累加 Tile**  
   每个大小为 \(16\times16\)，用于保存融合后的 SpMM–GeMM 输出。

2. **T4–T5：BF16 源 Tile**  
   每个大小为 \(16\times32\)，用于保存 gather 得到的邻居特征。

3. **T6–T7：BF16 权重 Tile**  
   每个大小为 \(32\times16\)，用于保存采用 VNNI 格式打包的权重。

当 \(K=128\) 时，完整的 \(16\times128\) 输出需要 8 个累加 Tile，但只有 T0–T3 可用于累加，所以输出被分成两个 64 列 pass。

在每个 pass 内，内积维度 \(K=128\) 又被划分为：

\[
K/32=4
\]

个 K-block。

源 Tile T4 与 T5 在不同 K-block 间交替使用：

- 偶数 K-block 使用 T4；
- 奇数 K-block 使用 T5。

权重 Tile T6–T7 会针对每个 K-block 从 L1 缓存重新加载。

### 算法 2：一个输出 pass 内的 AMX Tile 执行过程

**输入：**

- Tile 批次行 \([r_0,\ldots,r_{15}]\)；
- 输出 pass 索引 \(p\)；
- 邻居列表 \(\mathcal{N}\)。

```text
1: TILEZERO(T0,T1,T2,T3)                  // 清空累加器
2: for j=0 到 |N|-1
3:     _mm_prefetch(H[N[j+1],:])          // 预取下一个邻居
4:     将 H[N[j],0:K] gather 到 BF16 行缓冲区
5:     for kb=0 到 K/32-1                 // K-block 循环
6:         若 kb 为偶数，Tsrc=T4，否则 Tsrc=T5
7:         TILELOADD(Tsrc,buffer[kb·32:(kb+1)·32])
8:         TILELOADD(T6,W_vnni[kb,p·4+0])
9:         TILELOADD(T7,W_vnni[kb,p·4+1])
10:        TDPBF16PS(T0,Tsrc,T6)           // C0 += src × W
11:        TDPBF16PS(T1,Tsrc,T7)           // C1 += src × W
12:        TILELOADD(T6,W_vnni[kb,p·4+2])
13:        TILELOADD(T7,W_vnni[kb,p·4+3])
14:        TDPBF16PS(T2,Tsrc,T6)           // C2 += src × W
15:        TDPBF16PS(T3,Tsrc,T7)           // C3 += src × W

16: TILESTORED(T0 → C[r,p·64+0:16])
17: TILESTORED(T1 → C[r,p·64+16:32])
18: TILESTORED(T2 → C[r,p·64+32:48])
19: TILESTORED(T3 → C[r,p·64+48:64])
```

## 3.5 软件预取与智能 Memset

### 软件预取

通过 `A.indices` 间接读取邻居特征 \(H[j,:]\) 会产生不规则访问模式，使硬件预取器难以预测。

TFS 显式插入 `_mm_prefetch`，在当前邻居执行 `TDPBF16PS` 时预取下一个邻居的特征向量。

这样可以将 gather 延迟隐藏在 Tile 计算之后。

### 智能 Memset

传统实现会在计算开始前，将完整输出缓冲区：

\[
C[N\times K]
\]

全部清零。

TFS 不执行全局清零，而是在处理当前 Tile 批次前，通过 `TILEZERO` 只清零对应的 16 行。

由于排序后的批次会生成连续输出行，这种按行粒度的清零方式可以避免提前访问尚未使用的内存，并减少缓存污染。

对于大图而言，全局 `memset` 可能会把有用数据驱逐出缓存，智能 `memset` 可以避免这一问题。

## 3.6 混合精度流水线

完整的 GCN 推理包含多层融合内核，并在层间应用非线性激活函数。

TFS 实现了两层 GCN 的混合精度流水线，在性能与数值稳定性之间进行平衡。

本文采用 Kipf 和 Welling [13] 的标准两层配置。该流水线也可以自然扩展到更深模型：

- 所有中间层采用第一层的精度策略；
- 最后一层采用第二层的精度策略。

### 第一层

- 输入：BF16；
- AMX 内部累加：FP32；
- 输出：FP32；
- 激活：FP32 ReLU；
- 层间转换：FP32 转 BF16。

### 第二层

- 输入：BF16；
- AMX 内部累加：FP32；
- 输出：直接存储为 BF16。

由于 AMX 在 FP32 Tile 中累加 SpMM–GeMM 结果，第一层保留 FP32 输出可以防止误差跨层累积。

第二层之后不再有后续层消费该结果，因此直接输出 BF16 即可。

# 4 实验评估

## 4.1 实验设置

### 硬件平台

所有实验均运行在双路 Intel Xeon Max 9462 系统上：

- 微架构：Sapphire Rapids；
- 每个插槽 32 个核心；
- 总计 64 个核心；
- HBM2e 以内存缓存模式运行。

软件环境：

- Intel `icpx 2024.1.0`；
- Intel MKL 2023.2.0。

除非特别说明：

- 使用 32 个 OpenMP 线程，即单个物理插槽；
- 特征维度 \(K=128\)。

### 数据集

表 1 汇总了来自 SuiteSparse [2]、SNAP [14]、OGB [8] 和 DGL [19] 的 25 个测试矩阵，覆盖 8 个以上应用领域。

图规模范围：

- 最小：`email-Enron`，3.7 万节点；
- 最大：`com-Friendster`，6560 万节点、36 亿条边。

度数比范围：

- `amazon0601`：约 1，度数非常均匀；
- `FullChip`：259463，度数极度偏斜。

### 对比基线

本文比较以下方法。

1. **Intel MKL v2023.2.0**

   使用供应商优化的两阶段流水线：

   - `mkl_sparse_s_mm` 执行 FP32 SpMM；
   - `cblas_sgemm` 执行 FP32 GeMM。

2. **AVX-512 TFS**

   与 TFS 使用相同的：

   - 融合逻辑；
   - 度数排序；
   - BF16 数据流。

   但将 AMX Tile 指令替换为 AVX-512 BF16 指令，用于隔离 AMX 硬件本身的贡献。

3. **DGL v2.1.0**

4. **PyTorch CPU 稀疏后端**

   使用 `torch.sparse.mm`，底层由 MKL 支持。

DGL 和 PyTorch 均使用 FP32，并采用相同的 32 线程配置。

### 测量方法

每个配置在 SLURM 独占节点上运行 5 次：

```text
--exclusive
```

报告中位数。

端到端测量包含：

- 两层 GCN；
- 层间 ReLU 激活。

数值精度采用元素级相对误差：

\[
\text{rel\_err}
=
\frac{
\|C_{\text{TFS}}-C_{\text{MKL}}\|_F
}{
\|C_{\text{MKL}}\|_F
}
\]

## 表 1：数据集特征

度数比定义为：

\[
\text{Deg Ratio}
=
\frac{\text{最大度数}}{\text{平均度数}}
\]

| # | 矩阵 | 领域 | N | NNZ | 平均度数 | 度数比 |
|---:|---|---|---:|---:|---:|---:|
| 1 | email-Enron | 电子邮件 | 37 K | 368 K | 10.0 | 138 |
| 2 | scircuit | 电路 | 171 K | 959 K | 5.6 | 63 |
| 3 | reddit | GNN 基准 | 233 K | 114.6 M | 492.0 | 44 |
| 4 | mycielskian19 | 合成 | 393 K | 903.2 M | 2,297 | 86 |
| 5 | amazon0601 | 共同购买 | 403 K | 3.4 M | 8.4 | 1 |
| 6 | web-Google | Web | 916 K | 5.1 M | 5.6 | 82 |
| 7 | com-Youtube | 社交 | 1.13 M | 6.0 M | 5.3 | 5,461 |
| 8 | hollywood-2009 | 合作网络 | 1.14 M | 113.9 M | 99.9 | 115 |
| 9 | soc-Pokec | 社交 | 1.63 M | 30.6 M | 18.8 | 467 |
| 10 | as-Skitter | 互联网 | 1.70 M | 22.2 M | 13.1 | 2,710 |
| 11 | roadNet-CA | 道路 | 1.97 M | 5.5 M | 2.8 | 4 |
| 12 | kron_g500-logn21 | 合成 | 2.10 M | 182.1 M | 86.8 | 2,464 |
| 13 | wiki-Talk | 社交 | 2.39 M | 5.0 M | 2.1 | 47,694 |
| 14 | ogbn-products | GNN 基准 | 2.45 M | 123.7 M | 50.5 | 346 |
| 15 | sx-stackoverflow | 问答 | 2.60 M | 11.4 M | 4.4 | 1,152 |
| 16 | FullChip | 电路 | 2.99 M | 26.6 M | 8.9 | 259,463 |
| 17 | cit-Patents | 引文 | 3.77 M | 16.5 M | 4.4 | 176 |
| 18 | com-LiveJournal | 社交 | 4.00 M | 69.4 M | 17.3 | 854 |
| 19 | rajat31 | 科学计算 | 4.69 M | 20.3 M | 4.3 | 289 |
| 20 | soc-LiveJournal1 | 社交 | 4.85 M | 69.0 M | 14.2 | 1,426 |
| 21 | cage15 | 科学计算 | 5.15 M | 99.2 M | 19.2 | 2 |
| 22 | indochina-2004 | Web 爬取 | 7.41 M | 194.1 M | 26.2 | 267 |
| 23 | rgg_n_2_24_s0 | 几何图 | 16.8 M | 265.1 M | 15.8 | 3 |
| 24 | road_usa | 道路 | 23.9 M | 57.7 M | 2.4 | 4 |
| 25 | com-Friendster | 社交 | 65.6 M | 3.6 B | 55.1 | 95 |

## 4.2 内核性能

图 4 展示了 TFS 相较于 Intel MKL 的算子级加速比。共有 24 个矩阵能够由 MKL 正常完成，`com-Friendster` 因 MKL 崩溃而未计入图中。

总体结果：

- TFS 在 24 个矩阵中的 21 个上领先；
- 胜率为 87.5%；
- 几何平均加速比为 2.21×；
- 最高加速比为 `ogbn-products` 上的 6.67×。

另外 6 个矩阵超过 3×：

- `soc-Pokec`：5.05×；
- `cit-Patents`：4.80×；
- `soc-LiveJournal1`：3.70×；
- `com-LiveJournal`：3.68×；
- `rgg_n_2_24_s0`：3.41×；
- `cage15`：3.36×。

`com-Friendster` 包含 6560 万个节点和 36 亿条边，MKL 因内部 CSR 表示发生 32 位索引溢出而崩溃。TFS 则在 8.2 秒内正常完成。

### TFS 落后的三个矩阵

1. **email-Enron：0.70×**

   仅有 3.7 万个节点，规模过小，无法摊薄 AMX 初始化和配置开销。

2. **FullChip：0.29×**

   度数比达到 259463，其中单个节点的度数约为 230 万，极端高度节点支配整体执行时间。

3. **wiki-Talk：0.94×**

   平均度数仅为 2.1，但度数比达到 47694，极端稀疏与极端偏斜同时存在。

这些数据集的共同特点是：即使执行度数排序，仍无法完全缓解极端度数偏斜。

排序后：

- `FullChip` 的 Tile 效率仅为 \(\eta=0.45\)；
- `wiki-Talk` 为 \(\eta=0.73\)；
- 其余 21 个获胜矩阵均达到 \(\eta\ge0.93\)。

最大的加速通常出现在同时具备以下特征的图上：

- 平均度数中等或较高；
- 度数比有限；
- Tile 批次可以被较充分填满。

例如：

- `ogbn-products`：平均度数 50.5，度数比 346；
- `soc-Pokec`：平均度数 18.8，度数比 467。

## 4.3 端到端 GCN 推理

表 2 报告两层 GCN 的端到端推理结果。

在 19 个 BF16 精度稳定的矩阵中，有 17 个可以由 MKL 完成。TFS 在其中 15 个上取得领先，胜率为 88%。

加速范围：

- 最低：`com-Youtube`，1.14×；
- 最高：`ogbn-products`，4.09×。

MKL 在以下两个端到端任务中崩溃：

- `com-Friendster`；
- `road_usa`。

`road_usa` 可以完成单层内核测试并获得 2.36× 加速，但两层流水线需要更大的内存分配，导致 MKL 发生段错误。

在标准 GNN 数据集上，TFS 的端到端加速分别为：

- `ogbn-products`：4.09×；
- `soc-Pokec`：3.43×；
- `soc-LiveJournal1`：2.77×；
- `com-LiveJournal`：2.72×；
- `reddit`：1.51×。

所有 6 个 GNN 相关数据集的相对误差均不超过 0.012，表明混合精度流水线没有给目标工作负载带来明显精度损失。

TFS 在两个精度稳定的数据集上落后：

- `email-Enron`：0.62×，图规模过小；
- `wiki-Talk`：0.85×，平均度数极低且度数偏斜严重。

端到端加速通常低于算子级加速，因为 TFS 还包含：

- FP32 到 BF16 的层间转换；
- ReLU 激活。

例如：

- `ogbn-products` 从 6.67× 降至 4.09×；
- `soc-Pokec` 从 5.05× 降至 3.43×。

15 个获胜矩阵的端到端几何平均加速比为 2.18×。

## 表 2：两层 GCN 端到端推理

配置：\(K=128\)，32 线程。

| # | 矩阵 | 领域 | TFS（ms） | MKL（ms） | 内核加速 | 端到端加速 | rel_err | 状态 |
|---:|---|---|---:|---:|---:|---:|---:|---|
| 1 | amazon0601 | 共同购买 | 12.65 | 25.62 | 2.88× | 2.03× | 0.010 | 稳定 |
| 2 | as-Skitter | 互联网 | 100.52 | 162.63 | 2.18× | 1.62× | 0.011 | 稳定 |
| 3 | cage15 | 科学计算 | 277.93 | 601.46 | 3.36× | 2.16× | 142.3 | 数值发散 |
| 4 | cit-Patents | 引文 | 77.05 | 271.32 | 4.80× | 3.52× | 0.011 | 稳定 |
| 5 | com-Friendster | 社交 | 22,453 | — | — | — | — | 索引溢出 |
| 6 | com-LiveJournal | 社交 | 237.90 | 646.45 | 3.68× | 2.72× | 0.009 | 稳定 |
| 7 | com-Youtube | 社交 | 47.33 | 54.17 | 1.41× | 1.14× | 0.011 | 稳定 |
| 8 | email-Enron | 电子邮件 | 2.30 | 1.43 | 0.70× | 0.62× | 0.011 | 稳定 |
| 9 | FullChip | 电路 | 1,220 | 334.34 | 0.29× | 0.27× | 1.0 | 数值发散 |
| 10 | hollywood-2009 | 合作网络 | 298.88 | 392.32 | 1.88× | 1.31× | 0.009 | 稳定 |
| 11 | indochina-2004 | Web 爬取 | 347.33 | 696.06 | 2.29× | 2.00× | 0.009 | 稳定 |
| 12 | kron_g500-logn21 | 合成 | 634.74 | 612.60 | 1.33× | 0.97× | 0.967 | 数值发散 |
| 13 | mycielskian19 | 合成 | 2,132 | 4,814 | 2.74× | 2.26× | 0.009 | 稳定 |
| 14 | ogbn-products | GNN 基准 | 329.69 | 1,348 | 6.67× | 4.09× | 0.011 | 稳定 |
| 15 | rajat31 | 科学计算 | 84.00 | 138.67 | 2.07× | 1.65× | 1.0 | 数值发散 |
| 16 | reddit | GNN 基准 | 292.82 | 441.37 | 2.36× | 1.51× | 0.010 | 稳定 |
| 17 | rgg_n_2_24_s0 | 几何图 | 885.20 | 2,022 | 3.41× | 2.28× | 0.010 | 稳定 |
| 18 | roadNet-CA | 道路 | 35.25 | 66.23 | 2.24× | 1.88× | 0.012 | 稳定 |
| 19 | road_usa | 道路 | 479.64 | — | 2.36× | — | — | 段错误 |
| 20 | scircuit | 电路 | 4.70 | 6.75 | 2.17× | 1.44× | 1.0 | 数值发散 |
| 21 | soc-LiveJournal1 | 社交 | 245.07 | 680.00 | 3.70× | 2.77× | 0.010 | 稳定 |
| 22 | soc-Pokec | 社交 | 96.50 | 331.18 | 5.05× | 3.43× | 0.011 | 稳定 |
| 23 | sx-stackoverflow | 问答 | 59.93 | 98.83 | 1.94× | 1.65× | 1.0 | 数值发散 |
| 24 | web-Google | Web | 20.66 | 46.73 | 2.55× | 2.26× | 0.010 | 稳定 |
| 25 | wiki-Talk | 社交 | 106.69 | 90.34 | 0.94× | 0.85× | 0.010 | 稳定 |

精度稳定且排除溢出数据后：

- 获胜：15/17；
- 胜率：88%；
- 失败数据集：`email-Enron`、`wiki-Talk`。

### 与 GNN 框架的比较

本文还在相同平台上测试了 DGL 和 PyTorch CPU 稀疏后端。

在全部方法都可完成的 14 个精度稳定矩阵上：

- TFS 在 14/14 个矩阵上快于 DGL；
- 加速范围为 1.19–4.46×；
- 几何平均加速为 2.52×；
- TFS 在 14/14 个矩阵上快于 PyTorch sparse；
- 加速范围为 1.45–4.45×。

DGL 和 PyTorch 在多数数据集上还慢于直接使用 MKL 的基线：

- DGL 存在消息传递调度开销；
- `torch.sparse.mm` 使用的是更通用的 MKL 代码路径；
- 本文 MKL 基线使用了更具竞争力的 inspector-executor 接口。

在 `road_usa` 上，DGL 和 PyTorch 均发生段错误，而 TFS 正常完成。

## 4.4 特征维度敏感性

图 5a 分析了特征维度：

\[
K\in\{32,64,128,256\}
\]

对 TFS 加速比的影响。

总体胜率为：

- \(K=32\)：42%；
- \(K=64\)：71%；
- \(K=128\)：75%；
- \(K=256\)：46%。

最适合 TFS 的范围为：

\[
K=64\sim128
\]

这与 GCN [13]、GraphSAGE [6] 等 GNN 架构常用的隐藏维度一致。

### \(K=32\)

AMX Tile 利用不足。每个 pass 只需要激活 2 个累加 Tile，单次邻居 gather 能够摊薄的 Tile 计算量较少。

### \(K=256\)

输出 pass 增加至 4 个：

- gather 总量增加；
- 内存压力增大；
- 权重矩阵大小变为：

\[
256\times256\times2=128\ \text{KB}
\]

超过 48 KB L1 缓存容量，导致权重 Tile 加载溢出到 L2。

因此，\(K=64\sim128\) 在 Tile 利用率与 gather 效率之间取得了较好的平衡。

代表性结果：

- `ogbn-products` 在 \(K=32\sim128\) 时保持 2.99–5.83× 加速，\(K=256\) 时下降到 2.11×；
- `road_usa` 从 1.62× 稳步上升到 2.31×；
- `FullChip` 在所有 \(K\) 下均低于 1.0×，说明极端度数偏斜才是其性能差的根本原因。

## 4.5 线程扩展性

图 5b 展示 TFS 从 1 到 64 个线程的扩展情况。

从 1 到 32 个线程，TFS 基本接近线性扩展，并达到 85%–100% 的并行效率。

在 32 线程时：

- `indochina-2004` 并行效率为 96%；
- `web-Google` 为 92%。

接近线性扩展的原因是：

- Tile 批次之间相互独立；
- 每个 OpenMP 线程处理不同的 16 行批次；
- 无需线程间同步；
- 不存在共享可变状态；
- 度数排序使不同批次的负载更加均衡。

使用 64 个线程时，性能开始下降，因为系统为双路架构，跨插槽内存访问带来额外开销。

因此，使用单个物理插槽的 32 线程配置最优。

## 4.6 AMX 硬件隔离实验

本文的核心主张之一是：

> TFS 的加速来自 AMX 的 Tile 级计算密度，而不只是 BF16 数据格式。

为验证这一点，本文实现了 AVX-512 版本的 TFS。该版本使用与 AMX TFS 完全相同的：

- 融合逻辑；
- 度数排序；
- BF16 数据流。

唯一差异是：将 AMX Tile 指令替换为 AVX-512 BF16 指令：

```c
_mm512_dpbf16_ps
```

图 6a 比较三种配置：

1. MKL FP32 两阶段；
2. AVX-512 TFS BF16 融合；
3. AMX TFS BF16 融合。

### AVX-512 TFS 的结果

尽管 AVX-512 TFS：

- 使用 BF16，内存占用只有 FP32 的一半；
- 完成算子融合；
- 消除了中间矩阵 \(Z\)；

但它在全部 25 个矩阵上仍然慢于 MKL，加速比只有 0.10–0.97×。

这说明 BF16 和融合本身不足以抵消 MKL 高度优化的 FP32 SpMM 优势。

### 原因：指令吞吐差异

一条 `_mm512_dpbf16_ps` 只处理一个 512 位向量，即 16 个 FP32 输出。

一条 `TDPBF16PS` 则一次完成整个 \(16\times16\) Tile 的计算，也就是 4096 次 FP32 FMA。

从单条指令处理的计算量看，AMX 具有约 256× 的优势。

### AMX 相较于 AVX-512 的结果

AMX TFS 在所有矩阵上均比 AVX-512 TFS 快 2.8–13.7×。

最高比值出现在平均度数较高、能够充分摊薄 gather 开销的图上：

- `indochina-2004`：13.7×；
- `mycielskian19`：9.9×；
- `reddit`：9.7×；
- `ogbn-products`：8.8×；
- `cage15`：8.6×。

最低为：

- `wiki-Talk`：1.3×。

`wiki-Talk` 极度稀疏，可供 Tile 执行的有效计算较少。

因此，真正将 TFS 从“慢于 MKL”转变为“快于 MKL”的，是每条 `TDPBF16PS` 指令执行 8192 次 BF16 乘加的 Tile 级计算密度。

## 表 3：消融实验

配置：\(K=128\)，32 线程。

表中数据表示相较于 BF16 两阶段基线的加速比。每一列在前一列基础上累积加入一种技术。

| 矩阵 | +朴素融合 | +完整 K 维 gather | +度数排序 | +预取 |
|---|---:|---:|---:|---:|
| web-Google | 1.24 | 1.17 | 5.04 | 5.51 |
| amazon0601 | 1.28 | 1.27 | 4.62 | 4.94 |
| cit-Patents | 1.40 | 1.88 | 8.11 | 9.49 |
| as-Skitter | 0.89 | 1.33 | 3.05 | 3.63 |
| soc-Pokec | 1.24 | 1.67 | 4.87 | 5.99 |
| hollywood | 0.46 | 0.84 | 1.40 | 1.46 |
| indochina | 1.17 | 0.91 | 2.67 | 2.79 |
| 几何平均 | 1.04× | 1.27× | 约 3.75× | 3.97× |

## 4.7 消融实验分析

### 朴素融合

最初版本会重复 gather 每个邻居的特征：

- 4 个输出 Tile 面板；
- 4 个 K-block；
- 每个邻居总计执行 16 次冗余 gather。

过量内存流量抵消了融合收益，几何平均加速仅为 1.04×。

### 加入完整 \(K\) 维 gather

调整循环顺序后，每个邻居的完整 \(K\) 维特征只读取一次，然后在 4 个输出 Tile 列之间复用。

这将 gather 流量降低 16×，几何平均加速提升至 1.27×。

但度数偏斜问题仍然存在，异构批次仍会浪费大量 Tile 周期。

在 `web-Google` 和 `indochina-2004` 上，完整 \(K\) 维 gather 甚至略微降低性能，因为在未进行度数排序时，更大的单邻居数据足迹增加了缓存压力。

### 加入度数排序

加入度数升序排序和智能 `memset` 后，几何平均加速显著提升至约 3.75×，相较于完整 \(K\) 维 gather 版本提升 2.95×。

度数排序还是智能 `memset` 的正确性前提。

内核中的 `active_from` 指针依赖行顺序单调递增。若只移除排序，7 个矩阵中有 5 个出现正确性错误。

因此，度数排序不仅是性能优化，也是系统级正确性保证。

### 加入软件预取

最终加入软件预取后，几何平均加速从约 3.75× 提升至 3.97×，增益约为 6%。

消融结果表明：

> 控制每个 Tile 批次内部的度数分布，是决定性能的最关键优化。

## 4.8 BF16 精度分析

25 个矩阵中：

- 19 个矩阵精度稳定，占 76%；
- 6 个矩阵出现 BF16 数值不稳定，`rel_err≈1.0`。

19 个稳定矩阵中：

- 17 个可与 MKL 直接比较，`rel_err<0.02`；
- 2 个数据集上 MKL 崩溃。

全部 6 个 GNN 基准数据集：

- `ogbn-products`；
- `reddit`；
- `com-LiveJournal`；
- `soc-LiveJournal1`；
- `soc-Pokec`；
- `com-Youtube`；

其相对误差均在 0.009–0.012 之间。

这说明 BF16 对 TFS 的目标 GNN 推理工作负载是足够的，也与生产 GNN 框架采用 BF16 的趋势一致。

出现不稳定的 6 个矩阵为：

- `cage15`；
- `FullChip`；
- `kron_g500-logn21`；
- `rajat31`；
- `scircuit`；
- `sx-stackoverflow`。

这些矩阵都不是 GNN 任务矩阵，其数值范围超出了 BF16 7 位尾数所能稳定表示的范围。

度数排序只改变不同节点行的处理顺序，不改变单行内部的累加顺序。相对误差始终以 MKL FP32 结果作为参照。

因此，这些数值不稳定现象主要来自数据数值范围，而不是度数排序。

# 5 相关工作

## 5.1 SpMM 优化

稀疏矩阵—稠密矩阵乘法已经在 CPU 和 GPU 上得到广泛研究。

GPU 方向包括：

- GE-SpMM [10]：提出合并访问的行缓存策略；
- ASpT [7]：提出自适应稀疏分块；
- Merge-based 方法 [16]：在 GPU warp 间平衡工作量。

Intel MKL [12] 通过 inspector-executor 接口提供高度优化的 CPU SpMM。

JitSpMM [4] 使用即时汇编代码生成进一步加速 CPU SpMM，相较于 MKL 可实现约 1.4× 加速。

FusedMM [17] 将 SDDMM 和 SpMM 统一为一个 GNN 内核。

这些工作通常：

- 单独优化 SpMM；
- 或融合其他算子组合。

TFS 的区别在于，它将 SpMM 与下游 GeMM 直接融合，从而消除中间矩阵物化。

## 5.2 GNN 系统加速

DGL [19] 和 PyTorch Geometric [3] 是主流 GNN 框架，二者均依赖供应商稀疏计算库。

相关工作包括：

- GNNAdvisor [20]：面向 GPU 的工作负载感知 GNN 内核优化；
- FeatGraph [9]：特征维度感知调度框架；
- TC-GNN [21]：探索使用 GPU Tensor Core 加速 GNN 稀疏内核。

TFS 的不同之处在于：

- 目标平台是 CPU Tile 加速器 AMX；
- 融合的是 SpMM 与 GeMM；
- 并非仅优化单独的 SpMM。

## 5.3 神经网络算子融合

算子融合是深度学习编译器中的常见优化。

FlashAttention [1] 将注意力内部子操作融合，从而消除 \(O(N^2)\) 中间矩阵的物化。

TFS 将类似思想推广到稀疏—稠密边界，将 SpMM 和 GeMM 跨越不规则与规则计算模式直接融合。

传统 SIMD 单元通常没有足够大的寄存器容量同时保存：

- SpMM 的累积状态；
- 权重矩阵分块。

AMX 提供 8 KB Tile 寄存器文件，使这种寄存器级融合成为可能。

## 5.4 Intel AMX 与 Tile 架构

AMX 已被用于深度学习中的稠密 GeMM 和卷积内核。

NVIDIA Tensor Core 也被 TC-GNN [21] 用于稀疏计算。

Salehi 和 Cheshmi [18] 的同期工作通过编译器 Tile 调度，在缓存层融合 GeMM–SpMM，相较于 MKL 实现 1.33× 加速。

TFS 的几何平均加速为 2.21×，其优势来自寄存器级融合，而不是缓存级复用。

TFS 与该工作存在三点差异：

1. TFS 融合的是 GCN 前向传播顺序，即先 SpMM 后 GeMM；
2. TFS 在 AMX Tile 寄存器内部彻底消除中间矩阵，而非只在缓存中复用；
3. TFS 引入度数感知调度，解决稀疏图特有的 Tile 负载不均衡。

度数感知调度也不同于 Gorder 和 RCM 等图重排序方法。

- Gorder、RCM 主要优化缓存局部性；
- TFS 排序的目标是让同一 Tile 批次中的度数相近，提高 Tile 计算利用率。

二者目标正交，因此可以组合使用。

# 6 结论

本文提出 TFS，一种面向 Intel AMX 的 Tile 感知 SpMM–GeMM 融合框架，用于加速 GNN 推理。

TFS 在 AMX Tile 寄存器内部融合：

- 稀疏邻居聚合；
- 稠密线性变换。

由此消除了中间矩阵 \(Z\) 的内存流量。

实现该方法的关键是度数感知 Tile 调度。按照节点度数升序排列，可将平均 Tile 效率 \(\eta\) 提升至 0.96，将高度不规则的稀疏计算转化为适合 Tile 执行的工作负载。

由于 \(\eta\) 只依赖节点度数序列，因此可直接从 CSR 的 `indptr` 数组中以：

\[
O(N)
\]

复杂度计算，并可在内核启动前决定采用：

- TFS 融合执行；
- 或传统两阶段执行。

在来自 8 个以上应用领域的 25 个矩阵上，TFS 相较于 Intel MKL：

- 算子级最高加速 6.67×；
- 端到端最高加速 4.09×；
- 线程数扩展至 32 个物理核心时接近线性。

在所有精度稳定的数据集上，TFS 还优于 DGL 和 PyTorch 稀疏后端。

AMX 隔离实验进一步证明：

> 加速的主要来源是 AMX 的 Tile 计算密度，而不是 BF16 算术本身。

TFS 仍存在局限：

1. 对度数比超过 10000× 的极端偏斜图，效果较差；
2. 对小于 10 万个节点的小图，AMX 初始化成本难以摊薄；
3. 对要求 FP32 精度的科学计算矩阵，BF16 可能不够稳定。

未来可以设计自适应精度扩展，根据每一行的：

- 度数；
- 数值范围；

动态选择 BF16 或 FP32 Tile 路径，以同时缓解极端偏斜和精度问题。

最后，Tile 感知融合模型并不局限于 Intel AMX。相同的：

- 度数感知调度；
- 特征驻留数据流；

也可以映射到：

- GPU Tensor Core；
- NPU Tile 引擎；
- 未来其他基于 Tile 的加速器。

这为异构架构上的统一稀疏—稠密融合提供了一条可行路径。

# 参考文献

参考文献作者、题名、会议与期刊信息保留原文，以确保引用信息准确。

[1] Tri Dao, Dan Fu, Stefano Ermon, Atri Rudra, and Christopher Ré. 2022. FlashAttention: Fast and memory-efficient exact attention with IO-awareness. In Advances in Neural Information Processing Systems, Vol. 35. Curran Associates, Inc., 16344–16359.

[2] Timothy A Davis and Yifan Hu. 2011. The University of Florida sparse matrix collection. ACM Trans. Math. Software 38, 1 (2011), 1–25.

[3] Matthias Fey and Jan Eric Lenssen. 2019. Fast graph representation learning with PyTorch Geometric. In ICLR Workshop on Representation Learning on Graphs and Manifolds. OpenReview.net.

[4] Qiang Fu, Thomas B. Rolinger, and H. Howie Huang. 2024. JITSPMM: Just-in-Time Instruction Generation for Accelerated Sparse Matrix-Matrix Multiplication. In Proceedings of the IEEE/ACM International Symposium on Code Generation and Optimization (CGO). IEEE, 448–459.

[5] Justin Gilmer, Samuel S Schoenholz, Patrick F Riley, Oriol Vinyals, and George E Dahl. 2017. Neural message passing for quantum chemistry. In Proceedings of the 34th International Conference on Machine Learning (ICML). PMLR, 1263–1272.

[6] Will Hamilton, Zhitao Ying, and Jure Leskovec. 2017. Inductive representation learning on large graphs. In Advances in Neural Information Processing Systems, Vol. 30. Curran Associates, Inc., 1024–1034.

[7] Changwan Hong, Aravind Sukumaran-Rajam, Israt Nisa, Kunal Singh, and P Sadayappan. 2019. Adaptive sparse tiling for sparse matrix multiplication. In Proceedings of the 24th ACM SIGPLAN Symposium on Principles and Practice of Parallel Programming (PPoPP). ACM, 300–314.

[8] Weihua Hu, Matthias Fey, Marinka Zitnik, Yuxiao Dong, Hongyu Ren, Bowen Liu, Michele Catasta, and Jure Leskovec. 2020. Open graph benchmark: Datasets for machine learning on graphs. In Advances in Neural Information Processing Systems, Vol. 33. Curran Associates, Inc., 22118–22133.

[9] Yuwei Hu, Zihao Ye, Minjie Wang, et al. 2020. FeatGraph: A flexible and efficient backend for graph neural network systems. In Proceedings of the International Conference for High Performance Computing, Networking, Storage and Analysis (SC). IEEE, 1–13.

[10] Guyue Huang, Guohao Dai, Yu Wang, and Huazhong Yang. 2020. GE-SpMM: General-purpose sparse matrix-matrix multiplication on GPUs for graph neural networks. In Proceedings of the International Conference for High Performance Computing, Networking, Storage and Analysis (SC). IEEE, 1–12.

[11] Intel Corporation. 2023. Intel Architecture Instruction Set Extensions Programming Reference. Chapter 3: Intel AMX.

[12] Intel Corporation. 2023. Intel oneAPI Math Kernel Library (oneMKL).

[13] Thomas N Kipf and Max Welling. 2017. Semi-supervised classification with graph convolutional networks. In Proceedings of the 5th International Conference on Learning Representations (ICLR). OpenReview.net.

[14] Jure Leskovec and Andrej Krevl. 2014. SNAP Datasets: Stanford large network dataset collection.

[15] Yaguang Li, Rose Yu, Cyrus Shahabi, and Yan Liu. 2018. Diffusion convolutional recurrent neural network: Data-driven traffic forecasting. In Proceedings of the 6th International Conference on Learning Representations (ICLR). OpenReview.net.

[16] Duane Merrill and Michael Garland. 2016. Merge-based parallel sparse matrix-vector multiplication. In Proceedings of the International Conference for High Performance Computing, Networking, Storage and Analysis (SC). IEEE, 678–689.

[17] Md. Khaledur Rahman, Majedul Haque Sujon, and Ariful Azad. 2021. FusedMM: A Unified SDDMM-SpMM Kernel for Graph Embedding and Graph Neural Networks. In IEEE International Parallel and Distributed Processing Symposium (IPDPS). IEEE, 256–266.

[18] Mohammad Mahdi Salehi and Kazem Cheshmi. 2025. Loop Fusion in Matrix Multiplications with Sparse Dependence. In Proceedings of the 39th ACM International Conference on Supercomputing (ICS). ACM, 625–639.

[19] Minjie Wang, Lingfan Yu, Da Zheng, et al. 2019. Deep Graph Library: Towards Efficient and Scalable Deep Learning on Graphs. In ICLR Workshop on Representation Learning on Graphs and Manifolds. OpenReview.net.

[20] Yuke Wang, Boyuan Feng, Gushu Li, Shuangchen Li, Lei Deng, Yuan Xie, and Yufei Ding. 2021. GNNAdvisor: An adaptive and efficient runtime system for GNN acceleration on GPUs. In Proceedings of the 15th USENIX Symposium on Operating Systems Design and Implementation (OSDI). USENIX Association, 515–531.

[21] Yuke Wang, Boyuan Feng, Zheng Wang, Guyue Huang, and Yufei Ding. 2023. TC-GNN: Bridging sparse GNN computation and dense tensor cores on GPUs. In Proceedings of the 2023 USENIX Annual Technical Conference (ATC). USENIX Association, 149–164.

[22] Rex Ying, Ruining He, Pok Eksombatchai, William L Hamilton, and Jure Leskovec. 2018. Graph convolutional neural networks for web-scale recommender systems. In Proceedings of the 24th ACM SIGKDD International Conference on Knowledge Discovery & Data Mining. ACM, 974–983.
