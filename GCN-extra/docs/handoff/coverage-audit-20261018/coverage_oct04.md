# 10 月 4 日实验覆盖补审与上一轮勘误

固定 GitHub HEAD：`9369b9e0491d59e328a99153b6d97314c62285da`。本轮于 2026-10-18 只读核查本地快照及服务器仍保留的 run，未运行性能实验，未改动实验快照。配套机器台账：`coverage_oct04.json`；共 15 个实验/控制单元、73 条带作用域的 lineage 记录。这里的单元包括数值、计时和归档控制，不能视为 15 个新算法；73 条也包含 build/submit/reconcile，不能当作独立性能实验数。

## 1. 上一轮覆盖声明应如何纠正

上一轮审到了最终主线的主要实现与结果，但没有完整追完 10 月 4 日的取消、修订、重建和最初 S/M 独立全图轮次，不能称为“所有历史实验都看完”。现在能明确区分：

1. 原始 B/FAST/ACCURATE 主轮：17 图，2,720 个正式计时；另有两个小图 smoke。
2. 紧接的连续计时/cache-control：17 图，850 个正式计时；另有两个 smoke。
3. 第一轮 S/M 解耦：17 图，2,720 个计时，**不是** journal-p0 的 A3 数据。
4. A3 行序机制对照：重新跑 17 图，2,890 个计时；另有中间 build 和 smoke。
5. feature precision：早期串行首触混杂轮被取消，然后重建 parallel-static-copy 版本，5 图 diagnostic 加 12 图 extension；三个 smoke；最终总共 3,600 个计时（包含 smoke）。
6. build、submission、时限调整、报告 QA、归档与 cleanup 是以上实验的来源链；不能只列最终 results 一级目录，也不能把每个来源目录虚增成新算法。

`runs/cleanup-current-20261004/cleanup_summary.json` 记载删除 190 项、2,800 个文件、40,737,022 字节；脚本还主动从 active handoff 删掉旧事件。故 HEAD 一级目录与 active handoff 天生不是全历史索引。清理前 Git 历史由主审另外恢复，本文件不宣称已代替那部分。

## 2. Previous-review erratum：最终 FP32 实验已修正首触混杂

**撤回上一轮 `audit_projection.md:114` 的“最终 native FP32 使用串行 first touch”的归因。** 上一轮总报告第 9 行链接了该分审，因此此勘误也替代总审所引用分审的这句话；总报告自己的第 41 行仅列出表示/page 控制，没有直接写出错误首触断言。

准确来源链如下：

- `feature-precision-ablation-20261004-170251`：最早 build，递归清单中没有实际 graph run。
- `feature-precision-ablation-20261004-212957`：增加 CPU family/model143 与 ISA 检查；smoke `10867467` 三图通过。随后 diagnostic `10867476` 原生 FP32 直接读取串行生成的 H0；Products、Reddit 完成，Myciel 部分运行。
- `experiments/feature_precision_8aeef16/serial_input_confound_event.json` 明确将该轮标为 CANCELLED，原因是 native FP32 首触与 BF16 并行准备不同；Products native FP32 kernel 约 1100 ms，而 matched、并行展开输入约 240 ms。这个差异用于发现混杂，不能作改进加速比。
- 远端 `...212957/logs/submit-diagnostic-20261004-213212/slurm-10867476.err` 明确记载 `CANCELLED AT 2026-10-04T21:43:17`。同一 job 的 `event.json` 却是 PASS/exit0、completion空对象。这是事件状态不能独自作为完成证据的具体例子。应以取消事件、Slurm stderr 和 graph 完成覆盖一起判定：**被取消的部分诊断**。
- 最终 `feature-precision-ablation-20261004-214015`：smoke `10867490`、5 图 diagnostic `10867492`、12 图 extension `10868182`。

最终源码证据（均为 HEAD 可见文件）：

- `feature_precision_runtime.hpp:213` 分配 `prepared_fp32`；214–218 用 OpenMP static 做逐元素精确拷贝并 `memcmp` 检查。
- 同文件 289 行：`input=matched?q.p:prepared_fp32.p`。**native 用精确复制原始 FP32 的 prepared_fp32；matched 用 BF16 无损展开的 qFloat。它们是两份不同控制，不能混为一谈。**
- 同文件 221–222 行创建 qFloat；它控制输入数值匹配，不能用它取代 native 路径的语义。
- 同文件 315–328 行 preparation-inclusive 分支明确把 FP32 copy 或 BF16 conversion 计入对应准备边界。
- `PROTOCOL.md:23` 已写明这次修正；`feature_precision_job.sh:32–43` 检查单 socket、物理核数、SPR model143 与 ISA；没有 NUMA interleave。
- `results/feature-precision-20261004/raw_config.csv:2` 起全部 20 条记录（17 真实图+3 smoke）均为 `source_srand_12345_fp32_parallel_static_copy`。本轮又与远端20份真实stdout逐条确认。

滞后的文档是 `SOURCE_AUDIT.md:72` 和最终报告 `GCN_EXTRA_FEATURE_PRECISION_ABLATION.md:26`，仍描述 native H0 串行首触；上一轮跟着这两处文字，没有充分对照实际 runtime 和 raw config。**应修正文档/审阅结论，不能因此反过来否定已经实施的控制。**

剩余边界仍成立：同样的静态元素分区不保证物理页完全相同，H1 FP32 scatter 与 BF16 conversion 仍有页面分布差异；FP32 读512B/源行而 BF16读256B；固定四条 prefetch 对FP32只覆盖半行；native FP32还绕过层间BF16量化。因此修正首触后仍不能称为“纯decode成本实验”。

## 3. 本轮逐项核验的证据量

| 轮次 | 原始日志与导出表核验 | 本轮结果 |
|---|---|---|
| 原始 B 主实验 | 17 图正式 2,720 TIME；3个批次+smoke共3,040 TIME；每图 parsed 全部tag与stdout核对 | 0缺失、0 parsed不一致；正式1,020 CHECK与smoke120全pass |
| 连续计时控制 | 正式850 TIME；含smoke950；每图 parsed全部tag核对 | 0缺失、0 parsed不一致；正式612 CHECK与smoke72全pass |
| 初始 S/M | 从服务器17份methods.log重新提取，与原始reconciled2,720条逐字段比较 | 0缺失；1,020 CHECK、408 EQUIV、476 PROFILE_GATE全部pass |
| A3/P0 | 上一轮已完成2,890条全量对账；本轮补3次build之间的源码diff、smoke及run lineage | 没有发现新内核变更；修订为counter assertion/dependency、详细profile输出 |
| 最终 feature | 20份driver.stdout.log，3,600 TIME与GitHub raw_time.csv全量逐字段对账 | 0缺失；480 CHECK、120 EQ、160 PROFILE_GATE全pass；全部config确认FP32 parallel copy |

原始 B 主轮另外有 765 条正式阶段记录、5,712 条正式 sampled profile 记录；已自动核实导出表与实际stdout一致。这里不意味着人工解释了每条重复profile/PMU数值，也不把 sampled thread time 写成壁钟占比。

证据文件：`coverage_oct04_raw_verification.json`、`coverage_oct04_remote_raw_summary.json`、`coverage_oct04_remote_raw.json`。远端来源目录、事件、状态与全部源码差异分别在 `coverage_oct04_remote_lineage.json`、`coverage_oct04_remote_nested.json`、`coverage_oct04_remote_changes.json`。

## 4. 初始 S/M 与 A3 的完整关系

初始 S/M 两个 build 为161106、162142；smoke依次为10865904、10865927。原定独享formal10865930/10865931因用户转向feature被取消（`feature-precision-switch-20261004-164044/event.json`），不能把提交状态算完成结果。

之后用户允许共享节点并恢复计划：再次smoke10867477；focus10867479四图、validation10867699十三图；最终归档 `projection-window-reconciled-20261004-222427`。该轮测的是 S32/64/128 与 M64/256/FULL 的冻结子集及 B256控制，并有标准误差、相同M的逐位等价、插桩一致性、两种计时顺序、setup、MKL stage、PMU、线程尾部和结构采样。它远比“试几个B值”丰富。

A3单独建立215613、220554、221445三个study：215613无graph run；220554加入A1/A3真实逻辑计数一致性assertion及scheduler dependency，smoke10868124通过；221445补A3详细profile输出，smoke10868161、focus10868162、validation10868206。最终 `journal-a3-reconciled-20261004-230637` 才是GitHub `results/journal-p0-20261004` 的来源。A3保留S64边界但改row/window嵌套，实验含义不等于第一轮S/M重复。

## 5. 原始 B 与 cache 的来源边界

主构建123445；smoke10864873（N37/N137、空行、尾部和degree边界）；high10864874、medium10864875、low10864876。固定B2/4/8/16/32/64/FULL×FAST/ACCURATE共14候选，另加原TFS和源码MKL。四个数值边界、单层/完整两层、单独阶段与采样profile均已核driver。独立未修改paper binary作anchor；用于加速比的仍是同一process supplemental计时，不能拿anchor替换分母。

cache构建131154直接复制已验证kernel，只在runtime插入“每方法即时warmup+连续5次E2E”，提前return。smoke10864904；high10864906、medium10864907、low10864908。只含B8/16/64/FULL×两精度；没有重做主轮全部单层/阶段profile。133536事件仅把scheduler时限从2小时改30分钟，不是新算法或放宽数值门槛。

## 6. 根目录clean tar并非更多未见的新算法

本轮真正打开了 `evidence-current-controlled-20261004-clean.tar.gz`：SHA256与sidecar一致；1,198文件、12,675,398解压字节。1,185文件与HEAD逐字节相同；7个差异是README/contract/gitignore及4份handoff的时间版本；6个archive-only项是两个build指针、4个当时编译的binary。主/cache kernel与当前canonical源以及original均逐字节一致。

所以这个tar解决的是原始受控实验二进制与运行来源保存，不能把它当作清理前所有实验的历史备份。清理前失效/替代实验需另查Git父提交，不得以“tar看过了”替代。

## 7. 明确剩余工作

- 清理前源文件、旧协议与run由主审恢复核验；本子审不覆盖该部分。
- 最早两次S/M smoke已核协议、来源、状态、门槛计数，但没有人工逐行解释所有PMU/profile。
- 取消的feature10867476已核修正原因、per-graph完成、Slurm取消及source diff；未将其所有中断partial计时作正式性能分析，也不应这样做。
- 没有新跑benchmark或声明重现性能；全量“记录一致”与完整硬件因果解释是不同审阅深度。