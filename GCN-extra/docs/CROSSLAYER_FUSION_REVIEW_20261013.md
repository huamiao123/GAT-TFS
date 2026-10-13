# 10月7–8日新增GCN跨层融合实验：审阅入口与证据边界

本页用于审阅本轮新增源码、协议、原始结果和修正过程。实验发生于2026-10-07/08，
2026-10-13归档发布；发布没有重新编译或运行性能实验。全部仍是探索性证据，
`paper_eligible=false`。发布父提交：`672481d4ef34ec07f13f427485647e4def00062c`。

## 建议阅读顺序

1. [服务器最新交接原文](../results/crosslayer-fusion-20261008/handoff_snapshot/CURRENT_HANDOFF.md)。保留原文，包括尚未完善的统计和约束表述。
2. [最新自适应C++完整源码](../results/crosslayer-fusion-20261008/runs/recompute-window-adaptive-build-20261008-234221/source_snapshot/recompute_window_adaptive.cpp)。重点看`run_parallel_w0_batched`、`weighted_duplication`、`make_min_source_order`与`main`。
3. [按需生成H1与源缓存实现](../results/crosslayer-fusion-20261008/runs/recompute-window-adaptive-build-20261008-234221/source_snapshot/recompute_window_kernel.cpp)。重点看`SourceCache`与`compute_h1_group`；也保留早期serial window实现。
4. [自适应原始运行日志](../results/crosslayer-fusion-20261008/runs/recompute-window-adaptive-20261008-234255/program.log)、[原始接受记录](../results/crosslayer-fusion-20261008/runs/recompute-window-adaptive-20261008-234255/event.json)。
5. [标准中位数重算及统计](../results/crosslayer-fusion-20261008/adaptive_recomputed_statistics.csv)、[全部18个原始主样本的解析表](../results/crosslayer-fusion-20261008/adaptive_raw_samples.csv)。原日志不改写。
6. [全部新增实验源码](../results/crosslayer-fusion-20261008/experiments/)、[全部run及冻结构建](../results/crosslayer-fusion-20261008/runs/)、[run总清单](../results/crosslayer-fusion-20261008/run_inventory.json)。

## 1. 当前方法到底融合什么

基线在两层之间生成完整FP32 H1，再ReLU、转换完整BF16 H1，执行第二层。
候选按第二层destination分区的需求生成第一层源行H1，在线程内缓存复用，然后立即
执行对应第二层归约和AMX投影。没有跨过ReLU改变模型计算图。

核心步骤：

```text
destination分区与min_source顺序
    -> 收集若干destination tile所缺的第一层源行
    -> H0邻居归约 -> partial BF16 -> AMX W1 -> ReLU -> BF16 H1缓存
    -> 第二层读取H1缓存并归约 -> partial BF16 -> AMX W2 -> BF16输出
```

`source_batch_blocks=16/64`指跨16/64个destination tile批量收集待生成源行。
一个tile是16个destination，因此对应256/1024个destination的批次；它不是以前的
“每行B个邻居”参数。跨tile打包旨在减少不足16行的AMX source projection group。

原canonical `paper_methods_kernels.hpp`未修改。新代码复用其中的AVX归约和AMX投影
组件；`min_source`是新的调度顺序，与DegreeSort做对照，不应把两者称为同一种排序。
原TFS/MKL也没有被新原型覆盖。

## 2. 算子、精度与比较对象

- 随机H/W、seed12345、两层128→128→128、单位边权图；不是训练模型分类实验。
- H/W按原BF16位截断；第一层后ReLU与BF16边界保留，第二层最终输出BF16。
- 最新自适应的基线是**同min_source顺序的materialized FULL FAST**，不是原TFS、MKL或FULL ACCURATE。
- 当前exact gate是完整最终BF16输出的`memcmp`。它不等同于FP32中间误差门槛或分类准确率。
- 原始运行请求32线程，默认NUMA、无interleave。最新shared结果不是正式论文速度表。
- 原交接中的10节点上限不覆盖用户此前最多5节点的要求；发布不使用计算节点。

## 3. 自适应结果的正确统计口径

最新run为`recompute-window-adaptive-20261008-234255`、Job10902324/qhcn019。
warmup之后六次交替顺序测量，三个图最终BF16 exact均通过。

| 图 | rho_w | 所选计划 | 原日志upper median ms | 通常定义median ms |
|---|---:|---|---|---|
| RoadNet-CA | 1.045113744 | fused min_source、batch64 | 58.727→50.427，1.165× | 57.652→50.395，**1.144×** |
| amazon0601 | 3.665222816 | materialized FULL | baseline/candidate均50.273 | baseline/candidate均49.423；非独立计时 |
| reddit | 26.062167191 | materialized FULL | baseline/candidate均164.298 | baseline/candidate均163.867；非独立计时 |

原程序对六个样本排序后取`values[3]`，这是upper median；通常定义是第3/4值的平均。
归档保留两种统计，速度比不选择最快样本，也不跨节点混合。
RoadNet六个candidate样本CV约16.2%、baseline约5.7%，全部原样保留，不能将本轮比值视为稳定正式收益。

回退路径实际上只运行baseline，并把其时间赋给candidate，再复制输出做检查。
所以“时间相同”仅表示分支复用，不能称为对两个独立实现测得零额外开销。
排序和`rho_w`分析在主forward计时之外，目前也没有单列预处理成本。
回退仍使用min_source顺序；相对默认DegreeSort/FULL是否无回归，还需独立对照。

## 4. 必须保留的前序实验与负结果

- [独享RoadNet批量矩阵](../results/crosslayer-fusion-20261008/runs/recompute-window-batch-20261008-222837/)，Job10902135/qhcn819：36条PAR_RESULT，全部exact；批次1/4/16/64与三种顺序。外层事件误按12条计数标FAILED，属于bookkeeping；[修正记录](../results/crosslayer-fusion-20261008/experiments/recompute_window_batch_20261008/recompute_window_batch_exclusive_correction.json)保留。
- [batch16交替测量](../results/crosslayer-fusion-20261008/runs/recompute-window-interleave-20261008-230519/)、[batch64交替测量](../results/crosslayer-fusion-20261008/runs/recompute-window-interleave-20261008-231443/)：RoadNet identity/min_source为正，DegreeSort为负；都是共享节点。
- [Amazon交替负对照](../results/crosslayer-fusion-20261008/runs/recompute-window-interleave-20261008-230957/)：batch16候选耗时约为基线4.16–5.86倍；[共享/独享标注修正](../results/crosslayer-fusion-20261008/experiments/recompute_window_interleave_20261008/recompute_window_interleave_amazon_correction.json)保留。
- [Amazon/Reddit此前交叉验证](../results/crosslayer-fusion-20261008/runs/recompute-window-parallel-cross-10901444/)：Reddit融合路径约慢一个数量级。此前Amazon正结果的基线变动较大，不作为最终收益证据。
- [源复制率结构分析](../results/crosslayer-fusion-20261008/experiments/source_partition_weighted_20261008/)、[留出图结构数据](../results/crosslayer-fusion-20261008/runs/source-partition-weighted-heldout-20261008-234650/)、[Road USA](../results/crosslayer-fusion-20261008/runs/source-partition-weighted-roadusa-20261008-235043/)：结构筛查不等于运行速度验证。
- [热点源聚类负对照](../results/crosslayer-fusion-20261008/experiments/source_partition_hot_20261008/)：并非按最热源排序就能改善加权复制率。
- [旧adaptive计时](../results/crosslayer-fusion-20261008/runs/recompute-window-adaptive-20261008-233857/)的回退candidate为0，已被v2 supersede；[缺少serial快照的失败构建](../results/crosslayer-fusion-20261008/runs/recompute-window-interleave-build-20261008-233639/)也保留。
- 10月7日的排序、负载、state reuse、pair plan，以及10月8日prefetch、event trace、frontier和recompute结构实验也完整收录在`experiments/`与`runs/`，不把全部历史尝试称为当前接受方法。

## 5. 当前局限与下一步

`rho_w`按源行自身degree加权跨线程重复生成的聚合量；阈值1.25是探索值，尚未进行
校准/留出运行验证。它也没有完整包含AMX填充、projection成本、缓存footprint与负载尾部。

最新候选缓存容量按整个线程分区的全部unique source预先分配，源行一旦生成会保留到
分区结束。它不是受L1/L2容量限制、可淘汰的tile小缓冲。RoadNet统计source_rows2043040，
H1缓存数据逻辑量约523MB，已经略大于N1971281的全图BF16 H1约505MB；此外还存在每线程
mark/slot映射等辅助存储。基线另有FP32 H1，收益/内存峰值仍需分开实测，不能由未建
单一全局H1矩阵直接宣称DRAM流量减少。

候选计时包含线程内缺失源扫描、缓存分配/生成/消费等工作；排序、策略结构分析、初始
输入/权重准备在外。新adaptive目前主要记录整体时间与结构计数，还未补齐新路径的
细分stage/PMU，不能沿用P0/P3旧路径的profile百分比解释当前瓶颈。

后续优先：统一统计和fallback计时、单列策略/排序成本、同顺序与默认DegreeSort强对照、
缓存及真实访存测量、留出图/非128形状和稳定独享复测。本发布没有宣称这些已完成。

## 6. 审计与复现边界

- [完整文件来源与SHA256](../results/crosslayer-fusion-20261008/exported_files.csv)、[原manifest核验](../results/crosslayer-fusion-20261008/original_manifest_verification.csv)、[Slurm accounting](../results/crosslayer-fusion-20261008/slurm_accounting.txt)。
- [发布时依赖快照](../results/crosslayer-fusion-20261008/reference_dependencies/)保留original/src/helpers。它是发布时的补充，不自动构成每次历史构建的完整冻结依赖闭包；以各build原manifest为准。
- 两个早期构建`recompute-window-kernel-build-20261008-193715/193923`的manifest指向后来改过的live prototype，原期待hash与发布时hash不同；两者原失败记录和hash差异均保留，不能声称其旧源码已完整恢复。最新adaptive build的两份源文件和binary hash核验通过。
- [排除文件清单](../results/crosslayer-fusion-20261008/excluded_files.csv)标记未上传的图数据、可执行程序等；源码、原始文本记录不被其替代。
- [最新交接与三份历史总账快照](../results/crosslayer-fusion-20261008/handoff_snapshot/)保留发布前的服务器状态；新增出版核验另在当前handoff总账记录。

冻结源码中的include及compile command保留原服务器布局。若复现，应在wzh下独立目录
恢复`original/`、`src/`与`runs/<build>/source_snapshot/`的相对位置，再按原flags编译；
不要直接修改冻结文件的include来掩盖布局或覆盖canonical实现。
