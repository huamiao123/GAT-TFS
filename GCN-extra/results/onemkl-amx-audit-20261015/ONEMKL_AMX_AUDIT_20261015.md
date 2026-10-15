# 原始 TFS 源码 oneMKL / AMX 路径审计（2026-10-15）

## 问题与结论

用户给出的 `D:\Apaper\Delta-TFS\TFS\code\code` 与仓库 `GCN-extra/original` 的三份对应文件 **逐字节一致**：`gcn_e2e_v3.cpp`、`amx_tfs_v3.cpp`、`mkl_baseline.cpp` 的 SHA-256 分别为 `85c51bde92792f98223e80107e20b042f8f8d0e40068cdf6ce168143f0c7743f`、`4e574786055816df80beb6e68181fe19a8ee02cf4be1d43717bc1e79b88cab29`、`00d330db4b5243f4fac5c5beccc2a06bc9199afa4724ca6a8d9babfa717b2295`。服务器只读原件 `/data/yx/TFS/code` 与 wzh 冻结副本也通过 `cmp`。

原版 `mkl_baseline.cpp` 使用 `mkl_sparse_s_mm` 和 `cblas_sgemm`，均传递 FP32 数据；原版 `gcn_e2e_v3.cpp` 中的两层 oneMKL 部分使用同样的函数。原版 TFS 使用 `_tile_dpbf16ps`（BF16 操作数、FP32 累加）。

在 **qhcn266、32 个物理核、同一 socket** 上，对原版独立 oneMKL 程序及包含 TFS 的原版两层程序，以进程范围 `perf stat` 采集 `amx_ops_retired.bf16:u`。两张图、两次重复、单独采集这个事件且计数器运行覆盖率 100%：

| 图 | 原版 oneMKL 单独进程（重复 1 / 2） | 原版两层程序，含 TFS 正对照（重复 1 / 2） |
|---|---:|---:|
| email-Enron | 0 / 0 | 45,330,202,624 / 45,330,202,624 |
| web-Google | 0 / 0 | 586,134,585,344 / 586,134,585,344 |

这个 PMU 事件报告的是 **AMX BF16 操作计数**，不能解释成 `_tile_dpbf16ps` 指令条数。原版两层程序同时运行 TFS 和 oneMKL，因此它是计数器正对照；独立 oneMKL 进程没有执行 TFS。相同结论也出现在同时采集 5 个事件的首轮实验中，但首轮事件被轮换，运行覆盖率约 80%；主结论采用上表 100% 覆盖率的单事件复测。两轮所有 16 个进程退出码均为 0。

**在上述原源码、oneMKL 版本、硬件和图上，原 oneMKL FP32 基线没有执行 AMX-BF16 操作。** `MKL_VERBOSE` 首行列出 CPU/库支持 AMX，但实际打印的是 `SGEMM(...)` 调用；不能把能力列表当成这个调用使用 AMX 的证据。本结论是对该事件及这些运行成立；未声称排除所有可能的 AMX 指令类型或所有 oneMKL 版本。此前 FULL 相对 oneMKL 的速度比是原论文式混合精度比较，不能代表对 oneMKL BF16/AMX GEMM 的胜幅。

## 运行与证据

- 作业：`10971508`（五事件首轮，47 秒）和 `10971873`（单事件复测，18 秒），均 `COMPLETED 0:0`，节点均为 `qhcn266`，普通 `intel` 分区。
- 编译：Intel `icpx 2024.1.0`；原始 oneMKL 程序使用源码自身给出的 `-O3 -march=sapphirerapids -qopenmp -qmkl=parallel`；两层正对照另加 `-mamx-bf16 -mamx-tile -mavx512bf16`。运行日志显示 oneMKL `2023.0 Update 2 Product build 20230613`。
- 绑核：`OMP_NUM_THREADS=32`，`OMP_PROC_BIND=close`，`OMP_PLACES=cores`，物理 CPU `32–63`；默认 NUMA policy。`perf_event_paranoid=2`，用户态事件 `:u`。
- 图：只读 `/home/huangjianqiang_group/hdacp1/data/yx/TFS/data/{email-Enron,web-Google}`；SHA-256 见 `graph.sha256`。
- 服务器原始目录：`/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/onemkl-amx-audit-20261015-074005`，其中 `single_event/` 为无轮换复测；目录保存原始 stdout、stderr、`perf stat` CSV、Slurm 日志、环境、CPU/NUMA、源文件与二进制哈希。
- 本仓库 `summary.csv` 逐进程记录两个测量模式的事件值与计数器运行覆盖率；`multi_event/logs/` 和 `single_event/logs/` 保存原始 `perf stat` CSV。完整且未改写的 stdout/stderr（包括 `MKL_VERBOSE`）保留在上述服务器原始目录。

## 后续基线缺口

原源码没有调用 oneMKL 的 BF16 GEMM API，因此仍需另建 **BF16 oneMKL + AMX** 对照。该对照必须明确 SpMM 的输入表示、聚合后是否量化、BF16 GEMM 转换开销和两层数值误差；不能仅把 `cblas_sgemm` 改成 BF16 GEMM 后直接沿用现在的 FP32 oneMKL 加速比。
