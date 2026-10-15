# oneMKL BF16 分离式 SpMM→GEMM 对照（2026-10-15）

## 目的与实现

对照原 TFS V3，重新运行**分离的**两层 GCN：每层先计算完整的 `Z=AH`，然后用 oneMKL BF16 GEMM 计算 `ZW`。图、权重、输入特征、随机种子、宽度 128、ReLU 与原始源码一致。原 TFS V3 函数直接包含原文件，未改动其内核。

oneMKL 2023.2 的本实验 Sparse BLAS 调用是 `mkl_sparse_s_mm`，因此稀疏阶段实际为 **BF16 量化的 H → FP32 解码 → FP32 SpMM**。其完整 FP32 中间结果 `Z` 落地，再量化为 BF16，调用 `cblas_gemm_bf16bf16f32`；GEMM 的 BF16 输入与 BF16 权重产生 FP32 输出。这是一条真正分离的路径，但**不能称为全程 BF16 SpMM**。层间 ReLU 后将 FP32 H1 转为 BF16，再解码给第二层 Sparse BLAS。初始 H/W 准备、inspector、DegreeSort 和 TFS W packing 均在计时区间外。

所有方法在每张图的同一进程连续运行，先 warmup，每个方法 5 次正式测量，轮换顺序并报告中位数。普通 `intel` 共享节点、32 核、`OMP_PROC_BIND=close`、`OMP_PLACES=cores`、默认 NUMA 策略。17 图分别在两个连续作业中运行，均落在 `qhcn071`；这是诊断性结果，不作为独享节点论文正式数据。

## 17 图端到端结果

单位 ms。`MKL/TFS` 与 `MKL/FULL` 是**时间比**；大于 1 表示 oneMKL 较慢。FULL 为关闭 DegreeSort 的现有 FULL FAST 路径。

| 图 | oneMKL BF16 | 原 TFS | FULL | MKL/TFS | MKL/FULL |
|---|---:|---:|---:|---:|---:|
| ogbn-products | 474.416 | 334.605 | 269.571 | 1.418 | 1.760 |
| reddit | 217.359 | 305.190 | 127.082 | 0.712 | 1.710 |
| mycielskian19 | 3631.302 | 2193.352 | 2073.522 | 1.656 | 1.751 |
| hollywood-2009 | 227.453 | 299.464 | 99.261 | 0.760 | 2.291 |
| indochina-2004 | 602.386 | 352.823 | 157.037 | 1.707 | 3.836 |
| soc-Pokec | 156.826 | 98.050 | 77.362 | 1.599 | 2.027 |
| cit-Patents | 154.450 | 77.705 | 71.722 | 1.988 | 2.153 |
| soc-LiveJournal1 | 454.222 | 261.193 | 194.068 | 1.739 | 2.341 |
| com-LiveJournal | 409.033 | 244.180 | 170.838 | 1.675 | 2.394 |
| as-Skitter | 98.784 | 104.014 | 50.234 | 0.950 | 1.966 |
| rgg_n_2_24_s0 | 936.388 | 922.053 | 442.330 | 1.016 | 2.117 |
| web-Google | 31.795 | 21.274 | 19.098 | 1.495 | 1.665 |
| roadNet-CA | 54.829 | 35.119 | 30.351 | 1.561 | 1.806 |
| wiki-Talk | 94.691 | 107.924 | 31.935 | 0.877 | 2.965 |
| email-Enron | 1.715 | 2.306 | 0.781 | 0.744 | 2.196 |
| amazon0601 | 17.364 | 12.560 | 9.801 | 1.382 | 1.772 |
| com-Youtube | 41.817 | 47.683 | 22.958 | 0.877 | 1.821 |
| **几何平均时间比** | — | — | — | **1.234** | **2.099** |

oneMKL BF16 在 6/17 图快于原 TFS，但在 17/17 图慢于 FULL No DegreeSort。与该程序的 FP32 oneMKL 路径比较，FP32/BF16 的几何平均时间比为 1.591；两者的数值路径不同，不能把该差值只归因于 AMX。

## 计时与正确性

逐次记录 `SpMM1/2`、两次 `Z FP32→BF16`、两次 BF16 GEMM、ReLU、层间 FP32→BF16 和 BF16→FP32、最终输出转换。products 的两次 SpMM 阶段中位数之和为 427.317 ms，总时间中位数 474.416 ms；reddit 为 210.962 / 217.359 ms；mycielskian19 为 3623.220 / 3631.302 ms。这些阶段中位数与端到端中位数来自不同的排序，不能当成精确可加分解。

17/17 图通过 BF16 最终输出与原 TFS 的数值门槛（relative L2 和 normalized max 均不超过 0.03，无 NaN/Inf），最大 relative L2 为 0.006440。此比较验证了可接受的数值接近性，**不是逐位一致**；完整 `Z` 的额外 BF16 量化会造成差异。FULL DegreeSort 与 FULL No DegreeSort 的输出在 17 图逐位相同。

`MKL_VERBOSE` 在 BF16 路径打印 `GEMM_BF16BF16F32`；`perf stat -e amx_ops_retired.bf16` 分别在 email-Enron 和 web-Google 记录到 3,606,970,368 与 90,088,538,112 个事件。因此在这套环境与调用中，BF16 GEMM **确实使用 AMX**。这些 PMU 数值覆盖整个进程的多次迭代，不是单次 GEMM 的计数。稀疏阶段仍是 FP32 oneMKL Sparse BLAS。

## W 预打包的同节点对照

为使 oneMKL 与原 TFS 的 W 准备边界一致，另做 `cblas_gemm_bf16bf16f32_pack` 预打包 W（计时外），再在计时区间调用 `cblas_gemm_bf16bf16f32_compute`。SpMM、`Z` 量化、ReLU、图与数值门槛均未改变。32 核 17 图在**同一个作业、同一个节点 `qhcn522`**内交替测了预打包与未预打包：

| 图 | 未预打包 BF16 (ms) | 预打包 BF16 (ms) | 预打包/未预打包 |
|---|---:|---:|---:|
| ogbn-products | 878.623 | 878.463 | 1.000 |
| reddit | 780.312 | 779.801 | 0.999 |
| mycielskian19 | 6004.422 | 5954.663 | 0.992 |
| roadNet-CA | 73.942 | 78.224 | 1.058 |
| wiki-Talk | 125.773 | 129.057 | 1.026 |
| **17 图几何平均** | — | — | **1.014** |

预打包仅 4/17 图更快；两次 GEMM 阶段时间之和的 17 图几何平均比（预打包/未预打包）为 **1.140**。这说明在当前 oneMKL 版本、矩阵形状和该接口下，预打包没有救回分离式路径。不能据此断言所有 oneMKL 版本或形状下预打包都慢。17/17 图预打包路径仍慢于同作业的 FULL No DegreeSort，几何平均时间比为 **2.751**；仅 1/17 图快于原 TFS。

同一 graph 在 `qhcn522` 上的绝对时间明显不同于前述 `qhcn071`，例如 products 的 BF16 oneMKL 从 474 ms 变为 879 ms。两轮都是共享节点，CPU 绑定集合也不同；不能跨两轮拼接毫秒数计算加速比。表内的预打包比较只使用 `qhcn522` 的同作业数据。

另在 `qhcn015` 用 4 核重复完整 17 图，同样在各图的同一进程内交替运行。预打包仅 1/17 图更快，预打包/未预打包端到端时间比的几何平均为 **1.036**，预打包/FULL No DegreeSort 为 **1.873**。4 核与 32 核分开统计，不合并其绝对时间。两种线程数均未发现 W 预打包改善这条分离式路径。

## 证据与限制

- 源码：`GCN-extra/experiments/onemkl_bf16_twostep_20261015/`。
- 17 图汇总：`GCN-extra/results/onemkl-bf16-twostep-20261015/fixed17.csv`；PMU 原始输出在同一目录。
- 服务器完整原始日志、源文件快照、环境、哈希和 Slurm 记录：`/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/onemkl-bf16-twostep-20261015-081637/`。
- 预打包 oneMKL W 的单因素对照源码：`GCN-extra/experiments/onemkl_bf16_twostep_packed_20261015/`；32 核与 4 核完整结果、32 核原始逐次日志：`GCN-extra/results/onemkl-bf16-twostep-packed-20261015/`。服务器源快照与 Slurm 证据：`GCN-extra/runs/onemkl-bf16-twostep-packed-20261015-163448/full17/`、`full17_4core/`。

这组实验回答的是当前 oneMKL 2023.2 可用接口下，分离 SpMM→BF16 GEMM 的实际性能。共享节点波动和 `Z` 量化差异限制了论文级因果结论；若要写正式论文表，应在独享节点用同一输入精度与可接受的数值定义重跑。
