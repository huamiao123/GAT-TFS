# GAT–TFS 方法审计、优化假设与实验设计

日期：2026-10-01。

本文保存设计阶段的方法审计和实验依据。文中旧服务器时间来自作业 `10810186`；JavaScript/V8 数值计算和访问量计算属于数值控制或成本模型，不属于硬件实验。后续已完成新内核与完整组专用化的两轮硬件作业 `10849337/10849393`，最新数据见 [实现与真实图结果](JOINT_ONLINE_RESULTS_20261001.md)。文中“待测”描述保留为原设计阶段的状态，以新结果报告为准。

## 1. 研究问题仍然成立，但需要更准确地定义收益与代价

三层模型维持 `Din → 8×32 → 8×32 → 1×C`，前两层各 head 独立，输出 concatenate。每个 head 的原始 Vanilla GAT 为：

\[
Z^h=HW^h,\quad
e_{ij}^h=\operatorname{LeakyReLU}(L_i^h+R_j^h),\quad
O^h=A_\alpha^h Z^h.
\]

aggregate-first 数据流使用：

\[
b_L^h=W^ha_L^h,\quad b_R^h=W^ha_R^h,
\]

\[
L^h=Hb_L^h,\quad R^h=Hb_R^h,\quad
O^h=(A_\alpha^hH)W^h.
\]

这在实数运算中与原定义等价。各 head 的 attention 不同，所以仍然需要独立的 `m/l/U` 和独立的 weighted aggregation。共享邻接和源特征读取，可以在保持这一语义的前提下改善系统成本。

需要回答的问题是：**局部中间状态、跨 head 源特征复用和适合矩阵硬件的 PH 计算，能否补偿 aggregate-first 的宽稀疏计算开销，并超过优化后的纯 GAT？**

比较对象要明确：

- 相对完整物化 `U[N×K×D]` 的 aggregate-first，局部 U 能避免大量中间矩阵写回。
- 相对纯 GAT transform-first，标准路径主要物化 `Z[N×K×d]`；aggregate-first 增加 `D/d` 倍稀疏特征运算。
- 两种顺序的有效线性投影 FLOP 都是 `2NKDd`。局部执行还可能失去大 GEMM 的效率，因此不能仅凭结合律宣称减少了投影运算。
- 特征的逻辑读取量、cache 流量和 DRAM 流量应分别报告。

## 2. 原 TFS 源码的真实执行顺序

提供的原始 [amx_tfs_v3.cpp](https://github.com/huamiao123/GAT-TFS/blob/5100717/GAT/code/code/amx_tfs_v3.cpp) 使用 degree ascending 的逻辑 row permutation、TR16、动态 row panels、smart zeroing、下一邻居预取和原 row scatter。邻居 step 中 gather H 到局部 Hbuf，然后不断计算 `Hbuf×W`，把输出 partial sums 保持在 TMM，最终才 store。

旧 GAT B1 在该顺序上增加每 head 的动态 attention：每条边准备 BF16 `pH`，再执行 `(pH)W`。它避免完整 U，但把 UW 投影重复到了边上。该路径代码在 [baseline_tfs.cpp](https://github.com/huamiao123/GAT-TFS/blob/5100717/baseline/src/baseline_tfs.cpp)。

当前 local-U 候选改变为：

```text
H[bL,bR] → CSR neighbor blocks → independent m/l/U[D]
       → worker-local U for 16 destination rows
       → one UW per tile/head → divide output by l → hidden ELU
```

它保留 degree permutation、TR16 和动态 panels，但 sparse aggregation 由 AVX-512 完成，邻居扫描期间 U 在 FP32 worker scratch 中。原 local AMX high/low 候选只用 AMX 加速最后的 UW，未用 AMX 计算 PH。其源码和已经完成的说明见 [local_online.cpp](https://github.com/huamiao123/GAT-TFS/blob/5100717/baseline/ours/local_online.cpp)、[作业10810186报告](https://github.com/huamiao123/GAT-TFS/blob/5100717/baseline/docs/LOCAL_ONLINE_RESULTS_20260930.md)。

此前约 251 ms / 8.22 s 的版本对应这个 **FP32 local-U 候选**。它的精度和执行顺序都与旧 B1 BF16 不同，性能比值包含这些差异。

## 3. 已完成的真实图结果：作业10810186

条件：qhcn059 共享 intel 分区、一个节点、16 个同 socket 物理核44–59、显式 NUMA5/6/7 interleave；icpx2024.1.0、oneMKL2023.0 Update2。Workspace 预分配，degree sort、weight preparation、比较和打印在计时外。一轮预热、三轮测量，中位数如下。

图和特征来自官方数据：arxiv `N169343/E2484941/Din128/C40`；products `N2449029/E126167053/Din100/C47`。图无向、去重，每个节点一个 self-loop。参数为 seed11 的随机未训练模型，尚无训练 checkpoint 的任务精度验收。

| 三层路径 | arxiv/ms | products/ms |
|---|---:|---:|
| B0 FP32，纯 GAT | 73.996 | 3908.668 |
| B0 BF16，纯 GAT | 68.437 | 3817.678 |
| B1，原 TFS BF16 edge projection | 425.709 | 15807.121 |
| local Online FP32 | 250.887 | 8221.782 |
| local Online AMX high/low UW | 270.005 | 8534.037 |

local FP32 相对旧 B1 加速 1.697× / 1.923×，相对纯 FP32 B0 则仍慢 3.39× / 2.10×。这证明删除逐边 UW 有作用，但现有 aggregate-first 实现尚未超过纯 GAT。

products 第二层：B0 FP32 总时间 1736.068 ms，其中 projection 146.129 ms，score/exp/weighted kernel 1485.770 ms；local FP32 总时间 4840.405 ms，其中 L/R 18.925 ms，kernel 4799.471 ms。

该 local FP32 kernel 的 1/128 tile 采样 worker 时间组成约为：

| 分组阶段 | products采样占比 |
|---|---:|
| weighted SpMM：H gather、FP32 FMA、U update | 79.87% |
| CSR/source/right、score、max、exp、block sum | 13.97% |
| UW SGEMM | 3.90% |
| rescale 检查和实际 U scaling | 1.17% |
| local U 初始化 | 0.65% |
| normalization/scatter | 0.38% |

这是 sampled worker sums 的组成，不能乘以墙钟总时间得到精确阶段毫秒数。该 profile 含 counters，products 第二层 profile wall 相对 speed median 增加约1.57%；arxiv 增加约7.79%。共享节点波动、采样和 counters 的影响仍需分别控制。

products 第二层 block32 的精确计数为：`42,882,792` 个 row/head neighbor blocks、`4,476,949` 次已有 U rescale、`1,146,098,944` 个 rescaled feature elements。rescales/blocks 为10.44%，但计数比例不是时间占比。当前 local 路径的主要优化对象应为宽特征 sparse aggregation；未来 PH 的 AMX mapping 仍需保留 rescale 统计。

## 4. products第二层的精确逻辑工作量

取 `N=2,449,029`、`E=126,167,053`、`D=256`、`K=8`、`d=32`。以一次乘加记为2 FLOP，计算如下。所有值均为按图和 shape 推导的有效工作模型。

| 工作 | 计算式 | 精确值 |
|---|---|---:|
| B0 稀疏聚合 FLOP | `2KEd` | 64,597,531,136 |
| aggregate-first 稀疏聚合 FLOP | `2KED` | 516,780,249,088 |
| 两者线性投影 FLOP | `2NKDd` | 320,999,129,088 |
| 旧逐边 UW 有效 FLOP | `2KEDd` | 16,536,967,970,816 |
| B0 FP32 source Z 逻辑读取字节 | `4EKd` | 129,195,062,272 |
| 当前逐head FP32 source H逻辑读取字节 | `4EKD` | 1,033,560,498,176 |
| 联合heads后 FP32 source H逻辑读取字节 | `4ED` | 129,195,062,272 |
| 完整 Z 字节 | `4NKd` | 2,507,805,696 |
| 完整多head U字节，若物化 | `4NKD` | 20,062,445,568 |
| TR16全heads局部U字节 | `4×16KD` | 131,072 |
| 单row全heads U字节 | `4KD` | 8,192 |

旧 B1 的实际 AMX 工作还受 destination row padding、输入/输出维度 padding 影响。表中逐边 UW 是有效边工作量。

跨 heads 共享 H 的 load 可以让源代码层的逻辑加载下降最多8倍，但每 head 的加权乘加仍独立，因此 FMA 数不下降。能否提速取决于当前 kernel 的指令、cache、访存延迟和带宽瓶颈，必须通过硬件结果判断。

## 5. 已实现的第一项优化：grouped-head FP32 Online

新增源码为 [joint_online.hpp](../ours/joint_online.hpp)、[joint_online.cpp](../ours/joint_online.cpp)，独立于既有 B0/B1/local。当前实现保留原 block-based Online Softmax：

\[
m'=\max(m,m_B),\quad r=\exp(m-m'),
\]

\[
l'=rl+\sum p_j,\quad U'=rU+\sum p_jH_j,
\quad O=(UW)/l.
\]

循环按 destination row、head group、neighbor block、feature vector、edge、group内head组织。一次 `_mm512` source H load 服务 group 内全部独立 head 的 FMA，group 可设1/2/4/8。只有一个 head 的末层实际分发 G1，少 head 层使用较小 specialization，部分尾组用 active guard。

每个 worker 的 U 为 `[16][heads][padded_D]`。一行的全部 heads 依次完成 block Online；完成 TR16 后，对每个 head 调用一次 SGEMM，A 的 row stride 为 `heads*padded_D`，W 为连续的该 head `D×d`。最后在较窄的 d 维输出上除以 l，并按原节点/head 位置写回。hidden ELU 沿用原定义。

rescale 合并进已有 accumulator load：

```text
load U vector
    → 若已有状态且running max上升，multiply by r
    → grouped independent weighted FMAs
    → store U vector
```

这一改动减少单独遍历 U 的 load/store；实际 rescale 的乘法仍保留。新采样项 `spmm_rescale` 包含融合后的 rescale 和 weighted update，r 的 exp 在 score/state preparation 中。不能为其另造独立的 rescale wall time。

实现将有 inner clocks 的采样模板与无 inner clocks 的速度模板分开；counters 也使用独立模板。采样记录 source-vector loads 和 FMA-vector 数，属于源码访问计数。完整 `blocks/updates/rescales` 在独立运行中收集。

新实现全部 G 都使用 TR16全heads U scratch，第二层为128 KiB/worker。因此 **joint G1→G8** 才是比较干净的 head grouping 控制：scratch 布局、SGEMM stride、调度和 attention 接口相同。旧 local→joint G8 同时改变了 row/head 遍历顺序、U驻留和 SGEMM lda，不能把全部收益归给 source load 复用。

编译器是否把 G 个 accumulator 留在 ZMM 中，需要编译产物和 hardware counters 检查。SGEMM lda 的变化也可能改变 MKL 算法选择与浮点结果。撰写本文时未取得这项新候选的硬件结论。

## 6. 必须补齐的数学控制和数值验收

### 6.1 同一层输入与同一 attention 的控制

首先使用相同 reference 层输入 H、同一 master W/a，以及相同 L/R，比对 weighted aggregation 和 UW；再分别启用 H(Wa) 的 attention reassociation、逐层输出递推。

固定 L/R 的 stable FP64 oracle 应对抽样 destination 精确计算 score、stable exponent、denominator、weighted H 和 UW。抽样需要覆盖度数区间、空行/tails、mixed Leaky branches、late peak 和 near cancellation。每层报告 max_abs、mean_abs、relative L2；一个 scalar oracle 或采样 oracle 不能代替完整三层输出验证。

对 joint G1/G2/G4/G8，记录同输入/同 L/R 下的差异及完整三层差异。profiling/counters 版本也需和未采样路径对照。指纹是一种快速对照，不是没有碰撞的逐位证明。

本轮先前的内存JS/V8 grouped-head模型已做36组配置对照：G1/G2/G4/G8的输出逐位一致，抽样oracle最大relative L2约2.948e-7；D256/K8/G8的模型source loads下降8倍，FMA数不变。该结果支持循环重排的数学可行性，**不是本次C++内核、MKL/AMX后端或服务器加速比的验证**。

### 6.2 旧BF16 attention路径额外改变了什么

旧 B0 BF16 从 `Z=Q(H)Q(W)` 计算 `Za`；旧 B1 使用 `Q(H)Q(Wa)`。量化不满足结合律，故后者额外量化 Wa，不能宣称两者已有相同的 attention。

一个内存 JS 数值反例：`D=1,d=2`，source H 为 `[1,2]`，W为 `[1,1]`，aR为 FP32 `[1,0.004]`，aL为0。H和W均能由BF16精确表示。

| 量 | master FP32和B0 BF16 | 旧B1 reordered BF16 |
|---|---:|---:|
| source1 R | 1.003999948501587 | 1.0078125 |
| source2 R | 2.007999897003174 | 2.015625 |

`Wa=1.003999948501587`，RNE BF16为 `1.0078125`。两个source的R绝对差分别为0.003812551498413086、0.007625102996826172；相对差约0.379736%。L为0时两条边位于Leaky正分支，用FP64 stable exp隔离attention可得：

```text
alpha_B0 = [0.26815571098097185, 0.7318442890190281]
alpha_B1 = [0.26740816618822555, 0.7325918338117745]
max_abs_alpha_difference = 0.0007475447927464129
```

这是数值反例，不是服务器性能结果。

未来 BF16 matched-control 更合理的定义是 `Hq=Q(H), Wq=Q(W)`；用FP32计算 `b=Wq*a`，保持 b 和 a 为FP32，然后用展开的Hq计算 L/R。这样在实数上与 `(Hq Wq)a` 相同，剩余差异是FP32运算顺序。Q还须匹配B0/AMX的有效RNE和subnormal/DAZ/FTZ规则。

### 6.3 纯FP32重排也可能产生大的attention差异

另一个内存数值控制使用相同H/W/a：

```text
H = [[ 1e8, 1],
     [-1e8, 2]]
W = [[1, 1],
     [0, 1]]
aR = [1, -1], L = 0
```

在实数运算中，`HW=[[1e8,1e8+1],[-1e8,-1e8+2]]`。若transform-first把Z存为FP32，两个较小增量被舍掉，Z变成 `[[1e8,1e8],[-1e8,-1e8]]`，因此其R为 `[0,0]`。aggregate-first先计算 `WaR=[0,-1]`，再点乘H，得到R为 `[-1,-2]`。

LeakyReLU slope为0.2时，rounded transform-first attention为 `[0.5,0.5]`，aggregate-first约为 `[0.549834,0.450166]`。后者在该例中更接近精确数学定义。因此，与materialized FP32 Z路径产生attention差异，不能自动解释为aggregate-first算法错误；需要高精度oracle区分参考路径的舍入损失、候选重排误差和实现缺陷。这个反例也不能替代真实模型的精度门槛。它属于内存数值控制，未运行服务器或性能测量。

### 6.4 当前真实图的精度限制

旧10810186的 products local FP32最终 `max_abs=0.006919`、`relative_L2=1.494e-5`，超过旧 `max_abs≤0.003` 门槛。local AMX high/low最终 `max_abs=0.009336`、`relative_L2=2.002e-5`，也未通过。旧 BF16 B0/B1约百分之二的 relative L2尚未完成误差来源隔离。

较小 relative L2、finite输出或小图通过，都不能替代该真实图门槛。通过与量化B0的matched-control，也不能替代 master FP32 与训练checkpoint的任务精度验收。随机未训练参数下的统计应保持这一范围说明。

## 7. 第二项待研究路线：AMX计算PH而非只计算UW

一个 destination 的全部8个 heads共用neighbor block。可以组织：

\[
\Delta U=P[8\times B]\,H[B\times D].
\]

P 的每行来自不同 head 的 score/max/exp，不能共享权重。矩阵乘得到 FP32 ΔU，再以AVX对各head独立更新 `U←rU+ΔU`。在线state放在FP32局部scratch，允许在每个feature block内消费结果。

已有 [head_as_row.hpp](https://github.com/huamiao123/GAT-TFS/blob/5100717/implementation/src/amx_head_row.hpp) 提供合法的 head-as-row AMX原型，可复用其数学和VNNI packing，但其每block动态vector分配、标量packing、tile config/release和密集fine timers需要移出热循环。它不是现成的高效三层内核。

AMX tile 的rows可配置，8个head可以使用8行；不能仅由head数推断必须补16行或固定50%吞吐。在直接head-as-row布局下，每个输出tile至多16个FP32列，覆盖 D256需要16个输出tile，还要保留P/H operands，因此完整U不能按这个布局常驻8个TMM。应明确选择feature blocking、FP32 state或spill/reload，并统计相关字节和时间。规则和指令可查 [Intel Intrinsics Guide](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html)、[Intel SDM及优化手册入口](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)。

### 7.1 BF16 P残差补偿的精度与成本

若H已经按matched-control量化为BF16，可以把FP32 p拆成BF16 high/residual：

\[
\hat p=p_0+p_1\quad\text{或}\quad p_0+p_1+p_2.
\]

分别计算 `p_component×Hq`。由于H只有一个BF16分量，两/三分量P分别需要两/三组乘积。末尾UW保持同一FP32后端和展开的Wq，便于归因。

残差可能为负；subnormal/flush规则、分量加法顺序、AMX累积顺序和near cancellation均需测试。matched-control先保持原FP32 p的denominator，并记录 `sum(p_hat)-sum(p)`；改用量化后的denominator应列为另一个数值控制，不能悄悄改变被比较的目标。

一个内存JS相消控制取 `p=[1,0.9990004897117615]`、`H=[4096,-4096]`。各后端共同使用原FP32 p累加得到的 **FP32 denominator `1.9990005493164062`**。以FP64计算精确numerator，再除以这个共同FP32 denominator，得到matched oracle `2.0480205180658975`。以下输出按FP32 component accumulation和FP32输出舍入模拟；没有混入另一个FP64 denominator的oracle：

| P分量数 | 输出 | 绝对误差 |
|---|---:|---:|
| 1 | 0 | 2.048020518 |
| 2 | 2.047898292541504 | 1.222255244e-4 |
| 3 | 2.048020601272583 | 8.32066855e-8 |

两分量的relative error约5.968e-5。这一控制展示两分量不能保证任意精度门槛，三分量改善了该例。它未模拟真实AMX内部累积，不能据此保证AMX correctness，更不能得到加速比。

### 7.2 tails和packing必须进入成本模型

依据旧products精确block计数，`42,882,792/8=5,360,349` 个row-neighbor blocks。若PH固定reduction32，实际邻居槽位为 `5,360,349×32=171,531,168`，相对 `E126,167,053` 放大1.359555953倍。

配置8head rows时，固定32的PH issued工作模型为：

| P分量数 | 推导FLOP |
|---|---:|
| 1 | 702,591,664,128 |
| 2 | 1,405,183,328,256 |
| 3 | 2,107,774,992,384 |

这些数值是模型推导，尚无新硬件测量。短row、tail可比较AVX fallback或B16/B32配置；不同shape的packing和config成本须包含。Hblock应一次gather/pack后跨heads、跨P分量复用。额外global Q(H) conversion、P packing、VNNI transpose、tile load/store、ΔU merge和最后UW都需要计时。

## 8. Degree Sort与Online max的其他研究边界

Degree Sort在原同步推进16行的kernel中减少inactive行。新的逐行local-U没有这种row padding，其收益变为调度和cache作用。可以在相同joint kernel下比较自然顺序、degree升序、保持tile成员不变但倒序分发panels、度数分桶后保留原节点顺序。应记录worker负载、尾部panel时间和cache/DRAM指标，再决定是否做更复杂的图重排。图预处理成本单列，并说明可复用次数。

Vanilla GAT的additive score和正斜率LeakyReLU还提供一个数学性质：

\[
\arg\max_{j\in N(i)}e_{ij}^h=\arg\max_{j\in N(i)}R_j^h.
\]

每head若按R降序处理邻居，首block就得到最终max，后续无需rescale；但R每层变化，动态排序和多份索引很昂贵。products保存8份uint32邻接约需4.04 GB，还需排序工作，不能隐藏在预处理中。

把邻居按 `R_j≥-L_i` 和负分支拆开，实数上可写成：

\[
O_i=\frac{e^{L_i}\sum_{j\in N(i),R_j\ge -L_i}e^{R_j}H_j+
e^{\beta L_i}\sum_{j\in N(i),R_j<-L_i}e^{\beta R_j}H_j}
{e^{L_i}\sum_{j\in N(i),R_j\ge -L_i}e^{R_j}+
e^{\beta L_i}\sum_{j\in N(i),R_j<-L_i}e^{\beta R_j}}.
\]

全局R前缀和不能代替每个destination的邻域受限和；全局预计算exp还会产生range/underflow问题。该性质可以作为后续理论边界，当前新增joint实现仍只使用已经定义的block Online。单个head内的排序独立于query，也不意味着不同heads的attention相同。

## 9. 一手文献与新颖性范围

- [Brody等：How Attentive are Graph Attention Networks，ICLR2022](https://arxiv.org/html/2105.14491v3) 第3.2节讨论Vanilla GAT的static attention ordering。上述R排序性质有既有理论背景。
- [FlashAttention-2，ICLR2024](https://proceedings.iclr.cc/paper_files/paper/2024/file/98ed250b203d1ac6b24bbcf263e3d4a7-Paper-Conference.pdf) 第3.1.1节保留unnormalized accumulator、最后归一化，减少non-matmul工作。其dense GPU结果不直接证明CPU irregular GAT的收益。
- [FlashAttention-4](https://arxiv.org/html/2603.05451v1) 第3.1.4节讨论max未更新时跳过rescale。近似exp等其它策略需要独立精度验收，当前joint候选未使用。
- [Graphite，ISCA2022作者PDF](https://iacoma.cs.uiuc.edu/iacoma-papers/isca22.pdf) 涉及local aggregation后立即消费、source feature复用以及带宽/延迟/L1 fill buffer分析；也指出预取过量可能降速。其GCN/GraphSAGE设置与当前多head Online GAT不同。
- [PyTorch官方CPU FlashAttention源码](https://github.com/pytorch/pytorch/blob/main/aten/src/ATen/native/cpu/FlashAttentionKernel.cpp) 展示packing、FP32 scratch/rescale、矩阵后端和最终normalization的工程设计。该链接指向main，属于2026-10-01查阅的参考实现；其shape条件不能直接套用当前PH。复现该参考实现时应另固定commit。
- [Fused3S，ICS2025](https://arxiv.org/html/2505.08098v1) 已讨论GPU SDDMM、Online state和SpMM融合。当前研究不能宣称首次把Online Softmax与SpMM融合；该工作没有本任务的post-aggregation UW。
- [CPU GAT优化学位论文，Illinois2024：Accelerating graph attention network inference on CPUs with layer fusion](https://www.ideals.illinois.edu/items/131797)。2026-10-01通过官方条目的 `citation_pdf_url` 找到并只读获取[官方全文PDF](https://www.ideals.illinois.edu/items/131797/bitstreams/437754/data.pdf)，在内存解析42页正文；以下PDF页码从封面计为第1页，印刷页码为正文页脚。
  - §3.1（印刷p16／PDF p21）明确先用MKL GEMM计算全图embeddings，再执行attention/aggregation融合。Algorithm 3.1（印刷p18／PDF p23）的聚合输入是变换后的z；Algorithm 3.3、3.4（印刷p19／PDF p24）先计算 `z=WH`，随后聚合z。因此其描述的是transform-first，聚合各head的 `z^h`，没有本任务的原始H聚合后 `UW`。
  - S0在score阶段将head循环放在edge循环内部，但aggregation仍按head调用邻居聚合；S1先逐head执行GEMM，再执行融合kernel；S2在destination batch内部逐head执行attention和aggregation（Algorithms 3.4–3.6，印刷p19–20／PDF p24–25）。这些方案旨在复用attention系数和z的缓存，不能解释为稀疏聚合中一次原始H向量加载供全部heads更新。Dense projection本身可能复用H，与本任务PH阶段的source复用应区分。
  - §6.2（印刷p33／PDF p38）将initial GEMM与后续操作融合列为未来改进；已描述的算法没有post-aggregation UW。§3.1及Algorithm 3.2（印刷p17–18／PDF p22–23）明确使用fast exponential approximation和批内attention buffer；正文未描述本任务的block Online `m/l/U`运行状态，不能把其融合直接当作已经实现相同的exact Online数据流。
  - 该论文仍是CPU GAT attention/aggregation fusion、head loop placement、按shape生成JIT kernel以减少边界检查、软件prefetch和OpenMP动态调度的先行工作（§3.1–3.2）。全文证据允许区分其已描述的transform-first流程与当前aggregate-first流程，但不排除其它先行工作，也不保证head grouping、去guard或整体组合的新颖性。

现阶段的研究贡献应围绕**量化且公平地解释multi-head aggregate-first的放大、在独立attention下实现source复用、测量PH mapping和Online state成本、给出适用shape与精度边界**。源码存在AMX指令或继承Degree Sort，都不足以构成性能和新颖性结论。

## 10. 下一轮实验顺序和报告要求

1. 编译独立joint内核，先完成共同输入/固定L/R的correctness，再比较attention reassociation以及完整三层。输出误差、profile/counter一致性和空行/shape tails分别记录。
2. 在相同kernel、U布局、UW后端和NUMA策略下比较joint G1/G2/G4/G8，至少覆盖arxiv与products；旧local和B0保留作整体比较。无inner-clock的速度中位数与采样运行分开。
3. 每层报告L/R、kernel、activation、total；采样再分init、score、fused SpMM/rescale、UW、normalization/scatter。记录block/max/rescale计数、source/FMA逻辑计数，并说明不能把worker份额视为wall-time份额。
4. 对编译产物检查ZMM spills，并在硬件可用时采cache misses、DRAM bytes、访存延迟/带宽与worker尾部负载。缺少这些数据时，保持访问模型与实测的区分。
5. 若joint控制证实PH仍是主瓶颈，再实现独立AMX PH后端：先fixed P控制，再Online state merge，随后完整三层。P分量、tails、packing和输出误差必须同时呈现。
6. 任务精度与训练有效性仍需实际checkpoint。当前实现是forward研究；训练时attention对W的依赖、dropout位置/mask、backward和内存生存期都需要单独实现与验证，forward代数等价不能代替完整训练验证。

新硬件结果完成后应写独立结果记录，保留代码、图/参数、编译选项、二进制、规则哈希、线程绑定、NUMA、原始日志和失败记录。本文本身不报告新的加速比。
