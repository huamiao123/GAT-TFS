# TFS B1 低开销采样诊断：真实 arxiv / products

## 结论

当前 B1 慢的主要位置已经缩小到逐邻居执行的 weighted TFS 后端。products 第二层中，特征读取、动态加权、BF16 转换和 Hbuf 写入合计约占采样工作时间的 34%；tile load 与 AMX 计算合计约 29%；邻居预取、失活行清零和 tile 初始化约 23%；分数、exp 和 denominator 约 13%。这是工作线程采样时间的组成，不能直接当作 layer wall time 的组成或换算成绝对阶段毫秒数。

源码明确的额外工作是：每条边、每个独立 head 都要读取原始 D 维 H，准备 BF16 的 pH，然后执行 `(pH)W`。B0 则只对每个节点做一次 HW，之后沿边聚合 d 维 Z。矩阵计算加速不能消除前者的重复投影和逐边 staging。Degree Sort 已经启用；不能把所有问题归因于没用排序，也不能把当前 B1 的代价只概括为 D/d=8 倍稀疏聚合放大。

本轮没有改变生产 B1，没有新增 Ours，也没有解决 products 的 BF16 精度偏差。下一步应独立评估“先在 local tile 聚合 U[D]，再一次 UW”的候选方法，保留本 B1 作为对照；是否更快必须实测。

## 实验与计时边界

- Slurm 10809713，COMPLETED 0:0；qhcn059，共享 intel，1 节点，16 个同 socket 物理核 44–59，NUMA 5/6/7 显式 interleave。Elapsed 00:05:45，srun MaxRSS 20552972 KB。
- 编译：icpx 2024.1.0，oneMKL 2023.0 Update2；独立诊断二进制 sample_tfs。原 gat_baseline 和 baseline_tfs.cpp 未修改。构建记录 sample-build-20260930-183053。
- 官方真实 arxiv / products 图与特征：arxiv N=169343、E=2484941、Din=128、C=40；products N=2449029、E=126167053、Din=100、C=47。无向、去重、每节点一个 self-loop，与现有输入一致。
- 同一三层 8×32、8×32、1×C 模型，独立 heads、seed=11 随机未训练参数。未使用训练 checkpoint。
- 每个路径 1 次预热、3 次测量，奇数测量轮反转执行顺序。Workspace 预分配；计时含 input conversion、L/R、max prescan、weighted kernel、normalization、activation；加载、模型初始化、Degree Sort、workspace 分配与输出比较在外。
- 新增独立 B1 诊断副本：采样单位为一个 head 的完整 16 destination rows tile，分别以约 1/128、1/512 的概率采样；每轮更换选择种子。未采样 tile 走无时钟的模板特化。关闭采样副本用于检查复制内核造成的代码生成影响。
- 边界 fixture smoke、两张真实图、所有路径和测量轮的最终三层输出与原 B1 **逐位一致**。该结论只验证诊断副本没有改变 B1；不表示 B1 与 FP32 reference 数值一致或任务精度验收通过。

## 计时扰动对照

三层 median，单位 ms：

| 路径 | arxiv | 相对原版 | products | 相对原版 |
|---|---:|---:|---:|---:|
| 原版 B1 | 433.195 | — | 15959.170 | — |
| 诊断副本，关闭采样 | 428.307 | -1.13% | 16121.300 | +1.02% |
| 采样 1/128 | 439.530 | +1.46% | 16161.535 | +1.27% |
| 采样 1/512 | 434.856 | +0.38% | 16115.490 | +0.98% |

两个采样密度的整体扰动都较小。关闭采样副本也有约 1% 差异；因此不能将表中差异全部解释为时钟成本，三次重复和共享节点也限制了统计精度。低全局开销并不保证被采样 tile 的各阶段完全无扰动。

原版 B1 各层 wall time / weighted kernel wall time：

| 数据集 | Layer 1 | Layer 2 | Layer 3 |
|---|---:|---:|---:|
| arxiv | 142.965 / 138.247 | 260.206 / 254.662 | 29.835 / 27.314 |
| products | 5262.282 / 5166.928 | 9376.111 / 9266.577 | 1329.306 / 1263.162 |

第二层 weighted kernel 分别占 layer wall time 97.9%、98.8%，再次确认瓶颈位置。各字段分别取中位数，不保证相加等于 E2E 中位数。

## 第二层采样细分

下表是每轮采样工作线程时间归一化后，取三轮比例的中位数，**不是 wall-time 分解**。使用 raw 值，未用近似时钟校正改变主结论；中位数比例不必精确相加为 100%。

| 采样阶段 | arxiv 1/128 | arxiv 1/512 | products 1/128 | products 1/512 |
|---|---:|---:|---:|---:|
| 邻居预取 / 失活行清零 / tile 初始化 | 19.35% | 19.63% | 23.02% | 23.03% |
| source/right 读取、score、exp、denominator | 12.98% | 12.41% | 12.85% | 12.84% |
| H gather、BF16 expand、乘 p、RNE、Hbuf store | 27.56% | 27.72% | 34.55% | 34.20% |
| tile load + AMX TDPBF16PS | 36.90% | 36.62% | 28.68% | 28.97% |
| tile store、den 写出、输出 scatter | 3.21% | 3.67% | 0.88% | 0.87% |

products 第二层 1/512 每轮采样 2487–2491 个 tile，129410–137543 个 neighbor steps；1/128 每轮 9835 个 tile，506098–519803 steps。两种密度的比例接近。arxiv 第二层 1/128 因抽到高 degree tile，sampled steps 为 9272–21581，AMX 合计阶段比例波动较大；1/512 每轮 171–176 tile、2446–2724 steps，仍不能据此宣称没有高 degree 尾部效应。

空 Clock::now() 对的 median 间隔 arxiv 19ns、products 18ns。summary.json 另列按测量区间数量扣除该 floor 的近似校正比例；这不包含所有调用、寄存器和流水线扰动，不是精确 overhead correction。products 第二层校正后 staging 约 34.5%–34.8%、tile load+compute 约 28.8%–29.1%，排序不变。

schedule 阶段名不代表 OpenMP 调度器单独耗时，它主要计入循环内预取和失活行处理。头间 barrier、动态领取 panel、被采样区间之外的指令间隙、缓存并行重叠和尾部等待，没有被此比例完整分解。当前 B1 用 stable-max prescan，没有 running-max 更新，因此本轮不测 Online Softmax accumulator rescale；不能把本结果当作未来 Online Ours 的 rescale 代价。

## 无插桩硬件采样核对

对未修改 gat_baseline 的 products B1 做 perf cycles:u 199Hz 采样：约 210K samples，Lost Samples=0。整个进程的 cycle samples：tfs_aggregate 93.82%，OpenMP wait 2.30%，max_prescan 0.69%，SVML exp 0.68%，Intel memcpy 0.78%。支持瓶颈主要在 TFS 主核的判断；这些是含启动、预热和三次测量的进程级 cycle 比例，不是某一层 wall 比例。

首次 perf annotate 因服务器 perf 不支持 --percent-limit 失败，已保留初始错误并用兼容参数从同一 perf.data 重新生成 annotation，无需重新跑模型。服务器旧 objdump 无法可靠解码 BF16/AMX，出现 `(bad)` 和错误指令边界；汇编级 instruction attribution 不作为结论依据。tile load 和 AMX compute 尚未获得可信的独立占比；gather / conversion / store 也仍作为组合阶段报告。

## 加速比与后续行动

本轮只运行 B1 的原版和诊断副本，没有重跑 B0，不能把跨作业 B0 时间拼成新的严格 speedup。最近同一次作业 history-10809340 的预分配结果：arxiv B0 BF16 68.349ms、B1 444.112ms，B1 慢约 6.50 倍；products 3812.717ms vs 16080.420ms，慢约 4.22 倍。本轮 B1 时间量级一致。

优先研究的两项成本是逐边重复 `(pH)W` 和逐边 feature staging。增加 Degree Sort 或只改善最后 tile store 难以改变主要工作量；是否改预取也必须有独立开关对照，本报告未证明预取一定有益或有害。候选 Ours 应尽量将 UW 改为每 destination/local tile 一次，同时保持真正独立 attention 与 Online Softmax 正确 rescale，并完整测量三层。不得直接替换当前 B1。

products BF16 B1 在上一轮相对 FP32 reference 的 max_abs=11.5483、relative_L2=0.0200900 仍未解决；本轮逐位复现同一路径不能消除这个问题。所有性能结果仅用于探索诊断，未完成模型精度或论文性能验收。

## 证据

- bench/tfs_sampled.cpp、sample_main.cpp：独立采样诊断及全模型逐位比较。
- scripts/summarize_sample.py：raw 与近似 floor-adjusted 比例、各层 wall time、扰动对照。
- runs/sample-10809713：manifest、规则快照、source/artifact hashes、两个数据集完整 JSON logs、summary.json、perf symbol report / annotation / 初始错误。
- runs/sample-build-20260930-183053：编译日志、编译状态、规则及源文件/二进制哈希。
- perf.data 保留服务器 wzh 下，不提交大体积二进制采样文件。
