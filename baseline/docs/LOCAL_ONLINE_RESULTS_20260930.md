# 局部 U 聚合后一次 UW：实现与真实图结果

## 当前结果

独立路径已实现并跑完三层真实 arxiv / products。FP32 candidate 相对原 B1 快 1.70 / 1.92 倍，减少了逐边重复投影，但仍比纯 GAT FP32 B0 慢 3.39 / 2.10 倍。当前实现没有取得对纯 GAT 的加速，性能问题尚未完全解决。

AMX high/low residual 后端比同一局部 FP32 后端更慢。这一 shape 下，补偿 packing、四组 BF16 矩阵乘和 tile 操作并未带来净收益；不能将 AMX 指令存在视为加速证据。

products 的输出误差显著小于原 BF16 B1，但当前 FP32 candidate 相对 B0 FP32 的最大绝对误差仍超过旧 max_abs=0.003 门槛。没有通过真实 checkpoint 的任务精度验收。下面的时间仅用于探索，不进入论文正式结果表。

## 数据流和实现

新增 `baseline/ours/`，独立于现有 B0/B1：

```text
master W/a -> bL/bR (FP32 static preparation)
H -> FP32 H[bL,bR] -> per-head L/R
destination CSR -> neighbor blocks32 -> score/max/exp
independent m,l,U[D] -> FP32 row rescale + AVX-512 weighted aggregation
worker-local 16 destination rows U -> one UW per tile/head
divide output[d] by l -> original-node/head positions -> hidden ELU
```

模型为 Din→8×32→8×32→1×C。独立 heads、图/self-loop/activation 与基线一致。保留原 TFS 的 ascending degree logical permutation、TR16 destination tiles、dynamic R64 panels、original-row scatter；执行顺序改为 aggregate-local-U 后一次 UW。完整 Z[N×Kd]、U[N×D]、e[E]、alpha[E] 均不物化。局部 U 有 L1/L2 读写，不能把 `U_global_bytes=0` 理解为没有 intermediate traffic。

- `local_online_fp32`：FP32 sparse/attention/state，thread-local single-thread SGEMM 完成 UW；外层 OpenMP 并行，MKL worker-local 线程数恢复。
- `local_online_amx_hilo`：相同 FP32 attention 和 FP32 U；将 U/W 分成 RNE BF16 high 与 residual low，用 AMX 累加 high×high、high×low、low×high、low×low 四个乘积。该 UW 是接近 FP32 的混合精度实现，不能称为精确 FP32 GEMM。
- AMX 只加速最终 UW，weighted PH 当前由 AVX-512 完成。Online rescale 在 FP32 local U 上执行，邻居扫描期间没有活跃 TMM accumulator，所以 AMX rescale spill/reload=0；这是执行顺序的选择，尚未实现 AMX sparse PH 的 row-rescale 方案。

## 实验条件

Slurm `10810186`，COMPLETED 0:0，qhcn059 shared intel，1 节点，16 个同 socket 物理核44–59，显式 NUMA5/6/7 interleave，elapsed00:04:09，srun MaxRSS42318076KB。icpx2024.1.0、oneMKL2023.0 Update2。源文件、输入图、二进制与规则哈希保存在 run。

官方真实图与特征：arxiv N169343/E2484941/Din128/C40；products N2449029/E126167053/Din100/C47。无向、去重、每节点一个 self-loop。seed11 随机未训练参数，非训练 checkpoint。

五条路径同一进程、同一模型和绑定策略。每路径先一轮输出比较，再一次预热，三次测量，奇数测量轮反向执行顺序。Workspace 预分配；图加载、模型初始化、weight packing、degree sort、compare/finite scan、打印均在 timer 外。各层使用该路径上一层输出。steady timer 包含 L/R、Online/基线 softmax、weighted kernel、normalize 与 activation。独立 profile/counters 不进入 speed medians。

## 三层端到端时间

单位 ms，三次中位数：

| 路径 | arxiv | products |
|---|---:|---:|
| B0 FP32，纯 GAT | 73.996 | 3908.668 |
| B0 BF16，纯 GAT | 68.437 | 3817.678 |
| B1 原 TFS BF16 | 425.709 | 15807.121 |
| local Online FP32 | 250.887 | 8221.782 |
| local Online AMX high/low | 270.005 | 8534.037 |

FP32 candidate 对 B1 的 speedup=1.697 / 1.923；对 B0 FP32 的 speedup=0.295 / 0.475，即仍慢3.39 / 2.10倍。B1 是 BF16 edge-projection，candidate 是 FP32 sparse/attention，前一个比例包含精度与执行顺序差异，不能称为纯融合收益。FP32 candidate 与 FP32 B0 更适合检验新执行流的整体代价。AMX candidate 对 B1 为1.577 / 1.852，对局部 FP32 则慢约7.62% / 3.80%。

## 第二层时间与剩余瓶颈

| 路径/字段 ms | arxiv | products |
|---|---:|---:|
| B0 FP32 layer total | 35.322 | 1736.068 |
| B0 FP32 projection | 10.218 | 146.129 |
| B0 FP32 score/exp/weighted kernel | 20.427 | 1485.770 |
| B1 layer total | 259.868 | 9324.168 |
| B1 weighted TFS kernel | 254.280 | 9212.406 |
| local FP32 layer total | 126.928 | 4840.405 |
| local FP32 L/R | 1.292 | 18.925 |
| local FP32 online/local-U/UW/normalize kernel | 124.186 | 4799.471 |
| local AMX layer total | 144.838 | 5058.029 |
| local AMX kernel | 142.053 | 5017.030 |

local FP32 第二层 kernel 的 1/128 tile 采样 worker 时间组成：

| 组合阶段 | arxiv | products |
|---|---:|---:|
| weighted SpMM：H gather + FP32 FMA/U update | 62.25% | 79.87% |
| CSR/source/right读取、score、max、exp、block sum | 18.75% | 13.97% |
| UW SGEMM | 12.48% | 3.90% |
| rescale 检查与实际 U scaling | 2.50% | 1.17% |
| local U 初始化 | 2.62% | 0.65% |
| normalization + scatter | 1.24% | 0.38% |

FP32 没有 BF16 U packing；profile 字段里的约0.05% / 0.15% packing 来自空计时区间，不是实际 packing 工作。rescale 时间也含不发生 rescale 的检查和时钟 floor。denominator/reference 状态赋值等少量指令处在计时段间隙；score_exp_den 字段不能当成独立完整 denominator wall time。分组计时避免逐条向量/AMX 指令插桩，但仍不是精确硬件指令成本。

以上是 sampled worker sums 的 raw 比例，**不是 wall-time 分解**。profile+counters 的第二层 wall time 相对 speed median：arxiv FP32+7.79%、AMX-3.48%；products FP32+1.57%、AMX-0.38%。一轮采样、动态调度、共享节点和 counter 写入的影响尚未单独隔离，不能据此将某比例乘以 layer wall 得到精确阶段毫秒数。

products AMX 第二层采样 SpMM 约78.20%，U high/low packing约1.90%，UW tile load+compute+store约4.09%。因此只优化最终 UW 并不能覆盖当前主要成本。D=256、heads8、d32 的 8倍 sparse feature amplification 仍然存在；当前每个 head 独立遍历并读取原始 H，H 跨 heads 的读取复用还可研究。逻辑访问量不等于实际 DRAM 流量，本轮没有采集 memory-bandwidth counters。

## Online rescale 的真实计数

FP32 第二层、block32；running-max updates 包含首次建立 m，rescale 仅统计已存在 accumulator 后最大值上升：

| 字段 | arxiv | products |
|---|---:|---:|
| neighbor blocks | 1580688 | 42882792 |
| running-max updates | 1403398 | 24069181 |
| physical U rescales | 48654 | 4476949 |
| rescaled feature elements | 12455424 | 1146098944 |
| rescales / blocks | 3.078% | 10.440% |
| per-row/head fraction P50 | 0 | 0 |
| P90 | 0 | 0.3333 |
| P95 | 0 | 0.5 |
| P99 | 0.5 | 0.5 |

这是精确计数，在 speed 外单独运行。products 的10.44%不能直接称为10.44%的性能开销；一行可能只有一个 block，也可能有许多 blocks，row/head percentile 与全局加权比率的含义不同。

## 数值结果与边界

real-graph comparator 为此前已对 R0 验证的 B0 FP32，本轮未重新运行大型 R0。相同 master W/a/H 的代数 attention 定义一致，但 HW→attention 与 H(Wa) 浮点执行不同；在线 denominator/rescale 的加法顺序也不同。大型输出误差的来源尚未通过 shared-input/matched-attention 控制完全隔离。

| 图/路径 | Layer1 max_abs / rel_L2 | Layer2 max_abs / rel_L2 | Final max_abs / mean_abs / rel_L2 |
|---|---|---|---|
| arxiv local FP32 | 2.980e-6 / 2.479e-7 | 2.325e-6 / 3.734e-7 | 1.818e-6 / 5.533e-8 / 3.675e-7 |
| arxiv local AMX | 5.126e-6 / 3.526e-6 | 4.947e-6 / 4.064e-6 | 5.424e-6 / 8.813e-7 / 5.226e-6 |
| products local FP32 | 0.006065 / 7.748e-7 | 0.041679 / 1.380e-5 | 0.006919 / 9.833e-6 / 1.494e-5 |
| products local AMX | 0.005966 / 3.543e-6 | 0.048580 / 1.534e-5 | 0.009336 / 2.029e-5 / 2.002e-5 |

所有输出 finite。旧 max_abs=0.003 门槛下 products 两个候选均未通过，不能仅凭较小 relative L2 宣布 correctness 全部通过。旧 BF16 B0/B1 在同一轮仍有约百分之二的 relative L2；详细值保存在 logs。

小图包含 D17/33/128/256、heads1/2/8、d7/32/40/47，blocks16/32/64、empty rows、TR16 tails；先完成全部 FP32 测试，再测试 AMX。另有 fixed-attention FP64 stable oracle 检验 equal scores、late peak、mixed Leaky branches、±10000 common offsets。FP32 smoke 保持 max_abs/rel_L2≤1e-4。一个 AMX near-cancelling 用例 D256/K8/d7 的 max_abs2.51e-6、rel_L2约1.04e-4，使用公开的 abs≤1e-5 fallback，且 AMX 对同一 local FP32 backend max_abs≤1e-5；此 fallback 只用于数值 smoke，不是 task gate。失败作业10809920/10809928及日志均保留。

profile/counter 版 FP32 三层与未采样版本逐位一致；AMX版各层64-bit output fingerprint一致。fingerprint并非无碰撞的逐位证明。

## 当前完成和下一项工作

已实现并测量 local-U Online 数据流，保留原 B1。逐边重复 UW 的工作已从候选中移除；当前主瓶颈转为宽特征、多 head 的 sparse aggregation。尚未取得对纯 GAT 的性能收益，也未通过 products 的旧绝对误差门槛。

后续应先用相同 reference 层输入隔离 H(Wa) attention 误差、denominator/Online 累积误差和 AMX UW 残差误差；再研究跨 heads 复用 source H 读取，同时保持各 head attention/m/l/U 独立。不要仅继续加速已占很小比例的 UW。实际 checkpoint 及任务精度门槛仍待提供。

源码为 `ours/local_online.hpp/.cpp`、`ours/main.cpp`；可重复脚本为 `build_local.sh`、`run_local.slurm`、`local_payload.sh`、`summarize_local.py`。原始证据为 `runs/local-10810186/`、三次 local-build 目录及两个失败作业目录。
