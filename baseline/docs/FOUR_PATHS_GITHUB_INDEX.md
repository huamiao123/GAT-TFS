# 四个版本：源码和独立结果

本索引使用作业 local-10810186 的同一组实验。用户所指“200+ ms 修改后的 B1”为 `local_online_fp32`（arxiv 250.887 ms、products 8221.782 ms），这里标为 **B1 局部聚合改进版 FP32**。原始日志名称保持不变；该版本的精度为 FP32，不能称为 B1 BF16。

| 版本 | 源码 | 独立结果 | arxiv 三层 median ms | products 三层 median ms |
|---|---|---|---:|---:|
| 修改后的 B0 FP32 | [baseline_standard.cpp](../src/baseline_standard.cpp) | [B0_FP32.json](../results/four_paths_10810186/B0_FP32.json) | 73.996 | 3908.668 |
| 修改后的 B0 BF16 | [baseline_standard.cpp](../src/baseline_standard.cpp) | [B0_BF16.json](../results/four_paths_10810186/B0_BF16.json) | 68.437 | 3817.678 |
| B1 修改前：原 TFS BF16 计算顺序 | [baseline_tfs.cpp](../src/baseline_tfs.cpp) | [B1_before_BF16.json](../results/four_paths_10810186/B1_before_BF16.json) | 425.709 | 15807.121 |
| B1 修改后：局部聚合 FP32 | [local_online.cpp](../ours/local_online.cpp)、[接口](../ours/local_online.hpp) | [B1_after_FP32.json](../results/four_paths_10810186/B1_after_FP32.json) | 250.887 | 8221.782 |

B0 两种精度共用一个源码文件，`standard_layer` 的 `fp32` 参数分别取 true/false。五条实际路径的统一运行入口为 [ours/main.cpp](../ours/main.cpp)；其中还含不在上表中的 AMX high/low 后端。

“B1 修改前”指尚未将逐边 `(pH)W` 改为局部 U 聚合后一次 UW 的计算顺序，仍已应用 NUMA 修正。B0 的运行环境修正见 [arxiv_payload.sh](../scripts/arxiv_payload.sh)，本轮统一比较启动脚本为 [local_payload.sh](../scripts/local_payload.sh)。四个版本使用同一 16 核、NUMA interleave、图、随机未训练参数和计时边界。

独立 JSON 从已有日志提取，包含两张图的原始测量、逐层中位数、误差及可用 profile/Online counters；不是新实验，也没有改变任何模型实现。可用 [export_four_paths.py](../scripts/export_four_paths.py) 重新生成。

完整原始结果：[arxiv_local.log](../runs/local-10810186/arxiv_local.log)、[products_local.log](../runs/local-10810186/products_local.log)、[summary.json](../runs/local-10810186/summary.json)。详细精度、计时与限制见 [LOCAL_ONLINE_RESULTS_20260930.md](LOCAL_ONLINE_RESULTS_20260930.md)。

局部聚合 FP32 相对 B1 提速，但仍慢于纯 GAT；products 旧 max_abs=0.003 门槛未通过，任务精度和正式性能验收仍未完成。索引的命名不改变这个状态。
