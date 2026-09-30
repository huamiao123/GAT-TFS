# 第三个基线 B1：为何原来 586 ms，统一 NUMA 后仍有 449 ms

## 重测结果

用户所指的 500+ ms 对应 `tfs_bf16`（B1），不是纯 GAT B0。实验 arxiv-10809143 在原节点 qhcn059、原 16 个物理核 44–59、显式 NUMA 5/6/7 interleave 下完整重跑正确性、固定 p、微基准、三层模型和 profile。模型仍为 Din→8×32→8×32→1×40；图为 arxiv，N=169343，E=2484941；参数使用同一显式随机 seed=11，非真实训练 checkpoint。

| 路径 | 三层中位数 ms | P95 ms |
|---|---:|---:|
| B0 FP32 | 73.439 | 75.124 |
| B0 BF16 | 67.976 | 69.006 |
| B1 TFS BF16 | 448.673 | 458.663 |
| B1 attention + B0 backend 诊断 | 68.035 | 68.685 |

统一内存策略使 B1 从旧 586.236 ms 降低约 23.5%，但相对现在的 BF16 B0 仍慢 6.60 倍。正确性回归 64 个边界组合、独立 NumPy 三层 oracle 均通过；所有路径输出有限。真实任务精度门槛仍未配置。此为共享节点探索结果，不能直接用于论文验收。

## 时间在哪里

speed binary 各阶段分别取 7 次中位数，预热 2 次；字段中位数不必精确相加：

| B1 环节 / ms | Layer 1 | Layer 2 | Layer 3 |
|---|---:|---:|---:|
| input RNE conversion | 0.703 | 1.235 | 1.274 |
| L/R | 0.571 | 0.857 | 0.680 |
| max prescan | 0.803 | 0.782 | 0.389 |
| weighted TFS 主核 | **147.020** | **253.439** | **35.354** |
| normalization | 1.232 | 1.281 | 0.207 |
| ELU | 1.433 | 1.463 | — |
| layer total | **151.737** | **259.107** | **37.866** |

第二层 TFS 主核约占该层 97.8%。Degree Sort 约 16.82 ms，属于静态准备，在 forward 计时之外，不是 449 ms 的原因。

固定**同一份** p 和 denominator 后，第二层后端：

- B0 projection + weighted aggregation + normalization：24.338 ms。
- B1 weighted TFS + normalization：243.980 ms。

固定 p 时不含动态 exp/attention 分数生成，input conversion 也在计时外；B1 仍慢约 10.03 倍。因此主要差异来自后端执行，不是 attention 前端。另一个辅助证据是，采用 B1 attention 前端但保留 B0 后端的完整模型仍只有约 68 ms。

## 源码实际做了多少工作

`src/baseline_tfs.cpp` 已使用 Degree Sort、16 destination rows、dynamic R panels、独立 heads、smart zeroing、下一邻居预取、TMM 输出驻留。对照原 `GAT/code/code/amx_tfs_v3.cpp`，原代码也在 neighbor-step 循环内部执行 W tile loads 和 TDPBF16PS。当前 B1 采用这一执行顺序，符合 `IMPLEMENTATION_CONTRACT.md` 第 6.1/6.5 节的选择。

但是在本 GAT 适配里，每个邻居、每个 head 都会：

1. gather 原始 BF16 H_j[D]；
2. expand 为 FP32、乘动态 p、再 RNE 转回 BF16，写入线程私有 Hbuf；
3. 让 AMX 执行 `(p H_j)W^h`，累加到该 destination 的输出。

所以同一个 source 节点的投影会在多个 destination/head 的边上重复执行。B0 则每层先执行一次所有节点的 HW，再沿边聚合 Z。B1 当前不是“把整个邻居和 U 聚合完，只对每个 destination 执行一次 UW”的执行顺序；不能只用 D/d=8 的 sparse workload amplification 描述它。

第二层 D=256、K=8、d=32：

- B0 projection：2NKDd ≈ **22.196 GFLOP**。
- B0 weighted Z aggregation：2EKd ≈ **1.272 GFLOP**。
- B1 有效边 projection：2EKDd ≈ **325.706 GFLOP**。
- B1 含 tile padding 的实际 AMX 运算：2×172894453760 ≈ **345.789 GFLOP**。

projection 工作量约增加 **15.58 倍**。其中 E/N≈14.67 是邻居重复投影的基本放大，剩余来自 row padding。本层 Degree Sort 后有效 tile 行比例约 **94.2%**，padding 只增加约 6.2%；主要负担已经存在于有效边计算里。第三层 d=40 还要 pad 到 48，另有输出列 padding。

此外，第二层 B1 的逻辑源 feature 读取约 **10.18 GB**，还需 pX/RNE 的 Hbuf 写入与 AMX tile 读取；B0 的逻辑 Z 读取约 **2.54 GB**。这些是源码推导的 load 字节量，**不是实际 DRAM 流量**，缓存可能复用。AMX 能加速矩阵计算，但不能消除随机 gather、动态加权、量化和 staging。

## 定位边界与下一步

可以确定：NUMA 放大了旧结果；修正后主要瓶颈仍为 weighted TFS 主核；逐边重复 projection 与动态 feature staging 是源码可证实的额外工作。不能据此宣称所有 TFS 数据流或所有图都一定慢。

当前细粒度 profile 对每个向量/tile 读时钟，第二层总时间变成约 3654 ms，而 speed binary 只有 259 ms。这些 profile 数字严重受插桩影响，不能用于声称 gather、conversion 或 AMX 分别占真实耗时的某个比例。若要继续拆到这一级，应采用低频采样或分阶段受控 microbenchmark，并量化插桩影响。

进一步的候选研究方向是让 UW 按 destination/local tile 执行一次，或减少逐边 staging；这会改变 B1 的执行顺序，应作为单独方法开发并与该 B1 比较，不能悄悄替换基线。当前本轮只诊断和重测，没有更改 B1 计算内核。

## 证据

- `baseline/runs/arxiv-10809143/`：manifest、规则/技能文本与哈希、源码/二进制/图/参数哈希、正确性、固定 p、全部原始时序与 profile 日志。
- Slurm COMPLETED 0:0，qhcn059，1 节点、16 核，elapsed 00:01:26。
- `scripts/audit_tfs_work.py` 从保存的 speed JSON 推导上述工作量；`scripts/summarize.py` 汇总原始时序。
