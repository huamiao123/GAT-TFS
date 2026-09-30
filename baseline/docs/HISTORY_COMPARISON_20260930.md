# 昨天 TFS 比基线快，今天却慢：同图同参数对照

## 已核实的历史事实

昨天的原始报告 `implementation/runs/FINE_TIMING_PURE_GAT_20260929.md`，作业 10803234：

| 图 | 旧 Reference | 旧 FP32 TFS local-U | 相对旧 Reference | 旧并行 transform-first 控制 |
|---|---:|---:|---:|---:|
| arxiv | 1405.917 ms | 530.881 ms | 2.648× | 370.607 ms |
| products | 70963.990 ms | 22696.066 ms | 3.127× | 8844.274 ms |

用户记得的加速确实存在。其含义是旧 TFS 整体实现相对旧 Reference 更快。旧 Reference 的 projection 使用多线程 MKL，但 L/R、score、softmax、weighted aggregation、activation 的显式循环主要单线程。旧 TFS/并行控制路径则使用 16 worker。

昨天的匹配并行控制在两图上已经比旧 TFS 更快，所以旧结果并未证明 aggregate-first 比充分并行的 transform-first 快。products 旧 FP32 TFS 当时还超出固定 max_abs=0.003 门槛，应保留其速度为诊断数据。

## “TFS”名称下实际换了哪些实现

| 路径 | sparse 聚合对象 | W 的执行位置 | 主要稀疏/矩阵工作量 |
|---|---|---|---|
| 旧 Reference | Z=HW | 每个 source 先投影一次 | O(NKDd + EKd)，主要单线程 sparse |
| 旧 FP32 TFS local-U | 原始 H | 每个 destination 的局部 U 聚合完成后做 UW | O(EKD + NKDd)，16 worker |
| 今天 B0 FP32/BF16 | Z=HW | 每个 source 先投影一次 | O(NKDd + EKd)，并行 AVX sparse |
| 今天 B1 TFS edge-projection | 加权原始 H | 每条边执行 (pH)W，输出 TMM 驻留 | O(EKDd)，16 worker AMX |

两版 TFS 都有 Degree Sort 和 destination blocking，但计算顺序不同：旧版是局部 U→GeMM；B1 遵照新的实现合同，复用原 AMX TFS 的 neighbor-step output accumulation。上次简短答复没有充分区分这个变化与基线变化，容易让读者误认为同一套 TFS 内核突然退化。

当前 B0 还进行了 head 合并投影、AVX L/R、并行 row-max、融合 score/exp/aggregation、向量化 normalize/ELU，并将 workspace/静态权重准备移出重复前向计时。必须逐项说明，不能把新 B0 的时间与昨天旧 Reference 直接混为相同基线。

## 本轮严格对照设计

- 图与官方 raw 特征：ogbn-arxiv（N=169343，E=2484941，Din=128，C=40）；ogbn-products（N=2449029，E=126167053，Din=100，C=47）。都是双向去重、每点一个 self-loop 的既定 GATBIN1 文件，不重新生成随机图。
- 参数仍是固定随机 fixture，非训练 checkpoint；新旧 seed 为 11/22/33，程序逐字节检查 W/aL/aR 一致，degree permutation 也逐项检查。
- `bench/compare_history.cpp` 直接 include 保存的旧实现，链接今天的内核，未改新旧计算内核。每次都用同一图、同一输入和参数执行相同三层 8×32/8×32/1×C，warmup 后正反交替路径顺序。
- **统一口径对照**：三层前向包含每层缓冲区分配/初始化、权重准备和释放；输入 H 复制、图读入、模型生成、Degree Sort、误差计算和打印在计时外。旧路径不能重用 workspace，所以没有拿它的 allocation-inclusive 时间直接与新路径的 preallocated 时间计算“纯内核收益”。
- **今天运行口径**：另外测新三条路径的预分配前向，分别报告；两种计时表不可混算加速。
- qhcn059，intel 共享节点，1 节点、16 个物理核 44–59，同 socket、NUMA 5/6/7 显式 interleave，OMP/MKL 16，MKL_DYNAMIC=FALSE。每条路径 1 次预热、3 次测量。
- 小图边界回归通过后，再运行同参数三层误差检查。真实图上会报告不同精度的误差；未配置真实训练任务精度验收，速度仅用于诊断。

## arxiv 已完成的结果

统一包含分配的三层中位数：

| 路径 | ms |
|---|---:|
| 旧 Reference | 1346.797 |
| 旧并行 transform-first | 334.957 |
| 旧 FP32 TFS local-U | 440.355 |
| 新 B0 FP32 | 177.017 |
| 新 B0 BF16 | 204.093 |
| 新 B1 TFS edge-projection | 544.954 |

旧 TFS 相对旧 Reference 仍快 **3.058×**；相对旧并行控制则慢 **1.315×**。在相同分配口径下，新 B1 比旧 local-U TFS 慢约 **23.8%**。基线实现变化与 TFS 执行顺序变化同时存在，不是一个因素。

今天预分配前向：B0 FP32 **74.065 ms**，B0 BF16 **68.349 ms**，B1 **444.112 ms**。例如新 FP32 B0 同口径从 1347 ms→177 ms 的收益已经存在，即使不移出缓冲区分配；预分配再减少约 103 ms。BF16 多出 input BF16 缓冲区和转换，单次分配口径可能比 FP32 慢，预分配口径则更快。

arxiv 同参数三层 FP32 路径 relative L2：旧并行控制 1.90e-7，旧 local-U TFS 3.59e-7，新 FP32 B0 3.32e-7；新 BF16 B0 为 0.003105，新 B1 为 0.003177。不同 precision 不视为零误差。

## products 与最终状态

products 对照已全部完成。下表与上面的 arxiv 表一样，**包含分配、初始化、权重准备和释放**，单位 ms：

| 路径 | products 三层中位数 ms |
|---|---:|
| 旧 Reference | 70227.515 |
| 旧并行 transform-first | 8358.110 |
| 旧 FP32 TFS local-U | 17110.845 |
| 新 B0 FP32 | 5363.311 |
| 新 B0 BF16 | 5643.935 |
| 新 B1 TFS edge-projection | 18024.315 |

旧 TFS 相对旧 Reference 仍快 **4.104×**；但相对旧并行控制慢 **2.047×**。相同分配口径下，新 B1 比旧 TFS 慢约 **5.34%**。新 FP32 B0 相对旧 Reference 已快 **13.09×**；这不是 BF16 改变精度造成的收益。

今天预分配前向单独报告：

| 路径 | arxiv ms | products ms |
|---|---:|---:|
| 新 B0 FP32 | 74.065 | 3910.139 |
| 新 B0 BF16 | 68.349 | 3812.717 |
| 新 B1 TFS | 444.112 | 16080.420 |

products 新 B1 的第二层 9370.874 ms，其中 weighted TFS 主核 9261.065 ms，约占 98.8%；新 BF16 B0 第二层总计 1671.635 ms，其中 weighted Z 聚合 1488.890 ms、投影 65.281 ms、转换 18.971 ms。

## 真实图精度：本轮发现必须单独保留的问题

同参数的 products 完整三层输出，相对旧 FP32 Reference：

| 路径 | max_abs | relative L2 |
|---|---:|---:|
| 旧并行 transform-first | 0.001072 | 1.140e-6 |
| 旧 FP32 TFS local-U | 0.003298 | 4.200e-6 |
| 新 B0 FP32 | 0.000532 | 9.429e-7 |
| 新 B0 BF16 | **25.1209** | **0.0253783** |
| 新 B1 BF16 | **11.5483** | **0.0200900** |

新 FP32 B0 与旧 FP32 Reference 基本一致。旧 local-U TFS 仍略超出原固定 max_abs=0.003 门槛，未修改门槛。新 BF16 两条路径在 products 上存在显著 logits 差异，**不能把它们的速度当作已通过精度验收的加速结果**。BF16 是否适合真实训练参数和任务准确率尚未验证；不能仅依据 finite 或小图通过宣布大图精度通过。本轮未隔离 BF16 量化、attention 敏感性与其它数值因素各自的贡献，所以没有将该误差未经验证地归因于 AMX 硬件或某个特定 kernel bug。

## 最终判断

1. 用户记忆中的昨日加速属实，而且在同一真实图上能复现。
2. 昨天比较的是单线程 sparse Reference 与并行 TFS。今天主基线已经换成并行、SIMD、融合 attention/aggregation 的强 B0；相对性能反转的主要来源是**参照基线变强**。
3. 新 B1 与旧 TFS 也不是同一计算顺序。在统一计时范围下，新 B1 相对旧 local-U TFS 的退化约为 arxiv 23.8%、products 5.3%，而不是突然慢了数倍。
4. 预分配使今天的 forward 再变快；需要单独报告。NUMA 修正也明显改变时间。不能把这些收益都归到 TFS 或 AMX。
5. 两版 aggregate-first TFS 在本轮两个真实图上都未胜过匹配并行的 transform-first；这只说明当前实现与随机参数条件，不能泛化到全部 TFS、真实训练精度或其它图。
6. 当前优先使用新 FP32 B0 建立可信正确性/性能基准；任何 BF16 加速结论需补充任务精度校准，并单独排查 products 的误差。

作业 history-10809340 已 COMPLETED 0:0，elapsed 00:10:44，最大 RSS 18956172 KB（约 18.1 GiB），1 节点。当前没有留在运行的本轮作业。源码/图/二进制/规则哈希及完整日志已保存。所有性能数字仍是共享节点上的 3 次探索测量，不是正式论文验收。

## 证据路径

- 原历史：`implementation/runs/pure-gat-baseline-10803234/` 及 `FINE_TIMING_PURE_GAT_20260929.md`。
- 本轮：`baseline/runs/history-10809340/`，服务端 build 证据 `baseline/runs/history-build-20260930-165236/`。
- 汇总脚本 `scripts/summarize_history.py` 分开输出 allocation-inclusive bridge 与 preallocated production。
