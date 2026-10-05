# 从 TFS 的执行冗余到投影放置：期刊扩展研究记录

## 研究范围与证据状态

范围是静态推理阶段稀疏聚合–稠密投影，不涉及训练、动态图，也不局限于 GAT。当前受控算子为单位边权的两层 128→128→128；H/W 按原源码生成，尚不能称为训练好模型的分类实验。本记录先给出可证伪假设，实际结论以 PROJECTION_WINDOW_RESULTS_20261004.md 和逐次原始记录为准。

源码约束很关键：原 TFS 的热路径不使用 CSR values。因此非单位边权图不能直接进入“原 TFS 与 MKL 等价”表。推广到归一化 GCN、一般加权 SpMM，需要另建双方语义一致的加权算子比较；不能将单位边权的通过检查推断为所有 GCN 已得到验证。

## 应保留的 TFS 机制

本扩展保留升序 DegreeSort 的逻辑排列、16 行 destination tile、R64 动态调度、原 BF16 权重打包、AMX 矩阵乘和局部 sparse→dense 消费。局部 FP32 partial 立即由 AMX 消费，未创建完整 N×D 中间矩阵。

原源码逐邻居做投影，当前修改版逐邻居范围归约后做投影。这是需要明确报告的执行变化，不能用相同数学 FLOP 描述两者真实工作，也不能说多余计算已经全部消失。

## 真实执行工作模型

对 DegreeSort 后的 tile t，令其最大度为 m_t、真实边数为 e_t，TR=16。D=F=128 时一次完整 tile 投影包含 32 条 TDPBF16PS，每条完成 2×16×16×32=16384 FLOP。

- 原 TFS：TDP 数为 32Σ_t m_t；还在两个输出 panel 中重复 gather，同一层源特征访问请求约为 2E×256 bytes。
- 有限投影范围 M：FAST 为 32Σ_t ceil(m_t/M) 条；ACCURATE 再乘 2。
- FULL：非空 tile 每层投影一次，但仍须完成 E×128 次 FP32 加法以及 BF16 解码。
- 16 行 padding 由 16Σ_t m_t 描述；有效度槽占用率为 E/(16Σ_t m_t)。它与真实边数 E 不能互换。

单位边权的新归约内核只做加法。一般加权 SpMM 的 2ED 数学 FLOP 和实际单位归约的 ED 次加法也应区分。

这个模型预测：M 越大，AMX 工作越少；但 source load/decode/FP32 reduction 不随投影次数一同减少。若总时间逐渐停滞，应查这些阶段或调度关键路径，不能由 TDP 数下降直接推断加速。

## 为什么解耦 S 与 M

原有 B 参数同时控制两件事：跨 destination 的访问交替，以及重新量化/投影的范围。当前实验固定 M，改变 S；每个 destination 的 CSR FP32 加法顺序、量化点和 AMX 指令序列保持一致，并要求逐位相同。

小 S 会缩短不同 destination 间的访问交替距离，但引入局部 FP32 state 读写。每个仍有邻居的非初始窗口大约增加一对 128 维 FP32 load/store，即 1024 bytes。长 row 上摊销额外请求近似为 1024/S bytes/edge：S32 约32，S64约16，S128约8。相对于源 BF16 的256 bytes/edge，这分别约为12.5%、6.25%、3.125%。这是局部请求量估算，不能称为实际 DRAM 流量。

只有测得同 M 的逐位等价输出、相同 TDP 数以及对应硬件访问变化，才可以支持访问组织的解释。若输出相同但没有访问指标变化，还应排除编译器指令差异、依赖链和线程尾部。细粒度插桩自身可能制造尾部，须结合独立 PMU 区域与未插桩 E2E。

## 计时解释约束

正式 E2E 不插内核计时器。24 个子阶段及每线程记录来自独立剖析运行；父子区间重叠，不能相加。每个剖析输出额外通过与未插桩输出逐位比较。细分到很短的操作时，计时器和编译器屏障的影响可能大于操作本身；保留 clocked_sections，供后续估计扰动。

PMU 分别保存 userspace cycles、instructions、generic cache misses/references，以及 SPR 的 EXE.AMX_BUSY。每个 TID 单独缩放，保留线程覆盖与启停偏差。EXE.AMX_BUSY 是 speculative arithmetic busy cycles，不是退役 TDP 数；不直接等于 AMX 的 E2E 时间占比。Generic cache events 通常描述 LLC，但定义与 CPU 有关；不等于 L1/L2 miss 或 DRAM bytes。[Linux 官方手册](https://man7.org/linux/man-pages/man2/perf_event_open.2.html)；[Intel 官方事件定义](https://github.com/intel/perfmon/blob/78eb739dafa28c1b296f7b4d5fb7e1a7e81b1537/SPR/events/sapphirerapids_core.json)。

## 已有工作覆盖与新增贡献门槛

不能把以下动作单独包装为新贡献：换 block size、简单局部融合、普通缓存/预取、仅用平均度做调参、常规 hi/lo BF16 分解。

已有工作已覆盖计算分配后融合、稀疏依赖下的 tile fusion、访存模型及矩阵单元策略选择：[SparseLNR](https://arxiv.org/abs/2205.11622)、[TileFusion，ICS 2025](https://www.cheshmi.cc/KazemCheshmi_files/tile_fusion_ics25.pdf)、[J-Stream](https://par.nsf.gov/servlets/purl/10274755)、[DDB，ICS 2022](https://iacoma.cs.uiuc.edu/iacoma-papers/ics22_2.pdf)、[MaSpMM，TACO 2026](https://doi.org/10.1145/3803422)。共同邻居的聚合冗余消减也已有 [HAG](https://arxiv.org/abs/1906.03707)。

较值得形成主线的是：受局部存储、稀疏依赖和数值误差约束的 AMX 投影放置。联合研究 S（访问窗口）、M（投影范围）和 P（partial 表示），解释为什么在哪个归约边界投影，以及宽 partial 应保留多久。需要有明确执行空间、硬件成本模型、误差约束、固定策略、留出图验证与失败边界；逐图选最快参数不是已实现策略。

## 数值方向必须补足的内容

FAST/ACCURATE 不应成为独立的“精度花样”。一个有价值的问题是量化位置如何约束合法投影范围。对正常范围的值，BF16 截断误差可用相对精度上界描述；然而 row 中的取消效应、FP32 长归约、AMX 对非正常值的处理和层间传播会破坏简单相对误差结论。相对原 TFS 的误差与相对理想 FP32 算子的误差必须分别保留。

可先研究保守的绝对误差预算或输出范数界，之后再决定如何选择 M/P。当前随机 H/W 通过的误差门限是实验 gate，不是任意图/特征/权重上的数学保证。

## 后续实验应由本轮数据决定

1. 若固定 M 的窗口化显著改善，进一步区分 source reuse、L2/LLC 与线程关键路径，建立可预测且有留出验证的策略。
2. 若投影减少后宽归约成为主成本，研究归约表示/消费位置，而不是继续减少已经很少的 TDP。
3. 共同邻居共享需先测可用复用量，再研究有界 tile 生命周期内共享 D 维或 F 维 partial；必须与 HAG/TileFusion 相关机制比较。
4. 真正期刊级实验还需形状泛化、边权语义泛化、真实推理输入、参数选择开销及更强已有融合方法。当前一轮加速或诊断结果不能替代这些工作。
