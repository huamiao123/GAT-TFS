# 独立多头 Online 聚合优化：实现与真实图结果

日期：2026-10-01。两轮作业均已完成，退出码 `0:0`。本报告只对本轮共享节点、随机参数前向实验负责；没有训练后的任务精度验收，也没有正式独享节点性能结论。

## 1. 当前采用的方法

模型保持 `Din → 8×32 → 8×32 → 1×C`。每个 head 的 W、aL/aR、attention 和 Online 状态独立；原图、self-loop、concat、ELU、输出节点顺序保持一致。

```text
静态准备：每 head bL=W*aL，bR=W*aR
H → H[bL,bR] → CSR neighbor blocks → 独立 Online m/l/U[D]
  → worker-local U[16 rows][heads][D]
  → 每 head 立即执行 UW → 输出除以 l → hidden ELU
```

这是一条 **FP32 aggregate-first Online 候选**。保留原 TFS 的 Degree Sort、TR16 destination blocking、dynamic panels、按原节点位置 scatter；聚合用 AVX-512，UW 用单线程 per-worker SGEMM。没有使用 AMX 计算 PH，也没有把完整 Z/U/e/alpha 写到全局 buffer。它不能被称作“已优化的 B1 BF16”。原 B0/B1/local 内核均未改动。

### 本轮优化内容

1. **跨 head 复用源 H 加载。** 对同一个 destination 和 neighbor block，一次 H 向量加载馈送组内多个独立 head 的 FMA。group=1/2/4/8 是每组同时处理的 head 数；模型始终仍有 8/8/1 个 head。没有共享 attention。
2. **把实际 rescale 合入 U 更新。** 在本来就需要的 U load 后乘 r，再做 weighted FMA，然后 store，避免另扫一遍 D 维 U。数学上的 rescale 次数和乘法仍保留。
3. **局部 U 立即消费。** 聚合完成 TR16 后立即 per-head UW，只对 d 维输出归一化，避免完整 U 的主存往返。第二层 scratch=128 KiB/worker；组大小不改变这个布局。
4. **完整 head 组专用化。** 第二轮用 `Full=true` 编译期消除 active-head guards；尾 head 组保留检查。数学、循环顺序、SGEMM、scratch 与 v1 相同。

相对旧原始 B1，先 local-U 再 UW 避免了逐边重复 `(pH)W` 的投影。相对旧 local，跨 head 源复用和循环重排改善了宽聚合；`joint_g1→g8` 比 `old_local→joint_g8` 更适合分析分组收益，因为后者还包含 row/head 顺序与 SGEMM stride 改动。

## 2. 实验条件和证据

- 节点 `qhcn059`，共享 `intel` 分区，1 节点，16 个物理核 `32–47`；NUMA `4/5` 显式 interleave，在分配前生效；OMP close/cores，MKL dynamic=false。
- icpx 2024.1，`-O3 -fp-model precise`，AVX-512/OpenMP/MKL，速度版不维护内循环时钟与完整计数；profile/counters 独立执行。
- 每条路径完整运行三层，下一层使用该路径自己的输出；预分配 workspace，静态 Degree Sort/W preparation 单列。比较、reference copies、打印在 E2E timer 外。
- 一次 warmup、三次交替顺序测量，报告三层 E2E 中位数。各层/phase 中位数独立计算，不能保证相加恰好等于 E2E 中位数。
- 官方图和特征：arxiv `N=169343,E=2484941,Din=128,C=40`；products `N=2449029,E=126167053,Din=100,C=47`。统一已准备的 CSR；参数 seed11，**未训练**。

| 作业 | 内容 | Slurm 状态 | Elapsed | step MaxRSS |
|---|---|---|---|---|
| 10849337 | 原三基线、旧 local、joint G1/G2/G4/G8、固定 attention 控制 | COMPLETED 0:0 | 5m45s | 44,757,092 KiB |
| 10849393 | 同一二进制内 v1/full 与强 B0 对照 | COMPLETED 0:0 | 4m12s | 33,567,768 KiB |

[第一轮完整结果](../runs/joint-10849337/RESULTS.md)、[第二轮完整结果](../runs/joint-full-10849393/RESULTS.md)含原始诊断；目录内保存 source/binary/data/rules hashes、manifest、pinning、stdout/stderr、Slurm accounting 和源码快照。构建证据：[v1](../runs/joint-build-20261001-150558/)、[full](../runs/joint-full-build-20261001-152302/)。

## 3. 第一轮三层结果

单位 ms；同一作业内比较。

| 路径 | arxiv E2E median | products E2E median |
|---|---:|---:|
| B0 FP32：纯标准 GAT | 74.015 | 3820.758 |
| B0 BF16：纯标准 GAT | 67.599 | 3723.109 |
| 原 B1 TFS BF16 | 432.552 | 15698.650 |
| 旧 local Online FP32 | 248.122 | 7982.355 |
| joint G1 | 215.849 | 6380.318 |
| joint G2 | 154.657 | 4499.233 |
| joint G4 | 152.238 | 4940.558 |
| joint G8 | 142.562 | 4661.921 |

arxiv G8 相对旧 local 快 **1.740×**，products G2 快 **1.774×**。但它们分别仍比 B0 FP32 慢 **1.926× / 1.178×**。相对 BF16 B1 的收益包含精度和执行顺序差异，不能归为单纯同精度 TFS 收益。

### products 每层：第二层与整体最优不同

| 路径 | Layer1 ms | Layer2 ms | Layer3 ms |
|---|---:|---:|---:|
| B0 FP32 | 1609.161 | 1688.217 | 522.549 |
| B0 BF16 | 1612.241 | 1626.604 | 489.720 |
| 原 B1 | 5304.312 | 9116.377 | 1269.039 |
| 旧 local | 2341.149 | 4652.282 | 987.091 |
| joint G1 | 1752.396 | 3591.152 | 1004.686 |
| joint G2 | 1230.077 | 2243.382 | 1020.746 |
| joint G4 | 1544.149 | 2373.065 | 1008.627 |
| joint G8 | 1563.221 | 2083.908 | 1021.174 |

G8 在第二层最好，但第一层 D=100 上比 G2 慢，因此完整模型 G2 更好。第三层只有一个 head，不能获得多头 source reuse；aggregate-first 聚合256维，标准路径只聚合47维，仍有约5.45×稀疏维度放大。

## 4. 第二轮：消除分支的收益有限

| 同一二进制路径 | arxiv ms | products ms |
|---|---:|---:|
| B0 FP32 | 74.242 | 3838.167 |
| B0 BF16 | 67.922 | 3736.940 |
| v1 G2 | 157.734 | 4515.483 |
| v1 G8 | 139.486 | 4645.452 |
| Full G2 | 148.886 | 4407.019 |
| Full G4 | 145.185 | 4742.111 |
| Full G8 | 135.469 | 4592.146 |

同作业内，Full G8 相对 v1 G8 快 **1.02965× / 1.01161×**，Full G2 相对 v1 G2 快 **1.05943× / 1.02461×**。分支专用化没有改变主要瓶颈，所有 Full 候选仍慢于 B0。

Full G8 第二层 products=2091.764 ms，B0 FP32=1697.501 ms；Full G2 第二层=2252.300 ms。不能只选 G8 的第二层结果宣称完整三层最优。第一轮 old local 与第二轮最快候选约有1.83×/1.81×差异，这只是跨作业描述；严格分支消除收益采用上面的同作业比值。

## 5. 具体慢在哪里

第一轮 products G8 第二层：L/R median=18.525 ms，kernel median=2043.369 ms，layer median=2083.908 ms。B0 FP32 同层 projection=145.173 ms，L/R=17.579 ms，max prescan=41.138 ms，aggregate kernel=1441.150 ms。

G8 第二层 1/128 tile 采样的 worker 时间组成如下。它们是采样 worker sums，**不是**可相加的 layer wall decomposition，也不是各阶段精确全图毫秒数。

| 子阶段 | 采样 worker ms | 采样组成 |
|---|---:|---:|
| U 初始化 | 4.056 | 1.39% |
| source/score/max/exp/den preparation | 56.513 | 19.35% |
| weighted SpMM + fused rescale | 207.666 | 71.12% |
| UW SGEMM | 21.816 | 7.47% |
| output normalization/scatter | 1.954 | 0.67% |

第二轮 Full G8 同层 worker SpMM/rescale=202.378 ms，仍是最大项。只加速 UW 没有覆盖主要成本。速度版和 profile 版输出 fingerprint 相同，但采样、计数和共享节点波动会影响 profile wall time，不能声称零扰动。

products 第二层 blocks=42,882,792，running-max updates（含初始化）=24,069,181，实际已有状态 rescales=4,476,949，比例10.43997%，rescaled feature elements=1,146,098,944。旧数学计数保持不变。rescale 已合入 SpMM，其独立 wall time不能从联合循环中凭空拆出；编译器在无更新时也可能发出乘1，数学次数不等于实际指令次数。

### 工作量和汇编证据

第二层 D256/K8/d32：标准稀疏工作 `2KEd≈64.60 GFLOP`，aggregate-first `2KED≈516.78 GFLOP`，仍有 **8×** 稀疏 FMA 放大。两者线性投影有效工作均约321.00 GFLOP。

G8 源 H 逻辑加载从逐head `4EKD≈1.034 TB` 降至 `4ED≈129.20 GB`；FMA数量没有下降。G2只在每两个head内复用，逻辑源加载约为G8的4倍。**这些不是实测 DRAM 流量。** 本轮没有硬件 cache/DRAM counters，实际瓶颈中的带宽、延迟、TLB和指令份额尚未分离。

[v1 汇编审查](../runs/joint-build-20261001-150558/assembly_audit.md)确认一次 source vector load 喂给8个独立 FMA，accumulator 没有 per-edge 栈spill。[Full 汇编](../runs/joint-full-build-20261001-152302/assembly_audit.md)显示八条FMA连续发出，active-head guards消失。不能把已证实的 source reuse 等同于8倍性能或DRAM收益。

## 6. 正确性：实现控制通过，原始门槛没有通过

- 原基线64个边界/负测试通过；v1的576组配置全部与旧 local逐位一致，counter相同。
- Full的1008组配置全部与v1逐位一致，覆盖D17/33/128/256、K1/2/3/4/5/7/8、d7/32/47、group1/2/4/8、block16/32/64。
- 真图同输入/同L/R下，各组与旧 local完全一致；Full同输入与v1完全一致。所有速度/profile输出finite，profile逐层fingerprint核对通过。

固定同一层输入和**同一 attention**后的 products 最大绝对差异：

| 控制 | Layer1 | Layer2 | Layer3 |
|---|---:|---:|---:|
| TF vs AF，同L/R的Online/聚合/投影浮点路径变化 | .006050 | .022377 | .000841 |
| TF backend，仅L/R改为H(Wa) | .001633 | .008362 | .000246 |

所以误差不只是 attention reassociation。这里fixed-L/R冻结score定义，但TF最大值预扫描与AF Online的exp差值、分母归约、rescale和投影在FP32中仍可能有不同舍入；第一行不能将softmax浮点误差排除。两行不是可相加的独立误差项；下述FP64 oracle才用完全相同权重验证结合律。

完整三层 candidate 对 B0 FP32 的 products 误差，v1/Full保持一致：

| 层 | max abs | mean abs | relative L2 |
|---|---:|---:|---:|
| 1 | .00606537 | 1.3591e-7 | 7.7477e-7 |
| 2 | .04167938 | 5.0072e-6 | 1.3803e-5 |
| 3 | .00691891 | 9.8326e-6 | 1.4943e-5 |

原先 absolute `.003` 门槛仍未通过，没有因relative L2较小而放宽门槛。`implementation_gate=PASS` 只说明本轮实现对照，**不等于 master/task acceptance**。

独立 FP64 sampled oracle 使用相同 FP32 score、FP64 stable exp/den，并以同一权重计算 A(HW)/(AH)W；products各层代数差异约1e-14。第二层只覆盖20个完整rows、1519条edges，单row预算允许degree≤244；57920个更高degree的rows不在oracle覆盖内，最大degree17482的行明确跳过。它证明抽样代数和数值控制，不能代表全图极端行或训练精度。

## 7. 下一步研究方向

当前有效改进是独立多头 source reuse与local-U执行；主要问题仍是wide PH和数据访问，以及真实高degree行的FP32归约误差。应先对极端行用更高精度控制分离TF/AF误差，再做source access、预取、blocking与hardware counters研究。

未来 `P[8,B] × H[B,D]` 的AMX PH映射值得验证，但P量化、两/三残差分量、tail padding、动态packing、FP32状态merge和row rescale均有成本。当前FP32源复用收益不能直接预测AMX胜过B0。没有完成PH-AMX实现，也没有用近似softmax替换当前Online算法。

[方法和文献审计](METHOD_RESEARCH_20261001.md)包含精确成本模型、BF16/FP32 attention反例、PH-AMX数值与硬件接口问题，以及Illinois CPU GAT论文全文对照。已有CPU fusion/head-loop/JIT优化不能泛称新颖；本研究应量化独立多头aggregate-first的适用边界。

## 8. 源码入口

- [v1内核](../ours/joint_online.cpp)、[Full专用化](../ours/joint_full.cpp)
- [原基线对照driver](../ours/joint_main.cpp)、[同binary v1/full driver](../ours/joint_compare_main.cpp)
- [固定attention FP64 oracle](../ours/joint_checks.cpp)
- [v1构建](../scripts/build_joint.sh)、[Full构建](../scripts/build_joint_full.sh)
- [v1 Slurm](../scripts/run_joint.slurm)、[Full Slurm](../scripts/run_joint_full.slurm)
- [结果汇总脚本](../scripts/summarize_joint.py)

远端根目录：`/home/huangjianqiang_group/hdacp1/data/wzh/GAT/baseline`。全部远端写入限定wzh；yx/shared/startup配置未修改。
