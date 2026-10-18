# GCN-extra 实验覆盖补审与勘误（2026-10-18）

**对“所有实验都看完了”的回答：上一轮没有做到。** 上一轮覆盖了当前快照的主要实验族，深查了若干关键实现和结果，但把这表述成全历史审阅完成，超出了实际证据。用户指出从10月4日起有大量实验后，本轮重新按 Git 历史、服务器运行版本、失败/取消链和原始日志做了补审，并发现真实遗漏和一条需要撤回的技术判断。

本报告是 `GITHUB_DEEP_REVIEW_20261018.md` 及其附件的覆盖范围补充与勘误。旧报告保留历史原文，不应继续单独用于证明“全实验覆盖”。本轮没有运行性能内核、编译、提交集群任务、修改冻结实验结果或推送 GitHub。没有使用 NUMA interleave；128→128→128 仍是主比较形状，128→32→32 仅作为既有扩展实验记录。

## 1. 为什么上一轮会漏：当前目录不等于实验历史

当前固定 GitHub 提交为 `9369b9e0491d59e328a99153b6d97314c62285da`。上一轮确认的16,442文件与15个一级results目录，是**当前快照完整性**，不是历史实验完整性。

本轮重新查该路径19条Git提交，定位10月4日清理提交 `bdcf9b0`；从清理前 `8aeef1613fdeaeb929379874c4152eccde620f74` 恢复3,969文件，逐Git blob校验，0缺失、0不匹配。其中**2,789个路径已不在当前HEAD**，含早期block sweep、nozero/replay、25图覆盖、ILP64补充、旧源协议及论文V3复现。

服务器快照另有36,052文件、33个experiments直接目录、140个runs容器、17个results目录和438条事件（437个不同ID）。19个源码目录在GitHub被移到crosslayer归档。发布容器还重复存了13,308文件。因此，15、33、140、438都不能直接当成独立科学实验数量。

本次把33个源码目录加3个根源码实验族，建立**36个当前组织单元**的逐项定位；清理前历史另列11个实验/控制问题。组织单元、构建版本、Slurm提交、实际执行、控制实验、分析修订、论文结论分别计数，不把它们混在一起。

完整可导航清单见 [服务器覆盖台账](coverage-audit-20261018/coverage_server.md) 与同目录JSON；历史恢复见 [清理前实验补审](coverage-audit-20261018/coverage_history.md)。

## 2. 这轮实际补核到哪里

“已对账”指按method/graph/shape/repeat等键从原始日志重新提取，与导出表核对并按注明范围重算统计；不等于重新运行性能，也不等于已证明硬件因果。

| 时间/问题链 | 本轮实际审阅 | 证据与边界 |
|---|---|---|
| 10/3–4清理前：B/FULL、replay、nozero、25图及两张ILP64大图 | 阅读执行分支、6版冻结bench源码diff；13个formal/smoke执行共10,294条已测计时全对stdout，1,672组中位数一致；保留全空图失败与加权图拒绝 | [coverage_history](coverage-audit-20261018/coverage_history.md)。旧显式NUMA interleave结果不纳入现行主性能证据 |
| 10/4旧源协议与真正论文V3基线 | 旧源正式17图2,890行全部对stdout并复算578组min/median；论文V3的340条baseline计时全部独立解析原始日志，68组min/median一致 | 原source E2E只有0.01ms打印精度；不把独立anchor误当同轮分母 |
| 10/4原始主轮与cache控制 | 主轮正式2,720条、cache正式850条与原始日志核对；读原始anchor、计时边界、smoke、stage/profile及冻结源码关系 | [coverage_oct04](coverage-audit-20261018/coverage_oct04.md) |
| 10/4第一轮S/M与后来的A3/P0 | 初始S/M独立17图2,720条全对账；1,020标准gate、408同M逐位gate、476profile gate全通过；A3/P0的2,890条为另一轮，上轮已全核，本轮补build/smoke来源 | 不能拿A3数字替代初始S/M；两者问题设计不同 |
| 10/4 feature precision三版 | 查旧首触混杂、取消证据、最终runtime；最终17真实图+3smoke共3,600条TIME全对账；480 CHECK、120 EQ、160PROFILE_GATE全通过 | **撤回旧附件的首触断言**，见下节 |
| 10/5 P1/P2/P3/P4 | 继承上轮源码、数值反例、原始计时复核；本轮补P2分析v1/v2关系、residual失败launcher与selective版本演进 | [历史版本补审](coverage-audit-20261018/coverage_misc_versions.md)。P2-v2是AMX反汇编解码修复，同一份性能结果；不是另一轮更快的实验 |
| 10/7–8 grouping→order→state→pair→地址控制→factorial→prefetch→trace | 八族13run、62已运行job、247 graph/fixture packet；142份主CSV、37,110条计时逐字段对原始日志，0不匹配/遗漏；重算中位数，核失败版本与控制链 | [coverage_oct07](coverage-audit-20261018/coverage_oct07.md)。未逐指令重审所有反汇编、未重放所有真实CSR |
| 10/8后半跨层结构、重计算窗口、batch/parallel/adaptive与source partition | 补核52容器的实际日志及源码差异；106组结果、372对原始时间，198条result/gate转录对账；18组六样本按常规定义重算中位数 | [后半段版本补审](coverage-audit-20261018/coverage_late_versions.md)；赋值产生的fallback时间不当作独立候选执行 |
| 10/13–15 partial bundle、ordered source、H-panel、FULL Identity、oneMKL各路径 | 继承上轮逐方法/基线审阅及原始样本核对；本轮补服务器定位、旧版本和失败/取消链 | 含NUMA interleave的oneMKL分支仍仅保留历史，不用于用户约束下加速结论；见[历史版本补审](coverage-audit-20261018/coverage_misc_versions.md) |

原始计时数量用于表明核查范围，不相加包装成“总实验数”。smoke、正式、分析重导出与重复归档需分别处理。

## 3. 必须明确更正和补充的事实

### 3.1 最终FP32实验的首触控制已经实施，上一轮附件说错了

撤回旧 `review-20261018/audit_projection.md:114` 中“native FP32仍串行first touch”的断言。最终 `feature_precision_runtime.hpp:213–218` 已用OpenMP static将原始H0精确拷贝到prepared_fp32，289行实测native使用这份buffer；matched路径另用BF16无损展开的qFloat。20条raw config和20份stdout均确认 `source_srand_12345_fp32_parallel_static_copy`。

旧 `SOURCE_AUDIT.md:72` 和结果报告第26行没有同步更新。我上一轮依赖这两处文字，没有充分追到最终runtime和配置，这是实际审阅错误。早期有混杂的10867476确实被取消，但它的generic event却写PASS/exit0/completion空对象；本轮以Slurm取消日志、专门取消事件和逐图完成情况交叉判断，保留“取消的部分诊断”。

修正后仍有明确边界：FP32/BF16请求字节数、prefetch覆盖和层间量化不同，所以不能宣称已经隔离纯decode成本。不能用这些剩余边界否定已实施的首触控制。

### 3.2 replay不是只有代码，没有做过实验

早期冻结bench和实际日志已经测试replay_b8/replay_b32/replay_bfull及nozero变体：输出panel外循环重复聚合，换取AMX输出累加器驻留。后来正式paper_methods配置没有启用它，不代表项目从未测过。任何新的“重算换驻留”方案，需明确与该旧路径的粒度和数据生命周期差异。

### 3.3 访问/投影解耦已完成两轮有区别的研究

初始S/M轮冻结S32/64/128和M64/256/FULL子集，加B256控制；固定M改变S，检查相同投影边界下的逐位等价。A3另保留S64边界而改row/window循环次序，针对机制做对照。这不是“只有B64扫参”。

第一轮Mycielskian19中，FULL 2450.815ms、S64/MFULL 1084.969ms，但B64本已1120.540ms；因此修复FULL严重退化是有意义的机制证据，不能将相对慢FULL的2.26×说成对强B64同等幅度突破。该轮固定S64/MFULL整体对TFS GM1.3195，而B256为1.3263，支持继续研究机制而非宣布一个固定窗口已普遍最优。

### 3.4 10/7–8已经严格研究过“共享为什么没有转化成性能”

既有链条包含双行S64窗口驻留、动态head合并、静态精确窗口配对、固定地址scratch、三种预定随机种子、相同预取目标/次数的发出时机控制、离线2bit消费事件和三个replay执行器。它们控制过pair成员、地址、CSR行内加法顺序、预取次数、dispatch对象、精度语义和PACK_EPI强基线。

PAIR_SHARE相对PACK_EPI约1.0×；PF8推广候选诊断GM约1.008–1.015，未过门槛；trace可小幅胜动态匹配，却落后PACKdegree/PACKqguard。不能再把这些整体当成从未尝试，也不能拿其中较弱分母的收益代替对强基线的收益。

双行驻留的未解边界更具体：已有acc0[8]/acc1[8]在S64窗口内驻留，但跨窗口仍写回partial。无head比较的独立全行轮转、减少公共子和加法次数、受容量限制的投影结果生产/消费，才需要分别检验其与旧实现的区别；这些仍是候选，不是已验证突破。

### 3.5 早期基线纠正和失败版本影响结论，不只是工程杂事

旧source协议对齐的是 `gcn_e2e_bench.cpp`，其17图min5 TFS/MKL GM0.89133；后来定位论文Table2对应 `gcn_e2e_v3.cpp`，真正V3复现为GM1.90616。两者实现和运行批次不同，旧0.891不能用于评价论文TFS；这也不是可相除为优化加速比的单变量实验。

state-reuse经历编译宏失败、两次subnormal语义失败和一次NaN语义失败，最终局部精确浮点语义处理后才测性能。这里没有放宽门槛。Friendster/Road-USA也经历LP64失败、ILP64成功再被源忠实协议排除；不能说它们从未做过，也不能把旧ILP64结果悄悄并入当前17图。

### 3.6 “失败/重试”必须细分到实际发生的位置

- P3的112010版本因preflight覆盖shell的RUN变量，拼出logs/logs路径而失败，内核没有运行。修复launcher后的112212才成功验证D1严格FP32门槛失败但最终BF16相同；实验成功复现反例，不等于接受D1。
- P4的122940→123418新增 `project_used_L1_full_L2` 强对照，并改回原源码随机输入及W1/W2生成次序，不是仅重新编译。旧270条小图样本和新1000条跨形状样本不能混成同一个版本。
- FULL DegreeSort的10968050在affinity检查阶段失败；10968053已完成tail37的正确性和16条计时，之后因节点缺rg而失败。前者没有kernel样本，后者是有效单图结果加批次后处理失败。
- packed oneMKL首次只是三处枚举名编译错误；10973279经sacct确认等待中取消，Start=None、Elapsed=0，没有性能样本。
- 早期两次S/M smoke已进一步核实际stdout：各180标准gate、72等价gate、84profile gate全部通过，关闭了早一份oct04分审中仅核状态/计数的缺口。重复PMU的硬件解释仍未宣称完成。
- recompute-window-01实际成功，03→04才修正再生副本的存活起点。RoadNet W=1峰值从30,034变为2,539，是模型计数修复，不能解释为执行内核省了相同倍数内存；行/列度与单位问题仍需保留。
- batch两次外层FAILED保存了完整正确性通过的raw。旧附件把它直接归因为预期计数错，本轮可独立确认的是外层状态与完整raw不一致；未找到完整外层验收脚本，具体阈值不能当成已核实的事实。

## 4. 覆盖标签与仍然不能声称的事情

每份分审都区分：

- **已定位**：目录、事件、日期、提交和归档位置已登记。
- **已追执行**：核对冻结源码、launcher、实际配置、门槛与错误日志；不只看README。
- **已核原始结果**：按照该分审列出的范围，对stdout、CSV、统计值进行复算。
- **尚未独立验证**：庞大反汇编逐指令审计、真实CSR结构统计重放、完整PMU因果分析或新性能复现。

定向文件内容核查覆盖856份文本，827份在HEAD找到逐字相同副本，29份没有。该差集有完整路径和SHA256。全服务器文本hash尝试超时，**没有完成全量内容哈希**；这不影响已完成的目标文件核验，也不能被省略成“服务器与GitHub完全一致”。

最早smoke重复PMU、被取消任务中断后的每一条partial计时、所有反汇编和每个未运行的编辑状态，没有被冒称逐行人工看完。现有证据可以核实“做了什么、真正跑了什么、哪些门槛失败、哪些数据支持哪些结论”，但不能保证每个硬件机制归因都已证明。

明确的缺失材料包括 `recompute-window-interleave-build-20261008-233639` 的build.log（本地与服务器均仅12文件，无法确认具体失败原因），以及部分旧失败launcher/外层验收脚本的完整冻结副本。它们已进入对应台账的remaining字段，不会用后续成功版本替代。

## 5. 后续交接规则

1. 新想法先对照本覆盖台账，指出与已有runtime的具体差异，不能仅按题目/目录名判断“没做过”。
2. 每个实验按问题→冻结源码→实际运行→原始结果→失败/替代关系登记；失败、取消和未运行都保留。
3. 凡旧说明与实际runtime/config冲突，以实际执行来源为准并记勘误；不能把旧说明当最终事实。
4. 正式候选保留FULL Identity、PACK_EPI及适用的project_all/project_used/mixed强基线。新收益不得借用旧较弱分母，NUMA interleave不进入合规比较。
5. 本轮状态是**覆盖补审与更正**，不构成新性能突破或期刊创新已获验证的声明。

服务器保存：`GCN-extra/docs/handoff/EXPERIMENT_COVERAGE_AUDIT_20261018.md`；附件目录 `GCN-extra/docs/handoff/coverage-audit-20261018/`。CURRENT_HANDOFF和三个持久台账将引用本报告，旧审阅原文与冻结实验保持不变。
