# 《TFS期刊扩展技术说明》已完成工作的总索引

对应用户桌面的 `TFS期刊扩展技术说明_综合研究方案_20261004.txt`。
实验分支：`gcn-extra-experiments-20261003`。
本页核对的实验内容提交：`bc38db54af69b1a373724d55116cf6286b181b9a`。
本页随后作为文档索引提交；不改变任何内核、计时或数值门槛。

## 1. 原文、调研和基础对照

- [归档的综合研究方案原文](TFS_JOURNAL_STUDY_INPUT_20261004.txt)。
- [源码审阅、真实执行工作模型、相关工作及创新边界](TFS_JOURNAL_RESEARCH_NOTES_20261004.md)。这是研究分析，不是一个新增性能内核。
- [原TFS AMX源码](../original/amx_tfs_v3.cpp)、[原两层TFS/MKL配对harness](../original/gcn_e2e_v3.cpp)、[原独立MKL参考源码](../original/mkl_baseline.cpp)。实际两层比较使用配对harness中的原MKL helper。
- [已有Neighbor-Block Reduction内核](../src/paper_methods_kernels.hpp)、[runtime及数值检查](../src/paper_methods_runtime.hpp)。支持B2/4/8/16/32/64/FULL与FAST/ACCURATE，是后续研究的基础。
- [基础协议](PAPER_METHODS_PROTOCOL_20261004.md)、[基础控制实验结果](PAPER_METHODS_RESULTS_20261004.md)、[连续计时对照](PAPER_CACHE_CONTROL_RESULTS_20261004.md)、[解释](PAPER_METHODS_INTERPRETATION_20261004.md)。这些前置结果与后续轮次不跨节点互作分母。

## 2. 按综合文档方向列出全部已实现实验

| 工作 | 实际做过的内容 | 源码 / 执行入口 | GitHub结果 / 解释 |
|---|---|---|---|
| 配套：feature精度驻留 | B64/FULL × BF16/FP32严格2×2；native两层、matched-input、准备计入/排除、stage、PMU、页分布诊断；17张真实图 | [feature_precision_8aeef16](../experiments/feature_precision_8aeef16/)、[固定源码审计](../experiments/feature_precision_8aeef16/SOURCE_AUDIT.md) | [完整结果目录](../results/feature-precision-20261004/)、[主报告](../results/feature-precision-20261004/GCN_EXTRA_FEATURE_PRECISION_ABLATION.md)、[解释](FEATURE_PRECISION_INTERPRETATION_20261004.md) |
| P0：访问窗口S / 投影范围M解耦 | S32/64/128与M64/256/FULL及原block控制；同范围逐位等价、真实投影计数；17图 | [kernel](../src/projection_window_kernels.hpp)、[runtime](../src/projection_window_runtime.hpp)、[build](../scripts/build_projection_methods.sh)、[submit](../scripts/submit_projection_windows.sh) | [协议](PROJECTION_WINDOW_PROTOCOL_20261004.md)、[独立S/M结果](PROJECTION_WINDOW_RESULTS_20261004.md) |
| P0：A3机制消融 | A0 FULL、A1 S64/MFULL、A2 B64、A3 rowwise；A1/A3保持相同窗口/状态请求/投影预算，改变跨destination顺序；17图及结构探针 | [journal_decoupling_a3_20261004](../experiments/journal_decoupling_a3_20261004/) | [A3完整报告](JOURNAL_A3_RESULTS_20261004.md)、[P0机制解释](JOURNAL_P0_MECHANISM_20261005.md)、[逐次CSV/PMU目录](../results/journal-p0-20261004/) |
| P1：投影预算不变的有限行分组 | 同q64、至多4096行段内source/page min-hash分组及shuffle对照；FULL/B64两范围；17真实图+3受控图，另3smoke；预处理摊销、结构、线程、PMU | [journal_grouping_p1_20261005](../experiments/journal_grouping_p1_20261005/) | [结果目录](../results/journal-p1-20261005/)、[主报告](../results/journal-p1-20261005/JOURNAL_P1_RESULTS_20261005.md)、[解释](JOURNAL_FOLLOWUP_VALIDATION_20261005.md) |
| P2：同核AVX–AMX交错 | SERIAL/INTERLEAVED相同工作量、双buffer；5个工作集/度案例、7次交替成对测量；AMX反汇编解码、插桩逐位、PMU | [avx_amx_overlap_p2_20261005](../experiments/avx_amx_overlap_p2_20261005/) | [结果目录与反汇编](../results/p2-overlap-20261005/)、[报告](../results/p2-overlap-20261005/P2_OVERLAP_RESULTS_20261005.md) |
| P3：D1/D2实际AMX数值反例 | 文档抵消反例；原TFS、B2 ACC、D1单分量、D2双分量、FULL；同时检查FP32及BF16-final；启动失败/修复保留 | [delayed_residual_gate_20261005](../experiments/delayed_residual_gate_20261005/) | [结果/失败证据](../results/p3-residual-gate-20261005/)、[报告](../results/p3-residual-gate-20261005/P3_RESIDUAL_GATE_20261005.md) |
| P3：D2/D3广泛验证 | D2最终hi/lo修正，D3每四个B64块修正；对比原TFS、B64 ACC、S64/FULL ACC；3smoke+5真实图，完整两层、26项Layer1细分、PMU | [residual_broad.hpp](../experiments/journal_followup_p3_p4_20261005/residual_broad.hpp)、[实验目录](../experiments/journal_followup_p3_p4_20261005/) | [冻结源码/全部结果](../results/journal-p3-p4-20261005/p3/)、[总结果](../results/journal-p3-p4-20261005/JOURNAL_P3_P4_RESULTS_20261005.md)、[解释](JOURNAL_P3_P4_INTERPRETATION_20261005.md) |
| P4：选择性投影物化+冷源TFS融合 | source consumer频率选N/64、N/16、N/4；none/all、project_all、project_used及固定L1-project/L2-FULL强对照；128→32→32和128→128→128；5真实图+2受控关系图；双层24项profile、工作量、PMU | [selective_kernels.hpp](../experiments/journal_followup_p3_p4_20261005/selective_kernels.hpp)、[selective_shape.cpp](../experiments/journal_followup_p3_p4_20261005/selective_shape.cpp)、[实验目录](../experiments/journal_followup_p3_p4_20261005/) | [冻结源码/全部结果](../results/journal-p3-p4-20261005/p4/)、[总结果](../results/journal-p3-p4-20261005/JOURNAL_P3_P4_RESULTS_20261005.md)、[解释](JOURNAL_P3_P4_INTERPRETATION_20261005.md) |

P4正式图之前更换了首次smoke的H/W生成分布并增加mixed强对照，新建build重做smoke。
[首次smoke的冻结源码与日志](../results/journal-p3-p4-20261005/superseded_smoke/)
作为superseded过程证据保存，不进入主结果或性能分母。

## 3. 每个方向目前能说什么

| 方向 | 已获得的证据 | 尚不能声称 |
|---|---|---|
| 精度2×2 | 17图完成；多数图BF16较快，低degree例外；未发现可支持联合选择器的稳定粒度方向反转 | 仅凭logical bytes或generic PMU证明带宽/缓存唯一原因，或分类精度更好 |
| P0 | A3轮固定S64/MFULL相对原TFS几何平均1.314×；Myciel A0/A1约2.234×；相同投影预算下访问顺序会显著影响时间 | 所有图通用收益；已有可预测的自适应策略 |
| P1 | Reddit-source、Myciel-page有明显收益；固定page相对同范围DegreeSort平均约1.013×，收益集中 | 单靠局部source reuse即可选出赢家；整个P1状态管理空间已验证 |
| P2 | 确认真实指令交错；重稀疏micro当前版没有收益 | 完整两层流水收益或硬件有效重叠已被因果证明 |
| P3-D1 | 实际AMX反例relative L2约0.001956947，超过1e-3，拒绝 | D1精度不退化 |
| P3-D2/D3 | 本轮160条数值检查通过；五图均未胜过FULL ACC | 任意输入精度保证；D2/D3是有收益的主要优化 |
| P4 | 本轮540条数值检查通过；Myciel压缩1/4热点为491.4ms、FULL662.6ms，但mixed强对照340.3ms | 选择性物化超过所有强对照；其收益已构成独立期刊创新 |

## 4. 计时、PMU、工作量与过程记录在哪里

- P0：[结果目录](../results/journal-p0-20261004/)中的`time/stage/detail/thread/pmu/model/counters/structure`等CSV；[PMU实现](../src/projection_window_pmu.hpp)。独立S/M轮的原始run和完整Slurm/build文件仍在服务器，GitHub公开其协议、源码与结果报告；不要将P0目录的A3轮CSV误称为前一S/M轮原日志。
- P1：[结果目录](../results/journal-p1-20261005/)，包括主/交替计时、setup、结构、线程、PMU及frozen provenance/raw archive。
- P2：[结果目录](../results/p2-overlap-20261005/)，包括serial/interleaved反汇编、二进制hash、decoder issue、全部micro样本与PMU。
- P3反例：[结果目录](../results/p3-residual-gate-20261005/)，包括成功数值日志与failed_launch；D1拒绝未隐藏。
- P3/P4：[结果目录](../results/journal-p3-p4-20261005/)，包括`source_snapshot`、manifest、build command、数值gate、主样本min/max/CV、stage、子阶段、per-thread、per-TID PMU、logical work、raw archive、Slurm/affinity/input hashes及最终hash inventory。
- [实验总账](handoff/ALL_EXPERIMENTS.md)、[问题与修复记录](handoff/EXPERIMENT_ISSUES.md)、[项目进度](handoff/PROJECT_PROGRESS.md)、[机器可读事件](handoff/events.jsonl)。

## 5. 当前未完成的部分

主候选已获得首轮硬件证据，不等于整份期刊研究完成。尚未完成：

- P1无需逐图性能搜索的固定选择规则、预处理成本与保留图验证。
- P1更全面的partial/输出panel/状态驻留联合决策。
- P2完整两层有效流水；当前micro不支持直接接入主kernel。
- P3严格误差预算或误差驱动修正周期；现D3只是固定period4。
- P4同源集合、不同Q布局/map成本的消融；局部容量/生命周期约束及超越全局/逐层强对照的适用区域。
- 更多形状、一般边权同算子比较、真实推理checkpoint与任务精度、稳定独享复测和相关工作专项排重。

当前性能主要为随机H/W、单位边权的两层计算流，prepared边界按各轮协议定义。
原TFS/MKL源码未修改；P4新增shape harness单独标注；不同轮/节点/精度不能交叉作为速度比分母。
