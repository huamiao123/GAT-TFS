# 原 TFS / MKL 源码对齐复测（2026-10-04）

## 源码核对结论

原 TFS 和原 MKL 的源码、运行脚本均没有显式 NUMA 优化。DegreeSort 是逻辑行调度，不是 NUMA 优化。原脚本只声明 OMP_NUM_THREADS=32、OMP_PROC_BIND=close、OMP_PLACES=cores。
之前的实验给所有路径添加了 numactl interleave，并改过分配方式、随机输入生成顺序、MKL hint/线程控制、统计方式和编译选项；因此那些结果不能称为原源码协议复现。新结果不能与旧表混用，也不能把差异全部归因于 NUMA。

本轮原 E2E 源码完整保留，候选测量插入原计时与精度检查之后；删除两处插入可以逐字符恢复原文件。MKL helper、分配、首次写入、hint=10、输入生成、ReLU、转换函数未改。Reddit 和 Products 另跑未修改原文件编译出的二进制，前 8 个输出与插入版一致。
复现源码明确声明的运行设置与算子行为，不声称复现论文旧机器及其未知继承环境。原单层 MKL 用 H→W 随机顺序，原两层用交错 W1/W2→H0；单层独立程序只作单层对照，不能替代两层 MKL 基线。

## 实测协议

- 运行：source-formal-10864754 / qhcn819；source-formal-10864792 / qhcn819；每个作业一个独享 intel_expr 节点，32 线程。
- 所有路径使用默认 NUMA 策略，无 interleave/bind/membind；实际策略及 CPU 布局已保存。
- 原源码 MKL：FP32 SpMM→SGEMM；TFS/候选：BF16 输入与权重，FP32 累加/输出。数值误差另列，不是等精度竞速。
- 1 次预热、5 次实测，主表取最小值；CSV 另列 median/P25/P75，原 E2E stdout 保留 0.01 ms 精度。
- 两层准备后 E2E：第一层算子→ReLU→中间 BF16 转换→第二层算子；加载、排序、初始转换与权重 packing 在计时外。
- 细分 stage/profile 另跑，采样线程时间不能当作墙钟占比。
- 随机 H/W 的原源码 sum 推理计算流；不是训练后分类模型准确率，也不是包括初始化的应用 E2E。

通过 17 张真实图，765 项数值检查，0 失败；8 张图单列排除。

首轮 supervisor 将 RGG 的 N*128==2^31 临界尺寸误判为越界，原排除记录保留。修正为判断 N*128>2^31 后单独补测；补测未更改 C++ 源码、二进制或原 MKL/TFS 协议。最终合并只替换派生汇总中的该图状态，保留两份作业原始状态和全部日志。

主批次 shell 在 Python 写完 SOURCE_SUITE_COMPLETE、REPORT 和全部 CSV 后退出 2：运行时更新启动脚本导致 Bash 读取了变化的文件尾部。16 个逐图计算进程均正常退出并通过数值门，统计区间已结束，因此保留其有效计算数据，同时明确保留主作业 FAILED 状态。后续提交使用独立冻结的 Slurm/shell/Python 启动快照；修复另以共享正确性作业验证。

## 两层 E2E（ms，五轮最小值）

每图最优 block 是事后选择，仅用于研究上限；固定 block 结果见下表与完整 CSV。

| 图 | 原 MKL FP32 | 原 TFS | 最优 Fast | Fast 耗时 | 对 MKL | 对原 TFS | 对去清零 TFS |
|---|---:|---:|---|---:|---:|---:|---:|
| mycielskian19 | 4781.200 | 2154.040 | shared_b64_fast_nozero | 1094.980 | 4.366× | 1.967× | 1.936× |
| reddit | 438.970 | 307.650 | shared_b64_fast_nozero | 130.304 | 3.369× | 2.361× | 2.359× |
| hollywood-2009 | 385.420 | 362.210 | shared_b64_fast_nozero | 224.257 | 1.719× | 1.615× | 1.490× |
| ogbn-products | 1344.750 | 466.090 | shared_bfull_fast_nozero | 346.496 | 3.881× | 1.345× | 1.133× |
| indochina-2004 | 692.310 | 830.930 | shared_bfull_fast_nozero | 456.482 | 1.517× | 1.820× | 1.295× |
| soc-Pokec | 330.130 | 193.070 | shared_bfull_fast_nozero | 146.108 | 2.259× | 1.321× | 0.971× |
| com-LiveJournal | 643.470 | 499.310 | shared_bfull_fast_nozero | 403.996 | 1.593× | 1.236× | 0.911× |
| rgg_n_2_24_s0 | 2009.950 | 1819.500 | shared_b32_fast_nozero | 1319.887 | 1.523× | 1.379× | 0.978× |
| soc-LiveJournal1 | 679.700 | 594.270 | shared_b64_fast_nozero | 492.390 | 1.380× | 1.207× | 0.869× |
| as-Skitter | 162.800 | 199.670 | shared_b32_fast_nozero | 131.149 | 1.241× | 1.522× | 1.101× |
| email-Enron | 1.420 | 3.180 | shared_bfull_fast_nozero | 1.200 | 1.183× | 2.650× | 1.919× |
| amazon0601 | 25.500 | 38.630 | shared_b16_fast_nozero | 25.123 | 1.015× | 1.538× | 1.011× |
| web-Google | 46.470 | 75.780 | shared_bfull_fast_nozero | 50.655 | 0.917× | 1.496× | 0.925× |
| com-Youtube | 53.600 | 131.290 | shared_bfull_fast_nozero | 81.085 | 0.661× | 1.619× | 1.164× |
| cit-Patents | 270.190 | 312.010 | shared_b32_fast_nozero | 200.753 | 1.346× | 1.554× | 0.952× |
| roadNet-CA | 64.730 | 167.270 | shared_b16_fast_nozero | 109.755 | 0.590× | 1.524× | 0.940× |
| wiki-Talk | 90.580 | 291.180 | shared_bfull_fast_nozero | 151.173 | 0.599× | 1.926× | 1.415× |

## 固定方法：17 张图加速比几何平均

| 方法 | 对原 MKL | 对原 TFS | 对去清零 TFS | 快于原 TFS 的图数 |
|---|---:|---:|---:|---:|
| original_nozero | 1.200× | 1.346× | 1.000× | 17/17 |
| shared_b16_accurate_nozero | 1.302× | 1.461× | 1.085× | 17/17 |
| shared_b16_fast_nozero | 1.355× | 1.520× | 1.129× | 17/17 |
| shared_b2_accurate_nozero | 0.796× | 0.893× | 0.663× | 4/17 |
| shared_b2_fast_nozero | 0.936× | 1.051× | 0.780× | 9/17 |
| shared_b32_accurate_nozero | 1.363× | 1.529× | 1.136× | 17/17 |
| shared_b32_fast_nozero | 1.407× | 1.579× | 1.173× | 17/17 |
| shared_b4_accurate_nozero | 1.017× | 1.141× | 0.848× | 16/17 |
| shared_b4_fast_nozero | 1.130× | 1.267× | 0.942× | 16/17 |
| shared_b64_accurate_nozero | 1.391× | 1.561× | 1.160× | 17/17 |
| shared_b64_fast_nozero | 1.427× | 1.601× | 1.189× | 17/17 |
| shared_b8_accurate_nozero | 1.191× | 1.336× | 0.992× | 17/17 |
| shared_b8_fast_nozero | 1.267× | 1.421× | 1.056× | 17/17 |
| shared_bfull_accurate_nozero | 1.338× | 1.501× | 1.115× | 16/17 |
| shared_bfull_fast_nozero | 1.366× | 1.533× | 1.139× | 16/17 |
| source_mkl_fp32 | 1.000× | 1.122× | 0.833× | 9/17 |
| source_original_tfs | 0.891× | 1.000× | 0.743× | 0/17 |

对去清零 TFS 的比较扣除了移除原冗余全局 memset 的收益，但仍包含跨输出 panel 的 gather 复用、局部归约、AMX 次数变化等；不能把全部收益都归因于投影冗余消减。

## 排除项

- kron_g500-logn21：Previously verified nonunit CSR values; original TFS ignores values while MKL uses them
- com-Friendster：Original LP64 MKL range limit; ILP64 changes are outside source-faithful protocol
- cage15：Previously verified nonunit CSR values; original TFS ignores values while MKL uses them
- FullChip：Previously verified nonunit CSR values; original TFS ignores values while MKL uses them
- scircuit：Previously verified nonunit CSR values; original TFS ignores values while MKL uses them
- sx-stackoverflow：Previously verified nonunit CSR values; original TFS ignores values while MKL uses them
- rajat31：Previously verified nonunit CSR values; original TFS ignores values while MKL uses them
- road_usa：Original LP64 MKL range limit; ILP64 changes are outside source-faithful protocol

## 证据

- 本次完整路径：`/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/runs/source-formal-10864754`。
- `reconciled_summary.csv` / `reconciled_suite_status.json` / `best_per_graph.csv` / `audit_summary.json`；各作业原 `summary.csv` / `suite_status.json` 不覆盖。
- 每图 `stdout.log`、`time.csv`、`correct.csv`、`stage.csv`、`profile.csv`、`parsed.json` 和 exact command。
- `source_snapshot/`、`build_source.sha256`、`binary.sha256`、`dataset.sha256`、环境和 NUMA 观察文件。
- 详细协议：`docs/SOURCE_MKL_PROTOCOL_20261004.md`；生成可恢复性：build generation manifest。
