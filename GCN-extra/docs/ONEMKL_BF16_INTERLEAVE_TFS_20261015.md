# BF16 oneMKL 与原 TFS：双方开启 NUMA interleave

## 实验问题与控制

在相同图、H/W 随机种子、CSR、两层 GCN、128 输入/输出宽度、ReLU、32 个 OpenMP 线程下，比较**分离式** BF16 oneMKL SpMM→GEMM 与原 TFS V3。双方在每张图的同一进程中轮换运行；整个进程由 `numactl --interleave=all` 启动。因此 graph/H/W/TFS/MKL 的分配均继承同一内存策略。作业内部两次 `numactl --show` 均显示 `policy: interleave` 和 `interleavemask: 0 1 2 3 4 5 6 7`。

本次直接复用上次冻结的 driver、原 TFS V3 函数、编译选项和可执行文件，二进制 SHA-256 为 `97109515e895f3f755a0dbfc9b31d7ceca02ffc4bbf28e182df62a24e25c9f5f`。唯一实验变化是运行时 NUMA 策略。oneMKL BF16 路径每层先用 `mkl_sparse_s_mm` 对 BF16-量化输入解码后的 FP32 特征计算完整 `Z=AH`，再量化 Z 并用 `cblas_gemm_bf16bf16f32` 进行 BF16 GEMM。预打包版本把 W 的 oneMKL pack 放在计时区间外。两条 oneMKL 路径及原 TFS 均使用同一个 process-level interleave 策略；Sparse BLAS 仍是 FP32 接口，不能称为 BF16 SpMM。

执行环境：Slurm `intel` 共享分区，job `10973480`，节点 `qhcn509`，32 CPU，`OMP_PROC_BIND=close`、`OMP_PLACES=cores`，每个方法 1 次 warmup、5 次轮换顺序正式测量，报告中位数。申请内存 120 GiB；Slurm batch step 最大 RSS 约 73.5 GiB。该节点分配的 CPU 集合横跨多个 NUMA 域；`interleave=all` 涵盖 8 个 NUMA 域。

## 两层端到端结果

单位 ms。最后两列是 oneMKL/原 TFS 的**时间比**，大于 1 表示 oneMKL 慢。FULL-DS 仅作同作业参照。

| 图 | 原 TFS | oneMKL BF16 | BF16 W 预打包 | FULL-DS | BF16/TFS | 预打包/TFS |
|---|---:|---:|---:|---:|---:|---:|
| amazon0601 | 33.87 | 68.42 | 68.56 | 27.20 | 2.02 | 2.02 |
| as-Skitter | 253.61 | 268.85 | 275.20 | 140.22 | 1.06 | 1.09 |
| cit-Patents | 194.32 | 380.94 | 388.70 | 176.63 | 1.96 | 2.00 |
| com-LiveJournal | 598.07 | 736.15 | 742.91 | 404.00 | 1.23 | 1.24 |
| com-Youtube | 99.11 | 138.50 | 146.44 | 61.50 | 1.40 | 1.48 |
| email-Enron | 4.42 | 7.05 | 6.85 | 1.99 | 1.59 | 1.55 |
| hollywood-2009 | 806.43 | 800.58 | 805.82 | 281.19 | 0.99 | 1.00 |
| indochina-2004 | 549.83 | 1032.68 | 1074.26 | 257.23 | 1.88 | 1.95 |
| mycielskian19 | 4849.38 | 9244.36 | 9275.78 | 3773.40 | 1.91 | 1.91 |
| ogbn-products | 978.80 | 1472.49 | 1491.46 | 881.71 | 1.50 | 1.52 |
| reddit | 626.58 | 1011.89 | 1022.49 | 331.54 | 1.61 | 1.63 |
| rgg_n_2_24_s0 | 1685.90 | 3345.00 | 3364.42 | 1526.97 | 1.98 | 2.00 |
| roadNet-CA | 81.02 | 149.99 | 157.62 | 58.25 | 1.85 | 1.95 |
| soc-LiveJournal1 | 606.33 | 841.58 | 841.58 | 430.45 | 1.39 | 1.39 |
| soc-Pokec | 274.35 | 490.07 | 490.81 | 242.90 | 1.79 | 1.79 |
| web-Google | 52.37 | 131.23 | 137.04 | 51.60 | 2.51 | 2.62 |
| wiki-Talk | 181.36 | 197.88 | 209.96 | 71.89 | 1.09 | 1.16 |
| **17 图几何平均时间比** | — | — | — | — | **1.584** | **1.613** |

未预打包和预打包 BF16 oneMKL 均仅在 1/17 图略快于原 TFS（hollywood-2009）；预打包/未预打包自身的几何平均时间比为 **1.019**，没有改进端到端时间。FULL-DS 在本作业内快于原 TFS，几何平均 FULL-DS/TFS 时间比为 **0.654**。

## 阶段与正确性

| 图 | 未预打包 SpMM1+2 | 未预打包 GEMM1+2 | 两次 Z 量化 | 预打包 SpMM1+2 | 预打包 GEMM1+2 |
|---|---:|---:|---:|---:|---:|
| mycielskian19 | 9216.21 | 12.40 | 7.00 | 9281.50 | 13.85 |
| ogbn-products | 1338.84 | 59.23 | 25.02 | 1345.13 | 69.74 |
| reddit | 996.34 | 8.30 | 4.75 | 1000.91 | 8.13 |
| roadNet-CA | 38.21 | 50.10 | 19.44 | 38.19 | 58.70 |
| wiki-Talk | 73.26 | 55.70 | 22.63 | 70.47 | 67.92 |

阶段数值也各自取中位数，不应直接相加作为端到端时间。大图中分离式 SpMM 是主要开销；低平均度图的 GEMM 和转换占比更高。完整 CSV 保存 10 个阶段的两条路径逐项时间。

17/17 图的两条 BF16 路径均通过相对 L2 与 normalized max 不超过 0.03、无 NaN/Inf 的门槛；最大相对 L2 为 **0.006440**。FULL DegreeSort 与 FULL-DS 的最终输出在每张图逐位一致。三个关键方法（原 TFS、BF16 oneMKL、预打包 BF16 oneMKL）的 5 次测量中，最大单图变异系数约 **4.02%**，没有超过 5%。

## 解释边界与证据

这是**双方都开启 interleave** 的同节点比较，支持“在该节点/线程配置下，原 TFS 比分离式 BF16 oneMKL 快”的诊断结论。它不能单独回答“打开 interleave 分别给两者带来多少收益”：之前默认 NUMA 策略的数据来自其他共享节点，CPU 绑定和并发负载不同，不能跨作业直接相减。若要测 NUMA 策略本身的因果效应，需要在同一个 Slurm allocation 内交替运行 default 与 interleave 两种独立进程。

- 本地 launcher 与记录脚本：`GCN-extra/experiments/onemkl_bf16_interleave_20261015/`。
- 17 图 CSV、逐次原始输出、作业内 NUMA 证明：`GCN-extra/results/onemkl-bf16-interleave-20261015/`。
- 服务器完整运行目录：`/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/onemkl-bf16-interleave-20261015-171946/full17/`。
- 两条路径所用冻结源码与二进制：`/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/onemkl-bf16-twostep-packed-20261015-163448/source_snapshot/`、`build/bench`。
