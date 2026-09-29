# GAT × Online Softmax × AMX/TFS：实现型技术交接文档

> 版本：2026-09-28  
> 目标读者：将直接接手代码实现、性能优化与实验验证的 AI / 开发者  
> 使用方式：请与 **原始 TFS 源码**、**TFS 论文** 一起阅读。本文不是要求机械照搬某个既定方案，而是给出完整研究目标、设计空间、必须实现的基线与两条主路线。  
> 核心原则：**先保证数学正确和公平基线，再做 AMX 融合；不要默认 online softmax 一定优于 two-pass，也不要默认 aggregate-first 一定优于 transform-first。**

---

# 0. 最重要的任务说明

我们已有一套面向 Intel AMX 的 TFS（Tile-aware SpMM–GeMM Fusion）实现，原始目标是 GCN / 静态稀疏聚合场景：

\[
A H \rightarrow (AH)W
\]

TFS 的核心思想是：

1. 利用图结构和 degree sort / tile-aware schedule 提高 AMX tile 利用率；
2. 把稀疏聚合和后续稠密线性变换尽可能融合；
3. 尽量避免完整中间矩阵 \(AH\) 写回内存；
4. 让中间 partial result 尽可能停留在 tile / L1 附近。

现在我们想把这套思路拓展到 **vanilla GAT 推理**。

标准 GAT 每层不再是固定 \(A\)，而是：

\[
H
\rightarrow
\text{Attention Score}
\rightarrow
\text{Softmax}
\rightarrow
\text{Weighted Aggregation}
\rightarrow
\text{Linear Projection}
\]

也就是：

\[
\boxed{
\text{注意力打分}
\rightarrow
\text{Softmax}
\rightarrow
\text{加权稀疏聚合}
\rightarrow
\text{稠密线性变换}
}
\]

我们希望最终回答的问题是：

> **能否在 Intel AMX CPU 上，为 GAT 重新设计一条适合 AMX 的动态稀疏注意力数据流，使 attention score、softmax weight 和聚合中间结果不再形成大规模全局中间矩阵，而只在 AVX-512 寄存器、AMX tile、L1 小型 staging buffer 之间流动？**

必须同时实现和比较：

- 单头 GAT；
- 多头 GAT；
- Online Softmax 路线；
- Two-pass 精确扫描路线；
- 标准 GAT 先变换后聚合；
- TFS 风格先聚合后变换；
- TFS-GAT 基线；
- 完整框架/CPU 强基线。

**不要只实现一个最终 kernel。整个研究最重要的是建立完整、可解释、可复现实验链。**

---

# 1. 第一步：必须先读懂原始 TFS

在改代码之前，请先阅读：

1. TFS 论文；
2. TFS 源码；
3. build / benchmark 脚本；
4. graph preprocessing；
5. degree sort / permutation；
6. SpMM kernel；
7. GeMM kernel；
8. AMX tile configuration；
9. BF16/FP32 数据转换；
10. timing scope。

需要先输出一份简短的源码结构说明，至少回答：

- 原始 TFS 的输入矩阵分别是什么形状？
- 稀疏矩阵结构使用 CSR / CSC / 自定义 panel 中哪一种？
- adjacency value 是不是默认 1？
- 是否支持带权稀疏矩阵？
- Degree Sort 在哪里做？
- 节点 permutation 在哪里维护？
- tile shape 是什么？
- 一个 tile 的行对应 destination node 还是 source node？
- 一个 tile 的列对应 feature 维度还是节点块？
- `TDPBF16PS` 分别在哪些地方调用？
- FP32 accumulator 何时落回内存？
- SpMM→GeMM 之间是否真的一直保持 TMM resident，还是存在 L1/BF16 格式转换？
- 原始实现用了几个 TMM？
- 哪些 TMM 是 accumulator，哪些是输入 tile？
- 一个完整 panel 的生命周期是什么？
- kernel 的主要瓶颈是算力、tile 空转、L1/L2、gather 还是格式转换？
- 原论文中的 timing 是否包含 preprocessing / sorting / conversion？

**这一步不能跳过。后面的所有 GAT 设计必须以真实 TFS 实现为准，而不是根据概念图猜。**

---

# 2. vanilla GAT 的标准数学形式

设：

\[
H\in\mathbb{R}^{N\times D}
\]

单个 head：

\[
W\in\mathbb{R}^{D\times F}
\]

首先：

\[
Z=HW
\]

其中：

\[
Z\in\mathbb{R}^{N\times F}
\]

将 attention vector 拆为：

\[
a=[a_L||a_R]
\]

其中：

\[
a_L,a_R\in\mathbb{R}^{F}
\]

节点级 attention scalar：

\[
L_i=a_L^Tz_i
\]

\[
R_j=a_R^Tz_j
\]

边分数：

\[
e_{ij}
=
\operatorname{LeakyReLU}(L_i+R_j)
\]

对 destination node \(i\) 的所有邻居做 softmax：

\[
\alpha_{ij}
=
\frac{\exp(e_{ij})}
{\sum_{k\in N(i)}\exp(e_{ik})}
\]

最终输出：

\[
O_i
=
\sum_{j\in N(i)}
\alpha_{ij}z_j
\]

矩阵形式：

\[
O=A_\alpha(HW)
\]

---

# 3. 与 TFS 相关的精确代数重排

因为 \(A_\alpha\) 一旦确定，就是线性稀疏矩阵，所以：

\[
A_\alpha(HW)
=
(A_\alpha H)W
\]

此外：

\[
L=(HW)a_L
=
H(Wa_L)
\]

定义：

\[
b_L=Wa_L
\]

则：

\[
L=Hb_L
\]

同理：

\[
b_R=Wa_R
\]

\[
R=Hb_R
\]

因此单头 vanilla GAT 可以精确重写成：

\[
\boxed{
H
\rightarrow
L/R
\rightarrow
e_{ij}
\rightarrow
\alpha_{ij}
\rightarrow
U=A_\alpha H
\rightarrow
O=UW
}
\]

这是 **数学精确重排**，不是近似。

但必须注意：

> 数学上可交换，不代表性能上一定更好。

特别是多头场景，后文会说明 aggregate-first 可能显著增加稀疏计算量。

---

# 4. 我们真正要研究的不是“一个 kernel”，而是整个数据流设计空间

至少要保留以下三类主数据流。

## 4.1 标准：先变换、后聚合

每个 head：

\[
Z^{(h)}=HW^{(h)}
\]

然后：

\[
A_{\alpha^{(h)}}Z^{(h)}
\]

优点：

- sparse aggregation 的 feature width 是 \(d_h\)；
- 符合标准 GAT；
- 多头时不会重复聚合原始 D 维 H。

缺点：

- 需要生成/保存 \(Z\)；
- 如果直接框架实现，中间量较多；
- 不直接适配原始 TFS 的 aggregate→GeMM 方向。

---

## 4.2 TFS 风格：先聚合、后变换

每个 head：

\[
U^{(h)}
=
A_{\alpha^{(h)}}H
\]

再：

\[
O^{(h)}
=
U^{(h)}W^{(h)}
\]

优点：

- 可以直接研究 TFS 风格 SpMM→GeMM；
- 有机会不物化完整 \(U\)。

风险：

标准多头 GAT 的 sparse work 约为：

\[
O(EK d_h)
\]

而 aggregate-first：

\[
O(EK D)
\]

理论膨胀比例约：

\[
\frac{D}{d_h}
\]

例如：

\[
D=128,\quad d_h=16
\]

可能出现约 \(8\times\) 的稀疏 feature arithmetic / bytes 放大。

所以：

> **单头可以重点研究 aggregate-first；多头必须和标准 transform-first 正面对比，不能默认 aggregate-first 是正确方向。**

---

## 4.3 候选新方向：源节点块即时投影

目标是同时避免：

- 完整 \(Z=HW\) 物化；
- aggregate-first 的 \(D/d_h\) 稀疏工作膨胀。

思路：

1. 取一块 source features：

\[
H_B
\]

2. 使用 AMX 临时计算：

\[
Z_B^{(h)}=H_BW^{(h)}
\]

3. 不写回完整 \(N\times F\) 中间矩阵；
4. 立即让与该 source block 相关的 destination block 消费 \(Z_B^{(h)}\)；
5. 结束后覆盖该 block。

这个方向属于高级拓展，不要求第一阶段必须完成，但需要在代码结构中保留实验入口。

---

# 5. GAT 前端与后端的硬件分工

目前建议：

## AVX-512 更适合

- \(L_i/R_j\) scalar 处理；
- `LeakyReLU`；
- max；
- exp；
- softmax denominator；
- row-wise scalar state；
- 逐行 scale；
- 最终 normalize；
- BF16 staging / packing；
- 低 degree 节点的 vector aggregation。

## AMX 更适合

- dense BF16 matrix multiply；
- 高密度/高 degree block 的 weighted aggregation；
- TFS 的 SpMM–GeMM 主体；
- source-block projection；
- large enough tile/panel。

原则：

> 不要强迫 AMX 做它不擅长的逐元素/逐行控制操作。

---

# 6. AMX 上 Online Softmax 的核心冲突

标准 online softmax 对每一行维护：

\[
m_i
\]

\[
l_i
\]

新 score 到来：

\[
m_i^{new}
=
\max(m_i^{old},e_{ij})
\]

\[
r_i
=
\exp(m_i^{old}-m_i^{new})
\]

\[
p_{ij}
=
\exp(e_{ij}-m_i^{new})
\]

denominator：

\[
l_i^{new}
=
r_i l_i^{old}+p_{ij}
\]

融合加权聚合：

\[
U_i^{new}
=
r_i U_i^{old}+p_{ij}H_j
\]

如果同时处理 16 行：

\[
C=
\begin{bmatrix}
U_0\\
U_1\\
\cdots\\
U_{15}
\end{bmatrix}
\]

则：

\[
C_{new}
=
D_rC_{old}+PH
\]

其中：

\[
D_r
=
\operatorname{diag}(r_0,\ldots,r_{15})
\]

AMX 擅长：

\[
PH
\]

但当前 BF16 AMX 不擅长：

\[
D_rC
\]

原因：

1. \(C\) 是 FP32 accumulator；
2. `TDPBF16PS` source 是 BF16，destination/accumulator 是 FP32；
3. 没有 TMM row-wise multiply；
4. 没有简单的 TMM↔ZMM 直接寄存器传输；
5. 传统实现大概率需要：

\[
TMM
\rightarrow
L1
\rightarrow
ZMM
\rightarrow
scale
\rightarrow
L1
\rightarrow
TMM
\]

这就是 online 路线必须处理的问题。

---

# 7. 主路线 A：Online Softmax

必须实现 online softmax，但要分层实现。

---

## 7.1 A0：纯 FP32/标量正确性参考

先写完全独立于 TFS 的参考：

- CSR traversal；
- per-row online max；
- per-row denominator；
- per-row vector accumulator；
- FP32。

输出必须与标准两阶段 softmax + weighted aggregation 对齐。

这是 online 路线的 correctness oracle。

---

## 7.2 A1：AVX-512 Online Softmax + 普通聚合

先不碰 AMX。

每个 destination row：

- `m_i`：FP32；
- `l_i`：FP32；
- accumulator：ZMM；
- score：AVX；
- exp：AVX；
- row rescale：ZMM。

目的：

1. 验证 online 算法；
2. 测量真实图下 max 更新频率；
3. 测量 row rescale 代价；
4. 建立 low-degree AVX baseline。

建议统计：

- updates / row；
- updates / edge；
- P50 / P95 / P99 updates；
- 按 degree bucket 统计；
- `max_new - max_old` 分布；
- score range；
- row attention entropy；
- exp 输入范围。

---

## 7.3 A2：AMX Online Softmax —— 最直接正确基线

实现：

\[
TMM\ store
\rightarrow
AVX\ row\ scale
\rightarrow
BF16/格式转换
\rightarrow
tile reload
\]

也就是最直接的 spill-scale-reload。

目的不是最终性能，而是：

> **建立“AMX 上传统 online softmax”的正确基准成本。**

必须单独 microbenchmark：

- 一个 16-row tile scale 一次的时间；
- 16×16、16×32、16×64 等 feature tile；
- scale 发生 0/1/2/4/8 次时总成本；
- L1 resident 情况；
- 多 tile accumulator 情况。

---

# 8. Online Softmax 的高级优化方向

以下方向都应该保留为独立 variant，不要一开始全部耦合。

---

## 8.1 延迟重新定标 / 条件式重新定标

传统 online：

只要：

\[
m_{new}>m_{old}
\]

立即 rescale。

新的思路：

维护参考尺度：

\[
c_i
\]

允许新 score 暂时大于 \(c_i\)，只要：

\[
e_{ij}-c_i
\]

仍然在安全范围。

只有当：

\[
e_{ij}-c_i>\tau
\]

时才真正 rebase / rescale。

目标：

\[
\text{max update frequency}
\gg
\text{physical rescale frequency}
\]

必须扫不同阈值 \(\tau\)。

正确性要求：

- 必须保证不存在 overflow；
- 统计 BF16 weight 量化误差；
- 统计最终输出误差；
- 不能只看速度。

---

## 8.2 分段累加，不缩放当前 TMM

这是重点候选。

设当前一段使用 reference \(c_1\)：

\[
C^{(1)}
=
\sum_{j\in E_1}
e^{e_{ij}-c_1}H_j
\]

当参考尺度必须改变到 \(c_2\) 时：

**不要**

\[
C^{(1)}
\leftarrow
e^{c_1-c_2}C^{(1)}
\]

而是结束当前段。

若后面存在右乘线性变换：

\[
V^{(1)}
=
C^{(1)}W
\]

下一段：

\[
C^{(2)}
=
\sum_{j\in E_2}
e^{e_{ij}-c_2}H_j
\]

\[
V^{(2)}
=
C^{(2)}W
\]

最终使用尺度重新合并：

\[
O_i
=
\frac{
\sum_s
e^{c_s-M_i}
V_i^{(s)}
}{
\sum_s
e^{c_s-M_i}
l_i^{(s)}
}
\]

其中：

\[
M_i=\max_s c_s
\]

理论依据：

\[
(aC_1+bC_2)W
=
a(C_1W)+b(C_2W)
\]

即逐行 scale 可以穿过右侧线性变换：

\[
(D_rC)W
=
D_r(CW)
\]

这个方向的目标是：

> 把难以在 TMM 中完成的 D 维 row scale，推迟到较低维输出空间，再交给 ZMM 完成。

特别值得测试：

\[
D=128,\quad d_h=16/32
\]

如果最终输出维度远小于聚合维度，可能显著降低 scale 成本。

---

## 8.3 固定 reference / 不更新 reference

Softmax 对同一行可以减去任意常数：

\[
\alpha_{ij}
=
\frac{
e^{e_{ij}-c_i}
}{
\sum_k e^{e_{ik}-c_i}
}
\]

如果能在聚合开始前得到一个安全且足够 tight 的 \(c_i\)，则整个聚合过程不需要 running max。

这会直接把：

\[
D_rC
\]

删除。

可以研究：

- exact row max；
- safe upper bound；
- block max；
- panel max；
- 共享 tile reference。

---

## 8.4 16 行共享一个 tile reference

一个 16-row AMX tile 共用：

\[
c_T
\]

则每一行：

\[
\alpha_{ij}
=
\frac{
e^{e_{ij}-c_T}
}{
\sum_k e^{e_{ik}-c_T}
}
\]

数学上仍然正确。

好处：

- 16 个 row scale 变成一个公共 reference；
- AMX 控制明显简单。

风险：

如果 16 行真实 max 差很大，较小行会产生很小 BF16 权重。

必须统计：

\[
\Delta_i
=
c_T-\max_j e_{ij}
\]

以及：

- P50/P95/P99；
- BF16 zero weight ratio；
- 输出误差；
- attention mass error。

可以尝试在 Degree Sort 基础上进一步按：

\[
(\text{degree},\text{score/max range})
\]

进行二维 grouping。

---

# 9. 主路线 B：Two-pass 精确扫描

这条路线必须和 online 同等重要，不要把它当次要 fallback。

对于标准 vanilla GAT：

\[
e_{ij}
=
\operatorname{LeakyReLU}(L_i+R_j)
\]

LeakyReLU 单调递增，因此：

\[
\max_{j\in N(i)}e_{ij}
=
\operatorname{LeakyReLU}
\left(
L_i+\max_{j\in N(i)}R_j
\right)
\]

于是第一遍甚至不必完整计算每条边的 \(e_{ij}\)。

---

## 9.1 Two-pass 第一遍：只求最终 row max

每个 destination node：

\[
M_i^R
=
\max_{j\in N(i)}R_j
\]

然后：

\[
m_i
=
\operatorname{LeakyReLU}(L_i+M_i^R)
\]

第一遍只需要：

- 读 CSR index；
- gather \(R_j\)；
- max reduction。

不读取完整 D 维 H。

需要实现：

- scalar reference；
- AVX-512 optimized；
- degree-sorted version；
- panelized version。

统计：

- 第一遍 bytes；
- cache miss；
- cycles/edge；
- 相比主 weighted aggregation 的时间比例。

---

## 9.2 Two-pass 第二遍：固定最终 max 做 weighted aggregation

第二遍：

\[
e_{ij}
=
\operatorname{LeakyReLU}(L_i+R_j)
\]

\[
w_{ij}
=
e^{e_{ij}-m_i}
\]

\[
l_i
+=
w_{ij}
\]

\[
U_i
+=
w_{ij}H_j
\]

因为 \(m_i\) 已经是最终值：

\[
\boxed{
没有任何历史 accumulator rescale
}
\]

这使 AMX 主循环可以简化为：

\[
\boxed{
C\leftarrow C+PH
}
\]

因此 Two-pass 是非常自然的 AMX 方案。

---

## 9.3 Two-pass 的核心研究问题

不要只说“多扫一遍边很慢”。

必须实际比较内存流量。

第一遍每 edge 大致是：

- index；
- 一个 \(R_j\) scalar。

而第二遍 weighted aggregation 每 edge / block 会消费 D 维 H。

例如：

\[
D=128
\]

BF16 H row：

\[
256B
\]

所以必须测：

\[
\boxed{
\text{轻量 scalar edge pass}
}
\]

到底是否显著小于：

\[
\boxed{
\text{online 路线的 spill/rescale/reload}
}
\]

这可能是最终论文的重要结论。

---

# 10. vanilla GAT 的进一步数学优化：减少 edge-level exp

对于 LeakyReLU negative slope \(\lambda\)：

若：

\[
L_i+R_j\ge0
\]

则：

\[
e^{e_{ij}}
=
e^{L_i+R_j}
=
e^{L_i}e^{R_j}
\]

若：

\[
L_i+R_j<0
\]

则：

\[
e^{e_{ij}}
=
e^{\lambda(L_i+R_j)}
=
e^{\lambda L_i}e^{\lambda R_j}
\]

所以可以预计算：

\[
A_i=e^{L_i}
\]

\[
A_i'=e^{\lambda L_i}
\]

\[
B_j=e^{R_j}
\]

\[
B_j'=e^{\lambda R_j}
\]

边上只做：

- 判断 \(L_i+R_j\) 正负；
- 选择 \(A_iB_j\) 或 \(A_i'B_j'\)。

为了数值稳定，建议和最终 row max shift 结合，而不是直接使用 unshifted exponential。

这是可选高级优化，不作为第一版实现前置条件。

---

# 11. 动态权重如何进入 AMX

Attention/softmax 前端大概率由 AVX 生成一个小型权重块：

\[
P
\]

例如：

\[
16\times16
\]

BF16 大小：

\[
16\times16\times2
=
512B
\]

建议：

1. AVX 在 ZMM 中生成 FP32 weight；
2. 转 BF16；
3. 写入 L1 resident staging buffer；
4. `TILELOADD` 载入 AMX；
5. `TDPBF16PS` 做：

\[
C+=PH
\]

不要试图物化整个 \(E\)-size attention array。

需要 microbenchmark：

- AVX exp；
- FP32→BF16；
- 512B/1KB staging write；
- tileload；
- TDPBF16PS；
- 不同 K tile width。

---

# 12. 单头实现要求

单头先作为第一阶段主战场。

至少实现：

## SH-0
DGL / PyTorch FP32 reference。

## SH-1
优化版标准 GAT：

\[
HW
\rightarrow
Attention
\rightarrow
Softmax
\rightarrow
A_\alpha Z
\]

## SH-2
重排但不融合：

\[
L/R
\rightarrow
\alpha
\rightarrow
A_\alpha H
\rightarrow
UW
\]

完整物化：

- \(\alpha\)；
- \(U\)。

## SH-3
TFS-GAT：

\[
L/R
\rightarrow
e
\rightarrow
Softmax
\rightarrow
\boxed{\alpha\text{ 物化}}
\rightarrow
\boxed{TFS(A_\alpha,H,W)}
\]

要求：

- 仍然物化 attention；
- 不物化完整 \(U\)；
- 尽量复用原始 TFS。

## SH-4
Online-AMX：

\[
L/R
\rightarrow
OnlineSoftmax
\rightarrow
AMX\ weighted\ aggregation
\rightarrow
TFS/GeMM
\]

先实现 spill-scale-reload 正确版。

## SH-5
Online-AMX-Optimized：

至少实现一项：

- conditional rebase；
- segmented accumulation；
- shared tile reference；
- fixed reference。

## SH-6
TwoPass-AMX：

第一遍 row max；
第二遍 AMX：

\[
C+=PH
\]

后接 TFS/GeMM。

---

# 13. 多头实现要求

多头不能简单把单头循环 K 次就结束，需要同时研究数据流。

设：

\[
K=\text{head 数}
\]

每头宽度：

\[
d_h
\]

总输出：

\[
F=Kd_h
\]

---

## 13.1 标准多头基线

每头：

\[
Z^{(h)}
=
HW^{(h)}
\]

\[
e_{ij}^{(h)}
=
\operatorname{LeakyReLU}
\left(
a_L^{(h)T}z_i^{(h)}
+
a_R^{(h)T}z_j^{(h)}
\right)
\]

\[
O_i^{(h)}
=
\sum_j
\alpha_{ij}^{(h)}z_j^{(h)}
\]

最后 concat 或 average。

这是必须保留的公平基线。

---

## 13.2 多头 aggregate-first

对每个 head：

\[
U^{(h)}
=
A_{\alpha^{(h)}}H
\]

然后：

\[
O^{(h)}
=
U^{(h)}W^{(h)}
\]

必须明确报告 sparse work 膨胀：

\[
\frac{D}{d_h}
\]

所以至少测试：

\[
K=1,2,4,8
\]

\[
d_h=16,32,64
\]

\[
D=64,128,256
\]

并建立 selector：

什么时候使用：

\[
A_\alpha H\rightarrow W
\]

什么时候回退到：

\[
HW\rightarrow A_\alpha Z
\]

---

## 13.3 多头 Online Softmax 状态布局

每个 destination row 每个 head 都需要：

- `m[h]`；
- `l[h]`；
- reference scale；
- active segment count；
- optional accumulator metadata。

要尝试至少两种布局：

### Head-major
先处理一个 head 的多个节点。

优点：

- AMX/TFS 复用单头路径容易；
- 状态简单。

### Node-major / multi-head packed
同一批节点同时处理多个 head。

可能改善：

- graph index reuse；
- L/R source gather reuse；
- CSR traversal reuse。

但会增加：

- ZMM pressure；
- TMM pressure；
- staging buffer complexity。

不要预设哪个更优。

---

# 14. 低度节点 AVX、高度节点 AMX 的混合路径

必须考虑。

如果：

\[
D=128
\]

一个 row accumulator 需要 8 个 ZMM。

对于低 degree 节点：

\[
U_i += w_{ij}H_j
\]

完全可以用 AVX-512 完成。

且 online rescale：

\[
U_i\leftarrow r_iU_i
\]

对 ZMM 很自然。

所以应实现：

\[
\boxed{
degree < T
\Rightarrow
AVX
}
\]

\[
\boxed{
degree \ge T
\Rightarrow
AMX
}
\]

扫阈值：

\[
T=8,16,32,64,128
\]

并结合：

- D；
- d_h；
- tile occupancy；
- graph degree distribution。

最终可以形成：

\[
\boxed{
\text{AVX–AMX 自适应 GAT 执行器}
}
\]

---

# 15. 必须实现的基线体系

不能只和 DGL 比。

---

## Baseline A：DGL GATConv / PyG GAT

作用：

- 框架级 reference；
- FP32 正确性 reference；
- 通用框架性能参考。

---

## Baseline B：Optimized Vanilla GAT

这是最重要的主基线。

数据流：

\[
HW
\rightarrow
Attention
\rightarrow
Softmax
\rightarrow
A_\alpha Z
\]

要求：

- dense GeMM：oneDNN / AMX BF16；
- edge score：AVX-512；
- softmax：AVX-512 FP32；
- weighted SpMM：oneMKL 或自研 AVX-512，取强者；
- 不做跨阶段融合。

目的：

> 证明我们的收益不是“C++ 比 DGL 快”，而是新的 GAT 数据流比充分优化的传统实现快。

---

## Baseline C：Reordered-Unfused GAT

数据流：

\[
L/R
\rightarrow
e
\rightarrow
\alpha
\rightarrow
A_\alpha H
\rightarrow
UW
\]

物化：

- \(\alpha\)；
- \(U\)。

作用：

> 单独测量代数重排本身的影响。

---

## Baseline D：TFS-GAT

数据流：

\[
L/R
\rightarrow
e
\rightarrow
Softmax
\rightarrow
\alpha\text{ 物化}
\rightarrow
TFS(A_\alpha,H,W)
\]

作用：

> 隔离“原始 TFS 后端”与“我们新增 Attention/Softmax 融合”的贡献。

---

## Baseline E：TFS Oracle

提前准备好 attention weights，不计 attention 生成时间。

只测：

\[
A_\alpha H\rightarrow UW
\]

作用：

> 给出动态 GAT 接近静态 TFS 后端性能的理想上界。

可以报告：

\[
\text{Dynamic Attention Overhead}
=
\frac{
T_{\text{Ours}}-T_{\text{TFS Oracle}}
}{
T_{\text{TFS Oracle}}
}
\]

---

# 16. 公平精度设置

必须区分：

## Correctness reference
FP32。

## 性能主路径
建议：

- H：BF16；
- W：BF16；
- AMX product：BF16×BF16；
- TMM accumulation：FP32；
- score：FP32；
- max：FP32；
- exp：FP32；
- denominator：FP32；
- reference scale：FP32；
- 最终 normalize：FP32；
- 必要时输出再转 BF16。

所有性能基线必须尽量使用相同精度。

不能：

- Ours 用 BF16；
- Baseline 用 FP32；
- 然后把全部 speedup 归因于数据流。

---

# 17. 正确性验证

每个新 kernel 都要先经过小图 reference。

至少构造：

## Case 1：degree=1
最简单。

## Case 2：degree=2/4/8
检查 softmax。

## Case 3：最大 score 出现在第一个邻居
几乎不发生 rescale。

## Case 4：最大 score 最后出现
传统 online 最难情况。

## Case 5：score 单调递增
制造大量 max update。

## Case 6：score range 很大
测试数值范围。

## Case 7：空节点/孤立节点
按模型定义处理。

## Case 8：self-loop
和 DGL 行为对齐。

## Case 9：多头
每个 head 独立 reference。

报告：

- max absolute error；
- mean absolute error；
- max relative error；
- cosine similarity；
- softmax row sum；
- NaN / Inf；
- BF16 zero-weight ratio；
- 最终模型精度/任务指标。

---

# 18. 计时口径

必须同时报告：

## Kernel-only

分别计时：

1. \(L/R\) projection；
2. edge score；
3. row max；
4. exp；
5. denominator；
6. dynamic BF16 weight staging；
7. weighted SpMM；
8. GeMM；
9. row scale / rescale；
10. BF16 conversion；
11. concat / normalize。

## Full layer E2E

从 layer input H 到 layer output O。

必须包含：

- attention；
- softmax；
- sparse aggregation；
- dense projection；
- runtime conversion；
- runtime staging。

可以排除：

- graph file loading；
- 一次性模型加载；
- 静态 graph preprocessing；
- 静态 degree sort。

但要另外报告 preprocessing cost。

---

# 19. 静态 preprocessing 与动态 runtime 必须分开

图拓扑在推理过程中通常固定，因此以下可以视为一次性：

- CSR/CSC build；
- Degree Sort；
- permutation；
- panel metadata；
- source block mapping；
- static scheduling；
- tile occupancy metadata。

但以下不能放进 preprocessing：

- attention score；
- \(L/R\)；
- softmax max；
- attention weight；
- 每层动态 BF16 weight tile；
- runtime row grouping（如果依赖 score）。

如果某个方法需要根据本层 attention score 动态排序节点，则这部分必须计入 E2E。

---

# 20. 实验统计：必须做，不要只看 latency

为了判断 online vs two-pass，至少记录：

## 图结构

- N；
- E；
- avg degree；
- max degree；
- degree P50/P95/P99；
- degree histogram；
- tile occupancy；
- empty lane ratio。

## Online Softmax

- max updates/row；
- max updates/edge；
- physical rescale/row；
- physical rescale/edge；
- P50/P95/P99；
- score range；
- max increase delta；
- number of segments；
- spill bytes。

## Two-pass

- first-pass cycles/edge；
- first-pass bytes；
- first-pass cache misses；
- first-pass / E2E ratio。

## AMX

- tileload；
- tilestore；
- `TDPBF16PS` count；
- AMX cycles；
- tile utilization；
- BF16 conversion time；
- staging time。

## Memory

- L1/L2/LLC misses；
- memory bandwidth；
- bytes per edge；
- bytes per output feature。

---

# 21. 数据集建议

第一阶段至少覆盖三类图：

## 低度图
如 Cora / Citeseer 类。

观察：

- AMX 是否值得；
- AVX path 是否更强。

## 中高程度真实图
如 Reddit / ogbn-arxiv / products 类。

观察：

- AMX tile occupancy；
- two-pass overhead；
- degree sorting。

## 极度偏斜图
social / web graph。

观察：

- hub；
- load imbalance；
- multi-path selector；
- online rescale frequency。

最终以当前 TFS 论文使用的数据集优先，保证与原工作可对比。

---

# 22. Shape 扫描

至少扫：

\[
D=32,64,128,256
\]

单头：

\[
F=16,32,64,128
\]

多头：

\[
K=1,2,4,8
\]

\[
d_h=16,32,64
\]

还要扫：

- degree bucket；
- source block size；
- destination rows/tile；
- softmax edge block K；
- AVX/AMX threshold。

---

# 23. 实现顺序

严格建议按下面顺序，不要直接写“最终方案”。

---

## Stage 0：读源码

输出：

`TFS_CODE_WALKTHROUGH.md`

---

## Stage 1：FP32 GAT Reference

实现：

- 单头；
- 多头；
- 标准 softmax；
- online softmax；
- two-pass softmax。

全部相互对齐。

---

## Stage 2：CPU 强基线

实现：

- DGL reference；
- Optimized Vanilla；
- Reordered-Unfused。

先得到稳定 E2E。

---

## Stage 3：TFS-GAT

把动态 attention weights 写进 TFS 可消费的结构。

此时先允许：

\[
\alpha
\]

完整物化。

验证：

> 原 TFS 后端在动态 weighted adjacency 上是否正确。

---

## Stage 4：TwoPass-AMX

优先实现。

原因：

- 数学简单；
- 没有 TMM row rescale；
- 容易判断 AMX 主体性能。

流程：

\[
L/R
\]

↓

第一遍：

\[
m_i
\]

↓

第二遍：

\[
P
\]

↓

AMX：

\[
C+=PH
\]

↓

TFS/GeMM。

---

## Stage 5：Online-AMX Correct

使用 spill-scale-reload。

目的：

- 正确；
- 得到真实 rescale cost；
- 建立 baseline。

---

## Stage 6：Online-AMX Optimized

依次尝试：

1. conditional rebase；
2. segmented accumulation；
3. fixed row reference；
4. tile shared reference；
5. 其他新方法。

每个方案必须单独开关。

---

## Stage 7：Multi-head

先：

- standard transform-first；
- repeated single-head aggregate-first。

再优化：

- graph traversal reuse；
- head packing；
- source-panel projection；
- selector。

---

## Stage 8：AVX/AMX Hybrid

实现 degree-aware selector。

---

# 24. 推荐代码模块划分

最终代码不要全部堆在一个 benchmark 中。

建议：

```text
gat_tfs/
├── reference/
│   ├── gat_fp32_reference.*
│   ├── softmax_reference.*
│   └── multihead_reference.*
│
├── preprocess/
│   ├── graph_reorder.*
│   ├── degree_sort.*
│   ├── panel_builder.*
│   └── permutation.*
│
├── attention/
│   ├── lr_projection.*
│   ├── edge_score_avx512.*
│   ├── softmax_standard.*
│   ├── softmax_online.*
│   ├── softmax_twopass.*
│   └── weight_pack_bf16.*
│
├── kernels/
│   ├── avx_weighted_spmm.*
│   ├── amx_weighted_spmm.*
│   ├── amx_online_spill.*
│   ├── amx_online_segmented.*
│   ├── amx_twopass.*
│   └── tfs_backend_adapter.*
│
├── multihead/
│   ├── transform_first.*
│   ├── aggregate_first.*
│   ├── head_major.*
│   ├── node_major.*
│   └── source_panel_projection.*
│
├── baseline/
│   ├── vanilla_optimized.*
│   ├── reordered_unfused.*
│   ├── tfs_gat.*
│   └── tfs_oracle.*
│
├── benchmark/
│   ├── microbench_softmax.*
│   ├── microbench_tile_scale.*
│   ├── microbench_weight_staging.*
│   ├── bench_single_head.*
│   ├── bench_multi_head.*
│   └── bench_e2e.*
│
└── scripts/
    ├── run_shapes.*
    ├── run_graphs.*
    └── collect_results.*
```

如果原始 TFS 项目结构不适合这样拆，至少逻辑上保持这些模块边界。

---

# 25. 所有关键设计必须支持运行时开关

建议命令行：

```bash
--mode vanilla
--mode reordered
--mode tfs-gat
--mode online-amx
--mode twopass-amx
```

以及：

```bash
--heads 1
--head-dim 16
--input-dim 128
```

Online variant：

```bash
--online-rescale spill
--online-rescale conditional
--online-rescale segmented
--online-rescale fixed
--online-rescale tile-shared
```

执行器：

```bash
--backend avx
--backend amx
--backend hybrid
```

这样后续做消融非常重要。

---

# 26. 输出 CSV 必须统一格式

每次 benchmark 输出至少：

```text
dataset
N
E
avg_degree
max_degree
D
heads
head_dim
mode
backend
precision
block_size
degree_threshold
online_variant
time_lr_ms
time_score_ms
time_softmax_ms
time_pass1_ms
time_weight_pack_ms
time_spmm_ms
time_gemm_ms
time_rescale_ms
time_total_ms
max_updates
physical_rescales
num_segments
tile_util
l1_misses
l2_misses
llc_misses
max_abs_error
max_rel_error
```

这样后面才能系统画图。

---

# 27. 论文式核心问题

最终不要把论文写成：

> “我们把 GAT 搬到了 AMX。”

真正要回答的是：

### Q1
为什么 GPU sparse attention 的 online softmax fusion 不能直接搬到 AMX？

答案应落在：

- FP32 TMM accumulator；
- 无 tile row-wise arithmetic；
- TMM/ZMM 数据通路；
- sparse irregularity；
- tile utilization。

### Q2
Online Softmax 和 Two-pass 在 AMX CPU 上谁更合理？

不是先验回答，要靠：

- edge pass bytes；
- rescale bytes；
- degree；
- feature dimension；
- score distribution。

### Q3
TFS 的 aggregate-first 对单头有效时，多头还能否成立？

重点：

\[
D/d_h
\]

### Q4
是否需要 AVX/AMX 混合执行？

重点：

- low-degree；
- high-degree；
- tile occupancy。

### Q5
最终收益来自哪里？

通过：

- Vanilla；
- Reordered；
- TFS-GAT；
- Ours；

逐级消融回答。

---

# 28. 第一阶段最关键的实验表

必须优先得到：

| 方法 | 单头正确 | 多头正确 | α物化 | U物化 | Online | Two-pass | AVX | AMX |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| DGL | ✓ | ✓ | 框架内部 | — | × | × | 框架 | 框架 |
| Optimized Vanilla | ✓ | ✓ | ✓ | — | × | × | ✓ | GeMM |
| Reordered-Unfused | ✓ | ✓ | ✓ | ✓ | × | × | ✓ | GeMM |
| TFS-GAT | ✓ | ✓ | ✓ | × | × | × | ✓ | ✓ |
| Online-AMX | ✓ | ✓ | × | × | ✓ | × | ✓ | ✓ |
| TwoPass-AMX | ✓ | ✓ | × | × | × | ✓ | ✓ | ✓ |

---

# 29. 不要犯的错误

## 错误 1
只和 DGL 比。

## 错误 2
用 BF16 Ours 对 FP32 baseline 宣称数据流 speedup。

## 错误 3
只测试单头，然后默认多头成立。

## 错误 4
把 graph preprocessing 时间偷偷排除，但 Ours 的动态 preprocessing 又不计。

## 错误 5
只报 kernel latency，不报整层 E2E。

## 错误 6
只报平均 degree，不看 degree distribution。

## 错误 7
为了“全 AMX”而强迫低度节点使用 AMX。

## 错误 8
为了坚持 online softmax 而拒绝 two-pass。

## 错误 9
把数学上的 \(A(HW)=(AH)W\) 当成性能上的无条件等价。

## 错误 10
没有真正确认原始 TFS 的 FP32→BF16 中间格式和 tile 生命周期，就假设 SpMM→GeMM 可直接完全 TMM-resident。

---

# 30. 当前最有希望的研究主线

现阶段不要把核心贡献提前锁死为：

> “AMX 逐行缩放”。

更稳妥的目标是：

\[
\boxed{
\text{设计 AMX 友好的 GAT 行级尺度管理与动态稀疏注意力数据流}
}
\]

理想情况下：

- attention score 不全局落地；
- softmax weight 不全局落地；
- weighted aggregation 中间矩阵不全局落地；
- AMX 只负责适合它的矩阵乘；
- AVX 负责 max/exp/scale/normalize；
- online 与 two-pass 根据图和 shape 自适应选择；
- 单头与多头根据 \(D/d_h\) 选择不同 dataflow；
- 低度节点用 AVX；
- 高度/高密度块用 AMX。

最终系统可以成为：

\[
\boxed{
\text{Graph/Shape-Aware AVX–AMX GAT Inference Engine}
}
\]

而不是一个单一硬编码 kernel。

---

# 31. 推荐首先完成的最小可运行版本

第一周/第一阶段不要追求复杂创新。

先完成：

1. FP32 standard GAT reference；
2. FP32 online softmax reference；
3. FP32 two-pass reference；
4. Optimized Vanilla CPU baseline；
5. Reordered-Unfused；
6. TFS-GAT；
7. TwoPass-AMX；
8. Online-AMX spill-scale-reload；
9. 单头完整 timing；
10. K=4/8 的多头正确性。

只有完成这十项之后，再决定：

- conditional rebase 是否值得；
- segmented accumulation 是否值得；
- shared reference 是否值得；
- source-panel projection 是否值得。

---

# 32. 期望 AI 每完成一个阶段都输出什么

每次提交不要只说“实现完成”。

必须给：

## 代码改动
- 文件；
- 函数；
- 新数据结构；
- 编译参数。

## 正确性
- 对比 reference；
- 最大误差；
- 平均误差；
- 是否有 NaN/Inf。

## 性能
- shape；
- dataset；
- threads；
- NUMA；
- latency；
- throughput；
- 分项 breakdown。

## 解释
- 为什么快/慢；
- 是否符合 bytes/FLOP 模型；
- 是否出现新的瓶颈。

## 下一步
根据数据决定，不要按本文机械执行。

---

# 33. 最终研究目标的一句话版本

我们希望实现的不是简单的“GAT 用 AMX”。

而是：

> **针对 Intel AMX 缺乏 tile 内逐行算术、而 GAT Softmax 又需要行级动态尺度管理这一结构性冲突，重新设计 vanilla GAT 的注意力、Softmax、加权聚合与线性变换数据流；通过 Online Softmax / Two-pass、AVX–AMX 异构分工、TFS 后端融合以及单头/多头自适应数据流，尽可能消除边级 attention、softmax 权重和节点级聚合中间矩阵的全局物化。**

更激进的目标是：

> **让整条 GAT 推理链只在 AVX-512 寄存器、AMX tile 和 L1 小型块缓冲中流动，而不生成大规模中间张量。**

---

# 34. 给接手 AI 的最后要求

请不要把本文当作“答案”。

本文只是当前设计空间与实验协议。

真正接手后：

1. 先读 TFS 源码和论文；
2. 核对本文对 TFS 的任何假设；
3. 如果源码事实与本文不一致，以源码为准；
4. 任何新方案先做正确性 reference；
5. 用真实 profiling 数据决定方向；
6. 不要为了证明某个方案有效而修改计时口径；
7. Online 和 Two-pass 必须公平实现；
8. 多头必须作为一等公民，而不是单头完成后简单复制；
9. 优先追求“消除不必要数据移动”，而不是追求“所有计算都用 AMX”；
10. 如果发现比本文更好的数据流，应直接提出并实现，不受本文候选方法限制。

---

# 35. 当前建议的第一批 TODO

```text
[ ] 阅读并总结 TFS 源码
[ ] 确认 TFS 的真实 tile lifecycle
[ ] 确认 weighted CSR 是否能直接接入 TFS
[ ] 实现 FP32 single-head GAT reference
[ ] 实现 FP32 multi-head GAT reference
[ ] 实现 FP32 online softmax reference
[ ] 实现 FP32 two-pass reference
[ ] 实现 Optimized Vanilla CPU baseline
[ ] 实现 Reordered-Unfused baseline
[ ] 实现 TFS-GAT baseline
[ ] 实现 TwoPass-AMX
[ ] 实现 Online-AMX spill baseline
[ ] 统计真实 max-update 分布
[ ] microbench TMM spill-scale-reload
[ ] microbench AVX→BF16→L1→TILELOAD
[ ] 扫 single-head shapes
[ ] 扫 multi-head K/d_h
[ ] 建立 D/d_h 成本模型
[ ] 测 degree-aware AVX/AMX selector
[ ] 再决定 advanced online softmax 方案
```

---

# 36. 最终成功标准

这项工作真正成功，不是“代码跑通”。

至少满足：

### 正确性
GAT 单头/多头输出与 FP32 reference 在预期误差范围内。

### 性能
在至少一组真实图 + 真实 GAT shape 上：

\[
T_{\text{Ours}}
<
T_{\text{Optimized Vanilla}}
\]

并且：

\[
T_{\text{Ours}}
<
T_{\text{TFS-GAT}}
\]

说明新增 attention/softmax 融合确实有贡献。

### 解释性
能够回答：

- 为什么这个图快？
- 为什么那个图不快？
- 为什么 online 赢/输给 two-pass？
- 为什么单头和多头行为不同？
- 为什么 AVX 或 AMX 在某些 degree bucket 更合理？

### 泛化
最终方法不是只对一个固定：

\[
D=128,\ K=1
\]

有效，而是形成明确的：

\[
(\text{graph structure},D,K,d_h)
\rightarrow
\text{dataflow/backend}
\]

选择规则。

---

**交接结束。**
