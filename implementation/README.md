# 三层 Multi-Head Vanilla GAT：Online Softmax 与 TFS-Online

模型固定为 Din → 8×32 → 8×32 → 1×C。每个 head 独立拥有 W、aL、aR、score 和 softmax；前两层 ELU 后 concatenate，第三层输出 logits。LeakyReLU slope=0.2。输入是 destination-row CSR、双向去重边及每点 self-loop。当前是 FP32 三层前向和性能基线，没有 dropout、反向传播、损失函数或训练循环。

## 路径

| 路径 | 数据流 | 边级 score/alpha | 全局 N×D 的 U |
|---|---|---|---|
| reference | HW → score → stable softmax → 加权 Z | 完整保存，仅用于正确性 | 不适用 |
| online_fp32 | HW → score → block Online Softmax → 加权 Z | 不完整保存 | 不适用 |
| online_avx512 | 同上，score/exp/rescale/加权用 AVX-512 | 不完整保存 | 不适用 |
| tfs_online_reference | H(Wa) → score → Online Softmax → 加权原始 H → 完整 U → GeMM → 输出归一化 | 不完整保存 | 落地，用于代数验证 |
| tfs_online_fused | 同一 attention/softmax；原 TFS degree-sort、R-panel、16 行局部 U tile → 立即 GeMM | 不完整保存 | 不落地 |
| panel_transform_first_fp32 | 同一 TFS panel/线程调度，但聚合 Z；用于公平研究对照 | 不完整保存 | 不适用 |

每个 row/head 维护独立 m、l、U。TFS 路径的 U 是 D 维，先计算 U W，再在 head_dim 维除以 l。Layer 2 的 sparse feature elements 是 transform-first 的 8 倍。所有路径在每层及完整三层端到端与 reference 比较 max_abs_error、mean_abs_error、relative_L2_error；验收阈值 max_abs ≤ 0.003 且 relative_L2 ≤ 1e-4。

性能主基线采用 `reference` 纯 Vanilla GAT。`panel_transform_first_fp32` 使用 TFS 的 Degree Sort 与 panel 调度，只用于分析这些机制带来的差异，不充当主基线。细分计时与两图的同节点比较见 `runs/FINE_TIMING_PURE_GAT_20260929.md`。其中 products 的 TFS/AMX 未通过固定精度门槛，其速度仅作诊断。

tfs_online_fused 是**FP32 TFS-style tile-local fusion**，复用了原 TFS 源码的逻辑 degree-sort、R-panel、16 行 tile 与 OpenMP 动态 panel 调度。它尚未执行 BF16/AMX tile 指令。原源码的实际实现与精度/动态权重适配问题见 TFS_SOURCE_AUDIT.md；不要将本结果称为 AMX 加速。

## 文件和运行

- src/gat_online.cpp：前三条路径及共享模型、图格式、正确性函数。
- src/gat_tfs_online.cpp：两条 TFS 路径及同调度控制路径。
- tools/prepare_arxiv.py：把官方 OGB raw 转为 GATBIN；已用于 ogbn-arxiv 和 ogbn-products。
- tools/prepare_smoke.py：固定随机种子的 1024 节点正确性小图。
- scripts/build.sh、scripts/run_arxiv.slurm：前三条路径的历史基线。
- scripts/build_tfs.sh、scripts/run_tfs_arxiv.slurm：当前六路径实验。
- DATASETS_AND_TOOLCHAIN.md：可用数据集与编译链。
- runs/TFS_ONLINE_EXPERIMENT_SUMMARY.md：正确性、分段计时、稀疏膨胀、匹配线程的加速比和原始日志索引。
- runs/TFS_ONLINE_ISSUES.md：实现问题及性能对照限制。

构建：bash scripts/build_tfs.sh。运行：sbatch --export=ALL,BLOCK_EDGES=32,SPEED_REPEATS=2,TFS_OMP_THREADS=16,TFS_MKL_THREADS=16 scripts/run_tfs_arxiv.slurm。普通 intel 共享分区、单节点；对正式速度结论还需独享节点重复。BLOCK_EDGES 可选 16/32/64，PANEL_R 可用 auto/16/32/64/128。每个作业在 runs/tfs-arxiv-JOBID 下保存 profile.log、speed_rep0..N.log、manifest 和 hash；rep0 是预热。

细分计时覆盖 bL/bR 准备、L/R、score、online softmax、rescale、weighted SpMM、局部 GeMM、输出归一化、ELU、head concat，以及层和完整三层 E2E。融合路径的 per-stage worker_s 是线程时间合计，layer_total_s 是 wall time；两者不能直接相加。没有显式的 SpMM→GeMM 拷贝步骤，transition_worker_s 为 0；完整 U 的逻辑写读字节单独报告。AMX tile load/compute/store、BF16 pack 和 TMM spill/reload 尚无实测值。

初步结论：TFS fused 在 16 worker 条件下比原来稀疏循环单线程的 Online 路径快，但在相同 panel 和相同 16 worker 下，Layer 2 比 transform-first 控制路径慢约 1.74 倍。因而当前 FP32 实现的融合收益尚未抵消 8× sparse feature workload。

## 2026-09-29 新增实验性 AMX 路径

上面的历史结论只对应原六条 FP32/AVX 路径。`src/gat_tfs_amx.cpp` 实现实验性 AMX-BF16 `head-as-row` 聚合，使用 `src/amx_head_row.hpp` 的动态打包微核；每个 head 继续独立维护 online softmax 状态。它将局部 U 交给 FP32 MKL GeMM，不落地全局 U。默认使用 BF16 权重和特征的高低两部分补偿，因单 BF16 在小图三层输出上未达到原 FP32 容差。

构建：`bash scripts/build_tfs.sh`。各路径可在独立进程中用 `gat_tfs_online_speed GRAPH 32 benchmark 64 PATH` 测三层前向；AMX 速度入口是 `gat_tfs_amx_speed GRAPH 32 64 benchmark`。ogbn-arxiv 的三层精度通过，但 AMX 速度为匹配调度 FP32 transform-first 的 0.427×；详见 `runs/AMX_ARXIV_10797351_REPORT.md`。ogbn-products 的 FP32 TFS 和 AMX 均未通过固定的最大绝对误差门槛，其 0.396×/0.294× 速度仅为诊断数据；详见 `runs/PRODUCTS_AMX_20260929_REPORT.md`。这两个结果均来自共享节点，不是正式性能验收。

实现范围和当前限制详见 `IMPLEMENTATION_STATUS_20260929.md`。历史日志与 2026-09-28 时间数据原样保留；新计时边界的数值需要重新运行，不能直接与旧日志拼表。
