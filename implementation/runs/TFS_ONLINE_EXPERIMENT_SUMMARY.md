# Multi-Head GAT TFS-Online 第一阶段实验（2026-09-28）

## 状态与边界

已增加 tfs_online_reference、tfs_online_fused，两者均按每个 head 的独立 W/aL/aR 实现 bL=W aL、bR=W aR，计算 L=H bL、R=H bR，再按原 Vanilla GAT score 做分块 Online Softmax。累加的是原始 H 的 D 维特征，未归一化的 U 先做 GeMM，输出只在 head_dim 维除以 l。三层仍为 128→8×32→8×32→1×40，图、自环、ELU 与原路径一致。

tfs_online_reference 一次为一个 head 落地完整 N×D 的 U；tfs_online_fused 借用原 TFS v3 的 degree-sort、R-panel、16 destination rows、OpenMP dynamic panel 调度，最多保留一个 16×D 的线程局部 U tile，立即做 GeMM、归一化和原始节点顺序写回。没有完整 U 的全局 materialization。还增加 panel_transform_first_fp32 控制路径：同一 panel、同一线程数、同一 FP32 Online 更新，但聚合 Z 的 head_dim 维，用于隔离 8× sparse amplification。

这版 fused 使用 FP32 和局部 MKL GeMM；**尚未使用原 BF16 AMX tile 指令**。原 TFS 源码的 BF16 静态无权图 AMX 内核无法在保持当前 FP32 严格正确性和动态 per-row rescale 的条件下直接调用。对应源码审计见 GAT/TFS_SOURCE_AUDIT.md。以下为探索性共享节点实验，不作为论文级性能结论。

## 可复现性与证据

输入：ogbn-arxiv，169343 节点、2484941 条经双向去重且含 self-loop 的 CSR 边，128 输入维、40 类；block_edges=32，自动 R=64。编译器 oneAPI icpx 2024.1.0，FP32，MKL。运行 scripts/build_tfs.sh 和 scripts/run_tfs_arxiv.slurm。所有写入均在 wzh/GAT。

主实验 10782776：普通 intel 共享分区 qhcn100，申请 1 节点/16 CPU；OMP_NUM_THREADS=MKL_NUM_THREADS=16；rep0 预热，rep1/rep2 计时。日志在 runs/tfs-arxiv-10782776/profile.log、speed_rep1.log、speed_rep2.log，manifest.txt、artifact.sha256、preflight.sha256 同目录。作业 Slurm COMPLETED 0:0。

单线程复核 10782790：同一 qhcn100，OMP_NUM_THREADS=MKL_NUM_THREADS=1；rep0 预热、rep1 计时；Slurm COMPLETED 0:0。另有 10782741（初步 16 线程）和 10782750（初步 1 线程），以及 10782767（繁忙共享节点的控制路径实验）；原始日志保留，绝对时间波动较大。

## 正确性

主实验 profile 和所有 speed 重复均输出 CORRECTNESS PASS。每层六路径均与同输入的 Reference 对照；完整三层各路径还分别独立串联，最终输出与 Reference 比较 max_abs_error、mean_abs_error、relative_L2_error。TFS reference 与 fused 完整三层的对照结果相同：最大绝对误差 1.84774399e-6，平均绝对误差 5.41786471e-8，相对 L2 为 3.58968159e-7。Layer 2 两条 TFS 路径相对 Reference 均为 max_abs 2.44379044e-6、relative_L2 3.30420259e-7。

Attention 等价性也单独核查。H(Wa) 与 (HW)a 的 L/R 最大绝对差：Layer 1 为 8.34e-7，Layer 2 为 1.01e-6，Layer 3 为 4.17e-7。所有 head 的 score/softmax 各自独立，未共享 attention matrix。

## 稀疏工作量与局部 U

Layer 2：D=256、8 heads、head_dim=32。TFS-Online 对原始 H 处理 5089159168 个边-特征元素；transform-first 处理 636144896 个，严格为 8×。tfs_online_reference 的 U buffer 峰值约 173407232 字节（一 head 复用）；8 heads 合计的逻辑 U 写入加后续读取约 2774515712 字节。tfs_online_fused 的完整全局 U buffer 为 0，只有每线程最多 16×256 FP32 的局部 tile；这一字段不等同于硬件缓存流量测量。

Layer 2 有 1580688 个 neighbor blocks、1403397 次 running-max 更新、48653 次已有 accumulator 的实际 rescale，rescale/blocks=3.078%；被 rescale 的特征元素 12455168。每 row/head 的 rescale 次数 P50/P90/P95/P99 为 0/0/0/1。首 block 初始化 max 计入 max update，但不计入旧 U rescale。

## 16 线程初步速度

下表为 qhcn100 主实验 rep1/rep2 的中位数，单位秒。所有路径同一个图、参数、精度和一节点 16 CPU 资源；原三路径的稀疏循环仍是单线程，panel_transform_first_fp32 与 TFS fused 的稀疏部分均是同样的 16 worker panel 调度。

| 路径 | Layer 2 | 三层 E2E |
|---|---:|---:|
| Reference | 0.625 | 1.384 |
| 原 FP32 Online | 0.791 | 1.744 |
| 原 AVX-512 Online | 0.696 | 1.543 |
| TFS Online Reference（完整 U） | 2.296 | 4.095 |
| TFS Online Fused（局部 U） | 0.297 | 0.558 |
| 同调度 transform-first FP32 控制 | 0.170 | 0.410 |

若直接与现有 AVX-512 Online 实现比较，TFS fused 的 Layer 2 为 0.696/0.297≈2.34×、三层 E2E 为 1.543/0.558≈2.77×。**这不是可归因于融合的加速比**：原 AVX 稀疏循环单线程，TFS fused 使用 16 个 worker。

在匹配 TFS panel 调度和 16 worker 后，Layer 2 的 transform-first 控制路径为 0.170 s，TFS fused 为 0.297 s，即 TFS fused 慢约 1.74×；三层 E2E 0.410 s 对 0.558 s，慢约 1.36×。当前实现中，省去全局 U 并未抵消多 head 导致的稀疏工作量膨胀。

## 单线程复核

作业 10782790 的 Layer 2：原 AVX-512 Online 0.844 s、TFS Online Reference 2.477 s、TFS Online Fused 2.553 s、同 panel transform-first 控制 1.108 s。TFS fused 比同 panel 控制慢 2.30×。完整三层 E2E：原 AVX 1.788 s、TFS fused 4.605 s、同 panel 控制 2.417 s。单线程条件下，局部 tile 融合没有带来净加速。

## 详细时间与解释

作业 10782776 的 Layer 2 profile（TFS fused）：bL/bR 准备 0.000044 s、L/R 0.00580 s、W pack 0.000054 s；16 worker 的累计 edge score 0.387 s、online softmax 0.158 s、其中 rescale 0.00249 s、weighted SpMM 1.951 s、局部 GeMM 0.207 s、输出 normalization 0.00543 s、ELU 0.111 s、head concat 0.0223 s；layer wall 0.319 s。worker-sum 不可与 layer wall 直接相加比较。TFS reference 中 U 的全局写入归在 weighted SpMM，读取归在 GeMM，没有单独可计时的显式 transition 调用，所以 transition_worker_s=0；逻辑写读字节另列。

同 profile 的 panel transform-first 控制：projection 0.0294 s、L/R 0.0303 s，16 worker 累计 weighted aggregation 0.611 s；TFS fused 的 weighted SpMM worker 时间约为其 3.19×。当前瓶颈是更宽的稀疏特征聚合；实际 rescale 时间很小。尚无 AMX tile load/compute/store、BF16 pack、TMM spill/reload 时间或字节测量，不得把 FP32 路径误报为 AMX 结果。

## 结论与下一步

已正确实现并测量 Multi-Head GAT 的 aggregate-first TFS-Online 数据流，完成无完整全局 U 的 tile-local fusion。在当前 FP32 实现里，Layer 2 的 8× sparse feature amplification 没有被融合收益抵消。下一步若研究 AMX，需先定义 FP32 精度可接受的 BF16/补偿策略、每行独立 running-max rescale 的 TMM staging，并记录转换、spill/reload、tile compute 时间；还需要独享节点重复实验才能形成正式性能结论。
