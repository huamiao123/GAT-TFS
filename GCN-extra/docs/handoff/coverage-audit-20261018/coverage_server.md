# 服务器实验覆盖核查：从10月4日起逐级对账

本报告只证明目录、事件、特定内容副本的覆盖，不证明所有实验版本都已完成源码与原始样本深审。服务器根目录：`/home/huangjianqiang_group/hdacp1/data/wzh/GCN-extra/`；GitHub固定提交：`9369b9e0491d59e328a99153b6d97314c62285da`。

本轮只读，无编译、kernel执行、Slurm提交或远端写入。读取了当前wzh/AGENTS.md和tfs-research-engineering/SKILL.md。

实际清单：36,052个服务器文件，33个experiments直接目录、140个runs直接容器、17个results直接目录、438条事件/437个不同事件ID。GitHub HEAD为16,442文件、14个experiments直接目录、29个runs直接目录、15个results直接目录。19个experiment源码族搬入了crosslayer归档，不能只数一级results。

140个run容器也不是140次独立科学实验：其中publication-crosslayer-20261013-review单独含13,308份发布副本。438条事件含构建、提交、执行、失败、分析、交接；recompute-audit-prepared-20261008的ID重复两次。

定向内容对账：856个文本文件覆盖全部33个工作experiments文本、P2分析v1/v2及选定历史source_snapshot；827份能在HEAD找到逐字相同副本，29份不能。全库非二进制哈希尝试180秒超时，没有完整结果，不计入验证。

补审状态更新：10/4见[coverage_oct04.md](coverage_oct04.md)，已完成初始S/M 2720与最终feature 3600等原始计时核对并纠正首触误读；10/7–8八族见[coverage_oct07.md](coverage_oct07.md)，13 run/62 job/37110主计时已全量补核。两份子审保留其自身PMU、汇编、真实CSR重放与因果限制。本台账不再将这些已完成链标成“只看事件”。

## 逐日事件（中国时区；不是实验数量）

|日期|全部事件|构建|提交|执行记录|分析/报告|控制/问题|准备/其他|引用不同job标识|
|---|---:|---:|---:|---:|---:|---:|---:|---:|
|2026-10-04|67|10|24|22|7|2|2|24|
|2026-10-05|42|7|13|13|5|3|1|13|
|2026-10-07|135|12|49|49|10|7|8|49|
|2026-10-08|135|18|25|51|17|6|18|36|
|2026-10-09|1|0|0|0|1|0|0|0|
|2026-10-13|6|1|1|2|1|0|1|2|
|2026-10-14|30|5|4|9|6|0|6|9|
|2026-10-15|21|0|0|20|0|1|0|21|
|2026-10-18|1|0|0|0|1|0|0|0|

分类规则保存在coverage_server_daily.py与coverage_server_event_classification.json；执行记录可含本地轻量运行、数值、结构分析或失败，不可读作“成功性能实验”。job标识可能是build-163149，不是全为Slurm编号。

## 36个组织单元逐项定位

路径均相对上述server root/HEAD的GCN-extra。旧附件一栏仅引述其覆盖范围，不为未列出的版本背书；coverage_oct04与coverage_oct07另有负责审阅。

|单元|服务器canonical相对路径|HEAD路径|旧附件|本轮与待补|
|---|---|---|---|---|
|access_prefetch_20261008|`experiments/access_prefetch_20261008/`|`results/crosslayer-fusion-20261008/experiments/access_prefetch_20261008/`|audit_crosslayer.md|coverage_oct07.md：源码版本、失败链、全量保存主计时已补核；已补核1 run/7 job/2730主计时、PF0/1/8目标多重集和scope前奏。剩余缓存层级/MLP因果及最优预取距离；未搜索最优距离。 见[coverage_oct07.md](coverage_oct07.md)。|
|avx_amx_overlap_p2_20261005|`experiments/avx_amx_overlap_p2_20261005/`|`experiments/avx_amx_overlap_p2_20261005/`|audit_projection.md|本轮仅目录/事件/定向hash；P2额外核v1/v2；失败/修复链补审见coverage_misc_versions（进行中，以交付报告为准）；本轮已核P2 v1/v2 summary相同、decoder修复事件；旧analyze_overlap.py与HEAD无同内容副本。历史v1误解码不能当独立性能重跑。 补审任务见coverage_misc_versions，未交付前不标为完成。|
|delayed_residual_gate_20261005|`experiments/delayed_residual_gate_20261005/`|`experiments/delayed_residual_gate_20261005/`|audit_projection.md|已由coverage_misc_versions.md补核实际启动/编译/取消记录、源码差异与已有graph结果；P3启动失败、P4新增混合策略、FULL部分完成与packed修复分别记录。FULL旧失败launcher完整副本缺失限制保留。|
|event_trace_20261008|`experiments/event_trace_20261008/`|`results/crosslayer-fusion-20261008/experiments/event_trace_20261008/`|audit_crosslayer.md|coverage_oct07.md：源码版本、失败链、全量保存主计时已补核；已补核1 run/7 job/1680主计时、builder/validator/三replay及malformed gates。剩余本机全真实CSR独立replay和完整codegen比较。 见[coverage_oct07.md](coverage_oct07.md)。|
|feature_precision_8aeef16|`experiments/feature_precision_8aeef16/`|`experiments/feature_precision_8aeef16/`|audit_projection.md|coverage_oct04.md：三个build、取消链、最终3600计时及配置全量核对完成；最终native FP32 parallel-static copy已确认，撤回旧附件首触误读；早期取消partial不作正式性能统计。页面完全一致/纯decode成本仍未证明。见[coverage_oct04.md](coverage_oct04.md)。|
|frontier_structure_20261008|`experiments/frontier_structure_20261008/`|`results/crosslayer-fusion-20261008/experiments/frontier_structure_20261008/`|audit_crosslayer.md|已由coverage_late_versions.md补核52容器的实际错误/源码版本及372对原始时间；01生命周期模型等实质变更与工程失败分开。缺233639构建日志、batch外层验收脚本及独立CSR/PMU因果验证的限制保留。|
|full_degree_sort_ablation_20261015|`experiments/full_degree_sort_ablation_20261015/`|`experiments/full_degree_sort_ablation_20261015/`|audit_order_baselines.md|已由coverage_misc_versions.md补核实际启动/编译/取消记录、源码差异与已有graph结果；P3启动失败、P4新增混合策略、FULL部分完成与packed修复分别记录。FULL旧失败launcher完整副本缺失限制保留。|
|grouping_balance_20261007|`experiments/grouping_balance_20261007/`|`results/crosslayer-fusion-20261008/experiments/grouping_balance_20261007/`|audit_crosslayer.md|coverage_oct07.md：源码版本、失败链、全量保存主计时已补核；已补核1 run/8 job/8100主计时。剩余17图真实CSR minhash/LRU机会统计独立重放与真实缓存成因。 见[coverage_oct07.md](coverage_oct07.md)。|
|h_panel_reuse_20261014|`experiments/h_panel_reuse_20261014/`|`experiments/h_panel_reuse_20261014/`|audit_order_baselines.md|本轮目录/事件/定向hash；旧附件覆盖该族关键实现/结果；本轮未新增所有source_snapshot、失败重跑和全部raw样本逐版本深审证明。|
|journal_decoupling_a3_20261004|`experiments/journal_decoupling_a3_20261004/`|`experiments/journal_decoupling_a3_20261004/`|audit_projection.md|coverage_oct04.md：三次build diff、smoke、来源链完成；2890计时继承上一轮全量对账；A3与独立S/M的2720计时已分清；没有逐条人工解释全部PMU/profile或重跑硬件因果。见[coverage_oct04.md](coverage_oct04.md)。|
|journal_followup_p3_p4_20261005|`experiments/journal_followup_p3_p4_20261005/`|`experiments/journal_followup_p3_p4_20261005/`|audit_projection.md|已由coverage_misc_versions.md补核实际启动/编译/取消记录、源码差异与已有graph结果；P3启动失败、P4新增混合策略、FULL部分完成与packed修复分别记录。FULL旧失败launcher完整副本缺失限制保留。|
|journal_grouping_p1_20261005|`experiments/journal_grouping_p1_20261005/`|`experiments/journal_grouping_p1_20261005/`|audit_projection.md|本轮仅目录/事件/定向hash；P2额外核v1/v2；失败/修复链补审见coverage_misc_versions（进行中，以交付报告为准）；smoke/synthetic/focus/validation均已在旧附件概述；每个source snapshot和原始逐线程timer未逐行人工阅读。 补审任务见coverage_misc_versions，未交付前不标为完成。|
|onemkl_amx_audit_20261015|`experiments/onemkl_amx_audit_20261015/`|`experiments/onemkl_amx_audit_20261015/`|audit_order_baselines.md|本轮目录/事件/定向hash；失败/修复链补审见coverage_misc_versions（进行中，以交付报告为准）；旧附件覆盖该族关键实现/结果；本轮未新增所有source_snapshot、失败重跑和全部raw样本逐版本深审证明。 补审任务见coverage_misc_versions，未交付前不标为完成。|
|onemkl_bf16_interleave_20261015|`experiments/onemkl_bf16_interleave_20261015/`|`experiments/onemkl_bf16_interleave_20261015/`|audit_order_baselines.md|本轮目录/事件/定向hash；失败/修复链补审见coverage_misc_versions（进行中，以交付报告为准）；旧附件覆盖该族关键实现/结果；本轮未新增所有source_snapshot、失败重跑和全部raw样本逐版本深审证明。 补审任务见coverage_misc_versions，未交付前不标为完成。|
|onemkl_bf16_twostep_20261015|`experiments/onemkl_bf16_twostep_20261015/`|`experiments/onemkl_bf16_twostep_20261015/`|audit_order_baselines.md|本轮目录/事件/定向hash；失败/修复链补审见coverage_misc_versions（进行中，以交付报告为准）；旧附件覆盖该族关键实现/结果；本轮未新增所有source_snapshot、失败重跑和全部raw样本逐版本深审证明。 补审任务见coverage_misc_versions，未交付前不标为完成。|
|onemkl_bf16_twostep_packed_20261015|`experiments/onemkl_bf16_twostep_packed_20261015/`|`experiments/onemkl_bf16_twostep_packed_20261015/`|audit_order_baselines.md|已由coverage_misc_versions.md补核实际启动/编译/取消记录、源码差异与已有graph结果；P3启动失败、P4新增混合策略、FULL部分完成与packed修复分别记录。FULL旧失败launcher完整副本缺失限制保留。|
|order_balance_20261007|`experiments/order_balance_20261007/`|`results/crosslayer-fusion-20261008/experiments/order_balance_20261007/`|audit_crosslayer.md|coverage_oct07.md：源码版本、失败链、全量保存主计时已补核；已补核2 run/5 job/7410主计时，含两层3×3顺序与两形状。剩余全PMU线程计数重算及实际应用精度。 见[coverage_oct07.md](coverage_oct07.md)。|
|order_share_factorial_20261007|`experiments/order_share_factorial_20261007/`|`results/crosslayer-fusion-20261008/experiments/order_share_factorial_20261007/`|audit_crosslayer.md|coverage_oct07.md：源码版本、失败链、全量保存主计时已补核；已补核1 run/7 job/2730主计时、三个预定seed及推广门槛。剩余全seed统计显著性与更多真实图；原推广门槛未过。 见[coverage_oct07.md](coverage_oct07.md)。|
|ordered_source_plan_20261014|`experiments/ordered_source_plan_20261014/`|`experiments/ordered_source_plan_20261014/`|audit_crosslayer.md|本轮目录/事件/定向hash；旧附件覆盖该族关键实现/结果；本轮未新增所有source_snapshot、失败重跑和全部raw样本逐版本深审证明。|
|pair_order_control_20261007|`experiments/pair_order_control_20261007/`|`results/crosslayer-fusion-20261008/experiments/pair_order_control_20261007/`|audit_crosslayer.md|coverage_oct07.md：源码版本、失败链、全量保存主计时已补核；已补核1 run/3 job/450主计时及同地址/同成员顺序控制。剩余硬件成因与真实图泛化；本轮实验仅synthetic。 见[coverage_oct07.md](coverage_oct07.md)。|
|pair_plan_20261007|`experiments/pair_plan_20261007/`|`results/crosslayer-fusion-20261008/experiments/pair_plan_20261007/`|audit_crosslayer.md|coverage_oct07.md：源码版本、失败链、全量保存主计时已补核；已补核1 run/12 job/8850主计时，含动态/静态exact、真实/合成图cohort。剩余17图配对元数据本地重建及完整汇编因果审查。 见[coverage_oct07.md](coverage_oct07.md)。|
|partial_bundle_20261013|`experiments/partial_bundle_20261013/`|`experiments/partial_bundle_20261013/`|audit_crosslayer.md|本轮目录/事件/定向hash；旧附件覆盖该族关键实现/结果；本轮未新增所有source_snapshot、失败重跑和全部raw样本逐版本深审证明。|
|recompute_audit_20261008|`experiments/recompute_audit_20261008/`|`results/crosslayer-fusion-20261008/experiments/recompute_audit_20261008/`|audit_crosslayer.md|已由coverage_late_versions.md补核52容器的实际错误/源码版本及372对原始时间；01生命周期模型等实质变更与工程失败分开。缺233639构建日志、batch外层验收脚本及独立CSR/PMU因果验证的限制保留。|
|recompute_window_20261008|`experiments/recompute_window_20261008/`|`results/crosslayer-fusion-20261008/experiments/recompute_window_20261008/`|audit_crosslayer.md|已由coverage_late_versions.md补核52容器的实际错误/源码版本及372对原始时间；01生命周期模型等实质变更与工程失败分开。缺233639构建日志、batch外层验收脚本及独立CSR/PMU因果验证的限制保留。|
|recompute_window_adaptive_20261008|`experiments/recompute_window_adaptive_20261008/`|`results/crosslayer-fusion-20261008/experiments/recompute_window_adaptive_20261008/`|audit_crosslayer.md|已由coverage_late_versions.md补核52容器的实际错误/源码版本及372对原始时间；01生命周期模型等实质变更与工程失败分开。缺233639构建日志、batch外层验收脚本及独立CSR/PMU因果验证的限制保留。|
|recompute_window_batch_20261008|`experiments/recompute_window_batch_20261008/`|`results/crosslayer-fusion-20261008/experiments/recompute_window_batch_20261008/`|audit_crosslayer.md|已由coverage_late_versions.md补核52容器的实际错误/源码版本及372对原始时间；01生命周期模型等实质变更与工程失败分开。缺233639构建日志、batch外层验收脚本及独立CSR/PMU因果验证的限制保留。|
|recompute_window_interleave_20261008|`experiments/recompute_window_interleave_20261008/`|`results/crosslayer-fusion-20261008/experiments/recompute_window_interleave_20261008/`|audit_crosslayer.md|已由coverage_late_versions.md补核52容器的实际错误/源码版本及372对原始时间；01生命周期模型等实质变更与工程失败分开。缺233639构建日志、batch外层验收脚本及独立CSR/PMU因果验证的限制保留。|
|recompute_window_kernel_20261008|`experiments/recompute_window_kernel_20261008/`|`results/crosslayer-fusion-20261008/experiments/recompute_window_kernel_20261008/`|audit_crosslayer.md|已由coverage_late_versions.md补核52容器的实际错误/源码版本及372对原始时间；01生命周期模型等实质变更与工程失败分开。缺233639构建日志、batch外层验收脚本及独立CSR/PMU因果验证的限制保留。|
|recompute_window_parallel_20261008|`experiments/recompute_window_parallel_20261008/`|`results/crosslayer-fusion-20261008/experiments/recompute_window_parallel_20261008/`|audit_crosslayer.md|已由coverage_late_versions.md补核52容器的实际错误/源码版本及372对原始时间；01生命周期模型等实质变更与工程失败分开。缺233639构建日志、batch外层验收脚本及独立CSR/PMU因果验证的限制保留。|
|source_partition_audit_20261008|`experiments/source_partition_audit_20261008/`|`results/crosslayer-fusion-20261008/experiments/source_partition_audit_20261008/`|audit_crosslayer.md|已由coverage_late_versions.md补核52容器的实际错误/源码版本及372对原始时间；01生命周期模型等实质变更与工程失败分开。缺233639构建日志、batch外层验收脚本及独立CSR/PMU因果验证的限制保留。|
|source_partition_hot_20261008|`experiments/source_partition_hot_20261008/`|`results/crosslayer-fusion-20261008/experiments/source_partition_hot_20261008/`|audit_crosslayer.md|已由coverage_late_versions.md补核52容器的实际错误/源码版本及372对原始时间；01生命周期模型等实质变更与工程失败分开。缺233639构建日志、batch外层验收脚本及独立CSR/PMU因果验证的限制保留。|
|source_partition_weighted_20261008|`experiments/source_partition_weighted_20261008/`|`results/crosslayer-fusion-20261008/experiments/source_partition_weighted_20261008/`|audit_crosslayer.md|已由coverage_late_versions.md补核52容器的实际错误/源码版本及372对原始时间；01生命周期模型等实质变更与工程失败分开。缺233639构建日志、batch外层验收脚本及独立CSR/PMU因果验证的限制保留。|
|state_reuse_20261007|`experiments/state_reuse_20261007/`|`results/crosslayer-fusion-20261008/experiments/state_reuse_20261007/`|audit_crosslayer.md|coverage_oct07.md：源码版本、失败链、全量保存主计时已补核；已补核01编译、02/03 subnormal、04 NaN失败到05修复；5 run/13 job/5160主计时。剩余完整热循环反汇编逐指令审查及完整输入精度证明。 见[coverage_oct07.md](coverage_oct07.md)。|
|paper-method|`runs/paper-method-*`|`src/paper_methods_kernels.hpp`|audit_projection.md|coverage_oct04.md：源码、控制/取消/构建来源链、正式2720、含smoke3040计时全量对账完成；主要来源与raw已补核；没有人工解释所有重复PMU/profile，没有重跑性能/证明硬件因果。见[coverage_oct04.md](coverage_oct04.md)。|
|paper-cache-control|`runs/paper-cache-*`|`scripts/build_paper_cache_control.sh`|audit_projection.md|coverage_oct04.md：源码、控制/取消/构建来源链、正式850、含smoke950计时全量对账完成；主要来源与raw已补核；没有人工解释所有重复PMU/profile，没有重跑性能/证明硬件因果。见[coverage_oct04.md](coverage_oct04.md)。|
|projection-window-independent|`runs/projection-window-*`|`src/projection_window_kernels.hpp`|audit_projection.md|coverage_oct04.md：源码、控制/取消/构建来源链、正式2720计时全量对账完成；主要来源与raw已补核；没有人工解释所有重复PMU/profile，没有重跑性能/证明硬件因果。见[coverage_oct04.md](coverage_oct04.md)。|

## 已确认的版本差异与边界

1. 10/4独立projection-window结果含17图、2720计时、1020数值gate、408同scope逐位gate、476profile逐位gate；后续A3/P0的2890条不能替代。当前GitHub仅保留该reconcile容器5文件，服务器26文件。coverage_oct04已完成17份原始methods.log与2720条表全量对账，零缺失。
2. feature-precision 17:02/21:29旧runtime与21:40修正版不同。21:29轮因FP32串行first-touch造成placement混杂被取消接受；原始结果保留而不得做强性能分母。旧runtime/driver/protocol没有HEAD逐字相同副本。
3. P2 results/p2-overlap-20261005和-v2为同一次性能记录的不同反汇编审计。已验证summary对象完全相同；v2用iced-x86修复objdump2.30不识别AMX的问题，不是另一轮性能实测。HEAD保留v2内容，旧analysis脚本仍只在服务器。
4. 10/7 state-reuse有01编译宏失败、02/03 subnormal语义失败、04 NaN语义失败、05最终通过。coverage_oct07已逐版补核这条链，并与另七族共37110计时全量对账。仍未重做全部热循环反汇编和全真实CSR本机重放。
5. 10/15 full-degree-sort两次smoke失败分别是固定32核affinity断言、节点缺rg日志提取；packed oneMKL首次构建用错CblasBIdentifier（应为CblasBMatrix）。失败不是性能反例，也不应被成功轮吞掉。

## HEAD没有逐字副本的29个定向检查文件

这不是全库未上传文件总数，只是856项内容对账的确定差集。相同内容可能在多个旧文件中重复；机器清单保存每项SHA256。

- `runs/feature-precision-ablation-20261004-170251/source_snapshot/feature_precision_runtime.hpp`
- `runs/feature-precision-ablation-20261004-170251/source_snapshot/generate_driver.py`
- `runs/feature-precision-ablation-20261004-170251/source_snapshot/build_feature_precision.sh`
- `runs/feature-precision-ablation-20261004-170251/source_snapshot/PROTOCOL.md`
- `runs/feature-precision-ablation-20261004-170251/source_snapshot/feature_precision_job.sh`
- `runs/feature-precision-ablation-20261004-170251/source_snapshot/feature_precision_driver.cpp`
- `runs/feature-precision-ablation-20261004-170251/source_snapshot/source_state.json`
- `runs/onemkl-bf16-twostep-packed-20261015-163149/source_snapshot/driver.cpp`
- `runs/projection-window-build-20261004-161106/source_snapshot/generate_projection_methods.py`
- `runs/projection-window-build-20261004-161106/source_snapshot/projection_methods.cpp`
- `runs/projection-window-build-20261004-161106/source_snapshot/build_projection_methods.sh`
- `runs/projection-window-build-20261004-161106/source_snapshot/generation.json`
- `runs/projection-window-build-20261004-161106/source_snapshot/projection_window_runtime.hpp`
- `runs/feature-precision-ablation-20261004-212957/source_snapshot/generate_driver.py`
- `runs/feature-precision-ablation-20261004-212957/source_snapshot/feature_precision_driver.cpp`
- `runs/feature-precision-ablation-20261004-212957/source_snapshot/PROTOCOL.md`
- `runs/feature-precision-ablation-20261004-212957/source_snapshot/source_state.json`
- `runs/feature-precision-ablation-20261004-212957/source_snapshot/build_feature_precision.sh`
- `runs/feature-precision-ablation-20261004-212957/source_snapshot/feature_precision_runtime.hpp`
- `runs/feature-precision-ablation-20261004-214015/source_snapshot/feature_precision_driver.cpp`
- `runs/projection-window-build-20261004-162142/source_snapshot/projection_methods.cpp`
- `runs/projection-window-build-20261004-162142/source_snapshot/generation.json`
- `experiments/partial_bundle_20261013/prepare_event.json`
- `experiments/avx_amx_overlap_p2_20261005/prepare_launch.py`
- `experiments/avx_amx_overlap_p2_20261005/analysis-v2.log`
- `experiments/ordered_source_plan_20261014/prepare_event.json`
- `experiments/ordered_source_plan_20261014/encoding_repair_event.json`
- `results/p2-overlap-20261005/analyze_overlap.py`
- `results/p2-overlap-20261005/artifact_hashes.json`

## 失败/修复链及补审状态

事件的存在、路径和记录问题均已核实。state/pair链已由coverage_oct07补核，feature取消链由coverage_oct04补核；其余10/8版本与10/5、15版本分别等待coverage_late_versions、coverage_misc_versions交付。下表显式区分，不能仅据事件文本宣称全部compile.log或逐样本已审。

|事件|状态|证据|补审状态/事件自述问题（节选）|
|---|---|---|---|
|feature-precision-ablation-20261004-212957-serial-first-touch|CANCELLED|`runs/feature-precision-ablation-20261004-212957/logs/diagnostic-10867476`|已补核：[coverage_oct04.md](coverage_oct04.md)。Raw H0 FP32 serial first touch differs from BF16 parallel-static initial preparation; Products native FP32 kernel ~1100 ms while matched lossless-quantized FP32 with parallel-static first touch ~240 ms. Native contrast confounded by placement. Produc|
|logs|FAILED|`runs/delayed-residual-gate-20261005-112010/logs`|待coverage_misc_versions交付。已收到源码/日志补核纠正：本次为preflight路径错误、kernel未运行；下列“D1 expected”只是错误事件自述，D1反例出自112212成功运行。D1 expected to fail frozen FP32 accurate numerical gate; BF16 final may hide it. D2 only checked on this counterexample, not general validation.|
|state-reuse-20261007-01-build|FAILED|`runs/state-reuse-20261007-01`|已补核：[coverage_oct07.md](coverage_oct07.md)。Isolated immutable build; 128x128x128; default NUMA; no new major issue if PASS|
|state-reuse-build01-macro-fix-20261007|RESOLVED_FOR_REBUILD|`runs/state-reuse-20261007-01`|已补核：[coverage_oct07.md](coverage_oct07.md)。Original source defines TR macro; namespace qualification expanded to ::16 in new pair bound check. Changed isolated working header to use existing TR macro. Failed frozen snapshot retained. No numerical gate or algorithm changed.|
|state-reuse-20261007-02-state-10888776|FAILED|`runs/state-reuse-20261007-02/logs/state-10888776`|已补核：[coverage_oct07.md](coverage_oct07.md)。Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched|
|state-reuse-20261007-03-state-10888795|FAILED|`runs/state-reuse-20261007-03/logs/state-10888795`|已补核：[coverage_oct07.md](coverage_oct07.md)。Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched|
|state-reuse-semantic03-rootcause-20261007|ROOT_CAUSE_CONFIRMED|`runs/state-reuse-20261007-03/logs/state-10888795`|已补核：[coverage_oct07.md](coverage_oct07.md)。No-inline semantic fixture still fails only FP32 subnormals 0x00010000/0x80010000: fused BF16 0x0000/0x8000 vs original 0x0001/0x8001, MXCSR0x9fe1. icpx folds floating compare/select to vmaxps in production epilogue; original compare+conditional stor|
|state-reuse-20261007-04-state-10888814|FAILED|`runs/state-reuse-20261007-04/logs/state-10888814`|已补核：[coverage_oct07.md](coverage_oct07.md)。Matched in-process controls; default NUMA; preserve rejects and all raw samples; no other jobs touched|
|state-reuse-semantic04-nan-20261007|INVESTIGATING_COMPILER_FP_ASSUMPTIONS|`runs/state-reuse-20261007-04/logs/state-10888814`|已补核：[coverage_oct07.md](coverage_oct07.md)。Integer masking fixes subnormal preservation but vector NaN becomes zero under inherited icpx optimization. Tail NaN remains preserved. No numerical/performance claims from these runs. Will constrain only epilogue comparison semantics without changin|
|pair-plan-analysis01-20261007|FAILED|`results/pair_plan_20261007/analysis-01`|已补核：[coverage_oct07.md](coverage_oct07.md)。Independent audit of available records; pending jobs retained and excluded. 2|
|frontier-certificate-20261008-10899017|FAILED|`runs/frontier-certificate-20261008-01/compute-10899017`|待coverage_late_versions交付。Restricted state lower bound only; fixed node-ID order. No hardware speedup or unrestricted optimality claim.|
|frontier-certificate-link-failure-reviewed-10899017|FAILED_BUILD_RETAINED|`runs/frontier-certificate-20261008-01/compute-10899017`|待coverage_late_versions交付。Build failed before numerical or graph runs: icpx with GCC 8 libstdc++ requires explicit -lstdc++fs for std::filesystem. compile_certificate.stderr.log contains unresolved filesystem symbols. No source/kernel logic changed.|
|recompute-audit-20261008-10899639|FAILED|`runs/recompute-audit-20261008-01/compute-10899639`|待coverage_late_versions交付。Logical recomputation-cost audit only; no speedup or DRAM claim.|
|recompute-audit-20261008-10899667|FAILED|`runs/recompute-audit-20261008-02/compute-10899667`|待coverage_late_versions交付。Logical recomputation-cost audit only; no speedup or DRAM claim.|
|recompute-audit-20261008-10899674|FAILED|`runs/recompute-audit-20261008-03/compute-10899674`|待coverage_late_versions交付。Logical recomputation-cost audit only; no speedup or DRAM claim.|
|recompute-audit-cancelled-10899674|CANCELLED_IMPLEMENTATION|`runs/recompute-audit-20261008-03/compute-10899674`|待coverage_misc_versions交付。Python per-edge loop was too slow for the 903M-edge graph; no audit JSON was produced. Cancelled after 9 minutes before numerical output, preserving logs.|
|recompute-window-20261008-10899982|FAILED|`runs/recompute-window-20261008-02/compute-10899982`|待coverage_late_versions交付。Logical source reuse-window audit only; no speedup or DRAM claim.|
|recompute-window-kernel-build-20261008-193715|FAILED|`runs/recompute-window-kernel-build-20261008-193715`|待coverage_late_versions交付。Isolated single-thread shadow prototype; no paper performance claim|
|recompute-window-kernel-build-20261008-193923|FAILED|`runs/recompute-window-kernel-build-20261008-193923`|待coverage_late_versions交付。Isolated single-thread shadow prototype; no paper performance claim|
|recompute-window-parallel-build-20261008-202152|FAILED|`runs/recompute-window-parallel-build-20261008-202152`|待coverage_late_versions交付。Isolated 32-thread shadow prototype; no paper performance claim|
|recompute-window-parallel-build-20261008-211935|FAILED|`runs/recompute-window-parallel-build-20261008-211935`|待coverage_late_versions交付。Isolated 32-thread shadow prototype; no paper performance claim|
|recompute-window-batch-20261008-221757|FAILED|`runs/recompute-window-batch-20261008-221757`|待coverage_late_versions交付。Isolated source-generation packing prototype; exploratory only|
|recompute-window-batch-20261008-222234|FAILED|`runs/recompute-window-batch-20261008-222234`|待coverage_late_versions交付。Isolated source-generation packing prototype; exploratory only|
|recompute-window-batch-20261008-222602|FAILED|`runs/recompute-window-batch-20261008-222602`|待coverage_late_versions交付。Isolated source-generation packing prototype; exploratory only|
|recompute-window-batch-20261008-222837|FAILED|`runs/recompute-window-batch-20261008-222837`|待coverage_late_versions交付。Isolated source-generation packing prototype; exploratory only|
|recompute-window-interleave-build-20261008-233639|FAILED|`runs/recompute-window-interleave-build-20261008-233639`|待coverage_late_versions交付。Interleaved timing shadow prototype; no paper performance claim|
|full-degree-sort-ablation-20261015-113936-smoke-10968050|FAILED|`runs/full-degree-sort-ablation-20261015-113936/logs/smoke-20261015-114039`|待coverage_misc_versions交付。pre-kernel affinity assertion hard-coded 32; smoke allocated 4 physical cores|
|full-degree-sort-ablation-20261015-113936-smoke-10968053|FAILED|`runs/full-degree-sort-ablation-20261015-113936/logs/smoke-20261015-114139`|待coverage_misc_versions交付。tail_37 correctness passed; log extraction failed because rg unavailable on compute node|
|onemkl-bf16-packed-build-163149|FAILED|`runs/onemkl-bf16-twostep-packed-20261015-163149`|待coverage_misc_versions交付。First build used nonexistent CblasBIdentifier; installed oneMKL 2023.2 header uses CblasBMatrix.|
|onemkl-bf16-packed-10973279|CANCELLED|`runs/onemkl-bf16-twostep-packed-20261015-163448/full17_backfill`|待coverage_misc_versions交付。Second shorter queue submission cancelled while pending to avoid duplicate 17-graph jobs.|

## 140个run容器完整索引

HEAD同路径数=0不能直接推断未上传：存在改名、结果精简和crosslayer搬迁。最后一列仅列crosslayer已知搬迁；其他改名归档由内容哈希/来源manifest定位。没有把容器文件数当深审数量。

|run|服务器文件|HEAD同路径文件|HEAD crosslayer搬迁文件|
|---|---:|---:|---:|
|`access-prefetch-20261008-01`|1345|0|1343|
|`avx-amx-overlap-20261005-112746`|43|0|0|
|`cleanup-current-20261004`|8|8|0|
|`delayed-residual-gate-20261005-112010`|35|0|0|
|`delayed-residual-gate-20261005-112212`|41|0|0|
|`event-trace-20261008-01`|1652|0|1650|
|`evidence-export-20261005-115241`|2|0|0|
|`feature-precision-ablation-20261004-170251`|51|0|0|
|`feature-precision-ablation-20261004-212957`|334|0|0|
|`feature-precision-ablation-20261004-214015`|897|0|0|
|`feature-precision-switch-20261004-164044`|1|0|0|
|`frontier-certificate-20261008-01`|41|0|40|
|`frontier-certificate-20261008-02`|169|0|121|
|`frontier-structure-20261008-01`|412|0|324|
|`full-degree-sort-ablation-20261015-113936`|233|0|0|
|`grouping-balance-20261007-01`|1548|0|1539|
|`h-panel-reuse-10964897`|29|0|0|
|`h-panel-reuse-10964915`|32|0|0|
|`h-panel-reuse-10964919`|32|0|0|
|`h-panel-reuse-build-20261014-231427`|18|0|0|
|`h-panel-reuse-build-20261014-231751`|23|0|0|
|`h-panel-reuse-reverse-10964921`|38|0|0|
|`journal-a3-build-20261004-215613`|41|0|0|
|`journal-a3-build-20261004-220554`|185|0|0|
|`journal-a3-build-20261004-221445`|822|0|0|
|`journal-a3-reconciled-20261004-230637`|27|0|0|
|`journal-p1-build-20261005-111037`|967|0|0|
|`journal-p1-reconciled-20261005-115500`|36|0|0|
|`journal-p3-p4-reconciled-20261005-1301`|187|0|0|
|`onemkl-amx-audit-20261015-074005`|103|0|0|
|`onemkl-bf16-interleave-20261015-171946`|77|0|0|
|`onemkl-bf16-twostep-20261015-081637`|130|0|0|
|`onemkl-bf16-twostep-packed-20261015-163149`|16|0|0|
|`onemkl-bf16-twostep-packed-20261015-163228`|38|0|0|
|`onemkl-bf16-twostep-packed-20261015-163448`|123|0|0|
|`order-balance-20261007-01`|46|0|45|
|`order-balance-20261007-02`|637|0|633|
|`order-share-factorial-20261007-01`|1194|0|1192|
|`ordered-source-build-20261014-133737`|193|0|0|
|`ordered-source-detail-build-20261014-135515`|187|0|0|
|`ordered-source-timer-cal-20261014-140724`|31|0|0|
|`pair-order-control-20261007-01`|704|0|702|
|`pair-plan-20261007-01`|2164|0|2149|
|`paper-cache-build-20261004-131154`|24|22|0|
|`paper-cache-high-10864906`|113|113|0|
|`paper-cache-low-10864908`|147|147|0|
|`paper-cache-medium-10864907`|113|113|0|
|`paper-cache-reconciled-20261004`|9|9|0|
|`paper-cache-scheduling-adjustment-20261004-133536`|12|12|0|
|`paper-cache-smoke-10864904`|64|64|0|
|`paper-cache-submit-high-20261004-131618`|17|17|0|
|`paper-cache-submit-low-20261004-131628`|17|17|0|
|`paper-cache-submit-medium-20261004-131623`|17|17|0|
|`paper-cache-submit-smoke-20261004-131545`|17|17|0|
|`paper-method-build-20261004-123445`|27|25|0|
|`paper-method-high-10864874`|123|123|0|
|`paper-method-low-10864876`|161|161|0|
|`paper-method-medium-10864875`|123|123|0|
|`paper-method-publication-20261004`|1|1|0|
|`paper-method-reconciled-20261004`|15|15|0|
|`paper-method-report-qa-20261004`|1|1|0|
|`paper-method-smoke-10864873`|68|68|0|
|`paper-method-submit-high-20261004-123850`|18|18|0|
|`paper-method-submit-low-20261004-123901`|18|18|0|
|`paper-method-submit-medium-20261004-123855`|18|18|0|
|`paper-method-submit-smoke-20261004-123733`|18|18|0|
|`partial-bundle-build-20261013-225110`|171|0|0|
|`projection-window-build-20261004-161106`|28|0|0|
|`projection-window-build-20261004-162142`|33|0|0|
|`projection-window-focus-10867479`|165|0|0|
|`projection-window-reconciled-20261004-222427`|26|5|0|
|`projection-window-smoke-10865904`|125|0|0|
|`projection-window-smoke-10865927`|135|0|0|
|`projection-window-smoke-10867477`|136|0|0|
|`projection-window-submit-focus-20261004-162303`|16|0|0|
|`projection-window-submit-focus-20261004-213446`|18|0|0|
|`projection-window-submit-smoke-20261004-161133`|18|0|0|
|`projection-window-submit-smoke-20261004-162223`|18|0|0|
|`projection-window-submit-smoke-20261004-213400`|18|0|0|
|`projection-window-submit-validation-20261004-162310`|16|0|0|
|`projection-window-submit-validation-20261004-215536`|18|0|0|
|`projection-window-validation-10867699`|453|0|0|
|`publication-crosslayer-20261013-review`|13308|0|0|
|`recompute-audit-20261008-01`|25|0|25|
|`recompute-audit-20261008-02`|26|0|26|
|`recompute-audit-20261008-03`|27|0|27|
|`recompute-audit-20261008-04`|5|0|5|
|`recompute-audit-20261008-05`|54|0|53|
|`recompute-window-20261008-01`|55|0|54|
|`recompute-window-20261008-02`|32|0|32|
|`recompute-window-20261008-03`|55|0|54|
|`recompute-window-20261008-04`|55|0|54|
|`recompute-window-adaptive-20261008-233857`|13|0|13|
|`recompute-window-adaptive-20261008-234255`|13|0|13|
|`recompute-window-adaptive-build-20261008-233758`|19|0|18|
|`recompute-window-adaptive-build-20261008-234221`|19|0|18|
|`recompute-window-batch-20261008-221757`|13|0|13|
|`recompute-window-batch-20261008-222234`|13|0|13|
|`recompute-window-batch-20261008-222602`|13|0|13|
|`recompute-window-batch-20261008-222837`|13|0|13|
|`recompute-window-batch-build-20261008-221327`|19|0|18|
|`recompute-window-interleave-20261008-230519`|13|0|13|
|`recompute-window-interleave-20261008-230957`|13|0|13|
|`recompute-window-interleave-20261008-231443`|13|0|13|
|`recompute-window-interleave-20261008-233649`|13|0|13|
|`recompute-window-interleave-build-20261008-230314`|19|0|18|
|`recompute-window-interleave-build-20261008-233639`|12|0|12|
|`recompute-window-kernel-10900289`|20|0|20|
|`recompute-window-kernel-build-20261008-193715`|15|0|15|
|`recompute-window-kernel-build-20261008-193923`|15|0|15|
|`recompute-window-kernel-build-20261008-194137`|18|0|17|
|`recompute-window-parallel-10900662`|20|0|20|
|`recompute-window-parallel-10901373`|20|0|20|
|`recompute-window-parallel-build-20261008-202152`|16|0|16|
|`recompute-window-parallel-build-20261008-202420`|19|0|18|
|`recompute-window-parallel-build-20261008-211935`|16|0|16|
|`recompute-window-parallel-build-20261008-212208`|19|0|18|
|`recompute-window-parallel-cross-10901444`|21|0|21|
|`recompute-window-parallel-exclusive-10901902`|20|0|20|
|`recompute-window-sweep-10900343`|23|0|23|
|`recompute-window-sweep-10900360`|39|0|39|
|`residual-broad-build-20261005-121707`|229|0|0|
|`selective-shape-build-20261005-122940`|125|0|0|
|`selective-shape-build-20261005-123418`|345|0|0|
|`source-partition-10901309`|18|0|18|
|`source-partition-build-20261008-210228`|17|0|16|
|`source-partition-cross-10901392`|21|0|21|
|`source-partition-cross-10902064`|26|0|26|
|`source-partition-cross-10902071`|22|0|22|
|`source-partition-weighted-20261008-232054`|13|0|13|
|`source-partition-weighted-20261008-232839`|13|0|13|
|`source-partition-weighted-build-20261008-232004`|16|0|15|
|`source-partition-weighted-build-20261008-232709`|16|0|15|
|`source-partition-weighted-heldout-20261008-234650`|13|0|13|
|`source-partition-weighted-roadusa-20261008-235043`|12|0|12|
|`state-reuse-20261007-01`|30|0|30|
|`state-reuse-20261007-02`|162|0|150|
|`state-reuse-20261007-03`|173|0|161|
|`state-reuse-20261007-04`|173|0|161|
|`state-reuse-20261007-05`|1557|0|1545|

机器证据：coverage_server.json（36单元/140容器/29差集/失败链）、server_inventory_raw.json（所有路径及完整events）、coverage_server_inventory.json、coverage_server_event_classification.json、coverage_server_target_hash_matches.json、coverage_server_details.json。
