# 原 ICPP TFS 的 Online Softmax 扩展：实现与实测

日期：2026-10-01。成功作业：`10851370`，`COMPLETED 0:0`，4 分 52 秒。此前失败作业 `10851291`、`10851339` 保留。

**本轮确实保留原 TFS 邻居步 AMX 融合。products 相对原 B1 有 1.143×探索性加速，arxiv 略慢；两图均未超过强 B0。真实图的原 FP32 master 绝对误差门尚未通过。**

## 1. 实现范围

源码：[`../tfs_online/`](../tfs_online/README.md)，核心 [`icpp_online.cpp`](../tfs_online/icpp_online.cpp)。原 `src/`、`include/gat.hpp` 没有修改，服务器与本地冻结源码 SHA256 一致。

保留原 ICPP TFS / B1 的：

- destination-row CSR、升序 Degree Sort、动态 row panels、TR16 同步邻居步；
- 独立多头 attention、完整输入特征 gather、BF16 RNE、VNNI W packing；
- **每个邻居步执行 `(pH)W` 的真实 AMX `TDPBF16PS`**；
- 输出部分和驻留 TMM，结束后恢复原节点顺序；
- OpenMP persistent region、head barrier、尾部清零和邻居预取。

新增 block Online Softmax，`block=16/32/64`。每行、每 head 分别维护 `m,l,V`，其中：

\[
V=UW,\qquad V'=rV+\sum_{j\in B}(p_jH_j)W,\qquad O=V/l.
\]

Layer 2 的 rescale 状态是 `V[32]`。只有 running max 上涨且旧 denominator 非零，才执行：

```
output TMM -> thread buffer -> AVX-512 row scale -> output TMM
```

没有 global `Z[N,Kd]`、`e[E,K]`、`alpha[E,K]`、`U[N,D]`，也没有末尾单独 SGEMM。`L/R=H(Wa)` 仍使用 Vanilla GAT 的等价重排，本轮原 B1 与新路径均固定 **FP32 L/R policy**。这与过去使用 BF16 L/R 的历史 B1 计时不可直接混算。

该路径与过去 `local_online` / `joint_online` 的 FP32 local-U 候选分别归档。它们的 135 ms、200 多 ms 等数字不属于本轮保留逐邻居 AMX 的实现。

## 2. 数据、模型和计时边界

| 项目 | arxiv | products |
|---|---:|---:|
| N | 169,343 | 2,449,029 |
| CSR E（统一图，含 self-loop） | 2,484,941 | 126,167,053 |
| Din / C | 128 / 40 | 100 / 47 |
| Degree Sort，静态 ms | 16.547 | 326.120 |

模型固定 `Din -> 8x32 -> 8x32 -> 1xC`，前两层 concat+ELU。所有路径使用同一图、官方导出特征、seed11 未训练权重。三层计时使用各路径自身逐层输出，不以 reference 输出替换层间数据。

- qhcn059，共享 `intel`，Xeon Max 9462；一个节点，16 物理核 `16–31`；NUMA `2,3` 显式 interleave；OMP close/cores，MKL dynamic false。
- icpx 2024.1.0，oneMKL 2023.2，`-O3 -fp-model precise`，相同 AVX512/BF16/AMX/OpenMP/MKL 编译选项。
- workspace 预分配；1 warmup，3 次测量，路径顺序交替。冷启动 weight prep、Degree Sort、分配、校验、复制与 profile 不在 steady-state E2E 内。
- 测速 specialization 的热循环没有计时时钟或计数；细分 profile 和全量计数单独运行，输出指纹一致。
- Slurm step MaxRSS `40,682,656 KiB`。原始命令、绑定、源码/二进制/数据/规则 hashes 与源码快照都在 run 目录。

## 3. 完整三层性能

下表为本作业内 3 次测量的中位数；加速比为 `baseline_time / candidate_time`。

| 路径 | arxiv，ms | products，s |
|---|---:|---:|
| B0 FP32，标准 transform-first | 75.691 | 3.923 |
| B0 BF16，标准 transform-first | 69.442 | 3.829 |
| 原 B1 TFS，max prescan | 425.159 | 15.865 |
| ICPP TFS Online，block16 | 443.104 | 未测 |
| ICPP TFS Online，block32 | 441.302 | 13.882 |
| ICPP TFS Online，block64 | 441.919 | 未测 |

block32 相对原 B1：arxiv `0.963×`（耗时增加 3.80%）；products `1.143×`（耗时减少 12.50%）。相对 B0 BF16 仍分别慢 `6.355× / 3.625×`。

三次 products 原 B1：15.781 / 15.876 / 15.865 s；新 block32：13.882 / 13.888 / 13.825 s。该差异在本轮三个重复中均出现，但共享节点、少量重复和未训练参数使它仍属于探索性结果，不能直接进入正式论文性能表。

## 4. 第二层：无细分插桩的 wall time

以下是各字段中位数，单独取中位数后不保证精确相加为 layer/E2E 中位数。

| products L2，ms | B0 FP32 | B0 BF16 | 原 B1 | TFS Online B32 |
|---|---:|---:|---:|---:|
| Dense projection | 146.242 | 60.004 | — | — |
| FP32 -> BF16 conversion | 0.000 | 21.324 | 20.008 | 22.154 |
| L/R | 19.890 | 19.360 | 21.044 | 21.414 |
| Max prescan | 41.140 | 41.148 | 41.167 | 0 |
| Complete aggregation/fusion kernel | 1,498.814 | 1,499.117 | 9,164.525 | 8,323.746 |
| Normalization | 20.564 | 20.546 | 19.885 | 20.722 |
| ELU | 22.649 | 22.500 | 22.822 | 23.027 |
| Layer total | 1,748.902 | 1,685.119 | 9,289.012 | 8,411.022 |

arxiv L2 layer/kernel：原 B1 `263.808 / 257.449 ms`，Online B32 `264.216 / 258.531 ms`。arxiv 三层退步主要出现在 L1：`132.021 -> 146.965 ms`。

products 的改善不只是省掉约 41 ms 的 max 预扫描，融合 kernel 本身也减少了约 841 ms。但 block 化、状态向量化、访问顺序及编译生成一起变化，**目前没有单因素实验把这部分收益唯一归因于某一项**。新路径 L2 kernel 仍占本层时间约 99%，是真正需要继续优化的整体区间。

## 5. 细分计时：公开插桩限制

源码已分别计时 score generation、block maximum、exp/denominator、rescale store/vector/reload、source gather、weighted BF16 conversion、tile load、AMX compute、final store、output scatter、scheduling、tile config。完整三层数据见 [`profiles.tsv`](../runs/icpp-online-10851370/profiles.tsv)。

products L2 的采样 **worker 时间之和**（ms）如下，采样 period128，9,567 个 tile：

| 区间 | 采样 worker ms |
|---|---:|
| Score generation | 177.494 |
| Block max | 15.767 |
| Exp/denominator | 5.501 |
| Rescale tile store | 0.623 |
| AVX row scale | 1.690 |
| Rescale tile reload | 0.667 |
| Source gather | 2,682.392 |
| Weighted expansion/multiply/BF16 conversion | 2,876.371 |
| Tile load | 363.983 |
| AMX compute | 792.996 |
| Final tile store / output scatter | 0.654 / 12.896 |
| Scheduling / tile config | 336.159 / 0.071 |

**不能把这些数字当作 wall time 的百分比。** 每 16 维 load/convert、每条 TDP 指令及小组 tile-load 前后读钟，计时开销与被测操作同阶，且会打断流水线。满 TR16、D256 的一个邻居步仅 gather/convert 就包含约 1,024 次读钟，AMX load/compute 另有约 64 次。采样也不是全量无偏硬件采样。

实际 profile L2 kernel 为 `9,265.802 ms`，相比无插桩中位数 `8,323.746 ms` 高约 11.32%。这份细分表用于定位后续验证区间；精确归因应增加整个 feature staging/完整 AMX kb 循环的粗粒度独立计时或硬件计数。这里不宣称 gather/packing 各占真实 wall time 的多少比例。

## 6. Online rescale 的精确计数

block32、L2：

| 指标 | arxiv | products |
|---|---:|---:|
| Row/head blocks | 1,580,688 | 42,882,792 |
| Running-max updates，含初始化 | 1,403,397 | 24,072,096 |
| Actual row/head rescales | 48,653 | 4,479,864 |
| rescales / blocks | 3.078% | 10.447% |
| row/head 比值 P50 / P90 / P95 / P99 | 0 / 0 / 0 / 0.5 | 0 / 0.333 / 0.5 / 0.5 |
| Logical rescaled features，d=32 | 1,556,896 | 143,355,648 |
| 整 output tile staging events | 11,360 | 1,203,201 |
| Tile spill bytes | 23,265,280 | 2,464,155,648 |
| Tile reload bytes | 23,265,280 | 2,464,155,648 |
| Row-scale load / store bytes，各自 | 6,227,584 | 573,422,592 |
| AMX calls | 21,105,280 | 1,011,640,704 |
| Synchronized neighbor steps | 1,319,080 | 63,227,544 |
| Padded executed FMA | 172,894,453,760 | 8,287,360,647,168 |

把逻辑缩放维度从 U256 改为 V32，确实少 8 倍元素。**整 TR16 Tile 的 spill/reload 仍存在，不能说总 staging 流量少 8 倍。** 这些 bytes 是指令操作所需的线程局部 buffer 流量计数，不能等同 DRAM 实际流量。原有 no-rescale B1 没有这部分 staging。

arxiv block64 将 L2 rescales 降到15,406，但 E2E仍约441.919ms，没有因此优于block32。AMX调用与输入特征处理次数不随block变化。

## 7. 更大的结构性问题：逐邻居重复投影

过去的 `D/d=8×` 描述的是 local aggregate-first 稀疏向量维度。保留原 TFS 邻居步 `(pH)W` 后，还存在逐边重复的 dense projection：

\[
F_{TFS}=2KEDd,\qquad F_{TF}=2NDKd+2KEd.
\]

products L2：

- `E/N=51.517`：相同 source projection 被不同邻居边反复执行，transform-first 可在节点级一次投影后复用。
- 理论有效 TFS FLOPs `16.537 TFLOP`，标准 transform-first `0.385597 TFLOP`，比值 **42.887×**；这是有效算术量比，不是时间比，也未包含 attention 等其它步骤。
- 实际 padded AMX FLOPs `2 * executed_fma = 16.575 TFLOP`，比有效工作只多约0.23%。当前 Degree Sort 调度下16行同步执行的空槽负担很小；重复投影仍然存在。
- 所有 head 的 raw BF16 H 逻辑读取约516.780GB，实际cache/DRAM流量未测。

因此本轮解决的是 **保留 TFS 的 Online 状态与输出维 rescale 接入**。D-wide feature处理和重复投影仍在。下一轮可在保留真实AMX融合的前提下，研究两个独立head共享raw H gather：分别构造各自pH、W、m/l/V，禁止共用attention。它最多减少raw H逻辑读取，不能消除TDP计算次数，也不能预设E2E收益。

## 8. 正确性和尚未通过的门

### 实现检查通过

48组小图operator，另有block16/32/64的all-equal原B1隔离，以及2组真实attention layer wrapper。覆盖空行、TR16尾、feature/output尾、1/2/8 heads、D17/33/128/256、d7/32/47/64，degree跨越block边界且强制max上涨/不上涨。

- FP32输出维V simulator对FP64 stable同LR oracle，通过abs/relativeL2各1e-5门。
- AMX对BF16指令模拟，通过独立运算/operand规模的FP32 rounding budget。
- 相同输入、L/R、p/r的TR16 SIMD denominator replay逐位一致。
- All-equal scores，p=r=1，原B1/new numerator、denominator、normalized output逐位一致，rescale/spill/reload为零。
- profile on/off和速度specialization算术一致，row/head/global/AMX/staging计数一致。

这些不覆盖次正规数DAZ/FTZ，也不代表真实模型精度已接受。

### 真实图 master 门失败，未放宽

完整三层own-output，相对B0 FP32：

| 图 / 路径 | 最终 max_abs | mean_abs | relative L2 |
|---|---:|---:|---:|
| arxiv B0 BF16 | 0.003335 | 0.0004973 | 0.003105 |
| arxiv 原 B1 | 0.003581 | 0.0005077 | 0.003176 |
| arxiv TFS Online B32 | 0.003581 | 0.0005078 | 0.003177 |
| products B0 BF16 | 25.120958 | 0.016643 | 0.025378 |
| products 原 B1 | 6.083703 | 0.013573 | 0.015083 |
| products TFS Online B32 | 6.019003 | 0.013579 | 0.015092 |

products Online L2 own-output master max_abs **25.611694**。固定同一FP32输入与L/R，新Online对原B1 L2 max_abs **0.478546**、relativeL2 **1.5642e-4**，说明额外BF16(pH)/Online rescale顺序差异也必须单独量化，不能只说attention重排。

选定8行oracle使用完整邻居，包含全图最大degree行：arxiv13,162 / products17,482。它只覆盖这些行，不是全图FP64证明。products L2对同LR的quantized H/W FP64 stable oracle（不包含再次pH量化）max_abs **0.022499**，relativeL2 **3.3873e-4**；同LR master H/W max_abs **0.111123**。全量误差和各层mean_abs见 `checks.tsv`。

原 `.003` master门不变，BF16各路径尚未全面通过。没有训练checkpoint和准确率验证。**当前是已验证基本算子实现的研究候选，不能报告完整模型精度已验收。**

## 9. 两次失败与真正的 exp 差异

`10851291` 在第一case因denominator绝对差1.52587890625e-5停下；`10851339`增加独立SIMD重放后仍同差。汇编显示kernel实际调用 `__svml_expf16_z0`，而独立检查程序调用 `__svml_expf16`。subtract/multiply/add顺序一致，无denominator FMA。

第三次节点实测：1024个[-10,0]输入中739个返回值不同，最多4ULP，max_abs2.384185791e-7。检查显式匹配kernel实际入口后，48组SIMD分母重放全部逐位一致，确认前两次是oracle没有匹配实际生成的exp入口。这个内部oneAPI入口仅用于固定编译器的测试harness，标准exp对照保留，生产kernel未修改，也未调宽输出master门。

## 10. 可复现证据

- 服务器根：`/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline`
- 成功源码/原始结果：`tfs_online/`、`runs/icpp-online-10851370/`
- 失败结果：`runs/icpp-online-10851291/`、`runs/icpp-online-10851339/`
- 最终构建：`runs/icpp-online-build-20261001-221450/`
- 二进制SHA256：`7412a42ebb5439bd601044dc46659b1b2322d40721de77b59e01c16b898aa708`
- arxiv SHA256：`3407b49a3b659397aa581ea4c5afcdef37e929a70ed98217a52413ef21201280`
- products SHA256：`cbb38a8715b8cc7c99e7caf782aa286f835f45e2802d0f4860673fb442c69869`
- 入口：`scripts/build_icpp_online.sh`、`run_icpp_online.slurm`；解析：`summarize_icpp_online.py`。
- 构建、失败与方法修正已在服务器三份GAT handoff中记录。成功作业的完整结果已保存本地；2026-10-02会话切换网络权限后SSH返回Permission denied，成功条目及本地整理暂待同步到服务器。所有已完成服务器写入限于wzh。

**后续顺序：保留原TFS、补粗粒度归因、隔离BF16与高degree误差，再验证独立head的source gather复用。不能把之前local-U数字冒充本轮原TFS结果。**
