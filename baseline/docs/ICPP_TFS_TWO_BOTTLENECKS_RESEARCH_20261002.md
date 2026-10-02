# ICPP TFS 多头 Online GAT：两项瓶颈、数学边界与实验路线

日期：2026-10-02，更新至完整作业10854118和PH控制10854131完成。本文记录实验和可检验的研究推导；最新硬件结果见[完整块投影/PH报告](ICPP_TFS_BLOCK_RESULTS_20261002.md)。

## 1. 当前结论与状态

Layer 2 固定为输入 D=256、K=8 个独立 heads、每 head 输出 d=32。当前必须分别处理两项成本：

1. 聚合原始 H 时，每条边、每 head 需要处理 256 维，而 transform-first 只处理 32 维，产生 **8× 稀疏特征工作量放大**。
2. 原 ICPP 邻居同步执行流把 pH 立即送入 W，每个邻居位置都执行一次投影，产生大量 **重复的边级投影计算**。products Layer 2 的有效特征 FMA 相对标准 transform-first 的投影加聚合约为 **42.887×**，含 TR16 等 padding 的实际 FMA 比值约为 **42.985×**。

这两个比值描述不同执行流，不能直接相乘；它们也不是实测时间比值。

已完成的 head-pair 路径保留原 ICPP 邻居级 AMX 投影，对前两层共享原始 H 和 CSR source 的读取。作业 **10853989** 已完整测量：相对旧 Online，三层速度提高 arxiv 1.255×、products 1.472×，输出与旧 Online 逐位一致；但仍慢于纯 GAT B0 BF16，原 FP32-master 绝对误差门仍然失败。

block-local 路径已把每个邻居的投影改成每个neighbor block一次真AMX投影，并在10854118通过内部控制和完整三层测量。它减少第二项成本，**没有在数学上消除第一项8×工作量**。products L2 TDP少13.8–35.6倍，最佳三层相对原B1仅1.259倍；仍慢于Pair和B0。head-as-row PH原16行与新8行直接向量打包控制已在10854131完成，products双分量相对较快AVX约1.237倍；尚未接完整模型。

本研究保持三层模型 D_in→256→256→C，前两层 8×32、第三层 1×C；不共享 attention、不减 heads、不改 D、不使用近似 softmax、不增加 two-pass softmax 路径。

## 2. 三种计算流及其成本

以下先按实数算术讨论。每个 head 都有自己的 W、aL、aR、L/R、softmax 和输出。W 的当前存储布局为 `[D][K*d]`，只是存储拼接，不代表多个 heads 共用参数。

### 2.1 标准 transform-first

对 head h：

\[
Z^h=HW^h,\qquad O^h=A_\alpha^h Z^h.
\]

特征计算约为：

\[
F_{TF}=NKDd+EKd=Kd(ND+E).
\]

这里不计 attention、exp、normalization、activation 等额外操作。优势是每个 source/head 的投影在整层只计算一次；代价是完整 Z[N,Kd] 的保存和后续读取。当前强基线 B0 FP32/BF16 属于这一类。

### 2.2 Aggregate-first：局部 AH 后 UW

在 attention 完全相同的前提下：

\[
O^h=(A_\alpha^hH)W^h.
\]

attention 不需要完整 Z 时，可先计算：

\[
b_L^h=W^ha_L^h,\quad b_R^h=W^ha_R^h,
\quad L^h=Hb_L^h,\quad R^h=Hb_R^h.
\]

这些是代数等价变换，浮点收缩顺序不同仍需验证。特征计算约为：

\[
F_{AF}=EKD+NKDd.
\]

稀疏部分的放大比为：

\[
\frac{EKD}{EKd}=\frac Dd=8.
\]

每个 head 的 A 不同，因此不能只算一次 AH 给八个 heads 共用。通过 worker-local U 或 tile fusion，可以避免完整 U[N,K,D] 写回，但不会自动减掉 EKD。

### 2.3 原 ICPP TFS 的邻居级融合执行

当前实际继承的微内核对一组 16 个 destinations，按各自 CSR 邻居位置同步扫描。每个有效 row/head/neighbor 生成 p，形成 BF16(pH)，立即使用真实 AMX 与该 head 的 W 相乘，累加到驻留输出 V[d]：

\[
V_i^h\mathrel{+}=p_{ij}^hH_jW^h.
\]

Online 版本在 max 增长时缩放旧 V，继续生成新 block 的 p，并在最后除以 l。它保留的是输出 V[d]，不是完整 D 维累计 U。

有效投影工作约为：

\[
F_{edge}=EKDd.
\]

实际硬件工作还含 TR16 同步最长度、D padding、输出 d padding。相对标准 TF 的投影加聚合：

\[
\frac{F_{edge}}{F_{TF}}
=\frac{DE}{ND+E}.
\]

products 当前图 N=2,449,029、E=126,167,053，平均 CSR 度 E/N≈51.517；Layer 2 的 D=256、K=8、d=32：

\[
F_{TF}=192,798,330,112,
\quad F_{edge}=8,268,483,985,408,
\quad F_{edge}/F_{TF}=42.886699.
\]

实测 counter 的 padded FMA 为 8,287,360,647,168，对同一 F_TF 的比值为 42.984608。

**分母必须明确**：42.9× 的分母是 TF 的投影加稀疏聚合。如果只比较 projection 对 projection，边级投影相对节点级投影的比值是 E/N≈51.517×。这些算术计数不能写成“整层必然慢 42.9×”。

## 3. 什么属于保留原 ICPP TFS

### 原 GCN TFS 源码与计算量对照

用户提出“GCN 同样通过重复投影增加计算，而 GAT 的额外工作大量发生在 AMX 外”后，复核了原始 [amx_tfs_v3.cpp](../../GAT/code/code/amx_tfs_v3.cpp:139) 与 [gcn_e2e_bench.cpp](../../GAT/code/code/gcn_e2e_bench.cpp:103)。真实 loop 为 destination TR16 → output panel → neighbor step → K-block → AMX H_jW；不是先形成整行 AH 再只做一次 W。

因此原 GCN 两阶段特征数学工作为 2ED+2NDF FLOP，邻居级 TFS 的有效执行工作为2EDF；另外还有 TR16 最长度同步和 tail padding。按用户举例的约N=2.45M,E=123.7M,D=F=128，分别约112G和4.05T FLOP，约36.2×。这与当前 GAT L2约42.9×处于同一数量级，不能以“GCN没多算而GAT多算”解释差异。

这套 GCN hot path 没有 edge-values 参数，直接 gather BF16 H_j；GAT 需要动态、各 head 独立的 score/exp/p，并形成 pH staging。原 GCN 还按两个 output panels重复 gather完整 H；因此“规则AMX工作占比高”是合理机制解释，精确占比仍需同机、同口径 profiling，不能从 FLOP 数量或历史 worker 采样比例推导墙钟比例。

原两阶段 GCN 的 AH 是随后只消费一次的 destination intermediate；标准 GAT HW 是 source projection，可以跨 destination edges 复用并将每 head 聚合维度从D降至d。消除这两个中间矩阵的收益和代价不同。当前强GAT B0也使用BF16-AMX dense projection，不能将TFS相对FP32 MKL的收益当成单独fusion收益。

原 gcn_e2e_bench.cpp 的 MKL CSR handle 使用 csr.values，TFS kernel却忽略values；**二者只在values全为1等已确认条件下共享邻接数学**。当前尚未检查原实验输入values，不据此宣称历史结果无效，也不将这个kernel默认视为任意权重或归一化GCN。原BF16转换截高16位而非RNE，当前GAT采用RNE，两种精度口径也需分开。

原中文文档明确 products kernel6.67×对应32threads/D=F128，两层E2E4.09×；当前16core/D256/3layer GAT不能套用。文档Eq4使用的是“有效算术强度”表达，分子为数学工作而非源码全部执行工作，分母也不是完整bytes账本。硬件模型须分别列useful FLOP、issued AMX FLOP和bytes，补齐BF16/FP32宽度、每FMA按2FLOP、padding、output-pass重复gather、packing/staging和缓存层级；不能把该式直接当实际Roofline强度。静态逻辑字节也不等于实际DRAM流量。

本文以源码执行流为准；不能仅根据函数名字或 AMX 指令存在来判断是否融合了原 TFS。

当前原始实现和已经通过控制的 Online/pair 路径保留：

- destination-row CSR，原 graph/self-loop/edge multiplicity；
- DegreeSort 的 ascending row permutation，TR16 destination 批和按原 row ID 回写；
- OpenMP 动态 panel 调度；
- 每 head 独立 attention 和 softmax 状态；
- 原 BF16 W packing、AMX tile 的 kb/ob 消费顺序；
- 邻居扫描期间输出 V 驻留于 TMM；
- running-max 增长时真实 spill、AVX row rescale、reload；
- 最后在 head_dim 输出上 normalization，再进行原 activation/concat 定义。

pair 只改变两独立 heads 的执行交织以及原始 H/source 的共享读取，不把注意力或 W 合并。原有单 head 或较大 head_dim 路径仍回退到原 Online。

block-local 候选继续使用上述 CSR、DegreeSort、调度、W packing、输出 tile 和 rescale 生命周期，但把投影粒度从 edge 改为 neighbor block。应称为**原 ICPP 框架上的 block contraction 扩展**，并明确新量化接口；不能宣称它逐位保持旧邻居级投影。

局部 source-HW cache 则先在有限窗口内计算 Z，再聚合 Z。即使沿用 DegreeSort、AMX packing 和有界缓存，它仍属于**局部 transform-first**；不能把它冒称为原 aggregate-first TFS 或原邻居级执行流的原样实现。

## 4. Block-local U_B→真 AMX UW→驻留 V

### 4.1 数学定义

继续使用已有 block Online。每个 destination/head 维护 m、l、V[d]。对于新 neighbor block B：

\[
m'=\max(m,\max_{j\in B}e_{ij}),\quad r=\exp(m-m'),
\quad p_{ij}=\exp(e_{ij}-m'),
\]

\[
l'=rl+\sum_{j\in B}p_{ij}.
\]

只聚合这个 block 的原始输入：

\[
U_B=\sum_{j\in B}p_{ij}H_j,
\qquad V'=rV+U_BW.
\]

这是实数下的严格等价关系。旧累计 V 接收 r；新鲜、从零开始的 U_B 不接收旧状态 rescale。最后输出 V/l。

U_B 的实现 scratch 是每 worker `[16][paddedD]`，每个 block 清零，随即由 AMX UW 消费。没有完整 U[N,D]、完整 Z、e[E] 或 alpha[E]；没有最后另起一个全图 SGEMM。

### 4.2 减少的工作和留下的工作

相对每条边都投影，它把投影次数减少到 row/head 的 neighbor-block 数。理想情况下，较长邻域的 projection 次数近似减少 B 倍；短行、尾 block 和 TR16 同步 padding 必须计入。

稀疏聚合仍然执行 EKD 个有效 feature FMA，Layer 2 仍是 TF 的 8×。block size 增大并不证明总时间会更短：PH 访问、UB 清零/写读、RNE conversion、缓存容量、block-max/rescale 次数和短行 padding 都会变化。

### 4.3 BF16 量化变化

旧邻居级实现的核心形式是：

\[
\sum_j Q_{BF16}(p_jQ_{BF16}(H_j))Q_{BF16}(W).
\]

block 候选改为：

\[
Q_{BF16}\!\left(\sum_{j\in B}p_jQ_{BF16}(H_j)\right)Q_{BF16}(W),
\]

其中 U_B 的求和使用 FP32 FMA。两者的量化位置和累加顺序不同，不能承诺逐位相同，也不能仅凭“量化次数少了”承诺 master 精度提高。

验收分开进行：

1. FP32 block-UW simulator 对同 FP32 L/R、同 score 定义的 stable FP64 oracle，abs/relative-L2 门均为 1e-5；
2. BF16 H/W、逐 feature/edge FP32 FMA 的 U_B、RNE(U_B)、Intel TDPBF16PS even/odd pair-chain 模拟与实际 AMX 核比较；
3. old/new m、l 严格逐位一致，per-row block/max/rescale 与完整边覆盖一致；
4. profiling/counters 四种组合输出不变；
5. FP32-master 误差独立输出，原绝对误差 .003 门不修改。指令模拟通过不代表 master 门通过，更不代表模型准确率通过。

SVML 的 plain 与 `_z0` 入口曾产生可观测的 1 ULP 差别。checks 的匹配入口仅用于验证已选二进制算术；实际 block 编译产物仍需要确认符号，严格 denominator 门失败时必须找原因，不能静默放宽。

### 4.4 必须呈现的计数与时间

有效 PH FMA 为 E×K×D，issued AVX tail FMA 为 E×K×ceil(D/16)×16。D 的 AMX padding 则为 ceil(D/32)×32，两种 padding 单位不同。

block AMX 调用数依据每个 sorted TR16 tile 的最长度、neighbor block、kb 和 ob 推导。报告同时输出：有效/物理 FMA、active row/head blocks、block projection calls、转换元素、source synchronous steps、rescale events、spill/reload bytes。

wall 时间至少分 conversion、L/R、kernel、normalization、activation、每层和三层 E2E。采样 worker 时间分别记录 score、block max、exp/den、rescale store/vector/reload、PH、UB conversion、AMX load+compute、final store/scatter。

worker sums 不能作为可加总的 wall 份额；logical bytes 不等于 DRAM 流量。AMX 是否弥补 8× 放大必须由实际 PH/UW/packing 和 E2E 测量回答。

## 5. Head-as-row AMX PH：不要求 destination 邻居重叠

### 5.1 矩阵映射

对一个 destination i 的同一 neighbor block，构造：

\[
P_i[h,b]=p_{ij_b}^h\in\mathbb R^{K\times B},
\qquad H_B[b,k]=H_{j_b,k}\in\mathbb R^{B\times D}.
\]

\[
U_{B,i}=P_iH_B\in\mathbb R^{K\times D}.
\]

AMX rows 对应 heads，K 维 reduction 对应 neighbors，输出 columns 对应 features。一个 source H_B 被多个 head rows 共用，而每行 P 完全独立。

这个矩阵映射只用**同一个 destination 的邻居 block**，不需要不同 destinations 具有共同邻居。它与“16 个 destination rows 压缩共同 source columns”的 tile 方式不同。

当前固定 TR16-style helper 设想中，K=8 对应 **8 个有效 head rows、16 个物理 rows**，另外八行补零；必须计入物理 padding。Intel AMX 的 tile rows 可以配置小于 16，8 行是硬件允许的配置，不能把固定 50% 行利用率写成 AMX 不可改变的硬件定律。可配置 8 行是否带来实际收益需要单独控制。

### 5.2 这是减少共享输入访问和使用矩阵单元，不是消除 K 个独立乘积

数学有效 PH FMA 仍然是 K×B×D。它可减少重复 H gather/expand，并用 AMX 完成独立 heads 的 PH，**没有把八份独立 P 合成一份**。

若按固定 16 physical rows，B pad 到 32 的倍数、D pad 到 16 的倍数，则一 pass 的物理工作约为：

\[
16\,B_pD_p.
\]

实际/有效比值包含 16/K、B_p/B、D_p/D，以及 high/low pass 数。必须同时报告 numerator 算术、padding 和有效 throughput。

### 5.3 P packing、FP32 状态和 high/low 两 pass

概率和 denominator 仍在 FP32 Online 中计算。AMX BF16 输入需要额外转换与布局：P 按 heads×neighbors 打包；H_B 作为右操作数采用 BF16 pair/VNNI 布局，并复用到各个 head row。

直接 RNE(P) 引入概率量化。一个待验证的控制是：

\[
P_{hi}=Q_{BF16}(P),\qquad
P_{lo}=Q_{BF16}(P-\operatorname{expand}(P_{hi})),
\]

\[
PH_B\approx P_{hi}H_{BF16}+P_{lo}H_{BF16}.
\]

这里是对 **P 分解的两 pass**。它不同于同时对 U 和 W 分解、计算四个 high/low 乘积的旧 UW 后端。两 pass 仍不能宣称精确 FP32 PH；H 的 BF16 误差和 FP32 累加误差仍存在，signed H 下的 cancellation 会影响相对误差。

helper需要核对概率、H packing、两pass指令顺序和独立head状态。固定B32/K8/D256的真实样本instruction gates和den控制已经通过；仅测首32neighbors，不提供完整邻域/tails的生产覆盖。不能把该micro速度当完整三层速度。

### 5.4 与原 TFS UW 接口仍有实际成本

PH 输出以 head rows 排列，但 UW 的 W^h 是不同矩阵。不能把 K 行 U 与所有 heads 拼接 W 做一次普通 GEMM 后只取对角结果，因为这会计算大量无用的 cross-head 输出。

可研究的集成是：在有限 destination window 内保存/重排 U_B[dest,head,D]，按相同 head 收集 16 个 destinations，随后消费原 head-specific packed W；或者采用较小 group 并显式处理 tails。transition、U 写读、packing 和 V tile 生存期必须全部计时。

D=256 时，完整 FP32 U 的 16 destination×256 矩阵已是 16KB，而 AMX palette 1 总 tile storage 为 8KB，并且还要容纳输入 tiles。因此不能假设全部 U、所有 heads 和 operands 同时驻留 TMM。候选需要 feature blocking/L1或L2 staging；只把最终 V[d] 保持驻留是更清晰的当前接口。

## 6. 静态 GAT 两分支与 common-neighbor support 摘要候选

本节是**数学推导和未来候选**，没有实现或速度结果；当前生产 kernel 继续使用同一条 block Online，不同时展开新的 softmax 方案。

### 6.1 精确两分支分解

记负斜率 beta=.2，e_ij=LeakyReLU(L_i+R_j)，阈值 t_i=-L_i。在一个 head 内：

\[
J_i^+=\{j\in\mathcal N(i):R_j\ge t_i\},
\quad J_i^-=\{j\in\mathcal N(i):R_j<t_i\}.
\]

\[
O_i=
\frac{e^{L_i}\sum_{J_i^+}e^{R_j}H_jW
+e^{\beta L_i}\sum_{J_i^-}e^{\beta R_j}H_jW}
{e^{L_i}\sum_{J_i^+}e^{R_j}
+e^{\beta L_i}\sum_{J_i^-}e^{\beta R_j}}.
\]

若邻域完全落在同一分支，destination 的 e^(slope×L_i) 抵消，attention 只依赖对应 source R。但混合分支的集合依赖 i，不能从全图的两个无条件 source 和直接得到每个 destination 输出。

静态排序只表示每个 head 的 R 顺序与 query 无关。两个 destinations 即使邻域相同，也可能因跨越 LeakyReLU 的阈值而有不同 attention。更不能推出不同 heads 可以共享 attention。

### 6.2 支持模式如何形成可复用摘要

给定一个有界 destination 集合 I，对 union sources 中每个 j 建立支持模式：

\[
M_j=(A_{ij})_{i\in I}.
\]

若图是简单图，这可用 bitmask 表示；若存在重复边，必须保留 multiplicity/count vector，不能无意 deduplicate。self-loop 同样计入支持。

把支持模式相同的 sources 分组为 J_M。组内每个参与 destination 看见同一 source 子集，因而可为每个 head、每种斜率 q∈{1,beta} 按 R 排序，形成邻域受限摘要：

\[
S_{M,q}(t)=\sum_{j\in J_M,R_j<t}e^{qR_j},
\quad T_{M,q}(t)=\sum_{j\in J_M,R_j<t}e^{qR_j}H_j.
\]

对 destination i，仅查询其支持模式中包含 i 的 groups，并在阈值 t_i 处得到两分支的前缀/后缀摘要。这允许把多次同 source 子集的扫描替换为摘要查询，在实数下不改变 Vanilla GAT。

要保留 aggregate-first，摘要 T 的特征维仍是 D，最后使用独立 W^h。若先用 Z^h=HW^h 形成 d 维摘要，那是局部 transform-first 变体，不能改变名称来隐藏数据流。

### 6.3 为什么不能保证它普遍有效

- 支持模式高度分散，或只覆盖单个 destination 时，可复用查询少，排序/摘要成本可能比原扫描高。
- 窗口增大可能提高 source 复用，却增大摘要、索引、临时 H 与 U 工作集；DegreeSort 的度匹配与 source locality 可能冲突。
- 每个 head 的 R、阈值、两分支摘要不同；图支持可共享，attention 摘要不能共用。
- 图静态不意味着 R 静态。每层 H 变化、每次推理输入变化时，R 排序和数值摘要需要重新计算，其时间必须纳入 E2E。
- 全图 prefix 不能替代 destination 的邻域受限 prefix。稳定实现还需要局部 exp 基准和已有 Online 状态的有尺度 merge；不能直接计算可能 overflow/underflow 的 exp(R)。
- 使用 total-minus-prefix 求后缀，尤其 signed H 下可能发生 cancellation。不能以实数恒等式代替数值误差验证。
- 大量前缀 T[D] 与两个斜率可能扩大内存；使用范围摘要/层次 merge 是否更好尚无测量。

因此先记录 support-mask 分布、参与 destination 数、group 大小、同分支比例、M/S 复用比和窗口 working set。没有这些数据，不能声称它能够消除 8× 或保证加速。

### 6.4 模型适用限制

上述推导适用于当前 Vanilla GAT 的标量 LeakyReLU(L+R)、正负两个固定斜率、相同邻域/activation/output 定义。它不能直接套到 GATv2 的先非线性后向量收缩、带一般 edge-dependent attention 的模型、非线性 message 或多个 heads 共享 attention 的模型变体。

本文范围是静态图 forward 推理。训练的 edge dropout、参数/输入 gradient、backward 与摘要生存期需要独立实现。activation 只能放在约定的聚合/投影之后，不能为了矩阵结合律把 ELU/ReLU 移进 H、U 或 W。

## 7. 有界 source-HW cache 的独立定位

设一个窗口中有效 edges 为 M，唯一 sources 为 S。先将窗口中的每个 source/head 投影一次，再按原 attention Online 聚合，可将特征计算变为近似：

\[
SKDd+MKd.
\]

这不需要完整全图 Z，且能够同时减少重复 source 投影和 D-wide 边处理。可是窗口无 source 复用时 S≈M，投影优势会减弱；索引、packing、cache lookup 和 Z 写读也会增加成本。

这条路径是局部 TF，不是 aggregate-first。它应作为独立候选或控制，保留原 ICPP/Online 作为明确对照。前后 BF16 舍入不同，需要新 master gate；不能继承 pair 的逐位证明。

## 8. 已核实的一手文献与研究重合边界

以下链接此前已经联网查阅。本文写作阶段没有重新联网。内容定位基于论文/官方全文，而非二手综述；没有对未描述的实现作否定性新颖性保证。

### 8.1 Vanilla GAT 静态排序

[How Attentive are Graph Attention Networks?，ICLR 2022](https://arxiv.org/pdf/2105.14491)，§3.2、Theorem 1，PDF pp4–5，说明 Vanilla GAT 的 ranking 与 query 无关；multi-head 段说明结论对各 head 分别成立。本文两分支/support 摘要是基于其公式和本工程语义的推导，不是该论文已经测量的 CPU AMX 算法。

### 8.2 source-target 分离模型已经有人提出，但改变模型不被本任务接受

[Simplifying Graph Attention Networks with Source-Target Separation，ECAI 2020](https://ecai2020.eu/papers/1617_paper.pdf)，§3.2、Eq15–16，PDF p4，选择 exp(e_ij)=sum_m f_m(i)g_m(j) 并构造 SepGAT。该工作研究 implicit attention 与跨 heads batching，但其注意力函数不同于当前 Vanilla GAT；不能把它直接替换进本模型。

### 8.3 CPU 局部 aggregation-update fusion 已有先行工作

[Graphite，ISCA 2022](https://iacoma.cs.uiuc.edu/iacoma-papers/isca22.pdf)，§4.2、Algorithm 2，PDF pp5–6，实现局部 aggregation 后立即 update，让 intermediate 留在缓存中。它研究 GCN/GraphSAGE 等 CPU 执行；局部 U 消费、cache locality 和 compute-memory overlap 不应分别宣称首创。

### 8.4 CPU GAT fusion 和 head loop placement 已有先行工作

[Illinois 2024 thesis：Accelerating graph attention network inference on CPUs with layer fusion](https://www.ideals.illinois.edu/items/131797)，[官方全文](https://www.ideals.illinois.edu/items/131797/bitstreams/437754/data.pdf)。此前 2026-10-01 已读全文；今天 web 工具无法重新获取该 PDF。

§3.1（印刷p16/PDF p21）先做全图 MKL GEMM；Algorithms 3.1–3.6（印刷pp18–20/PDF pp23–25）对 transformed z 融合 attention/aggregation，比较 head loop placement。§6.2（印刷p33/PDF p38）将 initial GEMM fusion 列为未来改进。其已描述的流程不是本任务原始 H 的 Online aggregate-first，也未描述本工程 block m/l/V；但 head grouping、JIT、prefetch 和 CPU GAT fusion 不能泛称新颖。

### 8.5 source window、局部索引与 Online tensor-core sparse fusion 已有先行工作

[Fused3S，ICS 2025](https://hpcrl.github.io/ICS2025-webpage/program/Proceedings_ICS25/ics25-70.pdf)，§3.1–3.2、Algorithm 1，PDF pp3–5：row windows 内删除全零 columns，维护 compact source mapping/bitmap，并使用 Online m/l/O、tensor-core SpMM 和 output rescale。平台为 GPU，输入 value V 已给定；没有当前 post-aggregation UW。row-window/source compaction/Online sparse fusion 单项已经存在。

### 8.6 GAT 的 reuse 与 aggregate-first 顺序选择已经有人研究

[GRANII，CGO 2026](https://charithmendis.com/assets/pdf/26-cgo-granii.pdf)，§III-B、Eq4–6，PDF p4，比较复用 Theta=HW 与额外执行 (alpha H)W，并使用输入相关选择。attention 阶段仍使用 Theta，区别于当前 H(Wa) contracted attention；论文不描述本工程 AMX block-local Online 内核。矩阵重关联和输入相关选择不能单独当作新贡献。

### 8.7 向量/矩阵混合与稀疏密度选择已经有人研究

[Dense Dynamic Blocks，ICS 2022](https://iacoma.cs.uiuc.edu/iacoma-papers/ics22_2.pdf)，§3、DDB-MM/DDB-HYB/SpMM-OPT，在 POWER10 上研究 vector/matrix 组合、zero padding、有效 throughput 和选择策略。它不等于当前 AMX GAT，但说明矩阵单元峰值并不能保证稀疏收益；不能宣称首次依据密度选择向量/矩阵执行。

### 8.8 AMX 约束与原 TFS 身份

[Intel 官方 AMX intrinsics 文档](https://www.intel.com/content/www/us/en/developer/articles/code-sample/advanced-matrix-extensions-intrinsics-functions.html)，Tile and TMUL architecture，说明 palette 1 的 8KB/8 tiles、最多16 rows×64 bytes及可配置的 tile 尺寸。本文的 rows/padding、U staging 和 V 驻留讨论以这些约束为基础。

原 TFS 的官方 [ICPP 2026 schedule](https://icpp2026.github.io/schedule/) Session 7A 列出 *TFS: Tile-Aware SpMM–GeMM Fusion for Accelerating GNN Inference on Intel AMX*；[出版 DOI](https://doi.org/10.1145/3832810.3832855)。实际复用边界以本仓库 [baseline_tfs.cpp](../src/baseline_tfs.cpp)、[icpp_online.cpp](../tfs_online/icpp_online.cpp) 的执行流为准。

### 8.9 当前可以研究什么

可检验的研究点是：在保留独立多头 Vanilla GAT/Online、原 ICPP DegreeSort/AMX 输出生命周期的条件下，量化两项成本，比较 block contraction、head-as-row PH 和 source/support 复用的适用区域，同时给出精度与搬运边界。

没有找到能据此确认“相同完整 CPU AMX 独立多头 Online TFS 方法已经实现”的证据，但也没有足够证据保证新颖性，更没有普适加速证明。需要把完整系统、具体机制和已有单项优化分开比较。

## 9. 已完成结果：作业 10853989

证据：[summary.tsv](../runs/icpp-pair-10853989/summary.tsv)、[checks.tsv](../runs/icpp-pair-10853989/checks.tsv)、[online_stats.tsv](../runs/icpp-pair-10853989/online_stats.tsv)、[arxiv.log](../runs/icpp-pair-10853989/arxiv.log)、[products.log](../runs/icpp-pair-10853989/products.log)。保存了 source/rule/binary/data hashes、manifest、pinning、accounting 和 frozen source snapshot。

同一作业/节点/16 physical cores，1 warmup、3 次交替计时，中位数，seed11 未训练参数；workspace 预分配和固定准备阶段按 benchmark 约定处理。三层模型、graph、输入和 timing boundary 一致。结果为 exploratory，不能作为正式独享节点或任务准确率结论。

### 9.1 三层 E2E

| 路径 | arxiv，ms | products，s |
|---|---:|---:|
| B0 FP32 | 73.486 | 3.826526 |
| B0 BF16 | 67.191 | 3.723189 |
| 原 B1 TFS PREMAX | 435.465 | 15.800130 |
| ICPP TFS Online B32 | 411.819 | 14.751956 |
| ICPP TFS Pair B32 | 328.160 | 10.023882 |

Pair/Online 的加速比为 1.254971× / 1.471680×；Pair/原 B1 为 1.326991× / 1.576249×。但 Pair 仍分别约为 B0 BF16 时间的 4.884× / 2.692×。

前两层 K8,d32 的 raw-H/source 逻辑读取单位减少一半，独立 pH conversion 和 TDP 工作不变；第三层单 head 走 fallback，不能把全三层的总 DRAM 读取写成“减半”。本轮没有证明实际 DRAM 字节恰好减半。

### 9.2 第二层与输出状态

arxiv 第二层 Online kernel 256.091ms、Pair kernel 210.166ms；products 对应 9256.681ms、6239.236ms。整个三层的收益还包括第一层，并非只来自第二层。

products 第二层 Pair/Online 完全相同的 exact counters 包括：

- row/head blocks：42,882,792；max updates 含 initial：24,072,096；rescales：4,479,864；
- rescales/blocks：约10.4468%；
- logical rescaled features：143,355,648；
- TR16 rescale events：1,203,201；spill/reload 分别2,464,155,648 bytes；
- AMX calls：1,011,640,704；synchronous head-neighbor steps：63,227,544；
- executed padded FMA：8,287,360,647,168。

Pair/Online 的三层 own-output、denominator、maximum 已逐位一致，说明该改动保持旧 Online 算术；这不等于旧路径相对 master 的精度已经合格。

### 9.3 原 master 门仍失败

| 图 | Pair 最终层 max_abs_error | mean_abs_error | relative_L2_error | 原 abs .003 门 |
|---|---:|---:|---:|---|
| arxiv | 0.00358102470636 | 0.0005077876489 | 0.00317680528182 | FAIL |
| products | 6.01900291443 | 0.0135793652594 | 0.015091784138 | FAIL |

products Pair 第二层 max_abs 为25.6116943359，详细各层和其它路径见 checks.tsv。B0 BF16 自身也不代表自动通过 master 门；比较速度不提供任务准确率证书。当前参数未训练，labels/split/checkpoint accuracy 未验收。

## 10. 本轮硬件完成状态与下一步验收

本文最初建立时各候选未测；后续已完成block编译/控制/完整三层10854118及PH布局10854131。结果见[块投影报告](ICPP_TFS_BLOCK_RESULTS_20261002.md)。失败10854102及修复过程保留；没有放宽master门。

block-local独立FP32公式、量化模拟和Online max/den控制已通过；相同arxiv/products、B0/B1/Online/Pair、block16/32/64完整三层计时、projection/PH/conversion/AMX/rescale counters和profile已保存。完整模型FP32 master门仍失败。

第二项PH控制已比较8有效/16配置与8有效/8配置行、high/low两pass、packing/矩阵计算/输出误差。系统样本固定首B32，不宣称尾邻居验收。两份AVX控制有噪声，报告较快基线；尚无完整三层PH→UW集成，不能宣布解决总体瓶颈。

source/support 摘要先做图结构与真实 attention 统计，仅作为未测推导；局部 TF cache 维持单独路径身份。当前不实现近似 softmax，不共享 heads，不通过改模型来规避 D/d 放大。

最终必须分别回答：**减少的到底是 H 读取、有效数学 FMA、物理零填充 FMA、投影次数，还是仅局部计时？** 以及这些变化在固定精度门下是否抵消了新增 packing、staging、rescale 和 attention 成本。只有完整证据允许宣布优化成功。
