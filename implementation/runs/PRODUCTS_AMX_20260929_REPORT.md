# ogbn-products 大图 GAT / TFS / AMX 诊断实验

日期：2026-09-29。该结果是共享节点上的**诊断性能数据，未通过现行严格绝对误差门槛，不用于论文性能表**。

## 数据与方法

输入是 `wzh/datasets/ogbn-products_official/products` 中的 OGB products 原始特征、边和标签。`prepare_arxiv.py --dataset ogbn-products` 转为 destination-row CSR，双向去重，每点一个 self-loop。结果为 **2,449,029 节点、126,167,053 条 CSR 边、100 维 FP32 输入、47 类**。模型结构 `100→8×32→8×32→1×47`；各 head 的参数和 attention 独立。图文件为服务器 `wzh/GAT/data/products.gatbin`，其 SHA-256 及原始 CSV SHA-256 在两个 run 目录中。

使用 `intel` 共享节点、16 CPU、OMP/MKL 各 16 线程、邻居块 32、degree-sort panel 64。正确性作业 `10797365` 在 `qhcn030`；诊断计时作业 `10797454` 在 `qhcn017`。两次作业的时钟不能直接互相比；下表所有时间均来自 **同一个** `10797454` 作业。三条路径每轮单独进程，rep0 预热，rep1/rep2 测量；时间边界为完整三层前向，不含图读取、排序预处理、模型初始化或数值比较。中点为两次测量值的算术中点。

| 路径 | rep1 / s | rep2 / s | 中点 / s | 相对匹配调度 transform-first |
| --- | ---: | ---: | ---: | ---: |
| FP32 panel transform-first | 8.896665 | 8.851058 | 8.873861 | 1.000× |
| FP32 TFS-Online fused | 22.330946 | 22.431293 | 22.381119 | 0.396× |
| BF16 高低位补偿 AMX TFS-Online | 29.962900 | 30.304400 | 30.133650 | 0.294× |

FP32 TFS fused 约慢 2.52 倍，AMX 路径约慢 3.40 倍。这里的“加速比”仅是同作业时间比值；由于下述精度失败，不能视为可用实现的性能结论。

## 正确性状态

作业 `10797365` 完成 products 图转换，随后执行六路径 FP32 profile。逐层比较有输出；但完整三层 `tfs_online_reference` 和 `tfs_online_fused` 相对 FP32 reference 的 `max_abs=0.00329780579`，略超预先固定的 `0.003` 门槛，`relative_L2=4.20023251e-6`。作业以 `CORRECTNESS FAIL`、Slurm `FAILED 1:0` 结束，因此没有进入其计时段。没有调宽门槛。

作业 `10797454` 继续以诊断形式执行 AMX verify 和计时，Slurm `COMPLETED 0:0`；manifest 记录 AMX verify 的实际退出码。AMX 完整三层输出相对 FP32 reference 的 `max_abs=0.0064125061`、`mean_abs=1.54320377e-5`、`relative_L2=1.71394679e-5`，最大绝对误差也未过 `0.003`。这些数值说明相对误差较小，但不能替代原有验收门槛。

## 机制观察

第二层 `D=256, K=8, d_h=32`，TFS 相对 transform-first 的稀疏特征处理量仍为 8×。FP32 profile 中该层 `42,882,792` 个逻辑 head 邻居块，`4,476,996` 次 rescale，比例约 `10.44%`；ogbn-arxiv 对应为约 `3.08%`。AMX profile 第二层 packing worker 累计 `134.252 s`，tile compute worker 累计 `30.421 s`。这些 worker 时间跨 16 线程相加，不能直接当作层 wall time；但说明当前动态 gather/packing 是主要优化候选。AMX 仍使用局部 FP32 U + MKL GeMM，并非原 TFS 完整 tile 级 AMX 内核。

下一步若要形成可发表性能结果，需要先解释 products 上的绝对误差超限，明确数值验收标准或改进计算精度；然后在独占节点上增加重复次数。失败作业及其原始日志均保留在 `products-amx-10797365/`，诊断作业原始日志在 `products-diagnostic-10797454/`。
