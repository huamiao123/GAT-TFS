# TFS 期刊扩展 P0：访问窗口与投影范围解耦的证据

## 实验边界

本轮验证用户提供的《TFS 期刊扩展技术说明》中的 P0。固定两层 128→128→128、单位边权、随机生成的 H/W、DegreeSort、TR=16、R=64、32 个物理核、单 socket、默认 NUMA。两层 complete-forward 时间不含图加载、DegreeSort、初始 H 转换和 W packing；层间 ReLU 与 FP32→BF16 转换计入。A0/A1/A2/A3 和原 TFS、源码中的 MKL 在每张图的同一进程、同一节点上测量。原 TFS 输出 BF16，MKL 输出 FP32，因此对 MKL 的加速比不是完全同精度算法比较。

方法定义：A0=`bfull_fast`，逐行处理整个邻居范围后投影；A1=`s64_mfull_fast`，每行至多访问 64 个邻居便切到下一 destination row，直到 tile 内所有邻居完成后只投影一次；A2=`b64_fast`，每 64 个邻居归约一次并投影；A3=`s64_mfull_rowwise_fast`，保留 A1 的 64 邻居边界和 FP32 partial 状态往返，但按行完成全部窗口后才切换 destination。A1/A3 都保留原 TFS 的 tile-aware 局部聚合→AMX 投影，没有写出全图 AH。

结果及全部逐次记录见 [A3 结果](JOURNAL_A3_RESULTS_20261004.md)、[S/M 结果](PROJECTION_WINDOW_RESULTS_20261004.md)、[汇总 JSON](../results/journal-p0-20261004/summary.json) 和 [原始表目录](../results/journal-p0-20261004/README.md)。源码和正确性协议见 `../experiments/journal_decoupling_a3_20261004/PROTOCOL.md` 及 `PROJECTION_WINDOW_PROTOCOL_20261004.md`。

## 已验证的结果

17 图上 1088 个数值检查、459 个同投影边界逐位检查和 510 个剖析/未剖析逐位检查均通过。A1 与 A3 的两层真实邻居访问、64 邻居窗口、投影次数、逻辑 FP32 partial 读写字节和 AMX TDPBF16PS 次数相同；在相同投影边界上输出逐位相同。性能主表使用未插桩的连续五次 complete-forward 中位数；另有轮转顺序与原始重复值。

| 图 | 原 TFS ms | A2 B64 ms | A0 FULL ms | A1 S64/MFULL ms | A3 逐行 ms | A0/A1 | A3/A1 |
|---|---:|---:|---:|---:|---:|---:|---:|
| Mycielskian19 | 2262.86 | 1129.10 | 2479.39 | 1109.77 | 2471.47 | 2.234 | 2.227 |
| Reddit | 313.18 | 126.65 | 135.83 | 121.48 | 133.35 | 1.118 | 1.098 |
| Products | 342.44 | 296.75 | 293.62 | 296.82 | 300.11 | 0.989 | 1.011 |
| RoadNet-CA | 34.85 | 38.11 | 38.36 | 38.58 | 38.78 | 0.994 | 1.005 |

固定 A1 相对原 TFS 跨 17 图几何平均 **1.314 倍**，11/17 图更快；相对 A0 FULL **1.052 倍**，7/17 图更快。若去掉 Mycielskian19 和 Reddit，A1/A0 几何平均为 **0.997 倍**。因此优势明显集中在两张图，不能称为所有图通用的加速。A3/A0 的 17 图几何平均为 **0.996 倍**；它在 Myciel 基本回到 A0 的水平。

Myciel 的 A1/A3 每层都有 903,194,710 次真实邻居访问、897,060 个邻居窗口、24,576 次完整 tile 投影、786,432 条 TDPBF16PS，以及约 7.13 GB/7.33 GB 的逻辑 partial load/store 请求。原 TFS 的分析模型每层为 1,810,038,496 条 TDPBF16PS，A2 B64 为 28,705,920 条。逻辑 partial 字节不能当作 DRAM 字节。相同 AMX 投影预算下，A1 的两层时间分别约 544/538 ms，A3 约 1261/1253 ms。这说明投影次数本身不能解释两者的性能差异。

独立完整前向的 PMU 诊断中，Myciel 的 A0/A1/A3 generic cache misses 分别约 9.41B/4.04B/9.43B，AMX busy 事件约 25.28M/25.19M/25.25M。A1 的缓存相关事件与时间同时降低，A3 退回 A0；这是**相关证据**。这些 generic cache events 不是实测 DRAM 流量，AMX busy 不是退役指令数或 AMX 耗时。A1/A3 的 retired instructions 还不同，不能仅凭分析计数宣称“指令完全相同”。

确定性抽样的 64 邻居结构探针显示，Myciel 的跨行 source reuse 中位数约 2.06，row-step occupancy 中位数 1.0；Reddit 分别约 1.03 和 0.922；Products 为 1.0 和 0.313，RoadNet-CA 为 1.09 和 0.047。这些统计与两张获益图的结构相容，但只能提出候选机制，不能证明 reuse、缓存层级或延迟隐藏各自的因果贡献。

## 当前判断与后续实验门槛

P0 说明“邻居访问窗口”和“AMX 投影范围”确实是不同的决策：Myciel 可以保留 FULL 的低投影预算，同时获得接近 B64 的速度。A3 控制进一步表明，在本实现及这些图上，单纯添加 64 邻居循环边界和 partial 状态维护不足以产生这部分收益，跨 destination 的执行顺序很可能重要。

这仍不足以构成期刊方法。下一步应使用 **N/E/度分布相同，仅改变邻居 source 共享程度** 的受控图，以及保持 DegreeSort tile 边界不变的分组对照，分离 source reuse、并发未完成访存、线程尾部与预取行为。需要更明确的 L2/LLC/内存控制证据和留出图/维度验证；不能把事后选最优 S/M 称为已有自适应策略。若扩展到归一化或一般加权 GCN，还必须让原 TFS 和比较方法使用相同边权语义。训练好的分类模型与分类精度仍未测量。

本轮输入特征精度消融单独报告在 [17 图报告](../results/feature-precision-20261004/GCN_EXTRA_FEATURE_PRECISION_ABLATION.md)；它没有发现可直接支持“精度×粒度联合选择器”的优势方向反转，故不应将其包装为已验证的期刊贡献。
