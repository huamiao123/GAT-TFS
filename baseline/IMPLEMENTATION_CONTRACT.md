# 多头 Vanilla GAT / TFS-AMX 基线搭建与实现合同

> 版本：v1.0  
> 目标：**先把基线搭对、搭强、搭公平，再进入新的融合数据流设计。**  
> 目标平台：Intel Sapphire Rapids / Xeon Max 一代，支持 AMX-BF16、AVX-512 BF16。  
> 当前模型主线：固定图拓扑、多头 Vanilla GAT 前向推理；不包含训练、GATv2、Graph Transformer。  
> 当前典型三层形状：`128 -> 8x32 -> 8x32 -> 1xC`，当前代码中的 `C=40`。  

---

## 0. 先给最终结论

本阶段不要直接实现“我们的完整融合方案”。先固定以下三条路径：

1. **R0：FP32 Correctness Reference**  
   只负责给出可信结果，不负责性能。完整物化 `Z / score / alpha`，所有关键中间结果可检查。

2. **B0：Strong Standard GAT（不使用 TFS）**  
   使用标准 **Transform-First** 数据流：

   \[
   XW \rightarrow L/R \rightarrow \text{Score} \rightarrow \text{Softmax} \rightarrow \text{weighted SpMM}
   \]

   - 大型稠密 GEMM：优先 oneMKL BF16 GEMM，令库在目标 CPU 上使用 AMX；
   - 动态稀疏 attention / SpMM：自定义 AVX-512 内核；
   - 不物化全局 `alpha`，采用**精确最大值预扫描 + 第二遍直接聚合**，形成强基线；
   - 输出和 softmax 状态保持 FP32。

3. **B1：TFS-GAT Baseline（使用原 TFS 思想）**  
   使用加权版原始 TFS 数据流：

   \[
   Y_i^h
   =\frac{1}{\ell_i^h}
   \sum_{j\in\mathcal N(i)}
   p_{ij}^h\,(X_jW^h)
   \]

   实际 AMX 映射时把权重先乘到邻居特征：

   \[
   (p_{ij}^hX_j)W^h
   \]

   然后使用原 TFS 的 **16 destination rows + degree sort + AMX output accumulation**。  
   Attention 前端采用 `b_L=W a_L, b_R=W a_R` 重排，避免为了打分生成完整 `Z`。

> **正式性能结论至少同时报告 B0 和 B1；R0 只做正确性。**  
> 此外建议保留一个 `B0-FP32` 辅助结果，用来与原 TFS 工作的 MKL-FP32 口径对照，但真正用于判断“数据流是否更快”的主要比较应尽量使用 BF16-AMX 的强 B0。

---

# 1. 为什么必须重新定义基线

## 1.1 原 TFS 源码中的 MKL baseline 并不是 BF16-AMX baseline

项目固定提交中的 `GAT/code/code/mkl_baseline.cpp` 使用：

```cpp
mkl_sparse_s_mm(...);  // FP32 SpMM
cblas_sgemm(...);       // FP32 GeMM
```

因此原比较本质是：

\[
\boxed{\text{MKL FP32 two-step}}
\quad\text{vs}\quad
\boxed{\text{TFS BF16-AMX}}
\]

而 `amx_tfs_v3.cpp` 显式把 H 和 W 转成 BF16，并调用：

```cpp
_tile_dpbf16ps(...)
```

即：

\[
BF16\times BF16\rightarrow FP32\ accumulator.
\]

因此原 TFS 需要做精度比较是合理的；但我们现在搭 GAT 基线时，应进一步增加**精度匹配的强性能基线**，否则容易把 BF16 的收益和 TFS 数据流收益混在一起。

## 1.2 当前 GAT 原型也不能直接充当最终 baseline

当前 `implementation/src/gat_online.cpp`：

- `projection_lr()` 使用 `cblas_sgemm`，是 FP32；
- `reference()` 物化完整 `scores[E,K]` 和 `alpha[E,K]`；
- `online()` 是 FP32/AVX-512 online softmax；
- 尚未使用 AMX。

当前 `gat_tfs_online.cpp` 中的 `tfs_online_fused()`：

- 使用线程局部 `16 x D` FP32 `Utile`；
- 后续用 `cblas_sgemm`；
- 这是 TFS-inspired 的 FP32 local-U 原型，**不是原始 TFS AMX 内核，也不是最终 AMX baseline**。

因此本阶段应新建独立 baseline 代码，不在旧实验程序上继续堆分支。

---

# 2. 模型语义必须先冻结

## 2.1 符号

- `N`：节点数
- `E`：CSR 中实际参与聚合的边数
- `D`：当前层输入维度
- `K`：head 数
- `d`：每个 head 输出维度
- `X in R^{N x D}`：当前层输入
- `W^h in R^{D x d}`：第 h 个 head 的投影权重
- `a_L^h, a_R^h in R^d`
- `Z^h = X W^h`
- 当前 CSR 行表示 destination node，列索引表示 source node

## 2.2 Vanilla GAT 层

对每个 head：

\[
Z^h=XW^h
\]

\[
L_i^h=Z_i^ha_L^h,\qquad R_j^h=Z_j^ha_R^h
\]

\[
s_{ij}^h=\operatorname{LeakyReLU}(L_i^h+R_j^h)
\]

\[
\alpha_{ij}^h=
\frac{\exp(s_{ij}^h)}
{\sum_{k\in\mathcal N(i)}\exp(s_{ik}^h)}
\]

\[
Y_i^h=\sum_{j\in\mathcal N(i)}\alpha_{ij}^hZ_j^h
\]

隐藏层按模型定义 concat heads，再做 ELU；最后一层当前代码为单 head。

## 2.3 图语义

所有路径必须使用完全相同的图：

- 同一个 CSR；
- 同一方向；
- 同一 self-loop 规则；
- 同一去重规则；
- 同一节点顺序；
- 同一个 checkpoint / 权重。

图预处理不是“某个 baseline 的优化”，必须在所有路径之前统一完成。

---

# 3. 精度合同：先定义，再写代码

## 3.1 Master 数据

保留：

- `X_master`: FP32
- `W_master`: FP32
- `aL/aR`: FP32

用于 R0 与误差检查。

## 3.2 BF16 性能路径

对于需要 AMX 的矩阵乘：

- `X_bf16 = RNE_BF16(X_master)`
- `W_bf16 = RNE_BF16(W_master)`
- AMX / oneMKL 执行：

\[
BF16\times BF16\rightarrow FP32.
\]

**不要继续使用原 TFS 源码中简单截高 16 位的转换作为默认新 baseline。**  
为了复现实验可以保留 `legacy_truncate` 模式，但正式新 baseline 默认使用 round-to-nearest-even。

## 3.3 Softmax 相关状态

以下全部使用 FP32：

- `L/R`
- score
- row max
- `exp`
- denominator `ell`
- 最终 normalization

不要为了“全 BF16”强行量化 softmax 状态。

## 3.4 误差来源必须分开记录

至少区分：

1. X/W BF16 量化误差；
2. dense projection 累加顺序差异；
3. attention L/R 重排误差；
4. TFS 中 `p * X -> BF16` 的额外量化；
5. sparse reduction 顺序差异；
6. 多层累计误差。

---

# 4. R0：FP32 Correctness Reference

R0 不参与主要 speedup 图。

## 4.1 数据流

```text
X_fp32
  |
  | SGEMM FP32
  v
Z_fp32 [N, K*d]
  |
  +--> L/R FP32
  |
  v
score[E,K] FP32
  |
  v
alpha[E,K] FP32
  |
  v
weighted aggregation FP32
  |
  v
concat -> ELU -> output FP32
```

## 4.2 算子选择

- `Z = XW`：`cblas_sgemm`
- `L/R`：简单 FP32 循环或 AVX-512；正确性优先
- score：标量/AVX 均可
- softmax：经典 max-subtract FP32
- weighted aggregation：简单 FP32 实现
- activation：FP32

## 4.3 为什么要物化 score / alpha

因为 R0 的职责不是省内存，而是：

- 可以逐边检查 score；
- 可以逐边检查 alpha；
- 可以单独验证 denominator；
- 可以定位 B0/B1 的错误究竟来自哪一步。

---

# 5. B0：Strong Standard GAT（不使用 TFS）

B0 必须足够强，不能为了突出 TFS 故意保留弱实现。

---

## 5.1 B0 的总体数据流

```text
X_fp32
  |
  | FP32 -> BF16 (计入层间转换成本)
  v
X_bf16
  |
  | oneMKL BF16 GEMM (AMX candidate)
  v
Z_fp32 [N, K*d]
  |
  | fused AVX512 dot
  v
L/R_fp32 [N,K]
  |
  | exact max prescan on CSR
  v
M_fp32 [N,K]
  |
  | second CSR pass:
  | score -> exp -> denom + weighted Z aggregation
  v
Y_fp32 [N,K*d]
  |
  v
ELU / output
```

不生成全局 `score[E,K]`，也不生成全局 `alpha[E,K]`。

---

## 5.2 Step B0-1：输入 FP32 -> BF16

### 目的

给 AMX dense projection 提供 BF16 输入。

### 推荐实现

自定义 AVX-512 BF16 RNE conversion kernel。

### 计时规则

- 第一层若模型输入本来是 FP32：转换计入 layer E2E；
- 前一层若已经留下可复用 BF16 输出：不得重复转换；
- 但不能只给 B0 缓存 BF16、不给 B1 缓存。

---

## 5.3 Step B0-2：Dense projection `Z = X W_all`

将所有 head 的 W 拼成：

\[
W_{all}\in R^{D\times Kd}
\]

一次 GEMM：

\[
[N,D]\times[D,Kd]\rightarrow[N,Kd].
\]

### 为什么必须一次大 GEMM

不要默认做 K 个小 GEMM。一次 `N x D` 乘 `D x Kd`：

- 更容易让 oneMKL 获得高利用率；
- 减少调用和 packing 开销；
- W 连续；
- 输出天然是 `[node][head][d]`。

### 推荐算子

首选：

```cpp
cblas_gemm_bf16bf16f32(...)
```

输入 BF16，输出 FP32。

对于固定权重，可进一步使用：

```cpp
cblas_gemm_bf16bf16f32_pack(... W ...)
cblas_gemm_bf16bf16f32_compute(...)
```

W packing 可在模型初始化时完成。

### 必须验证

不能只因为调用了 BF16 API 就写“使用 AMX”。

至少记录：

- oneMKL 版本；
- `MKL_VERBOSE=1` 输出；
- CPU ISA；
- 必要时用 VTune / 经验证的硬件计数确认 AMX 执行。

### 当前三层形状

- L1：`[N,128] x [128,256] -> [N,256]`
- L2：`[N,256] x [256,256] -> [N,256]`
- L3：`[N,256] x [256,40] -> [N,40]`

这三类都应单独微基准。

---

## 5.4 Step B0-3：从 Z 计算 L/R

对每个 node/head：

\[
L=Z_h\cdot a_L^h,\qquad R=Z_h\cdot a_R^h.
\]

### 不建议直接使用 MKL 的原因

当前 `d=32`，输出只有 2 个标量。若每个 head 调一次 `N x d` 乘 `d x 2` 的 GEMM，调用和 skinny GEMM 开销可能不划算。

### 默认推荐

自定义 AVX-512 fused dot：

- Z 只读取一次；
- 同时累加 L 和 R；
- `d=32` 恰好两组 16 FP32；
- `d=40` 做 32 + tail 8。

### 必须保留的对照

微基准一次：

- AVX512 fused dot
- `cblas_sgemm(N,2,d)`

选更快的作为正式 baseline，之后固定，不按测试数据挑。

---

## 5.5 Step B0-4：精确最大值预扫描

由于 LeakyReLU 单调：

\[
m_i^h
=
\operatorname{LeakyReLU}
\left(L_i^h+\max_{j\in\mathcal N(i)}R_j^h\right).
\]

因此第一遍 CSR 只需：

- 读 neighbor index；
- 读 `R_j^h`；
- 求每行每 head 的 `maxR`；
- 最后加 `L_i^h` + LeakyReLU。

得到：

```text
M[N,K]
```

### 为什么它适合做强 baseline

- 不保存 `score[E,K]`；
- 不保存 `alpha[E,K]`；
- 不需要 online running-max rescale；
- 第一遍不读取 d 维或 D 维 feature；
- 数值稳定且容易验证。

### 推荐实现

自定义 AVX-512 CSR kernel。

对 K=8，可以把 8 个 head 作为向量 lane，一次处理一个 neighbor 的 `R[j,0:8]`。

---

## 5.6 Step B0-5：第二遍 CSR，Softmax + weighted aggregation 融合

对 destination row i：

初始化：

```text
den[K] = 0
out[K][d] = 0
```

对每条边 `j -> i`：

\[
s_h=LeakyReLU(L_i^h+R_j^h)
\]

\[
p_h=\exp(s_h-M_i^h)
\]

\[
den_h += p_h
\]

\[
out_h += p_h Z_j^h.
\]

最终：

\[
out_h/=den_h.
\]

### 为什么不用 MKL Sparse

当前 oneMKL `mkl_sparse_?_mm` 公共 CPU API是 `s/d/c/z` 数据类型；没有对应 BF16 sparse-mm API。更关键的是 GAT 的 edge weight：

- 每个 input 变化；
- 每个 layer 变化；
- 每个 head 不同。

如果为了每个 head 的 alpha 重建/优化 MKL sparse handle，会引入非常大的动态准备开销。

因此强 B0 应采用自定义 AVX-512 dynamic weighted SpMM，而不是把动态 attention 硬塞进 MKL sparse API。

### 推荐 AVX 数据布局

`Z[node][head][d]` 连续。

对每条边：

- score/exp 可按 head 向量化；
- 每个 head 的 `d=32` 用两个 ZMM 做 FMA；
- `p_h` broadcast 后：

```cpp
out0 = fmadd(p, z0, out0);
out1 = fmadd(p, z1, out1);
```

### 并行

OpenMP 按 destination row / row panel 分配；单行只由一个 worker 负责，不需要原子操作。

---

## 5.7 Step B0-6：normalize + activation + write

- denominator FP32；
- `out /= den` FP32；
- hidden layer ELU FP32；
- 直接写最终 `[N,K*d]` 布局，避免额外 concat pass；
- 下一层需要 BF16 时再执行统一 conversion。

---

# 6. B1：TFS-GAT Baseline（真正使用原 TFS 思想）

这一条不能使用当前 `gat_tfs_online.cpp` 的 local-U + SGEMM 代替。

它必须真正执行：

\[
\boxed{
\sum_j p_{ij}^h(X_jW^h)
}
\]

并让输出 partial sum 驻留在 AMX TMM 中。

---

## 6.1 TFS-GAT 的代数

因为 W 对 neighbor 求和是线性的：

\[
Y_i^h=
\frac{1}{\ell_i^h}
\left(\sum_jp_{ij}^hX_j\right)W^h
\]

也等价于：

\[
Y_i^h=
\frac{1}{\ell_i^h}
\sum_jp_{ij}^h(X_jW^h).
\]

为了继承原 TFS 内核，B1 选择后一种执行顺序。

在 AMX 输入侧实现为：

\[
\boxed{(p_{ij}^h X_j)W^h}
\]

即每一条有效边的 feature row 先乘对应 attention weight，再进入 AMX。

---

## 6.2 Step B1-0：静态权重预处理

### 每个 head 的 W pack

将：

\[
W^h\in R^{D\times d}
\]

转换成：

- BF16 RNE；
- TDPBF16PS 需要的配对/VNNI布局；
- 每层每 head 只做一次。

### Attention 重排向量

预计算：

\[
b_L^h=W^ha_L^h,
\qquad
b_R^h=W^ha_R^h.
\]

这是固定 checkpoint 下的模型静态准备，不应计入每次 steady-state inference；但 cold-start 单独报告。

---

## 6.3 Step B1-1：生成 L/R，不生成完整 Z

构造：

\[
B_{LR}=[b_L^1,\ldots,b_L^K,b_R^1,\ldots,b_R^K]
\in R^{D\times 2K}.
\]

然后：

\[
LR=X B_{LR}.
\]

### K=8 的主层

输出宽度 `2K=16`，是一个合理的 dense GEMM 形状。

候选：

```cpp
cblas_gemm_bf16bf16f32(...)
```

即：

\[
[N,D]_{BF16}\times[D,16]_{BF16}
\rightarrow[N,16]_{FP32}.
\]

### 最后一层 K=1

输出宽度只有 2。这里不应为了“必须 AMX”而强行使用慢路径。

必须微基准：

- BF16 oneMKL GEMM；
- FP32 SGEMM；
- 自定义 AVX512 两 dot。

选择最快者，固定为 baseline。

### 精度控制

同时保留 `fp32_lr_control`：

- `b_L/b_R` FP32；
- X FP32；
- SGEMM FP32。

用于判断 BF16 attention-prep 是否造成不可接受的 score 偏差。

---

## 6.4 Step B1-2：精确最大值预扫描

与 B0 完全使用同一个逻辑：

\[
M_i^h=
LeakyReLU(L_i^h+\max_j R_j^h).
\]

不使用 running-max online rescale。

理由：

- 当前目标是搭 baseline，不是验证新 online softmax；
- exact max pre-scan 数值清楚；
- 使 B0 / B1 attention normalization 机制尽量一致；
- 避免把 online rescale 开销混入“是否使用 TFS”的比较。

---

## 6.5 Step B1-3：加权 TFS AMX 主核

### 调度单位

先按原 TFS 做 destination degree sort：

```text
perm[sorted_position] = original_row
```

然后每个 worker 处理 row panel；核心 row tile：

```text
TR = 16 destination rows
```

### head 策略

B1 是“忠实原 TFS 思想”的 baseline，建议**每个 head 独立执行 TFS**：

```text
for head h:
    process degree-sorted destination rows
```

不要在 B1 中提前加入我们未来的 head-as-row 设计，否则 baseline 和 ours 会混淆。

### 每个 neighbor step

对于当前 16 行：

1. 找到每行第 s 个 neighbor `j_n`；
2. 计算：

\[
s_n=LeakyReLU(L_{i_n}^h+R_{j_n}^h)
\]

3. 利用已经预扫描得到的 `M[i_n,h]`：

\[
p_n=\exp(s_n-M_{i_n}^h)
\]

4. FP32 denominator：

\[
ell_n += p_n
\]

5. gather `X_bf16[j_n,:]`；
6. 将 BF16 expand 为 FP32，乘 `p_n`；
7. 使用 RNE 转回 BF16，形成：

\[
Hbuf[n,:]=BF16(p_n\cdot FP32(X_{j_n}^{BF16})).
\]

8. Hbuf 与 `W_h_bf16` 进入 AMX：

\[
C_{tile}\leftarrow C_{tile}+Hbuf\times W_h.
\]

### 为什么用未归一化 p，而不是 alpha

`p = exp(score-max)`：

- 不需要每条边做除法；
- p 通常在 `[0,1]`；
- denominator 单独 FP32 累积；
- 最后统一除：

\[
Y=C/\ell.
\]

---

## 6.6 B1 的 AMX tile 规划

GAT 每个 head 的输出 d 比原 TFS 的 128 更小，因此这条 baseline 比原 GCN TFS 更容易让全部输出列同时驻留。

### d = 32

候选：

- T0：C[:,0:16] FP32
- T1：C[:,16:32] FP32
- T2：A/Hbuf BF16
- T3：W block 0 BF16
- T4：W block 1 BF16
- T5-T7：备用 / 双缓冲

因此一次 neighbor scan 可以覆盖完整 d=32，不需要像原 `K_OUT=128` 那样做多个 output panel pass。

### d = 40

候选：

- T0/T1：32 列
- T2：tail 8 列（若目标 tile shape 合法）
- 其余放 A / W operands

若 tail tile 实现复杂，允许先 pad 到 48 列，但：

- 只写回前 40 列；
- 报告 executed padding；
- 后续再比较专用 tail kernel。

---

## 6.7 Step B1-4：normalize + activation

TMM 输出存 FP32。

对每个 row：

\[
Y_i^h = C_i^h / \ell_i^h.
\]

然后 hidden layer ELU。

直接写最终：

```text
output[node][head][d]
```

不要再建立独立 concat buffer。

---

## 6.8 B1 的一个关键精度问题

B0：

\[
X_{BF16}W_{BF16}\rightarrow Z_{FP32},
\quad p\cdot Z_{FP32}
\]

B1：

\[
p\cdot X_{BF16}
\rightarrow BF16
\rightarrow W_{BF16}
\]

所以 B1 多了一个：

\[
\boxed{pX\rightarrow BF16}
\]

的量化点。

这不是 bug，而是使用当前 AMX-BF16 将动态 edge weight 融入原 TFS 数据流的真实接口代价。

因此 B1 必须单独报告精度，不能因为 B0 也是 BF16 就假设二者数值完全等价。

---

# 7. 两层 benchmark：先 fixed-p，再 full-GAT

这是整个基线方案最重要的实验设计。

---

## 7.1 Benchmark A：Fixed-p operator benchmark

### 目的

**只回答：TFS 数据流本身值不值得。**

### 输入

由 R0 预先生成并保存：

- CSR；
- `X`；
- `W`；
- `p[E,K]`；
- `ell[N,K]`。

这里 `p=exp(score-max)`，不是归一化 alpha。

### A0：No-TFS operator

```text
X_bf16
  |
  | oneMKL BF16 GEMM
  v
Z_fp32
  |
  | AVX weighted SpMM using fixed p
  v
numerator
  |
  | / ell
  v
Y
```

### A1：TFS operator

```text
X_bf16 + fixed p + W_bf16
  |
  | weighted TFS AMX
  v
numerator FP32
  |
  | / ell
  v
Y
```

### 计时中明确不包含

- L/R；
- score；
- max prescan；
- exp；
- p 生成。

### 价值

若 A1 已经无法在目标 shape 上击败 A0，则说明：

> 原始 TFS 数据流本身在 GAT 当前形状上缺乏优势。

此时不能通过改变 softmax 基线去“救” TFS。

---

## 7.2 Benchmark B：Full-layer GAT benchmark

### B0 Strong Standard GAT

计入：

- conversion；
- XW GEMM；
- L/R；
- max prescan；
- score+exp+weighted aggregation；
- normalize；
- activation。

### B1 TFS-GAT

计入：

- conversion；
- LR GEMM；
- max prescan；
- weighted TFS AMX（内部 score+exp）；
- normalize；
- activation。

### 不计入 steady-state 的静态对象

- graph degree sort；
- W pack；
- `W a`；
- static metadata。

但这些必须有 cold-start 单独数字。

---

## 7.3 Benchmark C：Full-model E2E

三层各自传递自己的输出，不能每层重新喂 reference 的输入。

```text
Layer1 path-specific output
 -> Layer2
 -> Layer3
 -> final output
```

只有这个结果可以称“模型端到端”。

---

# 8. 算子选择总表

| 阶段 | R0 | B0 Standard | B1 TFS | 原因 |
|---|---|---|---|---|
| FP32->BF16 | 不需要 | AVX512 RNE | AVX512 RNE | 明确量化合同 |
| XW dense projection | SGEMM FP32 | oneMKL BF16 GEMM -> FP32 | 不生成完整 Z | B0 用强 AMX dense |
| `W a` | 不重要 | 不需要 | FP32 静态预处理 | B1 省 Z |
| X x `[bL,bR]` | 不需要 | 不需要 | K=8优先 oneMKL BF16 GEMM | 输出宽 16 |
| L/R from Z | FP32 | AVX512 fused dot | 不使用 | skinny reduction |
| max prescan | 经典 | AVX512 CSR | 同一逻辑 | 强、稳定 |
| score + exp | FP32 | AVX512/SVML | AVX512/SVML | 动态不规则 |
| weighted aggregation | FP32 | AVX512 dynamic SpMM | TFS AMX | 核心对比点 |
| normalize | FP32 | AVX512 | AVX512 | 低成本 |
| ELU | FP32 | AVX512/SVML | AVX512/SVML | 避免库调用开销 |
| W packing | 无 | oneMKL static pack可选 | custom AMX/VNNI pack | 静态 |

---

# 9. 为什么“所有能用 AMX 的地方都用 MKL”需要稍微修正

正确原则不是：

> 只要数学上是矩阵乘，就必须使用 AMX。

而是：

> **规则的大型 dense GEMM 优先使用 vendor BF16-AMX 实现；极瘦 GEMM、小 reduction、动态稀疏运算使用实际最快且语义匹配的实现。**

例如：

- `[N,256] x [256,256]`：非常适合 oneMKL BF16 GEMM；
- `[N,32] x [32,2]`：未必适合 AMX，AVX fused dot 可能更快；
- dynamic weighted CSR：oneMKL sparse API没有合适的 BF16 动态 per-head 路径，自定义 AVX 是合理强基线。

所以“强 baseline”优先级高于“强行让每一步出现 AMX 指令”。

---

# 10. 公平性合同

## 10.1 同一输入

每次比较：

- 同一图；
- 同一 X；
- 同一 W/a；
- 同一线程数；
- 同一 NUMA 策略；
- 同一激活；
- 同一 empty-row 语义。

## 10.2 同一预处理政策

若 B0 的 W pack 放计时外，则 B1 的 W pack 也放计时外。

若 degree sort 被视为固定图静态预处理，也必须单独报告时间和内存。

## 10.3 禁止弱化 B0

不得：

- 为 B0 每 head 单独调用小 GEMM，而 B1 使用一次大操作；
- 让 B0 物化 alpha，而 B1 不物化，然后把差全部归因 TFS；
- B0 每次重新 pack W，B1 缓存 W；
- B0 用单线程，B1 用多线程；
- 用 FP32 MKL 与 BF16 TFS 的速度直接作为“纯 TFS 数据流收益”。

## 10.4 允许 precision path 不完全相同，但必须公开

B1 的 `pX -> BF16` 是真实硬件约束，不能为了表面公平隐藏。

速度和精度要两张表回答两个问题：

1. 这条数据流快不快？
2. 这条数据流的误差是否可接受？

---

# 11. 线程、NUMA 与库线程

第一轮建议固定**单 socket 物理核**，避免跨 NUMA 先污染结果。

建议环境：

```bash
OMP_NUM_THREADS=<physical cores in one socket>
OMP_PROC_BIND=close
OMP_PLACES=cores
MKL_DYNAMIC=FALSE
```

## 11.1 B0

- dense GEMM 阶段：oneMKL 使用全部固定线程；
- sparse 阶段：自定义 OpenMP 使用相同线程数；
- 两阶段顺序执行，不嵌套。

## 11.2 B1

TFS kernel 使用一个 persistent OpenMP region：

- 每 worker 请求/初始化 AMX tile 状态；
- 避免每个 head 反复创建并行区；
- head 之间可以 `omp for` + barrier；
- 核心 TFS 内不要再调用多线程 MKL。

---

# 12. 计时合同

必须有四种计时口径。

## 12.1 Microkernel

只测：

- BF16 GEMM；
- L/R dot；
- max prescan；
- weighted SpMM；
- weighted TFS AMX；
- conversion。

## 12.2 Fixed-p operator

测第 7.1 节。

## 12.3 Full layer

从层输入准备完成开始，到该层 output 完成为止。

## 12.4 Full model

三层全部执行，比较放在 stop timer 之后。

### 绝对禁止

计时区间中出现：

- `compare()`；
- printf；
- reference copy；
- debug stats vector push_back；
- correctness scan。

---

# 13. Correctness gate

## 13.1 每个阶段都单独比较

至少输出：

- Z max-abs / rel-L2；
- L/R max-abs；
- row max；
- denominator；
- fixed-p operator output；
- layer output；
- model output。

## 13.2 必须检查 finite

任何路径中：

```text
NaN / Inf => FAIL
```

不能用旧代码的 `if (error > threshold)` 形式让 NaN 漏过。

## 13.3 误差阈值

不要现在在代码里写死“0.02就是正确”。

先记录完整分布：

- max abs
- mean abs
- relative L2
- RMSE
- P50/P90/P99 abs error
- 最终任务指标

然后根据真实 checkpoint 在开发集冻结阈值。

---

# 14. 必须覆盖的 shape / 边界测试

## 14.1 Degree

```text
0, 1, 2, 15, 16, 17, 31, 32, 33, 63, 64, 65, high-degree
```

## 14.2 Heads

```text
1, 2, 4, 8
```

至少保证当前 `8,8,1`。

## 14.3 D / d

当前必须：

```text
D=128,d=32
D=256,d=32
D=256,d=40
```

另外测试非整齐 tail。

## 14.4 Score

- equal scores
- large positive / negative
- mixed Leaky branches
- late peak
- large common offset

---

# 15. 建议代码目录

```text
baseline/
├── include/
│   ├── gat_model.hpp
│   ├── graph.hpp
│   ├── precision.hpp
│   ├── timing.hpp
│   └── contracts.hpp
├── src/
│   ├── ref_fp32.cpp
│   ├── baseline_standard.cpp
│   ├── baseline_tfs.cpp
│   ├── kernels/
│   │   ├── bf16_convert_avx512.cpp
│   │   ├── lr_from_z_avx512.cpp
│   │   ├── max_prescan_avx512.cpp
│   │   ├── weighted_spmm_avx512.cpp
│   │   ├── elu_avx512.cpp
│   │   ├── tfs_weight_pack.cpp
│   │   └── tfs_weighted_amx.cpp
│   └── common/
│       ├── graph_loader.cpp
│       ├── model_loader.cpp
│       └── validation.cpp
├── bench/
│   ├── bench_micro.cpp
│   ├── bench_fixed_p.cpp
│   ├── bench_layer.cpp
│   └── bench_model.cpp
├── tests/
│   ├── test_small_graph.cpp
│   ├── test_tail_shapes.cpp
│   ├── test_nonfinite.cpp
│   └── test_precision.cpp
└── scripts/
    ├── build.sh
    ├── run_correctness.sh
    └── run_benchmark.sh
```

---

# 16. 建议命令行接口

```bash
./gat_baseline \
  --graph xxx.gatbin \
  --weights checkpoint.bin \
  --path ref_fp32|standard_bf16|tfs_bf16 \
  --mode correctness|profile|benchmark \
  --threads 32 \
  --block 32 \
  --panel-r 64
```

Fixed-p：

```bash
./gat_fixed_p_bench \
  --graph xxx.gatbin \
  --fixture fixed_p.bin \
  --path standard|tfs \
  --threads 32
```

---

# 17. 日志字段

正式 benchmark 每次至少打印一行机器可解析 JSON/CSV：

```text
path
mode
layer
N
E
D
K
d
threads
precision
projection_ms
lr_ms
max_prescan_ms
score_exp_aggregate_ms
normalize_ms
activation_ms
convert_ms
total_ms
workspace_bytes
static_preprocess_ms
max_abs
relative_l2
finite
```

TFS 追加：

```text
degree_sort_ms
panel_R
tile_rows
p_times_x_convert_ms
gather_ms
amx_compute_ms
padding_executed_flops
```

---

# 18. 实施顺序：AI 必须按这个顺序做

## Phase 1：正确性基础

1. 独立 R0；
2. 修好 finite / size / timing gate；
3. 小图逐中间量比较；
4. 不做 AMX 性能结论。

验收：R0 可重复，所有负测试正确失败。

## Phase 2：B0 Strong Standard

1. BF16 RNE conversion；
2. oneMKL BF16 projection；
3. L/R AVX；
4. exact max prescan；
5. AVX softmax + weighted SpMM；
6. full-layer correctness；
7. microbenchmark。

验收：B0 是一个独立、优化的标准 GAT baseline，不依赖 TFS 代码。

## Phase 3：Fixed-p TFS

1. 从 R0 导出 p / ell fixture；
2. 实现 weighted TFS AMX；
3. 先只比较 fixed-p；
4. 检查 `pX -> BF16` 误差；
5. 检查 degree sort / tail / empty row。

验收：可以明确回答“只看 TFS 数据流，快还是慢”。

## Phase 4：Full TFS-GAT

1. precompute `bL/bR`；
2. LR 路径；
3. max prescan；
4. score+exp 接入 TFS neighbor loop；
5. full-layer；
6. full-model。

验收：B1 不再依赖预生成 p。

## Phase 5：公平性复核

1. 同一线程/NUMA；
2. 同一静态缓存政策；
3. W packing 都在相同口径；
4. compare 全在 timer 外；
5. 路径独立进程或 AB/BA；
6. 至少多次重复，报告中位数/P95。

只有此后才能开始“ours”。

---

# 19. Baseline 完成的硬验收条件

在进入新方法之前，必须同时满足：

### G0：语义

- R0 与独立框架 / 小图 oracle 一致；
- graph preprocessing 完全锁定。

### G1：B0 强度

- dense projection 确认走 vendor BF16 高性能路径；
- 不物化 score/alpha；
- weighted SpMM 已向量化、多线程；
- 没有明显 debug / allocation 热路径。

### G2：B1 真实性

- 确实执行 TMM + `TDPBF16PS`；
- partial output 驻留在 TMM；
- 不是 `Utile + cblas_sgemm` 冒充 TFS；
- 使用 degree-sorted destination panels；
- dynamic p 真正参与每条边。

### G3：正确性

- 所有输出 finite；
- fixed-p 与 full-layer 都有误差报告；
- 真实 checkpoint 最终任务指标通过预设门槛。

### G4：测量

- timing 不包含 compare/print；
- speed build 不维护重型 Stats；
- warmup / cold / steady 定义明确；
- 线程与 NUMA 固定。

---

# 20. 不足与风险：现在就要承认

## 20.1 TFS-GAT 可能天然计算更多

忠实原 TFS：

\[
\sum_j p_{ij}(X_jW)
\]

会对同一 source projection 在不同 destination edge 上重复计算。

它的主要乘法工作量接近：

\[
O(EKDd)
\]

而 Standard TF 是：

\[
O(NKDd)+O(EKd).
\]

因此 B1 是否能赢不能预设。

## 20.2 `pX -> BF16` 是额外精度接口

这可能使 B1 精度比 B0 差，必须实测。

## 20.3 B1 的 attention prep 与 B0 不同

B0 最自然是：

```text
Z -> L/R
```

B1 最自然是：

```text
X -> bL/bR -> L/R
```

两者代数等价，但浮点/量化路径不同。

因此建议增加一个**matched-attention control**（不是第三个主 baseline）：

- 使用 B1 的 `X -> bL/bR -> L/R`；
- 后半段仍走 Standard TF。

它专门用于回答：

> 性能/误差差异有多少来自 attention-prep，而不是 TFS？

## 20.4 不能强迫所有小 GEMM 使用 AMX

如果 `N x 32 x 2` 或 `N x 256 x 2` 的 SGEMM/AVX 明显更快，就应使用更快的实现。

论文需要的是**强 baseline**，不是“AMX 指令数量最多的 baseline”。

---

# 21. 最后建议的结果表结构

## 表 A：Correctness

| Path | Precision | Z err | LR err | Layer1 err | Layer2 err | E2E err | Task metric |
|---|---|---:|---:|---:|---:|---:|---:|
| R0 | FP32 | - | - | - | - | - | reference |
| B0 | BF16-AMX + FP32 sparse | | | | | | |
| B1 | TFS BF16-AMX | | | | | | |

## 表 B：Fixed-p Operator

| Layer shape | Standard TF | TFS | Speedup | TFS extra quantization err |
|---|---:|---:|---:|---:|
| 128 -> 8x32 | | | | |
| 256 -> 8x32 | | | | |
| 256 -> 1x40 | | | | |

## 表 C：Full Layer

| Layer | B0 total | B1 total | Projection/LR | Attention | Aggregate/TFS | Convert |
|---|---:|---:|---:|---:|---:|---:|

## 表 D：Full Model

| Path | Steady-state | Cold-start | Peak workspace | Accuracy |
|---|---:|---:|---:|---:|

---

# 22. 给后续 AI 的最简任务定义

> 不要实现新方法。先建立一个可信、强、可复现的 Vanilla GAT CPU baseline suite。  
> R0 为 FP32 correctness oracle。B0 为不使用 TFS 的强 Transform-First GAT：dense projection 用 oneMKL BF16 GEMM（目标平台验证 AMX），attention 使用 exact-max prescan + AVX-512 第二遍 softmax/weighted aggregation。B1 为忠实原 TFS 思想的 GAT baseline：使用 `bL/bR` 避免完整 Z，exact-max prescan 后，在 16 个 destination rows 上按 degree-sorted TFS 调度，将每条边的 `p*X` 转 BF16，再用 `TDPBF16PS` 与该 head 的 W 做融合累加，最后 FP32 normalize/activation。  
> 先做 fixed-p operator benchmark 隔离 TFS 数据流，再做 full-layer 和 full-model。所有 compare/print/debug 均移出计时；静态 W pack/degree sort 的 cold-start 与 steady-state 分开报告。任何 AMX 声明必须通过运行时/指令证据验证。所有 BF16 路径必须与 FP32 R0 做分阶段精度检查。

---

# 23. 参考依据

## 项目源码（固定提交）

Commit：`7fdde396d03af1ae32692c696059378dbf205f7f`

重点文件：

- `GAT/code/code/mkl_baseline.cpp`
- `GAT/code/code/amx_tfs_v3.cpp`
- `GAT/code/code/gcn_e2e_v3.cpp`
- `implementation/src/gat_online.cpp`
- `implementation/src/gat_tfs_online.cpp`
- `GAT/TFS_Tile_Aware_SpMM_GeMM_Fusion_AMX_中文版.md`

## Intel 官方接口核对

- oneMKL `mkl_sparse_?_mm`：公共 CPU 接口为 s/d/c/z；当前 baseline 不应假设存在透明 BF16 sparse-mm。
- oneMKL `cblas_gemm_bf16bf16f32`：BF16 x BF16 -> FP32 dense GEMM。
- oneMKL `cblas_gemm_bf16bf16f32_pack/compute`：允许固定权重预打包。
- oneMKL runtime dispatch / verbose：用于记录实际库调用与平台分派。

最终论文或实验记录中应固定 oneAPI/oneMKL 版本，不引用“MKL 一定自动使用 AMX”作为无证据事实。
